package com.elleann.android.data

import android.content.Context
import androidx.core.content.edit
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey
import com.elleann.android.AppContainer
import com.elleann.android.StoredToken
import com.elleann.android.TokenStore
import com.elleann.android.data.ElleProfileStore
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
    val profileStore  = ElleProfileStore(context, encryptedPrefs)

    private val json = Json {
        ignoreUnknownKeys = true; isLenient = true
        coerceInputValues = true; encodeDefaults = true
    }

    private val loggingInterceptor = RedactingLoggingInterceptor()

    private val authInterceptor = AuthInterceptorExtended(
        tokenStore       = baseContainer.tokenStore,
        adminKeyStore    = adminKeyStore,
        onReauthRequired = onReauthRequired,
    )

    private val okHttpClient = OkHttpClient.Builder()
        .addInterceptor(authInterceptor)
        .addInterceptor(loggingInterceptor)
        .connectTimeout(15, TimeUnit.SECONDS)
        .readTimeout(30, TimeUnit.SECONDS)
        .writeTimeout(30, TimeUnit.SECONDS)
        .build()

    // Use this for protected server file downloads, including Elle's locked
    // profile picture after uninstall/reinstall. rawHttpClient intentionally
    // has no auth headers and may fail against /api/video/* file endpoints.
    val authenticatedHttpClient: OkHttpClient = okHttpClient

    val rawHttpClient: OkHttpClient = OkHttpClient.Builder()
        .addInterceptor(loggingInterceptor)
        .connectTimeout(15, TimeUnit.SECONDS)
        .readTimeout(120, TimeUnit.SECONDS)
        .writeTimeout(30, TimeUnit.SECONDS)
        .build()

    // Cached Retrofit instance — only rebuild when host/port changes
    private var _cachedApi: ElleApiExtended? = null
    private var _cachedApiKey: String = ""

    fun getApi(): ElleApiExtended? {
        val stored = tokenStore.load()
        val host = stored?.host?.takeIf { it.isNotBlank() } ?: return null
        val port = stored.port.takeIf { it > 0 } ?: return null
        val key = "$host:$port"
        if (_cachedApi == null || _cachedApiKey != key) {
            _cachedApi = createApi(host, port)
            _cachedApiKey = key
        }
        return _cachedApi!!
    }

    private fun createApi(host: String, port: Int): ElleApiExtended {
        return Retrofit.Builder()
            .baseUrl("http://$host:$port/")
            .client(okHttpClient)
            .addConverterFactory(json.asConverterFactory("application/json".toMediaType()))
            .build()
            .create(ElleApiExtended::class.java)
    }

    val extendedApi: ElleApiExtended get() = getApi() ?: throw IllegalStateException("Not signed in — call getApi() only when isPaired is true")
    val adminApi: ElleApiExtended? get() = if (isPaired) getApi() else null
    fun pairedExtendedApi(): ElleApiExtended? = if (isPaired) getApi() else null

    // Real pairing check — both JWT and host must be present
    val isPaired: Boolean get() {
        val stored = tokenStore.load() ?: return false
        return stored.jwt.isNotBlank() && stored.host.isNotBlank() && stored.port > 0
    }

    val restBaseUrl: String?
        get() {
            val stored = tokenStore.load() ?: return null
            val host = stored.host.takeIf { it.isNotBlank() } ?: return null
            val port = stored.port.takeIf { it > 0 } ?: return null
            return "http://$host:$port"
        }

    private var _webSocket: ElleWebSocket? = null
    val webSocketOrNull: ElleWebSocket? get() = _webSocket
    val isWebSocketInitialized: Boolean get() = _webSocket != null

    fun initWebSocket() {
        val stored = tokenStore.load() ?: return
        val host = stored.host.takeIf { it.isNotBlank() } ?: return
        val port = stored.port.takeIf { it > 0 } ?: return

        _webSocket?.disconnect()
        _webSocket = ElleWebSocket(
            host   = host,
            port   = port,
            jwt    = stored.jwt,
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
            jwt  = current?.jwt ?: "",
            host = host,
            port = port,
        ))
        // Invalidate cached API so next call rebuilds with new coords
        _cachedApi = null
        _cachedApiKey = ""
    }

    // Store JWT after successful login
    fun onLoginSuccess(jwt: String, host: String, port: Int) {
        tokenStore.save(StoredToken(jwt = jwt, host = host, port = port))
        _cachedApi = null
        _cachedApiKey = ""
    }

    fun logout() {
        tokenStore.clear()
        _cachedApi = null
        _cachedApiKey = ""
        disconnectWebSocket()
    }

    @Suppress("UNUSED_PARAMETER")
    fun getPairingQrSvg(code: String, host: String, port: Int): String? = null

    companion object {
        private const val KEY_DEV_PIN_HASH = "elle_dev_pin_hash"
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
