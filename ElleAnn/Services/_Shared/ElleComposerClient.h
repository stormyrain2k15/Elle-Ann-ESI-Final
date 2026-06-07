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
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

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
};

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
