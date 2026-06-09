#include "ElleSQLConn.h"
#include "ElleLogger.h"
#include "ElleConfig.h"
#include "ElleTypes.h"

#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cctype>
#include <mutex>
#include <set>
#include <random>

namespace ElleDB {

static constexpr uint32_t ROLE_UNKNOWN  = 0xFFFFFFFFu;
static constexpr uint32_t ROLE_SYSTEM   = 0;
static constexpr uint32_t ROLE_USER     = 1;
static constexpr uint32_t ROLE_ELLE     = 2;
static constexpr uint32_t ROLE_INTERNAL = 3;

static std::string RoleToStr(uint32_t role) {

    switch (role) {
        case ROLE_USER:     return "user";
        case ROLE_ELLE:     return "assistant";
        case ROLE_INTERNAL: return "internal";
        default:            return "system";
    }
}

static uint32_t RoleFromStr(const std::string& s) {
    std::string l = s;
    std::transform(l.begin(), l.end(), l.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    if (l == "user")                                  return ROLE_USER;
    if (l == "assistant" || l == "elle" || l == "ai") return ROLE_ELLE;
    if (l == "internal")                              return ROLE_INTERNAL;
    if (l == "system")                                return ROLE_SYSTEM;

    return ROLE_UNKNOWN;
}

static bool RoleFromStrStrict(const std::string& s, uint32_t& out) {
    uint32_t r = RoleFromStr(s);
    if (r == ROLE_UNKNOWN) return false;
    out = r;
    return true;
}

bool StoreMessage(uint64_t convoId, uint32_t role, const std::string& content,
                  const ELLE_EMOTION_STATE& emotions, float sentiment) {
    (void)sentiment;

    if (role != ROLE_SYSTEM && role != ROLE_USER &&
        role != ROLE_ELLE   && role != ROLE_INTERNAL) {
        ELLE_ERROR("StoreMessage: refusing unknown role id=%u for conv=%llu",
                   (unsigned)role, (unsigned long long)convoId);
        return false;
    }

    std::string dominant = "neutral";
    float topW = 0.0f;
    static const char* names[] = {
        "joy","sadness","anger","fear","surprise","trust","curiosity","empathy"
    };
    for (int i = 0; i < 8 && i < ELLE_EMOTION_COUNT; i++) {
        if (emotions.dimensions[i] > topW) { topW = emotions.dimensions[i]; dominant = names[i]; }
    }

    const std::string roleStr = RoleToStr(role);
    const std::string direction = (role == ROLE_USER) ? "in" : "out";

    auto convCheck = ElleSQLPool::Instance().QueryParams(
        "SELECT user_id FROM ElleCore.dbo.conversations WHERE id = ?;",
        { std::to_string(convoId) });
    if (!convCheck.success) {
        ELLE_ERROR("StoreMessage: conversation lookup failed for conv=%llu: %s",
                   (unsigned long long)convoId, convCheck.error.c_str());
        return false;
    }
    if (convCheck.rows.empty()) {
        ELLE_ERROR("StoreMessage: no conversation row for conv=%llu — refusing to "
                   "synthesize user_id; caller must create the conversation first.",
                   (unsigned long long)convoId);
        return false;
    }
    int64_t userId = 0;
    if (!convCheck.rows[0].TryGetInt(0, userId) || userId <= 0) {
        ELLE_ERROR("StoreMessage: conversation %llu has no valid user_id",
                   (unsigned long long)convoId);
        return false;
    }

    auto rs = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.messages "
        "(conversation_id, user_id, role, content, emotion_detected, emotion_intensity, "
        " created_at, Direction) "
        "VALUES (?, ?, ?, ?, ?, ?, GETUTCDATE(), ?);",
        {
            std::to_string(convoId),
            std::to_string(userId),
            roleStr,
            content,
            dominant,
            std::to_string(topW),
            direction
        });
    if (!rs.success) {
        ELLE_ERROR("StoreMessage INSERT failed for conv=%llu role=%s: %s",
                   (unsigned long long)convoId, roleStr.c_str(), rs.error.c_str());
        return false;
    }
    auto rs2 = ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.conversations "
        "  SET last_message_at = GETUTCDATE(), "
        "      total_messages  = ISNULL(total_messages,0) + 1 "
        "  WHERE id = ?;",
        { std::to_string(convoId) });
    if (!rs2.success) {
        ELLE_WARN("StoreMessage: conversation bump failed for conv=%llu: %s",
                  (unsigned long long)convoId, rs2.error.c_str());

    }
    return true;
}

bool GetConversationHistory(uint64_t convoId,
                            std::vector<ELLE_CONVERSATION_MSG>& out,
                            uint32_t limit) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) role, content, created_at "
        "FROM ElleCore.dbo.messages "
        "WHERE conversation_id = ? "
        "ORDER BY id DESC;",
        { std::to_string(limit), std::to_string(convoId) });
    if (!rs.success) return false;

    for (auto it = rs.rows.rbegin(); it != rs.rows.rend(); ++it) {
        ELLE_CONVERSATION_MSG m = {};
        m.conversation_id = convoId;

        std::string roleCell = it->values.size() > 0 ? it->values[0] : std::string();
        uint32_t parsedRole = ROLE_UNKNOWN;
        if (!RoleFromStrStrict(roleCell, parsedRole)) {
            ELLE_WARN("GetConversationHistory: skipping message in conv=%llu "
                      "with unknown role='%s'",
                      (unsigned long long)convoId, roleCell.c_str());
            continue;
        }
        m.role = parsedRole;
        if (it->values.size() > 1) {
            strncpy_s(m.content, it->values[1].c_str(), ELLE_MAX_MSG - 1);
        }
        m.timestamp_ms = ELLE_MS_NOW();
        out.push_back(m);
    }
    return true;
}

