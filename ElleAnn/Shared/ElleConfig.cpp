#include "ElleConfig.h"
#include "ElleLogger.h"
#include "ElleServerInfo.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

static const JsonValue s_nullValue;

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (type != JsonType::Object) return s_nullValue;
    auto it = obj_val.find(key);
    return (it != obj_val.end()) ? it->second : s_nullValue;
}

bool ElleConfig::LoadFromServerInfo(const std::string& serverInfoPath) {
    ElleServerInfoFile si;
    std::string err;
    if (!LoadElleServerInfo(serverInfoPath, si, err)) {
        ELLE_ERROR("ServerInfo load failed: %s", err.c_str());
        return false;
    }

    const std::string httpKey = !si.my_server_name.empty() ? si.my_server_name : "PG_Elle";
    auto itHttp = si.servers.find(httpKey);
    if (itHttp != si.servers.end()) {
        m_http.bind_address = itHttp->second.host;
        m_http.port = (uint32_t)std::max(0, itHttp->second.port);
    }

    auto itAcc = si.odbc.find("Account");
    if (itAcc != si.odbc.end()) {

        m_root.obj_val["http_server"].type = JsonType::Object;
        m_root.obj_val["http_server"].obj_val["game_db_dsn"].type = JsonType::String;
        m_root.obj_val["http_server"].obj_val["game_db_dsn"].str_val = itAcc->second.dsn;
    }

    auto itCore = si.odbc.find("ElleCore");
    if (itCore != si.odbc.end()) {
        m_service.sql_connection_string = itCore->second.dsn;
    }

    {
        char modPath[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, modPath, MAX_PATH);
        std::string exeDir(modPath);
        size_t s = exeDir.find_last_of("\\/");
        if (s != std::string::npos) exeDir.resize(s + 1);
        std::string luaDir = exeDir + "9Data\\Hero\\LuaScript";
        m_root.obj_val["lua"].type = JsonType::Object;
        m_root.obj_val["lua"].obj_val["scripts_directory"].type = JsonType::String;
        m_root.obj_val["lua"].obj_val["scripts_directory"].str_val = luaDir;
        m_root.obj_val["lua"].obj_val["file_prefix"].type = JsonType::String;
        m_root.obj_val["lua"].obj_val["file_prefix"].str_val = "elle_";
    }

    m_configPath = serverInfoPath;
    return true;
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (type != JsonType::Array || index >= arr_val.size()) return s_nullValue;
    return arr_val[index];
}

bool JsonValue::has(const std::string& key) const {
    return type == JsonType::Object && obj_val.find(key) != obj_val.end();
}

ElleConfig& ElleConfig::Instance() {
    static ElleConfig inst;
    return inst;
}

namespace {
    struct JsonParser {
        const std::string& src;
        size_t pos = 0;

        JsonParser(const std::string& s) : src(s) {}

        void SkipWhitespace() {
            while (pos < src.size() && std::isspace((unsigned char)src[pos])) pos++;
        }

        char Peek() {
            SkipWhitespace();
            return pos < src.size() ? src[pos] : '\0';
        }

        char Next() {
            SkipWhitespace();
            return pos < src.size() ? src[pos++] : '\0';
        }

        bool Match(char c) {
            SkipWhitespace();
            if (pos < src.size() && src[pos] == c) { pos++; return true; }
            return false;
        }

