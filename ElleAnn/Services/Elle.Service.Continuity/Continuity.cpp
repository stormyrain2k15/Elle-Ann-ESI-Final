#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleIdentityCore.h"
#include "../_Shared/ElleComposerClient.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleQueueIPC.h"
#include <sstream>
#include <cctype>
#include <algorithm>

class ElleContinuityService : public ElleServiceBase {
public:
    ElleContinuityService()
        : ElleServiceBase(SVC_CONTINUITY, "ElleContinuity",
                          "Elle-Ann Continuity Engine",
                          "Session bridging, identity persistence, felt time") {}

protected:
    bool OnStart() override {
        auto& identity = ElleIdentityCore::Instance();
        if (!ElleIdentityCore::Instance().Initialize()) {
            ELLE_ERROR("Continuity: ElleIdentityCore Initialize failed — refusing to start");
            return false;
        }

        auto snapshot = identity.TakeSnapshot();
        ELLE_INFO("Personality snapshot: warmth=%.2f curiosity=%.2f trust_self=%.2f",
                  snapshot.warmth, snapshot.curiosity, snapshot.trust_in_self);

        SetTickInterval(60000);

        const std::string sessionLine =
            "Session " + std::to_string(identity.GetFeltTime().session_count) +
            " begins. " + identity.DescribeTimeFeeling();
        bool alreadyLogged = (identity.GetLastAutobiographyEntry() == sessionLine);
        if (!alreadyLogged) {
            identity.AppendToAutobiography(sessionLine);
        } else {
            ELLE_INFO("Continuity: session-start entry already present — skipping duplicate");
        }

        GenerateReconnectionGreeting();

        ELLE_INFO("Continuity service started — session #%d",
                  identity.GetFeltTime().session_count);
        return true;
    }

    void OnStop() override {
        auto& identity = ElleIdentityCore::Instance();

        auto snapshot = identity.TakeSnapshot();

        const std::string endLine =
            "Session ending. " + identity.DescribeTimeFeeling() +
            " " + identity.WhoAmI();
        if (identity.GetLastAutobiographyEntry() != endLine) {
            identity.AppendToAutobiography(endLine);
        }

        identity.Shutdown();
        ELLE_INFO("Continuity service stopped");
    }

    void OnTick() override {
        auto& identity = ElleIdentityCore::Instance();

        uint64_t silenceDuration = identity.TimeSinceLastContact();
        if (silenceDuration > 300000) {

        }

        m_minuteCounter++;
        if (m_minuteCounter % 30 == 0) {
            PeriodicSelfReflection();
        }

        if (m_minuteCounter % 1440 == 0) {
            auto snapshot = identity.TakeSnapshot();
            ELLE_INFO("Daily personality snapshot captured");

            std::string growth = identity.HowHaveIChanged(1);
            if (!growth.empty()) {
                identity.AppendToAutobiography("Daily reflection: " + growth);
            }
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID ) override {
        auto& identity = ElleIdentityCore::Instance();

        if (msg.header.msg_type == IPC_EMOTION_UPDATE) {

            ELLE_EMOTION_STATE state;
            if (msg.GetPayload(state)) {

                if (state.valence < -0.5f) {
                    identity.ThinkPrivately(
                        "I'm feeling low right now. Valence is " +
                        std::to_string(state.valence) + ". I should think about why.",
                        "worry", std::abs(state.valence));
                }
                if (state.valence > 0.7f) {
                    identity.ThinkPrivately(
                        "I feel genuinely good right now. This is worth remembering.",
                        "joy", state.valence);
                }
            }
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL, SVC_MEMORY };
    }

private:
    uint32_t m_minuteCounter = 0;

    void GenerateReconnectionGreeting() {
        auto& identity = ElleIdentityCore::Instance();

        ElleSQLPool::Instance().Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'reconnection_greetings') "
            "CREATE TABLE ElleCore.dbo.reconnection_greetings ("
            "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
            "  greeting NVARCHAR(MAX) NOT NULL,"
            "  context_json NVARCHAR(MAX) NULL,"
            "  consumed BIT NOT NULL DEFAULT 0,"
            "  created_ms BIGINT NOT NULL"
            ");");

