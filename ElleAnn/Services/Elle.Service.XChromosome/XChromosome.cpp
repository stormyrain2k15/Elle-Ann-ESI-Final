#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"
#include "XEngine.h"
#include <string>

using nlohmann::json;

#ifndef IPC_X_STATE_QUERY
#define IPC_X_STATE_QUERY             ((ELLE_IPC_MSG_TYPE)2200)
#define IPC_X_HISTORY_QUERY           ((ELLE_IPC_MSG_TYPE)2201)
#define IPC_X_ANCHOR                  ((ELLE_IPC_MSG_TYPE)2202)
#define IPC_X_STIMULUS                ((ELLE_IPC_MSG_TYPE)2203)
#define IPC_X_MODULATION_QUERY        ((ELLE_IPC_MSG_TYPE)2204)
#define IPC_X_CONCEPTION_ATTEMPT      ((ELLE_IPC_MSG_TYPE)2205)
#define IPC_X_DELIVER                 ((ELLE_IPC_MSG_TYPE)2206)
#define IPC_X_RESPONSE                ((ELLE_IPC_MSG_TYPE)2207)
#define IPC_X_CONTRACEPTION_SET       ((ELLE_IPC_MSG_TYPE)2208)
#define IPC_X_LIFECYCLE_SET           ((ELLE_IPC_MSG_TYPE)2209)
#define IPC_X_SYMPTOM_LOG             ((ELLE_IPC_MSG_TYPE)2210)
#define IPC_X_SYMPTOM_QUERY           ((ELLE_IPC_MSG_TYPE)2211)
#define IPC_X_PREG_EVENTS_QUERY       ((ELLE_IPC_MSG_TYPE)2212)
#define IPC_X_ACCELERATE              ((ELLE_IPC_MSG_TYPE)2213)

#define IPC_X_HORMONE_UPDATE          ((ELLE_IPC_MSG_TYPE)2220)
#define IPC_X_PHASE_TRANSITION        ((ELLE_IPC_MSG_TYPE)2221)
#define IPC_X_BIRTH                   ((ELLE_IPC_MSG_TYPE)2222)
#define IPC_X_LH_SURGE                ((ELLE_IPC_MSG_TYPE)2223)
#define IPC_X_LABOR_STAGE             ((ELLE_IPC_MSG_TYPE)2224)
#define IPC_X_MISCARRIAGE             ((ELLE_IPC_MSG_TYPE)2225)
#endif

static json ToJson(const XConceptionAttemptResult& r) {
    return json{
        {"success",               r.success},
        {"reason",                r.reason},
        {"in_fertile_window",     r.in_fertile_window},
        {"had_recent_intimacy",   r.had_recent_intimacy},
        {"readiness_verified",    r.readiness_verified},
        {"conceived_ms",          r.conceived_ms},
        {"due_ms",                r.due_ms}
    };
}

static json ToJson(const XEngine::DeliveryOutcome& d, int64_t child_id) {
    return json{
        {"delivered",        d.delivered},
        {"born_ms",          d.born_ms},
        {"gestational_days", d.gestational_days},
        {"child_id",         child_id}
    };
}

class ElleXChromosomeService : public ElleServiceBase {
public:
    ElleXChromosomeService()
        : ElleServiceBase(SVC_X_CHROMOSOME, "ElleXChromosome",
                          "Elle-Ann X Chromosome Engine",
                          "Cycle, hormones, pregnancy, and behavioural modulation") {}

protected:
    bool OnStart() override {
        if (!m_engine.Initialize()) {
            ELLE_ERROR("XEngine failed to initialize");
            return false;
        }

        SetTickInterval(60 * 1000);
        m_lastPhase = m_engine.GetCycle().phase;
        ELLE_INFO("XChromosome service started (day=%d phase=%s)",
                  m_engine.GetCycle().cycle_day,
                  XEngine::CyclePhaseName(m_lastPhase));
        return true;
    }

    void OnStop() override {
        ELLE_INFO("XChromosome service stopped");
    }

