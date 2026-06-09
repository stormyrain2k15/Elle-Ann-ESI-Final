#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/json.hpp"
#include "ComposerEngine.h"
#include <string>
#include <chrono>
#include <unordered_map>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Persist compose result to composer_log
// ---------------------------------------------------------------------------
static int64_t PersistLog(const std::string& requestId,
                           const std::string& kind,
                           const ComposeResult& r)
{
    using namespace std::chrono;
    int64_t nowMs = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count();

    json slotsJ = json::array();
    for (const auto& s : r.slots) {
        slotsJ.push_back({
            {"name",    s.name},
            {"lemma",   s.lemma},
            {"form",    s.form},
            {"surface", s.surface},
            {"score",   s.score}
        });
    }

    std::string sql =
        "INSERT INTO ElleHeart.dbo.composer_log "
        "(recorded_ms,request_id,kind,act,frame_id,slots_json,text,confidence) "
        "OUTPUT inserted.log_id "
        "VALUES (?,?,?,?,?,?,?,?)";

    std::vector<std::string> params = {
        std::to_string(nowMs),
        requestId,
        kind,
        r.act,
        std::to_string(r.frameId),
        slotsJ.dump(),
        r.text,
        std::to_string(r.confidence)
    };

    auto rs = ElleSQLPool::Instance().QueryParams(sql, params);
    if (!rs.success) {
        ELLE_WARN("Composer: failed to persist log for %s", requestId.c_str());
        return 0;
    }
    if (rs.rows.empty()) return 0;
    try {
        return std::stoll(rs.rows[0][0]);
    } catch (const std::exception& e) {
        ELLE_WARN("Composer: could not parse log row id '%s' for rid=%s: %s",
                  rs.rows[0][0].c_str(), requestId.c_str(), e.what());
        return 0;
    }
}

// ---------------------------------------------------------------------------
// Service
// ---------------------------------------------------------------------------
class ElleComposerService : public ElleServiceBase {
public:
    ElleComposerService()
        : ElleServiceBase(
              SVC_COMPOSER,
              "ElleComposer",
              "Elle-Ann Composer",
              "Deterministic sentence composer. No LLMs, no tokens, no tensors.")
    {}

protected:

    bool OnStart() override {
        if (!m_engine.LoadFrames()) {
            ELLE_ERROR("Composer: frame library failed to load");
            return false;
        }
        if (!m_engine.LoadInflections()) {
            ELLE_ERROR("Composer: inflection tables failed to load");
            return false;
        }
        SetTickInterval(0);
        ELLE_INFO("Composer service started — %s",
                  "deterministic sentence composition active.");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Composer service stopped.");
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_PROBABILITY };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch (static_cast<ELLE_IPC_MSG_TYPE>(msg.header.msg_type)) {
            case IPC_COMPOSE_REQUEST:
                HandleCompose(msg, sender);
                break;
            default:
                break;
        }
    }

private:

    void HandleCompose(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        std::string payload = msg.GetStringPayload();
        json envelope;
        if (!Elle::ExtractJsonObject(payload, envelope)) {
            SendError(sender, "?", "malformed_compose_request");
            return;
        }

        const std::string requestId = envelope.value("request_id", std::string("?"));
        const std::string kind      = envelope.value("kind",       std::string("CONVERSE"));
        const bool        streaming = envelope.value("stream",     false);

        // Run the 5-step pipeline.
        ComposeResult result = m_engine.Compose(envelope);

        // If streaming, emit clauses first.
        if (streaming && result.success) {
            for (const auto& clause : result.clauses) {
                json chunk;
                chunk["request_id"]  = requestId;
                chunk["clause_index"]= clause.index;
                chunk["text"]        = clause.text;
                chunk["final"]       = clause.final;

                auto chunkMsg = ElleIPCMessage::Create(
                    IPC_COMPOSE_STREAM_CHUNK, SVC_COMPOSER, sender);
                chunkMsg.header.correlation_id = msg.header.correlation_id;
                chunkMsg.SetStringPayload(chunk.dump());
                GetIPCHub().Send(sender, chunkMsg);
            }
        }

        // Persist to log.
        int64_t logId = 0;
        if (result.success) {
            logId = PersistLog(requestId, kind, result);
        }

        // Send final response.
        json resp;
        resp["request_id"] = requestId;
        resp["success"]    = result.success;
        resp["error"]      = result.error;
        resp["text"]       = result.text;
        resp["act"]        = result.act;
        resp["frame_id"]   = result.frameId;
        resp["confidence"] = result.confidence;
        resp["log_id"]     = logId;

        if (result.success && result.frameId > 0) {
            ElleDB::RecordMetric(
                std::string("composer_frame_uses_") + std::to_string(result.frameId),
                (double)++m_frameUses[result.frameId]);
            ElleDB::RecordMetric("composer_compositions_total",
                                 (double)++m_compositionsTotal);
        }

        json slotsJ = json::array();
        for (const auto& s : result.slots) {
            slotsJ.push_back({
                {"name",    s.name},
                {"lemma",   s.lemma},
                {"form",    s.form},
                {"score",   s.score}
            });
        }
        resp["slots"] = slotsJ;

        auto reply = ElleIPCMessage::Create(
            IPC_COMPOSE_RESPONSE, SVC_COMPOSER, sender);
        reply.header.correlation_id = msg.header.correlation_id;
        reply.SetStringPayload(resp.dump());
        GetIPCHub().Send(sender, reply);

        ELLE_INFO("Composer: [%s] kind=%s act=%s confidence=%.2f text=%.60s",
                  requestId.c_str(), kind.c_str(), result.act.c_str(),
                  result.confidence, result.text.c_str());
    }

    void SendError(ELLE_SERVICE_ID dest,
                   const std::string& requestId,
                   const std::string& reason)
    {
        json resp;
        resp["request_id"] = requestId;
        resp["success"]    = false;
        resp["error"]      = reason;
        resp["text"]       = "";

        auto reply = ElleIPCMessage::Create(
            IPC_COMPOSE_RESPONSE, SVC_COMPOSER, dest);
        reply.SetStringPayload(resp.dump());
        GetIPCHub().Send(dest, reply);

        ELLE_WARN("Composer error [%s]: %s", requestId.c_str(), reason.c_str());
    }

    ComposerEngine m_engine;
    std::unordered_map<int64_t, uint64_t> m_frameUses;
    uint64_t                              m_compositionsTotal = 0;
};

ELLE_SERVICE_MAIN(ElleComposerService)
