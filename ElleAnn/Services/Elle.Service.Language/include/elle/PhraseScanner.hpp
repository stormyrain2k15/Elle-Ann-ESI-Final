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
    std::size_t                endLexemeIdx   = 0;
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

    ISqlAccessLayer& m_db;
    LruCache<std::string, std::vector<PhraseRecord>> m_phraseCache;
};

}
