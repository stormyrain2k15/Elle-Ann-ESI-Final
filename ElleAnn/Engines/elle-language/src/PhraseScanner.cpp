// ============================================================================
// Elle Engine -- PhraseScanner implementation
// File: src/PhraseScanner.cpp
//
// Longest-match-first scan with contraction expansion at the lexeme stream
// level. The DB stores phrases by their *expanded* normalized form
// (e.g. "i am fine"), so we expand "I'm"->"i am" and "don't"->"do not"
// before matching.
// ============================================================================
#include "elle/PhraseScanner.hpp"
#include "elle/StringUtil.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace elle {

namespace {

// Hand-rolled contraction table. This is intentionally small and explicit.
// Real production would load this from the DB; the dictionary already has the
// form "I'm" -> WordID 1 (I), so we lean on this map only for phrase scanning.
const std::unordered_map<std::string, std::vector<std::string>>& contractionTable() {
    static const std::unordered_map<std::string, std::vector<std::string>> kTable = {
        {"i'm",     {"i", "am"}},
        {"you're",  {"you", "are"}},
        {"he's",    {"he", "is"}},
        {"she's",   {"she", "is"}},
        {"it's",    {"it", "is"}},
        {"we're",   {"we", "are"}},
        {"they're", {"they", "are"}},
        {"don't",   {"do", "not"}},
        {"doesn't", {"does", "not"}},
        {"didn't",  {"did", "not"}},
        {"isn't",   {"is", "not"}},
        {"aren't",  {"are", "not"}},
        {"wasn't",  {"was", "not"}},
        {"weren't", {"were", "not"}},
        {"won't",   {"will", "not"}},
        {"can't",   {"can", "not"}},
        {"couldn't",{"could", "not"}},
        {"wouldn't",{"would", "not"}},
        {"shouldn't",{"should", "not"}},
        {"i've",    {"i", "have"}},
        {"i'll",    {"i", "will"}},
        {"i'd",     {"i", "would"}}
    };
    return kTable;
}

// One position in the expanded stream maps back to one or more lexeme indices
// in the original lexeme vector (because "I'm" expands to two tokens).
struct ExpandedTok {
    std::string                normalized;
    std::vector<std::size_t>   originLexemeIndices;
};

std::vector<ExpandedTok> expandLexemes(const std::vector<Lexeme>& lexemes) {
    std::vector<ExpandedTok> out;
    out.reserve(lexemes.size() + 4);
    for (std::size_t i = 0; i < lexemes.size(); ++i) {
        const Lexeme& lx = lexemes[i];
        if (lx.isPunctuation) continue;

        auto it = contractionTable().find(lx.normalized);
        if (it != contractionTable().end()) {
            for (const auto& part : it->second) {
                ExpandedTok t;
                t.normalized = part;
                t.originLexemeIndices.push_back(i);
                out.push_back(std::move(t));
            }
        } else {
            ExpandedTok t;
            t.normalized = lx.normalized;
            t.originLexemeIndices.push_back(i);
            out.push_back(std::move(t));
        }
    }
    return out;
}

} // namespace

PhraseScanner::PhraseScanner(ISqlAccessLayer& db, std::size_t cacheSize)
    : m_db(db), m_phraseCache(cacheSize) {}

std::vector<PhraseMatch> PhraseScanner::scan(const std::vector<Lexeme>& lexemes,
                                             DebugTrace& trace) {
    std::vector<PhraseMatch> matches;
    const auto expanded = expandLexemes(lexemes);
    if (expanded.empty()) return matches;

    // For each starting position try to match the longest known phrase.
    std::vector<bool> consumed(expanded.size(), false);

    for (std::size_t i = 0; i < expanded.size(); ++i) {
        if (consumed[i]) continue;

        const auto& firstNorm = expanded[i].normalized;

        // Cache key is the first token's normalized form. The cache stores
        // all phrases whose first word resolves to this token.
        std::vector<PhraseRecord> candidates;
        if (auto hit = m_phraseCache.get(firstNorm)) {
            candidates = std::move(*hit);
        } else {
            // Resolve first word's WordID; without it we cannot ask the DB.
            auto wordOpt = m_db.findWordByNormalizedLemma(firstNorm);
            if (!wordOpt) {
                m_phraseCache.put(firstNorm, {});
                continue;
            }
            candidates = m_db.findPhrasesStartingWith(wordOpt->wordId, /*maxWordCount*/ 16);
            m_phraseCache.put(firstNorm, candidates);
        }

        // Longest first.
        bool matched = false;
        for (const auto& p : candidates) {
            const int len = p.wordCount;
            if (i + static_cast<std::size_t>(len) > expanded.size()) continue;

            bool ok = true;
            for (int k = 0; k < len; ++k) {
                // Compare expanded normalized tokens against the phrase's
                // expected wordSequence. We re-resolve the expected word's
                // normalized lemma via DB; cached after first use is implicit
                // through SqlServerAccessLayer's word cache (or in-memory map).
                auto expectedWord = m_db.findWordById(p.wordSequence[static_cast<std::size_t>(k)]);
                if (!expectedWord) { ok = false; break; }
                if (expanded[i + static_cast<std::size_t>(k)].normalized != expectedWord->normalizedLemma) {
                    ok = false;
                    break;
                }
            }
            if (!ok) continue;

            PhraseMatch m;
            m.phraseId = p.phraseId;
            m.normalizedForm = p.normalizedForm;
            // Map expanded indices back to lexeme indices.
            m.startLexemeIdx = expanded[i].originLexemeIndices.front();
            m.endLexemeIdx   = expanded[i + static_cast<std::size_t>(len) - 1].originLexemeIndices.back() + 1;
            // Pull candidate phrase senses.
            const auto ps = m_db.getSensesForPhrase(p.phraseId);
            m.phraseSenseCandidates.reserve(ps.size());
            for (const auto& s : ps) m.phraseSenseCandidates.push_back(s.phraseSenseId);
            matches.push_back(m);

            // Mark consumed.
            for (int k = 0; k < len; ++k) consumed[i + static_cast<std::size_t>(k)] = true;

            nlohmann::json payload = {
                {"phrase_id",       p.phraseId.value()},
                {"normalized",      p.normalizedForm},
                {"start_lexeme",    m.startLexemeIdx},
                {"end_lexeme",      m.endLexemeIdx},
                {"sense_count",     ps.size()}
            };
            trace.logJson("PhraseScanner", "phrase_matched",
                          "Longest-match-first hit.", std::move(payload));
            matched = true;
            break;
        }

        if (!matched) {
            // Nothing to log per-token at trace level except in verbose mode.
        }
    }

    return matches;
}

} // namespace elle
