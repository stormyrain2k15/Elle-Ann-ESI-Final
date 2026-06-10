#pragma once

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
#include <condition_variable>
#include <deque>
#include <set>
#include <cstring>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <memory>
#include <optional>

using json = nlohmann::json;

inline std::mutex g_diagMx;
inline json g_gameAuthDiag = json::object();

inline void SetGameAuthDiag(const json& j) {
    std::lock_guard<std::mutex> lk(g_diagMx);
    g_gameAuthDiag = j;
}
inline json GetGameAuthDiag() {
    std::lock_guard<std::mutex> lk(g_diagMx);
    return g_gameAuthDiag;
}

inline void RunGameAuthSelfTest() {
    json out = {
        {"ts_ms", (int64_t)ELLE_MS_NOW()},
        {"configured", false},
        {"pool_available", false},
        {"proc_usp_User_loginGame", "unknown"},
        {"proc_usp_set_login", "unknown"},
        {"tUser_select", "not_run"}
    };

    const std::string dsn = ElleConfig::Instance().GetString("http_server.game_db_dsn", "");
    if (dsn.empty()) {
        out["error"] = "http_server.game_db_dsn not set";
        SetGameAuthDiag(out);
        return;
    }
    out["configured"] = true;
    out["pool_available"] = ElleGameAccountPool::Instance().IsAvailable();

    auto& pool = ElleGameAccountPool::Instance();
    auto rsProc = pool.Query(
        "SELECT name FROM sys.objects WHERE type = 'P' AND name IN ('usp_User_loginGame','usp_set_login');");
    if (rsProc.success) {
        bool hasLoginGame = false;
        bool hasSetLogin  = false;
        for (auto& r : rsProc.rows) {
            const std::string nm = r.values.size() > 0 ? r.values[0] : "";
            if (nm == "usp_User_loginGame") hasLoginGame = true;
            if (nm == "usp_set_login")     hasSetLogin  = true;
        }
        out["proc_usp_User_loginGame"] = hasLoginGame ? "present" : "missing";
        out["proc_usp_set_login"]      = hasSetLogin  ? "present" : "missing";
    } else {
        out["proc_check_error"] = rsProc.error;
    }

    auto rsUser = pool.Query("SELECT TOP 1 nUserNo, sUserID FROM dbo.tUser WITH(NOLOCK);");
    if (rsUser.success) {
        out["tUser_select"] = rsUser.rows.empty() ? "ok_empty" : "ok";
        if (!rsUser.rows.empty()) {
            auto& r = rsUser.rows[0];
            out["sample_nUserNo"] = r.GetIntOr(0, 0);
            out["sample_sUserID"] = r.values.size() > 1 ? r.values[1] : "";
        }
    } else {
        out["tUser_select"] = "failed";
        out["tUser_error"] = rsUser.error;
    }

    SetGameAuthDiag(out);
}

inline constexpr const char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string Base64Encode(const unsigned char* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = (uint32_t)data[i] << 16;
        if (i + 1 < len) n |= (uint32_t)data[i + 1] << 8;
        if (i + 2 < len) n |= (uint32_t)data[i + 2];
        out.push_back(kB64[(n >> 18) & 63]);
        out.push_back(kB64[(n >> 12) & 63]);
        out.push_back(i + 1 < len ? kB64[(n >> 6) & 63] : '=');
        out.push_back(i + 2 < len ? kB64[n & 63] : '=');
    }
    return out;
}

inline bool SHA1Hash(const std::string& input, unsigned char out[20]) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    bool ok = false;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA1_ALGORITHM, nullptr, 0);
    if (status != 0) return false;
    status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    if (status == 0) {
        status = BCryptHashData(hHash, (PUCHAR)input.data(), (ULONG)input.size(), 0);
        if (status == 0) {
            status = BCryptFinishHash(hHash, out, 20, 0);
            ok = (status == 0);
        }
        BCryptDestroyHash(hHash);
    }
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return ok;
}

