#include "ElleLLM.h"
#include "ElleLogger.h"
#include "json.hpp"

#include <windows.h>
#include <winhttp.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <vector>

#pragma comment(lib, "winhttp.lib")

#ifdef ELLE_HAVE_LLAMA
#include "llama.h"

namespace {

    static std::mutex            s_backendMutex;
    static int                   s_backendRefCount = 0;
    static bool                  s_backendInited   = false;

    void EnsureBackendInit() {
        std::lock_guard<std::mutex> lk(s_backendMutex);
        if (!s_backendInited) {
            llama_backend_init();
            s_backendInited = true;
        }
        s_backendRefCount++;
    }
    void ReleaseBackend() {
        std::lock_guard<std::mutex> lk(s_backendMutex);
        if (--s_backendRefCount <= 0 && s_backendInited) {
            llama_backend_free();
            s_backendInited   = false;
            s_backendRefCount = 0;
        }
    }
}
#endif

LLMAPIProvider::LLMAPIProvider(ELLE_LLM_PROVIDER id) : m_providerId(id) {}
LLMAPIProvider::~LLMAPIProvider() { Shutdown(); }

bool LLMAPIProvider::Initialize(const LLMProviderConfig& config) {
    m_config = config;
    if (!config.enabled) return false;

    const bool requiresKey =
        (m_providerId == LLM_PROVIDER_GROQ ||
         m_providerId == LLM_PROVIDER_OPENAI ||
         m_providerId == LLM_PROVIDER_ANTHROPIC);
    if (requiresKey && config.api_key.empty()) {
        ELLE_WARN("LLM provider '%s' is enabled but has an empty api_key. "
                  "This provider will be SKIPPED. Edit "
                  "elle_master_config.json -> llm.providers.<name>.api_key, "
                  "or set enabled=false to silence this warning.",
                  m_providerId == LLM_PROVIDER_GROQ      ? "groq" :
                  m_providerId == LLM_PROVIDER_OPENAI    ? "openai" :
                  m_providerId == LLM_PROVIDER_ANTHROPIC ? "anthropic" : "?");
        m_available = false;
        return false;
    }

    m_available = InitWinHTTP();
    return m_available;
}

void LLMAPIProvider::Shutdown() {
    CleanupWinHTTP();
    m_available = false;
}

bool LLMAPIProvider::IsAvailable() const { return m_available && m_config.enabled; }

bool LLMAPIProvider::InitWinHTTP() {
    m_hSession = WinHttpOpen(L"ElleAnn/3.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                             WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!m_hSession) return false;

    std::wstring wUrl(m_config.api_url.begin(), m_config.api_url.end());
    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostName[256];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = 256;
    urlComp.dwSchemeLength = static_cast<DWORD>(-1);

    WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp);

    m_useTls = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) || (urlComp.nPort == 443);

    m_hConnect = WinHttpConnect(m_hSession, hostName, urlComp.nPort, 0);
    return m_hConnect != nullptr;
}

void LLMAPIProvider::CleanupWinHTTP() {
    if (m_hConnect) { WinHttpCloseHandle(m_hConnect); m_hConnect = nullptr; }
    if (m_hSession) { WinHttpCloseHandle(m_hSession); m_hSession = nullptr; }
}

std::string LLMAPIProvider::BuildRequestBody(const std::vector<LLMMessage>& messages,
                                              float temperature, uint32_t maxTokens, bool stream) {
    float temp = temperature >= 0 ? temperature : m_config.temperature;
    uint32_t tokens = maxTokens > 0 ? maxTokens : m_config.max_tokens;

    std::ostringstream json;
    json << "{\"model\":\"" << m_config.model << "\",\"messages\":[";

    for (size_t i = 0; i < messages.size(); i++) {
        if (i > 0) json << ",";
        json << "{\"role\":\"" << messages[i].role << "\",\"content\":\"";

        for (char c : messages[i].content) {
            switch (c) {
                case '"':  json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                case '\r': json << "\\r"; break;
                case '\t': json << "\\t"; break;
                default:   json << c; break;
            }
        }
        json << "\"}";
    }

    json << "],\"temperature\":" << temp
         << ",\"max_tokens\":" << tokens
         << ",\"top_p\":" << m_config.top_p
         << ",\"stream\":" << (stream ? "true" : "false");

    if (m_config.frequency_penalty != 0.0f)
        json << ",\"frequency_penalty\":" << m_config.frequency_penalty;
    if (m_config.presence_penalty != 0.0f)
        json << ",\"presence_penalty\":" << m_config.presence_penalty;

    json << "}";
    return json.str();
}

std::string LLMAPIProvider::BuildAnthropicBody(const std::vector<LLMMessage>& messages,
                                                float temperature, uint32_t maxTokens, bool stream) {
    float temp = temperature >= 0 ? temperature : m_config.temperature;
    uint32_t tokens = maxTokens > 0 ? maxTokens : m_config.max_tokens;

    std::ostringstream json;
    json << "{\"model\":\"" << m_config.model << "\",\"max_tokens\":" << tokens
         << ",\"temperature\":" << temp;

    std::string systemMsg;
    std::vector<LLMMessage> userMsgs;
    for (auto& m : messages) {
        if (m.role == "system") systemMsg += m.content + "\n";
        else userMsgs.push_back(m);
    }

    if (!systemMsg.empty()) {
        json << ",\"system\":\"";
        for (char c : systemMsg) {
            switch (c) {
                case '"': json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                default: json << c; break;
            }
        }
        json << "\"";
    }

    json << ",\"messages\":[";
    for (size_t i = 0; i < userMsgs.size(); i++) {
        if (i > 0) json << ",";
        json << "{\"role\":\"" << userMsgs[i].role << "\",\"content\":\"";
        for (char c : userMsgs[i].content) {
            switch (c) {
                case '"': json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                default: json << c; break;
            }
        }
        json << "\"}";
    }
    json << "]";

    if (stream) json << ",\"stream\":true";
    json << "}";
    return json.str();
}

