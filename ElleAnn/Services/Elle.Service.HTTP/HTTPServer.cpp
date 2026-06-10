#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleComposerClient.h"
#include "../_Shared/ElleSQLFallback.h"
#include "../_Shared/DictionaryLoader.h"
#include "../_Shared/json.hpp"
#include "../_Shared/ElleCrypto.h"
#include "../_Shared/ElleQR.h"
#include "../_Shared/ElleGameAccountDB.h"
#include "../_Shared/ElleUserContinuity.h"
#include "../_Shared/ElleLexicalAdmin.h"
#include "../_Shared/ElleBeliefAdmin.h"
#include "../_Shared/ElleUploadGuard.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <psapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "psapi.lib")

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <regex>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <set>
#include <cstring>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <condition_variable>


#include "HTTPServer.h"

bool ElleHTTPService::OnStart() {

        ELLE_LOG_HTTP("OnStart — HTTP service booting");

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            ELLE_ERROR("WSAStartup failed");
            ELLE_LOG_SOCKET("WSAStartup FAILED");
            return false;
        }

        RegisterRoutes();

        auto& cfg = ElleConfig::Instance().GetHTTP();

        {
            bool noAuth = (ElleConfig::Instance().GetInt("http_server.no_auth", 0) != 0)
                       || !cfg.auth_enabled;
            bool publicBind = (cfg.bind_address != "127.0.0.1"
                            && cfg.bind_address != "localhost"
                            && cfg.bind_address != "::1");
            if (noAuth && publicBind) {
                const char* override_env = std::getenv("ELLE_UNSAFE_ALLOW_PUBLIC_NO_AUTH");
                bool allowed = (override_env && std::string(override_env) == "1");
                if (!allowed) {
                    ELLE_ERROR("HTTP service refusing to start: bind_address=%s "
                               "with no_auth/!auth_enabled is unsafe. "
                               "Bind to 127.0.0.1 OR enable auth OR set "
                               "ELLE_UNSAFE_ALLOW_PUBLIC_NO_AUTH=1 to proceed.",
                               cfg.bind_address.c_str());
                    return false;
                }
                ELLE_WARN("HTTP service starting in UNSAFE MODE: bind=%s no_auth=1 "
                          "(ELLE_UNSAFE_ALLOW_PUBLIC_NO_AUTH=1 override)",
                          cfg.bind_address.c_str());
            }
        }

        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET) {
            ELLE_ERROR("socket() failed: %d", WSAGetLastError());
            ELLE_LOG_SOCKET("socket() FAILED wsa=%d", WSAGetLastError());
            return false;
        }

        BOOL reuse = TRUE;
        setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&reuse, sizeof(reuse));

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons((u_short)cfg.port);
        inet_pton(AF_INET, cfg.bind_address.c_str(), &addr.sin_addr);

        if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            ELLE_ERROR("bind() failed: %d", WSAGetLastError());
            return false;
        }

        if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            ELLE_ERROR("listen() failed: %d", WSAGetLastError());
            return false;
        }

        m_shuttingDown.store(false);
        m_acceptThread = std::thread(&ElleHTTPService::AcceptLoop, this);

        uint32_t workers = MaxConcurrent();
        if (workers == 0) workers = 8;
        for (uint32_t i = 0; i < workers; ++i) {
            m_httpWorkers.emplace_back(&ElleHTTPService::HttpWorkerLoop, this);
        }

        ELLE_INFO("HTTP server listening on %s:%d (%zu routes, %u workers)",
                  cfg.bind_address.c_str(), cfg.port, m_router.Count(), workers);
        ELLE_LOG_HTTP("server listening on %s:%d (routes=%zu workers=%u)",
                      cfg.bind_address.c_str(), cfg.port, m_router.Count(), workers);

        ELLE_INFO("HTTP policy: auth=%s cors=%s(%s) rate=%u/min "
                  "maxConn=%u maxWsFrame=%u maxUpload=%u "
                  "admin_auth_id_threshold=%lld",
                  cfg.auth_enabled ? "ON" : "OFF",
                  cfg.cors_enabled ? "ON" : "OFF",
                  cfg.cors_origins.empty() ? "*" : cfg.cors_origins.c_str(),
                  cfg.rate_limit_rpm, cfg.max_concurrent_connections,
                  cfg.max_ws_frame_bytes, cfg.max_upload_bytes,
                  (long long)ElleConfig::Instance().GetInt(
                      "http_server.admin_auth_id_threshold", 9));

        const std::string game_dsn = ElleConfig::Instance().GetString(
            "http_server.game_db_dsn", "");
        if (!game_dsn.empty()) {
            const uint32_t game_pool_size = (uint32_t)
                ElleConfig::Instance().GetInt("http_server.game_db_pool_size", 4);
            if (ElleGameAccountPool::Instance().Initialize(game_dsn,
                                                           game_pool_size)) {
                ELLE_INFO("game-DB auth path: ENABLED (pool=%u)",
                          game_pool_size);
            } else {
                ELLE_WARN("game-DB auth path: requested but Initialize() "
                          "failed — falling back to pair-code only");
            }
        } else {
            ELLE_INFO("game-DB auth path: disabled (http_server.game_db_dsn unset)");
        }

        return true;
    }

