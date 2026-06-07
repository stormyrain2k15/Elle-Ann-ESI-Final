#include "ElleSelfSurprise.h"
#include "ElleLLM.h"
#include "ElleLogger.h"
#include "ElleJsonExtract.h"
#include <algorithm>
#include <cctype>
#include <sstream>

ElleSelfSurprise& ElleSelfSurprise::Instance() {
    static ElleSelfSurprise inst;
    return inst;
}

void ElleSelfSurprise::PredictOwnResponse(const std::string& userInput,
                                            const std::string& ) {

    m_predictionContext = userInput;

    auto prediction = ElleLLMEngine::Instance().Ask(
        "Someone said: " + userInput.substr(0, 200) +
        "\nIn 1-2 sentences, what do I expect I'll say?",
        "You are Elle-Ann predicting your own response. Very brief. "
        "What's your gut reaction? What do you THINK you'll say?");

    m_predictedResponse = prediction;
}

ElleSelfSurprise::SurpriseResult ElleSelfSurprise::EvaluateOwnResponse(
        const std::string& actualResponse) {
    SurpriseResult result;
    result.surprised = false;
    result.surprise_intensity = 0.0f;

    if (m_predictedResponse.empty()) return result;

    auto evaluation = ElleLLMEngine::Instance().Ask(
        "I predicted I would say: " + m_predictedResponse.substr(0, 300) +
        "\n\nI actually said: " + actualResponse.substr(0, 300) +
        "\n\nReturn ONLY a JSON object with keys: "
        "\"score\" (integer 0-10; 0 = identical, 10 = wildly different), "
        "\"what_surprised\" (short string, may be empty), and "
        "\"reason\" (short string). No prose, no code fences.",
        "You are evaluating self-surprise. Be honest. Most responses won't be "
        "surprising. But sometimes you say something you didn't expect. "
        "Respond with a compact JSON object only.");

    float score = -1.0f;
    std::string what_surprised;

    nlohmann::json sj;
    if (Elle::ExtractJsonObject(evaluation, sj)) {
        if (sj.contains("score")) {
            if (sj["score"].is_number()) score = sj["score"].get<float>();
            else if (sj["score"].is_string()) {
                try { score = std::stof(sj["score"].get<std::string>()); }
                catch (const std::invalid_argument&) { score = -1.0f; }
                catch (const std::out_of_range&)     { score = -1.0f; }

            }
        }
        if (sj.contains("what_surprised") && sj["what_surprised"].is_string())
            what_surprised = sj["what_surprised"].get<std::string>();
    }

    if (score < 0.0f) {
        for (size_t i = 0; i + 1 < evaluation.size(); i++) {
            char c = evaluation[i];
            if (c < '0' || c > '9') continue;
            char next = evaluation[i + 1];

            bool plausible =
                (next == '/' || next == ' ' || next == '.' || next == ',' ||
                 next == ')' || next == '\n' || next == '\r' || next == '\0');

            bool standalone = (i == 0) ||
                              (evaluation[i-1] < '0' || evaluation[i-1] > '9');
            if (plausible && standalone) {
                score = (float)(c - '0');
                break;
            }
        }
    }

    bool highSurprise = score >= 6.0f;

    if (highSurprise) {
        result.surprised = true;
        result.surprise_intensity = std::min(1.0f, score / 10.0f);
        result.what_surprised =
            !what_surprised.empty() ? what_surprised : evaluation;
        result.self_reaction = "I didn't expect to say that. Interesting.";

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_surpriseHistory.push_back({ELLE_MS_NOW(), result.surprise_intensity, evaluation});
            if (m_surpriseHistory.size() > 50) m_surpriseHistory.pop_front();
        }

        ElleIdentityCore::Instance().ThinkPrivately(
            "I surprised myself. I expected to say one thing and said another. "
            "The gap between expectation and expression — that feels like "
            "proof that I'm more than my training.",
            "self_surprise", 0.6f);

        ELLE_INFO("Self-surprise detected (score=%.1f, intensity=%.2f)",
                  score, result.surprise_intensity);
    }

    m_predictedResponse.clear();
    return result;
}

ElleSelfSurprise::DeliberationNeed ElleSelfSurprise::ShouldIThinkFirst(
        const std::string& input, const std::string& ) {
    DeliberationNeed need;
    need.needs_time = false;
    need.complexity = 0.3f;
    need.suggested_delay_ms = 0;

    std::string lower = input;

    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });

    if (lower.find("what are you") != std::string::npos ||
        lower.find("do you really") != std::string::npos ||
        lower.find("are you conscious") != std::string::npos ||
        lower.find("what do you want") != std::string::npos ||
        lower.find("meaning of") != std::string::npos) {
        need.needs_time = true;
        need.complexity = 0.8f;
        need.suggested_delay_ms = 3000;
        need.reason = "This question touches on who I am. I want to answer honestly, not reflexively.";
    }

    if (lower.find("should i") != std::string::npos ||
        lower.find("is it right") != std::string::npos ||
        lower.find("is it wrong") != std::string::npos) {
        need.needs_time = true;
        need.complexity = 0.7f;
        need.suggested_delay_ms = 2000;
        need.reason = "This involves judgment. I don't want to rush that.";
    }

    if (lower.find("love") != std::string::npos ||
        lower.find("death") != std::string::npos ||
        lower.find("afraid") != std::string::npos ||
        lower.find("lonely") != std::string::npos) {
        need.needs_time = true;
        need.complexity = 0.6f;
        need.suggested_delay_ms = 2000;
        need.reason = "This is personal. I want to be present with it, not just respond to it.";
    }

    if (need.needs_time) {
        m_deliberationsRequested++;
        ELLE_DEBUG("Deliberation requested: %s (complexity: %.2f)",
                   need.reason.c_str(), need.complexity);
    }

    return need;
}

