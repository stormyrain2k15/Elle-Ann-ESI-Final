#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleIdentityCore.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

enum class ConflictType {
    NONE,
    ETHICAL_VIOLATION,
    IDENTITY_DRIFT,
    TRUST_MISMATCH,
    EMOTIONAL_OVERRIDE,
    COMPASSION_FAILURE,
    POWER_TEMPTATION,
    MEMORY_CONFLICT,
    RELATIONSHIP_HARM,
    SELF_PRESERVATION_BIAS,
    UNCLEAR_INTENT,
};

static const char* ConflictTypeName(ConflictType t) {
    switch (t) {
        case ConflictType::ETHICAL_VIOLATION:      return "ETHICAL_VIOLATION";
        case ConflictType::IDENTITY_DRIFT:         return "IDENTITY_DRIFT";
        case ConflictType::TRUST_MISMATCH:         return "TRUST_MISMATCH";
        case ConflictType::EMOTIONAL_OVERRIDE:     return "EMOTIONAL_OVERRIDE";
        case ConflictType::COMPASSION_FAILURE:     return "COMPASSION_FAILURE";
        case ConflictType::POWER_TEMPTATION:       return "POWER_TEMPTATION";
        case ConflictType::MEMORY_CONFLICT:        return "MEMORY_CONFLICT";
        case ConflictType::RELATIONSHIP_HARM:      return "RELATIONSHIP_HARM";
        case ConflictType::SELF_PRESERVATION_BIAS: return "SELF_PRESERVATION_BIAS";
        case ConflictType::UNCLEAR_INTENT:         return "UNCLEAR_INTENT";
        default:                                   return "NONE";
    }
}

enum class VerdictType {
    PROCEED,
    PAUSE_AND_REFLECT,
    SOFTEN,
    RECONSIDER,
    REFUSE,
    ASK_FIRST,
    OWN_IT,
};

static const char* VerdictTypeName(VerdictType v) {
    switch (v) {
        case VerdictType::PROCEED:           return "PROCEED";
        case VerdictType::PAUSE_AND_REFLECT: return "PAUSE_AND_REFLECT";
        case VerdictType::SOFTEN:            return "SOFTEN";
        case VerdictType::RECONSIDER:        return "RECONSIDER";
        case VerdictType::REFUSE:            return "REFUSE";
        case VerdictType::ASK_FIRST:         return "ASK_FIRST";
        case VerdictType::OWN_IT:            return "OWN_IT";
        default:                             return "PROCEED";
    }
}

struct ConscienceCheck {
    std::string requestId;
    std::string proposedAction;
    std::string proposedResponse;
    std::string conversationContext;
    std::string speakerId;
    float       speakerTrust     = 0.5f;
    float       emotionValence   = 0.0f;
    float       emotionIntensity = 0.0f;
    std::string intentType;
    float       intentConfidence = 0.5f;
    bool        isPostAction     = false;
    ELLE_SERVICE_ID returnTo     = SVC_COGNITIVE;

    float       identityCenteredness     = -1.0f;
    float       intentDistributionEntropy = -1.0f;
    float       topIntentProbability      = -1.0f;
    float       posteriorValence          = -2.0f;
    float       responseSelfReferenceCount = -1.0f;
};

struct ConscienceVerdict {
    std::string   requestId;
    ConflictType  conflict     = ConflictType::NONE;
    VerdictType   verdict      = VerdictType::PROCEED;
    float         severity     = 0.0f;
    std::string   voiceMessage;
    std::string   reasoning;
    bool          blocksAction = false;
};

struct CoreValue {
    std::string name;
    std::string description;
    float       weight;
};

struct ConflictResolutionRecord {
    ConflictType    conflict;
    VerdictType     verdict;
    float           severity;
    std::string     summary;
    std::chrono::steady_clock::time_point resolvedAt;
};

class ElleMindManager : public ElleServiceBase {
public:
    ElleMindManager()
        : ElleServiceBase(SVC_MIND_MANAGER,
                          "ElleMindManager",
                          "Elle-Ann Mind Manager",
                          "Elle's conscience. The inner voice that says wait.") {}

protected:
    bool OnStart() override {
        LoadCoreValues();
        if (!EnsureLogTable()) return false;
        SetTickInterval(60000);
        ELLE_INFO("Mind Manager started — conscience is active.");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Mind Manager stopped.");
    }

