#include "HTTPServer.h"

void ElleHTTPService::RegisterServerRoutes() {
        m_router.Register("GET", "/api/server/status", [](const HTTPRequest&) {
            std::vector<ELLE_SERVICE_STATUS> statuses;
            try { ElleDB::GetWorkerStatuses(statuses); }
            catch (const std::exception& e) {
                ELLE_WARN("GetWorkerStatuses failed: %s", e.what());
            }
            json svcArr = json::array();
            uint64_t now = ELLE_MS_NOW();
            for (auto& s : statuses) {
                uint64_t since = s.last_heartbeat_ms > 0 ? (now - s.last_heartbeat_ms) : 0;
                svcArr.push_back({
                    {"id", (int)s.service_id},
                    {"name", std::string(s.name)},
                    {"running", s.running != 0},
                    {"healthy", s.healthy != 0},
                    {"uptime_ms", s.uptime_ms},
                    {"last_heartbeat_ms", s.last_heartbeat_ms},
                    {"ms_since_heartbeat", since},
                    {"messages_processed", s.messages_processed},
                    {"errors", s.errors}
                });
            }

            auto& fb = ElleSQLFallback::Instance();
            auto poison = fb.GetPoisonLoadStatus();
            json sqlFallback = {
                {"enabled",                fb.IsEnabled()},
                {"max_retries",            fb.MaxRetries()},
                {"poison_load_interval_ms",fb.PoisonLoadIntervalMs()},
                {"pending_bytes",          fb.PendingBytes()},
                {"pending_files",          fb.FileCount()},
                {"poison_bytes",           fb.PoisonBytes()},
                {"poison_files",           fb.PoisonFileCount()},
                {"reaper", {
                    {"last_attempt_ms",  poison.last_attempt_ms},
                    {"last_success_ms",  poison.last_success_ms},
                    {"last_inserted",    poison.last_inserted},
                    {"total_attempts",   poison.total_attempts},
                    {"total_successes",  poison.total_successes},
                    {"total_inserted",   poison.total_inserted},
                    {"last_error",       poison.last_error}
                }}
            };

            return HTTPResponse::OK({
                {"services", svcArr},
                {"count", (int)statuses.size()},
                {"version", ELLE_VERSION_STRING},
                {"uptime_ms", (uint64_t)ELLE_MS_NOW()},
                {"sql_fallback", sqlFallback}
            });
        });
        m_router.Register("GET", "/api/server/console", [](const HTTPRequest& req) {
            int limit = std::max(1, req.QueryInt("limit", 100));
            std::string levelParam = req.QueryParam("level");

            int levelInt = -1;
            if      (levelParam == "TRACE") levelInt = 0;
            else if (levelParam == "DEBUG") levelInt = 1;
            else if (levelParam == "INFO")  levelInt = 2;
            else if (levelParam == "WARN")  levelInt = 3;
            else if (levelParam == "ERROR") levelInt = 4;
            else if (levelParam == "FATAL") levelInt = 5;

            std::string sql =
                "SELECT TOP (" + std::to_string(limit) + ") "
                "id, level, service, message, created_ms "
                "FROM ElleCore.dbo.log_entries ";
            std::vector<std::string> params;
            if (levelInt >= 0) {
                sql += "WHERE level = ? ";
                params.push_back(std::to_string(levelInt));
            }
            sql += "ORDER BY id DESC;";
            auto rs = ElleSQLPool::Instance().QueryParams(sql, params);
            json logs = json::array();
            if (rs.success) {
                for (auto& r : rs.rows) {
                    logs.push_back({
                        {"id", r.GetIntOr(0, 0)},
                        {"level", r.GetIntOr(1, 0)},
                        {"service", r.GetIntOr(2, 0)},
                        {"message", r.values.size() > 3 ? r.values[3] : ""},
                        {"created_ms", r.GetIntOr(4, 0)}
                    });
                }
            }
            return HTTPResponse::OK({{"logs", logs}});
        });
        m_router.Register("DELETE", "/api/server/console", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Exec("DELETE FROM ElleCore.dbo.log_entries;");
            return HTTPResponse::OK({{"cleared", rs}});
        });
        m_router.Register("GET", "/api/server/settings", [](const HTTPRequest&) {
            auto& http = ElleConfig::Instance().GetHTTP();
            auto& llm  = ElleConfig::Instance().GetLLM();
            std::string model = "unknown";
            auto it = llm.providers.find("groq");
            if (it != llm.providers.end()) model = it->second.model;
            return HTTPResponse::OK({
                {"bindAddress", http.bind_address},
                {"port", http.port},
                {"model", model},
                {"version", ELLE_VERSION_STRING}
            });
        });
        m_router.Register("PUT", "/api/server/settings", [](const HTTPRequest& req) {
            json body = req.BodyJSON();

            int n = 0;
            for (auto it = body.begin(); it != body.end(); ++it) {
                std::string key = it.key();
                std::string val = it.value().is_string() ? it.value().get<std::string>() : it.value().dump();
                auto rs = ElleSQLPool::Instance().QueryParams(
                    "IF EXISTS (SELECT 1 FROM ElleCore.dbo.system_settings WHERE setting_key = ?) "
                    "  UPDATE ElleCore.dbo.system_settings SET setting_value = ?, updated_at = GETUTCDATE() WHERE setting_key = ?; "
                    "ELSE "
                    "  INSERT INTO ElleCore.dbo.system_settings (setting_key, setting_value, updated_at) VALUES (?, ?, GETUTCDATE());",
                    { key, val, key, key, val });
                if (rs.success) n++;
            }
            return HTTPResponse::OK({{"updated", n}, {"note", "restart required for bind/port changes"}});
        });
        m_router.Register("GET", "/api/server/analytics", [](const HTTPRequest&) {
            MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem);
            GlobalMemoryStatusEx(&mem);
            PROCESS_MEMORY_COUNTERS pmc = {};
            GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
            int64_t memoryCount = ElleDB::CountTable("memory");
            int64_t msgCount    = ElleDB::CountTable("messages");
            return HTTPResponse::OK({
                {"ram_percent", (float)mem.dwMemoryLoad},
                {"process_ram_mb", (uint64_t)(pmc.WorkingSetSize / (1024ULL*1024ULL))},
                {"memory_count", std::max<int64_t>(memoryCount, 0)},
                {"message_count", std::max<int64_t>(msgCount, 0)}
            });
        });
        m_router.Register("POST", "/api/server/backup", [](const HTTPRequest&) {

            std::string dir = "data\\backups";
            CreateDirectoryA("data", nullptr);
            CreateDirectoryA(dir.c_str(), nullptr);
            uint64_t ts = ELLE_MS_NOW();
            std::string path = dir + "\\ElleCore-" + std::to_string(ts) + ".bak";
            auto rs = ElleSQLPool::Instance().QueryParams(
                "BACKUP DATABASE ElleCore TO DISK = ? WITH INIT, FORMAT;",
                { path });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({
                {"backup_id", std::to_string(ts)},
                {"path", path}, {"status", "complete"}
            });
        });
        m_router.Register("GET", "/api/server/backups", [](const HTTPRequest&) {
            json arr = json::array();
            WIN32_FIND_DATAA fd; HANDLE h = FindFirstFileA("data\\backups\\*.bak", &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    arr.push_back({
                        {"name", std::string(fd.cFileName)},
                        {"size", (uint64_t)(((uint64_t)fd.nFileSizeHigh << 32) | fd.nFileSizeLow)}
                    });
                } while (FindNextFileA(h, &fd));
                FindClose(h);
            }
            return HTTPResponse::OK({{"backups", arr}});
        });
        m_router.Register("POST", "/api/server/commit-memory", [this](const HTTPRequest&) {

            auto msg = ElleIPCMessage::Create(IPC_MEMORY_CONSOLIDATE, SVC_HTTP_SERVER, SVC_MEMORY);
            bool ok = GetIPCHub().Send(SVC_MEMORY, msg);
            return HTTPResponse::OK({{"triggered", ok}});
        });
        m_router.Register("POST", "/api/server/commit-emotional-memory", [this](const HTTPRequest&) {
            auto msg = ElleIPCMessage::Create(IPC_EMOTION_CONSOLIDATE, SVC_HTTP_SERVER, SVC_EMOTIONAL);
            bool ok = GetIPCHub().Send(SVC_EMOTIONAL, msg);
            return HTTPResponse::OK({{"triggered", ok}});
        });

    }