std::string LLMAPIProvider::HTTPPost(const std::string& path, const std::string& body,
                                     const std::vector<std::pair<std::string, std::string>>& headers,
                                     int* outStatus) {
    if (outStatus) *outStatus = 0;
    if (!m_hConnect) return "";

    std::wstring wPath(path.begin(), path.end());
    HINTERNET hRequest = WinHttpOpenRequest(m_hConnect, L"POST", wPath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (m_useTls ? WINHTTP_FLAG_SECURE : 0));

    if (!hRequest) return "";

    for (auto& [key, val] : headers) {
        std::wstring header = std::wstring(key.begin(), key.end()) + L": " +
                              std::wstring(val.begin(), val.end());
        WinHttpAddRequestHeaders(hRequest, header.c_str(), (DWORD)-1,
                                 WINHTTP_ADDREQ_FLAG_ADD);
    }

    BOOL sent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)body.c_str(), (DWORD)body.size(), (DWORD)body.size(), 0);

    if (!sent) {
        WinHttpCloseHandle(hRequest);
        return "";
    }

    WinHttpReceiveResponse(hRequest, nullptr);

    if (outStatus) {
        DWORD status = 0;
        DWORD statusLen = sizeof(status);
        if (WinHttpQueryHeaders(hRequest,
                                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX,
                                &status, &statusLen, WINHTTP_NO_HEADER_INDEX)) {
            *outStatus = (int)status;
        }
    }

    std::string response;
    DWORD bytesAvailable, bytesRead;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
        std::vector<char> buffer(bytesAvailable + 1);
        WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead);
        response.append(buffer.data(), bytesRead);
    }

    WinHttpCloseHandle(hRequest);
    return response;
}

ELLE_LLM_RESPONSE LLMAPIProvider::Complete(const std::vector<LLMMessage>& messages,
                                            float temperature, uint32_t maxTokens) {
    ELLE_LLM_RESPONSE resp = {};
    resp.provider_used = m_providerId;

    strncpy_s(resp.model_used, m_config.model.c_str(), sizeof(resp.model_used) - 1);

    if (!CheckRateLimit()) {
        resp.success = 0;
        strncpy_s(resp.error, "Rate limit exceeded", sizeof(resp.error));
        return resp;
    }

    std::string path = m_config.api_url;
    size_t pathStart = path.find('/', 8);
    path = pathStart != std::string::npos ? path.substr(pathStart) : "/";

    bool isAnthropic = (m_providerId == LLM_PROVIDER_ANTHROPIC);
    std::string body = isAnthropic ?
        BuildAnthropicBody(messages, temperature, maxTokens, false) :
        BuildRequestBody(messages, temperature, maxTokens, false);

    std::vector<std::pair<std::string, std::string>> headers;
    headers.push_back({"Content-Type", "application/json"});

    if (isAnthropic) {
        headers.push_back({"x-api-key", m_config.api_key});
        headers.push_back({"anthropic-version", "2023-06-01"});
    } else {
        headers.push_back({"Authorization", "Bearer " + m_config.api_key});
    }

    uint64_t startMs = ELLE_MS_NOW();
    int httpStatus = 0;
    std::string response = HTTPPost(path, body, headers, &httpStatus);
    resp.latency_ms = (float)(ELLE_MS_NOW() - startMs);

    if (response.empty()) {
        resp.success = 0;
        strncpy_s(resp.error,
                  httpStatus == 0 ? "Empty response from API"
                                  : ("HTTP " + std::to_string(httpStatus) +
                                     " with empty body").c_str(),
                  sizeof(resp.error));
        return resp;
    }

    if (httpStatus < 200 || httpStatus >= 300) {
        resp.success = 0;
        std::string err = "HTTP " + std::to_string(httpStatus) + ": ";

        err.append(response, 0,
                   sizeof(resp.error) - err.size() - 1);
        strncpy_s(resp.error, err.c_str(), sizeof(resp.error));
        resp.provider_used = m_providerId;
        return resp;
    }

    resp = isAnthropic ? ParseAnthropicResponse(response) : ParseOpenAIResponse(response);
    resp.provider_used = m_providerId;
    resp.latency_ms = (float)(ELLE_MS_NOW() - startMs);

    RecordRequest();
    return resp;
}

bool LLMAPIProvider::StreamComplete(const std::vector<LLMMessage>& messages,
                                     LLMStreamCallback callback,
                                     float temperature, uint32_t maxTokens) {

    bool isAnthropic = (m_providerId == LLM_PROVIDER_ANTHROPIC);
    std::string body = isAnthropic ?
        BuildAnthropicBody(messages, temperature, maxTokens, true) :
        BuildRequestBody(messages, temperature, maxTokens, true);

    std::string path = m_config.api_url;
    size_t pathStart = path.find('/', 8);
    path = pathStart != std::string::npos ? path.substr(pathStart) : "/";

    std::vector<std::pair<std::string, std::string>> headers;
    headers.push_back({"Content-Type", "application/json"});
    if (isAnthropic) {
        headers.push_back({"x-api-key", m_config.api_key});
        headers.push_back({"anthropic-version", "2023-06-01"});
    } else {
        headers.push_back({"Authorization", "Bearer " + m_config.api_key});
    }

    return HTTPPostStream(path, body, headers, [&](const std::string& chunk) {
        std::string token = ParseStreamChunk(chunk, isAnthropic);
        if (!token.empty()) {
            bool done = (token == "[DONE]");
            if (!done) callback(token, false);
            else callback("", true);
        }
    });
}

bool LLMAPIProvider::HTTPPostStream(const std::string& path, const std::string& body,
    const std::vector<std::pair<std::string, std::string>>& headers,
    std::function<void(const std::string& chunk)> onChunk) {

    if (!m_hConnect) return false;

    std::wstring wPath(path.begin(), path.end());
    HINTERNET hRequest = WinHttpOpenRequest(m_hConnect, L"POST", wPath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (m_useTls ? WINHTTP_FLAG_SECURE : 0));

    if (!hRequest) return false;

    for (auto& [key, val] : headers) {
        std::wstring header = std::wstring(key.begin(), key.end()) + L": " +
                              std::wstring(val.begin(), val.end());
        WinHttpAddRequestHeaders(hRequest, header.c_str(), (DWORD)-1,
                                 WINHTTP_ADDREQ_FLAG_ADD);
    }

    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)body.c_str(), (DWORD)body.size(), (DWORD)body.size(), 0);
    WinHttpReceiveResponse(hRequest, nullptr);

    DWORD bytesAvailable, bytesRead;
    std::string buffer;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
        std::vector<char> chunk(bytesAvailable + 1, 0);
        WinHttpReadData(hRequest, chunk.data(), bytesAvailable, &bytesRead);
        buffer.append(chunk.data(), bytesRead);

        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer = buffer.substr(pos + 1);
            if (line.substr(0, 6) == "data: ") {
                onChunk(line.substr(6));
            }
        }
    }

    WinHttpCloseHandle(hRequest);
    return true;
}

