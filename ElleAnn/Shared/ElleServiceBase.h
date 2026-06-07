#pragma once
#ifndef ELLE_SERVICE_BASE_H
#define ELLE_SERVICE_BASE_H

#include "ElleTypes.h"
#include "ElleConfig.h"
#include "ElleLogger.h"
#include "ElleQueueIPC.h"
#include "ElleSQLConn.h"
#include <string>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

class ElleServiceBase {
public:
    ElleServiceBase(ELLE_SERVICE_ID serviceId, const std::string& serviceName,
                    const std::string& displayName, const std::string& description);
    virtual ~ElleServiceBase();

    int Run(int argc, char* argv[]);

    ELLE_SERVICE_ID GetServiceId() const { return m_serviceId; }
    const std::string& GetName() const { return m_serviceName; }
    bool IsRunning() const { return m_running; }

    ElleIPCHub& GetIPCHub() { return m_ipcHub; }

    static ElleServiceBase* Current() { return s_instance; }

protected:

    virtual bool OnStart() = 0;

    virtual void OnStop() = 0;

    virtual void OnPause() {}

    virtual void OnResume() {}

    virtual void OnMessage(const ElleIPCMessage& , ELLE_SERVICE_ID ) {}

    virtual void OnConfigReload() {}

    virtual void OnTick() {}

    virtual std::vector<ELLE_SERVICE_ID> GetDependencies() { return {}; }

    void SetTickInterval(uint32_t ms) { m_tickIntervalMs = ms; }

    std::atomic<bool>& Running() { return m_running; }

    void InterruptibleSleep(uint32_t ms);

private:
    ELLE_SERVICE_ID m_serviceId;
    std::string     m_serviceName;
    std::string     m_displayName;
    std::string     m_argConfigPath;
    std::string     m_description;
    std::atomic<bool> m_running{false};
    uint32_t        m_tickIntervalMs = 100;

    ElleIPCHub      m_ipcHub;

    std::mutex                 m_stopMutex;
    std::condition_variable    m_stopCv;

    static ElleServiceBase* s_instance;
    SERVICE_STATUS          m_svcStatus;
    SERVICE_STATUS_HANDLE   m_svcStatusHandle = nullptr;

    static void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
    static DWORD WINAPI ServiceCtrlHandler(DWORD control, DWORD eventType,
                                            LPVOID eventData, LPVOID context);

    void ReportStatus(DWORD currentState, DWORD exitCode = NO_ERROR, DWORD waitHint = 0);

    bool InstallService();
    bool UninstallService();

    int RunConsole();

    int DoubleClickInstall();
    bool IsRunningFromSCM();

    void ServiceLoop();
    bool InitializeCore();
    void ShutdownCore();

    void ConnectDependenciesNonBlocking();

    std::thread        m_reconnectThread;
    std::set<ELLE_SERVICE_ID> m_everConnectedTo;
    std::mutex         m_reconnectMutex;
    void RunReconnectorLoop();

    void TickReconnector();
    static constexpr uint32_t kReconnectIntervalMs = 5000;

    static constexpr uint32_t kStartHintMs = 2000;
    static constexpr uint32_t kStopHintMs  = 2000;
};

#define ELLE_SERVICE_MAIN(ServiceClass)                          \
    int main(int argc, char* argv[]) {                           \
        ServiceClass svc;                                         \
        return svc.Run(argc, argv);                               \
    }

#endif