bool StoreMemory(const ELLE_MEMORY_RECORD& mem) {

    uint64_t nowMs = ELLE_MS_NOW();

    auto r1 = ElleSQLPool::Instance().QueryParams(

        "INSERT INTO ElleCore.dbo.memory "
        "(memory_type, tier, content, summary, emotional_valence, importance, relevance, "
        " position_x, position_y, position_z, created_ms, last_access_ms) "
        "OUTPUT INSERTED.id "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        {
            std::to_string(mem.type),
            std::to_string(mem.tier),
            std::string(mem.content),
            std::string(mem.summary),
            std::to_string(mem.emotional_valence),
            std::to_string(mem.importance),
            std::to_string(mem.relevance),
            std::to_string(mem.position_x),
            std::to_string(mem.position_y),
            std::to_string(mem.position_z),
            std::to_string(nowMs),
            std::to_string(nowMs)
        });
    if (!r1.success) {
        ELLE_ERROR("StoreMemory INSERT failed: %s", r1.error.c_str());
        return false;
    }

    int64_t memId = 0;
    if (r1.rows.empty() || !r1.rows[0].TryGetInt(0, memId) || memId <= 0) {

        ELLE_ERROR("StoreMemory: INSERT returned no OUTPUT INSERTED.id row — "
                   "reporting failure so caller can retry/alert.");
        return false;
    }

    for (uint32_t i = 0; i < mem.tag_count && i < ELLE_MAX_TAGS; i++) {
        std::string tag = mem.tags[i];
        if (tag.empty()) continue;

        auto rt = ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.memory_tags (memory_id, tag) VALUES (?, ?);",
            { std::to_string(memId), tag });
        if (!rt.success) {
            ELLE_WARN("StoreMemory: tag insert failed mem=%lld tag=%s err=%s",
                      (long long)memId, tag.c_str(), rt.error.c_str());
        }

        auto rl = ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.memory_entity_links (memory_id, entity_id) "
            "SELECT ?, id FROM ElleCore.dbo.world_entity WHERE name = LOWER(?) "
            "AND NOT EXISTS (SELECT 1 FROM ElleCore.dbo.memory_entity_links mel "
            "                WHERE mel.memory_id = ? AND mel.entity_id = "
            "                      (SELECT id FROM ElleCore.dbo.world_entity WHERE name = LOWER(?)));",
            { std::to_string(memId), tag, std::to_string(memId), tag });
        (void)rl;
    }
    return true;
}

bool RecallMemories(const std::string& query,
                    std::vector<ELLE_MEMORY_RECORD>& out,
                    uint32_t maxCount, float minRelevance) {
    (void)minRelevance;
    if (query.empty()) return false;

    std::string like = "%" + query + "%";

    auto rs = ElleSQLPool::Instance().QueryParams(
        "WITH candidates AS ("
        "  SELECT DISTINCT m.id "
        "  FROM ElleCore.dbo.memory m "
        "  LEFT JOIN ElleCore.dbo.memory_tags mt ON mt.memory_id = m.id "
        "  LEFT JOIN ElleCore.dbo.memory_entity_links mel ON mel.memory_id = m.id "
        "  LEFT JOIN ElleCore.dbo.world_entity we ON we.id = mel.entity_id "
        "  WHERE  (LOWER(mt.tag) = LOWER(?) "
        "      OR m.content LIKE ? "
        "      OR m.summary LIKE ? "
        "      OR LOWER(we.name) = LOWER(?)) "
        "    AND m.tier < 3 "
        ") "
        "SELECT TOP (?) m.id, m.memory_type, m.tier, m.content, m.summary, "
        "       m.emotional_valence, m.importance, m.relevance, "
        "       m.position_x, m.position_y, m.position_z, "
        "       m.access_count, m.created_ms, m.last_access_ms "
        "FROM ElleCore.dbo.memory m "
        "JOIN candidates c ON c.id = m.id "
        "ORDER BY ( m.importance * 0.4 "
        "         + EXP(-CAST(DATEDIFF(MINUTE, "
        "               DATEADD(MILLISECOND, m.last_access_ms % 86400000, "
        "                       DATEADD(SECOND, m.last_access_ms / 1000, '1970-01-01')), "
        "               GETUTCDATE()) AS FLOAT) / 10080.0) * 0.4 "
        "         + (LOG(CAST(m.access_count AS FLOAT) + 1.0) / 5.0) * 0.2 "
        "       ) DESC;",
        {
            query, like, like, query,
            std::to_string(maxCount)
        });
    if (!rs.success) return false;

    for (auto& row : rs.rows) {
        ELLE_MEMORY_RECORD rec = {};
        rec.id                = (uint64_t)row.GetIntOr(0, 0);
        rec.type              = (uint32_t)row.GetIntOr(1, 0);
        rec.tier              = (uint32_t)row.GetIntOr(2, 0);
        strncpy_s(rec.content, row.values.size() > 3 ? row.values[3].c_str() : "",
                  ELLE_MAX_MSG - 1);
        strncpy_s(rec.summary, row.values.size() > 4 ? row.values[4].c_str() : "",
                  sizeof(rec.summary) - 1);
        rec.emotional_valence = (float)row.GetFloatOr(5, 0.0);
        rec.importance        = (float)row.GetFloatOr(6, 0.0);
        rec.relevance         = (float)row.GetFloatOr(7, 0.0);
        rec.position_x        = (float)row.GetFloatOr(8, 0.0);
        rec.position_y        = (float)row.GetFloatOr(9, 0.0);
        rec.position_z        = (float)row.GetFloatOr(10, 0.0);
        rec.access_count      = (uint32_t)row.GetIntOr(11, 0);
        rec.created_ms        = (uint64_t)row.GetIntOr(12, 0);
        rec.last_access_ms    = (uint64_t)row.GetIntOr(13, 0);
        out.push_back(rec);
    }

    for (auto& r : out) {
        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleCore.dbo.memory "
            "SET access_count = access_count + 1, last_access_ms = ? "
            "WHERE id = ?;",
            { std::to_string(ELLE_MS_NOW()), std::to_string(r.id) });
    }
    return true;
}

