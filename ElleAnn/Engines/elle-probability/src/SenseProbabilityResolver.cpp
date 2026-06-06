// ============================================================================
// Elle Probability Engine -- Sense Probability Resolver implementation
// File: src/SenseProbabilityResolver.cpp
// ============================================================================
#include "elle/prob/SenseProbabilityResolver.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

namespace elle { namespace prob {

// Scoring weight dimension names -- must match WeightVector field names.
static constexpr const char* DIM_CTX_FRAME   = "contextFrameMatch";
static constexpr const char* DIM_COOCCUR     = "nearbyWordCooccur";
static constexpr const char* DIM_EXAMPLE     = "senseExampleOverlap";
static constexpr const char* DIM_EMOTION     = "emotionalAlignment";
static constexpr const char* DIM_FREQ        = "frequency";
static constexpr const char* DIM_POS_COMPAT  = "posCompatibility";
static constexpr const char* DIM_POSNEG      = "posNegDrawAlignment";
static constexpr const char* DIM_HINT        = "conversationHint";

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SenseProbabilityResolver::SenseProbabilityResolver(
    std::shared_ptr<BeliefStore> store)
    : m_store(std::move(store))
    , m_rng(std::random_device{}())
{}

// ---------------------------------------------------------------------------
// Domain key helpers
// ---------------------------------------------------------------------------

std::string SenseProbabilityResolver::senseDomain(std::int64_t id) {
    return "sense:" + std::to_string(id);
}

std::string SenseProbabilityResolver::phraseSenseDomain(std::int64_t id) {
    return "phraseSense:" + std::to_string(id);
}

std::string SenseProbabilityResolver::weightDomain(const std::string& dim) {
    return "weight:" + dim;
}

// ---------------------------------------------------------------------------
// Seed weights
// ---------------------------------------------------------------------------

void SenseProbabilityResolver::seedWeights(const WeightVector& w) {
    // For each weight dimension, register a belief with a single-hypothesis
    // distribution where the hypothesis ID is 0 and its mass encodes the
    // weight value. The BeliefStore will update these as evidence accumulates.
    //
    // Convention: hypothesis 0 -> current weight value (stored as raw mass,
    // not a probability). The updater operates in log-space but we retrieve
    // the MAP (0) and read the raw mass as the current weight.
    // We store each dimension as a two-bucket distribution:
    //   bucket 0 -> proportion at max (1.0)
    //   bucket 1 -> proportion at zero (0.0)
    // So the effective weight = P(0) * 1.0 + P(1) * 0.0 = P(0) = mass[0].
    // This lets Bayesian updates shift mass between "full weight" and "zero".

    auto seedDim = [&](const std::string& name, double value) {
        Distribution prior;
        prior.mass[0] = std::max(0.01, value);         // high mass -> high weight
        prior.mass[1] = std::max(0.01, 1.0 - value);   // low mass  -> downweighted
        prior.normalize();
        m_store->registerBelief(weightDomain(name), prior, 0.0); // weights don't decay
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

// ---------------------------------------------------------------------------
// Current weights (MAP read from BeliefStore)
// ---------------------------------------------------------------------------

WeightVector SenseProbabilityResolver::currentWeights() const {
    auto readDim = [&](const std::string& name) -> double {
        const auto d = m_store->getPosterior(weightDomain(name));
        if (d.empty()) return 0.5;
        return std::clamp(d.p(0), 0.05, 1.0);  // P(full weight bucket)
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

// ---------------------------------------------------------------------------
// Build likelihood distribution for one unit's sense candidates
// ---------------------------------------------------------------------------

double SenseProbabilityResolver::contextAlignmentScore(
    std::int64_t candidateId,
    const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const
{
    if (ctxHints.empty()) return 0.0;
    // Simple: sum context scores weighted by candidate ID proximity.
    // In a full integration, the language engine would pre-compute per-sense
    // context alignment. Here we use the best context hint score as a proxy.
    double best = 0.0;
    for (const auto& h : ctxHints) {
        if (h.score > best) best = h.score;
    }
    // Modulate by the sense's own frequency signal (higher ID = newer sense,
    // lower base frequency -- conservative adjustment).
    const double recency_penalty = 1.0 / (1.0 + 0.001 * static_cast<double>(candidateId));
    return best * recency_penalty;
}

double SenseProbabilityResolver::emotionalAlignmentScore(
    std::int64_t /* candidateId */,
    const std::unordered_map<std::int64_t, double>& emotionProfile,
    bool /* isPhrase */) const
{
    if (emotionProfile.empty()) return 0.0;

    // Sum the emotion profile weights -- higher total emotional loading
    // means the utterance is emotionally charged, which should boost
    // emotionally loaded senses. Without per-sense emotion data at this
    // layer we use the aggregate signal.
    double total = 0.0;
    for (const auto& [eid, w] : emotionProfile) {
        total += std::abs(w);
    }
    // Cap at 1.0, normalize against a typical max aggregate of 3.0.
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
        // Retrieve the current prior probability for this sense from the store.
        const std::string domain = isPhrase
            ? phraseSenseDomain(candidateId)
            : senseDomain(candidateId);

        // Prior mass from the belief store (or uniform if unseen).
        const double priorMass = [&]() -> double {
            const auto post = m_store->getPosterior(domain);
            if (post.empty()) return 1.0 / static_cast<double>(candidates.size());
            return post.p(candidateId);
        }();

        // Build likelihood components.
        const double ctx   = contextAlignmentScore(candidateId, ctxHints);
        const double emo   = emotionalAlignmentScore(candidateId, emotionProfile, isPhrase);

        // Weighted score = weighted sum of likelihood signals.
        double score = priorMass;
        score += weights.contextFrameMatch   * ctx;
        score += weights.emotionalAlignment  * emo;
        // Frequency and other signals require per-sense DB data which lives
        // in the language engine; the language engine injects pre-computed
        // sense scores via the context hints. We model them via the prior.

        likelihood.mass[candidateId] = std::max(score, std::numeric_limits<double>::epsilon());
    }

    likelihood.normalize();
    return likelihood;
}

// ---------------------------------------------------------------------------
// resolve
// ---------------------------------------------------------------------------

void SenseProbabilityResolver::resolve(const ProbabilityRequest& req,
                                        ProbabilityResult&        result) const
{
    const WeightVector weights = currentWeights();
    result.recommendedWeights  = weights;

    double confidenceProduct = 1.0;
    const bool stochastic    = req.stochastic;

    // Seed RNG if stochastic mode is requested.
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

        // Ensure all candidate beliefs exist in the store.
        for (auto cid : candidates) {
            const std::string dom = isPhrase
                ? phraseSenseDomain(cid)
                : senseDomain(cid);

            if (!m_store->getBelief(dom)) {
                // Seed uniform prior over all candidates for this sense.
                auto prior = BayesianUpdater::uniformPrior(candidates);
                m_store->registerBelief(dom, prior, 0.0);
            }
        }

        // Build likelihood from context and emotion signals.
        const Distribution likelihood = buildLikelihood(
            unit, req.contextHints, req.emotionalProfile, weights, isPhrase);

        // Build the posterior by updating each candidate's belief with
        // the likelihood as a batch of evidence packets.
        Distribution posterior;
        posterior.mass.reserve(candidates.size());

        ur.rankedCandidates.reserve(candidates.size());

        for (auto cid : candidates) {
            const double lkMass = likelihood.p(cid);
            const double lr     = std::max(lkMass / (1.0 / static_cast<double>(candidates.size())),
                                           0.01);  // likelihood ratio vs uniform

            const std::string dom = isPhrase
                ? phraseSenseDomain(cid)
                : senseDomain(cid);

            Evidence ev;
            ev.kind            = EvidenceKind::LEXICAL_MATCH;
            ev.hypothesisId    = cid;
            ev.likelihoodRatio = lr;
            ev.sourceWeight    = 1.0;
            ev.observedAt      = now();

            // Submit async -- we don't need the updated belief for the
            // immediate decision; we'll read the posterior next call.
            // For the current decision, use the likelihood-weighted mass.
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

        // Rank candidates.
        for (auto& sh : ur.rankedCandidates) {
            sh.probability = posterior.p(sh.hypothesisId);
        }
        std::sort(ur.rankedCandidates.begin(), ur.rankedCandidates.end(),
            [](const ScoredHypothesis& a, const ScoredHypothesis& b){
                return a.probability > b.probability;
            });

        // Choose winner.
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

        // Feed back weight evidence based on how much confidence we have.
        feedbackWeightEvidence(unit, winner, ur.winnerProbability, isPhrase);

        confidenceProduct *= std::max(ur.winnerProbability, 0.01);
        result.units.push_back(std::move(ur));
    }

    // Geometric mean confidence.
    if (!req.units.empty()) {
        result.overallConfidence = std::pow(confidenceProduct,
            1.0 / static_cast<double>(req.units.size()));
    }
}

// ---------------------------------------------------------------------------
// feedbackWeightEvidence
// ---------------------------------------------------------------------------

void SenseProbabilityResolver::feedbackWeightEvidence(
    const ProbabilityRequest::UnitSpec& /* unit */,
    std::int64_t                        /* winnerId */,
    double                              confidence,
    bool                                /* isPhrase */) const
{
    // High confidence -> evidence that the current weights are working well
    // (reinforce the current weight distribution toward "full weight" bucket 0).
    // Low confidence -> mild evidence toward "lower weight" bucket 1.

    const double lr = (confidence > 0.5)
        ? 1.0 + 2.0 * (confidence - 0.5)   // [1.0, 2.0]
        : 1.0 / (1.0 + 2.0 * (0.5 - confidence));  // [0.5, 1.0]

    auto feedback = [&](const std::string& dim) {
        Evidence ev;
        ev.kind            = EvidenceKind::CONVERSATION_TURN;
        ev.hypothesisId    = (confidence > 0.5) ? 0 : 1;
        ev.likelihoodRatio = lr;
        ev.sourceWeight    = 0.3;  // low weight -- gradual drift
        ev.observedAt      = now();
        ev.reason          = "confidence_feedback";
        m_store->submitAsync(UpdateJob{weightDomain(dim), {ev}, {}});
    };

    feedback(DIM_CTX_FRAME);
    feedback(DIM_EMOTION);
    feedback(DIM_HINT);
}

} } // namespace elle::prob
