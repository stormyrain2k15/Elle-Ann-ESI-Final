#pragma once

#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"

namespace elle {
    struct MeaningObject;
    struct ConversationContext;
    struct ScoringWeights;
}

namespace elle { namespace prob {

class Bridge {
public:

    explicit Bridge(ProbabilityEngineConfig cfg = ProbabilityEngineConfig::defaults());
    explicit Bridge(std::shared_ptr<ProbabilityEngine> engine);

    [[nodiscard]] WeightVector queryWeights() const;

    [[nodiscard]] static elle::ScoringWeights toScoringWeights(const WeightVector& w);

    [[nodiscard]] static WeightVector fromScoringWeights(const elle::ScoringWeights& sw);

    [[nodiscard]] static ProbabilityRequest fromMeaningObject(
        const elle::MeaningObject&       meaning,
        const elle::ConversationContext& convo);

    [[nodiscard]] ProbabilityResult analyze(const ProbabilityRequest& req,
                                            const std::string& speakerId = "default");

    void feedback(std::size_t  unitIndex,
                  std::int64_t confirmedSenseId,
                  bool         isPhrase,
                  double       confidence,
                  const std::string& speakerId = "default");

    void recordTrust(const std::string& speakerId,
                     TrustSignal        signal,
                     double             strength = 1.0);

    void injectHormonalState(const std::unordered_map<std::int64_t, double>& state);

    [[nodiscard]] ProbabilityEngine& engine() noexcept { return *m_engine; }
    [[nodiscard]] const ProbabilityEngine& engine() const noexcept { return *m_engine; }

private:
    std::shared_ptr<ProbabilityEngine> m_engine;
};

} }
