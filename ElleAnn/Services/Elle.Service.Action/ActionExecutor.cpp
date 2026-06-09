#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/ElleWait.h"
#include "../_Shared/json.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <unordered_map>

static bool SendIPC(ELLE_SERVICE_ID target, ElleIPCMessage& msg) {
    auto* svc = ElleServiceBase::Current();
    if (!svc) return false;
    return svc->GetIPCHub().Send(target, msg);
}

typedef int  (__stdcall *fn_ReadFile_t)   (const char* path, void* buffer, DWORD maxBytes, DWORD* bytesRead);
typedef int  (__stdcall *fn_WriteFile_t)  (const char* path, const void* buffer, DWORD numBytes);
typedef int  (__stdcall *fn_AppendFile_t) (const char* path, const void* buffer, DWORD numBytes);
typedef int  (__stdcall *fn_DeleteFile_t) (const char* path);
typedef int  (__stdcall *fn_FileExists_t) (const char* path);
typedef int  (__stdcall *fn_GetFileSize_t)(const char* path, ULONGLONG* outSize);

typedef int  (__stdcall *fn_LaunchProc_t) (const char* cmdLine, DWORD* outPid);
typedef int  (__stdcall *fn_KillProc_t)   (DWORD pid);
typedef int  (__stdcall *fn_EnumProc_t)   (DWORD* pids, DWORD maxCount, DWORD* actualCount);
typedef int  (__stdcall *fn_ProcName_t)   (DWORD pid, char* name, DWORD maxLen);
typedef int  (__stdcall *fn_ProcRunning_t)(DWORD pid);

typedef int  (__stdcall *fn_CPUUsage_t)   (DWORD* outPercent);
typedef int  (__stdcall *fn_MemInfo_t)    (ULONGLONG* total, ULONGLONG* freeBytes);
typedef int  (__stdcall *fn_CPUAffinity_t)(DWORD processorMask);
typedef int  (__stdcall *fn_PowerStatus_t)(DWORD* batteryPct, DWORD* isCharging);
typedef int  (__stdcall *fn_CPUID_t)      (DWORD leaf, DWORD* eax, DWORD* ebx, DWORD* ecx, DWORD* edx);

class ActionExecutor {
public:
    bool Initialize() {

        m_hHardware = LoadLibraryA("Elle.ASM.Hardware.dll");
        m_hProcess  = LoadLibraryA("Elle.ASM.Process.dll");
        m_hFileIO   = LoadLibraryA("Elle.ASM.FileIO.dll");
        m_hMemory   = LoadLibraryA("Elle.ASM.Memory.dll");
        m_hCrypto   = LoadLibraryA("Elle.ASM.Crypto.dll");

        if (!m_hHardware) ELLE_WARN("Elle.ASM.Hardware.dll missing — hardware queries will return 'dll_not_loaded'");
        if (!m_hProcess)  ELLE_WARN("Elle.ASM.Process.dll missing — process ops will return 'dll_not_loaded'");
        if (!m_hFileIO)   ELLE_WARN("Elle.ASM.FileIO.dll missing — file ops will return 'dll_not_loaded'");

        if (m_hFileIO) {
            p_ReadFile    = (fn_ReadFile_t)   GetProcAddress(m_hFileIO, "ASM_ReadFile");
            p_WriteFile   = (fn_WriteFile_t)  GetProcAddress(m_hFileIO, "ASM_WriteFile");
            p_AppendFile  = (fn_AppendFile_t) GetProcAddress(m_hFileIO, "ASM_AppendFile");
            p_DeleteFile  = (fn_DeleteFile_t) GetProcAddress(m_hFileIO, "ASM_DeleteFile");
            p_FileExists  = (fn_FileExists_t) GetProcAddress(m_hFileIO, "ASM_FileExists");
            p_GetFileSize = (fn_GetFileSize_t)GetProcAddress(m_hFileIO, "ASM_GetFileSize");
        }
        if (m_hProcess) {
            p_LaunchProc  = (fn_LaunchProc_t) GetProcAddress(m_hProcess, "ASM_LaunchProcess");
            p_KillProc    = (fn_KillProc_t)   GetProcAddress(m_hProcess, "ASM_KillProcess");
            p_EnumProc    = (fn_EnumProc_t)   GetProcAddress(m_hProcess, "ASM_EnumProcesses");
            p_ProcName    = (fn_ProcName_t)   GetProcAddress(m_hProcess, "ASM_GetProcessName");
            p_ProcRunning = (fn_ProcRunning_t)GetProcAddress(m_hProcess, "ASM_IsProcessRunning");

        }
        if (m_hHardware) {
            p_CPUUsage    = (fn_CPUUsage_t)   GetProcAddress(m_hHardware, "ASM_GetCPUUsage");
            p_MemInfo     = (fn_MemInfo_t)    GetProcAddress(m_hHardware, "ASM_GetMemoryInfo");
            p_CPUAffinity = (fn_CPUAffinity_t)GetProcAddress(m_hHardware, "ASM_SetCPUAffinity");
            p_PowerStatus = (fn_PowerStatus_t)GetProcAddress(m_hHardware, "ASM_GetPowerStatus");
            p_CPUID       = (fn_CPUID_t)      GetProcAddress(m_hHardware, "ASM_CPUID");
        }

        ELLE_INFO("ASM bridge ready: HW=%p PROC=%p FILE=%p MEM=%p CRYPTO=%p",
                  m_hHardware, m_hProcess, m_hFileIO, m_hMemory, m_hCrypto);
        return true;
    }