void ElleHTTPService::OnStop() {

        ElleGameAccountPool::Instance().Shutdown();

        m_shuttingDown.store(true);
        if (m_listenSocket != INVALID_SOCKET) {
            closesocket(m_listenSocket);
            m_listenSocket = INVALID_SOCKET;
        }
        if (m_acceptThread.joinable()) m_acceptThread.join();

        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            for (auto& c : m_wsClients) {
                if (c->socket != INVALID_SOCKET) closesocket(c->socket);
                c->alive = false;
            }
        }

        m_socketCv.notify_all();
        for (auto& t : m_httpWorkers) {
            if (t.joinable()) t.join();
        }
        m_httpWorkers.clear();
        {
            std::lock_guard<std::mutex> lock(m_socketMx);
            while (!m_socketQueue.empty()) {
                SOCKET s = m_socketQueue.front();
                m_socketQueue.pop_front();
                if (s != INVALID_SOCKET) closesocket(s);
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_wsThreadsMx);
            for (auto& t : m_wsThreads) {
                if (t.joinable()) t.join();
            }
            m_wsThreads.clear();
        }

        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            m_wsClients.clear();
        }

        WSACleanup();
        ELLE_INFO("HTTP server stopped");
    }

void ElleHTTPService::OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        if (msg.header.msg_type == IPC_EMOTION_UPDATE) {
            ELLE_EMOTION_STATE state;
            if (msg.GetPayload(state)) {
                m_cachedEmotions = state;
            }
        }

        if (msg.header.msg_type == IPC_CHAT_RESPONSE) {
            try {
                std::string s = msg.GetStringPayload();
                json j = json::parse(s);
                std::string rid = j.value("request_id", "");
                if (!rid.empty()) {
                    m_chatCorrelator.Complete(rid, j);
                }
            } catch (const std::exception& e) {
                ELLE_ERROR("IPC_CHAT_RESPONSE parse error: %s", e.what());
            }
            return;
        }

        if (msg.header.msg_type == IPC_EMOTION_UPDATE ||
            msg.header.msg_type == IPC_LOG_ENTRY ||
            msg.header.msg_type == IPC_TRUST_UPDATE ||
            msg.header.msg_type == IPC_WORLD_EVENT) {

            BroadcastIPCToWebSockets(msg);
        }
    }

std::vector<ELLE_SERVICE_ID> ElleHTTPService::GetDependencies() {

        return { SVC_HEARTBEAT, SVC_COGNITIVE, SVC_EMOTIONAL, SVC_MEMORY };
    }

uint32_t ElleHTTPService::MaxConcurrent() const {
        return ElleConfig::Instance().GetHTTP().max_concurrent_connections;
    }

void ElleHTTPService::PairingGcLocked(uint64_t now) {
        for (auto it = m_pairingCodes.begin(); it != m_pairingCodes.end(); ) {
            if (it->second.consumed || it->second.expires_ms <= now) {
                it = m_pairingCodes.erase(it);
            } else {
                ++it;
            }
        }
    }

std::string ElleHTTPService::LoginKey(const std::string& ip, const std::string& user) {
        std::string k;
        k.reserve(ip.size() + 1 + user.size());
        k.append(ip);
        k.push_back('|');

        for (char c : user) k.push_back((char)std::tolower((unsigned char)c));
        return k;
    }

uint64_t ElleHTTPService::LoginCheckLockedLocked(const std::string& key, uint64_t now) {
        auto it = m_loginFails.find(key);
        if (it == m_loginFails.end()) return 0;
        if (it->second.lockout_until > now) return it->second.lockout_until - now;
        return 0;
    }

void ElleHTTPService::LoginRecordFailLocked(const std::string& key, uint64_t now) {
        auto& rec = m_loginFails[key];
        if (rec.window_start == 0 || now - rec.window_start > kLoginWindowMs) {
            rec.window_start = now;
            rec.fail_count   = 0;
        }
        rec.fail_count++;
        if (rec.fail_count >= kLoginMaxFails) {
            rec.lockout_until = now + kLoginLockoutMs;
        }

        if (m_loginFails.size() > kMaxLoginKeys) LoginGcLocked(now);
    }

