package com.elleann.android.data.spec

sealed class WsEvent {

    data class Hello(val ts_ms: Long, val server_version: String) : WsEvent()

    data class PhaseTransition(
        val ts_ms:     Long,
        val from:      String,
        val to:        String,
        val cycle_day: Int,
    ) : WsEvent()

    data class LHSurge(
        val ts_ms:    Long,
        val lh_value: Double,
        val cycle_day: Int,
    ) : WsEvent()

    data class LaborStage(
        val ts_ms:          Long,
        val stage:          String,
        val contractions_per_10min: Int = 0,
    ) : WsEvent()

    data class Birth(
        val ts_ms:         Long,
        val pregnancy_id:  Long,
        val multiplicity:  Int,
    ) : WsEvent()

    data class Miscarriage(
        val ts_ms:         Long,
        val pregnancy_id:  Long,
        val gestational_day: Int,
    ) : WsEvent()

    data class HormoneTick(
        val ts_ms:     Long,
        val hormones:  HormoneVector,
        val cycle_phase: String,
    ) : WsEvent()
}

object WsReconnectPolicy {
    const val INITIAL_BACKOFF_MS: Long = 1_000
    const val MAX_BACKOFF_MS:     Long = 30_000
    const val BACKOFF_FACTOR:     Double = 2.0
    const val CLOSE_CODE_PAIRING_REVOKED: Int = 4011
}