inline std::string MakeWsAccept(const std::string& key) {
    static const std::string MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + MAGIC;
    unsigned char digest[20];
    if (!SHA1Hash(combined, digest)) return "";
    return Base64Encode(digest, 20);
}

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string query;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> queryParams;
    std::string body;
    bool isWebSocket = false;
    std::string wsKey;

    std::string QueryParam(const std::string& key, const std::string& def = "") const {
        auto it = queryParams.find(key);
        return it != queryParams.end() ? it->second : def;
    }

    static bool StrictParseLL(const std::string& s, long long& out) {
        if (s.empty()) return false;
        errno = 0;
        char* end = nullptr;
        long long v = std::strtoll(s.c_str(), &end, 10);
        if (errno == ERANGE) return false;
        if (!end || end == s.c_str() || *end != '\0') return false;
        out = v;
        return true;
    }

    int QueryInt(const std::string& key, int def = 0) const {
        auto it = queryParams.find(key);
        if (it == queryParams.end()) return def;
        long long v = 0;
        if (!StrictParseLL(it->second, v)) return def;
        if (v < (long long)INT32_MIN) return INT32_MIN;
        if (v > (long long)INT32_MAX) return INT32_MAX;
        return (int)v;
    }
    long long QueryLL(const std::string& key, long long def = 0) const {
        auto it = queryParams.find(key);
        if (it == queryParams.end()) return def;
        long long v = 0;
        if (!StrictParseLL(it->second, v)) return def;
        return v;
    }
    double QueryFloat(const std::string& key, double def = 0.0) const {
        auto it = queryParams.find(key);
        if (it == queryParams.end()) return def;
        char* end = nullptr;
        errno = 0;
        double v = std::strtod(it->second.c_str(), &end);
        if (errno != 0 || !end || end == it->second.c_str() || *end != '\0') return def;
        return v;
    }

    std::string QueryString(const std::string& key, const std::string& def = "") const {
        auto it = queryParams.find(key);
        return it == queryParams.end() ? def : it->second;
    }
    int PathInt(const std::string& param, int def = 0) const {
        auto it = headers.find("x-path-" + param);
        if (it == headers.end()) return def;
        long long v = 0;
        if (!StrictParseLL(it->second, v)) return def;
        if (v < (long long)INT32_MIN) return INT32_MIN;
        if (v > (long long)INT32_MAX) return INT32_MAX;
        return (int)v;
    }
    long long PathLL(const std::string& param, long long def = 0) const {
        auto it = headers.find("x-path-" + param);
        if (it == headers.end()) return def;
        long long v = 0;
        if (!StrictParseLL(it->second, v)) return def;
        return v;
    }

    json BodyJSON() const {
        if (body.empty()) return json::object();
        try {

            return json::parse(body.begin(), body.end(),
                               nullptr,
                               true,
                               false);
        }
        catch (const std::exception& e) {
            throw std::invalid_argument(
                std::string("invalid JSON body: ") + e.what());
        }
    }

    bool TryBodyJSON(json& out, std::string& outErr) const {
        if (body.empty()) { out = json::object(); return true; }
        try {
            out = json::parse(body.begin(), body.end(),
                              nullptr,
                              true,
                              false);
            return true;
        }
        catch (const std::exception& e) {
            outErr = e.what();
            return false;
        }
    }
};

struct HTTPResponse {
    int         status = 200;
    std::string statusText = "OK";
    std::string contentType = "application/json";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string Serialize() const {

        auto& http = ElleConfig::Instance().GetHTTP();
        std::string allowOrigin;
        if (http.cors_enabled) {

            const std::string& list = http.cors_origins;
            if (list.empty() || list == "*") allowOrigin = "*";
            else {
                auto c = list.find(',');
                allowOrigin = (c == std::string::npos) ? list : list.substr(0, c);

                auto trim = [](std::string& s){
                    while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(0,1);
                    while (!s.empty() && std::isspace((unsigned char)s.back()))  s.pop_back();
                };
                trim(allowOrigin);
            }
        }

        std::ostringstream ss;
        ss << "HTTP/1.1 " << status << " " << statusText << "\r\n";
        ss << "Content-Type: " << contentType << "\r\n";
        ss << "Content-Length: " << body.size() << "\r\n";
        if (http.cors_enabled && !allowOrigin.empty()) {
            ss << "Access-Control-Allow-Origin: " << allowOrigin << "\r\n";
            ss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
            ss << "Access-Control-Allow-Headers: Content-Type, Authorization, x-admin-key\r\n";
        }
        for (auto& [k, v] : headers) ss << k << ": " << v << "\r\n";
        ss << "\r\n" << body;
        return ss.str();
    }

    static HTTPResponse JSON(int status, const std::string& body) {
        HTTPResponse r;
        r.status = status;
        r.statusText = (status >= 200 && status < 300) ? "OK"
                    : (status == 404 ? "Not Found"
                    : (status == 400 ? "Bad Request"
                    : (status == 401 ? "Unauthorized"
                    : (status == 504 ? "Gateway Timeout" : "Error"))));
        r.body = body;
        return r;
    }

