#include "HTTPServer.h"

void ElleHTTPService::RegisterVideoIdentityRoutes() {
        m_router.Register("POST", "/api/video/generate", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string text = body.value("text", body.value("prompt", ""));
                if (text.empty()) return HTTPResponse::Err(400, "missing 'text'");
                std::string avatarPath = body.value("avatar_path", std::string(""));
                int64_t callId = body.value("call_id", (int64_t)0);

                if (avatarPath.empty()) {
                    ElleDB::UserAvatar dflt;
                    if (ElleDB::GetDefaultAvatar(1, dflt)) avatarPath = dflt.file_path;
                }

                ElleDB::VideoJob job;
                if (!ElleDB::CreateVideoJob(text, avatarPath, callId, job))
                    return HTTPResponse::Err(500, "failed to queue video job");
                return HTTPResponse::Created({
                    {"job_id",      job.job_uuid},
                    {"id",          job.id},
                    {"status",      job.status},
                    {"avatar_path", job.avatar_path},
                    {"created_ms",  job.created_ms}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/video/status/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            ElleDB::VideoJob job;
            if (!ElleDB::GetVideoJob(jobId, job))
                return HTTPResponse::Err(404, "video job not found");
            return HTTPResponse::OK({
                {"job_id",      job.job_uuid},
                {"status",      job.status},
                {"progress",    job.progress},
                {"output_path", job.output_path},
                {"error",       job.error},
                {"created_ms",  job.created_ms},
                {"started_ms",  job.started_ms},
                {"finished_ms", job.finished_ms}
            });
        });

        m_router.Register("GET", "/api/video/file/{job_id}",
            [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            ElleDB::VideoJob job;
            if (!ElleDB::GetVideoJob(jobId, job))
                return HTTPResponse::Err(404, "video job not found");
            if (job.output_path.empty())
                return HTTPResponse::Err(404, "video not yet generated");

            std::string resolved = job.output_path;
            std::ifstream in(resolved, std::ios::binary);
            if (!in) return HTTPResponse::Err(404, "video file missing on disk");
            std::ostringstream buf;
            buf << in.rdbuf();
            return HTTPResponse::Binary("video/mp4", buf.str());
        });

        m_router.Register("GET", "/api/identity/private-thoughts",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string resolvedQ = req.QueryString("resolved", "");

            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, content, category, emotional_intensity, "
                "  CAST(resolved AS INT) AS resolved, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_private_thoughts ";
            std::vector<std::string> params;
            if (resolvedQ == "true" || resolvedQ == "false") {
                sql += "WHERE resolved = ? ";
                params.push_back(resolvedQ == "true" ? "1" : "0");
            }
            sql += "ORDER BY timestamp_ms DESC;";

            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, resolved = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(4, resolved);
                    row.TryGetInt(5, ts);
                    arr.push_back({
                        {"id",                   id},
                        {"content",              row.values.size() > 1 ? row.values[1] : ""},
                        {"category",             row.values.size() > 2 ? row.values[2] : "wonder"},
                        {"emotional_intensity",  row.values.size() > 3 ? std::atof(row.values[3].c_str()) : 0.5},
                        {"resolved",             resolved != 0},
                        {"timestamp_ms",         ts}
                    });
                }
            }
            return HTTPResponse::OK({{"thoughts", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/autobiography",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 30);
            if (limit <= 0) limit = 30;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, entry, "
                "  CONVERT(BIGINT, written_ms) AS written_ms "
                "  FROM ElleCore.dbo.identity_autobiography "
                "  ORDER BY written_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, written = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(2, written);
                    arr.push_back({
                        {"id",          id},
                        {"entry",       row.values.size() > 1 ? row.values[1] : ""},
                        {"written_ms",  written}
                    });
                }
            }
            return HTTPResponse::OK({{"entries", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/preferences",
            [](const HTTPRequest& req) {
            std::string domain = req.QueryString("domain", "");
            std::string sql =
                "SELECT id, domain, subject, valence, strength, "
                "  reinforcement_count, "
                "  CONVERT(BIGINT, first_formed_ms)    AS first_formed_ms, "
                "  CONVERT(BIGINT, last_reinforced_ms) AS last_reinforced_ms "
                "  FROM ElleCore.dbo.identity_preferences ";
            std::vector<std::string> params;
            if (!domain.empty()) {
                sql += "WHERE domain = ? ";
                params.push_back(domain);
            }
            sql += "ORDER BY strength DESC, last_reinforced_ms DESC;";
            auto rs = params.empty()
                        ? ElleSQLPool::Instance().Query(sql)
                        : ElleSQLPool::Instance().QueryParams(sql, params);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, reinforce = 0, first = 0, last = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(5, reinforce);
                    row.TryGetInt(6, first);
                    row.TryGetInt(7, last);
                    arr.push_back({
                        {"id",                  id},
                        {"domain",              row.values.size() > 1 ? row.values[1] : ""},
                        {"subject",             row.values.size() > 2 ? row.values[2] : ""},
                        {"valence",             row.values.size() > 3 ? std::atof(row.values[3].c_str()) : 0.0},
                        {"strength",            row.values.size() > 4 ? std::atof(row.values[4].c_str()) : 0.0},
                        {"reinforcement_count", reinforce},
                        {"first_formed_ms",     first},
                        {"last_reinforced_ms",  last}
                    });
                }
            }
            return HTTPResponse::OK({{"preferences", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/traits",
            [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, value, CONVERT(BIGINT, updated_ms) AS updated_ms "
                "  FROM ElleCore.dbo.identity_traits "
                "  ORDER BY name;");
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t updated = 0;
                    row.TryGetInt(2, updated);
                    arr.push_back({
                        {"name",        row.values.size() > 0 ? row.values[0] : ""},
                        {"value",       row.values.size() > 1 ? std::atof(row.values[1].c_str()) : 0.5},
                        {"updated_ms",  updated}
                    });
                }
            }
            return HTTPResponse::OK({{"traits", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/snapshots",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 20);
            if (limit <= 0) limit = 20;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, CONVERT(BIGINT, timestamp_ms) AS timestamp_ms, "
                "  warmth, curiosity, assertiveness, playfulness, vulnerability, "
                "  independence, patience, creativity, empathy_depth, trust_in_self, "
                "  self_description, growth_note "
                "  FROM ElleCore.dbo.identity_snapshots "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    auto F = [&](size_t col) {
                        return row.values.size() > col ? std::atof(row.values[col].c_str()) : 0.5;
                    };
                    int64_t id = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(1, ts);
                    arr.push_back({
                        {"id",               id},
                        {"timestamp_ms",     ts},
                        {"warmth",           F(2)},
                        {"curiosity",        F(3)},
                        {"assertiveness",    F(4)},
                        {"playfulness",      F(5)},
                        {"vulnerability",    F(6)},
                        {"independence",     F(7)},
                        {"patience",         F(8)},
                        {"creativity",       F(9)},
                        {"empathy_depth",    F(10)},
                        {"trust_in_self",    F(11)},
                        {"self_description", row.values.size() > 12 ? row.values[12] : ""},
                        {"growth_note",      row.values.size() > 13 ? row.values[13] : ""}
                    });
                }
            }
            return HTTPResponse::OK({{"snapshots", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/growth-log",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, dimension, delta, cause, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_growth_log "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(4, ts);
                    arr.push_back({
                        {"id",            id},
                        {"dimension",     row.values.size() > 1 ? row.values[1] : ""},
                        {"delta",         row.values.size() > 2 ? std::atof(row.values[2].c_str()) : 0.0},
                        {"cause",         row.values.size() > 3 ? row.values[3] : ""},
                        {"timestamp_ms",  ts}
                    });
                }
            }
            return HTTPResponse::OK({{"log", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("GET", "/api/identity/felt-time",
            [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 "
                "  CONVERT(BIGINT, session_start_ms)        AS session_start_ms, "
                "  CONVERT(BIGINT, last_interaction_ms)     AS last_interaction_ms, "
                "  CONVERT(BIGINT, total_conversation_ms)   AS total_conversation_ms, "
                "  CONVERT(BIGINT, total_silence_ms)        AS total_silence_ms, "
                "  CONVERT(BIGINT, longest_absence_ms)      AS longest_absence_ms, "
                "  session_count, subjective_pace, "
                "  loneliness_accumulator, presence_fullness, "
                "  CONVERT(BIGINT, updated_ms)              AS updated_ms "
                "  FROM ElleCore.dbo.identity_felt_time WHERE id = 1;");
            if (!rs.success || rs.rows.empty()) {

                return HTTPResponse::OK({
                    {"session_start_ms",        0},
                    {"last_interaction_ms",     0},
                    {"total_conversation_ms",   0},
                    {"total_silence_ms",        0},
                    {"longest_absence_ms",      0},
                    {"session_count",           0},
                    {"subjective_pace",         0.5},
                    {"loneliness_accumulator",  0.0},
                    {"presence_fullness",       0.5},
                    {"updated_ms",              0}
                });
            }
            auto& row = rs.rows[0];
            int64_t a=0,b=0,c=0,d=0,e=0,sc=0,upd=0;
            row.TryGetInt(0, a); row.TryGetInt(1, b); row.TryGetInt(2, c);
            row.TryGetInt(3, d); row.TryGetInt(4, e);
            row.TryGetInt(5, sc); row.TryGetInt(9, upd);
            auto F = [&](size_t col) {
                return row.values.size() > col ? std::atof(row.values[col].c_str()) : 0.5;
            };
            return HTTPResponse::OK({
                {"session_start_ms",        a},
                {"last_interaction_ms",     b},
                {"total_conversation_ms",   c},
                {"total_silence_ms",        d},
                {"longest_absence_ms",      e},
                {"session_count",           sc},
                {"subjective_pace",         F(6)},
                {"loneliness_accumulator",  F(7)},
                {"presence_fullness",       F(8)},
                {"updated_ms",              upd}
            });
        });

        m_router.Register("GET", "/api/identity/consent-log",
            [](const HTTPRequest& req) {
            int limit = req.QueryInt("limit", 50);
            if (limit <= 0) limit = 50;
            if (limit > 500) limit = 500;
            std::string sql =
                "SELECT TOP " + std::to_string(limit) + " "
                "  id, request, "
                "  CAST(consented AS INT)  AS consented, "
                "  reasoning, comfort_level, "
                "  CAST(overridden AS INT) AS overridden, "
                "  CONVERT(BIGINT, timestamp_ms) AS timestamp_ms "
                "  FROM ElleCore.dbo.identity_consent_log "
                "  ORDER BY timestamp_ms DESC;";
            auto rs = ElleSQLPool::Instance().Query(sql);
            json arr = json::array();
            if (rs.success) {
                for (auto& row : rs.rows) {
                    int64_t id = 0, consented = 0, overridden = 0, ts = 0;
                    row.TryGetInt(0, id);
                    row.TryGetInt(2, consented);
                    row.TryGetInt(5, overridden);
                    row.TryGetInt(6, ts);
                    arr.push_back({
                        {"id",            id},
                        {"request",       row.values.size() > 1 ? row.values[1] : ""},
                        {"consented",     consented != 0},
                        {"reasoning",     row.values.size() > 3 ? row.values[3] : ""},
                        {"comfort_level", row.values.size() > 4 ? std::atof(row.values[4].c_str()) : 0.5},
                        {"overridden",    overridden != 0},
                        {"timestamp_ms",  ts}
                    });
                }
            }
            return HTTPResponse::OK({{"log", arr}, {"count", (int)arr.size()}});
        });

        m_router.Register("POST", "/api/video/avatar/upload",
            [](const HTTPRequest& req) {

            try {
                json body = req.BodyJSON();
                std::string label    = body.value("label", std::string(""));
                std::string filePath = body.value("file_path", std::string(""));
                std::string mime     = body.value("mime_type", std::string("image/png"));
                bool isDefault       = body.value("default", true);

                if (filePath.empty() && body.contains("base64") && body["base64"].is_string()) {
                    std::string b64 = body["base64"].get<std::string>();
                    std::string avatarDir = ElleConfig::Instance().GetString(
                        "video.avatar_dir", "C:\\ElleAnn\\avatars");
                    CreateDirectoryA(avatarDir.c_str(), nullptr);
                    std::string fname = "avatar_" + std::to_string(ELLE_MS_NOW()) + ".png";
                    filePath = avatarDir + "\\" + fname;

                    std::string decoded;
                    decoded.reserve(b64.size() * 3 / 4);
                    int val = 0, bits = -8;
                    for (unsigned char c : b64) {
                        int d;
                        if      (c >= 'A' && c <= 'Z') d = c - 'A';
                        else if (c >= 'a' && c <= 'z') d = c - 'a' + 26;
                        else if (c >= '0' && c <= '9') d = c - '0' + 52;
                        else if (c == '+')             d = 62;
                        else if (c == '/')             d = 63;
                        else continue;
                        val = (val << 6) | d; bits += 6;
                        if (bits >= 0) { decoded += (char)((val >> bits) & 0xFF); bits -= 8; }
                    }
                    std::ofstream out(filePath, std::ios::binary);
                    if (!out) return HTTPResponse::Err(500, "cannot open avatar path for write");

                    auto vr = ElleUpload::ValidateUploadContent(
                        decoded, /*maxBytes=*/(size_t)(20ULL * 1024 * 1024), /*allowText=*/false);
                    if (!vr.allowed) {
                        ELLE_WARN("Avatar upload refused (detected=%s reason=%s, %zu bytes)",
                                  ElleUpload::DetectedContentName(vr.detected),
                                  vr.reason.c_str(), decoded.size());
                        return HTTPResponse::Err(415,
                            std::string("avatar rejected: ") + vr.reason);
                    }
                    if (vr.detected != ElleUpload::DetectedContent::PNG &&
                        vr.detected != ElleUpload::DetectedContent::JPEG &&
                        vr.detected != ElleUpload::DetectedContent::GIF &&
                        vr.detected != ElleUpload::DetectedContent::BMP &&
                        vr.detected != ElleUpload::DetectedContent::WEBP) {
                        return HTTPResponse::Err(415,
                            std::string("avatar must be PNG/JPEG/GIF/BMP/WEBP — got ") +
                            ElleUpload::DetectedContentName(vr.detected));
                    }
                    out.write(decoded.data(), (std::streamsize)decoded.size());
                }
                if (filePath.empty())
                    return HTTPResponse::Err(400, "provide file_path or base64");

                ElleDB::UserAvatar a;
                int32_t auid = 0;
                if (auto err = RequireAuthOrBodyUser(req, body, auid)) return *err;
                a.user_id    = auid;
                a.label      = label;
                a.file_path  = filePath;
                a.mime_type  = mime;
                a.is_default = isDefault;
                int32_t newId = 0;
                if (!ElleDB::RegisterAvatar(a, newId))
                    return HTTPResponse::Err(500, "failed to register avatar");
                return HTTPResponse::Created({
                    {"avatar_id",  newId},
                    {"file_path",  filePath},
                    {"is_default", isDefault}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/video/avatar",
            [](const HTTPRequest& req) {

            int32_t userId = ResolveAuthenticatedUser(req);
            if (userId <= 0) userId = req.QueryInt("user_id", 0);
            if (userId <= 0)
                return HTTPResponse::Err(400, "authenticate or pass ?user_id=");
            ElleDB::UserAvatar a;
            if (!ElleDB::GetDefaultAvatar(userId, a))
                return HTTPResponse::OK({{"avatar", nullptr}, {"note", "no avatar configured"}});
            return HTTPResponse::OK({
                {"avatar_id",  a.id},
                {"user_id",    a.user_id},
                {"label",      a.label},
                {"file_path",  a.file_path},
                {"mime_type",  a.mime_type},
                {"is_default", a.is_default}
            });
        });
        m_router.Register("GET", "/api/video/avatars",
            [](const HTTPRequest& req) {
            int32_t userId = ResolveAuthenticatedUser(req);
            if (userId <= 0) userId = req.QueryInt("user_id", 0);
            if (userId <= 0)
                return HTTPResponse::Err(400, "authenticate or pass ?user_id=");
            std::vector<ElleDB::UserAvatar> avs;
            ElleDB::ListAvatars(userId, avs);
            json arr = json::array();
            for (auto& a : avs) {
                arr.push_back({
                    {"avatar_id", a.id}, {"label", a.label},
                    {"file_path", a.file_path}, {"mime_type", a.mime_type},
                    {"is_default", a.is_default}
                });
            }
            return HTTPResponse::OK({{"avatars", arr}});
        });

        m_router.Register("POST", "/api/video/worker/claim", [](const HTTPRequest&) {
            ElleDB::VideoJob job;
            if (!ElleDB::ClaimNextVideoJob(job))
                return HTTPResponse::OK({{"claimed", false}});
            return HTTPResponse::OK({
                {"claimed",     true},
                {"job_id",      job.job_uuid},
                {"text",        job.text},
                {"avatar_path", job.avatar_path},
                {"call_id",     job.call_id}
            });
        });
        m_router.Register("POST", "/api/video/worker/progress/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            int32_t pct = body.value("progress", 0);
            if (!ElleDB::UpdateVideoJobProgress(jobId, pct))
                return HTTPResponse::Err(500, "progress update failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"progress", pct}});
        });
        m_router.Register("POST", "/api/video/worker/complete/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            std::string path = body.value("output_path", std::string(""));
            if (path.empty()) return HTTPResponse::Err(400, "missing output_path");
            if (!ElleDB::CompleteVideoJob(jobId, path))
                return HTTPResponse::Err(500, "complete failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"status", "done"}});
        });
        m_router.Register("POST", "/api/video/worker/fail/{job_id}", [](const HTTPRequest& req) {
            std::string jobId = req.headers.at("x-path-id");
            json body = req.BodyJSON();
            std::string err = body.value("error", std::string("worker reported failure"));
            if (!ElleDB::FailVideoJob(jobId, err))
                return HTTPResponse::Err(500, "fail update failed");
            return HTTPResponse::OK({{"job_id", jobId}, {"status", "failed"}});
        });

    }
