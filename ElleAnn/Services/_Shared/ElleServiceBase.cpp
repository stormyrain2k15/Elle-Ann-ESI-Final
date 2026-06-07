#include "ElleServiceBase.h"
#include "ElleIdentityCore.h"
#include "ElleLLM.h"
#include "ElleComposerClient.h"
#include "ElleSQLConn.h"
#include "ElleConfig.h"
#include "ElleGameAccountDB.h"
#include <tlhelp32.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

ElleServiceBase* ElleServiceBase::s_instance = nullptr;

ElleServiceBase::ElleServiceBase(ELLE_SERVICE_ID serviceId, const std::string& serviceName,
                                  const std::string& displayName, const std::string& description)
    : m_serviceId(serviceId)
    , m_serviceName(serviceName)
    , m_displayName(displayName)
    , m_description(description)
{
    s_instance = this;
    ZeroMemory(&m_svcStatus, sizeof(SERVICE_STATUS));
    m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_svcStatus.dwCurrentState = SERVICE_STOPPED;
}

static std::string GetExeDirPortable() {
    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (exePath[0] == '\0') return "";
    PathRemoveFileSpecA(exePath);
    std::string dir(exePath);
    if (!dir.empty() && dir.back() != '\\' && dir.back() != '/') dir.push_back('\\');
    return dir;
}

ElleServiceBase::~ElleServiceBase() {
    if (s_instance == this) s_instance = nullptr;
}

int ElleServiceBase::Run(int argc, char* argv[]) {

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--install" || arg == "-i" || arg == "/install") {
            return InstallService() ? 0 : 1;
        }
        if (arg == "--uninstall" || arg == "-u" || arg == "/uninstall") {
            return UninstallService() ? 0 : 1;
        }
        if (arg == "--console" || arg == "-c" || arg == "/console") {
            return RunConsole();
        }
        if (arg == "--help" || arg == "-h" || arg == "/help") {
            std::cout << m_displayName << " v" << ELLE_VERSION_STRING << "\n\n"
                      << "Usage:\n"
                      << "  " << argv[0] << " --install    Install as Windows service\n"
                      << "  " << argv[0] << " --uninstall  Remove Windows service\n"
                      << "  " << argv[0] << " --console    Run in console mode (debug)\n"
                      << "  " << argv[0] << "              Double-click to install\n";
            return 0;
        }
    }

    if (!IsRunningFromSCM()) {
        return DoubleClickInstall();
    }

    std::wstring wName(m_serviceName.begin(), m_serviceName.end());
    SERVICE_TABLE_ENTRYW dispatchTable[] = {
        { const_cast<LPWSTR>(wName.c_str()), ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcherW(dispatchTable)) {
        DWORD err = GetLastError();
        if (err == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {

            return RunConsole();
        }
        return 1;
    }
    return 0;
}

void WINAPI ElleServiceBase::ServiceMain(DWORD argc, LPWSTR* argv) {
    if (!s_instance) return;

    {
        char exePath[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string exeDir(exePath);
        size_t ls = exeDir.find_last_of("\\/");
        if (ls != std::string::npos) exeDir.resize(ls);
        if (!exeDir.empty()) {
            SetCurrentDirectoryA(exeDir.c_str());
        }
    }

    if (argc > 1 && argv[1]) {
        std::wstring w(argv[1]);
        std::string a(w.begin(), w.end());
        s_instance->m_argConfigPath = a;
    }

    std::wstring wName(s_instance->m_serviceName.begin(), s_instance->m_serviceName.end());
    s_instance->m_svcStatusHandle = RegisterServiceCtrlHandlerExW(
        wName.c_str(), ServiceCtrlHandler, s_instance);

    if (!s_instance->m_svcStatusHandle) return;

    s_instance->ReportStatus(SERVICE_START_PENDING, NO_ERROR, kStartHintMs);

    if (!s_instance->InitializeCore()) {
        s_instance->ReportStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR);
        return;
    }

    if (!s_instance->OnStart()) {
        s_instance->ShutdownCore();
        s_instance->ReportStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR);
        return;
    }

    s_instance->ReportStatus(SERVICE_RUNNING);
    s_instance->ServiceLoop();
}

