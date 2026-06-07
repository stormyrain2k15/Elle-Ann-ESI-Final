#pragma once

#include "elle/Config.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <vector>

namespace elle {

class SemanticGraphWalker {
public:
    SemanticGraphWalker(ISqlAccessLayer& db, const GraphConfig& cfg);

    std::vector<ConceptPath>
    walk(const std::vector<ResolvedSense>& resolved, DebugTrace& trace);

private:
    ISqlAccessLayer& m_db;
    GraphConfig      m_cfg;
};

}
