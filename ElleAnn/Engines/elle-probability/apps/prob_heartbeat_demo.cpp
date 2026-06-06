// ============================================================================
// Elle Probability Engine -- Heartbeat Demo
// File: apps/prob_heartbeat_demo.cpp
//
// Mirrors the language engine's heartbeat_demo.cpp.
// Processes "I'm fine." under four contexts and shows how the probability
// engine produces different posteriors, intent distributions, and weight
// recommendations for the same surface string -- and how beliefs ACCUMULATE
// across repeated calls (the probability engine learns from the conversation).
//
// Run after building with CMake:
//   ./prob_heartbeat_demo
// ============================================================================
#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/AuditTrace.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace elle::prob;

// ---------------------------------------------------------------------------
// Helper: print a Distribution as a ranked table
// ---------------------------------------------------------------------------
static void printDistribution(const Distribution& d, const char* label) {
    // Sort by probability descending.
    std::vector<std::pair<std::int64_t, double>> entries(d.mass.begin(), d.mass.end());
    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b){ return a.second > b.second; });

    std::cout << "  " << label << " (entropy=" << std::fixed << std::setprecision(3)
              << d.entropy() << "):\n";
    for (const auto& [k, v] : entries) {
        if (v > 0.005) {
            std::cout << "    h=" << std::setw(3) << k
                      << "  p=" << std::setprecision(4) << v << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// Helper: print WeightVector
// ---------------------------------------------------------------------------
static void printWeights(const WeightVector& w) {
    std::cout << "  Live weights:\n"
              << "    contextFrameMatch   = " << w.contextFrameMatch   << "\n"
              << "    nearbyWordCooccur   = " << w.nearbyWordCooccur   << "\n"
              << "    senseExampleOverlap = " << w.senseExampleOverlap << "\n"
              << "    emotionalAlignment  = " << w.emotionalAlignment  << "\n"
              << "    frequency           = " << w.frequency           << "\n"
              << "    posCompatibility    = " << w.posCompatibility    << "\n"
              << "    posNegDrawAlignment = " << w.posNegDrawAlignment << "\n"
              << "    conversationHint    = " << w.conversationHint    << "\n";
}

// ---------------------------------------------------------------------------
// Build a ProbabilityRequest mimicking "I'm fine." in different contexts.
// Sense candidates:
//   1 -> neutral_okay (PhraseSenseID from language engine)
//   2 -> sad_withdrawn
//   3 -> angry_dismissive
//   4 -> reassuring
// ---------------------------------------------------------------------------
static ProbabilityRequest buildRequest(
    const std::vector<ProbabilityRequest::ContextHint>& ctxHints,
    const std::unordered_map<std::int64_t, double>&     emotionProfile,
    bool endsWithQuestion = false,
    int  exclamationCount = 0)
{
    ProbabilityRequest req;

    ProbabilityRequest::UnitSpec unit;
    unit.unitIndex  = 0;
    unit.isPhrase   = true;
    unit.phraseId   = 1;  // "I'm fine" -> PhraseID 1 from language engine
    unit.phraseSenseCandidateIds = {1, 2, 3, 4};

    req.units.push_back(unit);
    req.contextHints    = ctxHints;
    req.emotionalProfile= emotionProfile;
    req.endsWithQuestion= endsWithQuestion;
    req.exclamationCount= exclamationCount;

    return req;
}

// ---------------------------------------------------------------------------
// One scenario
// ---------------------------------------------------------------------------
static void runScenario(ProbabilityEngine& engine,
                        const char*        label,
                        const ProbabilityRequest& req,
                        const std::string& speakerId = "default")
{
    std::cout << "\n============================================================\n";
    std::cout << "Scenario: " << label << "\n";
    std::cout << "============================================================\n";

    const auto result = engine.analyze(req, speakerId);

    std::cout << "  Overall confidence: " << std::fixed << std::setprecision(4)
              << result.overallConfidence << "\n";
    std::cout << "  Speaker trust:      " << result.speakerTrust << "\n";
    std::cout << "  Likely intent:      " << [&]() -> const char* {
        switch (result.likelyAct) {
            case PragmaticAct::ASSERT:    return "ASSERT";
            case PragmaticAct::DEFLECT:   return "DEFLECT";
            case PragmaticAct::DENY:      return "DENY";
            case PragmaticAct::COMFORT:   return "COMFORT";
            case PragmaticAct::CHALLENGE: return "CHALLENGE";
            default:                      return "OTHER";
        }
    }() << "\n";

    // Per-unit results.
    for (const auto& u : result.units) {
        std::cout << "  unit[" << u.unitIndex << "]"
                  << " winner=PhraseSenseID(" << u.winningSenseId << ")"
                  << " p=" << std::setprecision(4) << u.winnerProbability
                  << " entropy=" << u.posteriorEntropy << "\n";

        std::cout << "    Ranked candidates:\n";
        for (const auto& sh : u.rankedCandidates) {
            std::cout << "      h=" << sh.hypothesisId
                      << "  p=" << std::setprecision(4) << sh.probability
                      << "  factors: ";
            for (const auto& [k, v] : sh.contributingFactors) {
                std::cout << k << "=" << std::setprecision(3) << v << " ";
            }
            std::cout << "\n";
        }
    }

    printDistribution(result.intentDistribution, "Intent distribution");
    printDistribution(result.emotionalPosterior,  "Emotional posterior");
    printWeights(result.recommendedWeights);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "Elle Probability Engine -- Heartbeat Demo\n";
    std::cout << "\"I'm fine.\" under four contexts.\n\n";

    ProbabilityEngineConfig cfg = ProbabilityEngineConfig::defaults();
    cfg.stochastic = false;  // deterministic for demo
    ProbabilityEngine engine(cfg);

    // Seed weights from language engine's defaults.
    WeightVector langDefaults;
    langDefaults.contextFrameMatch   = 1.0;
    langDefaults.nearbyWordCooccur   = 0.6;
    langDefaults.senseExampleOverlap = 0.5;
    langDefaults.emotionalAlignment  = 0.7;
    langDefaults.frequency           = 0.3;
    langDefaults.posCompatibility    = 0.4;
    langDefaults.posNegDrawAlignment = 0.5;
    langDefaults.conversationHint    = 0.8;
    engine.seedWeights(langDefaults);

    // ---- Scenario 1: Neutral status check ----
    runScenario(engine,
        "Neutral status check (default)",
        buildRequest({}, {}));

    // ---- Scenario 2: Emotional withdrawal ----
    // Context hint 2 = EMOTIONAL_WITHDRAWAL, high sadness.
    runScenario(engine,
        "Sad / withdrawn (EMOTIONAL_WITHDRAWAL + high sadness)",
        buildRequest(
            {{2, 0.9}},
            {{4 /*SADNESS*/, 1.5}, {12 /*NEG_DRAW*/, 1.2}}));

    // ---- Scenario 3: Angry / dismissive ----
    // Context hint 3 = DISMISSIVE_HOSTILE, high anger.
    runScenario(engine,
        "Angry / dismissive (DISMISSIVE_HOSTILE + high anger)",
        buildRequest(
            {{3, 0.9}},
            {{2 /*ANGER*/, 1.8}, {12 /*NEG_DRAW*/, 1.0}},
            false, 1));  // 1 exclamation

    // ---- Scenario 4: Reassurance ----
    // Context hint 4 = REASSURANCE, high comfort + trust.
    runScenario(engine,
        "Reassuring (REASSURANCE + comfort + trust)",
        buildRequest(
            {{4, 0.9}},
            {{8 /*COMFORT*/, 1.5}, {6 /*TRUST*/, 1.2}, {11 /*POS_DRAW*/, 1.0}}));

    // ---- Scenario 5: Second neutral pass (belief drift visible) ----
    // After 4 turns the probability engine has accumulated evidence.
    // This shows how beliefs drift across a conversation.
    runScenario(engine,
        "Second neutral pass (accumulated belief drift)",
        buildRequest({}, {}));

    std::cout << "\n[prob_heartbeat_demo] Completed 5 scenarios.\n";
    std::cout << "Belief count in store: "
              << engine.beliefStore().domainCount() << "\n";
    return 0;
}
