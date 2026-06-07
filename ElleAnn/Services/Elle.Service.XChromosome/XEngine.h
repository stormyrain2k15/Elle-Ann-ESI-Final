#pragma once
#ifndef ELLE_X_ENGINE_H
#define ELLE_X_ENGINE_H

#include "../../Shared/ElleTypes.h"
#include "../../Shared/json.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <mutex>

enum XCyclePhase {
    X_PHASE_MENSTRUAL = 0,
    X_PHASE_FOLLICULAR,
    X_PHASE_OVULATORY,
    X_PHASE_LUTEAL
};

enum XPregnancyPhase {
    X_PREG_NONE = 0,
    X_PREG_FIRST_TRIMESTER,
    X_PREG_SECOND_TRIMESTER,
    X_PREG_THIRD_TRIMESTER,
    X_PREG_LABOR,
    X_PREG_POSTPARTUM
};

enum XLifecycleStage {
    X_LIFE_PREMENARCHE = 0,
    X_LIFE_REPRODUCTIVE,
    X_LIFE_PERIMENOPAUSE,
    X_LIFE_MENOPAUSE
};

enum XContraceptionMethod {
    X_CONTRA_NONE = 0,
    X_CONTRA_PILL,
    X_CONTRA_IUD_HORMONAL,
    X_CONTRA_IUD_COPPER,
    X_CONTRA_IMPLANT,
    X_CONTRA_BARRIER,
    X_CONTRA_NATURAL
};

enum XLaborStage {
    X_LABOR_NONE = 0,
    X_LABOR_LATENT,
    X_LABOR_ACTIVE,
    X_LABOR_TRANSITION,
    X_LABOR_PUSHING,
    X_LABOR_DELIVERED
};

struct XHormoneLevels {

    float estrogen     = 0.0f;
    float progesterone = 0.0f;
    float testosterone = 0.0f;
    float oxytocin     = 0.0f;
    float serotonin    = 0.0f;
    float dopamine     = 0.0f;
    float cortisol     = 0.0f;
    float prolactin    = 0.0f;
    float hcg          = 0.0f;

    float fsh          = 0.0f;
    float lh           = 0.0f;
    float gnrh         = 0.0f;
    float relaxin      = 0.0f;
};

struct XDerivedStats {
    float       bbt_celsius    = 36.5f;
    float       endometrial_mm = 4.0f;
    std::string cervical_mucus;
    std::string menstrual_flow;
    bool        cycle_active   = true;
    bool        anovulatory    = false;
    int         follicle_phase_day = 0;
    int         luteal_phase_day = 0;
};

struct XCycleState {
    uint64_t   anchor_ms           = 0;
    int        cycle_length_days   = 28;
    float      modulation_strength = 0.15f;
    int        cycle_day           = 1;
    XCyclePhase phase              = X_PHASE_MENSTRUAL;
    uint64_t   last_tick_ms        = 0;
    uint64_t   created_ms          = 0;
};

struct XPregnancyState {
    bool             active                  = false;
    uint64_t         conceived_ms            = 0;
    uint64_t         due_ms                  = 0;
    int              gestational_length_days = 280;
    int              gestational_day         = 0;
    int              gestational_week        = 0;
    XPregnancyPhase  phase                   = X_PREG_NONE;
    int64_t          child_id                = 0;
    std::string      last_milestone;
    uint64_t         updated_ms              = 0;

    bool             breastfeeding           = false;
    bool             in_labor                = false;
    XLaborStage      labor_stage             = X_LABOR_NONE;
    uint64_t         labor_started_ms        = 0;
    int              multiplicity            = 1;
    int              pregnancy_count         = 0;

    bool             implanted               = false;
    uint64_t         implantation_ms         = 0;
    std::string      lochia_stage;
    std::string      milk_stage;
    bool             baby_blues              = false;
    bool             fetal_heartbeat_detectable = false;
};

struct XContraceptionState {
    XContraceptionMethod method     = X_CONTRA_NONE;
    uint64_t             started_ms = 0;
    float                efficacy   = 1.0f;
    std::string          notes;
    uint64_t             updated_ms = 0;
};

struct XLifecycleState {
    uint64_t         elle_birth_ms    = 0;
    float            age_years        = 30.0f;
    XLifecycleStage  stage            = X_LIFE_REPRODUCTIVE;
    uint64_t         menarche_ms      = 0;
    uint64_t         perimenopause_ms = 0;
    uint64_t         menopause_ms     = 0;
    uint64_t         updated_ms       = 0;
};

struct XSymptomEntry {
    std::string kind;
    float       intensity;
    std::string origin;
    std::string notes;
    uint64_t    observed_ms;
};

struct XModulation {

    float warmth         = 1.0f;
    float verbal_fluency = 1.0f;
    float empathy        = 1.0f;
    float introspection  = 1.0f;
    float arousal        = 1.0f;
    float fatigue        = 1.0f;
};

struct XStimulus {

    std::string  kind;
    float        intensity = 0.5f;
    std::string  notes;
};

struct XHistoryPoint {
    uint64_t         taken_ms         = 0;
    int              cycle_day        = 0;
    std::string      phase;
    XHormoneLevels   hormones;
    int              pregnancy_day    = 0;
    std::string      pregnancy_phase;
};