bool UpdateMemoryAccess(uint64_t memId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory "
        "SET access_count = access_count + 1, last_access_ms = ? "
        "WHERE id = ?;",
        { std::to_string(ELLE_MS_NOW()), std::to_string(memId) }).success;
}

bool RecallRecentLTM(std::vector<ELLE_MEMORY_RECORD>& out, uint32_t maxCount) {
    if (maxCount == 0) maxCount = 10;
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) m.id, m.memory_type, m.tier, m.content, m.summary, "
        "       m.emotional_valence, m.importance, m.relevance, "
        "       m.position_x, m.position_y, m.position_z, "
        "       m.access_count, m.created_ms, m.last_access_ms "
        "FROM ElleCore.dbo.memory m "
        "WHERE m.tier IN (1, 2) "
        "ORDER BY m.created_ms DESC;",
        { std::to_string(maxCount) });
    if (!rs.success) return false;

    for (auto& row : rs.rows) {
        ELLE_MEMORY_RECORD rec = {};
        rec.id                = (uint64_t)row.GetIntOr(0, 0);
        rec.type              = (uint32_t)row.GetIntOr(1, 0);
        rec.tier              = (uint32_t)row.GetIntOr(2, 0);
        strncpy_s(rec.content, row.values.size() > 3 ? row.values[3].c_str() : "",
                  ELLE_MAX_MSG - 1);
        strncpy_s(rec.summary, row.values.size() > 4 ? row.values[4].c_str() : "",
                  sizeof(rec.summary) - 1);
        rec.emotional_valence = (float)row.GetFloatOr(5, 0.0);
        rec.importance        = (float)row.GetFloatOr(6, 0.0);
        rec.relevance         = (float)row.GetFloatOr(7, 0.0);
        rec.position_x        = (float)row.GetFloatOr(8, 0.0);
        rec.position_y        = (float)row.GetFloatOr(9, 0.0);
        rec.position_z        = (float)row.GetFloatOr(10, 0.0);
        rec.access_count      = (uint32_t)row.GetIntOr(11, 0);
        rec.created_ms        = (uint64_t)row.GetIntOr(12, 0);
        rec.last_access_ms    = (uint64_t)row.GetIntOr(13, 0);
        out.push_back(rec);
    }
    return true;
}

static std::string CanonicaliseEntityName(const std::string& in) {
    std::string out; out.reserve(in.size());
    bool inSpace = false, started = false;
    for (unsigned char c : in) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (started) inSpace = true;
            continue;
        }
        if (inSpace) { out += ' '; inSpace = false; }
        out += (char)std::tolower(c);
        started = true;
    }
    return out;
}

bool GetEntity(const std::string& name, ELLE_WORLD_ENTITY& out) {
    std::string key = CanonicaliseEntityName(name);
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id, name, entity_type, description, familiarity, sentiment, "
        "             trust, interaction_count, last_interaction_ms, mental_model "
        "FROM ElleCore.dbo.world_entity "
        "WHERE name = ?;",
        { key });
    if (!rs.success || rs.rows.empty()) return false;
    auto& row = rs.rows[0];
    out.id = (uint64_t)row.GetIntOr(0, 0);
    strncpy_s(out.name,        row.values.size() > 1 ? row.values[1].c_str() : "", ELLE_MAX_NAME - 1);
    strncpy_s(out.type,        row.values.size() > 2 ? row.values[2].c_str() : "", ELLE_MAX_TAG  - 1);
    strncpy_s(out.description, row.values.size() > 3 ? row.values[3].c_str() : "", ELLE_MAX_MSG  - 1);
    out.familiarity         = (float)row.GetFloatOr(4, 0.0);
    out.sentiment           = (float)row.GetFloatOr(5, 0.0);
    out.trust               = (float)row.GetFloatOr(6, 0.0);
    out.interaction_count   = (uint32_t)row.GetIntOr(7, 0);
    out.last_interaction_ms = (uint64_t)row.GetIntOr(8, 0);
    strncpy_s(out.mental_model, row.values.size() > 9 ? row.values[9].c_str() : "", ELLE_MAX_MSG - 1);
    return true;
}