void ElleHTTPService::LoginRecordSuccessLocked(const std::string& key) {
        m_loginFails.erase(key);
    }

void ElleHTTPService::LoginGcLocked(uint64_t now) {
        for (auto it = m_loginFails.begin(); it != m_loginFails.end(); ) {
            const uint64_t last = std::max(it->second.window_start,
                                            it->second.lockout_until);
            if (last + kLoginRecordTtlMs < now) {
                it = m_loginFails.erase(it);
            } else {
                ++it;
            }
        }
    }

std::string ElleHTTPService::MintJwt(const std::string& deviceId, const std::string& deviceName, uint64_t iat_ms, uint64_t exp_ms, const std::string& secret) {

        const std::string header = R"({"alg":"HS256","typ":"JWT"})";
        std::ostringstream ps;
        ps << R"({"sub":")" << deviceId
           << R"(","name":")" << deviceName
           << R"(","iat":)" << iat_ms
           << R"(,"exp":)" << exp_ms << "}";
        const std::string payload = ps.str();

        std::string h64 = ElleCrypto::Base64UrlEncode(header.data(), header.size());
        std::string p64 = ElleCrypto::Base64UrlEncode(payload.data(), payload.size());
        std::string signingInput = h64 + "." + p64;

        uint8_t mac[32];
        if (!ElleCrypto::HmacSha256(secret.data(), secret.size(),
                                     signingInput.data(), signingInput.size(),
                                     mac)) {
            return {};
        }
        std::string s64 = ElleCrypto::Base64UrlEncode(mac, 32);
        return signingInput + "." + s64;
    }

void ElleHTTPService::AcceptLoop() {
        while (Running()) {
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(m_listenSocket, &readSet);

            timeval timeout = {1, 0};
            int result = select(0, &readSet, nullptr, nullptr, &timeout);
            if (result <= 0) continue;

            SOCKET clientSocket = accept(m_listenSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) continue;

            uint32_t cap = MaxConcurrent();
            {
                std::unique_lock<std::mutex> lock(m_socketMx);
                if (cap > 0 && m_socketQueue.size() >= cap) {
                    lock.unlock();
                    HTTPResponse r = HTTPResponse::Err(503, "server busy — try again");
                    std::string data = r.Serialize();
                    send(clientSocket, data.c_str(), (int)data.size(), 0);
                    shutdown(clientSocket, SD_SEND);
                    closesocket(clientSocket);
                    continue;
                }
                m_socketQueue.push_back(clientSocket);
            }
            m_socketCv.notify_one();
        }
    }

void ElleHTTPService::HttpWorkerLoop() {
        for (;;) {
            SOCKET s = INVALID_SOCKET;
            {
                std::unique_lock<std::mutex> lock(m_socketMx);
                m_socketCv.wait(lock, [this]{
                    return m_shuttingDown.load() || !m_socketQueue.empty();
                });
                if (m_shuttingDown.load() && m_socketQueue.empty()) return;
                s = m_socketQueue.front();
                m_socketQueue.pop_front();
            }
            if (s == INVALID_SOCKET) continue;
            try { HandleClient(s); }
            catch (const std::exception& e) { ELLE_ERROR("HTTP worker: %s", e.what()); }

        }
    }

