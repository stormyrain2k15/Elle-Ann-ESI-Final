#include "elle/WordFormResolver.hpp"

#include <nlohmann/json.hpp>

namespace elle {

WordFormResolver::WordFormResolver(ISqlAccessLayer& db,
                                   std::size_t wordCacheSize,
                                   std::size_t formCacheSize)
    : m_db(db), m_wordCache(wordCacheSize), m_formCache(formCacheSize) {}

void WordFormResolver::clearCache() {
    m_wordCache.clear();
    m_formCache.clear();
}

ResolvedWord WordFormResolver::resolve(const std::string& norm, DebugTrace& trace) {
    ResolvedWord r;
    r.surface = norm;

    if (auto hit = m_wordCache.get(norm)) {
        r.wordId = hit->wordId;
        trace.logJson("WordFormResolver", "word_cache_hit", norm,
                      nlohmann::json{{"word_id", hit->wordId.value()}});
        return r;
    }
    if (auto w = m_db.findWordByNormalizedLemma(norm)) {
        m_wordCache.put(norm, *w);
        r.wordId = w->wordId;
        trace.logJson("WordFormResolver", "word_resolved", norm,
                      nlohmann::json{{"word_id", w->wordId.value()}});
        return r;
    }

    if (auto hit = m_formCache.get(norm)) {
        r.wordId     = hit->wordId;
        r.wordFormId = hit->wordFormId;
        return r;
    }
    if (auto f = m_db.findWordFormByNormalized(norm)) {
        m_formCache.put(norm, *f);
        r.wordId     = f->wordId;
        r.wordFormId = f->wordFormId;
        trace.logJson("WordFormResolver", "form_resolved", norm,
                      nlohmann::json{{"word_id",      f->wordId.value()},
                                     {"word_form_id", f->wordFormId.value()}});
        return r;
    }

    r.isUnknown = true;
    trace.log("WordFormResolver", "unknown_word", norm);
    return r;
}

}
