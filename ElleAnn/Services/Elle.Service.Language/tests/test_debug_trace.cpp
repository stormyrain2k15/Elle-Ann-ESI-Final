#include <doctest/doctest.h>

#include "elle/Engine.hpp"
#include "elle/MeaningObjectBuilder.hpp"

#include <nlohmann/json.hpp>

using namespace elle;

TEST_CASE("trace records every layer's decision") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");

    const auto json = r.trace.toJson(2);
    REQUIRE_FALSE(json.empty());

    auto parsed = nlohmann::json::parse(json);
    REQUIRE(parsed.is_array());

    bool sawNormalizer       = false;
    bool sawPhraseScanner    = false;
    bool sawSequenceBuilder  = false;
    bool sawContextComparator= false;
    bool sawSenseResolver    = false;
    bool sawEmotionProcessor = false;
    bool sawGraphWalker      = false;

    for (const auto& e : parsed) {
        const std::string layer = e.at("layer").get<std::string>();
        if (layer == "InputNormalizer")          sawNormalizer       = true;
        if (layer == "PhraseScanner")            sawPhraseScanner    = true;
        if (layer == "IntegerSequenceBuilder")   sawSequenceBuilder  = true;
        if (layer == "ContextComparator")        sawContextComparator= true;
        if (layer == "SenseCandidateResolver")   sawSenseResolver    = true;
        if (layer == "EmotionalWeightProcessor") sawEmotionProcessor = true;
        if (layer == "SemanticGraphWalker")      sawGraphWalker      = true;
    }
    CHECK(sawNormalizer);
    CHECK(sawPhraseScanner);
    CHECK(sawSequenceBuilder);
    CHECK(sawContextComparator);
    CHECK(sawSenseResolver);
    CHECK(sawEmotionProcessor);
    CHECK(sawGraphWalker);
}

TEST_CASE("each chosen sense exposes a score breakdown explaining the win") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");
    REQUIRE_FALSE(r.meaning.resolvedSenses.empty());

    const auto& rs = r.meaning.resolvedSenses[0];
    REQUIRE_FALSE(rs.rankedCandidates.empty());

    for (const auto& c : rs.rankedCandidates) {
        CHECK_FALSE(c.scoreBreakdown.empty());
        CHECK_FALSE(c.reason.empty());
    }
}

TEST_CASE("MeaningObject JSON renders successfully and is parseable") {
    auto db = std::shared_ptr<ISqlAccessLayer>(makeInMemoryAccessLayer());
    Engine engine(db, EngineConfig::defaults());

    auto r = engine.analyze("I'm fine.");
    const auto json = meaningObjectToJson(r.meaning, 2);
    REQUIRE_FALSE(json.empty());
    CHECK_NOTHROW((void)nlohmann::json::parse(json));
}
