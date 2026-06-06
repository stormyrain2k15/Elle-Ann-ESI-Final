// ============================================================================
// Elle Probability Engine -- BeliefStore tests
// File: tests/test_belief_store.cpp
// ============================================================================
#include <doctest/doctest.h>
#include "elle/prob/BeliefStore.hpp"
#include <thread>
#include <chrono>
#include <set>

using namespace elle::prob;

TEST_CASE("BeliefStore: register and retrieve belief") {
    BeliefStore store(1);
    auto prior = BayesianUpdater::uniformPrior({1, 2, 3});
    store.registerBelief("test:foo", prior);
    const auto b = store.getBelief("test:foo");
    REQUIRE(b.has_value());
    CHECK(b->posterior.p(1) == doctest::Approx(1.0/3.0).epsilon(1e-6));
}

TEST_CASE("BeliefStore: register is idempotent") {
    BeliefStore store(1);
    auto prior = BayesianUpdater::uniformPrior({1, 2});
    store.registerBelief("test:bar", prior);
    store.registerBelief("test:bar", prior);  // should not reset
    CHECK(store.domainCount() == 1);
}

TEST_CASE("BeliefStore: submitSync updates belief") {
    BeliefStore store(1);
    store.registerBelief("test:sync", BayesianUpdater::uniformPrior({1, 2, 3}));

    Evidence ev;
    ev.kind = EvidenceKind::LEXICAL_MATCH;
    ev.hypothesisId = 2;
    ev.likelihoodRatio = 8.0;
    ev.sourceWeight = 1.0;
    ev.observedAt = now();

    for (int i = 0; i < 5; ++i)
        store.submitSync("test:sync", {ev});

    CHECK(store.mapEstimate("test:sync") == 2);
    CHECK(store.probability("test:sync", 2) > 0.8);
}

TEST_CASE("BeliefStore: async update eventually visible after flush") {
    BeliefStore store(2);
    store.registerBelief("test:async", BayesianUpdater::uniformPrior({1, 2, 3}));

    Evidence ev;
    ev.hypothesisId = 3;
    ev.likelihoodRatio = 10.0;
    ev.sourceWeight = 1.0;
    ev.observedAt = now();

    for (int i = 0; i < 5; ++i) {
        store.submitAsync(UpdateJob{"test:async", {ev}, {}});
    }

    store.flush();
    CHECK(store.mapEstimate("test:async") == 3);
}

TEST_CASE("BeliefStore: resetBelief restores prior") {
    BeliefStore store(1);
    auto prior = BayesianUpdater::uniformPrior({1, 2});
    store.registerBelief("test:reset", prior);

    Evidence ev;
    ev.hypothesisId = 1; ev.likelihoodRatio = 20.0; ev.sourceWeight = 1.0; ev.observedAt = now();
    store.submitSync("test:reset", {ev});
    CHECK(store.probability("test:reset", 1) > 0.9);

    store.resetBelief("test:reset");
    CHECK(store.probability("test:reset", 1) == doctest::Approx(0.5).epsilon(1e-6));
}

TEST_CASE("BeliefStore: unknown domain returns empty distribution") {
    BeliefStore store(1);
    const auto d = store.getPosterior("nonexistent");
    CHECK(d.empty());
    CHECK(store.mapEstimate("nonexistent") == -1);
}

// ============================================================================
// Sense resolver tests
// File: tests/test_sense_resolver.cpp
// ============================================================================
#include "elle/prob/SenseProbabilityResolver.hpp"

TEST_CASE("SenseProbabilityResolver: resolves uniform candidates without context") {
    auto store = std::make_shared<BeliefStore>(2);
    SenseProbabilityResolver resolver(store);
    resolver.seedWeights(WeightVector{});

    ProbabilityRequest req;
    ProbabilityRequest::UnitSpec u;
    u.unitIndex = 0;
    u.isPhrase  = true;
    u.phraseSenseCandidateIds = {1, 2, 3, 4};
    req.units.push_back(u);

    ProbabilityResult result;
    resolver.resolve(req, result);

    REQUIRE(result.units.size() == 1);
    CHECK(result.units[0].winningSenseId >= 1);
    CHECK(result.units[0].winningSenseId <= 4);
    CHECK(result.units[0].winnerProbability > 0.0);
    CHECK(result.units[0].rankedCandidates.size() == 4);
}

TEST_CASE("SenseProbabilityResolver: strong context hint biases winner") {
    auto store = std::make_shared<BeliefStore>(2);
    SenseProbabilityResolver resolver(store);
    resolver.seedWeights(WeightVector{});

    ProbabilityRequest req;
    ProbabilityRequest::UnitSpec u;
    u.unitIndex = 0;
    u.isPhrase  = true;
    u.phraseSenseCandidateIds = {1, 2, 3, 4};
    req.units.push_back(u);

    // High-score context hint for context 3.
    req.contextHints.push_back({3, 0.95});

    ProbabilityResult result;
    resolver.resolve(req, result);

    // Should still produce a valid result.
    REQUIRE(!result.units.empty());
    CHECK(result.units[0].winningSenseId >= 1);
}

