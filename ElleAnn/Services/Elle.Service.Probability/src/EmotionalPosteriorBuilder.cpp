#include "elle/prob/EmotionalPosteriorBuilder.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace elle { namespace prob {

static const std::vector<std::int64_t> ALL_EMOTION_IDS = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

static const char* emotionName(std::int64_t id) {
    switch (id) {
        case  1: return "VALENCE";
        case  2: return "ANGER";
        case  3: return "FEAR";
        case  4: return "SADNESS";
        case  5: return "JOY";
        case  6: return "TRUST";
        case  7: return "TENDERNESS";
        case  8: return "COMFORT";
        case  9: return "SHAME";
        case 10: return "CURIOSITY";
        case 11: return "POS_DRAW";
        case 12: return "NEG_DRAW";
        default: return "UNKNOWN";
    }
}

std::string EmotionalPosteriorBuilder::emotionDomain(std::int64_t emotionId) {
    return std::string("emotion:") + std::to_string(emotionId);
}

EmotionalPosteriorBuilder::EmotionalPosteriorBuilder(
    std::shared_ptr<BeliefStore>  store,
    EmotionalPosteriorConfig      cfg)
    : m_store(std::move(store))
    , m_cfg(std::move(cfg))
{
    ensureBeliefsDefined();
}

void EmotionalPosteriorBuilder::ensureBeliefsDefined() const {

    for (auto eid : ALL_EMOTION_IDS) {
        Distribution prior;
        prior.mass[0] = 1.0 - m_cfg.neutralBaseline;
        prior.mass[1] = m_cfg.neutralBaseline;
        prior.normalize();
        m_store->registerBelief(emotionDomain(eid), prior, m_cfg.halfLifeSecs);
    }
}

Distribution EmotionalPosteriorBuilder::update(
    const std::unordered_map<std::int64_t, double>& emotionProfile) const
{
    ensureBeliefsDefined();

    for (auto eid : ALL_EMOTION_IDS) {
        auto it = emotionProfile.find(eid);
        const double weight = (it != emotionProfile.end()) ? it->second : 0.0;

        const double rawLR = std::exp(m_cfg.likelihoodGain * weight);
        const double lr    = std::clamp(rawLR, m_cfg.minLR, m_cfg.maxLR);

        Evidence ev;
        ev.kind            = EvidenceKind::EMOTIONAL_SIGNAL;
        ev.hypothesisId    = 1;
        ev.likelihoodRatio = lr;
        ev.sourceWeight    = 1.0;
        ev.observedAt      = now();
        ev.reason          = std::string("emotion_profile:") + emotionName(eid);

        m_store->submitAsync(UpdateJob{emotionDomain(eid), {ev}, {}});
    }

    m_store->flush();
    return currentPosterior();
}

Distribution EmotionalPosteriorBuilder::currentPosterior() const {
    Distribution joint;
    for (auto eid : ALL_EMOTION_IDS) {

        const double intensity = m_store->probability(emotionDomain(eid), 1);
        joint.mass[eid] = intensity;
    }

    joint.normalize();
    return joint;
}

double EmotionalPosteriorBuilder::currentValence() const {

    const double posD = m_store->probability(emotionDomain(11), 1);
    const double negD = m_store->probability(emotionDomain(12), 1);
    const double joy  = m_store->probability(emotionDomain( 5), 1);
    const double sad  = m_store->probability(emotionDomain( 4), 1);
    return (posD + joy) - (negD + sad);
}

double EmotionalPosteriorBuilder::currentArousal() const {

    const double anger  = m_store->probability(emotionDomain( 2), 1);
    const double fear   = m_store->probability(emotionDomain( 3), 1);
    const double joy    = m_store->probability(emotionDomain( 5), 1);
    const double curio  = m_store->probability(emotionDomain(10), 1);
    return std::clamp((anger + fear + joy + curio) / 4.0, 0.0, 1.0);
}

double EmotionalPosteriorBuilder::currentDominance() const {

    const double trust = m_store->probability(emotionDomain(6), 1);
    const double shame = m_store->probability(emotionDomain(9), 1);
    const double fear  = m_store->probability(emotionDomain(3), 1);
    return std::clamp(trust - (shame + fear) * 0.5, -1.0, 1.0);
}

EmotionalPosteriorBuilder::VAD EmotionalPosteriorBuilder::currentVAD() const {
    return { currentValence(), currentArousal(), currentDominance() };
}

void EmotionalPosteriorBuilder::resetToNeutral() {
    for (auto eid : ALL_EMOTION_IDS) {
        m_store->resetBelief(emotionDomain(eid));
    }
}

void EmotionalPosteriorBuilder::seedPrior(
    const std::unordered_map<std::int64_t, double>& priorWeights)
{
    for (auto eid : ALL_EMOTION_IDS) {
        auto it = priorWeights.find(eid);
        const double w = (it != priorWeights.end())
            ? std::clamp(it->second, 0.0, 1.0)
            : m_cfg.neutralBaseline;

        Distribution prior;
        prior.mass[0] = std::max(1e-6, 1.0 - w);
        prior.mass[1] = std::max(1e-6, w);
        prior.normalize();

        Belief b;
        b.domain       = emotionDomain(eid);
        b.prior        = prior;
        b.posterior    = prior;
        b.halfLifeSecs = m_cfg.halfLifeSecs;
        b.lastUpdated  = now();
        m_store->upsertBelief(b);
    }
}

} }
