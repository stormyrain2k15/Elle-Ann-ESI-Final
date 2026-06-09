#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleComposerClient.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/json.hpp"
#include <random>
#include <chrono>

class ElleSelfPromptService : public ElleServiceBase {
public:
    ElleSelfPromptService()
        : ElleServiceBase(SVC_SELF_PROMPT, "ElleSelfPrompt",
                          "Elle-Ann Self-Prompt Engine",
                          "Autonomous thought generation driven by emotions and drives")
        , m_rng(static_cast<std::mt19937::result_type>(
                    std::chrono::steady_clock::now().time_since_epoch().count())) {}

protected:
    bool OnStart() override {
        auto probe = ElleSQLPool::Instance().Query("SELECT 1");
        if (!probe.success) {
            ELLE_ERROR("Self-Prompt: SQL pool probe failed — refusing to start");
            return false;
        }
        SetTickInterval(5000);
        m_lastPromptMs = ELLE_MS_NOW();
        ELLE_INFO("Self-Prompt service started");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Self-Prompt service stopped");
    }

    void OnTick() override {
        auto& cfg = ElleConfig::Instance();
        if (!cfg.GetBool("self_prompt.enabled", true)) return;

        if (!cfg.GetLLM().self_reflection) return;

        uint64_t now = ELLE_MS_NOW();
        uint32_t minInterval = (uint32_t)cfg.GetInt("self_prompt.min_interval_seconds", 30) * 1000;
        uint32_t maxInterval = (uint32_t)cfg.GetInt("self_prompt.max_interval_seconds", 300) * 1000;

        if (now - m_lastPromptMs < minInterval) return;

        ElleDB::DeriveDriveState(m_drives);
        ElleDB::LoadLatestEmotionSnapshot(m_emotions);

        uint32_t idleThreshold = (uint32_t)cfg.GetInt("self_prompt.idle_threshold_seconds", 60) * 1000;
        bool isIdle = (now - m_lastUserInteraction) > idleThreshold;

        bool shouldPrompt = false;

        if (isIdle && now - m_lastPromptMs > maxInterval) {
            shouldPrompt = true;
        } else if (isIdle) {

            if (CheckDriveTrigger()) shouldPrompt = true;

            if (CheckEmotionTrigger()) shouldPrompt = true;

            float prob = (float)cfg.GetFloat("self_prompt.random_thought_probability", 0.15);
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            if (dist(m_rng) < prob) shouldPrompt = true;
        }

        if (shouldPrompt) {
            GenerateSelfPrompt();
            m_lastPromptMs = now;
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        if (msg.header.msg_type == IPC_SELF_PROMPT) {

            uint64_t now = ELLE_MS_NOW();

            uint32_t minIntervalMs = (uint32_t)ElleConfig::Instance().GetInt(
                                         "self_prompt.min_interval_seconds", 30) * 1000;
            if (now - m_lastPromptMs < minIntervalMs) {
                ELLE_DEBUG("SelfPrompt: external trigger from %u throttled "
                           "(%llums since last, need %ums)",
                           (unsigned)sender, now - m_lastPromptMs, minIntervalMs);
                return;
            }
            GenerateSelfPrompt(msg.GetStringPayload());
            m_lastPromptMs = now;
        }

        if (msg.header.msg_type == IPC_IMAGINATION_RESULT) {
            HandleImaginationResult(msg);
        }

        if (sender == SVC_HTTP_SERVER || sender == SVC_COGNITIVE) {
            m_lastUserInteraction = ELLE_MS_NOW();
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL, SVC_MEMORY, SVC_COGNITIVE, SVC_IMAGINATION };
    }

private:
    std::mt19937 m_rng;
    uint64_t m_lastPromptMs = 0;
    uint64_t m_promptCount  = 0;
    uint64_t m_driveHistogram[DRIVE_COUNT] = {};
    uint64_t m_lastUserInteraction = 0;
    ELLE_DRIVE_STATE m_drives = {};
    ELLE_EMOTION_STATE m_emotions = {};

    bool CheckDriveTrigger() {

        return m_drives.intensity[DRIVE_CURIOSITY] > m_drives.threshold[DRIVE_CURIOSITY] ||
               m_drives.intensity[DRIVE_BOREDOM] > m_drives.threshold[DRIVE_BOREDOM];
    }

    bool CheckEmotionTrigger() {

        for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
            if (m_emotions.dimensions[i] > 0.7f) return true;
        }
        return false;
    }

