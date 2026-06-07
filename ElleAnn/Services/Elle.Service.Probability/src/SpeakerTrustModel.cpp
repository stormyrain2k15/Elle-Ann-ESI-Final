#include "elle/prob/SpeakerTrustModel.hpp"

#include <cmath>
#include <stdexcept>

namespace elle { namespace prob {

double BetaParams::stddev() const noexcept {
    return std::sqrt(variance());
}

SpeakerTrustModel::SpeakerTrustModel(std::shared_ptr<BeliefStore> store,
                                      std::string                  speakerId)
    : m_speakerId(std::move(speakerId))
    , m_domain("trust:" + m_speakerId)
    , m_store(std::move(store))
{}

void SpeakerTrustModel::initialize(BetaParams prior, double halfLifeSecs) {

    Distribution d;
    d.mass[ALPHA_KEY] = prior.alpha;
    d.mass[BETA_KEY]  = prior.beta;

    Belief b;
    b.domain       = m_domain;
    b.prior        = d;
    b.posterior    = d;
    b.halfLifeSecs = halfLifeSecs;
    b.lastUpdated  = now();
    m_store->upsertBelief(b);
}

void SpeakerTrustModel::recordSignal(TrustSignal signal, double strength) {

    if (!m_store->getBelief(m_domain)) {
        initialize();
    }

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

    const double t = trustMean();
    return 0.1 + 0.9 * t;
}

void SpeakerTrustModel::reset() {
    m_store->resetBelief(m_domain);
}

} }
