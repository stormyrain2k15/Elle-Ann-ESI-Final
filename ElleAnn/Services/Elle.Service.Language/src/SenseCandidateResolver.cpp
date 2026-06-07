#include "elle/SenseCandidateResolver.hpp"
#include "elle/StringUtil.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace elle {

namespace {

double logFrequencyScore(std::int64_t freq) {
    if (freq <= 0) return 0.0;

    const double raw = std::log10(static_cast<double>(freq) + 1.0);
    return std::min(raw / 7.0, 1.0);
}

double posNegAlignment(double pos, double neg, double sentenceValence) {

    if (std::abs(sentenceValence) < 1e-6) return 0.0;
    if (sentenceValence < 0.0) return neg - pos;
    return pos - neg;
}

double exampleOverlap(const std::vector<std::string>& examples,
                      const IntegerSequence& seq) {
    if (examples.empty()) return 0.0;

    std::unordered_set<std::string> words;
    for (const auto& u : seq.units) {
        for (const auto& lx : u.lexemes) {
            if (lx.isPunctuation) continue;
            words.insert(lx.normalized);
        }
    }

    double best = 0.0;
    for (const auto& ex : examples) {
        const auto tokens = str::splitWhitespace(str::toLowerAscii(ex));
        if (tokens.empty()) continue;
        std::size_t overlap = 0;
        for (const auto& tok : tokens) {

            std::string clean;
            for (char c : tok) {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '\'') {
                    clean.push_back(c);
                }
            }
            if (!clean.empty() && words.count(clean)) ++overlap;
        }
        const double s = static_cast<double>(overlap) /
                         static_cast<double>(tokens.size());
        if (s > best) best = s;
    }
    return best;
}

double sentenceValenceFromPunctuation(const IntegerSequence& seq) {
    double v = 0.0;
    if (seq.endsWithExclaim) v -= 0.20 * static_cast<double>(seq.exclamationCount);
    if (seq.ellipsisCount > 0) v -= 0.20 * static_cast<double>(seq.ellipsisCount);
    if (seq.endsWithQuestion) v += 0.05;
    return std::clamp(v, -1.0, 1.0);
}

}

SenseCandidateResolver::SenseCandidateResolver(ISqlAccessLayer& db,
                                               const ScoringWeights& weights,
                                               std::size_t senseCacheSize)
    : m_db(db),
      m_weights(weights),
      m_senseCache(senseCacheSize),
      m_phraseSenseCache(senseCacheSize),
      m_senseEmotionCache(senseCacheSize),
      m_phraseSenseEmotionCache(senseCacheSize) {}

