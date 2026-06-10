#include "HTTPServer.h"

void ElleHTTPService::RegisterAdminRoutes() {
        m_router.Register("POST", "/api/admin/config/reload", [this](const HTTPRequest&) {
            const bool localOk = ElleConfig::Instance().Reload();
            auto msg = ElleIPCMessage::Create(IPC_CONFIG_RELOAD,
                                              SVC_HTTP_SERVER, SVC_HTTP_SERVER);
            GetIPCHub().Broadcast(msg);
            ELLE_INFO("Config reload requested (local=%d) and broadcast",
                      localOk ? 1 : 0);
            return HTTPResponse::OK({
                {"applied_locally", localOk},
                {"broadcast",       true}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/lexical/incomplete",
            [](const HTTPRequest& req) {
                int    limit    = req.QueryInt("limit", 50);
                double minScore = req.QueryFloat("min_score", 0.0);
                if (limit <= 0)   limit = 50;
                if (limit > 1000) limit = 1000;
                if (minScore < 0.0) minScore = 0.0;
                if (minScore > 1.0) minScore = 1.0;

                ElleLanguage::LexicalAuditReport report;
                if (!ElleLanguage::FetchLexicalAuditReport(report, minScore, limit)) {
                    return HTTPResponse::Err(500, "lexical_audit_query_failed");
                }
                auto body = ElleLanguage::LexicalAuditReportToJson(report);
                return HTTPResponse::OK(body);
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/belief/audit",
            [](const HTTPRequest& req) {
                std::string domain = req.QueryString("domain", "");
                long long sinceMs  = req.QueryLL("since_ms", 0);
                int       limit    = req.QueryInt("limit", 200);
                if (limit <= 0)   limit = 200;
                if (limit > 5000) limit = 5000;
                if (sinceMs < 0)  sinceMs = 0;

                std::vector<ElleBeliefAdmin::BeliefAuditRow> rows;
                if (!ElleBeliefAdmin::FetchBeliefAudit(rows, domain, sinceMs, limit)) {
                    return HTTPResponse::Err(500, "belief_audit_query_failed");
                }
                return HTTPResponse::OK(ElleBeliefAdmin::BeliefAuditToJson(rows));
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/belief/snapshot",
            [](const HTTPRequest& req) {
                std::string domain = req.QueryString("domain", "");
                std::vector<ElleBeliefAdmin::BeliefSnapshotRow> rows;
                if (!ElleBeliefAdmin::FetchBeliefSnapshot(rows, domain)) {
                    return HTTPResponse::Err(500, "belief_snapshot_query_failed");
                }
                return HTTPResponse::OK(ElleBeliefAdmin::BeliefSnapshotToJson(rows));
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/bonding/dashboard",
            [](const HTTPRequest&) {
                ElleBeliefAdmin::BondingDashboardRow row;
                if (!ElleBeliefAdmin::FetchBondingDashboard(row)) {
                    return HTTPResponse::Err(500, "bonding_dashboard_query_failed");
                }
                return HTTPResponse::OK(ElleBeliefAdmin::BondingDashboardToJson(row));
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/bonding/trajectory",
            [](const HTTPRequest& req) {
                int limit = req.QueryInt("limit", 100);
                if (limit <= 0)    limit = 100;
                if (limit > 1000)  limit = 1000;
                std::vector<ElleBeliefAdmin::BondingTrajectoryPoint> pts;
                if (!ElleBeliefAdmin::FetchBondingTrajectory(pts, limit)) {
                    return HTTPResponse::Err(500, "bonding_trajectory_query_failed");
                }
                return HTTPResponse::OK(ElleBeliefAdmin::BondingTrajectoryToJson(pts));
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/sqlfallback/poison",
            [](const HTTPRequest& req) {
                int limit = req.QueryInt("limit", 200);
                if (limit <= 0)    limit = 200;
                if (limit > 5000)  limit = 5000;

                auto& fb = ElleSQLFallback::Instance();
                auto rows = fb.ListPoison((uint32_t)limit);

                json arr = json::array();
                for (const auto& r : rows) {
                    json params = json::array();
                    try {
                        params = json::parse(r.params_json);
                    } catch (...) {
                        params = json::array();
                    }
                    arr.push_back({
                        {"source_file", r.source_file},
                        {"ts_ms",       r.ts_ms},
                        {"kind",        r.kind},
                        {"idem",        r.idem},
                        {"retry_count", r.retry_count},
                        {"sql_or_proc", r.sql_or_proc},
                        {"params",      params},
                        {"raw_line",    r.raw_line}
                    });
                }

                return HTTPResponse::OK({
                    {"total_files",     fb.PoisonFileCount()},
                    {"total_bytes",     fb.PoisonBytes()},
                    {"returned_lines",  arr.size()},
                    {"limit",           limit},
                    {"lines",           arr}
                });
            }, AUTH_ADMIN);

        m_router.Register("POST", "/api/admin/sqlfallback/poison/load",
            [](const HTTPRequest& req) {
                int limit = req.QueryInt("limit", 500);
                if (limit <= 0)    limit = 500;
                if (limit > 5000)  limit = 5000;

                auto& fb = ElleSQLFallback::Instance();
                uint32_t inserted = fb.LoadPoisonIntoSql((uint32_t)limit);

                return HTTPResponse::OK({
                    {"inserted",        inserted},
                    {"total_files",     fb.PoisonFileCount()},
                    {"total_bytes",     fb.PoisonBytes()}
                });
            }, AUTH_ADMIN);

    }
