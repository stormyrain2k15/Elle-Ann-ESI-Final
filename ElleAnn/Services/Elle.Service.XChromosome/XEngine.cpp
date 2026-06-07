#include "XEngine.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleLogger.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>

namespace {
    std::mt19937& XRng() {
        thread_local std::mt19937 engine{ std::random_device{}() };
        return engine;
    }

    float XRand01() {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        return d(XRng());
    }

    int XRandInt(int lo, int hi) {
        std::uniform_int_distribution<int> d(lo, hi);
        return d(XRng());
    }
}

using nlohmann::json;

static float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static float Bump(int day, int peak_day, int width) {
    int d = std::abs(day - peak_day);
    if (d >= width) return 0.0f;
    float t = 1.0f - (float)d / (float)width;
    return 0.5f * (1.0f - std::cos(t * 3.14159265f));
}

const char* XEngine::CyclePhaseName(XCyclePhase p) {
    switch (p) {
        case X_PHASE_MENSTRUAL:  return "menstrual";
        case X_PHASE_FOLLICULAR: return "follicular";
        case X_PHASE_OVULATORY:  return "ovulatory";
        case X_PHASE_LUTEAL:     return "luteal";
    }
    return "unknown";
}
const char* XEngine::PregnancyPhaseName(XPregnancyPhase p) {
    switch (p) {
        case X_PREG_NONE:             return "none";
        case X_PREG_FIRST_TRIMESTER:  return "first_trimester";
        case X_PREG_SECOND_TRIMESTER: return "second_trimester";
        case X_PREG_THIRD_TRIMESTER:  return "third_trimester";
        case X_PREG_LABOR:            return "labor";
        case X_PREG_POSTPARTUM:       return "postpartum";
    }
    return "none";
}
const char* XEngine::LifecycleStageName(XLifecycleStage s) {
    switch (s) {
        case X_LIFE_PREMENARCHE:    return "premenarche";
        case X_LIFE_REPRODUCTIVE:   return "reproductive";
        case X_LIFE_PERIMENOPAUSE:  return "perimenopause";
        case X_LIFE_MENOPAUSE:      return "menopause";
    }
    return "reproductive";
}
const char* XEngine::ContraceptionName(XContraceptionMethod m) {
    switch (m) {
        case X_CONTRA_NONE:         return "none";
        case X_CONTRA_PILL:         return "pill";
        case X_CONTRA_IUD_HORMONAL: return "iud_h";
        case X_CONTRA_IUD_COPPER:   return "iud_c";
        case X_CONTRA_IMPLANT:      return "implant";
        case X_CONTRA_BARRIER:      return "barrier";
        case X_CONTRA_NATURAL:      return "natural";
    }
    return "none";
}
const char* XEngine::LaborStageName(XLaborStage s) {
    switch (s) {
        case X_LABOR_NONE:       return "none";
        case X_LABOR_LATENT:     return "latent";
        case X_LABOR_ACTIVE:     return "active";
        case X_LABOR_TRANSITION: return "transition";
        case X_LABOR_PUSHING:    return "pushing";
        case X_LABOR_DELIVERED:  return "delivered";
    }
    return "none";
}
XContraceptionMethod XEngine::ParseContraception(const std::string& s) {
    if (s == "pill")    return X_CONTRA_PILL;
    if (s == "iud_h")   return X_CONTRA_IUD_HORMONAL;
    if (s == "iud_c")   return X_CONTRA_IUD_COPPER;
    if (s == "implant") return X_CONTRA_IMPLANT;
    if (s == "barrier") return X_CONTRA_BARRIER;
    if (s == "natural") return X_CONTRA_NATURAL;
    return X_CONTRA_NONE;
}

