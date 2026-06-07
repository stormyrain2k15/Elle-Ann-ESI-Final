package com.elleann.android.data

import android.util.Log
import okhttp3.Interceptor
import okhttp3.Response

class RedactingLoggingInterceptor : Interceptor {

    private val REDACTED_HEADERS = setOf(
        "authorization",
        "x-admin-key",
        "cookie",
        "set-cookie",
    )

    private val isDebug: Boolean = try {
        Class.forName("com.elleann.android.BuildConfig")
            .getField("DEBUG")
            .getBoolean(null)
    } catch (_: Exception) { false }

    override fun intercept(chain: Interceptor.Chain): Response {
        val req = chain.request()
        if (isDebug) {
            Log.d("ElleHttp", "--> ${req.method} ${req.url}")
            req.headers.names().forEach { name ->
                val display = if (name.lowercase() in REDACTED_HEADERS) "<REDACTED>" else "(present)"
                Log.d("ElleHttp", "    $name: $display")
            }
        }
        val resp = chain.proceed(req)
        if (isDebug) {
            Log.d("ElleHttp", "<-- ${resp.code} ${req.url}")
        }
        return resp
    }
}