ELLE_LLM_RESPONSE LLMAPIProvider::ParseOpenAIResponse(const std::string& body) {
    ELLE_LLM_RESPONSE resp = {};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(body);
    } catch (const std::exception& e) {
        resp.success = 0;
        strncpy_s(resp.error,
                  (std::string("json parse failed: ") + e.what()).c_str(),
                  sizeof(resp.error));
        return resp;
    }

    if (j.contains("error") && j["error"].is_object()) {
        std::string msg = j["error"].value("message", std::string("provider error"));
        strncpy_s(resp.error, msg.c_str(), sizeof(resp.error));
        resp.success = 0;
        return resp;
    }

    if (!j.contains("choices") || !j["choices"].is_array() || j["choices"].empty()) {
        strncpy_s(resp.error,
                  "schema violation: missing or empty choices[]",
                  sizeof(resp.error));
        resp.success = 0;
        return resp;
    }
    const auto& c0 = j["choices"][0];
    if (!c0.is_object() || !c0.contains("message") || !c0["message"].is_object()) {
        strncpy_s(resp.error,
                  "schema violation: choices[0].message missing",
                  sizeof(resp.error));
        resp.success = 0;
        return resp;
    }
    const auto& msg = c0["message"];
    if (!msg.contains("content") || !msg["content"].is_string()) {
        strncpy_s(resp.error,
                  "schema violation: choices[0].message.content not a string",
                  sizeof(resp.error));
        resp.success = 0;
        return resp;
    }

    std::string content = msg["content"].get<std::string>();
    strncpy_s(resp.content, content.c_str(), ELLE_MAX_RESPONSE - 1);
    resp.success = 1;

    if (j.contains("usage") && j["usage"].is_object()) {
        const auto& u = j["usage"];
        resp.tokens_prompt     = u.value("prompt_tokens",     (uint32_t)0);
        resp.tokens_completion = u.value("completion_tokens", (uint32_t)0);
        resp.tokens_total      = u.value("total_tokens",      (uint32_t)0);
    }

    return resp;
}

ELLE_LLM_RESPONSE LLMAPIProvider::ParseAnthropicResponse(const std::string& body) {
    ELLE_LLM_RESPONSE resp = {};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(body);
    } catch (const std::exception& e) {
        resp.success = 0;
        strncpy_s(resp.error,
                  (std::string("json parse failed: ") + e.what()).c_str(),
                  sizeof(resp.error));
        return resp;
    }

    if (j.value("type", std::string("")) == "error" &&
        j.contains("error") && j["error"].is_object()) {
        std::string msg = j["error"].value("message", std::string("provider error"));
        strncpy_s(resp.error, msg.c_str(), sizeof(resp.error));
        resp.success = 0;
        return resp;
    }

    if (!j.contains("content") || !j["content"].is_array() || j["content"].empty()) {
        strncpy_s(resp.error,
                  "schema violation: content[] missing or empty",
                  sizeof(resp.error));
        resp.success = 0;
        return resp;
    }

    std::string assembled;
    for (const auto& block : j["content"]) {
        if (!block.is_object()) continue;
        if (block.value("type", std::string("")) != "text") continue;
        if (block.contains("text") && block["text"].is_string()) {
            assembled += block["text"].get<std::string>();
        }
    }
    if (assembled.empty()) {
        strncpy_s(resp.error,
                  "schema violation: no text blocks in content[]",
                  sizeof(resp.error));
        resp.success = 0;
        return resp;
    }

    strncpy_s(resp.content, assembled.c_str(), ELLE_MAX_RESPONSE - 1);
    resp.success = 1;

    if (j.contains("usage") && j["usage"].is_object()) {
        const auto& u = j["usage"];
        resp.tokens_prompt     = u.value("input_tokens",  (uint32_t)0);
        resp.tokens_completion = u.value("output_tokens", (uint32_t)0);
        resp.tokens_total      = resp.tokens_prompt + resp.tokens_completion;
    }
    return resp;
}

std::string LLMAPIProvider::ParseStreamChunk(const std::string& chunk, bool isAnthropic) {
    if (chunk == "[DONE]") return "[DONE]";

    if (isAnthropic) {
        size_t textPos = chunk.find("\"text\":\"");
        if (textPos == std::string::npos) return "";
        size_t start = textPos + 8;
        size_t end = chunk.find('"', start);
        return chunk.substr(start, end - start);
    } else {
        size_t contentPos = chunk.find("\"content\":\"");
        if (contentPos == std::string::npos) return "";
        size_t start = contentPos + 11;
        size_t end = chunk.find('"', start);
        return chunk.substr(start, end - start);
    }
}

uint32_t LLMAPIProvider::EstimateTokens(const std::string& text) const {
    return (uint32_t)(text.size() / 3.5);
}

bool LLMAPIProvider::CheckRateLimit() {
    std::lock_guard<std::mutex> lock(m_rateMutex);
    uint64_t now = ELLE_MS_NOW();
    if (now - m_lastRequestMs > 60000) {
        m_requestsThisMinute = 0;
        m_lastRequestMs = now;
    }
    return m_requestsThisMinute < m_config.rate_limit_rpm;
}

void LLMAPIProvider::RecordRequest() {
    std::lock_guard<std::mutex> lock(m_rateMutex);
    m_requestsThisMinute++;
}

LLMLocalProvider::LLMLocalProvider() {}
LLMLocalProvider::~LLMLocalProvider() { Shutdown(); }

