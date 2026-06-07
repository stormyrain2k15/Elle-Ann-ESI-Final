#include "elle/prob/IntentAnalyzer.hpp"

#include <algorithm>
#include <cmath>

namespace elle { namespace prob {

static const std::vector<std::int64_t> ALL_ACT_IDS = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

IntentAnalyzer::IntentAnalyzer(std::shared_ptr<BeliefStore> store)
    : m_store(std::move(store))
{

    m_store->registerBelief(
        DOMAIN_INTENT,
        BayesianUpdater::uniformPrior(ALL_ACT_IDS),
        120.0);
}

void IntentAnalyzer::analyze(const ProbabilityRequest& req,
                              ProbabilityResult&        result) const
{

    Distribution syntaxPriorDist = syntaxPrior(req);

    const auto& emotionPost = result.emotionalPosterior;
    const auto emoLikes     = emotionLikelihoods(emotionPost);
    const auto ctxLikes     = contextLikelihoods(req.contextHints);
    const auto trustLikes   = trustLikelihoods(result.speakerTrust);

    std::vector<Evidence> batch;
    batch.reserve(ALL_ACT_IDS.size() * 3);

    for (auto actId : ALL_ACT_IDS) {

        {
            auto it = emoLikes.find(actId);
            if (it != emoLikes.end() && std::abs(it->second - 1.0) > 1e-6) {
                Evidence ev;
                ev.kind            = EvidenceKind::EMOTIONAL_SIGNAL;
                ev.hypothesisId    = actId;
                ev.likelihoodRatio = it->second;
                ev.sourceWeight    = 0.7;
                ev.observedAt      = now();
                ev.reason          = "emotion_likelihood";
                batch.push_back(ev);
            }
        }

        {
            auto it = ctxLikes.find(actId);
            if (it != ctxLikes.end() && std::abs(it->second - 1.0) > 1e-6) {
                Evidence ev;
                ev.kind            = EvidenceKind::CONTEXT_FRAME;
                ev.hypothesisId    = actId;
                ev.likelihoodRatio = it->second;
                ev.sourceWeight    = 0.8;
                ev.observedAt      = now();
                ev.reason          = "context_likelihood";
                batch.push_back(ev);
            }
        }

        {
            auto it = trustLikes.find(actId);
            if (it != trustLikes.end() && std::abs(it->second - 1.0) > 1e-6) {
                Evidence ev;
                ev.kind            = EvidenceKind::SPEAKER_SIGNAL;
                ev.hypothesisId    = actId;
                ev.likelihoodRatio = it->second;
                ev.sourceWeight    = 0.5;
                ev.observedAt      = now();
                ev.reason          = "trust_likelihood";
                batch.push_back(ev);
            }
        }
    }

    {
        Belief syntaxBelief;
        syntaxBelief.domain       = DOMAIN_INTENT;
        syntaxBelief.prior        = syntaxPriorDist;
        syntaxBelief.posterior    = syntaxPriorDist;
        syntaxBelief.halfLifeSecs = 120.0;
        syntaxBelief.lastUpdated  = now();
        m_store->upsertBelief(syntaxBelief);
    }

    if (!batch.empty()) {
        m_store->submitSync(DOMAIN_INTENT, batch);
    }

    result.intentDistribution = m_store->getPosterior(DOMAIN_INTENT);
    const std::int64_t mapActId = result.intentDistribution.map();
    result.likelyAct = (mapActId >= 0 && mapActId <= 14)
        ? static_cast<PragmaticAct>(mapActId)
        : PragmaticAct::UNKNOWN;
}

Distribution IntentAnalyzer::syntaxPrior(const ProbabilityRequest& req) const {

    Distribution d = BayesianUpdater::uniformPrior(ALL_ACT_IDS);

    const double base = 1.0 / static_cast<double>(ALL_ACT_IDS.size());

    if (req.endsWithQuestion || req.questionCount > 0) {
        d.mass[actId(PragmaticAct::QUESTION)]   = base * 6.0;
        d.mass[actId(PragmaticAct::CONFIRM)]    = base * 3.0;
    }

    if (req.endsWithExclaim || req.exclamationCount > 1) {
        d.mass[actId(PragmaticAct::ASSERT)]  = base * 3.0;
        d.mass[actId(PragmaticAct::WARN)]    = base * 2.5;
        d.mass[actId(PragmaticAct::DENY)]    = base * 1.5;
    }

    if (!req.endsWithQuestion && req.exclamationCount == 0 && req.ellipsisCount == 0) {
        d.mass[actId(PragmaticAct::ASSERT)]  = base * 2.5;
    }

    if (req.ellipsisCount > 0) {
        d.mass[actId(PragmaticAct::DEFLECT)] = base * 2.0;
        d.mass[actId(PragmaticAct::COMFORT)] = base * 1.5;
    }

    d.normalize();
    return d;
}

Distribution IntentAnalyzer::currentIntentPosterior() const {
    return m_store->getPosterior(DOMAIN_INTENT);
}

void IntentAnalyzer::resetIntent() {
    m_store->resetBelief(DOMAIN_INTENT);
}

std::unordered_map<std::int64_t, double>
IntentAnalyzer::emotionLikelihoods(const Distribution& emotionPosterior) const {
    std::unordered_map<std::int64_t, double> lrs;

    constexpr std::int64_t EMO_ANGER     = 2;
    constexpr std::int64_t EMO_FEAR      = 3;
    constexpr std::int64_t EMO_SADNESS   = 4;
    constexpr std::int64_t EMO_JOY       = 5;
    constexpr std::int64_t EMO_TRUST     = 6;
    constexpr std::int64_t EMO_TENDERNESS= 7;
    constexpr std::int64_t EMO_COMFORT   = 8;
    constexpr std::int64_t EMO_SHAME     = 9;
    constexpr std::int64_t EMO_CURIOSITY = 10;

    const double anger     = emotionPosterior.p(EMO_ANGER);
    const double fear      = emotionPosterior.p(EMO_FEAR);
    const double sadness   = emotionPosterior.p(EMO_SADNESS);
    const double joy       = emotionPosterior.p(EMO_JOY);
    const double trust     = emotionPosterior.p(EMO_TRUST);
    const double tenderness= emotionPosterior.p(EMO_TENDERNESS);
    const double comfort   = emotionPosterior.p(EMO_COMFORT);
    const double shame     = emotionPosterior.p(EMO_SHAME);
    const double curiosity = emotionPosterior.p(EMO_CURIOSITY);

    lrs[actId(PragmaticAct::CHALLENGE)] = 1.0 + 2.0 * anger;
    lrs[actId(PragmaticAct::DENY)]      = 1.0 + 1.5 * anger;
    lrs[actId(PragmaticAct::WARN)]      = 1.0 + 1.5 * fear + anger * 0.5;
    lrs[actId(PragmaticAct::APOLOGIZE)] = 1.0 + 2.0 * shame + sadness * 0.5;
    lrs[actId(PragmaticAct::COMFORT)]   = 1.0 + 2.5 * comfort + tenderness * 2.0;
    lrs[actId(PragmaticAct::GREET)]     = 1.0 + 1.5 * joy + trust * 0.5;
    lrs[actId(PragmaticAct::THANK)]     = 1.0 + 1.5 * joy + trust;
    lrs[actId(PragmaticAct::PROMISE)]   = 1.0 + 2.0 * trust;
    lrs[actId(PragmaticAct::OFFER)]     = 1.0 + 1.5 * trust + tenderness * 0.5;
    lrs[actId(PragmaticAct::ASSERT)]    = 1.0 + 0.5 * trust;
    lrs[actId(PragmaticAct::QUESTION)]  = 1.0 + 2.0 * curiosity;
    lrs[actId(PragmaticAct::DEFLECT)]   = 1.0 + sadness * 1.5 + shame * 1.0;
    lrs[actId(PragmaticAct::REQUEST)]   = 1.0 + curiosity * 0.5;
    lrs[actId(PragmaticAct::CONFIRM)]   = 1.0 + trust * 0.5;

    return lrs;
}

std::unordered_map<std::int64_t, double>
IntentAnalyzer::contextLikelihoods(
    const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const
{
    std::unordered_map<std::int64_t, double> lrs;
    if (ctxHints.empty()) return lrs;

    double topScore = 0.0;
    std::int64_t topId = -1;
    for (const auto& h : ctxHints) {
        if (h.score > topScore) { topScore = h.score; topId = h.contextId; }
    }

    if (topScore < 0.1 || topId < 0) return lrs;

    switch (topId) {
        case 2:
            lrs[actId(PragmaticAct::DEFLECT)]  = 1.0 + topScore * 3.0;
            lrs[actId(PragmaticAct::ASSERT)]   = 1.0 + topScore * 1.5;
            break;
        case 3:
            lrs[actId(PragmaticAct::DENY)]     = 1.0 + topScore * 3.0;
            lrs[actId(PragmaticAct::CHALLENGE)]= 1.0 + topScore * 2.5;
            break;
        case 4:
            lrs[actId(PragmaticAct::COMFORT)]  = 1.0 + topScore * 3.0;
            lrs[actId(PragmaticAct::ASSERT)]   = 1.0 + topScore * 1.5;
            break;
        default:

            lrs[actId(PragmaticAct::ASSERT)] = 1.0 + topScore;
            break;
    }

    return lrs;
}

std::unordered_map<std::int64_t, double>
IntentAnalyzer::trustLikelihoods(double speakerTrust) const {
    std::unordered_map<std::int64_t, double> lrs;

    const double highTrust = std::clamp(speakerTrust, 0.0, 1.0);
    const double lowTrust  = 1.0 - highTrust;

    lrs[actId(PragmaticAct::ASSERT)]   = 1.0 + highTrust * 0.8;
    lrs[actId(PragmaticAct::PROMISE)]  = 1.0 + highTrust * 1.2;
    lrs[actId(PragmaticAct::CONFIRM)]  = 1.0 + highTrust * 0.6;
    lrs[actId(PragmaticAct::CHALLENGE)]= 1.0 + lowTrust  * 1.5;
    lrs[actId(PragmaticAct::DENY)]     = 1.0 + lowTrust  * 1.2;

    return lrs;
}

} }