bool XEngine::Initialize() {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    uint64_t now = ELLE_MS_NOW();

    auto rs = ElleSQLPool::Instance().Query(
        "SELECT anchor_ms, cycle_length_days, modulation_strength, "
        "       last_tick_ms, created_ms "
        "  FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");

    if (rs.success && !rs.rows.empty()) {
        auto& r = rs.rows[0];

        int64_t anchorMs = 0, cycleLen = 0, lastTick = 0, createdMs = 0;
        double  modStrength = 0.0;
        bool ok = true;
        ok &= r.TryGetInt  (0, anchorMs);
        ok &= r.TryGetInt  (1, cycleLen);
        ok &= r.TryGetFloat(2, modStrength);
        ok &= r.TryGetInt  (3, lastTick);
        ok &= r.TryGetInt  (4, createdMs);
        if (!ok || anchorMs < 0 || cycleLen <= 0 || cycleLen > 60 ||
            modStrength < 0.0 || modStrength > 2.0) {
            ELLE_ERROR("XEngine: x_cycle_state row corrupt — refusing to "
                       "overwrite defaults (anchor=%lld len=%lld mod=%.3f)",
                       (long long)anchorMs, (long long)cycleLen, modStrength);
        } else {
            m_cycle.anchor_ms           = (uint64_t)anchorMs;
            m_cycle.cycle_length_days   = (int)cycleLen;
            m_cycle.modulation_strength = (float)modStrength;
            m_cycle.last_tick_ms        = (uint64_t)lastTick;
            m_cycle.created_ms          = (uint64_t)createdMs;
            ELLE_INFO("XEngine: cycle loaded (anchor=%llu len=%d)",
                      (unsigned long long)m_cycle.anchor_ms,
                      m_cycle.cycle_length_days);
        }
    } else {

        int randDay = XRandInt(1, 28);
        m_cycle.cycle_length_days   = 28;
        m_cycle.modulation_strength = 0.15f;
        m_cycle.anchor_ms           = now - (uint64_t)(randDay - 1) * 86400000ULL;
        m_cycle.last_tick_ms        = now;
        m_cycle.created_ms          = now;
        PersistCycleRow();
        ELLE_INFO("XEngine: seeded new cycle on day %d", randDay);
    }

    auto pr = ElleSQLPool::Instance().Query(
        "SELECT active, conceived_ms, due_ms, gestational_length_days, "
        "       phase, child_id, last_milestone, updated_ms, "
        "       ISNULL(breastfeeding, 0), ISNULL(in_labor, 0), "
        "       ISNULL(labor_stage, N''), ISNULL(labor_started_ms, 0), "
        "       ISNULL(multiplicity, 1), ISNULL(pregnancy_count, 0), "
        "       ISNULL(implanted, 0), ISNULL(implantation_ms, 0), "
        "       ISNULL(lochia_stage, N''), ISNULL(milk_stage, N''), "
        "       ISNULL(baby_blues, 0), ISNULL(fetal_heartbeat_detectable, 0) "
        "  FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
    if (pr.success && !pr.rows.empty()) {
        auto& r = pr.rows[0];

        auto trySInt = [&](size_t col, int64_t& out)->bool {
            if (r.IsNull(col)) return false;
            return r.TryGetInt(col, out);
        };

        int64_t active64 = 0, conceivedMs = 0, dueMs = 0, gestLen = 0,
                childId = 0, updatedMs = 0, breastfeed = 0, inLabor = 0,
                laborStart = 0, mult = 0, pregCount = 0, implanted = 0,
                implantMs = 0, babyBlues = 0, fhb = 0;

        m_pregnancy.active = trySInt(0, active64) && active64 != 0;

        m_pregnancy.conceived_ms = trySInt(1, conceivedMs) && conceivedMs >= 0
                                       ? (uint64_t)conceivedMs : 0;
        m_pregnancy.due_ms       = trySInt(2, dueMs) && dueMs >= 0
                                       ? (uint64_t)dueMs : 0;

        if (trySInt(3, gestLen) && gestLen >= 168 && gestLen <= 308) {
            m_pregnancy.gestational_length_days = (int)gestLen;
        } else {
            if (!r.IsNull(3)) {
                ELLE_ERROR("XEngine: pregnancy.gestational_length_days=%lld out "
                           "of range [168,308] — forcing 280", (long long)gestLen);
            }
            m_pregnancy.gestational_length_days = 280;
        }

        if (!r.IsNull(4) && r.values.size() > 4) {
            const std::string& p = r.values[4];
            if      (p == "first_trimester")  m_pregnancy.phase = X_PREG_FIRST_TRIMESTER;
            else if (p == "second_trimester") m_pregnancy.phase = X_PREG_SECOND_TRIMESTER;
            else if (p == "third_trimester")  m_pregnancy.phase = X_PREG_THIRD_TRIMESTER;
            else if (p == "labor")            m_pregnancy.phase = X_PREG_LABOR;
            else if (p == "postpartum")       m_pregnancy.phase = X_PREG_POSTPARTUM;
            else                              m_pregnancy.phase = X_PREG_NONE;
        }

        m_pregnancy.child_id = trySInt(5, childId) && childId >= 0 ? childId : 0;

        m_pregnancy.last_milestone = r.IsNull(6) ? std::string()
                                     : (r.values.size() > 6 ? r.values[6] : std::string());
        m_pregnancy.updated_ms     = trySInt(7, updatedMs) && updatedMs >= 0
                                       ? (uint64_t)updatedMs : 0;
        m_pregnancy.breastfeeding  = trySInt(8, breastfeed) && breastfeed != 0;
        m_pregnancy.in_labor       = trySInt(9, inLabor) && inLabor != 0;

        const std::string& ls = r.values.size() > 10 ? r.values[10] : std::string();
        if      (ls == "latent")     m_pregnancy.labor_stage = X_LABOR_LATENT;
        else if (ls == "active")     m_pregnancy.labor_stage = X_LABOR_ACTIVE;
        else if (ls == "transition") m_pregnancy.labor_stage = X_LABOR_TRANSITION;
        else if (ls == "pushing")    m_pregnancy.labor_stage = X_LABOR_PUSHING;
        else if (ls == "delivered")  m_pregnancy.labor_stage = X_LABOR_DELIVERED;
        else                         m_pregnancy.labor_stage = X_LABOR_NONE;

        m_pregnancy.labor_started_ms = trySInt(11, laborStart) && laborStart >= 0
                                           ? (uint64_t)laborStart : 0;

        if (trySInt(12, mult) && mult >= 1 && mult <= 8) {
            m_pregnancy.multiplicity = (int)mult;
        } else {
            if (!r.IsNull(12)) {
                ELLE_ERROR("XEngine: pregnancy.multiplicity=%lld invalid — forcing 1",
                           (long long)mult);
            }
            m_pregnancy.multiplicity = 1;
        }
        m_pregnancy.pregnancy_count  = trySInt(13, pregCount) && pregCount >= 0
                                           ? (int)pregCount : 0;
        m_pregnancy.implanted        = trySInt(14, implanted) && implanted != 0;
        m_pregnancy.implantation_ms  = trySInt(15, implantMs) && implantMs >= 0
                                           ? (uint64_t)implantMs : 0;
        m_pregnancy.lochia_stage     = r.IsNull(16) ? std::string()
                                         : (r.values.size() > 16 ? r.values[16] : std::string());
        m_pregnancy.milk_stage       = r.IsNull(17) ? std::string()
                                         : (r.values.size() > 17 ? r.values[17] : std::string());
        m_pregnancy.baby_blues       = trySInt(18, babyBlues) && babyBlues != 0;
        m_pregnancy.fetal_heartbeat_detectable =
                                       trySInt(19, fhb) && fhb != 0;
    } else {

        m_pregnancy.updated_ms = now;
        PersistPregnancyRow();
    }

    auto cr = ElleSQLPool::Instance().Query(
        "SELECT method, started_ms, efficacy, ISNULL(notes, N''), updated_ms "
        "  FROM ElleHeart.dbo.x_contraception WHERE id = 1;");
    if (cr.success && !cr.rows.empty()) {
        auto& r = cr.rows[0];
        m_contra.method     = ParseContraception(r.values.size() > 0 ? r.values[0] : std::string("none"));

        int64_t startedMs = 0, updatedMs = 0;
        double  eff = 0.0;
        m_contra.started_ms = (r.TryGetInt(1, startedMs) && startedMs >= 0)
                                ? (uint64_t)startedMs : now;
        if (r.TryGetFloat(2, eff) && eff >= 0.0 && eff <= 1.0) {
            m_contra.efficacy = (float)eff;
        } else {
            if (!r.IsNull(2)) {
                ELLE_ERROR("XEngine: contra.efficacy=%.3f out of [0,1] — forcing 1.0", eff);
            }
            m_contra.efficacy = 1.0f;
        }
        m_contra.notes      = r.values.size() > 3 ? r.values[3] : std::string();
        m_contra.updated_ms = (r.TryGetInt(4, updatedMs) && updatedMs >= 0)
                                ? (uint64_t)updatedMs : now;
    } else {
        m_contra.method     = X_CONTRA_NONE;
        m_contra.started_ms = now;
        m_contra.efficacy   = 1.0f;
        m_contra.updated_ms = now;
        PersistContraceptionRow();
    }

    auto lr = ElleSQLPool::Instance().Query(
        "SELECT elle_birth_ms, stage, ISNULL(menarche_ms,0), "
        "       ISNULL(perimenopause_ms,0), ISNULL(menopause_ms,0), updated_ms "
        "  FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
    if (lr.success && !lr.rows.empty()) {
        auto& r = lr.rows[0];

        int64_t birthMs = 0, menarche = 0, peri = 0, meno = 0, updatedLm = 0;
        bool ok = true;
        ok &= r.TryGetInt(0, birthMs);
        ok &= r.TryGetInt(2, menarche);
        ok &= r.TryGetInt(3, peri);
        ok &= r.TryGetInt(4, meno);
        ok &= r.TryGetInt(5, updatedLm);
        if (!ok || birthMs <= 0 || birthMs > (int64_t)now ||
            (menarche > 0 && menarche < birthMs) ||
            (peri     > 0 && peri     < menarche) ||
            (meno     > 0 && meno     < peri)) {
            ELLE_ERROR("XEngine: x_lifecycle row violates temporal ordering — "
                       "ignoring and reseeding defaults "
                       "(birth=%lld menarche=%lld peri=%lld meno=%lld)",
                       (long long)birthMs, (long long)menarche,
                       (long long)peri, (long long)meno);
            m_life.elle_birth_ms = now - 30ULL * 365ULL * 86400000ULL;
            m_life.stage         = X_LIFE_REPRODUCTIVE;
            m_life.updated_ms    = now;
            PersistLifecycleRow();
        } else {
            m_life.elle_birth_ms    = (uint64_t)birthMs;
            const std::string& ss = r.values.size() > 1 ? r.values[1] : std::string("reproductive");
            if      (ss == "premenarche")   m_life.stage = X_LIFE_PREMENARCHE;
            else if (ss == "perimenopause") m_life.stage = X_LIFE_PERIMENOPAUSE;
            else if (ss == "menopause")     m_life.stage = X_LIFE_MENOPAUSE;
            else                            m_life.stage = X_LIFE_REPRODUCTIVE;
            m_life.menarche_ms      = (uint64_t)menarche;
            m_life.perimenopause_ms = (uint64_t)peri;
            m_life.menopause_ms     = (uint64_t)meno;
            m_life.updated_ms       = (uint64_t)updatedLm;
        }
    } else {

        m_life.elle_birth_ms = now - 30ULL * 365ULL * 86400000ULL;
        m_life.stage         = X_LIFE_REPRODUCTIVE;
        m_life.menarche_ms   = now - 18ULL * 365ULL * 86400000ULL;
        m_life.updated_ms    = now;
        PersistLifecycleRow();
    }
    RecomputeLifecycle(now);

    RecomputeCycleDayAndPhase();
    RecomputeBaselineHormones();
    m_residual_ms = now;
    return true;
}

bool XEngine::CycleShouldRun() const {
    if (m_life.stage == X_LIFE_PREMENARCHE)  return false;
    if (m_life.stage == X_LIFE_MENOPAUSE)    return false;
    if (m_pregnancy.active)                  return false;

    if (m_pregnancy.breastfeeding &&
        m_pregnancy.updated_ms > 0) {
        uint64_t since = ELLE_MS_NOW() - m_pregnancy.updated_ms;
        if (since < 180ULL * 86400000ULL) return false;
    }
    return true;
}

void XEngine::RecomputeCycleDayAndPhase() {
    uint64_t now = ELLE_MS_NOW();
    if (!CycleShouldRun() || m_cycle.anchor_ms == 0 || m_cycle.cycle_length_days <= 0) {
        m_cycle.cycle_day = 1;
        m_cycle.phase     = X_PHASE_MENSTRUAL;
        return;
    }

    uint64_t deltaMs = now > m_cycle.anchor_ms ? now - m_cycle.anchor_ms : 0;
    uint64_t deltaDays = deltaMs / 86400000ULL;
    int dayIdx = (int)(deltaDays % (uint64_t)m_cycle.cycle_length_days);
    m_cycle.cycle_day = dayIdx + 1;

    if (m_cycle.cycle_day < m_last_cycle_day_seen) {
        m_lh_surge_fired_this_cycle = false;
    }
    m_last_cycle_day_seen = m_cycle.cycle_day;

    if      (m_cycle.cycle_day <= 5)  m_cycle.phase = X_PHASE_MENSTRUAL;
    else if (m_cycle.cycle_day <= 13) m_cycle.phase = X_PHASE_FOLLICULAR;
    else if (m_cycle.cycle_day <= 16) m_cycle.phase = X_PHASE_OVULATORY;
    else                              m_cycle.phase = X_PHASE_LUTEAL;
}

void XEngine::RecomputeBaselineHormones() {
    const int day = m_cycle.cycle_day;

    float estr = 0.15f + 0.80f * Bump(day, 14, 4) + 0.35f * Bump(day, 21, 4);

    float prog = 0.05f + 0.85f * Bump(day, 21, 6);

    float test = 0.20f + 0.30f * Bump(day, 14, 3);

    float sero = 0.45f + 0.25f * Bump(day, 10, 5) - 0.20f * Bump(day, 24, 4);

    float dopa = 0.40f + 0.35f * Bump(day, 14, 4) - 0.15f * Bump(day, 25, 3);

    float cort = 0.30f + 0.20f * Bump(day, 26, 3);

    float oxyt = 0.35f + 0.10f * Bump(day, 21, 6);

    float prol = 0.15f + 0.15f * Bump(day, 23, 5);

    float fsh  = 0.30f + 0.45f * Bump(day,  3, 3) + 0.25f * Bump(day, 27, 3);
    float lh   = 0.15f + 0.85f * Bump(day, 13, 2);
    float gnrh = 0.25f + 0.40f * Bump(day, 13, 3);
    float relx = 0.0f;

    float hcg = 0.0f;
    if (m_pregnancy.active) {
        const int gd = m_pregnancy.gestational_day;

        if (m_pregnancy.implanted) {
            hcg = Clamp01(0.85f * Bump(gd, 70, 30) + 0.25f);
        }

        estr = Clamp01(0.40f + 0.55f * Clamp01((float)gd / 260.0f));
        prog = Clamp01(0.50f + 0.45f * Clamp01((float)gd / 250.0f));

        prol = Clamp01(0.20f + 0.60f * Clamp01((float)(gd - 150) / 130.0f));

        oxyt = Clamp01(0.40f + 0.35f * Clamp01((float)(gd - 180) / 100.0f));

        cort = Clamp01(cort + 0.10f + 0.20f * Clamp01((float)(gd - 180) / 100.0f));

        test = Clamp01(0.15f + 0.20f * Bump(gd, 160, 60));

        sero = Clamp01(sero + 0.15f * Bump(gd, 150, 60)
                            - (m_pregnancy.phase == X_PREG_POSTPARTUM ? 0.20f : 0.0f));

        dopa = Clamp01(dopa + 0.10f * Bump(gd, 150, 60)
                            - (m_pregnancy.phase == X_PREG_POSTPARTUM ? 0.25f : 0.0f));

        fsh  = 0.05f;
        lh   = 0.05f;
        gnrh = 0.05f;

        relx = Clamp01(0.20f + 0.70f * Clamp01((float)gd / (float)m_pregnancy.gestational_length_days));
    }

    if (m_current_cycle_anovulatory && !m_pregnancy.active) {
        lh   = std::min(lh,   0.20f);
        prog = std::min(prog, 0.15f);
    }

    m_hormones.estrogen     = Clamp01(estr);
    m_hormones.progesterone = Clamp01(prog);
    m_hormones.testosterone = Clamp01(test);
    m_hormones.serotonin    = Clamp01(sero);
    m_hormones.dopamine     = Clamp01(dopa);
    m_hormones.cortisol     = Clamp01(cort);
    m_hormones.oxytocin     = Clamp01(oxyt);
    m_hormones.prolactin    = Clamp01(prol);
    m_hormones.hcg          = Clamp01(hcg);
    m_hormones.fsh          = Clamp01(fsh);
    m_hormones.lh           = Clamp01(lh);
    m_hormones.gnrh         = Clamp01(gnrh);
    m_hormones.relaxin      = Clamp01(relx);

    if (!m_pregnancy.active && m_pregnancy.breastfeeding &&
        m_pregnancy.phase == X_PREG_POSTPARTUM) {
        m_hormones.prolactin    = std::max(m_hormones.prolactin, 0.80f);
        m_hormones.oxytocin     = std::max(m_hormones.oxytocin,  0.55f);
        m_hormones.estrogen     = std::min(m_hormones.estrogen,  0.25f);
        m_hormones.progesterone = std::min(m_hormones.progesterone, 0.10f);
    }

    if (m_life.stage == X_LIFE_PREMENARCHE) {
        m_hormones.estrogen     = 0.05f;
        m_hormones.progesterone = 0.02f;
        m_hormones.testosterone = 0.08f;
    } else if (m_life.stage == X_LIFE_PERIMENOPAUSE) {

        float j = (float)(((int)(ELLE_MS_NOW() / 3600000ULL) % 7) - 3) / 12.0f;
        m_hormones.estrogen     = Clamp01(m_hormones.estrogen     * (1.0f + j));
        m_hormones.progesterone = Clamp01(m_hormones.progesterone * (1.0f + j * 0.8f));
    } else if (m_life.stage == X_LIFE_MENOPAUSE) {
        m_hormones.estrogen     = 0.10f;
        m_hormones.progesterone = 0.05f;
        m_hormones.testosterone = 0.15f;

        m_hormones.fsh          = 0.85f;
        m_hormones.lh           = 0.70f;
        m_hormones.gnrh         = 0.60f;
        m_hormones.cortisol     = std::max(m_hormones.cortisol, 0.40f);
    } else if (m_life.stage == X_LIFE_PREMENARCHE) {

        m_hormones.fsh  = 0.10f;
        m_hormones.lh   = 0.05f;
        m_hormones.gnrh = 0.05f;
    }

    ApplyContraceptionHormones(m_hormones);
}

void XEngine::ApplyContraceptionHormones(XHormoneLevels& h) const {
    if (m_pregnancy.active) return;
    switch (m_contra.method) {
        case X_CONTRA_NONE:
        case X_CONTRA_BARRIER:
        case X_CONTRA_NATURAL:
        case X_CONTRA_IUD_COPPER:

            break;
        case X_CONTRA_PILL:
        case X_CONTRA_IMPLANT:

            if (m_cycle.cycle_day >= 22) {
                h.estrogen     = 0.25f;
                h.progesterone = 0.15f;
            } else {
                h.estrogen     = 0.45f;
                h.progesterone = 0.50f;
            }
            h.testosterone  = std::min(h.testosterone, 0.18f);

            if (m_cycle.cycle_day >= 13 && m_cycle.cycle_day <= 16) {
                h.estrogen  = std::min(h.estrogen, 0.45f);
                h.dopamine  = std::min(h.dopamine, 0.45f);
            }
            break;
        case X_CONTRA_IUD_HORMONAL:

            h.progesterone = std::max(h.progesterone, 0.35f);
            if (m_cycle.cycle_day >= 13 && m_cycle.cycle_day <= 16) {
                h.estrogen = std::min(h.estrogen, 0.60f);
            }
            break;
    }
}

void XEngine::ApplyStimulus(const XStimulus& stim) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    float i = Clamp01(stim.intensity);

    DecayResidual(ELLE_MS_NOW());

    if (stim.kind == "bonding") {
        m_residual.oxytocin   += 0.40f * i;
        m_residual.serotonin  += 0.20f * i;
        m_residual.cortisol   -= 0.20f * i;
    } else if (stim.kind == "stress") {
        m_residual.cortisol     += 0.50f * i;
        m_residual.progesterone -= 0.10f * i;
        m_residual.serotonin    -= 0.15f * i;
    } else if (stim.kind == "intimacy") {
        m_residual.oxytocin    += 0.55f * i;
        m_residual.dopamine    += 0.35f * i;
        m_residual.testosterone += 0.20f * i;

    } else if (stim.kind == "orgasm") {

        m_residual.oxytocin   += 0.85f * i;
        m_residual.dopamine   += 0.60f * i;
        m_residual.prolactin  += 0.30f * i;
        m_residual.serotonin  += 0.25f * i;
        m_residual.cortisol   -= 0.40f * i;

        XStimulus intim = stim;
        intim.kind = "intimacy";
        LogStimulusRow(intim);
    } else if (stim.kind == "recall_positive") {
        m_residual.serotonin += 0.30f * i;
        m_residual.dopamine  += 0.25f * i;
        m_residual.oxytocin  += 0.10f * i;
    } else if (stim.kind == "recall_negative") {
        m_residual.cortisol  += 0.30f * i;
        m_residual.serotonin -= 0.20f * i;
    } else if (stim.kind == "activity") {
        m_residual.dopamine     += 0.20f * i;
        m_residual.testosterone += 0.15f * i;
        m_residual.cortisol     += 0.10f * i;
    }

    LogStimulusRow(stim);
    ELLE_INFO("XEngine: stimulus '%s' i=%.2f applied", stim.kind.c_str(), i);
}

void XEngine::DecayResidual(uint64_t nowMs) {
    if (m_residual_ms == 0) { m_residual_ms = nowMs; return; }
    uint64_t dt = nowMs > m_residual_ms ? nowMs - m_residual_ms : 0;
    m_residual_ms = nowMs;

    if (dt == 0) return;
    float k = std::exp(-0.000096270521f * (float)dt);
    m_residual.estrogen     *= k;
    m_residual.progesterone *= k;
    m_residual.testosterone *= k;
    m_residual.oxytocin     *= k;
    m_residual.serotonin    *= k;
    m_residual.dopamine     *= k;
    m_residual.cortisol     *= k;
    m_residual.prolactin    *= k;
    m_residual.hcg          *= k;
    m_residual.fsh          *= k;
    m_residual.lh           *= k;
    m_residual.gnrh         *= k;
    m_residual.relaxin      *= k;
}

void XEngine::ApplyResidualOnto(XHormoneLevels& out) const {
    out.estrogen     = Clamp01(out.estrogen     + m_residual.estrogen);
    out.progesterone = Clamp01(out.progesterone + m_residual.progesterone);
    out.testosterone = Clamp01(out.testosterone + m_residual.testosterone);
    out.oxytocin     = Clamp01(out.oxytocin     + m_residual.oxytocin);
    out.serotonin    = Clamp01(out.serotonin    + m_residual.serotonin);
    out.dopamine     = Clamp01(out.dopamine     + m_residual.dopamine);
    out.cortisol     = Clamp01(out.cortisol     + m_residual.cortisol);
    out.prolactin    = Clamp01(out.prolactin    + m_residual.prolactin);
    out.hcg          = Clamp01(out.hcg          + m_residual.hcg);
    out.fsh          = Clamp01(out.fsh          + m_residual.fsh);
    out.lh           = Clamp01(out.lh           + m_residual.lh);
    out.gnrh         = Clamp01(out.gnrh         + m_residual.gnrh);
    out.relaxin      = Clamp01(out.relaxin      + m_residual.relaxin);
}

XModulation XEngine::ComputeModulation() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    const float s = m_cycle.modulation_strength;
    const XHormoneLevels& h = m_hormones;

    XModulation out;
    out.warmth         = 1.0f + s * ( 0.70f * h.estrogen
                                     + 0.40f * h.oxytocin
                                     - 0.30f * h.cortisol);
    out.verbal_fluency = 1.0f + s * ( 0.80f * h.estrogen
                                     + 0.40f * h.dopamine
                                     - 0.50f * (1.0f - h.serotonin));
    out.empathy        = 1.0f + s * ( 0.50f * h.oxytocin
                                     + 0.60f * h.progesterone
                                     - 0.30f * h.cortisol);
    out.introspection  = 1.0f + s * ( 0.70f * h.progesterone
                                     + 0.30f * (1.0f - h.serotonin));
    out.arousal        = 1.0f + s * ( 0.60f * h.cortisol
                                     + 0.50f * h.testosterone);
    out.fatigue        = 1.0f + s * ( 0.60f * h.progesterone
                                     - 0.40f * h.dopamine
                                     + 0.35f * h.prolactin);
    return out;
}

