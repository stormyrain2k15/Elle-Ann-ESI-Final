// ============================================================================
// Elle Probability Engine -- Bayesian Updater
// File: include/elle/prob/BayesianUpdater.hpp
//
// The core reasoning primitive. Takes a belief and a stream of evidence
// packets, applies Bayes' theorem in log-space to avoid underflow, and
// returns the updated posterior.
//
// Log-space Bayes:
//   log P(H|E) ∝ log P(H) + sum_i [ s_i * log LR_i ]
//
// where s_i is the source weight of evidence packet i and LR_i is its
// likelihood ratio. After summing in log-space we exponentiate and
// normalize. This is numerically stable across many update steps.
//
// Thread safety: stateless. Safe to call from many threads simultaneously.
// ============================================================================
#pragma once

#include "elle/prob/Types.hpp"

#include <vector>

namespace elle { namespace prob {

class BayesianUpdater {
public:
    BayesianUpdater() = default;

    // Apply a batch of evidence to a belief in-place.
    // Adds each evidence packet to belief.evidenceLog.
    // Calls belief.posterior.normalize() before returning.
    void update(Belief& belief, const std::vector<Evidence>& evidence) const;

    // Apply a single evidence packet.
    void update(Belief& belief, const Evidence& ev) const;

    // Reset a belief's posterior to its prior and clear the evidence log.
    static void reset(Belief& belief);

    // Build a uniform prior over a set of hypothesis IDs.
    [[nodiscard]] static Distribution uniformPrior(
        const std::vector<std::int64_t>& hypotheses);

    // Build an empirical prior from frequency counts.
    // counts: hypothesis id -> observed frequency
    [[nodiscard]] static Distribution empiricalPrior(
        const std::unordered_map<std::int64_t, double>& counts);

    // Compute the Kullback-Leibler divergence D_KL(P || Q).
    // Returns inf if Q assigns zero mass to any hypothesis that P does not.
    [[nodiscard]] static double kl(const Distribution& p,
                                   const Distribution& q) noexcept;

    // Jeffrey's divergence (symmetric KL): (D_KL(P||Q) + D_KL(Q||P)) / 2.
    [[nodiscard]] static double jeffreys(const Distribution& p,
                                         const Distribution& q) noexcept;

private:
    // Single-step log-space update for one hypothesis.
    // log_posterior[h] += source_weight * log(likelihood_ratio)
    static void applyLogEvidence(std::unordered_map<std::int64_t, double>& logMass,
                                 std::int64_t hypothesisId,
                                 double       likelihoodRatio,
                                 double       sourceWeight);
};

} } // namespace elle::prob
