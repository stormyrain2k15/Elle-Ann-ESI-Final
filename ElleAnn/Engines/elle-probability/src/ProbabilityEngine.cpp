// ============================================================================
// Elle Probability Engine -- Engine Facade implementation
// File: src/ProbabilityEngine.cpp
// ============================================================================
#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/AuditTrace.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace elle { namespace prob {

// ---------------------------------------------------------------------------
// ProbabilityEngineConfig
// ---------------------------------------------------------------------------

ProbabilityEngineConfig ProbabilityEngineConfig::defaults() {
    return ProbabilityEngineConfig{};
}

ProbabilityEngineConfig ProbabilityEngineConfig::loadFromString(const std::string& json) {
    ProbabilityEngineConfig cfg = defaults();
    try {
        auto j = nlohmann::json::parse(json);
        if (j.contains("workerThreads"))    cfg.workerThreads    = j["workerThreads"].get<std::size_t>();
        if (j.contains("stochastic"))       cfg.stochastic       = j["stochastic"].get<bool>();
        if (j.contains("trustHalfLifeSecs"))cfg.trustHalfLifeSecs= j["trustHalfLifeSecs"].get<double>();
        if (j.contains("senseHalfLifeSecs"))cfg.senseHalfLifeSecs= j["senseHalfLifeSecs"].get<double>();
        if (j.contains("intentHalfLifeSecs"))cfg.intentHalfLifeSecs=j["intentHalfLifeSecs"].get<double>();
        if (j.contains("emotion")) {
            auto& e = j["emotion"];
            if (e.contains("halfLifeSecs")) cfg.emotionConfig.halfLifeSecs = e["halfLifeSecs"].get<double>();
            if (e.contains("likelihoodGain")) cfg.emotionConfig.likelihoodGain = e["likelihoodGain"].get<double>();
        }
        if (j.contains("defaultTrustPrior")) {
            auto& tp = j["defaultTrustPrior"];
            if (tp.contains("alpha")) cfg.defaultTrustPrior.alpha = tp["alpha"].get<double>();
            if (tp.contains("beta"))  cfg.defaultTrustPrior.beta  = tp["beta"].get<double>();
        }
        if (j.contains("weights")) {
            cfg.weightsJson = j["weights"].dump();
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("ProbabilityEngineConfig parse error: ") + e.what());
    }
    return cfg;
}

ProbabilityEngineConfig ProbabilityEngineConfig::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open config file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return loadFromString(ss.str());
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ProbabilityEngine::ProbabilityEngine(ProbabilityEngineConfig config)
    : m_config(std::move(config))
    , m_store(std::make_shared<BeliefStore>(m_config.workerThreads))
    , m_senseResolver(std::make_unique<SenseProbabilityResolver>(m_store))
    , m_intentAnalyzer(std::make_unique<IntentAnalyzer>(m_store))
    , m_emotionBuilder(std::make_unique<EmotionalPosteriorBuilder>(
          m_store, m_config.emotionConfig))
{
    // Seed weights from config JSON if provided.
    if (!m_config.weightsJson.empty()) {
        try {
            auto j = nlohmann::json::parse(m_config.weightsJson);
            WeightVector w;
            auto readD = [&](const char* key, double& dst) {
                if (j.contains(key)) dst = j[key].get<double>();
            };
            readD("contextFrameMatch",   w.contextFrameMatch);
            readD("nearbyWordCooccur",   w.nearbyWordCooccur);
            readD("senseExampleOverlap", w.senseExampleOverlap);
            readD("emotionalAlignment",  w.emotionalAlignment);
            readD("frequency",           w.frequency);
            readD("posCompatibility",    w.posCompatibility);
            readD("posNegDrawAlignment", w.posNegDrawAlignment);
            readD("conversationHint",    w.conversationHint);
            m_senseResolver->seedWeights(w);
        } catch (...) {
            // Non-fatal: fall through to hardcoded defaults.
        }
    } else {
        // Seed defaults from the language engine's standard weights.
        m_senseResolver->seedWeights(WeightVector{});
    }
}

ProbabilityEngine::~ProbabilityEngine() {
    flush();
}