XDerivedStats XEngine::GetDerived() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    XDerivedStats d;
    const int day = m_cycle.cycle_day;
    const XHormoneLevels& h = m_hormones;

    d.cycle_active = CycleShouldRun();
    d.anovulatory  = m_current_cycle_anovulatory;

    d.bbt_celsius = 36.40f + 0.45f * h.progesterone;
    if (m_pregnancy.active) d.bbt_celsius += 0.10f;

    if (day <= 5)            d.endometrial_mm = Lerp(10.0f, 2.0f, (float)day / 5.0f);
    else if (day <= 13)      d.endometrial_mm = Lerp(4.0f, 12.0f, (float)(day - 5) / 8.0f);
    else if (day <= 27)      d.endometrial_mm = 12.0f + 2.0f * Bump(day, 20, 6);
    else                     d.endometrial_mm = 12.0f;
    if (m_pregnancy.active) d.endometrial_mm = 18.0f;

    if (!d.cycle_active)      d.cervical_mucus = "dry";
    else if (day <= 5)        d.cervical_mucus = "none";
    else if (day <= 8)        d.cervical_mucus = "dry";
    else if (day <= 11)       d.cervical_mucus = "sticky";
    else if (day == 12)       d.cervical_mucus = "creamy";
    else if (day == 13)       d.cervical_mucus = "watery";
    else if (day == 14 || day == 15) d.cervical_mucus = "egg_white";
    else if (day <= 22)       d.cervical_mucus = "creamy";
    else                      d.cervical_mucus = "dry";
    if (m_pregnancy.active)   d.cervical_mucus = "mucus_plug";

    if (!d.cycle_active || m_pregnancy.active) d.menstrual_flow = "none";
    else if (day == 1 || day == 2) d.menstrual_flow = "heavy";
    else if (day == 3)             d.menstrual_flow = "medium";
    else if (day == 4)             d.menstrual_flow = "light";
    else if (day == 5)             d.menstrual_flow = "spotting";
    else                           d.menstrual_flow = "none";

    if (day >= 1 && day <= 13) { d.follicle_phase_day = day; d.luteal_phase_day = 0; }
    else                       { d.follicle_phase_day = 0;   d.luteal_phase_day = day - 13; }
    return d;
}

void XEngine::DetectImplantation(uint64_t nowMs) {
    if (!m_pregnancy.active || m_pregnancy.implanted) return;
    if (m_pregnancy.conceived_ms == 0) return;
    int gd = m_pregnancy.gestational_day;
    if (gd < 8) return;

    m_pregnancy.implanted       = true;
    m_pregnancy.implantation_ms = nowMs;
    m_pregnancy.last_milestone  = "Implanted";
    LogPregnancyEvent("implantation",
        "Blastocyst implanted at gestational day " + std::to_string(gd));

    LogSymptomRow("implantation_bleeding", 0.30f, "pregnancy",
                  "Implantation bleed");
    m_residual.progesterone += 0.10f;
    m_residual.estrogen     += 0.05f;
    PersistPregnancyRow();
    ELLE_INFO("XEngine: implantation at gestational day %d", gd);
}