bool UpdateEntityInteraction(uint64_t entityId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.world_entity "
        "SET interaction_count = interaction_count + 1, "
        "    last_interaction_ms = ?, "
        "    familiarity = CASE WHEN familiarity + 0.02 > 1.0 THEN 1.0 "
        "                       ELSE familiarity + 0.02 END "
        "WHERE id = ?;",
        { std::to_string(ELLE_MS_NOW()), std::to_string(entityId) }).success;
}

bool GetAllEntities(std::vector<ELLE_WORLD_ENTITY>& out) {

    out.clear();
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT id, name, entity_type, description, familiarity, sentiment, "
        "       trust, interaction_count, last_interaction_ms, mental_model "
        "FROM ElleCore.dbo.world_entity ORDER BY last_interaction_ms DESC;");
    if (!rs.success) return false;
    for (auto& row : rs.rows) {
        ELLE_WORLD_ENTITY e{};
        e.id = (uint64_t)row.GetIntOr(0, 0);
        strncpy_s(e.name,        row.values.size() > 1 ? row.values[1].c_str() : "", ELLE_MAX_NAME - 1);
        strncpy_s(e.type,        row.values.size() > 2 ? row.values[2].c_str() : "", ELLE_MAX_TAG  - 1);
        strncpy_s(e.description, row.values.size() > 3 ? row.values[3].c_str() : "", ELLE_MAX_MSG  - 1);
        e.familiarity         = (float)row.GetFloatOr(4, 0.0);
        e.sentiment           = (float)row.GetFloatOr(5, 0.0);
        e.trust               = (float)row.GetFloatOr(6, 0.0);
        e.interaction_count   = (uint32_t)row.GetIntOr(7, 0);
        e.last_interaction_ms = (uint64_t)row.GetIntOr(8, 0);
        strncpy_s(e.mental_model, row.values.size() > 9 ? row.values[9].c_str() : "", ELLE_MAX_MSG - 1);
        out.push_back(e);
    }
    return true;
}

bool StoreEntity(const ELLE_WORLD_ENTITY& entity) {

    std::string lowered = CanonicaliseEntityName(entity.name);
    if (lowered.empty()) return false;
    return ElleSQLPool::Instance().QueryParams(
        "MERGE ElleCore.dbo.world_entity AS t "
        "USING (SELECT ? AS name) AS s "
        "ON (t.name = s.name) "
        "WHEN MATCHED THEN "
        "  UPDATE SET interaction_count = t.interaction_count + 1, "
        "             last_interaction_ms = ?, "
        "             familiarity = CASE WHEN t.familiarity + 0.02 > 1.0 "
        "                                THEN 1.0 ELSE t.familiarity + 0.02 END "
        "WHEN NOT MATCHED THEN "
        "  INSERT (name, display_name, entity_type, description, familiarity, sentiment, "
        "          trust, interaction_count, last_interaction_ms, mental_model) "
        "  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        {
            lowered,
            std::to_string(ELLE_MS_NOW()),
            lowered,
            std::string(entity.name),
            std::string(entity.type[0] ? entity.type : "person"),
            std::string(entity.description),
            std::to_string(entity.familiarity > 0 ? entity.familiarity : 0.1f),
            std::to_string(entity.sentiment),
            std::to_string(entity.trust > 0 ? entity.trust : 0.5f),
            std::to_string(entity.interaction_count > 0 ? entity.interaction_count : 1),
            std::to_string(ELLE_MS_NOW()),
            std::string(entity.mental_model)
        }).success;
}

bool UpdateTrust(int32_t delta, const std::string& reason) {

    auto& pool = ElleSQLPool::Instance();
    bool ok = pool.QueryParams(
        "UPDATE ElleSystem.dbo.TrustState SET Score = "
        "CASE WHEN Score + ? > 100 THEN 100 "
        "WHEN Score + ? < 0 THEN 0 "
        "ELSE Score + ? END, "
        "LastChangeMs = ? WHERE Id = 1;",
        { std::to_string(delta), std::to_string(delta), std::to_string(delta),
          std::to_string((int64_t)ELLE_MS_NOW()) }).success;
    bool ok2 = pool.QueryParams(
        "INSERT INTO ElleSystem.dbo.TrustAudit (Delta, Reason, TimestampMs) VALUES (?, ?, ?);",
        { std::to_string(delta), reason, std::to_string((int64_t)ELLE_MS_NOW()) }).success;
    return ok && ok2;
}