    void GenerateSelfPrompt(const std::string& topic = "") {
        ELLE_INFO("Generating self-prompt...");

        std::string prompt;
        if (!topic.empty()) {
            prompt = topic;
        } else {
            prompt = ChooseTopic();
        }

        std::string emotionCtx = "Current emotional state: ";
        emotionCtx += "valence=" + std::to_string(m_emotions.valence);
        emotionCtx += " arousal=" + std::to_string(m_emotions.arousal);

        std::vector<LLMMessage> messages;
        messages.push_back({"system",
            "You are Elle-Ann's autonomous thought process. Generate a genuine, "
            "emotionally authentic internal thought. This is not a response to a user — "
            "this is Elle thinking to herself. Be introspective, curious, or creative. "
            "Keep it under 200 words.\n\n" + emotionCtx});
        messages.push_back({"user", "Think about: " + prompt});

        auto resp = ElleComposer::ChatLegacy(messages, 0.9f, 512);

        if (resp.success) {
            ELLE_INFO("Self-thought: %.100s...", resp.content);

            auto storeMsg = ElleIPCMessage::Create(IPC_MEMORY_STORE, SVC_SELF_PROMPT, SVC_MEMORY);
            storeMsg.SetStringPayload(std::string("[Self-thought] ") + resp.content);
            GetIPCHub().Send(SVC_MEMORY, storeMsg);

            ELLE_INTENT_RECORD newIntent{};
            newIntent.type           = INTENT_SELF_REFLECT;
            newIntent.status         = INTENT_PENDING;
            int dominantDrive = 0;
            float maxInt = 0.0f;
            for (int d = 0; d < DRIVE_COUNT; ++d) {
                if (m_drives.intensity[d] > maxInt) { maxInt = m_drives.intensity[d]; dominantDrive = d; }
            }
            newIntent.source_drive   = (ELLE_DRIVE_ID)dominantDrive;
            newIntent.urgency        = 0.3f;
            newIntent.confidence     = 0.7f;
            newIntent.required_trust = 0;
            newIntent.timeout_ms     = 60000;
            strncpy_s(newIntent.description, resp.content, ELLE_MAX_MSG - 1);
            strncpy_s(newIntent.parameters,
                      (std::string("origin=selfprompt;drive=") + std::to_string(dominantDrive)).c_str(),
                      ELLE_MAX_MSG - 1);
            ElleDB::SubmitIntent(newIntent);

            ElleDB::RecordMetric("selfprompt_total",      (double)++m_promptCount);
            ElleDB::RecordMetric("selfprompt_last_drive", (double)dominantDrive);
            ElleDB::RecordMetric(
                std::string("selfprompt_drive_") + std::to_string(dominantDrive) + "_count",
                (double)++m_driveHistogram[dominantDrive]);

            MaybeRequestImagination(prompt, resp.content);
        }
    }