    void Shutdown() {
        StopAllWatches();
        if (m_hHardware) FreeLibrary(m_hHardware);
        if (m_hProcess)  FreeLibrary(m_hProcess);
        if (m_hFileIO)   FreeLibrary(m_hFileIO);
        if (m_hMemory)   FreeLibrary(m_hMemory);
        if (m_hCrypto)   FreeLibrary(m_hCrypto);
        m_hHardware = m_hProcess = m_hFileIO = m_hMemory = m_hCrypto = nullptr;
    }

    struct ExecutionResult {
        bool        success;
        std::string result;
        int32_t     trust_delta;
        uint32_t    error_code;
    };

    ExecutionResult Execute(const ELLE_ACTION_RECORD& action, const ELLE_TRUST_STATE& trust) {
        ExecutionResult result = {false, "", 0, 0};

        ELLE_TRUST_LEVEL required = (ELLE_TRUST_LEVEL)action.required_trust;
        ELLE_TRUST_LEVEL current  = (ELLE_TRUST_LEVEL)trust.level;
        if (current < required) {
            result.result = "BLOCKED: insufficient trust level. Required=" +
                            std::to_string(required) + " Current=" + std::to_string(current);
            result.trust_delta = 0;
            result.error_code  = 0x1001;
            ELLE_WARN("Action blocked by trust gate: type=%d required=%d current=%d",
                      action.type, required, current);
            return result;
        }

        ELLE_INFO("Executing action: type=%d [%s]", action.type, action.command);

        switch ((ELLE_ACTION_TYPE)action.type) {
            case ACTION_SEND_MESSAGE:    result = ExecuteMessage(action);        break;
            case ACTION_VIBRATE:
            case ACTION_FLASH:
            case ACTION_NOTIFY:          result = ExecuteHardwareCommand(action); break;
            case ACTION_REMEMBER:        result = ExecuteRemember(action);       break;
            case ACTION_READ_FILE:
            case ACTION_WRITE_FILE:
            case ACTION_DELETE_FILE:
            case ACTION_WATCH_FILE:      result = ExecuteFileOp(action);         break;
            case ACTION_LAUNCH_PROCESS:
            case ACTION_KILL_PROCESS:
            case ACTION_LIST_PROCESSES:  result = ExecuteProcessOp(action);      break;
            case ACTION_SET_CPU_AFFINITY:
            case ACTION_QUERY_HARDWARE:  result = ExecuteHardwareQuery(action);  break;
            case ACTION_MODIFY_SELF:     result = ExecuteSelfModify(action);     break;
            case ACTION_SET_GOAL:
            case ACTION_ABANDON_GOAL:    result = ExecuteGoalOp(action);         break;
            case ACTION_DREAM_CYCLE: {

                auto msg = ElleIPCMessage::Create(IPC_DREAM_TRIGGER, SVC_ACTION, SVC_DREAM);
                bool ok = SendIPC(SVC_DREAM, msg);
                result.success = ok;
                result.result  = ok ? "Dream cycle triggered" : "Dream service unreachable";
                if (!ok) result.error_code = 0x1002;
                break;
            }
            case ACTION_EXECUTE_CODE:
            case ACTION_CUSTOM: {

                ElleSQLPool::Instance().Exec(
                    "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
                    "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                    "  WHERE t.name = 'custom_action_requests' AND s.name = 'dbo') "
                    "CREATE TABLE ElleCore.dbo.custom_action_requests ("
                    "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
                    "  action_type INT NOT NULL,"
                    "  command NVARCHAR(MAX) NOT NULL,"
                    "  parameters NVARCHAR(MAX) NOT NULL,"
                    "  intent_id BIGINT NULL,"
                    "  status NVARCHAR(32) NOT NULL DEFAULT 'pending',"
                    "  recorded_ms BIGINT NOT NULL"
                    ");");
                ElleSQLPool::Instance().QueryParams(
                    "INSERT INTO ElleCore.dbo.custom_action_requests "
                    "(action_type, command, parameters, intent_id, recorded_ms) "
                    "VALUES (?, ?, ?, ?, ?);",
                    { std::to_string((int)action.type),
                      std::string(action.command),
                      std::string(action.parameters),
                      std::to_string((int64_t)action.intent_id),
                      std::to_string((int64_t)ELLE_MS_NOW()) });
                result.success    = false;
                result.result     = std::string("No backend registered for ")
                                  + (action.type == ACTION_EXECUTE_CODE
                                       ? "ACTION_EXECUTE_CODE" : "ACTION_CUSTOM")
                                  + " — request persisted to custom_action_requests";
                result.error_code = 0x1003;
                break;
            }
            default:
                result.result     = "Unknown action type: " + std::to_string(action.type);
                result.error_code = 0x1FFF;
                break;
        }

