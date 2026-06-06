// ============================================================================
// Elle Probability Engine -- Engine Facade
// File: include/elle/prob/ProbabilityEngine.hpp
//
// Top-level orchestrator. Owns the BeliefStore, worker pool, and all
// analysis subsystems. One instance per application; re-entrant across
// threads (analyze() is safe to call concurrently from many threads).
//
// Lifecycle:
//   1. Construct with config.
//   2. (Optional) call seedWeights() to load calibrated starting weights
//      from the language engine's config.
//   3. Call analyze() for each utterance -- returns ProbabilityResult.
//   4. Optionally call feedback() to record turn-level outcomes.
//   5. Destroy (shutdown is automatic in destructor).
//
// Integration with the language engine:
//   The language engine calls the probability engine at two points:
//     A. Before SenseCandidateResolver: get recommendedWeights() so scoring
//        uses live posteriors rather than static config values.
//     B. After MeaningObjectBuilder: call analyze() with the full
//        ProbabilityRequest (assembled from the MeaningObject + context)
//        so the probability engine can update its beliefs.
//
//   This two-pass design means the first call is a cheap weight query and
//   the second is the full Bayesian update. The language engine's hot path
//   is not blocked by the full probability computation.
// ============================================================================
#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/EmotionalPosteriorBuilder.hpp"
#include "elle/prob/IntentAnalyzer.hpp"
#include "elle/prob/SenseProbabilityResolver.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"
#include "elle/prob/Types.hpp"

#include <memory>
#include <string>

namespace elle { namespace prob {

// ============================================================================
// Engine configuration
// ============================================================================
struct ProbabilityEngineConfig {
    // Thread pool size for the BeliefStore worker pool.
    // 0 = use hardware_concurrency().
    std::size_t workerThreads = 0;

    // Emotional posterior configuration.
    EmotionalPosteriorConfig emotionConfig;

    // Default speaker trust prior (used when a new speaker is encountered).
    BetaParams defaultTrustPrior { 2.0, 2.0 };

    // Half-life for speaker trust decay (seconds). Default: 24 hours.
    double trustHalfLifeSecs = 86400.0;

    // Half-life for sense belief decay (seconds). 0 = no decay.
    // Sense posteriors represent recency-weighted usage in THIS conversation.
    double senseHalfLifeSecs = 600.0;  // 10 minutes

    // Half-life for intent belief decay (seconds).
    double intentHalfLifeSecs = 120.0;  // 2 minutes

    // Seed scoring weights from JSON config string. Empty = use hardcoded defaults.
    std::string weightsJson;

    // Enable stochastic sampling (non-deterministic MAP selection).
    bool stochastic = false;

    // Load from JSON file (merges with defaults for missing fields).
    [[nodiscard]] static ProbabilityEngineConfig loadFromFile(const std::string& path);
    [[nodiscard]] static ProbabilityEngineConfig loadFromString(const std::string& json);
    [[nodiscard]] static ProbabilityEngineConfig defaults();
};

// ============================================================================
// Engine
// ============================================================================
class ProbabilityEngine {
public:
    explicit ProbabilityEngine(ProbabilityEngineConfig config = ProbabilityEngineConfig::defaults());
    ~ProbabilityEngine();

    ProbabilityEngine(const ProbabilityEngine&)            = delete;
    ProbabilityEngine& operator=(const ProbabilityEngine&) = delete;

    // ---- Primary API ---------------------------------------------------

    // Full analysis: resolve sense posteriors, compute intent distribution,
    // update emotional beliefs, estimate speaker trust.
    // Thread-safe; may be called concurrently from many threads.
    [[nodiscard]] ProbabilityResult analyze(const ProbabilityRequest& req,
                                            const std::string& speakerId = "default");

    // Fast weight query: return the current live scoring weights.
    // Call this BEFORE the language engine's SenseCandidateResolver to
    // inject live posteriors into that pass.
    [[nodiscard]] WeightVector currentWeights() const;

    // ---- Feedback / reward -------------------------------------------

    // Record a turn-level outcome after the full language engine pass
    // has confirmed a resolution.
    // correctSenseId: the confirmed SenseID (or PhraseSenseID if isPhrase).
    // isPhrase: true if correctSenseId is a PhraseSenseID.
    void feedback(std::size_t  unitIndex,
                  std::int64_t correctSenseId,
                  bool         isPhrase,
                  double       confidence,
                  const std::string& speakerId = "default");

    // Record a trust signal for a speaker.
    void recordTrustSignal(const std::string& speakerId,
                           TrustSignal        signal,
                           double             strength = 1.0);

    // ---- Seed / reset ---------------------------------------------------

    // Seed the weight beliefs from a static WeightVector (e.g. from the
    // language engine's config on first load).
    void seedWeights(const WeightVector& w);

    // Seed the emotional prior from an external state snapshot
    // (e.g. XChromosome service pushes hormonal state).
    void seedEmotionalPrior(const std::unordered_map<std::int64_t, double>& priorWeights);

    // Reset all beliefs to their priors (full clean slate).
    void resetAll();

    // Reset only the current-turn beliefs (sense posteriors, intent).
    // Keeps weight and trust beliefs, which are long-lived.
    void resetTurn();

    // ---- Introspection --------------------------------------------------

    [[nodiscard]] const ProbabilityEngineConfig& config() const noexcept { return m_config; }

    // Return the belief store (read-only; for diagnostic tooling only).
    [[nodiscard]] const BeliefStore& beliefStore() const noexcept { return *m_store; }

    // Drain all async update jobs and wait for idle.
    void flush();

private:
    // Build or retrieve the SpeakerTrustModel for a given speaker ID.
    SpeakerTrustModel& getSpeakerModel(const std::string& speakerId);

    ProbabilityEngineConfig                           m_config;
    std::shared_ptr<BeliefStore>                      m_store;
    std::unique_ptr<SenseProbabilityResolver>         m_senseResolver;
    std::unique_ptr<IntentAnalyzer>                   m_intentAnalyzer;
    std::unique_ptr<EmotionalPosteriorBuilder>        m_emotionBuilder;

    // One trust model per speaker; created lazily.
    mutable std::mutex                                        m_speakerMutex;
    std::unordered_map<std::string,
                       std::unique_ptr<SpeakerTrustModel>>   m_speakerModels;
};

} } // namespace elle::prob
