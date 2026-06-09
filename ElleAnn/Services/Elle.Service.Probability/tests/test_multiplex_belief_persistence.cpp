#include <doctest/doctest.h>

#include "elle/prob/MultiplexBeliefPersistence.hpp"
#include "elle/prob/BeliefPersistence.hpp"
#include "elle/prob/JsonlBeliefPersistence.hpp"
#include "elle/prob/BeliefStore.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace elle::prob;
namespace fs = std::filesystem;

namespace {

Distribution makeDist(std::initializer_list<std::pair<std::int64_t, double>> rows) {
    Distribution d;
    for (auto& [h, m] : rows) d.mass[h] = m;
    d.normalize();
    return d;
}

std::string tempPath(const std::string& tag) {
    auto p = fs::temp_directory_path() /
        ("elle_mplex_" + tag + "_" + std::to_string(::getpid()) + ".jsonl");
    std::error_code ec;
    fs::remove(p, ec);
    return p.string();
}

std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> out;
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) out.push_back(line);
    return out;
}

}

TEST_CASE("MultiplexBeliefPersistence: empty multiplex is a safe no-op") {
    MultiplexBeliefPersistence m;
    CHECK(m.backendCount() == 0);
    m.upsertDomain("d", makeDist({{1, 1.0}}), 0.0);
    m.replacePosterior("d", makeDist({{1, 1.0}}), 1);
    Evidence ev{};
    ev.kind = EvidenceKind::LEXICAL_MATCH;
    ev.hypothesisId = 1;
    ev.likelihoodRatio = 1.0;
    ev.sourceWeight = 1.0;
    m.appendEvidence("d", ev);
    m.auditUpdate("d", "op", 1, 0.5, 0.4, 1, 0.9, "x");
    CHECK(m.loadAll().empty());
}

TEST_CASE("MultiplexBeliefPersistence: fans out every mutation to all backends") {
    auto mem  = std::make_shared<InMemoryBeliefPersistence>();
    auto path = tempPath("fanout");
    auto jsl  = std::make_shared<JsonlBeliefPersistence>(path);

    MultiplexBeliefPersistence mux({ mem, jsl });
    REQUIRE(mux.backendCount() == 2);
    CHECK(mux.primaryBackend() == mem);

    mux.upsertDomain("speaker_intent", makeDist({{1, 0.5}, {2, 0.5}}), 60.0);
    mux.replacePosterior("speaker_intent", makeDist({{1, 0.9}, {2, 0.1}}),
                         1700000000000LL);

    Evidence ev{};
    ev.kind            = EvidenceKind::LEXICAL_MATCH;
    ev.hypothesisId    = 1;
    ev.likelihoodRatio = 2.0;
    ev.sourceWeight    = 1.0;
    ev.reason          = "mux-test";
    mux.appendEvidence("speaker_intent", ev);
    mux.auditUpdate("speaker_intent", "update", 1, 0.69, 0.32, 1, 0.9,
                    "after-multiplex");

    auto rows = mem->loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].domain == "speaker_intent");
    CHECK(rows[0].lastUpdatedMs == 1700000000000LL);
    CHECK(mem->evidenceFor("speaker_intent").size() == 1);
    CHECK(mem->auditTrail().size() == 1);

    auto lines = readLines(path);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].find("\"op\":\"upsertDomain\"")     != std::string::npos);
    CHECK(lines[1].find("\"op\":\"replacePosterior\"") != std::string::npos);
    CHECK(lines[1].find("\"ts_ms\":1700000000000")     != std::string::npos);
    CHECK(lines[2].find("\"op\":\"appendEvidence\"")   != std::string::npos);
    CHECK(lines[2].find("\"reason\":\"mux-test\"")     != std::string::npos);
    CHECK(lines[3].find("\"op\":\"audit\"")            != std::string::npos);
}

TEST_CASE("MultiplexBeliefPersistence: loadAll() returns first non-empty backend") {
    auto empty = std::make_shared<JsonlBeliefPersistence>(tempPath("empty"));
    auto mem   = std::make_shared<InMemoryBeliefPersistence>();
    mem->upsertDomain("d", makeDist({{1, 1.0}}), 0.0);

    MultiplexBeliefPersistence mux({ empty, mem });
    auto rows = mux.loadAll();
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].domain == "d");
}

TEST_CASE("MultiplexBeliefPersistence: addBackend appends + ignores nullptrs") {
    MultiplexBeliefPersistence mux;
    mux.addBackend(nullptr);
    CHECK(mux.backendCount() == 0);
    auto mem = std::make_shared<InMemoryBeliefPersistence>();
    mux.addBackend(mem);
    CHECK(mux.backendCount() == 1);
    mux.upsertDomain("d", makeDist({{1, 1.0}}), 0.0);
    CHECK(mem->loadAll().size() == 1);
}

TEST_CASE("MultiplexBeliefPersistence integrates with BeliefStore::attachPersistence") {
    auto mem  = std::make_shared<InMemoryBeliefPersistence>();
    auto path = tempPath("integration");
    auto jsl  = std::make_shared<JsonlBeliefPersistence>(path);
    auto mux  = makeMultiplexBeliefPersistence({ mem, jsl });

    BeliefStore store(1);
    store.attachPersistence(mux);

    store.registerBelief("policy_choice", makeDist({{1, 0.5}, {2, 0.5}}), 60.0);

    Evidence e{};
    e.kind             = EvidenceKind::LEXICAL_MATCH;
    e.hypothesisId     = 1;
    e.likelihoodRatio  = 3.0;
    e.sourceWeight     = 1.0;
    e.reason           = "mux-via-store";
    store.submitSync("policy_choice", { e });

    CHECK(mem->loadAll().size() == 1);
    CHECK(mem->evidenceFor("policy_choice").size() == 1);
    CHECK(mem->auditTrail().size() >= 1);

    auto lines = readLines(path);
    REQUIRE(lines.size() >= 3);

    int op_upsert = 0, op_evidence = 0, op_audit = 0;
    for (auto& l : lines) {
        if (l.find("\"op\":\"upsertDomain\"")    != std::string::npos) ++op_upsert;
        if (l.find("\"op\":\"appendEvidence\"")  != std::string::npos) ++op_evidence;
        if (l.find("\"op\":\"audit\"")           != std::string::npos) ++op_audit;
    }
    CHECK(op_upsert   >= 1);
    CHECK(op_evidence >= 1);
    CHECK(op_audit    >= 1);
}
