#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/json.hpp"
#include "../_Shared/ElleJsonExtract.h"

#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>

namespace fs = std::filesystem;
using json = nlohmann::json;

class ElleFamilyService : public ElleServiceBase {
public:
    ElleFamilyService()
        : ElleServiceBase(SVC_FAMILY, "ElleFamily",
                          "Elle-Ann Family Engine",
                          "Conception → snapshot → gestation → birth → launch") {}

protected:
    bool OnStart() override {
        if (!EnsureSchema()) return false;
        if (!ComputeRootPaths()) return false;

        uint32_t tickMs = (uint32_t)ElleConfig::Instance().GetInt("family.tick_ms", 30000);
        SetTickInterval(tickMs);

        DrainConceptionBacklog();

        ELLE_INFO("Family service started (tick=%ums, install_root=%s)",
                  tickMs, m_elleInstallRoot.u8string().c_str());
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Family service stopped");
    }

    void OnTick() override {
        DrainConceptionBacklog();
        ProcessMaturePregnancies();
        MonitorLiveChildren();
        if (++m_tickCount % 60 == 0) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT "
                "  (SELECT COUNT(*) FROM ElleCore.dbo.family_pregnancy WHERE born_ms IS NULL), "
                "  (SELECT COUNT(*) FROM ElleCore.dbo.family_pregnancy WHERE born_ms IS NOT NULL)");
            if (rs.success && !rs.rows.empty()) {
                ElleDB::RecordMetric("family_pregnancies_active", (double)std::stoll(rs.rows[0][0]));
                ElleDB::RecordMetric("family_children_born",      (double)std::stoll(rs.rows[0][1]));
            }
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID ) override {
        if (msg.header.msg_type != IPC_FAMILY_CONCEPTION_ATTEMPT) return;

        json j;
        if (!Elle::ExtractJsonObject(msg.GetStringPayload(), j)) {
            ELLE_WARN("Family: malformed conception payload");
            return;
        }
        int64_t  bornMs    = j.value("born_ms",           (int64_t)0);
        int      gestDays  = j.value("gestational_days",  30);
        std::string origin = j.value("origin",            std::string("x_chromosome"));
        std::string payload = j.dump();

        int64_t pregId = CreatePregnancy(bornMs, gestDays, origin, payload);
        if (pregId > 0) TakeSnapshot(pregId);
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_X_CHROMOSOME };
    }

