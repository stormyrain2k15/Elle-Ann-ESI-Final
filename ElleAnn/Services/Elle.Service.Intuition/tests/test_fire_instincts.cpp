#include <doctest/doctest.h>
#include "core/IntuitionEngine.h"

using namespace elle::intuition;

static IntuitRequest MakeReq(std::vector<std::string> tags,
                             float trust   = 0.5f,
                             float arousal = 0.0f,
                             float intensity = 0.5f) {
    IntuitRequest req;
    req.requestId       = "t-0";
    req.stimulusTags    = std::move(tags);
    req.speakerTrust    = trust;
    req.emotionArousal  = arousal;
    req.emotionIntensity = intensity;
    req.isPreResponse   = true;
    return req;
}

TEST_CASE("FireInstincts: exact-match stimulus fires the corresponding pattern") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    auto firings = eng.FireInstincts(MakeReq({"distress"}, /*trust*/0.5f, 0.0f, 0.5f));

    REQUIRE_FALSE(firings.empty());
    bool comfortFired = false;
    for (auto& f : firings) {
        if (f.pullType == "COMFORT") {
            comfortFired = true;
            CHECK(f.strength > 0.5f);
            CHECK(f.reason.find("pattern:distress -> COMFORT") != std::string::npos);
        }
    }
    CHECK(comfortFired);
}

TEST_CASE("FireInstincts: trust_floor gates high-trust-only patterns") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    auto lowTrust  = eng.FireInstincts(MakeReq({"josh"}, /*trust*/0.2f, 0.0f, 0.5f));
    auto highTrust = eng.FireInstincts(MakeReq({"josh"}, /*trust*/0.95f, 0.0f, 0.5f));

    bool joshFiredLow  = false;
    bool joshFiredHigh = false;
    for (auto& f : lowTrust)  if (f.reason.find("josh") != std::string::npos) joshFiredLow  = true;
    for (auto& f : highTrust) if (f.reason.find("josh") != std::string::npos) joshFiredHigh = true;

    CHECK_FALSE(joshFiredLow);
    CHECK(joshFiredHigh);
}

TEST_CASE("FireInstincts: emotion_min gates intensity-required patterns") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    auto cold = eng.FireInstincts(MakeReq({"hostile"}, 0.5f, 0.0f, /*intensity*/0.0f));
    auto hot  = eng.FireInstincts(MakeReq({"hostile"}, 0.5f, 0.0f, /*intensity*/0.6f));

    bool hostileColdFired = false;
    bool hostileHotFired  = false;
    for (auto& f : cold) if (f.pullType == "PROTECT") hostileColdFired = true;
    for (auto& f : hot)  if (f.pullType == "PROTECT") hostileHotFired  = true;

    CHECK_FALSE(hostileColdFired);
    CHECK(hostileHotFired);
}

TEST_CASE("FireInstincts: arousal amplifies strength but caps at 1.0") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    auto calm = eng.FireInstincts(MakeReq({"crying"}, 0.5f, /*arousal*/0.0f, 0.5f));
    auto hyper = eng.FireInstincts(MakeReq({"crying"}, 0.5f, /*arousal*/1.0f, 0.5f));

    float calmStr  = 0.0f;
    float hyperStr = 0.0f;
    for (auto& f : calm)  if (f.pullType == "COMFORT") calmStr  = f.strength;
    for (auto& f : hyper) if (f.pullType == "COMFORT") hyperStr = f.strength;

    CHECK(hyperStr >= calmStr);
    CHECK(hyperStr <= 1.0f);
}

TEST_CASE("FireInstincts: deduplicates competing patterns by pullType (keeps strongest)") {
    IntuitionEngine eng;
    std::vector<InstinctPattern> seed;
    auto add = [&](const char* stim, const char* pull, float w) {
        InstinctPattern p;
        p.stimulusTag = stim; p.pullType = pull; p.weight = w;
        seed.push_back(p);
    };
    add("a", "COMFORT", 0.5f);
    add("b", "COMFORT", 0.9f);
    add("c", "COMFORT", 0.7f);
    eng.ReplacePatterns(std::move(seed));

    auto firings = eng.FireInstincts(MakeReq({"a", "b", "c"}, 1.0f, 0.0f, 0.5f));

    REQUIRE(firings.size() == 1);
    CHECK(firings[0].pullType == "COMFORT");
    CHECK(firings[0].strength == doctest::Approx(0.9f).epsilon(1e-3f));
}

TEST_CASE("FireInstincts: substring match scores at 0.6 weight") {
    IntuitionEngine eng;
    std::vector<InstinctPattern> seed;
    InstinctPattern p;
    p.stimulusTag = "fear"; p.pullType = "COMFORT"; p.weight = 1.0f;
    seed.push_back(p);
    eng.ReplacePatterns(std::move(seed));

    auto exact = eng.FireInstincts(MakeReq({"fear"}, 0.5f, 0.0f, 0.5f));
    auto subst = eng.FireInstincts(MakeReq({"fearful"}, 0.5f, 0.0f, 0.5f));

    REQUIRE(exact.size() == 1);
    REQUIRE(subst.size() == 1);
    CHECK(exact[0].strength == doctest::Approx(1.0f).epsilon(1e-3f));
    CHECK(subst[0].strength == doctest::Approx(0.6f).epsilon(1e-3f));
}

TEST_CASE("FireInstincts: empty stimulus produces no firings") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    auto firings = eng.FireInstincts(MakeReq({}, 0.5f, 0.0f, 0.5f));
    CHECK(firings.empty());
}

TEST_CASE("FireInstincts: case-insensitive tag matching") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    auto upper = eng.FireInstincts(MakeReq({"DISTRESS"}, 0.5f, 0.0f, 0.5f));
    auto mixed = eng.FireInstincts(MakeReq({"DiStReSs"}, 0.5f, 0.0f, 0.5f));
    REQUIRE_FALSE(upper.empty());
    REQUIRE_FALSE(mixed.empty());
    CHECK(upper[0].pullType == "COMFORT");
    CHECK(mixed[0].pullType == "COMFORT");
}

TEST_CASE("FireInstincts: results are sorted by strength descending") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    auto firings = eng.FireInstincts(MakeReq(
        {"threat", "distress", "joy", "curiosity"}, 0.5f, 0.0f, 0.5f));

    for (size_t i = 1; i < firings.size(); ++i) {
        CHECK(firings[i-1].strength >= firings[i].strength);
    }
}
