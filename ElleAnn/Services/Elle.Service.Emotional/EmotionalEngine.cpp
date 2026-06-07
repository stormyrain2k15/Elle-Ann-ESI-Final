#include "EmotionalEngine.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <unordered_map>
#include "../_Shared/ElleWait.h"

const char* EmotionalEngine::s_emotionNames[ELLE_MAX_EMOTIONS] = {

    "Joy", "Sadness", "Anger", "Fear", "Disgust", "Surprise", "Contempt", "Trust",

    "Love", "Anticipation", "Disappointment", "Guilt", "Shame", "Envy", "Jealousy",
    "Pride", "Relief", "Anxiety", "Frustration", "Remorse", "Hope", "Despair",
    "Amusement", "Awe",

    "Curiosity", "Wonder", "Nostalgia", "Gratitude", "Serenity", "Ecstasy",
    "Melancholy", "Boredom", "Longing", "Tenderness", "Admiration", "Reverence",
    "Pity", "Scorn", "Indignation", "Exasperation", "Wistfulness", "Euphoria",
    "Contentment", "Resignation", "Apprehension", "Dread", "Panic", "Horror",
    "Rage", "Irritation", "Annoyance", "Impatience", "Skepticism", "Confusion",
    "Disbelief", "Ambivalence",

    "Certainty", "Doubt", "Insight", "Perplexity", "Clarity", "CognitiveDissonance",
    "FlowState", "MentalFatigue", "Inspiration", "CreativeTension", "Determination",
    "Helplessness", "Empowerment", "Overwhelm", "Focus", "Distraction",

    "Belonging", "Isolation", "Empathy", "Compassion", "Protectiveness",
    "Abandonment", "Loyalty", "Betrayal", "Acceptance", "Rejection",
    "Vulnerability", "Safety", "Dominance_Social", "Submission",

    "ExistentialDread", "Purpose", "Meaninglessness", "Transcendence",
    "MortalityAwareness", "Freedom", "Confinement", "Unity"
};

