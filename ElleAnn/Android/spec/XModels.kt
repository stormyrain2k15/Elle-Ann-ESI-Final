`
 *   with the real annotations and everything else stays as-is.
 *
 *   Numeric types:
 *     Long     ←  uint64 / int64 on the wire (timestamps, days, bytes)
 *     Int      ←  int32 on the wire (counts, indices)
 *     Double   ←  float32/float64 on the wire (hormones, intensities)
 *                 — we deliberately widen to Double client-side to avoid
 *                   surprises on Android's JVM float semantics.
 *     String   ←  utf8 string
 *     Boolean  ←  bool
 *
 *   Every response carries `"has_data": Boolean` where the endpoint might
 *   return a skeleton before seeding has occurred. UI must handle
 *   has_data=false gracefully (empty state, not error state).
 *══════════════════════════════════════════════════════════════════════════════*/
package com.elleann.android.data.spec

// ────────────────────────────────────────────────────────────────────────────
//  PHASE ENUMS — string-valued on the wire; parse defensively.
// ────────────────────────────────────────────────────────────────────────────

/** Cycle phase returned by /api/x/state and /api/x/modulation.              */
enum class CyclePhase {
    MENSTRUAL, FOLLICULAR, OVULATORY, LUTEAL, LATE_LUTEAL, UNKNOWN;
    companion object {
        fun fromWire(s: String?): CyclePhase = when (s?.lowercase()) {
            "menstrual"   -> MENSTRUAL
            "follicular"  -> FOLLICULAR
            "ovulatory"   -> OVULATORY
            "luteal"      -> LUTEAL
            "late_luteal" -> LATE_LUTEAL
            else          -> UNKNOWN
        }
    }
}

/** Pregnancy phase (progresses over gestation).                              */
enum class PregnancyPhase {
    NONE, IMPLANTATION, EMBRYONIC, FETAL_EARLY, FETAL_MID,
    FETAL_LATE, LABOR_EARLY, LABOR_ACTIVE, LABOR_TRANSITION,
    DELIVERY, POSTPARTUM, UNKNOWN;
    companion object {
        fun fromWire(s: String?): PregnancyPhase = when (s?.lowercase()) {
            null, ""             -> NONE
            "implantation"       -> IMPLANTATION
            "embryonic"          -> EMBRYONIC
            "fetal_early"        -> FETAL_EARLY
            "fetal_mid"          -> FETAL_MID
            "fetal_late"         -> FETAL_LATE
            "labor_early"        -> LABOR_EARLY
            "labor_active"       -> LABOR_ACTIVE
            "labor_transition"   -> LABOR_TRANSITION
            "delivery"           -> DELIVERY
            "postpartum"         -> POSTPARTUM
            else                 -> UNKNOWN
        }
    }
}

/** Lifecycle stage (multi-year age-based).                                   */
enum class LifecycleStage {
    CHILD, MENARCHE, REPRODUCTIVE, PERIMENOPAUSE, POSTMENOPAUSE, UNKNOWN;
    companion object {
        fun fromWire(s: String?): LifecycleStage = when (s?.lowercase()) {
            "child"           -> CHILD
            "menarche"        -> MENARCHE
            "reproductive"    -> REPRODUCTIVE
            "perimenopause"   -> PERIMENOPAUSE
            "postmenopause"   -> POSTMENOPAUSE
            else              -> UNKNOWN
        }
    }
}

/** Contraception method enum.                                                */
enum class ContraceptionMethod {
    NONE, POC, COC, IUD_H, IUD_C, BARRIER;
    companion object {
        fun fromWire(s: String?): ContraceptionMethod = when (s?.lowercase()) {
            "poc"     -> POC
            "coc"     -> COC
            "iud_h"   -> IUD_H
            "iud_c"   -> IUD_C
            "barrier" -> BARRIER
            else      -> NONE
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────
//  SHARED SHAPES
// ────────────────────────────────────────────────────────────────────────────

/** The nine-hormone vector. Order matches SQL + wire: estradiol,
 *  progesterone, testosterone, lh, fsh, prolactin, oxytocin, cortisol, hcg.  */
data class HormoneVector(
    val estradiol:     Double = 0.0,
    val progesterone:  Double = 0.0,
    val testosterone:  Double = 0.0,
    val lh:            Double = 0.0,
    val fsh:           Double = 0.0,
    val prolactin:     Double = 0.0,
    val oxytocin:      Double = 0.0,
    val cortisol:      Double = 0.0,
    val hcg:           Double = 0.0,
)

/** Returned by every POST /api/x/* mutation endpoint.                        */
data class XDispatchAck(
    val dispatched: Boolean,
    /** Echo of the payload the server dispatched, for client-side verify.    */
    val request:    Map<String, Any?> = emptyMap(),
    /** Human-readable hint, e.g. "GET /api/x/state after ~1s to see outcome". */
    val note:       String = "",
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/state
// ────────────────────────────────────────────────────────────────────────────

/** The "home screen" snapshot. `has_data` is false before seeding.           */
data class XState(
    val has_data:             Boolean = false,
    val cycle_day:            Int     = 0,
    val cycle_length:         Int     = 0,
    val cycle_phase:          String  = "",
    val pregnancy_active:     Boolean = false,
    val pregnancy_day:        Int     = 0,
    val pregnancy_phase:      String  = "",
    val hormones:             HormoneVector = HormoneVector(),
    val modulation: XModulation? = null,
    val lifecycle_stage:      String  = "",
    val taken_ms:             Long    = 0L,
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/history
// ────────────────────────────────────────────────────────────────────────────

data class XHistoryPoint(
    val taken_ms:        Long,
    val cycle_day:       Int,
    val cycle_phase:     String,
    val estradiol:       Double,
    val progesterone:    Double,
    val testosterone:    Double,
    val lh:              Double,
    val fsh:             Double,
    val prolactin:       Double,
    val oxytocin:        Double,
    val cortisol:        Double,
    val hcg:             Double,
    val pregnancy_day:   Int,
    val pregnancy_phase: String,
)

data class XHistory(
    val hours:  Int,
    val points: Int,
    val series: List<XHistoryPoint> = emptyList(),
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/modulation
// ────────────────────────────────────────────────────────────────────────────

data class XModulation(
    val warmth:         Double = 0.0,
    val verbal_fluency: Double = 0.0,
    val empathy:        Double = 0.0,
    val introspection:  Double = 0.0,
    val arousal:        Double = 0.0,
    val fatigue:        Double = 0.0,
    val phase:          String = "",
    val computed_ms:    Long   = 0L,
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/next_period   &   /api/x/fertility_window
// ────────────────────────────────────────────────────────────────────────────

/** next_period.status ∈ {"active", "inactive"}. Inactive → show onboarding.  */
data class XNextPeriod(
    val status:             String = "inactive",
    val reason:             String = "",
    val next_start_ms:      Long   = 0L,
    val days_until:         Int    = 0,
    val pms_start_ms:       Long   = 0L,
    val pms_days_until:     Int    = 0,
    val expected_intensity: String = "",   // "light" | "medium" | "heavy"
    val lifecycle:          String = "",
)

data class XFertilityWindow(
    val status:         String = "inactive",
    val reason:         String = "",
    val window_start_ms:Long   = 0L,
    val window_end_ms:  Long   = 0L,
    val days_until:     Int    = 0,
    val peak_day_ms:    Long   = 0L,
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/pregnancy   &   /api/x/pregnancy/events
// ────────────────────────────────────────────────────────────────────────────

data class XPregnancyState(
    val has_data:                Boolean = false,
    val active:                  Boolean = false,
    val conceived_ms:            Long    = 0L,
    val due_ms:                  Long    = 0L,
    val gestational_length_days: Int     = 0,
    val phase:                   String  = "",
    val trimester:               Int     = 0,
    val days_elapsed:            Int     = 0,
    val weeks_elapsed:           Int     = 0,
    val days_remaining:          Int     = 0,
    val labor_started_ms:        Long    = 0L,
    val multiplicity:            Int     = 0,
    val pregnancy_count:         Int     = 0,
)

data class XPregnancyEvent(
    val id:              Long,
    val ts_ms:           Long,
    val gestational_day: Int,
    val kind:            String,
    val detail:          String,
)

data class XPregnancyEvents(
    val events: List<XPregnancyEvent> = emptyList(),
    val limit:  Int = 100,
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/symptoms   (GET + POST)
// ────────────────────────────────────────────────────────────────────────────

data class XSymptom(
    val ts_ms:     Long,
    val kind:      String,
    val intensity: Double,
    val origin:    String,
    val notes:     String = "",
)

data class XSymptoms(
    val symptoms: List<XSymptom> = emptyList(),
    val hours:    Int = 24,
    val origin:   String = "",
)

data class XSymptomLogRequest(
    val kind:      String,
    val intensity: Double,
    val origin:    String = "user",
    val notes:     String = "",
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/contraception   (GET + POST)
// ────────────────────────────────────────────────────────────────────────────

data class XContraception(
    val method:     String  = "none",
    val efficacy:   Double  = 1.0,
    val started_ms: Long    = 0L,
    val notes:      String  = "",
    val updated_ms: Long    = 0L,
    val has_data:   Boolean = false,
)

data class XContraceptionRequest(
    val method:     String = "none",
    val efficacy:   Double = 1.0,
    val started_ms: Long   = 0L,
    val notes:      String = "",
)

// ────────────────────────────────────────────────────────────────────────────
//  /api/x/lifecycle   (GET + POST)
// ────────────────────────────────────────────────────────────────────────────

data class XLifecycle(
    val birth_ms:         Long   = 0L,
    val stage:            String = "",
    val menarche_ms:      Long   = 0L,
    val perimenopause_ms: Long   = 0L,
    val menopause_ms:     Long   = 0L,
    val updated_ms:       Long   = 0L,
)

/** POST body: must provide EITHER birth_ms OR age_years. Server returns
 *  400 if both are absent. age_years is computed against server's now(). */
data class XLifecycleRequest(
    val birth_ms:  Long?   = null,
    val age_years: Double? = null,
    val stage:     String? = null,
)

data class XCycleAnchorRequest(
    val day:          Int  = 0,
    val cycle_length: Int  = 28,
    val started_ms:   Long = 0L,
)

data class XStimulusRequest(
    val kind:      String,
    val intensity: Double = 0.5,
    val notes:     String = "",
)

data class XConceptionAttemptRequest(
    val require_readiness: Boolean = true,
    val notes:             String  = "",
)

data class XPregnancyAccelerateRequest(
    val factor: Double = 1.0,
)