DWORD WINAPI ElleServiceBase::ServiceCtrlHandler(DWORD control, DWORD eventType,
                                                   LPVOID eventData, LPVOID context) {
    auto* svc = static_cast<ElleServiceBase*>(context);
    switch (control) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:

            svc->ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, kStopHintMs);
            svc->m_running = false;

            svc->m_stopCv.notify_all();
            svc->OnStop();
            svc->ShutdownCore();
            svc->ReportStatus(SERVICE_STOPPED);
            return NO_ERROR;

        case SERVICE_CONTROL_PAUSE:
            svc->ReportStatus(SERVICE_PAUSE_PENDING);
            svc->OnPause();
            svc->ReportStatus(SERVICE_PAUSED);
            return NO_ERROR;

        case SERVICE_CONTROL_CONTINUE:
            svc->ReportStatus(SERVICE_CONTINUE_PENDING);
            svc->OnResume();
            svc->ReportStatus(SERVICE_RUNNING);
            return NO_ERROR;

        case SERVICE_CONTROL_INTERROGATE:
            return NO_ERROR;

        default:
            return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

void ElleServiceBase::ReportStatus(DWORD currentState, DWORD exitCode, DWORD waitHint) {
    static DWORD checkPoint = 1;
    m_svcStatus.dwCurrentState = currentState;
    m_svcStatus.dwWin32ExitCode = exitCode;
    m_svcStatus.dwWaitHint = waitHint;

    if (currentState == SERVICE_START_PENDING || currentState == SERVICE_STOP_PENDING) {
        m_svcStatus.dwControlsAccepted = 0;
        m_svcStatus.dwCheckPoint = checkPoint++;
    } else {
        m_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN |
                                          SERVICE_ACCEPT_PAUSE_CONTINUE;
        m_svcStatus.dwCheckPoint = 0;
    }

    SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

bool ElleServiceBase::InstallService() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) {
        std::cerr << "ERROR: Cannot open Service Control Manager. Run as Administrator.\n";
        return false;
    }

    SC_HANDLE hService = CreateServiceA(
        hSCM,
        m_serviceName.c_str(),
        m_displayName.c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        (std::string("\"") + exePath + "\"").c_str(),
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!hService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            std::cout << "Service already installed. Updating...\n";
            hService = OpenServiceA(hSCM, m_serviceName.c_str(), SERVICE_ALL_ACCESS);
            if (hService) {
                ChangeServiceConfigA(hService, SERVICE_WIN32_OWN_PROCESS,
                    SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                    (std::string("\"") + exePath + "\"").c_str(), nullptr, nullptr, nullptr, nullptr, nullptr,
                    m_displayName.c_str());
            }
        } else {
            std::cerr << "ERROR: CreateService failed (error " << err << ")\n";
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    SERVICE_DESCRIPTIONA desc;
    desc.lpDescription = const_cast<LPSTR>(m_description.c_str());
    ChangeServiceConfig2A(hService, SERVICE_CONFIG_DESCRIPTION, &desc);

    SC_ACTION actions[3] = {
        { SC_ACTION_RESTART, 1000 },
        { SC_ACTION_RESTART, 2000 },
        { SC_ACTION_RESTART, 5000 }
    };
    SERVICE_FAILURE_ACTIONSA failActions;
    ZeroMemory(&failActions, sizeof(failActions));
    failActions.dwResetPeriod = 86400;
    failActions.cActions = 3;
    failActions.lpsaActions = actions;
    ChangeServiceConfig2A(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &failActions);

    std::cout << "Service '" << m_displayName << "' installed successfully.\n";
    std::cout << "Starting service...\n";

    if (StartServiceA(hService, 0, nullptr)) {
        std::cout << "Service started.\n";
    } else {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            std::cout << "Service is already running.\n";
        } else {
            std::cerr << "WARNING: Failed to start service (error " << err << ")\n";
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return true;
}

bool ElleServiceBase::UninstallService() {
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) {
        std::cerr << "ERROR: Cannot open Service Control Manager. Run as Administrator.\n";
        return false;
    }

    SC_HANDLE hService = OpenServiceA(hSCM, m_serviceName.c_str(), SERVICE_ALL_ACCESS);
    if (!hService) {
        std::cerr << "ERROR: Service not found.\n";
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status;
    if (ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        std::cout << "Stopping service...";
        const DWORD pollMs = 100;
        const DWORD ceilMs = 5000;
        DWORD waited = 0;
        HANDLE hSelf = GetCurrentProcess();
        while (status.dwCurrentState != SERVICE_STOPPED && waited < ceilMs) {
            WaitForSingleObject(hSelf, pollMs);
            waited += pollMs;
            if (!QueryServiceStatus(hService, &status)) break;
            if ((waited % 2000) == 0) std::cout << '.';
        }
        std::cout << (status.dwCurrentState == SERVICE_STOPPED
                          ? " stopped.\n" : " timed out.\n");
    }

    if (!DeleteService(hService)) {
        std::cerr << "ERROR: Failed to delete service.\n";
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return false;
    }

    std::cout << "Service '" << m_displayName << "' uninstalled.\n";
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return true;
}

int ElleServiceBase::DoubleClickInstall() {

    FreeConsole();

    bool ok = InstallService();

    const char* msg = ok
        ? "<SERVICE UPLOAD ONLY OK>"
        : "<SERVICE UPLOAD FAILED>\n\n"
          "Most common cause: this exe is not running as Administrator.\n"
          "Right-click → Run as administrator, then double-click again.";
    MessageBoxA(nullptr, msg, m_displayName.c_str(),
                MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONERROR));
    return ok ? 0 : 1;
}

