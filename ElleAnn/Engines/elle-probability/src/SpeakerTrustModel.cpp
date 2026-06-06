// ============================================================================
// Elle Probability Engine -- Speaker Trust Model implementation
// File: src/SpeakerTrustModel.cpp
// ============================================================================
#include "elle/prob/SpeakerTrustModel.hpp"

#include <cmath>
#include <stdexcept>

namespace elle { namespace prob {

// ---------------------------------------------------------------------------
// BetaParams helpers
// ---------------------------------------------------------------------------

double BetaParams::stddev() const noexcept {
    return std::sqrt(variance());
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SpeakerTrustModel::SpeakerTrustModel(std::shared_ptr<BeliefStore> store,
                                      std::string                  speakerId)
    : m_speakerId(std::move(speakerId))
    , m_domain("trust:" + m_speakerId)
    , m_store(std::move(store))
{}

// ---------------------------------------------------------------------------
// initialize
// ---------------------------------------------------------------------------

void SpeakerTrustModel::initialize(BetaParams prior, double halfLifeSecs) {
    // Encode the Beta distribution as a two-bucket distribution.
    // We store alpha and beta as raw masses (not normalized probabilities).
    Distribution d;
    d.mass[ALPHA_KEY] = prior.alpha;
    d.mass[BETA_KEY]  = prior.beta;
    // Intentionally NOT normalizing -- we store raw counts, not probabilities.
    // The mean is computed as alpha / (alpha + beta) directly from the masses.

    Belief b;
    b.domain       = m_domain;
    b.prior        = d;
    b.posterior    = d;
    b.halfLifeSecs = halfLifeSecs;
    b.lastUpdated  = now();
    m_store->upsertBelief(b);
}

// ---------------------------------------------------------------------------
// recordSignal
// ---------------------------------------------------------------------------

void SpeakerTrustModel::recordSignal(TrustSignal signal, double strength) {
    // Ensure belief exists.
    if (!m_store->getBelief(m_domain)) {
        initialize();
    }

    // Map signal to alpha/beta increment.
    double dAlpha = 0.0;
    double dBeta  = 0.0;

    switch (signal) {
        case TrustSignal::CONFIRMED_ACCURATE:    dAlpha = 1.0 * strength; break;
        case TrustSignal::KEPT_PROMISE:          dAlpha = 1.5 * strength; break;
        case TrustSignal::CONSISTENT_WITH_HISTORY: dAlpha = 0.5 * strength; break;
        case TrustSignal::IDENTITY_CONFIRMED:    dAlpha = 2.0 * strength; break;
        case TrustSignal::CORRECTION_NEEDED:     dBeta  = 1.0 * strength; break;
        case TrustSignal::CONTRADICTED:          dBeta  = 1.5 * strength; break;
        case TrustSignal::HOSTILE_FRAMING:       dBeta  = 2.0 * strength; break;
    }

    // We update by directly modifying the posterior mass (raw counts).
    // This is the conjugate update: Beta(alpha, beta) + 1 success = Beta(alpha+1, beta).
    // We achieve this via the belief store by submitting evidence that
    // directly adjusts the ALPHA_KEY or BETA_KEY bucket.

    // For the Beta encoding, we use additive updates on the raw masses,
    // which requires a synchronous update to correctly increment the count.
    auto belief = m_store->getBelief(m_domain);
    if (!belief) return;

    if (dAlpha > 0.0) {
        belief->posterior.mass[ALPHA_KEY] =
            belief->posterior.mass.count(ALPHA_KEY)
                ? belief->posterior.mass[ALPHA_KEY] + dAlpha
                : dAlpha;
    }
    if (dBeta > 0.0) {
        belief->posterior.mass[BETA_KEY] =
            belief->posterior.mass.count(BETA_KEY)
                ? belief->posterior.mass[BETA_KEY] + dBeta
                : dBeta;
    }
    belief->lastUpdated = now();
    m_store->upsertBelief(std::move(*belief));
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

BetaParams SpeakerTrustModel::currentParams() const {
    auto b = m_store->getBelief(m_domain);
    if (!b) return {2.0, 2.0};

    const double alpha = b->posterior.mass.count(ALPHA_KEY)
        ? b->posterior.mass.at(ALPHA_KEY) : 2.0;
    const double beta  = b->posterior.mass.count(BETA_KEY)
        ? b->posterior.mass.at(BETA_KEY) : 2.0;

    return { std::max(alpha, 0.01), std::max(beta, 0.01) };
}

double SpeakerTrustModel::trustMean() const {
    const auto p = currentParams();
    return p.mean();
}

double SpeakerTrustModel::trustStddev() const {
    const auto p = currentParams();
    return p.stddev();
}

double SpeakerTrustModel::sourceWeight() const {
    // Map trust mean [0, 1] to source weight [0.1, 1.0].
    const double t = trustMean();
    return 0.1 + 0.9 * t;
}

void SpeakerTrustModel::reset() {
    m_store->resetBelief(m_domain);
}

} } // namespace elle::prob
