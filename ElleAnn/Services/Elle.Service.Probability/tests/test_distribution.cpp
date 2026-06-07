#include <doctest/doctest.h>
#include "elle/prob/Types.hpp"

#include <cmath>

using namespace elle::prob;

TEST_CASE("Distribution: normalize sums to 1") {
    Distribution d;
    d.mass[1] = 3.0;
    d.mass[2] = 1.0;
    d.mass[3] = 0.0;
    d.normalize();

    CHECK(d.p(1) == doctest::Approx(0.75).epsilon(1e-9));
    CHECK(d.p(2) == doctest::Approx(0.25).epsilon(1e-9));
    CHECK(d.p(3) == doctest::Approx(0.0).epsilon(1e-9));

    double total = 0.0;
    for (const auto& [k, v] : d.mass) total += v;
    CHECK(total == doctest::Approx(1.0).epsilon(1e-9));
}

TEST_CASE("Distribution: normalize all-zero becomes uniform") {
    Distribution d;
    d.mass[10] = 0.0;
    d.mass[20] = 0.0;
    d.mass[30] = 0.0;
    d.normalize();

    CHECK(d.p(10) == doctest::Approx(1.0/3.0).epsilon(1e-9));
    CHECK(d.p(20) == doctest::Approx(1.0/3.0).epsilon(1e-9));
    CHECK(d.p(30) == doctest::Approx(1.0/3.0).epsilon(1e-9));
}

TEST_CASE("Distribution: entropy is 0 for degenerate") {
    Distribution d;
    d.mass[5] = 1.0;
    d.normalize();
    CHECK(d.entropy() == doctest::Approx(0.0).epsilon(1e-9));
}

TEST_CASE("Distribution: entropy is log2(n) for uniform over n") {
    Distribution d;
    d.mass[1] = 0.25;
    d.mass[2] = 0.25;
    d.mass[3] = 0.25;
    d.mass[4] = 0.25;
    CHECK(d.entropy() == doctest::Approx(2.0).epsilon(1e-9));
}

TEST_CASE("Distribution: map returns highest-mass hypothesis") {
    Distribution d;
    d.mass[1] = 0.1;
    d.mass[2] = 0.7;
    d.mass[3] = 0.2;
    CHECK(d.map() == 2);
}

TEST_CASE("Distribution: sample is deterministic at boundaries") {
    Distribution d;
    d.mass[1] = 0.5;
    d.mass[2] = 0.5;
    d.normalize();

    const auto s1 = d.sample(0.0);
    const auto s2 = d.sample(0.99);
    CHECK((s1 == 1 || s1 == 2));
    CHECK((s2 == 1 || s2 == 2));
}

TEST_CASE("Distribution: p returns 0 for unknown hypothesis") {
    Distribution d;
    d.mass[1] = 1.0;
    CHECK(d.p(999) == 0.0);
}

TEST_CASE("Distribution: support counts nonzero entries") {
    Distribution d;
    d.mass[1] = 0.5;
    d.mass[2] = 0.5;
    d.mass[3] = 0.0;
    CHECK(d.support() == 2);
}

TEST_CASE("WeightVector: lerp interpolates correctly") {
    WeightVector a, b;
    a.contextFrameMatch = 0.0;
    b.contextFrameMatch = 1.0;
    const auto mid = a.lerp(b, 0.5);
    CHECK(mid.contextFrameMatch == doctest::Approx(0.5).epsilon(1e-9));
}

TEST_CASE("WeightVector: hadamard product") {
    WeightVector a, b;
    a.contextFrameMatch = 2.0;
    b.contextFrameMatch = 3.0;
    const auto h = a.hadamard(b);
    CHECK(h.contextFrameMatch == doctest::Approx(6.0).epsilon(1e-9));
}
