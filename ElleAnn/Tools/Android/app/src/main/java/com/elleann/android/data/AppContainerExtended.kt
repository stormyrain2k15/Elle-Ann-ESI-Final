package com.elleann.android.data

import android.content.Context
import androidx.core.content.edit
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey
import com.elleann.android.AppContainer
import com.elleann.android.StoredToken
import com.elleann.android.TokenStore
import com.jakewharton.retrofit2.converter.kotlinx.serialization.asConverterFactory
import kotlinx.serialization.json.Json
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import retrofit2.Retrofit
import java.util.concurrent.TimeUnit

class AppContainerExtended(
    context: Context,
    val baseContainer: AppContainer,
    private val onReauthRequired: () -> Unit,
) {
    private val masterKey = MasterKey.Builder(context)
        .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
        .build()

    private val encryptedPrefs = EncryptedSharedPreferences.create(
        context, "elle_secure_prefs", masterKey,
        EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
        EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM,
    )

    val tokenStore: TokenStore get() = baseContainer.tokenStore
    val adminKeyStore = AdminKeyStore(encryptedPrefs)

    private val json = Json {
        ignoreUnknownKeys = true; isLenient = true
        coerceInputValues = true; encodeDefaults = true
    }

    private val loggingInterceptor = RedactingLoggingInterceptor()

    private val authInterceptor = AuthInterceptorExtended(
        tokenStore    = baseContainer.tokenStore,
        adminKeyStore = adminKeyStore,
        onReauthRequired = onReauthRequired,
    )

    private val okHttpClient = OkHttpClient.Builder()
        .addInterceptor(authInterceptor)
        .addInterceptor(loggingInterceptor)
        .connectTimeout(15, TimeUnit.SECONDS)
        .readTimeout(30, TimeUnit.SECONDS)
        .writeTimeout(30, TimeUnit.SECONDS)
        .build()

    val rawHttpClient: OkHttpClient = OkHttpClient.Builder()
        .addInterceptor(loggingInterceptor)
        .connectTimeout(15, TimeUnit.SECONDS)
        .readTimeout(120, TimeUnit.SECONDS)
        .writeTimeout(30, TimeUnit.SECONDS)
        .build()

    private var _webSocket: ElleWebSocket? = null
    val webSocketOrNull: ElleWebSocket? get() = _webSocket
    val isWebSocketInitialized: Boolean get() = _webSocket != null

    fun getApi(): ElleApiExtended {
        val stored = tokenStore.load()
        val host = stored?.host?.takeIf { it.isNotBlank() } ?: DEFAULT_HOST
        val port = stored?.port?.takeIf { it > 0 } ?: DEFAULT_PORT

        return Retrofit.Builder()
            .baseUrl("http://$host:$port/")
            .client(okHttpClient)
            .addConverterFactory(json.asConverterFactory("application/json".toMediaType()))
            .build()
            .create(ElleApiExtended::class.java)
    }

    val extendedApi: ElleApiExtended get() = getApi()

    val adminApi: ElleApiExtended? get() = extendedApi

    fun pairedExtendedApi(): ElleApiExtended? = getApi()

    val isPaired: Boolean get() = true

    val restBaseUrl: String?
        get() {
            val stored = tokenStore.load()
            val host = stored?.host?.takeIf { it.isNotBlank() } ?: DEFAULT_HOST
            val port = stored?.port?.takeIf { it > 0 } ?: DEFAULT_PORT
            return "http://$host:$port"
        }

    fun initWebSocket() {
        val stored = tokenStore.load()
        val host = stored?.host?.takeIf { it.isNotBlank() } ?: DEFAULT_HOST
        val port = stored?.port?.takeIf { it > 0 } ?: DEFAULT_PORT

        _webSocket?.disconnect()
        _webSocket = ElleWebSocket(
            host   = host,
            port   = port,
            jwt    = stored?.jwt ?: "",
            client = okHttpClient,
        )
        _webSocket?.connect()
    }

    fun reconnectWebSocketIfNeeded() {
        if (_webSocket == null) {
            initWebSocket()
        } else if (!_webSocket!!.isConnected()) {
            _webSocket!!.connect()
        }
    }

    fun disconnectWebSocket() {
        _webSocket?.disconnect()
        _webSocket = null
    }

    fun setServerCoords(host: String, port: Int) {
        val current = tokenStore.load()
        tokenStore.save(StoredToken(
            jwt = current?.jwt ?: "",
            host = host,
            port = port
        ))
    }

    @Suppress("UNUSED_PARAMETER")
    fun getPairingQrSvg(code: String, host: String, port: Int): String? = null

    companion object {
        private const val KEY_DEV_PIN_HASH = "elle_dev_pin_hash"

        const val DEFAULT_HOST = "158.62.137.73"
        const val DEFAULT_PORT = 8000
    }

    fun setDevPin(pin: String) =
        encryptedPrefs.edit { putString(KEY_DEV_PIN_HASH, sha256(pin)) }

    fun verifyDevPin(pin: String): Boolean {
        val stored = encryptedPrefs.getString(KEY_DEV_PIN_HASH, null) ?: return false
        return stored == sha256(pin)
    }

    fun hasDevPin(): Boolean = encryptedPrefs.contains(KEY_DEV_PIN_HASH)

    private fun sha256(input: String): String {
        val digest = java.security.MessageDigest.getInstance("SHA-256")
        return digest.digest(input.toByteArray(Charsets.UTF_8))
            .joinToString("") { "%02x".format(it) }
    }
}
