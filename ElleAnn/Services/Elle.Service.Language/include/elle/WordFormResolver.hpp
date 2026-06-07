#pragma once

#include "elle/Cache.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace elle {

struct ResolvedWord {
    std::optional<WordID>     wordId;
    std::optional<WordFormID> wordFormId;
    bool                      isUnknown = false;
    std::string               surface;
};

class WordFormResolver {
public:
    explicit WordFormResolver(ISqlAccessLayer& db,
                              std::size_t wordCacheSize = 50000,
                              std::size_t formCacheSize = 50000);

    ResolvedWord resolve(const std::string& normalizedSurface, DebugTrace& trace);

    void clearCache();

private:
    ISqlAccessLayer& m_db;
    LruCache<std::string, WordRecord>     m_wordCache;
    LruCache<std::string, WordFormRecord> m_formCache;
};

}
