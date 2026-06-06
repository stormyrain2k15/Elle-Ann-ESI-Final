// ============================================================================
// Elle Engine -- Unknown word fallback
// File: tests/test_unknown_word.cpp
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("unknown word is captured in unresolvedWords") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("flibbertigibbet");

    CHECK(r.meaning.unresolvedWords.size() == 1);
    CHECK(r.meaning.unresolvedWords[0] == "flibbertigibbet");
    REQUIRE_FALSE(r.meaning.sequence.units.empty());
    CHECK(r.meaning.sequence.units[0].isUnknown);
}

TEST_CASE("unknown word does not halt processing of known neighbors") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("happy quazzlebop sad");

    bool sawHappy = false, sawSad = false;
    int unknown = 0;
    for (const auto& u : r.meaning.sequence.units) {
        if (u.wordId && u.wordId->value() == 10) sawHappy = true;
        if (u.wordId && u.wordId->value() == 11) sawSad = true;
        if (u.isUnknown) ++unknown;
    }
    CHECK(sawHappy);
    CHECK(sawSad);
    CHECK(unknown == 1);
}
