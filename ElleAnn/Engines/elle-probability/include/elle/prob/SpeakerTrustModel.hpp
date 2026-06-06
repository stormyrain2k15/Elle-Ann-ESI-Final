// ============================================================================
// Elle Probability Engine -- Speaker Trust Model
// File: include/elle/prob/SpeakerTrustModel.hpp
//
// Maintains a Bayesian estimate of speaker trust in [0, 1] as a Beta
// distribution. The Beta(α, β) family is the conjugate prior for Bernoulli
// observations, which maps naturally to a trust accumulation model:
//
//   - Each "trustworthy" signal (confirmed assertion, kept promise,
//     accurate information) increments α.
//   - Each "untrustworthy" signal (correction needed, contradicted,
//     deceptive framing detected) increments β.
//   - Mean trust = α / (α + β).
//   - Uncertainty = variance = αβ / [(α+β)²(α+β+1)].
//
// Why this matters for Elle:
//   - High trust: evidence from this speaker gets higher sourceWeight in
//     the Bayesian updater, meaning it shifts beliefs more strongly.
//   - Low trust: ASSERT claims are downweighted; CHALLENGE acts are
//     upweighted; emotional signals are treated more cautiously.
//   - The model persists across conversation turns and decays slowly toward
//     the prior (mild skepticism: α=2, β=2) over the half-life.
//
// The trust estimate feeds directly into SenseProbabilityResolver's
// conversationHint weight and IntentAnalyzer's trustLikelihoods.
// ============================================================================
#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>
#include <string>

namespace elle { namespace prob {

struct BetaParams {
    double alpha = 2.0;   // pseudo-counts for trustworthy observations
    double beta  = 2.0;   // pseudo-counts for untrustworthy observations

    [[nodiscard]] double mean()     const noexcept { return alpha / (alpha + beta); }
    [[nodiscard]] double variance() const noexcept {
        const double n = alpha + beta;
        return (alpha * beta) / (n * n * (n + 1.0));
    }
    [[nodiscard]] double stddev()   const noexcept;
};

enum class TrustSignal : std::uint8_t {
    CONFIRMED_ACCURATE   = 0,  // speaker's claim was verified correct
    KEPT_PROMISE         = 1,
    CONSISTENT_WITH_HISTORY = 2,
    CORRECTION_NEEDED    = 3,  // speaker made an error that was corrected
    CONTRADICTED         = 4,  // speaker statement contradicted another
    HOSTILE_FRAMING      = 5,  // speaker used manipulative/hostile framing
    IDENTITY_CONFIRMED   = 6,  // speaker is a known trusted entity (e.g. Josh)
};

class SpeakerTrustModel {
public:
    // speakerId: arbitrary string identifying the speaker ("josh", "stranger_1", …)
    // The BeliefStore holds one Beta-encoded belief per speaker under
    // domain "trust:<speakerId>".
    SpeakerTrustModel(std::shared_ptr<BeliefStore> store,
                      std::string                  speakerId);

    // Register or re-register this speaker with the given prior.
    void initialize(BetaParams prior = {2.0, 2.0}, double halfLifeSecs = 86400.0);

    // Record a trust signal and update the Beta posterior.
    void recordSignal(TrustSignal signal, double strength = 1.0);

    // Return the current mean trust in [0, 1].
    [[nodiscard]] double trustMean()     const;

    // Return the standard deviation of the trust estimate.
    [[nodiscard]] double trustStddev()   const;

    // Return the current BetaParams extracted from the belief store.
    [[nodiscard]] BetaParams currentParams() const;

    // Return a source weight to use in Evidence packets from this speaker.
    // Maps trust mean to [0.1, 1.0] with a floor so even untrusted speakers
    // contribute some evidence.
    [[nodiscard]] double sourceWeight() const;

    // Reset to prior.
    void reset();

    [[nodiscard]] const std::string& speakerId() const noexcept { return m_speakerId; }

private:
    // Beta distribution is encoded in the Distribution as a two-entry map:
    //   hypothesis 0 -> alpha
    //   hypothesis 1 -> beta
    // (stored unnormalized; mean is extracted by ratio, not by mass lookup)
    static constexpr std::int64_t ALPHA_KEY = 0;
    static constexpr std::int64_t BETA_KEY  = 1;

    std::string                  m_speakerId;
    std::string                  m_domain;
    std::shared_ptr<BeliefStore> m_store;
};

} } // namespace elle::prob