bool ElleServiceBase::IsRunningFromSCM() {

    DWORD parentPid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        DWORD myPid = GetCurrentProcessId();
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (pe.th32ProcessID == myPid) {
                    parentPid = pe.th32ParentProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &pe));
        }

        if (parentPid) {
            if (Process32FirstW(hSnap, &pe)) {
                do {
                    if (pe.th32ProcessID == parentPid) {
                        bool isSvc = (_wcsicmp(pe.szExeFile,
                                               L"services.exe") == 0);
                        CloseHandle(hSnap);
                        return isSvc;
                    }
                } while (Process32NextW(hSnap, &pe));
            }
        }
        CloseHandle(hSnap);
    }
    return false;
}

int ElleServiceBase::RunConsole() {
    std::cout << "=== " << m_displayName << " v" << ELLE_VERSION_STRING << " ===\n";
    std::cout << "Running in console mode. Press Ctrl+C to stop.\n\n";

    SetConsoleCtrlHandler([](DWORD type) -> BOOL {
        if (s_instance) {
            s_instance->m_running = false;
            s_instance->m_stopCv.notify_all();
            s_instance->OnStop();
        }
        return TRUE;
    }, TRUE);

    if (!InitializeCore()) {
        std::cerr << "FATAL: Core initialization failed.\n";
        return 1;
    }

    if (!OnStart()) {
        std::cerr << "FATAL: Service start failed.\n";
        ShutdownCore();
        return 1;
    }

    m_running = true;
    std::cout << "Service running. Type 'quit' to exit.\n";

    std::string input;
    while (m_running) {
        if (std::cin.peek() != EOF) {
            std::getline(std::cin, input);
            if (input == "quit" || input == "exit" || input == "q") {
                m_running = false;
                m_stopCv.notify_all();
            } else if (input == "status") {
                std::cout << "Running: " << (m_running ? "yes" : "no") << "\n"
                          << "IPC Messages Sent: " << m_ipcHub.MessagesSent() << "\n"
                          << "IPC Messages Recv: " << m_ipcHub.MessagesReceived() << "\n";
            } else if (input == "reload") {
                if (ElleConfig::Instance().Reload()) {
                    std::cout << "Config reloaded.\n";
                } else {
                    std::cout << "Config reload failed.\n";
                }
            }
        }
        OnTick();
        InterruptibleSleep(m_tickIntervalMs);
    }

    OnStop();
    ShutdownCore();
    std::cout << "Service stopped.\n";
    return 0;
}

