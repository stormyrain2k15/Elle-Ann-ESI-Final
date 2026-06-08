#include <doctest/doctest.h>
#include "core/IntuitionEngine.h"

using namespace elle::intuition;

TEST_CASE("Positive feedback (was_correct) strengthens the matching pull") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    float beforeAvg = 0.0f;
    int   count    = 0;
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "COMFORT") { beforeAvg += p.weight; ++count; }
    }
    REQUIRE(count > 0);
    beforeAvg /= count;

    for (int i = 0; i < 5; ++i) eng.AdjustPatternWeight("COMFORT", +0.05f);

    float afterAvg = 0.0f;
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "COMFORT") { afterAvg += p.weight; }
    }
    afterAvg /= count;

    CHECK(afterAvg > beforeAvg);
}

TEST_CASE("Negative feedback decays only the matching pull") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    float openBefore = 0.0f;
    float alertBefore = 0.0f;
    int   openN  = 0;
    int   alertN = 0;
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "OPEN")  { openBefore  += p.weight; ++openN; }
        if (p.pullType == "ALERT") { alertBefore += p.weight; ++alertN; }
    }
    REQUIRE(openN > 0); REQUIRE(alertN > 0);
    openBefore /= openN; alertBefore /= alertN;

    for (int i = 0; i < 5; ++i) eng.AdjustPatternWeight("OPEN", -0.05f);

    float openAfter = 0.0f, alertAfter = 0.0f;
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "OPEN")  openAfter  += p.weight;
        if (p.pullType == "ALERT") alertAfter += p.weight;
    }
    openAfter /= openN; alertAfter /= alertN;

    CHECK(openAfter  < openBefore);
    CHECK(alertAfter == doctest::Approx(alertBefore).epsilon(1e-5f));
}

TEST_CASE("AdjustPatternWeight on unknown pull is a no-op") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    auto before = eng.Patterns();

    eng.AdjustPatternWeight("NONEXISTENT_PULL_TYPE_ZZZ", +0.5f);
    auto after = eng.Patterns();

    REQUIRE(before.size() == after.size());
    for (size_t i = 0; i < before.size(); ++i) {
        CHECK(after[i].weight == doctest::Approx(before[i].weight).epsilon(1e-5f));
    }
}

TEST_CASE("Decay leaves low-weight patterns alone (floor)") {
    IntuitionEngine eng;
    std::vector<InstinctPattern> seed;
    InstinctPattern a; a.stimulusTag="x"; a.pullType="ALERT"; a.weight = 0.20f; seed.push_back(a);
    InstinctPattern b; b.stimulusTag="y"; b.pullType="OPEN";  b.weight = 0.80f; seed.push_back(b);
    eng.ReplacePatterns(std::move(seed));

    for (int i = 0; i < 10; ++i) eng.Decay(0.30f, 0.01f);
    auto snap = eng.Patterns();

    float alertW = 0.0f;
    float openW  = 0.0f;
    for (auto& p : snap) {
        if (p.pullType == "ALERT") alertW = p.weight;
        if (p.pullType == "OPEN")  openW  = p.weight;
    }
    CHECK(alertW == doctest::Approx(0.20f).epsilon(1e-5f));
    CHECK(openW  == doctest::Approx(0.70f).epsilon(1e-5f));
}
