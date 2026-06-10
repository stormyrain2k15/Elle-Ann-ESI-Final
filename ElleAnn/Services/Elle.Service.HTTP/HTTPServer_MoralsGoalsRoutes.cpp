#include "HTTPServer.h"

void ElleHTTPService::RegisterMoralsGoalsRoutes() {
        m_router.Register("GET", "/api/morals/rules", [](const HTTPRequest& req) {
            std::string category = req.QueryParam("category");
            std::string sql = "SELECT id, principle, ISNULL(category,''), is_hard_rule "
                              "FROM ElleCore.dbo.moral_rules ";
            std::vector<std::string> params;
            if (!category.empty()) { sql += "WHERE category = ? "; params.push_back(category); }
            sql += "ORDER BY id;";
            auto rs = ElleSQLPool::Instance().QueryParams(sql, params);
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"principle", r.values.size() > 1 ? r.values[1] : ""},
                    {"category",  r.values.size() > 2 ? r.values[2] : ""},
                    {"is_hard_rule", r.GetIntOr(3, 0) != 0}
                });
            }
            return HTTPResponse::OK({{"rules", arr}});
        }, AUTH_ADMIN);
        m_router.Register("POST", "/api/morals/rules", [](const HTTPRequest& req) {

            auto it = req.headers.find("x-admin-key");
            if (it == req.headers.end() || it->second.empty()) {
                return HTTPResponse::Err(401, "missing x-admin-key");
            }
            auto& cfg = ElleConfig::Instance();
            std::string expected = cfg.GetString("http_server.admin_key",
                                   cfg.GetHTTP().jwt_secret);
            if (expected.empty()) {
                return HTTPResponse::Err(503, "admin endpoint disabled: no admin_key configured");
            }
            const std::string& got = it->second;
            if (got.size() != expected.size()) {
                return HTTPResponse::Err(403, "invalid admin key");
            }
            unsigned diff = 0;
            for (size_t i = 0; i < got.size(); i++)
                diff |= (unsigned char)got[i] ^ (unsigned char)expected[i];
            if (diff != 0) return HTTPResponse::Err(403, "invalid admin key");

            json body = req.BodyJSON();
            std::string principle = body.value("principle", "");
            std::string category  = body.value("category", "core");
            bool hard = body.value("is_hard_rule", false);
            if (principle.empty()) return HTTPResponse::Err(400, "missing principle");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.moral_rules (principle, category, is_hard_rule) "
                "VALUES (?, ?, ?);",
                { principle, category, std::to_string(hard ? 1 : 0) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({
                {"principle", principle}, {"category", category}, {"stored", true}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/goals", [](const HTTPRequest&) {
            std::vector<ELLE_GOAL_RECORD> goals;
            try { ElleDB::GetActiveGoals(goals); }
            catch (const std::exception& e) {
                ELLE_WARN("GetActiveGoals failed: %s", e.what());
            }
            json arr = json::array();
            for (auto& g : goals) {
                arr.push_back({
                    {"id", g.id},
                    {"description", std::string(g.description)},
                    {"progress", g.progress}
                });
            }
            return HTTPResponse::OK({{"goals", arr}});
        });
    }
