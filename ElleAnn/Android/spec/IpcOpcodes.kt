package com.elleann.android.data.spec

object IpcOpcodes {

    const val STATE_QUERY:          Int = 2200
    const val HISTORY_QUERY:        Int = 2201
    const val ANCHOR:               Int = 2202
    const val STIMULUS:             Int = 2203
    const val MODULATION_QUERY:     Int = 2204
    const val CONCEPTION_ATTEMPT:   Int = 2205
    const val DELIVER:              Int = 2206
    const val RESPONSE:             Int = 2207
    const val CONTRACEPTION_SET:    Int = 2208
    const val LIFECYCLE_SET:        Int = 2209
    const val SYMPTOM_LOG:          Int = 2210
    const val SYMPTOM_QUERY:        Int = 2211
    const val PREG_EVENTS_QUERY:    Int = 2212
    const val ACCELERATE:           Int = 2213

    const val HORMONE_UPDATE:       Int = 2220
    const val PHASE_TRANSITION:     Int = 2221
    const val BIRTH:                Int = 2222
    const val LH_SURGE:             Int = 2223
    const val LABOR_STAGE:          Int = 2224
    const val MISCARRIAGE:          Int = 2225

    fun broadcastTypeFor(opcode: Int): String? = when (opcode) {
        HORMONE_UPDATE   -> "hormone_tick"
        PHASE_TRANSITION -> "phase_transition"
        BIRTH            -> "birth"
        LH_SURGE         -> "lh_surge"
        LABOR_STAGE      -> "labor_stage"
        MISCARRIAGE      -> "miscarriage"
        else             -> null
    }
}
