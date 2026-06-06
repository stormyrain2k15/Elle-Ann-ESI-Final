// ============================================================================
// Elle Engine -- Context Comparator
// File: include/elle/ContextComparator.hpp
// ============================================================================
#pragma once

#include "elle/Cache.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <vector>

namespace elle {

class ContextComparator {
public:
    explicit ContextComparator(ISqlAccessLayer& db,
                               std::size_t cacheSize = 1024);

    // Returns context-frame matches ranked by score, with contribution map.
    // Caller-supplied conversation-level hints are folded into scoring.
    std::vector<ContextFrameMatch>
    compare(const IntegerSequence& seq,
            const ConversationContext& convo,
            DebugTrace& trace);

    void clearCache() { m_frameCache.clear(); m_kwCache.clear(); }

private:
    ISqlAccessLayer& m_db;
    LruCache<std::int64_t, std::vector<ContextFrameRecord>>     m_frameCache; // 1 -> all
    LruCache<std::int64_t, std::vector<ContextKeywordRecord>>   m_kwCache;
};

} // namespace elle
