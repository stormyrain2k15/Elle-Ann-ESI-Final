package com.elleann.android.data.spec

object SyncPolicy {

    const val STATE_REFRESH_ON_FOREGROUND: Boolean = true
    const val STATE_REFRESH_ON_WS_EVENT:   Boolean = true
    const val STATE_POLL_INTERVAL_MS:      Long    = 0L

    const val MODULATION_POLL_WHILE_VISIBLE_MS: Long = 60_000L

    const val HISTORY_REFRESH_INTERVAL_MS: Long = 5 * 60_000L

    const val FERTILITY_REFRESH_ON_COLD_START: Boolean = true

    val RETRY_BACKOFF_MS: List<Long> = listOf(0L, 1_000L, 3_000L)

    const val MAX_CONCURRENT_REQUESTS: Int = 4

    const val OFFLINE_WRITE_QUEUE: Boolean = false

}
