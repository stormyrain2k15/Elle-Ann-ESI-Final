#include "ElleUserContinuity.h"
#include "ElleSQLConn.h"
#include "ElleLogger.h"
#include "ElleTypes.h"

#include <chrono>

namespace ElleDB {

bool TouchUserContinuityOnPair(int64_t nUserNo,
                               const std::string& sUserID,
                               const std::string& sUserName) {
    if (nUserNo <= 0) return false;
    const uint64_t now = (uint64_t)ELLE_MS_NOW();

    static constexpr const char* kSql =
        "MERGE ElleCore.dbo.UserContinuity AS t "
        "USING (SELECT ? AS nUserNo, ? AS sUserID, ? AS sUserName, ? AS now_ms) AS s "
        "   ON t.nUserNo = s.nUserNo "
        "WHEN MATCHED THEN UPDATE SET "
        "       sUserID_cached = s.sUserID, "
        "       sUserName_cached = s.sUserName, "
        "       last_seen_ms = s.now_ms, "
        "       total_pairings = t.total_pairings + 1, "
        "       updated_ms = s.now_ms "
        "WHEN NOT MATCHED THEN INSERT "
        "      (nUserNo, sUserID_cached, sUserName_cached, "
        "       first_met_ms, last_seen_ms, total_pairings, "
        "       created_ms, updated_ms) "
        "  VALUES (s.nUserNo, s.sUserID, s.sUserName, "
        "          s.now_ms, s.now_ms, 1, s.now_ms, s.now_ms);";

    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, {
        std::to_string(nUserNo), sUserID, sUserName, std::to_string(now)
    });
    if (!rs.success) {
        ELLE_WARN("TouchUserContinuityOnPair(%lld): %s",
                  (long long)nUserNo, rs.error.c_str());
        return false;
    }
    return true;
}

bool UpdateUserBond(int64_t nUserNo, double bondScore,
                    const std::string& bondLabel) {
    if (nUserNo <= 0) return false;
    const uint64_t now = (uint64_t)ELLE_MS_NOW();
    static constexpr const char* kSql =
        "UPDATE ElleCore.dbo.UserContinuity "
        "   SET last_bond_score    = ?, "
        "       last_bond_label    = ?, "
        "       last_bond_updated_ms = ?, "
        "       updated_ms         = ? "
        " WHERE nUserNo = ?;";
    char score_str[32];
    std::snprintf(score_str, sizeof(score_str), "%.6f", bondScore);
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, {
        score_str, bondLabel,
        std::to_string(now), std::to_string(now),
        std::to_string(nUserNo)
    });
    return rs.success;
}

bool AppendUserNote(int64_t nUserNo, const std::string& note) {
    if (nUserNo <= 0 || note.empty()) return false;

    UserContinuityRow row;
    if (!GetUserContinuity(nUserNo, row)) return false;
    std::string merged = row.private_note;
    if (!merged.empty()) merged += "\n";
    merged += note;

    if (merged.size() > 4000) {
        merged.erase(0, merged.size() - 4000);
    }
    static constexpr const char* kSql =
        "UPDATE ElleCore.dbo.UserContinuity "
        "   SET private_note = ?, updated_ms = ? "
        " WHERE nUserNo = ?;";
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, {
        merged, std::to_string((uint64_t)ELLE_MS_NOW()),
        std::to_string(nUserNo)
    });
    return rs.success;
}

bool GetUserContinuity(int64_t nUserNo, UserContinuityRow& out) {
    if (nUserNo <= 0) return false;
    static constexpr const char* kSql =
        "SELECT nUserNo, ISNULL(sUserID_cached,''), ISNULL(sUserName_cached,''), "
        "       first_met_ms, last_seen_ms, "
        "       ISNULL(total_conversations,0), ISNULL(total_messages,0), "
        "       ISNULL(total_pairings,0), "
        "       ISNULL(last_bond_score,0), ISNULL(last_bond_label,''), "
        "       ISNULL(last_bond_updated_ms,0), ISNULL(private_note,''), "
        "       created_ms, updated_ms "
        "  FROM ElleCore.dbo.UserContinuity WHERE nUserNo = ?;";
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, { std::to_string(nUserNo) });
    if (!rs.success || rs.rows.empty()) return false;
    const auto& r = rs.rows[0];
    int64_t v = 0;
    out.nUserNo            = r.TryGetInt(0, v) ? v : 0;
    out.sUserID_cached     = r[1];
    out.sUserName_cached   = r[2];
    out.first_met_ms       = (uint64_t)r.GetIntOr(3, 0);
    out.last_seen_ms       = (uint64_t)r.GetIntOr(4, 0);
    out.total_conversations = (int32_t)r.GetIntOr(5, 0);
    out.total_messages     = (int32_t)r.GetIntOr(6, 0);
    out.total_pairings     = (int32_t)r.GetIntOr(7, 0);
    out.last_bond_score    = r.GetFloatOr(8, 0.0);
    out.last_bond_label    = r[9];
    out.last_bond_updated_ms = (uint64_t)r.GetIntOr(10, 0);
    out.private_note       = r[11];
    out.created_ms         = (uint64_t)r.GetIntOr(12, 0);
    out.updated_ms         = (uint64_t)r.GetIntOr(13, 0);
    return true;
}

