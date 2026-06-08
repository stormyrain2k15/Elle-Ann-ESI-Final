#include <doctest/doctest.h>
#include "core/IntuitionEngine.h"

using namespace elle::intuition;

TEST_CASE("LoadDefaults populates 31 baseline patterns") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    CHECK(eng.PatternCount() == 31);
}

TEST_CASE("ReplacePatterns swaps the working set") {
    IntuitionEngine eng;
    eng.LoadDefaults();
    CHECK(eng.PatternCount() == 31);

    std::vector<InstinctPattern> minimal;
    InstinctPattern p;
    p.stimulusTag = "alpha"; p.pullType = "OPEN"; p.weight = 0.8f;
    minimal.push_back(p);
    eng.ReplacePatterns(std::move(minimal));

    CHECK(eng.PatternCount() == 1);
    auto snapshot = eng.Patterns();
    REQUIRE(snapshot.size() == 1);
    CHECK(snapshot[0].stimulusTag == "alpha");
    CHECK(snapshot[0].pullType == "OPEN");
}

TEST_CASE("AdjustPatternWeight clamps to [0.1, 1.0]") {
    IntuitionEngine eng;
    eng.LoadDefaults();

    eng.AdjustPatternWeight("OPEN", +5.0f);
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "OPEN") {
            CHECK(p.weight <= 1.0f + 1e-5f);
            CHECK(p.weight >= 0.1f - 1e-5f);
        }
    }

    eng.AdjustPatternWeight("OPEN", -5.0f);
    for (auto& p : eng.Patterns()) {
        if (p.pullType == "OPEN") {
            CHECK(p.weight >= 0.1f - 1e-5f);
        }
    }
}

TEST_CASE("Decay slowly relaxes weights toward floor but not below") {
    IntuitionEngine eng;
    std::vector<InstinctPattern> seed;
    InstinctPattern a; a.stimulusTag="x"; a.pullType="OPEN";  a.weight = 0.5f; seed.push_back(a);
    InstinctPattern b; b.stimulusTag="y"; b.pullType="ALERT"; b.weight = 0.25f; seed.push_back(b);
    eng.ReplacePatterns(std::move(seed));

    eng.Decay(0.3f, 0.05f);
    auto snap = eng.Patterns();

    float openW  = 0.0f;
    float alertW = 0.0f;
    for (auto& p : snap) {
        if (p.pullType == "OPEN")  openW  = p.weight;
        if (p.pullType == "ALERT") alertW = p.weight;
    }
    CHECK(openW  == doctest::Approx(0.45f).epsilon(1e-5f));
    CHECK(alertW == doctest::Approx(0.25f).epsilon(1e-5f));
}
