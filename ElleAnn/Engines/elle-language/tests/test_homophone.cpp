// ============================================================================
// Elle Engine -- Homophone handling: there / their / they're
// File: tests/test_homophone.cpp
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("homophones there/their/they're each resolve to distinct WordIDs") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("there their they're");

    std::int64_t there  = 0;
    std::int64_t their  = 0;
    std::int64_t theyre = 0;
    for (const auto& u : r.meaning.sequence.units) {
        if (u.normalized == "there"   && u.wordId) there  = u.wordId->value();
        if (u.normalized == "their"   && u.wordId) their  = u.wordId->value();
        if (u.normalized == "they're" && u.wordId) theyre = u.wordId->value();
    }
    CHECK(there  == 5);
    CHECK(their  == 6);
    CHECK(theyre == 7);
    CHECK(there  != their);
    CHECK(their  != theyre);
}

TEST_CASE("homophone relation exists between there and their") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    auto relations = db->getWordRelations(WordID{5}, rel::HOMOPHONE);
    bool hasTheir = false;
    for (const auto& r : relations) {
        if (r.toId == 6) hasTheir = true;
    }
    CHECK(hasTheir);
}
