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

        m_router.Register("GET", "/api/admin/sqlfallback/status",
            [](const HTTPRequest&) {
                auto& fb = ElleSQLFallback::Instance();
                auto s = fb.GetPoisonLoadStatus();
                return HTTPResponse::OK({
                    {"enabled",                fb.IsEnabled()},
                    {"pending_files",          fb.FileCount()},
                    {"pending_bytes",          fb.PendingBytes()},
                    {"poison_files",           fb.PoisonFileCount()},
                    {"poison_bytes",           fb.PoisonBytes()},
                    {"max_retries",            fb.MaxRetries()},
                    {"poison_load_interval_ms",fb.PoisonLoadIntervalMs()},
                    {"last_attempt_ms",        s.last_attempt_ms},
                    {"last_success_ms",        s.last_success_ms},
                    {"last_inserted",          s.last_inserted},
                    {"total_attempts",         s.total_attempts},
                    {"total_successes",        s.total_successes},
                    {"total_inserted",         s.total_inserted},
                    {"last_error",             s.last_error}
                });
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/sqlfallback/console",
            [](const HTTPRequest&) {
                static const char kHtml[] =
"<!doctype html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"<meta charset=\"utf-8\">\n"
"<title>Elle SQL Fallback Console</title>\n"
"<script src=\"https://unpkg.com/htmx.org@1.9.12\" integrity=\"sha384-ujb1lZYygJmzgSwoxRggbCHcjc0rB2XoQrxeTUQyRjrOnlCoYta87iKBWq3EsdM2\" crossorigin=\"anonymous\"></script>\n"
"<style>\n"
"  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; max-width: 1100px; margin: 2rem auto; padding: 0 1rem; background: #0f1419; color: #d8dee9; }\n"
"  h1 { color: #88c0d0; letter-spacing: -0.02em; }\n"
"  h2 { color: #81a1c1; border-bottom: 1px solid #3b4252; padding-bottom: 0.25rem; margin-top: 2rem; }\n"
"  .panel { background: #1e242c; border: 1px solid #3b4252; border-radius: 6px; padding: 1rem; margin: 1rem 0; }\n"
"  .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 0.75rem; }\n"
"  .metric { background: #252b36; border-radius: 4px; padding: 0.6rem 0.8rem; }\n"
"  .metric .k { font-size: 0.75rem; color: #6d7894; text-transform: uppercase; letter-spacing: 0.05em; }\n"
"  .metric .v { font-size: 1.4rem; color: #eceff4; font-family: 'SF Mono', Consolas, monospace; margin-top: 0.2rem; }\n"
"  .metric.warn .v { color: #ebcb8b; }\n"
"  .metric.err  .v { color: #bf616a; }\n"
"  .metric.ok   .v { color: #a3be8c; }\n"
"  button { background: #5e81ac; color: #eceff4; border: 0; border-radius: 4px; padding: 0.55rem 1rem; font-size: 0.95rem; cursor: pointer; margin-right: 0.5rem; }\n"
"  button:hover { background: #81a1c1; }\n"
"  button.danger { background: #bf616a; }\n"
"  button.danger:hover { background: #d08770; }\n"
"  table { width: 100%; border-collapse: collapse; font-size: 0.85rem; font-family: 'SF Mono', Consolas, monospace; }\n"
"  th { text-align: left; color: #88c0d0; padding: 0.4rem 0.5rem; border-bottom: 1px solid #3b4252; font-weight: 600; }\n"
"  td { padding: 0.35rem 0.5rem; border-bottom: 1px solid #2a3038; vertical-align: top; }\n"
"  tr:hover td { background: #252b36; }\n"
"  .muted { color: #6d7894; }\n"
"  .pill { display: inline-block; padding: 0.1rem 0.5rem; border-radius: 10px; font-size: 0.7rem; background: #3b4252; color: #d8dee9; }\n"
"  .pill.exec { background: #5e81ac; }\n"
"  .pill.proc { background: #b48ead; }\n"
"  .pill.qp   { background: #8fbcbb; color: #2e3440; }\n"
"  .result { margin-top: 0.5rem; padding: 0.5rem 0.75rem; border-radius: 4px; background: #252b36; font-family: 'SF Mono', Consolas, monospace; font-size: 0.85rem; }\n"
"  .result.ok  { border-left: 3px solid #a3be8c; }\n"
"  .result.err { border-left: 3px solid #bf616a; }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h1>Elle SQL Fallback Console</h1>\n"
"<p class=\"muted\">Operational view of the SQL fallback queue and poison ledger. All endpoints below require <code>AUTH_ADMIN</code>; pass your admin token via <code>HX-Headers</code> or the proxy you front this with.</p>\n"
"\n"
"<h2>Status</h2>\n"
"<div class=\"panel\">\n"
"  <button hx-get=\"/api/admin/sqlfallback/status\" hx-target=\"#status-panel\" hx-trigger=\"load, every 5s, click\" hx-swap=\"innerHTML\">Refresh status</button>\n"
"  <div id=\"status-panel\" class=\"grid\" style=\"margin-top: 1rem;\"></div>\n"
"</div>\n"
"\n"
"<h2>Poison ledger</h2>\n"
"<div class=\"panel\">\n"
"  <button hx-get=\"/api/admin/sqlfallback/poison?limit=100\" hx-target=\"#poison-panel\" hx-swap=\"innerHTML\">Show last 100 poison lines</button>\n"
"  <button class=\"danger\" hx-post=\"/api/admin/sqlfallback/poison/load?limit=500\" hx-target=\"#load-result\" hx-swap=\"innerHTML\" hx-confirm=\"Replay up to 500 poisoned lines back into SQL. Continue?\">Bulk replay (limit 500)</button>\n"
"  <div id=\"load-result\"></div>\n"
"  <div id=\"poison-panel\" style=\"margin-top: 1rem;\"></div>\n"
"</div>\n"
"\n"
"<script>\n"
"  function fmtMs(ms) { if (!ms) return '<span class=\"muted\">never</span>'; const d = new Date(Number(ms)); return d.toISOString().replace('T',' ').replace('Z',' UTC'); }\n"
"  function fmtBytes(n) { if (!n) return '0 B'; const u = ['B','KB','MB','GB']; let i=0; let v=Number(n); while (v>=1024 && i<u.length-1) { v/=1024; i++; } return v.toFixed(i?2:0)+' '+u[i]; }\n"
"  document.body.addEventListener('htmx:afterRequest', (e) => {\n"
"    const target = e.detail.target;\n"
"    if (!target) return;\n"
"    if (target.id === 'status-panel') {\n"
"      try {\n"
"        const j = JSON.parse(e.detail.xhr.responseText);\n"
"        const cls = (k,v) => k==='last_error' && v ? 'err' : k==='poison_files' && Number(v)>0 ? 'warn' : k==='enabled' && v===false ? 'err' : 'ok';\n"
"        const rows = [\n"
"          ['enabled', String(j.enabled)],\n"
"          ['pending_files', j.pending_files],\n"
"          ['pending_bytes', fmtBytes(j.pending_bytes)],\n"
"          ['poison_files', j.poison_files],\n"
"          ['poison_bytes', fmtBytes(j.poison_bytes)],\n"
"          ['max_retries', j.max_retries],\n"
"          ['poison_load_interval_ms', j.poison_load_interval_ms],\n"
"          ['last_attempt', fmtMs(j.last_attempt_ms)],\n"
"          ['last_success', fmtMs(j.last_success_ms)],\n"
"          ['last_inserted', j.last_inserted],\n"
"          ['total_attempts', j.total_attempts],\n"
"          ['total_successes', j.total_successes],\n"
"          ['total_inserted', j.total_inserted],\n"
"          ['last_error', j.last_error || '\u2014']\n"
"        ];\n"
"        target.innerHTML = rows.map(([k,v]) => `<div class=\"metric ${cls(k,v)}\"><div class=\"k\">${k}</div><div class=\"v\">${v}</div></div>`).join('');\n"
"      } catch (err) { target.innerHTML = '<div class=\"result err\">Failed to parse status: '+err.message+'</div>'; }\n"
"    }\n"
"    if (target.id === 'poison-panel') {\n"
"      try {\n"
"        const j = JSON.parse(e.detail.xhr.responseText);\n"
"        if (!j.lines || j.lines.length === 0) { target.innerHTML = '<p class=\"muted\">No poison lines.</p>'; return; }\n"
"        const head = '<table><thead><tr><th>ts</th><th>kind</th><th>idem</th><th>retries</th><th>sql_or_proc</th><th>params</th></tr></thead><tbody>';\n"
"        const body = j.lines.map(r => `<tr><td>${fmtMs(r.ts_ms)}</td><td><span class=\"pill ${r.kind==='Exec'?'exec':r.kind==='CallProc'?'proc':'qp'}\">${r.kind}</span></td><td>${r.idem||''}</td><td>${r.retry_count}</td><td>${(r.sql_or_proc||'').slice(0,120)}</td><td class=\"muted\">${JSON.stringify(r.params).slice(0,80)}</td></tr>`).join('');\n"
"        target.innerHTML = head + body + '</tbody></table><p class=\"muted\" style=\"margin-top: 0.5rem;\">Showing '+j.returned_lines+' of '+j.total_files+' poison file(s) ('+fmtBytes(j.total_bytes)+').</p>';\n"
"      } catch (err) { target.innerHTML = '<div class=\"result err\">Failed to parse poison list: '+err.message+'</div>'; }\n"
"    }\n"
"    if (target.id === 'load-result') {\n"
"      try {\n"
"        const j = JSON.parse(e.detail.xhr.responseText);\n"
"        target.innerHTML = `<div class=\"result ok\">Inserted ${j.inserted} line(s) from poison ledger back into SQL. ${j.total_files} file(s) remaining (${fmtBytes(j.total_bytes)}).</div>`;\n"
"      } catch (err) { target.innerHTML = '<div class=\"result err\">'+err.message+'</div>'; }\n"
"    }\n"
"  });\n"
"</script>\n"
"</body>\n"
"</html>\n";
                return HTTPResponse::Binary("text/html; charset=utf-8", std::string(kHtml));
            }, AUTH_ADMIN);

    }
