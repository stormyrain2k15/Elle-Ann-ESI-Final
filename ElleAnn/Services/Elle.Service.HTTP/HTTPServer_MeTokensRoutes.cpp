#include "HTTPServer.h"

void ElleHTTPService::RegisterMeTokensRoutes() {
        m_router.Register("GET", "/api/me", [](const HTTPRequest& req) {
            auto it = req.headers.find("x-auth-device-id");
            if (it == req.headers.end() || it->second.empty())
                return HTTPResponse::Err(401, "no device identity on request");
            const std::string& deviceId = it->second;

            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 1 nUserNo, PairedAt, LastSeen "
                "  FROM ElleCore.dbo.PairedDevices "
                "  WHERE DeviceId = ? AND Revoked = 0;",
                { deviceId });
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::Err(401, "device not paired");
            int64_t nUserNo = 0;
            std::string pairedAt = rs.rows[0].values.size() > 1 ? rs.rows[0][1] : "";
            std::string lastSeen = rs.rows[0].values.size() > 2 ? rs.rows[0][2] : "";
            rs.rows[0].TryGetInt(0, nUserNo);
            if (nUserNo <= 0)
                return HTTPResponse::Err(500, "device row has no nUserNo");

            ElleGameAuth::UserIdentity user;
            std::string username;
            if (ElleGameAuth::GetUserById(nUserNo, user)) {
                username = user.sUserID;
            }
            return HTTPResponse::OK({
                {"user_id",              (int32_t)nUserNo},
                {"username",             username},
                {"device_id",            deviceId},
                {"paired_at",            pairedAt},
                {"last_seen",            lastSeen},
                {"authoritative_source", "Account.dbo.tUser"}
            });
        });

        m_router.Register("GET", "/api/me/recap", [](const HTTPRequest& req) {
            auto it = req.headers.find("x-auth-device-id");
            if (it == req.headers.end() || it->second.empty())
                return HTTPResponse::Err(401, "no device identity on request");
            const std::string& deviceId = it->second;

            auto idRs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 1 nUserNo, "
                "  DATEDIFF(MINUTE, LastSeen, GETUTCDATE()) AS quiet_min, "
                "  CONVERT(NVARCHAR(33), LastSeen, 126) AS last_seen "
                "  FROM ElleCore.dbo.PairedDevices "
                "  WHERE DeviceId = ? AND Revoked = 0;",
                { deviceId });
            if (!idRs.success || idRs.rows.empty())
                return HTTPResponse::Err(401, "device not paired");
            int64_t nUserNo = 0;
            int64_t quietMin = 0;
            idRs.rows[0].TryGetInt(0, nUserNo);
            idRs.rows[0].TryGetInt(1, quietMin);
            const std::string lastSeen = idRs.rows[0].values.size() > 2
                                              ? idRs.rows[0][2] : "";

            std::string lastMemSummary;
            int64_t     lastMemMs = 0;
            {
                auto mRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 1 created_ms, "
                    "  COALESCE(NULLIF(LTRIM(RTRIM(summary)), ''), "
                    "           SUBSTRING(content, 1, 140)) AS s "
                    "  FROM ElleCore.dbo.memory "
                    "  ORDER BY created_ms DESC;");
                if (mRs.success && !mRs.rows.empty()) {
                    mRs.rows[0].TryGetInt(0, lastMemMs);
                    if (mRs.rows[0].values.size() > 1)
                        lastMemSummary = mRs.rows[0][1];
                }
            }

            float lastValence = 0.0f, prevValence = 0.0f, valenceDelta = 0.0f;
            int64_t lastEmotionMs = 0;
            {
                auto eRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 2 valence, taken_ms "
                    "  FROM ElleCore.dbo.emotion_snapshots "
                    "  ORDER BY taken_ms DESC;");
                if (eRs.success && !eRs.rows.empty()) {
                    int64_t v = 0;
                    if (eRs.rows[0].values.size() > 0)
                        lastValence = (float)std::atof(eRs.rows[0][0].c_str());
                    eRs.rows[0].TryGetInt(1, v);
                    lastEmotionMs = v;
                    if (eRs.rows.size() > 1 && eRs.rows[1].values.size() > 0)
                        prevValence = (float)std::atof(eRs.rows[1][0].c_str());
                    valenceDelta = lastValence - prevValence;
                }
            }

            int64_t pendingIntents = 0;
            {
                auto qRs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.IntentQueue "
                    "  WHERE Status = 0;");
                if (qRs.success && !qRs.rows.empty())
                    qRs.rows[0].TryGetInt(0, pendingIntents);
            }

            int64_t openThreads = 0;
            std::string topThread;
            {
                auto tRs = ElleSQLPool::Instance().Query(
                    "SELECT TOP 1 topic FROM ElleCore.dbo.ElleThreads "
                    "  WHERE status IS NULL OR status = 'open' "
                    "  ORDER BY emotional_weight DESC, last_touched DESC;");
                if (tRs.success && !tRs.rows.empty() && !tRs.rows[0].values.empty())
                    topThread = tRs.rows[0][0];
                auto cRs = ElleSQLPool::Instance().Query(
                    "SELECT COUNT(*) FROM ElleCore.dbo.ElleThreads "
                    "  WHERE status IS NULL OR status = 'open';");
                if (cRs.success && !cRs.rows.empty())
                    cRs.rows[0].TryGetInt(0, openThreads);
            }

            return HTTPResponse::OK({
                {"user_id",                (int32_t)nUserNo},
                {"last_seen",              lastSeen},
                {"quiet_minutes",          quietMin},
                {"last_memory_ms",         lastMemMs},
                {"last_memory_summary",    lastMemSummary},
                {"last_emotion_ms",        lastEmotionMs},
                {"emotion_valence_now",    lastValence},
                {"emotion_valence_delta",  valenceDelta},
                {"pending_intents",        pendingIntents},
                {"open_threads",           openThreads},
                {"top_thread",             topThread}
            });
        });

        m_router.Register("POST", "/api/tokens/conversations",
            [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            int32_t userId = 0;
            if (auto err = RequireAuthOrBodyUser(req, body, userId)) return *err;
            std::string title = body.value("title", std::string("New conversation"));
            int32_t newId = 0;
            if (!ElleDB::CreateConversation(userId, title, newId))
                return HTTPResponse::Err(500, "CreateConversation failed");
            return HTTPResponse::Created({
                {"id", newId}, {"user_id", userId}, {"title", title}, {"is_active", true}
            });
        });
        m_router.Register("GET", "/api/tokens/conversations", [](const HTTPRequest& req) {
            int limit = std::max(1, req.QueryInt("limit", 50));
            std::vector<ElleDB::ConversationRow> rows;
            if (!ElleDB::ListConversations(rows, (uint32_t)limit))
                return HTTPResponse::Err(500, "ListConversations failed");
            json j = json::array();
            for (auto& c : rows) {
                j.push_back({
                    {"id", c.id}, {"user_id", c.user_id}, {"title", c.title},
                    {"started_at", c.started_at}, {"last_message_at", c.last_message_at},
                    {"total_messages", c.total_messages}, {"is_active", c.is_active}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/tokens/conversations/{id}", [](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            ElleDB::ConversationRow c;
            if (!ElleDB::GetConversation(convId, c))
                return HTTPResponse::Err(404, "conversation not found");
            return HTTPResponse::OK({
                {"id", c.id}, {"user_id", c.user_id}, {"title", c.title},
                {"started_at", c.started_at}, {"last_message_at", c.last_message_at},
                {"total_messages", c.total_messages}, {"is_active", c.is_active}
            });
        });
        m_router.Register("POST", "/api/tokens/conversations/{id}/messages", [this](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            json body = req.BodyJSON();
            std::string content = body.value("content", body.value("message", ""));
            std::string role    = body.value("role", std::string("user"));
            uint32_t roleInt = (role == "user") ? 1 : (role == "assistant" || role == "elle" ? 2 : 0);
            if (content.empty()) return HTTPResponse::Err(400, "missing content");
            if (!ElleDB::StoreMessage((uint64_t)convId, roleInt, content,
                                       m_cachedEmotions, 0.0f))
                return HTTPResponse::Err(500, "StoreMessage failed");
            return HTTPResponse::Created({
                {"conversation_id", convId}, {"role", role}, {"stored", true}
            });
        });
        m_router.Register("GET", "/api/tokens/conversations/{id}/messages", [](const HTTPRequest& req) {
            int32_t convId = req.PathInt("id");
            int limit = std::max(1, req.QueryInt("limit", 50));
            std::vector<ELLE_CONVERSATION_MSG> msgs;
            if (!ElleDB::GetConversationHistory((uint64_t)convId, msgs, (uint32_t)limit))
                return HTTPResponse::Err(500, "GetConversationHistory failed");
            json j = json::array();
            for (auto& m : msgs) {
                j.push_back({
                    {"conversation_id", m.conversation_id},
                    {"role", m.role == 1 ? "user" : (m.role == 2 ? "assistant" : "system")},
                    {"content", std::string(m.content)},
                    {"timestamp_ms", m.timestamp_ms}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/tokens/video-calls",
            [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            int32_t userId = 0;
            if (auto err = RequireAuthOrBodyUser(req, body, userId)) return *err;
            int32_t convId = body.value("conversation_id", 0);
            if (convId <= 0)
                return HTTPResponse::Err(400, "conversation_id must be positive");
            std::string callId;
            if (!ElleDB::StartVoiceCall(userId, convId, callId))
                return HTTPResponse::Err(500, "StartVoiceCall failed");
            return HTTPResponse::Created({
                {"call_id", callId}, {"user_id", userId}, {"conversation_id", convId},
                {"status", "active"}
            });
        });
        m_router.Register("PUT", "/api/tokens/video-calls/{id}/end", [](const HTTPRequest& req) {
            std::string callId = req.headers.at("x-path-id");
            if (!ElleDB::EndVoiceCall(callId)) return HTTPResponse::Err(500, "end failed");
            return HTTPResponse::OK({{"call_id", callId}, {"status", "ended"}});
        });
        m_router.Register("POST", "/api/tokens/interactions", [this](const HTTPRequest& req) {

            json body = req.BodyJSON();
            std::string text = body.value("text", body.value("description", std::string("interaction")));
            auto rs = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.SelfReflections "
                "(reflection_text, reflection_type, effectiveness_score, reflection_date) "
                "VALUES (?, 'interaction', ?, GETUTCDATE());",
                { text, std::to_string(m_cachedEmotions.valence) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"logged", true}, {"text", text}});
        });
        m_router.Register("POST", "/api/ai/voice-call/{id}/end", [](const HTTPRequest& req) {
            std::string callId = req.headers.at("x-path-id");
            ElleDB::EndVoiceCall(callId);
            return HTTPResponse::OK({{"call_id", callId}, {"status", "ended"}});
        });

    }
