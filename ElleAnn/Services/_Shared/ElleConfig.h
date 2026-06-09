#pragma once
#ifndef ELLE_CONFIG_H
#define ELLE_CONFIG_H

#include "ElleTypes.h"
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <functional>

enum class JsonType { Null, Bool, Int, Float, String, Array, Object };

struct JsonValue {
    JsonType type = JsonType::Null;
    bool     bool_val = false;
    int64_t  int_val = 0;
    double   float_val = 0.0;
    std::string str_val;
    std::vector<JsonValue> arr_val;
    std::unordered_map<std::string, JsonValue> obj_val;

    bool        is_null()   const { return type == JsonType::Null; }
    bool        as_bool()   const { return bool_val; }
    int64_t     as_int()    const { return int_val; }
    double      as_float()  const { return float_val; }
    const std::string& as_str() const { return str_val; }

    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](size_t index) const;
    bool has(const std::string& key) const;
};

struct LLMProviderConfig {
    bool        enabled = false;
    std::string api_url;
    std::string api_key;
    std::string model;
    uint32_t    max_tokens = 4096;
    float       temperature = 0.85f;
    float       top_p = 0.95f;
    float       frequency_penalty = 0.0f;
    float       presence_penalty = 0.0f;
    uint32_t    timeout_ms = 30000;
    uint32_t    rate_limit_rpm = 60;

    std::string model_path;
    std::string binary_path;
    uint32_t    context_size = 8192;
    uint32_t    gpu_layers = 0;
    uint32_t    threads = 4;
    uint32_t    batch_size = 512;
    bool        mlock = false;
    bool        mmap = true;
    bool        use_gpu = false;
};

struct LLMConfig {
    ELLE_LLM_MODE mode = LLM_MODE_HYBRID;

    std::string primary_provider  = "groq";
    std::string fallback_provider = "local_llama";
    bool        stream = true;
    uint32_t    max_context_tokens = 131072;
    std::string system_prompt_prefix;
    bool        chain_of_thought = true;
    bool        self_reflection = true;
    float       creative_temp_boost = 0.15f;
    float       reasoning_temp_drop = -0.2f;
    std::unordered_map<std::string, LLMProviderConfig> providers;
};

struct EmotionTriggerConfig {
    std::string pattern;
    std::string emotion;
    float       delta = 0.0f;
};

struct EmotionConfig {
    float       decay_rate = 0.05f;
    uint32_t    tick_interval_ms = 1000;
    float       contagion_weight = 0.35f;
    float       baseline_return_rate = 0.02f;
    float       emotional_inertia = 0.15f;
    float       mood_threshold = 0.6f;
    uint32_t    mood_duration_ticks = 300;
    bool        sentiment_analysis = true;
    std::unordered_map<std::string, float> baselines;
    std::vector<EmotionTriggerConfig> triggers;
};

struct ServiceConfig {
    uint32_t    iocp_threads = 4;
    uint32_t    heartbeat_ms = 5000;
    uint32_t    heartbeat_timeout_ms = 15000;
    uint32_t    dead_man_timeout_ms = 60000;
    uint32_t    watchdog_ms = 10000;
    uint32_t    stale_worker_ms = 30000;
    uint32_t    max_restarts = 5;
    std::string pipe_prefix;
    uint32_t    pipe_buffer_size = 65536;
    uint32_t    pipe_max_instances = 16;
    std::string sql_connection_string;
    uint32_t    sql_pool_size = 8;
    uint32_t    sql_timeout_seconds = 30;
};

struct TrustConfig {
    int32_t     initial_score = 5;
    int32_t     max_score = 100;
    int32_t     success_delta = 2;
    int32_t     failure_delta = -3;
    uint32_t    decay_idle_hours = 24;
    int32_t     decay_amount = 1;
    bool        audit_all = true;
};

struct MemoryConfig {
    uint32_t    stm_capacity = 256;
    float       stm_decay_seconds = 15.0f;
    float       promote_threshold = 0.6f;
    uint32_t    buffer_to_ltm_seconds   = 86400;
    uint32_t    ltm_to_archive_seconds  = 2592000;
    uint32_t    aging_interval_min       = 30;
    uint32_t    consolidation_interval_min = 5;
    uint32_t    recall_interval_sec = 30;
    float       cluster_threshold = 0.75f;
    uint32_t    max_clusters = 512;
    bool        narrative_gen = true;
    bool        dream_consolidation = true;
    uint32_t    dream_interval_min = 30;
    uint32_t    archive_after_days = 90;
};

struct HTTPConfig {

    std::string bind_address = "0.0.0.0";
    uint32_t    port = 8000;
    std::string ws_path = "/command";
    uint32_t    max_connections = 256;
    bool        auth_enabled = true;
    std::string jwt_secret;
    uint32_t    jwt_expiry_hours = 24;

    bool        cors_enabled = true;
    std::string cors_origins = "http://localhost:3000";
    uint32_t    rate_limit_rpm = 0;
    uint32_t    max_concurrent_connections = 256;
    uint32_t    max_ws_frame_bytes = 1 * 1024 * 1024;
    uint32_t    max_upload_bytes   = 10 * 1024 * 1024;
};

class ElleConfig {
public:
    static ElleConfig& Instance();

    bool Load(const std::string& configPath);
    bool LoadFromServerInfo(const std::string& serverInfoPath);

    void LoadDefaults();

    bool LayerJsonOver(const std::string& jsonPath);

    std::string DumpJsonRedacted() const;
    bool Reload();

    void RegisterReloadCallback(std::function<void()> cb);

    const LLMConfig&     GetLLM()      const { return m_llm; }
    const EmotionConfig& GetEmotion()  const { return m_emotion; }
    const ServiceConfig& GetService()  const { return m_service; }
    const TrustConfig&   GetTrust()    const { return m_trust; }
    const MemoryConfig&  GetMemory()   const { return m_memory; }
    const HTTPConfig&    GetHTTP()     const { return m_http; }

    std::string GetString(const std::string& path, const std::string& def = "") const;
    int64_t     GetInt(const std::string& path, int64_t def = 0) const;
    double      GetFloat(const std::string& path, double def = 0.0) const;
    bool        GetBool(const std::string& path, bool def = false) const;

    const std::string& GetConfigPath() const { return m_configPath; }

private:
    ElleConfig() = default;
    ~ElleConfig() = default;
    ElleConfig(const ElleConfig&) = delete;
    ElleConfig& operator=(const ElleConfig&) = delete;

    bool ParseJSON(const std::string& json, JsonValue& root);
    void PopulateFromJSON(const JsonValue& root);
    const JsonValue* Navigate(const std::string& dotPath) const;

    std::string     m_configPath;
    JsonValue       m_root;
    mutable std::mutex m_mutex;

    LLMConfig       m_llm;
    EmotionConfig   m_emotion;
    ServiceConfig   m_service;
    TrustConfig     m_trust;
    MemoryConfig    m_memory;
    HTTPConfig      m_http;

    std::vector<std::function<void()>> m_reloadCallbacks;
};

#endif
