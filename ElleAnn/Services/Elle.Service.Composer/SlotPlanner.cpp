#include "SlotPlanner.h"
#include "InflectionTables.h"
#include "../_Shared/ElleLogger.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>
#include <sstream>
#include <unordered_set>

// ---------------------------------------------------------------------------
// ParseTemplate
// Parses "[NAME:POS:form]" and "[NAME:POS:form]?" tokens from a frame template.
// ---------------------------------------------------------------------------
std::vector<SlotSpec> SlotPlanner::ParseTemplate(const std::string& tmpl) {
    std::vector<SlotSpec> slots;
    std::regex slotRe(R"(\[([A-Z_]+):([A-Z_]+)(?::([A-Za-z0-9_]+))?\](\?)?)");
    auto begin = std::sregex_iterator(tmpl.begin(), tmpl.end(), slotRe);
    auto end   = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        SlotSpec s;
        s.name     = (*it)[1].str();
        s.posTag   = (*it)[2].str();
        s.form     = (*it)[3].matched ? (*it)[3].str() : "-";
        s.optional = (*it)[4].matched;
        slots.push_back(std::move(s));
    }
    return slots;
}

// ---------------------------------------------------------------------------
// BuildCandidates
// Simple semantic-graph hinting from the user_meaning senses + memory context.
// Without direct Language Engine DB access from this service, we extract
// meaningful content words from the text fields passed in the request payload.
// ---------------------------------------------------------------------------
std::vector<std::string> SlotPlanner::BuildCandidates(
    const SlotSpec& slot,
    const json&     userMeaning,
    const std::vector<std::string>& memoryCtx) const
{
    std::unordered_set<std::string> seen;
    std::vector<std::string> candidates;

    auto addWord = [&](const std::string& w) {
        if (w.size() < 2) return;
        std::string low = w;
        std::transform(low.begin(), low.end(), low.begin(),
            [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if (seen.insert(low).second) candidates.push_back(low);
    };

    // Extract from user_meaning tokens.
    if (userMeaning.is_object() && userMeaning.contains("tokens")) {
        for (auto& t : userMeaning["tokens"]) {
            if (t.is_string()) addWord(t.get<std::string>());
        }
    }

    // Extract content words from memory context.
    for (const auto& ctx : memoryCtx) {
        std::istringstream ss(ctx);
        std::string word;
        while (ss >> word) {
            // Strip punctuation.
            word.erase(std::remove_if(word.begin(), word.end(),
                [](unsigned char c){ return std::ispunct(c); }), word.end());
            addWord(word);
        }
    }

    // POS-based defaults per slot tag to ensure something is always available.
    if (slot.posTag == "PRON") {
        for (auto& p : {"I", "you", "we", "it"}) addWord(p);
    } else if (slot.posTag == "PRED") {
        for (auto& v : {"hear", "understand", "feel", "know", "think", "see",
                         "want", "need", "remember", "wonder"}) addWord(v);
    } else if (slot.posTag == "ADJ") {
        for (auto& a : {"okay", "good", "real", "hard", "sure", "right",
                         "quiet", "safe", "clear", "honest"}) addWord(a);
    } else if (slot.posTag == "NOUNPHRASE") {
        for (auto& n : {"that", "it", "more", "what", "this", "something",
                         "everything", "anything"}) addWord(n);
    } else if (slot.posTag == "INTENSIFIER") {
        for (auto& i : {"really", "just", "still", "even", "already"}) addWord(i);
    } else if (slot.posTag == "MODIFIER") {
        for (auto& m : {"though", "anyway", "then", "at least", "after all"}) addWord(m);
    }

    return candidates;
}

// ---------------------------------------------------------------------------
// PosCompatScore
// ---------------------------------------------------------------------------
float SlotPlanner::PosCompatScore(const std::string& lemma,
                                   const std::string& posTag) const
{
    // Lightweight heuristics -- no DB lookup at this layer.
    static const std::unordered_set<std::string> pronouns  = {"i","you","we","they","he","she","it"};
    static const std::unordered_set<std::string> commonVerbs = {"hear","know","feel","think","want",
        "need","see","say","tell","make","give","take","find","seem","look","get","go","come"};
    static const std::unordered_set<std::string> adverbs    = {"really","just","still","even",
        "already","quite","maybe","perhaps","always","never","often"};

    std::string low = lemma;
    std::transform(low.begin(), low.end(), low.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (posTag == "PRON"       && pronouns.count(low))   return 1.0f;
    if (posTag == "PRED"       && commonVerbs.count(low)) return 1.0f;
    if (posTag == "INTENSIFIER"&& adverbs.count(low))     return 0.9f;
    if (posTag == "NOUNPHRASE" && !pronouns.count(low) &&
                                  !commonVerbs.count(low)) return 0.7f;
    if (posTag == "ADJ") {
        if (low.back() == 'y' || low.back() == 'e' || low.back() == 'l') return 0.6f;
    }
    return 0.4f;
}

// ---------------------------------------------------------------------------
// EmotionalAlignScore
// ---------------------------------------------------------------------------
float SlotPlanner::EmotionalAlignScore(const std::string& lemma,
                                        const json&        emotionVec) const
{
    float valence = 0.0f;
    if (emotionVec.is_object()) {
        valence = emotionVec.value("valence", 0.0f);
    }

    // Positive-draw words.
    static const std::unordered_set<std::string> posWords = {
        "hear","understand","care","love","safe","good","okay","sure","glad",
        "right","clear","trust","hope","warmth","kind","together","remember"
    };
    // Negative-draw words.
    static const std::unordered_set<std::string> negWords = {
        "hurt","wrong","alone","lost","hard","difficult","painful","sad",
        "scared","worried","dark","heavy","empty","broken","cold"
    };

    std::string low = lemma;
    std::transform(low.begin(), low.end(), low.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (valence > 0.2f  && posWords.count(low)) return 1.0f;
    if (valence < -0.2f && negWords.count(low)) return 0.9f;
    if (valence > 0.2f  && negWords.count(low)) return 0.1f;
    if (valence < -0.2f && posWords.count(low)) return 0.2f;
    return 0.5f;
}

// ---------------------------------------------------------------------------
// FrequencyScore
// ---------------------------------------------------------------------------
float SlotPlanner::FrequencyScore(const std::string& lemma) const {
    // Shorter common words get a frequency boost.
    const size_t len = lemma.size();
    if (len <= 3)  return 1.0f;
    if (len <= 5)  return 0.8f;
    if (len <= 8)  return 0.6f;
    if (len <= 12) return 0.4f;
    return 0.2f;
}

// ---------------------------------------------------------------------------
// ScoreCandidate
// ---------------------------------------------------------------------------
float SlotPlanner::ScoreCandidate(
    const std::string& lemma,
    const SlotSpec&    slot,
    const json&        emotionVec,
    const json&        userMeaning,
    const SlotWeights& w) const
{
    float score = 0.0f;
    score += w.posCompatibility    * PosCompatScore(lemma, slot.posTag);
    score += w.emotionalAlignment  * EmotionalAlignScore(lemma, emotionVec);
    score += w.frequency           * FrequencyScore(lemma);

    // Context match: does the lemma appear in the user's tokens?
    float ctxMatch = 0.0f;
    if (userMeaning.is_object() && userMeaning.contains("tokens")) {
        for (auto& t : userMeaning["tokens"]) {
            if (t.is_string()) {
                std::string tok = t.get<std::string>();
                std::transform(tok.begin(), tok.end(), tok.begin(),
                    [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                if (tok == lemma) { ctxMatch = 1.0f; break; }
            }
        }
    }
    score += w.contextFrameMatch * ctxMatch;

    return score;
}

// ---------------------------------------------------------------------------
// Plan
// ---------------------------------------------------------------------------
std::vector<ResolvedSlot> SlotPlanner::Plan(
    const std::vector<SlotSpec>&    slots,
    const json&                     userMeaning,
    const json&                     emotionVec,
    const json&                     identityThreads,
    const std::vector<std::string>& memoryCtx,
    const SlotWeights&              weights) const
{
    std::vector<ResolvedSlot> resolved;
    resolved.reserve(slots.size());

    for (const auto& slot : slots) {
        auto candidates = BuildCandidates(slot, userMeaning, memoryCtx);

        std::string bestLemma;
        float       bestScore = -1.0f;

        for (const auto& lemma : candidates) {
            float s = ScoreCandidate(lemma, slot, emotionVec, userMeaning, weights);
            if (s > bestScore) {
                bestScore = s;
                bestLemma = lemma;
            }
        }

        if (bestLemma.empty() && !slot.optional) {
            // Last resort: use slot name as lowercase placeholder.
            bestLemma = slot.posTag == "PRON" ? "I" : "it";
            bestScore = 0.0f;
        }

        ResolvedSlot rs;
        rs.name    = slot.name;
        rs.lemma   = bestLemma;
        rs.form    = slot.form;
        rs.surface = bestLemma.empty() ? "" : m_inflection.Inflect(bestLemma, slot.form);
        rs.score   = bestScore;
        resolved.push_back(std::move(rs));
    }

    return resolved;
}
