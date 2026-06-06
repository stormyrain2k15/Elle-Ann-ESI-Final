// ============================================================================
// Elle Probability Engine -- Sense Probability Resolver
// File: include/elle/prob/SenseProbabilityResolver.hpp
//
// Replaces the language engine's static weighted-sum scoring with live
// Bayesian posteriors. For each WordUnit the resolver:
//
//   1. Retrieves the current posterior for each candidate SenseID from
//      the BeliefStore (or seeds a uniform prior if unseen).
//   2. Constructs a likelihood vector from the context frame scores,
//      emotional profile, and scoring weight posteriors.
//   3. Runs a single Bayesian update step to produce the per-unit
//      posterior over candidates.
//   4. Selects winner via MAP (deterministic) or weighted sampling
//      (stochastic mode).
//   5. Feeds back the outcome as a CONVERSATION_TURN evidence packet
//      so future calls benefit from the accumulated history.
//
// The scoring weight vector is itself a belief held in the BeliefStore
// under domain "weight:*". Every resolved unit generates evidence that
// nudges the weight distribution toward what actually worked.
// ============================================================================
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

    // Resolve all units in a ProbabilityRequest and populate the
    // units[] field of the returned ProbabilityResult.
    // Also updates weight beliefs based on context alignment.
    void resolve(const ProbabilityRequest& req,
                 ProbabilityResult&        result) const;

    // Retrieve the current live scoring weights (MAP from weight beliefs).
    [[nodiscard]] WeightVector currentWeights() const;

    // Seed the weight beliefs from a static WeightVector.  Called once at
    // startup and whenever the language engine passes calibrated weights.
    void seedWeights(const WeightVector& w);

private:
    // Build a likelihood distribution for one unit's sense candidates using
    // the current weight vector and context/emotional evidence.
    Distribution buildLikelihood(
        const ProbabilityRequest::UnitSpec&   unit,
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints,
        const std::unordered_map<std::int64_t, double>&     emotionProfile,
        const WeightVector&                   weights,
        bool                                  isPhrase) const;

    // Compute the context alignment score for one sense candidate.
    double contextAlignmentScore(
        std::int64_t                                        candidateId,
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const;

    // Compute the emotional alignment score for one sense candidate.
    double emotionalAlignmentScore(
        std::int64_t                                     candidateId,
        const std::unordered_map<std::int64_t, double>&  emotionProfile,
        bool                                             isPhrase) const;

    // Generate a CONVERSATION_TURN evidence packet to update the weight
    // beliefs after a unit has been resolved.
    void feedbackWeightEvidence(const ProbabilityRequest::UnitSpec& unit,
                                std::int64_t                        winnerId,
                                double                              confidence,
                                bool                                isPhrase) const;

    // Domain key helpers.
    static std::string senseDomain(std::int64_t senseId);
    static std::string phraseSenseDomain(std::int64_t phraseSenseId);
    static std::string weightDomain(const std::string& dimension);

    std::shared_ptr<BeliefStore> m_store;
    mutable std::mt19937_64      m_rng;   // guarded by m_rngMutex
    mutable std::mutex           m_rngMutex;
};

} } // namespace elle::prob