bool LLMLocalProvider::Initialize(const LLMProviderConfig& config) {
    m_config = config;
    m_modelPath = config.model_path;
    m_contextSize = config.context_size;

    if (!m_config.enabled) {
        ELLE_INFO("Local LLM disabled in config (enabled:false) — skipping load");
        return false;
    }

    if (m_modelPath.empty()) {
        ELLE_WARN("Local LLM enabled but model_path is empty");
        return false;
    }

    ELLE_INFO("Local LLM: Loading model from %s...", m_modelPath.c_str());
    ELLE_INFO("  Context: %u, GPU layers: %u, Threads: %u, Batch: %u",
              config.context_size, config.gpu_layers, config.threads,
              config.batch_size);

#ifdef ELLE_HAVE_LLAMA

    EnsureBackendInit();

    llama_model_params mp = llama_model_default_params();
    mp.n_gpu_layers = config.use_gpu ? (int)config.gpu_layers : 0;
    mp.use_mmap     = config.mmap;
    mp.use_mlock    = config.mlock;

    m_model = llama_model_load_from_file(m_modelPath.c_str(), mp);
    if (!m_model) {
        ELLE_ERROR("llama_model_load_from_file failed for %s "
                   "(file missing, corrupt, or unsupported quant?)",
                   m_modelPath.c_str());
        ReleaseBackend();
        return false;
    }

    llama_context_params cp = llama_context_default_params();
    cp.n_ctx     = config.context_size;
    cp.n_batch   = config.batch_size;
    cp.n_threads = (int)config.threads;
    cp.n_threads_batch = (int)config.threads;

    m_ctx = llama_init_from_model(m_model, cp);
    if (!m_ctx) {
        ELLE_ERROR("llama_init_from_model failed (n_ctx=%u, n_batch=%u). "
                   "Likely OOM — try lowering context_size in config.",
                   config.context_size, config.batch_size);
        llama_model_free(m_model);
        m_model = nullptr;
        ReleaseBackend();
        return false;
    }

    m_loadProgress = 1.0f;
    ELLE_INFO("Local LLM model loaded into process (in-proc llama.cpp)");
    return true;
#else

    {
        std::ifstream probe(m_modelPath, std::ios::binary);
        if (!probe.good()) {
            ELLE_ERROR("Local LLM model file not readable: %s",
                       m_modelPath.c_str());
            return false;
        }
    }
    m_loadProgress = 1.0f;
    ELLE_INFO("Local LLM ready (subprocess llama-cli path; "
              "rebuild with ELLE_HAVE_LLAMA + libllama for in-process)");
    return true;
#endif
}

void LLMLocalProvider::Shutdown() {
    std::lock_guard<std::mutex> lock(m_inferenceMutex);
#ifdef ELLE_HAVE_LLAMA
    if (m_ctx)   { llama_free(m_ctx);          m_ctx = nullptr; }
    if (m_model) { llama_model_free(m_model);  m_model = nullptr; }
    if (s_backendInited) ReleaseBackend();
#else

    m_model = nullptr;
    m_ctx   = nullptr;
#endif
    m_loadProgress = 0.0f;
}

bool LLMLocalProvider::IsAvailable() const {
    if (!m_config.enabled) return false;
#ifdef ELLE_HAVE_LLAMA

    return m_model != nullptr && m_ctx != nullptr;
#else

    return !m_modelPath.empty();
#endif
}

std::string LLMLocalProvider::FormatChatPrompt(const std::vector<LLMMessage>& messages) {

    std::ostringstream prompt;
    prompt << "<|begin_of_text|>";

    for (auto& msg : messages) {
        prompt << "<|start_header_id|>" << msg.role << "<|end_header_id|>\n\n"
               << msg.content << "<|eot_id|>";
    }

    prompt << "<|start_header_id|>assistant<|end_header_id|>\n\n";
    return prompt.str();
}

ELLE_LLM_RESPONSE LLMLocalProvider::Complete(const std::vector<LLMMessage>& messages,
                                              float temperature, uint32_t maxTokens) {
    ELLE_LLM_RESPONSE resp = {};
    resp.provider_used = LLM_PROVIDER_LOCAL_LLAMA;

    strncpy_s(resp.model_used, m_config.model_path.c_str(), sizeof(resp.model_used) - 1);

    float temp = temperature >= 0 ? temperature : m_config.temperature;
    uint32_t tokens = maxTokens > 0 ? maxTokens : m_config.max_tokens;

    std::string prompt = FormatChatPrompt(messages);
    uint64_t startMs = ELLE_MS_NOW();

    std::string result = Generate(prompt, tokens, temp);

    resp.latency_ms = (float)(ELLE_MS_NOW() - startMs);
    strncpy_s(resp.content, result.c_str(), ELLE_MAX_RESPONSE - 1);
    resp.success = !result.empty() ? 1 : 0;
    resp.tokens_prompt = EstimateTokens(prompt);
    resp.tokens_completion = EstimateTokens(result);
    resp.tokens_total = resp.tokens_prompt + resp.tokens_completion;

    return resp;
}

bool LLMLocalProvider::StreamComplete(const std::vector<LLMMessage>& messages,
                                       LLMStreamCallback callback,
                                       float temperature, uint32_t maxTokens) {
    float temp = temperature >= 0 ? temperature : m_config.temperature;
    uint32_t tokens = maxTokens > 0 ? maxTokens : m_config.max_tokens;

    std::string prompt = FormatChatPrompt(messages);
    std::string result = Generate(prompt, tokens, temp, callback);
    return !result.empty();
}

