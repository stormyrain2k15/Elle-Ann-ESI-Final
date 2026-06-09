#pragma once

#include "elle/Types.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace elle::lex {

struct CompletenessFlags {
    bool hasDefinition       = false;
    bool hasPartOfSpeech     = false;
    bool hasUsageExample     = false;
    bool hasContextExample   = false;
    bool hasEmotionWeighting = false;
    bool hasValencePull      = false;
    bool hasRelation         = false;
    bool hasConcept          = false;
    bool hasAnagramKey       = false;
};

struct CompletenessReport {
    WordID            wordId{0};
    std::string       lemma;
    std::string       normalizedLemma;
    std::string       anagramKey;
    bool              isPalindrome = false;
    int               senseCount   = 0;
    CompletenessFlags flags{};
    std::vector<std::string> missingRequirements;
    float             score = 0.0f;
    bool              isCognitivelyComplete = false;
};

inline std::string computeAnagramKey(std::string_view normalizedLemma) {
    std::string key;
    key.reserve(normalizedLemma.size());
    for (char c : normalizedLemma) {
        unsigned char u = static_cast<unsigned char>(c);
        if (u >= 'A' && u <= 'Z') u = static_cast<unsigned char>(u - 'A' + 'a');
        if (u >= 'a' && u <= 'z') key.push_back(static_cast<char>(u));
    }
    std::sort(key.begin(), key.end());
    return key;
}

inline bool isPalindromeNormalized(std::string_view normalizedLemma) {
    std::string filtered;
    filtered.reserve(normalizedLemma.size());
    for (char c : normalizedLemma) {
        unsigned char u = static_cast<unsigned char>(c);
        if (u >= 'A' && u <= 'Z') u = static_cast<unsigned char>(u - 'A' + 'a');
        if (u >= 'a' && u <= 'z') filtered.push_back(static_cast<char>(u));
    }
    if (filtered.empty()) return false;
    size_t i = 0, j = filtered.size() - 1;
    while (i < j) {
        if (filtered[i] != filtered[j]) return false;
        ++i; --j;
    }
    return true;
}

struct EvaluateInputs {
    WordRecord                       word;
    std::vector<SenseRecord>         senses;
    std::vector<std::vector<std::string>>     usageExamplesBySense;
    std::vector<std::vector<std::string>>     contextExamplesBySense;
    std::vector<std::vector<EmotionWeight>>   emotionsBySense;
    std::vector<RelationRecord>      wordRelations;
    std::vector<std::vector<RelationRecord>>  senseRelationsBySense;
    std::vector<std::vector<ConceptMemberRecord>> conceptsBySense;
};

inline CompletenessReport evaluate(const EvaluateInputs& in) {
    CompletenessReport r;
    r.wordId          = in.word.wordId;
    r.lemma           = in.word.lemma;
    r.normalizedLemma = in.word.normalizedLemma;
    r.anagramKey      = in.word.anagramKey.empty()
                         ? computeAnagramKey(in.word.normalizedLemma)
                         : in.word.anagramKey;
    r.isPalindrome    = in.word.isPalindrome;
    r.senseCount      = static_cast<int>(in.senses.size());

    for (const auto& s : in.senses) {
        if (!s.definition.empty()) r.flags.hasDefinition = true;
        if (s.partOfSpeechId.has_value()) r.flags.hasPartOfSpeech = true;
        if (s.positiveDraw != 0.0 || s.negativeDraw != 0.0) r.flags.hasValencePull = true;
    }

    for (const auto& vec : in.usageExamplesBySense) {
        if (!vec.empty()) { r.flags.hasUsageExample = true; break; }
    }
    for (const auto& vec : in.contextExamplesBySense) {
        if (!vec.empty()) { r.flags.hasContextExample = true; break; }
    }
    for (const auto& vec : in.emotionsBySense) {
        if (!vec.empty()) { r.flags.hasEmotionWeighting = true; break; }
    }

    if (!in.wordRelations.empty()) r.flags.hasRelation = true;
    if (!r.flags.hasRelation) {
        for (const auto& vec : in.senseRelationsBySense) {
            if (!vec.empty()) { r.flags.hasRelation = true; break; }
        }
    }

    for (const auto& vec : in.conceptsBySense) {
        if (!vec.empty()) { r.flags.hasConcept = true; break; }
    }

    r.flags.hasAnagramKey = !r.anagramKey.empty();

    if (!r.flags.hasDefinition)       r.missingRequirements.push_back("HAS_DEFINITION");
    if (!r.flags.hasPartOfSpeech)     r.missingRequirements.push_back("HAS_PART_OF_SPEECH");
    if (!r.flags.hasUsageExample)     r.missingRequirements.push_back("HAS_USAGE_EXAMPLE");
    if (!r.flags.hasContextExample)   r.missingRequirements.push_back("HAS_CONTEXT_EXAMPLE");
    if (!r.flags.hasEmotionWeighting) r.missingRequirements.push_back("HAS_EMOTION_WEIGHTING");
    if (!r.flags.hasValencePull)      r.missingRequirements.push_back("HAS_VALENCE_PULL");
    if (!r.flags.hasRelation)         r.missingRequirements.push_back("HAS_RELATION");
    if (!r.flags.hasConcept)          r.missingRequirements.push_back("HAS_CONCEPT");
    if (!r.flags.hasAnagramKey)       r.missingRequirements.push_back("HAS_ANAGRAM_KEY");

    int hits = 0;
    if (r.flags.hasDefinition)       ++hits;
    if (r.flags.hasPartOfSpeech)     ++hits;
    if (r.flags.hasUsageExample)     ++hits;
    if (r.flags.hasContextExample)   ++hits;
    if (r.flags.hasEmotionWeighting) ++hits;
    if (r.flags.hasValencePull)      ++hits;
    if (r.flags.hasRelation)         ++hits;
    if (r.flags.hasConcept)          ++hits;
    if (r.flags.hasAnagramKey)       ++hits;
    r.score = static_cast<float>(hits) / 9.0f;

    r.isCognitivelyComplete =
        r.flags.hasDefinition       &&
        r.flags.hasPartOfSpeech     &&
        r.flags.hasUsageExample     &&
        r.flags.hasContextExample   &&
        r.flags.hasEmotionWeighting &&
        r.flags.hasValencePull      &&
        r.flags.hasRelation         &&
        r.flags.hasAnagramKey;

    return r;
}

}
