package com.elleann.android.data

import com.elleann.android.TokenStore
import okhttp3.Interceptor
import okhttp3.Response

class AuthInterceptorExtended(
    private val tokenStore: TokenStore,
    @Suppress("UNUSED_PARAMETER") adminKeyStore: AdminKeyStore? = null,
    private val onReauthRequired: () -> Unit,
) : Interceptor {

    override fun intercept(chain: Interceptor.Chain): Response {
        val requestBuilder = chain.request().newBuilder()

        return chain.proceed(requestBuilder.build())
    }
}

@Suppress("UNUSED_PARAMETER")
class AdminKeyStore(prefs: android.content.SharedPreferences? = null) {
    fun getKey(): String = ""
    fun setKey(key: String) { }
    fun clearKey() { }
    fun hasKey(): Boolean = false
}