bool UpsertGameSession(const GameSessionStateRow& row) {
    if (row.nUserNo <= 0) return false;
    static constexpr const char* kSql =
        "MERGE ElleCore.dbo.GameSessionState AS t "
        "USING (SELECT ? AS nUserNo) AS s ON t.nUserNo = s.nUserNo "
        "WHEN MATCHED THEN UPDATE SET "
        "       char_index = ?, char_name = ?, "
        "       zone_id = ?, zone_name = ?, "
        "       last_x = ?, last_y = ?, last_z = ?, "
        "       last_hp = ?, last_hp_max = ?, "
        "       last_session_ms = ?, "
        "       last_disconnect_ms = NULL, "
        "       last_disconnect_reason = NULL "
        "WHEN NOT MATCHED THEN INSERT "
        "      (nUserNo, char_index, char_name, zone_id, zone_name, "
        "       last_x, last_y, last_z, last_hp, last_hp_max, "
        "       last_session_ms) "
        "  VALUES (s.nUserNo, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    char xs[32], ys[32], zs[32];
    std::snprintf(xs, sizeof(xs), "%.6f", row.last_x);
    std::snprintf(ys, sizeof(ys), "%.6f", row.last_y);
    std::snprintf(zs, sizeof(zs), "%.6f", row.last_z);

    std::vector<std::string> p = {
        std::to_string(row.nUserNo),

        std::to_string(row.char_index), row.char_name,
        std::to_string(row.zone_id),    row.zone_name,
        xs, ys, zs,
        std::to_string(row.last_hp), std::to_string(row.last_hp_max),
        std::to_string(row.last_session_ms),

        std::to_string(row.char_index), row.char_name,
        std::to_string(row.zone_id),    row.zone_name,
        xs, ys, zs,
        std::to_string(row.last_hp), std::to_string(row.last_hp_max),
        std::to_string(row.last_session_ms)
    };
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, p);
    return rs.success;
}

bool MarkGameSessionDisconnected(int64_t nUserNo,
                                 const std::string& reason) {
    if (nUserNo <= 0) return false;
    static constexpr const char* kSql =
        "UPDATE ElleCore.dbo.GameSessionState "
        "   SET last_disconnect_ms = ?, "
        "       last_disconnect_reason = ? "
        " WHERE nUserNo = ?;";
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, {
        std::to_string((uint64_t)ELLE_MS_NOW()),
        reason.substr(0, 64),
        std::to_string(nUserNo)
    });
    return rs.success;
}

bool GetGameSession(int64_t nUserNo, GameSessionStateRow& out) {
    if (nUserNo <= 0) return false;
    static constexpr const char* kSql =
        "SELECT nUserNo, ISNULL(char_index,-1), ISNULL(char_name,''), "
        "       ISNULL(zone_id,-1), ISNULL(zone_name,''), "
        "       ISNULL(last_x,0), ISNULL(last_y,0), ISNULL(last_z,0), "
        "       ISNULL(last_hp,0), ISNULL(last_hp_max,0), "
        "       last_session_ms, ISNULL(last_disconnect_ms,0), "
        "       ISNULL(last_disconnect_reason,'') "
        "  FROM ElleCore.dbo.GameSessionState WHERE nUserNo = ?;";
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.QueryParams(kSql, { std::to_string(nUserNo) });
    if (!rs.success || rs.rows.empty()) return false;
    const auto& r = rs.rows[0];
    int64_t v = 0;
    out.nUserNo            = r.TryGetInt(0, v) ? v : 0;
    out.char_index         = (int32_t)r.GetIntOr(1, -1);
    out.char_name          = r[2];
    out.zone_id            = (int32_t)r.GetIntOr(3, -1);
    out.zone_name          = r[4];
    out.last_x             = r.GetFloatOr(5, 0.0);
    out.last_y             = r.GetFloatOr(6, 0.0);
    out.last_z             = r.GetFloatOr(7, 0.0);
    out.last_hp            = (int32_t)r.GetIntOr(8, 0);
    out.last_hp_max        = (int32_t)r.GetIntOr(9, 0);
    out.last_session_ms    = (uint64_t)r.GetIntOr(10, 0);
    out.last_disconnect_ms = (uint64_t)r.GetIntOr(11, 0);
    out.last_disconnect_reason = r[12];
    return true;
}

}
