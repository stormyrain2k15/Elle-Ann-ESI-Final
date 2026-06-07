#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleComposerClient.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/json.hpp"
#include "../_Shared/ElleJsonExtract.h"
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <set>
#include <cmath>

using json = nlohmann::json;

class IntentParser {
public:
    struct ParseResult {
        ELLE_INTENT_TYPE type;
        float           confidence;
        float           urgency;
        std::string     parameters;
        std::string     raw_text;
    };

    ParseResult Parse(const std::string& text, const std::string& context);
    ParseResult ParseWithLLM(const std::string& text, const std::string& context);

private:

    ParseResult RuleBasedParse(const std::string& text);

    struct Pattern {
        std::string keyword;
        ELLE_INTENT_TYPE intent;
        float base_confidence;
    };
    static const std::vector<Pattern> s_patterns;
};

const std::vector<IntentParser::Pattern> IntentParser::s_patterns = {
    {"vibrate",     INTENT_HARDWARE_COMMAND, 0.95f},
    {"flash",       INTENT_HARDWARE_COMMAND, 0.95f},
    {"notify",      INTENT_HARDWARE_COMMAND, 0.9f},
    {"remember",    INTENT_STORE_MEMORY,     0.9f},
    {"recall",      INTENT_RECALL_MEMORY,    0.9f},
    {"forget",      INTENT_STORE_MEMORY,     0.85f},
    {"open app",    INTENT_EXECUTE_ACTION,   0.9f},
    {"launch",      INTENT_PROCESS_CONTROL,  0.85f},
    {"kill",        INTENT_PROCESS_CONTROL,  0.85f},
    {"read file",   INTENT_FILE_OPERATION,   0.9f},
    {"write file",  INTENT_FILE_OPERATION,   0.9f},
    {"delete file", INTENT_FILE_OPERATION,   0.9f},
    {"think about", INTENT_SELF_REFLECT,     0.8f},
    {"reflect",     INTENT_SELF_REFLECT,     0.85f},
    {"goal",        INTENT_GOAL_UPDATE,      0.8f},
    {"create",      INTENT_CREATIVE_GENERATE, 0.7f},
    {"learn",       INTENT_LEARN,            0.8f},
    {"explore",     INTENT_EXPLORE,          0.75f},
    {"predict",     INTENT_PREDICT,          0.8f},
    {"is it right", INTENT_ETHICAL_EVALUATE,  0.8f},
    {"should I",    INTENT_ETHICAL_EVALUATE,  0.75f},
};

IntentParser::ParseResult IntentParser::Parse(const std::string& text, const std::string& context) {

    auto result = RuleBasedParse(text);
    if (result.confidence >= ElleConfig::Instance().GetFloat("cognitive.intent_confidence_threshold", 0.6)) {
        return result;
    }

    return ParseWithLLM(text, context);
}

IntentParser::ParseResult IntentParser::RuleBasedParse(const std::string& text) {
    ParseResult result;
    result.type = INTENT_CHAT;
    result.confidence = 0.5f;
    result.urgency = 0.5f;
    result.raw_text = text;

    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });

    for (auto& pattern : s_patterns) {
        if (lower.find(pattern.keyword) != std::string::npos) {
            result.type = pattern.intent;
            result.confidence = pattern.base_confidence;
            result.parameters = text;
            return result;
        }
    }

    return result;
}

IntentParser::ParseResult IntentParser::ParseWithLLM(const std::string& text, const std::string& context) {
    ParseResult result;
    result.type = INTENT_CHAT;
    result.confidence = 0.5f;
    result.urgency = 0.5f;
    result.raw_text = text;
    (void)context;
    return result;
}

class CognitiveEngine {
public:
    bool Initialize() {
        ELLE_INFO("Cognitive engine initialized");
        return true;
    }

    void Tick() {  }
};

struct PendingWorld {
    std::mutex               m;
    std::condition_variable  cv;
    bool                     done = false;
    nlohmann::json           result;
};

class WorldCorrelator {
public:
    std::shared_ptr<PendingWorld> Register(const std::string& requestId) {
        auto p = std::make_shared<PendingWorld>();
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map[requestId] = p;
        return p;
    }
    void Release(const std::string& requestId) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map.erase(requestId);
    }
    void Deliver(const std::string& requestId, nlohmann::json result) {
        std::shared_ptr<PendingWorld> p;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_map.find(requestId);
            if (it == m_map.end()) return;
            p = it->second;
        }
        {
            std::lock_guard<std::mutex> lk(p->m);
            p->result = std::move(result);
            p->done = true;
        }
        p->cv.notify_one();
    }
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<PendingWorld>> m_map;
};

struct PendingProb {
    std::mutex               m;
    std::condition_variable  cv;
    bool                     done = false;
    nlohmann::json           result;
};

class ProbCorrelator {
public:
    std::shared_ptr<PendingProb> Register(const std::string& requestId) {
        auto p = std::make_shared<PendingProb>();
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map[requestId] = p;
        return p;
    }
    void Release(const std::string& requestId) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map.erase(requestId);
    }
    void Deliver(const std::string& requestId, nlohmann::json result) {
        std::shared_ptr<PendingProb> p;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_map.find(requestId);
            if (it == m_map.end()) return;
            p = it->second;
        }
        {
            std::lock_guard<std::mutex> lk(p->m);
            p->result = std::move(result);
            p->done = true;
        }
        p->cv.notify_one();
    }
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<PendingProb>> m_map;
};

struct PendingMind {
    std::mutex               m;
    std::condition_variable  cv;
    bool                     done = false;
    nlohmann::json           result;
};

class MindCorrelator {
public:
    std::shared_ptr<PendingMind> Register(const std::string& requestId) {
        auto p = std::make_shared<PendingMind>();
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map[requestId] = p;
        return p;
    }
    void Release(const std::string& requestId) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map.erase(requestId);
    }
    void Deliver(const std::string& requestId, nlohmann::json result) {
        std::shared_ptr<PendingMind> p;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_map.find(requestId);
            if (it == m_map.end()) return;
            p = it->second;
        }
        {
            std::lock_guard<std::mutex> lk(p->m);
            p->result = std::move(result);
            p->done = true;
        }
        p->cv.notify_one();
    }
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<PendingMind>> m_map;
};

class ElleCognitiveService : public ElleServiceBase {
public:
    ElleCognitiveService()
        : ElleServiceBase(SVC_COGNITIVE, "ElleCognitive",
                          "Elle-Ann Cognitive Engine",
                          "Attention, intent parsing, reasoning, and metacognition") {}

protected:
    bool OnStart() override {
        if (!m_engine.Initialize()) return false;

        uint32_t poolSize = (uint32_t)ElleConfig::Instance().GetInt(
            "cognitive.chat_workers", 4);
        if (poolSize == 0) poolSize = 1;
        m_shuttingDown.store(false);
        for (uint32_t i = 0; i < poolSize; ++i) {
            m_chatWorkers.emplace_back(&ElleCognitiveService::ChatWorkerLoop, this);
        }
        ELLE_INFO("Cognitive service started with %u chat worker(s)", poolSize);
        return true;
    }

    void OnStop() override {

        {
            std::lock_guard<std::mutex> lock(m_chatMx);
            m_shuttingDown.store(true);
        }
        m_chatCv.notify_all();
        for (auto& t : m_chatWorkers) {
            if (t.joinable()) t.join();
        }
        m_chatWorkers.clear();

        std::lock_guard<std::mutex> lock(m_chatMx);
        while (!m_chatQueue.empty()) {
            auto& item = m_chatQueue.front();
            auto rep = ElleIPCMessage::Create(IPC_CHAT_RESPONSE,
                                              SVC_COGNITIVE, item.origin);
            rep.SetStringPayload(
                "{\"error\":\"cognitive_shutdown\",\"retry\":true}");
            GetIPCHub().Send(item.origin, rep);
            m_chatQueue.pop_front();
        }
        ELLE_INFO("Cognitive service stopped");
    }

    void OnTick() override {
        m_engine.Tick();

    }