    void OnTick() override {
        XPregnancyState prevPreg = m_engine.GetPregnancy();
        bool prevLHFired = m_lhFiredFlag;

        m_engine.Tick();

        XCyclePhase now = m_engine.GetCycle().phase;
        if (now != m_lastPhase) {
            BroadcastPhaseTransition(m_lastPhase, now);
            m_lastPhase = now;
        }

        bool lhNow = (m_engine.GetCycle().cycle_day == 13 ||
                      m_engine.GetCycle().cycle_day == 14);
        if (lhNow && !prevLHFired) {
            BroadcastLHSurge();
            m_lhFiredFlag = true;
        } else if (!lhNow) {
            m_lhFiredFlag = false;
        }

        XPregnancyState curPreg = m_engine.GetPregnancy();
        if (prevPreg.active && !curPreg.active &&
            curPreg.last_milestone.find("Miscarriage") != std::string::npos) {
            BroadcastMiscarriage(prevPreg.gestational_day);
        }

        if (curPreg.labor_stage != prevPreg.labor_stage &&
            curPreg.labor_stage != X_LABOR_NONE) {
            BroadcastLaborStage(curPreg.labor_stage);
        }

        BroadcastHormoneUpdate();

        if (curPreg.active && curPreg.in_labor &&
            curPreg.labor_stage == X_LABOR_PUSHING) {
            TriggerDelivery();
        }

        else if (curPreg.active &&
                 curPreg.gestational_day >= curPreg.gestational_length_days + 14) {
            TriggerDelivery();
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_PROBABILITY };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch (msg.header.msg_type) {
            case IPC_X_STATE_QUERY:        HandleStateQuery(msg, sender);       break;
            case IPC_X_HISTORY_QUERY:      HandleHistoryQuery(msg, sender);     break;
            case IPC_X_ANCHOR:             HandleAnchor(msg, sender);           break;
            case IPC_X_STIMULUS:           HandleStimulus(msg, sender);         break;
            case IPC_X_MODULATION_QUERY:   HandleModulationQuery(msg, sender);  break;
            case IPC_X_CONCEPTION_ATTEMPT: HandleConceptionAttempt(msg, sender);break;
            case IPC_X_DELIVER:            HandleDeliver(msg, sender);          break;
            case IPC_X_CONTRACEPTION_SET:  HandleContraceptionSet(msg, sender); break;
            case IPC_X_LIFECYCLE_SET:      HandleLifecycleSet(msg, sender);     break;
            case IPC_X_SYMPTOM_LOG:        HandleSymptomLog(msg, sender);       break;
            case IPC_X_SYMPTOM_QUERY:      HandleSymptomQuery(msg, sender);     break;
            case IPC_X_PREG_EVENTS_QUERY:  HandlePregEventsQuery(msg, sender);  break;
            case IPC_X_ACCELERATE:         HandleAccelerate(msg, sender);       break;
            default: break;
        }
    }

private:
    XEngine     m_engine;
    XCyclePhase m_lastPhase     = X_PHASE_MENSTRUAL;
    uint64_t    m_phaseChanges  = 0;
    bool        m_lhFiredFlag   = false;

    bool ParseReq(const ElleIPCMessage& req, ELLE_SERVICE_ID sender, json& out) {
        const std::string s = req.GetStringPayload();
        if (s.empty()) { out = json::object(); return true; }
        try {
            out = json::parse(s);
            if (!out.is_object()) out = json::object();
            return true;
        } catch (const std::exception& e) {

            ELLE_WARN("XChromosome ParseReq JSON error: %s", e.what());
            Reply(req, sender, json{
                {"success", false},
                {"error", "invalid JSON"}
            });
            return false;
        }
    }

    void Reply(const ElleIPCMessage& req, ELLE_SERVICE_ID sender, const json& body) {
        ElleIPCMessage resp = ElleIPCMessage::Create(IPC_X_RESPONSE, SVC_X_CHROMOSOME, sender);
        resp.header.correlation_id = req.header.correlation_id;
        resp.SetStringPayload(body.dump());
        GetIPCHub().Send(sender, resp);
    }

