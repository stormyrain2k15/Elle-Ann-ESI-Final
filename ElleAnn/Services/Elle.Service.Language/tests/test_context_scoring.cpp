#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

TEST_CASE("ContextComparator ranks BASEBALL_CONTEXT highest for 'bat is heavy'") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    ConversationContext convo;
    convo.activeContextHints.push_back(ContextID{5});

    auto r = engine.analyze("The bat is heavy.", convo);

    REQUIRE_FALSE(r.meaning.contextFrames.empty());
    CHECK(r.meaning.contextFrames.front().code == "BASEBALL_CONTEXT");
}

TEST_CASE("punctuation cue elevates DISMISSIVE_HOSTILE") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine!!!");

    bool sawHostile = false;
    double hostileScore = 0.0;
    for (const auto& m : r.meaning.contextFrames) {
        if (m.code == "DISMISSIVE_HOSTILE") { sawHostile = true; hostileScore = m.score; }
    }
    CHECK(sawHostile);
    CHECK(hostileScore > 0.5);
}

TEST_CASE("contributing keywords are recorded so scoring is inspectable") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");

    REQUIRE_FALSE(r.meaning.contextFrames.empty());
    const auto& top = r.meaning.contextFrames.front();
    CHECK_FALSE(top.contributingKeywords.empty());
}