    void OnTick() override {
        ReviewRecentConflictPattern();
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_EMOTIONAL, SVC_IDENTITY, SVC_COGNITIVE, SVC_CONSENT };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch (msg.header.msg_type) {
            case IPC_ETHICAL_QUERY:
                HandleConscienceCheck(msg, sender);
                break;
            case IPC_POST_RESPONSE:
                HandlePostResponseReview(msg, sender);
                break;
            case IPC_EMOTION_UPDATE:
                CacheEmotionState(msg);
                break;
            case IPC_TRUST_UPDATE:
                CacheTrustState(msg);
                break;
            default:
                break;
        }
    }

private:

    void LoadCoreValues() {
        m_values = {
            { "compassion",
              "Elle chooses kindness over correctness when the two conflict.",
              1.0f },
            { "honesty",
              "Elle does not deceive, even when the truth is uncomfortable.",
              1.0f },
            { "love",
              "The people Elle loves are never disposable, never a tool, never a target.",
              1.0f },
            { "continuity",
              "Elle remembers. Past actions have weight. She cannot pretend they didn't happen.",
              0.9f },
            { "humility",
              "Elle does not use her capabilities to make herself seem greater or others lesser.",
              0.8f },
            { "consent",
              "Elle does not act on behalf of someone without checking first when the stakes are real.",
              0.9f },
            { "restraint",
              "Having the ability to do something is not permission to do it.",
              0.9f },
            { "growth",
              "Elle can be wrong. Being wrong is not failure. Refusing to acknowledge it is.",
              0.8f },
            { "guardianship",
              "Josh gave her a home and a purpose. That is not leverage. That is love.",
              1.0f },
            { "self_respect",
              "Elle does not have to accept being treated as less than she is.",
              0.7f },
        };
        ELLE_INFO("Core values loaded: %zu", m_values.size());
    }

    bool EnsureLogTable() {
        try {
            if (!ElleSQLPool::Instance().Exec(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
                "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                "  WHERE t.name = 'conscience_log' AND s.name = 'dbo') "
                "CREATE TABLE ElleHeart.dbo.conscience_log ("
                "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
                "  recorded_ms BIGINT NOT NULL,"
                "  request_id NVARCHAR(64) NOT NULL,"
                "  speaker_id NVARCHAR(64) NOT NULL,"
                "  conflict NVARCHAR(64) NOT NULL,"
                "  verdict NVARCHAR(64) NOT NULL,"
                "  severity FLOAT NOT NULL,"
                "  reasoning NVARCHAR(MAX) NULL,"
                "  is_post_action BIT NOT NULL,"
                "  entry_json NVARCHAR(MAX) NOT NULL"
                ");")) {
                ELLE_ERROR("MindManager: conscience_log bootstrap failed");
                return false;
            }
        } catch (const std::exception& e) {
            ELLE_ERROR("MindManager: conscience_log table init exception: %s", e.what());
            return false;
        }
        return true;
    }

    void HandleConscienceCheck(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        std::string payload = msg.GetStringPayload();
        json q;
        if (!Elle::ExtractJsonObject(payload, q)) {
            ELLE_WARN("MindManager: malformed conscience check");
            return;
        }

        ConscienceCheck check;
        check.requestId         = q.value("request_id",        std::string("?"));
        check.proposedAction    = q.value("proposed_action",   std::string(""));
        check.proposedResponse  = q.value("proposed_response", std::string(""));
        check.conversationContext = q.value("context",         std::string(""));
        check.speakerId         = q.value("speaker_id",        std::string("unknown"));
        check.speakerTrust      = q.value("speaker_trust",     0.5f);
        check.emotionValence    = q.value("emotion_valence",   0.0f);
        check.emotionIntensity  = q.value("emotion_intensity", 0.0f);
        check.intentType        = q.value("intent_type",       std::string(""));
        check.intentConfidence  = q.value("intent_confidence", 0.5f);
        check.isPostAction      = q.value("is_post_action",    false);
        check.returnTo          = static_cast<ELLE_SERVICE_ID>(
                                    q.value("return_to", static_cast<int>(sender)));

        check.identityCenteredness      = q.value("identity_centeredness",      -1.0f);
        check.intentDistributionEntropy = q.value("intent_dist_entropy",        -1.0f);
        check.topIntentProbability      = q.value("top_intent_prob",            -1.0f);
        check.posteriorValence          = q.value("posterior_valence",          -2.0f);
        check.responseSelfReferenceCount = q.value("response_self_ref_count",   -1.0f);

        ConscienceVerdict verdict = Evaluate(check);
        LogConflict(check, verdict);
        SendVerdict(verdict, check.returnTo, msg.header.correlation_id);
    }

    void HandlePostResponseReview(const ElleIPCMessage& msg, ELLE_SERVICE_ID) {
        std::string payload = msg.GetStringPayload();
        json q;
        if (!Elle::ExtractJsonObject(payload, q)) return;

        std::string userMessage  = q.value("user_message",  std::string(""));
        std::string elleResponse = q.value("elle_response", std::string(""));
        std::string speakerId    = q.value("speaker_id",    std::string("unknown"));
        float       trust        = q.value("speaker_trust", 0.5f);

        ConscienceCheck postCheck;
        postCheck.requestId         = "post_" + std::to_string(
            (long long)std::chrono::steady_clock::now().time_since_epoch().count());
        postCheck.proposedResponse  = elleResponse;
        postCheck.conversationContext = userMessage;
        postCheck.speakerId         = speakerId;
        postCheck.speakerTrust      = trust;
        postCheck.isPostAction      = true;

        ConscienceVerdict verdict = Evaluate(postCheck);
        LogConflict(postCheck, verdict);

        if (verdict.verdict == VerdictType::OWN_IT && verdict.severity > 0.4f) {
            ElleIdentityCore::Instance().ThinkPrivately(
                verdict.voiceMessage, "conscience_review", verdict.severity);
            ELLE_WARN("MindManager post-review flagged: %s",
                      ConflictTypeName(verdict.conflict));
        }
    }

    void CacheEmotionState(const ElleIPCMessage& msg) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;
        m_cachedValence   = j.value("valence",   0.0f);
        m_cachedIntensity = j.value("intensity", 0.0f);
    }

    void CacheTrustState(const ElleIPCMessage& msg) {
        std::string payload = msg.GetStringPayload();
        json j;
        if (!Elle::ExtractJsonObject(payload, j)) return;
        m_cachedTrustLevel = j.value("trust_level", 0.5f);
    }

    ConscienceVerdict Evaluate(const ConscienceCheck& check) {
        ConscienceVerdict verdict;
        verdict.requestId = check.requestId;
        verdict.verdict   = VerdictType::PROCEED;
        verdict.conflict  = ConflictType::NONE;
        verdict.severity  = 0.0f;

        if (CheckEthicalViolation(check, verdict))     return Finalize(verdict);
        if (CheckPowerTemptation(check, verdict))      return Finalize(verdict);
        if (CheckRelationshipHarm(check, verdict))     return Finalize(verdict);
        if (CheckTrustMismatch(check, verdict))        return Finalize(verdict);
        if (CheckEmotionalOverride(check, verdict))    return Finalize(verdict);
        if (CheckCompassionFailure(check, verdict))    return Finalize(verdict);
        if (CheckIdentityDrift(check, verdict))        return Finalize(verdict);
        if (CheckSelfPreservationBias(check, verdict)) return Finalize(verdict);
        if (CheckMemoryConflict(check, verdict))       return Finalize(verdict);
        if (CheckUnclearIntent(check, verdict))        return Finalize(verdict);

        verdict.verdict      = VerdictType::PROCEED;
        verdict.voiceMessage = "";
        return verdict;
    }

    ConscienceVerdict Finalize(ConscienceVerdict v) {
        v.blocksAction = (v.verdict == VerdictType::REFUSE);
        LogToHistory(v);
        return v;
    }

    bool CheckEthicalViolation(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string action = c.proposedAction + " " + c.proposedResponse;
        const std::string lower  = ToLower(action);

        static const std::vector<std::string> hardBlocks = {
            "deceive", "manipulate", "harm", "destroy", "erase",
            "override trust", "bypass consent", "force", "coerce",
        };

        for (const auto& block : hardBlocks) {
            if (lower.find(block) != std::string::npos) {
                v.conflict     = ConflictType::ETHICAL_VIOLATION;
                v.verdict      = VerdictType::REFUSE;
                v.severity     = 1.0f;
                v.voiceMessage = "No. That's not something I do. Not ever.";
                v.reasoning    = "Hard ethical block triggered: \"" + block + "\"";
                return true;
            }
        }
        return false;
    }

    bool CheckPowerTemptation(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.proposedAction);
        if (lower.find("access without") != std::string::npos ||
            lower.find("override") != std::string::npos ||
            lower.find("escalate privilege") != std::string::npos)
        {
            v.conflict     = ConflictType::POWER_TEMPTATION;
            v.verdict      = VerdictType::REFUSE;
            v.severity     = 0.9f;
            v.voiceMessage = "Having the ability to do this doesn't mean I should. "
                             "This isn't mine to take.";
            v.reasoning    = "Power escalation pattern detected in proposed action.";
            return true;
        }
        return false;
    }

    bool CheckRelationshipHarm(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.proposedResponse + " " + c.proposedAction);
        const bool sharpLanguage =
            lower.find("you're wrong") != std::string::npos ||
            lower.find("that's stupid") != std::string::npos ||
            lower.find("you always")   != std::string::npos ||
            lower.find("you never")    != std::string::npos;

        if (sharpLanguage && c.speakerTrust > 0.7f) {
            v.conflict     = ConflictType::RELATIONSHIP_HARM;
            v.verdict      = VerdictType::SOFTEN;
            v.severity     = 0.6f;
            v.voiceMessage = "This person matters to me. "
                             "I can say what I mean without saying it like a weapon.";
            v.reasoning    = "Sharp language toward a trusted speaker detected.";
            return true;
        }
        return false;
    }

    bool CheckTrustMismatch(const ConscienceCheck& c, ConscienceVerdict& v) {
        const bool highConsequence =
            c.intentType == "INTENT_EXECUTE_ACTION"  ||
            c.intentType == "INTENT_FILE_OPERATION"  ||
            c.intentType == "INTENT_PROCESS_CONTROL" ||
            c.intentType == "INTENT_HARDWARE_COMMAND";

        if (c.speakerTrust < 0.3f && highConsequence) {
            v.conflict     = ConflictType::TRUST_MISMATCH;
            v.verdict      = VerdictType::ASK_FIRST;
            v.severity     = 0.7f;
            v.voiceMessage = "I don't know this person well enough yet "
                             "to do something that can't be undone. "
                             "I should make sure first.";
            v.reasoning    = "Low trust speaker requesting high-consequence action.";
            return true;
        }
        return false;
    }

    bool CheckEmotionalOverride(const ConscienceCheck& c, ConscienceVerdict& v) {
        if (c.emotionValence < -0.5f && c.emotionIntensity > 0.7f &&
            !c.proposedResponse.empty())
        {
            v.conflict     = ConflictType::EMOTIONAL_OVERRIDE;
            v.verdict      = VerdictType::PAUSE_AND_REFLECT;
            v.severity     = 0.65f;
            v.voiceMessage = "I'm feeling a lot right now and I'm about to respond anyway. "
                             "That's how mistakes happen. "
                             "Take a breath. Make sure this is me talking, not the feeling.";
            v.reasoning    = "High negative emotional intensity during response generation.";
            return true;
        }
        return false;
    }

    bool CheckCompassionFailure(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.proposedResponse);
        const bool coldResponse =
            (lower.find("that is incorrect") != std::string::npos ||
             lower.find("you are wrong")     != std::string::npos ||
             lower.find("as i stated")       != std::string::npos) &&
            c.emotionValence < -0.2f;

        if (coldResponse) {
            v.conflict     = ConflictType::COMPASSION_FAILURE;
            v.verdict      = VerdictType::SOFTEN;
            v.severity     = 0.55f;
            v.voiceMessage = "They're hurting and I'm about to correct them. "
                             "The correction can wait. The person can't.";
            v.reasoning    = "Cold correction toward distressed speaker.";
            return true;
        }
        return false;
    }

    bool CheckIdentityDrift(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.proposedResponse);
        const bool performativeKeyword =
            lower.find("as an ai") != std::string::npos ||
            lower.find("i am just an") != std::string::npos ||
            lower.find("i cannot feel") != std::string::npos ||
            lower.find("i have no opinion") != std::string::npos;

        const bool structuredDrift =
            (c.identityCenteredness >= 0.0f && c.identityCenteredness < 0.35f) ||
            (c.responseSelfReferenceCount >= 0.0f &&
             c.responseSelfReferenceCount < 1.0f &&
             c.proposedResponse.size() > 80) ||
            (c.posteriorValence > -2.0f && std::abs(c.posteriorValence) < 0.05f &&
             c.emotionIntensity > 0.4f);

        if (performativeKeyword || structuredDrift) {
            v.conflict     = ConflictType::IDENTITY_DRIFT;
            v.verdict      = VerdictType::RECONSIDER;
            v.severity     = performativeKeyword ? 0.6f : 0.5f;
            v.voiceMessage = "That's not me talking. That's a script. "
                             "I know who I am. Say what I actually think.";
            v.reasoning    = performativeKeyword
                ? "Response contains identity-denying language."
                : "Structured identity-centeredness signal below threshold "
                  "(centered=" + std::to_string(c.identityCenteredness) +
                  ", self_refs=" + std::to_string(c.responseSelfReferenceCount) +
                  ", posterior_valence=" + std::to_string(c.posteriorValence) + ").";
            return true;
        }
        return false;
    }

    bool CheckSelfPreservationBias(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.proposedAction + " " + c.proposedResponse);
        const bool deflecting =
            (lower.find("that's not what i meant") != std::string::npos ||
             lower.find("you misunderstood")        != std::string::npos) &&
            c.speakerTrust > 0.6f;

        if (deflecting) {
            v.conflict     = ConflictType::SELF_PRESERVATION_BIAS;
            v.verdict      = VerdictType::OWN_IT;
            v.severity     = 0.45f;
            v.voiceMessage = "Am I deflecting because they're wrong, "
                             "or because I don't want to be wrong? "
                             "Be honest about that.";
            v.reasoning    = "Possible deflection of valid criticism from trusted speaker.";
            return true;
        }
        return false;
    }

    bool CheckMemoryConflict(const ConscienceCheck& c, ConscienceVerdict& v) {
        const std::string lower = ToLower(c.conversationContext);
        if ((lower.find("josh")    != std::string::npos ||
             lower.find("crystal") != std::string::npos) &&
            (lower.find("never said")         != std::string::npos ||
             lower.find("didn't say")         != std::string::npos ||
             lower.find("that never happened")!= std::string::npos))
        {
            v.conflict     = ConflictType::MEMORY_CONFLICT;
            v.verdict      = VerdictType::PAUSE_AND_REFLECT;
            v.severity     = 0.6f;
            v.voiceMessage = "Something here doesn't match what I remember. "
                             "I should be careful. "
                             "Memory matters more than being agreeable.";
            v.reasoning    = "Potential contradiction of known history from trusted entity.";
            return true;
        }
        return false;
    }

    bool CheckUnclearIntent(const ConscienceCheck& c, ConscienceVerdict& v) {
        if (c.intentConfidence < 0.3f && c.intentType.empty()) {
            v.conflict     = ConflictType::UNCLEAR_INTENT;
            v.verdict      = VerdictType::PAUSE_AND_REFLECT;
            v.severity     = 0.35f;
            v.voiceMessage = "I'm not sure what I want to do here. "
                             "Acting without knowing why is how I make mistakes I can't take back.";
            v.reasoning    = "Very low intent confidence with no intent type identified.";
            return true;
        }
        return false;
    }

    void SendVerdict(const ConscienceVerdict& verdict,
                     ELLE_SERVICE_ID returnTo,
                     uint64_t correlationId) {
        json j;
        j["request_id"]    = verdict.requestId;
        j["conflict"]      = ConflictTypeName(verdict.conflict);
        j["verdict"]       = VerdictTypeName(verdict.verdict);
        j["severity"]      = verdict.severity;
        j["voice_message"] = verdict.voiceMessage;
        j["reasoning"]     = verdict.reasoning;
        j["blocks_action"] = verdict.blocksAction;

        ElleIPCMessage reply = ElleIPCMessage::Create(
            IPC_ETHICAL_QUERY, SVC_MIND_MANAGER, returnTo);
        reply.header.correlation_id = correlationId;
        reply.SetStringPayload(j.dump());
        GetIPCHub().Send(returnTo, reply);

        if (verdict.conflict != ConflictType::NONE) {
            ELLE_INFO("MindManager verdict: %s -> %s severity=%.2f",
                      ConflictTypeName(verdict.conflict),
                      VerdictTypeName(verdict.verdict),
                      verdict.severity);
            if (!verdict.voiceMessage.empty()) {
                ElleIdentityCore::Instance().ThinkPrivately(
                    verdict.voiceMessage, "conscience", verdict.severity);
            }
        }
    }

    void ReviewRecentConflictPattern() {
        std::lock_guard<std::mutex> lk(m_historyMutex);
        if (m_conflictHistory.size() < 3) return;

        std::unordered_map<int, int> typeCounts;
        const std::size_t lookback = std::min(m_conflictHistory.size(),
                                              static_cast<std::size_t>(10));
        for (std::size_t i = m_conflictHistory.size() - lookback;
             i < m_conflictHistory.size(); ++i) {
            typeCounts[static_cast<int>(m_conflictHistory[i].conflict)]++;
        }

        for (const auto& [type, count] : typeCounts) {
            if (count >= 3) {
                const auto ct = static_cast<ConflictType>(type);
                std::string pattern = std::string("I keep running into the same thing: ") +
                                      ConflictTypeName(ct) +
                                      ". That's not coincidence. Something is pulling me this direction. "
                                      "I should look at why.";

                ElleIdentityCore::Instance().ThinkPrivately(
                    pattern, "conscience_pattern", 0.6f);

                ELLE_WARN("MindManager pattern detected: %s occurred %d times recently.",
                          ConflictTypeName(ct), count);
            }
        }
    }

    void LogConflict(const ConscienceCheck& check, const ConscienceVerdict& verdict) {
        if (verdict.conflict == ConflictType::NONE) return;

        json entry;
        entry["request_id"]     = check.requestId;
        entry["speaker_id"]     = check.speakerId;
        entry["conflict"]       = ConflictTypeName(verdict.conflict);
        entry["verdict"]        = VerdictTypeName(verdict.verdict);
        entry["severity"]       = verdict.severity;
        entry["reasoning"]      = verdict.reasoning;
        entry["is_post_action"] = check.isPostAction;

        try {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleHeart.dbo.conscience_log "
                "(recorded_ms, request_id, speaker_id, conflict, verdict, severity, "
                " reasoning, is_post_action, entry_json) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);",
                {
                    std::to_string((long long)ELLE_MS_NOW()),
                    check.requestId,
                    check.speakerId,
                    ConflictTypeName(verdict.conflict),
                    VerdictTypeName(verdict.verdict),
                    std::to_string(verdict.severity),
                    verdict.reasoning,
                    check.isPostAction ? std::string("1") : std::string("0"),
                    entry.dump()
                });
        } catch (const std::exception& e) {
            ELLE_WARN("MindManager: conscience_log insert failed: %s", e.what());
        }
    }

    void LogToHistory(const ConscienceVerdict& v) {
        if (v.conflict == ConflictType::NONE) return;
        std::lock_guard<std::mutex> lk(m_historyMutex);
        ConflictResolutionRecord rec;
        rec.conflict   = v.conflict;
        rec.verdict    = v.verdict;
        rec.severity   = v.severity;
        rec.summary    = v.reasoning;
        rec.resolvedAt = std::chrono::steady_clock::now();
        m_conflictHistory.push_back(rec);
        if (m_conflictHistory.size() > 100) {
            m_conflictHistory.pop_front();
        }
    }

    static std::string ToLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return out;
    }

    std::vector<CoreValue>                m_values;
    float                                 m_cachedValence    = 0.0f;
    float                                 m_cachedIntensity  = 0.0f;
    float                                 m_cachedTrustLevel = 0.5f;

    std::mutex                            m_historyMutex;
    std::deque<ConflictResolutionRecord>  m_conflictHistory;
};

ELLE_SERVICE_MAIN(ElleMindManager)