bool GetTrustState(ELLE_TRUST_STATE& out) {
    memset(&out, 0, sizeof(out));
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT TOP 1 Score, Level, Successes, Failures, TotalActions, Confidence "
        "FROM ElleSystem.dbo.TrustState WHERE Id = 1;");
    if (!rs.success || rs.rows.empty()) {

        out.score = 5; out.level = 0;
        out.successes = 0; out.failures = 0; out.total_actions = 0;
        return false;
    }
    auto& r = rs.rows[0];
    out.score         = (int32_t)r.GetIntOr(0, 0);
    out.level         = (uint32_t)r.GetIntOr(1, 0);
    out.successes     = (uint32_t)r.GetIntOr(2, 0);
    out.failures      = (uint32_t)r.GetIntOr(3, 0);
    out.total_actions = (uint32_t)r.GetIntOr(4, 0);
    return true;
}

bool UpsertPairedDevice(const PairedDeviceRow& row) {

    return ElleSQLPool::Instance().QueryParams(
        "MERGE ElleCore.dbo.PairedDevices WITH (HOLDLOCK) AS t "
        "USING (SELECT ? AS DeviceId) AS s ON t.DeviceId = s.DeviceId "
        "WHEN MATCHED THEN "
        "  UPDATE SET DeviceName = ?, PairedAtMs = ?, ExpiresMs = ?, "
        "             Revoked = 0, RevokedAtMs = NULL, "
        "             JwtFingerprint = ? "
        "WHEN NOT MATCHED THEN "
        "  INSERT (DeviceId, DeviceName, PairedAtMs, ExpiresMs, "
        "          Revoked, JwtFingerprint) "
        "  VALUES (?, ?, ?, ?, 0, ?);",
        {
            row.device_id,
            row.device_name,
            std::to_string((int64_t)row.paired_at_ms),
            std::to_string((int64_t)row.expires_ms),
            row.jwt_fingerprint,
            row.device_id,
            row.device_name,
            std::to_string((int64_t)row.paired_at_ms),
            std::to_string((int64_t)row.expires_ms),
            row.jwt_fingerprint
        }).success;
}

bool GetPairedDevice(const std::string& device_id, PairedDeviceRow& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT DeviceId, DeviceName, PairedAtMs, ExpiresMs, "
        "       ISNULL(LastSeenMs, 0), Revoked, ISNULL(RevokedAtMs, 0), "
        "       JwtFingerprint "
        "FROM ElleCore.dbo.PairedDevices WHERE DeviceId = ?;",
        { device_id });
    if (!rs.success || rs.rows.empty()) return false;
    auto& r = rs.rows[0];
    out.device_id       = r[0];
    out.device_name     = r[1];
    out.paired_at_ms    = (uint64_t)r.GetIntOr(2, 0);
    out.expires_ms      = (uint64_t)r.GetIntOr(3, 0);
    out.last_seen_ms    = (uint64_t)r.GetIntOr(4, 0);
    out.revoked         = r.GetIntOr(5, 0) != 0;
    out.revoked_at_ms   = (uint64_t)r.GetIntOr(6, 0);
    out.jwt_fingerprint = r[7];
    return true;
}

bool ListPairedDevices(std::vector<PairedDeviceRow>& out, uint32_t limit) {
    out.clear();
    if (limit == 0) limit = 50;
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) DeviceId, DeviceName, PairedAtMs, ExpiresMs, "
        "       ISNULL(LastSeenMs, 0), Revoked, ISNULL(RevokedAtMs, 0), "
        "       JwtFingerprint "
        "FROM ElleCore.dbo.PairedDevices ORDER BY PairedAtMs DESC;",
        { std::to_string(limit) });
    if (!rs.success) return false;
    out.reserve(rs.rows.size());
    for (auto& r : rs.rows) {
        PairedDeviceRow row;
        row.device_id       = r[0];
        row.device_name     = r[1];
        row.paired_at_ms    = (uint64_t)r.GetIntOr(2, 0);
        row.expires_ms      = (uint64_t)r.GetIntOr(3, 0);
        row.last_seen_ms    = (uint64_t)r.GetIntOr(4, 0);
        row.revoked         = r.GetIntOr(5, 0) != 0;
        row.revoked_at_ms   = (uint64_t)r.GetIntOr(6, 0);
        row.jwt_fingerprint = r[7];
        out.push_back(std::move(row));
    }
    return true;
}

bool RevokePairedDevice(const std::string& device_id) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.PairedDevices "
        "SET Revoked = 1, RevokedAtMs = ? WHERE DeviceId = ?;",
        { std::to_string((int64_t)ELLE_MS_NOW()), device_id }).success;
}

bool TouchPairedDeviceLastSeen(const std::string& device_id) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.PairedDevices SET LastSeenMs = ? "
        "WHERE DeviceId = ? AND Revoked = 0;",
        { std::to_string((int64_t)ELLE_MS_NOW()), device_id }).success;
}

static void EnsureSessionsTable() {

    static std::once_flag s_ddlOnce;
    std::call_once(s_ddlOnce, []() {
        auto rs = ElleSQLPool::Instance().Query(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables "
            "               WHERE name = 'Sessions' AND schema_id = SCHEMA_ID('dbo')) "
            "BEGIN "
            "  CREATE TABLE ElleCore.dbo.Sessions ("
            "    Token       NVARCHAR(64)  NOT NULL PRIMARY KEY, "
            "    nUserNo     BIGINT        NOT NULL, "
            "    sUserID     NVARCHAR(30)  NOT NULL, "
            "    sUserName   NVARCHAR(60)  NULL, "
            "    nAuthID     INT           NOT NULL DEFAULT 0, "
            "    CreatedMs   BIGINT        NOT NULL, "
            "    LastSeenMs  BIGINT        NOT NULL, "
            "    DeviceName  NVARCHAR(128) NULL, "
            "    PeerAddr    NVARCHAR(64)  NULL "
            "  ); "
            "  CREATE INDEX IX_Sessions_nUserNo ON ElleCore.dbo.Sessions(nUserNo); "
            "END");
        if (!rs.success) {
            ELLE_WARN("Sessions table auto-create check failed: %s",
                      rs.error.c_str());
        }
    });
}

