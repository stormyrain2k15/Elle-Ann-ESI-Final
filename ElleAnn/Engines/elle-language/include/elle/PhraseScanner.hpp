// ============================================================================
// Elle Engine -- Phrase Scanner
// File: include/elle/PhraseScanner.hpp
//
// Longest-match-first phrase scanning. Operates over the lexeme stream and
// returns spans claimed by a known PhraseID + its candidate PhraseSenseIDs.
// ============================================================================
#pragma once

#include "elle/Cache.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <cstddef>
#include <vector>

namespace elle {

struct PhraseMatch {
    PhraseID                   phraseId;
    std::size_t                startLexemeIdx = 0;
    std::size_t                endLexemeIdx   = 0;   // exclusive
    std::vector<PhraseSenseID> phraseSenseCandidates;
    std::string                normalizedForm;
};

class PhraseScanner {
public:
    explicit PhraseScanner(ISqlAccessLayer& db, std::size_t cacheSize = 4096);

    std::vector<PhraseMatch> scan(const std::vector<Lexeme>& lexemes,
                                  DebugTrace& trace);

    void clearCache() { m_phraseCache.clear(); }

private:
    // Caches the FULL phrase set keyed by "first word's normalized form".
    // A miss falls through to a single DB call.
    ISqlAccessLayer& m_db;
    LruCache<std::string, std::vector<PhraseRecord>> m_phraseCache;
};

} // namespace elle