void ElleServiceBase::InterruptibleSleep(uint32_t ms) {
    if (ms == 0) return;
    if (!m_running.load(std::memory_order_acquire)) return;
    std::unique_lock<std::mutex> lk(m_stopMutex);
    m_stopCv.wait_for(lk, std::chrono::milliseconds(ms),
                      [this]{ return !m_running.load(std::memory_order_acquire); });
}

void ElleServiceBase::ServiceLoop() {
    m_running = true;
    while (m_running) {
        OnTick();
        InterruptibleSleep(m_tickIntervalMs);
    }
}

bool ElleServiceBase::InitializeCore() {
    ELLE_INFO("Initializing %s...", m_displayName.c_str());

    m_running.store(true, std::memory_order_release);

    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string exeDir(exePath);
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) exeDir.resize(lastSlash + 1);

    auto fileExists = [](const std::string& p) -> bool {
        DWORD a = GetFileAttributesA(p.c_str());
        return (a != INVALID_FILE_ATTRIBUTES) && !(a & FILE_ATTRIBUTE_DIRECTORY);
    };

    std::string shortName = m_serviceName;
    static const std::string kPfx = "Elle.Service.";
    if (shortName.rfind(kPfx, 0) == 0) shortName = shortName.substr(kPfx.size());

    std::string configPath;
    std::vector<std::string> tried;

    auto tryPath = [&](const std::string& p) -> bool {
        tried.push_back(p);
        if (fileExists(p)) { configPath = p; return true; }
        return false;
    };

    if (!m_argConfigPath.empty()) tryPath(m_argConfigPath);

    if (configPath.empty()) {
        tryPath(exeDir + "9Data\\ServerInfo\\_" + shortName + "serverinfo.txt");
    }

    if (configPath.empty()) {
        tryPath(exeDir + "9Data\\ServerInfo\\_ServerInfo.txt");
    }

    if (configPath.empty()) {
        WIN32_FIND_DATAA fd{};
        HANDLE h = FindFirstFileA((exeDir + "*ServerInfo*.txt").c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::string p = exeDir + fd.cFileName;
                    if (configPath.empty()) configPath = p;
                    tried.push_back(p);
                }
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
    }

    if (configPath.empty()) tryPath(exeDir + "elle_master_config.json");
    if (configPath.empty()) tryPath(exeDir + "9Data\\ServerInfo\\elle_master_config.json");

    if (configPath.empty()) {
        std::cerr << "WARN: no config found; running on built-in defaults. "
                     "Tried:\n";
        for (auto& p : tried) std::cerr << "  - " << p << "\n";
        ELLE_WARN("ElleServiceBase: no config file located, defaults applied");

        ElleConfig::Instance().LoadDefaults();
    } else if (!ElleConfig::Instance().Load(configPath)) {
        const DWORD gle = GetLastError();
        std::cerr << "WARN: config load/validation failed: " << configPath
                  << " (GetLastError=" << gle << "). "
                  << "Continuing on built-in defaults so the service starts.\n";
        ELLE_WARN("Config load failed (%s); using defaults", configPath.c_str());
        ElleConfig::Instance().LoadDefaults();
    } else {
        ELLE_INFO("Config loaded from: %s", configPath.c_str());
    }

    {
        std::string master;
        if (tryPath(exeDir + "elle_master_config.json")) master = configPath;
        else if (tryPath(exeDir + "9Data\\ServerInfo\\elle_master_config.json"))
            master = configPath;
        configPath = "";
        if (!master.empty() && master.size() >= 5 &&
            master.substr(master.size() - 5) == ".json") {
            if (!ElleConfig::Instance().LayerJsonOver(master)) {
                ELLE_WARN("Master JSON layer load failed: %s", master.c_str());
            } else {
                ELLE_INFO("Master config layered from: %s", master.c_str());
            }
        }
    }

    const auto& logCfg = ElleConfig::Instance().GetString("logging.level", "info");
    ELLE_LOG_LEVEL logLevel = LOG_INFO;
    if (logCfg == "trace") logLevel = LOG_TRACE;
    else if (logCfg == "debug") logLevel = LOG_DEBUG;
    else if (logCfg == "warn") logLevel = LOG_WARN;
    else if (logCfg == "error") logLevel = LOG_ERROR;

    ElleLogger::Instance().Initialize(m_serviceId, ELLE_LOG_TARGET_ALL, logLevel);
    {
        char exePath2[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, exePath2, MAX_PATH);
        std::string exeDir2(exePath2);
        size_t ls2 = exeDir2.find_last_of("\\/");
        if (ls2 != std::string::npos) exeDir2.resize(ls2 + 1);
        std::string defLog = exeDir2 + "debug\\" + m_serviceName + ".log";
        ElleLogger::Instance().SetLogFile(
            ElleConfig::Instance().GetString("logging.file_path", defLog),
        (uint32_t)ElleConfig::Instance().GetInt("logging.max_file_size_mb", 100),
        (uint32_t)ElleConfig::Instance().GetInt("logging.max_files", 10)
        );
    }

    auto& svcCfg = ElleConfig::Instance().GetService();
    if (!ElleSQLPool::Instance().Initialize(svcCfg.sql_connection_string, svcCfg.sql_pool_size)) {
        std::cerr << "FATAL: SQL pool init failed. Connection string: "
                  << svcCfg.sql_connection_string << "\n";
        ELLE_ERROR("Failed to initialize SQL connection pool");
        return false;
    }
    ELLE_INFO("SQL pool initialized (%d connections)", svcCfg.sql_pool_size);

    {
        const std::string gameDsn = ElleConfig::Instance().GetString(
            "http_server.game_db_dsn", "");
        const int gamePoolSize = (int)ElleConfig::Instance().GetInt(
            "http_server.game_db_pool_size", 4);
        if (!gameDsn.empty()) {
            if (!ElleGameAccountPool::Instance().Initialize(
                    gameDsn, (uint32_t)std::max(1, gamePoolSize))) {
                ELLE_ERROR("Game Account DB pool init failed (http_server.game_db_dsn set)");
            }
        }
    }

    ElleDB::RegisterWorker(m_serviceId, m_serviceName);

    if (!m_ipcHub.Initialize(m_serviceId, svcCfg.iocp_threads)) {
        std::cerr << "FATAL: IPC hub init failed (threads=" << svcCfg.iocp_threads << ")\n";
        ELLE_ERROR("Failed to initialize IPC hub");
        return false;
    }
    ELLE_INFO("IPC hub initialized (%d IOCP threads)", svcCfg.iocp_threads);

    ElleIdentityCore::SetIPCHub(&m_ipcHub);

    m_ipcHub.SetMessageHandler([this](const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {

        if (msg.header.msg_type == IPC_HEARTBEAT &&
            m_serviceId != SVC_HEARTBEAT &&
            sender == SVC_HEARTBEAT) {
            auto reply = ElleIPCMessage::Create(IPC_HEARTBEAT, m_serviceId, SVC_HEARTBEAT);
            m_ipcHub.Send(SVC_HEARTBEAT, reply);

            ElleDB::UpdateWorkerHeartbeat(m_serviceId);
        }

        if (msg.header.msg_type == IPC_IDENTITY_DELTA) {
            ElleIdentityCore::Instance().ApplyDelta(msg.GetStringPayload());
        }

        if (msg.header.msg_type == IPC_CONFIG_RELOAD) {
            const bool ok = ElleConfig::Instance().Reload();

            if (ok) {
                const auto& svc = ElleConfig::Instance().GetService();
                if (!svc.sql_connection_string.empty()) {
                    const bool poolOk = ElleSQLPool::Instance().Reinitialize(
                        svc.sql_connection_string,
                        svc.sql_pool_size > 0 ? svc.sql_pool_size : 8);
                    ELLE_INFO("IPC_CONFIG_RELOAD: SQL pool re-init %s",
                              poolOk ? "succeeded" : "FAILED");
                }
            }
            ELLE_INFO("IPC_CONFIG_RELOAD applied (sender=%d, ok=%d) — "
                      "calling OnConfigReload()", (int)sender, ok ? 1 : 0);
            this->OnConfigReload();
        }

        if (msg.header.msg_type == IPC_COMPOSE_RESPONSE ||
            msg.header.msg_type == IPC_COMPOSE_STREAM_CHUNK) {
            ElleComposer::Client::Instance().Deliver(msg);
        }

        this->OnMessage(msg, sender);
    });

    ConnectDependenciesNonBlocking();
    if (!m_reconnectThread.joinable()) {
        m_reconnectThread = std::thread([this]{ this->RunReconnectorLoop(); });
    }

    if (m_serviceId == SVC_COGNITIVE || m_serviceId == SVC_SELF_PROMPT ||
        m_serviceId == SVC_DREAM || m_serviceId == SVC_GOAL_ENGINE ||
        m_serviceId == SVC_HTTP_SERVER || m_serviceId == SVC_IMAGINATION ||
        m_serviceId == SVC_MEMORY || m_serviceId == SVC_BONDING ||
        m_serviceId == SVC_CONTINUITY || m_serviceId == SVC_INNER_LIFE ||
        m_serviceId == SVC_SOLITUDE) {
        ElleLLMEngine::Instance().BindHub(&m_ipcHub, m_serviceId);
        ELLE_INFO("%s registered as Composer client", m_displayName.c_str());
    }

    ELLE_INFO("%s core initialization complete", m_displayName.c_str());
    return true;
}

