#include <doctest/doctest.h>

#include "elle/prob/JsonlBeliefPersistence.hpp"
#include "elle/prob/BeliefStore.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

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
        ("elle_jsonl_" + tag + "_" + std::to_string(::getpid()) + ".jsonl");
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

TEST_CASE("JsonlBeliefPersistence writes one line per upsertDomain") {
    auto path = tempPath("upsert");
    JsonlBeliefPersistence p(path);
    p.upsertDomain("speaker_intent", makeDist({{1, 0.5}, {2, 0.5}}), 60.0);

    auto lines = readLines(path);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].find("\"op\":\"upsertDomain\"") != std::string::npos);
    CHECK(lines[0].find("\"domain\":\"speaker_intent\"") != std::string::npos);
    CHECK(lines[0].find("\"half_life_secs\":60") != std::string::npos);
}

TEST_CASE("JsonlBeliefPersistence accumulates lines across calls") {
    auto path = tempPath("accumulate");
    JsonlBeliefPersistence p(path);
    p.upsertDomain("d1", makeDist({{1, 1.0}}), 0.0);
    p.replacePosterior("d1", makeDist({{1, 0.9}, {2, 0.1}}), 1700000000000LL);

    Evidence e{};
    e.kind             = EvidenceKind::LEXICAL_MATCH;
    e.hypothesisId     = 1;
    e.likelihoodRatio  = 2.0;
    e.sourceWeight     = 1.0;
    e.reason           = "smoke";
    p.appendEvidence("d1", e);

    p.auditUpdate("d1", "update", 1, 0.69, 0.32, 1, 0.9, "after-evidence");

    auto lines = readLines(path);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].find("\"op\":\"upsertDomain\"")     != std::string::npos);
    CHECK(lines[1].find("\"op\":\"replacePosterior\"") != std::string::npos);
    CHECK(lines[1].find("\"ts_ms\":1700000000000")     != std::string::npos);
    CHECK(lines[2].find("\"op\":\"appendEvidence\"")   != std::string::npos);
    CHECK(lines[2].find("\"reason\":\"smoke\"")        != std::string::npos);
    CHECK(lines[3].find("\"op\":\"audit\"")            != std::string::npos);
    CHECK(lines[3].find("\"map_probability\":0.9")     != std::string::npos);
}

TEST_CASE("JsonlBeliefPersistence escapes special characters in strings") {
    auto path = tempPath("escape");
    JsonlBeliefPersistence p(path);
    p.upsertDomain("name\"with\\specials\nand\ttab", makeDist({{1, 1.0}}), 0.0);

    auto lines = readLines(path);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].find("\\\"") != std::string::npos);
    CHECK(lines[0].find("\\\\") != std::string::npos);
    CHECK(lines[0].find("\\n") != std::string::npos);
    CHECK(lines[0].find("\\t") != std::string::npos);
}

TEST_CASE("JsonlBeliefPersistence::loadAll returns empty by design") {
    auto path = tempPath("loadall");
    JsonlBeliefPersistence p(path);
    p.upsertDomain("d", makeDist({{1, 1.0}}), 0.0);
    auto rows = p.loadAll();
    CHECK(rows.empty());
}

TEST_CASE("JsonlBeliefPersistence integrates with BeliefStore::attachPersistence") {
    auto path = tempPath("integration");
    auto backend = makeJsonlBeliefPersistence(path);
    BeliefStore store(1);
    store.attachPersistence(backend);

    store.registerBelief("d1", makeDist({{1, 0.5}, {2, 0.5}}), 0.0);

    Evidence e{};
    e.kind             = EvidenceKind::LEXICAL_MATCH;
    e.hypothesisId     = 1;
    e.likelihoodRatio  = 3.0;
    e.sourceWeight     = 1.0;
    e.reason           = "via-store";
    store.submitSync("d1", { e });

    auto lines = readLines(path);
    REQUIRE(lines.size() >= 4);

    int op_upsert = 0, op_evidence = 0, op_audit = 0;
    for (auto& l : lines) {
        if (l.find("\"op\":\"upsertDomain\"")    != std::string::npos) ++op_upsert;
        if (l.find("\"op\":\"appendEvidence\"")  != std::string::npos) ++op_evidence;
        if (l.find("\"op\":\"audit\"")           != std::string::npos) ++op_audit;
    }
    CHECK(op_upsert  >= 1);
    CHECK(op_evidence >= 1);
    CHECK(op_audit    >= 1);
}