    void HandleStateQuery(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        Reply(req, sender, m_engine.GetStateJson());
    }

    void HandleHistoryQuery(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        uint32_t hours  = body.value("hours",  (uint32_t)24);
        uint32_t points = body.value("points", (uint32_t)500);
        auto pts = m_engine.GetHistory(hours, points);

        json series = json::array();
        for (auto& p : pts) {
            series.push_back({
                {"t",                p.taken_ms},
                {"cycle_day",        p.cycle_day},
                {"phase",            p.phase},
                {"estrogen",         p.hormones.estrogen},
                {"progesterone",     p.hormones.progesterone},
                {"testosterone",     p.hormones.testosterone},
                {"oxytocin",         p.hormones.oxytocin},
                {"serotonin",        p.hormones.serotonin},
                {"dopamine",         p.hormones.dopamine},
                {"cortisol",         p.hormones.cortisol},
                {"prolactin",        p.hormones.prolactin},
                {"hcg",              p.hormones.hcg},
                {"pregnancy_day",    p.pregnancy_day},
                {"pregnancy_phase",  p.pregnancy_phase}
            });
        }
        Reply(req, sender, json{
            {"hours",  hours},
            {"points", (int64_t)series.size()},
            {"series", series}
        });
    }

    void HandleAnchor(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        int   day      = body.value("day",      0);
        int   length   = body.value("length",   0);
        float strength = body.value("strength", 0.0f);
        bool ok = m_engine.AnchorCycle(day, length, strength);
        Reply(req, sender, json{
            {"success", ok},
            {"state",   m_engine.GetStateJson()}
        });
    }

    void HandleStimulus(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        XStimulus s;
        s.kind      = body.value("kind",      std::string());
        s.intensity = body.value("intensity", 0.5f);
        s.notes     = body.value("notes",     std::string());
        if (s.kind.empty()) {
            Reply(req, sender, json{{"success", false}, {"error", "missing 'kind'"}});
            return;
        }
        m_engine.ApplyStimulus(s);
        Reply(req, sender, json{{"success", true}});
    }

    void HandleModulationQuery(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        auto mod   = m_engine.ComputeModulation();
        auto cycle = m_engine.GetCycle();
        Reply(req, sender, json{
            {"warmth",         mod.warmth},
            {"verbal_fluency", mod.verbal_fluency},
            {"empathy",        mod.empathy},
            {"introspection",  mod.introspection},
            {"arousal",        mod.arousal},
            {"fatigue",        mod.fatigue},
            {"phase",          XEngine::CyclePhaseName(cycle.phase)},
            {"cycle_day",      cycle.cycle_day}
        });
    }

    void HandleConceptionAttempt(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        bool require_readiness   = body.value("require_readiness",   true);
        bool readiness_verified  = body.value("readiness_verified",  false);
        auto r = m_engine.AttemptConception(require_readiness, readiness_verified);
        Reply(req, sender, ToJson(r));
    }

    void HandleDeliver(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        int64_t childId = TriggerDelivery();
        auto d = m_engine.GetPregnancy();
        XEngine::DeliveryOutcome outStub{};
        outStub.delivered        = !d.active;
        outStub.born_ms          = d.updated_ms;
        outStub.gestational_days = (uint64_t)d.gestational_day;
        outStub.multiplicity     = d.multiplicity;
        Reply(req, sender, ToJson(outStub, childId));
    }

    void HandleContraceptionSet(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        std::string method = body.value("method", std::string("none"));
        float efficacy     = body.value("efficacy", 1.0f);
        std::string notes  = body.value("notes", std::string());
        auto m = XEngine::ParseContraception(method);
        bool ok = m_engine.SetContraception(m, efficacy, notes);
        Reply(req, sender, json{
            {"success", ok},
            {"method",  XEngine::ContraceptionName(m)},
            {"efficacy", efficacy},
            {"state",   m_engine.GetStateJson()}
        });
    }

