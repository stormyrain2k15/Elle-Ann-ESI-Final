// ============================================================================
// Elle -- Language / Probability Engine Integration Bridge
// File: include/elle/prob/Bridge.hpp
//
// This header defines the two integration points where the language engine
// calls the probability engine. No changes to the language engine's internal
// pipeline are needed; both calls are additive and non-breaking.
//
// ---- Integration Point A: Before SenseCandidateResolver ----
//
//   auto probWeights = bridge.queryWeights();
//   // Convert to language engine ScoringWeights and pass to SenseCandidateResolver
//   ScoringWeights sw = Bridge::toScoringWeights(probWeights);
//   // ... run the normal language engine pipeline ...
//
// ---- Integration Point B: After MeaningObjectBuilder ----
//
//   auto req = Bridge::fromMeaningObject(meaning, convo);
//   auto probResult = bridge.analyze(req, speakerId);
//   // probResult.traceJson can be merged with DebugTrace.toJson()
//   // probResult.recommendedWeights feeds back to Point A next turn
//
// The bridge owns the ProbabilityEngine instance and exposes only the
// clean API surface the language engine needs.
// ============================================================================
#pragma once

#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"

// Forward-declare the language engine types so this header doesn't pull in
// the full language engine headers. The .cpp implementation includes them.
namespace elle {
    struct MeaningObject;
    struct ConversationContext;
    struct ScoringWeights;
}

namespace elle { namespace prob {

class Bridge {
public:
    // Construct with an existing engine or create a new one.
    explicit Bridge(ProbabilityEngineConfig cfg = ProbabilityEngineConfig::defaults());
    explicit Bridge(std::shared_ptr<ProbabilityEngine> engine);

    // ---- Integration Point A: weight query --------------------------------

    // Return the current live scoring weights for use by SenseCandidateResolver.
    [[nodiscard]] WeightVector queryWeights() const;

    // Convert a WeightVector to the language engine's ScoringWeights.
    // (Implementation in Bridge.cpp which includes elle/Config.hpp)
    [[nodiscard]] static elle::ScoringWeights toScoringWeights(const WeightVector& w);

    // Convert the language engine's ScoringWeights to a WeightVector.
    [[nodiscard]] static WeightVector fromScoringWeights(const elle::ScoringWeights& sw);

    // ---- Integration Point B: full analysis --------------------------------

    // Build a ProbabilityRequest from a completed MeaningObject.
    [[nodiscard]] static ProbabilityRequest fromMeaningObject(
        const elle::MeaningObject&       meaning,
        const elle::ConversationContext& convo);

    // Run the full probability analysis for one turn.
    [[nodiscard]] ProbabilityResult analyze(const ProbabilityRequest& req,
                                            const std::string& speakerId = "default");

    // ---- Feedback ----------------------------------------------------------

    // Record confirmed sense resolution (call after the language engine
    // has made its final decision so the probability engine can learn).
    void feedback(std::size_t  unitIndex,
                  std::int64_t confirmedSenseId,
                  bool         isPhrase,
                  double       confidence,
                  const std::string& speakerId = "default");

    // Record a trust signal for a speaker.
    void recordTrust(const std::string& speakerId,
                     TrustSignal        signal,
                     double             strength = 1.0);

    // ---- XChromosome / hormonal state hook ---------------------------------

    // Called by the XChromosome service when the hormonal state changes.
    // Passes the new emotional prior into the probability engine.
    void injectHormonalState(const std::unordered_map<std::int64_t, double>& state);

    // ---- Access ------------------------------------------------------------

    [[nodiscard]] ProbabilityEngine& engine() noexcept { return *m_engine; }
    [[nodiscard]] const ProbabilityEngine& engine() const noexcept { return *m_engine; }

private:
    std::shared_ptr<ProbabilityEngine> m_engine;
};

} } // namespace elle::prob
