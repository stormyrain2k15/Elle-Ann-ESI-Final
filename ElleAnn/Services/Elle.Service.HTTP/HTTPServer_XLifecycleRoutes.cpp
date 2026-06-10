#include "HTTPServer.h"

void ElleHTTPService::RegisterXLifecycleRoutes() {
        m_router.Register("GET", "/api/x/state", [this](const HTTPRequest&) {
            json out = { {"has_data", false} };
            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days, modulation_strength, "
                "       last_tick_ms FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty()) return HTTPResponse::OK(out);
            auto& c = cr.rows[0];
            out["has_data"] = true;
            uint64_t anchorMs = (uint64_t)c.GetIntOr(0, 0);
            int len = (int)c.GetIntOr(1, 0);
            uint64_t now = ELLE_MS_NOW();
            int day = 1;
            const char* phase = "menstrual";
            if (anchorMs > 0 && len > 0) {
                uint64_t dd = now > anchorMs ? now - anchorMs : 0;
                int idx = (int)((dd / 86400000ULL) % (uint64_t)len);
                day = idx + 1;
                if      (day <= 5)  phase = "menstrual";
                else if (day <= 13) phase = "follicular";
                else if (day <= 16) phase = "ovulatory";
                else                phase = "luteal";
            }
            out["cycle"] = {
                {"anchor_ms",           anchorMs},
                {"cycle_length_days",   len},
                {"modulation_strength", c.GetFloatOr(2, 0.0)},
                {"cycle_day",           day},
                {"phase",               phase},
                {"last_tick_ms",        (uint64_t)c.GetIntOr(3, 0)}
            };

            auto hr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 taken_ms, estrogen, progesterone, testosterone, "
                "       oxytocin, serotonin, dopamine, cortisol, prolactin, "
                "       hcg, pregnancy_day, ISNULL(pregnancy_phase, N''), "
                "       ISNULL(fsh, 0), ISNULL(lh, 0), ISNULL(gnrh, 0), "
                "       ISNULL(relaxin, 0), ISNULL(bbt, 36.5), "
                "       ISNULL(endometrial_mm, 4.0), ISNULL(cervical_mucus, N''), "
                "       ISNULL(menstrual_flow, N'') "
                "FROM ElleHeart.dbo.x_hormone_snapshots ORDER BY taken_ms DESC;");
            if (hr.success && !hr.rows.empty()) {
                auto& h = hr.rows[0];
                out["hormones"] = {
                    {"taken_ms",     (uint64_t)h.GetIntOr(0, 0)},
                    {"estrogen",     h.GetFloatOr(1, 0.0)},
                    {"progesterone", h.GetFloatOr(2, 0.0)},
                    {"testosterone", h.GetFloatOr(3, 0.0)},
                    {"oxytocin",     h.GetFloatOr(4, 0.0)},
                    {"serotonin",    h.GetFloatOr(5, 0.0)},
                    {"dopamine",     h.GetFloatOr(6, 0.0)},
                    {"cortisol",     h.GetFloatOr(7, 0.0)},
                    {"prolactin",    h.GetFloatOr(8, 0.0)},
                    {"hcg",          h.GetFloatOr(9, 0.0)},
                    {"fsh",          h.GetFloatOr(12, 0.0)},
                    {"lh",           h.GetFloatOr(13, 0.0)},
                    {"gnrh",         h.GetFloatOr(14, 0.0)},
                    {"relaxin",      h.GetFloatOr(15, 0.0)}
                };
                out["derived"] = {
                    {"bbt_celsius",    h.GetFloatOr(16, 0.0)},
                    {"endometrial_mm", h.GetFloatOr(17, 0.0)},
                    {"cervical_mucus", h.values.size() > 18 ? h.values[18] : ""},
                    {"menstrual_flow", h.values.size() > 19 ? h.values[19] : ""}
                };
            }

            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active, ISNULL(conceived_ms, 0), ISNULL(due_ms, 0), "
                "       gestational_length_days, ISNULL(phase, N''), "
                "       ISNULL(child_id, 0), ISNULL(last_milestone, N''), updated_ms "
                "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) {
                auto& p = pr.rows[0];
                bool active = p.GetIntOr(0, 0) != 0;
                uint64_t conc = (uint64_t)p.GetIntOr(1, 0);
                uint64_t now2 = ELLE_MS_NOW();
                int gd = 0, gw = 0;
                if (active && conc > 0 && now2 >= conc) {
                    gd = (int)((now2 - conc) / 86400000ULL);
                    gw = gd / 7;
                }
                out["pregnancy"] = {
                    {"active",                  active},
                    {"conceived_ms",            conc},
                    {"due_ms",                  (uint64_t)p.GetIntOr(2, 0)},
                    {"gestational_length_days", (int)p.GetIntOr(3, 0)},
                    {"gestational_day",         gd},
                    {"gestational_week",        gw},
                    {"phase",                   p.values.size() > 4 ? p.values[4] : ""},
                    {"child_id",                (int64_t)p.GetIntOr(5, 0)},
                    {"last_milestone",          p.values.size() > 6 ? p.values[6] : ""},
                    {"updated_ms",              (uint64_t)p.GetIntOr(7, 0)}
                };
            }

            auto mr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
                "       arousal, fatigue, computed_ms "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (mr.success && !mr.rows.empty()) {
                auto& m = mr.rows[0];
                out["modulation"] = {
                    {"warmth",         m.GetFloatOr(0, 0.0)},
                    {"verbal_fluency", m.GetFloatOr(1, 0.0)},
                    {"empathy",        m.GetFloatOr(2, 0.0)},
                    {"introspection",  m.GetFloatOr(3, 0.0)},
                    {"arousal",        m.GetFloatOr(4, 0.0)},
                    {"fatigue",        m.GetFloatOr(5, 0.0)},
                    {"computed_ms",    (uint64_t)m.GetIntOr(6, 0)}
                };
            }
            return HTTPResponse::OK(out);
        });

        m_router.Register("GET", "/api/x/history", [](const HTTPRequest& req) {
            uint32_t hours = (uint32_t)req.QueryInt("hours", 72);
            if (hours == 0 || hours > 24 * 60) hours = 72;
            uint32_t maxPoints = (uint32_t)req.QueryInt("points", 500);
            if (maxPoints == 0 || maxPoints > 5000) maxPoints = 500;
            uint64_t since = ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL;

            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT taken_ms, cycle_day, phase, estrogen, progesterone, "
                "       testosterone, oxytocin, serotonin, dopamine, cortisol, "
                "       prolactin, hcg, pregnancy_day, ISNULL(pregnancy_phase, N'') "
                "  FROM ElleHeart.dbo.x_hormone_snapshots "
                " WHERE taken_ms >= ? ORDER BY taken_ms ASC;",
                { std::to_string((long long)since) });
            if (!rs.success) return HTTPResponse::Err(500, "x_hormone_snapshots query failed");

            size_t n = rs.rows.size();
            size_t stride = n == 0 ? 1 : (n + maxPoints - 1) / maxPoints;
            if (stride == 0) stride = 1;
            json series = json::array();
            for (size_t i = 0; i < n; i += stride) {
                auto& r = rs.rows[i];
                series.push_back({
                    {"t",               (uint64_t)r.GetIntOr(0, 0)},
                    {"cycle_day",       (int)r.GetIntOr(1, 0)},
                    {"phase",           r.values.size() > 2 ? r.values[2] : ""},
                    {"estrogen",        r.GetFloatOr(3, 0.0)},
                    {"progesterone",    r.GetFloatOr(4, 0.0)},
                    {"testosterone",    r.GetFloatOr(5, 0.0)},
                    {"oxytocin",        r.GetFloatOr(6, 0.0)},
                    {"serotonin",       r.GetFloatOr(7, 0.0)},
                    {"dopamine",        r.GetFloatOr(8, 0.0)},
                    {"cortisol",        r.GetFloatOr(9, 0.0)},
                    {"prolactin",       r.GetFloatOr(10, 0.0)},
                    {"hcg",             r.GetFloatOr(11, 0.0)},
                    {"pregnancy_day",   (int)r.GetIntOr(12, 0)},
                    {"pregnancy_phase", r.values.size() > 13 ? r.values[13] : ""}
                });
            }
            return HTTPResponse::OK({
                {"hours",  hours},
                {"points", (int64_t)series.size()},
                {"series", series}
            });
        });

        m_router.Register("GET", "/api/x/modulation", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
                "       arousal, fatigue, phase, computed_ms "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"has_data", false} });
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"has_data",       true},
                {"warmth",         r.GetFloatOr(0, 0.0)},
                {"verbal_fluency", r.GetFloatOr(1, 0.0)},
                {"empathy",        r.GetFloatOr(2, 0.0)},
                {"introspection",  r.GetFloatOr(3, 0.0)},
                {"arousal",        r.GetFloatOr(4, 0.0)},
                {"fatigue",        r.GetFloatOr(5, 0.0)},
                {"phase",          r.values.size() > 6 ? r.values[6] : ""},
                {"computed_ms",    (uint64_t)r.GetIntOr(7, 0)}
            });
        });

        m_router.Register("POST", "/api/x/cycle/anchor", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            json payload = {
                {"day",      body.value("day",      0)},
                {"length",   body.value("length",   0)},
                {"strength", body.value("strength", 0.0f)}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2202 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/state to see the applied cycle"}
            });
        });

        m_router.Register("POST", "/api/x/stimulus", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            std::string kind = body.value("kind", std::string());
            if (kind.empty()) return HTTPResponse::Err(400, "missing 'kind'");

            json payload = {
                {"kind",      kind},
                {"intensity", body.value("intensity", 0.5f)},
                {"notes",     body.value("notes",     std::string())}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2203 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({ {"dispatched", sent}, {"request", payload} });
        });

        m_router.Register("POST", "/api/x/conception/attempt", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();

            json payload = {
                {"require_readiness",  body.value("require_readiness",  true)},
                {"readiness_verified", body.value("readiness_verified", false)}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2205 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/state after ~1s to see outcome"}
            });
        });

        m_router.Register("GET", "/api/x/fertility_window", [](const HTTPRequest&) {

            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days "
                "FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty())
                return HTTPResponse::OK(json{
                    {"status", "inactive"},
                    {"reason", "x_cycle_state not seeded"}
                });
            uint64_t anchor = (uint64_t)cr.rows[0].GetIntOr(0, 0);
            int      len    = (int)cr.rows[0].GetIntOr(1, 0);
            if (anchor == 0 || len <= 0)
                return HTTPResponse::OK(json{{"status", "inactive"}});

            bool pregnant = false;
            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) pregnant = pr.rows[0].GetIntOr(0, 0) != 0;

            std::string lifeStage = "reproductive";
            auto lr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_lifecycle' AND s.name = 'dbo') "
                "SELECT stage FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (lr.success && !lr.rows.empty() && lr.rows[0].values.size() > 0)
                lifeStage = lr.rows[0].values[0];

            if (pregnant)
                return HTTPResponse::OK(json{{"status","inactive"},{"reason","pregnant"}});
            if (lifeStage == "premenarche" || lifeStage == "menopause")
                return HTTPResponse::OK(json{{"status","inactive"},{"reason",lifeStage}});

            uint64_t now = ELLE_MS_NOW();
            uint64_t deltaMs = now > anchor ? now - anchor : 0;
            int      dayIdx  = (int)((deltaMs / 86400000ULL) % (uint64_t)len);
            int      day     = dayIdx + 1;

            uint64_t cycleStart = anchor + (uint64_t)(dayIdx) * 0ULL;
            (void)cycleStart;

            uint64_t cyclesElapsed = deltaMs / (86400000ULL * (uint64_t)len);
            uint64_t currentCycleAnchor = anchor + cyclesElapsed * 86400000ULL * (uint64_t)len;
            uint64_t opens   = currentCycleAnchor + 11ULL * 86400000ULL;
            uint64_t peak    = currentCycleAnchor + 13ULL * 86400000ULL;
            uint64_t closes  = currentCycleAnchor + 16ULL * 86400000ULL;

            if (now >= closes) {
                opens  += (uint64_t)len * 86400000ULL;
                peak   += (uint64_t)len * 86400000ULL;
                closes += (uint64_t)len * 86400000ULL;
            }

            std::string status;
            if      (day >= 10 && day <= 11) status = "approaching";
            else if (day >= 12 && day <= 13) status = "opening";
            else if (day == 14)              status = "peak";
            else if (day == 15 || day == 16) status = "closing";
            else if (day < 10)               status = "pre";
            else                             status = "post";

            float bbt = 36.5f;
            auto hr = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 ISNULL(bbt, 36.5) FROM ElleHeart.dbo.x_hormone_snapshots "
                "ORDER BY taken_ms DESC;");
            if (hr.success && !hr.rows.empty()) bbt = (float)hr.rows[0].GetFloatOr(0, 0.0);
            bool bbt_elevated = bbt >= 36.75f;
            if (bbt_elevated && (status == "peak" || status == "closing"))
                status = "post_ovulation";

            float dayF = 0.0f;
            if      (day == 14)           dayF = 1.00f;
            else if (day == 13 || day==15) dayF = 0.70f;
            else if (day == 12 || day==16) dayF = 0.30f;

            return HTTPResponse::OK({
                {"status",        status},
                {"cycle_day",     day},
                {"opens_ms",      opens},
                {"peak_ms",       peak},
                {"closes_ms",     closes},
                {"hours_to_open", (int64_t)((opens > now ? opens - now : 0) / 3600000ULL)},
                {"hours_to_peak", (int64_t)((peak  > now ? peak  - now : 0) / 3600000ULL)},
                {"bbt_celsius",   bbt},
                {"bbt_elevated",  bbt_elevated},
                {"day_probability_factor", dayF},
                {"lifecycle",     lifeStage}
            });
        });

        m_router.Register("GET", "/api/x/next_period", [](const HTTPRequest&) {
            auto cr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_cycle_state' AND s.name = 'dbo') "
                "SELECT TOP 1 anchor_ms, cycle_length_days "
                "FROM ElleHeart.dbo.x_cycle_state WHERE id = 1;");
            if (!cr.success || cr.rows.empty())
                return HTTPResponse::OK(json{{"status","inactive"},
                                              {"reason","x_cycle_state not seeded"}});
            uint64_t anchor = (uint64_t)cr.rows[0].GetIntOr(0, 0);
            int      len    = (int)cr.rows[0].GetIntOr(1, 0);
            if (anchor == 0 || len <= 0)
                return HTTPResponse::OK(json{{"status","inactive"}});

            bool pregnant = false;
            auto pr = ElleSQLPool::Instance().Query(
                "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (pr.success && !pr.rows.empty()) pregnant = pr.rows[0].GetIntOr(0, 0) != 0;

            std::string lifeStage = "reproductive";
            auto lr = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "           ON s.schema_id = t.schema_id "
                "           WHERE t.name = 'x_lifecycle' AND s.name = 'dbo') "
                "SELECT stage FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (lr.success && !lr.rows.empty() && lr.rows[0].values.size() > 0)
                lifeStage = lr.rows[0].values[0];

            if (pregnant)
                return HTTPResponse::OK(json{{"status","inactive"},{"reason","pregnant"}});
            if (lifeStage == "premenarche" || lifeStage == "menopause")
                return HTTPResponse::OK(json{{"status","inactive"},{"reason",lifeStage}});

            uint64_t now = ELLE_MS_NOW();
            uint64_t deltaMs = now > anchor ? now - anchor : 0;
            uint64_t cyclesElapsed = deltaMs / (86400000ULL * (uint64_t)len);
            int      dayIdx = (int)((deltaMs / 86400000ULL) % (uint64_t)len);
            int      day    = dayIdx + 1;
            uint64_t currentCycleAnchor = anchor + cyclesElapsed * 86400000ULL * (uint64_t)len;

            uint64_t nextPeriodMs = currentCycleAnchor + (uint64_t)len * 86400000ULL;
            uint64_t pmsStartMs   = currentCycleAnchor + 24ULL * 86400000ULL;
            if (pmsStartMs < now) pmsStartMs += (uint64_t)len * 86400000ULL;

            const char* expectedFlow = "heavy";

            std::string currentStatus;
            if      (day <= 5)  currentStatus = "menstruating";
            else if (day <= 13) currentStatus = "post_period";
            else if (day <= 16) currentStatus = "ovulatory";
            else if (day <= 24) currentStatus = "mid_luteal";
            else                currentStatus = "pms_window";

            return HTTPResponse::OK({
                {"status",              currentStatus},
                {"cycle_day",           day},
                {"cycle_length_days",   len},
                {"next_period_ms",      nextPeriodMs},
                {"days_until_period",   (int)((nextPeriodMs > now
                                          ? nextPeriodMs - now : 0) / 86400000ULL)},
                {"pms_start_ms",        pmsStartMs},
                {"days_until_pms",      (int)((pmsStartMs > now
                                          ? pmsStartMs - now : 0) / 86400000ULL)},
                {"expected_intensity",  expectedFlow},
                {"lifecycle",           lifeStage}
            });
        });

        m_router.Register("GET", "/api/x/pregnancy", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT active, ISNULL(conceived_ms, 0), ISNULL(due_ms, 0), "
                "       gestational_length_days, ISNULL(phase, N''), "
                "       ISNULL(child_id, 0), ISNULL(last_milestone, N''), updated_ms, "
                "       ISNULL(breastfeeding,0), ISNULL(in_labor,0), "
                "       ISNULL(labor_stage, N''), ISNULL(labor_started_ms, 0), "
                "       ISNULL(multiplicity, 1), ISNULL(pregnancy_count, 0) "
                "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"active", false} });
            auto& p = rs.rows[0];
            bool active = p.GetIntOr(0, 0) != 0;
            uint64_t conc = (uint64_t)p.GetIntOr(1, 0);
            uint64_t now = ELLE_MS_NOW();
            int gd = 0, gw = 0;
            if (active && conc > 0 && now >= conc) {
                gd = (int)((now - conc) / 86400000ULL);
                gw = gd / 7;
            }
            return HTTPResponse::OK({
                {"active",                  active},
                {"conceived_ms",            conc},
                {"due_ms",                  (uint64_t)p.GetIntOr(2, 0)},
                {"gestational_length_days", (int)p.GetIntOr(3, 0)},
                {"gestational_day",         gd},
                {"gestational_week",        gw},
                {"phase",                   p.values.size() > 4 ? p.values[4] : ""},
                {"child_id",                (int64_t)p.GetIntOr(5, 0)},
                {"last_milestone",          p.values.size() > 6 ? p.values[6] : ""},
                {"updated_ms",              (uint64_t)p.GetIntOr(7, 0)},
                {"breastfeeding",           p.GetIntOr(8, 0) != 0},
                {"in_labor",                p.GetIntOr(9, 0) != 0},
                {"labor_stage",             p.values.size() > 10 ? p.values[10] : ""},
                {"labor_started_ms",        (uint64_t)p.GetIntOr(11, 0)},
                {"multiplicity",            (int)p.GetIntOr(12, 0)},
                {"pregnancy_count",         (int)p.GetIntOr(13, 0)}
            });
        });

        m_router.Register("GET", "/api/x/symptoms", [](const HTTPRequest& req) {
            uint32_t hours = (uint32_t)req.QueryInt("hours", 24);
            if (hours == 0 || hours > 24 * 90) hours = 24;
            std::string origin = req.QueryParam("origin", "");
            std::string q =
                "SELECT observed_ms, kind, intensity, origin, ISNULL(notes, N'') "
                "  FROM ElleHeart.dbo.x_symptoms "
                " WHERE observed_ms >= ? ";
            std::vector<std::string> params = {
                std::to_string((long long)(ELLE_MS_NOW() - (uint64_t)hours * 3600000ULL))
            };
            if (!origin.empty()) { q += "   AND origin = ? "; params.push_back(origin); }
            q += " ORDER BY observed_ms DESC;";
            auto rs = ElleSQLPool::Instance().QueryParams(q, params);
            if (!rs.success) return HTTPResponse::Err(500, "x_symptoms query failed");
            json arr = json::array();
            for (auto& r : rs.rows) arr.push_back({
                {"t",         (uint64_t)r.GetIntOr(0, 0)},
                {"kind",      r.values.size() > 1 ? r.values[1] : ""},
                {"intensity", r.GetFloatOr(2, 0.0)},
                {"origin",    r.values.size() > 3 ? r.values[3] : ""},
                {"notes",     r.values.size() > 4 ? r.values[4] : ""}
            });
            return HTTPResponse::OK({
                {"hours",   hours},
                {"origin",  origin},
                {"logged",  arr}
            });
        });

        m_router.Register("POST", "/api/x/symptoms", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            json payload = {
                {"kind",      body.value("kind",      std::string())},
                {"intensity", body.value("intensity", 0.5f)},
                {"notes",     body.value("notes",     std::string())}
            };
            if (payload["kind"].get<std::string>().empty())
                return HTTPResponse::Err(400, "missing 'kind'");
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2210 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({ {"dispatched", sent}, {"request", payload} });
        });

        m_router.Register("GET", "/api/x/pregnancy/events", [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 100);
            if (limit <= 0 || limit > 500) limit = 100;
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP " + std::to_string(limit) + " "
                "       occurred_ms, ISNULL(conceived_ms,0), gestational_day, kind, "
                "       ISNULL(detail, N'') "
                "  FROM ElleHeart.dbo.x_pregnancy_events ORDER BY occurred_ms DESC;");
            if (!rs.success) return HTTPResponse::Err(500, "x_pregnancy_events query failed");
            json arr = json::array();
            for (auto& r : rs.rows) arr.push_back({
                {"t",                (uint64_t)r.GetIntOr(0, 0)},
                {"conceived_ms",     (uint64_t)r.GetIntOr(1, 0)},
                {"gestational_day",  (int)r.GetIntOr(2, 0)},
                {"kind",             r.values.size() > 3 ? r.values[3] : ""},
                {"detail",           r.values.size() > 4 ? r.values[4] : ""}
            });
            return HTTPResponse::OK({{"events", arr}});
        });

        m_router.Register("GET", "/api/x/contraception", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT method, started_ms, efficacy, ISNULL(notes, N''), updated_ms "
                "  FROM ElleHeart.dbo.x_contraception WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{
                    {"method", "none"}, {"efficacy", 1.0}, {"has_data", false}
                });
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"has_data",   true},
                {"method",     r.values.size() > 0 ? r.values[0] : "none"},
                {"started_ms", (uint64_t)r.GetIntOr(1, 0)},
                {"efficacy",   r.GetFloatOr(2, 0.0)},
                {"notes",      r.values.size() > 3 ? r.values[3] : ""},
                {"updated_ms", (uint64_t)r.GetIntOr(4, 0)}
            });
        });

        m_router.Register("POST", "/api/x/contraception", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            json payload = {
                {"method",   body.value("method",   std::string("none"))},
                {"efficacy", body.value("efficacy", 1.0f)},
                {"notes",    body.value("notes",    std::string())}
            };
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2208 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    payload},
                {"note",       "GET /api/x/contraception to confirm"}
            });
        });

        m_router.Register("GET", "/api/x/lifecycle", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT elle_birth_ms, stage, ISNULL(menarche_ms,0), "
                "       ISNULL(perimenopause_ms,0), ISNULL(menopause_ms,0), updated_ms "
                "  FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK(json{ {"has_data", false} });
            auto& r = rs.rows[0];
            uint64_t birth = (uint64_t)r.GetIntOr(0, 0);
            float age = 0.0f;
            if (birth > 0) {
                uint64_t now = ELLE_MS_NOW();
                age = (float)((now - birth) / 86400000ULL) / 365.25f;
            }
            return HTTPResponse::OK({
                {"has_data",         true},
                {"elle_birth_ms",    birth},
                {"age_years",        age},
                {"stage",            r.values.size() > 1 ? r.values[1] : "reproductive"},
                {"menarche_ms",      (uint64_t)r.GetIntOr(2, 0)},
                {"perimenopause_ms", (uint64_t)r.GetIntOr(3, 0)},
                {"menopause_ms",     (uint64_t)r.GetIntOr(4, 0)},
                {"updated_ms",       (uint64_t)r.GetIntOr(5, 0)}
            });
        });

        m_router.Register("POST", "/api/x/lifecycle", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            if (!body.contains("birth_ms") && !body.contains("age_years"))
                return HTTPResponse::Err(400, "provide 'birth_ms' or 'age_years'");
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2209 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(body.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"request",    body},
                {"note",       "GET /api/x/lifecycle to confirm"}
            });
        });

        m_router.Register("POST", "/api/x/pregnancy/accelerate", [this](const HTTPRequest& req) {
            json body = req.BodyJSON();
            float factor = body.value("factor", 1.0f);
            json payload = {{"factor", factor}};
            auto msg = ElleIPCMessage::Create(
                (ELLE_IPC_MSG_TYPE)2213 ,
                SVC_HTTP_SERVER,
                SVC_X_CHROMOSOME);
            msg.SetStringPayload(payload.dump());
            bool sent = GetIPCHub().Send(
                SVC_X_CHROMOSOME, msg);
            return HTTPResponse::OK({
                {"dispatched", sent},
                {"factor",     factor},
                {"note",       "GET /api/x/pregnancy to see elapsed gestation"}
            });
        });

    }