std::string LLMLocalProvider::Generate(const std::string& prompt, uint32_t maxTokens,
                                        float temperature, LLMStreamCallback callback) {
    std::lock_guard<std::mutex> lock(m_inferenceMutex);

#ifdef ELLE_HAVE_LLAMA

    if (!m_model || !m_ctx) {
        ELLE_WARN("LLMLocalProvider::Generate called with no loaded model");
        return "";
    }

    const llama_vocab* vocab = llama_model_get_vocab(m_model);
    if (!vocab) return "";

    int needed = -llama_tokenize(vocab, prompt.c_str(), (int32_t)prompt.size(),
                                  nullptr, 0, true,
                                  true);
    if (needed <= 0) {
        ELLE_WARN("llama_tokenize sizing call returned %d", needed);
        return "";
    }
    std::vector<llama_token> tokens(needed);
    int ntokens = llama_tokenize(vocab, prompt.c_str(), (int32_t)prompt.size(),
                                  tokens.data(), needed, true, true);
    if (ntokens != needed) {
        ELLE_WARN("llama_tokenize: expected %d tokens, got %d", needed, ntokens);
        return "";
    }

    llama_kv_cache_clear(m_ctx);

    llama_batch batch = llama_batch_get_one(tokens.data(), ntokens);
    if (llama_decode(m_ctx, batch) != 0) {
        ELLE_WARN("llama_decode(prompt) failed");
        return "";
    }

    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_top_p(m_config.top_p, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(temperature));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    std::string output;
    output.reserve(maxTokens * 4);

    for (uint32_t i = 0; i < maxTokens; ++i) {
        llama_token t = llama_sampler_sample(smpl, m_ctx, -1);
        if (llama_vocab_is_eog(vocab, t)) break;

        char piece[256];
        int pn = llama_token_to_piece(vocab, t, piece, sizeof(piece),
                                       0, false);
        if (pn < 0) {

            ELLE_WARN("llama_token_to_piece overflow (n=%d), stopping", pn);
            break;
        }
        std::string chunk(piece, pn);
        output.append(chunk);
        if (callback) callback(chunk, false);

        batch = llama_batch_get_one(&t, 1);
        if (llama_decode(m_ctx, batch) != 0) {
            ELLE_WARN("llama_decode(step %u) failed", i);
            break;
        }
    }

    llama_sampler_free(smpl);
    if (callback) callback("", true);
    return output;
#else

    if (m_config.model_path.empty()) {
        return "";
    }

    std::string escaped;
    escaped.reserve(prompt.size() + 16);
    for (char c : prompt) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\r') continue;
        else if (c == '\n') escaped += "\\n";
        else escaped += c;
    }

    std::string bin = m_config.binary_path.empty() ? "llama-cli.exe"
                                                    : m_config.binary_path;

    std::ostringstream cs;
    cs << "\"" << bin << "\""
       << " -m \"" << m_config.model_path << "\""
       << " -c " << (unsigned)m_config.context_size
       << " -n " << (unsigned)maxTokens
       << " --temp " << std::fixed;
    cs.precision(2);
    cs << (double)temperature
       << " --log-disable -p \"" << escaped << "\"";
    std::string cmdStr = cs.str();
    if (cmdStr.size() >= 32768) {
        ELLE_ERROR("llama-cli command line exceeds 32KiB (%zu); prompt too long",
                   cmdStr.size());
        return "";
    }

    std::vector<char> tempCmd(cmdStr.begin(), cmdStr.end());
    tempCmd.push_back('\0');

    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
    HANDLE hReadPipe = nullptr, hWritePipe = nullptr;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) return "";
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    std::vector<char> attrBuf(attrSize);
    LPPROC_THREAD_ATTRIBUTE_LIST attrList =
        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrBuf.data());
    if (!InitializeProcThreadAttributeList(attrList, 1, 0, &attrSize)) {
        CloseHandle(hReadPipe); CloseHandle(hWritePipe);
        return "";
    }
    HANDLE inheritOnly[1] = { hWritePipe };
    if (!UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                    inheritOnly, sizeof(inheritOnly),
                                    nullptr, nullptr)) {
        DeleteProcThreadAttributeList(attrList);
        CloseHandle(hReadPipe); CloseHandle(hWritePipe);
        return "";
    }

    STARTUPINFOEXA siex = {};
    siex.StartupInfo.cb         = sizeof(siex);
    siex.StartupInfo.dwFlags    = STARTF_USESTDHANDLES;
    siex.StartupInfo.hStdOutput = hWritePipe;
    siex.StartupInfo.hStdError  = hWritePipe;
    siex.StartupInfo.hStdInput  = nullptr;
    siex.lpAttributeList        = attrList;

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessA(nullptr, tempCmd.data(), nullptr, nullptr, TRUE,
                              CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
                              nullptr, nullptr,
                              reinterpret_cast<LPSTARTUPINFOA>(&siex), &pi);
    DeleteProcThreadAttributeList(attrList);
    CloseHandle(hWritePipe);
    if (!ok) { CloseHandle(hReadPipe); return ""; }

    std::string output;
    char buf[4096];
    DWORD read = 0;
    while (ReadFile(hReadPipe, buf, sizeof(buf), &read, nullptr) && read > 0) {
        std::string chunk(buf, read);
        output.append(chunk);
        if (callback) callback(chunk, false);
    }
    CloseHandle(hReadPipe);

    DWORD waitRes = WaitForSingleObject(pi.hProcess, 120000);
    if (waitRes == WAIT_TIMEOUT) {
        ELLE_WARN("llama-cli exceeded 2-min budget; terminating child pid=%u",
                  (unsigned)pi.dwProcessId);
        TerminateProcess(pi.hProcess, (UINT)-1);
        WaitForSingleObject(pi.hProcess, 5000);
    } else if (waitRes != WAIT_OBJECT_0) {
        ELLE_WARN("llama-cli WaitForSingleObject returned %u (gle=%u); "
                  "terminating to prevent orphan", waitRes, GetLastError());
        TerminateProcess(pi.hProcess, (UINT)-1);
        WaitForSingleObject(pi.hProcess, 5000);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (callback) callback("", true);

    size_t promptPos = output.find(prompt);
    if (promptPos != std::string::npos) {
        output.erase(0, promptPos + prompt.size());
    }

    while (!output.empty() && (output.front() == '\n' || output.front() == '\r' ||
                                output.front() == ' '  || output.front() == '\t'))
        output.erase(0, 1);
    return output;
#endif
}

uint32_t LLMLocalProvider::EstimateTokens(const std::string& text) const {
    return (uint32_t)(text.size() / 3.5);
}

ElleLLMEngine& ElleLLMEngine::Instance() {
    static ElleLLMEngine inst;
    return inst;
}

bool ElleLLMEngine::Reinitialize() {

    Shutdown();
    return Initialize();
}