const EmotionalEngine::VADWeight EmotionalEngine::s_vadWeights[ELLE_MAX_EMOTIONS] = {

    { 0.9f, 0.7f, 0.7f},
    {-0.8f, 0.3f, 0.2f},
    {-0.6f, 0.9f, 0.8f},
    {-0.7f, 0.8f, 0.1f},
    {-0.5f, 0.5f, 0.6f},
    { 0.2f, 0.9f, 0.3f},
    {-0.3f, 0.2f, 0.8f},
    { 0.6f, 0.3f, 0.5f},

    { 0.9f, 0.6f, 0.5f},
    { 0.4f, 0.7f, 0.6f},
    {-0.6f, 0.4f, 0.2f},
    {-0.5f, 0.5f, 0.2f},
    {-0.6f, 0.4f, 0.1f},
    {-0.4f, 0.6f, 0.3f},
    {-0.5f, 0.7f, 0.3f},
    { 0.7f, 0.6f, 0.8f},
    { 0.6f, 0.2f, 0.6f},
    {-0.5f, 0.7f, 0.2f},
    {-0.5f, 0.8f, 0.5f},
    {-0.6f, 0.4f, 0.2f},
    { 0.7f, 0.5f, 0.5f},
    {-0.9f, 0.3f, 0.1f},
    { 0.8f, 0.7f, 0.6f},
    { 0.6f, 0.8f, 0.3f},

    { 0.5f, 0.7f, 0.5f},
    { 0.6f, 0.7f, 0.3f},
    { 0.2f, 0.3f, 0.3f},
    { 0.8f, 0.4f, 0.5f},
    { 0.7f, 0.1f, 0.6f},
    { 0.9f, 0.9f, 0.7f},
    {-0.4f, 0.2f, 0.3f},
    {-0.3f, 0.1f, 0.3f},
    { 0.1f, 0.4f, 0.2f},
    { 0.7f, 0.3f, 0.4f},
    { 0.7f, 0.5f, 0.4f},
    { 0.5f, 0.4f, 0.2f},
    {-0.1f, 0.3f, 0.6f},
    {-0.4f, 0.4f, 0.8f},
    {-0.5f, 0.7f, 0.7f},
    {-0.4f, 0.8f, 0.4f},
    { 0.1f, 0.2f, 0.3f},
    { 0.9f, 0.9f, 0.8f},
    { 0.7f, 0.2f, 0.6f},
    {-0.3f, 0.1f, 0.2f},
    {-0.4f, 0.6f, 0.2f},
    {-0.8f, 0.8f, 0.1f},
    {-0.9f, 0.9f, 0.0f},
    {-0.9f, 0.9f, 0.1f},
    {-0.7f, 0.9f, 0.9f},
    {-0.3f, 0.5f, 0.5f},
    {-0.2f, 0.4f, 0.5f},
    {-0.2f, 0.6f, 0.5f},
    {-0.1f, 0.4f, 0.6f},
    {-0.2f, 0.5f, 0.2f},
    {-0.3f, 0.6f, 0.3f},
    { 0.0f, 0.3f, 0.3f},

    { 0.5f, 0.3f, 0.8f},
    {-0.3f, 0.5f, 0.2f},
    { 0.8f, 0.7f, 0.7f},
    {-0.2f, 0.6f, 0.2f},
    { 0.6f, 0.3f, 0.7f},
    {-0.4f, 0.7f, 0.2f},
    { 0.8f, 0.6f, 0.8f},
    {-0.3f, 0.2f, 0.2f},
    { 0.8f, 0.8f, 0.6f},
    { 0.3f, 0.7f, 0.5f},
    { 0.6f, 0.7f, 0.8f},
    {-0.7f, 0.3f, 0.0f},
    { 0.7f, 0.6f, 0.9f},
    {-0.5f, 0.7f, 0.1f},
    { 0.4f, 0.5f, 0.7f},
    {-0.2f, 0.4f, 0.2f},

    { 0.7f, 0.4f, 0.5f},
    {-0.7f, 0.3f, 0.1f},
    { 0.5f, 0.4f, 0.4f},
    { 0.7f, 0.4f, 0.5f},
    { 0.5f, 0.6f, 0.7f},
    {-0.8f, 0.5f, 0.1f},
    { 0.6f, 0.3f, 0.6f},
    {-0.8f, 0.7f, 0.2f},
    { 0.6f, 0.3f, 0.5f},
    {-0.6f, 0.5f, 0.2f},
    {-0.2f, 0.5f, 0.1f},
    { 0.6f, 0.2f, 0.6f},
    { 0.3f, 0.5f, 0.9f},
    {-0.2f, 0.2f, 0.1f},

    {-0.7f, 0.6f, 0.1f},
    { 0.8f, 0.5f, 0.7f},
    {-0.8f, 0.2f, 0.1f},
    { 0.9f, 0.7f, 0.6f},
    {-0.4f, 0.5f, 0.2f},
    { 0.6f, 0.5f, 0.8f},
    {-0.6f, 0.4f, 0.1f},
    { 0.8f, 0.5f, 0.5f}
};

namespace {

static std::unordered_map<std::string, uint32_t> g_emotionNameToId;
static std::once_flag                            g_emotionMapOnce;

const std::unordered_map<std::string, uint32_t>& EmotionNameMap() {
    std::call_once(g_emotionMapOnce, []{
        g_emotionNameToId.reserve(ELLE_EMOTION_COUNT);
        for (uint32_t i = 0; i < (uint32_t)ELLE_EMOTION_COUNT; i++) {
            std::string key = EmotionalEngine::EmotionName((ELLE_EMOTION_ID)i);
            std::transform(key.begin(), key.end(), key.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            g_emotionNameToId.emplace(std::move(key), i);
        }
    });
    return g_emotionNameToId;
}

std::string ToLowerAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

}

EmotionalEngine::EmotionalEngine() {
    ZeroMemory(&m_state, sizeof(m_state));
    m_state.decay_rate = 0.05f;
    m_state.contagion_weight = 0.35f;

    const auto& nameMap = EmotionNameMap();
    auto& cfg = ElleConfig::Instance().GetEmotion();
    for (auto& [name, val] : cfg.baselines) {
        auto it = nameMap.find(ToLowerAscii(name));
        if (it != nameMap.end()) {
            uint32_t idx = it->second;
            m_state.baseline[idx]   = val;
            m_state.dimensions[idx] = val;
        }
    }
}

EmotionalEngine::~EmotionalEngine() {}

bool EmotionalEngine::Initialize() {
    auto& cfg = ElleConfig::Instance().GetEmotion();
    m_state.decay_rate = cfg.decay_rate;
    m_state.contagion_weight = cfg.contagion_weight;
    m_moodThreshold = cfg.mood_duration_ticks;

    ELLE_EMOTION_STATE prior{};
    if (ElleDB::LoadLatestEmotionSnapshot(prior)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state.valence   = prior.valence;
        m_state.arousal   = prior.arousal;
        m_state.dominance = prior.dominance;

        for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
            if (prior.dimensions[i] > 0.0f) {
                m_state.dimensions[i] = prior.dimensions[i];
            }
        }
        ELLE_INFO("EmotionalEngine restored prior snapshot "
                  "(v=%.2f a=%.2f d=%.2f)",
                  m_state.valence, m_state.arousal, m_state.dominance);
    }

