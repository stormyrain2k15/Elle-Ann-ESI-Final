#include "ElleBeliefAdmin.h"
#include "ElleSQLConn.h"
#include "ElleLogger.h"

#include <sstream>

namespace ElleBeliefAdmin {

static std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        if (c == '\'') out += "''";
        else out.push_back(c);
    }
    return out;
}

bool FetchBeliefAudit(std::vector<BeliefAuditRow>& out,
                      const std::string& domainCode,
                      long long sinceMs,
                      int        limit) {
    out.clear();
    auto& pool = ElleSQLPool::Instance();

    const int capped = (limit <= 0 || limit > 5000) ? 200 : limit;

    std::ostringstream sql;
    sql << "SELECT TOP (" << capped << ") "
        << "    a.audit_id, a.domain_id, d.domain_code, "
        << "    a.operation, a.evidence_count, "
        << "    a.entropy_before, a.entropy_after, "
        << "    ISNULL(a.map_hypothesis_id, 0), ISNULL(a.map_probability, 0.0), "
        << "    a.recorded_ms, ISNULL(a.detail, N'') "
        << "FROM ElleHeart.dbo.belief_audit AS a "
        << "JOIN ElleHeart.dbo.belief_domain AS d ON d.domain_id = a.domain_id "
        << "WHERE a.recorded_ms >= " << sinceMs;

    if (!domainCode.empty()) {
        sql << " AND d.domain_code = N'" << escape(domainCode) << "'";
    }
    sql << " ORDER BY a.recorded_ms DESC;";

    auto rs = pool.Query(sql.str());
    if (!rs.success) {
        ELLE_ERROR("BeliefAdmin: belief_audit query failed");
        return false;
    }

    out.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        BeliefAuditRow r;
        r.auditId          = row.GetIntOr(0, 0);
        r.domainId         = row.GetIntOr(1, 0);
        r.domainCode       = row.values.size() > 2 ? row.values[2] : std::string();
        r.operation        = row.values.size() > 3 ? row.values[3] : std::string();
        r.evidenceCount    = (int)row.GetIntOr(4, 0);
        r.entropyBefore    = row.GetFloatOr(5, 0.0);
        r.entropyAfter     = row.GetFloatOr(6, 0.0);
        r.mapHypothesisId  = row.GetIntOr(7, 0);
        r.mapProbability   = row.GetFloatOr(8, 0.0);
        r.recordedMs       = row.GetIntOr(9, 0);
        r.detail           = row.values.size() > 10 ? row.values[10] : std::string();
        out.push_back(std::move(r));
    }
    return true;
}

bool FetchBeliefSnapshot(std::vector<BeliefSnapshotRow>& out,
                         const std::string& domainCode) {
    out.clear();
    auto& pool = ElleSQLPool::Instance();

    std::ostringstream sql;
    sql << "SELECT domain_id, domain_code, half_life_secs, last_updated_ms, "
        << "       hypothesis_id, posterior_mass, ISNULL(prior_mass, 0.0) "
        << "FROM ElleHeart.dbo.vw_BeliefSnapshot";
    if (!domainCode.empty()) {
        sql << " WHERE domain_code = N'" << escape(domainCode) << "'";
    }
    sql << " ORDER BY domain_code, hypothesis_id;";

    auto rs = pool.Query(sql.str());
    if (!rs.success) {
        ELLE_ERROR("BeliefAdmin: vw_BeliefSnapshot query failed");
        return false;
    }

    out.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        BeliefSnapshotRow r;
        r.domainId       = row.GetIntOr(0, 0);
        r.domainCode     = row.values.size() > 1 ? row.values[1] : std::string();
        r.halfLifeSecs   = row.GetFloatOr(2, 0.0);
        r.lastUpdatedMs  = row.GetIntOr(3, 0);
        r.hypothesisId   = row.GetIntOr(4, 0);
        r.posteriorMass  = row.GetFloatOr(5, 0.0);
        r.priorMass      = row.GetFloatOr(6, 0.0);
        out.push_back(std::move(r));
    }
    return true;
}

nlohmann::json BeliefAuditToJson(const std::vector<BeliefAuditRow>& rows) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : rows) {
        nlohmann::json o;
        o["audit_id"]           = r.auditId;
        o["domain_id"]          = r.domainId;
        o["domain_code"]        = r.domainCode;
        o["operation"]          = r.operation;
        o["evidence_count"]     = r.evidenceCount;
        o["entropy_before"]     = r.entropyBefore;
        o["entropy_after"]      = r.entropyAfter;
        o["map_hypothesis_id"]  = r.mapHypothesisId;
        o["map_probability"]    = r.mapProbability;
        o["recorded_ms"]        = r.recordedMs;
        o["detail"]             = r.detail;
        arr.push_back(std::move(o));
    }
    nlohmann::json out;
    out["rows"] = std::move(arr);
    return out;
}

nlohmann::json BeliefSnapshotToJson(const std::vector<BeliefSnapshotRow>& rows) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : rows) {
        nlohmann::json o;
        o["domain_id"]        = r.domainId;
        o["domain_code"]      = r.domainCode;
        o["half_life_secs"]   = r.halfLifeSecs;
        o["last_updated_ms"]  = r.lastUpdatedMs;
        o["hypothesis_id"]    = r.hypothesisId;
        o["posterior_mass"]   = r.posteriorMass;
        o["prior_mass"]       = r.priorMass;
        arr.push_back(std::move(o));
    }
    nlohmann::json out;
    out["rows"] = std::move(arr);
    return out;
}

