#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include <algorithm>
#include "../_Shared/json.hpp"
using json = nlohmann::json;

class ElleQueueWorkerService : public ElleServiceBase {
public:
    ElleQueueWorkerService()
        : ElleServiceBase(SVC_QUEUE_WORKER, "ElleQueueWorker",
                          "Elle-Ann Queue Worker",
                          "SQL queue polling and intent/action routing") {}

protected:
    bool OnStart() override {
        auto probe = ElleSQLPool::Instance().Query("SELECT 1");
        if (!probe.success) {
            ELLE_ERROR("Queue worker: SQL pool probe failed — refusing to start");
            return false;
        }
        SetTickInterval(500);
        ELLE_INFO("Queue worker started (poll interval: 500ms)");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Queue worker stopped");
    }

    void OnTick() override {

        if (m_dbCooldownTicks > 0) {
            m_dbCooldownTicks--;
            return;
        }

        std::vector<ELLE_INTENT_RECORD> intents;
        bool intentsOk = ElleDB::GetPendingIntents(intents, 10);
        for (auto& intent : intents) {
            auto msg = ElleIPCMessage::Create(IPC_INTENT_REQUEST, SVC_QUEUE_WORKER, SVC_COGNITIVE);

            json j;
            j["id"]             = intent.id;
            j["type"]           = intent.type;
            j["status"]         = intent.status;
            j["source_drive"]   = intent.source_drive;
            j["urgency"]        = intent.urgency;
            j["confidence"]     = intent.confidence;
            j["description"]    = std::string(intent.description);
            j["parameters"]     = std::string(intent.parameters);
            j["required_trust"] = intent.required_trust;
            j["created_ms"]     = intent.created_ms;
            j["timeout_ms"]     = intent.timeout_ms;
            msg.SetStringPayload(j.dump());
            GetIPCHub().Send(SVC_COGNITIVE, msg);
            m_intentsRouted++;
        }

        std::vector<ELLE_ACTION_RECORD> actions;
        bool actionsOk = ElleDB::GetPendingActions(actions, 10);
        for (auto& action : actions) {
            auto msg = ElleIPCMessage::Create(IPC_ACTION_REQUEST, SVC_QUEUE_WORKER, SVC_ACTION);
            msg.SetPayload(action);
            GetIPCHub().Send(SVC_ACTION, msg);
            m_actionsRouted++;
        }

        if (intentsOk && actionsOk) {
            if (m_consecutiveDbFailures > 0) {
                ELLE_INFO("Queue poll recovered after %u consecutive DB failures",
                          m_consecutiveDbFailures);
            }
            m_consecutiveDbFailures = 0;
        } else {
            m_consecutiveDbFailures++;

            uint32_t backoff = 1u << std::min<uint32_t>(m_consecutiveDbFailures, 4u);
            m_dbCooldownTicks = backoff;
            if (m_consecutiveDbFailures == 1 || (m_consecutiveDbFailures % 16) == 0) {
                ELLE_ERROR("Queue DB poll failed (%u consecutive) — backing off %u ticks",
                           m_consecutiveDbFailures, backoff);
            }
            return;
        }

        m_pollCount++;
        if (m_pollCount % 120 == 0) {
            ELLE_INFO("Queue stats: %llu intents, %llu actions routed",
                      m_intentsRouted, m_actionsRouted);
        }

        if (m_pollCount % 10 == 0) {
            auto& cfg = ElleConfig::Instance();
            uint32_t intentTimeout = (uint32_t)cfg.GetInt("cognitive.intent_timeout_ms", 120000);
            uint32_t actionTimeout = (uint32_t)cfg.GetInt("action.default_timeout_ms", 60000);
            uint32_t maxRetries    = (uint32_t)cfg.GetInt("queues.max_retries", 3);
            uint32_t maxAttempts   = (uint32_t)cfg.GetInt("queues.max_action_attempts", 3);
            uint32_t reapedI = ElleDB::ReapStaleIntents(intentTimeout, maxRetries);
            uint32_t reapedA = ElleDB::ReapStaleActions(actionTimeout, maxAttempts);
            if (reapedI > 0 || reapedA > 0) {
                ELLE_WARN("Queue reaper: re-queued/failed %u intents + %u actions",
                          reapedI, reapedA);
                m_intentsReaped += reapedI;
                m_actionsReaped += reapedA;
            }
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID ) override {

        if (msg.header.msg_type == IPC_INTENT_RESPONSE) {
            ELLE_INTENT_RECORD intent;
            if (msg.GetPayload(intent)) {
                ElleDB::UpdateIntentStatus(intent.id, (ELLE_INTENT_STATUS)intent.status, intent.response);
            }
        }
        if (msg.header.msg_type == IPC_ACTION_RESPONSE) {
            ELLE_ACTION_RECORD action;
            if (msg.GetPayload(action)) {
                ElleDB::UpdateActionStatus(action.id, (ELLE_ACTION_STATUS)action.status, action.result);
            }
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_COGNITIVE, SVC_ACTION };
    }

private:
    uint64_t m_intentsRouted = 0;
    uint64_t m_actionsRouted = 0;
    uint64_t m_intentsReaped = 0;
    uint64_t m_actionsReaped = 0;
    uint32_t m_pollCount = 0;

    uint32_t m_consecutiveDbFailures = 0;
    uint32_t m_dbCooldownTicks       = 0;
};

ELLE_SERVICE_MAIN(ElleQueueWorkerService)
