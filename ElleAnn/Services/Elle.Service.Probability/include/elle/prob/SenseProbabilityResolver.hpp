#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace elle { namespace prob {

class SenseProbabilityResolver {
public:
    explicit SenseProbabilityResolver(std::shared_ptr<BeliefStore> store);

    void resolve(const ProbabilityRequest& req,
                 ProbabilityResult&        result) const;

    [[nodiscard]] WeightVector currentWeights() const;

    void seedWeights(const WeightVector& w);

private:

    Distribution buildLikelihood(
        const ProbabilityRequest::UnitSpec&   unit,
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints,
        const std::unordered_map<std::int64_t, double>&     emotionProfile,
        const WeightVector&                   weights,
        bool                                  isPhrase) const;

    double contextAlignmentScore(
        std::int64_t                                        candidateId,
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const;

    double emotionalAlignmentScore(
        std::int64_t                                     candidateId,
        const std::unordered_map<std::int64_t, double>&  emotionProfile,
        bool                                             isPhrase) const;

    void feedbackWeightEvidence(const ProbabilityRequest::UnitSpec& unit,
                                std::int64_t                        winnerId,
                                double                              confidence,
                                bool                                isPhrase) const;

    static std::string senseDomain(std::int64_t senseId);
    static std::string phraseSenseDomain(std::int64_t phraseSenseId);
    static std::string weightDomain(const std::string& dimension);

    std::shared_ptr<BeliefStore> m_store;
    mutable std::mt19937_64      m_rng;
    mutable std::mutex           m_rngMutex;
};

} }