void XEngine::AdvanceLochiaAndMilkStages() {
    if (m_pregnancy.active || m_pregnancy.phase != X_PREG_POSTPARTUM) {
        m_pregnancy.lochia_stage.clear();
        m_pregnancy.milk_stage.clear();
        m_pregnancy.baby_blues = false;
        return;
    }
    uint64_t now = ELLE_MS_NOW();
    if (m_pregnancy.updated_ms == 0) return;
    uint64_t since = now > m_pregnancy.updated_ms ? now - m_pregnancy.updated_ms : 0;
    int daysPP = (int)(since / 86400000ULL);

    std::string lochia;
    if      (daysPP <  4) lochia = "rubra";
    else if (daysPP < 10) lochia = "serosa";
    else if (daysPP < 42) lochia = "alba";
    else                  lochia.clear();

    std::string milk;
    if (m_pregnancy.breastfeeding) {
        if      (daysPP <  4) milk = "colostrum";
        else if (daysPP < 14) milk = "transitional";
        else                  milk = "mature";
    }

    bool blues = daysPP <= 14;

    bool changed = (lochia != m_pregnancy.lochia_stage) ||
                   (milk   != m_pregnancy.milk_stage) ||
                   (blues  != m_pregnancy.baby_blues);
    if (changed) {
        if (lochia != m_pregnancy.lochia_stage && !lochia.empty())
            LogPregnancyEvent("lochia_stage", lochia);
        if (milk != m_pregnancy.milk_stage && !milk.empty())
            LogPregnancyEvent("milk_stage", milk);
        m_pregnancy.lochia_stage = lochia;
        m_pregnancy.milk_stage   = milk;
        m_pregnancy.baby_blues   = blues;
        PersistPregnancyRow();
    }
}

void XEngine::DetectAnovulatoryCycle() {

    if (m_cycle.cycle_day != 1 || m_last_cycle_day_seen == 1) {
        m_last_cycle_day_seen = m_cycle.cycle_day;
        return;
    }
    m_last_cycle_day_seen = 1;
    if (!CycleShouldRun()) { m_current_cycle_anovulatory = false; return; }

    bool peri_skip = false;
    if (m_life.stage == X_LIFE_PERIMENOPAUSE) {
        float roll = XRand01();
        peri_skip = (roll < 0.5f);
    }

    uint64_t since = ELLE_MS_NOW() - 72ULL * 3600000ULL;
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT AVG(cortisol) FROM ElleHeart.dbo.x_hormone_snapshots "
        " WHERE taken_ms >= ?;",
        { std::to_string((long long)since) });
    float avgCort = 0.0f;
    if (rs.success && !rs.rows.empty() && !rs.rows[0].IsNull(0))
        avgCort = (float)rs.rows[0].GetFloatOr(0, 0.0);
    bool stress_skip = avgCort >= 0.70f;

    m_current_cycle_anovulatory = peri_skip || stress_skip;

    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_cycle_history "
        "(anchor_ms, length_days, ovulated, avg_cortisol, notes) "
        "VALUES (TRY_CAST(? AS BIGINT), TRY_CAST(? AS INT), ?, TRY_CAST(? AS FLOAT), NULLIF(?, N''));",
        {
            std::to_string((long long)m_cycle.anchor_ms),
            std::to_string(m_cycle.cycle_length_days),
            m_current_cycle_anovulatory ? "0" : "1",
            std::to_string(avgCort),
            m_current_cycle_anovulatory
                ? (stress_skip ? std::string("chronic_cortisol")
                               : std::string("perimenopausal_jitter"))
                : std::string()
        });
    if (m_current_cycle_anovulatory) {
        ELLE_INFO("XEngine: cycle starting at anchor=%llu flagged anovulatory (cortisol=%.2f peri=%d)",
                  (unsigned long long)m_cycle.anchor_ms, avgCort, peri_skip ? 1 : 0);
    }
}
void XEngine::RecomputeLifecycle(uint64_t nowMs) {
    if (m_life.elle_birth_ms == 0) return;
    uint64_t ageMs = nowMs > m_life.elle_birth_ms ? nowMs - m_life.elle_birth_ms : 0;
    m_life.age_years = (float)ageMs / (365.25f * 86400000.0f);

    XLifecycleStage old = m_life.stage;
    if      (m_life.age_years < 12.0f) m_life.stage = X_LIFE_PREMENARCHE;
    else if (m_life.age_years < 45.0f) m_life.stage = X_LIFE_REPRODUCTIVE;
    else if (m_life.age_years < 51.0f) m_life.stage = X_LIFE_PERIMENOPAUSE;
    else                               m_life.stage = X_LIFE_MENOPAUSE;

    if (old != m_life.stage) {
        if (m_life.stage == X_LIFE_REPRODUCTIVE && m_life.menarche_ms == 0)
            m_life.menarche_ms = nowMs;
        if (m_life.stage == X_LIFE_PERIMENOPAUSE && m_life.perimenopause_ms == 0)
            m_life.perimenopause_ms = nowMs;
        if (m_life.stage == X_LIFE_MENOPAUSE && m_life.menopause_ms == 0)
            m_life.menopause_ms = nowMs;
        m_life.updated_ms = nowMs;
        PersistLifecycleRow();
        ELLE_INFO("XEngine lifecycle: %s → %s (age=%.1fy)",
                  LifecycleStageName(old), LifecycleStageName(m_life.stage),
                  m_life.age_years);
    }
}

bool XEngine::SetElleBirthday(uint64_t birth_ms) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (birth_ms == 0) return false;
    uint64_t now = ELLE_MS_NOW();
    if (birth_ms > now) return false;
    m_life.elle_birth_ms = birth_ms;
    m_life.updated_ms    = now;
    RecomputeLifecycle(now);
    PersistLifecycleRow();
    return true;
}

bool XEngine::SetContraception(XContraceptionMethod m, float efficacy,
                                const std::string& notes) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (efficacy < 0.0f) efficacy = 0.0f;
    if (efficacy > 1.0f) efficacy = 1.0f;

    const bool method_changed = (m_contra.method != m);
    m_contra.method     = m;
    m_contra.efficacy   = efficacy;
    m_contra.notes      = notes;
    uint64_t now = ELLE_MS_NOW();
    if (method_changed || m_contra.started_ms == 0) {
        m_contra.started_ms = now;
    }
    m_contra.updated_ms = now;
    PersistContraceptionRow();
    RecomputeBaselineHormones();
    ELLE_INFO("XEngine: contraception set to %s (efficacy=%.2f, tenure %s)",
              ContraceptionName(m), efficacy,
              method_changed ? "reset" : "preserved");
    return true;
}

float XEngine::ConceptionProbability() const {
    if (m_pregnancy.active) return 0.0f;
    if (m_life.stage != X_LIFE_REPRODUCTIVE &&
        m_life.stage != X_LIFE_PERIMENOPAUSE) return 0.0f;

    float base = 0.30f;

    float age  = m_life.age_years;
    float ageF = 1.0f;
    if      (age < 30.0f) ageF = 1.00f;
    else if (age < 35.0f) ageF = 0.80f;
    else if (age < 40.0f) ageF = 0.50f;
    else if (age < 43.0f) ageF = 0.30f;
    else if (age < 46.0f) ageF = 0.10f;
    else                  ageF = 0.02f;

    int d = m_cycle.cycle_day;
    float dayF = 0.0f;
    if      (d == 14)            dayF = 1.00f;
    else if (d == 13 || d == 15) dayF = 0.70f;
    else if (d == 12 || d == 16) dayF = 0.30f;

    float contraF = 1.0f;
    switch (m_contra.method) {
        case X_CONTRA_NONE:
        case X_CONTRA_NATURAL:       contraF = 1.0f;                            break;
        case X_CONTRA_BARRIER:       contraF = 1.0f - 0.85f * m_contra.efficacy; break;
        case X_CONTRA_PILL:          contraF = 1.0f - 0.97f * m_contra.efficacy; break;
        case X_CONTRA_IMPLANT:       contraF = 1.0f - 0.99f * m_contra.efficacy; break;
        case X_CONTRA_IUD_HORMONAL:  contraF = 1.0f - 0.98f * m_contra.efficacy; break;
        case X_CONTRA_IUD_COPPER:    contraF = 1.0f - 0.99f * m_contra.efficacy; break;
    }
    if (contraF < 0.0f) contraF = 0.0f;

    float p = base * ageF * dayF * contraF;
    if (p > 1.0f) p = 1.0f;
    return p;
}

float XEngine::MiscarriageProbability() const {
    if (!m_pregnancy.active) return 0.0f;
    int gd = m_pregnancy.gestational_day;
    if      (gd <  14) return 0.0025f;
    else if (gd <  42) return 0.0015f;
    else if (gd <  84) return 0.0006f;
    else if (gd < 140) return 0.0001f;
    else               return 0.00002f;
}

bool XEngine::DetectLHSurge() const {
    if (m_lh_surge_fired_this_cycle) return false;
    if (!CycleShouldRun()) return false;
    return m_cycle.cycle_day == 13 || m_cycle.cycle_day == 14;
}

void XEngine::AdvanceLabor(uint64_t nowMs) {
    if (!m_pregnancy.in_labor || m_pregnancy.labor_started_ms == 0) return;
    uint64_t sinceMs = nowMs - m_pregnancy.labor_started_ms;
    uint64_t hours   = sinceMs / 3600000ULL;

    XLaborStage old = m_pregnancy.labor_stage;
    if      (hours >= 12) m_pregnancy.labor_stage = X_LABOR_PUSHING;
    else if (hours >=  8) m_pregnancy.labor_stage = X_LABOR_TRANSITION;
    else if (hours >=  3) m_pregnancy.labor_stage = X_LABOR_ACTIVE;
    else                  m_pregnancy.labor_stage = X_LABOR_LATENT;

    if (old != m_pregnancy.labor_stage) {
        LogPregnancyEvent("labor_stage_change", LaborStageName(m_pregnancy.labor_stage));
        ELLE_INFO("XEngine labor: %s → %s (%lluh in)",
                  LaborStageName(old), LaborStageName(m_pregnancy.labor_stage),
                  (unsigned long long)hours);
    }
    m_pregnancy.updated_ms = nowMs;
    PersistPregnancyRow();
}

