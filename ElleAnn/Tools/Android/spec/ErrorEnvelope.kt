package com.elleann.android.data.spec

data class ErrorEnvelope(
    val error:   String,
    val details: String? = null,
)

enum class ErrorDisposition {

    USER_FIXABLE,

    REAUTH_REQUIRED,

    TRANSIENT_BACKOFF,

    CLIENT_BUG;

    companion object {
        fun fromStatus(code: Int): ErrorDisposition = when (code) {
            400, 409    -> USER_FIXABLE
            401         -> REAUTH_REQUIRED
            403         -> CLIENT_BUG
            404         -> CLIENT_BUG
            429, 500, 503 -> TRANSIENT_BACKOFF
            else        -> if (code in 500..599) TRANSIENT_BACKOFF else CLIENT_BUG
        }
    }
}
