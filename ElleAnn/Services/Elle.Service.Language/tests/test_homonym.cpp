#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("homonym 'bat' resolves to baseball sense under BASEBALL_CONTEXT") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    ConversationContext convo;
    convo.activeContextHints.push_back(ContextID{5});

    auto r = engine.analyze("The bat is heavy.", convo);

    bool foundBaseballBat = false;
    for (const auto& rs : r.meaning.resolvedSenses) {
        if (rs.chosenSenseId && rs.chosenSenseId->value() == 5) {
            foundBaseballBat = true;
        }
    }
    CHECK(foundBaseballBat);
}

TEST_CASE("homonym 'bat' resolves to mammal sense under WILDLIFE_CONTEXT") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    ConversationContext convo;
    convo.activeContextHints.push_back(ContextID{6});

    auto r = engine.analyze("The bat flew at dusk.", convo);

    bool foundMammalBat = false;
    for (const auto& rs : r.meaning.resolvedSenses) {
        if (rs.chosenSenseId && rs.chosenSenseId->value() == 4) {
            foundMammalBat = true;
        }
    }
    CHECK(foundMammalBat);
}

TEST_CASE("homonym candidates all present in ranked list regardless of winner") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("bat");
    REQUIRE_FALSE(r.meaning.resolvedSenses.empty());

    const auto& rs = r.meaning.resolvedSenses[0];
    int senseCount = 0;
    for (const auto& c : rs.rankedCandidates) {
        if (c.senseId) ++senseCount;
    }
    CHECK(senseCount >= 3);
}
