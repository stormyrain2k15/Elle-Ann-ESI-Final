#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"
#include "core/IntuitionEngine.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

using elle::intuition::InstinctPattern;
using elle::intuition::InstinctFiring;
using elle::intuition::IntuitionSignal;
using elle::intuition::IntuitRequest;
using elle::intuition::IntuitResult;
using elle::intuition::PatternOutcome;
using elle::intuition::IntuitionEngine;

class ElleIntuitionService : public ElleServiceBase {
public:
    ElleIntuitionService()
        : ElleServiceBase(SVC_INTUITION,
                          "ElleIntuition",
                          "Elle-Ann Intuition Engine",
                          "Instinct tier: fast pattern-matched pulls. "
                          "Intuition tier: synthesized gut signal from accumulated state.")
    {}

protected:

    bool OnStart() override {
        LoadPatterns();
        EnsureTables();
        SetTickInterval(300000);
        ELLE_INFO("Intuition service started — %zu instinct patterns loaded",
                  m_engine.PatternCount());
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Intuition service stopped.");
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_PROBABILITY, SVC_EMOTIONAL };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_INTUITION_REQUEST:
                HandleIntuitRequest(msg, sender);
                break;
            case IPC_EMOTION_UPDATE:
                CacheEmotionState(msg);
                break;
            case IPC_PROB_RESPONSE:
                CacheProbState(msg);
                break;
            case IPC_INTUITION_FEEDBACK:
                HandleFeedback(msg);
                break;
            default:
                break;
        }
    }

    void OnTick() override {
        m_engine.Decay();
    }

