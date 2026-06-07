#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("phrase win blocks redundant single-word units") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine");

    REQUIRE(r.meaning.sequence.units.size() == 1);
    CHECK(r.meaning.sequence.units[0].phraseId.has_value());

    for (const auto& u : r.meaning.sequence.units) {
        if (u.phraseId) continue;
        CHECK_FALSE((u.wordId && u.wordId->value() == 3));
    }
}

TEST_CASE("longest-phrase wins when overlapping phrases share a prefix") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("Don't worry about me.");

    bool foundPhrase3 = false;
    for (const auto& u : r.meaning.sequence.units) {
        if (u.phraseId && u.phraseId->value() == 3) foundPhrase3 = true;
    }
    CHECK(foundPhrase3);
}