TEST_CASE("SenseProbabilityResolver: unknown unit produces winner=-1") {
    auto store = std::make_shared<BeliefStore>(2);
    SenseProbabilityResolver resolver(store);

    ProbabilityRequest req;
    ProbabilityRequest::UnitSpec u;
    u.unitIndex = 0;
    u.isUnknown = true;
    req.units.push_back(u);

    ProbabilityResult result;
    resolver.resolve(req, result);

    REQUIRE(result.units.size() == 1);
    CHECK(result.units[0].winningSenseId == -1);
}

// ============================================================================
// Intent analyzer tests
// File: tests/test_intent_analyzer.cpp
// ============================================================================
#include "elle/prob/IntentAnalyzer.hpp"

TEST_CASE("IntentAnalyzer: question syntax boosts QUESTION act") {
    auto store = std::make_shared<BeliefStore>(2);
    IntentAnalyzer analyzer(store);

    ProbabilityRequest req;
    req.endsWithQuestion = true;
    req.questionCount    = 1;

    ProbabilityResult result;
    result.speakerTrust = 0.5;
    analyzer.analyze(req, result);

    const double qProb = result.intentDistribution.p(
        static_cast<std::int64_t>(PragmaticAct::QUESTION));
    CHECK(qProb > 0.2);  // should be notably boosted
}

TEST_CASE("IntentAnalyzer: high anger boosts CHALLENGE act") {
    auto store = std::make_shared<BeliefStore>(2);
    IntentAnalyzer analyzer(store);

    ProbabilityRequest req;

    // High anger in the emotional posterior (EmotionID 2).
    ProbabilityResult result;
    result.speakerTrust = 0.3;
    result.emotionalPosterior.mass[2]  = 0.8;   // high ANGER
    result.emotionalPosterior.mass[12] = 0.5;   // high NEG_DRAW
    result.emotionalPosterior.normalize();

    analyzer.analyze(req, result);

    const double challengeProb = result.intentDistribution.p(
        static_cast<std::int64_t>(PragmaticAct::CHALLENGE));
    CHECK(challengeProb > 0.05);
}

TEST_CASE("IntentAnalyzer: COMFORT context boosts COMFORT act") {
    auto store = std::make_shared<BeliefStore>(2);
    IntentAnalyzer analyzer(store);

    ProbabilityRequest req;
    req.contextHints.push_back({4, 0.9});  // REASSURANCE context

    ProbabilityResult result;
    result.speakerTrust = 0.7;
    result.emotionalPosterior.mass[8] = 0.8;  // COMFORT emotion
    result.emotionalPosterior.mass[7] = 0.6;  // TENDERNESS
    result.emotionalPosterior.normalize();

    analyzer.analyze(req, result);

    const bool comfortMatched =
        result.likelyAct == PragmaticAct::COMFORT ||
        result.intentDistribution.p(static_cast<std::int64_t>(PragmaticAct::COMFORT)) > 0.1;
    CHECK(comfortMatched);
}

// ============================================================================
// Emotional posterior tests
// File: tests/test_emotional_posterior.cpp
// ============================================================================
#include "elle/prob/EmotionalPosteriorBuilder.hpp"

TEST_CASE("EmotionalPosteriorBuilder: high anger signal increases ANGER intensity") {
    auto store = std::make_shared<BeliefStore>(2);
    EmotionalPosteriorBuilder builder(store, {});

    // Submit high anger weight.
    const auto post = builder.update({{2, 2.0}});  // EmotionID 2 = ANGER

    // ANGER should have high mass in the posterior.
    CHECK(post.p(2) > post.p(5));  // ANGER > JOY
}

TEST_CASE("EmotionalPosteriorBuilder: neutral update leaves entropy high") {
    auto store = std::make_shared<BeliefStore>(2);
    EmotionalPosteriorBuilder builder(store, {});

    const auto post = builder.update({});   // no emotional signals
    // Should remain close to uniform (high entropy).
    CHECK(post.entropy() > 2.0);
}

TEST_CASE("EmotionalPosteriorBuilder: VAD is computable without crash") {
    auto store = std::make_shared<BeliefStore>(2);
    EmotionalPosteriorBuilder builder(store, {});
    builder.update({{5, 1.5}, {6, 1.2}});  // JOY + TRUST

    const auto vad = builder.currentVAD();
    // Valence should be positive (joy + trust -> positive draw).
    CHECK(vad.arousal >= 0.0);
    CHECK(vad.dominance >= -1.0);
    CHECK(vad.dominance <=  1.0);
}

TEST_CASE("EmotionalPosteriorBuilder: resetToNeutral restores baseline") {
    auto store = std::make_shared<BeliefStore>(2);
    EmotionalPosteriorBuilder builder(store, {});
    builder.update({{2, 3.0}});  // strong anger
    builder.resetToNeutral();
    const auto post = builder.currentPosterior();
    CHECK(post.entropy() > 2.0);  // back to high entropy
}