void ElleServiceBase::ShutdownCore() {
    ELLE_INFO("Shutting down %s...", m_displayName.c_str());

    {
        std::lock_guard<std::mutex> lk(m_stopMutex);
        m_running = false;
    }
    m_stopCv.notify_all();
    if (m_reconnectThread.joinable()) m_reconnectThread.join();

    ElleIdentityCore::SetIPCHub(nullptr);
    m_ipcHub.Shutdown();
    ElleSQLPool::Instance().Shutdown();
    ElleLogger::Instance().Shutdown();
}

void ElleServiceBase::ConnectDependenciesNonBlocking() {
    auto deps = GetDependencies();
    for (auto dep : deps) {
        if (!m_running.load()) break;

        if (m_ipcHub.ConnectTo(dep, 1500)) {
            ELLE_INFO("Connected to %s", ElleIPC::GetServiceName(dep));
        } else {
            ELLE_INFO("Peer %s not yet up — reconnector will pick it up",
                      ElleIPC::GetServiceName(dep));
        }
    }
}

void ElleServiceBase::RunReconnectorLoop() {

    if (m_running.load()) TickReconnector();

    while (m_running.load()) {
        InterruptibleSleep(kReconnectIntervalMs);
        if (!m_running.load()) break;
        TickReconnector();
    }

}