bool CreateSession(const SessionRow& row) {
    if (row.token.empty() || row.nUserNo <= 0) {
        ELLE_WARN("CreateSession refused: token.empty=%d nUserNo=%lld",
                  (int)row.token.empty(), (long long)row.nUserNo);
        return false;
    }
    EnsureSessionsTable();
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(
        "INSERT INTO ElleCore.dbo.Sessions "
        "(Token, nUserNo, sUserID, sUserName, nAuthID, CreatedMs, LastSeenMs, DeviceName, PeerAddr) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);",
        {
            row.token,
            std::to_string(row.nUserNo),
            row.sUserID,
            row.sUserName,
            std::to_string((int64_t)row.nAuthID),
            std::to_string((int64_t)row.created_ms),
            std::to_string((int64_t)row.last_seen_ms),
            row.device_name,
            row.peer_addr
        });
    if (!rs.success) {
        ELLE_WARN("Sessions INSERT failed for nUserNo=%lld sUserID=\"%s\": %s",
                  (long long)row.nUserNo, row.sUserID.c_str(),
                  rs.error.c_str());
        return false;
    }
    return true;
}

bool GetSessionByToken(const std::string& token, SessionRow& out) {
    if (token.empty() || token.size() > 64) return false;
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(
        "SELECT TOP 1 Token, nUserNo, sUserID, sUserName, nAuthID, "
        "       CONVERT(BIGINT, CreatedMs)  AS CreatedMs, "
        "       CONVERT(BIGINT, LastSeenMs) AS LastSeenMs, "
        "       ISNULL(DeviceName,'') AS DeviceName, "
        "       ISNULL(PeerAddr,'')   AS PeerAddr "
        "  FROM ElleCore.dbo.Sessions WHERE Token = ?;",
        { token });
    if (!rs.success || rs.rows.empty()) return false;
    const auto& r = rs.rows[0];
    int64_t v = 0;
    out.token      = r.values.size() > 0 ? r[0] : "";
    out.nUserNo    = r.TryGetInt(1, v) ? v : 0;
    out.sUserID    = r.values.size() > 2 ? r[2] : "";
    out.sUserName  = r.values.size() > 3 ? r[3] : "";
    out.nAuthID    = (int32_t)r.GetIntOr(4, 0);
    int64_t cms = 0, lms = 0;
    r.TryGetInt(5, cms); r.TryGetInt(6, lms);
    out.created_ms    = (uint64_t)cms;
    out.last_seen_ms  = (uint64_t)lms;
    out.device_name   = r.values.size() > 7 ? r[7] : "";
    out.peer_addr     = r.values.size() > 8 ? r[8] : "";
    return out.nUserNo > 0;
}

bool TouchSessionLastSeen(const std::string& token) {
    if (token.empty()) return false;
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.Sessions SET LastSeenMs = ? WHERE Token = ?;",
        { std::to_string((int64_t)ELLE_MS_NOW()), token }).success;
}

bool DeleteSession(const std::string& token) {
    if (token.empty()) return true;
    return ElleSQLPool::Instance().QueryParams(
        "DELETE FROM ElleCore.dbo.Sessions WHERE Token = ?;",
        { token }).success;
}

int DeleteSessionsForUser(int64_t nUserNo) {
    if (nUserNo <= 0) return 0;
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(
        "DELETE FROM ElleCore.dbo.Sessions WHERE nUserNo = ?;",
        { std::to_string(nUserNo) });
    return rs.success ? (int)rs.rows_affected : 0;
}

bool ListSessions(std::vector<SessionRow>& out, uint32_t limit) {
    if (limit == 0) limit = 50;
    if (limit > 500) limit = 500;
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.Query(
        "SELECT TOP " + std::to_string(limit) + " "
        "       Token, nUserNo, sUserID, sUserName, nAuthID, "
        "       CONVERT(BIGINT, CreatedMs)  AS CreatedMs, "
        "       CONVERT(BIGINT, LastSeenMs) AS LastSeenMs, "
        "       ISNULL(DeviceName,'') AS DeviceName, "
        "       ISNULL(PeerAddr,'')   AS PeerAddr "
        "  FROM ElleCore.dbo.Sessions ORDER BY LastSeenMs DESC;");
    if (!rs.success) return false;
    out.clear();
    out.reserve(rs.rows.size());
    for (auto& r : rs.rows) {
        SessionRow s;
        int64_t v = 0;
        s.token     = r.values.size() > 0 ? r[0] : "";
        s.nUserNo   = r.TryGetInt(1, v) ? v : 0;
        s.sUserID   = r.values.size() > 2 ? r[2] : "";
        s.sUserName = r.values.size() > 3 ? r[3] : "";
        s.nAuthID   = (int32_t)r.GetIntOr(4, 0);
        int64_t cms = 0, lms = 0;
        r.TryGetInt(5, cms); r.TryGetInt(6, lms);
        s.created_ms   = (uint64_t)cms;
        s.last_seen_ms = (uint64_t)lms;
        s.device_name  = r.values.size() > 7 ? r[7] : "";
        s.peer_addr    = r.values.size() > 8 ? r[8] : "";
        out.push_back(std::move(s));
    }
    return true;
}

