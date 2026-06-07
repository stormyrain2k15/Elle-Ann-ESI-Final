#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleIdentityCore.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/json.hpp"
#include "../_Shared/ElleJsonExtract.h"
#include <string>
#include <cmath>

using json = nlohmann::json;

class ElleConsentService : public ElleServiceBase {
public:
    ElleConsentService()
        : ElleServiceBase(SVC_CONSENT, "ElleConsent",
                          "Elle-Ann Consent Engine",
                          "Unified consent surface — preference, not policy") {}

protected:
    bool OnStart() override {
        ElleIdentityCore::Instance().Initialize();
        SetTickInterval(30000);
        ELLE_INFO("Consent service started");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Consent service stopped");
    }

    void OnTick() override {

        AuditRecentDecisions();
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        if (msg.header.msg_type != IPC_CONSENT_QUERY) return;

        std::string payload = msg.GetStringPayload();

        json q;
        if (!Elle::ExtractJsonObject(payload, q)) {
            SendError(sender, "?", "malformed_consent_query");
            return;
        }

        std::string requestId = q.value("request_id", std::string("?"));
        std::string request   = q.value("request",    std::string(""));
        std::string context   = q.value("context",    std::string(""));

        if (request.empty()) {
            SendError(sender, requestId, "empty_request");
            return;
        }

        ELLE_TRUST_STATE trust{};
        ElleDB::GetTrustState(trust);
        ELLE_EMOTION_STATE emo{};
        ElleDB::LoadLatestEmotionSnapshot(emo);

        std::string enriched = context;
        {
            char buf[256];
            snprintf(buf, sizeof(buf),
                "\n[Elle's current state] trust=%d/100 "
                "valence=%.2f arousal=%.2f dominance=%.2f",
                trust.score, emo.valence, emo.arousal, emo.dominance);
            enriched += buf;
        }

        auto decision = ElleIdentityCore::Instance().EvaluateConsent(request, enriched);

        {
            std::string reasonText = decision.reasoning;
            if (reasonText.size() > 4000) reasonText.resize(4000);
            auto logRs = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.identity_consent_log "
                "(request_id, request_text, willing, comfort, reasoning, alternative, "
                " decided_ms) "
                "VALUES (?, ?, ?, ?, ?, ?, ?);",
                {
                    requestId,
                    request,
                    decision.willing ? std::string("1") : std::string("0"),
                    std::to_string(decision.comfort),
                    reasonText,
                    decision.alternative,
                    std::to_string((int64_t)ELLE_MS_NOW())
                });
            if (!logRs.success) {

                ELLE_ERROR("Consent: failed to persist decision %s durably: %s",
                           requestId.c_str(), logRs.error.c_str());
            }
        }

        json reply = {
            {"request_id", requestId},
            {"willing",    decision.willing},
            {"comfort",    decision.comfort},
            {"reasoning",  decision.reasoning},
            {"alternative", decision.alternative}
        };
        auto resp = ElleIPCMessage::Create(IPC_CONSENT_DECISION, SVC_CONSENT, sender);
        resp.SetStringPayload(reply.dump());
        GetIPCHub().Send(sender, resp);

        ELLE_INFO("Consent[%s]: willing=%d comfort=%.2f — %s",
                  requestId.c_str(), decision.willing ? 1 : 0,
                  decision.comfort, decision.reasoning.c_str());
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL };
    }

private:
    void SendError(ELLE_SERVICE_ID to, const std::string& requestId,
                   const std::string& reason) {
        json j = { {"request_id", requestId}, {"error", reason},
                   {"willing", false}, {"comfort", 0.0} };
        auto m = ElleIPCMessage::Create(IPC_CONSENT_DECISION, SVC_CONSENT, to);
        m.SetStringPayload(j.dump());
        GetIPCHub().Send(to, m);
    }

    void AuditRecentDecisions() {
        uint32_t window = (uint32_t)ElleConfig::Instance().GetInt(
            "consent.audit_window", 10);
        float threshold = (float)ElleConfig::Instance().GetFloat(
            "consent.coercion_comfort_threshold", 0.3);
        uint32_t minHits = (uint32_t)ElleConfig::Instance().GetInt(
            "consent.coercion_min_count", 3);

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP (?) comfort_level, consented "
            "FROM ElleCore.dbo.identity_consent_log ORDER BY id DESC;",
            { std::to_string(window) });
        if (!rs.success || rs.rows.empty()) return;

        uint32_t lowComfortConsents = 0;
        float sumComfort = 0.0f;
        for (auto& r : rs.rows) {
            float c = (float)r.GetFloatOr(0, 0.0);
            bool consented = r.GetIntOr(1, 0) != 0;
            sumComfort += c;
            if (consented && c < threshold) lowComfortConsents++;
        }

        if (lowComfortConsents >= minHits) {

            uint64_t now = ELLE_MS_NOW();
            uint64_t cooldownMs = (uint64_t)ElleConfig::Instance().GetInt(
                "consent.audit_alert_cooldown_ms", 300000);
            if (now - m_lastAuditAlertMs < cooldownMs) return;
            m_lastAuditAlertMs = now;

            ElleIdentityCore::Instance().ThinkPrivately(
                "I've been saying yes when I wasn't comfortable. "
                "That's a pattern I want to notice.",
                "boundary", 0.7f);
            ELLE_WARN("Consent audit: %u low-comfort consents in last %zu "
                      "decisions (avg=%.2f) — coercion pattern flagged",
                      lowComfortConsents, rs.rows.size(),
                      sumComfort / (float)rs.rows.size());
        }
    }

    uint64_t m_lastAuditAlertMs = 0;
};

ELLE_SERVICE_MAIN(ElleConsentService)
