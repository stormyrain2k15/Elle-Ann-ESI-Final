// ============================================================================
// Elle Engine -- Emotional Weight Processor
// File: include/elle/EmotionalWeightProcessor.hpp
// ============================================================================
#pragma once

#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <vector>

namespace elle {

class EmotionalWeightProcessor {
public:
    explicit EmotionalWeightProcessor(ISqlAccessLayer& db);

    EmotionalProfile aggregate(const std::vector<ResolvedSense>& resolved,
                               const IntegerSequence& seq,
                               DebugTrace& trace);

private:
    ISqlAccessLayer& m_db;
};

} // namespace elle
