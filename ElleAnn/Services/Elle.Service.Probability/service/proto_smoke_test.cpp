#include "service/ProbabilityHost.h"
#include "service/ProbabilityProto.h"

#include "elle/prob/SpeakerTrustModel.hpp"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <iostream>

using nlohmann::json;
using namespace elleann::prob;

static int g_failures = 0;
static int g_checks   = 0;

#define EXPECT(cond, label)                                                 \
    do {                                                                    \
        ++g_checks;                                                         \
        if (!(cond)) {                                                      \
            ++g_failures;                                                   \
            std::cerr << "[FAIL] " << label << " (at " << __LINE__ << ")\n";\
        } else {                                                            \
            std::cout << "[OK]   " << label << "\n";                        \
        }                                                                   \
    } while (0)

static json buildAnalyzeEnvelope() {
    return json{
        {"request_id", "rid-analyze-1"},
        {"text",       "I'm fine."},
        {"speaker_id", "user_demo"},
        {"convo", {
            {"speakerRelationship", "intimate"},
            {"activeContextHints", json::array({77})},
            {"recentWordIds",      json::array({101, 202})},
            {"prefersBaseball",    false}
        }}
    };
}

static json buildScoreEnvelope() {
    return json{
        {"request_id", "rid-score-1"},
        {"speaker_id", "user_demo"},
        {"request", {
            {"units", json::array({
                {
                    {"unitIndex", 0},
                    {"isPhrase",  true},
                    {"phraseId",  1},
                    {"phraseSenseCandidateIds", json::array({1, 2, 3, 4})}
                }
            })},
            {"contextHints",   json::array({{{"contextId", 2}, {"score", 0.9}}})},
            {"emotionalProfile", {{"4", 1.5}, {"12", 1.2}}},
            {"speakerRelationship", "intimate"},
            {"endsWithQuestion",  false},
            {"exclamationCount",  0}
        }}
    };
}

static json buildHormonalEnvelope() {
    return json{
        {"request_id", "rid-hormonal-1"},
        {"state", {
            {"5", 1.2},
            {"6", 0.8},
            {"12", 0.4}
        }}
    };
}

static json buildTrustEnvelope() {
    return json{
        {"request_id", "rid-trust-1"},
        {"speaker_id", "user_demo"},
        {"signal",     "CONFIRMED_ACCURATE"},
        {"strength",   1.0}
    };
}

int main() {
    std::cout << "==== Probability IPC Envelope Smoke ====\n";

    HostConfig cfg;
    cfg.autoLoadOnStart     = true;
    cfg.useInMemoryLanguage = true;
    ProbabilityHost host;
    EXPECT(host.start(cfg), "host.start()");
    EXPECT(host.ready(),    "host.ready()");

    {
        json env = buildAnalyzeEnvelope();
        elle::ConversationContext convo = convoFromJson(env["convo"]);
        EXPECT(convo.speakerRelationship == "intimate",         "convo.speakerRelationship");
        EXPECT(convo.activeContextHints.size() == 1,            "convo.activeContextHints");
        EXPECT(convo.activeContextHints[0].value() == 77,       "convo.activeContextHints[0]");
        EXPECT(convo.recentWordIds.size()      == 2,            "convo.recentWordIds");

        auto outcome = host.analyzeText(env.value("text", std::string()),
                                        convo,
                                        env.value("speaker_id", std::string("default")));
        EXPECT(outcome.success, "analyzeText.success");

        json rep = json{
            {"request_id", env.value("request_id", std::string())},
            {"success",    outcome.success}
        };
        if (outcome.success) rep["result"] = resultToJson(outcome.result);
        EXPECT(rep["request_id"] == "rid-analyze-1",            "analyze.rep.request_id");
        EXPECT(rep["result"].contains("likelyAct"),             "analyze.rep.result.likelyAct");
        EXPECT(rep["result"].contains("speakerTrust"),          "analyze.rep.result.speakerTrust");
        EXPECT(rep["result"].contains("recommendedWeights"),    "analyze.rep.result.recommendedWeights");
    }

    {
        json env = buildScoreEnvelope();
        auto req = requestFromJson(env["request"]);
        EXPECT(req.units.size() == 1,                            "score.req.units");
        EXPECT(req.units[0].isPhrase,                            "score.req.units[0].isPhrase");
        EXPECT(req.units[0].phraseSenseCandidateIds.size() == 4, "score.req.phraseSenseCandidateIds");
        EXPECT(req.contextHints.size() == 1,                     "score.req.contextHints");
        EXPECT(req.contextHints[0].contextId == 2,               "score.req.contextHints[0].contextId");
        EXPECT(req.emotionalProfile[4]  == 1.5,                  "score.req.emotionalProfile[4]");
        EXPECT(req.emotionalProfile[12] == 1.2,                  "score.req.emotionalProfile[12]");
        EXPECT(req.speakerRelationship == "intimate",            "score.req.speakerRelationship");

        auto outcome = host.scoreRequest(req, env.value("speaker_id", std::string("default")));
        EXPECT(outcome.success, "scoreRequest.success");
        EXPECT(outcome.result.units.size() == 1, "scoreRequest.units");
        EXPECT(outcome.result.units[0].rankedCandidates.size() >= 1,
               "scoreRequest.rankedCandidates");
    }

    {
        json env = buildHormonalEnvelope();
        auto state = hormonalStateFromJson(env["state"]);
        EXPECT(state.size() == 3,             "hormonal.state.size");
        EXPECT(state[5]  == 1.2,              "hormonal.state[5]");
        EXPECT(state[6]  == 0.8,              "hormonal.state[6]");
        EXPECT(state[12] == 0.4,              "hormonal.state[12]");
        EXPECT(host.injectHormonalState(state), "host.injectHormonalState()");
    }

    {
        json env = buildTrustEnvelope();
        auto sig = trustSignalFromString(env.value("signal", std::string()));
        EXPECT(sig == elle::prob::TrustSignal::CONFIRMED_ACCURATE,
               "trust.signal mapping");
        EXPECT(host.recordTrust(env.value("speaker_id", std::string("default")),
                                sig, env.value("strength", 1.0)),
               "host.recordTrust()");
    }

    {
        auto w = host.queryWeights();
        json wjson = weightsToJson(w);
        EXPECT(wjson.contains("contextFrameMatch"), "weightsToJson.contextFrameMatch");
        elle::prob::WeightVector rt = weightsFromJson(wjson);
        EXPECT(rt.contextFrameMatch == w.contextFrameMatch, "weights.roundtrip.contextFrameMatch");
        EXPECT(rt.emotionalAlignment == w.emotionalAlignment,
               "weights.roundtrip.emotionalAlignment");
    }

    {
        EXPECT(host.feedback(0, 1, true, 0.9, "user_demo"), "host.feedback()");
        EXPECT(host.resetTurn(),                            "host.resetTurn()");
    }

    host.stop();

    std::cout << "\n==== Summary: " << (g_checks - g_failures) << " / "
              << g_checks << " checks passed ====\n";
    return g_failures == 0 ? 0 : 1;
}
