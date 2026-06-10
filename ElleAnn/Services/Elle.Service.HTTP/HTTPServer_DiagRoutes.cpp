#include "HTTPServer.h"

void ElleHTTPService::RegisterDiagRoutes() {
        m_router.Register("GET", "/api/diag/queues", [](const HTTPRequest&) {
            ElleDB::QueueSnapshot s;
            if (!ElleDB::GetQueueSnapshot(s)) {
                return HTTPResponse::Err(500, "queue snapshot failed");
            }
            json j = {
                {"intents", {
                    {"pending",              s.intent_pending},
                    {"processing",           s.intent_processing},
                    {"completed_last_hour",  s.intent_completed_1h},
                    {"failed_last_hour",     s.intent_failed_1h},
                    {"stale_processing",     s.intent_stale_processing}
                }},
                {"actions", {
                    {"queued",                      s.action_queued},
                    {"locked",                      s.action_locked},
                    {"executing",                   s.action_executing},
                    {"completed_success_last_hour", s.action_success_1h},
                    {"completed_failure_last_hour", s.action_failure_1h},
                    {"timed_out_last_hour",         s.action_timeout_1h},
                    {"stale_locked",                s.action_stale_locked}
                }},
                {"hardware_actions", {
                    {"pending",    s.hardware_pending},
                    {"dispatched", s.hardware_dispatched}
                }},
                {"taken_ms", (int64_t)ELLE_MS_NOW()}
            };
            return HTTPResponse::OK(j);
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/sqlqueue", [](const HTTPRequest&) {
            auto& fb = ElleSQLFallback::Instance();
            json j = {
                {"enabled",       fb.IsEnabled()},
                {"file_count",    (uint64_t)fb.FileCount()},
                {"pending_bytes", (uint64_t)fb.PendingBytes()}
            };
            return HTTPResponse::OK(j);
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/fiesta", [](const HTTPRequest&) {
            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            std::filesystem::path snap =
                std::filesystem::path(buf).parent_path() /
                "diag" / "fiesta_state.json";
            std::error_code ec;
            if (!std::filesystem::exists(snap, ec)) {
                return HTTPResponse::OK({
                    {"available", false},
                    {"reason",
                       "fiesta service has not written a snapshot yet "
                       "(file missing): " + snap.string()}
                });
            }
            std::ifstream f(snap, std::ios::binary);
            if (!f.is_open()) {
                return HTTPResponse::Err(500,
                    "cannot open " + snap.string());
            }
            std::string body((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
            try {
                json j = json::parse(body);

                uint64_t now = (uint64_t)std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                        std::chrono::system_clock::now()
                            .time_since_epoch()).count();
                uint64_t updated = j.value("updated_ms", (uint64_t)0);
                j["age_ms"]   = (updated && now >= updated) ? (now - updated) : 0;
                j["stale"]    = (updated == 0) ||
                                ((now - updated) > 30000);
                j["available"] = true;
                return HTTPResponse::OK(j);
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500,
                    std::string("snapshot parse failed: ") + e.what());
            }
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/effective-config", [](const HTTPRequest&) {
            auto raw = ElleConfig::Instance().DumpJsonRedacted();
            json j;
            try { j = json::parse(raw); }
            catch (const std::exception& e) {
                return HTTPResponse::Err(500,
                    std::string("config dump parse failed: ") + e.what());
            }
            return HTTPResponse::OK({
                {"loaded", true},
                {"config", j}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/routes", [this](const HTTPRequest&) {
            json arr = json::array();
            for (const auto& e : m_router.AllRoutes()) {
                const char* lvl = "user";
                switch (e.auth) {
                    case AUTH_PUBLIC:        lvl = "public";        break;
                    case AUTH_USER:          lvl = "user";          break;
                    case AUTH_ADMIN:         lvl = "admin";         break;
                    case AUTH_INTERNAL_ONLY: lvl = "internal_only"; break;
                }
                arr.push_back({
                    {"method", e.method},
                    {"pattern", e.pattern},
                    {"auth",   lvl}
                });
            }
            return HTTPResponse::OK({{"routes", arr}, {"count", (int)arr.size()}});
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/wires", [this](const HTTPRequest&) {
            json wires = json::array();
            const auto stamps = GetIPCHub().LastSeenPerService();
            const uint64_t now = ELLE_MS_NOW();
            const std::pair<ELLE_SERVICE_ID, const char*> services[] = {
                {SVC_HTTP_SERVER,   "HTTPServer"},
                {SVC_COGNITIVE,     "Cognitive"},
                {SVC_EMOTIONAL,     "Emotional"},
                {SVC_MEMORY,        "Memory"},
                {SVC_BONDING,       "Bonding"},
                {SVC_QUEUE_WORKER,  "QueueWorker"},
                {SVC_ACTION,        "Action"},
                {SVC_FIESTA,        "Fiesta"},
                {SVC_HEARTBEAT,     "Heartbeat"},
                {SVC_INNER_LIFE,    "InnerLife"},
                {SVC_WORLD_MODEL,   "WorldModel"},
                {SVC_SOLITUDE,      "Solitude"},
                {SVC_FAMILY,        "Family"},
                {SVC_X_CHROMOSOME,  "XChromosome"},
                {SVC_DREAM,         "Dream"},
                {SVC_IDENTITY,      "Identity"},
                {SVC_CONSENT,       "Consent"},
            };
            for (const auto& [id, name] : services) {
                auto it = stamps.find(id);
                const uint64_t lastMs = (it != stamps.end()) ? it->second : 0;
                const int64_t  quiet  = lastMs ? (int64_t)((now - lastMs) / 60000) : -1;
                const char*    state  = lastMs == 0
                                          ? "unknown"
                                          : (now - lastMs > 5 * 60 * 1000 ? "stale" : "up");
                wires.push_back({
                    {"service",       name},
                    {"service_id",    (int)id},
                    {"pipe_name",     std::string("elle_ipc_") + name},
                    {"state",         state},
                    {"last_seen_ms",  lastMs},
                    {"quiet_minutes", quiet}
                });
            }
            return HTTPResponse::OK({
                {"wires",   wires},
                {"now_ms",  now},
                {"count",  (int)wires.size()}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/heartbeats", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT ServiceId, "
                "  CONVERT(BIGINT, LastHeartbeatMs) AS hb_ms, "
                "  Healthy, "
                "  DATEDIFF(SECOND, "
                "           DATEADD(SECOND, LastHeartbeatMs/1000, '1970-01-01'), "
                "           GETUTCDATE()) AS quiet_sec "
                "  FROM ElleSystem.dbo.Workers "
                "  ORDER BY ServiceId;");
            if (!rs.success)
                return HTTPResponse::Err(500, "SQL query failed");
            json arr = json::array();
            for (auto& row : rs.rows) {
                int64_t svcId = 0, hbMs = 0, healthy = 0, quietSec = 0;
                row.TryGetInt(0, svcId);
                row.TryGetInt(1, hbMs);
                row.TryGetInt(2, healthy);
                row.TryGetInt(3, quietSec);

                const char* state = (healthy == 0) ? "down"
                                  : (quietSec > 300) ? "stale" : "up";
                arr.push_back({
                    {"service_id",   svcId},
                    {"last_hb_ms",   hbMs},
                    {"quiet_sec",    quietSec},
                    {"healthy",      healthy != 0},
                    {"state",        state}
                });
            }
            return HTTPResponse::OK({{"heartbeats", arr}, {"count", (int)arr.size()}});
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/diag/health", [this](const HTTPRequest&) {
            json issues = json::array();
            const uint64_t now = ELLE_MS_NOW();

            auto& llm    = ElleConfig::Instance().GetLLM();
            const bool llmHealthy = ElleComposer::Client::Instance().IsBound();
            std::string activeProvider = "composer";
            std::string activeModel    = "deterministic";
            (void)llm;
            if (!llmHealthy) issues.push_back("composer: unbound");

            const auto stamps = GetIPCHub().LastSeenPerService();
            int wiresUp = 0;
            for (auto& kv : stamps) {
                if (now - kv.second <= 5 * 60 * 1000) wiresUp++;
            }
            const int wiresTotal = (int)stamps.size();

            int hbUp = 0, hbTotal = 0;
            {
                auto rs = ElleSQLPool::Instance().Query(
                    "SELECT Healthy, "
                    "  DATEDIFF(SECOND, "
                    "           DATEADD(SECOND, LastHeartbeatMs/1000, '1970-01-01'), "
                    "           GETUTCDATE()) AS quiet_sec "
                    "  FROM ElleSystem.dbo.Workers;");
                if (rs.success) {
                    hbTotal = (int)rs.rows.size();
                    for (auto& row : rs.rows) {
                        int64_t healthy = 0, quiet = 0;
                        row.TryGetInt(0, healthy);
                        row.TryGetInt(1, quiet);
                        if (healthy && quiet <= 300) hbUp++;
                    }
                    if (hbTotal > 0 && hbUp < hbTotal) {
                        issues.push_back("heartbeats: " +
                            std::to_string(hbTotal - hbUp) + " stale of " +
                            std::to_string(hbTotal));
                    }
                }
            }

            int64_t intentPending = 0, actionPending = 0, memoryCount = 0;
            {
                auto rs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.IntentQueue WHERE Status = 0;");
                if (rs.success && !rs.rows.empty()) rs.rows[0].TryGetInt(0, intentPending);
            }
            {
                auto rs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.action_queue WHERE status = 0;");
                if (rs.success && !rs.rows.empty()) rs.rows[0].TryGetInt(0, actionPending);
            }
            {
                auto rs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.memory;");
                if (rs.success && !rs.rows.empty()) rs.rows[0].TryGetInt(0, memoryCount);
            }

            if (intentPending > 1000)
                issues.push_back("intent_queue: " + std::to_string(intentPending) + " pending");
            if (actionPending > 1000)
                issues.push_back("action_queue: " + std::to_string(actionPending) + " pending");

            return HTTPResponse::OK({
                {"ts_ms",            (int64_t)now},
                {"llm", {
                    {"provider",  activeProvider},
                    {"model",     activeModel},
                    {"healthy",   llmHealthy}
                }},
                {"wires_up_count",   wiresUp},
                {"wires_total",      wiresTotal},
                {"heartbeats_up",    hbUp},
                {"heartbeats_total", hbTotal},
                {"intent_pending",   intentPending},
                {"action_pending",   actionPending},
                {"memory_count",     memoryCount},
                {"issues",           issues}
            });
        }, AUTH_ADMIN);

    }
