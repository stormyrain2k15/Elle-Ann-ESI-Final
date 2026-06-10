package com.elleann.android.data

import android.content.SharedPreferences
import com.elleann.android.TokenStore
import okhttp3.Interceptor
import okhttp3.Response
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey

class AuthInterceptorExtended(
    private val tokenStore: TokenStore,
    private val adminKeyStore: AdminKeyStore? = null,
    private val onReauthRequired: () -> Unit,
) : Interceptor {

    override fun intercept(chain: Interceptor.Chain): Response {
        val stored = tokenStore.load()
        val requestBuilder = chain.request().newBuilder()

        // Attach JWT if we have one
        if (stored?.jwt?.isNotBlank() == true) {
            requestBuilder.addHeader("Authorization", "Bearer ${stored.jwt}")
        }

        val adminKey = adminKeyStore?.getKey().orEmpty()
        if (adminKey.isNotBlank()) {
            requestBuilder.addHeader("x-admin-key", adminKey)
        }

        val response = chain.proceed(requestBuilder.build())

        // On 401, the token is expired or invalid — trigger reauth
        if (response.code == 401) {
            tokenStore.clear()
            onReauthRequired()
        }

        return response
    }
}

class AdminKeyStore(private val prefs: SharedPreferences? = null) {

    companion object {
        private const val KEY_ADMIN_KEY = "elle_admin_key"
    }

    fun getKey(): String = prefs?.getString(KEY_ADMIN_KEY, "") ?: ""

    fun setKey(key: String) {
        prefs?.edit()?.putString(KEY_ADMIN_KEY, key)?.apply()
    }

    fun clearKey() {
        prefs?.edit()?.remove(KEY_ADMIN_KEY)?.apply()
    }

    fun hasKey(): Boolean = prefs?.getString(KEY_ADMIN_KEY, null)?.isNotBlank() == true
}