    static HTTPResponse OK(const json& j) { return JSON(200, j.dump()); }
    static HTTPResponse Created(const json& j) { return JSON(201, j.dump()); }
    static HTTPResponse Accepted(const json& j) { return JSON(202, j.dump()); }
    static HTTPResponse Err(int code, const std::string& msg) {
        json j;
        j["error"] = msg;
        return JSON(code, j.dump());
    }

    static HTTPResponse Binary(const std::string& contentType,
                                std::string body) {
        HTTPResponse r;
        r.status = 200;
        r.statusText = "OK";
        r.contentType = contentType;
        r.body = std::move(body);
        return r;
    }
};

inline HTTPResponse InternalErrorResponse(const std::string& detail) {
    ELLE_ERROR("HTTP 500: %s", detail.c_str());
#if defined(_DEBUG)
    return HTTPResponse::Err(500, detail);
#else
    (void)detail;
    return HTTPResponse::Err(500, "internal server error");
#endif
}

typedef std::function<HTTPResponse(const HTTPRequest&)> RouteHandler;

enum HttpAuthLevel {
    AUTH_PUBLIC = 0,
    AUTH_USER,
    AUTH_ADMIN,
    AUTH_INTERNAL_ONLY
};

struct JwtVerifyResult {
    bool        valid       = false;
    std::string sub;
    uint64_t    exp_ms       = 0;
    std::string failureReason;
};

struct PairedCacheEntry {
    bool     revoked   = false;
    uint64_t cached_ms = 0;
    bool     exists    = false;
};
inline std::mutex                                     g_pairedCacheMx;
inline std::unordered_map<std::string, PairedCacheEntry> g_pairedCache;
inline constexpr uint64_t kPairedCacheTtlMs = 30ull * 1000ull;

inline std::pair<bool, bool>
PairedDeviceStatusCached(const std::string& device_id, uint64_t now_ms) {
    {
        std::lock_guard<std::mutex> lk(g_pairedCacheMx);
        auto it = g_pairedCache.find(device_id);
        if (it != g_pairedCache.end() &&
            now_ms - it->second.cached_ms < kPairedCacheTtlMs) {
            return { it->second.exists, it->second.revoked };
        }
    }
    ElleDB::PairedDeviceRow row;
    bool exists = ElleDB::GetPairedDevice(device_id, row);
    bool revoked = exists && row.revoked;
    {
        std::lock_guard<std::mutex> lk(g_pairedCacheMx);
        PairedCacheEntry e;
        e.exists    = exists;
        e.revoked   = revoked;
        e.cached_ms = now_ms;
        g_pairedCache[device_id] = e;

        if (g_pairedCache.size() > 4096) {
            std::vector<std::pair<std::string, uint64_t>> ordered;
            ordered.reserve(g_pairedCache.size());
            for (auto& kv : g_pairedCache) ordered.emplace_back(kv.first, kv.second.cached_ms);
            std::sort(ordered.begin(), ordered.end(),
                      [](const auto& a, const auto& b){ return a.second < b.second; });
            for (size_t i = 0; i < ordered.size() / 2; i++)
                g_pairedCache.erase(ordered[i].first);
        }
    }
    return { exists, revoked };
}