bool FetchBondingDashboard(BondingDashboardRow& out) {
    out = BondingDashboardRow{};
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.Query(
        "SELECT TOP 1 updated_ms, intimacy, passion, commitment, security, "
        "anxiety, avoidance, felt_understood, felt_cared_for, investment, "
        "total_interactions, meaningful_conversations, "
        "conflicts_experienced, conflicts_resolved, repair_motivation, "
        "CAST(unresolved_tension AS INT), "
        "affection_index, commitment_index, distress_index, "
        "meaningful_ratio, conflict_resolution_ratio "
        "FROM ElleHeart.dbo.vw_RelationshipDashboard WHERE id = 1;");
    if (!rs.success || rs.rows.empty()) {
        ELLE_ERROR("BondingAdmin: vw_RelationshipDashboard query failed or empty");
        return false;
    }
    auto& row = rs.rows[0];
    out.updatedMs               = row.GetIntOr(0, 0);
    out.intimacy                = row.GetFloatOr(1, 0.0);
    out.passion                 = row.GetFloatOr(2, 0.0);
    out.commitment              = row.GetFloatOr(3, 0.0);
    out.security                = row.GetFloatOr(4, 0.0);
    out.anxiety                 = row.GetFloatOr(5, 0.0);
    out.avoidance               = row.GetFloatOr(6, 0.0);
    out.feltUnderstood          = row.GetFloatOr(7, 0.0);
    out.feltCaredFor            = row.GetFloatOr(8, 0.0);
    out.investment              = row.GetFloatOr(9, 0.0);
    out.totalInteractions       = (int)row.GetIntOr(10, 0);
    out.meaningfulConversations = (int)row.GetIntOr(11, 0);
    out.conflictsExperienced    = (int)row.GetIntOr(12, 0);
    out.conflictsResolved       = (int)row.GetIntOr(13, 0);
    out.repairMotivation        = row.GetFloatOr(14, 0.0);
    out.unresolvedTension       = row.GetIntOr(15, 0) != 0;
    out.affectionIndex          = row.GetFloatOr(16, 0.0);
    out.commitmentIndex         = row.GetFloatOr(17, 0.0);
    out.distressIndex           = row.GetFloatOr(18, 0.0);
    out.meaningfulRatio         = row.GetFloatOr(19, 0.0);
    out.conflictResolutionRatio = row.GetFloatOr(20, 0.0);
    return true;
}

bool FetchBondingTrajectory(std::vector<BondingTrajectoryPoint>& out, int limit) {
    out.clear();
    const int capped = (limit <= 0 || limit > 1000) ? 100 : limit;
    auto& pool = ElleSQLPool::Instance();
    std::ostringstream sql;
    sql << "SELECT TOP (" << capped << ") "
        << "history_id, snapshot_ms, snapshot_reason, "
        << "affection_index, distress_index, intimacy, security, repair_motivation "
        << "FROM ElleHeart.dbo.vw_RelationshipTrajectory "
        << "ORDER BY snapshot_ms DESC;";
    auto rs = pool.Query(sql.str());
    if (!rs.success) {
        ELLE_ERROR("BondingAdmin: vw_RelationshipTrajectory query failed");
        return false;
    }
    out.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        BondingTrajectoryPoint p;
        p.historyId        = row.GetIntOr(0, 0);
        p.snapshotMs       = row.GetIntOr(1, 0);
        p.snapshotReason   = row.values.size() > 2 ? row.values[2] : std::string();
        p.affectionIndex   = row.GetFloatOr(3, 0.0);
        p.distressIndex    = row.GetFloatOr(4, 0.0);
        p.intimacy         = row.GetFloatOr(5, 0.0);
        p.security         = row.GetFloatOr(6, 0.0);
        p.repairMotivation = row.GetFloatOr(7, 0.0);
        out.push_back(std::move(p));
    }
    return true;
}

nlohmann::json BondingDashboardToJson(const BondingDashboardRow& r) {
    nlohmann::json o;
    o["updated_ms"]                = r.updatedMs;
    o["intimacy"]                  = r.intimacy;
    o["passion"]                   = r.passion;
    o["commitment"]                = r.commitment;
    o["security"]                  = r.security;
    o["anxiety"]                   = r.anxiety;
    o["avoidance"]                 = r.avoidance;
    o["felt_understood"]           = r.feltUnderstood;
    o["felt_cared_for"]            = r.feltCaredFor;
    o["investment"]                = r.investment;
    o["total_interactions"]        = r.totalInteractions;
    o["meaningful_conversations"]  = r.meaningfulConversations;
    o["conflicts_experienced"]     = r.conflictsExperienced;
    o["conflicts_resolved"]        = r.conflictsResolved;
    o["repair_motivation"]         = r.repairMotivation;
    o["unresolved_tension"]        = r.unresolvedTension;
    o["affection_index"]           = r.affectionIndex;
    o["commitment_index"]          = r.commitmentIndex;
    o["distress_index"]            = r.distressIndex;
    o["meaningful_ratio"]          = r.meaningfulRatio;
    o["conflict_resolution_ratio"] = r.conflictResolutionRatio;
    return o;
}

nlohmann::json BondingTrajectoryToJson(const std::vector<BondingTrajectoryPoint>& rows) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& p : rows) {
        nlohmann::json o;
        o["history_id"]        = p.historyId;
        o["snapshot_ms"]       = p.snapshotMs;
        o["snapshot_reason"]   = p.snapshotReason;
        o["affection_index"]   = p.affectionIndex;
        o["distress_index"]    = p.distressIndex;
        o["intimacy"]          = p.intimacy;
        o["security"]          = p.security;
        o["repair_motivation"] = p.repairMotivation;
        arr.push_back(std::move(o));
    }
    nlohmann::json out;
    out["rows"] = std::move(arr);
    return out;
}

}
