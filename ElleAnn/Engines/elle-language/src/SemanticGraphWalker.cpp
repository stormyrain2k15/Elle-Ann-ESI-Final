// ============================================================================
// Elle Engine -- SemanticGraphWalker implementation
// File: src/SemanticGraphWalker.cpp
//
// Bounded BFS. Starts from every chosen (phrase-)sense's concept nodes,
// expands typed edges whose strength meets minEdgeStrength, never exceeds
// maxDepth, maxNodesPerPath, or maxTotalNodes.
// ============================================================================
#include "elle/SemanticGraphWalker.hpp"

#include <nlohmann/json.hpp>

#include <queue>
#include <unordered_set>

namespace elle {

SemanticGraphWalker::SemanticGraphWalker(ISqlAccessLayer& db, const GraphConfig& cfg)
    : m_db(db), m_cfg(cfg) {}

std::vector<ConceptPath>
SemanticGraphWalker::walk(const std::vector<ResolvedSense>& resolved, DebugTrace& trace) {
    std::vector<ConceptPath> out;
    std::unordered_set<std::int64_t> visitedGlobal;

    // Seed nodes: every concept that contains a chosen sense.
    std::vector<SemanticNodeID> seeds;
    for (const auto& r : resolved) {
        std::vector<ConceptMemberRecord> cms;
        if (r.chosenSenseId)        cms = m_db.getConceptsForSense(*r.chosenSenseId);
        else if (r.chosenPhraseSenseId) cms = m_db.getConceptsForPhraseSense(*r.chosenPhraseSenseId);
        for (const auto& cm : cms) {
            for (auto n : m_db.getNodesForConcept(cm.conceptId)) seeds.push_back(n);
        }
    }

    for (auto seed : seeds) {
        if (visitedGlobal.count(seed.value())) continue;
        if (static_cast<int>(visitedGlobal.size()) >= m_cfg.maxTotalNodes) break;

        ConceptPath path;
        path.nodes.push_back(seed);
        path.totalStrength = 1.0;
        std::unordered_set<std::int64_t> visitedLocal{seed.value()};
        visitedGlobal.insert(seed.value());

        std::queue<std::pair<SemanticNodeID, int>> q;
        q.push({seed, 0});

        while (!q.empty() &&
               static_cast<int>(path.nodes.size()) < m_cfg.maxNodesPerPath) {
            auto [node, depth] = q.front(); q.pop();
            if (depth >= m_cfg.maxDepth) continue;

            for (const auto& edge : m_db.getEdgesFromNode(node)) {
                if (edge.strength < m_cfg.minEdgeStrength) continue;
                const auto next = edge.toNode.value();
                if (visitedLocal.count(next)) continue;
                visitedLocal.insert(next);
                visitedGlobal.insert(next);
                path.nodes.push_back(edge.toNode);
                path.edges.push_back(edge);
                path.totalStrength += edge.strength * edge.confidence;
                q.push({edge.toNode, depth + 1});
                if (static_cast<int>(path.nodes.size()) >= m_cfg.maxNodesPerPath) break;
                if (static_cast<int>(visitedGlobal.size()) >= m_cfg.maxTotalNodes) break;
            }
        }
        out.push_back(std::move(path));
    }

    nlohmann::json payload = {
        {"path_count",      out.size()},
        {"nodes_visited",   visitedGlobal.size()},
        {"max_depth",       m_cfg.maxDepth},
        {"max_per_path",    m_cfg.maxNodesPerPath}
    };
    trace.logJson("SemanticGraphWalker", "graph_walked",
                  "Bounded BFS over semantic graph completed.", std::move(payload));
    return out;
}

} // namespace elle