void ElleHTTPService::HandleClient(SOCKET clientSocket) {
        try {
            static std::once_flag s_gameAuthSelfTestOnce;
            std::call_once(s_gameAuthSelfTestOnce, [](){
                try { RunGameAuthSelfTest(); }
                catch (const std::exception& e) {
                    SetGameAuthDiag({
                        {"ts_ms", (int64_t)ELLE_MS_NOW()},
                        {"error", std::string("selftest exception: ") + e.what()}
                    });
                }
            });

            int optval = 1;
            setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
                       (const char*)&optval, sizeof(optval));

            DWORD recvTimeout = 15000;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,
                       (const char*)&recvTimeout, sizeof(recvTimeout));

            std::string raw;
            raw.reserve(4096);
            char buf[8192];
            size_t headerEnd = std::string::npos;

            while (headerEnd == std::string::npos) {
                int n = recv(clientSocket, buf, sizeof(buf), 0);
                if (n <= 0) break;
                raw.append(buf, n);
                headerEnd = raw.find("\r\n\r\n");
                if (raw.size() > 1024 * 1024) break;
            }

            if (headerEnd == std::string::npos) {
                closesocket(clientSocket);
                return;
            }

            HTTPRequest req = ParseRequest(raw);

            {
                sockaddr_storage peer{};
                int peerLen = (int)sizeof(peer);
                if (getpeername(clientSocket, (sockaddr*)&peer, &peerLen) == 0) {
                    char host[NI_MAXHOST] = {0};
                    if (getnameinfo((sockaddr*)&peer, peerLen,
                                    host, sizeof(host),
                                    nullptr, 0, NI_NUMERICHOST) == 0) {
                        req.headers["x-peer-addr"] = host;
                    }
                }
            }

            auto clIt = req.headers.find("content-length");
            if (clIt != req.headers.end()) {
                long long clSigned = 0;
                if (!HTTPRequest::StrictParseLL(clIt->second, clSigned) ||
                    clSigned < 0) {
                    SendResponse(clientSocket,
                                 HTTPResponse::Err(400, "invalid Content-Length"));
                    return;
                }
                uint64_t capBytes = ElleConfig::Instance().GetHTTP().max_upload_bytes;
                if (capBytes == 0) capBytes = 10ULL * 1024 * 1024;
                if ((uint64_t)clSigned > capBytes) {
                    SendResponse(clientSocket,
                                 HTTPResponse::Err(413, "payload too large"));
                    return;
                }
                size_t contentLen = (size_t)clSigned;
                while (req.body.size() < contentLen) {
                    int n = recv(clientSocket, buf, sizeof(buf), 0);
                    if (n <= 0) break;
                    req.body.append(buf, n);
                }
                if (req.body.size() > contentLen) req.body.resize(contentLen);
            }

            ELLE_DEBUG("HTTP %s %s", req.method.c_str(), req.path.c_str());

            if (req.method == "OPTIONS") {
                HTTPResponse resp;
                resp.status = 204;
                resp.statusText = "No Content";
                SendResponse(clientSocket, resp);
                return;
            }

            if (req.isWebSocket) {
                HandleWebSocketUpgrade(clientSocket, req);
                return;
            }

            HTTPResponse resp = m_router.Dispatch(req);
            SendResponse(clientSocket, resp);

        } catch (const std::invalid_argument& e) {

            ELLE_DEBUG("HTTP 400: %s", e.what());
            try { SendResponse(clientSocket, HTTPResponse::Err(400, e.what())); }
            catch (const std::exception& se) {
                ELLE_WARN("HTTP 400 write failed: %s", se.what());
                closesocket(clientSocket);
            }
        } catch (const std::exception& e) {
            try { SendResponse(clientSocket, InternalErrorResponse(e.what())); }
            catch (const std::exception& se) {
                ELLE_WARN("HTTP 500 write failed: %s", se.what());
                closesocket(clientSocket);
            }

        }
    }

void ElleHTTPService::SendResponse(SOCKET clientSocket, const HTTPResponse& resp) {
        std::string data = resp.Serialize();
        int totalSent = 0;
        int remaining = (int)data.size();
        while (remaining > 0) {
            int sent = send(clientSocket, data.c_str() + totalSent, remaining, 0);
            if (sent == SOCKET_ERROR) break;
            totalSent += sent;
            remaining -= sent;
        }
        shutdown(clientSocket, SD_SEND);
        char drain[1024];
        while (recv(clientSocket, drain, sizeof(drain), 0) > 0) {}
        closesocket(clientSocket);
    }

HTTPRequest ElleHTTPService::ParseRequest(const std::string& raw) {
        HTTPRequest req;
        std::istringstream iss(raw);
        std::string line;

        std::getline(iss, line);
        std::istringstream reqLine(line);
        reqLine >> req.method >> req.path;

        size_t qPos = req.path.find('?');
        if (qPos != std::string::npos) {
            req.query = req.path.substr(qPos + 1);
            req.path = req.path.substr(0, qPos);
            ParseQueryString(req.query, req.queryParams);
        }

        while (std::getline(iss, line) && line != "\r" && !line.empty()) {
            if (line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(0, 1);

                std::transform(key.begin(), key.end(), key.begin(),
                               [](unsigned char c){ return (char)std::tolower(c); });
                req.headers[key] = val;
            }
        }

        if (req.headers.count("upgrade")) {
            std::string up = req.headers["upgrade"];
            std::transform(up.begin(), up.end(), up.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            if (up == "websocket") {
                req.isWebSocket = true;
                req.wsKey = req.headers["sec-websocket-key"];
            }
        }

        size_t headerEnd = raw.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            req.body = raw.substr(headerEnd + 4);
        }

        return req;
    }