inline JwtVerifyResult VerifyJwtHs256(const std::string& token,
                                      const std::string& secret,
                                      uint64_t now_ms) {    JwtVerifyResult r;
    if (token.empty() || secret.empty()) {
        r.failureReason = "empty token or secret";
        return r;
    }

    size_t d1 = token.find('.');
    if (d1 == std::string::npos) { r.failureReason = "no dots"; return r; }
    size_t d2 = token.find('.', d1 + 1);
    if (d2 == std::string::npos) { r.failureReason = "one dot"; return r; }
    if (token.find('.', d2 + 1) != std::string::npos) {
        r.failureReason = "too many dots";
        return r;
    }
    const std::string h64 = token.substr(0, d1);
    const std::string p64 = token.substr(d1 + 1, d2 - d1 - 1);
    const std::string s64 = token.substr(d2 + 1);
    if (h64.empty() || p64.empty() || s64.empty()) {
        r.failureReason = "empty segment";
        return r;
    }

    auto headerBytes = ElleCrypto::Base64UrlDecode(h64);
    if (headerBytes.empty()) { r.failureReason = "header b64 invalid"; return r; }
    try {
        auto jh = nlohmann::json::parse(std::string(headerBytes.begin(),
                                                     headerBytes.end()));
        if (!jh.is_object()) { r.failureReason = "header not object"; return r; }

        std::string alg = jh.value("alg", "");
        if (alg != "HS256") { r.failureReason = "bad alg=" + alg; return r; }

        if (jh.contains("typ") && jh["typ"].is_string() &&
            jh["typ"].get<std::string>() != "JWT") {
            r.failureReason = "bad typ";
            return r;
        }
    } catch (const nlohmann::json::exception&) {
        r.failureReason = "header json parse";
        return r;
    }

    const std::string signingInput = h64 + "." + p64;
    uint8_t expected[32];
    if (!ElleCrypto::HmacSha256(secret.data(), secret.size(),
                                 signingInput.data(), signingInput.size(),
                                 expected)) {
        r.failureReason = "hmac compute failed";
        return r;
    }
    auto sigBytes = ElleCrypto::Base64UrlDecode(s64);
    if (sigBytes.size() != 32) {
        r.failureReason = "sig size != 32";
        return r;
    }
    if (!ElleCrypto::ConstantTimeEquals(expected, sigBytes.data(), 32)) {
        r.failureReason = "signature mismatch";
        return r;
    }

    auto payloadBytes = ElleCrypto::Base64UrlDecode(p64);
    if (payloadBytes.empty()) { r.failureReason = "payload b64 invalid"; return r; }
    try {
        auto jp = nlohmann::json::parse(std::string(payloadBytes.begin(),
                                                     payloadBytes.end()));
        if (!jp.is_object()) { r.failureReason = "payload not object"; return r; }
        if (!jp.contains("sub") || !jp["sub"].is_string()) {
            r.failureReason = "missing sub";
            return r;
        }
        if (!jp.contains("exp") || !jp["exp"].is_number_integer()) {
            r.failureReason = "missing exp";
            return r;
        }
        r.sub = jp["sub"].get<std::string>();
        int64_t exp = jp["exp"].get<int64_t>();
        if (exp <= 0) { r.failureReason = "non-positive exp"; return r; }
        r.exp_ms = (uint64_t)exp;
        if (r.exp_ms <= now_ms) {
            r.failureReason = "expired";
            return r;
        }
        if (r.sub.empty() || r.sub.size() > 128) {
            r.failureReason = "bad sub size";
            return r;
        }
    } catch (const nlohmann::json::exception&) {
        r.failureReason = "payload json parse";
        return r;
    }

    r.valid = true;
    return r;
}

struct RouteEntry {
    std::string   method;
    std::string   pattern;
    std::regex    re;
    std::vector<std::string> paramNames;
    RouteHandler  handler;
    HttpAuthLevel auth = AUTH_USER;
};

class RouteDispatch {
public:

    void Register(const std::string& method, const std::string& pattern,
                  RouteHandler h, HttpAuthLevel auth = AUTH_USER) {
        RouteEntry entry;
        entry.method  = method;
        entry.pattern = pattern;
        entry.auth    = auth;
        std::string regexStr;
        std::string token;
        bool inParam = false;
        for (char c : pattern) {
            if (c == '{') { inParam = true; token.clear(); continue; }
            if (c == '}') {
                inParam = false;
                entry.paramNames.push_back(token);
                regexStr += "([^/]+)";
                continue;
            }
            if (inParam) { token.push_back(c); continue; }
            if (c == '.' || c == '+' || c == '*' || c == '?' || c == '(' || c == ')' ||
                c == '[' || c == ']' || c == '^' || c == '$' || c == '\\' || c == '|') {
                regexStr.push_back('\\');
            }
            regexStr.push_back(c);
        }
        entry.re = std::regex("^" + regexStr + "$");
        entry.handler = std::move(h);
        m_routes.push_back(std::move(entry));
    }

    HTTPResponse Dispatch(HTTPRequest& req) {

        const auto& httpCfg = ElleConfig::Instance().GetHTTP();
        uint32_t rpm = httpCfg.rate_limit_rpm;
        if (rpm > 0) {
            uint64_t now = ELLE_MS_NOW();
            std::lock_guard<std::mutex> lock(m_rlMutex);
            if (now - m_rlWindowStart >= 60000) {
                m_rlWindowStart = now;
                m_rlCount = 0;
            }
            if (m_rlCount >= rpm) {
                return HTTPResponse::Err(429, "rate limit exceeded");
            }
            m_rlCount++;
        }

        RouteEntry* matched = nullptr;
        std::smatch mm;
        for (auto& e : m_routes) {
            if (e.method != req.method) continue;
            if (std::regex_match(req.path, mm, e.re)) { matched = &e; break; }
        }

        if (req.method == "OPTIONS") return HTTPResponse::OK(json::object());

        const bool noAuth = (ElleConfig::Instance().GetInt("http_server.no_auth", 0) != 0);
        if (matched && matched->auth != AUTH_PUBLIC) {

            if (noAuth) {
                req.headers["x-auth-nuserno"]   = "1";
                req.headers["x-auth-user-id"]   = "single";
                req.headers["x-auth-user-name"] = "Single";
                req.headers["x-auth-id-level"]  = "9";
                req.headers["x-auth-device-id"] = "single";
            }
        }

        if (!noAuth && httpCfg.auth_enabled && matched && matched->auth != AUTH_PUBLIC) {

            HttpAuthLevel need = matched->auth;
            const int adminThreshold = (int)ElleConfig::Instance().GetInt(
                "http_server.admin_auth_id_threshold", 9);

            std::string token;
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
                    token = v.substr(kBearer.size());
                    while (!token.empty() && (token.front() == ' ' ||
                                              token.front() == '\t'))
                        token.erase(0, 1);
                }
            }