// ---------------------------------------------------------------------------
// analyze
// ---------------------------------------------------------------------------

ProbabilityResult ProbabilityEngine::analyze(const ProbabilityRequest& req,
                                              const std::string& speakerId)
{
    ProbabilityResult result;

    // 1. Speaker trust.
    {
        auto& model = getSpeakerModel(speakerId);
        result.speakerTrust = model.trustMean();
    }

    // 2. Emotional posterior -- update from the incoming emotional profile.
    result.emotionalPosterior = m_emotionBuilder->update(req.emotionalProfile);

    // 3. Sense resolution.
    m_senseResolver->resolve(req, result);

    // 4. Intent analysis (uses emotional posterior and speaker trust already in result).
    m_intentAnalyzer->analyze(req, result);

    // 5. Audit trace.
    result.traceJson = AuditTrace::toJson(result);

    return result;
}

// ---------------------------------------------------------------------------
// currentWeights
// ---------------------------------------------------------------------------

WeightVector ProbabilityEngine::currentWeights() const {
    return m_senseResolver->currentWeights();
}

// ---------------------------------------------------------------------------
// feedback
// ---------------------------------------------------------------------------

void ProbabilityEngine::feedback(std::size_t  /* unitIndex */,
                                  std::int64_t correctSenseId,
                                  bool         isPhrase,
                                  double       confidence,
                                  const std::string& speakerId)
{
    // Record trust signal: the speaker provided a correct resolution.
    auto& model = getSpeakerModel(speakerId);
    model.recordSignal(TrustSignal::CONFIRMED_ACCURATE, confidence);

    // Submit CONVERSATION_TURN evidence to the correct sense's belief.
    const std::string domain = isPhrase
        ? ("phraseSense:" + std::to_string(correctSenseId))
        : ("sense:" + std::to_string(correctSenseId));

    Evidence ev;
    ev.kind            = EvidenceKind::CONVERSATION_TURN;
    ev.hypothesisId    = correctSenseId;
    ev.likelihoodRatio = 1.0 + 2.0 * confidence;  // strong positive signal
    ev.sourceWeight    = model.sourceWeight();
    ev.observedAt      = now();
    ev.reason          = "turn_feedback";

    m_store->submitAsync(UpdateJob{domain, {ev}, {}});
}

// ---------------------------------------------------------------------------
// recordTrustSignal
// ---------------------------------------------------------------------------

void ProbabilityEngine::recordTrustSignal(const std::string& speakerId,
                                           TrustSignal        signal,
                                           double             strength)
{
    auto& model = getSpeakerModel(speakerId);
    model.recordSignal(signal, strength);
}

// ---------------------------------------------------------------------------
// Seed / reset
// ---------------------------------------------------------------------------

void ProbabilityEngine::seedWeights(const WeightVector& w) {
    m_senseResolver->seedWeights(w);
}

void ProbabilityEngine::seedEmotionalPrior(
    const std::unordered_map<std::int64_t, double>& priorWeights)
{
    m_emotionBuilder->seedPrior(priorWeights);
}

void ProbabilityEngine::resetAll() {
    m_store->resetAll();
    m_emotionBuilder->resetToNeutral();
    m_intentAnalyzer->resetIntent();
}

void ProbabilityEngine::resetTurn() {
    m_intentAnalyzer->resetIntent();
    m_emotionBuilder->resetToNeutral();
}

void ProbabilityEngine::flush() {
    m_store->flush();
}

// ---------------------------------------------------------------------------
// getSpeakerModel (lazy construction)
// ---------------------------------------------------------------------------

SpeakerTrustModel& ProbabilityEngine::getSpeakerModel(const std::string& speakerId) {
    std::lock_guard<std::mutex> lk(m_speakerMutex);
    auto it = m_speakerModels.find(speakerId);
    if (it == m_speakerModels.end()) {
        auto model = std::make_unique<SpeakerTrustModel>(m_store, speakerId);
        model->initialize(m_config.defaultTrustPrior, m_config.trustHalfLifeSecs);
        auto [ins, ok] = m_speakerModels.emplace(speakerId, std::move(model));
        return *ins->second;
    }
    return *it->second;
}

} } // namespace elle::prob
