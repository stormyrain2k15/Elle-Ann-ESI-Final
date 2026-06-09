#include <doctest/doctest.h>

#include "elle/prob/BeliefPersistence.hpp"

#include <chrono>

using namespace elle::prob;

namespace {

Distribution makeDist(std::initializer_list<std::pair<std::int64_t, double>> rows) {
    Distribution d;
    for (auto& [h, m] : rows) d.mass[h] = m;
    d.normalize();
    return d;
}

std::int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

}

TEST_CASE("InMemoryBeliefPersistence: upsertDomain seeds prior+posterior on first call") {
    InMemoryBeliefPersistence p;
    auto prior = makeDist({{1, 0.5}, {2, 0.5}});
    p.upsertDomain("speaker_intent", prior, 60.0);

    auto rows = p.loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].domain == "speaker_intent");
    CHECK(rows[0].halfLifeSecs == doctest::Approx(60.0));
    CHECK(rows[0].prior.p(1) == doctest::Approx(0.5));
    CHECK(rows[0].posterior.p(1) == doctest::Approx(0.5));
}

TEST_CASE("InMemoryBeliefPersistence: replacePosterior updates without touching prior") {
    InMemoryBeliefPersistence p;
    p.upsertDomain("d1", makeDist({{1, 0.5}, {2, 0.5}}), 0.0);
    p.replacePosterior("d1", makeDist({{1, 0.9}, {2, 0.1}}), nowMs());

    auto rows = p.loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].posterior.p(1) == doctest::Approx(0.9));
    CHECK(rows[0].prior.p(1) == doctest::Approx(0.5));
}

TEST_CASE("InMemoryBeliefPersistence: replacePosterior on unknown domain creates a row") {
    InMemoryBeliefPersistence p;
    p.replacePosterior("brand_new", makeDist({{7, 1.0}}), nowMs());

    auto rows = p.loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].domain == "brand_new");
    CHECK(rows[0].posterior.p(7) == doctest::Approx(1.0));
}

TEST_CASE("InMemoryBeliefPersistence: appendEvidence accumulates per-domain") {
    InMemoryBeliefPersistence p;
    p.upsertDomain("d1", makeDist({{1, 1.0}}), 0.0);

    Evidence e1{};
    e1.kind = EvidenceKind::LEXICAL_MATCH;
    e1.hypothesisId = 1;
    e1.likelihoodRatio = 2.0;
    e1.sourceWeight = 1.0;
    e1.reason = "a";
    p.appendEvidence("d1", e1);

    Evidence e2{};
    e2.kind = EvidenceKind::CONVERSATION_TURN;
    e2.hypothesisId = 1;
    e2.likelihoodRatio = 1.5;
    e2.sourceWeight = 0.8;
    e2.reason = "b";
    p.appendEvidence("d1", e2);

    auto ev = p.evidenceFor("d1");
    REQUIRE(ev.size() == 2);
    CHECK(ev[0].reason == "a");
    CHECK(ev[1].reason == "b");
}

TEST_CASE("InMemoryBeliefPersistence: auditUpdate appends to trail") {
    InMemoryBeliefPersistence p;
    p.upsertDomain("d1", makeDist({{1, 1.0}}), 0.0);
    p.auditUpdate("d1", "update", 3, 0.69, 0.23, 1, 0.95, "stable");
    p.auditUpdate("d1", "decay",  0, 0.23, 0.40, 1, 0.70, "drift");

    auto trail = p.auditTrail();
    REQUIRE(trail.size() == 2);
    CHECK(trail[0].operation == "update");
    CHECK(trail[0].evidenceCount == 3);
    CHECK(trail[0].mapProbability == doctest::Approx(0.95));
    CHECK(trail[1].operation == "decay");
    CHECK(trail[1].entropyAfter == doctest::Approx(0.40));
}

TEST_CASE("InMemoryBeliefPersistence: loadAll returns every registered domain") {
    InMemoryBeliefPersistence p;
    p.upsertDomain("a", makeDist({{1, 1.0}}), 0.0);
    p.upsertDomain("b", makeDist({{1, 1.0}}), 30.0);
    p.upsertDomain("c", makeDist({{1, 1.0}}), 90.0);

    auto rows = p.loadAll();
    CHECK(rows.size() == 3);
    CHECK(p.domainCount() == 3);
}

TEST_CASE("InMemoryBeliefPersistence: re-upsert preserves posterior") {
    InMemoryBeliefPersistence p;
    auto initialPrior = makeDist({{1, 0.5}, {2, 0.5}});
    p.upsertDomain("d", initialPrior, 0.0);
    p.replacePosterior("d", makeDist({{1, 0.95}, {2, 0.05}}), nowMs());

    auto newPrior = makeDist({{1, 0.3}, {2, 0.7}});
    p.upsertDomain("d", newPrior, 120.0);

    auto rows = p.loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].halfLifeSecs == doctest::Approx(120.0));
    CHECK(rows[0].prior.p(1) == doctest::Approx(0.3));
    CHECK(rows[0].posterior.p(1) == doctest::Approx(0.95));
}

TEST_CASE("makeInMemoryBeliefPersistence returns a usable shared_ptr") {
    auto p = makeInMemoryBeliefPersistence();
    REQUIRE(static_cast<bool>(p));
    p->upsertDomain("d", makeDist({{1, 1.0}}), 0.0);
    auto rows = p->loadAll();
    CHECK(rows.size() == 1);
}