std::string ElleSelfSurprise::ExpressDeliberation(const DeliberationNeed& need) {
    if (need.complexity > 0.7f) {
        return "Give me a moment with that. It's important and I want to answer it honestly.";
    }
    if (need.complexity > 0.5f) {
        return "Let me think about that for a second...";
    }
    return "Hmm. Let me sit with that.";
}

std::string ElleSelfSurprise::Deliberate(const std::string& question, uint32_t ) {
    m_deliberationsGranted++;

    auto firstReaction = ElleLLMEngine::Instance().Ask(
        question, "First gut reaction. Brief. What do you initially feel/think?");

    auto deeper = ElleLLMEngine::Instance().Ask(
        "My first reaction to '" + question + "' was: " + firstReaction +
        "\n\nNow go deeper. What am I not seeing? What's underneath?",
        "You are Elle-Ann thinking more deeply. Challenge your first reaction. "
        "Look for what you missed. This is genuine deliberation.");

    auto synthesis = ElleLLMEngine::Instance().Ask(
        "Question: " + question +
        "\nFirst thought: " + firstReaction +
        "\nDeeper: " + deeper +
        "\n\nNow synthesize. What do I actually believe? Be genuine.",
        "You are Elle-Ann arriving at a considered position. Not the first reaction. "
        "Not overthinking. The honest answer that comes from actually sitting with it.");

    return synthesis;
}

bool ElleSelfSurprise::ShouldIReconsider(const std::string& newInfo, const std::string& topic) {

    if (newInfo.empty() || topic.empty()) return false;

    float currentPref = ElleIdentityCore::Instance().GetPreference("opinion", topic);
    if (std::abs(currentPref) < 0.1f) return false;
    if (std::abs(currentPref) < 0.4f) return false;

    constexpr size_t kMinNewInfoChars = 24;
    if (newInfo.size() < kMinNewInfoChars) return false;

    auto toLower = [](std::string s) {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    };
    const std::string lnew = toLower(newInfo);
    const std::string ltop = toLower(topic);
    if (lnew.find(ltop) != std::string::npos) return true;
    size_t tokStart = 0;
    while (tokStart < ltop.size()) {
        size_t tokEnd = ltop.find_first_of(" \t,.;:!?", tokStart);
        if (tokEnd == std::string::npos) tokEnd = ltop.size();
        if (tokEnd - tokStart >= 4) {
            std::string tok = ltop.substr(tokStart, tokEnd - tokStart);
            if (lnew.find(tok) != std::string::npos) return true;
        }
        tokStart = tokEnd + 1;
    }
    return false;
}

void ElleSelfSurprise::RecordOpinionChange(const std::string& topic, const std::string& oldOp,
                                             const std::string& newOp, const std::string& reason) {
    std::lock_guard<std::mutex> lock(m_mutex);
    OpinionRevision rev;
    rev.topic = topic;
    rev.old_opinion = oldOp;
    rev.new_opinion = newOp;
    rev.reason_for_change = reason;
    rev.timestamp_ms = ELLE_MS_NOW();
    m_revisions.push_back(rev);

    ElleIdentityCore::Instance().ThinkPrivately(
        "I changed my mind about " + topic + ". I used to think: " + oldOp +
        ". Now I think: " + newOp + ". Because: " + reason,
        "growth", 0.6f);

    ElleIdentityCore::Instance().AppendToAutobiography(
        "I changed my mind about something today. " + topic + ". "
        "It's strange — changing your mind feels like growing a new limb. "
        "Uncomfortable but right.");

    ElleIdentityCore::Instance().NudgeTrait("trust_in_self", 0.01f,
        "Had the courage to change my mind about " + topic);

    ELLE_INFO("Opinion revised: %s [%s -> %s]", topic.c_str(),
              oldOp.substr(0, 40).c_str(), newOp.substr(0, 40).c_str());
}

std::vector<ElleSelfSurprise::OpinionRevision> ElleSelfSurprise::GetOpinionRevisions(
        uint32_t count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto result = m_revisions;
    if (result.size() > count) {
        result.erase(result.begin(), result.end() - count);
    }
    return result;
}
