#pragma once
#ifndef ELLE_COMPOSER_CLIENT_H
#define ELLE_COMPOSER_CLIENT_H

#include "ElleTypes.h"
#include "ElleQueueIPC.h"
#include "ElleLogger.h"
#include "json.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct LLMMessage {
    std::string role;
    std::string content;
};

namespace ElleComposer {

struct ComposeReply {
    bool        success    = false;
    std::string error;
    std::string text;
    std::string act;
    int64_t     frameId    = 0;
    float       confidence = 0.0f;
    nlohmann::json full;
};

class Client {
public:
    static Client& Instance() {
        static Client c;
        return c;
    }

    void Bind(ElleIPCHub* hub, ELLE_SERVICE_ID requester) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_boundHub       = hub;
        m_boundRequester = requester;
    }

    bool IsBound() const {
        return m_boundHub != nullptr;
    }

    ComposeReply RequestBound(const std::string& kind,
                              const nlohmann::json& payload,
                              uint32_t timeoutMs = 3000) {
        ElleIPCHub*     hub = nullptr;
        ELLE_SERVICE_ID req = SVC_COGNITIVE;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            hub = m_boundHub;
            req = m_boundRequester;
        }
        if (!hub) {
            ComposeReply r;
            r.error = "composer_client_unbound";
            return r;
        }
        return Request(*hub, req, kind, payload, timeoutMs);
    }

    ComposeReply Request(ElleIPCHub& hub,
                         ELLE_SERVICE_ID requester,
                         const std::string& kind,
                         const nlohmann::json& payload,
                         uint32_t timeoutMs = 3000)
    {
        char rid[64];
        snprintf(rid, sizeof(rid), "comp-%s-%llu-%u",
                 kind.c_str(),
                 (unsigned long long)ELLE_MS_NOW(),
                 (unsigned)(size_t)std::hash<std::thread::id>{}(std::this_thread::get_id()));

        nlohmann::json env = payload;
        env["request_id"] = rid;
        env["kind"]       = kind;
        if (!env.contains("stream")) env["stream"] = false;

        auto pending = std::make_shared<Pending>();
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_pending[rid] = pending;
        }

        auto msg = ElleIPCMessage::Create(IPC_COMPOSE_REQUEST,
                                          requester, SVC_COMPOSER);
        msg.SetStringPayload(env.dump());
        if (!hub.Send(SVC_COMPOSER, msg)) {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_pending.erase(rid);
            ComposeReply r;
            r.error = "compose_send_failed";
            return r;
        }

        std::unique_lock<std::mutex> lk(pending->m);
        bool got = pending->cv.wait_for(lk,
                        std::chrono::milliseconds(timeoutMs),
                        [&]{ return pending->done; });
        nlohmann::json result = got ? std::move(pending->result)
                                    : nlohmann::json::object();
        lk.unlock();

        {
            std::lock_guard<std::mutex> g(m_mutex);
            m_pending.erase(rid);
        }

        ComposeReply r;
        r.full = result;
        if (!got) {
            r.error = "compose_timeout";
            ELLE_DEBUG("ComposerClient: timeout for kind=%s after %ums",
                       kind.c_str(), timeoutMs);
            return r;
        }
        r.success    = result.value("success", false);
        r.error      = result.value("error",   std::string());
        r.text       = result.value("text",    std::string());
        r.act        = result.value("act",     std::string());
        r.frameId    = result.value("frame_id",(int64_t)0);
        r.confidence = (float)result.value("confidence", 0.0);
        return r;
    }

    void Deliver(const ElleIPCMessage& msg) {
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(msg.GetStringPayload());
        } catch (const std::exception&) {
            return;
        }
        std::string rid = j.value("request_id", std::string());
        if (rid.empty()) return;

        std::shared_ptr<Pending> p;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_pending.find(rid);
            if (it == m_pending.end()) return;
            p = it->second;
        }
        {
            std::lock_guard<std::mutex> lk(p->m);
            p->result = std::move(j);
            p->done   = true;
        }
        p->cv.notify_one();
    }

private:
    Client() = default;

    struct Pending {
        std::mutex              m;
        std::condition_variable cv;
        bool                    done = false;
        nlohmann::json          result;
    };

    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<Pending>> m_pending;
    ElleIPCHub*     m_boundHub       = nullptr;
    ELLE_SERVICE_ID m_boundRequester = SVC_COGNITIVE;
};