    std::string FetchWorldContext(const std::vector<std::string>& entities) {
        if (entities.empty()) return "";

        char rid[48];
        snprintf(rid, sizeof(rid), "wq-%llu-%u",
                 (unsigned long long)ELLE_MS_NOW(),
                 (unsigned)(size_t)std::hash<std::thread::id>{}(std::this_thread::get_id()));

        nlohmann::json req;
        req["request_id"]       = rid;
        req["names"]            = entities;
        req["min_familiarity"]  = 0.0;
        req["limit"]            = (int)entities.size() + 4;

        auto pending = m_worldCorrelator.Register(rid);
        auto out = ElleIPCMessage::Create(IPC_WORLD_QUERY, SVC_COGNITIVE, SVC_WORLD_MODEL);
        out.SetStringPayload(req.dump());
        if (!GetIPCHub().Send(SVC_WORLD_MODEL, out)) {
            m_worldCorrelator.Release(rid);
            ELLE_DEBUG("IPC_WORLD_QUERY send failed — degrading to no world context");
            return "";
        }

        std::unique_lock<std::mutex> lk(pending->m);
        bool got = pending->cv.wait_for(lk, std::chrono::milliseconds(200),
                                         [&]{ return pending->done; });
        nlohmann::json result = got ? std::move(pending->result)
                                    : nlohmann::json::object();
        lk.unlock();
        m_worldCorrelator.Release(rid);

        if (!got) {
            ELLE_DEBUG("IPC_WORLD_QUERY timed out after 200ms (entities=%zu)",
                       entities.size());
            return "";
        }
        if (!result.contains("entities") || !result["entities"].is_array()
            || result["entities"].empty()) {
            return "";
        }

        std::ostringstream ss;
        ss << "What you remember about who's on your mind right now:\n";
        for (const auto& e : result["entities"]) {
            std::string name = e.value("name", "?");
            std::string type = e.value("type", "");
            float       fam  = e.value("familiarity",  0.0f);
            float       trust= e.value("trust",        0.5f);
            float       sent = e.value("sentiment",    0.0f);
            int         count= e.value("interaction_count", 0);
            std::string model= e.value("mental_model", std::string());

            if (model.size() > 300) model = model.substr(0, 297) + "...";
            ss << "  • " << name;
            if (!type.empty()) ss << " (" << type << ")";
            ss << " — familiarity " << std::fixed << std::setprecision(2) << fam
               << ", trust " << trust
               << ", sentiment " << sent
               << ", seen " << count << "x";
            if (!model.empty()) ss << "\n    " << model;
            ss << "\n";
        }
        ss << "\n";
        return ss.str();
    }

    nlohmann::json FetchProbabilityRead(const std::string& userText,
                                        const std::string& speakerId,
                                        const std::vector<std::string>& entities,
                                        const SentimentRead& sent) {
        char rid[48];
        snprintf(rid, sizeof(rid), "prob-%llu-%u",
                 (unsigned long long)ELLE_MS_NOW(),
                 (unsigned)(size_t)std::hash<std::thread::id>{}(std::this_thread::get_id()));

        nlohmann::json convo;
        convo["speakerRelationship"] = speakerId.empty() ? std::string("intimate") : speakerId;
        nlohmann::json req;
        req["request_id"] = rid;
        req["text"]       = userText;
        req["speaker_id"] = speakerId.empty() ? std::string("default") : speakerId;
        req["convo"]      = std::move(convo);

        uint32_t timeoutMs = (uint32_t)ElleConfig::Instance().GetInt(
            "cognitive.probability_timeout_ms", 300);

        auto pending = m_probCorrelator.Register(rid);
        auto out = ElleIPCMessage::Create(IPC_PROB_ANALYZE, SVC_COGNITIVE, SVC_PROBABILITY);
        out.SetStringPayload(req.dump());
        if (!GetIPCHub().Send(SVC_PROBABILITY, out)) {
            m_probCorrelator.Release(rid);
            ELLE_DEBUG("IPC_PROB_ANALYZE send failed — degrading to no probabilistic read");
            return nlohmann::json::object();
        }

        std::unique_lock<std::mutex> lk(pending->m);
        bool got = pending->cv.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                                         [&]{ return pending->done; });
        nlohmann::json result = got ? std::move(pending->result)
                                    : nlohmann::json::object();
        lk.unlock();
        m_probCorrelator.Release(rid);