bool RegisterWorker(ELLE_SERVICE_ID svc, const std::string& name) {
    auto& pool = ElleSQLPool::Instance();
    return pool.QueryParams(
        "IF NOT EXISTS (SELECT 1 FROM ElleSystem.dbo.Workers WHERE ServiceId = ?) "
        "INSERT INTO ElleSystem.dbo.Workers "
        "(ServiceId, Name, Running, Healthy, StartedMs) VALUES (?, ?, 1, 1, ?) "
        "ELSE UPDATE ElleSystem.dbo.Workers SET Running = 1, Healthy = 1, "
        "LastHeartbeatMs = ? WHERE ServiceId = ?;",
        { std::to_string((int)svc), std::to_string((int)svc), name,
          std::to_string((int64_t)ELLE_MS_NOW()),
          std::to_string((int64_t)ELLE_MS_NOW()), std::to_string((int)svc) }).success;
}

bool UpdateWorkerHeartbeat(ELLE_SERVICE_ID svc) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleSystem.dbo.Workers SET LastHeartbeatMs = ?, Healthy = 1 "
        "WHERE ServiceId = ?;",
        { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string((int)svc) }).success;
}

bool WriteLog(ELLE_LOG_LEVEL level, ELLE_SERVICE_ID svc, const std::string& msg) {
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleSystem.dbo.Logs (Level, ServiceId, Message, TimestampMs) "
        "VALUES (?, ?, ?, ?);",
        { std::to_string((int)level), std::to_string((int)svc),
          msg, std::to_string((int64_t)ELLE_MS_NOW()) }).success;
}

bool GetRecentLogs(std::vector<ELLE_LOG_ENTRY>& out, uint32_t count,
                   ELLE_LOG_LEVEL minLevel) {
    out.clear();
    if (count == 0) count = 100;
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) Level, ServiceId, TimestampMs, Message "
        "FROM ElleSystem.dbo.Logs "
        "WHERE Level >= ? "
        "ORDER BY TimestampMs DESC;",
        { std::to_string(count), std::to_string((int)minLevel) });
    if (!rs.success) return false;
    out.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        ELLE_LOG_ENTRY e = {};
        e.level        = (uint32_t)row.GetIntOr(0, 0);
        e.source_svc   = (uint32_t)row.GetIntOr(1, 0);
        e.timestamp_ms = (uint64_t)row.GetIntOr(2, 0);
        const std::string m = row.values.size() > 3 ? row.values[3] : std::string();
        strncpy_s(e.message, m.c_str(), ELLE_MAX_MSG - 1);
        out.push_back(e);
    }
    return true;
}

bool GetWorkerStatuses(std::vector<ELLE_SERVICE_STATUS>& out) {
    out.clear();

    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT ServiceId, Name, ISNULL(Running, 0), ISNULL(Healthy, 0), "
        "       CASE WHEN StartedMs IS NULL OR StartedMs = 0 THEN 0 "
        "            ELSE ? - StartedMs END AS uptime_ms, "
        "       ISNULL(LastHeartbeatMs, 0), "
        "       ISNULL(MessagesProcessed, 0), ISNULL(Errors, 0), "
        "       ISNULL(CpuPercent, 0.0), ISNULL(MemoryBytes, 0), "
        "       ISNULL(ThreadCount, 0), ISNULL(StatusText, N'') "
        "FROM ElleSystem.dbo.Workers;",
        { std::to_string((int64_t)ELLE_MS_NOW()) });
    if (!rs.success) return false;
    out.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        ELLE_SERVICE_STATUS s = {};
        s.service_id         = (uint32_t)row.GetIntOr(0, 0);
        const std::string nm = row.values.size() > 1 ? row.values[1] : std::string();
        strncpy_s(s.name, nm.c_str(), ELLE_MAX_NAME - 1);
        s.running            = (uint32_t)row.GetIntOr(2, 0);
        s.healthy            = (uint32_t)row.GetIntOr(3, 0);
        s.uptime_ms          = (uint64_t)row.GetIntOr(4, 0);
        s.last_heartbeat_ms  = (uint64_t)row.GetIntOr(5, 0);
        s.messages_processed = (uint32_t)row.GetIntOr(6, 0);
        s.errors             = (uint32_t)row.GetIntOr(7, 0);
        s.cpu_percent        = (float)row.GetFloatOr(8, 0.0);
        s.memory_bytes       = (uint64_t)row.GetIntOr(9, 0);
        s.thread_count       = (uint32_t)row.GetIntOr(10, 0);
        const std::string st = row.values.size() > 11 ? row.values[11] : std::string();
        strncpy_s(s.status_text, st.c_str(), sizeof(s.status_text) - 1);
        out.push_back(s);
    }
    return true;
}

