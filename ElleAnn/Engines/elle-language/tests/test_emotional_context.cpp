// ============================================================================
// Elle Engine -- Emotional context shift for "I'm fine"
// File: tests/test_emotional_context.cpp
//
// Same surface string, four different chosen PhraseSenseIDs depending on
// conversation context and punctuation cues.
// ============================================================================
#include <doctest/doctest.h>

#include "elle/Engine.hpp"

using namespace elle;

static std::int64_t chosenPS(const AnalysisResult& r) {
    REQUIRE_FALSE(r.meaning.resolvedSenses.empty());
    REQUIRE(r.meaning.resolvedSenses[0].chosenPhraseSenseId.has_value());
    return r.meaning.resolvedSenses[0].chosenPhraseSenseId->value();
}

TEST_CASE("default 'I'm fine.' picks the neutral PhraseSense (1)") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());
    const auto r = engine.analyze("I'm fine.");
    CHECK(chosenPS(r) == 1);
}

TEST_CASE("with EMOTIONAL_WITHDRAWAL hint, picks PhraseSense 2 (sad)") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());
    ConversationContext c;
    c.activeContextHints.push_back(ContextID{2});
    const auto r = engine.analyze("I'm fine.", c);
    CHECK(chosenPS(r) == 2);
}

TEST_CASE("with DISMISSIVE_HOSTILE hint, picks PhraseSense 3 (angry)") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());
    ConversationContext c;
    c.activeContextHints.push_back(ContextID{3});
    const auto r = engine.analyze("I'm fine!", c);
    CHECK(chosenPS(r) == 3);
}

TEST_CASE("with REASSURANCE hint, picks PhraseSense 4 (reassuring)") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());
    ConversationContext c;
    c.activeContextHints.push_back(ContextID{4});
    const auto r = engine.analyze("I'm fine.", c);
    CHECK(chosenPS(r) == 4);
}

TEST_CASE("punctuation alone (... + no hint) tilts towards sad sense") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());
    const auto r = engine.analyze("I'm fine...");
    // Without an explicit hint, the punctuation must at minimum elevate the
    // sad PhraseSense above neutral or be visible in the ranked candidates.
    REQUIRE_FALSE(r.meaning.resolvedSenses.empty());
    const auto& rs = r.meaning.resolvedSenses[0];
    bool sadAhead = false;
    for (std::size_t i = 0; i < rs.rankedCandidates.size(); ++i) {
        if (rs.rankedCandidates[i].phraseSenseId
            && rs.rankedCandidates[i].phraseSenseId->value() == 2) {
            // sad found; check it scored above neutral if neutral comes later.
            for (std::size_t j = i + 1; j < rs.rankedCandidates.size(); ++j) {
                if (rs.rankedCandidates[j].phraseSenseId
                    && rs.rankedCandidates[j].phraseSenseId->value() == 1) {
                    sadAhead = true;
                    break;
                }
            }
            break;
        }
    }
    const bool sadWonOrAhead = sadAhead ||
        (rs.chosenPhraseSenseId && rs.chosenPhraseSenseId->value() == 2);
    CHECK_MESSAGE(sadWonOrAhead,
                  "ellipsis should at least pull sad PhraseSense above neutral");
}