void ElleHTTPService::ParseQueryString(const std::string& q, std::unordered_map<std::string, std::string>& out) {
        size_t start = 0;
        while (start < q.size()) {
            size_t amp = q.find('&', start);
            std::string pair = q.substr(start, amp == std::string::npos ? q.size() - start : amp - start);
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                out[pair.substr(0, eq)] = UrlDecode(pair.substr(eq + 1));
            } else {
                out[pair] = "";
            }
            if (amp == std::string::npos) break;
            start = amp + 1;
        }
    }

std::string ElleHTTPService::UrlDecode(const std::string& s) {

        auto hex = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
            return -1;
        };
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '%' && i + 2 < s.size()) {
                int hi = hex(s[i + 1]);
                int lo = hex(s[i + 2]);
                if (hi < 0 || lo < 0) {
                    out.push_back(s[i]);
                } else {
                    out.push_back((char)((hi << 4) | lo));
                    i += 2;
                }
            } else if (s[i] == '+') {
                out.push_back(' ');
            } else {
                out.push_back(s[i]);
            }
        }
        return out;
    }

void ElleHTTPService::HandleWebSocketUpgrade(SOCKET clientSocket, const HTTPRequest& req) {

        const auto& httpCfg = ElleConfig::Instance().GetHTTP();
        if (httpCfg.auth_enabled) {
            const std::string& secret = httpCfg.jwt_secret;
            std::string adminKey = ElleConfig::Instance().GetString(
                "http_server.admin_key", secret);
            if (secret.empty() && adminKey.empty()) {
                ELLE_WARN("WS upgrade refused: auth_enabled=true but no "
                          "jwt_secret/admin_key configured");
                SendResponse(clientSocket,
                             HTTPResponse::Err(503, "auth misconfigured"));
                return;
            }
            auto constTimeEq = [](const std::string& a, const std::string& b) {
                if (a.size() != b.size()) return false;
                unsigned diff = 0;
                for (size_t i = 0; i < a.size(); ++i)
                    diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
                return diff == 0;
            };
            bool ok = false;
            auto authIt = req.headers.find("authorization");
            if (authIt != req.headers.end()) {
                static const std::string kBearer = "Bearer ";
                const std::string& v = authIt->second;
                if (v.size() > kBearer.size() &&
                    std::equal(kBearer.begin(), kBearer.end(), v.begin(),
                               [](char x, char y){
                                   return std::tolower((unsigned char)x) ==
                                          std::tolower((unsigned char)y);
                               })) {
                    std::string tok = v.substr(kBearer.size());
                    while (!tok.empty() && (tok.front() == ' ' ||
                                            tok.front() == '\t'))
                        tok.erase(0, 1);
                    if (!secret.empty() && constTimeEq(tok, secret)) ok = true;
                }
            }
            if (!ok) {
                auto keyIt = req.headers.find("x-admin-key");
                if (keyIt != req.headers.end() && !adminKey.empty() &&
                    constTimeEq(keyIt->second, adminKey)) ok = true;
            }

            if (!ok) {
                auto protoIt = req.headers.find("sec-websocket-protocol");
                if (protoIt != req.headers.end()) {
                    static const std::string kElle = "elle.";
                    std::string v = protoIt->second;

                    size_t p = 0;
                    while (p < v.size() && !ok) {
                        size_t comma = v.find(',', p);
                        std::string one = v.substr(p,
                            comma == std::string::npos ? v.size() - p : comma - p);
                        while (!one.empty() && (one.front() == ' ' ||
                                                one.front() == '\t'))
                            one.erase(0, 1);
                        while (!one.empty() && (one.back() == ' ' ||
                                                one.back() == '\t'))
                            one.pop_back();
                        if (one.size() > kElle.size() &&
                            one.compare(0, kElle.size(), kElle) == 0) {
                            std::string tok = one.substr(kElle.size());
                            if (!secret.empty() && constTimeEq(tok, secret)) ok = true;
                        }
                        if (comma == std::string::npos) break;
                        p = comma + 1;
                    }
                }
            }
            if (!ok) {
                ELLE_WARN("WS upgrade refused: missing/invalid auth");
                SendResponse(clientSocket,
                             HTTPResponse::Err(401, "WS auth required"));
                return;
            }
        }

        if (req.wsKey.empty()) {
            SendResponse(clientSocket, HTTPResponse::Err(400, "Missing Sec-WebSocket-Key"));
            return;
        }
        std::string accept = MakeWsAccept(req.wsKey);
        if (accept.empty()) {
            SendResponse(clientSocket, HTTPResponse::Err(500, "SHA1 failed"));
            return;
        }

        std::ostringstream resp;
        resp << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << accept << "\r\n\r\n";
        std::string r = resp.str();
        int sent = send(clientSocket, r.c_str(), (int)r.size(), 0);
        if (sent == SOCKET_ERROR) {
            closesocket(clientSocket);
            return;
        }

        DWORD zero = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,
                   (const char*)&zero, sizeof(zero));

        auto client = std::make_shared<WSClient>();
        client->socket = clientSocket;
        client->connected_ms = ELLE_MS_NOW();
        client->id = "ws-" + std::to_string(ELLE_MS_NOW());

        uint32_t cap = MaxConcurrent();
        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            if (cap > 0 && m_wsClients.size() >= cap) {
                ELLE_WARN("WS cap %u reached — refusing new client", cap);
                closesocket(clientSocket);
                return;
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            m_wsClients.push_back(client);
        }

        ELLE_INFO("WebSocket client connected: %s", client->id.c_str());
        ELLE_LOG_SOCKET("WS CONNECT id=%s", client->id.c_str());

        json welcome = {
            {"type", "connected"},
            {"client_id", client->id},
            {"server", "Elle-Ann"},
            {"version", ELLE_VERSION_STRING}
        };
        WsSendText(client->socket, client->sendMutex, welcome.dump());

        {
            std::lock_guard<std::mutex> lock(m_wsThreadsMx);

            for (auto it = m_wsThreads.begin(); it != m_wsThreads.end(); ) {
                if (it->joinable() && m_reapableWsThreadIds.count(it->get_id())) {
                    it->join();
                    m_reapableWsThreadIds.erase(it->get_id());
                    it = m_wsThreads.erase(it);
                } else {
                    ++it;
                }
            }
            m_wsThreads.emplace_back([this, client]() {
                this->WebSocketReadLoop(client);
                std::lock_guard<std::mutex> lk(m_wsThreadsMx);
                m_reapableWsThreadIds.insert(std::this_thread::get_id());
            });
        }
    }