        std::string ParseString() {
            if (Next() != '"') return "";
            std::string result;
            while (pos < src.size() && src[pos] != '"') {
                if (src[pos] == '\\' && pos + 1 < src.size()) {
                    pos++;
                    switch (src[pos]) {
                        case '"':  result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/':  result += '/'; break;
                        case 'n':  result += '\n'; break;
                        case 't':  result += '\t'; break;
                        case 'r':  result += '\r'; break;
                        case 'b':  result += '\b'; break;
                        case 'f':  result += '\f'; break;
                        case 'u': {

                            auto readHex4 = [&](size_t at, unsigned& out) -> bool {
                                if (at + 3 >= src.size()) return false;
                                out = 0;
                                for (int k = 0; k < 4; k++) {
                                    char c = src[at + k];
                                    unsigned d;
                                    if      (c >= '0' && c <= '9') d = (unsigned)(c - '0');
                                    else if (c >= 'a' && c <= 'f') d = 10u + (unsigned)(c - 'a');
                                    else if (c >= 'A' && c <= 'F') d = 10u + (unsigned)(c - 'A');
                                    else return false;
                                    out = (out << 4) | d;
                                }
                                return true;
                            };
                            unsigned cp = 0;
                            if (!readHex4(pos + 1, cp)) break;
                            pos += 4;

                            if (cp >= 0xD800 && cp <= 0xDBFF) {
                                unsigned lo = 0;
                                if (pos + 6 < src.size() &&
                                    src[pos + 1] == '\\' && src[pos + 2] == 'u' &&
                                    readHex4(pos + 3, lo) &&
                                    lo >= 0xDC00 && lo <= 0xDFFF) {
                                    cp = 0x10000u
                                       + (((cp - 0xD800u) << 10) | (lo - 0xDC00u));
                                    pos += 6;
                                } else {

                                    cp = 0xFFFD;
                                }
                            } else if (cp >= 0xDC00 && cp <= 0xDFFF) {

                                cp = 0xFFFD;
                            }

                            if (cp < 0x80) {
                                result += (char)cp;
                            } else if (cp < 0x800) {
                                result += (char)(0xC0 | (cp >> 6));
                                result += (char)(0x80 | (cp & 0x3F));
                            } else if (cp < 0x10000) {
                                result += (char)(0xE0 | (cp >> 12));
                                result += (char)(0x80 | ((cp >> 6) & 0x3F));
                                result += (char)(0x80 | (cp & 0x3F));
                            } else {
                                result += (char)(0xF0 |  (cp >> 18));
                                result += (char)(0x80 | ((cp >> 12) & 0x3F));
                                result += (char)(0x80 | ((cp >>  6) & 0x3F));
                                result += (char)(0x80 |  (cp        & 0x3F));
                            }
                            break;
                        }
                        default:   result += src[pos]; break;
                    }
                } else {
                    result += src[pos];
                }
                pos++;
            }
            if (pos < src.size()) pos++;
            return result;
        }

        JsonValue ParseValue() {
            SkipWhitespace();
            if (pos >= src.size()) return {};

            char c = src[pos];
            if (c == '"') {
                JsonValue v;
                v.type = JsonType::String;
                v.str_val = ParseString();
                return v;
            }
            if (c == '{') return ParseObject();
            if (c == '[') return ParseArray();
            if (c == 't' || c == 'f') return ParseBool();
            if (c == 'n') return ParseNull();
            return ParseNumber();
        }

        JsonValue ParseObject() {
            JsonValue v;
            v.type = JsonType::Object;
            Next();
            if (Peek() == '}') { Next(); return v; }
            while (true) {
                std::string key = ParseString();
                Match(':');
                v.obj_val[key] = ParseValue();
                if (!Match(',')) break;
            }
            Match('}');
            return v;
        }

        JsonValue ParseArray() {
            JsonValue v;
            v.type = JsonType::Array;
            Next();
            if (Peek() == ']') { Next(); return v; }
            while (true) {
                v.arr_val.push_back(ParseValue());
                if (!Match(',')) break;
            }
            Match(']');
            return v;
        }

        JsonValue ParseNumber() {
            SkipWhitespace();
            size_t start = pos;
            bool isFloat = false;
            if (pos < src.size() && src[pos] == '-') pos++;
            while (pos < src.size() && std::isdigit((unsigned char)src[pos])) pos++;
            if (pos < src.size() && src[pos] == '.') { isFloat = true; pos++; }
            while (pos < src.size() && std::isdigit((unsigned char)src[pos])) pos++;
            if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
                isFloat = true; pos++;
                if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) pos++;
                while (pos < src.size() && std::isdigit((unsigned char)src[pos])) pos++;
            }
            std::string num = src.substr(start, pos - start);
            JsonValue v;