            if (need == AUTH_INTERNAL_ONLY) {
                auto peerIt = req.headers.find("x-peer-addr");
                std::string peer = (peerIt != req.headers.end())
                                   ? peerIt->second : "";
                bool isLoop = (peer == "127.0.0.1" || peer == "::1" ||
                               peer.rfind("127.", 0) == 0);
                if (!isLoop) {
                    return HTTPResponse::Err(403, "internal-only route");
                }
            }

            if (token.empty()) {
                return HTTPResponse::Err(401, "authentication required");
            }

            ElleDB::SessionRow sess;
            if (!ElleDB::GetSessionByToken(token, sess)) {
                return HTTPResponse::Err(401, "invalid or expired session");
            }

            if ((need == AUTH_ADMIN || need == AUTH_INTERNAL_ONLY) &&
                sess.nAuthID < adminThreshold) {
                ELLE_INFO("admin-gate refused nUserNo=%lld nAuthID=%d "
                          "(threshold=%d) for %s %s",
                          (long long)sess.nUserNo, sess.nAuthID,
                          adminThreshold, req.method.c_str(), req.path.c_str());
                return HTTPResponse::Err(403, "insufficient privilege");
            }

            ElleDB::TouchSessionLastSeen(token);

            {
                auto dit = req.headers.find("x-client-device-id");
                if (dit != req.headers.end() && !dit->second.empty()) {
                    ElleDB::TouchPairedDeviceLastSeen(dit->second);
                }
            }

            req.headers["x-auth-nuserno"]   = std::to_string(sess.nUserNo);
            req.headers["x-auth-user-id"]   = sess.sUserID;
            req.headers["x-auth-user-name"] = sess.sUserName;
            req.headers["x-auth-id-level"]  = std::to_string(sess.nAuthID);
            req.headers["x-auth-device-id"] = sess.token;
        }

        if (matched) {
            for (size_t i = 0; i < matched->paramNames.size() && i + 1 < mm.size(); i++) {
                req.headers["x-path-" + matched->paramNames[i]] = mm[i + 1].str();
            }
            return matched->handler(req);
        }
        return HTTPResponse::Err(404, "Not found: " + req.method + " " + req.path);
    }

    size_t Count() const { return m_routes.size(); }
    const std::vector<RouteEntry>& AllRoutes() const { return m_routes; }

private:
    std::vector<RouteEntry> m_routes;

    std::mutex m_rlMutex;
    uint64_t   m_rlWindowStart = 0;
    uint32_t   m_rlCount = 0;
};

struct WSClient {
    SOCKET      socket = INVALID_SOCKET;
    std::string id;
    bool        alive = true;
    uint64_t    connected_ms = 0;
    std::mutex  sendMutex;
};

inline bool GetIntHeader(const HTTPRequest& req, const std::string& key,
                                long long& out) {
    auto it = req.headers.find(key);
    if (it == req.headers.end() || it->second.empty()) return false;
    char* end = nullptr;
    long long v = std::strtoll(it->second.c_str(), &end, 10);
    if (!end || end == it->second.c_str() || *end != '\0') return false;
    out = v;
    return true;
}

struct LLMMsg { std::string role; std::string content; };