void ElleHTTPService::WebSocketReadLoop(std::shared_ptr<WSClient> client) {

        enum class Exit {
            CLEAN_CLOSE, PROTOCOL_VIOLATION, NETWORK_ERROR, HANDLER_EXCEPTION
        } exitReason = Exit::CLEAN_CLOSE;
        std::string exitDetail;
        uint64_t    lastViolationLogMs = 0;
        uint32_t    suppressedViolations = 0;

        auto logViolation = [&](const std::string& reason) {
            uint64_t now = ELLE_MS_NOW();
            if (now - lastViolationLogMs < 5000) {
                suppressedViolations++;
                return;
            }
            if (suppressedViolations > 0) {
                ELLE_WARN("WS %s: %u violations suppressed in last window",
                          client->id.c_str(), suppressedViolations);
                suppressedViolations = 0;
            }
            lastViolationLogMs = now;
            ELLE_WARN("WS %s protocol violation: %s", client->id.c_str(),
                      reason.c_str());
        };

        while (client->alive && Running()) {
            std::string payload;
            int opcode = 0;
            WsFrameStatus st = WsReadFrameStatus(client->socket, payload, opcode);
            if (st == WsFrameStatus::CleanClose) {
                exitReason = Exit::CLEAN_CLOSE;
                break;
            }
            if (st == WsFrameStatus::NetworkError) {
                exitReason = Exit::NETWORK_ERROR;
                exitDetail = "recv failed (" + std::to_string(WSAGetLastError()) + ")";
                break;
            }
            if (st == WsFrameStatus::ProtocolViolation) {
                logViolation("malformed frame");
                exitReason = Exit::PROTOCOL_VIOLATION;
                exitDetail = "malformed frame";

                unsigned char close1002[4] = { 0x88, 0x02, 0x03, 0xEA };
                std::lock_guard<std::mutex> lk(client->sendMutex);
                send(client->socket, (const char*)close1002, 4, 0);
                break;
            }

            if (opcode == 0x8) {
                exitReason = Exit::CLEAN_CLOSE;
                break;
            }
            if (opcode == 0x9) {
                std::lock_guard<std::mutex> lk(client->sendMutex);
                unsigned char pong[2] = {0x8A, 0x00};
                send(client->socket, (const char*)pong, 2, 0);
                continue;
            }
            if (opcode == 0xA) {
                continue;
            }
            if (opcode == 0x1) {

                if (!IsValidUtf8(payload)) {
                    logViolation("non-UTF-8 text frame");
                    exitReason = Exit::PROTOCOL_VIOLATION;
                    exitDetail = "non-UTF-8 text frame";
                    unsigned char close1007[4] = { 0x88, 0x02, 0x03, 0xEF };
                    std::lock_guard<std::mutex> lk(client->sendMutex);
                    send(client->socket, (const char*)close1007, 4, 0);
                    break;
                }
                try {
                    HandleWebSocketMessage(client, payload);
                } catch (const std::exception& e) {

                    ELLE_WARN("WS %s handler exception: %s",
                              client->id.c_str(), e.what());
                    json err = {
                        {"type", "error"},
                        {"message", std::string("handler exception: ") + e.what()}
                    };
                    WsSendText(client->socket, client->sendMutex, err.dump());
                }
                continue;
            }

            logViolation("unknown opcode 0x" + std::to_string(opcode));
            exitReason = Exit::PROTOCOL_VIOLATION;
            exitDetail = "unknown opcode";
            break;
        }

        switch (exitReason) {
            case Exit::CLEAN_CLOSE:
                ELLE_INFO("WS %s closed cleanly", client->id.c_str());
                break;
            case Exit::PROTOCOL_VIOLATION:
                ELLE_WARN("WS %s closed due to protocol violation: %s",
                          client->id.c_str(), exitDetail.c_str());
                break;
            case Exit::NETWORK_ERROR:
                ELLE_INFO("WS %s network error: %s",
                          client->id.c_str(), exitDetail.c_str());
                break;
            case Exit::HANDLER_EXCEPTION:
                ELLE_ERROR("WS %s torn down by handler exception: %s",
                           client->id.c_str(), exitDetail.c_str());
                break;
        }

        client->alive = false;
        closesocket(client->socket);
        client->socket = INVALID_SOCKET;

        std::lock_guard<std::mutex> lock(m_wsMutex);
        m_wsClients.erase(
            std::remove_if(m_wsClients.begin(), m_wsClients.end(),
                [&](const std::shared_ptr<WSClient>& c) { return c.get() == client.get(); }),
            m_wsClients.end());
        ELLE_INFO("WebSocket client disconnected: %s", client->id.c_str());
        ELLE_LOG_SOCKET("WS DISCONNECT id=%s", client->id.c_str());
    }