            try {
                if (isFloat) {
                    v.type = JsonType::Float;
                    v.float_val = std::stod(num);
                } else {
                    v.type = JsonType::Int;
                    v.int_val = std::stoll(num);
                }
            } catch (const std::exception& ex) {
                ELLE_WARN("Config JSON number parse error at offset %zu: %s "
                          "(value=`%s`) — treating as 0",
                          start, ex.what(), num.c_str());
                v.type = JsonType::Int;
                v.int_val = 0;
            }
            return v;
        }

        JsonValue ParseBool() {
            JsonValue v;
            v.type = JsonType::Bool;
            if (src.substr(pos, 4) == "true") { v.bool_val = true; pos += 4; }
            else { v.bool_val = false; pos += 5; }
            return v;
        }

        JsonValue ParseNull() {
            pos += 4;
            return {};
        }
    };
}

bool ElleConfig::Load(const std::string& configPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_configPath = configPath;

    if (configPath.find("ServerInfo") != std::string::npos &&
        (configPath.size() >= 4 && configPath.substr(configPath.size() - 4) == ".txt")) {
        return LoadFromServerInfo(configPath);
    }

    std::ifstream file(configPath);
    if (!file.is_open()) return false;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string json = ss.str();
    file.close();

    if (!ParseJSON(json, m_root)) {
        std::cerr << "Config parse failed: " << configPath << "\n";
        ELLE_ERROR("Config parse failed: %s", configPath.c_str());
        return false;
    }

    const char* kRequired[] = { "llm", "emotions", "memory", "http_server", "services" };
    for (const char* sec : kRequired) {
        const auto& v = m_root[sec];
        if (v.is_null() || v.type != JsonType::Object) {
            std::cerr << "Config validation failed: missing/invalid section '" << sec
                      << "' in " << configPath << "\n";
            ELLE_ERROR("Config validation failed: section '%s' is missing or not an object "
                       "in %s — refusing to start with defaults only.",
                       sec, configPath.c_str());
            return false;
        }
    }

    {
        const auto& llm = m_root["llm"];
        std::string mode = llm["mode"].str_val;
        if (mode != "api" && mode != "local" && mode != "hybrid") {
            std::cerr << "Config validation failed: llm.mode='" << mode
                      << "' (must be api|local|hybrid)\n";
            ELLE_ERROR("Config validation failed: llm.mode = '%s' (must be api|local|hybrid)",
                       mode.c_str());
            return false;
        }
        const auto& provs = llm["providers"];
        if (provs.is_null() || provs.type != JsonType::Object || provs.obj_val.empty()) {
            std::cerr << "Config validation failed: llm.providers must be a non-empty object\n";
            ELLE_ERROR("Config validation failed: llm.providers must be a non-empty object");
            return false;
        }
    }

    PopulateFromJSON(m_root);
    return true;
}

bool ElleConfig::Reload() {
    if (m_configPath.empty()) return false;
    if (!Load(m_configPath)) return false;
    for (auto& cb : m_reloadCallbacks) cb();
    return true;
}

void ElleConfig::RegisterReloadCallback(std::function<void()> cb) {
    m_reloadCallbacks.push_back(std::move(cb));
}

