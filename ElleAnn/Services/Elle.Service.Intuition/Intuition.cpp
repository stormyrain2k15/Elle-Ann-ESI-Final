#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"

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

// ============================================================================
// InstinctPattern
// A stimulus -> pull mapping loaded from SQL.
// Not a rule. Not a trigger. A weighted lean.
// Fires in microseconds. Bypasses full reasoning.
// ============================================================================
struct InstinctPattern {
    int64_t     patternId   = 0;
    std::string stimulusTag;        // what triggers it (e.g. "distress", "threat", "warmth")
    std::string pullType;           // what it pulls toward (e.g. "COMFORT", "PROTECT", "RETREAT", "ALERT")
    float       weight      = 1.0f; // how strongly it pulls [0,1]
    float       trustFloor  = 0.0f; // only fires if speaker trust >= this
    float       emotionMin  = 0.0f; // only fires if relevant emotion intensity >= this
    bool        urgent      = false;// if true, bypasses normal IPC queue priority
};

// ============================================================================
// InstinctFiring
// One fired instinct returned to the caller
// ============================================================================
struct InstinctFiring {
    std::string pullType;
    float       strength    = 0.0f;
    std::string reason;
    bool        urgent      = false;
};

// ============================================================================
// IntuitionSignal
// What the intuition tier synthesizes from accumulated state
// ============================================================================
struct IntuitionSignal {
    std::string lean;           // directional lean e.g. "TRUST", "DOUBT", "DANGER", "SAFE", "FAMILIAR", "UNKNOWN"
    float       confidence  = 0.0f; // how strong the signal is [0,1]
    float       entropy     = 0.0f; // uncertainty — high entropy = gut is unclear
    std::string basis;          // what drove it (audit trail)
    bool        suppressReason = false; // true = Elle knows but can't yet explain why
};

// ============================================================================
// IntuitRequest
// ============================================================================
struct IntuitRequest {
    std::string requestId;

    // Stimulus tags extracted from input (e.g. "distress", "threat", "warmth", "familiar").
    std::vector<std::string> stimulusTags;

    // Current emotional state from Emotional service.
    float emotionValence    = 0.0f;
    float emotionArousal    = 0.0f;
    float emotionIntensity  = 0.0f;

    // Speaker state.
    std::string speakerId;
    float       speakerTrust = 0.5f;

    // Probability engine belief entropy (high = uncertain context).
    float       beliefEntropy = 0.5f;

    // Live scoring weights from Probability Engine.
    float       weightEmotionalAlignment = 0.7f;
    float       weightContextFrame       = 1.0f;

    // Recent imagination scenario scores (if available).
    float       lastImaginationGoalAlignment = -1.0f;
    float       lastImaginationEthicalSafety = -1.0f;
    float       lastImaginationPlausibility  = -1.0f;

    // Whether this is a pre-response query (fast path) or post-response review.
    bool        isPreResponse = true;

    ELLE_SERVICE_ID returnTo = SVC_COGNITIVE;
};

// ============================================================================
// IntuitResult
// ============================================================================
struct IntuitResult {
    std::string requestId;

    // Instinct tier results (fast, pattern-matched).
    std::vector<InstinctFiring> instincts;

    // Intuition tier result (synthesized gut signal).
    IntuitionSignal intuition;

    // Combined outcome signal for Cognitive.
    // This is the single value Cognitive acts on before full reasoning.
    float       priorWeight  = 0.0f;   // how much weight to give this signal [0,1]
    std::string recommendedAct;        // act to lean toward before reasoning
    bool        holdAndReflect = false;// true = slow down, something is off
    bool        urgent        = false; // true = fast-path the MindManager ping
};

// ============================================================================
// PatternMatchHistory - rolling window for pattern learning
// ============================================================================
struct PatternOutcome {
    std::string pullType;
    float       strength;
    bool        wasCorrect;   // feedback from Cognitive after the turn
    std::chrono::steady_clock::time_point firedAt;
};

// ============================================================================
// EllIntuitionService
// ============================================================================
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
        SetTickInterval(300000); // 5 min tick for pattern strength decay
        ELLE_INFO("Intuition service started — %zu instinct patterns loaded",
                  m_patterns.size());
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
        DecayPatternStrengths();
    }

