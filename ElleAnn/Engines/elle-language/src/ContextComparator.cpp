// ============================================================================
// Elle Engine -- ContextComparator implementation
// File: src/ContextComparator.cpp
//
// Score a context frame as:
//   score = sum( kw.weight if (kw.wordId in seq.words) or
//                            (kw.phraseId in seq.phrases) )
//         + conversation_hint_boost
// ============================================================================
#include "elle/ContextComparator.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <unordered_set>

namespace elle {

ContextComparator::ContextComparator(ISqlAccessLayer& db, std::size_t cacheSize)
    : m_db(db), m_frameCache(cacheSize), m_kwCache(cacheSize) {}

std::vector<ContextFrameMatch>
ContextComparator::compare(const IntegerSequence& seq,
                           const ConversationContext& convo,
                           DebugTrace& trace) {
    // Collect sentence IDs once.
    std::unordered_set<std::int64_t> wordIds;
    std::unordered_set<std::int64_t> phraseIds;
    for (const auto& u : seq.units) {
        if (u.wordId)   wordIds.insert(u.wordId->value());
        if (u.phraseId) phraseIds.insert(u.phraseId->value());
    }

    // Get all frames (cached).
    std::vector<ContextFrameRecord> frames;
    if (auto hit = m_frameCache.get(0)) {
        frames = std::move(*hit);
    } else {
        frames = m_db.getAllContextFrames();
        m_frameCache.put(0, frames);
    }

    std::vector<ContextFrameMatch> out;
    out.reserve(frames.size());

    const std::unordered_set<std::int64_t> hintSet = [&]() {
        std::unordered_set<std::int64_t> s;
        for (auto c : convo.activeContextHints) s.insert(c.value());
        return s;
    }();

    for (const auto& f : frames) {
        std::vector<ContextKeywordRecord> kws;
        if (auto hit = m_kwCache.get(f.contextId.value())) {
            kws = std::move(*hit);
        } else {
            kws = m_db.getContextKeywordsForFrame(f.contextId);
            m_kwCache.put(f.contextId.value(), kws);
        }

        ContextFrameMatch match;
        match.contextId = f.contextId;
        match.code      = f.code;
        match.name      = f.name;

        for (const auto& kw : kws) {
            if (kw.wordId && wordIds.count(kw.wordId->value())) {
                match.score += kw.weight;
                match.contributingKeywords["WordID:" + std::to_string(kw.wordId->value())] = kw.weight;
            }
            if (kw.phraseId && phraseIds.count(kw.phraseId->value())) {
                match.score += kw.weight;
                match.contributingKeywords["PhraseID:" + std::to_string(kw.phraseId->value())] = kw.weight;
            }
        }

        // Tone-from-punctuation boost: anger frames respect exclamations.
        if (f.code == "DISMISSIVE_HOSTILE" && seq.endsWithExclaim) {
            match.score += 0.40 + 0.10 * static_cast<double>(seq.exclamationCount - 1);
            match.contributingKeywords["punctuation:!"] = 0.40;
        }
        if (f.code == "EMOTIONAL_WITHDRAWAL" && seq.ellipsisCount > 0) {
            match.score += 0.35;
            match.contributingKeywords["punctuation:..."] = 0.35;
        }
        if (f.code == "CASUAL_STATUS_CHECK" && seq.endsWithQuestion) {
            match.score += 0.20;
            match.contributingKeywords["punctuation:?"] = 0.20;
        }

        // Caller-supplied hint.
        if (hintSet.count(f.contextId.value())) {
            match.score += 0.75;
            match.contributingKeywords["conversation_hint"] = 0.75;
        }

        if (match.score > 0.0) out.push_back(std::move(match));
    }

    std::sort(out.begin(), out.end(),
              [](const ContextFrameMatch& a, const ContextFrameMatch& b) {
                  return a.score > b.score;
              });

    nlohmann::json payload = nlohmann::json::array();
    for (const auto& m : out) {
        payload.push_back({
            {"context_id", m.contextId.value()},
            {"code",       m.code},
            {"score",      m.score}
        });
    }
    trace.logJson("ContextComparator", "ranked_frames",
                  "Context frames ranked by sentence overlap.", std::move(payload));
    return out;
}

} // namespace elle