void ElleConfig::LoadDefaults() {
    std::lock_guard<std::mutex> lock(m_mutex);
    static const char* kDefaultsJson = R"DEFAULTS({
        "llm": { "mode": "hybrid", "primary_provider": "groq",
                 "fallback_provider": "local_llama", "providers": {} },
        "emotions": { "decay_rate": 0.05, "tick_interval_ms": 1000 },
        "memory":   { "short_term_capacity": 256, "consolidation_interval_ms": 60000 },
        "http_server": {
            "bind_address": "0.0.0.0",
            "port": 8000,
            "websocket_path": "/command",
            "max_connections": 256,
            "cors_enabled": true,
            "cors_origins": ["*"],
            "no_auth": 1,
            "auth_enabled": false,
            "admin_auth_id_threshold": 9,
            "rate_limit_rpm": 0,
            "max_upload_bytes": 10485760
        },
        "services": {
            "named_pipes": { "prefix": "\\\\.\\pipe\\elle_" },
            "sql_connection_string": "",
            "sql_pool_size": 4
        },
        "logging": { "level": "info" }
    })DEFAULTS";
    JsonValue root;
    if (!ParseJSON(kDefaultsJson, root)) {
        ELLE_ERROR("LoadDefaults: built-in defaults JSON failed to parse");
        return;
    }
    m_root = std::move(root);
    PopulateFromJSON(m_root);
    for (auto& cb : m_reloadCallbacks) cb();
}

bool ElleConfig::LayerJsonOver(const std::string& jsonPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ifstream f(jsonPath);
    if (!f.is_open()) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();
    f.close();

    JsonValue layer;
    if (!ParseJSON(json, layer)) {
        ELLE_WARN("LayerJsonOver: parse failed for %s", jsonPath.c_str());
        return false;
    }
    if (layer.type != JsonType::Object) return false;

    for (auto& kv : layer.obj_val) {
        m_root.obj_val[kv.first] = kv.second;
    }

    PopulateFromJSON(m_root);
    for (auto& cb : m_reloadCallbacks) cb();
    return true;
}

namespace {

bool IsSensitiveKey(const std::string& key) {
    static const std::vector<std::string> kPatterns = {
        "password", "passwd", "secret", "api_key", "apikey",
        "admin_key", "jwt", "token", "private_key", "auth_key",
        "pwd",
    };
    std::string k = key;
    for (auto& c : k) c = (char)std::tolower((unsigned char)c);
    for (auto& pat : kPatterns) {
        if (k.find(pat) != std::string::npos) return true;
    }
    return false;
}

void EscapeJsonString(std::ostringstream& out, const std::string& in) {
    out.put('"');
    for (unsigned char c : in) {
        switch (c) {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:
                if (c < 0x20) {
                    char b[8];
                    std::snprintf(b, sizeof(b), "\\u%04x", (unsigned)c);
                    out << b;
                } else {
                    out.put((char)c);
                }
        }
    }
    out.put('"');
}

void DumpNode(std::ostringstream& out, const JsonValue& v,
              const std::string& parentKey, int indent) {
    auto pad = [&](int n) { for (int i = 0; i < n; ++i) out.put(' '); };
    switch (v.type) {
        case JsonType::Null:    out << "null"; return;
        case JsonType::Bool:    out << (v.bool_val ? "true" : "false"); return;
        case JsonType::Int:
            out << v.int_val;
            return;
        case JsonType::Float:

            if (v.float_val == (double)(long long)v.float_val) {
                out << (long long)v.float_val;
            } else {
                out << v.float_val;
            }
            return;
        case JsonType::String: {
            const std::string& s = IsSensitiveKey(parentKey)
                ? std::string("***") : v.str_val;
            EscapeJsonString(out, s);
            return;
        }
        case JsonType::Array: {
            out << "[";
            for (size_t i = 0; i < v.arr_val.size(); ++i) {
                if (i) out << ",";
                out << "\n"; pad(indent + 2);
                DumpNode(out, v.arr_val[i], parentKey, indent + 2);
            }
            if (!v.arr_val.empty()) { out << "\n"; pad(indent); }
            out << "]";
            return;
        }
        case JsonType::Object: {
            out << "{";
            bool first = true;
            for (auto& kv : v.obj_val) {
                if (!first) out << ",";
                first = false;
                out << "\n"; pad(indent + 2);
                EscapeJsonString(out, kv.first);
                out << ": ";
                DumpNode(out, kv.second, kv.first, indent + 2);
            }
            if (!v.obj_val.empty()) { out << "\n"; pad(indent); }
            out << "}";
            return;
        }
    }
}

}