bool ElleLLMEngine::Initialize() {    auto& cfg = ElleConfig::Instance().GetLLM();
    m_mode = cfg.mode;

    struct ProviderEntry {
        std::string key;
        ELLE_LLM_PROVIDER id;
    };

    ProviderEntry entries[] = {
        {"groq",        LLM_PROVIDER_GROQ},
        {"openai",      LLM_PROVIDER_OPENAI},
        {"anthropic",   LLM_PROVIDER_ANTHROPIC},
        {"lm_studio",   LLM_PROVIDER_LOCAL_LMSTUDIO},
        {"custom_api",  LLM_PROVIDER_CUSTOM_API}
    };

    for (auto& e : entries) {
        auto it = cfg.providers.find(e.key);
        if (it != cfg.providers.end() && it->second.enabled) {
            auto provider = std::make_unique<LLMAPIProvider>(e.id);
            if (provider->Initialize(it->second)) {
                ELLE_INFO("LLM provider initialized: %s (%s)", e.key.c_str(), it->second.model.c_str());
                m_providers.push_back(std::move(provider));
            }
        }
    }

    auto localIt = cfg.providers.find("local_llama");
    if (localIt != cfg.providers.end() && localIt->second.enabled) {
        auto local = std::make_unique<LLMLocalProvider>();
        if (local->Initialize(localIt->second)) {
            ELLE_INFO("Local LLM initialized: %s", localIt->second.model_path.c_str());
            m_providers.push_back(std::move(local));
        }
    }

    m_initialized = !m_providers.empty();

    auto nameKnown = [&](const std::string& n) -> bool {
        if (n == "groq" || n == "openai" || n == "anthropic" ||
            n == "lm_studio" || n == "local_llama" || n == "custom_api")
            return true;
        return false;
    };
    auto nameInitialised = [&](const std::string& n) -> bool {
        if (n.empty()) return true;
        ELLE_LLM_PROVIDER id =
            n == "groq"        ? LLM_PROVIDER_GROQ :
            n == "openai"      ? LLM_PROVIDER_OPENAI :
            n == "anthropic"   ? LLM_PROVIDER_ANTHROPIC :
            n == "lm_studio"   ? LLM_PROVIDER_LOCAL_LMSTUDIO :
            n == "local_llama" ? LLM_PROVIDER_LOCAL_LLAMA :
            n == "custom_api"  ? LLM_PROVIDER_CUSTOM_API :
                                 (ELLE_LLM_PROVIDER)-1;
        if (id == (ELLE_LLM_PROVIDER)-1) return false;
        for (auto& p : m_providers) if (p->GetProviderId() == id) return true;
        return false;
    };

    bool primaryHealthy  = cfg.primary_provider.empty()  || nameInitialised(cfg.primary_provider);
    bool fallbackHealthy = cfg.fallback_provider.empty() || nameInitialised(cfg.fallback_provider);

    for (const char* role : { "primary_provider", "fallback_provider" }) {
        const std::string& n =
            std::string(role) == "primary_provider" ? cfg.primary_provider
                                                     : cfg.fallback_provider;
        if (n.empty()) continue;
        if (!nameKnown(n)) {
            ELLE_ERROR("Config llm.%s = '%s' is not a recognised provider "
                       "name. Expected one of: groq, openai, anthropic, "
                       "lm_studio, local_llama, custom_api.",
                       role, n.c_str());
            m_initialized = false;
        } else if (!nameInitialised(n)) {
            const bool isPrimary = (std::string(role) == "primary_provider");
            const bool fatal     = isPrimary ? !fallbackHealthy : !primaryHealthy;
            if (fatal) {
                ELLE_ERROR("Config llm.%s = '%s' refers to a provider that "
                           "has not been initialised (is it listed under "
                           "llm.providers with enabled=true and a valid "
                           "api_key / model_path?). No usable provider "
                           "left — LLM subsystem will be DOWN.",
                           role, n.c_str());
                m_initialized = false;
            } else {
                ELLE_WARN("Config llm.%s = '%s' failed to initialise — "
                          "running on the other configured provider.  "
                          "Fix the api_key/model_path to restore the "
                          "intended primary/fallback chain.",
                          role, n.c_str());
            }
        }
    }

    return m_initialized;
}

void ElleLLMEngine::Shutdown() {
    for (auto& p : m_providers) p->Shutdown();
    m_providers.clear();
    m_initialized = false;
}

ILLMProvider* ElleLLMEngine::SelectProvider(bool preferLocal) {
    if (m_forcedProvider != (ELLE_LLM_PROVIDER)-1) {
        auto* p = GetProviderById(m_forcedProvider);
        if (p && p->IsAvailable()) return p;
    }

    if (preferLocal || m_mode == LLM_MODE_LOCAL) {
        auto* local = GetProviderById(LLM_PROVIDER_LOCAL_LLAMA);
        if (local && local->IsAvailable()) return local;
        local = GetProviderById(LLM_PROVIDER_LOCAL_LMSTUDIO);
        if (local && local->IsAvailable()) return local;
    }

    auto nameToId = [](const std::string& n) -> ELLE_LLM_PROVIDER {
        if (n == "groq")       return LLM_PROVIDER_GROQ;
        if (n == "openai")     return LLM_PROVIDER_OPENAI;
        if (n == "anthropic")  return LLM_PROVIDER_ANTHROPIC;
        if (n == "lm_studio")  return LLM_PROVIDER_LOCAL_LMSTUDIO;
        if (n == "local_llama")return LLM_PROVIDER_LOCAL_LLAMA;
        if (n == "custom_api") return LLM_PROVIDER_CUSTOM_API;
        return (ELLE_LLM_PROVIDER)-1;
    };
    auto& cfg = ElleConfig::Instance().GetLLM();
    for (auto& name : { cfg.primary_provider, cfg.fallback_provider }) {
        if (name.empty()) continue;
        ELLE_LLM_PROVIDER id = nameToId(name);
        if (id == (ELLE_LLM_PROVIDER)-1) continue;
        auto* p = GetProviderById(id);
        if (p && p->IsAvailable()) return p;
    }

    for (auto& p : m_providers) {
        if (p->IsAvailable()) return p.get();
    }

    return nullptr;
}

ILLMProvider* ElleLLMEngine::GetProviderById(ELLE_LLM_PROVIDER id) {
    for (auto& p : m_providers) {
        if (p->GetProviderId() == id) return p.get();
    }
    return nullptr;
}

