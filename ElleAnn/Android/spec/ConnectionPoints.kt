package com.elleann.android.data.spec

object ConnectionPoints {

    const val DEFAULT_PORT: Int = 8000

    const val DEFAULT_BIND_ADDRESS: String = "127.0.0.1"

    enum class Scheme { HTTP, HTTPS }

    const val WS_PATH_DEFAULT: String = "/command"

    const val AUTH_HEADER: String = "Authorization"
    const val AUTH_SCHEME: String = "Bearer"

    const val X_API_PREFIX: String = "/api/x"

    object CrossCutting {

        const val HEALTH: String = "/api/health"

        const val HEALTHZ: String = "/healthz"

        const val SESSION_GREETING: String = "/api/session/greeting"
    }

}