double SenseCandidateResolver::scoreSense(const SenseRecord& s,
                                          const WordUnit& unit,
                                          const IntegerSequence& seq,
                                          const std::vector<ContextFrameMatch>& matches,
                                          const ConversationContext& convo,
                                          ScoredSense& out) {
    double total = 0.0;

    const double freq = logFrequencyScore(s.frequency);
    out.scoreBreakdown["frequency"] = freq * m_weights.frequency;
    total += freq * m_weights.frequency;

    double ctx = 0.0;
    for (const auto& m : matches) {

        ctx += 0.5 * m.score / (1.0 + m.score);

        if (unit.wordId) {
            const std::string key = "WordID:" + std::to_string(unit.wordId->value());
            auto it = m.contributingKeywords.find(key);
            if (it != m.contributingKeywords.end()) ctx += it->second;
        }
    }
    out.scoreBreakdown["context_frames"] = ctx * m_weights.contextFrameMatch;
    total += ctx * m_weights.contextFrameMatch;

    const auto usage   = m_db.getSenseUsageExamples(s.senseId);
    const auto contxt  = m_db.getSenseContextExamples(s.senseId);
    const double exU = exampleOverlap(usage, seq);
    const double exC = exampleOverlap(contxt, seq);
    const double ex  = 0.5 * (exU + exC);
    out.scoreBreakdown["example_overlap"] = ex * m_weights.senseExampleOverlap;
    total += ex * m_weights.senseExampleOverlap;

    double coocc = 0.0;
    if (unit.wordId) {
        for (const auto& u2 : seq.units) {
            if (u2.positionInSentence == unit.positionInSentence) continue;
            if (!u2.wordId) continue;

            const auto rels = m_db.getWordRelations(*unit.wordId, std::nullopt);
            for (const auto& r : rels) {
                if (r.toId == u2.wordId->value()) {
                    coocc += 0.5 * r.strength;
                }
            }
        }
    }
    out.scoreBreakdown["nearby_cooccurrence"] = coocc * m_weights.nearbyWordCooccur;
    total += coocc * m_weights.nearbyWordCooccur;

    double pos = 0.0;
    if (s.partOfSpeechId) {

        if (unit.positionInSentence >= 2 &&
            unit.positionInSentence < seq.units.size()) {
            const auto& prev = seq.units[unit.positionInSentence - 1];
            if (prev.wordId && prev.wordId->value() == 2 ) {
                if (s.partOfSpeechId->value() == 3 ) pos += 0.6;
                if (s.partOfSpeechId->value() == 2 ) pos -= 0.6;
            }
        }
    }
    out.scoreBreakdown["pos_compatibility"] = pos * m_weights.posCompatibility;
    total += pos * m_weights.posCompatibility;

    const double valHint  = sentenceValenceFromPunctuation(seq);
    const double pnAlign  = posNegAlignment(s.positiveDraw, s.negativeDraw, valHint);
    out.scoreBreakdown["draw_alignment"] = pnAlign * m_weights.posNegDrawAlignment;
    total += pnAlign * m_weights.posNegDrawAlignment;

    double convoBoost = 0.0;
    if (unit.wordId) {
        for (auto w : convo.recentWordIds) {
            if (w == *unit.wordId) convoBoost += 0.5;
        }
    }
    out.scoreBreakdown["conversation_hint"] = convoBoost * m_weights.conversationHint;
    total += convoBoost * m_weights.conversationHint;

    out.score = total;
    return total;
}

double SenseCandidateResolver::scorePhraseSense(const PhraseSenseRecord& s,
                                                const WordUnit& unit,
                                                const IntegerSequence& seq,
                                                const std::vector<ContextFrameMatch>& matches,
                                                const ConversationContext& convo,
                                                ScoredSense& out) {
    double total = 0.0;

    double ctx = 0.0;
    if (unit.phraseId) {
        const std::string key = "PhraseID:" + std::to_string(unit.phraseId->value());
        for (const auto& m : matches) {
            auto it = m.contributingKeywords.find(key);
            if (it == m.contributingKeywords.end()) continue;

            static const std::unordered_map<std::string, std::string> kFrameToSense = {
                {"CASUAL_STATUS_CHECK",  "neutral_okay"},
                {"EMOTIONAL_WITHDRAWAL", "sad_withdrawn"},
                {"DISMISSIVE_HOSTILE",   "angry_dismissive"},
                {"REASSURANCE",          "reassuring"},
            };
            auto mapIt = kFrameToSense.find(m.code);
            if (mapIt != kFrameToSense.end() && mapIt->second == s.gloss) {
                ctx += 1.5 * m.score;
                out.scoreBreakdown["frame_alignment::" + m.code] = 1.5 * m.score;
            } else {

                ctx += 0.25 * m.score;
            }
        }
    }
    out.scoreBreakdown["context_frames"] = ctx * m_weights.contextFrameMatch;
    total += ctx * m_weights.contextFrameMatch;

    const double freq    = logFrequencyScore(s.frequency);
    const double orderBoost = 1.0 / (1.0 + static_cast<double>(s.senseOrder));
    out.scoreBreakdown["frequency"]   = freq       * m_weights.frequency;
    out.scoreBreakdown["sense_order"] = orderBoost * 0.5;
    total += freq * m_weights.frequency;
    total += orderBoost * 0.5;

    const double valHint = sentenceValenceFromPunctuation(seq);
    const double pn      = posNegAlignment(s.positiveDraw, s.negativeDraw, valHint);
    out.scoreBreakdown["draw_alignment"] = pn * m_weights.posNegDrawAlignment;
    total += pn * m_weights.posNegDrawAlignment;

    (void)convo;

    out.score = total;
    return total;
}