void ElleHTTPService::HandleWebSocketMessage(std::shared_ptr<WSClient> client, const std::string& payload) {
        json msg;
        try { msg = json::parse(payload); }
        catch (const std::exception& e) {

            ELLE_DEBUG("WS invalid JSON: %s", e.what());
            WsSendText(client->socket, client->sendMutex,
                       R"({"type":"error","error":"invalid_json"})");
            return;
        }

        std::string type = msg.value("type", "");

        if (type == "ping") {
            WsSendText(client->socket, client->sendMutex,
                       R"({"type":"pong"})");
        } else if (type == "chat") {

            std::string message = msg.value("message", "");
            uint64_t convId = msg.value("conversation_id", (uint64_t)1);
            std::string userId = msg.value("user_id", std::string("default"));
            std::string clientReqId = msg.value("request_id", "");

            std::string requestId = "ws-" + std::to_string(ELLE_MS_NOW()) +
                                    "-" + std::to_string(++m_requestSeq);

            json env = {
                {"request_id", requestId},
                {"user_text", message},
                {"user_id", userId},
                {"conv_id", convId},
                {"origin", "ws"}
            };

            auto pending = m_chatCorrelator.Register(requestId);
            auto ipcMsg = ElleIPCMessage::Create(IPC_CHAT_REQUEST, SVC_HTTP_SERVER, SVC_COGNITIVE);
            ipcMsg.SetStringPayload(env.dump());
            json out;
            out["type"] = "chat_response";
            out["request_id"] = clientReqId;
            out["conversation_id"] = convId;

            if (!GetIPCHub().Send(SVC_COGNITIVE, ipcMsg)) {
                m_chatCorrelator.Cancel(requestId);
                out["error"] = "Cognitive service unreachable";
                WsSendText(client->socket, client->sendMutex, out.dump());
                return;
            }

            std::unique_lock<std::mutex> lk(pending->m);
            bool ok = pending->cv.wait_for(lk, std::chrono::seconds(45),
                                           [&]{ return pending->done; });
            if (!ok) {
                m_chatCorrelator.Cancel(requestId);
                out["error"] = "Cognitive timeout";
            } else {
                json r = pending->result;
                if (r.contains("response")) out["response"] = r["response"];
                if (r.contains("error"))    out["error"] = r["error"];
                if (r.contains("mode"))     out["mode"] = r["mode"];
                if (r.contains("memories_used")) out["memories_used"] = r["memories_used"];
            }
            WsSendText(client->socket, client->sendMutex, out.dump());
        } else if (type == "subscribe") {

            json out = {{"type", "subscribed"}, {"topic", msg.value("topic", "")}};
            WsSendText(client->socket, client->sendMutex, out.dump());
        } else {
            json out = {{"type", "ack"}, {"received", type}};
            WsSendText(client->socket, client->sendMutex, out.dump());
        }
    }