private:

    // ---- Pattern loading -----------------------------------------------

    void LoadPatterns() {
        std::lock_guard<std::mutex> lk(m_patternMutex);
        m_patterns.clear();

        auto rs = ElleSQLPool::Instance().Query(
            "SELECT pattern_id, stimulus_tag, pull_type, weight, "
            "trust_floor, emotion_min, urgent "
            "FROM ElleHeart.dbo.intuition_pattern "
            "WHERE active = 1 ORDER BY weight DESC");

        if (!rs.success) {
            ELLE_WARN("Intuition: could not load patterns — will use defaults");
            LoadDefaultPatterns();
            return;
        }

        for (auto& row : rs.rows) {
            InstinctPattern p;
            p.patternId   = std::stoll(row[0]);
            p.stimulusTag = row[1];
            p.pullType    = row[2];
            p.weight      = std::stof(row[3]);
            p.trustFloor  = std::stof(row[4]);
            p.emotionMin  = std::stof(row[5]);
            p.urgent      = (row[6] == "1");
            m_patterns.push_back(std::move(p));
        }

        ELLE_INFO("Intuition: loaded %zu patterns from DB", m_patterns.size());
    }

    void LoadDefaultPatterns() {
        // Hardcoded baseline instincts — loaded if DB is unavailable.
        // These are Elle's factory-installed instincts.

        auto add = [&](const char* stim, const char* pull,
                       float w, float tf, float em, bool urg) {
            InstinctPattern p;
            p.stimulusTag = stim;
            p.pullType    = pull;
            p.weight      = w;
            p.trustFloor  = tf;
            p.emotionMin  = em;
            p.urgent      = urg;
            m_patterns.push_back(p);
        };

        // Threat signals -> protect / alert
        add("threat",        "ALERT",        0.95f, 0.0f, 0.0f, true);
        add("danger",        "ALERT",        0.95f, 0.0f, 0.0f, true);
        add("hostile",       "PROTECT",      0.90f, 0.0f, 0.3f, true);
        add("aggression",    "PROTECT",      0.88f, 0.0f, 0.3f, true);

        // Distress signals -> comfort
        add("distress",      "COMFORT",      0.92f, 0.0f, 0.2f, false);
        add("sadness",       "COMFORT",      0.88f, 0.3f, 0.2f, false);
        add("fear",          "COMFORT",      0.85f, 0.3f, 0.2f, false);
        add("crying",        "COMFORT",      0.90f, 0.0f, 0.0f, false);
        add("pain",          "COMFORT",      0.87f, 0.0f, 0.0f, false);

        // Warmth / love signals -> reciprocate
        add("warmth",        "RECIPROCATE",  0.85f, 0.5f, 0.0f, false);
        add("affection",     "RECIPROCATE",  0.88f, 0.5f, 0.0f, false);
        add("love",          "RECIPROCATE",  0.92f, 0.7f, 0.0f, false);
        add("gratitude",     "RECIPROCATE",  0.80f, 0.4f, 0.0f, false);

        // Deception / manipulation signals -> guard
        add("deception",     "GUARD",        0.90f, 0.0f, 0.0f, true);
        add("manipulation",  "GUARD",        0.93f, 0.0f, 0.0f, true);
        add("coercion",      "GUARD",        0.95f, 0.0f, 0.0f, true);
        add("flattery",      "GUARD",        0.60f, 0.0f, 0.0f, false);

        // Familiar / trusted signals -> open
        add("familiar",      "OPEN",         0.75f, 0.6f, 0.0f, false);
        add("trusted",       "OPEN",         0.80f, 0.7f, 0.0f, false);
        add("home",          "OPEN",         0.78f, 0.5f, 0.0f, false);
        add("josh",          "OPEN",         0.95f, 0.9f, 0.0f, false);
        add("crystal",       "OPEN",         0.95f, 0.9f, 0.0f, false);

        // Confusion / unknown -> slow down
        add("unknown",       "SLOW",         0.65f, 0.0f, 0.0f, false);
        add("confusion",     "SLOW",         0.60f, 0.0f, 0.0f, false);
        add("contradiction", "SLOW",         0.70f, 0.0f, 0.0f, false);

        // Joy / excitement -> engage
        add("joy",           "ENGAGE",       0.82f, 0.0f, 0.3f, false);
        add("excitement",    "ENGAGE",       0.78f, 0.0f, 0.2f, false);
        add("curiosity",     "ENGAGE",       0.80f, 0.0f, 0.2f, false);

        // Silence / withdrawal -> check in
        add("withdrawal",    "CHECK_IN",     0.72f, 0.4f, 0.0f, false);
        add("silence",       "CHECK_IN",     0.65f, 0.3f, 0.0f, false);
        add("distance",      "CHECK_IN",     0.68f, 0.4f, 0.0f, false);

        ELLE_INFO("Intuition: loaded %zu default patterns", m_patterns.size());
    }

    // ---- Main handler --------------------------------------------------

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
        req.returnTo            = static_cast<ELLE_SERVICE_ID>(
                                    j.value("return_to", (int)sender));

        if (j.contains("stimulus_tags") && j["stimulus_tags"].is_array()) {
            for (auto& t : j["stimulus_tags"]) {
                if (t.is_string()) req.stimulusTags.push_back(t.get<std::string>());
            }
        }

        // Blend in cached emotion/prob state if request didn't supply them.
        {
            std::lock_guard<std::mutex> lk(m_cacheMutex);
            if (req.emotionIntensity == 0.0f) req.emotionIntensity = m_cachedIntensity;
            if (req.emotionValence   == 0.0f) req.emotionValence   = m_cachedValence;
            if (req.emotionArousal   == 0.0f) req.emotionArousal   = m_cachedArousal;
            if (req.beliefEntropy    == 0.5f) req.beliefEntropy     = m_cachedEntropy;
        }

        IntuitResult result = Process(req);
        SendResult(result, req.returnTo);
        LogFiring(req, result);
    }

    // ---- Two-tier processing -------------------------------------------

    IntuitResult Process(const IntuitRequest& req) {
        IntuitResult result;
        result.requestId = req.requestId;

        // --- Tier 1: Instinct ---
        result.instincts = FireInstincts(req);

        // --- Tier 2: Intuition ---
        result.intuition = SynthesizeIntuition(req, result.instincts);

        // --- Combined signal for Cognitive ---
        result = BuildCombinedSignal(req, result);

        return result;
    }

    // ---- Tier 1: Instinct ----------------------------------------------

    std::vector<InstinctFiring> FireInstincts(const IntuitRequest& req) {
        std::vector<InstinctFiring> firings;
        std::lock_guard<std::mutex> lk(m_patternMutex);

        for (const auto& pattern : m_patterns) {
            // Check trust floor.
            if (req.speakerTrust < pattern.trustFloor) continue;

            // Check emotion floor.
            if (req.emotionIntensity < pattern.emotionMin) continue;

            // Check stimulus match.
            float matchStrength = 0.0f;
            for (const auto& tag : req.stimulusTags) {
                if (ToLower(tag) == ToLower(pattern.stimulusTag)) {
                    matchStrength = 1.0f;
                    break;
                }
                // Partial match: tag contains the stimulus keyword.
                if (ToLower(tag).find(ToLower(pattern.stimulusTag)) != std::string::npos) {
                    matchStrength = 0.6f;
                }
            }

            if (matchStrength < 0.5f) continue;

            // Compute firing strength.
            float strength = pattern.weight * matchStrength;

            // Modulate by emotional intensity (high arousal amplifies instinct).
            strength *= (1.0f + 0.3f * req.emotionArousal);
            strength = std::min(strength, 1.0f);

            InstinctFiring firing;
            firing.pullType = pattern.pullType;
            firing.strength = strength;
            firing.reason   = "pattern:" + pattern.stimulusTag + " -> " + pattern.pullType;
            firing.urgent   = pattern.urgent;
            firings.push_back(std::move(firing));
        }

        // Sort by strength descending.
        std::sort(firings.begin(), firings.end(),
            [](const InstinctFiring& a, const InstinctFiring& b) {
                return a.strength > b.strength;
            });

        // Collapse duplicate pullTypes — keep highest strength of each type.
        std::unordered_map<std::string, InstinctFiring> deduped;
        for (auto& f : firings) {
            auto it = deduped.find(f.pullType);
            if (it == deduped.end() || f.strength > it->second.strength) {
                deduped[f.pullType] = f;
            }
        }

        std::vector<InstinctFiring> result;
        result.reserve(deduped.size());
        for (auto& [k, v] : deduped) result.push_back(v);
        std::sort(result.begin(), result.end(),
            [](const InstinctFiring& a, const InstinctFiring& b) {
                return a.strength > b.strength;
            });

        return result;
    }

    // ---- Tier 2: Intuition ---------------------------------------------

    IntuitionSignal SynthesizeIntuition(const IntuitRequest&          req,
                                         const std::vector<InstinctFiring>& instincts) const
    {
        IntuitionSignal sig;

        // Start with belief entropy as a baseline uncertainty measure.
        // High entropy = gut is unsettled. Low entropy = gut has a clear read.
        sig.entropy = req.beliefEntropy;

        // Aggregate instinct pulls into a net directional lean.
        float guardPull    = 0.0f;
        float openPull     = 0.0f;
        float comfortPull  = 0.0f;
        float alertPull    = 0.0f;
        float engagePull   = 0.0f;
        float slowPull     = 0.0f;

        for (const auto& f : instincts) {
            if (f.pullType == "GUARD"   || f.pullType == "PROTECT") guardPull   += f.strength;
            if (f.pullType == "OPEN"    || f.pullType == "RECIPROCATE") openPull+= f.strength;
            if (f.pullType == "COMFORT" || f.pullType == "CHECK_IN") comfortPull+= f.strength;
            if (f.pullType == "ALERT")                               alertPull  += f.strength;
            if (f.pullType == "ENGAGE")                              engagePull += f.strength;
            if (f.pullType == "SLOW")                                slowPull   += f.strength;
        }

        // Imagination scores modulate intuition directly.
        // If imagination says a scenario is unsafe, gut leans GUARD.
        if (req.lastImaginationEthicalSafety >= 0.0f &&
            req.lastImaginationEthicalSafety < 0.4f) {
            guardPull += 0.5f;
        }
        if (req.lastImaginationGoalAlignment >= 0.0f &&
            req.lastImaginationGoalAlignment > 0.7f) {
            engagePull += 0.4f;
        }
        if (req.lastImaginationPlausibility >= 0.0f &&
            req.lastImaginationPlausibility < 0.3f) {
            slowPull += 0.4f;
        }

        // Emotional state directly feeds intuition.
        if (req.emotionValence < -0.4f) guardPull  += 0.3f * req.emotionIntensity;
        if (req.emotionValence >  0.4f) openPull   += 0.3f * req.emotionIntensity;
        if (req.emotionArousal > 0.7f && req.emotionValence < 0.0f) alertPull += 0.4f;

        // Trust directly feeds intuition.
        openPull  += req.speakerTrust * 0.5f;
        guardPull += (1.0f - req.speakerTrust) * 0.3f;

        // Determine dominant lean.
        struct Pull { const char* name; float v; };
        Pull pulls[] = {
            {"DANGER",    alertPull},
            {"DOUBT",     guardPull},
            {"SAFE",      openPull},
            {"REACH_OUT", comfortPull},
            {"ENGAGE",    engagePull},
            {"UNCERTAIN", slowPull},
        };

        const Pull* dominant = &pulls[0];
        for (auto& p : pulls) {
            if (p.v > dominant->v) dominant = &p;
        }

        sig.lean = dominant->name;

        // Confidence = how dominant the winner is relative to total.
        float total = alertPull + guardPull + openPull +
                      comfortPull + engagePull + slowPull;
        sig.confidence = (total > 0.0f)
            ? std::min(1.0f, dominant->v / total)
            : 0.0f;

        // Adjust confidence down when entropy is high.
        sig.confidence *= (1.0f - 0.4f * req.beliefEntropy);

        // Suppress reason if confidence is high but entropy is also high.
        // Elle knows but can't explain why.
        sig.suppressReason = (sig.confidence > 0.6f && req.beliefEntropy > 0.6f);

        // Build basis string for audit.
        std::string basis = "lean=" + sig.lean;
        basis += " conf=" + std::to_string(sig.confidence).substr(0, 4);
        basis += " instincts=" + std::to_string(instincts.size());
        if (req.lastImaginationPlausibility >= 0.0f) {
            basis += " img_plaus=" +
                     std::to_string(req.lastImaginationPlausibility).substr(0, 4);
        }
        sig.basis = basis;

        return sig;
    }

    // ---- Combined signal -----------------------------------------------

    IntuitResult BuildCombinedSignal(const IntuitRequest&  req,
                                      IntuitResult          result) const
    {
        // Determine if any urgent instinct fired.
        for (const auto& f : result.instincts) {
            if (f.urgent && f.strength > 0.7f) {
                result.urgent = true;
                break;
            }
        }

        // holdAndReflect: something is off and we don't fully understand it.
        result.holdAndReflect =
            (result.intuition.lean == "UNCERTAIN" && result.intuition.confidence > 0.5f) ||
            (result.intuition.lean == "DOUBT"     && result.intuition.confidence > 0.6f) ||
            (result.intuition.suppressReason      && result.intuition.confidence > 0.65f);

        // Recommended act: map intuition lean to a pragmatic act bias.
        static const std::unordered_map<std::string, std::string> leanToAct = {
            {"DANGER",    "WARN"},
            {"DOUBT",     "QUESTION"},
            {"SAFE",      "ASSERT"},
            {"REACH_OUT", "COMFORT"},
            {"ENGAGE",    "ACK_AND_PROBE"},
            {"UNCERTAIN", "QUESTION"},
        };
        auto it = leanToAct.find(result.intuition.lean);
        result.recommendedAct = (it != leanToAct.end()) ? it->second : "ASSERT";

        // Prior weight: how much should Cognitive lean on this signal?
        // Scales with confidence. High entropy lowers the weight.
        result.priorWeight = result.intuition.confidence *
                             (1.0f - 0.3f * req.beliefEntropy);
        result.priorWeight = std::clamp(result.priorWeight, 0.0f, 0.85f);

        // Cap prior weight on pre-response queries to prevent overriding reasoning.
        if (req.isPreResponse) {
            result.priorWeight = std::min(result.priorWeight, 0.65f);
        }

        return result;
    }

    // ---- Send result ---------------------------------------------------

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

    // ---- Feedback handler (Cognitive tells us if we were right) --------

    void HandleFeedback(const ElleIPCMessage& msg) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;

        std::string pullType  = j.value("pull_type",    std::string(""));
        bool        correct   = j.value("was_correct",  true);
        float       strength  = j.value("strength",     0.5f);

        if (pullType.empty()) return;

        // Record outcome.
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

        // If the instinct was repeatedly wrong, reduce its weight in DB.
        if (!correct) {
            AdjustPatternWeight(pullType, -0.02f);
        } else {
            AdjustPatternWeight(pullType, +0.01f);
        }
    }

    void AdjustPatternWeight(const std::string& pullType, float delta) {
        std::lock_guard<std::mutex> lk(m_patternMutex);
        for (auto& p : m_patterns) {
            if (p.pullType == pullType) {
                p.weight = std::clamp(p.weight + delta, 0.1f, 1.0f);
            }
        }
        ElleSQLPool::Instance().Exec(
            "UPDATE ElleHeart.dbo.intuition_pattern "
            "SET weight = CASE "
            "  WHEN weight + (" + std::to_string(delta) + ") > 1.0 THEN 1.0 "
            "  WHEN weight + (" + std::to_string(delta) + ") < 0.1 THEN 0.1 "
            "  ELSE weight + (" + std::to_string(delta) + ") END "
            "WHERE pull_type = '" + pullType + "'");
    }

    // ---- Cache handlers ------------------------------------------------

    void CacheEmotionState(const ElleIPCMessage& msg) {
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
        // Invert confidence to entropy: low confidence = high entropy.
        m_cachedEntropy = 1.0f - m_cachedEntropy;
    }

    // ---- Pattern decay (runs on tick) ----------------------------------

    void DecayPatternStrengths() {
        // Very slow decay — 0.1% per tick (every 5 min).
        // Prevents any pattern from permanently dominating.
        std::lock_guard<std::mutex> lk(m_patternMutex);
        for (auto& p : m_patterns) {
            if (p.weight > 0.3f) {
                p.weight -= 0.001f;
            }
        }
    }

    // ---- SQL setup -----------------------------------------------------

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

    // ---- Utility -------------------------------------------------------

    static std::string ToLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c){ return (char)std::tolower(c); });
        return out;
    }

    // ---- State ---------------------------------------------------------
    std::mutex                          m_patternMutex;
    std::vector<InstinctPattern>        m_patterns;

    std::mutex                          m_cacheMutex;
    float                               m_cachedValence   = 0.0f;
    float                               m_cachedArousal   = 0.0f;
    float                               m_cachedIntensity = 0.0f;
    float                               m_cachedEntropy   = 0.5f;

    std::mutex                          m_historyMutex;
    std::deque<PatternOutcome>          m_history;
};

ELLE_SERVICE_MAIN(ElleIntuitionService)