inline bool CallGroqDirect(const std::vector<LLMMsg>& messages,
                           std::string& outResponse,
                           std::string& outError)
{
    auto& cfg = ElleConfig::Instance().GetLLM();
    const LLMProviderConfig* groq = nullptr;
    auto it = cfg.providers.find("groq");
    if (it != cfg.providers.end() && it->second.enabled) {
        groq = &it->second;
    }
    if (!groq) { outError = "No Groq provider configured"; return false; }

    json msgArr = json::array();
    for (auto& m : messages) {
        msgArr.push_back({{"role", m.role}, {"content", m.content}});
    }
    json body = {
        {"model", groq->model.empty() ? "llama-3.3-70b-versatile" : groq->model},
        {"messages", msgArr},
        {"temperature", 0.85},
        {"max_tokens", 2048},
        {"stream", false}
    };
    std::string bodyStr = body.dump();

    std::string host = "api.groq.com";
    std::string path = "/openai/v1/chat/completions";
    if (!groq->api_url.empty()) {
        std::string url = groq->api_url;
        if (url.rfind("https://", 0) == 0) url = url.substr(8);
        else if (url.rfind("http://", 0) == 0) url = url.substr(7);
        size_t slash = url.find('/');
        if (slash != std::string::npos) {
            host = url.substr(0, slash);
            path = url.substr(slash);
            if (path.find("/chat/completions") == std::string::npos) path += "/chat/completions";
        } else {
            host = url;
        }
    }

    auto toWide = [](const std::string& s) -> std::wstring {
        int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring w(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], len);
        return w;
    };
    std::wstring wHost = toWide(host);
    std::wstring wPath = toWide(path);

    HINTERNET hSession = WinHttpOpen(L"Elle-Ann/3.0",
                                      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { outError = "WinHttpOpen failed"; return false; }

    DWORD dwTimeout = 60000;
    WinHttpSetTimeouts(hSession, dwTimeout, dwTimeout, dwTimeout, dwTimeout);

    HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(),
                                         INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); outError = "WinHttpConnect failed"; return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wPath.c_str(),
                                             nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES,
                                             WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); outError = "WinHttpOpenRequest failed"; return false; }

    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"Authorization: Bearer " + toWide(groq->api_key) + L"\r\n";

    BOOL sent = WinHttpSendRequest(hRequest,
                                    headers.c_str(), (DWORD)-1,
                                    (LPVOID)bodyStr.data(), (DWORD)bodyStr.size(),
                                    (DWORD)bodyStr.size(), 0);
    if (!sent) {
        outError = "WinHttpSendRequest failed: " + std::to_string(GetLastError());
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        outError = "WinHttpReceiveResponse failed: " + std::to_string(GetLastError());
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    std::string responseStr;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        std::string chunk(dwSize, 0);
        DWORD dwRead = 0;
        if (!WinHttpReadData(hRequest, &chunk[0], dwSize, &dwRead)) break;
        responseStr.append(chunk.data(), dwRead);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    try {
        json resp = json::parse(responseStr);
        if (resp.contains("error")) {
            outError = resp["error"].dump();
            return false;
        }
        if (resp.contains("choices") && resp["choices"].is_array() && !resp["choices"].empty()) {
            auto& msg = resp["choices"][0]["message"];
            if (msg.contains("content")) {
                outResponse = msg["content"].get<std::string>();
                return true;
            }
        }
        outError = "Unexpected Groq response shape";
        return false;
    } catch (const std::exception& e) {
        outError = std::string("JSON parse error: ") + e.what() +
                   " | raw: " + responseStr.substr(0, 200);
        return false;
    }
}

inline bool WsSendText(SOCKET s, std::mutex& mtx, const std::string& payload) {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<unsigned char> frame;
    frame.push_back(0x81);
    size_t len = payload.size();
    if (len < 126) {
        frame.push_back((unsigned char)len);
    } else if (len < 65536) {
        frame.push_back(126);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) frame.push_back((len >> (i * 8)) & 0xFF);
    }
    frame.insert(frame.end(), payload.begin(), payload.end());
    int total = 0;
    int remaining = (int)frame.size();
    while (remaining > 0) {
        int sent = send(s, (const char*)frame.data() + total, remaining, 0);
        if (sent == SOCKET_ERROR) return false;
        total += sent;
        remaining -= sent;
    }
    return true;
}

enum class WsFrameStatus {
    Ok,
    CleanClose,
    NetworkError,
    ProtocolViolation
};