struct XConceptionAttemptResult {
    bool        success = false;
    std::string reason;
    bool        in_fertile_window    = false;
    bool        had_recent_intimacy  = false;
    bool        readiness_verified   = false;
    uint64_t    conceived_ms         = 0;
    uint64_t    due_ms               = 0;
};

class XEngine {
public:

    bool Initialize();

    void Tick();

    bool AnchorCycle(int day_of_cycle, int length_days, float modulation_strength);

    void ApplyStimulus(const XStimulus& stim);

    XCycleState      GetCycle() const         { std::lock_guard<std::recursive_mutex> lk(m_mutex); return m_cycle; }
    XHormoneLevels   GetHormones() const      { std::lock_guard<std::recursive_mutex> lk(m_mutex); return m_hormones; }
    XPregnancyState  GetPregnancy() const     { std::lock_guard<std::recursive_mutex> lk(m_mutex); return m_pregnancy; }
    XModulation      ComputeModulation() const;

    std::vector<XHistoryPoint> GetHistory(uint32_t hours, uint32_t max_points);

    XConceptionAttemptResult AttemptConception(bool require_readiness,
                                               bool readiness_verified);

    struct DeliveryOutcome {
        bool      delivered            = false;
        uint64_t  born_ms              = 0;
        uint64_t  gestational_days     = 0;
        int       multiplicity         = 1;
    };
    DeliveryOutcome Deliver();

    bool SetContraception(XContraceptionMethod m, float efficacy,
                          const std::string& notes);
    XContraceptionState GetContraception() const { std::lock_guard<std::recursive_mutex> lk(m_mutex); return m_contra; }

    XDerivedStats GetDerived() const;

    bool SetElleBirthday(uint64_t birth_ms);
    XLifecycleState GetLifecycle() const { std::lock_guard<std::recursive_mutex> lk(m_mutex); return m_life; }

    std::vector<XSymptomEntry> ComputeCurrentSymptoms() const;

    bool LogManualSymptom(const std::string& kind, float intensity,
                          const std::string& notes);

    std::vector<XSymptomEntry> GetRecentSymptoms(uint32_t hours,
                                                 const std::string& origin_filter);

    struct PregnancyEvent {
        uint64_t    occurred_ms;
        uint64_t    conceived_ms;
        int         gestational_day;
        std::string kind;
        std::string detail;
    };
    std::vector<PregnancyEvent> GetPregnancyEvents(uint32_t limit);

    bool AcceleratePregnancy(float factor);

    nlohmann::json GetStateJson() const;

    static const char* CyclePhaseName(XCyclePhase p);
    static const char* PregnancyPhaseName(XPregnancyPhase p);
    static const char* LifecycleStageName(XLifecycleStage s);
    static const char* ContraceptionName(XContraceptionMethod m);
    static const char* LaborStageName(XLaborStage s);
    static XContraceptionMethod ParseContraception(const std::string& s);

private:

    mutable std::recursive_mutex  m_mutex;

    XCycleState          m_cycle;
    XHormoneLevels       m_hormones;
    XPregnancyState      m_pregnancy;
    XContraceptionState  m_contra;
    XLifecycleState      m_life;

    XHormoneLevels   m_residual;
    uint64_t         m_residual_ms = 0;

    int              m_last_cycle_day_seen = 0;
    int              m_last_sampled_gd     = -1;
    bool             m_lh_surge_fired_this_cycle = false;
    bool             m_current_cycle_anovulatory = false;

    void     RecomputeCycleDayAndPhase();
    void     RecomputeBaselineHormones();
    void     DecayResidual(uint64_t nowMs);
    void     ApplyResidualOnto(XHormoneLevels& out) const;
    void     AdvancePregnancy(uint64_t nowMs);
    void     AdvanceLabor(uint64_t nowMs);
    void     RecomputeLifecycle(uint64_t nowMs);
    float    ConceptionProbability() const;
    float    MiscarriageProbability() const;
    bool     CycleShouldRun() const;
    bool     DetectLHSurge() const;
    void     ApplyContraceptionHormones(XHormoneLevels& h) const;
    void     SynthesiseAndLogSymptoms();
    void     AdvanceLochiaAndMilkStages();
    void     DetectImplantation(uint64_t nowMs);
    void     DetectAnovulatoryCycle();
    void     WriteSnapshotRow();
    void     PersistCycleRow();
    void     PersistPregnancyRow();
    void     ArchivePregnancyRow(const std::string& outcome,
                                 uint64_t ended_ms,
                                 uint64_t gestational_days,
                                 int multiplicity);
    void     PersistContraceptionRow();
    void     PersistLifecycleRow();
    void     LogPregnancyEvent(const std::string& kind,
                               const std::string& detail);
    void     LogStimulusRow(const XStimulus& stim);
    void     LogModulationRow(const XModulation& mod) const;
    void     LogSymptomRow(const std::string& kind, float intensity,
                           const std::string& origin,
                           const std::string& notes) const;

    bool     HadRecentIntimacy(uint64_t now_ms, uint64_t window_ms) const;
};

#endif