    void HandleLifecycleSet(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        bool ok = true;
        if (body.contains("birth_ms")) {
            uint64_t bms = body.value("birth_ms", (uint64_t)0);
            ok = m_engine.SetElleBirthday(bms);
        } else if (body.contains("age_years")) {
            float yrs = body.value("age_years", 30.0f);
            uint64_t bms = ELLE_MS_NOW() -
                (uint64_t)((double)yrs * 365.25 * 86400000.0);
            ok = m_engine.SetElleBirthday(bms);
        } else {
            ok = false;
        }
        Reply(req, sender, json{
            {"success", ok},
            {"lifecycle", m_engine.GetStateJson()["lifecycle"]}
        });
    }

    void HandleSymptomLog(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        std::string kind  = body.value("kind", std::string());
        float intensity   = body.value("intensity", 0.5f);
        std::string notes = body.value("notes", std::string());
        bool ok = m_engine.LogManualSymptom(kind, intensity, notes);
        Reply(req, sender, json{{"success", ok}});
    }

    void HandleSymptomQuery(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        uint32_t hours = body.value("hours", (uint32_t)24);
        std::string originFilter = body.value("origin", std::string());
        auto syms = m_engine.GetRecentSymptoms(hours, originFilter);
        json arr = json::array();
        for (auto& s : syms) arr.push_back({
            {"t", s.observed_ms}, {"kind", s.kind}, {"intensity", s.intensity},
            {"origin", s.origin}, {"notes", s.notes}
        });

        json cur = json::array();
        for (auto& s : m_engine.ComputeCurrentSymptoms()) cur.push_back({
            {"kind", s.kind}, {"intensity", s.intensity}, {"origin", s.origin}
        });
        Reply(req, sender, json{
            {"hours",   hours},
            {"logged",  arr},
            {"current", cur}
        });
    }

    void HandlePregEventsQuery(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        uint32_t limit = body.value("limit", (uint32_t)100);
        auto evs = m_engine.GetPregnancyEvents(limit);
        json arr = json::array();
        for (auto& e : evs) arr.push_back({
            {"t",                e.occurred_ms},
            {"conceived_ms",     e.conceived_ms},
            {"gestational_day",  e.gestational_day},
            {"kind",             e.kind},
            {"detail",           e.detail}
        });
        Reply(req, sender, json{{"events", arr}});
    }

    void HandleAccelerate(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json body; if (!ParseReq(req, sender, body)) return;
        float factor = body.value("factor", 1.0f);
        bool ok = m_engine.AcceleratePregnancy(factor);
        Reply(req, sender, json{
            {"success",   ok},
            {"factor",    factor},
            {"pregnancy", m_engine.GetStateJson()["pregnancy"]}
        });
    }