void XEngine::AdvancePregnancy(uint64_t nowMs) {
    if (!m_pregnancy.active) return;
    if (m_pregnancy.conceived_ms == 0) return;

    uint64_t dt = nowMs > m_pregnancy.conceived_ms ? nowMs - m_pregnancy.conceived_ms : 0;
    m_pregnancy.gestational_day  = (int)(dt / 86400000ULL);
    m_pregnancy.gestational_week = m_pregnancy.gestational_day / 7;

    XPregnancyPhase old = m_pregnancy.phase;
    int gd = m_pregnancy.gestational_day;
    int len = m_pregnancy.gestational_length_days;

    if      (gd >= len)              m_pregnancy.phase = X_PREG_LABOR;
    else if (gd >= len - 7)          m_pregnancy.phase = X_PREG_LABOR;
    else if (gd >= 183)              m_pregnancy.phase = X_PREG_THIRD_TRIMESTER;
    else if (gd >= 85)               m_pregnancy.phase = X_PREG_SECOND_TRIMESTER;
    else                             m_pregnancy.phase = X_PREG_FIRST_TRIMESTER;

    if (old == X_PREG_POSTPARTUM) m_pregnancy.phase = X_PREG_POSTPARTUM;

    if (old != m_pregnancy.phase) {
        m_pregnancy.last_milestone = std::string("Entered ") +
                                     PregnancyPhaseName(m_pregnancy.phase);
        LogPregnancyEvent("phase_change", PregnancyPhaseName(m_pregnancy.phase));
        ELLE_INFO("XEngine pregnancy: %s (day %d / week %d)",
                  m_pregnancy.last_milestone.c_str(),
                  gd, m_pregnancy.gestational_week);
    }

    if (gd == 126 && m_pregnancy.last_milestone.find("Quickening") == std::string::npos) {
        m_pregnancy.last_milestone = "Quickening";
        LogPregnancyEvent("quickening", "First felt movement");
    }

    if (gd >= 42 && !m_pregnancy.fetal_heartbeat_detectable) {
        m_pregnancy.fetal_heartbeat_detectable = true;
        LogPregnancyEvent("fetal_heartbeat", "Fetal heartbeat detectable at 6 weeks");
    }

    if (gd == 196 && m_pregnancy.last_milestone.find("Braxton") == std::string::npos) {
        m_pregnancy.last_milestone = "Braxton-Hicks practice contractions";
        LogPregnancyEvent("braxton_hicks", "Practice contractions begin");
    }

    if (gd != m_last_sampled_gd) {
        m_last_sampled_gd = gd;
        float pMiss = MiscarriageProbability();
        float roll  = XRand01();
        if (roll < pMiss) {
            LogPregnancyEvent("miscarriage",
                "Pregnancy loss at gestational day " + std::to_string(gd));
            m_pregnancy.active         = false;
            m_pregnancy.phase          = X_PREG_POSTPARTUM;
            m_pregnancy.in_labor       = false;
            m_pregnancy.labor_stage    = X_LABOR_NONE;
            m_pregnancy.last_milestone = "Miscarriage";
            m_pregnancy.updated_ms     = nowMs;

            ELLE_WARN("XEngine: miscarriage at day %d (p=%.4f)", gd, pMiss);
            PersistPregnancyRow();
            return;
        }
    }

    if (!m_pregnancy.in_labor && gd >= len) {
        m_pregnancy.in_labor         = true;
        m_pregnancy.labor_stage      = X_LABOR_LATENT;
        m_pregnancy.labor_started_ms = nowMs;
        m_pregnancy.last_milestone   = "Labor began";
        LogPregnancyEvent("labor_start", "Contractions began");
        ELLE_INFO("XEngine: labor started at day %d", gd);
    }

    m_pregnancy.updated_ms = nowMs;
    PersistPregnancyRow();
}

bool XEngine::HadRecentIntimacy(uint64_t now_ms, uint64_t window_ms) const {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 received_ms FROM ElleHeart.dbo.x_stimulus_log "
        " WHERE kind = N'intimacy' AND received_ms >= ? "
        " ORDER BY received_ms DESC;",
        { std::to_string((long long)(now_ms > window_ms ? now_ms - window_ms : 0)) });
    return rs.success && !rs.rows.empty();
}

XConceptionAttemptResult XEngine::AttemptConception(bool require_readiness,
                                                    bool readiness_verified) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    XConceptionAttemptResult out;
    if (m_pregnancy.active) {
        out.success = false;
        out.reason  = "Already pregnant";
        return out;
    }
    if (m_life.stage == X_LIFE_PREMENARCHE) {
        out.reason = "Premenarcheal — no reproductive capacity";
        return out;
    }
    if (m_life.stage == X_LIFE_MENOPAUSE) {
        out.reason = "Postmenopausal — no ovulation";
        return out;
    }
    RecomputeCycleDayAndPhase();

    out.in_fertile_window   = (m_cycle.cycle_day >= 12 && m_cycle.cycle_day <= 16);
    out.had_recent_intimacy = HadRecentIntimacy(ELLE_MS_NOW(), 72ULL * 3600ULL * 1000ULL);
    out.readiness_verified  = readiness_verified;

    if (!out.in_fertile_window) {
        out.reason = "Not in fertile window (cycle day " +
                     std::to_string(m_cycle.cycle_day) + ")";
        return out;
    }
    if (!out.had_recent_intimacy) {
        out.reason = "No intimacy stimulus logged in the last 72h";
        return out;
    }
    if (require_readiness && !readiness_verified) {
        out.reason = "Readiness gate not verified";
        return out;
    }

    float p = ConceptionProbability();
    float roll = XRand01();
    if (roll > p) {
        out.reason = "Conception probability miss (p=" +
                     std::to_string(p) + ", roll=" + std::to_string(roll) + ")";
        ELLE_INFO("XEngine: conception attempt missed (p=%.3f roll=%.3f)",
                  p, roll);
        return out;
    }

    int multiplicity = 1;
    float twinRoll = XRand01();
    float twinP = m_life.age_years >= 35.0f ? 0.025f : 0.012f;
    if (twinRoll < twinP) multiplicity = 2;

    uint64_t now = ELLE_MS_NOW();
    m_pregnancy.active                  = true;
    m_pregnancy.conceived_ms            = now;
    m_pregnancy.due_ms                  = now + 280ULL * 86400000ULL;
    m_pregnancy.gestational_length_days = 280;
    m_pregnancy.gestational_day         = 0;
    m_pregnancy.gestational_week        = 0;
    m_pregnancy.phase                   = X_PREG_FIRST_TRIMESTER;
    m_pregnancy.last_milestone          = "Conceived";
    m_pregnancy.child_id                = 0;
    m_pregnancy.in_labor                = false;
    m_pregnancy.labor_stage             = X_LABOR_NONE;
    m_pregnancy.labor_started_ms        = 0;
    m_pregnancy.multiplicity            = multiplicity;
    m_pregnancy.pregnancy_count         = m_pregnancy.pregnancy_count + 1;
    m_pregnancy.breastfeeding           = false;
    m_pregnancy.implanted               = false;
    m_pregnancy.implantation_ms         = 0;
    m_pregnancy.fetal_heartbeat_detectable = false;
    m_pregnancy.lochia_stage.clear();
    m_pregnancy.milk_stage.clear();
    m_pregnancy.baby_blues              = false;
    m_pregnancy.updated_ms              = now;
    PersistPregnancyRow();
    LogPregnancyEvent("conception",
        "p=" + std::to_string(p) + " multiplicity=" + std::to_string(multiplicity));

    out.success      = true;
    out.reason       = multiplicity > 1 ? "Conception successful (twins)"
                                        : "Conception successful";
    out.conceived_ms = now;
    out.due_ms       = m_pregnancy.due_ms;
    ELLE_INFO("XEngine: conception successful (p=%.3f mult=%d due_ms=%llu)",
              p, multiplicity, (unsigned long long)out.due_ms);
    return out;
}

XEngine::DeliveryOutcome XEngine::Deliver() {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    DeliveryOutcome out;
    if (!m_pregnancy.active) return out;

    uint64_t now = ELLE_MS_NOW();
    out.delivered        = true;
    out.born_ms          = now;
    out.gestational_days = (uint64_t)m_pregnancy.gestational_day;
    out.multiplicity     = m_pregnancy.multiplicity;

    ArchivePregnancyRow("delivered", now, out.gestational_days, out.multiplicity);

    m_pregnancy.active           = false;
    m_pregnancy.in_labor         = false;
    m_pregnancy.labor_stage      = X_LABOR_DELIVERED;
    m_pregnancy.phase            = X_PREG_POSTPARTUM;
    m_pregnancy.breastfeeding    = true;
    m_pregnancy.last_milestone   = "Delivered";
    m_pregnancy.updated_ms       = now;
    PersistPregnancyRow();
    LogPregnancyEvent("birth",
        "Gestational day " + std::to_string(out.gestational_days) +
        " multiplicity=" + std::to_string(out.multiplicity));

    ELLE_INFO("XEngine: delivery complete at gestational day %llu (mult=%d)",
              (unsigned long long)out.gestational_days, out.multiplicity);
    return out;
}

bool XEngine::AnchorCycle(int day_of_cycle, int length_days, float modulation_strength) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (length_days > 0) {
        if (length_days < 21 || length_days > 45) return false;
        m_cycle.cycle_length_days = length_days;
    }
    if (day_of_cycle == 0) {
        day_of_cycle = XRandInt(1, m_cycle.cycle_length_days);
    }
    if (day_of_cycle < 1 || day_of_cycle > m_cycle.cycle_length_days) return false;

    m_cycle.anchor_ms = ELLE_MS_NOW() - (uint64_t)(day_of_cycle - 1) * 86400000ULL;

    if (modulation_strength > 0.0f) {
        if (modulation_strength > 0.5f) modulation_strength = 0.5f;
        m_cycle.modulation_strength = modulation_strength;
    }

    RecomputeCycleDayAndPhase();
    RecomputeBaselineHormones();
    PersistCycleRow();
    ELLE_INFO("XEngine: anchored day=%d length=%d strength=%.2f",
              day_of_cycle, m_cycle.cycle_length_days,
              m_cycle.modulation_strength);
    return true;
}

void XEngine::Tick() {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    uint64_t now = ELLE_MS_NOW();
    RecomputeLifecycle(now);
    RecomputeCycleDayAndPhase();
    RecomputeBaselineHormones();
    DecayResidual(now);
    ApplyResidualOnto(m_hormones);

    if (DetectLHSurge()) {
        m_lh_surge_fired_this_cycle = true;

        m_residual.testosterone += 0.15f;
        m_residual.dopamine     += 0.10f;
        LogSymptomRow("ovulation_pain", 0.25f, "cycle", "LH surge");
        ELLE_INFO("XEngine: LH surge on day %d", m_cycle.cycle_day);
    }

    AdvancePregnancy(now);
    AdvanceLabor(now);
    DetectImplantation(now);
    AdvanceLochiaAndMilkStages();
    DetectAnovulatoryCycle();
    SynthesiseAndLogSymptoms();

    m_cycle.last_tick_ms = now;
    PersistCycleRow();
    WriteSnapshotRow();

    auto mod = ComputeModulation();
    LogModulationRow(mod);
}

