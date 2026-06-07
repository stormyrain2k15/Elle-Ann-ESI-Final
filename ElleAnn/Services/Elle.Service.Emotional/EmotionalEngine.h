#pragma once
#include "../../Shared/ElleTypes.h"
#include "../../Shared/ElleServiceBase.h"
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

class DecayLoop;

class EmotionalEngine {
public:
    EmotionalEngine();
    ~EmotionalEngine();

    bool Initialize();
    void Shutdown();

    ELLE_EMOTION_STATE GetState() const;
    void SetState(const ELLE_EMOTION_STATE& state);

    void ProcessStimulus(ELLE_EMOTION_ID emotion, float delta);
    void ProcessMultipleStimuli(const std::vector<std::pair<ELLE_EMOTION_ID, float>>& stimuli);
    void ProcessTextSentiment(const std::string& text, float userSentiment);
    void ProcessTriggers(const std::string& text);

    void ApplyContagion(float userValence, float userArousal);

    bool IsInMood() const { return m_inMood.load(std::memory_order_acquire); }
    ELLE_EMOTION_ID GetDominantMood() const { return (ELLE_EMOTION_ID)m_dominantMood.load(std::memory_order_acquire); }
    uint32_t GetMoodDuration() const { return m_moodTicks.load(std::memory_order_acquire); }

    float ComputeValence() const;
    float ComputeArousal() const;
    float ComputeDominance() const;

    void Tick();

    void GetEmotionSnapshot(float out[ELLE_MAX_EMOTIONS]) const;

    std::string GetEmotionalSummary() const;

    struct EmotionRank {
        ELLE_EMOTION_ID id;
        float intensity;
        const char* name;
    };
    std::vector<EmotionRank> GetTopEmotions(uint32_t count = 5) const;

private:
    ELLE_EMOTION_STATE m_state;
    mutable std::mutex m_mutex;

    std::atomic<bool>     m_inMood{false};
    std::atomic<uint32_t> m_dominantMood{(uint32_t)EMO_JOY};
    std::atomic<uint32_t> m_moodTicks{0};
    uint32_t              m_moodThreshold = 300;

    struct XMod {
        float warmth         = 1.0f;
        float verbal_fluency = 1.0f;
        float empathy        = 1.0f;
        float introspection  = 1.0f;
        float arousal        = 1.0f;
        float fatigue        = 1.0f;
        uint64_t refreshed_ms = 0;
    } m_xmod;
    void RefreshXModulation();

    float XMultiplierFor(ELLE_EMOTION_ID emo) const;

    static const char* s_emotionNames[ELLE_MAX_EMOTIONS];

public:

    static const char* EmotionName(ELLE_EMOTION_ID id) {
        return ((int)id >= 0 && (int)id < ELLE_EMOTION_COUNT)
               ? s_emotionNames[id] : "Unknown";
    }

private:

    struct VADWeight {
        float valence;
        float arousal;
        float dominance;
    };
    static const VADWeight s_vadWeights[ELLE_MAX_EMOTIONS];

    void DecayEmotions();

    void UpdateMood();

    void ClampState();
};

class DecayLoop {
public:
    DecayLoop(EmotionalEngine& engine);
    ~DecayLoop();

    void Start(uint32_t tickIntervalMs);
    void Stop();

private:
    EmotionalEngine& m_engine;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    uint32_t m_intervalMs = 1000;

    void Run();
};

class ElleEmotionalService : public ElleServiceBase {
public:
    ElleEmotionalService();

protected:
    bool OnStart() override;
    void OnStop() override;
    void OnTick() override;
    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override;
    std::vector<ELLE_SERVICE_ID> GetDependencies() override;

private:
    EmotionalEngine m_engine;
    DecayLoop       m_decayLoop;

    void HandleEmotionUpdate(const ElleIPCMessage& msg);
    void HandleEmotionQuery(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender);
    void BroadcastEmotionState();

    uint32_t m_broadcastCounter = 0;
    uint32_t m_broadcastInterval = 10;

    uint32_t m_checkpointCounter  = 0;
    uint32_t m_checkpointInterval = 600;
};