void ElleHTTPService::BroadcastIPCToWebSockets(const ElleIPCMessage& msg) {
        json payload;
        payload["type"] = "ipc_broadcast";
        payload["msg_type"] = (int)msg.header.msg_type;
        payload["tick"] = (uint64_t)ELLE_MS_NOW();

        if (msg.header.msg_type == IPC_EMOTION_UPDATE) {
            ELLE_EMOTION_STATE state;
            if (msg.GetPayload(state)) {
                payload["emotion"] = {
                    {"valence", state.valence},
                    {"arousal", state.arousal},
                    {"dominance", state.dominance},
                    {"tick", state.tick_count}
                };
            }
        }
        else if (msg.header.msg_type == IPC_WORLD_EVENT) {

            payload["type"] = "world_event";
            try {
                std::string s = msg.GetStringPayload();
                if (!s.empty()) payload["data"] = json::parse(s);
            } catch (const std::exception&) {
                payload["data"] = { {"raw", msg.GetStringPayload()} };
            }
        }

        std::string out = payload.dump();
        std::vector<std::shared_ptr<WSClient>> snapshot;
        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            snapshot = m_wsClients;
        }
        for (auto& c : snapshot) {
            if (!c->alive || c->socket == INVALID_SOCKET) continue;
            WsSendText(c->socket, c->sendMutex, out);
        }
    }

int32_t ElleHTTPService::ResolveAuthenticatedUser(const HTTPRequest& req) {
        auto it = req.headers.find("x-auth-device-id");
        if (it == req.headers.end()) return 0;
        const std::string& deviceId = it->second;
        if (deviceId.empty()) return 0;

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP 1 nUserNo FROM ElleCore.dbo.PairedDevices "
            "  WHERE DeviceId = ? AND Revoked = 0;",
            { deviceId });
        if (!rs.success || rs.rows.empty()) return 0;
        int64_t v = 0;
        return rs.rows[0].TryGetInt(0, v) && v > 0 && v <= INT32_MAX
                   ? (int32_t)v : 0;
    }

std::optional<HTTPResponse> ElleHTTPService::RequireUserId(const json& body, int32_t& out) {
        if (!body.contains("user_id"))
            return HTTPResponse::Err(400, "user_id is required");
        auto& v = body.at("user_id");
        int64_t raw = 0;
        if (v.is_number_integer())      raw = v.get<int64_t>();
        else if (v.is_number_unsigned()) raw = (int64_t)v.get<uint64_t>();
        else if (v.is_string()) {
            if (!HTTPRequest::StrictParseLL(v.get<std::string>(), raw))
                return HTTPResponse::Err(400, "user_id must be a positive integer");
        } else {
            return HTTPResponse::Err(400, "user_id must be a positive integer");
        }
        if (raw <= 0 || raw > INT32_MAX)
            return HTTPResponse::Err(400, "user_id must be a positive integer");
        out = (int32_t)raw;
        return std::nullopt;
    }

std::optional<HTTPResponse> ElleHTTPService::RequireAuthOrBodyUser(const HTTPRequest& req, const json& body, int32_t& out) {
        const int32_t fromJwt = ResolveAuthenticatedUser(req);
        if (fromJwt > 0) { out = fromJwt; return std::nullopt; }
        return RequireUserId(body, out);
    }

void ElleHTTPService::RegisterRoutes() {
        RegisterIntroRoutes();
        RegisterAuthRoutes();
        RegisterDiagRoutes();
        RegisterAdminRoutes();
        RegisterMemoryRoutes();
        RegisterEmotionRoutes();
        RegisterMeTokensRoutes();
        RegisterVideoIdentityRoutes();
        RegisterAIRoutes();
        RegisterDictionaryRoutes();
        RegisterEducationRoutes();
        RegisterEmotionalContextRoutes();
        RegisterXLifecycleRoutes();
        RegisterServerRoutes();
        RegisterModelsRoutes();
        RegisterMoralsGoalsRoutes();
        RegisterMiscRoutes();
        RegisterSHNRoutes();
        ELLE_INFO("Registered %zu API routes", m_router.Count());
    }

ELLE_SERVICE_MAIN(ElleHTTPService)