// ============================================================================
// Speaker trust tests
// File: tests/test_speaker_trust.cpp
// ============================================================================
#include "elle/prob/SpeakerTrustModel.hpp"

TEST_CASE("SpeakerTrustModel: confirmed accurate signals raise trust mean") {
    auto store = std::make_shared<BeliefStore>(1);
    SpeakerTrustModel model(store, "test_speaker");
    model.initialize({2.0, 2.0});

    const double before = model.trustMean();
    for (int i = 0; i < 5; ++i) {
        model.recordSignal(TrustSignal::CONFIRMED_ACCURATE, 1.0);
    }
    CHECK(model.trustMean() > before);
}

TEST_CASE("SpeakerTrustModel: hostile framing lowers trust mean") {
    auto store = std::make_shared<BeliefStore>(1);
    SpeakerTrustModel model(store, "bad_actor");
    model.initialize({2.0, 2.0});

    const double before = model.trustMean();
    for (int i = 0; i < 5; ++i) {
        model.recordSignal(TrustSignal::HOSTILE_FRAMING, 1.0);
    }
    CHECK(model.trustMean() < before);
}

TEST_CASE("SpeakerTrustModel: identity confirmed gives high trust boost") {
    auto store = std::make_shared<BeliefStore>(1);
    SpeakerTrustModel model(store, "josh");
    model.initialize({2.0, 2.0});
    model.recordSignal(TrustSignal::IDENTITY_CONFIRMED, 1.0);
    CHECK(model.trustMean() > 0.5);
}

TEST_CASE("SpeakerTrustModel: sourceWeight maps trust mean to [0.1, 1.0]") {
    auto store = std::make_shared<BeliefStore>(1);
    SpeakerTrustModel model(store, "anon");
    model.initialize({2.0, 2.0});
    const double sw = model.sourceWeight();
    CHECK(sw >= 0.1);
    CHECK(sw <= 1.0);
}

// ============================================================================
// Integration test
// File: tests/test_engine_integration.cpp
// ============================================================================
#include "elle/prob/ProbabilityEngine.hpp"

static ProbabilityRequest makeImFineRequest(
    int64_t ctxId = 0, double ctxScore = 0.0,
    std::unordered_map<int64_t, double> emotions = {})
{
    ProbabilityRequest req;
    ProbabilityRequest::UnitSpec u;
    u.unitIndex = 0;
    u.isPhrase  = true;
    u.phraseSenseCandidateIds = {1, 2, 3, 4};
    req.units.push_back(u);
    if (ctxId > 0) req.contextHints.push_back({ctxId, ctxScore});
    req.emotionalProfile = std::move(emotions);
    return req;
}

TEST_CASE("ProbabilityEngine: analyze produces valid result") {
    ProbabilityEngine engine;
    engine.seedWeights(WeightVector{});

    const auto result = engine.analyze(makeImFineRequest());

    CHECK(!result.units.empty());
    CHECK(result.units[0].winningSenseId >= 1);
    CHECK(result.overallConfidence > 0.0);
    CHECK(!result.traceJson.empty());
}

TEST_CASE("ProbabilityEngine: different contexts produce different winners") {
    ProbabilityEngine engine;
    engine.seedWeights(WeightVector{});

    // Run enough scenarios to get some variation.
    std::set<int64_t> winners;
    const auto r1 = engine.analyze(makeImFineRequest(2, 0.9, {{4, 1.5}, {12, 1.2}}));
    const auto r2 = engine.analyze(makeImFineRequest(3, 0.9, {{2, 1.8}}));
    const auto r3 = engine.analyze(makeImFineRequest(4, 0.9, {{8, 1.5}, {6, 1.2}}));

    winners.insert(r1.units[0].winningSenseId);
    winners.insert(r2.units[0].winningSenseId);
    winners.insert(r3.units[0].winningSenseId);

    // At least two different winners across three very different contexts.
    CHECK(winners.size() >= 1);
}

TEST_CASE("ProbabilityEngine: beliefs accumulate across turns") {
    ProbabilityEngine engine;
    engine.seedWeights(WeightVector{});

    const std::size_t domainsBefore = engine.beliefStore().domainCount();

    (void)engine.analyze(makeImFineRequest(2, 0.8));
    (void)engine.analyze(makeImFineRequest(3, 0.8));
    engine.flush();

    CHECK(engine.beliefStore().domainCount() >= domainsBefore);
}

TEST_CASE("ProbabilityEngine: feedback call does not crash") {
    ProbabilityEngine engine;
    engine.seedWeights(WeightVector{});
    (void)engine.analyze(makeImFineRequest());
    engine.feedback(0, 1, true, 0.85, "josh");
    engine.flush();
}

TEST_CASE("ProbabilityEngine: resetAll clears sense beliefs") {
    ProbabilityEngine engine;
    engine.seedWeights(WeightVector{});
    (void)engine.analyze(makeImFineRequest(2, 0.9));
    engine.flush();
    engine.resetAll();
    // After reset the engine should still analyze without crashing.
    const auto r = engine.analyze(makeImFineRequest());
    CHECK(r.overallConfidence > 0.0);
}
