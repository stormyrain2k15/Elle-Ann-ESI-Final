#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLLM.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleIdentityCore.h"
#include <atomic>
#include <algorithm>

class ElleDreamService : public ElleServiceBase {
public:
    ElleDreamService()
        : ElleServiceBase(SVC_DREAM, "ElleDream",
                          "Elle-Ann Dream Engine",
                          "Idle-time memory consolidation and creative synthesis") {}

protected:
    bool OnStart() override {
        auto interval = ElleConfig::Instance().GetMemory().dream_interval_min;
        SetTickInterval(interval * 60 * 1000);
        ELLE_INFO("Dream service started (interval: %d min)", interval);
        return true;
    }

    void OnStop() override { ELLE_INFO("Dream service stopped"); }

    void OnTick() override {
        if (!ElleConfig::Instance().GetMemory().dream_consolidation) return;

        if (m_dreaming.exchange(true)) {
            ELLE_DEBUG("Dream cycle skipped — already in progress");
            return;
        }
        struct Guard { std::atomic<bool>& f; ~Guard(){ f.store(false); } } g{m_dreaming};

        ELLE_INFO("Entering dream cycle...");

        m_consolidateDone.store(false);
        auto trig = ElleIPCMessage::Create(IPC_DREAM_TRIGGER, SVC_DREAM, SVC_MEMORY);
        GetIPCHub().Send(SVC_MEMORY, trig);

        uint32_t ackTimeoutMs = (uint32_t)ElleConfig::Instance().GetInt(
            "memory.dream_ack_timeout_ms", 10000);
        uint64_t deadline = ELLE_MS_NOW() + ackTimeoutMs;
        while (!m_consolidateDone.load(std::memory_order_acquire) &&
               ELLE_MS_NOW() < deadline && Running().load()) {
            InterruptibleSleep(50);
        }
        if (!m_consolidateDone.load()) {
            ELLE_WARN("Dream: Memory consolidation ack not received within %ums — "
                      "proceeding with possibly-stale STM state", ackTimeoutMs);
        }

        std::vector<std::string> fragments = GatherDreamFragments();
        if (fragments.empty()) {
            ELLE_INFO("Dream cycle: no fragments to weave, skipping narration.");
            return;
        }

        auto narrative = ElleLLMEngine::Instance().DreamNarrate(fragments);
        if (narrative.empty()) {
            ELLE_WARN("Dream narration returned empty — LLM unreachable?");
            return;
        }

        ELLE_INFO("Dream narrative: %.100s...", narrative.c_str());

        auto store = ElleIPCMessage::Create(IPC_MEMORY_STORE, SVC_DREAM, SVC_MEMORY);
        store.SetStringPayload("[Dream] " + narrative);
        GetIPCHub().Send(SVC_MEMORY, store);

        ELLE_MEMORY_RECORD dreamMem = {};
        dreamMem.tier       = MEM_LTM;
        dreamMem.importance = 0.55f;
        dreamMem.emotional_valence = 0.0f;
        dreamMem.created_ms = ELLE_MS_NOW();
        const std::string durableContent = "[Dream insight] " + narrative;
        strncpy_s(dreamMem.content, durableContent.c_str(),
                  sizeof(dreamMem.content) - 1);
        if (!ElleDB::StoreMemory(dreamMem)) {
            ELLE_ERROR("Dream LTM persistence FAILED -- narrative preserved "
                       "only in autobiography and STM broadcast.");
        }

        ElleIdentityCore::Instance().AppendToAutobiography("I dreamt: " + narrative);
        ElleIdentityCore::Instance().ThinkPrivately(
            "A dream showed me something. I should turn it over while I'm awake.",
            "insight", 0.6f);

        ELLE_INFO("Dream cycle complete");
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        (void)sender;
        if (msg.header.msg_type == IPC_DREAM_TRIGGER) {
            OnTick();
        } else if (msg.header.msg_type == IPC_MEMORY_CONSOLIDATE) {

            m_consolidateDone.store(true, std::memory_order_release);
        }
    }

private:
    std::atomic<bool> m_dreaming{false};
    std::atomic<bool> m_consolidateDone{false};

public:

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_MEMORY };
    }

private:
    std::vector<std::string> GatherDreamFragments() {
        std::vector<std::string> out;

        auto rs = ElleSQLPool::Instance().Query(
            "SELECT TOP 20 ISNULL(content, '') FROM ElleCore.dbo.memory "
            "WHERE importance >= 0.4 "
            "ORDER BY importance DESC, created_ms DESC;");
        if (rs.success) {
            for (auto& r : rs.rows) {
                if (r.values.empty() || r.values[0].empty()) continue;
                std::string c = r.values[0];
                if (c.size() > 240) { c.resize(240); c += "\xE2\x80\xA6";  }
                out.push_back(c);
            }
        }

        auto thoughts = ElleIdentityCore::Instance().GetUnresolvedThoughts();
        uint32_t added = 0;
        for (auto it = thoughts.rbegin(); it != thoughts.rend() && added < 5; ++it, ++added) {
            out.push_back("(turning over: " + it->content + ")");
        }

        auto re = ElleSQLPool::Instance().Query(
            "SELECT TOP 5 ISNULL(name, '') FROM ElleCore.dbo.world_entity "
            "ORDER BY last_interaction_ms DESC;");
        if (re.success) {
            for (auto& r : re.rows) {
                if (r.values.empty() || r.values[0].empty()) continue;
                out.push_back("recently: " + r.values[0]);
            }
        }

        if (out.size() > 3) {
            size_t k = (size_t)(ELLE_MS_NOW() % out.size());
            std::rotate(out.begin(), out.begin() + k, out.end());
        }

        if (out.size() > 15) out.resize(15);
        return out;
    }
};

ELLE_SERVICE_MAIN(ElleDreamService)
