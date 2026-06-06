// ============================================================================
// Elle Probability Engine -- Emotional Posterior Builder
// File: include/elle/prob/EmotionalPosteriorBuilder.hpp
//
// Maintains a live Bayesian posterior over the 12 emotion dimensions defined
// in the language engine (EmotionID 1-12: VALENCE, ANGER, FEAR, SADNESS,
// JOY, TRUST, TENDERNESS, COMFORT, SHAME, CURIOSITY, POS_DRAW, NEG_DRAW).
//
// The EmotionalProfile from the language engine gives us aggregated weights
// per emotion. This layer converts those weights into likelihood ratios and
// applies them to the BeliefStore beliefs under domain "emotion:<id>".
//
// The result is a full posterior distribution over the emotion space that:
//   - updates incrementally across conversation turns
//   - decays back to neutral baseline over a configurable half-life
//   - feeds back into the sense resolver's emotional alignment score
//   - is available to Elle's other services (VAD engine, XChromosome engine)
//     via the BeliefStore query interface
//
// Philosophy note: emotion in language is probabilistic. "I'm fine." has a
// distribution over emotional states, not a single correct emotion. This layer
// maintains that distribution explicitly rather than collapsing to a scalar.
// ============================================================================
#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>

namespace elle { namespace prob {

// Configures decay and baseline for the emotional posterior.
struct EmotionalPosteriorConfig {
    // Half-life in seconds for each emotion dimension. After this time
    // without reinforcing evidence the posterior decays toward neutral.
    double halfLifeSecs = 300.0;  // 5 minutes default

    // Neutral baseline mass (uniform across all 12 dimensions).
    double neutralBaseline = 1.0 / 12.0;

    // How aggressively language-engine weights are converted to likelihood
    // ratios. lr = exp(gain * weight).
    double likelihoodGain = 2.0;

    // Minimum likelihood ratio (prevents a single strong signal from
    // collapsing the distribution entirely).
    double minLR = 0.1;
    double maxLR = 10.0;
};

class EmotionalPosteriorBuilder {
public:
    explicit EmotionalPosteriorBuilder(std::shared_ptr<BeliefStore> store,
                                       EmotionalPosteriorConfig     cfg = {});

    // Ingest the emotional profile from a language engine analysis result
    // and update the per-emotion beliefs in the store.
    // Returns the updated joint emotional posterior (Distribution over
    // EmotionID.value() space).
    Distribution update(
        const std::unordered_map<std::int64_t, double>& emotionProfile) const;

    // Return the current emotional posterior without updating.
    [[nodiscard]] Distribution currentPosterior() const;

    // Return the current valence estimate: net positive - negative draw.
    [[nodiscard]] double currentValence() const;

    // Return the arousal estimate (anger + fear + joy + curiosity weighted).
    [[nodiscard]] double currentArousal() const;

    // Return the dominance estimate (trust - shame - fear weighted).
    [[nodiscard]] double currentDominance() const;

    // Full VAD vector.
    struct VAD { double valence; double arousal; double dominance; };
    [[nodiscard]] VAD currentVAD() const;

    // Reset all emotion beliefs to neutral baseline.
    void resetToNeutral();

    // Seed the emotional prior from an external snapshot (e.g. XChromosome
    // hormonal state). This sets the prior without clearing evidence.
    void seedPrior(const std::unordered_map<std::int64_t, double>& priorWeights);

private:
    // Domain key for an emotion dimension.
    static std::string emotionDomain(std::int64_t emotionId);

    // Register all 12 emotion beliefs with neutral uniform priors if not
    // already present.
    void ensureBeliefsDefined() const;

    std::shared_ptr<BeliefStore>  m_store;
    EmotionalPosteriorConfig      m_cfg;
};

} } // namespace elle::prob
