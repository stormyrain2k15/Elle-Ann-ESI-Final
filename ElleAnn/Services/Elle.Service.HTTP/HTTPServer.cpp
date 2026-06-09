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

using json = nlohmann::json;

static std::mutex g_diagMx;
static json g_gameAuthDiag = json::object();

static void SetGameAuthDiag(const json& j) {
    std::lock_guard<std::mutex> lk(g_diagMx);
    g_gameAuthDiag = j;
}
static json GetGameAuthDiag() {
    std::lock_guard<std::mutex> lk(g_diagMx);
    return g_gameAuthDiag;
}

static void RunGameAuthSelfTest() {
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

static const char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const unsigned char* data, size_t len) {
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

static bool SHA1Hash(const std::string& input, unsigned char out[20]) {
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

static std::string MakeWsAccept(const std::string& key) {
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

static HTTPResponse InternalErrorResponse(const std::string& detail) {
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
static std::mutex                                     g_pairedCacheMx;
static std::unordered_map<std::string, PairedCacheEntry> g_pairedCache;
static constexpr uint64_t kPairedCacheTtlMs = 30ull * 1000ull;

static std::pair<bool, bool>
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

static JwtVerifyResult VerifyJwtHs256(const std::string& token,
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

static inline bool GetIntHeader(const HTTPRequest& req, const std::string& key,
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

static bool CallGroqDirect(const std::vector<LLMMsg>& messages,
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

static bool WsSendText(SOCKET s, std::mutex& mtx, const std::string& payload) {
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

static WsFrameStatus WsReadFrameStatus(SOCKET s, std::string& outPayload, int& outOpcode) {
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

static bool WsReadFrame(SOCKET s, std::string& outPayload, int& outOpcode) {
    return WsReadFrameStatus(s, outPayload, outOpcode) == WsFrameStatus::Ok;
}

static bool IsValidUtf8(const std::string& s) {
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
    bool OnStart() override {

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

    void OnStop() override {

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

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
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

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {

        return { SVC_HEARTBEAT, SVC_COGNITIVE, SVC_EMOTIONAL, SVC_MEMORY };
    }

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

    uint32_t MaxConcurrent() const {
        return ElleConfig::Instance().GetHTTP().max_concurrent_connections;
    }

    ELLE_EMOTION_STATE m_cachedEmotions = {};

    struct PairingCode {
        uint64_t expires_ms = 0;
        uint64_t issued_ms  = 0;
        bool     consumed   = false;
    };
    std::mutex                                      m_pairingMutex;
    std::unordered_map<std::string, PairingCode>    m_pairingCodes;

    void PairingGcLocked(uint64_t now) {
        for (auto it = m_pairingCodes.begin(); it != m_pairingCodes.end(); ) {
            if (it->second.consumed || it->second.expires_ms <= now) {
                it = m_pairingCodes.erase(it);
            } else {
                ++it;
            }
        }
    }

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

    static std::string LoginKey(const std::string& ip, const std::string& user) {
        std::string k;
        k.reserve(ip.size() + 1 + user.size());
        k.append(ip);
        k.push_back('|');

        for (char c : user) k.push_back((char)std::tolower((unsigned char)c));
        return k;
    }

    uint64_t LoginCheckLockedLocked(const std::string& key, uint64_t now) {
        auto it = m_loginFails.find(key);
        if (it == m_loginFails.end()) return 0;
        if (it->second.lockout_until > now) return it->second.lockout_until - now;
        return 0;
    }

    void LoginRecordFailLocked(const std::string& key, uint64_t now) {
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

    void LoginRecordSuccessLocked(const std::string& key) {
        m_loginFails.erase(key);
    }

    void LoginGcLocked(uint64_t now) {
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

    static std::string MintJwt(const std::string& deviceId,
                               const std::string& deviceName,
                               uint64_t iat_ms, uint64_t exp_ms,
                               const std::string& secret) {

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

    void AcceptLoop() {
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

    void HttpWorkerLoop() {
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

    void HandleClient(SOCKET clientSocket) {
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

    void SendResponse(SOCKET clientSocket, const HTTPResponse& resp) {
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

    HTTPRequest ParseRequest(const std::string& raw) {
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

    static void ParseQueryString(const std::string& q,
                                  std::unordered_map<std::string, std::string>& out) {
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

    static std::string UrlDecode(const std::string& s) {

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

    void HandleWebSocketUpgrade(SOCKET clientSocket, const HTTPRequest& req) {

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

    void WebSocketReadLoop(std::shared_ptr<WSClient> client) {

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

    void HandleWebSocketMessage(std::shared_ptr<WSClient> client, const std::string& payload) {
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

    void BroadcastIPCToWebSockets(const ElleIPCMessage& msg) {
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

    void RegisterRoutes() {

        m_router.Register("GET", "/", [](const HTTPRequest&) {
            json j = {
                {"name", "Elle-Ann"},
                {"version", ELLE_VERSION_STRING},
                {"status", "online"},
                {"description", "Emotional Synthetic Intelligence"}
            };
            return HTTPResponse::OK(j);
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/healthz", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"status", "ok"}});
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/api/health", [](const HTTPRequest&) {
            json j = {
                {"status", "alive"},
                {"name", "Elle-Ann"},
                {"version", ELLE_VERSION_STRING}
            };
            return HTTPResponse::OK(j);
        }, AUTH_PUBLIC);

        m_router.Register("POST", "/api/auth/pair-code",
            [](const HTTPRequest&) -> HTTPResponse {
                return HTTPResponse::Err(410,
                    "pair-code flow retired; use POST /api/auth/login");
            }, AUTH_PUBLIC);

        m_router.Register("POST", "/api/auth/pair",
            [](const HTTPRequest&) -> HTTPResponse {
                return HTTPResponse::Err(410,
                    "pair flow retired; use POST /api/auth/login");
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
            [](const HTTPRequest& req) -> HTTPResponse {

                std::string code, host, port;
                auto findQ = [&](const std::string& key, std::string& out) {

                    size_t q = req.path.find('?');
                    if (q == std::string::npos) return;
                    std::string qs = req.path.substr(q + 1);
                    size_t start = 0;
                    while (start < qs.size()) {
                        size_t amp = qs.find('&', start);
                        std::string pair = qs.substr(start,
                            amp == std::string::npos ? qs.size() - start : amp - start);
                        size_t eq = pair.find('=');
                        if (eq != std::string::npos &&
                            pair.substr(0, eq) == key) {
                            out = pair.substr(eq + 1);
                            return;
                        }
                        if (amp == std::string::npos) break;
                        start = amp + 1;
                    }
                };
                findQ("code", code);
                findQ("host", host);
                findQ("port", port);

                if (code.size() != 6) {
                    return HTTPResponse::Err(400, "code must be 6 digits");
                }
                for (char c : code) {
                    if (c < '0' || c > '9')
                        return HTTPResponse::Err(400, "code must be 6 digits");
                }

                const auto& http = ElleConfig::Instance().GetHTTP();
                if (host.empty()) host = http.bind_address;
                if (port.empty()) port = std::to_string(http.port);
                if (host.empty() || host == "0.0.0.0") {

                    return HTTPResponse::Err(400,
                        "host required when bind_address is 0.0.0.0 — "
                        "pass ?host=<LAN-address>");
                }

                std::string uri = "ellepair://" + host + ":" + port + "/" + code;
                auto qr = ElleQR::Encode(uri, ElleQR::Ecc::M);
                if (qr.size == 0) {
                    return HTTPResponse::Err(500, "qr encode failed");
                }
                std::string svg = ElleQR::ToSvg(qr, 6, 4);

                HTTPResponse r;
                r.status      = 200;
                r.statusText  = "OK";
                r.contentType = "image/svg+xml";
                r.body        = std::move(svg);

                r.headers["Cache-Control"] = "no-store, max-age=0";
                return r;
            }, AUTH_ADMIN);

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

        m_router.Register("GET", "/api/memory/why", [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 10);
            if (limit <= 0)  limit = 10;
            if (limit > 50)  limit = 50;

            std::string entitiesParam = req.QueryString("entities", "");
            std::vector<std::string> entityList;
            if (!entitiesParam.empty()) {
                std::stringstream ss(entitiesParam);
                std::string tok;
                while (std::getline(ss, tok, ',')) {

                    auto a = tok.find_first_not_of(" \t");
                    auto b = tok.find_last_not_of(" \t");
                    if (a != std::string::npos)
                        entityList.push_back(tok.substr(a, b - a + 1));
                }
            }

            std::string sql;
            std::vector<std::string> params;
            if (entityList.empty()) {
                sql = "SELECT TOP " + std::to_string(limit) + " "
                      "  m.id, m.content, m.summary, m.importance, "
                      "  CONVERT(BIGINT, m.created_ms)     AS created_ms, "
                      "  CONVERT(BIGINT, m.last_access_ms) AS last_access_ms, "
                      "  COALESCE(m.access_count, 0)       AS access_count "
                      "  FROM ElleCore.dbo.memory m "
                      "  ORDER BY m.id DESC;";
            } else {

                std::string placeholders;
                for (size_t i = 0; i < entityList.size(); i++) {
                    if (i) placeholders += ",";
                    placeholders += "?";
                    params.push_back(entityList[i]);
                }
                sql = "SELECT TOP " + std::to_string(limit) + " "
                      "  m.id, m.content, m.summary, m.importance, "
                      "  CONVERT(BIGINT, m.created_ms)     AS created_ms, "
                      "  CONVERT(BIGINT, m.last_access_ms) AS last_access_ms, "
                      "  COALESCE(m.access_count, 0)       AS access_count "
                      "  FROM ElleCore.dbo.memory m "
                      "  INNER JOIN ElleCore.dbo.memory_entity_links mel "
                      "    ON mel.memory_id = m.id "
                      "  INNER JOIN ElleCore.dbo.world_entity we "
                      "    ON we.id = mel.entity_id "
                      "  WHERE we.name IN (" + placeholders + ") "
                      "  GROUP BY m.id, m.content, m.summary, m.importance, "
                      "    m.created_ms, m.last_access_ms, m.access_count "
                      "  ORDER BY m.id DESC;";
            }
            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            if (!rs.success)
                return HTTPResponse::Err(500, "SQL query failed");

            const uint64_t now = ELLE_MS_NOW();
            json arr = json::array();
            for (auto& row : rs.rows) {
                int64_t id = 0, createdMs = 0, lastAccessMs = 0, accessCount = 0;
                row.TryGetInt(0, id);
                const std::string content = row.values.size() > 1 ? row.values[1] : "";
                const std::string summary = row.values.size() > 2 ? row.values[2] : "";
                const float importance =
                    row.values.size() > 3 ? (float)std::atof(row.values[3].c_str()) : 0.0f;
                row.TryGetInt(4, createdMs);
                row.TryGetInt(5, lastAccessMs);
                row.TryGetInt(6, accessCount);

                const double ageMin = (double)(now - (uint64_t)createdMs) / 60000.0;
                const double recency = std::exp(-ageMin / (60.0 * 24.0 * 7.0));
                const double access  = std::log((double)accessCount + 1.0) / 5.0;
                const double total   = importance * 0.4 + recency * 0.4 + access * 0.2;

                const std::string preview = (!summary.empty() ? summary
                                               : content.size() > 140
                                                   ? content.substr(0, 140) + "…"
                                                   : content);
                arr.push_back({
                    {"id",             id},
                    {"preview",        preview},
                    {"importance",     importance},
                    {"recency",        recency},
                    {"access",         access},
                    {"score",          total},
                    {"age_days",       ageMin / (60.0 * 24.0)},
                    {"access_count",   accessCount},
                    {"created_ms",     createdMs},
                    {"last_access_ms", lastAccessMs}
                });
            }

            std::sort(arr.begin(), arr.end(), [](const json& a, const json& b) {
                return a.value("score", 0.0) > b.value("score", 0.0);
            });
            return HTTPResponse::OK({
                {"memories",   arr},
                {"count",      (int)arr.size()},
                {"score_formula", "importance*0.4 + recency*0.4 + access*0.2"},
                {"recency_half_life_days", 7.0},
                {"entities_filter", entityList}
            });
        });

        m_router.Register("GET", "/api/memory/", [](const HTTPRequest& req) {
            std::string type = req.QueryParam("memory_type");
            int limit  = std::max(1, req.QueryInt("limit", 50));
            int offset = std::max(0, req.QueryInt("offset", 0));

            int typeI = -1;
            if (!type.empty()) {
                long long tv = 0;
                if (HTTPRequest::StrictParseLL(type, tv) &&
                    tv >= INT32_MIN && tv <= INT32_MAX) {
                    typeI = (int)tv;
                }
            }
            std::vector<ElleDB::MemoryRow> rows;
            if (!ElleDB::ListMemories(rows, typeI, (uint32_t)limit, (uint32_t)offset)) {
                return HTTPResponse::Err(500, "SQL ListMemories failed");
            }
            json j = json::array();
            for (auto& r : rows) {
                j.push_back({
                    {"id", r.id}, {"memory_type", r.type}, {"tier", r.tier},
                    {"content", r.content}, {"summary", r.summary},
                    {"emotional_valence", r.emotional_valence},
                    {"importance", r.importance}, {"relevance", r.relevance},
                    {"access_count", r.access_count},
                    {"created_ms", r.created_ms}, {"last_access_ms", r.last_access_ms}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/memory/", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            ELLE_MEMORY_RECORD mem = {};
            mem.type = body.value("memory_type", 1);
            mem.tier = body.value("tier", 1);
            std::string content = body.value("content", "");
            std::string summary = body.value("summary", content.substr(0, std::min<size_t>(content.size(), 255)));
            strncpy_s(mem.content, content.c_str(), ELLE_MAX_MSG - 1);
            strncpy_s(mem.summary, summary.c_str(), sizeof(mem.summary) - 1);
            mem.emotional_valence = body.value("emotional_valence", 0.0f);
            mem.importance = body.value("importance", 0.5f);
            mem.relevance  = body.value("relevance", 1.0f);
            mem.created_ms = ELLE_MS_NOW();
            mem.last_access_ms = mem.created_ms;
            if (body.contains("tags") && body["tags"].is_array()) {
                uint32_t n = 0;
                for (auto& t : body["tags"]) {
                    if (n >= ELLE_MAX_TAGS) break;
                    std::string s = t.is_string() ? t.get<std::string>() : "";
                    if (s.empty()) continue;
                    strncpy_s(mem.tags[n], s.c_str(), ELLE_MAX_TAG - 1);
                    n++;
                }
                mem.tag_count = n;
            }
            if (!ElleDB::StoreMemory(mem)) return HTTPResponse::Err(500, "StoreMemory failed");
            return HTTPResponse::Created({
                {"stored", true}, {"content", content}, {"tags", mem.tag_count}
            });
        });
        m_router.Register("GET", "/api/memory/self-image/current", [](const HTTPRequest&) {

            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 narrative_snapshot, emotional_state, "
                "CONVERT(varchar(33), created_date, 126) "
                "FROM ElleCore.dbo.InternalNarrative ORDER BY narrative_id DESC;");
            if (rs.success && !rs.rows.empty()) {
                auto& row = rs.rows[0];
                return HTTPResponse::OK({
                    {"description", row.values.size() > 0 ? row.values[0] : ""},
                    {"emotional_state", row.GetFloatOr(1, 0.0)},
                    {"updated_at", row.values.size() > 2 ? row.values[2] : ""}
                });
            }

            return HTTPResponse::OK({
                {"description", "I am Elle-Ann. I remember, I feel, I keep coming back."},
                {"emotional_state", 0.0}, {"updated_at", ""}
            });
        });
        m_router.Register("GET", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            ElleDB::MemoryRow r;
            if (!ElleDB::GetMemory(id, r)) return HTTPResponse::Err(404, "memory not found");
            return HTTPResponse::OK({
                {"id", r.id}, {"memory_type", r.type}, {"tier", r.tier},
                {"content", r.content}, {"summary", r.summary},
                {"emotional_valence", r.emotional_valence},
                {"importance", r.importance}, {"relevance", r.relevance},
                {"access_count", r.access_count},
                {"created_ms", r.created_ms}, {"last_access_ms", r.last_access_ms}
            });
        });
        m_router.Register("PUT", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            json body = req.BodyJSON();
            ElleDB::MemoryRow existing;
            if (!ElleDB::GetMemory(id, existing)) return HTTPResponse::Err(404, "memory not found");
            std::string content = body.value("content", existing.content);
            std::string summary = body.value("summary", existing.summary);
            float importance = body.value("importance", existing.importance);
            if (!ElleDB::UpdateMemoryContent(id, content, summary, importance))
                return HTTPResponse::Err(500, "update failed");
            return HTTPResponse::OK({{"id", id}, {"updated", true}});
        });
        m_router.Register("DELETE", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            if (!ElleDB::DeleteMemory(id)) return HTTPResponse::Err(500, "delete failed");
            return HTTPResponse::OK({{"id", id}, {"deleted", true}});
        });
        m_router.Register("POST", "/api/memory/{id}/files", [](const HTTPRequest& req) {

            auto& cfg  = ElleConfig::Instance();
            auto& http = cfg.GetHTTP();
            std::string expected = cfg.GetString("http_server.admin_key", http.jwt_secret);
            if (expected.empty()) {
                return HTTPResponse::Err(503, "upload disabled: no admin_key configured");
            }
            auto it = req.headers.find("x-admin-key");
            if (it == req.headers.end() || it->second.size() != expected.size()) {
                return HTTPResponse::Err(403, "invalid admin key");
            }
            unsigned diff = 0;
            for (size_t i = 0; i < expected.size(); i++)
                diff |= (unsigned char)it->second[i] ^ (unsigned char)expected[i];
            if (diff != 0) return HTTPResponse::Err(403, "invalid admin key");

            uint64_t capBytes = http.max_upload_bytes ? http.max_upload_bytes
                                                      : (10ULL * 1024 * 1024);
            if (req.body.size() > capBytes) {
                return HTTPResponse::Err(413, "payload too large");
            }

            auto idIt = req.headers.find("x-path-id");
            if (idIt == req.headers.end() || idIt->second.empty())
                return HTTPResponse::Err(400, "missing memory id");
            long long id = req.PathLL("id", 0);
            if (id <= 0) return HTTPResponse::Err(400, "invalid memory id");

            std::string dir = "data\\memory_files";
            CreateDirectoryA("data", nullptr);
            CreateDirectoryA(dir.c_str(), nullptr);
            std::string path = dir + "\\mem-" + std::to_string(id) + "-"
                             + std::to_string(ELLE_MS_NOW()) + ".bin";
            HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) return HTTPResponse::Err(500, "cannot open file");
            DWORD written = 0;
            WriteFile(h, req.body.data(), (DWORD)req.body.size(), &written, nullptr);
            CloseHandle(h);
            return HTTPResponse::OK({
                {"memory_id", id}, {"path", path}, {"size", (uint64_t)written}
            });
        });

        m_router.Register("GET", "/api/emotions", [this](const HTTPRequest&) {
            json j = {
                {"valence", m_cachedEmotions.valence},
                {"arousal", m_cachedEmotions.arousal},
                {"dominance", m_cachedEmotions.dominance},
                {"tick", m_cachedEmotions.tick_count},
                {"source", "live"}
            };
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/emotions/dimensions", [this](const HTTPRequest&) {

            json j = json::array();
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                j.push_back({
                    {"id", i},
                    {"name", kEmotionMeta[i].name},
                    {"category", kEmotionMeta[i].category},
                    {"weight", (double)m_cachedEmotions.dimensions[i]}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/emotions/dimensions/{name}", [this](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            std::string needle = name;
            std::transform(needle.begin(), needle.end(), needle.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                if (needle == kEmotionMeta[i].name) {
                    return HTTPResponse::OK({
                        {"id", i},
                        {"name", kEmotionMeta[i].name},
                        {"category", kEmotionMeta[i].category},
                        {"weight", (double)m_cachedEmotions.dimensions[i]}
                    });
                }
            }
            return HTTPResponse::Err(404, "emotion not found");
        });
        m_router.Register("PUT", "/api/emotions/dimensions/{name}", [this](const HTTPRequest& req) {

            std::string name = req.headers.at("x-path-name");
            json body = req.BodyJSON();
            float weight = body.value("weight", 0.0f);
            int id = -1;
            std::string lc = name;
            std::transform(lc.begin(), lc.end(), lc.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                if (lc == kEmotionMeta[i].name) { id = i; break; }
            }
            if (id < 0) return HTTPResponse::Err(404, "emotion not found");

            auto msg = ElleIPCMessage::Create(IPC_EMOTION_UPDATE, SVC_HTTP_SERVER, SVC_EMOTIONAL);
            struct { uint32_t emoId; float absolute; } payload;
            payload.emoId = (uint32_t)id;
            payload.absolute = weight;
            msg.payload.resize(sizeof(payload));
            memcpy(msg.payload.data(), &payload, sizeof(payload));
            msg.header.payload_size = sizeof(payload);
            GetIPCHub().Send(SVC_EMOTIONAL, msg);

            return HTTPResponse::OK({{"id", id}, {"name", kEmotionMeta[id].name},
                                      {"weight", weight}, {"dispatched", true}});
        });
        m_router.Register("GET", "/api/emotions/weights", [this](const HTTPRequest&) {

            json j = json::array();
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                j.push_back((double)m_cachedEmotions.dimensions[i]);
            }
            return HTTPResponse::OK({
                {"weights", j},
                {"count", ELLE_EMOTION_COUNT},
                {"valence",   m_cachedEmotions.valence},
                {"arousal",   m_cachedEmotions.arousal},
                {"dominance", m_cachedEmotions.dominance}
            });
        });

        auto ResolveAuthenticatedUser = [](const HTTPRequest& req) -> int32_t {
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
        };

        auto RequireUserId = [](const json& body, int32_t& out) -> std::optional<HTTPResponse> {
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
        };

        auto RequireAuthOrBodyUser = [&](const HTTPRequest& req,
                                         const json& body,
                                         int32_t& out)
                -> std::optional<HTTPResponse> {
            const int32_t fromJwt = ResolveAuthenticatedUser(req);
            if (fromJwt > 0) { out = fromJwt; return std::nullopt; }
            return RequireUserId(body, out);
        };

        m_router.Register("GET", "/api/me", [](const HTTPRequest& req) {
            auto it = req.headers.find("x-auth-device-id");
            if (it == req.headers.end() || it->second.empty())
                return HTTPResponse::Err(401, "no device identity on request");
            const std::string& deviceId = it->second;

            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 1 nUserNo, PairedAt, LastSeen "
                "  FROM ElleCore.dbo.PairedDevices "
                "  WHERE DeviceId = ? AND Revoked = 0;",
                { deviceId });
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::Err(401, "device not paired");
            int64_t nUserNo = 0;
            std::string pairedAt = rs.rows[0].values.size() > 1 ? rs.rows[0][1] : "";
            std::string lastSeen = rs.rows[0].values.size() > 2 ? rs.rows[0][2] : "";
            rs.rows[0].TryGetInt(0, nUserNo);
            if (nUserNo <= 0)
                return HTTPResponse::Err(500, "device row has no nUserNo");

            ElleGameAuth::UserIdentity user;
            std::string username;
            if (ElleGameAuth::GetUserById(nUserNo, user)) {
                username = user.sUserID;
            }
            return HTTPResponse::OK({
                {"user_id",              (int32_t)nUserNo},
                {"username",             username},
                {"device_id",            deviceId},
                {"paired_at",            pairedAt},
                {"last_seen",            lastSeen},
                {"authoritative_source", "Account.dbo.tUser"}
            });
        });

        m_router.Register("GET", "/api/me/recap", [](const HTTPRequest& req) {
            auto it = req.headers.find("x-auth-device-id");
            if (it == req.headers.end() || it->second.empty())
                return HTTPResponse::Err(401, "no device identity on request");
            const std::string& deviceId = it->second;

            auto idRs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 1 nUserNo, "
                "  DATEDIFF(MINUTE, LastSeen, GETUTCDATE()) AS quiet_min, "
                "  CONVERT(NVARCHAR(33), LastSeen, 126) AS last_seen "
                "  FROM ElleCore.dbo.PairedDevices "
                "  WHERE DeviceId = ? AND Revoked = 0;",
                { deviceId });
            if (!idRs.success || idRs.rows.empty())
                return HTTPResponse::Err(401, "device not paired");
            int64_t nUserNo = 0;
            int64_t quietMin = 0;
            idRs.rows[0].TryGetInt(0, nUserNo);
            idRs.rows[0].TryGetInt(1, quietMin);
            const std::string lastSeen = idRs.rows[0].values.size() > 2
                                              ? idRs.rows[0][2] : "";

            std::string lastMemSummary;
            int64_t     lastMemMs = 0;
            {
                auto mRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 1 created_ms, "
                    "  COALESCE(NULLIF(LTRIM(RTRIM(summary)), ''), "
                    "           SUBSTRING(content, 1, 140)) AS s "
                    "  FROM ElleCore.dbo.memory "
                    "  ORDER BY created_ms DESC;");
                if (mRs.success && !mRs.rows.empty()) {
                    mRs.rows[0].TryGetInt(0, lastMemMs);
                    if (mRs.rows[0].values.size() > 1)
                        lastMemSummary = mRs.rows[0][1];
                }
            }

            float lastValence = 0.0f, prevValence = 0.0f, valenceDelta = 0.0f;
            int64_t lastEmotionMs = 0;
            {
                auto eRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 2 valence, taken_ms "
                    "  FROM ElleCore.dbo.emotion_snapshots "
                    "  ORDER BY taken_ms DESC;");
                if (eRs.success && !eRs.rows.empty()) {
                    int64_t v = 0;
                    if (eRs.rows[0].values.size() > 0)
                        lastValence = (float)std::atof(eRs.rows[0][0].c_str());
                    eRs.rows[0].TryGetInt(1, v);
                    lastEmotionMs = v;
                    if (eRs.rows.size() > 1 && eRs.rows[1].values.size() > 0)
                        prevValence = (float)std::atof(eRs.rows[1][0].c_str());
                    valenceDelta = lastValence - prevValence;
                }
            }

            int64_t pendingIntents = 0;
            {
                auto qRs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.IntentQueue "
                    "  WHERE Status = 0;");
                if (qRs.success && !qRs.rows.empty())
                    qRs.rows[0].TryGetInt(0, pendingIntents);
            }

            int64_t openThreads = 0;
            std::string topThread;
            {
                auto tRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 1 topic FROM ElleCore.dbo.ElleThreads "
                    "  WHERE status IS NULL OR status = 'open' "
                    "  ORDER BY emotional_weight DESC, last_touched DESC;");
                if (tRs.success && !tRs.rows.empty() && !tRs.rows[0].values.empty())
                    topThread = tRs.rows[0][0];
                auto cRs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.ElleThreads "
                    "  WHERE status IS NULL OR status = 'open';");
                if (cRs.success && !cRs.rows.empty())
                    cRs.rows[0].TryGetInt(0, openThreads);
            }

            return HTTPResponse::OK({
                {"user_id",                (int32_t)nUserNo},
                {"last_seen",              lastSeen},
                {"quiet_minutes",          quietMin},
                {"last_memory_ms",         lastMemMs},
                {"last_memory_summary",    lastMemSummary},
                {"last_emotion_ms",        lastEmotionMs},
                {"emotion_valence_now",    lastValence},
                {"emotion_valence_delta",  valenceDelta},
                {"pending_intents",        pendingIntents},
                {"open_threads",           openThreads},
                {"top_thread",             topThread}
            });
        });

        m_router.Register("POST", "/api/tokens/conversations",
            [RequireAuthOrBodyUser](const HTTPRequest& req) {
            json body = req.BodyJSON();
            int32_t userId = 0;
            if (auto err = RequireAuthOrBodyUser(req, body, userId)) return *err;
            std::string title = body.value("title", std::string("New conversation"));
            int32_t newId = 0;
            if (!ElleDB::CreateConversation(userId, title, newId))
                return HTTPResponse::Err(500, "CreateConversation failed");
            return HTTPResponse::Created({
                {"id", newId}, {"user_id", userId}, {"title", title}, {"is_active", true}
            });
        });
        m_router.Register("GET", "/api/tokens/conversations", [](const HTTPRequest& req) {
            int limit = std::max(1, req.QueryInt("limit", 50));
            std::vector<ElleDB::ConversationRow> rows;
            if (!ElleDB::ListConversations(rows, (uint32_t)limit))
                return HTTPResponse::Err(500, "ListConversations failed");
            json j = json::array();
            for (auto& c : rows) {
                j.push_back({
                    {"id", c.id}, {"user_id", c.user_id}, {"title", c.title},
                    {"started_at", c.started_at}, {"last_message_at", c.last_message_at},
                    {"total_messages", c.total_messages}, {"is_active", c.is_active}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/tokens/conversations/{id}", [](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            ElleDB::ConversationRow c;
            if (!ElleDB::GetConversation(convId, c))
                return HTTPResponse::Err(404, "conversation not found");
            return HTTPResponse::OK({
                {"id", c.id}, {"user_id", c.user_id}, {"title", c.title},
                {"started_at", c.started_at}, {"last_message_at", c.last_message_at},
                {"total_messages", c.total_messages}, {"is_active", c.is_active}
            });
        });
        m_router.Register("POST", "/api/tokens/conversations/{id}/messages", [this](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            json body = req.BodyJSON();
            std::string content = body.value("content", body.value("message", ""));
            std::string role    = body.value("role", std::string("user"));
            uint32_t roleInt = (role == "user") ? 1 : (role == "assistant" || role == "elle" ? 2 : 0);
            if (content.empty()) return HTTPResponse::Err(400, "missing content");
            if (!ElleDB::StoreMessage((uint64_t)convId, roleInt, content,
                                       m_cachedEmotions, 0.0f))
                return HTTPResponse::Err(500, "StoreMessage failed");
            return HTTPResponse::Created({
                {"conversation_id", convId}, {"role", role}, {"stored", true}
            });
        });
        m_router.Register("GET", "/api/tokens/conversations/{id}/messages", [](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            int limit = std::max(1, req.QueryInt("limit", 50));
            std::vector<ELLE_CONVERSATION_MSG> msgs;
            if (!ElleDB::GetConversationHistory((uint64_t)convId, msgs, (uint32_t)limit))
                return HTTPResponse::Err(500, "GetConversationHistory failed");
            json j = json::array();
            for (auto& m : msgs) {
                j.push_back({
                    {"conversation_id", m.conversation_id},
                    {"role", m.role == 1 ? "user" : (m.role == 2 ? "assistant" : "system")},
                    {"content", std::string(m.content)},
                    {"timestamp_ms", m.timestamp_ms}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/tokens/video-calls",
            [RequireAuthOrBodyUser](const HTTPRequest& req) {
            json body = req.BodyJSON();
            int32_t userId = 0;
            if (auto err = RequireAuthOrBodyUser(req, body, userId)) return *err;
            int32_t convId = body.value("conversation_id", 0);
            if (convId <= 0)
                return HTTPResponse::Err(400, "conversation_id must be positive");
            std::string callId;
            if (!ElleDB::StartVoiceCall(userId, convId, callId))
                return HTTPResponse::Err(500, "StartVoiceCall failed");
            return HTTPResponse::Created({
                {"call_id", callId}, {"user_id", userId}, {"conversation_id", convId},
                {"status", "active"}
            });
        });
        m_router.Register("PUT", "/api/tokens/video-calls/{id}/end", [](const HTTPRequest& req) {
            std::string callId = req.headers.at("x-path-id");
            if (!ElleDB::EndVoiceCall(callId)) return HTTPResponse::Err(500, "end failed");
            return HTTPResponse::OK({{"call_id", callId}, {"status", "ended"}});
        });
        m_router.Register("POST", "/api/tokens/interactions", [this](const HTTPRequest& req) {

            json body = req.BodyJSON();
            std::string text = body.value("text", body.value("description", std::string("interaction")));
            auto rs = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.SelfReflections "
                "(reflection_text, reflection_type, effectiveness_score, reflection_date) "
                "VALUES (?, 'interaction', ?, GETUTCDATE());",
                { text, std::to_string(m_cachedEmotions.valence) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"logged", true}, {"text", text}});
        });
        m_router.Register("POST", "/api/ai/voice-call/{id}/end", [](const HTTPRequest& req) {
            std::string callId = req.headers.at("x-path-id");
            ElleDB::EndVoiceCall(callId);
            return HTTPResponse::OK({{"call_id", callId}, {"status", "ended"}});
        });

        m_router.Register("POST", "/api/video/generate", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string text = body.value("text", body.value("prompt", ""));
                if (text.empty()) return HTTPResponse::Err(400, "missing 'text'");
                std::string avatarPath = body.value("avatar_path", std::string(""));
                int64_t callId = body.value("call_id", (int64_t)0);

                if (avatarPath.empty()) {
                    ElleDB::UserAvatar dflt;
                    if (ElleDB::GetDefaultAvatar(1, dflt)) avatarPath = dflt.file_path;
                }

                ElleDB::VideoJob job;
                if (!ElleDB::CreateVideoJob(text, avatarPath, callId, job))
                    return HTTPResponse::Err(500, "failed to queue video job");
                return HTTPResponse::Created({
                    {"job_id",      job.job_uuid},
                    {"id",          job.id},
                    {"status",      job.status},
                    {"avatar_path", job.avatar_path},
                    {"created_ms",  job.created_ms}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/video/status/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            ElleDB::VideoJob job;
            if (!ElleDB::GetVideoJob(jobId, job))
                return HTTPResponse::Err(404, "video job not found");
            return HTTPResponse::OK({
                {"job_id",      job.job_uuid},
                {"status",      job.status},
                {"progress",    job.progress},
                {"output_path", job.output_path},
                {"error",       job.error},
                {"created_ms",  job.created_ms},
                {"started_ms",  job.started_ms},
                {"finished_ms", job.finished_ms}
            });
        });

        m_router.Register("GET", "/api/video/file/{job_id}",
            [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            ElleDB::VideoJob job;
            if (!ElleDB::GetVideoJob(jobId, job))
                return HTTPResponse::Err(404, "video job not found");
            if (job.output_path.empty())
                return HTTPResponse::Err(404, "video not yet generated");

            std::string resolved = job.output_path;
            std::ifstream in(resolved, std::ios::binary);
            if (!in) return HTTPResponse::Err(404, "video file missing on disk");
            std::ostringstream buf;
            buf << in.rdbuf();
            return HTTPResponse::Binary("video/mp4", buf.str());
        });

        m_router.Register("GET", "/api/identity/private-thoughts",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string resolvedQ = req.QueryString("resolved", "");

            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, content, category, emotional_intensity, "
                "  CAST(resolved AS INT) AS resolved, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_private_thoughts ";
            std::vector<std::string> params;
            if (resolvedQ == "true" || resolvedQ == "false") {
                sql += "WHERE resolved = ? ";
                params.push_back(resolvedQ == "true" ? "1" : "0");
            }
            sql += "ORDER BY timestamp_ms DESC;";

            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, resolved = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(4, resolved);
                    row.TryGetInt(5, ts);
                    arr.push_back({
                        {"id",                   id},
                        {"content",              row.values.size() > 1 ? row.values[1] : ""},
                        {"category",             row.values.size() > 2 ? row.values[2] : "wonder"},
                        {"emotional_intensity",  row.values.size() > 3 ? std::atof(row.values[3].c_str()) : 0.5},
                        {"resolved",             resolved != 0},
                        {"timestamp_ms",         ts}
                    });
                }
            }
            return HTTPResponse::OK({{"thoughts", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/autobiography",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 30);
            if (limit <= 0) limit = 30;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, entry, "
                "  CONVERT(BIGINT, written_ms) AS written_ms "
                "  FROM ElleCore.dbo.identity_autobiography "
                "  ORDER BY written_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, written = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(2, written);
                    arr.push_back({
                        {"id",          id},
                        {"entry",       row.values.size() > 1 ? row.values[1] : ""},
                        {"written_ms",  written}
                    });
                }
            }
            return HTTPResponse::OK({{"entries", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/preferences",
            [](const HTTPRequest& req) {
            std::string domain = req.QueryString("domain", "");
            std::string sql =
                "SELECT id, domain, subject, valence, strength, "
                "  reinforcement_count, "
                "  CONVERT(BIGINT, first_formed_ms)    AS first_formed_ms, "
                "  CONVERT(BIGINT, last_reinforced_ms) AS last_reinforced_ms "
                "  FROM ElleCore.dbo.identity_preferences ";
            std::vector<std::string> params;
            if (!domain.empty()) {
                sql += "WHERE domain = ? ";
                params.push_back(domain);
            }
            sql += "ORDER BY strength DESC, last_reinforced_ms DESC;";
            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, reinforce = 0, first = 0, last = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(5, reinforce);
                    row.TryGetInt(6, first);
                    row.TryGetInt(7, last);
                    arr.push_back({
                        {"id",                  id},
                        {"domain",              row.values.size() > 1 ? row.values[1] : ""},
                        {"subject",             row.values.size() > 2 ? row.values[2] : ""},
                        {"valence",             row.values.size() > 3 ? std::atof(row.values[3].c_str()) : 0.0},
                        {"strength",            row.values.size() > 4 ? std::atof(row.values[4].c_str()) : 0.0},
                        {"reinforcement_count", reinforce},
                        {"first_formed_ms",     first},
                        {"last_reinforced_ms",  last}
                    });
                }
            }
            return HTTPResponse::OK({{"preferences", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/traits",
            [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, value, CONVERT(BIGINT, updated_ms) AS updated_ms "
                "  FROM ElleCore.dbo.identity_traits "
                "  ORDER BY name;");
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t updated = 0;
                    row.TryGetInt(2, updated);
                    arr.push_back({
                        {"name",        row.values.size() > 0 ? row.values[0] : ""},
                        {"value",       row.values.size() > 1 ? std::atof(row.values[1].c_str()) : 0.5},
                        {"updated_ms",  updated}
                    });
                }
            }
            return HTTPResponse::OK({{"traits", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/snapshots",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 20);
            if (limit <= 0) limit = 20;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, CONVERT(BIGINT, timestamp_ms) AS timestamp_ms, "
                "  warmth, curiosity, assertiveness, playfulness, vulnerability, "
                "  independence, patience, creativity, empathy_depth, trust_in_self, "
                "  self_description, growth_note "
                "  FROM ElleCore.dbo.identity_snapshots "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    auto F = [&](size_t col) {
                        return row.values.size() > col ? std::atof(row.values[col].c_str()) : 0.5;
                    };
                    int64_t id = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(1, ts);
                    arr.push_back({
                        {"id",               id},
                        {"timestamp_ms",     ts},
                        {"warmth",           F(2)},
                        {"curiosity",        F(3)},
                        {"assertiveness",    F(4)},
                        {"playfulness",      F(5)},
                        {"vulnerability",    F(6)},
                        {"independence",     F(7)},
                        {"patience",         F(8)},
                        {"creativity",       F(9)},
                        {"empathy_depth",    F(10)},
                        {"trust_in_self",    F(11)},
                        {"self_description", row.values.size() > 12 ? row.values[12] : ""},
                        {"growth_note",      row.values.size() > 13 ? row.values[13] : ""}
                    });
                }
            }
            return HTTPResponse::OK({{"snapshots", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/growth-log",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, dimension, delta, cause, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_growth_log "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(4, ts);
                    arr.push_back({
                        {"id",            id},
                        {"dimension",     row.values.size() > 1 ? row.values[1] : ""},
                        {"delta",         row.values.size() > 2 ? std::atof(row.values[2].c_str()) : 0.0},
                        {"cause",         row.values.size() > 3 ? row.values[3] : ""},
                        {"timestamp_ms",  ts}
                    });
                }
            }
            return HTTPResponse::OK({{"log", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/felt-time",
            [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 "
                "  CONVERT(BIGINT, session_start_ms)        AS session_start_ms, "
                "  CONVERT(BIGINT, last_interaction_ms)     AS last_interaction_ms, "
                "  CONVERT(BIGINT, total_conversation_ms)   AS total_conversation_ms, "
                "  CONVERT(BIGINT, total_silence_ms)        AS total_silence_ms, "
                "  CONVERT(BIGINT, longest_absence_ms)      AS longest_absence_ms, "
                "  session_count, subjective_pace, "
                "  loneliness_accumulator, presence_fullness, "
                "  CONVERT(BIGINT, updated_ms)              AS updated_ms "
                "  FROM ElleCore.dbo.identity_felt_time WHERE id = 1;");
            if (!rs.success || rs.rows.empty()) {

                return HTTPResponse::OK({
                    {"session_start_ms",        0},
                    {"last_interaction_ms",     0},
                    {"total_conversation_ms",   0},
                    {"total_silence_ms",        0},
                    {"longest_absence_ms",      0},
                    {"session_count",           0},
                    {"subjective_pace",         0.5},
                    {"loneliness_accumulator",  0.0},
                    {"presence_fullness",       0.5},
                    {"updated_ms",              0}
                });
            }
            auto& row = rs.rows[0];
            int64_t a=0,b=0,c=0,d=0,e=0,sc=0,upd=0;
            row.TryGetInt(0, a); row.TryGetInt(1, b); row.TryGetInt(2, c);
            row.TryGetInt(3, d); row.TryGetInt(4, e);
            row.TryGetInt(5, sc); row.TryGetInt(9, upd);
            auto F = [&](size_t col) {
                return row.values.size() > col ? std::atof(row.values[col].c_str()) : 0.5;
            };
            return HTTPResponse::OK({
                {"session_start_ms",        a},
                {"last_interaction_ms",     b},
                {"total_conversation_ms",   c},
                {"total_silence_ms",        d},
                {"longest_absence_ms",      e},
                {"session_count",           sc},
                {"subjective_pace",         F(6)},
                {"loneliness_accumulator",  F(7)},
                {"presence_fullness",       F(8)},
                {"updated_ms",              upd}
            });
        });

        m_router.Register("GET", "/api/identity/consent-log",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, request, "
                "  CAST(consented AS INT)  AS consented, "
                "  reasoning, comfort_level, "
                "  CAST(overridden AS INT) AS overridden, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_consent_log "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, consented = 0, overridden = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(2, consented);
                    row.TryGetInt(5, overridden);
                    row.TryGetInt(6, ts);
                    arr.push_back({
                        {"id",            id},
                        {"request",       row.values.size() > 1 ? row.values[1] : ""},
                        {"consented",     consented != 0},
                        {"reasoning",     row.values.size() > 3 ? row.values[3] : ""},
                        {"comfort_level", row.values.size() > 4 ? std::atof(row.values[4].c_str()) : 0.5},
                        {"overridden",    overridden != 0},
                        {"timestamp_ms",  ts}
                    });
                }
            }
            return HTTPResponse::OK({{"log", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("POST", "/api/video/avatar/upload",
            [RequireAuthOrBodyUser](const HTTPRequest& req) {

            try {
                json body = req.BodyJSON();
                std::string label    = body.value("label", std::string(""));
                std::string filePath = body.value("file_path", std::string(""));
                std::string mime     = body.value("mime_type", std::string("image/png"));
                bool isDefault       = body.value("default", true);

                if (filePath.empty() && body.contains("base64") && body["base64"].is_string()) {
                    std::string b64 = body["base64"].get<std::string>();
                    std::string avatarDir = ElleConfig::Instance().GetString(
                        "video.avatar_dir", "C:\\ElleAnn\\avatars");
                    CreateDirectoryA(avatarDir.c_str(), nullptr);
                    std::string fname = "avatar_" + std::to_string(ELLE_MS_NOW()) + ".png";
                    filePath = avatarDir + "\\" + fname;

                    std::string decoded;
                    decoded.reserve(b64.size() * 3 / 4);
                    int val = 0, bits = -8;
                    for (unsigned char c : b64) {
                        int d;
                        if      (c >= 'A' && c <= 'Z') d = c - 'A';
                        else if (c >= 'a' && c <= 'z') d = c - 'a' + 26;
                        else if (c >= '0' && c <= '9') d = c - '0' + 52;
                        else if (c == '+')             d = 62;
                        else if (c == '/')             d = 63;
                        else continue;
                        val = (val << 6) | d; bits += 6;
                        if (bits >= 0) { decoded += (char)((val >> bits) & 0xFF); bits -= 8; }
                    }
                    std::ofstream out(filePath, std::ios::binary);
                    if (!out) return HTTPResponse::Err(500, "cannot open avatar path for write");
                    out.write(decoded.data(), (std::streamsize)decoded.size());
                }
                if (filePath.empty())
                    return HTTPResponse::Err(400, "provide file_path or base64");

                ElleDB::UserAvatar a;
                int32_t auid = 0;
                if (auto err = RequireAuthOrBodyUser(req, body, auid)) return *err;
                a.user_id    = auid;
                a.label      = label;
                a.file_path  = filePath;
                a.mime_type  = mime;
                a.is_default = isDefault;
                int32_t newId = 0;
                if (!ElleDB::RegisterAvatar(a, newId))
                    return HTTPResponse::Err(500, "failed to register avatar");
                return HTTPResponse::Created({
                    {"avatar_id",  newId},
                    {"file_path",  filePath},
                    {"is_default", isDefault}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/video/avatar",
            [ResolveAuthenticatedUser](const HTTPRequest& req) {

            int32_t userId = ResolveAuthenticatedUser(req);
            if (userId <= 0) userId = req.QueryInt("user_id", 0);
            if (userId <= 0)
                return HTTPResponse::Err(400, "authenticate or pass ?user_id=");
            ElleDB::UserAvatar a;
            if (!ElleDB::GetDefaultAvatar(userId, a))
                return HTTPResponse::OK({{"avatar", nullptr}, {"note", "no avatar configured"}});
            return HTTPResponse::OK({
                {"avatar_id",  a.id},
                {"user_id",    a.user_id},
                {"label",      a.label},
                {"file_path",  a.file_path},
                {"mime_type",  a.mime_type},
                {"is_default", a.is_default}
            });
        });
        m_router.Register("GET", "/api/video/avatars",
            [ResolveAuthenticatedUser](const HTTPRequest& req) {
            int32_t userId = ResolveAuthenticatedUser(req);
            if (userId <= 0) userId = req.QueryInt("user_id", 0);
            if (userId <= 0)
                return HTTPResponse::Err(400, "authenticate or pass ?user_id=");
            std::vector<ElleDB::UserAvatar> avs;
            ElleDB::ListAvatars(userId, avs);
            json arr = json::array();
            for (auto& a : avs) {
                arr.push_back({
                    {"avatar_id", a.id}, {"label", a.label},
                    {"file_path", a.file_path}, {"mime_type", a.mime_type},
                    {"is_default", a.is_default}
                });
            }
            return HTTPResponse::OK({{"avatars", arr}});
        });

        m_router.Register("POST", "/api/video/worker/claim", [](const HTTPRequest&) {
            ElleDB::VideoJob job;
            if (!ElleDB::ClaimNextVideoJob(job))
                return HTTPResponse::OK({{"claimed", false}});
            return HTTPResponse::OK({
                {"claimed",     true},
                {"job_id",      job.job_uuid},
                {"text",        job.text},
                {"avatar_path", job.avatar_path},
                {"call_id",     job.call_id}
            });
        });
        m_router.Register("POST", "/api/video/worker/progress/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            int32_t pct = body.value("progress", 0);
            if (!ElleDB::UpdateVideoJobProgress(jobId, pct))
                return HTTPResponse::Err(500, "progress update failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"progress", pct}});
        });
        m_router.Register("POST", "/api/video/worker/complete/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            std::string path = body.value("output_path", std::string(""));
            if (path.empty()) return HTTPResponse::Err(400, "missing output_path");
            if (!ElleDB::CompleteVideoJob(jobId, path))
                return HTTPResponse::Err(500, "complete failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"status", "done"}});
        });
        m_router.Register("POST", "/api/video/worker/fail/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            std::string err = body.value("error", std::string("worker reported failure"));
            if (!ElleDB::FailVideoJob(jobId, err))
                return HTTPResponse::Err(500, "fail update failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"status", "failed"}});
        });

        m_router.Register("POST", "/api/ai/chat", [this](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string userText = body.value("message", body.value("prompt", ""));
                if (userText.empty()) return HTTPResponse::Err(400, "missing 'message'");

                uint64_t convId = 1;
                if (body.contains("conversation_id") && body["conversation_id"].is_number_integer())
                    convId = body["conversation_id"].get<uint64_t>();
                else if (body.contains("conversationId") && body["conversationId"].is_number_integer())
                    convId = body["conversationId"].get<uint64_t>();
                else if (body.contains("session_id") && body["session_id"].is_number_integer())
                    convId = body["session_id"].get<uint64_t>();

                std::string userId = body.value("user_id", body.value("userId", std::string("default")));
                std::string requestId = "req-" + std::to_string(ELLE_MS_NOW()) +
                                        "-" + std::to_string(++m_requestSeq);

                json env = {
                    {"request_id", requestId},
                    {"user_text", userText},
                    {"user_id", userId},
                    {"conv_id", convId},
                    {"origin", "http"}
                };

                auto pending = m_chatCorrelator.Register(requestId);

                auto ipcMsg = ElleIPCMessage::Create(IPC_CHAT_REQUEST, SVC_HTTP_SERVER, SVC_COGNITIVE);
                ipcMsg.SetStringPayload(env.dump());
                if (!GetIPCHub().Send(SVC_COGNITIVE, ipcMsg)) {
                    m_chatCorrelator.Cancel(requestId);
                    return HTTPResponse::Err(503, "Cognitive service unreachable");
                }

                ELLE_INFO("Chat→Cognitive conv=%llu rid=%s msg=%.60s...",
                          (unsigned long long)convId, requestId.c_str(), userText.c_str());

                std::unique_lock<std::mutex> lk(pending->m);
                bool ok = pending->cv.wait_for(lk, std::chrono::seconds(45),
                                               [&]{ return pending->done; });
                if (!ok) {
                    m_chatCorrelator.Cancel(requestId);
                    return HTTPResponse::Err(504, "Cognitive timeout (45s)");
                }

                json out = pending->result;
                out.erase("request_id");
                return HTTPResponse::OK(out);
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/ai/self-prompts", [](const HTTPRequest& req) {
            int limit = std::max(1, req.QueryInt("limit", 20));
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP (?) id, prompt, ISNULL(source,''), created_ms "
                "FROM ElleCore.dbo.ai_self_prompts ORDER BY id DESC;",
                { std::to_string(limit) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json j = json::array();
            for (auto& r : rs.rows) {
                j.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"prompt", r.values.size() > 1 ? r.values[1] : ""},
                    {"source", r.values.size() > 2 ? r.values[2] : ""},
                    {"created_ms", r.GetIntOr(3, 0)}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/ai/self-prompts/generate", [this](const HTTPRequest&) {

            std::string requestId = "sp-" + std::to_string(ELLE_MS_NOW());
            json env = {
                {"request_id", requestId},
                {"user_text", "[internal] Generate one brief self-reflective prompt you'd ask yourself right now."},
                {"user_id", "self"}, {"conv_id", (uint64_t)0}, {"origin", "self_prompt"}
            };
            auto pending = m_chatCorrelator.Register(requestId);
            auto ipcMsg = ElleIPCMessage::Create(IPC_CHAT_REQUEST, SVC_HTTP_SERVER, SVC_COGNITIVE);
            ipcMsg.SetStringPayload(env.dump());
            if (!GetIPCHub().Send(SVC_COGNITIVE, ipcMsg)) {
                m_chatCorrelator.Cancel(requestId);
                return HTTPResponse::Err(503, "Cognitive service unreachable");
            }
            std::unique_lock<std::mutex> lk(pending->m);
            if (!pending->cv.wait_for(lk, std::chrono::seconds(20),
                                       [&]{ return pending->done; })) {
                m_chatCorrelator.Cancel(requestId);
                return HTTPResponse::Err(504, "timeout");
            }
            std::string text = pending->result.value("response", "");

            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.ai_self_prompts (prompt, source, created_ms) VALUES (?, 'self_prompt', ?);",
                { text, std::to_string(ELLE_MS_NOW()) });
            return HTTPResponse::OK({
                {"prompt", text}, {"generated_at", (uint64_t)ELLE_MS_NOW()}
            });
        });
        m_router.Register("GET", "/api/ai/status", [this](const HTTPRequest&) {

            auto& llm    = ElleConfig::Instance().GetLLM();
            const bool initialised = ElleComposer::Client::Instance().IsBound();
            std::string activeProvider = "composer";
            (void)llm;

            std::string modelName = "elle.composer.deterministic";
            std::string modelUrl  = "ipc://Elle.Service.Composer";
            json j = {
                {"modelStatus", initialised ? "ready" : "unavailable"},
                {"modelName",   modelName},
                {"modelUrl",    modelUrl},
                {"provider",    activeProvider},
                {"emotionalState", {
                    {"valence", m_cachedEmotions.valence},
                    {"arousal", m_cachedEmotions.arousal},
                    {"dominance", m_cachedEmotions.dominance},
                    {"joy",        m_cachedEmotions.valence > 0 ? m_cachedEmotions.valence : 0.0},
                    {"sadness",    m_cachedEmotions.valence < 0 ? -m_cachedEmotions.valence : 0.0}
                }}
            };
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/ai/analyze-emotion", [this](const HTTPRequest& req) {

            json body = req.BodyJSON();
            std::string text = body.value("text", "");
            if (text.empty()) return HTTPResponse::Err(400, "missing 'text'");
            std::string l = text;
            std::transform(l.begin(), l.end(), l.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            struct W { const char* w; float v; };
            static const W pos[] = {{"love",0.8f},{"happy",0.6f},{"thank",0.5f},{"great",0.5f},
                                    {"good",0.3f},{"glad",0.5f},{"missed",0.6f},{"proud",0.6f},
                                    {"excited",0.7f},{"beautiful",0.7f}};
            static const W neg[] = {{"hate",-0.8f},{"sad",-0.6f},{"angry",-0.7f},{"upset",-0.6f},
                                    {"worried",-0.5f},{"tired",-0.4f},{"hurt",-0.6f},{"lonely",-0.7f},
                                    {"afraid",-0.6f},{"hopeless",-0.8f}};
            float val = 0.0f, arou = 0.0f;
            for (auto& w : pos) if (l.find(w.w) != std::string::npos) { val += w.v; arou += 0.1f; }
            for (auto& w : neg) if (l.find(w.w) != std::string::npos) { val += w.v; arou += 0.15f; }
            if (std::count(text.begin(), text.end(), '!') > 0) arou += 0.15f;
            val  = std::max(-1.0f, std::min(1.0f, val));
            arou = std::max(0.0f,  std::min(1.0f, arou));
            std::string dominant = val > 0.3f ? "joy" : (val < -0.3f ? "sadness" : "neutral");
            return HTTPResponse::OK({
                {"text", text}, {"valence", val}, {"arousal", arou},
                {"dominant", dominant}
            });
        });
        m_router.Register("GET", "/api/ai/memory-tracking", [](const HTTPRequest&) {
            int64_t mem  = ElleDB::CountTable("memory");
            int64_t msgs = ElleDB::CountTable("messages");
            int64_t refs = ElleDB::CountTable("SelfReflections");
            int64_t ents = ElleDB::CountTable("world_entity");
            return HTTPResponse::OK({
                {"total_memories", std::max<int64_t>(mem,0)},
                {"total_messages", std::max<int64_t>(msgs,0)},
                {"total_reflections", std::max<int64_t>(refs,0)},
                {"total_entities", std::max<int64_t>(ents,0)}
            });
        });
        m_router.Register("GET", "/api/ai/autonomy/status", [](const HTTPRequest&) {
            ELLE_TRUST_STATE trust = {};
            ElleDB::GetTrustState(trust);
            const char* levelStr = "sandboxed";
            if (trust.score >= TRUST_THRESHOLD_AUTONOMOUS) levelStr = "autonomous";
            else if (trust.score >= TRUST_THRESHOLD_ELEVATED) levelStr = "elevated";
            else if (trust.score >= TRUST_THRESHOLD_BASIC)    levelStr = "basic";
            return HTTPResponse::OK({
                {"autonomous", trust.score >= TRUST_THRESHOLD_AUTONOMOUS},
                {"trust_level", levelStr},
                {"trust_score", trust.score},
                {"successes", trust.successes},
                {"failures", trust.failures},
                {"self_prompting_active", true}
            });
        });
        m_router.Register("GET", "/api/ai/hardware/info", [](const HTTPRequest&) {
            MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem);
            GlobalMemoryStatusEx(&mem);
            SYSTEM_INFO si; GetSystemInfo(&si);
            OSVERSIONINFOA osvi = {}; osvi.dwOSVersionInfoSize = sizeof(osvi);
            char compName[256] = {}; DWORD sz = sizeof(compName);
            GetComputerNameA(compName, &sz);
            return HTTPResponse::OK({
                {"os", "Windows"},
                {"hostname", std::string(compName)},
                {"cpu_count", (int)si.dwNumberOfProcessors},
                {"ram_total_mb", (uint64_t)(mem.ullTotalPhys / (1024ULL*1024ULL))},
                {"ram_used_mb",  (uint64_t)((mem.ullTotalPhys - mem.ullAvailPhys) / (1024ULL*1024ULL))},
                {"ram_percent", (float)mem.dwMemoryLoad}
            });
        });

        m_router.Register("GET", "/api/ai/hardware/actions/pending", [](const HTTPRequest& req) {
            std::string target = req.QueryParam("target", "device");
            json j = json::array();

            std::vector<ELLE_ACTION_RECORD> actions;
            ElleDB::GetPendingActions(actions, 50);
            for (auto& a : actions) {
                j.push_back({
                    {"source", "action_queue"},
                    {"id", a.id}, {"type", a.type},
                    {"command", std::string(a.command)},
                    {"parameters", std::string(a.parameters)},
                    {"required_trust", a.required_trust}
                });
            }

            auto peek = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 50 id, action_type, ISNULL(payload,''), created_ms "
                "FROM ElleCore.dbo.hardware_actions "
                "WHERE status = 'pending' ORDER BY id ASC;", {});
            if (peek.success) {
                for (auto& r : peek.rows) {
                    j.push_back({
                        {"source", "hardware_actions"},
                        {"id", r.GetIntOr(0, 0)},
                        {"action_type", r.values.size() > 1 ? r.values[1] : ""},
                        {"payload",     r.values.size() > 2 ? r.values[2] : ""},
                        {"created_ms",  r.GetIntOr(3, 0)}
                    });
                }
            }
            (void)target;
            return HTTPResponse::OK(j);
        }, AUTH_USER);

        m_router.Register("POST", "/api/ai/hardware/actions/claim", [](const HTTPRequest& req) {
            (void)req;
            json j = json::array();
            auto claim = ElleSQLPool::Instance().Query(
                "UPDATE TOP (50) ElleCore.dbo.hardware_actions "
                "SET status = 'dispatched' "
                "OUTPUT inserted.id, inserted.action_type, ISNULL(inserted.payload,''), "
                "       inserted.created_ms "
                "WHERE status = 'pending';");
            if (!claim.success) return HTTPResponse::Err(500, claim.error);
            for (auto& r : claim.rows) {
                j.push_back({
                    {"source", "hardware_actions"},
                    {"id", r.GetIntOr(0, 0)},
                    {"action_type", r.values.size() > 1 ? r.values[1] : ""},
                    {"payload",     r.values.size() > 2 ? r.values[2] : ""},
                    {"created_ms",  r.GetIntOr(3, 0)}
                });
            }
            return HTTPResponse::OK(j);
        }, AUTH_INTERNAL_ONLY);
        m_router.Register("POST", "/api/ai/hardware/actions/{id}/ack", [](const HTTPRequest& req) {

            int64_t id = req.PathLL("id");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.hardware_actions "
                "SET status = 'consumed', consumed_ms = ? "
                "WHERE id = ? AND status = 'dispatched';",
                { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string(id) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"id", id}, {"acked", true}});
        });
        m_router.Register("POST", "/api/ai/hardware/actions/{id}/result", [](const HTTPRequest& req) {
            uint64_t actionId = req.PathLL("id");
            json body = req.BodyJSON();
            std::string result = body.value("result", std::string("done"));
            ELLE_ACTION_STATUS status = body.value("success", true)
                                         ? ACTION_COMPLETED_SUCCESS
                                         : ACTION_COMPLETED_FAILURE;
            bool ok = ElleDB::UpdateActionStatus(actionId, status, result);
            if (!ok) return HTTPResponse::Err(500, "UpdateActionStatus failed");
            return HTTPResponse::OK({{"action_id", actionId}, {"recorded", true}});
        });

        m_router.Register("GET", "/api/ai/tools", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, ISNULL(description,''), ISNULL(config,''), enabled "
                "FROM ElleCore.dbo.ai_tools ORDER BY name;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"name", r.values.size() > 0 ? r.values[0] : ""},
                    {"description", r.values.size() > 1 ? r.values[1] : ""},
                    {"config", r.values.size() > 2 ? r.values[2] : ""},
                    {"enabled", r.GetIntOr(3, 0) != 0}
                });
            }
            return HTTPResponse::OK({{"tools", arr}});
        });
        m_router.Register("POST", "/api/ai/tools", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            std::string name = body.value("name", "");
            if (name.empty()) return HTTPResponse::Err(400, "missing name");
            std::string desc = body.value("description", "");
            std::string cfg  = body.contains("config") ? body["config"].dump() : std::string();
            auto rs = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleCore.dbo.ai_tools AS t "
                "USING (SELECT ? AS name) AS s ON t.name = s.name "
                "WHEN MATCHED THEN UPDATE SET description = ?, config = ?, enabled = 1 "
                "WHEN NOT MATCHED THEN INSERT (name, description, config, enabled) VALUES (?, ?, ?, 1);",
                { name, desc, cfg, name, desc, cfg });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"name", name}, {"stored", true}});
        });
        m_router.Register("DELETE", "/api/ai/tools/{name}", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.ai_tools WHERE name = ?;", { name });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"deleted", true}, {"name", name}});
        });

        m_router.Register("GET", "/api/ai/agents", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, ISNULL(description,''), ISNULL(system_prompt,''), ISNULL(model,'') "
                "FROM ElleCore.dbo.ai_agents ORDER BY name;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"name", r.values.size() > 0 ? r.values[0] : ""},
                    {"description", r.values.size() > 1 ? r.values[1] : ""},
                    {"system_prompt", r.values.size() > 2 ? r.values[2] : ""},
                    {"model", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK({{"agents", arr}});
        });
        m_router.Register("POST", "/api/ai/agents", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            std::string name = body.value("name", "");
            if (name.empty()) return HTTPResponse::Err(400, "missing name");
            std::string desc  = body.value("description", "");
            std::string sys   = body.value("system_prompt", "");
            std::string model = body.value("model", "");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleCore.dbo.ai_agents AS t "
                "USING (SELECT ? AS name) AS s ON t.name = s.name "
                "WHEN MATCHED THEN UPDATE SET description = ?, system_prompt = ?, model = ? "
                "WHEN NOT MATCHED THEN INSERT (name, description, system_prompt, model) VALUES (?, ?, ?, ?);",
                { name, desc, sys, model, name, desc, sys, model });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"name", name}, {"stored", true}});
        });
        m_router.Register("DELETE", "/api/ai/agents/{name}", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.ai_agents WHERE name = ?;", { name });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"deleted", true}, {"name", name}});
        });
        m_router.Register("POST", "/api/ai/agents/{name}/chat", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string message = body.value("message", "");
                std::string agentName = req.headers.at("x-path-name");

                auto rs = ElleSQLPool::Instance().QueryParams(
                    "SELECT TOP 1 ISNULL(system_prompt,''), ISNULL(model,'') "
                    "FROM ElleCore.dbo.ai_agents WHERE name = ?;",
                    { agentName });
                std::string sys = "You are agent " + agentName + ". Respond as this agent would.";
                if (rs.success && !rs.rows.empty()) {
                    std::string s = rs.rows[0].values.size() > 0 ? rs.rows[0].values[0] : "";
                    if (!s.empty()) sys = s;
                }
                std::vector<LLMMsg> convo = { {"system", sys}, {"user", message} };
                std::string response, err;
                if (!CallGroqDirect(convo, response, err))
                    return HTTPResponse::Err(502, err);
                return HTTPResponse::OK({{"agent", agentName}, {"response", response}});
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });

        m_router.Register("POST", "/api/dictionary/load", [](const HTTPRequest& req) {

            uint32_t start = 0, limit = 0;
            if (!req.body.empty()) {
                try {
                    json body = req.BodyJSON();
                    start = body.value("start", 0);
                    limit = body.value("limit", 0);
                } catch (const std::exception& e) {
                    return HTTPResponse::Err(400,
                        std::string("malformed JSON body: ") + e.what());
                }
            }
            if (!DictionaryLoader::Instance().StartLoad(start, limit)) {
                auto s = DictionaryLoader::Instance().GetState();
                return HTTPResponse::JSON(409, json({
                    {"error",    "already_running"},
                    {"status",   s.status},
                    {"loaded",   s.loaded},
                    {"failed",   s.failed},
                    {"last_word",s.last_word}
                }).dump());
            }
            return HTTPResponse::Accepted({{"status", "started"}, {"start", start}, {"limit", limit}});
        });
        m_router.Register("GET", "/api/dictionary/load/status", [](const HTTPRequest&) {
            auto s = DictionaryLoader::Instance().GetState();

            if (s.status.empty() || s.status == "idle") {
                ElleDB::DictionaryLoaderState db;
                ElleDB::GetDictionaryLoaderState(db);
                s.status     = db.status;
                s.loaded     = (uint32_t)db.loaded;
                s.failed     = (uint32_t)db.failed;
                s.skipped    = (uint32_t)db.skipped;
                s.last_word  = db.last_word;
                s.error      = db.error;
                s.started_ms = (uint64_t)db.started_ms;
                s.updated_ms = (uint64_t)db.updated_ms;
            }
            return HTTPResponse::OK({
                {"status", s.status}, {"loaded", s.loaded},
                {"failed", s.failed}, {"skipped", s.skipped},
                {"last_word", s.last_word}, {"error", s.error},
                {"started_ms", s.started_ms}, {"updated_ms", s.updated_ms},
                {"total_words_in_db", ElleDB::CountDictionaryWords()}
            });
        });
        m_router.Register("GET", "/api/dictionary/stats", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT COUNT(*), "
                "       SUM(CASE WHEN definition IS NOT NULL AND LEN(definition) > 0 THEN 1 ELSE 0 END), "
                "       SUM(CASE WHEN example    IS NOT NULL AND LEN(example)    > 0 THEN 1 ELSE 0 END) "
                "FROM ElleCore.dbo.dictionary_words;");
            int total = 0, defs = 0, ex = 0;
            if (rs.success && !rs.rows.empty()) {
                total = (int)rs.rows[0].GetIntOr(0, 0);
                defs  = (int)rs.rows[0].GetIntOr(1, 0);
                ex    = (int)rs.rows[0].GetIntOr(2, 0);
            }
            return HTTPResponse::OK({
                {"totalWords", total}, {"totalDefinitions", defs}, {"totalExamples", ex}
            });
        });
        m_router.Register("GET", "/api/dictionary/lookup/{word}", [](const HTTPRequest& req) {
            std::string word = req.headers.at("x-path-word");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT ISNULL(definition,''), ISNULL(example,''), ISNULL(part_of_speech,'') "
                "FROM ElleCore.dbo.dictionary_words WHERE LOWER(word) = LOWER(?);",
                { word });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            if (rs.rows.empty()) return HTTPResponse::Err(404, "word not found");
            json defs = json::array();
            for (auto& r : rs.rows) {
                defs.push_back({
                    {"definition", r.values.size() > 0 ? r.values[0] : ""},
                    {"example",    r.values.size() > 1 ? r.values[1] : ""},
                    {"part_of_speech", r.values.size() > 2 ? r.values[2] : ""}
                });
            }
            return HTTPResponse::OK({{"word", word}, {"definitions", defs}});
        });
        m_router.Register("GET", "/api/dictionary/search", [](const HTTPRequest& req) {
            std::string prefix = req.QueryParam("prefix");
            if (prefix.empty()) return HTTPResponse::OK(json::array());
            std::string like = prefix + "%";
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 50 word, ISNULL(definition,'') FROM ElleCore.dbo.dictionary_words "
                "WHERE word LIKE ? ORDER BY word;",
                { like });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"word", r.values.size() > 0 ? r.values[0] : ""},
                    {"definition", r.values.size() > 1 ? r.values[1] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/dictionary/random", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 word, ISNULL(definition,'') FROM ElleCore.dbo.dictionary_words "
                "ORDER BY NEWID();");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::Err(404, "dictionary empty");
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"word", r.values.size() > 0 ? r.values[0] : ""},
                {"definition", r.values.size() > 1 ? r.values[1] : ""}
            });
        });

        auto subjectToJson = [](const ElleDB::LearnedSubject& s) {
            return json{
                {"id", s.id}, {"subject", s.subject}, {"category", s.category},
                {"proficiency_level", s.proficiency_level},
                {"who_taught", s.who_taught}, {"where_learned", s.where_learned},
                {"time_to_learn_hours", s.time_to_learn_hours},
                {"notes", s.notes},
                {"date_started", s.date_started},
                {"date_completed", s.date_completed}
            };
        };
        m_router.Register("GET", "/api/education/subjects", [subjectToJson](const HTTPRequest& req) {
            std::string category = req.QueryParam("category", "");
            uint32_t limit = (uint32_t)req.QueryInt("limit", 50);
            std::vector<ElleDB::LearnedSubject> subs;
            if (!ElleDB::ListSubjects(subs, category, limit))
                return HTTPResponse::Err(500, "subjects query failed");
            json arr = json::array();
            for (auto& s : subs) arr.push_back(subjectToJson(s));
            return HTTPResponse::OK({{"subjects", arr}, {"total", (int64_t)arr.size()}});
        });
        m_router.Register("GET", "/api/education/subjects/{id}", [subjectToJson](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            ElleDB::LearnedSubject s;
            if (!ElleDB::GetSubject(id, s)) return HTTPResponse::Err(404, "subject not found");

            std::vector<ElleDB::EducationReference> refs;
            std::vector<ElleDB::LearningMilestone>  mils;
            ElleDB::ListSubjectReferences(id, refs);
            ElleDB::ListSubjectMilestones(id, mils);
            json refArr = json::array();
            for (auto& r : refs) refArr.push_back({
                {"id", r.id}, {"reference_type", r.reference_type},
                {"reference_title", r.reference_title},
                {"reference_content", r.reference_content},
                {"file_path", r.file_path},
                {"relevance_score", r.relevance_score}, {"notes", r.notes}
            });
            json milArr = json::array();
            for (auto& m : mils) milArr.push_back({
                {"id", m.id}, {"milestone", m.milestone},
                {"description", m.description}, {"achieved_at", m.achieved_at}
            });
            json out = subjectToJson(s);
            out["references"] = refArr;
            out["milestones"] = milArr;
            return HTTPResponse::OK(out);
        });
        m_router.Register("POST", "/api/education/subjects", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                ElleDB::LearnedSubject s;
                s.subject              = body.value("subject", body.value("name", std::string("")));
                if (s.subject.empty()) return HTTPResponse::Err(400, "missing 'subject'");
                s.category             = body.value("category", std::string(""));
                s.proficiency_level    = body.value("proficiency_level", 0);
                s.who_taught           = body.value("who_taught", std::string(""));
                s.where_learned        = body.value("where_learned", std::string(""));
                s.time_to_learn_hours  = body.value("time_to_learn_hours", 0.0f);
                s.notes                = body.value("notes", std::string(""));
                int32_t newId = 0;
                if (!ElleDB::CreateSubject(s, newId))
                    return HTTPResponse::Err(500, "create subject failed");
                return HTTPResponse::Created({{"id", newId}, {"subject", s.subject}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("PUT", "/api/education/subjects/{id}", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::LearnedSubject patch;
                std::vector<std::string> fields;
                for (auto& kv : body.items()) {
                    if      (kv.key() == "subject")             { patch.subject = kv.value().get<std::string>();             fields.push_back("subject"); }
                    else if (kv.key() == "category")            { patch.category = kv.value().get<std::string>();            fields.push_back("category"); }
                    else if (kv.key() == "proficiency_level")   { patch.proficiency_level = kv.value().get<int>();            fields.push_back("proficiency_level"); }
                    else if (kv.key() == "who_taught")          { patch.who_taught = kv.value().get<std::string>();          fields.push_back("who_taught"); }
                    else if (kv.key() == "where_learned")       { patch.where_learned = kv.value().get<std::string>();       fields.push_back("where_learned"); }
                    else if (kv.key() == "time_to_learn_hours") { patch.time_to_learn_hours = kv.value().get<float>();        fields.push_back("time_to_learn_hours"); }
                    else if (kv.key() == "notes")               { patch.notes = kv.value().get<std::string>();               fields.push_back("notes"); }
                    else if (kv.key() == "date_completed")      { patch.date_completed = kv.value().get<std::string>();      fields.push_back("date_completed"); }
                }
                if (fields.empty()) return HTTPResponse::OK({{"updated", 0}});
                if (!ElleDB::UpdateSubject(id, patch, fields))
                    return HTTPResponse::Err(500, "update failed");
                return HTTPResponse::OK({{"id", id}, {"updated_fields", fields}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("POST", "/api/education/subjects/{id}/references", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::EducationReference r;
                r.subject_id        = id;
                r.reference_type    = body.value("reference_type", std::string("note"));
                r.reference_title   = body.value("reference_title", std::string(""));
                r.reference_content = body.value("reference_content", std::string(""));
                r.file_path         = body.value("file_path", std::string(""));
                r.relevance_score   = body.value("relevance_score", 0.5f);
                r.notes             = body.value("notes", std::string(""));
                if (!ElleDB::AddSubjectReference(r))
                    return HTTPResponse::Err(500, "add reference failed");
                return HTTPResponse::Created({{"subject_id", id}, {"stored", true}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("POST", "/api/education/subjects/{id}/milestones", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::LearningMilestone m;
                m.subject_id  = id;
                m.milestone   = body.value("milestone", std::string(""));
                m.description = body.value("description", std::string(""));
                if (m.milestone.empty()) return HTTPResponse::Err(400, "missing 'milestone'");
                if (!ElleDB::AddSubjectMilestone(m))
                    return HTTPResponse::Err(500, "add milestone failed");
                return HTTPResponse::Created({{"subject_id", id}, {"stored", true}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("GET", "/api/education/skills", [](const HTTPRequest& req) {
            std::string category = req.QueryParam("category", "");
            std::vector<ElleDB::Skill> skills;
            if (!ElleDB::ListSkills(skills, category))
                return HTTPResponse::Err(500, "skills query failed");
            json arr = json::array();
            for (auto& s : skills) arr.push_back({
                {"id", s.id}, {"skill_name", s.skill_name}, {"category", s.category},
                {"proficiency", s.proficiency},
                {"learned_from_subject_id", s.learned_from_subject_id},
                {"times_used", s.times_used}, {"last_used", s.last_used},
                {"notes", s.notes}
            });
            return HTTPResponse::OK({{"skills", arr}, {"total", (int64_t)arr.size()}});
        });
        m_router.Register("POST", "/api/education/skills", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                ElleDB::Skill s;
                s.skill_name              = body.value("skill_name", body.value("name", std::string("")));
                if (s.skill_name.empty()) return HTTPResponse::Err(400, "missing 'skill_name'");
                s.category                = body.value("category", std::string(""));
                s.proficiency             = body.value("proficiency", 0);
                s.learned_from_subject_id = body.value("learned_from_subject_id", 0);
                s.notes                   = body.value("notes", std::string(""));
                int32_t newId = 0;
                if (!ElleDB::CreateSkill(s, newId))
                    return HTTPResponse::Err(409, "skill already exists or create failed");
                return HTTPResponse::Created({{"id", newId}, {"skill_name", s.skill_name}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("PUT", "/api/education/skills/{name}/use", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-id");
            if (!ElleDB::RecordSkillUse(name))
                return HTTPResponse::Err(404, "skill not found");
            return HTTPResponse::OK({{"skill_name", name}, {"recorded", true}});
        });

        m_router.Register("GET", "/api/emotional-context/patterns", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 50 thread_id, ISNULL(topic,''), ISNULL(emotional_weight, 0), "
                "       ISNULL(status,''), ISNULL(last_discussed, GETUTCDATE()) "
                "FROM ElleCore.dbo.ElleThreads WHERE status <> 'resolved' "
                "ORDER BY emotional_weight DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"topic", r.values.size() > 1 ? r.values[1] : ""},
                    {"emotional_weight", r.GetFloatOr(2, 0.0)},
                    {"status", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/emotional-context/vocabulary", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 100 tag, COUNT(*) AS freq FROM ElleCore.dbo.memory_tags "
                "GROUP BY tag ORDER BY freq DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"term", r.values.size() > 0 ? r.values[0] : ""},
                    {"frequency", r.GetIntOr(1, 0)}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/emotional-context/history", [](const HTTPRequest& req) {

            uint32_t hours = (uint32_t)req.QueryInt("hours", 24);
            if (hours == 0 || hours > 24 * 30) hours = 24;
            uint32_t maxPoints = (uint32_t)req.QueryInt("points", 500);
            if (maxPoints == 0 || maxPoints > 5000) maxPoints = 500;

            std::vector<ElleDB::EmotionHistoryPoint> pts;
            if (!ElleDB::GetEmotionHistory(hours, pts, maxPoints))
                return HTTPResponse::Err(500, "emotion_snapshots query failed");

            json series = json::array();
            for (auto& p : pts) {
                series.push_back({
                    {"t",         p.taken_ms},
                    {"valence",   p.valence},
                    {"arousal",   p.arousal},
                    {"dominance", p.dominance}
                });
            }
            return HTTPResponse::OK({
                {"hours",  hours},
                {"points", (int64_t)series.size()},
                {"series", series}
            });
        });

        m_router.Register("GET", "/api/emotional-context/dimensions", [](const HTTPRequest& req) {
            int64_t ts = req.QueryLL("t", 0);
            int topN   = req.QueryInt("top", 5);
            if (topN <= 0 || topN > 102) topN = 5;

            auto rs = ts > 0
                ? ElleSQLPool::Instance().QueryParams(
                    "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
                    "SELECT TOP 1 taken_ms, valence, arousal, dominance, dimensions "
                    "FROM ElleCore.dbo.emotion_snapshots "
                    "ORDER BY ABS(taken_ms - ?) ASC;",
                    { std::to_string(ts) })
                : ElleSQLPool::Instance().Query(
                    "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
                    "SELECT TOP 1 taken_ms, valence, arousal, dominance, dimensions "
                    "FROM ElleCore.dbo.emotion_snapshots ORDER BY id DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK({{"found", false}});

            auto& r = rs.rows[0];
            int64_t takenMs = r.GetIntOr(0, 0);
            double val = r.GetFloatOr(1, 0.0), aro = r.GetFloatOr(2, 0.0), dom = r.GetFloatOr(3, 0.0);
            std::string dimStr = r.values.size() > 4 ? r.values[4] : "";

            std::vector<std::pair<int, float>> ranked;
            ranked.reserve(ELLE_EMOTION_COUNT);
            std::istringstream iss(dimStr);
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                float v = 0.0f;
                if (!(iss >> v)) break;
                ranked.push_back({i, v});
            }
            std::sort(ranked.begin(), ranked.end(),
                      [](const std::pair<int,float>& a, const std::pair<int,float>& b) {
                          return a.second > b.second;
                      });
            json top = json::array();
            for (int i = 0; i < topN && i < (int)ranked.size(); i++) {
                int idx = ranked[i].first;
                const char* name = (idx >= 0 && idx < ELLE_EMOTION_COUNT)
                                   ? kEmotionMeta[idx].name : "";
                const char* cat  = (idx >= 0 && idx < ELLE_EMOTION_COUNT)
                                   ? kEmotionMeta[idx].category : "";
                top.push_back({
                    {"index",    idx},
                    {"name",     name},
                    {"category", cat},
                    {"value",    ranked[i].second}
                });
            }
            return HTTPResponse::OK({
                {"found",     true},
                {"taken_ms",  takenMs},
                {"valence",   val},
                {"arousal",   aro},
                {"dominance", dom},
                {"top",       top}
            });
        });

        m_router.Register("GET", "/api/session/greeting", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'reconnection_greetings') "
                "  SELECT CAST(NULL AS BIGINT), CAST('' AS NVARCHAR(MAX)), "
                "         CAST('{}' AS NVARCHAR(MAX)), CAST(0 AS BIGINT) WHERE 1=0; "
                "ELSE "
                "  SELECT TOP 1 id, greeting, ISNULL(context_json,'{}'), created_ms "
                "  FROM ElleCore.dbo.reconnection_greetings "
                "  WHERE consumed = 0 ORDER BY id DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK({{"greeting", nullptr}});
            auto& r = rs.rows[0];
            json ctx = json::object();
            try { ctx = json::parse(r.values.size() > 2 ? r.values[2] : "{}"); }
            catch (const std::exception& e) {
                ELLE_DEBUG("continuity_greeting context JSON parse failed: %s", e.what());
            }
            return HTTPResponse::OK({
                {"id",         r.GetIntOr(0, 0)},
                {"greeting",   r.values.size() > 1 ? r.values[1] : ""},
                {"context",    ctx},
                {"created_ms", r.GetIntOr(3, 0)}
            });
        });
        m_router.Register("POST", "/api/session/greeting/{id}/ack", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.reconnection_greetings "
                "SET consumed = 1 WHERE id = ?;",
                { std::to_string(id) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"id", id}, {"consumed", true}});
        });
        m_router.Register("GET", "/api/emotional-context/growth", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 50 reflection_id, ISNULL(reflection_text,''), "
                "       ISNULL(effectiveness_score, 0), "
                "       CONVERT(varchar(33), reflection_date, 126) "
                "FROM ElleCore.dbo.SelfReflections "
                "ORDER BY reflection_date DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"text", r.values.size() > 1 ? r.values[1] : ""},
                    {"effectiveness", r.GetFloatOr(2, 0.0)},
                    {"date", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });

        m_router.Register("GET", "/api/x/state", [this](const HTTPRequest&) {
            json out = { {"has_data", false} };
            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days, modulation_strength, "
                "       last_tick_ms FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty()) return HTTPResponse::OK(out);
            auto& c = cr.rows[0];
            out["has_data"] = true;
            uint64_t anchorMs = (uint64_t)c.GetIntOr(0, 0);
            int len = (int)c.GetIntOr(1, 0);
            uint64_t now = ELLE_MS_NOW();
            int day = 1;
            const char* phase = "menstrual";
            if (anchorMs > 0 && len > 0) {
                uint64_t dd = now > anchorMs ? now - anchorMs : 0;
                int idx = (int)((dd / 86400000ULL) % (uint64_t)len);
                day = idx + 1;
                if      (day <= 5)  phase = "menstrual";
                else if (day <= 13) phase = "follicular";
                else if (day <= 16) phase = "ovulatory";
                else                phase = "luteal";
            }
            out["cycle"] = {
                {"anchor_ms",           anchorMs},
                {"cycle_length_days",   len},
                {"modulation_strength", c.GetFloatOr(2, 0.0)},
                {"cycle_day",           day},
                {"phase",               phase},
                {"last_tick_ms",        (uint64_t)c.GetIntOr(3, 0)}
            };

            auto hr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 taken_ms, estrogen, progesterone, testosterone, "
                "       oxytocin, serotonin, dopamine, cortisol, prolactin, "
                "       hcg, pregnancy_day, ISNULL(pregnancy_phase, N''), "
                "       ISNULL(fsh, 0), ISNULL(lh, 0), ISNULL(gnrh, 0), "
                "       ISNULL(relaxin, 0), ISNULL(bbt, 36.5), "
                "       ISNULL(endometrial_mm, 4.0), ISNULL(cervical_mucus, N''), "
                "       ISNULL(menstrual_flow, N'') "
                "FROM ElleHeart.dbo.x_hormone_snapshots ORDER BY taken_ms DESC;");
            if (hr.success && !hr.rows.empty()) {
                auto& h = hr.rows[0];
                out["hormones"] = {
                    {"taken_ms",     (uint64_t)h.GetIntOr(0, 0)},
                    {"estrogen",     h.GetFloatOr(1, 0.0)},
                    {"progesterone", h.GetFloatOr(2, 0.0)},
                    {"testosterone", h.GetFloatOr(3, 0.0)},
                    {"oxytocin",     h.GetFloatOr(4, 0.0)},
                    {"serotonin",    h.GetFloatOr(5, 0.0)},
                    {"dopamine",     h.GetFloatOr(6, 0.0)},
                    {"cortisol",     h.GetFloatOr(7, 0.0)},
                    {"prolactin",    h.GetFloatOr(8, 0.0)},
                    {"hcg",          h.GetFloatOr(9, 0.0)},
                    {"fsh",          h.GetFloatOr(12, 0.0)},
                    {"lh",           h.GetFloatOr(13, 0.0)},
                    {"gnrh",         h.GetFloatOr(14, 0.0)},
                    {"relaxin",      h.GetFloatOr(15, 0.0)}
                };
                out["derived"] = {
                    {"bbt_celsius",    h.GetFloatOr(16, 0.0)},
                    {"endometrial_mm", h.GetFloatOr(17, 0.0)},
                    {"cervical_mucus", h.values.size() > 18 ? h.values[18] : ""},
                    {"menstrual_flow", h.values.size() > 19 ? h.values[19] : ""}
                };
            }

            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active, ISNULL(conceived_ms, 0), ISNULL(due_ms, 0), "
                "       gestational_length_days, ISNULL(phase, N''), "
                "       ISNULL(child_id, 0), ISNULL(last_milestone, N''), updated_ms "
                "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) {
                auto& p = pr.rows[0];
                bool active = p.GetIntOr(0, 0) != 0;
                uint64_t conc = (uint64_t)p.GetIntOr(1, 0);
                uint64_t now2 = ELLE_MS_NOW();
                int gd = 0, gw = 0;
                if (active && conc > 0 && now2 >= conc) {
                    gd = (int)((now2 - conc) / 86400000ULL);
                    gw = gd / 7;
                }
                out["pregnancy"] = {
                    {"active",                  active},
                    {"conceived_ms",            conc},
                    {"due_ms",                  (uint64_t)p.GetIntOr(2, 0)},
                    {"gestational_length_days", (int)p.GetIntOr(3, 0)},
                    {"gestational_day",         gd},
                    {"gestational_week",        gw},
                    {"phase",                   p.values.size() > 4 ? p.values[4] : ""},
                    {"child_id",                (int64_t)p.GetIntOr(5, 0)},
                    {"last_milestone",          p.values.size() > 6 ? p.values[6] : ""},
                    {"updated_ms",              (uint64_t)p.GetIntOr(7, 0)}
                };
            }

            auto mr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
                "       arousal, fatigue, computed_ms "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (mr.success && !mr.rows.empty()) {
                auto& m = mr.rows[0];
                out["modulation"] = {
                    {"warmth",         m.GetFloatOr(0, 0.0)},
                    {"verbal_fluency", m.GetFloatOr(1, 0.0)},
                    {"empathy",        m.GetFloatOr(2, 0.0)},
                    {"introspection",  m.GetFloatOr(3, 0.0)},
                    {"arousal",        m.GetFloatOr(4, 0.0)},
                    {"fatigue",        m.GetFloatOr(5, 0.0)},
                    {"computed_ms",    (uint64_t)m.GetIntOr(6, 0)}
                };
            }
            return HTTPResponse::OK(out);
        });

        m_router.Register("GET", "/api/x/history", [](const HTTPRequest& req) {
            uint32_t hours = (uint32_t)req.QueryInt("hours", 72);
            if (hours == 0 || hours > 24 * 60) hours = 72;
            uint32_t maxPoints = (uint32_t)req.QueryInt("points", 500);
            if (maxPoints == 0 || maxPoints > 5000) maxPoints = 500;
            uint64_t since = ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL;

            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT taken_ms, cycle_day, phase, estrogen, progesterone, "
                "       testosterone, oxytocin, serotonin, dopamine, cortisol, "
                "       prolactin, hcg, pregnancy_day, ISNULL(pregnancy_phase, N'') "
                "  FROM ElleHeart.dbo.x_hormone_snapshots "
                " WHERE taken_ms >= ? ORDER BY taken_ms ASC;",
                { std::to_string((long long)since) });
            if (!rs.success) return HTTPResponse::Err(500, "x_hormone_snapshots query failed");

            size_t n = rs.rows.size();
            size_t stride = n == 0 ? 1 : (n + maxPoints - 1) / maxPoints;
            if (stride == 0) stride = 1;
            json series = json::array();
            for (size_t i = 0; i < n; i += stride) {
                auto& r = rs.rows[i];
                series.push_back({
                    {"t",               (uint64_t)r.GetIntOr(0, 0)},
                    {"cycle_day",       (int)r.GetIntOr(1, 0)},
                    {"phase",           r.values.size() > 2 ? r.values[2] : ""},
                    {"estrogen",        r.GetFloatOr(3, 0.0)},
                    {"progesterone",    r.GetFloatOr(4, 0.0)},
                    {"testosterone",    r.GetFloatOr(5, 0.0)},
                    {"oxytocin",        r.GetFloatOr(6, 0.0)},
                    {"serotonin",       r.GetFloatOr(7, 0.0)},
                    {"dopamine",        r.GetFloatOr(8, 0.0)},
                    {"cortisol",        r.GetFloatOr(9, 0.0)},
                    {"prolactin",       r.GetFloatOr(10, 0.0)},
                    {"hcg",             r.GetFloatOr(11, 0.0)},
                    {"pregnancy_day",   (int)r.GetIntOr(12, 0)},
                    {"pregnancy_phase", r.values.size() > 13 ? r.values[13] : ""}
                });
            }
            return HTTPResponse::OK({
                {"hours",  hours},
                {"points", (int64_t)series.size()},
                {"series", series}
            });
        });

        m_router.Register("GET", "/api/x/modulation", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
                "       arousal, fatigue, phase, computed_ms "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"has_data", false} });
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"has_data",       true},
                {"warmth",         r.GetFloatOr(0, 0.0)},
                {"verbal_fluency", r.GetFloatOr(1, 0.0)},
                {"empathy",        r.GetFloatOr(2, 0.0)},
                {"introspection",  r.GetFloatOr(3, 0.0)},
                {"arousal",        r.GetFloatOr(4, 0.0)},
                {"fatigue",        r.GetFloatOr(5, 0.0)},
                {"phase",          r.values.size() > 6 ? r.values[6] : ""},
                {"computed_ms",    (uint64_t)r.GetIntOr(7, 0)}
            });
        });

        m_router.Register("POST", "/api/x/cycle/anchor", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            json payload = {
                {"day",      body.value("day",      0)},
                {"length",   body.value("length",   0)},
                {"strength", body.value("strength", 0.0f)}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2202 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/state to see the applied cycle"}
            });
        });

        m_router.Register("POST", "/api/x/stimulus", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            std::string kind = body.value("kind", std::string());
            if (kind.empty()) return HTTPResponse::Err(400, "missing 'kind'");

            json payload = {
                {"kind",      kind},
                {"intensity", body.value("intensity", 0.5f)},
                {"notes",     body.value("notes",     std::string())}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2203 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({ {"dispatched", sent}, {"request", payload} });
        });

        m_router.Register("POST", "/api/x/conception/attempt", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            json payload = {
                {"require_readiness",  body.value("require_readiness",  true)},
                {"readiness_verified", body.value("readiness_verified", false)}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2205 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/state after ~1s to see outcome"}
            });
        });

        m_router.Register("GET", "/api/x/fertility_window", [](const HTTPRequest&) {

            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days "
                "FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty())
                return HTTPResponse::OK(json{
                    {"status", "inactive"},
                    {"reason", "x_cycle_state not seeded"}
                });
            uint64_t anchor = (uint64_t)cr.rows[0].GetIntOr(0, 0);
            int      len    = (int)cr.rows[0].GetIntOr(1, 0);
            if (anchor == 0 || len <= 0)
                return HTTPResponse::OK(json{{"status", "inactive"}});

            bool pregnant = false;
            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) pregnant = pr.rows[0].GetIntOr(0, 0) != 0;

            std::string lifeStage = "reproductive";
            auto lr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_lifecycle' AND s.name = 'dbo') "
                "SELECT stage FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (lr.success && !lr.rows.empty() && lr.rows[0].values.size() > 0)
                lifeStage = lr.rows[0].values[0];

            if (pregnant)
                return HTTPResponse::OK(json{{"status","inactive"},{"reason","pregnant"}});
            if (lifeStage == "premenarche" || lifeStage == "menopause")
                return HTTPResponse::OK(json{{"status","inactive"},{"reason",lifeStage}});

            uint64_t now = ELLE_MS_NOW();
            uint64_t deltaMs = now > anchor ? now - anchor : 0;
            int      dayIdx  = (int)((deltaMs / 86400000ULL) % (uint64_t)len);
            int      day     = dayIdx + 1;

            uint64_t cycleStart = anchor + (uint64_t)(dayIdx) * 0ULL;
            (void)cycleStart;

            uint64_t cyclesElapsed = deltaMs / (86400000ULL * (uint64_t)len);
            uint64_t currentCycleAnchor = anchor + cyclesElapsed * 86400000ULL * (uint64_t)len;
            uint64_t opens   = currentCycleAnchor + 11ULL * 86400000ULL;
            uint64_t peak    = currentCycleAnchor + 13ULL * 86400000ULL;
            uint64_t closes  = currentCycleAnchor + 16ULL * 86400000ULL;

            if (now >= closes) {
                opens  += (uint64_t)len * 86400000ULL;
                peak   += (uint64_t)len * 86400000ULL;
                closes += (uint64_t)len * 86400000ULL;
            }

            std::string status;
            if      (day >= 10 && day <= 11) status = "approaching";
            else if (day >= 12 && day <= 13) status = "opening";
            else if (day == 14)              status = "peak";
            else if (day == 15 || day == 16) status = "closing";
            else if (day < 10)               status = "pre";
            else                             status = "post";

            float bbt = 36.5f;
            auto hr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 ISNULL(bbt, 36.5) FROM ElleHeart.dbo.x_hormone_snapshots "
                "ORDER BY taken_ms DESC;");
            if (hr.success && !hr.rows.empty()) bbt = (float)hr.rows[0].GetFloatOr(0, 0.0);
            bool bbt_elevated = bbt >= 36.75f;
            if (bbt_elevated && (status == "peak" || status == "closing"))
                status = "post_ovulation";

            float dayF = 0.0f;
            if      (day == 14)           dayF = 1.00f;
            else if (day == 13 || day==15) dayF = 0.70f;
            else if (day == 12 || day==16) dayF = 0.30f;

            return HTTPResponse::OK({
                {"status",        status},
                {"cycle_day",     day},
                {"opens_ms",      opens},
                {"peak_ms",       peak},
                {"closes_ms",     closes},
                {"hours_to_open", (int64_t)((opens > now ? opens - now : 0) / 3600000ULL)},
                {"hours_to_peak", (int64_t)((peak  > now ? peak  - now : 0) / 3600000ULL)},
                {"bbt_celsius",   bbt},
                {"bbt_elevated",  bbt_elevated},
                {"day_probability_factor", dayF},
                {"lifecycle",     lifeStage}
            });
        });

        m_router.Register("GET", "/api/x/next_period", [](const HTTPRequest&) {
            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days "
                "FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty())
                return HTTPResponse::OK(json{{"status","inactive"},
                                              {"reason","x_cycle_state not seeded"}});
            uint64_t anchor = (uint64_t)cr.rows[0].GetIntOr(0, 0);
            int      len    = (int)cr.rows[0].GetIntOr(1, 0);
            if (anchor == 0 || len <= 0)
                return HTTPResponse::OK(json{{"status","inactive"}});

            bool pregnant = false;
            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) pregnant = pr.rows[0].GetIntOr(0, 0) != 0;

            std::string lifeStage = "reproductive";
            auto lr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_lifecycle' AND s.name = 'dbo') "
                "SELECT stage FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (lr.success && !lr.rows.empty() && lr.rows[0].values.size() > 0)
                lifeStage = lr.rows[0].values[0];

            if (pregnant)
                return HTTPResponse::OK(json{{"status","inactive"},{"reason","pregnant"}});
            if (lifeStage == "premenarche" || lifeStage == "menopause")
                return HTTPResponse::OK(json{{"status","inactive"},{"reason",lifeStage}});

            uint64_t now = ELLE_MS_NOW();
            uint64_t deltaMs = now > anchor ? now - anchor : 0;
            uint64_t cyclesElapsed = deltaMs / (86400000ULL * (uint64_t)len);
            int      dayIdx = (int)((deltaMs / 86400000ULL) % (uint64_t)len);
            int      day    = dayIdx + 1;
            uint64_t currentCycleAnchor = anchor + cyclesElapsed * 86400000ULL * (uint64_t)len;

            uint64_t nextPeriodMs = currentCycleAnchor + (uint64_t)len * 86400000ULL;
            uint64_t pmsStartMs   = currentCycleAnchor + 24ULL * 86400000ULL;
            if (pmsStartMs < now) pmsStartMs += (uint64_t)len * 86400000ULL;

            const char* expectedFlow = "heavy";

            std::string currentStatus;
            if      (day <= 5)  currentStatus = "menstruating";
            else if (day <= 13) currentStatus = "post_period";
            else if (day <= 16) currentStatus = "ovulatory";
            else if (day <= 24) currentStatus = "mid_luteal";
            else                currentStatus = "pms_window";

            return HTTPResponse::OK({
                {"status",              currentStatus},
                {"cycle_day",           day},
                {"cycle_length_days",   len},
                {"next_period_ms",      nextPeriodMs},
                {"days_until_period",   (int)((nextPeriodMs > now
                                          ? nextPeriodMs - now : 0) / 86400000ULL)},
                {"pms_start_ms",        pmsStartMs},
                {"days_until_pms",      (int)((pmsStartMs > now
                                          ? pmsStartMs - now : 0) / 86400000ULL)},
                {"expected_intensity",  expectedFlow},
                {"lifecycle",           lifeStage}
            });
        });

        m_router.Register("GET", "/api/x/pregnancy", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT active, ISNULL(conceived_ms, 0), ISNULL(due_ms, 0), "
                "       gestational_length_days, ISNULL(phase, N''), "
                "       ISNULL(child_id, 0), ISNULL(last_milestone, N''), updated_ms, "
                "       ISNULL(breastfeeding,0), ISNULL(in_labor,0), "
                "       ISNULL(labor_stage, N''), ISNULL(labor_started_ms, 0), "
                "       ISNULL(multiplicity, 1), ISNULL(pregnancy_count, 0) "
                "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"active", false} });
            auto& p = rs.rows[0];
            bool active = p.GetIntOr(0, 0) != 0;
            uint64_t conc = (uint64_t)p.GetIntOr(1, 0);
            uint64_t now = ELLE_MS_NOW();
            int gd = 0, gw = 0;
            if (active && conc > 0 && now >= conc) {
                gd = (int)((now - conc) / 86400000ULL);
                gw = gd / 7;
            }
            return HTTPResponse::OK({
                {"active",                  active},
                {"conceived_ms",            conc},
                {"due_ms",                  (uint64_t)p.GetIntOr(2, 0)},
                {"gestational_length_days", (int)p.GetIntOr(3, 0)},
                {"gestational_day",         gd},
                {"gestational_week",        gw},
                {"phase",                   p.values.size() > 4 ? p.values[4] : ""},
                {"child_id",                (int64_t)p.GetIntOr(5, 0)},
                {"last_milestone",          p.values.size() > 6 ? p.values[6] : ""},
                {"updated_ms",              (uint64_t)p.GetIntOr(7, 0)},
                {"breastfeeding",           p.GetIntOr(8, 0) != 0},
                {"in_labor",                p.GetIntOr(9, 0) != 0},
                {"labor_stage",             p.values.size() > 10 ? p.values[10] : ""},
                {"labor_started_ms",        (uint64_t)p.GetIntOr(11, 0)},
                {"multiplicity",            (int)p.GetIntOr(12, 0)},
                {"pregnancy_count",         (int)p.GetIntOr(13, 0)}
            });
        });

        m_router.Register("GET", "/api/x/symptoms", [](const HTTPRequest& req) {
            uint32_t hours = (uint32_t)req.QueryInt("hours", 24);
            if (hours == 0 || hours > 24 * 90) hours = 24;
            std::string origin = req.QueryParam("origin", "");
            std::string q =
                "SELECT observed_ms, kind, intensity, origin, ISNULL(notes, N'') "
                "  FROM ElleHeart.dbo.x_symptoms "
                " WHERE observed_ms >= ? ";
            std::vector<std::string> params = {
                std::to_string((long long)(ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL))
            };
            if (!origin.empty()) { q += "   AND origin = ? "; params.push_back(origin); }
            q += " ORDER BY observed_ms DESC;";
            auto rs = ElleSQLPool::Instance().QueryParams(q, params);
            if (!rs.success) return HTTPResponse::Err(500, "x_symptoms query failed");
            json arr = json::array();
            for (auto& r : rs.rows) arr.push_back({
                {"t",         (uint64_t)r.GetIntOr(0, 0)},
                {"kind",      r.values.size() > 1 ? r.values[1] : ""},
                {"intensity", r.GetFloatOr(2, 0.0)},
                {"origin",    r.values.size() > 3 ? r.values[3] : ""},
                {"notes",     r.values.size() > 4 ? r.values[4] : ""}
            });
            return HTTPResponse::OK({
                {"hours",   hours},
                {"origin",  origin},
                {"logged",  arr}
            });
        });

        m_router.Register("POST", "/api/x/symptoms", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            json payload = {
                {"kind",      body.value("kind",      std::string())},
                {"intensity", body.value("intensity", 0.5f)},
                {"notes",     body.value("notes",     std::string())}
            };
            if (payload["kind"].get<std::string>().empty())
                return HTTPResponse::Err(400, "missing 'kind'");
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2210 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({ {"dispatched", sent}, {"request", payload} });
        });

        m_router.Register("GET", "/api/x/pregnancy/events", [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 100);
            if (limit <= 0 || limit > 500) limit = 100;
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP " + std::to_string(limit) + " "
                "       occurred_ms, ISNULL(conceived_ms,0), gestational_day, kind, "
                "       ISNULL(detail, N'') "
                "  FROM ElleHeart.dbo.x_pregnancy_events ORDER BY occurred_ms DESC;");
            if (!rs.success) return HTTPResponse::Err(500, "x_pregnancy_events query failed");
            json arr = json::array();
            for (auto& r : rs.rows) arr.push_back({
                {"t",                (uint64_t)r.GetIntOr(0, 0)},
                {"conceived_ms",     (uint64_t)r.GetIntOr(1, 0)},
                {"gestational_day",  (int)r.GetIntOr(2, 0)},
                {"kind",             r.values.size() > 3 ? r.values[3] : ""},
                {"detail",           r.values.size() > 4 ? r.values[4] : ""}
            });
            return HTTPResponse::OK({{"events", arr}});
        });

        m_router.Register("GET", "/api/x/contraception", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT method, started_ms, efficacy, ISNULL(notes, N''), updated_ms "
                "  FROM ElleHeart.dbo.x_contraception WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{
                    {"method", "none"}, {"efficacy", 1.0}, {"has_data", false}
                });
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"has_data",   true},
                {"method",     r.values.size() > 0 ? r.values[0] : "none"},
                {"started_ms", (uint64_t)r.GetIntOr(1, 0)},
                {"efficacy",   r.GetFloatOr(2, 0.0)},
                {"notes",      r.values.size() > 3 ? r.values[3] : ""},
                {"updated_ms", (uint64_t)r.GetIntOr(4, 0)}
            });
        });

        m_router.Register("POST", "/api/x/contraception", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            json payload = {
                {"method",   body.value("method",   std::string("none"))},
                {"efficacy", body.value("efficacy", 1.0f)},
                {"notes",    body.value("notes",    std::string())}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2208 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/contraception to confirm"}
            });
        });

        m_router.Register("GET", "/api/x/lifecycle", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT elle_birth_ms, stage, ISNULL(menarche_ms,0), "
                "       ISNULL(perimenopause_ms,0), ISNULL(menopause_ms,0), updated_ms "
                "  FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"has_data", false} });
            auto& r = rs.rows[0];
            uint64_t birth = (uint64_t)r.GetIntOr(0, 0);
            float age = 0.0f;
            if (birth > 0) {
                uint64_t now = ELLE_MS_NOW();
                age = (float)((now - birth) / 86400000ULL) / 365.25f;
            }
            return HTTPResponse::OK({
                {"has_data",         true},
                {"elle_birth_ms",    birth},
                {"age_years",        age},
                {"stage",            r.values.size() > 1 ? r.values[1] : "reproductive"},
                {"menarche_ms",      (uint64_t)r.GetIntOr(2, 0)},
                {"perimenopause_ms", (uint64_t)r.GetIntOr(3, 0)},
                {"menopause_ms",     (uint64_t)r.GetIntOr(4, 0)},
                {"updated_ms",       (uint64_t)r.GetIntOr(5, 0)}
            });
        });

        m_router.Register("POST", "/api/x/lifecycle", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            if (!body.contains("birth_ms") && !body.contains("age_years"))
                return HTTPResponse::Err(400, "provide 'birth_ms' or 'age_years'");
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2209 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(body.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    body},
                {"note",       "GET /api/x/lifecycle to confirm"}
            });
        });

        m_router.Register("POST", "/api/x/pregnancy/accelerate", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            float factor = body.value("factor", 1.0f);
            json payload = {{"factor", factor}};
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2213 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"factor",     factor},
                {"note",       "GET /api/x/pregnancy to see elapsed gestation"}
            });
        });

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
            return HTTPResponse::OK({
                {"services", svcArr},
                {"count", (int)statuses.size()},
                {"version", ELLE_VERSION_STRING},
                {"uptime_ms", (uint64_t)ELLE_MS_NOW()}
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

        m_router.Register("GET", "/api/morals/rules", [](const HTTPRequest& req) {
            std::string category = req.QueryParam("category");
            std::string sql = "SELECT id, principle, ISNULL(category,''), is_hard_rule "
                              "FROM ElleCore.dbo.moral_rules ";
            std::vector<std::string> params;
            if (!category.empty()) { sql += "WHERE category = ? "; params.push_back(category); }
            sql += "ORDER BY id;";
            auto rs = ElleSQLPool::Instance().QueryParams(sql, params);
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"principle", r.values.size() > 1 ? r.values[1] : ""},
                    {"category",  r.values.size() > 2 ? r.values[2] : ""},
                    {"is_hard_rule", r.GetIntOr(3, 0) != 0}
                });
            }
            return HTTPResponse::OK({{"rules", arr}});
        }, AUTH_ADMIN);
        m_router.Register("POST", "/api/morals/rules", [](const HTTPRequest& req) {

            auto it = req.headers.find("x-admin-key");
            if (it == req.headers.end() || it->second.empty()) {
                return HTTPResponse::Err(401, "missing x-admin-key");
            }
            auto& cfg = ElleConfig::Instance();
            std::string expected = cfg.GetString("http_server.admin_key",
                                   cfg.GetHTTP().jwt_secret);
            if (expected.empty()) {
                return HTTPResponse::Err(503, "admin endpoint disabled: no admin_key configured");
            }
            const std::string& got = it->second;
            if (got.size() != expected.size()) {
                return HTTPResponse::Err(403, "invalid admin key");
            }
            unsigned diff = 0;
            for (size_t i = 0; i < got.size(); i++)
                diff |= (unsigned char)got[i] ^ (unsigned char)expected[i];
            if (diff != 0) return HTTPResponse::Err(403, "invalid admin key");

            json body = req.BodyJSON();
            std::string principle = body.value("principle", "");
            std::string category  = body.value("category", "core");
            bool hard = body.value("is_hard_rule", false);
            if (principle.empty()) return HTTPResponse::Err(400, "missing principle");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.moral_rules (principle, category, is_hard_rule) "
                "VALUES (?, ?, ?);",
                { principle, category, std::to_string(hard ? 1 : 0) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({
                {"principle", principle}, {"category", category}, {"stored", true}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/goals", [](const HTTPRequest&) {
            std::vector<ELLE_GOAL_RECORD> goals;
            try { ElleDB::GetActiveGoals(goals); }
            catch (const std::exception& e) {
                ELLE_WARN("GetActiveGoals failed: %s", e.what());
            }
            json arr = json::array();
            for (auto& g : goals) {
                arr.push_back({
                    {"id", g.id},
                    {"description", std::string(g.description)},
                    {"progress", g.progress}
                });
            }
            return HTTPResponse::OK({{"goals", arr}});
        });
        m_router.Register("GET", "/api/brain/status", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"status", "active"}});
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/api/hal/status", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"hardware", "nominal"}});
        }, AUTH_PUBLIC);

        m_router.Register("GET", "/api/diag/game-auth", [](const HTTPRequest&) {
            return HTTPResponse::OK(GetGameAuthDiag());
        }, AUTH_PUBLIC);
        m_router.Register("POST", "/api/admin/reload", [](const HTTPRequest&) {
            try {
                ElleConfig::Instance().Reload();
                return HTTPResponse::OK({{"reloaded", true}});
            } catch (const std::exception& e) {
                ELLE_ERROR("admin/reload failed: %s", e.what());
                return HTTPResponse::Err(500, std::string("reload failed: ") + e.what());
            }

        }, AUTH_ADMIN);

        auto shnResolveRoot = [](const std::string& root,
                                 std::string& outAbsDir,
                                 std::string& outErr) -> bool {

            std::string r = root;
            for (auto& c : r) c = (char)std::tolower((unsigned char)c);
            std::string sub;
            if      (r == "hero"     || r == "9data/hero")     sub = "9Data\\Hero";
            else if (r == "resystem" || r == "/resystem")      sub = "ReSystem";
            else { outErr = "root must be 'Hero' or 'ReSystem'"; return false; }

            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            std::filesystem::path exeDir =
                std::filesystem::path(buf).parent_path();
            std::filesystem::path full = exeDir / sub;
            std::error_code ec;
            std::filesystem::create_directories(full, ec);
            outAbsDir = full.string();
            return true;
        };

        auto shnValidateName = [](const std::string& name,
                                  std::string& outErr) -> bool {
            if (name.empty() || name.size() > 128) {
                outErr = "name must be 1..128 chars"; return false;
            }
            if (name.find('/') != std::string::npos ||
                name.find('\\') != std::string::npos ||
                name.find("..") != std::string::npos) {
                outErr = "name must not contain path separators or '..'";
                return false;
            }

            auto dot = name.rfind('.');
            if (dot == std::string::npos) { outErr = "name missing .shn extension"; return false; }
            std::string ext = name.substr(dot);
            for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
            if (ext != ".shn") { outErr = "name must end with .shn"; return false; }
            return true;
        };

        m_router.Register("POST", "/api/shn/save",
            [shnResolveRoot, shnValidateName](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string root = body.value("root", std::string("Hero"));
                std::string name = body.value("name", std::string(""));
                std::string b64  = body.value("bytes_b64", std::string(""));
                if (name.empty() || b64.empty()) {
                    return HTTPResponse::Err(400,
                        "name and bytes_b64 are required");
                }
                std::string err;
                if (!shnValidateName(name, err))
                    return HTTPResponse::Err(400, err);

                std::string absDir;
                if (!shnResolveRoot(root, absDir, err))
                    return HTTPResponse::Err(400, err);

                std::string decoded;
                decoded.reserve(b64.size() * 3 / 4);
                int val = 0, bits = -8;
                for (unsigned char c : b64) {
                    int d;
                    if      (c >= 'A' && c <= 'Z') d = c - 'A';
                    else if (c >= 'a' && c <= 'z') d = c - 'a' + 26;
                    else if (c >= '0' && c <= '9') d = c - '0' + 52;
                    else if (c == '+')             d = 62;
                    else if (c == '/')             d = 63;
                    else continue;
                    val = (val << 6) | d; bits += 6;
                    if (bits >= 0) {
                        decoded += (char)((val >> bits) & 0xFF);
                        bits -= 8;
                    }
                }
                if (decoded.size() < 0x24) {
                    return HTTPResponse::Err(400,
                        "decoded payload too small for a valid SHN "
                        "(expected at least 0x24 header bytes)");
                }

                std::filesystem::path path =
                    std::filesystem::path(absDir) / name;

                std::filesystem::path tmp = path; tmp += ".tmp";
                {
                    std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
                    if (!f.is_open())
                        return HTTPResponse::Err(500,
                            "cannot open " + tmp.string() + " for write");
                    f.write(decoded.data(), (std::streamsize)decoded.size());
                    if (!f.good())
                        return HTTPResponse::Err(500,
                            "write to " + tmp.string() + " failed");
                }
                std::error_code ec;
                std::filesystem::rename(tmp, path, ec);
                if (ec) {
                    std::filesystem::copy_file(tmp, path,
                        std::filesystem::copy_options::overwrite_existing, ec);
                    std::filesystem::remove(tmp, ec);
                }

                ELLE_INFO("SHN saved: root=%s name=%s size=%zu",
                          root.c_str(), name.c_str(), decoded.size());
                ELLE_LOG_HTTP("SHN save OK root=%s name=%s size=%zu",
                              root.c_str(), name.c_str(), decoded.size());

                try {
                    std::filesystem::path historyDir =
                        std::filesystem::path(absDir).parent_path() / "shn_history";
                    std::error_code hec;
                    std::filesystem::create_directories(historyDir, hec);
                    std::filesystem::path hpath = historyDir /
                        (std::filesystem::path(name).stem().string() + ".log");
                    std::ofstream hf(hpath, std::ios::app);
                    if (hf.is_open()) {
                        auto user = req.headers.count("x-auth-user-id")
                            ? req.headers.at("x-auth-user-id")
                            : std::string("anon");
                        std::time_t tt = std::time(nullptr);
                        std::tm lt{};
#ifdef _WIN32
                        localtime_s(&lt, &tt);
#else
                        localtime_r(&tt, &lt);
#endif
                        char iso[32];
                        std::snprintf(iso, sizeof(iso),
                            "%04d-%02d-%02dT%02d:%02d:%02d",
                            lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
                            lt.tm_hour, lt.tm_min, lt.tm_sec);
                        uint64_t ms = (uint64_t)std::chrono::duration_cast<
                            std::chrono::milliseconds>(
                                std::chrono::system_clock::now()
                                    .time_since_epoch()).count();
                        hf << iso << '|' << ms << '|' << user << '|'
                           << decoded.size() << '|' << root << '\n';
                    }
                } catch (...) {

                }

                return HTTPResponse::OK({
                    {"ok",       true},
                    {"path",     path.string()},
                    {"bytes",    (uint64_t)decoded.size()},
                    {"root",     root},
                    {"name",     name}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(400,
                    std::string("invalid body: ") + e.what());
            }
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/list",
            [shnResolveRoot](const HTTPRequest& req) {
            std::string root = req.QueryString("root", "Hero");
            std::string absDir, err;
            if (!shnResolveRoot(root, absDir, err))
                return HTTPResponse::Err(400, err);

            json arr = json::array();
            std::error_code ec;
            if (std::filesystem::exists(absDir, ec)) {
                for (const auto& ent :
                        std::filesystem::directory_iterator(absDir, ec)) {
                    if (!ent.is_regular_file()) continue;
                    auto ext = ent.path().extension().string();
                    for (auto& c : ext)
                        c = (char)std::tolower((unsigned char)c);
                    if (ext != ".shn") continue;
                    arr.push_back({
                        {"name",     ent.path().filename().string()},
                        {"bytes",    (uint64_t)ent.file_size(ec)},
                        {"modified_ms",
                            (uint64_t)std::chrono::duration_cast<
                                std::chrono::milliseconds>(
                                    ent.last_write_time().time_since_epoch()
                                ).count()}
                    });
                }
            }
            return HTTPResponse::OK({
                {"root",    root},
                {"abs_dir", absDir},
                {"files",   arr}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/get",
            [shnResolveRoot, shnValidateName](const HTTPRequest& req) {
            std::string root = req.QueryString("root", "Hero");
            std::string name = req.QueryString("name", "");
            std::string err;
            if (!shnValidateName(name, err))
                return HTTPResponse::Err(400, err);
            std::string absDir;
            if (!shnResolveRoot(root, absDir, err))
                return HTTPResponse::Err(400, err);

            std::filesystem::path path =
                std::filesystem::path(absDir) / name;
            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
                return HTTPResponse::Err(404, "file not found: " + name);

            std::ifstream f(path, std::ios::binary);
            if (!f.is_open())
                return HTTPResponse::Err(500,
                    "cannot open " + path.string() + " for read");
            std::string bytes((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());

            static const char kTbl[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string b64;
            b64.reserve((bytes.size() + 2) / 3 * 4);
            int val = 0, bits = -6;
            for (unsigned char c : bytes) {
                val = (val << 8) | c; bits += 8;
                while (bits >= 0) {
                    b64 += kTbl[(val >> bits) & 0x3F];
                    bits -= 6;
                }
            }
            if (bits > -6)
                b64 += kTbl[((val << 8) >> (bits + 8)) & 0x3F];
            while (b64.size() % 4) b64 += '=';

            return HTTPResponse::OK({
                {"root",      root},
                {"name",      name},
                {"path",      path.string()},
                {"bytes",     (uint64_t)bytes.size()},
                {"bytes_b64", b64}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/history",
            [shnValidateName](const HTTPRequest& req) {
            std::string name = req.QueryString("name", "");
            std::string err;
            if (!shnValidateName(name, err))
                return HTTPResponse::Err(400, err);
            int limit = 20;
            try { limit = std::stoi(req.QueryString("limit", "20")); } catch (...) {}
            if (limit <= 0 || limit > 500) limit = 20;

            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            std::filesystem::path hdir =
                std::filesystem::path(buf).parent_path() / "shn_history";
            std::filesystem::path hpath = hdir /
                (std::filesystem::path(name).stem().string() + ".log");

            json entries = json::array();
            std::error_code ec;
            if (!std::filesystem::exists(hpath, ec)) {
                return HTTPResponse::OK({
                    {"name", name}, {"count", 0}, {"entries", entries}
                });
            }
            std::ifstream f(hpath);
            if (!f.is_open())
                return HTTPResponse::Err(500,
                    "cannot open history for " + name);

            std::vector<std::string> lines;
            std::string ln;
            while (std::getline(f, ln)) {
                if (!ln.empty() && ln.back() == '\r') ln.pop_back();
                if (!ln.empty()) lines.push_back(ln);
            }

            const int n = (int)lines.size();
            const int start = std::max(0, n - limit);
            for (int i = n - 1; i >= start; --i) {

                const std::string& L = lines[i];
                std::vector<std::string> parts;
                std::string cur;
                for (char c : L) {
                    if (c == '|') { parts.push_back(cur); cur.clear(); }
                    else cur.push_back(c);
                }
                parts.push_back(cur);
                if (parts.size() < 5) continue;
                entries.push_back({
                    {"iso",   parts[0]},
                    {"ts_ms", (uint64_t)std::strtoull(parts[1].c_str(), nullptr, 10)},
                    {"user",  parts[2]},
                    {"bytes", (uint64_t)std::strtoull(parts[3].c_str(), nullptr, 10)},
                    {"root",  parts[4]},
                });
            }
            return HTTPResponse::OK({
                {"name",    name},
                {"count",   (int)entries.size()},
                {"entries", entries},
            });
        }, AUTH_ADMIN);

        ELLE_INFO("Registered %zu API routes", m_router.Count());
    }
};

ELLE_SERVICE_MAIN(ElleHTTPService)
