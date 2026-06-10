#include "HTTPServer.h"

void ElleHTTPService::RegisterModelsRoutes() {
        m_router.Register("GET", "/api/models/slots", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT slot_number, name, endpoint, model, enabled, ISNULL(last_ping_ms, 0) "
                "FROM ElleCore.dbo.model_slots ORDER BY slot_number;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"slot_number", r.GetIntOr(0, 0)},
                    {"name",     r.values.size() > 1 ? r.values[1] : ""},
                    {"endpoint", r.values.size() > 2 ? r.values[2] : ""},
                    {"model",    r.values.size() > 3 ? r.values[3] : ""},
                    {"enabled",  r.GetIntOr(4, 0) != 0},
                    {"last_ping_ms", r.GetIntOr(5, 0)}
                });
            }
            return HTTPResponse::OK({{"slots", arr}});
        });
        m_router.Register("POST", "/api/models/slots", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            int slot = body.value("slot_number", -1);
            std::string name = body.value("name", "");
            std::string endpoint = body.value("endpoint", "");
            std::string model = body.value("model", "");
            if (slot < 0 || name.empty() || endpoint.empty()) {
                return HTTPResponse::Err(400, "slot_number, name, endpoint required");
            }
            auto rs = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleCore.dbo.model_slots AS t "
                "USING (SELECT ? AS slot_number) AS s ON t.slot_number = s.slot_number "
                "WHEN MATCHED THEN UPDATE SET name = ?, endpoint = ?, model = ?, updated_at = GETUTCDATE() "
                "WHEN NOT MATCHED THEN INSERT (slot_number, name, endpoint, model) VALUES (?, ?, ?, ?);",
                { std::to_string(slot), name, endpoint, model,
                  std::to_string(slot), name, endpoint, model });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"slot_number", slot}, {"stored", true}});
        });
        m_router.Register("DELETE", "/api/models/slots/{slot_number}", [](const HTTPRequest& req) {
            int slot = req.PathInt("slot_number");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.model_slots WHERE slot_number = ?;",
                { std::to_string(slot) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"slot_number", slot}, {"removed", true}});
        });
        m_router.Register("POST", "/api/models/slots/{slot_number}/ping", [](const HTTPRequest& req) {
            int slot = req.PathInt("slot_number");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT endpoint FROM ElleCore.dbo.model_slots WHERE slot_number = ?;",
                { std::to_string(slot) });
            if (!rs.success || rs.rows.empty()) return HTTPResponse::Err(404, "slot not found");
            std::string endpoint = rs.rows[0].values.empty() ? "" : rs.rows[0].values[0];
            if (endpoint.empty())
                return HTTPResponse::OK({{"slot_number", slot}, {"alive", false},
                                         {"endpoint", ""}, {"error", "no endpoint configured"}});

            auto parseHostPort = [](const std::string& url,
                                    std::string& host, uint16_t& port) -> bool {
                std::string s = url;
                uint16_t defaultPort = 80;
                if (s.rfind("https://", 0) == 0) { s = s.substr(8); defaultPort = 443; }
                else if (s.rfind("http://", 0) == 0) { s = s.substr(7); defaultPort = 80; }

                auto slash = s.find('/');
                if (slash != std::string::npos) s = s.substr(0, slash);

                auto colon = s.rfind(':');
                if (colon != std::string::npos) {
                    host = s.substr(0, colon);
                    long long p = 0;
                    if (HTTPRequest::StrictParseLL(s.substr(colon + 1), p) &&
                        p > 0 && p <= 65535) {
                        port = (uint16_t)p;
                    } else {
                        return false;
                    }
                } else {
                    host = s;
                    port = defaultPort;
                }
                return !host.empty();
            };

            std::string host;
            uint16_t port = 0;
            if (!parseHostPort(endpoint, host, port)) {
                return HTTPResponse::OK({{"slot_number", slot}, {"alive", false},
                                         {"endpoint", endpoint},
                                         {"error", "unparseable endpoint"}});
            }

            uint64_t t0 = ELLE_MS_NOW();
            bool alive = false;
            std::string probeErr;

            addrinfo hints{}; hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
            addrinfo* res = nullptr;
            std::string portStr = std::to_string((int)port);
            if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) == 0 && res) {
                SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (s != INVALID_SOCKET) {

                    u_long nb = 1; ioctlsocket(s, FIONBIO, &nb);
                    int rc = connect(s, res->ai_addr, (int)res->ai_addrlen);
                    if (rc == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
                        fd_set wfds; FD_ZERO(&wfds); FD_SET(s, &wfds);
                        timeval tv{}; tv.tv_sec = 2; tv.tv_usec = 0;
                        int sel = select(0, nullptr, &wfds, nullptr, &tv);
                        if (sel > 0) {
                            int soErr = 0; int soLen = (int)sizeof(soErr);
                            if (getsockopt(s, SOL_SOCKET, SO_ERROR,
                                           (char*)&soErr, &soLen) == 0 && soErr == 0) {
                                alive = true;
                            } else {
                                probeErr = "connect error " + std::to_string(soErr);
                            }
                        } else {
                            probeErr = "connect timeout";
                        }
                    } else if (rc == 0) {
                        alive = true;
                    } else {
                        probeErr = "connect failed " + std::to_string(WSAGetLastError());
                    }
                    closesocket(s);
                } else {
                    probeErr = "socket() failed";
                }
                freeaddrinfo(res);
            } else {
                probeErr = "DNS resolve failed for " + host;
            }
            uint64_t t1 = ELLE_MS_NOW();

            if (alive) {
                ElleSQLPool::Instance().QueryParams(
                    "UPDATE ElleCore.dbo.model_slots SET last_ping_ms = ? WHERE slot_number = ?;",
                    { std::to_string(t1), std::to_string(slot) });
            }
            json resp = {
                {"slot_number", slot},
                {"alive", alive},
                {"endpoint", endpoint},
                {"host", host},
                {"port", (int)port},
                {"latency_ms", (int64_t)(t1 - t0)}
            };
            if (!alive) resp["error"] = probeErr;
            return HTTPResponse::OK(resp);
        });
        m_router.Register("GET", "/api/models/workers", [](const HTTPRequest&) {
            std::vector<ELLE_SERVICE_STATUS> statuses;
            ElleDB::GetWorkerStatuses(statuses);
            json arr = json::array();
            uint64_t now = ELLE_MS_NOW();
            for (auto& s : statuses) {
                arr.push_back({
                    {"worker_id", std::string(s.name)},
                    {"service_id", (int)s.service_id},
                    {"running", s.running != 0},
                    {"healthy", s.healthy != 0},
                    {"last_heartbeat_ms", s.last_heartbeat_ms},
                    {"ms_ago", s.last_heartbeat_ms > 0 ? (now - s.last_heartbeat_ms) : 0}
                });
            }
            return HTTPResponse::OK({{"workers", arr}});
        });
        m_router.Register("POST", "/api/models/workers", [](const HTTPRequest& req) {

            json body = req.BodyJSON();
            std::string id = "worker-" + std::to_string(ELLE_MS_NOW());
            std::string val = body.dump();
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.system_settings (setting_key, setting_value, updated_at) "
                "VALUES (?, ?, GETUTCDATE());",
                { id, val });
            return HTTPResponse::Created({{"worker_id", id}});
        });
        m_router.Register("PUT", "/api/models/workers/{worker_id}/decommission", [](const HTTPRequest& req) {
            std::string wid = req.headers.at("x-path-worker_id");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.system_settings WHERE setting_key = ?;", { wid });
            return HTTPResponse::OK({
                {"worker_id", wid}, {"decommissioned", rs.success}
            });
        });
        m_router.Register("GET", "/api/models/personality", [](const HTTPRequest&) {

            auto rs = ElleSQLPool::Instance().Query(
                "SELECT trait_id, ISNULL(trait_name,''), ISNULL(current_value, 0) "
                "FROM ElleCore.dbo.PersonalityTraits ORDER BY trait_id;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json traits = json::object();
            for (auto& r : rs.rows) {
                std::string name = r.values.size() > 1 ? r.values[1] : "";
                if (!name.empty()) traits[name] = r.GetFloatOr(2, 0.0);
            }
            if (traits.empty()) {
                traits["warmth"] = 0.8; traits["curiosity"] = 0.9; traits["empathy"] = 0.85;
            }
            return HTTPResponse::OK({{"name", "Elle-Ann"}, {"traits", traits}});
        });
        m_router.Register("GET", "/api/models/token-cache/stats", [](const HTTPRequest&) {

            auto rs = ElleSQLPool::Instance().Query(
                "SELECT ISNULL(setting_key,''), ISNULL(setting_value,'0') "
                "FROM ElleCore.dbo.system_settings "
                "WHERE setting_key LIKE 'llm_%';");
            uint64_t hits = 0, misses = 0, total = 0;
            if (rs.success) {
                for (auto& r : rs.rows) {
                    std::string k = r.values.size() > 0 ? r.values[0] : "";

                    long long parsed = 0;
                    uint64_t v = 0;
                    if (r.values.size() > 1 &&
                        HTTPRequest::StrictParseLL(r.values[1], parsed) &&
                        parsed >= 0) {
                        v = (uint64_t)parsed;
                    }
                    if (k == "llm_cache_hits")   hits = v;
                    if (k == "llm_cache_misses") misses = v;
                    if (k == "llm_total_requests") total = v;
                }
            }
            return HTTPResponse::OK({
                {"cache_hits", hits}, {"cache_misses", misses}, {"total_requests", total}
            });
        });

    }