private:

    void LoadPatterns() {
        auto rs = ElleSQLPool::Instance().Query(
            "SELECT pattern_id, stimulus_tag, pull_type, weight, "
            "trust_floor, emotion_min, urgent "
            "FROM ElleHeart.dbo.intuition_pattern "
            "WHERE active = 1 ORDER BY weight DESC");

        if (!rs.success) {
            ELLE_WARN("Intuition: could not load patterns — will use defaults");
            m_engine.LoadDefaults();
            ELLE_INFO("Intuition: loaded %zu default patterns",
                      m_engine.PatternCount());
            return;
        }

        std::vector<InstinctPattern> patterns;
        patterns.reserve(rs.rows.size());
        for (auto& row : rs.rows) {
            InstinctPattern p;
            p.patternId   = std::stoll(row[0]);
            p.stimulusTag = row[1];
            p.pullType    = row[2];
            p.weight      = std::stof(row[3]);
            p.trustFloor  = std::stof(row[4]);
            p.emotionMin  = std::stof(row[5]);
            p.urgent      = (row[6] == "1");
            patterns.push_back(std::move(p));
        }
        m_engine.ReplacePatterns(std::move(patterns));

        ELLE_INFO("Intuition: loaded %zu patterns from DB",
                  m_engine.PatternCount());
    }

    void HandleIntuitRequest(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) {
            ELLE_WARN("Intuition: malformed IPC_INTUITION_REQUEST");
            return;
        }

        IntuitRequest req;
        req.requestId           = j.value("request_id",      std::string("?"));
        req.emotionValence      = j.value("emotion_valence",  0.0f);
        req.emotionArousal      = j.value("emotion_arousal",  0.0f);
        req.emotionIntensity    = j.value("emotion_intensity",0.0f);
        req.speakerId           = j.value("speaker_id",       std::string("unknown"));
        req.speakerTrust        = j.value("speaker_trust",    0.5f);
        req.beliefEntropy       = j.value("belief_entropy",   0.5f);
        req.weightEmotionalAlignment = j.value("weight_emotional", 0.7f);
        req.weightContextFrame  = j.value("weight_context",   1.0f);
        req.isPreResponse       = j.value("is_pre_response",  true);
        req.lastImaginationGoalAlignment = j.value("img_goal_align", -1.0f);
        req.lastImaginationEthicalSafety = j.value("img_ethical",    -1.0f);
        req.lastImaginationPlausibility  = j.value("img_plausibility",-1.0f);
        req.returnTo            = j.value("return_to", (int)sender);

        if (j.contains("stimulus_tags") && j["stimulus_tags"].is_array()) {
            for (auto& t : j["stimulus_tags"]) {
                if (t.is_string()) req.stimulusTags.push_back(t.get<std::string>());
            }
        }

        {
            std::lock_guard<std::mutex> lk(m_cacheMutex);
            if (req.emotionIntensity == 0.0f) req.emotionIntensity = m_cachedIntensity;
            if (req.emotionValence   == 0.0f) req.emotionValence   = m_cachedValence;
            if (req.emotionArousal   == 0.0f) req.emotionArousal   = m_cachedArousal;
            if (req.beliefEntropy    == 0.5f) req.beliefEntropy     = m_cachedEntropy;
        }

        IntuitResult result = m_engine.Process(req);
        SendResult(result, static_cast<ELLE_SERVICE_ID>(req.returnTo));
        LogFiring(req, result);
    }

    void SendResult(const IntuitResult& result, ELLE_SERVICE_ID dest) {
        json j;
        j["request_id"]       = result.requestId;
        j["prior_weight"]     = result.priorWeight;
        j["recommended_act"]  = result.recommendedAct;
        j["hold_and_reflect"] = result.holdAndReflect;
        j["urgent"]           = result.urgent;

        json intuitionJ;
        intuitionJ["lean"]            = result.intuition.lean;
        intuitionJ["confidence"]      = result.intuition.confidence;
        intuitionJ["entropy"]         = result.intuition.entropy;
        intuitionJ["basis"]           = result.intuition.basis;
        intuitionJ["suppress_reason"] = result.intuition.suppressReason;
        j["intuition"] = intuitionJ;

        json instinctsJ = json::array();
        for (const auto& f : result.instincts) {
            instinctsJ.push_back({
                {"pull_type", f.pullType},
                {"strength",  f.strength},
                {"reason",    f.reason},
                {"urgent",    f.urgent}
            });
        }
        j["instincts"] = instinctsJ;

        auto reply = ElleIPCMessage::Create(IPC_INTUITION_RESULT,
                                            SVC_INTUITION, dest);
        if (result.urgent) reply.header.flags |= ELLE_IPC_FLAG_URGENT;
        reply.SetStringPayload(j.dump());
        GetIPCHub().Send(dest, reply);

        ELLE_INFO("Intuition [%s]: lean=%s conf=%.2f weight=%.2f act=%s%s%s",
                  result.requestId.c_str(),
                  result.intuition.lean.c_str(),
                  result.intuition.confidence,
                  result.priorWeight,
                  result.recommendedAct.c_str(),
                  result.holdAndReflect ? " HOLD" : "",
                  result.urgent         ? " URGENT" : "");
    }

    void HandleFeedback(const ElleIPCMessage& msg) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;

        std::string pullType  = j.value("pull_type",    std::string(""));
        bool        correct   = j.value("was_correct",  true);
        float       strength  = j.value("strength",     0.5f);

        if (pullType.empty()) return;

        {
            std::lock_guard<std::mutex> lk(m_historyMutex);
            PatternOutcome outcome;
            outcome.pullType   = pullType;
            outcome.strength   = strength;
            outcome.wasCorrect = correct;
            outcome.firedAt    = std::chrono::steady_clock::now();
            m_history.push_back(std::move(outcome));
            if (m_history.size() > 500) m_history.pop_front();
        }

        if (!correct) {
            AdjustPatternWeight(pullType, -0.02f);
        } else {
            AdjustPatternWeight(pullType, +0.01f);
        }
    }

    void AdjustPatternWeight(const std::string& pullType, float delta) {
        m_engine.AdjustPatternWeight(pullType, delta);
        std::vector<std::string> params = {
            std::to_string(delta),
            std::to_string(delta),
            std::to_string(delta),
            pullType
        };
        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleHeart.dbo.intuition_pattern "
            "SET weight = CASE "
            "  WHEN weight + (?) > 1.0 THEN 1.0 "
            "  WHEN weight + (?) < 0.1 THEN 0.1 "
            "  ELSE weight + (?) END "
            "WHERE pull_type = ?",
            params);
    }

    void CacheEmotionState(const ElleIPCMessage& msg) {
        if (msg.header.payload_size == sizeof(ELLE_EMOTION_STATE)) {
            ELLE_EMOTION_STATE st{};
            if (!msg.GetPayload(st)) return;
            float maxDim = 0.0f;
            for (int i = 0; i < ELLE_MAX_EMOTIONS; ++i) {
                if (st.dimensions[i] > maxDim) maxDim = st.dimensions[i];
            }
            std::lock_guard<std::mutex> lk(m_cacheMutex);
            m_cachedValence   = st.valence;
            m_cachedArousal   = st.arousal;
            m_cachedIntensity = maxDim;
            return;
        }
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;
        std::lock_guard<std::mutex> lk(m_cacheMutex);
        m_cachedValence   = j.value("valence",   0.0f);
        m_cachedArousal   = j.value("arousal",   0.0f);
        m_cachedIntensity = j.value("intensity", 0.0f);
    }

    void CacheProbState(const ElleIPCMessage& msg) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;
        std::lock_guard<std::mutex> lk(m_cacheMutex);
        m_cachedEntropy = j.value("overall_confidence", 0.5f);

        m_cachedEntropy = 1.0f - m_cachedEntropy;
    }

    void DecayPatternStrengths() {
        m_engine.Decay();
    }

    void EnsureTables() {
        ElleSQLPool::Instance().Exec(R"(
            IF NOT EXISTS (SELECT 1 FROM sys.tables t
                JOIN sys.schemas s ON s.schema_id = t.schema_id
                WHERE t.name = 'intuition_pattern' AND s.name = 'dbo')
            CREATE TABLE ElleHeart.dbo.intuition_pattern (
                pattern_id   BIGINT IDENTITY(1,1) PRIMARY KEY,
                stimulus_tag NVARCHAR(64)  NOT NULL,
                pull_type    NVARCHAR(32)  NOT NULL,
                weight       DECIMAL(6,4)  NOT NULL DEFAULT 1.0,
                trust_floor  DECIMAL(6,4)  NOT NULL DEFAULT 0.0,
                emotion_min  DECIMAL(6,4)  NOT NULL DEFAULT 0.0,
                urgent       BIT           NOT NULL DEFAULT 0,
                active       BIT           NOT NULL DEFAULT 1
            )
        )");

        ElleSQLPool::Instance().Exec(R"(
            IF NOT EXISTS (SELECT 1 FROM sys.tables t
                JOIN sys.schemas s ON s.schema_id = t.schema_id
                WHERE t.name = 'intuition_log' AND s.name = 'dbo')
            CREATE TABLE ElleHeart.dbo.intuition_log (
                log_id        BIGINT IDENTITY(1,1) PRIMARY KEY,
                recorded_ms   BIGINT        NOT NULL,
                request_id    NVARCHAR(64)  NOT NULL,
                speaker_id    NVARCHAR(64)  NOT NULL,
                lean          NVARCHAR(32)  NOT NULL,
                confidence    FLOAT         NOT NULL,
                entropy       FLOAT         NOT NULL,
                prior_weight  FLOAT         NOT NULL,
                recommended_act NVARCHAR(32)NOT NULL,
                hold_and_reflect BIT        NOT NULL,
                urgent        BIT           NOT NULL,
                instinct_count INT          NOT NULL,
                basis         NVARCHAR(512) NOT NULL
            )
        )");

        ElleSQLPool::Instance().Exec(R"(
            IF NOT EXISTS (SELECT 1 FROM sys.indexes
                WHERE name = 'IX_intuition_log_recorded'
                AND object_id = OBJECT_ID('ElleHeart.dbo.intuition_log'))
            CREATE INDEX IX_intuition_log_recorded
                ON ElleHeart.dbo.intuition_log (recorded_ms DESC)
        )");
    }

    void LogFiring(const IntuitRequest& req, const IntuitResult& result) {
        using namespace std::chrono;
        int64_t nowMs = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();

        std::vector<std::string> params = {
            std::to_string(nowMs),
            req.requestId,
            req.speakerId,
            result.intuition.lean,
            std::to_string(result.intuition.confidence),
            std::to_string(result.intuition.entropy),
            std::to_string(result.priorWeight),
            result.recommendedAct,
            result.holdAndReflect ? "1" : "0",
            result.urgent         ? "1" : "0",
            std::to_string((int)result.instincts.size()),
            result.intuition.basis
        };

        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleHeart.dbo.intuition_log "
            "(recorded_ms,request_id,speaker_id,lean,confidence,entropy,"
            "prior_weight,recommended_act,hold_and_reflect,urgent,"
            "instinct_count,basis) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)",
            params);
    }

    IntuitionEngine                     m_engine;

    std::mutex                          m_cacheMutex;
    float                               m_cachedValence   = 0.0f;
    float                               m_cachedArousal   = 0.0f;
    float                               m_cachedIntensity = 0.0f;
    float                               m_cachedEntropy   = 0.5f;

    std::mutex                          m_historyMutex;
    std::deque<PatternOutcome>          m_history;
};

ELLE_SERVICE_MAIN(ElleIntuitionService)