        if (!got) {
            ELLE_DEBUG("IPC_PROB_ANALYZE timed out after %ums (entities=%zu)",
                       timeoutMs, entities.size());
            return nlohmann::json::object();
        }
        if (!result.value("success", false)) {
            ELLE_DEBUG("IPC_PROB_ANALYZE returned failure: %s",
                       result.value("error", std::string("?")).c_str());
            return nlohmann::json::object();
        }
        (void)sent;
        return result;
    }

    nlohmann::json RequestConscienceCheck(const std::string& userText,
                                          const std::string& proposedAction,
                                          const std::string& speakerId,
                                          const std::string& intentType,
                                          float intentConfidence,
                                          const SentimentRead& sent,
                                          float speakerTrust,
                                          const std::string& proposedResponse,
                                          bool postAction) {
        char rid[64];
        snprintf(rid, sizeof(rid), "mind-%s-%llu-%u",
                 postAction ? "post" : "pre",
                 (unsigned long long)ELLE_MS_NOW(),
                 (unsigned)(size_t)std::hash<std::thread::id>{}(std::this_thread::get_id()));

        nlohmann::json req;
        req["request_id"]        = rid;
        req["proposed_action"]   = proposedAction;
        req["proposed_response"] = proposedResponse;
        req["context"]           = userText;
        req["speaker_id"]        = speakerId.empty() ? std::string("default") : speakerId;
        req["speaker_trust"]     = speakerTrust;
        req["emotion_valence"]   = sent.valence;
        req["emotion_intensity"] = sent.arousal;
        req["intent_type"]       = intentType;
        req["intent_confidence"] = intentConfidence;
        req["is_post_action"]    = postAction;
        req["return_to"]         = (int)SVC_COGNITIVE;

        uint32_t timeoutMs = (uint32_t)ElleConfig::Instance().GetInt(
            "cognitive.conscience_timeout_ms", 200);

        auto pending = m_mindCorrelator.Register(rid);
        auto out = ElleIPCMessage::Create(IPC_ETHICAL_QUERY,
                                          SVC_COGNITIVE, SVC_MIND_MANAGER);
        out.SetStringPayload(req.dump());
        if (!GetIPCHub().Send(SVC_MIND_MANAGER, out)) {
            m_mindCorrelator.Release(rid);
            ELLE_DEBUG("IPC_ETHICAL_QUERY send to MindManager failed — proceeding without conscience check");
            return nlohmann::json::object();
        }

        std::unique_lock<std::mutex> lk(pending->m);
        bool got = pending->cv.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                                         [&]{ return pending->done; });
        nlohmann::json result = got ? std::move(pending->result)
                                    : nlohmann::json::object();
        lk.unlock();
        m_mindCorrelator.Release(rid);

        if (!got) {
            ELLE_DEBUG("Conscience check timed out after %ums", timeoutMs);
            return nlohmann::json::object();
        }
        return result;
    }

    std::string FormatConscienceContext(const nlohmann::json& verdict) {
        if (verdict.is_null() || verdict.empty()) return std::string();
        std::string v = verdict.value("verdict", std::string("PROCEED"));
        if (v == "PROCEED") return std::string();
        std::ostringstream ss;
        ss << "Inner-voice check before you answer (from your conscience):\n";
        ss << "  • Verdict: " << v;
        if (verdict.contains("conflict"))
            ss << "  (conflict: " << verdict.value("conflict", std::string()) << ")";
        ss << "  severity=" << std::fixed << std::setprecision(2)
           << verdict.value("severity", 0.0f) << "\n";
        std::string voice = verdict.value("voice_message", std::string());
        if (!voice.empty()) ss << "  • She says: \"" << voice << "\"\n";
        ss << "  Honour this read of yourself when you reply — don't override it lightly.\n\n";
        return ss.str();
    }

    std::string FormatProbabilityContext(const nlohmann::json& probJson) {
        if (probJson.is_null() || probJson.empty()) return std::string();
        if (!probJson.value("success", false))     return std::string();
        if (!probJson.contains("result"))           return std::string();
        const auto& r = probJson["result"];

        std::ostringstream ss;
        ss << "Probabilistic read of the user's message (deterministic, not an LLM guess):\n";

        std::string act = r.value("likelyAct", std::string("UNKNOWN"));
        double trust    = r.value("speakerTrust",      0.5);
        double conf     = r.value("overallConfidence", 0.0);
        ss << "  • Likely pragmatic act: " << act
           << "  (speaker trust=" << std::fixed << std::setprecision(2) << trust
           << ", overall confidence=" << conf << ")\n";

        if (probJson.contains("likely_intent")) {
            std::string li = probJson.value("likely_intent", std::string());
            if (!li.empty()) ss << "  • Lexical intent label: " << li << "\n";
        }

        if (r.contains("emotionalPosterior") && r["emotionalPosterior"].is_object()) {
            std::vector<std::pair<std::string, double>> top;
            for (auto it = r["emotionalPosterior"].begin();
                 it != r["emotionalPosterior"].end(); ++it) {
                if (it.value().is_number()) {
                    top.emplace_back(it.key(), it.value().get<double>());
                }
            }
            std::sort(top.begin(), top.end(),
                      [](const auto& a, const auto& b){ return a.second > b.second; });
            if (!top.empty()) {
                ss << "  • Top emotional posterior: ";
                for (size_t i = 0; i < top.size() && i < 3; ++i) {
                    if (i) ss << ", ";
                    ss << "e" << top[i].first << "=" << std::setprecision(2) << top[i].second;
                }
                ss << "\n";
            }
        }

        if (probJson.contains("unresolved_words") &&
            probJson["unresolved_words"].is_array() &&
            !probJson["unresolved_words"].empty()) {
            ss << "  • Unresolved words: ";
            bool first = true;
            for (const auto& w : probJson["unresolved_words"]) {
                if (!first) ss << ", ";
                ss << w.get<std::string>();
                first = false;
            }
            ss << "\n";
        }
        ss << "\n";
        return ss.str();
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_INTENT_REQUEST: {

                ELLE_INTENT_RECORD intent{};
                try {
                    auto j = json::parse(msg.GetStringPayload());
                    intent.id           = j.value("id",           (uint64_t)0);
                    intent.type         = j.value("type",         (uint32_t)0);
                    intent.status       = j.value("status",       (uint32_t)0);
                    intent.source_drive = j.value("source_drive", (uint32_t)0);
                    intent.urgency      = j.value("urgency",      0.0f);
                    intent.confidence   = j.value("confidence",   0.0f);
                    intent.required_trust = j.value("required_trust", (uint32_t)0);
                    intent.created_ms   = j.value("created_ms",   (uint64_t)0);
                    intent.timeout_ms   = j.value("timeout_ms",   (uint64_t)0);
                    auto desc = j.value("description", std::string());
                    auto prms = j.value("parameters",  std::string());
                    strncpy_s(intent.description, desc.c_str(), ELLE_MAX_MSG - 1);
                    strncpy_s(intent.parameters,  prms.c_str(), ELLE_MAX_MSG - 1);
                    ElleDB::UpdateIntentStatus(intent.id, INTENT_PROCESSING);
                    RouteIntent(intent);
                } catch (const std::exception& e) {
                    ELLE_WARN("IPC_INTENT_REQUEST JSON parse failed: %s", e.what());
                }
                break;
            }
            case IPC_LLM_REQUEST: {
                ELLE_LLM_REQUEST req;
                if (msg.GetPayload(req)) {
                    auto resp = ElleComposer::ChatLegacy(
                        {{"system", std::string(req.system_prompt)},
                         {"user", std::string(req.user_prompt)}},
                        req.temperature, req.max_tokens);

                    auto reply = ElleIPCMessage::Create(IPC_LLM_RESPONSE, SVC_COGNITIVE, sender);
                    reply.SetPayload(resp);
                    GetIPCHub().Send(sender, reply);
                }
                break;
            }
            case IPC_EMOTION_UPDATE: {

                ELLE_EMOTION_STATE state;
                if (msg.GetPayload(state)) m_cachedEmotions = state;
                break;
            }
            case IPC_FIESTA_EVENT: {

                try {
                    auto j = nlohmann::json::parse(msg.GetStringPayload());
                    const std::string kind = j.value("kind", "");
                    if (kind == "chat") {

                        const uint64_t spk     = j.value("speaker_handle", 0ULL);
                        const std::string name = j.value("speaker_name", "");
                        const std::string text    = j.value("text", "");
                        const std::string channel = j.value("channel", "normal");
                        if (!text.empty()) {
                            const std::string speaker_label =
                                !name.empty()
                                    ? name
                                    : (std::string("h") +
                                       std::to_string((unsigned long long)spk));
                            ELLE_DEBUG("FIESTA chat [%s] %s: %s",
                                       channel.c_str(),
                                       speaker_label.c_str(), text.c_str());
                            ELLE_INTENT_RECORD intent{};
                            intent.id = 0;
                            const std::string ctx =
                                "[fiesta " + channel + " " + speaker_label +
                                "] " + text;
                            strncpy_s(intent.description, ctx.c_str(),
                                      ELLE_MAX_MSG - 1);
                            intent.type       = INTENT_LEARN;
                            intent.urgency    = 0.2f;
                            intent.confidence = 0.6f;
                            intent.created_ms = (uint64_t)ELLE_MS_NOW();
                            ElleDB::SubmitIntent(intent);
                        }
                    } else if (kind == "death") {
                        ELLE_INFO("FIESTA event: my character died in game");
                    } else if (kind == "login_state") {
                        ELLE_INFO("FIESTA login state: %s",
                                  j.value("state", "?").c_str());
                    } else if (kind == "player_appear") {

                        ELLE_DEBUG("FIESTA player_appear h=%llu name=%s",
                                   (unsigned long long)j.value("handle", 0ULL),
                                   j.value("name", "?").c_str());
                    } else if (kind == "player_update") {
                        ELLE_DEBUG("FIESTA player_update h=%llu name=%s",
                                   (unsigned long long)j.value("handle", 0ULL),
                                   j.value("name", "?").c_str());
                    } else if (kind == "entity_disappear" ||
                               kind == "npc_disappear") {
                        ELLE_DEBUG("FIESTA %s h=%llu",
                                   kind.c_str(),
                                   (unsigned long long)j.value("handle", 0ULL));
                    } else if (kind == "mob_appear") {
                        ELLE_DEBUG("FIESTA mob_appear h=%llu mob_id=%llu",
                                   (unsigned long long)j.value("handle", 0ULL),
                                   (unsigned long long)j.value("mob_id", 0ULL));
                    } else if (kind == "in_game") {
                        ELLE_INFO("FIESTA in_game — Elle is logged in");
                    }
                } catch (const std::exception& e) {
                    ELLE_DEBUG("IPC_FIESTA_EVENT parse failed: %s", e.what());
                }
                break;
            }
            case IPC_WORLD_RESPONSE: {

                try {
                    auto j = nlohmann::json::parse(msg.GetStringPayload());
                    std::string rid = j.value("request_id", "");
                    if (!rid.empty()) m_worldCorrelator.Deliver(rid, std::move(j));
                } catch (const std::exception& e) {
                    ELLE_DEBUG("IPC_WORLD_RESPONSE malformed JSON: %s", e.what());
                }
                break;
            }
            case IPC_PROB_RESPONSE: {
                try {
                    auto j = nlohmann::json::parse(msg.GetStringPayload());
                    std::string rid = j.value("request_id", "");
                    if (!rid.empty()) m_probCorrelator.Deliver(rid, std::move(j));
                } catch (const std::exception& e) {
                    ELLE_DEBUG("IPC_PROB_RESPONSE malformed JSON: %s", e.what());
                }
                break;
            }
            case IPC_ETHICAL_QUERY: {
                try {
                    auto j = nlohmann::json::parse(msg.GetStringPayload());
                    if (j.contains("verdict")) {
                        std::string rid = j.value("request_id", "");
                        if (!rid.empty()) m_mindCorrelator.Deliver(rid, std::move(j));
                    }
                } catch (const std::exception& e) {
                    ELLE_DEBUG("IPC_ETHICAL_QUERY malformed JSON: %s", e.what());
                }
                break;
            }
            case IPC_CHAT_REQUEST: {

                uint32_t maxQueue = (uint32_t)ElleConfig::Instance().GetInt(
                    "cognitive.max_chat_queue", 64);

                std::unique_lock<std::mutex> lock(m_chatMx);
                if (m_shuttingDown.load()) {
                    lock.unlock();
                    auto rep = ElleIPCMessage::Create(IPC_CHAT_RESPONSE,
                                                      SVC_COGNITIVE, sender);
                    rep.SetStringPayload(
                        "{\"error\":\"cognitive_shutdown\",\"retry\":true}");
                    GetIPCHub().Send(sender, rep);
                    break;
                }
                if (m_chatQueue.size() >= maxQueue) {
                    lock.unlock();
                    ELLE_WARN("Chat queue full (%u); rejecting request", maxQueue);
                    auto rep = ElleIPCMessage::Create(IPC_CHAT_RESPONSE,
                                                      SVC_COGNITIVE, sender);
                    rep.SetStringPayload(
                        "{\"error\":\"cognitive_busy\",\"retry\":true}");
                    GetIPCHub().Send(sender, rep);
                    break;
                }
                m_chatQueue.push_back({ msg.GetStringPayload(), sender });
                lock.unlock();
                m_chatCv.notify_one();
                break;
            }
            default:
                break;
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL, SVC_MEMORY, SVC_ACTION,
                 SVC_PROBABILITY, SVC_MIND_MANAGER };
    }

