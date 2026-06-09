#include <doctest/doctest.h>

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/BeliefPersistence.hpp"

#include <memory>

using namespace elle::prob;

namespace {

Distribution makeDist(std::initializer_list<std::pair<std::int64_t, double>> rows) {
    Distribution d;
    for (auto& [h, m] : rows) d.mass[h] = m;
    d.normalize();
    return d;
}

Evidence makeEvidence(std::int64_t hyp, double lr, EvidenceKind kind = EvidenceKind::LEXICAL_MATCH) {
    Evidence e{};
    e.kind             = kind;
    e.hypothesisId     = hyp;
    e.likelihoodRatio  = lr;
    e.sourceWeight     = 1.0;
    e.reason           = "test";
    return e;
}

}

TEST_CASE("attachPersistence + registerBelief writes the domain to the backend") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    BeliefStore store(1);
    store.attachPersistence(backend);

    store.registerBelief("d1", makeDist({{1, 0.5}, {2, 0.5}}), 60.0);

    auto rows = backend->loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].domain == "d1");
    CHECK(rows[0].halfLifeSecs == doctest::Approx(60.0));
}

TEST_CASE("submitSync persists posterior + evidence + audit row") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    BeliefStore store(1);
    store.attachPersistence(backend);

    store.registerBelief("d1", makeDist({{1, 0.5}, {2, 0.5}}), 0.0);

    std::vector<Evidence> ev = { makeEvidence(1, 4.0) };
    store.submitSync("d1", ev);

    auto evList = backend->evidenceFor("d1");
    CHECK(evList.size() == 1);
    CHECK(evList[0].hypothesisId == 1);

    auto audit = backend->auditTrail();
    REQUIRE(!audit.empty());
    bool foundUpdate = false;
    for (const auto& a : audit) {
        if (a.domain == "d1" && a.operation == "update" && a.evidenceCount == 1) {
            foundUpdate = true;
            CHECK(a.mapHypothesisId == 1);
        }
    }
    CHECK(foundUpdate);

    auto rows = backend->loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].posterior.p(1) > 0.6);
}

TEST_CASE("applyDecayAll persists a decay audit row per domain") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    BeliefStore store(1);
    store.attachPersistence(backend);

    store.registerBelief("a", makeDist({{1, 0.95}, {2, 0.05}}), 0.001);
    store.registerBelief("b", makeDist({{1, 0.50}, {2, 0.50}}), 0.001);

    store.applyDecayAll();

    auto audit = backend->auditTrail();
    int decayCount = 0;
    for (const auto& a : audit) {
        if (a.operation == "decay") ++decayCount;
    }
    CHECK(decayCount >= 2);
}

TEST_CASE("loadFromPersistence rehydrates beliefs from the backend") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    backend->upsertDomain("d1", makeDist({{1, 0.5}, {2, 0.5}}), 30.0);
    backend->replacePosterior("d1", makeDist({{1, 0.9}, {2, 0.1}}), 1700000000000LL);
    backend->upsertDomain("d2", makeDist({{7, 1.0}}), 0.0);

    BeliefStore store(1);
    store.attachPersistence(backend);
    auto restored = store.loadFromPersistence();
    CHECK(restored == 2);

    auto b1 = store.getBelief("d1");
    REQUIRE(b1.has_value());
    CHECK(b1->posterior.p(1) == doctest::Approx(0.9));
    CHECK(b1->halfLifeSecs == doctest::Approx(30.0));

    auto b2 = store.getBelief("d2");
    REQUIRE(b2.has_value());
    CHECK(b2->posterior.p(7) == doctest::Approx(1.0));
}

TEST_CASE("attachPersistence is idempotent — re-attach replaces the backend") {
    auto first  = std::make_shared<InMemoryBeliefPersistence>();
    auto second = std::make_shared<InMemoryBeliefPersistence>();
    BeliefStore store(1);
    store.attachPersistence(first);
    store.registerBelief("d1", makeDist({{1, 1.0}}), 0.0);

    store.attachPersistence(second);
    store.registerBelief("d2", makeDist({{1, 1.0}}), 0.0);

    CHECK(first->domainCount()  == 1);
    CHECK(second->domainCount() == 1);
    CHECK(first->loadAll().at(0).domain  == "d1");
    CHECK(second->loadAll().at(0).domain == "d2");
}

TEST_CASE("BeliefStore without persistence still works (no-op backend path)") {
    BeliefStore store(1);

    store.registerBelief("d1", makeDist({{1, 0.5}, {2, 0.5}}), 0.0);
    std::vector<Evidence> ev = { makeEvidence(1, 3.0) };
    store.submitSync("d1", ev);

    auto b = store.getBelief("d1");
    REQUIRE(b.has_value());
    CHECK(b->posterior.p(1) > 0.5);
}
