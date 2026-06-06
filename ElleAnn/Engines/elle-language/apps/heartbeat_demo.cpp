// ============================================================================
// Elle Engine -- Heartbeat demo
// File: apps/heartbeat_demo.cpp
//
// Processes the sentence "I'm fine." under four conversational contexts and
// prints the resolved meaning, the chosen PhraseSenseID, the rejected
// candidates with score breakdowns, and the active context frames.
//
// This is the canonical proof that an integer-indexed dictionary database
// can produce four DIFFERENT meanings for the same surface string without
// LLM token prediction.
// ============================================================================
#include "elle/Engine.hpp"
#include "elle/MeaningObjectBuilder.hpp"

#include <iostream>
#include <vector>

namespace {

struct Scenario {
    const char*                          label;
    elle::ConversationContext            convo;
};

void runOne(elle::Engine& engine, const Scenario& sc) {
    std::cout << "\n============================================================\n";
    std::cout << "Scenario: " << sc.label << "\n";
    std::cout << "============================================================\n";

    const auto result = engine.analyze("I'm fine.", sc.convo);

    std::cout << "Normalized input : " << result.meaning.normalizedInput << "\n";
    std::cout << "Likely intent    : " << result.meaning.likelyIntent << "\n";
    std::cout << "Overall confidence: " << result.meaning.overallConfidence << "\n";

    std::cout << "\nContext frames (ranked):\n";
    for (const auto& m : result.meaning.contextFrames) {
        std::cout << "  - " << m.code << "  score=" << m.score
                  << "  (ContextID=" << m.contextId.value() << ")\n";
    }

    std::cout << "\nResolved senses:\n";
    for (const auto& r : result.meaning.resolvedSenses) {
        std::cout << "  unit[" << r.unitIndex << "]";
        if (r.chosenPhraseSenseId) {
            std::cout << "  -> PhraseSenseID=" << r.chosenPhraseSenseId->value();
        } else if (r.chosenSenseId) {
            std::cout << "  -> SenseID=" << r.chosenSenseId->value();
        }
        std::cout << "  confidence=" << r.confidence << "\n";
        for (const auto& c : r.rankedCandidates) {
            std::cout << "      " << c.reason << "  score=" << c.score << "\n";
            for (const auto& [k, v] : c.scoreBreakdown) {
                std::cout << "        " << k << " = " << v << "\n";
            }
        }
    }

    std::cout << "\nEmotional profile (EmotionID -> weight):\n";
    for (const auto& [eid, w] : result.meaning.emotionalProfile.byEmotionId) {
        std::cout << "  emotion[" << eid << "] = " << w << "\n";
    }

    std::cout << "\nConcept paths (semantic graph):\n";
    int idx = 0;
    for (const auto& p : result.meaning.conceptPaths) {
        std::cout << "  path[" << idx++ << "] strength=" << p.totalStrength
                  << " nodes=";
        for (auto n : p.nodes) std::cout << n.value() << " ";
        std::cout << "\n";
    }
}

} // namespace

int main() {
    auto db = std::shared_ptr<elle::ISqlAccessLayer>(elle::makeInMemoryAccessLayer());
    elle::Engine engine(db, elle::EngineConfig::defaults());

    using elle::ConversationContext;
    using elle::ContextID;

    std::vector<Scenario> scenarios = {
        {"Neutral status check (default)", ConversationContext{}},
        {"Sad / withdrawn (hint: EMOTIONAL_WITHDRAWAL)",
         ConversationContext{{ContextID{2}}, {}, "", false}},
        {"Angry / dismissive (hint: DISMISSIVE_HOSTILE)",
         ConversationContext{{ContextID{3}}, {}, "", false}},
        {"Reassuring (hint: REASSURANCE)",
         ConversationContext{{ContextID{4}}, {}, "", false}}
    };

    for (const auto& sc : scenarios) runOne(engine, sc);

    std::cout << "\n[heartbeat_demo] Completed " << scenarios.size() << " scenarios.\n";
    return 0;
}
