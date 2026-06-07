#include "elle/prob/SenseProbabilityResolver.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

namespace elle { namespace prob {

static constexpr const char* DIM_CTX_FRAME   = "contextFrameMatch";
static constexpr const char* DIM_COOCCUR     = "nearbyWordCooccur";
static constexpr const char* DIM_EXAMPLE     = "senseExampleOverlap";
static constexpr const char* DIM_EMOTION     = "emotionalAlignment";
static constexpr const char* DIM_FREQ        = "frequency";
static constexpr const char* DIM_POS_COMPAT  = "posCompatibility";
static constexpr const char* DIM_POSNEG      = "posNegDrawAlignment";
static constexpr const char* DIM_HINT        = "conversationHint";

SenseProbabilityResolver::SenseProbabilityResolver(
    std::shared_ptr<BeliefStore> store)
    : m_store(std::move(store))
    , m_rng(std::random_device{}())
{}

std::string SenseProbabilityResolver::senseDomain(std::int64_t id) {
    return "sense:" + std::to_string(id);
}

std::string SenseProbabilityResolver::phraseSenseDomain(std::int64_t id) {
    return "phraseSense:" + std::to_string(id);
}

std::string SenseProbabilityResolver::weightDomain(const std::string& dim) {
    return "weight:" + dim;
}

void SenseProbabilityResolver::seedWeights(const WeightVector& w) {

    auto seedDim = [&](const std::string& name, double value) {
        Distribution prior;
        prior.mass[0] = std::max(0.01, value);
        prior.mass[1] = std::max(0.01, 1.0 - value);
        prior.normalize();
        m_store->registerBelief(weightDomain(name), prior, 0.0);
    };

    seedDim(DIM_CTX_FRAME,  w.contextFrameMatch);
    seedDim(DIM_COOCCUR,    w.nearbyWordCooccur);
    seedDim(DIM_EXAMPLE,    w.senseExampleOverlap);
    seedDim(DIM_EMOTION,    w.emotionalAlignment);
    seedDim(DIM_FREQ,       w.frequency);
    seedDim(DIM_POS_COMPAT, w.posCompatibility);
    seedDim(DIM_POSNEG,     w.posNegDrawAlignment);
    seedDim(DIM_HINT,       w.conversationHint);
}

WeightVector SenseProbabilityResolver::currentWeights() const {
    auto readDim = [&](const std::string& name) -> double {
        const auto d = m_store->getPosterior(weightDomain(name));
        if (d.empty()) return 0.5;
        return std::clamp(d.p(0), 0.05, 1.0);
    };

    WeightVector w;
    w.contextFrameMatch   = readDim(DIM_CTX_FRAME);
    w.nearbyWordCooccur   = readDim(DIM_COOCCUR);
    w.senseExampleOverlap = readDim(DIM_EXAMPLE);
    w.emotionalAlignment  = readDim(DIM_EMOTION);
    w.frequency           = readDim(DIM_FREQ);
    w.posCompatibility    = readDim(DIM_POS_COMPAT);
    w.posNegDrawAlignment = readDim(DIM_POSNEG);
    w.conversationHint    = readDim(DIM_HINT);
    return w;
}

double SenseProbabilityResolver::contextAlignmentScore(
    std::int64_t candidateId,
    const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const
{
    if (ctxHints.empty()) return 0.0;

    double best = 0.0;
    for (const auto& h : ctxHints) {
        if (h.score > best) best = h.score;
    }

    const double recency_penalty = 1.0 / (1.0 + 0.001 * static_cast<double>(candidateId));
    return best * recency_penalty;
}

double SenseProbabilityResolver::emotionalAlignmentScore(
    std::int64_t ,
    const std::unordered_map<std::int64_t, double>& emotionProfile,
    bool ) const
{
    if (emotionProfile.empty()) return 0.0;

    double total = 0.0;
    for (const auto& [eid, w] : emotionProfile) {
        total += std::abs(w);
    }

    return std::min(total / 3.0, 1.0);
}

Distribution SenseProbabilityResolver::buildLikelihood(
    const ProbabilityRequest::UnitSpec&                     unit,
    const std::vector<ProbabilityRequest::ContextHint>&     ctxHints,
    const std::unordered_map<std::int64_t, double>&         emotionProfile,
    const WeightVector&                                     weights,
    bool                                                    isPhrase) const
{
    const auto& candidates = isPhrase
        ? unit.phraseSenseCandidateIds
        : unit.senseCandidateIds;

    Distribution likelihood;
    if (candidates.empty()) return likelihood;

    for (auto candidateId : candidates) {

        const std::string domain = isPhrase
            ? phraseSenseDomain(candidateId)
            : senseDomain(candidateId);

        const double priorMass = [&]() -> double {
            const auto post = m_store->getPosterior(domain);
            if (post.empty()) return 1.0 / static_cast<double>(candidates.size());
            return post.p(candidateId);
        }();

        const double ctx   = contextAlignmentScore(candidateId, ctxHints);
        const double emo   = emotionalAlignmentScore(candidateId, emotionProfile, isPhrase);

        double score = priorMass;
        score += weights.contextFrameMatch   * ctx;
        score += weights.emotionalAlignment  * emo;

        likelihood.mass[candidateId] = std::max(score, std::numeric_limits<double>::epsilon());
    }

    likelihood.normalize();
    return likelihood;
}

void SenseProbabilityResolver::resolve(const ProbabilityRequest& req,
                                        ProbabilityResult&        result) const
{
    const WeightVector weights = currentWeights();
    result.recommendedWeights  = weights;

    double confidenceProduct = 1.0;
    const bool stochastic    = req.stochastic;

    if (stochastic && req.randomSeed != 0) {
        std::lock_guard<std::mutex> lk(m_rngMutex);
        m_rng.seed(req.randomSeed);
    }

    result.units.clear();
    result.units.reserve(req.units.size());

    for (const auto& unit : req.units) {
        ProbabilityResult::UnitResult ur;
        ur.unitIndex = unit.unitIndex;

        const bool isPhrase = unit.isPhrase &&
                              !unit.phraseSenseCandidateIds.empty();

        const auto& candidates = isPhrase
            ? unit.phraseSenseCandidateIds
            : unit.senseCandidateIds;

        if (unit.isUnknown || candidates.empty()) {
            ur.winningSenseId    = -1;
            ur.winnerProbability = 0.0;
            ur.posteriorEntropy  = 0.0;
            result.units.push_back(std::move(ur));
            continue;
        }

        for (auto cid : candidates) {
            const std::string dom = isPhrase
                ? phraseSenseDomain(cid)
                : senseDomain(cid);

            if (!m_store->getBelief(dom)) {

                auto prior = BayesianUpdater::uniformPrior(candidates);
                m_store->registerBelief(dom, prior, 0.0);
            }
        }

        const Distribution likelihood = buildLikelihood(
            unit, req.contextHints, req.emotionalProfile, weights, isPhrase);

        Distribution posterior;
        posterior.mass.reserve(candidates.size());

        ur.rankedCandidates.reserve(candidates.size());

        for (auto cid : candidates) {
            const double lkMass = likelihood.p(cid);
            const double lr     = std::max(lkMass / (1.0 / static_cast<double>(candidates.size())),
                                           0.01);

            const std::string dom = isPhrase
                ? phraseSenseDomain(cid)
                : senseDomain(cid);

            Evidence ev;
            ev.kind            = EvidenceKind::LEXICAL_MATCH;
            ev.hypothesisId    = cid;
            ev.likelihoodRatio = lr;
            ev.sourceWeight    = 1.0;
            ev.observedAt      = now();

            posterior.mass[cid] = likelihood.p(cid);

            ScoredHypothesis sh;
            sh.hypothesisId              = cid;
            sh.contributingFactors["lr"] = lr;
            sh.contributingFactors["ctx"] = contextAlignmentScore(cid, req.contextHints);
            sh.contributingFactors["emo"] = emotionalAlignmentScore(cid, req.emotionalProfile, isPhrase);
            ur.rankedCandidates.push_back(std::move(sh));

            m_store->submitAsync(UpdateJob{dom, {ev}, {}});
        }

        posterior.normalize();

        for (auto& sh : ur.rankedCandidates) {
            sh.probability = posterior.p(sh.hypothesisId);
        }
        std::sort(ur.rankedCandidates.begin(), ur.rankedCandidates.end(),
            [](const ScoredHypothesis& a, const ScoredHypothesis& b){
                return a.probability > b.probability;
            });

        std::int64_t winner = -1;
        if (stochastic) {
            double u01;
            {
                std::lock_guard<std::mutex> lk(m_rngMutex);
                u01 = std::uniform_real_distribution<double>{0.0, 1.0}(m_rng);
            }
            winner = posterior.sample(u01);
        } else {
            winner = posterior.map();
        }

        ur.winningSenseId      = winner;
        ur.isPhraseSense       = isPhrase;
        ur.winnerProbability   = posterior.p(winner);
        ur.posteriorEntropy    = posterior.entropy();

        feedbackWeightEvidence(unit, winner, ur.winnerProbability, isPhrase);

        confidenceProduct *= std::max(ur.winnerProbability, 0.01);
        result.units.push_back(std::move(ur));
    }

    if (!req.units.empty()) {
        result.overallConfidence = std::pow(confidenceProduct,
            1.0 / static_cast<double>(req.units.size()));
    }
}

void SenseProbabilityResolver::feedbackWeightEvidence(
    const ProbabilityRequest::UnitSpec& ,
    std::int64_t                        ,
    double                              confidence,
    bool                                ) const
{

    const double lr = (confidence > 0.5)
        ? 1.0 + 2.0 * (confidence - 0.5)
        : 1.0 / (1.0 + 2.0 * (0.5 - confidence));

    auto feedback = [&](const std::string& dim) {
        Evidence ev;
        ev.kind            = EvidenceKind::CONVERSATION_TURN;
        ev.hypothesisId    = (confidence > 0.5) ? 0 : 1;
        ev.likelihoodRatio = lr;
        ev.sourceWeight    = 0.3;
        ev.observedAt      = now();
        ev.reason          = "confidence_feedback";
        m_store->submitAsync(UpdateJob{weightDomain(dim), {ev}, {}});
    };

    feedback(DIM_CTX_FRAME);
    feedback(DIM_EMOTION);
    feedback(DIM_HINT);
}

} }