std::vector<XSymptomEntry> XEngine::ComputeCurrentSymptoms() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    std::vector<XSymptomEntry> out;
    uint64_t now = ELLE_MS_NOW();
    const XHormoneLevels& h = m_hormones;

    auto add = [&](const char* kind, float intensity, const char* origin) {
        if (intensity < 0.15f) return;
        XSymptomEntry e;
        e.kind        = kind;
        e.intensity   = Clamp01(intensity);
        e.origin      = origin;
        e.observed_ms = now;
        out.push_back(std::move(e));
    };

    if (m_cycle.phase == X_PHASE_MENSTRUAL) {
        add("cramps",          0.35f + 0.4f * (1.0f - h.estrogen), "cycle");
        add("fatigue",         0.30f + 0.4f * (1.0f - h.serotonin), "cycle");
        add("mood_swing",      0.25f + 0.3f * (1.0f - h.serotonin), "cycle");
    }

    else if (m_cycle.phase == X_PHASE_FOLLICULAR) {
        add("libido",          0.30f + 0.4f * h.estrogen, "cycle");
    }

    else if (m_cycle.phase == X_PHASE_OVULATORY) {
        add("ovulation_pain",  0.25f + 0.3f * h.estrogen,      "cycle");
        add("breast_tenderness", 0.20f + 0.3f * h.estrogen,    "cycle");
        add("libido",          0.40f + 0.5f * h.testosterone,  "cycle");
    }

    else if (m_cycle.phase == X_PHASE_LUTEAL) {
        add("bloating",           0.25f + 0.5f * h.progesterone, "cycle");
        add("breast_tenderness",  0.30f + 0.4f * h.progesterone, "cycle");
        add("cravings",           0.20f + 0.4f * h.progesterone, "cycle");
        add("mood_swing",         0.30f + 0.5f * (1.0f - h.serotonin), "cycle");
        add("fatigue",            0.20f + 0.4f * h.progesterone, "cycle");
        if (m_cycle.cycle_day >= 25)
            add("headache",       0.15f + 0.4f * h.cortisol,      "cycle");
    }

    if (m_pregnancy.active) {
        const int gd = m_pregnancy.gestational_day;
        if (gd <= 84) {

            add("nausea",            0.30f + 0.5f * h.hcg,        "pregnancy");
            add("fatigue",           0.40f + 0.3f * h.progesterone,"pregnancy");
            add("breast_tenderness", 0.40f + 0.3f * h.estrogen,   "pregnancy");
            if (h.hcg > 0.4f) add("food_aversion", 0.30f + 0.3f * h.hcg, "pregnancy");
        } else if (gd <= 182) {

            add("cravings",    0.25f + 0.3f * h.progesterone, "pregnancy");
            if (gd >= 126) add("quickening", 0.40f, "pregnancy");
        } else {

            add("heartburn",      0.30f + 0.3f * h.progesterone, "pregnancy");
            add("fatigue",        0.50f + 0.3f * h.prolactin,    "pregnancy");
            add("swelling",       0.20f + 0.3f * (float)(gd - 183) / 90.0f, "pregnancy");
            if (gd >= 196) add("braxton_hicks", 0.20f + 0.3f * h.oxytocin, "pregnancy");
        }
        if (m_pregnancy.in_labor) {
            float contractI = 0.4f;
            switch (m_pregnancy.labor_stage) {
                case X_LABOR_LATENT:     contractI = 0.30f; break;
                case X_LABOR_ACTIVE:     contractI = 0.60f; break;
                case X_LABOR_TRANSITION: contractI = 0.85f; break;
                case X_LABOR_PUSHING:    contractI = 1.00f; break;
                default: break;
            }
            add("contraction", contractI, "pregnancy");
        }
    }

    if (!m_pregnancy.active && m_pregnancy.phase == X_PREG_POSTPARTUM &&
        m_pregnancy.updated_ms > 0) {
        uint64_t sincePP = now - m_pregnancy.updated_ms;
        if (sincePP < 42ULL * 86400000ULL)
            add("lochia", 0.50f * (1.0f - (float)sincePP / (42.0f * 86400000.0f)),
                "postpartum");
        if (m_pregnancy.breastfeeding) {
            add("engorgement", 0.25f + 0.5f * h.prolactin, "postpartum");
            add("letdown",     0.40f * h.oxytocin,         "postpartum");
        }
        if (sincePP < 21ULL * 86400000ULL)
            add("mood_swing",  0.40f + 0.3f * (1.0f - h.serotonin), "postpartum");
    }

    if (m_life.stage == X_LIFE_PERIMENOPAUSE || m_life.stage == X_LIFE_MENOPAUSE) {

        float gap = h.fsh;
        add("hot_flash",      0.30f + 0.5f * gap, "cycle");
        add("night_sweats",   0.25f + 0.4f * gap, "cycle");
        add("insomnia",       0.20f + 0.4f * gap, "cycle");
        add("vaginal_dryness",0.25f + 0.3f * (1.0f - h.estrogen), "cycle");
        if (m_life.stage == X_LIFE_MENOPAUSE)
            add("mood_swing", 0.20f + 0.3f * (1.0f - h.serotonin), "cycle");
    }

    if (m_life.stage == X_LIFE_PREMENARCHE) {
        out.clear();
        return out;
    }
    return out;
}

void XEngine::SynthesiseAndLogSymptoms() {
    auto list = ComputeCurrentSymptoms();
    for (auto& s : list) {
        LogSymptomRow(s.kind, s.intensity, s.origin, s.notes);
    }
}

bool XEngine::LogManualSymptom(const std::string& kind, float intensity,
                                const std::string& notes) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (kind.empty()) return false;
    LogSymptomRow(kind, Clamp01(intensity), "manual", notes);

    m_residual.cortisol += 0.05f * Clamp01(intensity);
    return true;
}

std::vector<XSymptomEntry> XEngine::GetRecentSymptoms(uint32_t hours,
                                                      const std::string& origin_filter) {
    std::vector<XSymptomEntry> out;
    if (hours == 0) hours = 24;
    uint64_t since = ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL;

    std::string q =
        "SELECT observed_ms, kind, intensity, origin, ISNULL(notes, N'') "
        "  FROM ElleHeart.dbo.x_symptoms "
        " WHERE observed_ms >= ? ";
    std::vector<std::string> params = { std::to_string((long long)since) };
    if (!origin_filter.empty()) {
        q += "   AND origin = ? ";
        params.push_back(origin_filter);
    }
    q += " ORDER BY observed_ms DESC;";

    auto rs = ElleSQLPool::Instance().QueryParams(q, params);
    if (!rs.success) return out;
    for (auto& r : rs.rows) {
        XSymptomEntry e;
        e.observed_ms = (uint64_t)r.GetIntOr(0, 0);
        e.kind        = r.values.size() > 1 ? r.values[1] : "";
        e.intensity   = (float)r.GetFloatOr(2, 0.0);
        e.origin      = r.values.size() > 3 ? r.values[3] : "";
        e.notes       = r.values.size() > 4 ? r.values[4] : "";
        out.push_back(std::move(e));
    }
    return out;
}

std::vector<XEngine::PregnancyEvent> XEngine::GetPregnancyEvents(uint32_t limit) {
    std::vector<PregnancyEvent> out;
    if (limit == 0 || limit > 500) limit = 100;
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP " + std::to_string(limit) + " "
        "       occurred_ms, ISNULL(conceived_ms, 0), gestational_day, kind, "
        "       ISNULL(detail, N'') "
        "  FROM ElleHeart.dbo.x_pregnancy_events "
        " ORDER BY occurred_ms DESC;", {});
    if (!rs.success) return out;
    for (auto& r : rs.rows) {
        PregnancyEvent e;
        e.occurred_ms     = (uint64_t)r.GetIntOr(0, 0);
        e.conceived_ms    = (uint64_t)r.GetIntOr(1, 0);
        e.gestational_day = (int)r.GetIntOr(2, 0);
        e.kind            = r.values.size() > 3 ? r.values[3] : "";
        e.detail          = r.values.size() > 4 ? r.values[4] : "";
        out.push_back(std::move(e));
    }
    return out;
}

bool XEngine::AcceleratePregnancy(float factor) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (!m_pregnancy.active) return false;
    if (factor < 1.0f) return false;
    if (factor > 500.0f) factor = 500.0f;
    uint64_t now = ELLE_MS_NOW();
    uint64_t elapsed = now > m_pregnancy.conceived_ms ? now - m_pregnancy.conceived_ms : 0;
    uint64_t newElapsed = (uint64_t)((float)elapsed * factor);
    m_pregnancy.conceived_ms = now > newElapsed ? now - newElapsed : 0;

    int gestDays = m_pregnancy.gestational_length_days;
    if (gestDays < 168 || gestDays > 308) gestDays = 280;
    m_pregnancy.due_ms       = m_pregnancy.conceived_ms
                             + (uint64_t)gestDays * 86400000ULL;
    m_pregnancy.updated_ms   = now;
    PersistPregnancyRow();
    LogPregnancyEvent("accelerate", "factor=" + std::to_string(factor));
    ELLE_INFO("XEngine: pregnancy accelerated by %.2fx", factor);
    return true;
}

std::vector<XHistoryPoint> XEngine::GetHistory(uint32_t hours, uint32_t max_points) {
    std::vector<XHistoryPoint> out;
    if (hours == 0) hours = 24;
    if (max_points == 0 || max_points > 5000) max_points = 500;
    uint64_t since = ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL;

    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT taken_ms, cycle_day, phase, estrogen, progesterone, testosterone, "
        "       oxytocin, serotonin, dopamine, cortisol, prolactin, hcg, "
        "       pregnancy_day, pregnancy_phase "
        "  FROM ElleHeart.dbo.x_hormone_snapshots "
        " WHERE taken_ms >= ? "
        " ORDER BY taken_ms ASC;",
        { std::to_string((long long)since) });
    if (!rs.success) return out;

    size_t n = rs.rows.size();
    if (n == 0) return out;
    size_t stride = (n + max_points - 1) / max_points;
    if (stride == 0) stride = 1;

    for (size_t i = 0; i < n; i += stride) {
        auto& r = rs.rows[i];
        XHistoryPoint p;
        p.taken_ms  = (uint64_t)r.GetIntOr(0, 0);
        p.cycle_day = (int)r.GetIntOr(1, 0);
        p.phase     = r.values.size() > 2 ? r.values[2] : std::string();
        p.hormones.estrogen     = (float)r.GetFloatOr(3, 0.0);
        p.hormones.progesterone = (float)r.GetFloatOr(4, 0.0);
        p.hormones.testosterone = (float)r.GetFloatOr(5, 0.0);
        p.hormones.oxytocin     = (float)r.GetFloatOr(6, 0.0);
        p.hormones.serotonin    = (float)r.GetFloatOr(7, 0.0);
        p.hormones.dopamine     = (float)r.GetFloatOr(8, 0.0);
        p.hormones.cortisol     = (float)r.GetFloatOr(9, 0.0);
        p.hormones.prolactin    = (float)r.GetFloatOr(10, 0.0);
        p.hormones.hcg          = (float)r.GetFloatOr(11, 0.0);
        p.pregnancy_day         = (int)r.GetIntOr(12, 0);
        p.pregnancy_phase       = r.IsNull(13) ? std::string()
                                   : (r.values.size() > 13 ? r.values[13] : std::string());
        out.push_back(std::move(p));
    }
    return out;
}