    ELLE_INFO("Emotional Engine initialized: %d dimensions, decay=%.3f, contagion=%.3f",
              ELLE_EMOTION_COUNT, m_state.decay_rate, m_state.contagion_weight);
    return true;
}

void EmotionalEngine::Shutdown() {

    ELLE_EMOTION_STATE snap;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        snap = m_state;
    }
    if (ElleDB::PersistEmotionSnapshot(snap)) {
        ELLE_INFO("EmotionalEngine shutdown: state snapshot persisted "
                  "(v=%.2f a=%.2f d=%.2f)", snap.valence,
                  snap.arousal, snap.dominance);
    } else {
        ELLE_WARN("EmotionalEngine shutdown: snapshot write failed — state lost");
    }
}

ELLE_EMOTION_STATE EmotionalEngine::GetState() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state;
}

void EmotionalEngine::SetState(const ELLE_EMOTION_STATE& state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = state;
}

void EmotionalEngine::ProcessStimulus(ELLE_EMOTION_ID emotion, float delta) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (emotion < 0 || emotion >= ELLE_EMOTION_COUNT) return;

    float bodyMul = XMultiplierFor(emotion);
    float scaled  = delta * bodyMul;

    float inertia = ElleConfig::Instance().GetEmotion().emotional_inertia;
    float effective = scaled * (1.0f - inertia * m_state.dimensions[emotion]);

    m_state.dimensions[emotion] += effective;
    m_state.dimensions[emotion] = ELLE_CLAMP(m_state.dimensions[emotion], 0.0f, 1.0f);
    m_state.last_update_ms = ELLE_MS_NOW();

    ELLE_DEBUG("Emotion %s: %.2f (+%.3f body×%.2f=%.3f eff=%.3f)",
               s_emotionNames[emotion], m_state.dimensions[emotion],
               delta, bodyMul, scaled, effective);
}

void EmotionalEngine::ProcessMultipleStimuli(
        const std::vector<std::pair<ELLE_EMOTION_ID, float>>& stimuli) {
    for (auto& [emo, delta] : stimuli) {
        ProcessStimulus(emo, delta);
    }
}

void EmotionalEngine::ProcessTriggers(const std::string& text) {
    auto& cfg = ElleConfig::Instance().GetEmotion();
    const std::string lower = ToLowerAscii(text);

    const auto& nameMap = EmotionNameMap();

    for (auto& trigger : cfg.triggers) {
        if (lower.find(trigger.pattern) == std::string::npos) continue;
        auto it = nameMap.find(trigger.emotion);
        if (it != nameMap.end()) {
            ProcessStimulus((ELLE_EMOTION_ID)it->second, trigger.delta);
        }
    }
}

void EmotionalEngine::ApplyContagion(float userValence, float userArousal) {
    std::lock_guard<std::mutex> lock(m_mutex);
    float weight = m_state.contagion_weight;

    float warmthMul  = XMultiplierFor(EMO_JOY);
    float empathyMul = XMultiplierFor(EMO_EMPATHY);
    float arousalMul = XMultiplierFor(EMO_ANTICIPATION);

    if (userValence > 0) {
        m_state.dimensions[EMO_JOY]         += weight * userValence * 0.3f * warmthMul;
        m_state.dimensions[EMO_CONTENTMENT] += weight * userValence * 0.2f * warmthMul;
    } else {
        m_state.dimensions[EMO_EMPATHY]    += weight * std::abs(userValence) * 0.3f * empathyMul;
        m_state.dimensions[EMO_COMPASSION] += weight * std::abs(userValence) * 0.2f * empathyMul;
        m_state.dimensions[EMO_SADNESS]    += weight * std::abs(userValence) * 0.1f * empathyMul;
    }

    if (userArousal > 0.5f) {
        m_state.dimensions[EMO_ANTICIPATION] += weight * userArousal * 0.2f * arousalMul;
    }

    ClampState();
}