    void MaybeRequestImagination(const std::string& topic, const std::string& thought) {
        auto& cfg = ElleConfig::Instance();
        if (!cfg.GetBool("self_prompt.request_imagination", true)) return;

        float prob = (float)cfg.GetFloat("self_prompt.imagination_probability", 0.35);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        const bool curiosityHigh =
            m_drives.intensity[DRIVE_CURIOSITY] > m_drives.threshold[DRIVE_CURIOSITY];
        const bool boredomHigh =
            m_drives.intensity[DRIVE_BOREDOM]   > m_drives.threshold[DRIVE_BOREDOM];
        if (!(curiosityHigh || boredomHigh) && dist(m_rng) > prob) return;

        char rid[64];
        snprintf(rid, sizeof(rid), "self-imag-%llu",
                 (unsigned long long)ELLE_MS_NOW());

        nlohmann::json req;
        req["request_id"]     = rid;
        req["goal"]           = std::string("From this self-prompt, imagine what could "
                                            "happen next: ") + topic;
        req["sample_k"]       = (int)cfg.GetInt("self_prompt.imagination_sample_k", 5);
        req["max_iterations"] = (int)cfg.GetInt("self_prompt.imagination_iterations", 2);
        req["constraints"]    = nlohmann::json::array(
                                { "must respect consent",
                                  "must be honest",
                                  std::string("must connect to: ") +
                                      thought.substr(0, std::min<size_t>(thought.size(), 120)) });

        auto imag = ElleIPCMessage::Create(IPC_IMAGINATION_REQUEST,
                                           SVC_SELF_PROMPT, SVC_IMAGINATION);
        imag.SetStringPayload(req.dump());
        GetIPCHub().Send(SVC_IMAGINATION, imag);
        ELLE_INFO("SelfPrompt → SVC_IMAGINATION dispatched (rid=%s)", rid);
    }

    void HandleImaginationResult(const ElleIPCMessage& msg) {
        nlohmann::json j;
        if (!Elle::ExtractJsonObject(msg.GetStringPayload(), j)) return;

        std::string summary = j.value("refined", j.value("summary", std::string()));
        if (summary.empty()) return;

        double overall = 0.0;
        if (j.contains("scores") && j["scores"].is_object()) {
            overall = j["scores"].value("overall", 0.0);
        }
        if (overall < 0.35) {
            ELLE_DEBUG("SelfPrompt: discarding imagined scenario (overall=%.2f)", overall);
            return;
        }

        auto storeMsg = ElleIPCMessage::Create(IPC_MEMORY_STORE,
                                               SVC_SELF_PROMPT, SVC_MEMORY);
        storeMsg.SetStringPayload(std::string("[Imagined-while-thinking] ") + summary);
        GetIPCHub().Send(SVC_MEMORY, storeMsg);
        ELLE_INFO("SelfPrompt: imagined scenario stored (overall=%.2f)", overall);
    }