std::string ElleConfig::DumpJsonRedacted() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream out;
    DumpNode(out, m_root, "", 0);
    return out.str();
}

bool ElleConfig::ParseJSON(const std::string& json, JsonValue& root) {
    JsonParser parser(json);
    root = parser.ParseValue();
    if (root.type != JsonType::Object) return false;

    parser.SkipWhitespace();
    if (parser.pos < parser.src.size()) {
        ELLE_ERROR("Config JSON has trailing content after root object at "
                   "offset %zu -- refusing to load partial config",
                   parser.pos);
        return false;
    }
    return true;
}

void ElleConfig::PopulateFromJSON(const JsonValue& root) {

    const auto& llm = root["llm"];
    if (!llm.is_null()) {
        std::string mode = llm["mode"].str_val;
        if (mode == "api") m_llm.mode = LLM_MODE_API;
        else if (mode == "local") m_llm.mode = LLM_MODE_LOCAL;
        else m_llm.mode = LLM_MODE_HYBRID;

        m_llm.stream = llm["stream_responses"].bool_val;
        m_llm.max_context_tokens = (uint32_t)llm["max_context_tokens"].int_val;
        m_llm.system_prompt_prefix = llm["system_prompt_prefix"].str_val;
        m_llm.chain_of_thought = llm["chain_of_thought"].bool_val;
        m_llm.self_reflection = llm["self_reflection_enabled"].bool_val;
        m_llm.creative_temp_boost = (float)llm["creative_temperature_boost"].float_val;
        m_llm.reasoning_temp_drop = (float)llm["reasoning_temperature_drop"].float_val;

        if (llm.has("primary_provider") && !llm["primary_provider"].str_val.empty()) {
            m_llm.primary_provider = llm["primary_provider"].str_val;
        }
        if (llm.has("fallback_provider") && !llm["fallback_provider"].str_val.empty()) {
            m_llm.fallback_provider = llm["fallback_provider"].str_val;
        }

        const auto& providers = llm["providers"];
        if (!providers.is_null()) {
            for (auto& [name, prov] : providers.obj_val) {
                LLMProviderConfig pc;
                pc.enabled = prov["enabled"].bool_val;
                pc.api_url = prov["api_url"].str_val;
                pc.api_key = prov["api_key"].str_val;
                pc.model = prov["model"].str_val;
                pc.max_tokens = prov.has("max_tokens") ? (uint32_t)prov["max_tokens"].int_val : 4096;
                pc.temperature = prov.has("temperature") ? (float)prov["temperature"].float_val : 0.85f;
                pc.top_p = prov.has("top_p") ? (float)prov["top_p"].float_val : 0.95f;
                pc.timeout_ms = prov.has("timeout_ms") ? (uint32_t)prov["timeout_ms"].int_val : 30000;
                pc.model_path = prov["model_path"].str_val;
                pc.context_size = prov.has("context_size") ? (uint32_t)prov["context_size"].int_val : 8192;
                pc.gpu_layers = prov.has("gpu_layers") ? (uint32_t)prov["gpu_layers"].int_val : 0;
                pc.threads = prov.has("threads") ? (uint32_t)prov["threads"].int_val : 4;
                pc.batch_size = prov.has("batch_size") ? (uint32_t)prov["batch_size"].int_val : 512;
                pc.mlock = prov["mlock"].bool_val;
                pc.mmap = prov.has("mmap") ? prov["mmap"].bool_val : true;
                pc.use_gpu = prov["use_gpu"].bool_val;

                pc.frequency_penalty = prov.has("frequency_penalty")
                    ? (float)prov["frequency_penalty"].float_val : 0.0f;
                pc.presence_penalty  = prov.has("presence_penalty")
                    ? (float)prov["presence_penalty"].float_val  : 0.0f;
                pc.rate_limit_rpm    = prov.has("rate_limit_rpm")
                    ? (uint32_t)prov["rate_limit_rpm"].int_val  : 0;
                m_llm.providers[name] = pc;
            }
        }
    }

    const auto& emo = root["emotions"];
    if (!emo.is_null()) {
        m_emotion.decay_rate = (float)emo["decay_rate_per_tick"].float_val;
        m_emotion.tick_interval_ms = (uint32_t)emo["tick_interval_ms"].int_val;
        m_emotion.contagion_weight = (float)emo["contagion_weight"].float_val;
        m_emotion.baseline_return_rate = (float)emo["baseline_return_rate"].float_val;
        m_emotion.emotional_inertia = (float)emo["emotional_inertia"].float_val;
        m_emotion.mood_threshold = (float)emo["mood_formation_threshold"].float_val;
        m_emotion.mood_duration_ticks = (uint32_t)emo["mood_duration_ticks"].int_val;
        m_emotion.sentiment_analysis = emo["sentiment_analysis_enabled"].bool_val;

        m_emotion.baselines.clear();
        m_emotion.triggers.clear();

        const auto& baselines = emo["baselines"];
        if (!baselines.is_null()) {
            for (auto& [k, v] : baselines.obj_val) {
                m_emotion.baselines[k] = (float)v.float_val;
            }
        }

        const auto& triggers = emo["triggers"];
        if (!triggers.is_null()) {
            for (auto& t : triggers.arr_val) {
                EmotionTriggerConfig tc;
                tc.pattern = t["pattern"].str_val;
                tc.emotion = t["emotion"].str_val;
                tc.delta = (float)t["delta"].float_val;
                m_emotion.triggers.push_back(tc);
            }
        }
    }

    const auto& svc = root["services"];
    if (!svc.is_null()) {
        m_service.iocp_threads = (uint32_t)svc["iocp_thread_count"].int_val;
        m_service.heartbeat_ms = (uint32_t)svc["heartbeat_interval_ms"].int_val;
        m_service.heartbeat_timeout_ms = (uint32_t)svc["heartbeat_timeout_ms"].int_val;
        m_service.dead_man_timeout_ms = (uint32_t)svc["dead_man_switch_timeout_ms"].int_val;
        m_service.watchdog_ms = (uint32_t)svc["watchdog_interval_ms"].int_val;
        m_service.stale_worker_ms = (uint32_t)svc["stale_worker_timeout_ms"].int_val;
        m_service.max_restarts = (uint32_t)svc["max_restart_attempts"].int_val;

        const auto& pipes = svc["named_pipes"];
        if (!pipes.is_null()) {
            m_service.pipe_prefix = pipes["prefix"].str_val;
            m_service.pipe_buffer_size = (uint32_t)pipes["buffer_size"].int_val;
            m_service.pipe_max_instances = (uint32_t)pipes["max_instances"].int_val;
        }

        const auto& sql = svc["sql_pipes"];
        if (!sql.is_null()) {
            m_service.sql_connection_string = sql["connection_string"].str_val;
            m_service.sql_pool_size = (uint32_t)sql["pool_size"].int_val;
            m_service.sql_timeout_seconds = (uint32_t)sql["command_timeout_seconds"].int_val;
        }
    }

    const auto& trust = root["trust"];
    if (!trust.is_null()) {
        m_trust.initial_score = (int32_t)trust["initial_score"].int_val;
        m_trust.max_score = (int32_t)trust["max_score"].int_val;
        m_trust.success_delta = (int32_t)trust["success_delta"].int_val;
        m_trust.failure_delta = (int32_t)trust["failure_delta"].int_val;
        m_trust.decay_idle_hours = (uint32_t)trust["decay_on_idle_hours"].int_val;
        m_trust.decay_amount = (int32_t)trust["decay_amount"].int_val;
        m_trust.audit_all = trust["audit_all_actions"].bool_val;
    }

    const auto& mem = root["memory"];
    if (!mem.is_null()) {
        m_memory.stm_capacity = (uint32_t)mem["stm_capacity"].int_val;
        m_memory.stm_decay_seconds = (float)mem["stm_decay_seconds"].int_val;
        m_memory.promote_threshold = (float)mem["stm_promote_threshold"].float_val;
        m_memory.consolidation_interval_min = (uint32_t)mem["ltm_consolidation_interval_minutes"].int_val;
        m_memory.recall_interval_sec = (uint32_t)mem["recall_loop_interval_seconds"].int_val;
        m_memory.cluster_threshold = (float)mem["cluster_similarity_threshold"].float_val;
        m_memory.max_clusters = (uint32_t)mem["max_clusters"].int_val;
        m_memory.narrative_gen = mem["narrative_generation_enabled"].bool_val;
        m_memory.dream_consolidation = mem["dream_consolidation_enabled"].bool_val;
        m_memory.dream_interval_min = (uint32_t)mem["dream_interval_minutes"].int_val;
        m_memory.archive_after_days = (uint32_t)mem["archive_after_days"].int_val;
    }

    const auto& http = root["http_server"];
    if (!http.is_null()) {
        m_http.bind_address = http["bind_address"].str_val;
        m_http.port = (uint32_t)http["port"].int_val;
        m_http.ws_path = http["websocket_path"].str_val;
        m_http.max_connections = (uint32_t)http["max_connections"].int_val;

        if (!http["auth_enabled"].is_null())
            m_http.auth_enabled = http["auth_enabled"].bool_val;
        m_http.jwt_secret = http["jwt_secret"].str_val;
        m_http.jwt_expiry_hours = (uint32_t)http["jwt_expiry_hours"].int_val;

        if (!http["cors_enabled"].is_null())
            m_http.cors_enabled = http["cors_enabled"].bool_val;
        if (http["cors_origins"].type == JsonType::Array) {
            std::string joined;
            for (auto& v : http["cors_origins"].arr_val) {
                if (!joined.empty()) joined += ",";
                joined += v.str_val;
            }
            if (!joined.empty()) m_http.cors_origins = joined;
        } else if (!http["cors_origins"].is_null()) {
            m_http.cors_origins = http["cors_origins"].str_val;
        }
        if (!http["rate_limit_rpm"].is_null())
            m_http.rate_limit_rpm = (uint32_t)http["rate_limit_rpm"].int_val;
        if (!http["max_concurrent_connections"].is_null())
            m_http.max_concurrent_connections =
                (uint32_t)http["max_concurrent_connections"].int_val;
        if (!http["max_ws_frame_bytes"].is_null())
            m_http.max_ws_frame_bytes = (uint32_t)http["max_ws_frame_bytes"].int_val;
        if (!http["max_upload_bytes"].is_null())
            m_http.max_upload_bytes = (uint32_t)http["max_upload_bytes"].int_val;
    }
}