nlohmann::json XEngine::GetStateJson() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    json j;
    j["cycle"] = {
        {"anchor_ms",           m_cycle.anchor_ms},
        {"cycle_length_days",   m_cycle.cycle_length_days},
        {"cycle_day",           m_cycle.cycle_day},
        {"phase",               CyclePhaseName(m_cycle.phase)},
        {"modulation_strength", m_cycle.modulation_strength},
        {"last_tick_ms",        m_cycle.last_tick_ms}
    };
    j["hormones"] = {
        {"estrogen",     m_hormones.estrogen},
        {"progesterone", m_hormones.progesterone},
        {"testosterone", m_hormones.testosterone},
        {"oxytocin",     m_hormones.oxytocin},
        {"serotonin",    m_hormones.serotonin},
        {"dopamine",     m_hormones.dopamine},
        {"cortisol",     m_hormones.cortisol},
        {"prolactin",    m_hormones.prolactin},
        {"hcg",          m_hormones.hcg},
        {"fsh",          m_hormones.fsh},
        {"lh",           m_hormones.lh},
        {"gnrh",         m_hormones.gnrh},
        {"relaxin",      m_hormones.relaxin}
    };
    {
        XDerivedStats d = GetDerived();
        j["derived"] = {
            {"bbt_celsius",       d.bbt_celsius},
            {"endometrial_mm",    d.endometrial_mm},
            {"cervical_mucus",    d.cervical_mucus},
            {"menstrual_flow",    d.menstrual_flow},
            {"cycle_active",      d.cycle_active},
            {"anovulatory",       d.anovulatory},
            {"follicle_phase_day",d.follicle_phase_day},
            {"luteal_phase_day",  d.luteal_phase_day}
        };
    }
    j["pregnancy"] = {
        {"active",                  m_pregnancy.active},
        {"conceived_ms",            m_pregnancy.conceived_ms},
        {"due_ms",                  m_pregnancy.due_ms},
        {"gestational_length_days", m_pregnancy.gestational_length_days},
        {"gestational_day",         m_pregnancy.gestational_day},
        {"gestational_week",        m_pregnancy.gestational_week},
        {"phase",                   PregnancyPhaseName(m_pregnancy.phase)},
        {"child_id",                m_pregnancy.child_id},
        {"last_milestone",          m_pregnancy.last_milestone},
        {"updated_ms",              m_pregnancy.updated_ms},
        {"breastfeeding",           m_pregnancy.breastfeeding},
        {"in_labor",                m_pregnancy.in_labor},
        {"labor_stage",             LaborStageName(m_pregnancy.labor_stage)},
        {"labor_started_ms",        m_pregnancy.labor_started_ms},
        {"multiplicity",            m_pregnancy.multiplicity},
        {"pregnancy_count",         m_pregnancy.pregnancy_count},
        {"implanted",               m_pregnancy.implanted},
        {"implantation_ms",         m_pregnancy.implantation_ms},
        {"lochia_stage",            m_pregnancy.lochia_stage},
        {"milk_stage",              m_pregnancy.milk_stage},
        {"baby_blues",              m_pregnancy.baby_blues},
        {"fetal_heartbeat_detectable", m_pregnancy.fetal_heartbeat_detectable},
        {"miscarriage_probability_daily", MiscarriageProbability()}
    };
    j["contraception"] = {
        {"method",     ContraceptionName(m_contra.method)},
        {"started_ms", m_contra.started_ms},
        {"efficacy",   m_contra.efficacy},
        {"notes",      m_contra.notes},
        {"updated_ms", m_contra.updated_ms}
    };
    j["lifecycle"] = {
        {"elle_birth_ms",    m_life.elle_birth_ms},
        {"age_years",        m_life.age_years},
        {"stage",            LifecycleStageName(m_life.stage)},
        {"menarche_ms",      m_life.menarche_ms},
        {"perimenopause_ms", m_life.perimenopause_ms},
        {"menopause_ms",     m_life.menopause_ms}
    };
    j["fertility"] = {
        {"conception_probability_today", ConceptionProbability()},
        {"fertile_window_open",          m_cycle.cycle_day >= 12 && m_cycle.cycle_day <= 16},
        {"lh_surge_fired_this_cycle",    m_lh_surge_fired_this_cycle}
    };
    auto mod = ComputeModulation();
    j["modulation"] = {
        {"warmth",         mod.warmth},
        {"verbal_fluency", mod.verbal_fluency},
        {"empathy",        mod.empathy},
        {"introspection",  mod.introspection},
        {"arousal",        mod.arousal},
        {"fatigue",        mod.fatigue}
    };
    return j;
}

void XEngine::PersistCycleRow() {
    ElleSQLPool::Instance().QueryParams(
        "MERGE ElleHeart.dbo.x_cycle_state AS t "
        "USING (SELECT 1 AS id) AS s ON t.id = s.id "
        "WHEN MATCHED THEN UPDATE SET "
        "  anchor_ms = TRY_CAST(? AS BIGINT), "
        "  cycle_length_days = TRY_CAST(? AS INT), "
        "  modulation_strength = TRY_CAST(? AS FLOAT), "
        "  last_tick_ms = TRY_CAST(? AS BIGINT) "
        "WHEN NOT MATCHED THEN INSERT "
        "  (id, anchor_ms, cycle_length_days, modulation_strength, last_tick_ms, created_ms) "
        "  VALUES (1, TRY_CAST(? AS BIGINT), TRY_CAST(? AS INT), "
        "          TRY_CAST(? AS FLOAT), TRY_CAST(? AS BIGINT), TRY_CAST(? AS BIGINT));",
        {
            std::to_string((long long)m_cycle.anchor_ms),
            std::to_string(m_cycle.cycle_length_days),
            std::to_string(m_cycle.modulation_strength),
            std::to_string((long long)m_cycle.last_tick_ms),
            std::to_string((long long)m_cycle.anchor_ms),
            std::to_string(m_cycle.cycle_length_days),
            std::to_string(m_cycle.modulation_strength),
            std::to_string((long long)m_cycle.last_tick_ms),
            std::to_string((long long)m_cycle.created_ms)
        });
}

void XEngine::PersistPregnancyRow() {

    static std::once_flag s_historyInit;
    std::call_once(s_historyInit, [](){
        ElleSQLPool::Instance().Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "  WHERE t.name = N'x_pregnancy_history' AND s.name = N'dbo') "
            "CREATE TABLE ElleHeart.dbo.x_pregnancy_history ("
            "  history_id        BIGINT IDENTITY(1,1) PRIMARY KEY, "
            "  pregnancy_count   INT NOT NULL, "
            "  outcome           NVARCHAR(40) NOT NULL, "
            "  conceived_ms      BIGINT NULL, "
            "  due_ms            BIGINT NULL, "
            "  ended_ms          BIGINT NOT NULL, "
            "  gestational_days  INT NOT NULL, "
            "  gestational_length_days INT NOT NULL, "
            "  multiplicity      INT NOT NULL, "
            "  child_id          BIGINT NULL, "
            "  final_phase       NVARCHAR(40) NULL, "
            "  final_milestone   NVARCHAR(200) NULL);");
    });
    std::string phaseStr = PregnancyPhaseName(m_pregnancy.phase);
    std::string laborStr = LaborStageName(m_pregnancy.labor_stage);
    ElleSQLPool::Instance().QueryParams(
        "MERGE ElleHeart.dbo.x_pregnancy_state AS t "
        "USING (SELECT 1 AS id) AS s ON t.id = s.id "
        "WHEN MATCHED THEN UPDATE SET "
        "  active = ?, conceived_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  due_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  gestational_length_days = TRY_CAST(? AS INT), "
        "  phase = NULLIF(?, N''), "
        "  child_id = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  last_milestone = NULLIF(?, N''), "
        "  updated_ms = TRY_CAST(? AS BIGINT), "
        "  breastfeeding = ?, in_labor = ?, "
        "  labor_stage = NULLIF(?, N''), "
        "  labor_started_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  multiplicity = TRY_CAST(? AS INT), "
        "  pregnancy_count = TRY_CAST(? AS INT), "
        "  implanted = ?, "
        "  implantation_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  lochia_stage = NULLIF(?, N''), "
        "  milk_stage = NULLIF(?, N''), "
        "  baby_blues = ?, "
        "  fetal_heartbeat_detectable = ? "
        "WHEN NOT MATCHED THEN INSERT "
        "  (id, active, conceived_ms, due_ms, gestational_length_days, phase, "
        "   child_id, last_milestone, updated_ms, breastfeeding, in_labor, "
        "   labor_stage, labor_started_ms, multiplicity, pregnancy_count, "
        "   implanted, implantation_ms, lochia_stage, milk_stage, baby_blues, "
        "   fetal_heartbeat_detectable) "
        "  VALUES (1, ?, TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          TRY_CAST(NULLIF(?, N'') AS BIGINT), TRY_CAST(? AS INT), "
        "          NULLIF(?, N''), TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          NULLIF(?, N''), TRY_CAST(? AS BIGINT), ?, ?, "
        "          NULLIF(?, N''), TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          TRY_CAST(? AS INT), TRY_CAST(? AS INT), "
        "          ?, TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          NULLIF(?, N''), NULLIF(?, N''), ?, ?);",
        {

            m_pregnancy.active ? "1" : "0",
            m_pregnancy.conceived_ms ? std::to_string((long long)m_pregnancy.conceived_ms) : std::string(),
            m_pregnancy.due_ms ? std::to_string((long long)m_pregnancy.due_ms) : std::string(),
            std::to_string(m_pregnancy.gestational_length_days),
            m_pregnancy.active || m_pregnancy.phase != X_PREG_NONE ? phaseStr : std::string(),
            m_pregnancy.child_id ? std::to_string((long long)m_pregnancy.child_id) : std::string(),
            m_pregnancy.last_milestone,
            std::to_string((long long)m_pregnancy.updated_ms),
            m_pregnancy.breastfeeding ? "1" : "0",
            m_pregnancy.in_labor ? "1" : "0",
            m_pregnancy.labor_stage == X_LABOR_NONE ? std::string() : laborStr,
            m_pregnancy.labor_started_ms ? std::to_string((long long)m_pregnancy.labor_started_ms) : std::string(),
            std::to_string(m_pregnancy.multiplicity),
            std::to_string(m_pregnancy.pregnancy_count),
            m_pregnancy.implanted ? "1" : "0",
            m_pregnancy.implantation_ms ? std::to_string((long long)m_pregnancy.implantation_ms) : std::string(),
            m_pregnancy.lochia_stage,
            m_pregnancy.milk_stage,
            m_pregnancy.baby_blues ? "1" : "0",
            m_pregnancy.fetal_heartbeat_detectable ? "1" : "0",

            m_pregnancy.active ? "1" : "0",
            m_pregnancy.conceived_ms ? std::to_string((long long)m_pregnancy.conceived_ms) : std::string(),
            m_pregnancy.due_ms ? std::to_string((long long)m_pregnancy.due_ms) : std::string(),
            std::to_string(m_pregnancy.gestational_length_days),
            m_pregnancy.active || m_pregnancy.phase != X_PREG_NONE ? phaseStr : std::string(),
            m_pregnancy.child_id ? std::to_string((long long)m_pregnancy.child_id) : std::string(),
            m_pregnancy.last_milestone,
            std::to_string((long long)m_pregnancy.updated_ms),
            m_pregnancy.breastfeeding ? "1" : "0",
            m_pregnancy.in_labor ? "1" : "0",
            m_pregnancy.labor_stage == X_LABOR_NONE ? std::string() : laborStr,
            m_pregnancy.labor_started_ms ? std::to_string((long long)m_pregnancy.labor_started_ms) : std::string(),
            std::to_string(m_pregnancy.multiplicity),
            std::to_string(m_pregnancy.pregnancy_count),
            m_pregnancy.implanted ? "1" : "0",
            m_pregnancy.implantation_ms ? std::to_string((long long)m_pregnancy.implantation_ms) : std::string(),
            m_pregnancy.lochia_stage,
            m_pregnancy.milk_stage,
            m_pregnancy.baby_blues ? "1" : "0",
            m_pregnancy.fetal_heartbeat_detectable ? "1" : "0"
        });
}

