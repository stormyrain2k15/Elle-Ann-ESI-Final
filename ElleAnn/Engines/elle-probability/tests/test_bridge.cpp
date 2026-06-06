// ============================================================================
// test_bridge.cpp -- Bridge regression tests.
//
// These tests only compile when the language-engine bridge is enabled
// (ELLE_PROB_HAS_LANGUAGE_BRIDGE). The CMakeLists adds this file to the
// test binary's sources only in that case.
// ============================================================================

#include <doctest/doctest.h>

#include "elle/prob/Bridge.hpp"
#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"

#include "elle/Config.hpp"
#include "elle/Types.hpp"

using namespace elle::prob;

TEST_CASE("Bridge: ScoringWeights round-trips via WeightVector") {
    elle::ScoringWeights sw{};
    sw.contextFrameMatch   = 1.5;
    sw.nearbyWordCooccur   = 0.4;
    sw.senseExampleOverlap = 0.55;
    sw.emotionalAlignment  = 0.85;
    sw.frequency           = 0.25;
    sw.posCompatibility    = 0.45;
    sw.posNegDrawAlignment = 0.65;
    sw.conversationHint    = 0.95;

    const WeightVector wv = Bridge::fromScoringWeights(sw);
    CHECK(wv.contextFrameMatch   == doctest::Approx(1.5));
    CHECK(wv.nearbyWordCooccur   == doctest::Approx(0.4));
    CHECK(wv.senseExampleOverlap == doctest::Approx(0.55));
    CHECK(wv.emotionalAlignment  == doctest::Approx(0.85));
    CHECK(wv.frequency           == doctest::Approx(0.25));
    CHECK(wv.posCompatibility    == doctest::Approx(0.45));
    CHECK(wv.posNegDrawAlignment == doctest::Approx(0.65));
    CHECK(wv.conversationHint    == doctest::Approx(0.95));

    const elle::ScoringWeights rt = Bridge::toScoringWeights(wv);
    CHECK(rt.contextFrameMatch   == doctest::Approx(sw.contextFrameMatch));
    CHECK(rt.nearbyWordCooccur   == doctest::Approx(sw.nearbyWordCooccur));
    CHECK(rt.senseExampleOverlap == doctest::Approx(sw.senseExampleOverlap));
    CHECK(rt.emotionalAlignment  == doctest::Approx(sw.emotionalAlignment));
    CHECK(rt.frequency           == doctest::Approx(sw.frequency));
    CHECK(rt.posCompatibility    == doctest::Approx(sw.posCompatibility));
    CHECK(rt.posNegDrawAlignment == doctest::Approx(sw.posNegDrawAlignment));
    CHECK(rt.conversationHint    == doctest::Approx(sw.conversationHint));
}

TEST_CASE("Bridge: MeaningObject -> ProbabilityRequest carries units") {
    elle::MeaningObject m{};
    m.rawInput = "fine";
    elle::WordUnit u{};
    u.wordId   = elle::WordID{42};
    u.senseCandidates.push_back(elle::SenseID{1});
    u.senseCandidates.push_back(elle::SenseID{2});
    u.senseCandidates.push_back(elle::SenseID{3});
    m.sequence.units.push_back(std::move(u));
    m.sequence.endsWithQuestion = false;

    elle::ConversationContext c{};
    c.speakerRelationship = "stranger";

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, c);
    REQUIRE(req.units.size() == 1);
    CHECK(req.units[0].wordId == 42);
    CHECK(req.units[0].senseCandidateIds.size() == 3);
    CHECK(req.units[0].senseCandidateIds[0] == 1);
    CHECK(req.units[0].senseCandidateIds[1] == 2);
    CHECK(req.units[0].senseCandidateIds[2] == 3);
    CHECK(req.speakerRelationship == "stranger");
}

TEST_CASE("Bridge: phrase units round-trip phraseSenseCandidates") {
    elle::MeaningObject m{};
    elle::WordUnit u{};
    u.phraseId = elle::PhraseID{99};
    u.phraseSenseCandidates.push_back(elle::PhraseSenseID{901});
    u.phraseSenseCandidates.push_back(elle::PhraseSenseID{902});
    m.sequence.units.push_back(std::move(u));

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});
    REQUIRE(req.units.size() == 1);
    CHECK(req.units[0].isPhrase);
    CHECK(req.units[0].phraseId == 99);
    REQUIRE(req.units[0].phraseSenseCandidateIds.size() == 2);
    CHECK(req.units[0].phraseSenseCandidateIds[0] == 901);
    CHECK(req.units[0].phraseSenseCandidateIds[1] == 902);
}

