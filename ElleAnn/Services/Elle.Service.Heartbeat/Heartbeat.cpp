#include "../../Shared/ElleTypes.h"
#include "../../Shared/ElleServiceBase.h"
#include "../../Shared/ElleLogger.h"
#include "../../Shared/ElleConfig.h"
#include "../../Shared/ElleSQLConn.h"
#include <mutex>
#include <vector>

class ElleHeartbeatService : public ElleServiceBase {
public:
    ElleHeartbeatService()
        : ElleServiceBase(SVC_HEARTBEAT, "ElleHeartbeat",
                          "Elle-Ann Heartbeat Monitor",
                          "Service health monitoring, dead man switch, and watchdog") {}

protected:
    bool OnStart() override {
        auto& cfg = ElleConfig::Instance().GetService();
        SetTickInterval(cfg.heartbeat_ms);
        m_timeoutMs = cfg.heartbeat_timeout_ms;
        m_deadManMs = cfg.dead_man_timeout_ms;
        m_watchdogMs = cfg.watchdog_ms;

        for (int i = 0; i < ELLE_SERVICE_COUNT; i++) {
            m_lastHeartbeat[i] = ELLE_MS_NOW();
            m_healthy[i] = false;
        }
        m_healthy[SVC_HEARTBEAT] = true;

        ELLE_INFO("Heartbeat service started (interval=%dms, timeout=%dms, deadman=%dms)",
                  cfg.heartbeat_ms, m_timeoutMs, m_deadManMs);
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Heartbeat service stopped");
    }

    void OnTick() override {
        uint64_t now = ELLE_MS_NOW();

        ElleDB::UpdateWorkerHeartbeat(SVC_HEARTBEAT);

        auto msg = ElleIPCMessage::Create(IPC_HEARTBEAT, SVC_HEARTBEAT, (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        GetIPCHub().Broadcast(msg);

        std::vector<uint64_t> last(ELLE_SERVICE_COUNT);
        std::vector<char>     healthy(ELLE_SERVICE_COUNT);
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            for (int i = 0; i < ELLE_SERVICE_COUNT; i++) {
                last[i]    = m_lastHeartbeat[i];
                healthy[i] = m_healthy[i] ? 1 : 0;
            }
        }

        for (int i = 0; i < ELLE_SERVICE_COUNT; i++) {
            if (i == SVC_HEARTBEAT) continue;

            uint64_t elapsed = (now >= last[i]) ? (now - last[i]) : 0;

            if (elapsed > m_deadManMs && healthy[i]) {
                ELLE_FATAL("DEAD MAN SWITCH: %s not responding for %llums!",
                           ElleIPC::GetServiceName((ELLE_SERVICE_ID)i), elapsed);
                {
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    m_healthy[i] = false;
                }
                AttemptRestart((ELLE_SERVICE_ID)i);
            } else if (elapsed > m_timeoutMs && healthy[i]) {
                ELLE_WARN("Service %s heartbeat timeout (%llums)",
                          ElleIPC::GetServiceName((ELLE_SERVICE_ID)i), elapsed);
                std::lock_guard<std::mutex> lock(m_stateMutex);
                m_healthy[i] = false;
            }
        }

        m_tickCount++;
        if (m_tickCount % 60 == 0) {
            LogHealthSummary();
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        if ((int)sender < 0 || (int)sender >= ELLE_SERVICE_COUNT) {
            ELLE_WARN("Heartbeat OnMessage: sender %d out of range [0, %d)",
                      (int)sender, ELLE_SERVICE_COUNT);
            return;
        }
        if (msg.header.msg_type == IPC_HEARTBEAT) {
            std::lock_guard<std::mutex> lock(m_stateMutex);

            if (!m_healthy[sender] &&
                m_restartAttempts[sender] > 0) {
                ELLE_INFO("Heartbeat: %s recovered — resetting restart budget "
                          "(was %u)",
                          ElleIPC::GetServiceName(sender),
                          m_restartAttempts[sender]);
                m_restartAttempts[sender] = 0;
            }
            m_lastHeartbeat[sender] = ELLE_MS_NOW();
            m_healthy[sender] = true;
        }
        if (msg.header.msg_type == IPC_SERVICE_STATUS) {
            ELLE_SERVICE_STATUS status;
            if (msg.GetPayload(status)) {
                if ((int)status.service_id < 0 ||
                    (int)status.service_id >= ELLE_SERVICE_COUNT) return;
                std::lock_guard<std::mutex> lock(m_stateMutex);
                if (!m_healthy[status.service_id] && status.healthy &&
                    m_restartAttempts[status.service_id] > 0) {
                    m_restartAttempts[status.service_id] = 0;
                }
                m_lastHeartbeat[status.service_id] = ELLE_MS_NOW();
                m_healthy[status.service_id] = status.healthy;
            }
        }
    }

private:
    std::mutex m_stateMutex;
    uint64_t m_lastHeartbeat[ELLE_SERVICE_COUNT] = {};
    bool     m_healthy[ELLE_SERVICE_COUNT] = {};
    uint32_t m_timeoutMs = 15000;
    uint32_t m_deadManMs = 60000;
    uint32_t m_watchdogMs = 10000;
    uint32_t m_tickCount = 0;
    uint32_t m_restartAttempts[ELLE_SERVICE_COUNT] = {};

    void AttemptRestart(ELLE_SERVICE_ID svc) {
        if ((int)svc < 0 || (int)svc >= ELLE_SERVICE_COUNT) {
            ELLE_WARN("AttemptRestart: service id %d out of range", (int)svc);
            return;
        }
        auto maxRestarts = ElleConfig::Instance().GetService().max_restarts;
        uint32_t attempts;
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            if (m_restartAttempts[svc] >= maxRestarts) {
                ELLE_FATAL("Max restart attempts reached for %s — manual intervention required",
                           ElleIPC::GetServiceName(svc));
                return;
            }
            attempts = ++m_restartAttempts[svc];
        }

        ELLE_INFO("Attempting restart of %s (attempt %u/%u)",
                  ElleIPC::GetServiceName(svc), attempts, maxRestarts);

        SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (hSCM) {
            std::string svcName = std::string("Elle") + ElleIPC::GetServiceName(svc);
            SC_HANDLE hService = OpenServiceA(hSCM, svcName.c_str(), SERVICE_ALL_ACCESS);
            if (hService) {
                SERVICE_STATUS status;
                ControlService(hService, SERVICE_CONTROL_STOP, &status);

                InterruptibleSleep(2000);
                StartServiceA(hService, 0, nullptr);
                CloseServiceHandle(hService);
            }
            CloseServiceHandle(hSCM);
        }
    }

    void LogHealthSummary() {
        uint64_t now = ELLE_MS_NOW();
        std::vector<uint64_t> last(ELLE_SERVICE_COUNT);
        std::vector<char>     healthy(ELLE_SERVICE_COUNT);
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            for (int i = 0; i < ELLE_SERVICE_COUNT; i++) {
                last[i]    = m_lastHeartbeat[i];
                healthy[i] = m_healthy[i] ? 1 : 0;
            }
        }
        ELLE_INFO("=== SERVICE HEALTH SUMMARY ===");
        for (int i = 0; i < ELLE_SERVICE_COUNT; i++) {
            uint64_t age = (now >= last[i]) ? (now - last[i]) : 0;
            ELLE_INFO("  %s: %s (last seen %llums ago)",
                      ElleIPC::GetServiceName((ELLE_SERVICE_ID)i),
                      healthy[i] ? "HEALTHY" : "UNHEALTHY", age);
        }
    }
};

ELLE_SERVICE_MAIN(ElleHeartbeatService)