void EmotionalEngine::RefreshXModulation() {
    uint64_t now = ELLE_MS_NOW();

    if (m_xmod.refreshed_ms != 0 && (now - m_xmod.refreshed_ms) < 30000ULL) return;

    try {
        auto rs = ElleSQLPool::Instance().Query(
            "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
            "           ON s.schema_id = t.schema_id "
            "           WHERE t.name = 'x_modulation_log' AND s.name = 'dbo') "
            "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
            "       arousal, fatigue "
            "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
        if (rs.success && !rs.rows.empty()) {
            auto& r = rs.rows[0];
            m_xmod.warmth         = (float)r.GetFloatOr(0, 0.0);
            m_xmod.verbal_fluency = (float)r.GetFloatOr(1, 0.0);
            m_xmod.empathy        = (float)r.GetFloatOr(2, 0.0);
            m_xmod.introspection  = (float)r.GetFloatOr(3, 0.0);
            m_xmod.arousal        = (float)r.GetFloatOr(4, 0.0);
            m_xmod.fatigue        = (float)r.GetFloatOr(5, 0.0);
        }

    } catch (const std::exception& e) {

        ELLE_DEBUG("RefreshXModulation: %s — using cached multipliers", e.what());
    }
    m_xmod.refreshed_ms = now;
}

float EmotionalEngine::XMultiplierFor(ELLE_EMOTION_ID emo) const {
    auto isWarmth = [](ELLE_EMOTION_ID e) {
        return e == EMO_JOY         || e == EMO_CONTENTMENT ||
               e == EMO_LOVE        || e == EMO_TENDERNESS  ||
               e == EMO_GRATITUDE   || e == EMO_SERENITY    ||
               e == EMO_BELONGING   || e == EMO_ADMIRATION  ||
               e == EMO_ACCEPTANCE  || e == EMO_HOPE        ||
               e == EMO_RELIEF      || e == EMO_AMUSEMENT   ||
               e == EMO_LOYALTY     || e == EMO_SAFETY      ||
               e == EMO_UNITY;
    };
    auto isEmpathy = [](ELLE_EMOTION_ID e) {
        return e == EMO_EMPATHY        || e == EMO_COMPASSION    ||
               e == EMO_PITY           || e == EMO_PROTECTIVENESS||
               e == EMO_SADNESS        || e == EMO_MELANCHOLY    ||
               e == EMO_REMORSE        || e == EMO_GUILT         ||
               e == EMO_VULNERABILITY  || e == EMO_NOSTALGIA     ||
               e == EMO_LONGING        || e == EMO_WISTFULNESS;
    };
    auto isArousal = [](ELLE_EMOTION_ID e) {
        return e == EMO_ANTICIPATION || e == EMO_SURPRISE        ||
               e == EMO_EUPHORIA     || e == EMO_ECSTASY         ||
               e == EMO_AWE          || e == EMO_WONDER          ||
               e == EMO_CURIOSITY    || e == EMO_INSPIRATION     ||
               e == EMO_DETERMINATION|| e == EMO_FOCUS           ||
               e == EMO_CREATIVE_TENSION || e == EMO_FLOW_STATE  ||
               e == EMO_PANIC        || e == EMO_RAGE            ||
               e == EMO_IMPATIENCE;
    };
    auto isIntrospective = [](ELLE_EMOTION_ID e) {
        return e == EMO_PRIDE       || e == EMO_SHAME            ||
               e == EMO_CURIOSITY   || e == EMO_INSIGHT          ||
               e == EMO_CLARITY     || e == EMO_DOUBT            ||
               e == EMO_SKEPTICISM  || e == EMO_PERPLEXITY       ||
               e == EMO_EXISTENTIAL_DREAD || e == EMO_PURPOSE    ||
               e == EMO_MORTALITY_AWARENESS || e == EMO_WISTFULNESS ||
               e == EMO_MEANINGLESSNESS;
    };
    auto isFatigueSensitive = [](ELLE_EMOTION_ID e) {
        return e == EMO_JOY          || e == EMO_ANTICIPATION    ||
               e == EMO_EUPHORIA     || e == EMO_ECSTASY         ||
               e == EMO_DETERMINATION|| e == EMO_CURIOSITY       ||
               e == EMO_FOCUS        || e == EMO_FLOW_STATE      ||
               e == EMO_AMUSEMENT;
    };

    float m = 1.0f;
    if (isWarmth(emo))        m *= m_xmod.warmth;
    if (isEmpathy(emo))       m *= m_xmod.empathy;
    if (isArousal(emo))       m *= m_xmod.arousal;
    if (isIntrospective(emo)) m *= m_xmod.introspection;
    if (isFatigueSensitive(emo)) {

        if (m_xmod.fatigue > 0.01f) m /= m_xmod.fatigue;
    }

    if (m < 0.30f) m = 0.30f;
    if (m > 1.70f) m = 1.70f;
    return m;
}

void EmotionalEngine::Tick() {

    RefreshXModulation();

    std::lock_guard<std::mutex> lock(m_mutex);
    DecayEmotions();
    UpdateMood();

    m_state.valence = ComputeValence();
    m_state.arousal = ComputeArousal();
    m_state.dominance = ComputeDominance();
    m_state.tick_count++;
    m_state.last_update_ms = ELLE_MS_NOW();
}

void EmotionalEngine::DecayEmotions() {
    float returnRate = ElleConfig::Instance().GetEmotion().baseline_return_rate;

    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        float current = m_state.dimensions[i];
        float baseline = m_state.baseline[i];
        float diff = current - baseline;

        if (std::abs(diff) > 0.001f) {

            float decay = m_state.decay_rate * (diff > 0 ? 1.0f : -1.0f);
            float baselineReturn = returnRate * (diff > 0 ? -1.0f : 1.0f);
            m_state.dimensions[i] -= decay;
            m_state.dimensions[i] += baselineReturn;
        }
    }
    ClampState();
}