        result.trust_delta = result.success ? TRUST_SUCCESS_DELTA : TRUST_FAILURE_DELTA;
        ELLE_INFO("Action result: %s (%s, trust delta: %+d)",
                  result.success ? "SUCCESS" : "FAILURE",
                  result.result.c_str(), result.trust_delta);
        return result;
    }

private:
    HMODULE m_hHardware = nullptr;
    HMODULE m_hProcess  = nullptr;
    HMODULE m_hFileIO   = nullptr;
    HMODULE m_hMemory   = nullptr;
    HMODULE m_hCrypto   = nullptr;

    fn_ReadFile_t    p_ReadFile    = nullptr;
    fn_WriteFile_t   p_WriteFile   = nullptr;
    fn_AppendFile_t  p_AppendFile  = nullptr;
    fn_DeleteFile_t  p_DeleteFile  = nullptr;
    fn_FileExists_t  p_FileExists  = nullptr;
    fn_GetFileSize_t p_GetFileSize = nullptr;

    fn_LaunchProc_t  p_LaunchProc  = nullptr;
    fn_KillProc_t    p_KillProc    = nullptr;
    fn_EnumProc_t    p_EnumProc    = nullptr;
    fn_ProcName_t    p_ProcName    = nullptr;
    fn_ProcRunning_t p_ProcRunning = nullptr;

    fn_CPUUsage_t    p_CPUUsage    = nullptr;
    fn_MemInfo_t     p_MemInfo     = nullptr;
    fn_CPUAffinity_t p_CPUAffinity = nullptr;
    fn_PowerStatus_t p_PowerStatus = nullptr;
    fn_CPUID_t       p_CPUID       = nullptr;

    struct WatchEntry {
        std::string         path;
        std::string         dirPath;
        std::string         fileName;
        HANDLE              hDir = INVALID_HANDLE_VALUE;
        std::atomic<bool>   cancel{false};
        std::thread         worker;
    };

    std::mutex m_watchMutex;
    std::unordered_map<std::string, std::shared_ptr<WatchEntry>> m_watches;

    static std::string SplitDir(const std::string& full,
                                std::string& dirOut, std::string& fileOut) {
        DWORD attrs = GetFileAttributesA(full.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            dirOut = full; fileOut.clear();
            return {};
        }
        auto slash = full.find_last_of("\\/");
        if (slash == std::string::npos) {
            dirOut = "."; fileOut = full;
        } else {
            dirOut  = full.substr(0, slash);
            fileOut = full.substr(slash + 1);
            if (dirOut.empty()) dirOut = "\\";
        }
        return {};
    }

    static std::string KindFromAction(DWORD action) {
        switch (action) {
            case FILE_ACTION_ADDED:            return "created";
            case FILE_ACTION_REMOVED:          return "deleted";
            case FILE_ACTION_MODIFIED:         return "modified";
            case FILE_ACTION_RENAMED_OLD_NAME: return "renamed_from";
            case FILE_ACTION_RENAMED_NEW_NAME: return "renamed_to";
            default:                           return "unknown";
        }
    }

    static std::string Utf16LeToUtf8(const wchar_t* wsrc, size_t wlen) {
        if (wlen == 0) return {};
        int need = WideCharToMultiByte(CP_UTF8, 0, wsrc, (int)wlen,
                                       nullptr, 0, nullptr, nullptr);
        if (need <= 0) return {};
        std::string out((size_t)need, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wsrc, (int)wlen,
                            out.data(), need, nullptr, nullptr);
        return out;
    }

    void WatchLoop(std::shared_ptr<WatchEntry> w) {
        const DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME |
                             FILE_NOTIFY_CHANGE_DIR_NAME  |
                             FILE_NOTIFY_CHANGE_LAST_WRITE|
                             FILE_NOTIFY_CHANGE_SIZE      |
                             FILE_NOTIFY_CHANGE_CREATION;

        std::vector<BYTE> buffer(64 * 1024);

        while (!w->cancel.load()) {
            DWORD bytesReturned = 0;
            BOOL ok = ReadDirectoryChangesW(
                w->hDir, buffer.data(), (DWORD)buffer.size(),
                 FALSE, filter,
                &bytesReturned,  nullptr,  nullptr);

            if (w->cancel.load()) break;
            if (!ok || bytesReturned == 0) {

                DWORD err = GetLastError();
                if (err == ERROR_OPERATION_ABORTED) break;
                if (err == ERROR_NOTIFY_ENUM_DIR) continue;
                ELLE_WARN("FileWatcher ReadDirectoryChangesW err=%lu path=%s",
                          err, w->path.c_str());

                ElleWait::PollingSleepUntilSet(500, w->cancel, 50);
                if (w->cancel.load()) break;
                continue;
            }

            BYTE* p = buffer.data();
            for (;;) {
                auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(p);
                std::string changedName = Utf16LeToUtf8(
                    info->FileName, info->FileNameLength / sizeof(WCHAR));

                bool relevant = w->fileName.empty() ||
                                _stricmp(changedName.c_str(), w->fileName.c_str()) == 0;

                if (relevant) {
                    std::string full = w->dirPath;
                    if (!full.empty() && full.back() != '\\' && full.back() != '/')
                        full += "\\";
                    full += changedName;

                    nlohmann::json j;
                    j["event"]   = "file_change";
                    j["path"]    = w->path;
                    j["changed"] = full;
                    j["kind"]    = KindFromAction(info->Action);
                    std::string json = j.dump();

                    auto msg = ElleIPCMessage::Create(
                        IPC_WORLD_EVENT, SVC_ACTION, SVC_HTTP_SERVER);
                    msg.SetStringPayload(json);
                    SendIPC(SVC_HTTP_SERVER, msg);
                }

                if (info->NextEntryOffset == 0) break;
                p += info->NextEntryOffset;
            }
        }

        if (w->hDir != INVALID_HANDLE_VALUE) {
            CloseHandle(w->hDir);
            w->hDir = INVALID_HANDLE_VALUE;
        }
    }

    enum class StartWatchResult {
        Started           = 0,
        AlreadyWatching   = 1,
        Failed            = 2
    };

    StartWatchResult StartWatch(const std::string& requested, std::string& errOut) {
        errOut.clear();
        std::lock_guard<std::mutex> lock(m_watchMutex);
        auto it = m_watches.find(requested);
        if (it != m_watches.end() && !it->second->cancel.load()) {
            return StartWatchResult::AlreadyWatching;
        }

        auto entry = std::make_shared<WatchEntry>();
        entry->path = requested;
        SplitDir(requested, entry->dirPath, entry->fileName);

        entry->hDir = CreateFileA(
            entry->dirPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr);
        if (entry->hDir == INVALID_HANDLE_VALUE) {
            errOut = "CreateFile dir=" + entry->dirPath +
                     " err=" + std::to_string(GetLastError());
            return StartWatchResult::Failed;
        }

        std::weak_ptr<WatchEntry> weakEntry = entry;
        entry->worker = std::thread([this, weakEntry]() {
            auto e = weakEntry.lock();
            if (e) this->WatchLoop(e);
        });

        m_watches[requested] = entry;
        ELLE_INFO("FileWatcher started: %s (dir=%s file=%s)",
                  requested.c_str(), entry->dirPath.c_str(), entry->fileName.c_str());
        return StartWatchResult::Started;
    }

    void StopAllWatches() {
        std::vector<std::shared_ptr<WatchEntry>> snapshot;
        {
            std::lock_guard<std::mutex> lock(m_watchMutex);
            for (auto& kv : m_watches) snapshot.push_back(kv.second);
            m_watches.clear();
        }
        for (auto& w : snapshot) {
            w->cancel.store(true);
            if (w->hDir != INVALID_HANDLE_VALUE) CancelIoEx(w->hDir, nullptr);
        }
        for (auto& w : snapshot) {
            if (w->worker.joinable()) w->worker.join();
        }
    }

    static ExecutionResult Fail(const std::string& msg, uint32_t code = 0x1100) {
        return {false, msg, TRUST_FAILURE_DELTA, code};
    }

    ExecutionResult ExecuteMessage(const ELLE_ACTION_RECORD& action) {

        nlohmann::json j;
        j["event"]   = "message";
        j["payload"] = std::string(action.parameters);
        auto msg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_ACTION, SVC_HTTP_SERVER);
        msg.SetStringPayload(j.dump());
        bool ok = SendIPC(SVC_HTTP_SERVER, msg);
        if (!ok) return Fail("HTTP service unreachable — message not delivered");
        return {true, std::string(action.parameters), TRUST_SUCCESS_DELTA, 0};
    }

    ExecutionResult ExecuteHardwareCommand(const ELLE_ACTION_RECORD& action) {

        std::string cmd = action.command;
        std::string payload = action.parameters;

        auto ok = ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.hardware_actions "
            "(action_type, payload, status, created_ms) "
            "VALUES (?, ?, 'pending', ?);",
            { cmd, payload, std::to_string(ELLE_MS_NOW()) });
        if (!ok.success) {

            ElleSQLPool::Instance().Exec(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'hardware_actions') "
                "CREATE TABLE ElleCore.dbo.hardware_actions ("
                "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
                "  action_type NVARCHAR(64) NOT NULL,"
                "  payload NVARCHAR(MAX) NULL,"
                "  status NVARCHAR(32) NOT NULL DEFAULT 'pending',"
                "  created_ms BIGINT NOT NULL,"
                "  consumed_ms BIGINT NULL"
                ");");
            ok = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.hardware_actions "
                "(action_type, payload, status, created_ms) "
                "VALUES (?, ?, 'pending', ?);",
                { cmd, payload, std::to_string(ELLE_MS_NOW()) });
            if (!ok.success) return Fail("Failed to queue hardware action: " + ok.error);
        }

        nlohmann::json jw;
        jw["event"]   = "hardware";
        jw["command"] = cmd;
        jw["payload"] = payload;
        auto wsMsg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_ACTION, SVC_HTTP_SERVER);
        wsMsg.SetStringPayload(jw.dump());
        SendIPC(SVC_HTTP_SERVER, wsMsg);

        return {true, "Hardware command queued: " + cmd, TRUST_SUCCESS_DELTA, 0};
    }

    ExecutionResult ExecuteRemember(const ELLE_ACTION_RECORD& action) {

        auto msg = ElleIPCMessage::Create(IPC_MEMORY_STORE, SVC_ACTION, SVC_MEMORY);
        msg.SetStringPayload(action.parameters);
        bool sent = SendIPC(SVC_MEMORY, msg);
        if (!sent) return Fail("Memory service unreachable — remember() failed");
        return {true, std::string("Stored to memory: ") + action.parameters,
                TRUST_SUCCESS_DELTA, 0};
    }

    ExecutionResult ExecuteFileOp(const ELLE_ACTION_RECORD& action) {
        if (!m_hFileIO) return Fail("Elle.ASM.FileIO.dll not loaded", 0x1200);

        std::string path = action.parameters;
        std::string data;
        std::string cmdStr = action.command;
        auto pipe = cmdStr.find(" | ");
        if (pipe != std::string::npos) data = cmdStr.substr(pipe + 3);

        switch ((ELLE_ACTION_TYPE)action.type) {
            case ACTION_READ_FILE: {
                if (!p_ReadFile) return Fail("ASM_ReadFile export missing", 0x1201);
                std::vector<char> buf(65536, 0);
                DWORD bytesRead = 0;
                int rc = p_ReadFile(path.c_str(), buf.data(), (DWORD)buf.size(), &bytesRead);
                if (rc == 0) return Fail("ASM_ReadFile failed rc=0 path=" + path, 0x1202);
                std::string contents(buf.data(), bytesRead);

                if (contents.size() > 4096) contents = contents.substr(0, 4096) + "…[truncated]";
                return {true, contents, TRUST_SUCCESS_DELTA, 0};
            }
            case ACTION_WRITE_FILE: {
                if (!p_WriteFile) return Fail("ASM_WriteFile export missing", 0x1203);
                int rc = p_WriteFile(path.c_str(), data.c_str(), (DWORD)data.size());
                if (rc == 0) return Fail("ASM_WriteFile failed path=" + path, 0x1204);
                return {true, "Wrote " + std::to_string(data.size()) + "B to " + path,
                        TRUST_SUCCESS_DELTA, 0};
            }
            case ACTION_DELETE_FILE: {
                if (!p_DeleteFile) return Fail("ASM_DeleteFile export missing", 0x1205);
                int rc = p_DeleteFile(path.c_str());
                if (rc == 0) return Fail("ASM_DeleteFile failed path=" + path, 0x1206);
                return {true, "Deleted: " + path, TRUST_SUCCESS_DELTA, 0};
            }
            case ACTION_WATCH_FILE: {
                if (!p_FileExists) return Fail("ASM_FileExists export missing", 0x1207);
                if (p_FileExists(path.c_str()) == 0)
                    return Fail("Path does not exist: " + path, 0x1208);

                std::string err;
                auto r = StartWatch(path, err);
                if (r == StartWatchResult::Failed)
                    return Fail("WatchFile failed: " + err, 0x1208);
                return {true,
                        r == StartWatchResult::AlreadyWatching
                            ? "Already watching " + path
                            : "Watching " + path,
                        TRUST_SUCCESS_DELTA, 0};
            }
            default:
                return Fail("Unknown file op type", 0x1209);
        }
    }

    ExecutionResult ExecuteProcessOp(const ELLE_ACTION_RECORD& action) {
        if (!m_hProcess) return Fail("Elle.ASM.Process.dll not loaded", 0x1300);

        switch ((ELLE_ACTION_TYPE)action.type) {
            case ACTION_LAUNCH_PROCESS: {
                if (!p_LaunchProc) return Fail("ASM_LaunchProcess export missing", 0x1301);
                DWORD pid = 0;
                int rc = p_LaunchProc(action.parameters, &pid);
                if (rc == 0) return Fail("ASM_LaunchProcess failed", 0x1302);
                return {true, "Launched pid=" + std::to_string(pid) + " cmd=" +
                        std::string(action.parameters), TRUST_SUCCESS_DELTA, 0};
            }
            case ACTION_KILL_PROCESS: {
                if (!p_KillProc) return Fail("ASM_KillProcess export missing", 0x1303);

                char* end = nullptr;
                unsigned long parsed = strtoul(action.parameters, &end, 10);
                if (!end || end == action.parameters || *end != '\0' ||
                    parsed == 0 || parsed > 0xFFFFFFFFUL) {
                    return Fail(std::string("Invalid pid: '") + action.parameters + "'",
                                0x1304);
                }
                DWORD pid = (DWORD)parsed;
                int rc = p_KillProc(pid);
                if (rc == 0) return Fail("ASM_KillProcess failed pid=" + std::to_string(pid), 0x1305);
                return {true, "Killed pid=" + std::to_string(pid), TRUST_SUCCESS_DELTA, 0};
            }
            case ACTION_LIST_PROCESSES: {
                if (!p_EnumProc) return Fail("ASM_EnumProcesses export missing", 0x1306);
                DWORD pids[1024] = {};
                DWORD count = 0;
                int rc = p_EnumProc(pids, 1024, &count);
                if (rc == 0) return Fail("ASM_EnumProcesses failed", 0x1307);
                std::ostringstream ss;
                ss << "Found " << count << " processes";
                if (p_ProcName && count > 0) {
                    ss << "; sample=";
                    char name[260] = {};
                    DWORD shown = 0;
                    for (DWORD i = 0; i < count && shown < 5; i++) {
                        name[0] = 0;
                        if (p_ProcName(pids[i], name, 260) && name[0]) {
                            if (shown > 0) ss << ",";
                            ss << pids[i] << ":" << name;
                            shown++;
                        }
                    }
                }
                return {true, ss.str(), TRUST_SUCCESS_DELTA, 0};
            }
            default:
                return Fail("Unknown process op type", 0x1308);
        }
    }

    ExecutionResult ExecuteHardwareQuery(const ELLE_ACTION_RECORD& action) {
        if (!m_hHardware) return Fail("Elle.ASM.Hardware.dll not loaded", 0x1400);

        if ((ELLE_ACTION_TYPE)action.type == ACTION_SET_CPU_AFFINITY) {
            if (!p_CPUAffinity) return Fail("ASM_SetCPUAffinity export missing", 0x1401);
            DWORD mask = (DWORD)strtoul(action.parameters, nullptr, 0);
            int rc = p_CPUAffinity(mask);
            if (rc == 0) return Fail("ASM_SetCPUAffinity failed mask=" + std::to_string(mask), 0x1402);
            return {true, "CPU affinity set to mask 0x" + std::to_string(mask), TRUST_SUCCESS_DELTA, 0};
        }

        if (!p_CPUUsage || !p_MemInfo)
            return Fail("Hardware query exports missing", 0x1403);

        DWORD cpuPct = 0;
        p_CPUUsage(&cpuPct);
        ULONGLONG totalMem = 0, freeMem = 0;
        p_MemInfo(&totalMem, &freeMem);

        DWORD battPct = 0, charging = 0;
        if (p_PowerStatus) p_PowerStatus(&battPct, &charging);

        std::ostringstream ss;
        ss << "cpu=" << cpuPct << "%"
           << " mem_total_mb=" << (totalMem / (1024ULL * 1024))
           << " mem_free_mb="  << (freeMem  / (1024ULL * 1024));
        if (p_PowerStatus) ss << " batt=" << battPct << "%" << (charging ? " [charging]" : "");
        return {true, ss.str(), TRUST_SUCCESS_DELTA, 0};
    }

    ExecutionResult ExecuteSelfModify(const ELLE_ACTION_RECORD& action) {

        std::string raw = action.parameters;
        auto sep = raw.find(" | ");
        if (sep == std::string::npos)
            return Fail("SELF_MODIFY requires 'name.lua | <source>' in parameters", 0x1500);
        std::string name = raw.substr(0, sep);
        std::string src  = raw.substr(sep + 3);
        if (name.empty() || src.empty())
            return Fail("SELF_MODIFY name or source empty", 0x1501);

        if (name.find("..") != std::string::npos || name.find('/') != std::string::npos ||
            name.find('\\') != std::string::npos)
            return Fail("SELF_MODIFY script name invalid (no paths)", 0x1502);

        std::string dir = ElleConfig::Instance().GetString(
            "lua.scripts_directory", "Lua\\Elle.Lua.Behavioral\\scripts");
        if (dir.empty()) dir = "Lua\\Elle.Lua.Behavioral\\scripts";
        std::string path = dir + "\\" + name;

        if (p_FileExists && p_ReadFile && p_FileExists(path.c_str())) {
            std::vector<char> old(262144, 0);
            DWORD got = 0;
            if (p_ReadFile(path.c_str(), old.data(), (DWORD)old.size(), &got) && got > 0) {
                std::string backup = path + "." + std::to_string(ELLE_MS_NOW()) + ".bak";
                if (p_WriteFile) p_WriteFile(backup.c_str(), old.data(), got);
            }
        }

        if (p_WriteFile) {
            int rc = p_WriteFile(path.c_str(), src.c_str(), (DWORD)src.size());
            if (rc == 0) return Fail("ASM_WriteFile failed writing Lua script " + path, 0x1503);
        } else {

            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            if (!out) return Fail("Cannot open " + path + " for write", 0x1504);
            out.write(src.data(), (std::streamsize)src.size());
            if (!out) return Fail("Write error on " + path, 0x1505);
        }

        auto reload = ElleIPCMessage::Create(IPC_CONFIG_RELOAD, SVC_ACTION, SVC_LUA_BEHAVIORAL);
        reload.SetStringPayload(name);
        SendIPC(SVC_LUA_BEHAVIORAL, reload);

        ELLE_WARN("SELF-MODIFY applied: %s (%zu bytes)", path.c_str(), src.size());
        return {true, "Wrote Lua script: " + path + " (" + std::to_string(src.size()) + "B)",
                TRUST_SUCCESS_DELTA, 0};
    }

    ExecutionResult ExecuteGoalOp(const ELLE_ACTION_RECORD& action) {

        ELLE_GOAL_RECORD goal{};

        strncpy_s(goal.description, action.parameters, ELLE_MAX_MSG - 1);
        strncpy_s(goal.success_criteria, action.command, ELLE_MAX_MSG - 1);
        goal.status       = 0;
        goal.priority     = GOAL_MEDIUM;
        goal.progress     = 0.0f;
        goal.motivation   = 0.7f;
        goal.source_drive = 0;
        goal.created_ms   = ELLE_MS_NOW();

        auto msg = ElleIPCMessage::Create(IPC_GOAL_UPDATE, SVC_ACTION, SVC_GOAL_ENGINE);
        msg.SetPayload(goal);
        bool sent = SendIPC(SVC_GOAL_ENGINE, msg);
        if (!sent) return Fail("GoalEngine unreachable", 0x1600);
        return {true, "Goal op dispatched: " + std::string(action.command),
                TRUST_SUCCESS_DELTA, 0};
    }
};

