// ============================================================================
// Elle Engine -- Semantic graph traversal
// File: tests/test_semantic_graph.cpp
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("graph walker produces ConceptPaths for 'I'm fine'") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");
    REQUIRE_FALSE(r.meaning.conceptPaths.empty());

    bool sawAcceptable = false;
    for (const auto& p : r.meaning.conceptPaths) {
        for (auto n : p.nodes) {
            if (n.value() == 1 /*state.acceptable*/) sawAcceptable = true;
        }
    }
    CHECK(sawAcceptable);
}

TEST_CASE("traversal respects max depth bound") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    auto cfg = EngineConfig::defaults();
    cfg.graph.maxDepth = 1;
    cfg.graph.maxNodesPerPath = 3;
    Engine engine(db, cfg);

    auto r = engine.analyze("I'm fine.");
    for (const auto& p : r.meaning.conceptPaths) {
        CHECK(p.nodes.size() <= 3u);
    }
}

TEST_CASE("ConceptPath nodes/edges are size-consistent") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("The bat flew at dusk.");
    for (const auto& p : r.meaning.conceptPaths) {
        if (!p.nodes.empty()) {
            // edges connect nodes; in our BFS path each non-root node adds one edge.
            CHECK(p.edges.size() == p.nodes.size() - 1);
        }
    }
}
