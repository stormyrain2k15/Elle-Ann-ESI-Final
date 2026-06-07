#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"

#include "service/ProbabilityHost.h"
#include "service/ProbabilityProto.h"

#include "elle/prob/Types.hpp"
#include "elle/prob/Bridge.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"
#include "elle/Types.hpp"

#include <cstdint>
#include <exception>
#include <string>
#include <unordered_map>
#include <vector>

using nlohmann::json;
using elleann::prob::resultToJson;
using elleann::prob::weightsToJson;
using elleann::prob::weightsFromJson;
using elleann::prob::convoFromJson;
using elleann::prob::requestFromJson;
using elleann::prob::trustSignalFromString;
using elleann::prob::hormonalStateFromJson;

class ElleProbabilityService : public ElleServiceBase {
public:
    ElleProbabilityService()
        : ElleServiceBase(SVC_PROBABILITY, "ElleProbability",
                          "Elle-Ann Probability Engine",
                          "Bayesian probability + language analyzer (deterministic LLM replacement)") {}

protected:
    bool OnStart() override {
        elleann::prob::HostConfig cfg;
        cfg.engineConfigPath =
            ElleConfig::Instance().GetString("probability.language_config", "");
        cfg.probabilityConfigPath =
            ElleConfig::Instance().GetString("probability.engine_config", "");
        cfg.autoLoadOnStart =
            ElleConfig::Instance().GetBool("probability.auto_load_on_start", true);
        cfg.useInMemoryLanguage =
            ElleConfig::Instance().GetBool("probability.use_in_memory_language", false);

        if (!m_host.start(cfg)) {
            ELLE_ERROR("Probability host failed to start (auto_load=%d)",
                       (int)cfg.autoLoadOnStart);
            return false;
        }
        ELLE_INFO("Probability service started (ready=%d, in_memory_lang=%d)",
                  (int)m_host.ready(), (int)cfg.useInMemoryLanguage);
        return true;
    }

    void OnStop() override {
        m_host.stop();
        ELLE_INFO("Probability service stopped");
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_PROB_ANALYZE:         HandleAnalyze(msg, sender);        break;
            case IPC_PROB_SCORE:           HandleScore(msg, sender);          break;
            case IPC_PROB_FEEDBACK:        HandleFeedback(msg, sender);       break;
            case IPC_PROB_TRUST:           HandleTrust(msg, sender);          break;
            case IPC_PROB_INJECT_HORMONAL: HandleInjectHormonal(msg, sender); break;
            case IPC_PROB_RELOAD:          HandleReload(msg, sender);         break;
            case IPC_PROB_QUERY_WEIGHTS:   HandleQueryWeights(msg, sender);   break;
            case IPC_PROB_SEED_WEIGHTS:    HandleSeedWeights(msg, sender);    break;
            case IPC_PROB_RESET:           HandleReset(msg, sender);          break;
            default: break;
        }
    }