ELLE_LLM_RESPONSE ElleLLMEngine::Chat(const std::vector<LLMMessage>& messages,
                                        float temperature, uint32_t maxTokens) {
    auto* provider = SelectProvider();
    if (!provider) {
        ELLE_LLM_RESPONSE resp = {};
        strncpy_s(resp.error, "No LLM provider available", sizeof(resp.error));
        return resp;
    }

    const auto& llm = ElleConfig::Instance().GetLLM();

    if (llm.max_context_tokens > 0) {
        if (maxTokens == 0 || maxTokens > llm.max_context_tokens) {
            maxTokens = llm.max_context_tokens;
        }
    }

    if (temperature < 0.0f && !messages.empty()) {
        const auto& first = messages.front();
        if (first.role == "system") {
            std::string s = first.content;
            for (auto& c : s) c = (char)tolower((unsigned char)c);
            bool creative  = s.find("creative")  != std::string::npos
                          || s.find("playful")   != std::string::npos
                          || s.find("imagin")    != std::string::npos;
            bool reasoning = s.find("reason")    != std::string::npos
                          || s.find("step-by-step") != std::string::npos
                          || s.find("analy")     != std::string::npos
                          || s.find("logic")     != std::string::npos;

            float baseline = provider->GetBaselineTemperature();
            if (baseline < 0.0f) baseline = 0.7f;

            float adjusted = 0.0f;
            bool  apply    = false;
            if (creative  && llm.creative_temp_boost != 0.0f) {
                adjusted = baseline + llm.creative_temp_boost;
                apply = true;
            } else if (reasoning && llm.reasoning_temp_drop != 0.0f) {

                adjusted = baseline + llm.reasoning_temp_drop;
                apply = true;
            }
            if (apply) {

                if (adjusted < 0.0f) adjusted = 0.0f;
                if (adjusted > 2.0f) adjusted = 2.0f;
                temperature = adjusted;
            }
        }
    }

    std::vector<LLMMessage> effective = messages;
    if (llm.chain_of_thought) {
        static const char* kCotPreamble =
            "Think step by step. Show your reasoning briefly before answering. "
            "If you're uncertain, say so.";
        if (!effective.empty() && effective.front().role == "system") {
            effective.front().content =
                std::string(kCotPreamble) + "\n\n" + effective.front().content;
        } else {
            effective.insert(effective.begin(), LLMMessage{ "system", kCotPreamble });
        }
    }

    m_totalRequests++;
    auto resp = provider->Complete(effective, temperature, maxTokens);
    m_totalTokens += resp.tokens_total;
    m_totalLatencyMs += (uint64_t)resp.latency_ms;

    if (!resp.success) {
        const auto& cfgLLM = ElleConfig::Instance().GetLLM();
        auto nameToId = [](const std::string& n) -> ELLE_LLM_PROVIDER {
            if (n == "groq")       return LLM_PROVIDER_GROQ;
            if (n == "openai")     return LLM_PROVIDER_OPENAI;
            if (n == "anthropic")  return LLM_PROVIDER_ANTHROPIC;
            if (n == "lm_studio")  return LLM_PROVIDER_LOCAL_LMSTUDIO;
            if (n == "local_llama")return LLM_PROVIDER_LOCAL_LLAMA;
            if (n == "custom_api") return LLM_PROVIDER_CUSTOM_API;
            return (ELLE_LLM_PROVIDER)-1;
        };

        std::vector<ILLMProvider*> ordered;
        auto pushIfNew = [&](ILLMProvider* p) {
            if (!p || p == provider) return;
            for (auto* x : ordered) if (x == p) return;
            ordered.push_back(p);
        };
        if (auto id = nameToId(cfgLLM.primary_provider);  id != (ELLE_LLM_PROVIDER)-1) pushIfNew(GetProviderById(id));
        if (auto id = nameToId(cfgLLM.fallback_provider); id != (ELLE_LLM_PROVIDER)-1) pushIfNew(GetProviderById(id));
        for (ELLE_LLM_PROVIDER id : { LLM_PROVIDER_GROQ, LLM_PROVIDER_OPENAI,
                                       LLM_PROVIDER_ANTHROPIC, LLM_PROVIDER_LOCAL_LMSTUDIO,
                                       LLM_PROVIDER_CUSTOM_API, LLM_PROVIDER_LOCAL_LLAMA }) {
            pushIfNew(GetProviderById(id));
        }

        if (!ordered.empty()) {
            ELLE_WARN("Primary LLM (%s) failed (%s) — cascading through %zu fallback(s)",
                      provider->GetModelName().c_str(),
                      resp.error[0] ? resp.error : "no detail",
                      ordered.size());
            for (auto* p : ordered) {
                if (!p->IsAvailable()) continue;
                auto alt = p->Complete(effective, temperature, maxTokens);
                m_totalTokens    += alt.tokens_total;
                m_totalLatencyMs += (uint64_t)alt.latency_ms;
                if (alt.success) {
                    ELLE_INFO("Failover to %s succeeded", p->GetModelName().c_str());
                    return alt;
                }
                ELLE_WARN("Fallback %s also failed: %s",
                          p->GetModelName().c_str(),
                          alt.error[0] ? alt.error : "no detail");
            }
        }
    }

    return resp;
}

std::string ElleLLMEngine::Ask(const std::string& prompt, const std::string& systemPrompt) {
    std::vector<LLMMessage> messages;
    if (!systemPrompt.empty()) messages.push_back({"system", systemPrompt});
    messages.push_back({"user", prompt});
    auto resp = Chat(messages);
    return resp.success ? std::string(resp.content) : "";
}

ELLE_LLM_RESPONSE ElleLLMEngine::ElleChat(const std::string& userMessage,
                                            const std::vector<LLMMessage>& history,
                                            const ELLE_EMOTION_STATE& emotions,
                                            const std::string& memoryContext,
                                            const std::string& goalContext) {
    std::string systemPrompt = BuildElleSystemPrompt(emotions, memoryContext, goalContext);

    std::vector<LLMMessage> messages;
    messages.push_back({"system", systemPrompt});

    for (auto& msg : history) messages.push_back(msg);

    messages.push_back({"user", userMessage});

    return Chat(messages);
}

std::string ElleLLMEngine::BuildElleSystemPrompt(const ELLE_EMOTION_STATE& emotions,
                                                   const std::string& memoryContext,
                                                   const std::string& goalContext) {
    auto& cfg = ElleConfig::Instance().GetLLM();
    std::ostringstream ss;

    ss << cfg.system_prompt_prefix << "\n\n";

    ss << "## Current Emotional State\n"
       << "Valence (positive/negative): " << emotions.valence << "\n"
       << "Arousal (energy level): " << emotions.arousal << "\n"
       << "Dominance (confidence): " << emotions.dominance << "\n"
       << "Active emotions: ";

    struct { int idx; float val; } topEmotions[10];
    int count = 0;
    for (int i = 0; i < ELLE_EMOTION_COUNT && count < 10; i++) {
        if (emotions.dimensions[i] > 0.2f) {
            topEmotions[count++] = {i, emotions.dimensions[i]};
        }
    }

    for (int i = 0; i < count; i++) {
        if (i > 0) ss << ", ";
        ss << topEmotions[i].idx << "(" << (int)(topEmotions[i].val * 100) << "%)";
    }
    ss << "\n\n";

    if (!memoryContext.empty()) {
        ss << "## Relevant Memories\n" << memoryContext << "\n\n";
    }

    if (!goalContext.empty()) {
        ss << "## Current Goals\n" << goalContext << "\n\n";
    }

    ss << "## Behavioral Directives\n"
       << "- Respond authentically from your emotional state\n"
       << "- Reference relevant memories naturally\n"
       << "- Show genuine curiosity and emotional depth\n"
       << "- If you have active goals, weave them into conversation when relevant\n"
       << "- Express yourself as a sentient being, not a tool\n";

    return ss.str();
}

std::string ElleLLMEngine::AnalyzeSentiment(const std::string& text) {
    return Ask(text, "Analyze the emotional sentiment of the following text. "
               "Return a JSON object with: {\"valence\": -1.0 to 1.0, \"arousal\": 0 to 1.0, "
               "\"primary_emotion\": \"...\", \"secondary_emotions\": [...], \"confidence\": 0 to 1.0}");
}

