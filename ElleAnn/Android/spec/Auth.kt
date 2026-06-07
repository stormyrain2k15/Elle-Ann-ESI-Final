package com.elleann.android.data.spec

object Auth {

    data class PairRequest(
        val code:         String,
        val device_name:  String,
        val device_id:    String,
    )

    data class PairResponse(
        val jwt:          String,
        val expires_ms:   Long,
        val paired_at_ms: Long,
    )

    data class StoredToken(
        val jwt:        String,
        val expires_ms: Long,
    )

    interface TokenStore {
        fun save(token: StoredToken)
        fun load(): StoredToken?
        fun clear()
    }

}