const JsonValue* ElleConfig::Navigate(const std::string& dotPath) const {
    const JsonValue* current = &m_root;
    std::istringstream iss(dotPath);
    std::string segment;
    while (std::getline(iss, segment, '.')) {
        if (current->type != JsonType::Object) return nullptr;
        auto it = current->obj_val.find(segment);
        if (it == current->obj_val.end()) return nullptr;
        current = &it->second;
    }
    return current;
}

std::string ElleConfig::GetString(const std::string& path, const std::string& def) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const JsonValue* v = Navigate(path);
    return (v && v->type == JsonType::String) ? v->str_val : def;
}

int64_t ElleConfig::GetInt(const std::string& path, int64_t def) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const JsonValue* v = Navigate(path);
    if (!v) return def;
    if (v->type == JsonType::Int) return v->int_val;
    if (v->type == JsonType::Float) return (int64_t)v->float_val;
    return def;
}

double ElleConfig::GetFloat(const std::string& path, double def) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const JsonValue* v = Navigate(path);
    if (!v) return def;
    if (v->type == JsonType::Float) return v->float_val;
    if (v->type == JsonType::Int) return (double)v->int_val;
    return def;
}

bool ElleConfig::GetBool(const std::string& path, bool def) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const JsonValue* v = Navigate(path);
    return (v && v->type == JsonType::Bool) ? v->bool_val : def;
}