inline WsFrameStatus WsReadFrameStatus(SOCKET s, std::string& outPayload, int& outOpcode) {
    unsigned char hdr[2];
    int r = recv(s, (char*)hdr, 2, MSG_WAITALL);
    if (r == 0)               return WsFrameStatus::CleanClose;
    if (r == SOCKET_ERROR)    return WsFrameStatus::NetworkError;
    if (r != 2)               return WsFrameStatus::NetworkError;

    bool fin = (hdr[0] & 0x80) != 0;
    (void)fin;
    outOpcode = hdr[0] & 0x0F;
    bool masked = (hdr[1] & 0x80) != 0;
    if (!masked) return WsFrameStatus::ProtocolViolation;

    uint64_t payloadLen = hdr[1] & 0x7F;
    if (payloadLen == 126) {
        unsigned char ext[2];
        int er = recv(s, (char*)ext, 2, MSG_WAITALL);
        if (er == 0)            return WsFrameStatus::CleanClose;
        if (er != 2)            return WsFrameStatus::NetworkError;
        payloadLen = ((uint64_t)ext[0] << 8) | ext[1];

        if (payloadLen < 126)   return WsFrameStatus::ProtocolViolation;
    } else if (payloadLen == 127) {
        unsigned char ext[8];
        int er = recv(s, (char*)ext, 8, MSG_WAITALL);
        if (er == 0)            return WsFrameStatus::CleanClose;
        if (er != 8)            return WsFrameStatus::NetworkError;
        payloadLen = 0;
        for (int i = 0; i < 8; i++) payloadLen = (payloadLen << 8) | ext[i];

        if (payloadLen & (1ULL << 63)) return WsFrameStatus::ProtocolViolation;
        if (payloadLen <= 0xFFFF)      return WsFrameStatus::ProtocolViolation;
    }

    uint64_t maxFrame = (uint64_t)ElleConfig::Instance().GetHTTP().max_ws_frame_bytes;
    if (maxFrame == 0) maxFrame = 1ULL * 1024 * 1024;
    if (payloadLen > maxFrame) return WsFrameStatus::ProtocolViolation;

    if ((outOpcode & 0x08) && (payloadLen > 125 || !fin))
        return WsFrameStatus::ProtocolViolation;

    unsigned char mask[4] = {0};
    int mr = recv(s, (char*)mask, 4, MSG_WAITALL);
    if (mr == 0)  return WsFrameStatus::CleanClose;
    if (mr != 4)  return WsFrameStatus::NetworkError;

    outPayload.resize((size_t)payloadLen);
    if (payloadLen > 0) {
        int pr = recv(s, &outPayload[0], (int)payloadLen, MSG_WAITALL);
        if (pr == 0)               return WsFrameStatus::CleanClose;
        if (pr != (int)payloadLen) return WsFrameStatus::NetworkError;
        for (size_t i = 0; i < payloadLen; i++) {
            outPayload[i] = outPayload[i] ^ mask[i % 4];
        }
    }
    return WsFrameStatus::Ok;
}

inline bool WsReadFrame(SOCKET s, std::string& outPayload, int& outOpcode) {
    return WsReadFrameStatus(s, outPayload, outOpcode) == WsFrameStatus::Ok;
}

inline bool IsValidUtf8(const std::string& s) {
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80)      { i += 1; continue; }
        int len;
        uint32_t cp;
        if ((c & 0xE0) == 0xC0) { len = 2; cp = c & 0x1F; }
        else if ((c & 0xF0) == 0xE0) { len = 3; cp = c & 0x0F; }
        else if ((c & 0xF8) == 0xF0) { len = 4; cp = c & 0x07; }
        else return false;
        if (i + len > n) return false;
        for (int k = 1; k < len; k++) {
            unsigned char cc = (unsigned char)s[i + k];
            if ((cc & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (cc & 0x3F);
        }

        if (len == 2 && cp < 0x80)    return false;
        if (len == 3 && cp < 0x800)   return false;
        if (len == 4 && cp < 0x10000) return false;

        if (cp >= 0xD800 && cp <= 0xDFFF) return false;

        if (cp > 0x10FFFF) return false;
        i += len;
    }
    return true;
}

struct PendingChat {
    std::mutex              m;
    std::condition_variable cv;
    bool                    done = false;
    json                    result;
};

class ChatCorrelator {
public:
    std::shared_ptr<PendingChat> Register(const std::string& requestId) {
        auto p = std::make_shared<PendingChat>();
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map[requestId] = p;
        return p;
    }
    void Complete(const std::string& requestId, const json& result) {
        std::shared_ptr<PendingChat> p;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_map.find(requestId);
            if (it == m_map.end()) return;
            p = it->second;
            m_map.erase(it);
        }
        {
            std::lock_guard<std::mutex> lk(p->m);
            p->result = result;
            p->done = true;
        }
        p->cv.notify_all();
    }
    void Cancel(const std::string& requestId) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_map.erase(requestId);
    }
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<PendingChat>> m_map;
};