inline std::string Ask(const std::string& prompt,
                        const std::string& systemPrompt = "",
                        uint32_t timeoutMs = 5000,
                        const std::string& fallback = "") {
    nlohmann::json p;
    p["prompt"] = prompt;
    if (!systemPrompt.empty()) p["system"] = systemPrompt;
    auto r = Client::Instance().RequestBound("ASK_INNER", p, timeoutMs);
    return (r.success && !r.text.empty()) ? r.text : fallback;
}

inline ComposeReply Converse(const nlohmann::json& history,
                              float temperature  = -1.0f,
                              uint32_t maxWords   = 200,
                              uint32_t timeoutMs  = 5000) {
    nlohmann::json p;
    p["history"]     = history;
    p["temperature"] = temperature;
    p["max_words"]   = maxWords;
    return Client::Instance().RequestBound("CONVERSE", p, timeoutMs);
}

inline std::string SelfReflect(const std::string& context,
                                const nlohmann::json& emotion,
                                uint32_t timeoutMs = 5000,
                                const std::string& fallback = "") {
    nlohmann::json p;
    p["context"] = context;
    p["emotion"] = emotion;
    auto r = Client::Instance().RequestBound("SELF_REFLECT", p, timeoutMs);
    return (r.success && !r.text.empty()) ? r.text : fallback;
}

inline std::string SelfReflectStr(const std::string& context,
                                   const ELLE_EMOTION_STATE& emotion,
                                   uint32_t timeoutMs = 5000,
                                   const std::string& fallback = "") {
    nlohmann::json em;
    em["valence"] = emotion.valence;
    em["arousal"] = emotion.arousal;
    nlohmann::json dims = nlohmann::json::object();
    for (int i = 0; i < ELLE_EMOTION_COUNT; ++i) {
        dims[std::to_string(i)] = emotion.dimensions[i];
    }
    em["dimensions"] = dims;
    return SelfReflect(context, em, timeoutMs, fallback);
}

inline std::string FormGoal(const std::string& drives,
                             const std::string& emotion,
                             uint32_t timeoutMs = 5000,
                             const std::string& fallback = "") {
    nlohmann::json p;
    p["drives"]  = drives;
    p["emotion"] = emotion;
    auto r = Client::Instance().RequestBound("FORM_GOAL", p, timeoutMs);
    return (r.success && !r.text.empty()) ? r.text : fallback;
}

inline std::string RewriteScenario(const nlohmann::json& scenario,
                                    uint32_t timeoutMs = 5000,
                                    const std::string& fallback = "") {
    auto r = Client::Instance().RequestBound("REWRITE_SCENARIO", scenario, timeoutMs);
    return (r.success && !r.text.empty()) ? r.text : fallback;
}

inline ELLE_LLM_RESPONSE ChatLegacy(const std::vector<LLMMessage>& messages,
                                     float temperature = -1.0f,
                                     uint32_t maxTokens = 0,
                                     uint32_t timeoutMs = 5000) {
    nlohmann::json history = nlohmann::json::array();
    for (const auto& m : messages) {
        history.push_back({{"role", m.role}, {"text", m.content}});
    }
    auto r = Converse(history, temperature,
                       maxTokens ? (uint32_t)(maxTokens / 3) : 200,
                       timeoutMs);
    ELLE_LLM_RESPONSE out{};
    out.success     = r.success;
    out.latency_ms  = 0;
    out.tokens_used = (uint32_t)(r.text.size() / 4);
    const std::string& body = r.text.empty() ? r.error : r.text;
    std::strncpy(out.content, body.c_str(), ELLE_MAX_MSG - 1);
    std::strncpy(out.model_used, "composer", ELLE_MAX_MSG - 1);
    return out;
}

inline std::string ComposeText(ElleIPCHub& hub,
                               ELLE_SERVICE_ID requester,
                               const std::string& kind,
                               const nlohmann::json& payload,
                               uint32_t timeoutMs = 3000,
                               const std::string& fallback = "")
{
    auto r = Client::Instance().Request(hub, requester, kind, payload, timeoutMs);
    if (!r.success || r.text.empty()) {
        return fallback;
    }
    return r.text;
}

}

#endif
