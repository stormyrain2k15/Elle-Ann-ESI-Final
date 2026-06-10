package com.elleann.android

import android.app.Application
import android.content.Context
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey
import com.jakewharton.retrofit2.converter.kotlinx.serialization.asConverterFactory
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json
import okhttp3.MediaType.Companion.toMediaType
import com.elleann.android.data.RedactingLoggingInterceptor
import okhttp3.OkHttpClient
import retrofit2.Retrofit
import java.util.concurrent.TimeUnit

@Serializable
data class StoredToken(
    val jwt: String,
    val host: String,
    val port: Int,
    val expiresMs: Long = Long.MAX_VALUE,
)

@Serializable
data class EmotionsResponse(
    val valence: Float? = null,
    val arousal: Float? = null,
    val dominance: Float? = null,
    val mood: String? = null,
    val joy: Float? = null,
    val sadness: Float? = null,
    val anger: Float? = null,
    val fear: Float? = null,
    val disgust: Float? = null,
    val trust: Float? = null,
    val surprise: Float? = null,
    val anticipation: Float? = null,
)

@Serializable
data class PairResponse(
    @kotlinx.serialization.SerialName("token") val jwt: String,
    @kotlinx.serialization.SerialName("expires_ms")  val expiresMs:  Long = Long.MAX_VALUE,
    @kotlinx.serialization.SerialName("created_ms")  val pairedAtMs: Long = 0,
    @kotlinx.serialization.SerialName("nUserNo")     val nUserNo:    Int? = null,
    @kotlinx.serialization.SerialName("sUserID")     val sUserID:    String? = null,
    @kotlinx.serialization.SerialName("sUserName")   val sUserName:  String? = null,
    @kotlinx.serialization.SerialName("nAuthID")     val nAuthID:    Int? = null,
)

class TokenStore(context: Context) {
    companion object {
        private const val PREFS_NAME = "elle_secure_prefs"
        private const val KEY_TOKEN  = "elle_jwt"
    }

    private val json = Json { ignoreUnknownKeys = true; isLenient = true }

    private val prefs: android.content.SharedPreferences by lazy {
        val masterKey = MasterKey.Builder(context)
            .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
            .build()
        EncryptedSharedPreferences.create(
            context, PREFS_NAME, masterKey,
            EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
            EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM,
        )
    }

    fun save(token: StoredToken) =
        prefs.edit().putString(KEY_TOKEN, json.encodeToString(StoredToken.serializer(), token)).apply()

    fun load(): StoredToken? = runCatching {
        val raw = prefs.getString(KEY_TOKEN, null) ?: return null
        json.decodeFromString(StoredToken.serializer(), raw)
    }.getOrNull()

    fun clear() = prefs.edit().remove(KEY_TOKEN).apply()
}

interface ElleApi {
    @retrofit2.http.GET("/api/health")
    suspend fun health(): Map<String, String>

    @retrofit2.http.GET("/api/emotions")
    suspend fun emotions(): EmotionsResponse

    @retrofit2.http.POST("/api/auth/login")
    suspend fun login(@retrofit2.http.Body body: com.elleann.android.data.models.LoginRequest): PairResponse
}

class AppContainer(context: Context) {
    val tokenStore = TokenStore(context)

    private val json = Json {
        ignoreUnknownKeys = true; isLenient = true
        coerceInputValues = true; encodeDefaults = true
    }

    val baseOkHttpClient: OkHttpClient = OkHttpClient.Builder()
        .addInterceptor(RedactingLoggingInterceptor())
        .connectTimeout(10, TimeUnit.SECONDS)
        .readTimeout(20, TimeUnit.SECONDS)
        .build()

    fun apiFor(host: String, port: Int): ElleApi {
        return Retrofit.Builder()
            .baseUrl("http://$host:$port/")
            .client(baseOkHttpClient)
            .addConverterFactory(json.asConverterFactory("application/json".toMediaType()))
            .build()
            .create(ElleApi::class.java)
    }

    val api: ElleApi?
        get() = tokenStore.load()?.let { apiFor(it.host, it.port) }
}

class ElleApp : Application() {
    lateinit var container: AppContainer
        private set
    lateinit var chatCacheManager: com.elleann.android.data.ChatCacheManager
        private set

    override fun onCreate() {
        super.onCreate()
        container = AppContainer(applicationContext)

        chatCacheManager = com.elleann.android.data.ChatCacheManager(applicationContext)
        com.elleann.android.data.ChatCacheManager.installAsGlobal(chatCacheManager)

        // Flush cache on crash
        val previousHandler = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            runCatching { com.elleann.android.data.ChatCacheManager.crashFlush() }
            previousHandler?.uncaughtException(thread, throwable) ?: run {
                android.os.Process.killProcess(android.os.Process.myPid())
                kotlin.system.exitProcess(10)
            }
        }

        // Flush cache when app goes to background (user closes app or switches away)
        // This is the CORRECT place to flush — not on page navigation
        androidx.lifecycle.ProcessLifecycleOwner.get().lifecycle.addObserver(
            object : androidx.lifecycle.DefaultLifecycleObserver {
                override fun onStop(owner: androidx.lifecycle.LifecycleOwner) {
                    runCatching { chatCacheManager.flushAllSync() }
                }
            }
        )
    }
}
