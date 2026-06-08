#include <doctest/doctest.h>
#include "core/IntuitionEngine.h"

using namespace elle::intuition;

TEST_CASE("BuildCombinedSignal: urgent flag set when any firing is urgent + strong") {
    IntuitionEngine eng;

    IntuitRequest req;
    IntuitResult  r;
    r.intuition.lean       = "DANGER";
    r.intuition.confidence = 0.8f;

    InstinctFiring f;
    f.pullType = "ALERT"; f.strength = 0.9f; f.urgent = true;
    r.instincts.push_back(f);

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.urgent == true);
}

TEST_CASE("BuildCombinedSignal: urgent NOT set when firing is urgent but weak") {
    IntuitionEngine eng;

    IntuitRequest req;
    IntuitResult  r;
    r.intuition.lean       = "DANGER";
    r.intuition.confidence = 0.8f;

    InstinctFiring f;
    f.pullType = "ALERT"; f.strength = 0.4f; f.urgent = true;
    r.instincts.push_back(f);

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.urgent == false);
}

TEST_CASE("BuildCombinedSignal: lean→recommendedAct mapping") {
    IntuitionEngine eng;
    IntuitRequest   req;

    struct Case { const char* lean; const char* act; };
    Case cases[] = {
        {"DANGER",    "WARN"},
        {"DOUBT",     "QUESTION"},
        {"SAFE",      "ASSERT"},
        {"REACH_OUT", "COMFORT"},
        {"ENGAGE",    "ACK_AND_PROBE"},
        {"UNCERTAIN", "QUESTION"},
    };
    for (auto& c : cases) {
        IntuitResult r;
        r.intuition.lean       = c.lean;
        r.intuition.confidence = 0.5f;
        auto out = eng.BuildCombinedSignal(req, std::move(r));
        CHECK(out.recommendedAct == c.act);
    }
}

TEST_CASE("BuildCombinedSignal: unknown lean falls back to ASSERT") {
    IntuitionEngine eng;
    IntuitRequest   req;
    IntuitResult    r;
    r.intuition.lean = "WEIRD_LEAN";
    r.intuition.confidence = 0.5f;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.recommendedAct == "ASSERT");
}

TEST_CASE("BuildCombinedSignal: holdAndReflect fires on confident UNCERTAIN") {
    IntuitionEngine eng;
    IntuitRequest   req;
    IntuitResult    r;
    r.intuition.lean       = "UNCERTAIN";
    r.intuition.confidence = 0.7f;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.holdAndReflect == true);
}

TEST_CASE("BuildCombinedSignal: holdAndReflect fires on confident DOUBT") {
    IntuitionEngine eng;
    IntuitRequest   req;
    IntuitResult    r;
    r.intuition.lean       = "DOUBT";
    r.intuition.confidence = 0.7f;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.holdAndReflect == true);
}

TEST_CASE("BuildCombinedSignal: holdAndReflect fires on suppressReason + high conf") {
    IntuitionEngine eng;
    IntuitRequest   req;
    IntuitResult    r;
    r.intuition.lean           = "SAFE";
    r.intuition.confidence     = 0.7f;
    r.intuition.suppressReason = true;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.holdAndReflect == true);
}

TEST_CASE("BuildCombinedSignal: priorWeight scales with confidence, decays with entropy") {
    IntuitionEngine eng;

    IntuitRequest a; a.beliefEntropy = 0.0f; a.isPreResponse = false;
    IntuitRequest b; b.beliefEntropy = 0.5f; b.isPreResponse = false;

    IntuitResult ra; ra.intuition.confidence = 0.8f;
    IntuitResult rb; rb.intuition.confidence = 0.8f;

    auto outA = eng.BuildCombinedSignal(a, std::move(ra));
    auto outB = eng.BuildCombinedSignal(b, std::move(rb));

    CHECK(outA.priorWeight  > outB.priorWeight);
    CHECK(outA.priorWeight <= 0.85f);
    CHECK(outA.priorWeight >= 0.0f);
}

TEST_CASE("BuildCombinedSignal: pre-response priorWeight is capped at 0.65") {
    IntuitionEngine eng;
    IntuitRequest   req;
    req.beliefEntropy = 0.0f;
    req.isPreResponse = true;

    IntuitResult r;
    r.intuition.confidence = 1.0f;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.priorWeight <= 0.65f + 1e-5f);
}

TEST_CASE("BuildCombinedSignal: post-response priorWeight can exceed 0.65 up to 0.85") {
    IntuitionEngine eng;
    IntuitRequest   req;
    req.beliefEntropy = 0.0f;
    req.isPreResponse = false;

    IntuitResult r;
    r.intuition.confidence = 1.0f;

    auto out = eng.BuildCombinedSignal(req, std::move(r));
    CHECK(out.priorWeight > 0.65f);
    CHECK(out.priorWeight <= 0.85f + 1e-5f);
}

TEST_CASE("Process: full pipeline produces consistent output for known input") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    IntuitRequest req;
    req.requestId      = "smoke-1";
    req.stimulusTags   = {"distress", "crying"};
    req.emotionValence = -0.5f;
    req.emotionArousal = 0.4f;
    req.emotionIntensity = 0.7f;
    req.speakerTrust   = 0.6f;
    req.beliefEntropy  = 0.3f;
    req.isPreResponse  = true;

    auto r = eng.Process(req);
    CHECK(r.requestId == "smoke-1");
    CHECK(r.intuition.lean == "REACH_OUT");
    CHECK(r.recommendedAct == "COMFORT");
    CHECK_FALSE(r.instincts.empty());
    CHECK(r.priorWeight >= 0.0f);
    CHECK(r.priorWeight <= 0.65f);
}
