#include <doctest/doctest.h>
#include "core/IntuitionEngine.h"

using namespace elle::intuition;

TEST_CASE("SynthesizeIntuition: threat firings → DANGER lean") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;

    std::vector<InstinctFiring> firings;
    InstinctFiring f;
    f.pullType = "ALERT"; f.strength = 0.9f;
    firings.push_back(f);

    auto sig = eng.SynthesizeIntuition(req, firings);
    CHECK(sig.lean == "DANGER");
    CHECK(sig.confidence > 0.5f);
}

TEST_CASE("SynthesizeIntuition: comfort firings → REACH_OUT lean") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 0.0f;

    std::vector<InstinctFiring> firings;
    InstinctFiring f;
    f.pullType = "COMFORT"; f.strength = 0.85f;
    firings.push_back(f);

    auto sig = eng.SynthesizeIntuition(req, firings);
    CHECK(sig.lean == "REACH_OUT");
}

TEST_CASE("SynthesizeIntuition: high speaker trust biases toward SAFE") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 1.0f;

    auto sig = eng.SynthesizeIntuition(req, {});
    CHECK(sig.lean == "SAFE");
}

TEST_CASE("SynthesizeIntuition: negative valence + high arousal → DANGER") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy   = 0.0f;
    req.emotionValence  = -0.8f;
    req.emotionArousal  =  0.9f;
    req.emotionIntensity = 1.0f;
    req.speakerTrust    = 0.0f;

    std::vector<InstinctFiring> firings;
    InstinctFiring a; a.pullType = "ALERT"; a.strength = 1.0f; firings.push_back(a);

    auto sig = eng.SynthesizeIntuition(req, firings);
    CHECK(sig.lean == "DANGER");
}

TEST_CASE("SynthesizeIntuition: low ethical safety pulls toward DOUBT") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 0.0f;
    req.lastImaginationEthicalSafety = 0.1f;

    auto sig = eng.SynthesizeIntuition(req, {});
    CHECK(sig.lean == "DOUBT");
}

TEST_CASE("SynthesizeIntuition: low plausibility pulls toward UNCERTAIN") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 0.0f;
    req.lastImaginationPlausibility = 0.1f;

    auto sig = eng.SynthesizeIntuition(req, {});
    CHECK(sig.lean == "UNCERTAIN");
}

TEST_CASE("SynthesizeIntuition: high goal alignment pulls toward ENGAGE") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 0.0f;
    req.lastImaginationGoalAlignment = 0.95f;

    auto sig = eng.SynthesizeIntuition(req, {});
    CHECK(sig.lean == "ENGAGE");
}

TEST_CASE("SynthesizeIntuition: belief entropy attenuates confidence") {
    IntuitionEngine eng;

    IntuitRequest lo, hi;
    lo.beliefEntropy = 0.0f;
    hi.beliefEntropy = 0.9f;
    lo.speakerTrust = hi.speakerTrust = 1.0f;

    auto loSig = eng.SynthesizeIntuition(lo, {});
    auto hiSig = eng.SynthesizeIntuition(hi, {});

    CHECK(hiSig.confidence < loSig.confidence);
    CHECK(loSig.lean == "SAFE");
    CHECK(hiSig.lean == "SAFE");
}

TEST_CASE("SynthesizeIntuition: suppressReason set when confident and entropic") {
    IntuitionEngine eng;

    IntuitRequest req;
    req.beliefEntropy   = 0.8f;
    req.emotionValence  = 0.0f;
    req.emotionArousal  = 0.0f;
    req.emotionIntensity = 0.0f;
    req.speakerTrust    = 0.0f;

    std::vector<InstinctFiring> firings;
    for (int i = 0; i < 5; ++i) {
        InstinctFiring f;
        f.pullType = "COMFORT"; f.strength = 1.0f;
        firings.push_back(f);
    }

    auto sig = eng.SynthesizeIntuition(req, firings);
    CHECK(sig.confidence > 0.6f);
    CHECK(sig.suppressReason == true);
}

TEST_CASE("SynthesizeIntuition: neutral speaker without firings biases toward DOUBT (guard prior)") {
    IntuitionEngine eng;
    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 0.0f;

    auto sig = eng.SynthesizeIntuition(req, {});
    CHECK(sig.lean == "DOUBT");
}

TEST_CASE("SynthesizeIntuition: basis string carries lean + confidence + count") {
    IntuitionEngine eng;
    IntuitRequest req;
    req.beliefEntropy = 0.0f;
    req.speakerTrust  = 1.0f;
    req.lastImaginationPlausibility = 0.5f;

    std::vector<InstinctFiring> firings;
    InstinctFiring a; a.pullType = "OPEN"; a.strength = 0.8f; firings.push_back(a);

    auto sig = eng.SynthesizeIntuition(req, firings);
    CHECK(sig.basis.find("lean=") != std::string::npos);
    CHECK(sig.basis.find("conf=") != std::string::npos);
    CHECK(sig.basis.find("instincts=1") != std::string::npos);
    CHECK(sig.basis.find("img_plaus=") != std::string::npos);
}