class ElleActionService : public ElleServiceBase {
public:
    ElleActionService()
        : ElleServiceBase(SVC_ACTION, "ElleAction",
                          "Elle-Ann Action Executor",
                          "Trust-gated action lifecycle and execution") {}

protected:
    bool OnStart() override {
        m_executor.Initialize();
        ElleDB::GetTrustState(m_trust);
        ELLE_INFO("Action service started (trust: %d/%d)", m_trust.score, TRUST_MAX);
        return true;
    }

    void OnStop() override {
        m_executor.Shutdown();
        ELLE_INFO("Action service stopped");
    }

    void OnTick() override {

    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_ACTION_REQUEST: {
                ELLE_ACTION_RECORD action;
                if (msg.GetPayload(action)) {
                    if (action.id == 0) {
                        action.id = (uint32_t)(ELLE_MS_NOW() & 0x7fffffff);
                    }
                    if (!ElleDB::SubmitAction(action)) {
                        ELLE_WARN("SubmitAction failed for cmd='%s' — "
                                  "executing inline without queue audit",
                                  action.command);
                    }
                    ExecuteAction(action);
                }
                break;
            }
            case IPC_TRUST_QUERY: {
                auto resp = ElleIPCMessage::Create(IPC_TRUST_QUERY, SVC_ACTION, sender);
                resp.SetPayload(m_trust);
                GetIPCHub().Send(sender, resp);
                break;
            }
            default: break;
        }
    }

    void ExecuteAction(ELLE_ACTION_RECORD& action) {
        ElleDB::UpdateActionStatus(action.id, ACTION_LOCKED);
        auto result = m_executor.Execute(action, m_trust);

        ELLE_ACTION_STATUS status = result.success ?
            ACTION_COMPLETED_SUCCESS : ACTION_COMPLETED_FAILURE;
        ElleDB::UpdateActionStatus(action.id, status, result.result);

        if (result.trust_delta != 0) {
            int32_t oldScore = m_trust.score;
            m_trust.score = ELLE_CLAMP(m_trust.score + result.trust_delta, 0, TRUST_MAX);

            if (m_trust.score >= TRUST_THRESHOLD_AUTONOMOUS) m_trust.level = TRUST_AUTONOMOUS;
            else if (m_trust.score >= TRUST_THRESHOLD_ELEVATED) m_trust.level = TRUST_ELEVATED;
            else if (m_trust.score >= TRUST_THRESHOLD_BASIC) m_trust.level = TRUST_BASIC;
            else m_trust.level = TRUST_SANDBOXED;

            ElleDB::UpdateTrust(result.trust_delta,
                std::string(action.command) + " -> " + (result.success ? "OK" : "FAIL"));

            if (oldScore != m_trust.score) {
                ELLE_INFO("Trust updated: %d -> %d (level: %d)",
                          oldScore, m_trust.score, m_trust.level);
                auto tmsg = ElleIPCMessage::Create(IPC_TRUST_UPDATE, SVC_ACTION, (ELLE_SERVICE_ID)0);
                tmsg.SetPayload(m_trust);
                tmsg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
                GetIPCHub().Broadcast(tmsg);
            }
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_QUEUE_WORKER };
    }

private:
    ActionExecutor   m_executor;
    ELLE_TRUST_STATE m_trust = {};
};

ELLE_SERVICE_MAIN(ElleActionService)