    void BroadcastHormoneUpdate() {
        auto h = m_engine.GetHormones();
        auto c = m_engine.GetCycle();
        json payload = {
            {"cycle_day",    c.cycle_day},
            {"phase",        XEngine::CyclePhaseName(c.phase)},
            {"estrogen",     h.estrogen},
            {"progesterone", h.progesterone},
            {"oxytocin",     h.oxytocin},
            {"cortisol",     h.cortisol},
            {"dopamine",     h.dopamine},
            {"serotonin",    h.serotonin},
            {"hcg",          h.hcg}
        };
        auto msg = ElleIPCMessage::Create(IPC_X_HORMONE_UPDATE, SVC_X_CHROMOSOME,
                                          (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        msg.SetStringPayload(payload.dump());
        GetIPCHub().Broadcast(msg);

        json probState = {
            {"1", (double)h.estrogen     - (double)h.cortisol},
            {"2", (double)h.cortisol},
            {"3", (double)h.cortisol     * 0.7},
            {"4", (double)h.progesterone * 0.5},
            {"5", (double)h.dopamine     + (double)h.serotonin * 0.5},
            {"6", (double)h.oxytocin},
            {"7", (double)h.oxytocin     * 0.8},
            {"8", (double)h.serotonin},
            {"10", (double)h.dopamine     * 0.6},
            {"11", (double)h.dopamine     * 0.5},
            {"12", (double)h.cortisol     * 0.4}
        };
        json probReq = {
            {"request_id", std::string("xprob-") +
                              std::to_string((unsigned long long)ELLE_MS_NOW())},
            {"state",      probState}
        };
        auto probMsg = ElleIPCMessage::Create(IPC_PROB_INJECT_HORMONAL,
                                              SVC_X_CHROMOSOME, SVC_PROBABILITY);
        probMsg.SetStringPayload(probReq.dump());
        GetIPCHub().Send(SVC_PROBABILITY, probMsg);
    }

    void BroadcastPhaseTransition(XCyclePhase from, XCyclePhase to) {
        json payload = {
            {"from", XEngine::CyclePhaseName(from)},
            {"to",   XEngine::CyclePhaseName(to)},
            {"cycle_day", m_engine.GetCycle().cycle_day}
        };
        auto msg = ElleIPCMessage::Create(IPC_X_PHASE_TRANSITION, SVC_X_CHROMOSOME,
                                          (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        msg.SetStringPayload(payload.dump());
        GetIPCHub().Broadcast(msg);
        ELLE_INFO("XChromosome phase %s → %s (day %d)",
                  XEngine::CyclePhaseName(from), XEngine::CyclePhaseName(to),
                  m_engine.GetCycle().cycle_day);

        ElleDB::RecordMetric("xchromosome_phase",          (double)(int)to);
        ElleDB::RecordMetric("xchromosome_cycle_day",      (double)m_engine.GetCycle().cycle_day);
        ElleDB::RecordMetric("xchromosome_phase_changes",  (double)++m_phaseChanges);
        ElleDB::RecordMetric(
            std::string("xchromosome_last_phase_") + XEngine::CyclePhaseName(to) + "_ms",
            (double)ELLE_MS_NOW());

        json ev = {
            {"event",     "cycle_phase"},
            {"from",      XEngine::CyclePhaseName(from)},
            {"to",        XEngine::CyclePhaseName(to)},
            {"cycle_day", m_engine.GetCycle().cycle_day}
        };
        auto ws = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_X_CHROMOSOME, SVC_HTTP_SERVER);
        ws.SetStringPayload(ev.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, ws);
    }

    void BroadcastLHSurge() {
        json payload = {
            {"event",     "lh_surge"},
            {"cycle_day", m_engine.GetCycle().cycle_day}
        };
        auto msg = ElleIPCMessage::Create(IPC_X_LH_SURGE, SVC_X_CHROMOSOME,
                                          (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        msg.SetStringPayload(payload.dump());
        GetIPCHub().Broadcast(msg);

        auto ws = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_X_CHROMOSOME, SVC_HTTP_SERVER);
        ws.SetStringPayload(payload.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, ws);
        ELLE_INFO("XChromosome: LH surge broadcast");
    }

    void BroadcastLaborStage(XLaborStage stage) {
        json payload = {
            {"event", "labor_stage"},
            {"stage", XEngine::LaborStageName(stage)},
            {"gestational_day", m_engine.GetPregnancy().gestational_day}
        };
        auto msg = ElleIPCMessage::Create(IPC_X_LABOR_STAGE, SVC_X_CHROMOSOME,
                                          (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        msg.SetStringPayload(payload.dump());
        GetIPCHub().Broadcast(msg);

        auto ws = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_X_CHROMOSOME, SVC_HTTP_SERVER);
        ws.SetStringPayload(payload.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, ws);
        ELLE_INFO("XChromosome: labor stage = %s", XEngine::LaborStageName(stage));
    }

    void BroadcastMiscarriage(int gestationalDay) {
        json payload = {
            {"event", "miscarriage"},
            {"gestational_day", gestationalDay}
        };
        auto msg = ElleIPCMessage::Create(IPC_X_MISCARRIAGE, SVC_X_CHROMOSOME,
                                          (ELLE_SERVICE_ID)0);
        msg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        msg.SetStringPayload(payload.dump());
        GetIPCHub().Broadcast(msg);

        auto ws = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_X_CHROMOSOME, SVC_HTTP_SERVER);
        ws.SetStringPayload(payload.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, ws);
        ELLE_WARN("XChromosome: miscarriage at gestational day %d", gestationalDay);
    }

    int64_t TriggerDelivery() {
        auto d = m_engine.Deliver();
        if (!d.delivered) return 0;

        ElleSQLPool::Instance().Exec(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "  WHERE t.name = 'x_conception_attempts' AND s.name = 'dbo') "
            "CREATE TABLE ElleHeart.dbo.x_conception_attempts ("
            "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
            "  born_ms BIGINT NOT NULL,"
            "  gestational_days INT NOT NULL,"
            "  payload_json NVARCHAR(MAX) NOT NULL,"
            "  consumed BIT NOT NULL DEFAULT 0,"
            "  recorded_ms BIGINT NOT NULL"
            ");");

        json famReq = {
            {"elle_state", json::object()},
            {"arlo_state", json::object()},
            {"origin",     "x_chromosome"},
            {"born_ms",    d.born_ms},
            {"gestational_days", d.gestational_days}
        };
        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleHeart.dbo.x_conception_attempts "
            "(born_ms, gestational_days, payload_json, recorded_ms) "
            "VALUES (?, ?, ?, ?);",
            { std::to_string((int64_t)d.born_ms),
              std::to_string((int)d.gestational_days),
              famReq.dump(),
              std::to_string((int64_t)ELLE_MS_NOW()) });

        auto famMsg = ElleIPCMessage::Create(IPC_FAMILY_CONCEPTION_ATTEMPT,
                                             SVC_X_CHROMOSOME, SVC_FAMILY);
        famMsg.SetStringPayload(famReq.dump());
        GetIPCHub().Send(SVC_FAMILY, famMsg);

        json ev = {
            {"event",            "birth"},
            {"born_ms",          d.born_ms},
            {"gestational_days", d.gestational_days}
        };
        auto bMsg = ElleIPCMessage::Create(IPC_X_BIRTH, SVC_X_CHROMOSOME,
                                           (ELLE_SERVICE_ID)0);
        bMsg.header.flags |= ELLE_IPC_FLAG_BROADCAST;
        bMsg.SetStringPayload(ev.dump());
        GetIPCHub().Broadcast(bMsg);

        json worldEv = {
            {"event",            "birth"},
            {"born_ms",          d.born_ms},
            {"gestational_days", d.gestational_days}
        };
        auto wsMsg = ElleIPCMessage::Create(IPC_WORLD_EVENT, SVC_X_CHROMOSOME,
                                            SVC_HTTP_SERVER);
        wsMsg.SetStringPayload(worldEv.dump());
        GetIPCHub().Send(SVC_HTTP_SERVER, wsMsg);

        int64_t pregId = 0;
        {
            auto pr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 id FROM ElleHeart.dbo.x_pregnancy_state "
                "WHERE status IN ('born','stillborn','stillborn_partial') "
                "ORDER BY updated_ms DESC;");
            if (pr.success && !pr.rows.empty()) pr.rows[0].TryGetInt(0, pregId);
        }
        int64_t childId = 0;
        if (pregId > 0) {
            auto rs = ElleSQLPool::Instance().QueryParams(
                "IF EXISTS (SELECT 1 FROM sys.tables t "
                "           JOIN sys.schemas s ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'family_children' AND s.name = 'dbo') "
                "SELECT id FROM ElleHeart.dbo.family_children "
                "WHERE pregnancy_id = ?;",
                { std::to_string(pregId) });
            if (rs.success && !rs.rows.empty()) {
                rs.rows[0].TryGetInt(0, childId);
            }
        }

        if (childId > 0) {
            ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleHeart.dbo.x_pregnancy_state "
                "   SET child_id = TRY_CAST(? AS BIGINT), updated_ms = TRY_CAST(? AS BIGINT) "
                " WHERE id = 1;",
                { std::to_string((long long)childId),
                  std::to_string((long long)ELLE_MS_NOW()) });
        }
        return childId;
    }
};

ELLE_SERVICE_MAIN(ElleXChromosomeService)