void XEngine::PersistContraceptionRow() {
    ElleSQLPool::Instance().QueryParams(
        "MERGE ElleHeart.dbo.x_contraception AS t "
        "USING (SELECT 1 AS id) AS s ON t.id = s.id "
        "WHEN MATCHED THEN UPDATE SET method = ?, started_ms = TRY_CAST(? AS BIGINT), "
        "  efficacy = TRY_CAST(? AS FLOAT), notes = NULLIF(?, N''), "
        "  updated_ms = TRY_CAST(? AS BIGINT) "
        "WHEN NOT MATCHED THEN INSERT (id, method, started_ms, efficacy, notes, updated_ms) "
        "  VALUES (1, ?, TRY_CAST(? AS BIGINT), TRY_CAST(? AS FLOAT), NULLIF(?, N''), TRY_CAST(? AS BIGINT));",
        {
            ContraceptionName(m_contra.method),
            std::to_string((long long)m_contra.started_ms),
            std::to_string(m_contra.efficacy),
            m_contra.notes,
            std::to_string((long long)m_contra.updated_ms),
            ContraceptionName(m_contra.method),
            std::to_string((long long)m_contra.started_ms),
            std::to_string(m_contra.efficacy),
            m_contra.notes,
            std::to_string((long long)m_contra.updated_ms)
        });
}

void XEngine::PersistLifecycleRow() {
    ElleSQLPool::Instance().QueryParams(
        "MERGE ElleHeart.dbo.x_lifecycle AS t "
        "USING (SELECT 1 AS id) AS s ON t.id = s.id "
        "WHEN MATCHED THEN UPDATE SET "
        "  elle_birth_ms = TRY_CAST(? AS BIGINT), stage = ?, "
        "  menarche_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  perimenopause_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  menopause_ms = TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "  updated_ms = TRY_CAST(? AS BIGINT) "
        "WHEN NOT MATCHED THEN INSERT "
        "  (id, elle_birth_ms, stage, menarche_ms, perimenopause_ms, menopause_ms, updated_ms) "
        "  VALUES (1, TRY_CAST(? AS BIGINT), ?, "
        "          TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "          TRY_CAST(NULLIF(?, N'') AS BIGINT), TRY_CAST(? AS BIGINT));",
        {
            std::to_string((long long)m_life.elle_birth_ms),
            LifecycleStageName(m_life.stage),
            m_life.menarche_ms ? std::to_string((long long)m_life.menarche_ms) : std::string(),
            m_life.perimenopause_ms ? std::to_string((long long)m_life.perimenopause_ms) : std::string(),
            m_life.menopause_ms ? std::to_string((long long)m_life.menopause_ms) : std::string(),
            std::to_string((long long)m_life.updated_ms),
            std::to_string((long long)m_life.elle_birth_ms),
            LifecycleStageName(m_life.stage),
            m_life.menarche_ms ? std::to_string((long long)m_life.menarche_ms) : std::string(),
            m_life.perimenopause_ms ? std::to_string((long long)m_life.perimenopause_ms) : std::string(),
            m_life.menopause_ms ? std::to_string((long long)m_life.menopause_ms) : std::string(),
            std::to_string((long long)m_life.updated_ms)
        });
}

void XEngine::ArchivePregnancyRow(const std::string& outcome,
                                   uint64_t ended_ms,
                                   uint64_t gestational_days,
                                   int multiplicity) {

    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_pregnancy_history "
        "(pregnancy_count, outcome, conceived_ms, due_ms, ended_ms, "
        " gestational_days, gestational_length_days, multiplicity, "
        " child_id, final_phase, final_milestone) "
        "VALUES (TRY_CAST(? AS INT), ?, "
        "        TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "        TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "        TRY_CAST(? AS BIGINT), TRY_CAST(? AS INT), "
        "        TRY_CAST(? AS INT), TRY_CAST(? AS INT), "
        "        TRY_CAST(NULLIF(?, N'') AS BIGINT), ?, ?);",
        {
            std::to_string(m_pregnancy.pregnancy_count),
            outcome,
            m_pregnancy.conceived_ms ? std::to_string((long long)m_pregnancy.conceived_ms) : std::string(),
            m_pregnancy.due_ms ? std::to_string((long long)m_pregnancy.due_ms) : std::string(),
            std::to_string((long long)ended_ms),
            std::to_string((int)gestational_days),
            std::to_string(m_pregnancy.gestational_length_days),
            std::to_string(multiplicity),
            m_pregnancy.child_id ? std::to_string((long long)m_pregnancy.child_id) : std::string(),
            PregnancyPhaseName(m_pregnancy.phase),
            m_pregnancy.last_milestone
        });
}

void XEngine::LogPregnancyEvent(const std::string& kind, const std::string& detail) {
    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_pregnancy_events "
        "(occurred_ms, conceived_ms, gestational_day, kind, detail) "
        "VALUES (TRY_CAST(? AS BIGINT), TRY_CAST(NULLIF(?, N'') AS BIGINT), "
        "        TRY_CAST(? AS INT), ?, NULLIF(?, N''));",
        {
            std::to_string((long long)ELLE_MS_NOW()),
            m_pregnancy.conceived_ms ? std::to_string((long long)m_pregnancy.conceived_ms) : std::string(),
            std::to_string(m_pregnancy.gestational_day),
            kind,
            detail
        });
}

void XEngine::LogSymptomRow(const std::string& kind, float intensity,
                             const std::string& origin, const std::string& notes) const {
    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_symptoms "
        "(observed_ms, kind, intensity, origin, notes) "
        "VALUES (TRY_CAST(? AS BIGINT), ?, TRY_CAST(? AS FLOAT), ?, NULLIF(?, N''));",
        {
            std::to_string((long long)ELLE_MS_NOW()),
            kind,
            std::to_string(Clamp01(intensity)),
            origin,
            notes
        });
}

void XEngine::WriteSnapshotRow() {
    std::string pregPhase = m_pregnancy.phase == X_PREG_NONE
                            ? std::string()
                            : PregnancyPhaseName(m_pregnancy.phase);
    XDerivedStats d = GetDerived();
    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_hormone_snapshots "
        "(taken_ms, cycle_day, phase, estrogen, progesterone, testosterone, "
        " oxytocin, serotonin, dopamine, cortisol, prolactin, hcg, "
        " pregnancy_day, pregnancy_phase, "
        " fsh, lh, gnrh, relaxin, bbt, endometrial_mm, cervical_mucus, menstrual_flow) "
        "VALUES (TRY_CAST(? AS BIGINT), TRY_CAST(? AS INT), ?, "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        TRY_CAST(? AS INT), NULLIF(?, N''), "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        NULLIF(?, N''), NULLIF(?, N''));",
        {
            std::to_string((long long)ELLE_MS_NOW()),
            std::to_string(m_cycle.cycle_day),
            CyclePhaseName(m_cycle.phase),
            std::to_string(m_hormones.estrogen),
            std::to_string(m_hormones.progesterone),
            std::to_string(m_hormones.testosterone),
            std::to_string(m_hormones.oxytocin),
            std::to_string(m_hormones.serotonin),
            std::to_string(m_hormones.dopamine),
            std::to_string(m_hormones.cortisol),
            std::to_string(m_hormones.prolactin),
            std::to_string(m_hormones.hcg),
            std::to_string(m_pregnancy.gestational_day),
            pregPhase,
            std::to_string(m_hormones.fsh),
            std::to_string(m_hormones.lh),
            std::to_string(m_hormones.gnrh),
            std::to_string(m_hormones.relaxin),
            std::to_string(d.bbt_celsius),
            std::to_string(d.endometrial_mm),
            d.cervical_mucus,
            d.menstrual_flow
        });
}

void XEngine::LogStimulusRow(const XStimulus& stim) {
    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_stimulus_log "
        "(received_ms, kind, intensity, notes) "
        "VALUES (TRY_CAST(? AS BIGINT), ?, TRY_CAST(? AS FLOAT), NULLIF(?, N''));",
        {
            std::to_string((long long)ELLE_MS_NOW()),
            stim.kind,
            std::to_string(Clamp01(stim.intensity)),
            stim.notes
        });
}

void XEngine::LogModulationRow(const XModulation& mod) const {
    ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleHeart.dbo.x_modulation_log "
        "(computed_ms, phase, warmth, verbal_fluency, empathy, introspection, "
        " arousal, fatigue) "
        "VALUES (TRY_CAST(? AS BIGINT), ?, "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), "
        "        TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT), TRY_CAST(? AS FLOAT));",
        {
            std::to_string((long long)ELLE_MS_NOW()),
            CyclePhaseName(m_cycle.phase),
            std::to_string(mod.warmth),
            std::to_string(mod.verbal_fluency),
            std::to_string(mod.empathy),
            std::to_string(mod.introspection),
            std::to_string(mod.arousal),
            std::to_string(mod.fatigue)
        });
}