class ElleHTTPService : public ElleServiceBase {
public:
    ElleHTTPService()
        : ElleServiceBase(SVC_HTTP_SERVER, "ElleHTTPServer",
                          "Elle-Ann HTTP/WebSocket Server",
                          "REST API + WebSocket command server") {}

protected:
    bool OnStart() override;

    void OnStop() override;

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override;

    std::vector<ELLE_SERVICE_ID> GetDependencies() override;

private:
    SOCKET m_listenSocket = INVALID_SOCKET;
    std::thread m_acceptThread;
    RouteDispatch m_router;

    std::vector<std::shared_ptr<WSClient>> m_wsClients;
    std::mutex m_wsMutex;
    std::atomic<uint64_t> m_requestSeq{0};
    ChatCorrelator m_chatCorrelator;

    std::atomic<bool> m_shuttingDown{false};

    std::mutex                  m_socketMx;
    std::condition_variable     m_socketCv;
    std::deque<SOCKET>          m_socketQueue;
    std::vector<std::thread>    m_httpWorkers;

    std::mutex                         m_wsThreadsMx;
    std::vector<std::thread>           m_wsThreads;
    std::set<std::thread::id>          m_reapableWsThreadIds;

    uint32_t MaxConcurrent() const;

    ELLE_EMOTION_STATE m_cachedEmotions = {};

    struct PairingCode {
        uint64_t expires_ms = 0;
        uint64_t issued_ms  = 0;
        bool     consumed   = false;
    };
    std::mutex                                      m_pairingMutex;
    std::unordered_map<std::string, PairingCode>    m_pairingCodes;

    void PairingGcLocked(uint64_t now);

    struct LoginFailRecord {
        uint32_t fail_count    = 0;
        uint64_t window_start  = 0;
        uint64_t lockout_until = 0;
    };
    static constexpr uint32_t kLoginMaxFails       = 5;
    static constexpr uint64_t kLoginWindowMs       = 15ull * 60 * 1000;
    static constexpr uint64_t kLoginLockoutMs      = 15ull * 60 * 1000;
    static constexpr uint64_t kLoginRecordTtlMs    = 60ull * 60 * 1000;
    static constexpr size_t   kMaxLoginKeys        = 4096;

    std::mutex                                          m_loginMutex;
    std::unordered_map<std::string, LoginFailRecord>    m_loginFails;

    static std::string LoginKey(const std::string& ip, const std::string& user);

    uint64_t LoginCheckLockedLocked(const std::string& key, uint64_t now);

    void LoginRecordFailLocked(const std::string& key, uint64_t now);

    void LoginRecordSuccessLocked(const std::string& key);

    void LoginGcLocked(uint64_t now);

    static std::string MintJwt(const std::string& deviceId,
                               const std::string& deviceName,
                               uint64_t iat_ms, uint64_t exp_ms,
                               const std::string& secret);

    void AcceptLoop();

    void HttpWorkerLoop();

    void HandleClient(SOCKET clientSocket);

    void SendResponse(SOCKET clientSocket, const HTTPResponse& resp);

    HTTPRequest ParseRequest(const std::string& raw);

    static void ParseQueryString(const std::string& q,
                                  std::unordered_map<std::string, std::string>& out);

    static std::string UrlDecode(const std::string& s);

    void HandleWebSocketUpgrade(SOCKET clientSocket, const HTTPRequest& req);

    void WebSocketReadLoop(std::shared_ptr<WSClient> client);

    void HandleWebSocketMessage(std::shared_ptr<WSClient> client, const std::string& payload);

    void BroadcastIPCToWebSockets(const ElleIPCMessage& msg);

    static int32_t ResolveAuthenticatedUser(const HTTPRequest& req);

    static std::optional<HTTPResponse> RequireUserId(const json& body, int32_t& out);

    static std::optional<HTTPResponse> RequireAuthOrBodyUser(const HTTPRequest& req,
                                                             const json& body,
                                                             int32_t& out);

    void RegisterRoutes();

    void RegisterIntroRoutes();

    void RegisterAuthRoutes();

    void RegisterDiagRoutes();

    void RegisterAdminRoutes();

    void RegisterMemoryRoutes();

    void RegisterEmotionRoutes();

    void RegisterMeTokensRoutes();

    void RegisterVideoIdentityRoutes();

    void RegisterAIRoutes();

    void RegisterDictionaryRoutes();

    void RegisterEducationRoutes();

    void RegisterEmotionalContextRoutes();

    void RegisterXLifecycleRoutes();

    void RegisterServerRoutes();

    void RegisterModelsRoutes();

    void RegisterMoralsGoalsRoutes();

    void RegisterMiscRoutes();

    void RegisterSHNRoutes();

};