std::vector<ResolvedSense>
SenseCandidateResolver::resolve(const IntegerSequence& seq,
                                const std::vector<ContextFrameMatch>& matches,
                                const ConversationContext& convo,
                                DebugTrace& trace) {
    std::vector<ResolvedSense> out;
    out.reserve(seq.units.size());

    for (const auto& u : seq.units) {
        ResolvedSense r;
        r.unitIndex = u.positionInSentence;

        if (u.phraseId && !u.phraseSenseCandidates.empty()) {
            std::vector<PhraseSenseRecord> recs;
            if (auto hit = m_phraseSenseCache.get(u.phraseId->value())) {
                recs = std::move(*hit);
            } else {
                recs = m_db.getSensesForPhrase(*u.phraseId);
                m_phraseSenseCache.put(u.phraseId->value(), recs);
            }
            for (const auto& rec : recs) {
                ScoredSense sc;
                sc.phraseSenseId = rec.phraseSenseId;
                scorePhraseSense(rec, u, seq, matches, convo, sc);
                sc.reason = "Phrase sense '" + rec.gloss + "' (PhraseSenseID="
                          + std::to_string(rec.phraseSenseId.value()) + ")";
                r.rankedCandidates.push_back(std::move(sc));
            }
            std::sort(r.rankedCandidates.begin(), r.rankedCandidates.end(),
                      [](const ScoredSense& a, const ScoredSense& b){ return a.score > b.score; });
            if (!r.rankedCandidates.empty()) {
                r.chosenPhraseSenseId = r.rankedCandidates.front().phraseSenseId;
                r.confidence = r.rankedCandidates.front().score;
            }
        }

        else if (u.wordId && !u.senseCandidates.empty()) {
            std::vector<SenseRecord> recs;
            if (auto hit = m_senseCache.get(u.wordId->value())) {
                recs = std::move(*hit);
            } else {
                recs = m_db.getSensesForWord(*u.wordId);
                m_senseCache.put(u.wordId->value(), recs);
            }
            for (const auto& rec : recs) {
                ScoredSense sc;
                sc.senseId = rec.senseId;
                scoreSense(rec, u, seq, matches, convo, sc);
                sc.reason = "Sense '" + rec.gloss + "' (SenseID="
                          + std::to_string(rec.senseId.value()) + ")";
                r.rankedCandidates.push_back(std::move(sc));
            }
            std::sort(r.rankedCandidates.begin(), r.rankedCandidates.end(),
                      [](const ScoredSense& a, const ScoredSense& b){ return a.score > b.score; });
            if (!r.rankedCandidates.empty()) {
                r.chosenSenseId = r.rankedCandidates.front().senseId;
                r.confidence = r.rankedCandidates.front().score;
            }
        }

        nlohmann::json payload = {
            {"unit_index", r.unitIndex},
            {"normalized", u.normalized},
            {"candidate_count", r.rankedCandidates.size()}
        };
        if (r.chosenSenseId)        payload["chosen_sense_id"]        = r.chosenSenseId->value();
        if (r.chosenPhraseSenseId)  payload["chosen_phrase_sense_id"] = r.chosenPhraseSenseId->value();
        nlohmann::json breakdown = nlohmann::json::array();
        for (const auto& c : r.rankedCandidates) {
            nlohmann::json item = {
                {"reason", c.reason},
                {"score",  c.score}
            };
            if (c.senseId)        item["sense_id"]        = c.senseId->value();
            if (c.phraseSenseId)  item["phrase_sense_id"] = c.phraseSenseId->value();
            nlohmann::json bd;
            for (const auto& [k, v] : c.scoreBreakdown) bd[k] = v;
            item["breakdown"] = std::move(bd);
            breakdown.push_back(std::move(item));
        }
        payload["candidates"] = std::move(breakdown);
        trace.logJson("SenseCandidateResolver", "unit_resolved",
                      "Per-unit sense decision recorded.", std::move(payload));

        out.push_back(std::move(r));
    }
    return out;
}

}
