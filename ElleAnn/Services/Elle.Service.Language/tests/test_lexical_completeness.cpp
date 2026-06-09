#include "doctest/doctest.h"

#include "elle/LexicalCompleteness.hpp"

#include <string>

using namespace elle;
using namespace elle::lex;

TEST_CASE("computeAnagramKey sorts and lowercases letters") {
    CHECK(computeAnagramKey("listen") == "eilnst");
    CHECK(computeAnagramKey("silent") == "eilnst");
    CHECK(computeAnagramKey("LISTEN") == "eilnst");
}

TEST_CASE("computeAnagramKey strips non-letters") {
    CHECK(computeAnagramKey("listen!") == "eilnst");
    CHECK(computeAnagramKey("li-sten 2") == "eilnst");
}

TEST_CASE("computeAnagramKey returns empty for empty input") {
    CHECK(computeAnagramKey("").empty());
    CHECK(computeAnagramKey("12345").empty());
}

TEST_CASE("Anagram pairs share the same key") {
    CHECK(computeAnagramKey("evil")  == computeAnagramKey("vile"));
    CHECK(computeAnagramKey("evil")  == computeAnagramKey("live"));
    CHECK(computeAnagramKey("dusty") == computeAnagramKey("study"));
    CHECK(computeAnagramKey("dusty") != computeAnagramKey("studyy"));
}

TEST_CASE("isPalindromeNormalized recognises classic palindromes") {
    CHECK(isPalindromeNormalized("racecar"));
    CHECK(isPalindromeNormalized("level"));
    CHECK(isPalindromeNormalized("madam"));
    CHECK(!isPalindromeNormalized("listen"));
    CHECK(!isPalindromeNormalized(""));
}

namespace {

EvaluateInputs makeCompleteInputs() {
    EvaluateInputs in;
    in.word.wordId          = WordID{1};
    in.word.lemma           = "love";
    in.word.normalizedLemma = "love";
    in.word.anagramKey      = "elov";
    in.word.isPalindrome    = false;

    SenseRecord s;
    s.senseId        = SenseID{1};
    s.wordId         = WordID{1};
    s.partOfSpeechId = PartOfSpeechID{1};
    s.definition     = "an intense feeling of deep affection";
    s.positiveDraw   = 0.9;
    in.senses.push_back(s);

    in.usageExamplesBySense   = { { "I love you" } };
    in.contextExamplesBySense = { { "expressed at the end of a long letter" } };
    in.emotionsBySense        = { { EmotionWeight{ EmotionID{1}, 0.85 } } };

    RelationRecord rel;
    rel.fromId = 1;
    rel.toId   = 2;
    rel.relationTypeId = RelationTypeID{1};
    rel.strength = 0.7;
    in.wordRelations.push_back(rel);

    ConceptMemberRecord cm;
    cm.conceptId = ConceptID{1};
    cm.senseId   = SenseID{1};
    cm.strength  = 1.0;
    in.conceptsBySense = { { cm } };

    return in;
}

}

TEST_CASE("evaluate flags a fully-populated word as cognitively complete") {
    auto report = evaluate(makeCompleteInputs());
    CHECK(report.isCognitivelyComplete);
    CHECK(report.score == doctest::Approx(1.0f));
    CHECK(report.missingRequirements.empty());
    CHECK(report.flags.hasDefinition);
    CHECK(report.flags.hasPartOfSpeech);
    CHECK(report.flags.hasUsageExample);
    CHECK(report.flags.hasContextExample);
    CHECK(report.flags.hasEmotionWeighting);
    CHECK(report.flags.hasValencePull);
    CHECK(report.flags.hasRelation);
    CHECK(report.flags.hasConcept);
    CHECK(report.flags.hasAnagramKey);
}

TEST_CASE("evaluate flags missing usage examples") {
    auto in = makeCompleteInputs();
    in.usageExamplesBySense = { {} };
    auto report = evaluate(in);
    CHECK(!report.isCognitivelyComplete);
    CHECK(report.score < 1.0f);
    bool foundUsage = false;
    for (const auto& m : report.missingRequirements) {
        if (m == "HAS_USAGE_EXAMPLE") { foundUsage = true; break; }
    }
    CHECK(foundUsage);
}

TEST_CASE("evaluate flags word with zero senses as deeply incomplete") {
    EvaluateInputs in;
    in.word.wordId = WordID{42};
    in.word.lemma = "ineffable";
    in.word.normalizedLemma = "ineffable";
    in.word.anagramKey = "abeefilnn";

    auto report = evaluate(in);
    CHECK(!report.isCognitivelyComplete);
    CHECK(report.senseCount == 0);
    CHECK(!report.flags.hasDefinition);
    CHECK(!report.flags.hasPartOfSpeech);
    CHECK(report.score < 0.2f);
}

TEST_CASE("evaluate flags word with definition but no relations as incomplete") {
    auto in = makeCompleteInputs();
    in.wordRelations.clear();
    in.senseRelationsBySense = { {} };
    auto report = evaluate(in);
    CHECK(!report.isCognitivelyComplete);
    CHECK(!report.flags.hasRelation);
    bool foundRel = false;
    for (const auto& m : report.missingRequirements) {
        if (m == "HAS_RELATION") { foundRel = true; break; }
    }
    CHECK(foundRel);
}

TEST_CASE("evaluate computes anagram key on-the-fly if missing on record") {
    auto in = makeCompleteInputs();
    in.word.anagramKey.clear();
    auto report = evaluate(in);
    CHECK(!report.anagramKey.empty());
    CHECK(report.anagramKey == computeAnagramKey(in.word.normalizedLemma));
    CHECK(report.flags.hasAnagramKey);
}

TEST_CASE("evaluate's missingRequirements list is stable and ordered") {
    EvaluateInputs in;
    in.word.wordId = WordID{99};
    in.word.lemma = "x";
    in.word.normalizedLemma = "x";
    auto report = evaluate(in);
    REQUIRE(report.missingRequirements.size() >= 7);
    CHECK(report.missingRequirements[0] == "HAS_DEFINITION");
    CHECK(report.missingRequirements[1] == "HAS_PART_OF_SPEECH");
    CHECK(report.missingRequirements[2] == "HAS_USAGE_EXAMPLE");
    CHECK(report.missingRequirements[3] == "HAS_CONTEXT_EXAMPLE");
    CHECK(report.missingRequirements[4] == "HAS_EMOTION_WEIGHTING");
    CHECK(report.missingRequirements[5] == "HAS_VALENCE_PULL");
    CHECK(report.missingRequirements[6] == "HAS_RELATION");
}
