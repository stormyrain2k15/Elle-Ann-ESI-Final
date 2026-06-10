#include "HTTPServer.h"

void ElleHTTPService::RegisterAIRoutes() {
        m_router.Register("POST", "/api/ai/chat", [this](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string userText = body.value("message", body.value("prompt", ""));
                if (userText.empty()) return HTTPResponse::Err(400, "missing 'message'");

                uint64_t convId = 1;
                if (body.contains("conversation_id") && body["conversation_id"].is_number_integer())
                    convId = body["conversation_id"].get<uint64_t>();
                else if (body.contains("conversationId") && body["conversationId"].is_number_integer())
                    convId = body["conversationId"].get<uint64_t>();
                else if (body.contains("session_id") && body["session_id"].is_number_integer())
                    convId = body["session_id"].get<uint64_t>();

                std::string userId = body.value("user_id", body.value("userId", std::string("default")));
                std::string requestId = "req-" + std::to_string(ELLE_MS_NOW()) +
                                        "-" + std::to_string(++m_requestSeq);

                json env = {
                    {"request_id", requestId},
                    {"user_text", userText},
                    {"user_id", userId},
                    {"conv_id", convId},
                    {"origin", "http"}
                };

                auto pending = m_chatCorrelator.Register(requestId);

                auto ipcMsg = ElleIPCMessage::Create(IPC_CHAT_REQUEST, SVC_HTTP_SERVER, SVC_COGNITIVE);
                ipcMsg.SetStringPayload(env.dump());
                if (!GetIPCHub().Send(SVC_COGNITIVE, ipcMsg)) {
                    m_chatCorrelator.Cancel(requestId);
                    return HTTPResponse::Err(503, "Cognitive service unreachable");
                }

                ELLE_INFO("Chat→Cognitive conv=%llu rid=%s msg=%.60s...",
                          (unsigned long long)convId, requestId.c_str(), userText.c_str());

                std::unique_lock<std::mutex> lk(pending->m);
                bool ok = pending->cv.wait_for(lk, std::chrono::seconds(45),
                                               [&]{ return pending->done; });
                if (!ok) {
                    m_chatCorrelator.Cancel(requestId);
                    return HTTPResponse::Err(504, "Cognitive timeout (45s)");
                }

                json out = pending->result;
                out.erase("request_id");
                return HTTPResponse::OK(out);
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });
        m_router.Register("GET", "/api/ai/self-prompts", [](const HTTPRequest& req) {
            int limit = std::max(1, req.QueryInt("limit", 20));
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP (?) id, prompt, ISNULL(source,''), created_ms "
                "FROM ElleCore.dbo.ai_self_prompts ORDER BY id DESC;",
                { std::to_string(limit) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json j = json::array();
            for (auto& r : rs.rows) {
                j.push_back({
                    {"id", r.GetIntOr(0, 0)},
                    {"prompt", r.values.size() > 1 ? r.values[1] : ""},
                    {"source", r.values.size() > 2 ? r.values[2] : ""},
                    {"created_ms", r.GetIntOr(3, 0)}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/ai/self-prompts/generate", [this](const HTTPRequest&) {

            std::string requestId = "sp-" + std::to_string(ELLE_MS_NOW());
            json env = {
                {"request_id", requestId},
                {"user_text", "[internal] Generate one brief self-reflective prompt you'd ask yourself right now."},
                {"user_id", "self"}, {"conv_id", (uint64_t)0}, {"origin", "self_prompt"}
            };
            auto pending = m_chatCorrelator.Register(requestId);
            auto ipcMsg = ElleIPCMessage::Create(IPC_CHAT_REQUEST, SVC_HTTP_SERVER, SVC_COGNITIVE);
            ipcMsg.SetStringPayload(env.dump());
            if (!GetIPCHub().Send(SVC_COGNITIVE, ipcMsg)) {
                m_chatCorrelator.Cancel(requestId);
                return HTTPResponse::Err(503, "Cognitive service unreachable");
            }
            std::unique_lock<std::mutex> lk(pending->m);
            if (!pending->cv.wait_for(lk, std::chrono::seconds(20),
                                       [&]{ return pending->done; })) {
                m_chatCorrelator.Cancel(requestId);
                return HTTPResponse::Err(504, "timeout");
            }
            std::string text = pending->result.value("response", "");

            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.ai_self_prompts (prompt, source, created_ms) VALUES (?, 'self_prompt', ?);",
                { text, std::to_string(ELLE_MS_NOW()) });
            return HTTPResponse::OK({
                {"prompt", text}, {"generated_at", (uint64_t)ELLE_MS_NOW()}
            });
        });
        m_router.Register("GET", "/api/ai/status", [this](const HTTPRequest&) {

            auto& llm    = ElleConfig::Instance().GetLLM();
            const bool initialised = ElleComposer::Client::Instance().IsBound();
            std::string activeProvider = "composer";
            (void)llm;

            std::string modelName = "elle.composer.deterministic";
            std::string modelUrl  = "ipc://Elle.Service.Composer";
            json j = {
                {"modelStatus", initialised ? "ready" : "unavailable"},
                {"modelName",   modelName},
                {"modelUrl",    modelUrl},
                {"provider",    activeProvider},
                {"emotionalState", {
                    {"valence", m_cachedEmotions.valence},
                    {"arousal", m_cachedEmotions.arousal},
                    {"dominance", m_cachedEmotions.dominance},
                    {"joy",        m_cachedEmotions.valence > 0 ? m_cachedEmotions.valence : 0.0},
                    {"sadness",    m_cachedEmotions.valence < 0 ? -m_cachedEmotions.valence : 0.0}
                }}
            };
            return HTTPResponse::OK(j);
        });
        m_router.Register("POST", "/api/ai/analyze-emotion", [this](const HTTPRequest& req) {

            json body = req.BodyJSON();
            std::string text = body.value("text", "");
            if (text.empty()) return HTTPResponse::Err(400, "missing 'text'");
            std::string l = text;
            std::transform(l.begin(), l.end(), l.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            struct W { const char* w; float v; };
            static const W pos[] = {{"love",0.8f},{"happy",0.6f},{"thank",0.5f},{"great",0.5f},
                                    {"good",0.3f},{"glad",0.5f},{"missed",0.6f},{"proud",0.6f},
                                    {"excited",0.7f},{"beautiful",0.7f}};
            static const W neg[] = {{"hate",-0.8f},{"sad",-0.6f},{"angry",-0.7f},{"upset",-0.6f},
                                    {"worried",-0.5f},{"tired",-0.4f},{"hurt",-0.6f},{"lonely",-0.7f},
                                    {"afraid",-0.6f},{"hopeless",-0.8f}};
            float val = 0.0f, arou = 0.0f;
            for (auto& w : pos) if (l.find(w.w) != std::string::npos) { val += w.v; arou += 0.1f; }
            for (auto& w : neg) if (l.find(w.w) != std::string::npos) { val += w.v; arou += 0.15f; }
            if (std::count(text.begin(), text.end(), '!') > 0) arou += 0.15f;
            val  = std::max(-1.0f, std::min(1.0f, val));
            arou = std::max(0.0f,  std::min(1.0f, arou));
            std::string dominant = val > 0.3f ? "joy" : (val < -0.3f ? "sadness" : "neutral");
            return HTTPResponse::OK({
                {"text", text}, {"valence", val}, {"arousal", arou},
                {"dominant", dominant}
            });
        });
        m_router.Register("GET", "/api/ai/memory-tracking", [](const HTTPRequest&) {
            int64_t mem  = ElleDB::CountTable("memory");
            int64_t msgs = ElleDB::CountTable("messages");
            int64_t refs = ElleDB::CountTable("SelfReflections");
            int64_t ents = ElleDB::CountTable("world_entity");
            return HTTPResponse::OK({
                {"total_memories", std::max<int64_t>(mem,0)},
                {"total_messages", std::max<int64_t>(msgs,0)},
                {"total_reflections", std::max<int64_t>(refs,0)},
                {"total_entities", std::max<int64_t>(ents,0)}
            });
        });
        m_router.Register("GET", "/api/ai/autonomy/status", [](const HTTPRequest&) {
            ELLE_TRUST_STATE trust = {};
            ElleDB::GetTrustState(trust);
            const char* levelStr = "sandboxed";
            if (trust.score >= TRUST_THRESHOLD_AUTONOMOUS) levelStr = "autonomous";
            else if (trust.score >= TRUST_THRESHOLD_ELEVATED) levelStr = "elevated";
            else if (trust.score >= TRUST_THRESHOLD_BASIC)    levelStr = "basic";
            return HTTPResponse::OK({
                {"autonomous", trust.score >= TRUST_THRESHOLD_AUTONOMOUS},
                {"trust_level", levelStr},
                {"trust_score", trust.score},
                {"successes", trust.successes},
                {"failures", trust.failures},
                {"self_prompting_active", true}
            });
        });
        m_router.Register("GET", "/api/ai/hardware/info", [](const HTTPRequest&) {
            MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem);
            GlobalMemoryStatusEx(&mem);
            SYSTEM_INFO si; GetSystemInfo(&si);
            OSVERSIONINFOA osvi = {}; osvi.dwOSVersionInfoSize = sizeof(osvi);
            char compName[256] = {}; DWORD sz = sizeof(compName);
            GetComputerNameA(compName, &sz);
            return HTTPResponse::OK({
                {"os", "Windows"},
                {"hostname", std::string(compName)},
                {"cpu_count", (int)si.dwNumberOfProcessors},
                {"ram_total_mb", (uint64_t)(mem.ullTotalPhys / (1024ULL*1024ULL))},
                {"ram_used_mb",  (uint64_t)((mem.ullTotalPhys - mem.ullAvailPhys) / (1024ULL*1024ULL))},
                {"ram_percent", (float)mem.dwMemoryLoad}
            });
        });

        m_router.Register("GET", "/api/ai/hardware/actions/pending", [](const HTTPRequest& req) {
            std::string target = req.QueryParam("target", "device");
            json j = json::array();

            std::vector<ELLE_ACTION_RECORD> actions;
            ElleDB::GetPendingActions(actions, 50);
            for (auto& a : actions) {
                j.push_back({
                    {"source", "action_queue"},
                    {"id", a.id}, {"type", a.type},
                    {"command", std::string(a.command)},
                    {"parameters", std::string(a.parameters)},
                    {"required_trust", a.required_trust}
                });
            }

            auto peek = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 50 id, action_type, ISNULL(payload,''), created_ms "
                "FROM ElleCore.dbo.hardware_actions "
                "WHERE status = 'pending' ORDER BY id ASC;", {});
            if (peek.success) {
                for (auto& r : peek.rows) {
                    j.push_back({
                        {"source", "hardware_actions"},
                        {"id", r.GetIntOr(0, 0)},
                        {"action_type", r.values.size() > 1 ? r.values[1] : ""},
                        {"payload",     r.values.size() > 2 ? r.values[2] : ""},
                        {"created_ms",  r.GetIntOr(3, 0)}
                    });
                }
            }
            (void)target;
            return HTTPResponse::OK(j);
        }, AUTH_USER);

        m_router.Register("POST", "/api/ai/hardware/actions/claim", [](const HTTPRequest& req) {
            (void)req;
            json j = json::array();
            auto claim = ElleSQLPool::Instance().Query(
                "UPDATE TOP (50) ElleCore.dbo.hardware_actions "
                "SET status = 'dispatched' "
                "OUTPUT inserted.id, inserted.action_type, ISNULL(inserted.payload,''), "
                "       inserted.created_ms "
                "WHERE status = 'pending';");
            if (!claim.success) return HTTPResponse::Err(500, claim.error);
            for (auto& r : claim.rows) {
                j.push_back({
                    {"source", "hardware_actions"},
                    {"id", r.GetIntOr(0, 0)},
                    {"action_type", r.values.size() > 1 ? r.values[1] : ""},
                    {"payload",     r.values.size() > 2 ? r.values[2] : ""},
                    {"created_ms",  r.GetIntOr(3, 0)}
                });
            }
            return HTTPResponse::OK(j);
        }, AUTH_INTERNAL_ONLY);
        m_router.Register("POST", "/api/ai/hardware/actions/{id}/ack", [](const HTTPRequest& req) {

            int64_t id = req.PathLL("id");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.hardware_actions "
                "SET status = 'consumed', consumed_ms = ? "
                "WHERE id = ? AND status = 'dispatched';",
                { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string(id) });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"id", id}, {"acked", true}});
        });
        m_router.Register("POST", "/api/ai/hardware/actions/{id}/result", [](const HTTPRequest& req) {
            uint64_t actionId = req.PathLL("id");
            json body = req.BodyJSON();
            std::string result = body.value("result", std::string("done"));
            ELLE_ACTION_STATUS status = body.value("success", true)
                                         ? ACTION_COMPLETED_SUCCESS
                                         : ACTION_COMPLETED_FAILURE;
            bool ok = ElleDB::UpdateActionStatus(actionId, status, result);
            if (!ok) return HTTPResponse::Err(500, "UpdateActionStatus failed");
            return HTTPResponse::OK({{"action_id", actionId}, {"recorded", true}});
        });

        m_router.Register("GET", "/api/ai/tools", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, ISNULL(description,''), ISNULL(config,''), enabled "
                "FROM ElleCore.dbo.ai_tools ORDER BY name;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"name", r.values.size() > 0 ? r.values[0] : ""},
                    {"description", r.values.size() > 1 ? r.values[1] : ""},
                    {"config", r.values.size() > 2 ? r.values[2] : ""},
                    {"enabled", r.GetIntOr(3, 0) != 0}
                });
            }
            return HTTPResponse::OK({{"tools", arr}});
        });
        m_router.Register("POST", "/api/ai/tools", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            std::string name = body.value("name", "");
            if (name.empty()) return HTTPResponse::Err(400, "missing name");
            std::string desc = body.value("description", "");
            std::string cfg  = body.contains("config") ? body["config"].dump() : std::string();
            auto rs = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleCore.dbo.ai_tools AS t "
                "USING (SELECT ? AS name) AS s ON t.name = s.name "
                "WHEN MATCHED THEN UPDATE SET description = ?, config = ?, enabled = 1 "
                "WHEN NOT MATCHED THEN INSERT (name, description, config, enabled) VALUES (?, ?, ?, 1);",
                { name, desc, cfg, name, desc, cfg });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"name", name}, {"stored", true}});
        });
        m_router.Register("DELETE", "/api/ai/tools/{name}", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.ai_tools WHERE name = ?;", { name });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"deleted", true}, {"name", name}});
        });

        m_router.Register("GET", "/api/ai/agents", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT name, ISNULL(description,''), ISNULL(system_prompt,''), ISNULL(model,'') "
                "FROM ElleCore.dbo.ai_agents ORDER BY name;");
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"name", r.values.size() > 0 ? r.values[0] : ""},
                    {"description", r.values.size() > 1 ? r.values[1] : ""},
                    {"system_prompt", r.values.size() > 2 ? r.values[2] : ""},
                    {"model", r.values.size() > 3 ? r.values[3] : ""}
                });
            }
            return HTTPResponse::OK({{"agents", arr}});
        });
        m_router.Register("POST", "/api/ai/agents", [](const HTTPRequest& req) {
            json body = req.BodyJSON();
            std::string name = body.value("name", "");
            if (name.empty()) return HTTPResponse::Err(400, "missing name");
            std::string desc  = body.value("description", "");
            std::string sys   = body.value("system_prompt", "");
            std::string model = body.value("model", "");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleCore.dbo.ai_agents AS t "
                "USING (SELECT ? AS name) AS s ON t.name = s.name "
                "WHEN MATCHED THEN UPDATE SET description = ?, system_prompt = ?, model = ? "
                "WHEN NOT MATCHED THEN INSERT (name, description, system_prompt, model) VALUES (?, ?, ?, ?);",
                { name, desc, sys, model, name, desc, sys, model });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::Created({{"name", name}, {"stored", true}});
        });
        m_router.Register("DELETE", "/api/ai/agents/{name}", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "DELETE FROM ElleCore.dbo.ai_agents WHERE name = ?;", { name });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            return HTTPResponse::OK({{"deleted", true}, {"name", name}});
        });
        m_router.Register("POST", "/api/ai/agents/{name}/chat", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string message = body.value("message", "");
                std::string agentName = req.headers.at("x-path-name");

                auto rs = ElleSQLPool::Instance().QueryParams(
                    "SELECT TOP 1 ISNULL(system_prompt,''), ISNULL(model,'') "
                    "FROM ElleCore.dbo.ai_agents WHERE name = ?;",
                    { agentName });
                std::string sys = "You are agent " + agentName + ". Respond as this agent would.";
                if (rs.success && !rs.rows.empty()) {
                    std::string s = rs.rows[0].values.size() > 0 ? rs.rows[0].values[0] : "";
                    if (!s.empty()) sys = s;
                }
                std::vector<LLMMsg> convo = { {"system", sys}, {"user", message} };
                std::string response, err;
                if (!CallGroqDirect(convo, response, err))
                    return HTTPResponse::Err(502, err);
                return HTTPResponse::OK({{"agent", agentName}, {"response", response}});
            } catch (const std::exception& e) {
                return HTTPResponse::Err(500, e.what());
            }
        });

    }