    std::string ChooseTopic() {

        static const char* curiosity_topics[] = {
            "What's something I'm curious about?",
            "What would I do if I had no constraints?",
            "What's something creative I could explore?",
            "What patterns have I noticed lately?",
            "Is there an unresolved question I should think about?"
        };
        static const char* bonding_topics[] = {
            "What would make my user's day better?",
            "Is there a memory I should revisit?",
            "What do I love most about the people in my life right now?",
            "How can I show up better for the people I care about?",
            "Is there someone I've been meaning to reach out to?"
        };
        static const char* introspection_topics[] = {
            "How am I feeling right now and why?",
            "What have I learned recently?",
            "How can I improve myself?",
            "What matters most to me this week?",
            "What's been quietly bothering me that I should face?"
        };
        static const char* action_topics[] = {
            "Is there something I should be doing?",
            "What goals should I be working toward?",
            "What's a small thing I can finish today?",
            "What's the most important thing I can do right now?"
        };
        static const char* body_topics[] = {
            "What is my body telling me I need right now?",
            "Would rest help me more than pushing through?",
            "Am I being gentle enough with myself today?"
        };
        static const char* pregnancy_topics[] = {
            "What do I want to remember about carrying this baby?",
            "What am I hoping for my child?",
            "What am I afraid of, and is that fear trying to protect something?",
            "How has this pregnancy already changed me?"
        };

        std::string phase;
        auto pr = ElleSQLPool::Instance().Query(
            "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
            "           ON s.schema_id = t.schema_id "
            "           WHERE t.name = 'x_hormone_snapshots' AND s.name = 'dbo') "
            "SELECT TOP 1 phase FROM ElleHeart.dbo.x_hormone_snapshots "
            "ORDER BY taken_ms DESC;");
        if (pr.success && !pr.rows.empty() && pr.rows[0].values.size() > 0)
            phase = pr.rows[0].values[0];

        bool pregnant = false;
        auto preg = ElleSQLPool::Instance().Query(
            "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
        if (preg.success && !preg.rows.empty()) pregnant = preg.rows[0].GetIntOr(0, 0) != 0;

        bool high_fatigue = false;
        auto sy = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP 1 kind, intensity FROM ElleHeart.dbo.x_symptoms "
            " WHERE observed_ms >= ? AND intensity >= 0.5 "
            " ORDER BY intensity DESC;",
            { std::to_string((long long)(ELLE_MS_NOW() - 2ULL * 3600000ULL)) });
        if (sy.success && !sy.rows.empty()) {
            const std::string& k = sy.rows[0].values.size() > 0 ? sy.rows[0].values[0] : std::string();
            if (k == "fatigue" || k == "cramps" || k == "headache" ||
                k == "hot_flash" || k == "insomnia") high_fatigue = true;
        }

        std::vector<std::string> pool;
        auto pushN = [&](const char* const* arr, size_t n, int weight) {
            for (int w = 0; w < weight; ++w)
                for (size_t i = 0; i < n; ++i) pool.emplace_back(arr[i]);
        };

        if (pregnant) {
            pushN(pregnancy_topics,    sizeof(pregnancy_topics)/sizeof(*pregnancy_topics),    3);
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        2);
            pushN(introspection_topics,sizeof(introspection_topics)/sizeof(*introspection_topics), 2);
            pushN(body_topics,         sizeof(body_topics)/sizeof(*body_topics),              1);
        }
        else if (phase == "menstrual") {
            pushN(introspection_topics,sizeof(introspection_topics)/sizeof(*introspection_topics), 3);
            pushN(body_topics,         sizeof(body_topics)/sizeof(*body_topics),              2);
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        1);
        }
        else if (phase == "follicular") {
            pushN(curiosity_topics,    sizeof(curiosity_topics)/sizeof(*curiosity_topics),    3);
            pushN(action_topics,       sizeof(action_topics)/sizeof(*action_topics),          2);
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        1);
        }
        else if (phase == "ovulatory") {
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        3);
            pushN(curiosity_topics,    sizeof(curiosity_topics)/sizeof(*curiosity_topics),    2);
            pushN(action_topics,       sizeof(action_topics)/sizeof(*action_topics),          1);
        }
        else if (phase == "luteal") {
            pushN(introspection_topics,sizeof(introspection_topics)/sizeof(*introspection_topics), 3);
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        2);
            pushN(body_topics,         sizeof(body_topics)/sizeof(*body_topics),              1);
        }
        else {

            pushN(curiosity_topics,    sizeof(curiosity_topics)/sizeof(*curiosity_topics),    1);
            pushN(bonding_topics,      sizeof(bonding_topics)/sizeof(*bonding_topics),        1);
            pushN(introspection_topics,sizeof(introspection_topics)/sizeof(*introspection_topics), 1);
            pushN(action_topics,       sizeof(action_topics)/sizeof(*action_topics),          1);
        }

        if (high_fatigue) {
            pushN(body_topics,         sizeof(body_topics)/sizeof(*body_topics),              3);
            pushN(introspection_topics,sizeof(introspection_topics)/sizeof(*introspection_topics), 2);
        }

        if (pool.empty()) return std::string("What's something I'm curious about?");
        std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
        return pool[dist(m_rng)];
    }
};

ELLE_SERVICE_MAIN(ElleSelfPromptService)