TEST_CASE("Bridge: contextFrames + activeContextHints both surface") {
    elle::MeaningObject m{};
    elle::ContextFrameMatch cfm{};
    cfm.contextId = elle::ContextID{1};
    cfm.score = 2.5;
    m.contextFrames.push_back(cfm);

    elle::ConversationContext c{};
    c.activeContextHints.push_back(elle::ContextID{2});

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, c);
    REQUIRE(req.contextHints.size() == 2);
    CHECK(req.contextHints[0].contextId == 1);
    CHECK(req.contextHints[0].score     == doctest::Approx(2.5));
    CHECK(req.contextHints[1].contextId == 2);
    CHECK(req.contextHints[1].score     == doctest::Approx(1.0));
}

TEST_CASE("Bridge: emotional profile carries through verbatim") {
    elle::MeaningObject m{};
    m.emotionalProfile.byEmotionId[2] = 0.6;  // SADNESS
    m.emotionalProfile.byEmotionId[7] = 0.3;  // JOY

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});
    CHECK(req.emotionalProfile.size() == 2);
    CHECK(req.emotionalProfile.at(2)  == doctest::Approx(0.6));
    CHECK(req.emotionalProfile.at(7)  == doctest::Approx(0.3));
}

TEST_CASE("Bridge: punctuation signals mirror the sequence") {
    elle::MeaningObject m{};
    m.sequence.endsWithQuestion = true;
    m.sequence.questionCount    = 2;
    m.sequence.exclamationCount = 1;
    m.sequence.ellipsisCount    = 3;
    m.sequence.endsWithExclaim  = false;

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});
    CHECK(req.endsWithQuestion);
    CHECK(req.questionCount    == 2);
    CHECK(req.exclamationCount == 1);
    CHECK(req.ellipsisCount    == 3);
    CHECK_FALSE(req.endsWithExclaim);
}

TEST_CASE("Bridge: analyze() runs the engine end-to-end") {
    Bridge b;
    elle::MeaningObject m{};
    elle::WordUnit u{};
    u.wordId = elle::WordID{1};
    u.senseCandidates.push_back(elle::SenseID{10});
    u.senseCandidates.push_back(elle::SenseID{11});
    m.sequence.units.push_back(std::move(u));
    m.sequence.endsWithQuestion = true;
    m.sequence.questionCount    = 1;

    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});
    const ProbabilityResult  r   = b.analyze(req, "test_speaker");
    CHECK(r.units.size() == 1);
    CHECK(r.overallConfidence > 0.0);
    CHECK(r.intentDistribution.support() > 0);
}

TEST_CASE("Bridge: feedback + trust signal modify the live engine") {
    Bridge b;
    elle::MeaningObject m{};
    elle::WordUnit u{};
    u.wordId = elle::WordID{1};
    u.senseCandidates.push_back(elle::SenseID{10});
    m.sequence.units.push_back(std::move(u));
    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});

    const ProbabilityResult before = b.analyze(req, "tester");

    b.feedback(0, 10, false, 0.95, "tester");
    b.recordTrust("tester", TrustSignal::CONFIRMED_ACCURATE, 1.0);

    const ProbabilityResult after = b.analyze(req, "tester");
    // Trust should have climbed off the 0.5 prior.
    CHECK(after.speakerTrust > before.speakerTrust);
}

TEST_CASE("Bridge: injectHormonalState seeds emotional prior") {
    Bridge b;
    b.injectHormonalState({{2, 0.7}, {5, 0.3}});  // sadness + resignation

    elle::MeaningObject m{};
    elle::WordUnit u{};
    u.wordId = elle::WordID{1};
    u.senseCandidates.push_back(elle::SenseID{10});
    m.sequence.units.push_back(std::move(u));
    const ProbabilityRequest req = Bridge::fromMeaningObject(m, {});

    const ProbabilityResult r = b.analyze(req, "x");
    // After injection the emotion posterior must have non-zero mass.
    CHECK(r.emotionalPosterior.support() > 0);
}