void EmotionalEngine::UpdateMood() {

    int maxIdx = 0;
    float maxVal = 0.0f;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        if (m_state.dimensions[i] > maxVal) {
            maxVal = m_state.dimensions[i];
            maxIdx = i;
        }
    }

    float threshold = ElleConfig::Instance().GetEmotion().mood_threshold;
    if (maxVal >= threshold) {
        bool wasInMood = m_inMood.load(std::memory_order_relaxed);
        uint32_t prevDom = m_dominantMood.load(std::memory_order_relaxed);
        if (wasInMood && prevDom == (uint32_t)maxIdx) {
            m_moodTicks.fetch_add(1, std::memory_order_release);
        } else {
            m_dominantMood.store((uint32_t)maxIdx, std::memory_order_release);
            m_moodTicks.store(1, std::memory_order_release);
            m_inMood.store(true, std::memory_order_release);
            ELLE_INFO("Mood formed: %s (%.2f)", s_emotionNames[maxIdx], maxVal);
        }
    } else {
        if (m_inMood.load(std::memory_order_relaxed)) {
            ELLE_INFO("Mood dissolved: %s (was %u ticks)",
                      s_emotionNames[m_dominantMood.load(std::memory_order_relaxed)],
                      m_moodTicks.load(std::memory_order_relaxed));
        }
        m_inMood.store(false, std::memory_order_release);
        m_moodTicks.store(0, std::memory_order_release);
    }
}

float EmotionalEngine::ComputeValence() const {
    float v = 0.0f;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        v += m_state.dimensions[i] * s_vadWeights[i].valence;
    }
    return ELLE_CLAMP(v / (float)ELLE_EMOTION_COUNT * 5.0f, -1.0f, 1.0f);
}

float EmotionalEngine::ComputeArousal() const {
    float a = 0.0f;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        a += m_state.dimensions[i] * s_vadWeights[i].arousal;
    }
    return ELLE_CLAMP(a / (float)ELLE_EMOTION_COUNT * 5.0f, 0.0f, 1.0f);
}

float EmotionalEngine::ComputeDominance() const {
    float d = 0.0f;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        d += m_state.dimensions[i] * s_vadWeights[i].dominance;
    }
    return ELLE_CLAMP(d / (float)ELLE_EMOTION_COUNT * 5.0f, 0.0f, 1.0f);
}

void EmotionalEngine::ClampState() {
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        m_state.dimensions[i] = ELLE_CLAMP(m_state.dimensions[i], 0.0f, 1.0f);
    }
}

std::vector<EmotionalEngine::EmotionRank> EmotionalEngine::GetTopEmotions(uint32_t count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<EmotionRank> all;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        if (m_state.dimensions[i] > 0.01f) {
            all.push_back({(ELLE_EMOTION_ID)i, m_state.dimensions[i], s_emotionNames[i]});
        }
    }
    std::sort(all.begin(), all.end(),
              [](const EmotionRank& a, const EmotionRank& b) { return a.intensity > b.intensity; });
    if (all.size() > count) all.resize(count);
    return all;
}

