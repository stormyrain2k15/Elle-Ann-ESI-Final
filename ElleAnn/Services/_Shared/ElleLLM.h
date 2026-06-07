#pragma once
#ifndef ELLE_LLM_H
#define ELLE_LLM_H

#include "ElleTypes.h"
#include "ElleQueueIPC.h"

#include <string>
#include <vector>
#include <functional>

struct LLMMessage {
    std::string role;
    std::string content;
};

typedef std::function<void(const std::string& token, bool done)> LLMStreamCallback;

class ElleLLMEngine {
public:
    static ElleLLMEngine& Instance();

    void BindHub(ElleIPCHub* hub, ELLE_SERVICE_ID requester);

    ELLE_LLM_RESPONSE Chat(const std::vector<LLMMessage>& messages,
                            float temperature = -1.0f,
                            uint32_t maxTokens = 0);

    bool StreamChat(const std::vector<LLMMessage>& messages,
                    LLMStreamCallback callback,
                    float temperature = -1.0f,
                    uint32_t maxTokens = 0);

    std::string Ask(const std::string& prompt,
                    const std::string& systemPrompt = "");

    ELLE_LLM_RESPONSE ElleChat(const std::string& userMessage,
                                const std::vector<LLMMessage>& history,
                                const ELLE_EMOTION_STATE& emotions,
                                const std::string& memoryContext = "",
                                const std::string& goalContext = "");

    std::string SelfReflect(const std::string& context,
                            const ELLE_EMOTION_STATE& emotions);

    std::string FormGoal(const std::string& driveContext,
                         const std::string& emotionContext);

    std::string DreamNarrate(const std::vector<std::string>& memories);

    bool IsInitialized() const { return m_bound; }
    uint32_t EstimateTokens(const std::string& text) const;

private:
    ElleLLMEngine() = default;
    ~ElleLLMEngine() = default;

    ElleIPCHub*     m_hub       = nullptr;
    ELLE_SERVICE_ID m_requester = SVC_COGNITIVE;
    bool            m_bound     = false;
};

#endif
