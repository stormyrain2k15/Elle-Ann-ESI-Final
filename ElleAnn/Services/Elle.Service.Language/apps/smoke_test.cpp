#include "elle/Engine.hpp"
#include "elle/MeaningObjectBuilder.hpp"

#include <iostream>

#define REQUIRE(cond, msg) do {                                                \
    if (!(cond)) {                                                             \
        std::cerr << "[smoke] FAIL: " << (msg)                                 \
                  << " (line " << __LINE__ << ")\n";                           \
        return 1;                                                              \
    } else {                                                                   \
        std::cout << "[smoke] PASS: " << (msg) << "\n";                        \
    }                                                                          \
} while (0)

int main() {
    auto db = std::shared_ptr<elle::ISqlAccessLayer>(elle::makeInMemoryAccessLayer());
    elle::Engine engine(db, elle::EngineConfig::defaults());

    {
        const auto r = engine.analyze("I'm fine.");
        REQUIRE(r.meaning.sequence.units.size() == 1,
                "phrase 'I'm fine' should collapse into one WordUnit");
        REQUIRE(r.meaning.sequence.units[0].phraseId
             && r.meaning.sequence.units[0].phraseId->value() == 1,
                "PhraseID for 'I'm fine' should be 1");
        REQUIRE(!r.meaning.resolvedSenses.empty(),
                "ResolvedSense list must be non-empty");
        REQUIRE(r.meaning.resolvedSenses[0].chosenPhraseSenseId.has_value(),
                "must select a PhraseSense");
    }

    {
        elle::ConversationContext baseball;
        baseball.activeContextHints.push_back(elle::ContextID{5});
        const auto r = engine.analyze("The bat is heavy.", baseball);
        bool sawBat = false;
        for (const auto& rs : r.meaning.resolvedSenses) {
            if (rs.chosenSenseId && rs.chosenSenseId->value() == 5 ) sawBat = true;
        }
        REQUIRE(sawBat, "with BASEBALL_CONTEXT hint, 'bat' must resolve to baseball-club sense (SenseID=5)");
    }
    {
        elle::ConversationContext wildlife;
        wildlife.activeContextHints.push_back(elle::ContextID{6});
        const auto r = engine.analyze("The bat flew at dusk.", wildlife);
        bool sawBat = false;
        for (const auto& rs : r.meaning.resolvedSenses) {
            if (rs.chosenSenseId && rs.chosenSenseId->value() == 4 ) sawBat = true;
        }
        REQUIRE(sawBat, "with WILDLIFE_CONTEXT hint, 'bat' must resolve to mammal sense (SenseID=4)");
    }

    {
        const auto r = engine.analyze("there their");
        bool sawThere = false, sawTheir = false;
        for (const auto& u : r.meaning.sequence.units) {
            if (u.wordId && u.wordId->value() == 5) sawThere = true;
            if (u.wordId && u.wordId->value() == 6) sawTheir = true;
        }
        REQUIRE(sawThere && sawTheir, "'there' and 'their' must each resolve to their own WordID");
    }

    {
        const auto r = engine.analyze("flibbertigibbet");
        REQUIRE(r.meaning.unresolvedWords.size() == 1, "unknown word should be flagged");
    }

    {
        elle::ConversationContext baseball;
        baseball.activeContextHints.push_back(elle::ContextID{5});
        const auto r = engine.analyze("The bat is heavy.", baseball);
        REQUIRE(!r.meaning.conceptPaths.empty(), "graph walker must produce at least one ConceptPath");
    }

    {
        const auto r = engine.analyze("I'm fine.");
        const auto id = engine.persist(r);
        REQUIRE(id == 0, "InMemoryAccessLayer must return 0 from persistAnalysisTrace");
    }

    std::cout << "\n[smoke] All checks passed.\n";
    return 0;
}