private:
    elleann::prob::ProbabilityHost m_host;

    bool ParseReq(const ElleIPCMessage& req, ELLE_SERVICE_ID sender, json& out) {
        const std::string s = req.GetStringPayload();
        if (s.empty()) { out = json::object(); return true; }
        try {
            out = json::parse(s);
            if (!out.is_object()) out = json::object();
            return true;
        } catch (const std::exception& e) {
            ELLE_WARN("Probability ParseReq JSON error: %s", e.what());
            Reply(req, sender, json{
                {"success", false},
                {"error",   "invalid JSON"}
            });
            return false;
        }
    }

    void Reply(const ElleIPCMessage& req, ELLE_SERVICE_ID sender, const json& body) {
        ElleIPCMessage resp = ElleIPCMessage::Create(IPC_PROB_RESPONSE,
                                                    SVC_PROBABILITY, sender);
        resp.header.correlation_id = req.header.correlation_id;
        resp.SetStringPayload(body.dump());
        GetIPCHub().Send(sender, resp);
    }

    std::string ExtractRequestId(const json& body) {
        if (body.contains("request_id") && body["request_id"].is_string()) {
            return body["request_id"].get<std::string>();
        }
        return std::string();
    }

    void HandleAnalyze(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        const std::string text     = body.value("text",       std::string());
        const std::string speaker  = body.value("speaker_id", std::string("default"));
        elle::ConversationContext convo;
        if (body.contains("convo") && body["convo"].is_object()) {
            convo = convoFromJson(body["convo"]);
        } else {
            convo = convoFromJson(body);
        }
        if (text.empty()) {
            Reply(req, sender, json{
                {"request_id", ExtractRequestId(body)},
                {"success",    false},
                {"error",      "empty_text"}
            });
            return;
        }
        auto outcome = m_host.analyzeText(text, convo, speaker);
        json rep = {
            {"request_id", ExtractRequestId(body)},
            {"success",    outcome.success},
            {"error",      outcome.error}
        };
        if (outcome.success) {
            rep["result"] = resultToJson(outcome.result);
            if (outcome.meaning) {
                rep["likely_intent"]      = outcome.meaning->likelyIntent;
                rep["overall_confidence"] = outcome.meaning->overallConfidence;
                rep["unresolved_words"]   = outcome.meaning->unresolvedWords;
            }
        }
        Reply(req, sender, rep);
    }

    void HandleScore(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        const std::string speaker = body.value("speaker_id", std::string("default"));
        if (!body.contains("request") || !body["request"].is_object()) {
            Reply(req, sender, json{
                {"request_id", ExtractRequestId(body)},
                {"success",    false},
                {"error",      "missing_request"}
            });
            return;
        }
        auto preq = requestFromJson(body["request"]);
        auto outcome = m_host.scoreRequest(preq, speaker);
        json rep = {
            {"request_id", ExtractRequestId(body)},
            {"success",    outcome.success},
            {"error",      outcome.error}
        };
        if (outcome.success) {
            rep["result"] = resultToJson(outcome.result);
        }
        Reply(req, sender, rep);
    }

    void HandleFeedback(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        std::size_t  unitIndex   = body.value("unit_index",          (std::size_t)0);
        std::int64_t confirmedId = body.value("confirmed_sense_id",  (std::int64_t)0);
        bool         isPhrase    = body.value("is_phrase",           false);
        double       confidence  = body.value("confidence",          1.0);
        std::string  speaker     = body.value("speaker_id",          std::string("default"));
        bool ok = m_host.feedback(unitIndex, confirmedId, isPhrase, confidence, speaker);
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok}
        });
    }

    void HandleTrust(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        const std::string speaker  = body.value("speaker_id", std::string("default"));
        const std::string sigName  = body.value("signal",     std::string("CONFIRMED_ACCURATE"));
        const double      strength = body.value("strength",   1.0);
        bool ok = m_host.recordTrust(speaker, trustSignalFromString(sigName), strength);
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok}
        });
    }

    void HandleInjectHormonal(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        std::unordered_map<std::int64_t, double> state =
            body.contains("state") ? hormonalStateFromJson(body["state"])
                                   : std::unordered_map<std::int64_t, double>{};
        bool ok = m_host.injectHormonalState(state);
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok},
            {"applied",    (std::int64_t)state.size()}
        });
    }

    void HandleReload(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; ParseReq(req, sender, body);
        bool ok = m_host.reload();
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok},
            {"ready",      m_host.ready()}
        });
    }

    void HandleQueryWeights(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; ParseReq(req, sender, body);
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    m_host.ready()},
            {"weights",    weightsToJson(m_host.queryWeights())}
        });
    }

    void HandleSeedWeights(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        if (!body.contains("weights") || !body["weights"].is_object()) {
            Reply(req, sender, json{
                {"request_id", ExtractRequestId(body)},
                {"success",    false},
                {"error",      "missing_weights"}
            });
            return;
        }
        bool ok = m_host.seedWeights(weightsFromJson(body["weights"]));
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok}
        });
    }

    void HandleReset(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; ParseReq(req, sender, body);
        const std::string scope = body.value("scope", std::string("turn"));
        bool ok = (scope == "all") ? m_host.resetAll() : m_host.resetTurn();
        Reply(req, sender, json{
            {"request_id", ExtractRequestId(body)},
            {"success",    ok},
            {"scope",      scope}
        });
    }
};

ELLE_SERVICE_MAIN(ElleProbabilityService)