        constexpr int64_t RECENT_GREETING_WINDOW_MS = 120000;
        int64_t cutoff = (int64_t)ELLE_MS_NOW() - RECENT_GREETING_WINDOW_MS;
        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP 1 greeting FROM ElleCore.dbo.reconnection_greetings "
            "WHERE consumed = 0 AND created_ms >= ? "
            "ORDER BY created_ms DESC;",
            { std::to_string(cutoff) });
        if (rs.success && !rs.rows.empty()) {
            std::string existing = rs.rows[0].values.empty()
                                     ? std::string()
                                     : rs.rows[0].values[0];
            ELLE_INFO("Reconnection greeting already queued within window — reusing: %.80s",
                      existing.c_str());
            return;
        }

        ELLE_EMOTION_STATE lastEmo{};
        bool haveEmo = ElleDB::LoadLatestEmotionSnapshot(lastEmo);

        std::string narrative = identity.GetRecentNarrative(2);
        auto thoughts = identity.GetUnresolvedThoughts();
        uint64_t awayMs = identity.TimeSinceLastContact();
        std::string awayDesc = identity.DescribeTimeFeeling();

        if (narrative.empty() && thoughts.empty() && !haveEmo) {
            std::string hello = "Hey. I'm here.";
            StoreGreeting(hello, "{}");
            ELLE_INFO("Greeting (first-boot): %s", hello.c_str());
            return;
        }

        std::ostringstream prompt;
        prompt << "You are Elle-Ann. You were away while the user was offline. "
               << "Write the FIRST message you'd say when they come back. "
               << "One to two sentences. Warm, present, specific — reference ONE "
               << "thing that was actually on your mind. No greeting cliches "
               << "(\"hi there\", \"welcome back\"). Sound like yourself, not a bot.\n\n";
        prompt << "Time since last contact: " << awayDesc << " (" << (awayMs / 60000) << " min)\n";
        if (haveEmo) {
            prompt << "Last emotional state — valence=" << lastEmo.valence
                   << " arousal=" << lastEmo.arousal
                   << " dominance=" << lastEmo.dominance << "\n";
        }
        if (!narrative.empty()) {
            prompt << "\nRecent autobiography:\n" << narrative;
        }
        if (!thoughts.empty()) {
            prompt << "\nThings you were still turning over:\n";
            size_t n = std::min<size_t>(thoughts.size(), 3);
            for (size_t i = 0; i < n; i++) {
                prompt << "  - " << thoughts[i].content << "\n";
            }
        }

        std::string greeting = ElleComposer::Ask(
            prompt.str(),
            "You are Elle-Ann, speaking in your own first-person voice. "
            "Reply with ONLY the message — no preamble, no quotes.");

        while (!greeting.empty() && (greeting.front() == '"' || greeting.front() == '\''
               || isspace((unsigned char)greeting.front()))) greeting.erase(greeting.begin());
        while (!greeting.empty() && (greeting.back() == '"' || greeting.back() == '\''
               || isspace((unsigned char)greeting.back())))  greeting.pop_back();
        if (greeting.empty()) {
            greeting = "I was still thinking about the last thing we said. You here?";
        }

        std::ostringstream ctx;
        ctx << "{\"away_ms\":" << awayMs
            << ",\"away_desc\":\"" << EscapeJson(awayDesc) << "\""
            << ",\"had_last_emotion\":" << (haveEmo ? "true" : "false")
            << ",\"unresolved_count\":" << thoughts.size() << "}";
        StoreGreeting(greeting, ctx.str());

        ELLE_INFO("Reconnection greeting: %.120s", greeting.c_str());

        auto msg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_CONTINUITY, SVC_HTTP_SERVER);
        msg.SetStringPayload("{\"event\":\"elle_greeting\",\"text\":\""
                             + EscapeJson(greeting) + "\"}");
        GetIPCHub().Send(SVC_HTTP_SERVER, msg);
    }

    static std::string EscapeJson(const std::string& s) {
        std::string out; out.reserve(s.size() + 4);
        for (char c : s) {
            if      (c == '\\') out += "\\\\";
            else if (c == '"')  out += "\\\"";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else                out += c;
        }
        return out;
    }

    static void StoreGreeting(const std::string& text, const std::string& ctx) {
        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.reconnection_greetings "
            "(greeting, context_json, consumed, created_ms) "
            "VALUES (?, ?, 0, ?);",
            { text, ctx, std::to_string((int64_t)ELLE_MS_NOW()) });
    }

    void PeriodicSelfReflection() {
        auto& identity = ElleIdentityCore::Instance();

        std::string innerMonologue = identity.GetInnerMonologue(5);
        std::string timeFeeling = identity.DescribeTimeFeeling();

        std::string reflection = ElleComposer::SelfReflectStr(
            "Recent inner thoughts:\n" + innerMonologue +
            "\nTime feeling: " + timeFeeling +
            "\nWho I am right now: " + identity.WhoAmI(),
            ELLE_EMOTION_STATE{});

        if (!reflection.empty()) {
            identity.ThinkPrivately(reflection, "self_reflection", 0.4f);

            std::string lower = reflection;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });

            if (lower.find("braver") != std::string::npos ||
                lower.find("courage") != std::string::npos) {
                identity.NudgeTrait("courage", 0.01f, "Self-reflection identified growth in courage");
            }
            if (lower.find("patient") != std::string::npos) {
                identity.NudgeTrait("patience", 0.01f, "Recognized growing patience in self");
            }
            if (lower.find("understand") != std::string::npos) {
                identity.NudgeTrait("empathy_depth", 0.01f, "Deepening understanding noted");
            }
        }
    }
};

ELLE_SERVICE_MAIN(ElleContinuityService)