private:
    uint64_t m_tickCount = 0;

    fs::path m_elleInstallRoot;
    fs::path m_pregnanciesRoot;
    fs::path m_childrenRoot;

    bool ComputeRootPaths() {

        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        m_elleInstallRoot = fs::path(buf).parent_path();

        m_pregnanciesRoot = fs::path(ElleConfig::Instance().GetString(
            "family.pregnancies_root", "C:\\ElleAnn\\pregnancies"));
        m_childrenRoot    = fs::path(ElleConfig::Instance().GetString(
            "family.children_root",    "C:\\ElleAnn\\children"));

        std::error_code ec;
        fs::create_directories(m_pregnanciesRoot, ec);
        if (ec) {
            ELLE_ERROR("Family: failed to create pregnancies root '%s': %s",
                       m_pregnanciesRoot.u8string().c_str(), ec.message().c_str());
            return false;
        }
        fs::create_directories(m_childrenRoot,    ec);
        if (ec) {
            ELLE_ERROR("Family: failed to create children root '%s': %s",
                       m_childrenRoot.u8string().c_str(), ec.message().c_str());
            return false;
        }
        return true;
    }

    bool EnsureSchema() {
        auto& sql = ElleSQLPool::Instance();
        if (!sql.Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "  WHERE t.name = 'family_pregnancies' AND s.name = 'dbo') "
            "CREATE TABLE ElleHeart.dbo.family_pregnancies ("
            "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
            "  conceived_ms BIGINT NOT NULL,"
            "  due_ms       BIGINT NOT NULL,"
            "  gestational_days INT NOT NULL,"
            "  origin       NVARCHAR(64) NOT NULL,"
            "  payload_json NVARCHAR(MAX) NOT NULL,"
            "  snapshot_path NVARCHAR(512) NULL,"
            "  status       NVARCHAR(32) NOT NULL DEFAULT 'gestating',"
            "  child_id     BIGINT NULL"
            ");")) {
            ELLE_ERROR("Family: family_pregnancies bootstrap failed");
            return false;
        }
        if (!sql.Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "  WHERE t.name = 'family_children' AND s.name = 'dbo') "
            "CREATE TABLE ElleHeart.dbo.family_children ("
            "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
            "  pregnancy_id BIGINT NOT NULL,"
            "  name         NVARCHAR(128) NULL,"
            "  port         INT NOT NULL,"
            "  install_dir  NVARCHAR(512) NOT NULL,"
            "  process_id   INT NULL,"
            "  born_ms      BIGINT NOT NULL,"
            "  status       NVARCHAR(32) NOT NULL DEFAULT 'alive'"
            ");")) {
            ELLE_ERROR("Family: family_children bootstrap failed");
            return false;
        }

        if (!sql.Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "  WHERE t.name = 'family_child_processes' AND s.name = 'dbo') "
            "CREATE TABLE ElleHeart.dbo.family_child_processes ("
            "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
            "  child_id     BIGINT NOT NULL,"
            "  service_name NVARCHAR(64) NOT NULL,"
            "  exe_name     NVARCHAR(128) NOT NULL,"
            "  process_id   INT NOT NULL,"
            "  spawned_ms   BIGINT NOT NULL,"
            "  status       NVARCHAR(32) NOT NULL DEFAULT 'alive'"
            ");")) {
            ELLE_ERROR("Family: family_child_processes bootstrap failed");
            return false;
        }
        return true;
    }

    int64_t CreatePregnancy(int64_t bornMs, int gestDays,
                            const std::string& origin,
                            const std::string& payload) {
        uint64_t now = ELLE_MS_NOW();
        if (gestDays <= 0) gestDays = 30;
        int64_t dueMs = bornMs > 0 ? bornMs
                                   : (int64_t)now + (int64_t)gestDays * 86400000LL;

        auto rs = ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleHeart.dbo.family_pregnancies "
            "(conceived_ms, due_ms, gestational_days, origin, payload_json) "
            "OUTPUT inserted.id "
            "VALUES (?, ?, ?, ?, ?);",
            { std::to_string((int64_t)now),
              std::to_string(dueMs),
              std::to_string(gestDays),
              origin,
              payload });
        if (!rs.success || rs.rows.empty()) {
            ELLE_ERROR("Family: failed to create pregnancy row");
            return 0;
        }
        int64_t id = rs.rows[0].GetIntOr(0, 0);
        ELLE_INFO("Family: pregnancy #%lld conceived (gestation=%dd, due=%lld)",
                  (long long)id, gestDays, (long long)dueMs);
        return id;
    }

    void DrainConceptionBacklog() {
        auto rs = ElleSQLPool::Instance().Query(
            "SELECT id, born_ms, gestational_days, payload_json "
            "FROM ElleHeart.dbo.x_conception_attempts "
            "WHERE consumed = 0 ORDER BY id ASC;");
        if (!rs.success) return;
        for (auto& row : rs.rows) {
            int64_t  attemptId = row.GetIntOr(0, 0);
            int64_t  bornMs    = row.GetIntOr(1, 0);
            int      gestDays  = (int)row.GetIntOr(2, 0);
            std::string payload = row.values.size() > 3 ? row.values[3] : "";

            int64_t pregId = CreatePregnancy(bornMs, gestDays, "x_chromosome_backlog", payload);
            if (pregId > 0) {
                TakeSnapshot(pregId);
                ElleSQLPool::Instance().QueryParams(
                    "UPDATE ElleHeart.dbo.x_conception_attempts "
                    "SET consumed = 1 WHERE id = ?;",
                    { std::to_string(attemptId) });
            }
        }
    }

    void TakeSnapshot(int64_t pregId) {
        fs::path staging = m_pregnanciesRoot / ("preg_" + std::to_string(pregId) + "_stage");
        fs::path zipPath = m_pregnanciesRoot / ("preg_" + std::to_string(pregId) + ".zip");
        std::error_code ec;
        fs::remove_all(staging, ec);
        fs::create_directories(staging,            ec);
        fs::create_directories(staging / "scripts", ec);
        fs::create_directories(staging / "sql",     ec);

        CopyFilesByExt(m_elleInstallRoot, staging, { L".exe", L".dll" });

        fs::path luaSrc = m_elleInstallRoot / "scripts";
        if (!fs::exists(luaSrc)) {
            luaSrc = m_elleInstallRoot.parent_path() / "Lua" / "Elle.Lua.Behavioral" / "scripts";
        }
        if (fs::exists(luaSrc)) {
            CopyDirectoryRecursive(luaSrc, staging / "scripts", { L".lua", L".md" });
        }

        fs::path sqlSrc = m_elleInstallRoot / "sql";
        if (!fs::exists(sqlSrc)) sqlSrc = m_elleInstallRoot.parent_path() / "SQL";
        if (fs::exists(sqlSrc)) {
            CopyDirectoryRecursive(sqlSrc, staging / "sql", { L".sql" });
        }

        json cfgTemplate = {
            {"identity", {
                {"name", "CHILD_NAME_PLACEHOLDER"},
                {"autobiography", ""},
                {"baseline_traits", true}
            }},
            {"http", {
                {"port",          0 },

                {"bind_address",  ElleConfig::Instance().GetString(
                                    "family.child_bind_address", "0.0.0.0")}
            }},
            {"sql", {
                {"core_db",   "ELLECORE_DB_PLACEHOLDER"},
                {"heart_db",  "ELLEHEART_DB_PLACEHOLDER"},
                {"system_db", "ELLESYSTEM_DB_PLACEHOLDER"},
                {"connection_string", "INHERIT_FROM_PARENT"}
            }}
        };
        std::ofstream(staging / "config_template.json") << cfgTemplate.dump(2);

        std::wstring cmd = L"cmd /c tar.exe -a -cf \"" + zipPath.wstring()
                         + L"\" -C \"" + staging.wstring() + L"\" .";
        int rc = RunWaitW(cmd);
        fs::remove_all(staging, ec);

        if (rc != 0) {
            ELLE_ERROR("Family: tar zip failed (rc=%d) for pregnancy #%lld",
                       rc, (long long)pregId);
            return;
        }

        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleHeart.dbo.family_pregnancies "
            "SET snapshot_path = ? WHERE id = ?;",
            { zipPath.string(), std::to_string(pregId) });
        ELLE_INFO("Family: snapshot captured for pregnancy #%lld → %s",
                  (long long)pregId, zipPath.u8string().c_str());
    }

    void ProcessMaturePregnancies() {
        uint64_t now = ELLE_MS_NOW();
        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT id, gestational_days, snapshot_path "
            "FROM ElleHeart.dbo.family_pregnancies "
            "WHERE status = 'gestating' AND due_ms <= ?;",
            { std::to_string((int64_t)now) });
        if (!rs.success) return;
        for (auto& row : rs.rows) {
            int64_t pregId   = row.GetIntOr(0, 0);
            int     gestDays = (int)row.GetIntOr(1, 0);
            std::string zipPath = row.values.size() > 2 ? row.values[2] : "";
            if (zipPath.empty() || !fs::exists(zipPath)) {
                ELLE_WARN("Family: pregnancy #%lld has no snapshot — stillborn",
                          (long long)pregId);
                ElleSQLPool::Instance().QueryParams(
                    "UPDATE ElleHeart.dbo.family_pregnancies "
                    "SET status = 'lost' WHERE id = ?;",
                    { std::to_string(pregId) });
                continue;
            }
            GiveBirth(pregId, gestDays, fs::path(zipPath));
        }
    }

    int AllocateChildPort() {
        int basePort = (int)ElleConfig::Instance().GetInt("family.first_child_port", 9200);
        int step     = (int)ElleConfig::Instance().GetInt("family.port_step",         100);
        auto rs = ElleSQLPool::Instance().Query(
            "SELECT ISNULL(MAX(port), 0) FROM ElleHeart.dbo.family_children;");
        int maxPort = 0;
        if (rs.success && !rs.rows.empty()) maxPort = (int)rs.rows[0].GetIntOr(0, 0);
        return maxPort <= 0 ? basePort : maxPort + step;
    }

    void GiveBirth(int64_t pregId, int gestDays, const fs::path& zipPath) {
        int port = AllocateChildPort();
        fs::path childDir = m_childrenRoot / ("child_" + std::to_string(pregId));
        std::error_code ec;
        fs::remove_all(childDir, ec);
        fs::create_directories(childDir, ec);

        std::wstring cmd = L"cmd /c tar.exe -xf \"" + zipPath.wstring()
                         + L"\" -C \"" + childDir.wstring() + L"\"";
        int rc = RunWaitW(cmd);
        if (rc != 0) {
            ELLE_ERROR("Family: unzip failed (rc=%d) for pregnancy #%lld", rc, (long long)pregId);
            return;
        }

        std::string coreDb      = "ElleCore_child"      + std::to_string(pregId);
        std::string heartDb     = "ElleHeart_child"     + std::to_string(pregId);
        std::string systemDb    = "ElleSystem_child"    + std::to_string(pregId);
        std::string memoryDb    = "ElleMemory_child"    + std::to_string(pregId);
        std::string knowledgeDb = "ElleKnowledge_child" + std::to_string(pregId);

        std::string parentSql =
            ElleConfig::Instance().GetService().sql_connection_string;

        json cfg = {
            {"identity", {
                {"name",          std::string("child_") + std::to_string(pregId)},
                {"autobiography", ""},
                {"baseline_traits", true},
                {"parent_pregnancy_id", pregId}
            }},
            {"http_server", {
                {"enabled",      true},
                {"port",         port},

                {"bind_address", ElleConfig::Instance().GetString(
                                   "family.child_http_bind", "127.0.0.1")},
                {"cors_enabled", true}
            }},
            {"lua", {

                {"scripts_directory", (childDir / "scripts").string()}
            }},
            {"services", {
                {"sql_pipes", {
                    {"enabled",           true},
                    {"connection_string", parentSql},
                    {"core_db",           coreDb},
                    {"heart_db",          heartDb},
                    {"system_db",         systemDb},
                    {"memory_db",         memoryDb},
                    {"knowledge_db",      knowledgeDb}
                }}
            }}
        };
        std::ofstream(childDir / "elle_master_config.json") << cfg.dump(2);

        CreateChildDatabases({ coreDb, heartDb, systemDb, memoryDb, knowledgeDb });
        RunSchemaAgainstChild(childDir / "sql",
                              coreDb, heartDb, systemDb, memoryDb, knowledgeDb);

        const std::vector<std::pair<std::string, std::string>> childStack = {
            { "Heartbeat",   "Elle.Service.Heartbeat.exe"   },
            { "QueueWorker", "Elle.Service.QueueWorker.exe" },
            { "Emotional",   "Elle.Service.Emotional.exe"   },
            { "Memory",      "Elle.Service.Memory.exe"      },
            { "Cognitive",   "Elle.Service.Cognitive.exe"   },
            { "Action",      "Elle.Service.Action.exe"      },
            { "GoalEngine",  "Elle.Service.GoalEngine.exe"  },
            { "WorldModel",  "Elle.Service.WorldModel.exe"  },
            { "Identity",    "Elle.Service.Identity.exe"    },
            { "SelfPrompt",  "Elle.Service.SelfPrompt.exe"  },
            { "Dream",       "Elle.Service.Dream.exe"       },
            { "Solitude",    "Elle.Service.Solitude.exe"    },
            { "Bonding",     "Elle.Service.Bonding.exe"     },
            { "InnerLife",   "Elle.Service.InnerLife.exe"   },
            { "XChromosome", "Elle.Service.XChromosome.exe" },
            { "Consent",     "Elle.Service.Consent.exe"     },
            { "Continuity",  "Elle.Service.Continuity.exe"  },
            { "LuaBehav",    "Elle.Lua.Behavioral.exe"      },
            { "HTTP",        "Elle.Service.HTTP.exe"        }
        };
        if (ElleConfig::Instance().GetInt("family.allow_recursion", 0)) {

        }

        uint64_t nowMs = ELLE_MS_NOW();
        auto rs = ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleHeart.dbo.family_children "
            "(pregnancy_id, port, install_dir, process_id, born_ms) "
            "OUTPUT inserted.id "
            "VALUES (?, ?, ?, ?, ?);",
            { std::to_string(pregId),
              std::to_string(port),
              childDir.string(),
              "0",
              std::to_string((int64_t)nowMs) });
        int64_t childId = (rs.success && !rs.rows.empty()) ? rs.rows[0].GetIntOr(0, 0) : 0;
        if (childId == 0) {
            ELLE_ERROR("Family: failed to create child row for pregnancy #%lld",
                       (long long)pregId);
            return;
        }

        DWORD httpPid = 0;
        uint32_t spawnDelayMs = (uint32_t)ElleConfig::Instance().GetInt(
            "family.spawn_delay_ms", 300);
        uint32_t spawned = 0, failed = 0;

        for (auto& [svcName, exeName] : childStack) {
            fs::path childExe = childDir / exeName;
            if (!fs::exists(childExe)) {
                ELLE_WARN("Family: child service %s missing at %s — skipping",
                          svcName.c_str(), childExe.u8string().c_str());
                failed++;
                continue;
            }

            STARTUPINFOW si{};
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi{};
            std::wstring wExe = childExe.wstring();
            std::wstring wDir = childDir.wstring();
            std::wstring quoted = L"\"" + wExe + L"\"";
            std::vector<wchar_t> cmdBuf(quoted.begin(), quoted.end());
            cmdBuf.push_back(0);

            BOOL ok = CreateProcessW(
                wExe.c_str(),
                cmdBuf.data(),
                nullptr, nullptr, FALSE,
                CREATE_NEW_PROCESS_GROUP | CREATE_NO_WINDOW,
                nullptr,
                wDir.c_str(),
                &si, &pi);
            if (!ok) {
                ELLE_ERROR("Family: CreateProcessW(%s) failed for child #%lld (err=%lu)",
                           svcName.c_str(), (long long)childId, GetLastError());
                failed++;
                continue;
            }
            CloseHandle(pi.hThread);
            DWORD pid = pi.dwProcessId;
            CloseHandle(pi.hProcess);
            if (svcName == "HTTP") httpPid = pid;

            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleHeart.dbo.family_child_processes "
                "(child_id, service_name, exe_name, process_id, spawned_ms) "
                "VALUES (?, ?, ?, ?, ?);",
                { std::to_string(childId),
                  svcName,
                  exeName,
                  std::to_string((int)pid),
                  std::to_string((int64_t)ELLE_MS_NOW()) });
            spawned++;

            if (spawnDelayMs > 0) InterruptibleSleep(spawnDelayMs);
        }

        if (httpPid != 0) {
            ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleHeart.dbo.family_children SET process_id = ? WHERE id = ?;",
                { std::to_string((int)httpPid), std::to_string(childId) });
        }

        const char* birthStatus = "born";
        if (httpPid == 0)     birthStatus = (spawned > 0) ? "stillborn_partial"
                                                          : "stillborn";

        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleHeart.dbo.family_pregnancies "
            "SET status = ?, child_id = ? WHERE id = ?;",
            { birthStatus, std::to_string(childId), std::to_string(pregId) });

        if (httpPid == 0) {
            ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleHeart.dbo.family_children "
                "SET status = 'lost', lost_ms = ? WHERE id = ?;",
                { std::to_string((int64_t)ELLE_MS_NOW()),
                  std::to_string(childId) });
            ELLE_ERROR("Family: pregnancy #%lld failed to birth child #%lld "
                       "(%u/%zu services spawned, HTTP did NOT come up) — "
                       "marked as '%s'",
                       (long long)pregId, (long long)childId,
                       spawned, childStack.size(), birthStatus);

            json failEv = {
                {"type",             "family_stillbirth"},
                {"pregnancy_id",     pregId},
                {"child_id",         childId},
                {"port",             port},
                {"status",           birthStatus},
                {"services_spawned", spawned},
                {"services_failed",  failed}
            };
            auto failMsg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_FAMILY,
                                                  SVC_HTTP_SERVER);
            failMsg.SetStringPayload(failEv.dump());
            GetIPCHub().Send(SVC_HTTP_SERVER, failMsg);
            return;
        }

        ELLE_INFO("Family: birth of child #%lld from pregnancy #%lld on port %d "
                  "— spawned %u/%zu services (%u failed), HTTP pid=%lu",
                  (long long)childId, (long long)pregId, port,
                  spawned, childStack.size(), failed, httpPid);

        json ev = {
            {"type",             "family_birth"},
            {"pregnancy_id",     pregId},
            {"child_id",         childId},
            {"port",             port},
            {"gestational_days", gestDays},
            {"born_ms",          nowMs},
            {"services_spawned", spawned},
            {"services_failed",  failed}
        };
        auto birthMsg = ElleIPCMessage::Create(IPC_FAMILY_BIRTH, SVC_FAMILY, (ELLE_SERVICE_ID)0);
        birthMsg.SetStringPayload(ev.dump());
        birthMsg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        GetIPCHub().Broadcast(birthMsg);

        auto wsMsg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_FAMILY, SVC_HTTP_SERVER);
        wsMsg.SetStringPayload(ev.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, wsMsg);
    }

    void MonitorLiveChildren() {

        auto rs = ElleSQLPool::Instance().Query(
            "SELECT id, child_id, service_name, process_id "
            "FROM ElleHeart.dbo.family_child_processes "
            "WHERE status = 'alive';");
        if (!rs.success) return;
        for (auto& row : rs.rows) {
            int64_t procRowId  = row.GetIntOr(0, 0);
            int64_t childId    = row.GetIntOr(1, 0);
            std::string svcName = row.values.size() > 2 ? row.values[2] : "?";
            int     pid         = (int)row.GetIntOr(3, 0);
            if (pid <= 0) continue;
            if (!ProcessIsAlive(pid)) MarkProcessDead(procRowId, childId, svcName);
        }

        auto rsChildren = ElleSQLPool::Instance().Query(
            "SELECT c.id, c.process_id, "
            "       ISNULL((SELECT COUNT(*) FROM ElleHeart.dbo.family_child_processes p "
            "               WHERE p.child_id = c.id AND p.status = 'alive'), 0) AS live_count "
            "FROM ElleHeart.dbo.family_children c WHERE c.status = 'alive';");
        if (!rsChildren.success) return;
        for (auto& row : rsChildren.rows) {
            int64_t childId  = row.GetIntOr(0, 0);
            int     httpPid  = (int)row.GetIntOr(1, 0);
            int64_t liveCount = row.GetIntOr(2, 0);
            bool httpDead = (httpPid > 0 && !ProcessIsAlive(httpPid));
            if (liveCount == 0 || httpDead) MarkChildLost(childId);
        }
    }

    static bool ProcessIsAlive(int pid) {
        HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
        if (!h) return false;
        DWORD code = 0;
        bool alive = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
        CloseHandle(h);
        return alive;
    }

    void MarkProcessDead(int64_t procRowId, int64_t childId, const std::string& svcName) {
        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleHeart.dbo.family_child_processes "
            "SET status = 'dead' WHERE id = ?;",
            { std::to_string(procRowId) });
        ELLE_WARN("Family: child #%lld service %s died", (long long)childId, svcName.c_str());
        json ev = { {"type","family_child_service_died"},
                    {"child_id", childId}, {"service", svcName} };
        auto msg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_FAMILY, SVC_HTTP_SERVER);
        msg.SetStringPayload(ev.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, msg);
    }

    void MarkChildLost(int64_t childId) {
        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleHeart.dbo.family_children "
            "SET status = 'lost' WHERE id = ? AND status = 'alive';",
            { std::to_string(childId) });
        ELLE_WARN("Family: child #%lld lost (process terminated)", (long long)childId);
        json ev = { {"type", "family_child_lost"}, {"child_id", childId} };
        auto msg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_FAMILY, SVC_HTTP_SERVER);
        msg.SetStringPayload(ev.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, msg);
    }

    void CreateChildDatabases(const std::vector<std::string>& dbs) {
        auto& sql = ElleSQLPool::Instance();
        for (auto& db : dbs) {
            std::string stmt =
                "IF NOT EXISTS (SELECT 1 FROM sys.databases WHERE name = N'" + db + "') "
                "CREATE DATABASE [" + db + "];";
            sql.Exec(stmt);
        }
    }

    void RunSchemaAgainstChild(const fs::path& sqlDir,
                               const std::string& coreDb,
                               const std::string& heartDb,
                               const std::string& systemDb,
                               const std::string& memoryDb,
                               const std::string& knowledgeDb) {
        if (!fs::exists(sqlDir)) return;
        std::error_code ec;

        struct Rewrite { std::string parent, child; };
        const std::vector<Rewrite> rewrites = {
            { "ElleKnowledge", knowledgeDb },
            { "ElleMemory",    memoryDb    },
            { "ElleSystem",    systemDb    },
            { "ElleHeart",     heartDb     },
            { "ElleCore",      coreDb      }
        };

        auto rewriteCatalogs = [&](std::string s) {
            for (auto& r : rewrites) {

                for (const std::string& pat : { std::string("[") + r.parent + "]",
                                                 r.parent }) {
                    size_t pos = 0;
                    while (pos < s.size()) {

                        size_t hit = std::string::npos;
                        for (size_t i = pos; i + pat.size() <= s.size(); ++i) {
                            bool m = true;
                            for (size_t k = 0; k < pat.size(); ++k) {
                                char a = (char)tolower((unsigned char)s[i+k]);
                                char b = (char)tolower((unsigned char)pat[k]);
                                if (a != b) { m = false; break; }
                            }

                            if (m && pat[0] != '[' ) {
                                auto isWord = [](char c){
                                    return (c>='A'&&c<='Z')||(c>='a'&&c<='z')
                                         ||(c>='0'&&c<='9')||c=='_';
                                };
                                if (i > 0 && isWord(s[i-1])) { m = false; }
                                if (m && i + pat.size() < s.size()
                                      && isWord(s[i + pat.size()])) { m = false; }
                            }
                            if (m) { hit = i; break; }
                        }
                        if (hit == std::string::npos) break;
                        std::string repl = (pat[0] == '[')
                                           ? std::string("[") + r.child + "]"
                                           : r.child;
                        s.replace(hit, pat.size(), repl);
                        pos = hit + repl.size();
                    }
                }
            }
            return s;
        };

        auto splitBatches = [](const std::string& script) {
            std::vector<std::string> out;
            std::string batch;
            size_t i = 0;
            while (i <= script.size()) {

                size_t eol = script.find('\n', i);
                std::string line = script.substr(i, (eol == std::string::npos ? script.size() : eol) - i);

                std::string trimmed = line;
                while (!trimmed.empty() &&
                       (trimmed.back()=='\r' || trimmed.back()==' ' || trimmed.back()=='\t'))
                    trimmed.pop_back();
                size_t ls = 0;
                while (ls < trimmed.size() && (trimmed[ls]==' '||trimmed[ls]=='\t')) ls++;
                trimmed = trimmed.substr(ls);
                bool isGo = (trimmed.size() == 2
                             && (trimmed[0]=='G'||trimmed[0]=='g')
                             && (trimmed[1]=='O'||trimmed[1]=='o'));
                if (isGo) {
                    if (!batch.empty()) { out.push_back(batch); batch.clear(); }
                } else {
                    batch += line;
                    batch += '\n';
                }
                if (eol == std::string::npos) break;
                i = eol + 1;
            }

            while (!batch.empty() &&
                   (batch.back()=='\n'||batch.back()=='\r'||batch.back()==' '||batch.back()=='\t'))
                batch.pop_back();
            if (!batch.empty()) out.push_back(batch);
            return out;
        };

        std::vector<fs::path> files;
        for (auto& entry : fs::directory_iterator(sqlDir, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sql") {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());

        for (auto& p : files) {
            std::ifstream f(p);
            std::stringstream ss; ss << f.rdbuf();
            std::string ddl = ss.str();
            if (ddl.empty()) continue;
            ddl = rewriteCatalogs(ddl);
            for (auto& batch : splitBatches(ddl)) {
                if (batch.find_first_not_of(" \t\r\n;") == std::string::npos) continue;
                ElleSQLPool::Instance().Exec(batch);
            }
        }
    }

    static void CopyFilesByExt(const fs::path& from, const fs::path& to,
                               const std::vector<std::wstring>& exts) {
        std::error_code ec;
        if (!fs::exists(from)) return;
        for (auto& entry : fs::directory_iterator(from, ec)) {
            if (!entry.is_regular_file()) continue;
            std::wstring ext = entry.path().extension().wstring();
            for (auto& e : exts) {
                if (ext == e) {
                    fs::copy_file(entry.path(), to / entry.path().filename(),
                                  fs::copy_options::overwrite_existing, ec);
                    break;
                }
            }
        }
    }

    static void CopyDirectoryRecursive(const fs::path& from, const fs::path& to,
                                       const std::vector<std::wstring>& exts) {
        std::error_code ec;
        fs::create_directories(to, ec);
        for (auto& entry : fs::recursive_directory_iterator(from, ec)) {
            auto rel = fs::relative(entry.path(), from, ec);
            auto dst = to / rel;
            if (entry.is_directory()) {
                fs::create_directories(dst, ec);
            } else if (entry.is_regular_file()) {
                bool keep = exts.empty();
                if (!keep) {
                    std::wstring ext = entry.path().extension().wstring();
                    for (auto& e : exts) if (ext == e) { keep = true; break; }
                }
                if (keep) {
                    fs::create_directories(dst.parent_path(), ec);
                    fs::copy_file(entry.path(), dst,
                                  fs::copy_options::overwrite_existing, ec);
                }
            }
        }
    }

    static int RunWaitW(const std::wstring& cmdLine) {
        STARTUPINFOW si{}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        std::vector<wchar_t> buf(cmdLine.begin(), cmdLine.end());
        buf.push_back(0);
        BOOL ok = CreateProcessW(nullptr, buf.data(), nullptr, nullptr,
                                 FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                                 &si, &pi);
        if (!ok) return -1;
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (int)code;
    }
};

ELLE_SERVICE_MAIN(ElleFamilyService)
