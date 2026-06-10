#include "HTTPServer.h"

void ElleHTTPService::RegisterAuthRoutes() {
        m_router.Register("POST", "/api/auth/pair-code",
            [](const HTTPRequest&) -> HTTPResponse {
                return HTTPResponse::Err(410,
                    "pair-code flow removed; use POST /api/auth/login with username+password");
            }, AUTH_PUBLIC);

        m_router.Register("POST", "/api/auth/pair",
            [](const HTTPRequest&) -> HTTPResponse {
                return HTTPResponse::Err(410,
                    "pair flow removed; use POST /api/auth/login with username+password");
            }, AUTH_PUBLIC);

        m_router.Register("POST", "/api/auth/login",
            [this](const HTTPRequest& req) -> HTTPResponse {
                if (ElleConfig::Instance().GetInt("http_server.no_auth", 0) != 0) {
                    return HTTPResponse::OK({
                        {"token", "single"},
                        {"nUserNo", 1},
                        {"sUserID", "single"},
                        {"sUserName", "Single"},
                        {"nAuthID", 9},
                        {"created_ms", (int64_t)ELLE_MS_NOW()}
                    });
                }
                std::string username, password, device_id, device_name;
                try {
                    auto j = json::parse(req.body);
                    if (j.contains("username") && j["username"].is_string())
                        username = j["username"].get<std::string>();
                    if (j.contains("password") && j["password"].is_string())
                        password = j["password"].get<std::string>();
                    if (j.contains("device_id") && j["device_id"].is_string())
                        device_id = j["device_id"].get<std::string>();
                    if (j.contains("device_name") && j["device_name"].is_string())
                        device_name = j["device_name"].get<std::string>();
                } catch (const json::exception&) {
                    return HTTPResponse::Err(400, "malformed JSON body");
                }

                if (username.empty() || username.size() > 32) {
                    return HTTPResponse::Err(400, "username required (<=32 chars)");
                }
                if (password.empty() || password.size() > 64) {
                    return HTTPResponse::Err(400, "password required (<=64 chars)");
                }

                if (!ElleGameAccountPool::Instance().IsAvailable()) {
                    return HTTPResponse::Err(503,
                        "game-account auth not configured "
                        "(set http_server.game_db_dsn)");
                }

                std::string peer;
                {
                    auto it = req.headers.find("x-peer-addr");
                    if (it != req.headers.end()) peer = it->second;
                }
                if (peer.empty()) peer = "unknown";

                const bool kTestNoAuth = (ElleConfig::Instance().GetInt(
                    "http_server.no_auth", 0) != 0);
                if (kTestNoAuth) {
                    ELLE_INFO("login: no_auth=1 — issuing synthetic "
                              "dev-tier token for user=\"%s\"",
                              username.c_str());
                    ELLE_LOG_HTTP("login SYNTHETIC user=\"%s\" peer=%s",
                                  username.c_str(), peer.c_str());
                    std::string tok = ElleCrypto::RandomHex(32);
                    if (tok.empty()) tok = std::string(64, 'f');
                    return HTTPResponse::OK({
                        {"ok",         true},
                        {"token",      tok},
                        {"nUserNo",    1},
                        {"sUserID",    username.empty() ? "single" : username},
                        {"sUserName",  username.empty() ? "Single" : username},
                        {"nAuthID",    9},
                        {"mode",       "no_auth_testing"}
                    });
                }

                const uint64_t now = (uint64_t)ELLE_MS_NOW();
                const std::string lkey = LoginKey(peer, username);

                {
                    std::lock_guard<std::mutex> lk(m_loginMutex);
                    LoginGcLocked(now);
                    const uint64_t remaining =
                        LoginCheckLockedLocked(lkey, now);
                    if (remaining > 0) {
                        ELLE_INFO("login: locked out user=\"%s\" peer=%s "
                                  "retry_after_ms=%llu",
                                  username.c_str(), peer.c_str(),
                                  (unsigned long long)remaining);
                        ELLE_LOG_HTTP("login LOCKED user=\"%s\" peer=%s retry_ms=%llu",
                                      username.c_str(), peer.c_str(),
                                      (unsigned long long)remaining);
                        HTTPResponse r = HTTPResponse::Err(429,
                            "too many failed attempts — try again later");
                        r.headers["Retry-After"] =
                            std::to_string((remaining + 999) / 1000);
                        return r;
                    }
                }

                ElleGameAuth::UserIdentity id;
                if (!ElleGameAuth::AuthenticateUser(username, password, id)) {
                    {
                        std::lock_guard<std::mutex> lk(m_loginMutex);
                        LoginRecordFailLocked(lkey, now);
                    }
                    ELLE_INFO("login: refused user=\"%s\" peer=%s",
                              username.c_str(), peer.c_str());
                    ELLE_LOG_HTTP("login REFUSED user=\"%s\" peer=%s",
                                  username.c_str(), peer.c_str());

                    return HTTPResponse::Err(401, "invalid credentials");
                }

                {
                    std::lock_guard<std::mutex> lk(m_loginMutex);
                    LoginRecordSuccessLocked(lkey);
                }

                if (device_id.empty()) {

                    device_id = "app:" + id.sUserID;
                }
                if (device_id.size() > 128) {
                    return HTTPResponse::Err(400, "device_id too long (<=128 chars)");
                }
                if (device_name.empty()) {
                    device_name = id.sUserID;
                }

                if (device_name.size() > 128) {
                    device_name.resize(128);
                }

                std::string token = ElleCrypto::RandomHex(32);
                if (token.empty() || token.size() != 64) {
                    return HTTPResponse::Err(500, "random token generation failed");
                }

                ElleDB::SessionRow sess;
                sess.token        = token;
                sess.nUserNo      = id.nUserNo;
                sess.sUserID      = id.sUserID;
                sess.sUserName    = id.sUserName;
                sess.nAuthID      = id.nAuthID;
                sess.created_ms   = now;
                sess.last_seen_ms = now;
                sess.device_name  = device_name;
                sess.peer_addr    = peer;

                if (sess.sUserID.size() > 30)   sess.sUserID.resize(30);
                if (sess.sUserName.size() > 60) sess.sUserName.resize(60);
                if (sess.device_name.size() > 128) sess.device_name.resize(128);
                if (sess.peer_addr.size() > 64) sess.peer_addr.resize(64);
                if (!ElleDB::CreateSession(sess)) {
                    auto probe = ElleSQLPool::Instance().Query(
                        "SELECT 1 FROM sys.databases WHERE name = 'ElleSystem';");
                    if (!probe.success || probe.rows.empty()) {
                        return HTTPResponse::Err(500,
                            "failed to persist session: missing database ElleSystem (create/restore ElleSystem and dbo.Sessions)");
                    }
                    auto probe2 = ElleSQLPool::Instance().Query(
                        "SELECT 1 FROM ElleSystem.sys.tables WHERE name = 'Sessions' AND schema_id = SCHEMA_ID('dbo');");
                    if (!probe2.success || probe2.rows.empty()) {
                        return HTTPResponse::Err(500,
                            "failed to persist session: missing table ElleSystem.dbo.Sessions");
                    }
                    return HTTPResponse::Err(500, "failed to persist session (SQL insert failed)");
                }

                if (id.nUserNo > 0) {
                    ElleDB::TouchUserContinuityOnPair(
                        id.nUserNo, id.sUserID, id.sUserName);
                }

                {
                    ElleDB::PairedDeviceRow pdrow;
                    pdrow.device_id    = device_id;
                    pdrow.device_name  = device_name;
                    pdrow.peer_addr    = peer;
                    pdrow.first_seen_ms = now;
                    pdrow.last_seen_ms  = now;
                    pdrow.revoked       = false;
                    if (!ElleDB::UpsertPairedDevice(pdrow)) {
                        ELLE_WARN("login: UpsertPairedDevice failed device=%s "
                                  "(non-fatal — login still succeeds)",
                                  device_id.c_str());
                    }
                    {
                        std::lock_guard<std::mutex> lk(g_pairedCacheMx);
                        g_pairedCache.erase(device_id);
                    }
                }

                ELLE_INFO("login: user=\"%s\" nUserNo=%lld nAuthID=%d "
                          "device=%s peer=%s",
                          id.sUserID.c_str(), (long long)id.nUserNo,
                          id.nAuthID, device_id.c_str(), peer.c_str());
                ELLE_LOG_HTTP("login OK user=\"%s\" nUserNo=%lld nAuthID=%d peer=%s",
                              id.sUserID.c_str(), (long long)id.nUserNo,
                              id.nAuthID, peer.c_str());

                return HTTPResponse::OK({
                    {"token",        token},
                    {"nUserNo",      id.nUserNo},
                    {"sUserID",      id.sUserID},
                    {"sUserName",    id.sUserName},
                    {"nAuthID",      id.nAuthID},
                    {"created_ms",   (int64_t)now}
                });
            }, AUTH_PUBLIC);

        m_router.Register("POST", "/api/auth/logout",
            [](const HTTPRequest& req) -> HTTPResponse {
                if (ElleConfig::Instance().GetInt("http_server.no_auth", 0) != 0) {
                    return HTTPResponse::OK({{"ok", true}});
                }
                auto it = req.headers.find("x-auth-device-id");
                if (it != req.headers.end() && !it->second.empty()) {
                    ElleDB::DeleteSession(it->second);
                }
                return HTTPResponse::OK({{"ok", true}});
            }, AUTH_USER);

        m_router.Register("GET", "/api/auth/me",
            [](const HTTPRequest& req) -> HTTPResponse {
                auto pick = [&](const char* key) -> std::string {
                    auto it = req.headers.find(key);
                    return it == req.headers.end() ? "" : it->second;
                };

                int64_t nUserNo = 0;
                try { nUserNo = std::stoll(pick("x-auth-nuserno")); }
                catch (const std::exception&) { nUserNo = 0; }
                int nAuthID = 0;
                try { nAuthID = std::stoi(pick("x-auth-id-level")); }
                catch (const std::exception&) { nAuthID = 0; }
                return HTTPResponse::OK({
                    {"nUserNo",   nUserNo},
                    {"sUserID",   pick("x-auth-user-id")},
                    {"sUserName", pick("x-auth-user-name")},
                    {"nAuthID",   nAuthID}
                });
            }, AUTH_USER);

        m_router.Register("GET", "/api/auth/devices",
            [](const HTTPRequest&) -> HTTPResponse {
                std::vector<ElleDB::PairedDeviceRow> rows;
                if (!ElleDB::ListPairedDevices(rows, 200)) {
                    return HTTPResponse::Err(500, "failed to list paired devices");
                }
                json arr = json::array();
                for (const auto& r : rows) {
                    arr.push_back({
                        {"device_id",       r.device_id},
                        {"device_name",     r.device_name},
                        {"paired_at_ms",    (int64_t)r.paired_at_ms},
                        {"expires_ms",      (int64_t)r.expires_ms},
                        {"last_seen_ms",    (int64_t)r.last_seen_ms},
                        {"revoked",         r.revoked},
                        {"revoked_at_ms",   (int64_t)r.revoked_at_ms},
                        {"jwt_fingerprint", r.jwt_fingerprint}
                    });
                }
                return HTTPResponse::OK({ {"devices", arr} });
            }, AUTH_ADMIN);

        m_router.Register("DELETE", "/api/auth/devices/{id}",
            [](const HTTPRequest& req) -> HTTPResponse {
                auto it = req.headers.find("x-path-id");
                if (it == req.headers.end() || it->second.empty()) {
                    return HTTPResponse::Err(400, "device id required");
                }

                ElleDB::RevokePairedDevice(it->second);

                {
                    std::lock_guard<std::mutex> lk(g_pairedCacheMx);
                    g_pairedCache.erase(it->second);
                }
                ELLE_INFO("Device revoked via admin: id=%s",
                          it->second.c_str());
                return HTTPResponse::OK({
                    {"revoked",   true},
                    {"device_id", it->second}
                });
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/auth/sessions",
            [](const HTTPRequest& req) -> HTTPResponse {
                uint32_t limit = 100;
                size_t q = req.path.find('?');
                if (q != std::string::npos) {
                    std::string qs = req.path.substr(q + 1);
                    size_t lp = qs.find("limit=");
                    if (lp != std::string::npos) {
                        try {
                            limit = (uint32_t)std::stoul(qs.substr(lp + 6));
                            if (limit == 0 || limit > 1000) limit = 100;
                        } catch (const std::exception&) { limit = 100; }
                    }
                }
                std::vector<ElleDB::SessionRow> rows;
                if (!ElleDB::ListSessions(rows, limit)) {
                    return HTTPResponse::Err(500, "failed to list sessions");
                }
                json arr = json::array();
                for (const auto& r : rows) {
                    arr.push_back({
                        {"token_prefix", r.token.substr(0, std::min<size_t>(8, r.token.size()))},
                        {"nUserNo",      r.nUserNo},
                        {"sUserID",      r.sUserID},
                        {"sUserName",    r.sUserName},
                        {"nAuthID",      r.nAuthID},
                        {"created_ms",   (int64_t)r.created_ms},
                        {"last_seen_ms", (int64_t)r.last_seen_ms},
                        {"device_name",  r.device_name},
                        {"peer_addr",    r.peer_addr}
                    });
                }
                return HTTPResponse::OK({ {"sessions", arr}, {"count", (int)arr.size()} });
            }, AUTH_ADMIN);

        m_router.Register("DELETE", "/api/auth/sessions/by-user/{nUserNo}",
            [](const HTTPRequest& req) -> HTTPResponse {
                auto it = req.headers.find("x-path-nUserNo");
                if (it == req.headers.end() || it->second.empty()) {
                    return HTTPResponse::Err(400, "nUserNo required");
                }
                int64_t target = 0;
                try { target = std::stoll(it->second); }
                catch (const std::exception&) {
                    return HTTPResponse::Err(400, "nUserNo not numeric");
                }
                int deleted = ElleDB::DeleteSessionsForUser(target);
                ELLE_INFO("Admin DELETE sessions/by-user/%lld → %d session(s) removed",
                          (long long)target, deleted);
                return HTTPResponse::OK({
                    {"nUserNo", target},
                    {"deleted", deleted}
                });
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/admin/logs",
            [](const HTTPRequest& req) -> HTTPResponse {
                uint32_t count = 100;
                int      svcFilter = -1;
                size_t q = req.path.find('?');
                if (q != std::string::npos) {
                    std::string qs = req.path.substr(q + 1);
                    size_t lp = qs.find("count=");
                    if (lp != std::string::npos) {
                        try {
                            count = (uint32_t)std::stoul(qs.substr(lp + 6));
                            if (count == 0 || count > 2000) count = 100;
                        } catch (const std::exception&) { count = 100; }
                    }
                    size_t sp = qs.find("svc=");
                    if (sp != std::string::npos) {
                        try { svcFilter = std::stoi(qs.substr(sp + 4)); }
                        catch (const std::exception&) { svcFilter = -1; }
                    }
                }
                std::vector<ELLE_LOG_ENTRY> entries;
                if (!ElleDB::GetRecentLogs(entries, count, svcFilter)) {
                    return HTTPResponse::Err(500, "failed to fetch logs");
                }
                json arr = json::array();
                for (const auto& e : entries) {
                    arr.push_back({
                        {"created_ms", (int64_t)e.created_ms},
                        {"level",      (int)e.level},
                        {"service",    (int)e.service},
                        {"message",    std::string(e.message)}
                    });
                }
                return HTTPResponse::OK({ {"logs", arr}, {"count", (int)arr.size()} });
            }, AUTH_ADMIN);

        m_router.Register("GET", "/api/auth/qr",
            [](const HTTPRequest&) -> HTTPResponse {
                return HTTPResponse::Err(410,
                    "pair-QR removed; use POST /api/auth/login with username+password");
            }, AUTH_PUBLIC);

    }
