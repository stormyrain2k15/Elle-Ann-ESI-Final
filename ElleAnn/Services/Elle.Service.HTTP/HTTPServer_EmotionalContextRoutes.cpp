#include "HTTPServer.h"

void ElleHTTPService::RegisterEmotionalContextRoutes() {
        m_router.Register("GET", "/api/emotional-context/patterns", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 50 thread_id, ISNULL(topic,''), ISNULL(emotional_weight, 0), "
                "       ISNULL(status,''), ISNULL(last_discussed, GETUTCDATE()) "
                "FROM ElleCore.dbo.ElleThreads WHERE status <> 'resolved' "
                "ORDER BY emotional_weight DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"topic", r.values.size() > 1 ? r.values[1] : ""},
                    {"emotional_weight", r.GetFloatOr(2, 0.0)},
                    {"status", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/emotional-context/vocabulary", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 100 tag, COUNT(*) AS freq FROM ElleCore.dbo.memory_tags "
                "GROUP BY tag ORDER BY freq DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"term", r.values.size() > 0 ? r.values[0] : ""},
                    {"frequency", r.GetIntOr(1, 0)}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/emotional-context/history", [](const HTTPRequest& req) {

            uint32_t hours = (uint32_t)req.QueryInt("hours", 24);
            if (hours == 0 || hours > 24 * 30) hours = 24;
            uint32_t maxPoints = (uint32_t)req.QueryInt("points", 500);
            if (maxPoints == 0 || maxPoints > 5000) maxPoints = 500;

            std::vector<ElleDB::EmotionHistoryPoint> pts;
            if (!ElleDB::GetEmotionHistory(hours, pts, maxPoints))
                return HTTPResponse::Err(500, "emotion_snapshots query failed");

            json series = json::array();
            for (auto& p : pts) {
                series.push_back({
                    {"t",         p.taken_ms},
                    {"valence",   p.valence},
                    {"arousal",   p.arousal},
                    {"dominance", p.dominance}
                });
            }
            return HTTPResponse::OK({
                {"hours",  hours},
                {"points", (int64_t)series.size()},
                {"series", series}
            });
        });

        m_router.Register("GET", "/api/emotional-context/dimensions", [](const HTTPRequest& req) {
            int64_t ts = req.QueryLL("t", 0);
            int topN   = req.QueryInt("top", 5);
            if (topN <= 0 || topN > 102) topN = 5;

            auto rs = ts > 0
                ? ElleSQLPool::Instance().QueryParams(
                    "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
                    "SELECT TOP 1 taken_ms, valence, arousal, dominance, dimensions "
                    "FROM ElleCore.dbo.emotion_snapshots "
                    "ORDER BY ABS(taken_ms - ?) ASC;",
                    { std::to_string(ts) })
                : ElleSQLPool::Instance().Query(
                    "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
                    "SELECT TOP 1 taken_ms, valence, arousal, dominance, dimensions "
                    "FROM ElleCore.dbo.emotion_snapshots ORDER BY id DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK({{"found", false}});

            auto& r = rs.rows[0];
            int64_t takenMs = r.GetIntOr(0, 0);
            double val = r.GetFloatOr(1, 0.0), aro = r.GetFloatOr(2, 0.0), dom = r.GetFloatOr(3, 0.0);
            std::string dimStr = r.values.size() > 4 ? r.values[4] : "";

            std::vector<std::pair<int, float>> ranked;
            ranked.reserve(ELLE_EMOTION_COUNT);
            std::istringstream iss(dimStr);
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                float v = 0.0f;
                if (!(iss >> v)) break;
                ranked.push_back({i, v});
            }
            std::sort(ranked.begin(), ranked.end(),
                      [](const std::pair<int,float>& a, const std::pair<int,float>& b) {
                          return a.second > b.second;
                      });
            json top = json::array();
            for (int i = 0; i < topN && i < (int)ranked.size(); i++) {
                int idx = ranked[i].first;
                const char* name = (idx >= 0 && idx < ELLE_EMOTION_COUNT)
                                   ? kEmotionMeta[idx].name : "";
                const char* cat  = (idx >= 0 && idx < ELLE_EMOTION_COUNT)
                                   ? kEmotionMeta[idx].category : "";
                top.push_back({
                    {"index",    idx},
                    {"name",     name},
                    {"category", cat},
                    {"value",    ranked[i].second}
                });
            }
            return HTTPResponse::OK({
                {"found",     true},
                {"taken_ms",  takenMs},
                {"valence",   val},
                {"arousal",   aro},
                {"dominance", dom},
                {"top",       top}
            });
        });

        m_router.Register("GET", "/api/session/greeting", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'reconnection_greetings') "
                "  SELECT CAST(NULL AS BIGINT), CAST('' AS NVARCHAR(MAX)), "
                "         CAST('{}' AS NVARCHAR(MAX)), CAST(0 AS BIGINT) WHERE 1=0; "
                "ELSE "
                "  SELECT TOP 1 id, greeting, ISNULL(context_json,'{}'), created_ms "
                "  FROM ElleCore.dbo.reconnection_greetings "
                "  WHERE consumed = 0 ORDER BY id DESC;");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::OK({{"greeting", nullptr}});
            auto& r = rs.rows[0];
            json ctx = json::object();
            try { ctx = json::parse(r.values.size() > 2 ? r.values[2] : "{}"); }
            catch (const std::exception& e) {
                ELLE_DEBUG("continuity_greeting context JSON parse failed: %s", e.what());
            }
            return HTTPResponse::OK({
                {"id",         r.GetIntOr(0, 0)},
                {"greeting",   r.values.size() > 1 ? r.values[1] : ""},
                {"context",    ctx},
                {"created_ms", r.GetIntOr(3, 0)}
            });
        });
        m_router.Register("POST", "/api/session/greeting/{id}/ack", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.reconnection_greetings "
                "SET consumed = 1 WHERE id = ?;",
                { std::to_string(id) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"id", id}, {"consumed", true}});
        });
        m_router.Register("GET", "/api/emotional-context/growth", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 50 reflection_id, ISNULL(reflection_text,''), "
                "       ISNULL(effectiveness_score, 0), "
                "       CONVERT(varchar(33), reflection_date, 126) "
                "FROM ElleCore.dbo.SelfReflections "
                "ORDER BY reflection_date DESC;");
            if (!rs.success) return HTTPResponse::OK(json::array());
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"text", r.values.size() > 1 ? r.values[1] : ""},
                    {"effectiveness", r.GetFloatOr(2, 0.0)},
                    {"date", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });

    }
