#include "ElleLLM.h"
#include "ElleComposerClient.h"
#include "ElleLogger.h"
#include "json.hpp"

using nlohmann::json;

namespace {

json EmotionToJson(const ELLE_EMOTION_STATE& e) {
    json j;
    j["valence"] = e.valence;
    j["arousal"] = e.arousal;
    json dims = json::object();
    for (int i = 0; i < ELLE_EMOTION_COUNT; ++i) {
        dims[std::to_string(i)] = e.dimensions[i];
    }
    j["dimensions"] = dims;
    return j;
}

json HistoryToJson(const std::vector<LLMMessage>& history) {
    json out = json::array();
    for (const auto& m : history) {
        out.push_back({{"role", m.role}, {"text", m.content}});
    }
    return out;
}

}

ElleLLMEngine& ElleLLMEngine::Instance() {
    static ElleLLMEngine inst;
    return inst;
}

void ElleLLMEngine::BindHub(ElleIPCHub* hub, ELLE_SERVICE_ID requester) {
    m_hub       = hub;
    m_requester = requester;
    m_bound     = (hub != nullptr);
}

uint32_t ElleLLMEngine::EstimateTokens(const std::string& text) const {
    return (uint32_t)(text.size() / 4 + 1);
}

ELLE_LLM_RESPONSE ElleLLMEngine::Chat(const std::vector<LLMMessage>& messages,
                                       float temperature,
                                       uint32_t maxTokens)
{
    ELLE_LLM_RESPONSE resp{};
    if (!m_bound || !m_hub) {
        resp.success = false;
        strncpy_s(resp.content, "composer_unbound", ELLE_MAX_MSG - 1);
        return resp;
    }
    json payload;
    payload["history"]     = HistoryToJson(messages);
    payload["temperature"] = temperature;
    payload["max_words"]   = (uint32_t)(maxTokens ? maxTokens / 3 : 200);

    auto r = ElleComposer::Client::Instance().Request(
        *m_hub, m_requester, "CONVERSE", payload, 5000);

    resp.success = r.success;
    strncpy_s(resp.content,
              r.text.empty() ? r.error.c_str() : r.text.c_str(),
              ELLE_MAX_MSG - 1);
    strncpy_s(resp.model_used, "composer", ELLE_MAX_MSG - 1);
    resp.latency_ms = 0;
    resp.tokens_used = (uint32_t)(r.text.size() / 4);
    return resp;
}

bool ElleLLMEngine::StreamChat(const std::vector<LLMMessage>& messages,
                                LLMStreamCallback callback,
                                float temperature,
                                uint32_t maxTokens)
{
    auto resp = Chat(messages, temperature, maxTokens);
    if (callback) {
        callback(resp.content, false);
        callback(std::string(), true);
    }
    return resp.success;
}

std::string ElleLLMEngine::Ask(const std::string& prompt,
                                const std::string& systemPrompt)
{
    if (!m_bound || !m_hub) return std::string();
    json payload;
    payload["prompt"] = prompt;
    if (!systemPrompt.empty()) payload["system"] = systemPrompt;

    return ElleComposer::ComposeText(*m_hub, m_requester, "ASK_INNER",
                                      payload, 5000, std::string());
}

ELLE_LLM_RESPONSE ElleLLMEngine::ElleChat(const std::string& userMessage,
                                          const std::vector<LLMMessage>& history,
                                          const ELLE_EMOTION_STATE& emotions,
                                          const std::string& memoryContext,
                                          const std::string& goalContext)
{
    ELLE_LLM_RESPONSE resp{};
    if (!m_bound || !m_hub) {
        resp.success = false;
        strncpy_s(resp.content, "composer_unbound", ELLE_MAX_MSG - 1);
        return resp;
    }
    json payload;
    payload["user_message"] = userMessage;
    payload["history"]      = HistoryToJson(history);
    payload["emotion"]      = EmotionToJson(emotions);
    if (!memoryContext.empty()) payload["memory_ctx"] = memoryContext;
    if (!goalContext.empty())   payload["goal"]       = goalContext;

    auto r = ElleComposer::Client::Instance().Request(
        *m_hub, m_requester, "CONVERSE", payload, 5000);

    resp.success = r.success;
    strncpy_s(resp.content,
              r.text.empty() ? r.error.c_str() : r.text.c_str(),
              ELLE_MAX_MSG - 1);
    strncpy_s(resp.model_used, "composer", ELLE_MAX_MSG - 1);
    return resp;
}

std::string ElleLLMEngine::SelfReflect(const std::string& context,
                                        const ELLE_EMOTION_STATE& emotions)
{
    if (!m_bound || !m_hub) return std::string();
    json payload;
    payload["context"] = context;
    payload["emotion"] = EmotionToJson(emotions);
    return ElleComposer::ComposeText(*m_hub, m_requester, "SELF_REFLECT",
                                      payload, 5000, std::string());
}

std::string ElleLLMEngine::FormGoal(const std::string& driveContext,
                                     const std::string& emotionContext)
{
    if (!m_bound || !m_hub) return std::string();
    json payload;
    payload["drives"]  = driveContext;
    payload["emotion"] = emotionContext;
    return ElleComposer::ComposeText(*m_hub, m_requester, "FORM_GOAL",
                                      payload, 5000, std::string());
}

std::string ElleLLMEngine::DreamNarrate(const std::vector<std::string>& memories)
{
    (void)memories;
    return std::string();
}
