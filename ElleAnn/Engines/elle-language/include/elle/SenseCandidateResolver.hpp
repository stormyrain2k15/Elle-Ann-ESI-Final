// ============================================================================
// Elle Engine -- Sense Candidate Resolver
// File: include/elle/SenseCandidateResolver.hpp
//
// Produces a ranked list of ScoredSense per WordUnit. Picks the winner.
// The score is a transparent weighted sum -- never a black box. Each
// contribution is recorded in ScoredSense::scoreBreakdown.
// ============================================================================
#pragma once

#include "elle/Cache.hpp"
#include "elle/Config.hpp"
#include "elle/ContextComparator.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <vector>

namespace elle {

class SenseCandidateResolver {
public:
    SenseCandidateResolver(ISqlAccessLayer& db,
                           const ScoringWeights& weights,
                           std::size_t senseCacheSize = 50000);

    // Score every WordUnit's candidate (phrase-)senses and choose one each.
    std::vector<ResolvedSense>
    resolve(const IntegerSequence& seq,
            const std::vector<ContextFrameMatch>& contextMatches,
            const ConversationContext& convo,
            DebugTrace& trace);

    void clearCache() {
        m_senseCache.clear();
        m_phraseSenseCache.clear();
        m_senseEmotionCache.clear();
        m_phraseSenseEmotionCache.clear();
    }

private:
    double scoreSense(const SenseRecord& sense,
                      const WordUnit& unit,
                      const IntegerSequence& seq,
                      const std::vector<ContextFrameMatch>& matches,
                      const ConversationContext& convo,
                      ScoredSense& scored);

    double scorePhraseSense(const PhraseSenseRecord& sense,
                            const WordUnit& unit,
                            const IntegerSequence& seq,
                            const std::vector<ContextFrameMatch>& matches,
                            const ConversationContext& convo,
                            ScoredSense& scored);

    ISqlAccessLayer&  m_db;
    ScoringWeights    m_weights;
    LruCache<std::int64_t, std::vector<SenseRecord>>          m_senseCache;
    LruCache<std::int64_t, std::vector<PhraseSenseRecord>>    m_phraseSenseCache;
    LruCache<std::int64_t, std::vector<EmotionWeight>>        m_senseEmotionCache;
    LruCache<std::int64_t, std::vector<EmotionWeight>>        m_phraseSenseEmotionCache;
};

} // namespace elle