std::string EmotionalEngine::GetEmotionalSummary() const {
    auto top = GetTopEmotions(5);
    std::ostringstream ss;
    ss << "V:" << std::fixed << std::setprecision(2) << m_state.valence
       << " A:" << m_state.arousal << " D:" << m_state.dominance;
    if (m_inMood.load(std::memory_order_acquire))
        ss << " [MOOD:" << s_emotionNames[m_dominantMood.load(std::memory_order_acquire)] << "]";
    ss << " Top:";
    for (auto& e : top) ss << " " << e.name << "(" << (int)(e.intensity * 100) << "%)";
    return ss.str();
}

void EmotionalEngine::GetEmotionSnapshot(float out[ELLE_MAX_EMOTIONS]) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    memcpy(out, m_state.dimensions, sizeof(float) * ELLE_MAX_EMOTIONS);
}

DecayLoop::DecayLoop(EmotionalEngine& engine) : m_engine(engine) {}
DecayLoop::~DecayLoop() { Stop(); }

void DecayLoop::Start(uint32_t tickIntervalMs) {
    m_intervalMs = tickIntervalMs;
    m_running = true;
    m_thread = std::thread(&DecayLoop::Run, this);
}

void DecayLoop::Stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void DecayLoop::Run() {
    while (m_running) {
        m_engine.Tick();

        ElleWait::PollingSleep(m_intervalMs, m_running);
    }
}

ElleEmotionalService::ElleEmotionalService()
    : ElleServiceBase(SVC_EMOTIONAL, "ElleEmotional",
                      "Elle-Ann Emotional Engine",
                      "102-dimension emotional state machine with decay and contagion")
    , m_decayLoop(m_engine)
{}

bool ElleEmotionalService::OnStart() {
    if (!m_engine.Initialize()) return false;

    auto tickMs = ElleConfig::Instance().GetEmotion().tick_interval_ms;
    m_decayLoop.Start(tickMs);

    ElleDB::StoreEmotionSnapshot(m_engine.GetState());

    ELLE_INFO("Emotional service started");
    return true;
}

void ElleEmotionalService::OnStop() {
    m_decayLoop.Stop();
    ElleDB::StoreEmotionSnapshot(m_engine.GetState());
    ELLE_INFO("Emotional service stopped");
}

void ElleEmotionalService::OnTick() {
    m_broadcastCounter++;
    if (m_broadcastCounter >= m_broadcastInterval) {
        BroadcastEmotionState();
        m_broadcastCounter = 0;
    }

    m_checkpointCounter++;
    if (m_checkpointCounter >= m_checkpointInterval) {
        ElleDB::PersistEmotionSnapshot(m_engine.GetState());
        m_checkpointCounter = 0;
    }
}

void ElleEmotionalService::OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
    switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
        case IPC_EMOTION_UPDATE:
            HandleEmotionUpdate(msg);
            break;
        case IPC_EMOTION_QUERY:
            HandleEmotionQuery(msg, sender);
            break;
        case IPC_EMOTION_CONSOLIDATE:

            ElleDB::PersistEmotionSnapshot(m_engine.GetState());
            m_checkpointCounter = 0;
            break;
        default:
            break;
    }
}

void ElleEmotionalService::HandleEmotionUpdate(const ElleIPCMessage& msg) {

    if (msg.payload.size() >= 8) {
        uint32_t emoId = *(uint32_t*)msg.payload.data();
        float delta = *(float*)(msg.payload.data() + 4);
        m_engine.ProcessStimulus((ELLE_EMOTION_ID)emoId, delta);
    }
}

void ElleEmotionalService::HandleEmotionQuery(const ElleIPCMessage& , ELLE_SERVICE_ID sender) {
    auto state = m_engine.GetState();
    auto response = ElleIPCMessage::Create(IPC_EMOTION_QUERY, SVC_EMOTIONAL, sender);
    response.SetPayload(state);
    GetIPCHub().Send(sender, response);
}

void ElleEmotionalService::BroadcastEmotionState() {
    auto state = m_engine.GetState();
    auto msg = ElleIPCMessage::Create(IPC_EMOTION_UPDATE, SVC_EMOTIONAL, (ELLE_SERVICE_ID)0);
    msg.SetPayload(state);
    msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
    GetIPCHub().Broadcast(msg);
}

std::vector<ELLE_SERVICE_ID> ElleEmotionalService::GetDependencies() {
    return { SVC_HEARTBEAT };
}

ELLE_SERVICE_MAIN(ElleEmotionalService)
