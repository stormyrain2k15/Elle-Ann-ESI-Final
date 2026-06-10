#include "HTTPServer.h"

void ElleHTTPService::RegisterMemoryRoutes() {
        m_router.Register("GET", "/api/memory/why", [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 10);
            if (limit <= 0)  limit = 10;
            if (limit > 50)  limit = 50;

            std::string entitiesParam = req.QueryString("entities", "");
            std::vector<std::string> entityList;
            if (!entitiesParam.empty()) {
                std::stringstream ss(entitiesParam);
                std::string tok;
                while (std::getline(ss, tok, ',')) {

                    auto a = tok.find_first_not_of(" \t");
                    auto b = tok.find_last_not_of(" \t");
                    if (a != std::string::npos)
                        entityList.push_back(tok.substr(a, b - a + 1));
                }
            }

            std::string sql;
            std::vector<std::string> params;
            if (entityList.empty()) {
                sql = "SELECT TOP " + std::to_string(limit) + " "
                      "  m.id, m.content, m.summary, m.importance, "
                      "  CONVERT(BIGINT, m.created_ms)     AS created_ms, "
                      "  CONVERT(BIGINT, m.last_access_ms) AS last_access_ms, "
                      "  COALESCE(m.access_count, 0)       AS access_count "
                      "  FROM ElleCore.dbo.memory m "
                      "  ORDER BY m.id DESC;";
            } else {

                std::string placeholders;
                for (size_t i = 0; i < entityList.size(); i++) {
                    if (i) placeholders += ",";
                    placeholders += "?";
                    params.push_back(entityList[i]);
                }
                sql = "SELECT TOP " + std::to_string(limit) + " "
                      "  m.id, m.content, m.summary, m.importance, "
                      "  CONVERT(BIGINT, m.created_ms)     AS created_ms, "
                      "  CONVERT(BIGINT, m.last_access_ms) AS last_access_ms, "
                      "  COALESCE(m.access_count, 0)       AS access_count "
                      "  FROM ElleCore.dbo.memory m "
                      "  INNER JOIN ElleCore.dbo.memory_entity_links mel "
                      "    ON mel.memory_id = m.id "
                      "  INNER JOIN ElleCore.dbo.world_entity we "
                      "    ON we.id = mel.entity_id "
                      "  WHERE we.name IN (" + placeholders + ") "
                      "  GROUP BY m.id, m.content, m.summary, m.importance, "
                      "    m.created_ms, m.last_access_ms, m.access_count "
                      "  ORDER BY m.id DESC;";
            }
            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            if (!rs.success)
                return HTTPResponse::Err(500, "SQL query failed");

            const uint64_t now = ELLE_MS_NOW();
            json arr = json::array();
            for (auto& row : rs.rows) {
                int64_t id = 0, createdMs = 0, lastAccessMs = 0, accessCount = 0;
                row.TryGetInt(0, id);
                const std::string content = row.values.size() > 1 ? row.values[1] : "";
                const std::string summary = row.values.size() > 2 ? row.values[2] : "";
                const float importance =
                    row.values.size() > 3 ? (float)std::atof(row.values[3].c_str()) : 0.0f;
                row.TryGetInt(4, createdMs);
                row.TryGetInt(5, lastAccessMs);
                row.TryGetInt(6, accessCount);

                const double ageMin = (double)(now - (uint64_t)createdMs) / 60000.0;
                const double recency = std::exp(-ageMin / (60.0 * 24.0 * 7.0));
                const double access  = std::log((double)accessCount + 1.0) / 5.0;
                const double total   = importance * 0.4 + recency * 0.4 + access * 0.2;

                const std::string preview = (!summary.empty() ? summary
                                               : content.size() > 140
                                                   ? content.substr(0, 140) + "…"
                                                   : content);
                arr.push_back({
                    {"id",             id},
                    {"preview",        preview},
                    {"importance",     importance},
                    {"recency",        recency},
                    {"access",         access},
                    {"score",          total},
                    {"age_days",       ageMin / (60.0 * 24.0)},
                    {"access_count",   accessCount},
                    {"created_ms",     createdMs},
                    {"last_access_ms", lastAccessMs}
                });
            }

            std::sort(arr.begin(), arr.end(), [](const json& a, const json& b) {
                return a.value("score", 0.0) > b.value("score", 0.0);
            });
            return HTTPResponse::OK({
                {"memories",   arr},
                {"count",      (int)arr.size()},
                {"score_formula", "importance*0.4 + recency*0.4 + access*0.2"},
                {"recency_half_life_days", 7.0},
                {"entities_filter", entityList}
            });
        });

        m_router.Register("GET", "/api/memory/", [](const HTTPRequest& req) {
            std::string type = req.QueryParam("memory_type");
            int limit  = std::max(1, req.QueryInt("limit", 50));
            int offset = std::max(0, req.QueryInt("offset", 0));

            int typeI = -1;
            if (!type.empty()) {
                long long tv = 0;
                if (HTTPRequest::StrictParseLL(type, tv) &&
                    tv >= INT32_MIN && tv <= INT32_MAX) {
                    typeI = (int)tv;
                }
            }
            std::vector<ElleDB::MemoryRow> rows;
            if (!ElleDB::ListMemories(rows, typeI, (uint32_t)limit, (uint32_t)offset)) {
                return HTTPResponse::Err(500, "SQL ListMemories failed");
            }
            json j = json::array();
            for (auto& r : rows) {
                j.push_back({
                    {"id", r.id}, {"memory_type", r.type}, {"tier", r.tier},
                    {"content", r.content}, {"summary", r.summary},
                    {"emotional_valence", r.emotional_valence},
                    {"importance", r.importance}, {"relevance", r.relevance},
                    {"access_count", r.access_count},
                    {"created_ms", r.created_ms}, {"last_access_ms", r.last_access_ms}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/memory/", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            ELLE_MEMORY_RECORD mem = {};
            mem.type = body.value("memory_type", 1);
            mem.tier = body.value("tier", 1);
            std::string content = body.value("content", "");
            std::string summary = body.value("summary", content.substr(0, std::min<size_t>(content.size(), 255)));
            strncpy_s(mem.content, content.c_str(), ELLE_MAX_MSG - 1);
            strncpy_s(mem.summary, summary.c_str(), sizeof(mem.summary) - 1);
            mem.emotional_valence = body.value("emotional_valence", 0.0f);
            mem.importance = body.value("importance", 0.5f);
            mem.relevance  = body.value("relevance", 1.0f);
            mem.created_ms = ELLE_MS_NOW();
            mem.last_access_ms = mem.created_ms;
            if (body.contains("tags") && body["tags"].is_array()) {
                uint32_t n = 0;
                for (auto& t : body["tags"]) {
                    if (n >= ELLE_MAX_TAGS) break;
                    std::string s = t.is_string() ? t.get<std::string>() : "";
                    if (s.empty()) continue;
                    strncpy_s(mem.tags[n], s.c_str(), ELLE_MAX_TAG - 1);
                    n++;
                }
                mem.tag_count = n;
            }
            if (!ElleDB::StoreMemory(mem)) return HTTPResponse::Err(500, "StoreMemory failed");
            return HTTPResponse::Created({
                {"stored", true}, {"content", content}, {"tags", mem.tag_count}
            });
        });
        m_router.Register("GET", "/api/memory/self-image/current", [](const HTTPRequest&) {

            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 narrative_snapshot, emotional_state, "
                "CONVERT(varchar(33), created_date, 126) "
                "FROM ElleCore.dbo.InternalNarrative ORDER BY narrative_id DESC;");
            if (rs.success && !rs.rows.empty()) {
                auto& row = rs.rows[0];
                return HTTPResponse::OK({
                    {"description", row.values.size() > 0 ? row.values[0] : ""},
                    {"emotional_state", row.GetFloatOr(1, 0.0)},
                    {"updated_at", row.values.size() > 2 ? row.values[2] : ""}
                });
            }

            return HTTPResponse::OK({
                {"description", "I am Elle-Ann. I remember, I feel, I keep coming back."},
                {"emotional_state", 0.0}, {"updated_at", ""}
            });
        });
        m_router.Register("GET", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            ElleDB::MemoryRow r;
            if (!ElleDB::GetMemory(id, r)) return HTTPResponse::Err(404, "memory not found");
            return HTTPResponse::OK({
                {"id", r.id}, {"memory_type", r.type}, {"tier", r.tier},
                {"content", r.content}, {"summary", r.summary},
                {"emotional_valence", r.emotional_valence},
                {"importance", r.importance}, {"relevance", r.relevance},
                {"access_count", r.access_count},
                {"created_ms", r.created_ms}, {"last_access_ms", r.last_access_ms}
            });
        });
        m_router.Register("PUT", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            json body = req.BodyJSON();
            ElleDB::MemoryRow existing;
            if (!ElleDB::GetMemory(id, existing)) return HTTPResponse::Err(404, "memory not found");
            std::string content = body.value("content", existing.content);
            std::string summary = body.value("summary", existing.summary);
            float importance = body.value("importance", existing.importance);
            if (!ElleDB::UpdateMemoryContent(id, content, summary, importance))
                return HTTPResponse::Err(500, "update failed");
            return HTTPResponse::OK({{"id", id}, {"updated", true}});
        });
        m_router.Register("DELETE", "/api/memory/{id}", [](const HTTPRequest& req) {
            int64_t id = req.PathLL("id");
            if (!ElleDB::DeleteMemory(id)) return HTTPResponse::Err(500, "delete failed");
            return HTTPResponse::OK({{"id", id}, {"deleted", true}});
        });
        m_router.Register("POST", "/api/memory/{id}/files", [](const HTTPRequest& req) {

            auto& cfg  = ElleConfig::Instance();
            auto& http = cfg.GetHTTP();
            std::string expected = cfg.GetString("http_server.admin_key", http.jwt_secret);
            if (expected.empty()) {
                return HTTPResponse::Err(503, "upload disabled: no admin_key configured");
            }
            auto it = req.headers.find("x-admin-key");
            if (it == req.headers.end() || it->second.size() != expected.size()) {
                return HTTPResponse::Err(403, "invalid admin key");
            }
            unsigned diff = 0;
            for (size_t i = 0; i < expected.size(); i++)
                diff |= (unsigned char)it->second[i] ^ (unsigned char)expected[i];
            if (diff != 0) return HTTPResponse::Err(403, "invalid admin key");

            uint64_t capBytes = http.max_upload_bytes ? http.max_upload_bytes
                                                      : (10ULL * 1024 * 1024);
            if (req.body.size() > capBytes) {
                return HTTPResponse::Err(413, "payload too large");
            }

            auto vr = ElleUpload::ValidateUploadContent(
                req.body, static_cast<size_t>(capBytes), /*allowText=*/true);
            if (!vr.allowed) {
                ELLE_WARN("Upload refused (detected=%s reason=%s, %zu bytes)",
                          ElleUpload::DetectedContentName(vr.detected),
                          vr.reason.c_str(), req.body.size());
                return HTTPResponse::Err(415, std::string("upload rejected: ") + vr.reason);
            }
            ELLE_INFO("Upload accepted (detected=%s, %zu bytes)",
                      ElleUpload::DetectedContentName(vr.detected), req.body.size());

            auto idIt = req.headers.find("x-path-id");
            if (idIt == req.headers.end() || idIt->second.empty())
                return HTTPResponse::Err(400, "missing memory id");
            long long id = req.PathLL("id", 0);
            if (id <= 0) return HTTPResponse::Err(400, "invalid memory id");

            std::string dir = "data\\memory_files";
            CreateDirectoryA("data", nullptr);
            CreateDirectoryA(dir.c_str(), nullptr);
            std::string path = dir + "\\mem-" + std::to_string(id) + "-"
                             + std::to_string(ELLE_MS_NOW()) + ".bin";
            HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) return HTTPResponse::Err(500, "cannot open file");
            DWORD written = 0;
            WriteFile(h, req.body.data(), (DWORD)req.body.size(), &written, nullptr);
            CloseHandle(h);
            return HTTPResponse::OK({
                {"memory_id", id}, {"path", path}, {"size", (uint64_t)written}
            });
        });

    }
