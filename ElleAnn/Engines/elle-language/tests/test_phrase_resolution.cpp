// ============================================================================
// Elle Engine -- Phrase resolution: "I'm fine"
// File: tests/test_phrase_resolution.cpp
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("'I'm fine' collapses into one PhraseUnit") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");

    REQUIRE(r.meaning.sequence.units.size() == 1);
    REQUIRE(r.meaning.sequence.units[0].phraseId.has_value());
    CHECK(r.meaning.sequence.units[0].phraseId->value() == 1);
}

TEST_CASE("'I'm fine' produces four candidate PhraseSenses") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");

    REQUIRE_FALSE(r.meaning.resolvedSenses.empty());
    const auto& rs = r.meaning.resolvedSenses[0];
    CHECK(rs.rankedCandidates.size() == 4);
}

TEST_CASE("'don't worry about me' resolves as PhraseID 3") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("Don't worry about me.");

    bool foundPhrase3 = false;
    for (const auto& u : r.meaning.sequence.units) {
        if (u.phraseId && u.phraseId->value() == 3) foundPhrase3 = true;
    }
    CHECK(foundPhrase3);
}