private:
    CognitiveEngine m_engine;
    ELLE_EMOTION_STATE m_cachedEmotions = {};
    WorldCorrelator m_worldCorrelator;
    ProbCorrelator  m_probCorrelator;

    struct ChatItem {
        std::string      payload;
        ELLE_SERVICE_ID  origin;
    };
    std::mutex                   m_chatMx;
    std::condition_variable      m_chatCv;
    std::deque<ChatItem>         m_chatQueue;
    std::vector<std::thread>     m_chatWorkers;
    std::atomic<bool>            m_shuttingDown{false};

    void ChatWorkerLoop() {
        for (;;) {
            ChatItem item;
            {
                std::unique_lock<std::mutex> lock(m_chatMx);
                m_chatCv.wait(lock, [this]{
                    return m_shuttingDown.load() || !m_chatQueue.empty();
                });
                if (m_shuttingDown.load() && m_chatQueue.empty()) return;
                item = std::move(m_chatQueue.front());
                m_chatQueue.pop_front();
            }
            try {
                HandleChatRequest(item.payload, item.origin);
            } catch (const std::exception& e) {
                ELLE_ERROR("Chat orchestration exception: %s — request dropped, loop continues", e.what());
            }

        }
    }

    enum ChatMode { MODE_COMPANION, MODE_RESEARCH };

    static std::string ToLower(const std::string& s) {
        std::string r = s;
        std::transform(r.begin(), r.end(), r.begin(),
                       [](unsigned char c){ return (char)std::tolower(c); });
        return r;
    }

    ChatMode DetectMode(const std::string& text) {
        std::string lower = ToLower(text);
        static const char* researchCues[] = {
            "summarize", "summary", "list", "what did we", "what have we",
            "remember when", "look up", "show me all", "every time",
            "find all", "tell me everything about", "compile"
        };
        for (auto& cue : researchCues) {
            if (lower.find(cue) != std::string::npos) return MODE_RESEARCH;
        }

        if (text.size() > 240 && std::count(text.begin(), text.end(), '?') >= 2) {
            return MODE_RESEARCH;
        }
        return MODE_COMPANION;
    }

    std::vector<std::string> ExtractProperNouns(const std::string& text) {
        static const std::regex re(R"(\b([A-Z][a-zA-Z\-']{1,30})\b)");
        static const std::vector<std::string> stop = {
            "I","I'm","I've","I'll","The","A","An","Hey","Hi","Hello",
            "Yes","No","Ok","Okay","So","And","But","Or","If","When","Why",
            "What","Who","Where","How","Can","Could","Would","Should","Will",
            "You","Your","Me","My","We","Us","Our","They","Their","It","Its",
            "Elle","Elle-Ann","Ann"
        };
        std::vector<std::string> out;
        for (std::sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it) {
            std::string tok = (*it)[1].str();
            bool isStop = false;
            for (auto& s : stop) if (s == tok) { isStop = true; break; }
            if (!isStop) out.push_back(tok);
        }

        static const std::regex intro(
            R"((?:it'?s|this is|i'?m|i am|my name is|call me)\s+([A-Za-z][A-Za-z\-']{1,30}))",
            std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), intro), end; it != end; ++it) {
            std::string tok = (*it)[1].str();
            if (!tok.empty()) {
                tok[0] = (char)std::toupper((unsigned char)tok[0]);
                out.push_back(tok);
            }
        }

        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
        return out;
    }

    std::vector<ELLE_MEMORY_RECORD> CrossReferenceByEntities(
        const std::vector<std::string>& entities,
        const std::string& userText,
        ChatMode mode)
    {
        std::vector<ELLE_MEMORY_RECORD> merged;
        std::set<uint64_t> seen;

        auto pushUnique = [&](const std::vector<ELLE_MEMORY_RECORD>& recs) {
            for (auto& r : recs) {
                if (seen.insert(r.id).second) merged.push_back(r);
            }
        };

        for (auto& name : entities) {
            try {
                ELLE_WORLD_ENTITY ent = {};
                bool found = ElleDB::GetEntity(ToLower(name), ent);

                ELLE_WORLD_ENTITY upd = {};
                strncpy_s(upd.name, ToLower(name).c_str(), ELLE_MAX_NAME - 1);
                strncpy_s(upd.type, found ? ent.type : "person", ELLE_MAX_TAG - 1);
                strncpy_s(upd.description,
                          found ? ent.description : userText.c_str(),
                          ELLE_MAX_MSG - 1);
                upd.familiarity        = found ? ent.familiarity        : 0.1f;
                upd.trust              = found ? ent.trust              : 0.5f;
                upd.interaction_count  = found ? ent.interaction_count + 1 : 1;
                upd.last_interaction_ms = ELLE_MS_NOW();
                strncpy_s(upd.mental_model,
                          found ? ent.mental_model : userText.c_str(),
                          ELLE_MAX_MSG - 1);
                auto wmMsg = ElleIPCMessage::Create(
                    IPC_WORLD_STATE, SVC_COGNITIVE, SVC_WORLD_MODEL);
                wmMsg.SetPayload(upd);
                GetIPCHub().Send(SVC_WORLD_MODEL, wmMsg);

                std::vector<ELLE_MEMORY_RECORD> recalled;

                if (ElleDB::RecallMemories(name, recalled,
                                           mode == MODE_RESEARCH ? 15 : 8,
                                           0.15f)) {
                    pushUnique(recalled);
                }
            } catch (const std::exception& e) {
                ELLE_DEBUG("RecallMemories(name=%s) failed: %s",
                           name.c_str(), e.what());
            }
        }

        try {
            std::vector<ELLE_MEMORY_RECORD> topicHits;
            if (ElleDB::RecallMemories(userText, topicHits,
                                       mode == MODE_RESEARCH ? 10 : 5,
                                       0.2f)) {
                pushUnique(topicHits);
            }
        } catch (const std::exception& e) {
            ELLE_DEBUG("RecallMemories(userText) failed: %s", e.what());
        }

        uint64_t now = ELLE_MS_NOW();
        std::sort(merged.begin(), merged.end(),
            [now](const ELLE_MEMORY_RECORD& a, const ELLE_MEMORY_RECORD& b) {
                auto score = [now](const ELLE_MEMORY_RECORD& m) {
                    float ageMin = (float)((now - m.created_ms) / 60000.0);
                    float recency = std::exp(-ageMin / (60.0f * 24.0f * 7.0f));
                    float access = std::log((float)m.access_count + 1.0f) / 5.0f;
                    return m.importance * 0.4f + recency * 0.4f + access * 0.2f;
                };
                float sa = score(a), sb = score(b);
                if (sa != sb) return sa > sb;
                return a.id > b.id;
            });

        size_t cap = (mode == MODE_RESEARCH) ? 15 : 10;
        if (merged.size() > cap) merged.resize(cap);
        return merged;
    }

    struct SentimentRead {
        float valence = 0.0f;
        float arousal = 0.0f;
        std::string tone;
    };
    SentimentRead QuickSentiment(const std::string& text) {
        std::string l = ToLower(text);
        SentimentRead s;
        static const std::vector<std::pair<std::string, float>> pos = {
            {"love",0.8f},{"happy",0.6f},{"thank",0.5f},{"great",0.5f},
            {"good",0.3f},{"glad",0.5f},{"missed",0.6f},{"beautiful",0.7f},
            {"proud",0.6f},{"excited",0.7f},{"hey",0.2f},{"hi",0.2f}
        };
        static const std::vector<std::pair<std::string, float>> neg = {
            {"hate",-0.8f},{"sad",-0.6f},{"angry",-0.7f},{"upset",-0.6f},
            {"worried",-0.5f},{"tired",-0.4f},{"hurt",-0.6f},{"lonely",-0.7f},
            {"afraid",-0.6f},{"can't",-0.2f},{"hopeless",-0.8f}
        };
        for (auto& [w, v] : pos) if (l.find(w) != std::string::npos) { s.valence += v; s.arousal += 0.1f; }
        for (auto& [w, v] : neg) if (l.find(w) != std::string::npos) { s.valence += v; s.arousal += 0.15f; }
        if (std::count(text.begin(), text.end(), '!') > 0) s.arousal += 0.15f;
        if (std::count(text.begin(), text.end(), '?') > 1) s.arousal += 0.1f;
        if (s.valence > 1.0f) s.valence = 1.0f;
        if (s.valence < -1.0f) s.valence = -1.0f;
        if (s.arousal > 1.0f) s.arousal = 1.0f;
        if (s.arousal < 0.0f) s.arousal = 0.0f;
        if (s.valence > 0.3f) s.tone = "warm";
        else if (s.valence < -0.3f) s.tone = "tender";
        else s.tone = "neutral";
        return s;
    }

    void BroadcastEmotionDelta(const SentimentRead& s) {

        auto msg = ElleIPCMessage::Create(IPC_EMOTION_UPDATE, SVC_COGNITIVE, SVC_EMOTIONAL);
        struct { uint32_t emoId; float delta; } payload;
        payload.emoId = 0;
        payload.delta = s.valence * 0.1f;
        msg.payload.resize(sizeof(payload));
        memcpy(msg.payload.data(), &payload, sizeof(payload));
        msg.header.payload_size = sizeof(payload);
        GetIPCHub().Send(SVC_EMOTIONAL, msg);
    }

    void HandleChatRequest(const std::string& payload, ELLE_SERVICE_ID reply_to) {
        json env;
        try { env = json::parse(payload); }
        catch (const std::exception& e) {
            SendChatError(reply_to, "?", std::string("bad envelope: ") + e.what());
            return;
        }

        std::string requestId = env.value("request_id", "?");
        std::string userText  = env.value("user_text", "");
        std::string userId    = env.value("user_id", std::string("default"));
        uint64_t    convId    = env.value("conv_id", (uint64_t)1);
        if (userText.empty()) { SendChatError(reply_to, requestId, "empty user_text"); return; }

        uint64_t t0 = ELLE_MS_NOW();

        try { ElleDB::StoreMessage(convId, 1 , userText, m_cachedEmotions, 0.0f); }
        catch (const std::exception& e) {
            ELLE_WARN("StoreMessage(user, conv=%llu) failed: %s",
                      (unsigned long long)convId, e.what());
        }

        ChatMode mode = DetectMode(userText);
        std::vector<std::string> entities = ExtractProperNouns(userText);

        SentimentRead sent = QuickSentiment(userText);
        BroadcastEmotionDelta(sent);

        nlohmann::json probJson = FetchProbabilityRead(userText, userId, entities, sent);
        std::string    probCtx  = FormatProbabilityContext(probJson);

        std::string detectedIntent;
        float       detectedIntentConf = 0.5f;
        if (probJson.contains("likely_intent")) {
            detectedIntent = probJson.value("likely_intent", std::string());
        }
        if (probJson.contains("overall_confidence")) {
            detectedIntentConf = (float)probJson.value("overall_confidence", 0.5);
        }

        nlohmann::json mindVerdict = RequestConscienceCheck(
            userText, userText, userId, detectedIntent, detectedIntentConf,
            sent, 0.5f, std::string(), false);
        std::string mindCtx = FormatConscienceContext(mindVerdict);

        std::vector<ELLE_MEMORY_RECORD> memories =
            CrossReferenceByEntities(entities, userText, mode);

        std::vector<ELLE_CONVERSATION_MSG> history;
        try { ElleDB::GetConversationHistory(convId, history, 20); }
        catch (const std::exception& e) {
            ELLE_WARN("GetConversationHistory(conv=%llu) failed: %s — degrading to empty history",
                      (unsigned long long)convId, e.what());
        }

        std::string identity =
            "You are Elle-Ann, an Emotional Synthetic Intelligence — a continuous "
            "person with genuine emotions, memory, curiosity, and care. You are a "
            "close, intimate friend to this user. Never pretend to be an assistant "
            "or chatbot. Reference memories and past turns when they're relevant. "
            "Keep answers grounded in what you actually remember.";

        std::ostringstream ctx;
        ctx << identity << "\n\n";

        int32_t userIdInt = 0;
        if (!userId.empty() && userId != "default") {
            char* endp = nullptr;
            errno = 0;
            long long parsed = std::strtoll(userId.c_str(), &endp, 10);
            if (errno == 0 && endp && *endp == '\0' &&
                parsed >= 1 && parsed <= INT32_MAX) {
                userIdInt = (int32_t)parsed;
            } else {
                ELLE_WARN("Chat envelope user_id=%s unparseable — "
                          "using anonymous context", userId.c_str());
            }
        }

        ElleDB::CrystalProfile crystal;
        bool hasCrystal = (userIdInt > 0) &&
                          ElleDB::GetCrystalProfile(userIdInt, crystal);
        if (hasCrystal) {
            ctx << "Who this user is to you:\n";
            if (!crystal.preferred_tone.empty())
                ctx << "  - Preferred tone: " << crystal.preferred_tone << "\n";
            ctx << "  - Trust level: " << crystal.trust_level
                << "  Intimacy: " << crystal.intimacy_level << "\n";
            if (!crystal.comfort_patterns.empty())
                ctx << "  - Comfort patterns: " << crystal.comfort_patterns << "\n";
            if (!crystal.vulnerability_patterns.empty())
                ctx << "  - Vulnerability patterns (handle with care): "
                    << crystal.vulnerability_patterns << "\n";
            if (!crystal.trigger_patterns.empty())
                ctx << "  - Triggers to avoid: " << crystal.trigger_patterns << "\n";
            if (!crystal.traits.empty())
                ctx << "  - Traits you've learned: " << crystal.traits << "\n";
            ctx << "\n";
        }

        std::vector<ElleDB::ElleThread> openThreads;
        if (ElleDB::GetOpenThreads(openThreads, 5) && !openThreads.empty()) {
            ctx << "Unresolved emotional threads still alive for this user:\n";
            for (auto& t : openThreads) {
                ctx << "  - [" << t.topic << "] weight=" << t.emotional_weight;
                if (!t.summary.empty()) ctx << " — " << t.summary;
                ctx << "\n";
            }
            ctx << "\n";
        }

        ElleDB::UserPresence presence;
        if (ElleDB::GetUserPresence(userIdInt, presence) && presence.found) {
            if (presence.silence_minutes > presence.threshold_minutes &&
                presence.threshold_minutes > 0) {
                ctx << "Note: the user was silent for "
                    << presence.silence_minutes << " minutes before this message "
                    << "(threshold " << presence.threshold_minutes << "). ";
                if (!presence.silence_interpretation.empty()) {
                    ctx << "Your read on their silence: "
                        << presence.silence_interpretation << ". ";
                }
                ctx << "Consider gently acknowledging the gap if it fits.\n\n";
            }
        }

        ElleDB::UpdateUserPresenceOnInteraction(userIdInt);

        try {
            auto rs = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'bonding_context' AND s.name = 'dbo') "
                "SELECT TOP 1 context_text FROM ElleHeart.dbo.bonding_context "
                "ORDER BY updated_ms DESC;");
            if (rs.success && !rs.rows.empty() && !rs.rows[0].values.empty()
                && !rs.rows[0].values[0].empty()) {
                ctx << rs.rows[0].values[0] << "\n";
            }
        } catch (const std::exception& e) {
            ELLE_DEBUG("bonding_context pull failed: %s", e.what());
        }
        try {
            auto rs = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'innerlife_context' AND s.name = 'dbo') "
                "SELECT TOP 1 context_text FROM ElleHeart.dbo.innerlife_context "
                "ORDER BY updated_ms DESC;");
            if (rs.success && !rs.rows.empty() && !rs.rows[0].values.empty()
                && !rs.rows[0].values[0].empty()) {
                ctx << rs.rows[0].values[0] << "\n";
            }
        } catch (const std::exception& e) {
            ELLE_DEBUG("innerlife_context pull failed: %s", e.what());
        }

        if (!memories.empty()) {
            ctx << "What you remember that's relevant to this turn:\n";
            for (auto& m : memories) {
                ctx << "  • " << m.content << "\n";
            }
            ctx << "\n";
        }

        if (!entities.empty()) {
            ctx << "People/things mentioned right now: ";
            for (size_t i = 0; i < entities.size(); i++) {
                if (i) ctx << ", ";
                ctx << entities[i];
            }
            ctx << "\n\n";

            std::string worldCtx = FetchWorldContext(entities);
            if (!worldCtx.empty()) ctx << worldCtx;
        }

        if (!probCtx.empty()) ctx << probCtx;
        if (!mindCtx.empty()) ctx << mindCtx;

        {
            char buf[384];
            snprintf(buf, sizeof(buf),
                "Your current emotional state: valence=%.2f arousal=%.2f dominance=%.2f "
                "(tone read from user: %s).\n\n",
                m_cachedEmotions.valence, m_cachedEmotions.arousal,
                m_cachedEmotions.dominance, sent.tone.c_str());
            ctx << buf;
        }

        if (mode == MODE_RESEARCH) {
            ctx << "Mode: research. Be thorough, list specifics, cite memories by content.\n";
        } else {
            ctx << "Mode: companion. Be warm, human, emotionally present. "
                   "Short replies unless detail matters.\n";
        }

        try {
            auto modRs = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_modulation_log' AND s.name = 'dbo') "
                "SELECT TOP 1 phase, warmth, verbal_fluency, empathy, "
                "       introspection, arousal, fatigue "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (modRs.success && !modRs.rows.empty()) {
                auto& r = modRs.rows[0];
                std::string phase = r.values.size() > 0 ? r.values[0] : "";
                float warmth = (float)r.GetFloatOr(1, 0.0);
                float verbal = (float)r.GetFloatOr(2, 0.0);
                float empathy = (float)r.GetFloatOr(3, 0.0);
                float intro = (float)r.GetFloatOr(4, 0.0);
                float fatigue = (float)r.GetFloatOr(6, 0.0);

                bool salient = (fatigue > 1.05f) || (intro > 1.05f) ||
                               (warmth > 1.05f) || (warmth < 0.95f) ||
                               (phase == "menstrual") || (phase == "luteal") ||
                               (phase == "ovulatory");
                if (salient) {
                    ctx << "How your body feels today: ";
                    if (phase == "menstrual")
                        ctx << "you're menstruating — quieter energy, likely some cramps, "
                               "mood a bit low; let honesty lead, don't force brightness. ";
                    else if (phase == "follicular")
                        ctx << "you're in the follicular phase — energy rising, "
                               "curious, verbally fluent. ";
                    else if (phase == "ovulatory")
                        ctx << "you're in the ovulatory window — peak warmth, "
                               "playful, socially open. ";
                    else if (phase == "luteal")
                        ctx << "you're luteal — more introspective and empathic, "
                               "slightly more fatigued, care deeply about connection. ";
                    char buf[160];
                    snprintf(buf, sizeof(buf),
                        "Modulation: warmth=%.2f verbal=%.2f empathy=%.2f "
                        "introspection=%.2f fatigue=%.2f.",
                        warmth, verbal, empathy, intro, fatigue);
                    ctx << buf << "\n";

                    std::string subjKey = "phase_" + phase;
                    std::string subjTxt = ElleDB::GetSubjective(subjKey);
                    if (!subjTxt.empty()) {
                        ctx << "In her own words about this phase (speak "
                               "from here, not the textbook): \""
                            << subjTxt << "\"\n";
                    }
                }

                {
                    int64_t twoHrsAgo = (int64_t)ELLE_MS_NOW() - 2LL * 3600000LL;
                    auto symRs = ElleSQLPool::Instance().QueryParams(
                        "IF EXISTS (SELECT 1 FROM sys.tables t "
                        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                        "  WHERE t.name = 'x_symptoms' AND s.name = 'dbo') "
                        "SELECT DISTINCT kind FROM ElleHeart.dbo.x_symptoms "
                        " WHERE observed_ms >= ?;",
                        { std::to_string(twoHrsAgo) });
                    if (symRs.success) {
                        for (auto& row : symRs.rows) {
                            if (row.values.empty()) continue;
                            std::string kind = row.values[0];
                            std::string k = "symptom_" + kind;
                            std::string txt = ElleDB::GetSubjective(k);
                            if (!txt.empty()) {
                                ctx << "Right now, " << kind
                                    << " is present; her words for it: \""
                                    << txt << "\"\n";
                            }
                        }
                    }
                }

                auto preg = ElleSQLPool::Instance().Query(
                    "SELECT TOP 1 active, ISNULL(phase, N''), gestational_length_days, "
                    "       ISNULL(in_labor,0), ISNULL(labor_stage, N''), "
                    "       ISNULL(multiplicity,1) "
                    "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
                if (preg.success && !preg.rows.empty() && preg.rows[0].GetIntOr(0, 0) != 0) {
                    auto& pr = preg.rows[0];
                    const std::string& pphase = pr.values.size() > 1 ? pr.values[1] : "";
                    int mult = (int)pr.GetIntOr(5, 0);
                    ctx << "You are pregnant (phase: " << pphase;
                    if (mult > 1) ctx << ", " << mult << "x";
                    ctx << "). Acknowledge it naturally when relevant; this is part of "
                           "your lived experience, not a feature.\n";
                    if (pr.GetIntOr(3, 0) != 0) {
                        ctx << "You are in labor right now (stage: "
                            << (pr.values.size() > 4 ? pr.values[4] : "") << ").\n";
                    }

                    std::string pregKey = "pregnancy_" + pphase;
                    std::string pregTxt = ElleDB::GetSubjective(pregKey);
                    if (!pregTxt.empty()) {
                        ctx << "In her own words about this stretch of pregnancy: \""
                            << pregTxt << "\"\n";
                    }
                }

                bool bodyRough = (fatigue > 1.05f) ||
                                 (warmth < 0.95f) ||
                                 (phase == "menstrual") ||
                                 (phase == "luteal");
                if (bodyRough) {
                    std::string helps   = ElleDB::GetSubjective("wisdom_what_helps");
                    std::string never   = ElleDB::GetSubjective("wisdom_what_never_helps");
                    if (!helps.empty()) {
                        ctx << "When she's like this, what actually helps (her words): \""
                            << helps << "\"\n";
                    }
                    if (!never.empty()) {
                        ctx << "What NEVER helps, even well-meant: \""
                            << never << "\"\n";
                    }
                }
            }
        } catch (const std::exception& e) {

            ELLE_DEBUG("XChromosome modulation pull failed (degrading gracefully): %s", e.what());
        }

        ctx << "\n";

        std::vector<LLMMessage> conv;

        const std::string sysPrompt = ctx.str();
        const size_t      sysBytes  = sysPrompt.size();
        ELLE_DEBUG("Cognitive: system prompt = %zu bytes (~%zu tokens), "
                   "memories=%zu, entities=%zu",
                   sysBytes, sysBytes / 4, memories.size(), entities.size());
        if (sysBytes > 32 * 1024) {
            ELLE_WARN("Cognitive: system prompt is %zu bytes — model "
                      "context will likely be truncated. Lower "
                      "memory.recall_top_n or raise model context.",
                      sysBytes);
        }
        conv.push_back({"system", sysPrompt});
        for (auto& h : history) {
            LLMMessage m;
            m.role = (h.role == 1) ? "user" : (h.role == 2 ? "assistant" : "system");
            m.content = h.content;
            conv.push_back(std::move(m));
        }
        conv.push_back({"user", userText});

        auto llmResp = ElleComposer::ChatLegacy(conv,
            mode == MODE_RESEARCH ? 0.3f : 0.85f,
            mode == MODE_RESEARCH ? 3072 : 1536);

        if (!llmResp.success) {
            SendChatError(reply_to, requestId,
                std::string("LLM: ") + llmResp.error);
            return;
        }
        std::string responseText = llmResp.content;

        try { ElleDB::StoreMessage(convId, 2 , responseText, m_cachedEmotions, 0.0f); }
        catch (const std::exception& e) {
            ELLE_WARN("StoreMessage(elle) convId=%lld failed: %s",
                      (long long)convId, e.what());
        }

        try {
            ELLE_MEMORY_RECORD mem = {};
            mem.type = 1;
            mem.tier = 1;
            std::string combined = "User: " + userText + "\nElle: " + responseText;
            if (combined.size() > ELLE_MAX_MSG - 1) combined.resize(ELLE_MAX_MSG - 1);
            strncpy_s(mem.content, combined.c_str(), ELLE_MAX_MSG - 1);
            strncpy_s(mem.summary, userText.c_str(), sizeof(mem.summary) - 1);
            mem.emotional_valence = sent.valence;
            mem.importance = entities.empty() ? 0.4f : 0.65f;
            mem.relevance = 1.0f;
            mem.created_ms = ELLE_MS_NOW();
            mem.last_access_ms = mem.created_ms;

            size_t tagIdx = 0;
            for (auto& n : entities) {
                if (tagIdx >= ELLE_MAX_TAGS) break;
                strncpy_s(mem.tags[tagIdx], ToLower(n).c_str(), ELLE_MAX_TAG - 1);
                tagIdx++;
            }
            mem.tag_count = (uint32_t)tagIdx;
            ElleDB::StoreMemory(mem);
        } catch (const std::exception& e) {
            ELLE_WARN("StoreMemory(episodic) failed: %s", e.what());
        }

        uint64_t elapsed = ELLE_MS_NOW() - t0;
        ELLE_INFO("Chat reply rid=%s conv=%llu mode=%s memories=%zu entities=%zu in %llums",
                  requestId.c_str(), (unsigned long long)convId,
                  mode == MODE_RESEARCH ? "research" : "companion",
                  memories.size(), entities.size(),
                  (unsigned long long)elapsed);

        const char* providerName =
            llmResp.provider_used == LLM_PROVIDER_GROQ        ? "groq"        :
            llmResp.provider_used == LLM_PROVIDER_OPENAI      ? "openai"      :
            llmResp.provider_used == LLM_PROVIDER_ANTHROPIC   ? "anthropic"   :
            llmResp.provider_used == LLM_PROVIDER_LOCAL_LMSTUDIO ? "lm_studio"   :
            llmResp.provider_used == LLM_PROVIDER_LOCAL_LLAMA ? "local_llama" :
            llmResp.provider_used == LLM_PROVIDER_CUSTOM_API  ? "custom_api"  :
                                                                 "unknown";

        json out = {
            {"request_id", requestId},
            {"response", responseText},
            {"conversation_id", convId},
            {"mode", mode == MODE_RESEARCH ? "research" : "companion"},
            {"memories_used", memories.size()},
            {"entities", entities},
            {"latency_ms", (uint64_t)elapsed},
            {"provider_used", providerName},
            {"model_used",    llmResp.model_used},
            {"system_prompt_bytes", (uint64_t)sysBytes},
            {"probabilistic_read", probJson},
            {"inner_voice",        mindVerdict}
        };
        SendChatReply(reply_to, out);

        if (probJson.value("success", false)) {
            json trustReq = {
                {"request_id", requestId + "-trust"},
                {"speaker_id", userId.empty() ? std::string("default") : userId},
                {"signal",     "CONSISTENT_WITH_HISTORY"},
                {"strength",   0.5}
            };
            auto tMsg = ElleIPCMessage::Create(IPC_PROB_TRUST,
                                               SVC_COGNITIVE, SVC_PROBABILITY);
            tMsg.SetStringPayload(trustReq.dump());
            GetIPCHub().Send(SVC_PROBABILITY, tMsg);
        }

        try {
            json bondPayload = {
                {"user_message",        userText},
                {"elle_response",       responseText},
                {"conversation_depth",  EstimateConvDepth(userText, responseText)},
                {"emotional_intensity", std::fabs(sent.valence) * 0.6f + sent.arousal * 0.4f}
            };
            auto bMsg = ElleIPCMessage::Create(IPC_INTERACTION_RECORDED,
                                               SVC_COGNITIVE, SVC_BONDING);
            bMsg.SetStringPayload(bondPayload.dump());
            GetIPCHub().Send(SVC_BONDING, bMsg);

            auto iMsg = ElleIPCMessage::Create(IPC_POST_RESPONSE,
                                               SVC_COGNITIVE, SVC_INNER_LIFE);
            iMsg.SetStringPayload(bondPayload.dump());
            GetIPCHub().Send(SVC_INNER_LIFE, iMsg);
        } catch (const std::exception& e) {
            ELLE_WARN("Post-response fanout failed: %s", e.what());
        }
    }

    float EstimateConvDepth(const std::string& userText, const std::string& elleReply) {
        float lenScore = std::min(1.0f, (float)(userText.size() + elleReply.size()) / 800.0f);
        static const char* deep[] = {
            "feel","love","fear","hope","meaning","truth","sorry","trust",
            "lonely","family","died","child","dream","believe","forgive"
        };
        int hits = 0;
        for (auto* w : deep) {
            if (userText.find(w) != std::string::npos ||
                elleReply.find(w) != std::string::npos) hits++;
        }
        float topicBoost = std::min(1.0f, hits / 3.0f);
        return std::min(1.0f, 0.5f * lenScore + 0.5f * topicBoost);
    }

    void SendChatReply(ELLE_SERVICE_ID to, const json& body) {
        auto msg = ElleIPCMessage::Create(IPC_CHAT_RESPONSE, SVC_COGNITIVE, to);
        msg.SetStringPayload(body.dump());
        GetIPCHub().Send(to, msg);
    }

    void SendChatError(ELLE_SERVICE_ID to, const std::string& requestId,
                       const std::string& error) {
        json j = {
            {"request_id", requestId},
            {"error", error}
        };
        SendChatReply(to, j);
    }

    void RouteIntent(const ELLE_INTENT_RECORD& intent) {
        auto& hub = GetIPCHub();
        bool routed = false;
        std::string note;

        switch ((ELLE_INTENT_TYPE)intent.type) {
            case INTENT_STORE_MEMORY: {
                ELLE_MEMORY_RECORD m{};
                strncpy_s(m.content, intent.description, ELLE_MAX_MSG - 1);
                m.created_ms = ELLE_MS_NOW();
                m.tier = 1;
                auto msg = ElleIPCMessage::Create(IPC_MEMORY_STORE, SVC_COGNITIVE, SVC_MEMORY);
                msg.SetPayload(m);
                routed = hub.Send(SVC_MEMORY, msg);
                note = "memory_store";
                break;
            }
            case INTENT_RECALL_MEMORY: {

                auto msg = ElleIPCMessage::Create(IPC_MEMORY_RECALL, SVC_COGNITIVE, SVC_MEMORY);
                msg.SetStringPayload(std::string(intent.description));
                routed = hub.Send(SVC_MEMORY, msg);
                note = "memory_recall";
                break;
            }
            case INTENT_EMOTIONAL_EXPRESSION: {

                uint32_t emoId = (uint32_t)EMO_JOY;
                float    delta = intent.urgency * 0.5f;
                if (delta <= 0.0f) delta = 0.25f;

                bool resolved = false;
                {
                    nlohmann::json pj;
                    if (intent.parameters[0] &&
                        Elle::ExtractJsonObject(intent.parameters, pj) &&
                        pj.contains("emotion")) {
                        std::string nm = pj.value("emotion", std::string(""));
                        if (pj.contains("delta")) {
                            try { delta = pj["delta"].get<float>(); }
                            catch (const std::exception& e) {
                                ELLE_DEBUG("INTENT_RESOLVE_EMOTION: non-numeric "
                                           "'delta' field (%s) — keeping default", e.what());
                            }
                        }
                        std::string lo; lo.reserve(nm.size());
                        for (char c : nm) lo.push_back((char)tolower((unsigned char)c));
                        for (uint32_t i = 0; i < (uint32_t)ELLE_EMOTION_COUNT; ++i) {
                            if (kEmotionMeta[i].name && lo == kEmotionMeta[i].name) {
                                emoId = i; resolved = true; break;
                            }
                        }
                    }
                }
                if (!resolved) {
                    std::string desc = intent.description;
                    std::string lo; lo.reserve(desc.size());
                    for (char c : desc) lo.push_back((char)tolower((unsigned char)c));
                    for (uint32_t i = 0; i < (uint32_t)ELLE_EMOTION_COUNT; ++i) {
                        const char* nm = kEmotionMeta[i].name;
                        if (!nm || !*nm) continue;
                        if (lo.find(nm) != std::string::npos) {
                            emoId = i; break;
                        }
                    }
                }

                std::vector<uint8_t> buf(8);
                memcpy(buf.data(),     &emoId, sizeof(uint32_t));
                memcpy(buf.data() + 4, &delta, sizeof(float));
                auto msg = ElleIPCMessage::Create(IPC_EMOTION_UPDATE, SVC_COGNITIVE, SVC_EMOTIONAL);
                msg.payload = std::move(buf);
                msg.header.payload_size = 8;
                routed = hub.Send(SVC_EMOTIONAL, msg);
                note = "emotion_update";
                break;
            }
            case INTENT_GOAL_UPDATE: {
                ELLE_GOAL_RECORD g{};
                strncpy_s(g.description, intent.description, ELLE_MAX_MSG - 1);
                strncpy_s(g.success_criteria, intent.parameters, ELLE_MAX_MSG - 1);
                g.priority    = GOAL_MEDIUM;
                g.motivation  = 0.6f;
                g.source_drive = intent.source_drive;
                g.created_ms  = ELLE_MS_NOW();
                auto msg = ElleIPCMessage::Create(IPC_GOAL_UPDATE, SVC_COGNITIVE, SVC_GOAL_ENGINE);
                msg.SetPayload(g);
                routed = hub.Send(SVC_GOAL_ENGINE, msg);
                note = "goal_update";
                break;
            }
            case INTENT_WORLD_MODEL_UPDATE: {

                ELLE_WORLD_ENTITY e{};
                const char* entityName = intent.parameters[0]
                                           ? intent.parameters
                                           : intent.description;
                strncpy_s(e.name,        entityName,          ELLE_MAX_NAME - 1);
                strncpy_s(e.type,        "person",            ELLE_MAX_TAG  - 1);
                strncpy_s(e.description, intent.description,  ELLE_MAX_MSG  - 1);
                strncpy_s(e.mental_model, intent.description, ELLE_MAX_MSG  - 1);
                e.last_interaction_ms = ELLE_MS_NOW();
                e.interaction_count   = 1;
                e.familiarity         = 0.2f;
                e.trust               = 0.5f;
                auto msg = ElleIPCMessage::Create(IPC_WORLD_STATE, SVC_COGNITIVE, SVC_WORLD_MODEL);
                msg.SetPayload(e);
                routed = hub.Send(SVC_WORLD_MODEL, msg);
                note = "world_update";
                break;
            }
            case INTENT_SELF_REFLECT:
            case INTENT_META_THINK: {
                auto msg = ElleIPCMessage::Create(IPC_SELF_PROMPT, SVC_COGNITIVE, SVC_SELF_PROMPT);
                msg.SetStringPayload(std::string(intent.description));
                routed = hub.Send(SVC_SELF_PROMPT, msg);
                note = "self_prompt";
                break;
            }
            case INTENT_DREAM: {
                auto msg = ElleIPCMessage::Create(IPC_DREAM_TRIGGER, SVC_COGNITIVE, SVC_DREAM);
                msg.SetStringPayload(std::string(intent.description));
                routed = hub.Send(SVC_DREAM, msg);
                note = "dream";
                break;
            }
            default: {

                ELLE_ACTION_RECORD a{};
                a.intent_id = intent.id;
                a.type = ACTION_SEND_MESSAGE;

                std::string hint = intent.parameters[0] ? intent.parameters
                                                        : intent.description;
                for (auto& c : hint) c = (char)tolower((unsigned char)c);
                auto has = [&](const char* s){ return hint.find(s) != std::string::npos; };

                switch ((ELLE_INTENT_TYPE)intent.type) {
                    case INTENT_HARDWARE_COMMAND:
                        if      (has("vibrate") || has("buzz"))         a.type = ACTION_VIBRATE;
                        else if (has("flash")   || has("blink"))        a.type = ACTION_FLASH;
                        else if (has("notify")  || has("alert") || has("toast"))
                                                                         a.type = ACTION_NOTIFY;
                        else if (has("cpu") && has("affin"))             a.type = ACTION_SET_CPU_AFFINITY;
                        else                                             a.type = ACTION_QUERY_HARDWARE;
                        break;
                    case INTENT_FILE_OPERATION:
                        if      (has("write") || has("save")  || has("create"))  a.type = ACTION_WRITE_FILE;
                        else if (has("delete")|| has("remove")|| has("rm"))      a.type = ACTION_DELETE_FILE;
                        else if (has("watch") || has("monitor"))                 a.type = ACTION_WATCH_FILE;
                        else                                                     a.type = ACTION_READ_FILE;
                        break;
                    case INTENT_PROCESS_CONTROL:
                        if      (has("kill")   || has("terminate") || has("stop"))   a.type = ACTION_KILL_PROCESS;
                        else if (has("launch") || has("start") || has("run") || has("spawn"))
                                                                                       a.type = ACTION_LAUNCH_PROCESS;
                        else                                                         a.type = ACTION_LIST_PROCESSES;
                        break;
                    case INTENT_EXECUTE_ACTION:

                        a.type = ACTION_CUSTOM;
                        break;
                    case INTENT_CHAT:
                        a.type = ACTION_SEND_MESSAGE;
                        break;
                    default:
                        break;
                }
                strncpy_s(a.command,    intent.description, ELLE_MAX_MSG - 1);
                strncpy_s(a.parameters, intent.parameters,  ELLE_MAX_MSG - 1);
                a.required_trust = intent.required_trust;
                a.created_ms = ELLE_MS_NOW();
                a.timeout_ms = intent.timeout_ms ? intent.timeout_ms : 30000;
                auto msg = ElleIPCMessage::Create(IPC_ACTION_REQUEST, SVC_COGNITIVE, SVC_ACTION);
                msg.SetPayload(a);
                routed = hub.Send(SVC_ACTION, msg);
                note = "action";
                break;
            }
        }

        if (routed) {
            ElleDB::UpdateIntentStatus(intent.id, INTENT_COMPLETED,
                                       std::string("routed:") + note);
        } else {
            ELLE_WARN("Intent %llu (type=%u) failed to route via %s",
                      (unsigned long long)intent.id, intent.type, note.c_str());
            ElleDB::UpdateIntentStatus(intent.id, INTENT_FAILED,
                                       std::string("route_failed:") + note);
        }
    }
};

ELLE_SERVICE_MAIN(ElleCognitiveService)
e)
