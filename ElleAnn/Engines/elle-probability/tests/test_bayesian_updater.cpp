// ============================================================================
// Elle Probability Engine -- BayesianUpdater tests
// File: tests/test_bayesian_updater.cpp
// ============================================================================
#include <doctest/doctest.h>
#include "elle/prob/BayesianUpdater.hpp"

#include <cmath>

using namespace elle::prob;

// Build a simple uniform belief over hypotheses {1,2,3}.
static Belief makeUniformBelief() {
    Belief b;
    b.domain = "test:uniform";
    b.prior  = BayesianUpdater::uniformPrior({1, 2, 3});
    b.posterior = b.prior;
    b.lastUpdated = now();
    return b;
}

TEST_CASE("BayesianUpdater: strong evidence concentrates posterior") {
    BayesianUpdater u;
    Belief b = makeUniformBelief();

    // Submit 5 pieces of strong evidence for hypothesis 2.
    for (int i = 0; i < 5; ++i) {
        Evidence ev;
        ev.kind            = EvidenceKind::LEXICAL_MATCH;
        ev.hypothesisId    = 2;
        ev.likelihoodRatio = 5.0;
        ev.sourceWeight    = 1.0;
        ev.observedAt      = now();
        u.update(b, ev);
    }

    // Hypothesis 2 should dominate.
    CHECK(b.posterior.p(2) > 0.9);
    CHECK(b.posterior.map() == 2);
}

TEST_CASE("BayesianUpdater: contradicting evidence flattens distribution") {
    BayesianUpdater u;
    Belief b = makeUniformBelief();

    // Push toward 1, then push back with evidence for 3.
    for (int i = 0; i < 3; ++i) {
        Evidence ev;
        ev.kind = EvidenceKind::LEXICAL_MATCH;
        ev.hypothesisId    = 1;
        ev.likelihoodRatio = 4.0;
        ev.sourceWeight    = 1.0;
        ev.observedAt      = now();
        u.update(b, ev);
    }
    for (int i = 0; i < 3; ++i) {
        Evidence ev;
        ev.kind = EvidenceKind::LEXICAL_MATCH;
        ev.hypothesisId    = 3;
        ev.likelihoodRatio = 4.0;
        ev.sourceWeight    = 1.0;
        ev.observedAt      = now();
        u.update(b, ev);
    }

    // Neither 1 nor 3 should be certain; entropy should be higher than degenerate.
    CHECK(b.posterior.entropy() > 0.5);
}

TEST_CASE("BayesianUpdater: low LR evidence barely moves posterior") {
    BayesianUpdater u;
    Belief b = makeUniformBelief();
    const double p1_before = b.posterior.p(1);

    Evidence ev;
    ev.hypothesisId    = 1;
    ev.likelihoodRatio = 1.01;
    ev.sourceWeight    = 1.0;
    ev.observedAt      = now();
    u.update(b, ev);

    // Posterior should be close to uniform still.
    CHECK(std::abs(b.posterior.p(1) - p1_before) < 0.05);
}

TEST_CASE("BayesianUpdater: reset restores prior") {
    BayesianUpdater u;
    Belief b = makeUniformBelief();

    Evidence ev;
    ev.hypothesisId = 2; ev.likelihoodRatio = 10.0; ev.sourceWeight = 1.0;
    ev.observedAt = now();
    u.update(b, ev);
    CHECK(b.posterior.p(2) > 0.8);

    BayesianUpdater::reset(b);
    CHECK(b.posterior.p(1) == doctest::Approx(1.0/3.0).epsilon(1e-6));
    CHECK(b.evidenceLog.empty());
}

TEST_CASE("BayesianUpdater: KL divergence is 0 for identical distributions") {
    Distribution p;
    p.mass[1] = 0.5; p.mass[2] = 0.5;
    CHECK(BayesianUpdater::kl(p, p) == doctest::Approx(0.0).epsilon(1e-9));
}

TEST_CASE("BayesianUpdater: KL divergence is non-symmetric") {
    Distribution p, q;
    p.mass[1] = 0.9; p.mass[2] = 0.1;
    q.mass[1] = 0.5; q.mass[2] = 0.5;
    const double kl_pq = BayesianUpdater::kl(p, q);
    const double kl_qp = BayesianUpdater::kl(q, p);
    CHECK(kl_pq != doctest::Approx(kl_qp).epsilon(1e-6));
}

TEST_CASE("BayesianUpdater: empirical prior sums to 1") {
    const auto d = BayesianUpdater::empiricalPrior({{1, 10.0}, {2, 20.0}, {3, 70.0}});
    CHECK(d.p(3) == doctest::Approx(0.7).epsilon(1e-9));
    double total = 0.0;
    for (const auto& [k, v] : d.mass) total += v;
    CHECK(total == doctest::Approx(1.0).epsilon(1e-9));
}

TEST_CASE("BayesianUpdater: evidence log accumulates") {
    BayesianUpdater u;
    Belief b = makeUniformBelief();
    for (int i = 0; i < 5; ++i) {
        Evidence ev;
        ev.hypothesisId = 1; ev.likelihoodRatio = 1.5; ev.sourceWeight = 1.0;
        ev.observedAt = now();
        u.update(b, ev);
    }
    CHECK(b.evidenceLog.size() == 5);
}
