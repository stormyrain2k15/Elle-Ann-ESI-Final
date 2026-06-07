#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>

namespace elle { namespace prob {

struct EmotionalPosteriorConfig {

    double halfLifeSecs = 300.0;

    double neutralBaseline = 1.0 / 12.0;

    double likelihoodGain = 2.0;

    double minLR = 0.1;
    double maxLR = 10.0;
};

class EmotionalPosteriorBuilder {
public:
    explicit EmotionalPosteriorBuilder(std::shared_ptr<BeliefStore> store,
                                       EmotionalPosteriorConfig     cfg = {});

    Distribution update(
        const std::unordered_map<std::int64_t, double>& emotionProfile) const;

    [[nodiscard]] Distribution currentPosterior() const;

    [[nodiscard]] double currentValence() const;

    [[nodiscard]] double currentArousal() const;

    [[nodiscard]] double currentDominance() const;

    struct VAD { double valence; double arousal; double dominance; };
    [[nodiscard]] VAD currentVAD() const;

    void resetToNeutral();

    void seedPrior(const std::unordered_map<std::int64_t, double>& priorWeights);

private:

    static std::string emotionDomain(std::int64_t emotionId);

    void ensureBeliefsDefined() const;

    std::shared_ptr<BeliefStore>  m_store;
    EmotionalPosteriorConfig      m_cfg;
};

} }
