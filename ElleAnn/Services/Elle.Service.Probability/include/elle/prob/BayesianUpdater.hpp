#pragma once

#include "elle/prob/Types.hpp"

#include <vector>

namespace elle { namespace prob {

class BayesianUpdater {
public:
    BayesianUpdater() = default;

    void update(Belief& belief, const std::vector<Evidence>& evidence) const;

    void update(Belief& belief, const Evidence& ev) const;

    static void reset(Belief& belief);

    [[nodiscard]] static Distribution uniformPrior(
        const std::vector<std::int64_t>& hypotheses);

    [[nodiscard]] static Distribution empiricalPrior(
        const std::unordered_map<std::int64_t, double>& counts);

    [[nodiscard]] static double kl(const Distribution& p,
                                   const Distribution& q) noexcept;

    [[nodiscard]] static double jeffreys(const Distribution& p,
                                         const Distribution& q) noexcept;

private:

    static void applyLogEvidence(std::unordered_map<std::int64_t, double>& logMass,
                                 std::int64_t hypothesisId,
                                 double       likelihoodRatio,
                                 double       sourceWeight);
};

} }