void ElleServiceBase::TickReconnector() {

    auto deps = GetDependencies();
    for (auto dep : deps) {
        if (!m_running.load()) break;

        const bool wasUp = [&]() {
            std::lock_guard<std::mutex> lk(m_reconnectMutex);
            return m_everConnectedTo.find(dep) != m_everConnectedTo.end();
        }();
        const bool aliveNow = m_ipcHub.IsConnectedTo(dep);

        if (wasUp && !aliveNow) {
            ELLE_WARN("Lost connection to %s — will reattempt",
                      ElleIPC::GetServiceName(dep));
            ELLE_LOG_SOCKET("IPC LOST %s — reattempt", ElleIPC::GetServiceName(dep));
            std::lock_guard<std::mutex> lk(m_reconnectMutex);
            m_everConnectedTo.erase(dep);

        }

        if (aliveNow) continue;

        if (m_ipcHub.ConnectTo(dep, 1000)) {
            bool first = false;
            {
                std::lock_guard<std::mutex> lk(m_reconnectMutex);
                first = m_everConnectedTo.insert(dep).second;
            }
            if (first) {
                const uint64_t now = (uint64_t)std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                ELLE_INFO("Mesh: first contact with %s (epoch %llu)",
                          ElleIPC::GetServiceName(dep),
                          (unsigned long long)now);
                ELLE_LOG_SOCKET("IPC first-contact %s epoch=%llu",
                                ElleIPC::GetServiceName(dep),
                                (unsigned long long)now);
            }
        }
    }
}