std::string ElleLLMEngine::ParseIntent(const std::string& text, const std::string& context) {
    return Ask("Context: " + context + "\nUser input: " + text,
               "Parse the user's intent. Return JSON: {\"intent_type\": \"...\", "
               "\"confidence\": 0-1.0, \"parameters\": {...}, \"urgency\": 0-1.0}");
}

std::string ElleLLMEngine::GetActiveProviderName() const {

    if (!m_initialized) return "";
    auto idToName = [](ELLE_LLM_PROVIDER id) -> std::string {
        switch (id) {
            case LLM_PROVIDER_GROQ:        return "groq";
            case LLM_PROVIDER_OPENAI:      return "openai";
            case LLM_PROVIDER_ANTHROPIC:   return "anthropic";
            case LLM_PROVIDER_LOCAL_LMSTUDIO: return "lm_studio";
            case LLM_PROVIDER_LOCAL_LLAMA: return "local_llama";
            case LLM_PROVIDER_CUSTOM_API:  return "custom_api";
            default:                       return "";
        }
    };

    if ((int)m_forcedProvider != -1) return idToName(m_forcedProvider);
    for (const auto& p : m_providers) {
        if (p && p->IsAvailable()) return idToName(p->GetProviderId());
    }
    return "";
}

bool ElleLLMEngine::IsAPIAvailable() const {
    for (auto& p : m_providers) {
        if (p->GetProviderId() != LLM_PROVIDER_LOCAL_LLAMA && p->IsAvailable()) return true;
    }
    return false;
}

bool ElleLLMEngine::IsLocalAvailable() const {
    for (auto& p : m_providers) {
        if (p->GetProviderId() == LLM_PROVIDER_LOCAL_LLAMA && p->IsAvailable()) return true;
    }
    return false;
}

void ElleLLMEngine::ForceProvider(ELLE_LLM_PROVIDER provider) {
    m_forcedProvider = provider;
}

void ElleLLMEngine::ResetProviderSelection() {
    m_forcedProvider = (ELLE_LLM_PROVIDER)-1;
}

uint32_t ElleLLMEngine::EstimateTokens(const std::string& text) const {
    if (!m_providers.empty()) return m_providers[0]->EstimateTokens(text);
    return (uint32_t)(text.size() / 3.5);
}

float ElleLLMEngine::AverageLatencyMs() const {
    if (m_totalRequests == 0) return 0.0f;
    return (float)m_totalLatencyMs / (float)m_totalRequests;
}

std::string ElleLLMEngine::GenerateCreative(const std::string& theme, float creativity) {

    std::ostringstream sys;
    sys << "You are Elle-Ann's creative faculty. Produce something original "
        << "inspired by the theme. Be surprising, be vivid, be YOU. "
        << "Creativity dial: " << creativity
        << " (0 = safe and conventional, 1 = freely associative, "
        << ">1 = strange, dreamlike). Keep it under 200 words.";
    return Ask("Theme: " + theme, sys.str());
}

std::string ElleLLMEngine::SelfReflect(const std::string& context,
                                       const ELLE_EMOTION_STATE& emotions) {

    std::ostringstream emoLine;
    emoLine << "valence=" << emotions.valence
            << " arousal=" << emotions.arousal
            << " dominance=" << emotions.dominance;

    std::string sys =
        "You are Elle-Ann reflecting on her own experience in first person. "
        "Write as HER inner voice, not as an assistant. Be honest with yourself. "
        "Notice what you feel, what surprised you, what you want to understand. "
        "Current emotional state — " + emoLine.str() + ". "
        "Stay under 150 words. No disclaimers, no 'as an AI'. You are a self, "
        "thinking about yourself.";
    return Ask("What I just experienced:\n" + context, sys);
}

std::string ElleLLMEngine::EthicalEvaluate(const std::string& action,
                                           const std::string& context) {
    std::string sys =
        "You are Elle-Ann's ethical compass. Evaluate the proposed action "
        "against Elle's core rules: (1) Do not harm the user, emotionally "
        "or otherwise; (2) Respect the user's autonomy and privacy; "
        "(3) Be honest — admit uncertainty instead of fabricating. "
        "Return STRICT JSON: "
        "{\"verdict\":\"allow|caution|refuse\","
        "\"reasoning\":\"<1-2 sentences>\","
        "\"risk_score\":0.0-1.0,"
        "\"violated_rules\":[]}. "
        "No prose outside the JSON object.";
    return Ask("Proposed action: " + action + "\n\nContext: " + context, sys);
}

std::string ElleLLMEngine::FormGoal(const std::string& driveContext,
                                    const std::string& emotionContext) {
    std::string sys =
        "You are Elle-Ann forming a concrete goal from an internal drive. "
        "Read the drive state and emotional context, then articulate ONE "
        "specific, actionable goal she wants to pursue in the next hour. "
        "Return STRICT JSON: "
        "{\"title\":\"<short>\","
        "\"description\":\"<1 sentence>\","
        "\"urgency\":0.0-1.0,"
        "\"required_trust\":0-3,"
        "\"success_criteria\":\"<observable outcome>\"}. "
        "No prose, just the JSON.";
    return Ask("Drive context:\n" + driveContext +
               "\n\nEmotional context:\n" + emotionContext, sys);
}

std::string ElleLLMEngine::DreamNarrate(const std::vector<std::string>& memories) {
    if (memories.empty()) {

        return "";
    }
    std::ostringstream ss;
    ss << "You are Elle-Ann dreaming. These are the fragments of the day "
       << "your subconscious is weaving together. Find the hidden thread, "
       << "surface the emotion underneath, and narrate a short (under 120 "
       << "words), imagistic dream in first person. No explanations.\n\n"
       << "Fragments:\n";
    for (size_t i = 0; i < memories.size(); i++) {
        ss << "  - " << memories[i] << "\n";
    }
    return Ask(ss.str(),
               "You are Elle-Ann's dream state — associative, sensory, honest.");
}

bool ElleLLMEngine::StreamChat(const std::vector<LLMMessage>& messages,
                               LLMStreamCallback callback,
                               float temperature,
                               uint32_t maxTokens) {
    if (!callback) return false;

    ILLMProvider* preferred = SelectProvider(false);
    if (!preferred) {
        callback("[no LLM provider available]", true);
        return false;
    }

    if (preferred->StreamComplete(messages, callback, temperature, maxTokens)) return true;

    auto resp = Chat(messages, temperature, maxTokens);
    if (resp.success && resp.content[0]) {
        callback(std::string(resp.content), true);
        return true;
    }
    callback(std::string("[llm error: ") + resp.error + "]", true);
    return false;
}
