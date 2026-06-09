#include <doctest/doctest.h>
#include "../core/SlotSpecParser.h"

using elle::composer::core::ScoreFrameByRecency;

TEST_CASE("ScoreFrameByRecency returns base weight when frame never used") {
    float s = ScoreFrameByRecency(1.0f, 0, 1'000'000);
    CHECK(s == doctest::Approx(1.0f));
}

TEST_CASE("ScoreFrameByRecency penalises freshly-used frames more than stale ones") {
    int64_t now = 1'000'000'000LL;
    float fresh = ScoreFrameByRecency(1.0f, now - 1000, now);
    float stale = ScoreFrameByRecency(1.0f, now - 600'000, now);
    CHECK(fresh < stale);
}

TEST_CASE("ScoreFrameByRecency converges to base weight as elapsed time grows") {
    int64_t now = 1'000'000'000LL;
    float baseline = 1.0f;
    float veryStale = ScoreFrameByRecency(baseline, now - 60'000'000, now);
    CHECK(veryStale == doctest::Approx(baseline).epsilon(0.01));
}

TEST_CASE("ScoreFrameByRecency at exactly one half-life applies expected penalty") {
    int64_t now = 1'000'000'000LL;
    int64_t halfLifeMs = 300'000;
    float s = ScoreFrameByRecency(1.0f, now - halfLifeMs, now);
    CHECK(s == doctest::Approx(1.0f - 0.6f * 0.5f).epsilon(0.001));
}

TEST_CASE("ScoreFrameByRecency is deterministic") {
    int64_t now = 1'234'567'890LL;
    float a = ScoreFrameByRecency(0.7f, now - 12'345, now);
    float b = ScoreFrameByRecency(0.7f, now - 12'345, now);
    CHECK(a == b);
}

TEST_CASE("ScoreFrameByRecency: heavier baseWeight beats lighter even after penalty") {
    int64_t now = 1'000'000'000LL;
    float heavyFresh = ScoreFrameByRecency(2.0f, now - 1000, now);
    float lightStale = ScoreFrameByRecency(0.5f, now - 600'000, now);
    CHECK(heavyFresh > lightStale);
}
