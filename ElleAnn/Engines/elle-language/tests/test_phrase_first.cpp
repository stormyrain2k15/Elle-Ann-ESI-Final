// ============================================================================
// Elle Engine -- Phrase-first matching
// File: tests/test_phrase_first.cpp
//
// When a phrase span and its constituent single-word lookups both exist,
// the phrase must win and no single-word units are emitted for the
// covered span.
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("phrase win blocks redundant single-word units") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine");

    // Sequence must be exactly one PhraseUnit, not three WordUnits (i, am, fine).
    REQUIRE(r.meaning.sequence.units.size() == 1);
    CHECK(r.meaning.sequence.units[0].phraseId.has_value());

    // Specifically: no WordUnit for 'fine' alone in the same sentence.
    for (const auto& u : r.meaning.sequence.units) {
        if (u.phraseId) continue;
        CHECK_FALSE((u.wordId && u.wordId->value() == 3));
    }
}

TEST_CASE("longest-phrase wins when overlapping phrases share a prefix") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("Don't worry about me.");

    // "don't worry about me" (5 words) must win over any 1-word fallbacks.
    bool foundPhrase3 = false;
    for (const auto& u : r.meaning.sequence.units) {
        if (u.phraseId && u.phraseId->value() == 3) foundPhrase3 = true;
    }
    CHECK(foundPhrase3);
}
