#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/AuditTrace.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace elle::prob;

static void printDistribution(const Distribution& d, const char* label) {

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
    unit.phraseId   = 1;
    unit.phraseSenseCandidateIds = {1, 2, 3, 4};

    req.units.push_back(unit);
    req.contextHints    = ctxHints;
    req.emotionalProfile= emotionProfile;
    req.endsWithQuestion= endsWithQuestion;
    req.exclamationCount= exclamationCount;

    return req;
}

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

int main() {
    std::cout << "Elle Probability Engine -- Heartbeat Demo\n";
    std::cout << "\"I'm fine.\" under four contexts.\n\n";

    ProbabilityEngineConfig cfg = ProbabilityEngineConfig::defaults();
    cfg.stochastic = false;
    ProbabilityEngine engine(cfg);

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

    runScenario(engine,
        "Neutral status check (default)",
        buildRequest({}, {}));

    runScenario(engine,
        "Sad / withdrawn (EMOTIONAL_WITHDRAWAL + high sadness)",
        buildRequest(
            {{2, 0.9}},
            {{4 , 1.5}, {12 , 1.2}}));

    runScenario(engine,
        "Angry / dismissive (DISMISSIVE_HOSTILE + high anger)",
        buildRequest(
            {{3, 0.9}},
            {{2 , 1.8}, {12 , 1.0}},
            false, 1));

    runScenario(engine,
        "Reassuring (REASSURANCE + comfort + trust)",
        buildRequest(
            {{4, 0.9}},
            {{8 , 1.5}, {6 , 1.2}, {11 , 1.0}}));

    runScenario(engine,
        "Second neutral pass (accumulated belief drift)",
        buildRequest({}, {}));

    std::cout << "\n[prob_heartbeat_demo] Completed 5 scenarios.\n";
    std::cout << "Belief count in store: "
              << engine.beliefStore().domainCount() << "\n";
    return 0;
}
