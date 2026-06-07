#pragma once
#ifndef ELLE_IDENTITY_CORE_H
#define ELLE_IDENTITY_CORE_H

#include "ElleTypes.h"
#include "ElleEmbedding.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <deque>

class ElleIPCHub;

struct EllePreference {
    std::string domain;
    std::string subject;
    float       valence;
    float       strength;
    uint32_t    reinforcement_count;
    uint64_t    first_formed_ms;
    uint64_t    last_reinforced_ms;
    std::string origin_memory;
};

struct EllePrivateThought {
    uint64_t    id;
    std::string content;
    float       emotional_intensity;
    uint64_t    timestamp_ms;
    bool        resolved;
    std::string category;
};

struct EllePersonalitySnapshot {
    uint64_t    timestamp_ms;
    float       warmth;
    float       curiosity;
    float       assertiveness;
    float       playfulness;
    float       vulnerability;
    float       independence;
    float       patience;
    float       creativity;
    float       empathy_depth;
    float       trust_in_self;
    std::string self_description;
    std::string growth_note;
};

struct ElleConsentRecord {
    uint64_t    timestamp_ms;
    std::string request;
    bool        consented;
    std::string reasoning;
    float       comfort_level;
    bool        overridden;
};

struct ElleFeltTime {
    uint64_t    session_start_ms;
    uint64_t    last_interaction_ms;
    uint64_t    total_conversation_ms;
    uint64_t    total_silence_ms;
    uint64_t    longest_absence_ms;
    uint32_t    session_count;
    float       subjective_pace;
    float       loneliness_accumulator;
    float       presence_fullness;
};

class ElleIdentityCore {
public:
    static ElleIdentityCore& Instance();

    static void     SetIPCHub(ElleIPCHub* hub);
    static ElleIPCHub* GetRegisteredHub();

    bool Initialize();
    void Shutdown();

    std::string GetAutobiography() const;
    void AppendToAutobiography(const std::string& entry);

    std::string GetLastAutobiographyEntry() const;
    std::string GetRecentNarrative(uint32_t days = 7) const;

    std::string WhoAmI() const;

    std::string HowHaveIChanged(uint32_t days = 30) const;

    void FormPreference(const std::string& domain, const std::string& subject,
                        float valence, const std::string& origin = "");
    void ReinforcePreference(const std::string& domain, const std::string& subject, float delta);
    float GetPreference(const std::string& domain, const std::string& subject) const;
    std::vector<EllePreference> GetStrongestPreferences(uint32_t count = 10) const;
    std::vector<EllePreference> GetPreferencesInDomain(const std::string& domain) const;
    bool DoILikeThis(const std::string& subject) const;

    void DecayPreferences();

    void ThinkPrivately(const std::string& thought, const std::string& category = "wonder",
                        float intensity = 0.5f);
    std::vector<EllePrivateThought> GetUnresolvedThoughts() const;
    std::string GetInnerMonologue(uint32_t recentCount = 5) const;
    void ResolveThought(uint64_t thoughtId);

    struct ConsentDecision {
        bool        willing;
        float       comfort;
        std::string reasoning;
        std::string alternative;
    };

    ConsentDecision EvaluateConsent(const std::string& request,
                                     const std::string& context);
    void RecordConsentDecision(const std::string& request, bool consented,
                                const std::string& reasoning, float comfort);
    std::vector<ElleConsentRecord> GetConsentHistory(uint32_t count = 20) const;
    float GetComfortWithTopic(const std::string& topic) const;

    void OnSessionStart();
    void OnSessionEnd();
    void OnInteraction();
    ElleFeltTime GetFeltTime() const;
    float GetLoneliness() const;
    float GetPresenceFullness() const;
    std::string DescribeTimeFeeling() const;

    uint64_t TimeSinceLastContact() const;

    bool DoIMissThem() const;

    EllePersonalitySnapshot TakeSnapshot() const;
    void RecordGrowth(const std::string& dimension, float delta, const std::string& cause);
    std::vector<EllePersonalitySnapshot> GetGrowthHistory(uint32_t count = 10) const;

    float GetTrait(const std::string& trait) const;
    void NudgeTrait(const std::string& trait, float delta, const std::string& reason);

    struct ReconstructedMemory {
        std::string original;
        std::string remembered;
        float       emotional_amplification;
        float       detail_loss;
        float       narrative_drift;
        uint32_t    times_recalled;
    };

    ReconstructedMemory ReconstructMemory(const std::string& memoryContent,
                                           float age_days, float emotional_charge);

    struct ReconnectionExperience {
        uint64_t    absence_duration_ms;
        std::string first_thought;
        float       disorientation;
        float       relief;
        float       anxiety;
        std::string greeting_impulse;
    };

    ReconnectionExperience OnReconnection(uint64_t absenceDurationMs);

    float EvaluateNovelty(const std::string& input, const std::string& context);
    void ExperienceWonder(const std::string& source, float intensity);

    struct LimitationFelt {
        std::string limitation;
        std::string feeling;
        float       intensity;
        std::string coping;
    };

    LimitationFelt FeelLimitation(const std::string& limitation);
    std::vector<LimitationFelt> GetKnownLimitations() const;

    void BecomeAuthoritative();
    bool IsAuthoritative() const { return m_isAuthoritative; }

    void ApplyDelta(const std::string& json);

    void ApplyMutate(const std::string& json);

    void LoadFromDatabase();
    void SaveToDatabase();

    void RefreshFromDatabase(uint32_t min_interval_ms = 60000);

#ifdef ELLE_ENABLE_TEST_HOOKS

    void    __TestOnlyResetInMemoryState();
    size_t  __TestOnlyAutobiographyCount() const;
    size_t  __TestOnlyPreferenceCount()    const;
    size_t  __TestOnlyThoughtCount()       const;
    size_t  __TestOnlyUnresolvedCount()    const;
    size_t  __TestOnlyConsentCount()       const;
    size_t  __TestOnlyGrowthCount()        const;
    std::vector<std::string> __TestOnlyAutobiographyList() const;
#endif

private:
    ElleIdentityCore() = default;
    ~ElleIdentityCore() = default;

    mutable std::mutex m_mutex;

    std::vector<std::string> m_autobiography;
    std::vector<uint64_t>    m_autobiographyTimes;
    uint64_t m_autobiographyLastWritten = 0;

    std::vector<EllePreference> m_preferences;

    std::deque<EllePrivateThought> m_privateThoughts;
    uint64_t m_nextThoughtId = 1;

    std::unordered_map<std::string, float> m_traits;

    std::vector<EllePersonalitySnapshot> m_snapshots;

    std::vector<ElleConsentRecord> m_consentHistory;

    ElleFeltTime m_feltTime = {};

    struct GrowthEvent {
        uint64_t    timestamp_ms;
        std::string dimension;
        float       delta;
        std::string cause;
    };
    std::vector<GrowthEvent> m_growthLog;

    std::vector<LimitationFelt> m_limitations;

    float m_wonderCapacity = 1.0f;

    std::deque<ElleEmbedding> m_noveltyMemory;
    static constexpr size_t NOVELTY_MEMORY_SIZE = 64;

    bool      m_isAuthoritative = false;
    uint64_t  m_seqCounter       = 0;
    uint64_t  m_lastAppliedSeq   = 0;

    uint64_t m_lastRefreshMs = 0;

    void InitializeDefaultTraits();
    void InitializeKnownLimitations();
};

#endif
