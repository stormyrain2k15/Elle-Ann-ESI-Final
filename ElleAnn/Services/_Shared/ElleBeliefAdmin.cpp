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

}