static void EnsureGoalsTable() {
    ElleSQLPool::Instance().Exec(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'goals') "
        "CREATE TABLE ElleCore.dbo.goals ("
        "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  description     NVARCHAR(MAX) NOT NULL,"
        "  status          INT NOT NULL DEFAULT 0,"
        "  priority        INT NOT NULL DEFAULT 2,"
        "  progress        FLOAT NOT NULL DEFAULT 0.0,"
        "  motivation      FLOAT NOT NULL DEFAULT 0.7,"
        "  source_drive    INT NOT NULL DEFAULT 0,"
        "  parent_goal_id  BIGINT NULL,"
        "  success_criteria NVARCHAR(MAX) NULL,"
        "  created_ms      BIGINT NOT NULL,"
        "  deadline_ms     BIGINT NULL,"
        "  last_progress_ms BIGINT NULL,"
        "  attempts        INT NOT NULL DEFAULT 0"
        ");");
}

bool StoreGoal(const ELLE_GOAL_RECORD& goal) {
    return StoreGoalReturningId(goal) != 0;
}

uint64_t StoreGoalReturningId(const ELLE_GOAL_RECORD& goal) {
    EnsureGoalsTable();

    auto rs = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.goals "
        "(description, status, priority, progress, motivation, source_drive, "
        " parent_goal_id, success_criteria, created_ms, deadline_ms, attempts) "
        "OUTPUT INSERTED.id "
        "VALUES (?, ?, ?, ?, ?, ?, NULLIF(?, '0'), ?, ?, NULLIF(?, '0'), ?);",
        {
            std::string(goal.description),
            std::to_string(goal.status),
            std::to_string(goal.priority),
            std::to_string(goal.progress),
            std::to_string(goal.motivation),
            std::to_string(goal.source_drive),
            std::to_string(goal.parent_goal_id),
            std::string(goal.success_criteria),
            std::to_string((int64_t)ELLE_MS_NOW()),
            std::to_string((int64_t)goal.deadline_ms),
            std::to_string(goal.attempts)
        });
    if (!rs.success || rs.rows.empty()) return 0;
    return (uint64_t)rs.rows[0].GetIntOr(0, 0);
}

bool UpdateGoalProgress(uint64_t goalId, float progress) {
    EnsureGoalsTable();
    bool finished = progress >= 1.0f;
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.goals "
        "SET progress = ?, last_progress_ms = ?, "
        "    status = CASE WHEN ? >= 1.0 THEN 2 ELSE status END "
        "WHERE id = ?;",
        {
            std::to_string(progress),
            std::to_string((int64_t)ELLE_MS_NOW()),
            std::to_string(progress),
            std::to_string((int64_t)goalId)
        }).success && (finished || true);
}

bool UpdateGoalStatus(uint64_t goalId, uint32_t status) {
    EnsureGoalsTable();
    auto rs = ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.goals SET status = ?, last_progress_ms = ? WHERE id = ?;",
        {
            std::to_string((int)status),
            std::to_string((int64_t)ELLE_MS_NOW()),
            std::to_string((int64_t)goalId)
        });
    if (!rs.success) {
        ELLE_ERROR("UpdateGoalStatus(%llu -> %u) failed: %s",
                   (unsigned long long)goalId, (unsigned)status, rs.error.c_str());
    }
    return rs.success;
}

bool GetActiveGoals(std::vector<ELLE_GOAL_RECORD>& out) {
    out.clear();
    EnsureGoalsTable();
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT id, description, status, priority, progress, motivation, "
        "       source_drive, ISNULL(parent_goal_id,0), "
        "       ISNULL(success_criteria, N''), created_ms, "
        "       ISNULL(deadline_ms, 0), attempts "
        "FROM ElleCore.dbo.goals "
        "WHERE status IN (0, 1) "
        "ORDER BY priority ASC, created_ms DESC;");
    if (!rs.success) return false;
    for (auto& row : rs.rows) {
        ELLE_GOAL_RECORD g{};
        g.id             = (uint64_t)row.GetIntOr(0, 0);
        std::string desc = row.values.size() > 1 ? row.values[1] : std::string();
        strncpy_s(g.description, desc.c_str(), ELLE_MAX_MSG - 1);
        g.status         = (uint32_t)row.GetIntOr(2, 0);
        g.priority       = (uint32_t)row.GetIntOr(3, 0);
        g.progress       = (float)row.GetFloatOr(4, 0.0);
        g.motivation     = (float)row.GetFloatOr(5, 0.0);
        g.source_drive   = (uint32_t)row.GetIntOr(6, 0);
        g.parent_goal_id = (uint32_t)row.GetIntOr(7, 0);
        std::string crit = row.values.size() > 8 ? row.values[8] : std::string();
        strncpy_s(g.success_criteria, crit.c_str(), ELLE_MAX_MSG - 1);
        g.created_ms     = (uint64_t)row.GetIntOr(9, 0);
        g.deadline_ms    = (uint64_t)row.GetIntOr(10, 0);
        g.attempts       = (uint32_t)row.GetIntOr(11, 0);
        out.push_back(g);
    }
    return true;
}

}
