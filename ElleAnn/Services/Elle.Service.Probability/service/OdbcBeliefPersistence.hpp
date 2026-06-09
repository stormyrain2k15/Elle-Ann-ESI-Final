#pragma once

#include "elle/prob/BeliefPersistence.hpp"

#include "../../_Shared/ElleSQLConn.h"
#include "../../_Shared/ElleLogger.h"

#include <chrono>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace elleann::prob {

class OdbcBeliefPersistence final : public elle::prob::IBeliefPersistence {
public:
    OdbcBeliefPersistence() = default;
    ~OdbcBeliefPersistence() override = default;

    void upsertDomain(const std::string& domain,
                      const elle::prob::Distribution& prior,
                      double halfLifeSecs) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& pool = ElleSQLPool::Instance();

        std::ostringstream sql;
        sql << "DECLARE @id BIGINT;\n"
            << "EXEC dbo.usp_BeliefUpsertDomain "
            << "@DomainCode = N'" << escape(domain) << "', "
            << "@HalfLifeSecs = " << halfLifeSecs << ", "
            << "@DomainID = @id OUTPUT;\n"
            << "SELECT @id;";
        auto rs = pool.Query(sql.str());
        if (!rs.success || rs.rows.empty()) {
            ELLE_ERROR("OdbcBeliefPersistence::upsertDomain failed for '%s'", domain.c_str());
            return;
        }
        const long long id = rs.rows[0].GetIntOr(0, 0);
        m_domainIds[domain] = id;

        std::ostringstream delSql;
        delSql << "DELETE FROM ElleHeart.dbo.belief_prior WHERE domain_id = " << id << ";";
        pool.Exec(delSql.str());

        for (const auto& [hyp, mass] : prior.mass) {
            std::ostringstream insSql;
            insSql << "INSERT INTO ElleHeart.dbo.belief_prior (domain_id, hypothesis_id, mass) VALUES ("
                   << id << "," << hyp << "," << mass << ");";
            if (!pool.Exec(insSql.str())) {
                ELLE_ERROR("OdbcBeliefPersistence::upsertDomain prior insert failed (%s, h=%lld)",
                           domain.c_str(), (long long)hyp);
            }
        }
    }

    void replacePosterior(const std::string& domain,
                          const elle::prob::Distribution& posterior,
                          std::int64_t whenMs) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& pool = ElleSQLPool::Instance();
        const long long id = resolveDomainIdLocked(domain);
        if (id <= 0) {
            ELLE_WARN("OdbcBeliefPersistence::replacePosterior: domain '%s' has no id; skipping",
                      domain.c_str());
            return;
        }

        std::ostringstream sql;
        sql << "DECLARE @rows dbo.HypothesisMass;\n";
        for (const auto& [hyp, mass] : posterior.mass) {
            sql << "INSERT INTO @rows VALUES (" << hyp << "," << mass << ");\n";
        }
        sql << "EXEC dbo.usp_BeliefReplacePosterior @DomainID = " << id << ", @Rows = @rows;\n"
            << "UPDATE ElleHeart.dbo.belief_domain SET last_updated_ms = " << whenMs
            << " WHERE domain_id = " << id << ";";
        if (!pool.Exec(sql.str())) {
            ELLE_ERROR("OdbcBeliefPersistence::replacePosterior failed for '%s'", domain.c_str());
        }
    }

    void appendEvidence(const std::string& domain,
                        const elle::prob::Evidence& ev) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& pool = ElleSQLPool::Instance();
        const long long id = resolveDomainIdLocked(domain);
        if (id <= 0) return;

        std::ostringstream sql;
        sql << "EXEC dbo.usp_BeliefAppendEvidence "
            << "@DomainID = " << id << ", "
            << "@Kind = " << static_cast<int>(ev.kind) << ", "
            << "@HypothesisID = " << ev.hypothesisId << ", "
            << "@LikelihoodRatio = " << ev.likelihoodRatio << ", "
            << "@SourceWeight = " << ev.sourceWeight << ", "
            << "@Reason = N'" << escape(ev.reason) << "';";
        pool.Exec(sql.str());
    }

    void auditUpdate(const std::string& domain,
                     const std::string& operation,
                     std::size_t evidenceCount,
                     double entropyBefore,
                     double entropyAfter,
                     std::int64_t mapHypothesisId,
                     double mapProbability,
                     const std::string& detail) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& pool = ElleSQLPool::Instance();
        const long long id = resolveDomainIdLocked(domain);
        if (id <= 0) return;

        std::ostringstream sql;
        sql << "EXEC dbo.usp_BeliefAudit "
            << "@DomainID = " << id << ", "
            << "@Operation = N'" << escape(operation) << "', "
            << "@EvidenceCount = " << static_cast<int>(evidenceCount) << ", "
            << "@EntropyBefore = " << entropyBefore << ", "
            << "@EntropyAfter = " << entropyAfter << ", "
            << "@MapHypothesisID = " << mapHypothesisId << ", "
            << "@MapProbability = " << mapProbability << ", "
            << "@Detail = N'" << escape(detail) << "';";
        pool.Exec(sql.str());
    }

    std::vector<elle::prob::PersistedBelief> loadAll() override {
        std::lock_guard<std::mutex> lk(m_mutex);
        std::vector<elle::prob::PersistedBelief> out;
        auto& pool = ElleSQLPool::Instance();

        auto domains = pool.Query(
            "SELECT domain_id, domain_code, half_life_secs, last_updated_ms "
            "FROM ElleHeart.dbo.belief_domain;");
        if (!domains.success) {
            ELLE_ERROR("OdbcBeliefPersistence::loadAll domains query failed");
            return out;
        }

        for (auto& row : domains.rows) {
            const long long id = row.GetIntOr(0, 0);
            const std::string code = row.values.size() > 1 ? row.values[1] : std::string();
            const double halfLife  = row.GetFloatOr(2, 0.0);
            const long long whenMs = row.GetIntOr(3, 0);
            if (id <= 0 || code.empty()) continue;

            m_domainIds[code] = id;

            elle::prob::PersistedBelief b;
            b.domain        = code;
            b.halfLifeSecs  = halfLife;
            b.lastUpdatedMs = whenMs;

            auto priorRs = pool.Query(
                "SELECT hypothesis_id, mass FROM ElleHeart.dbo.belief_prior WHERE domain_id = "
                + std::to_string(id) + ";");
            if (priorRs.success) {
                for (auto& pr : priorRs.rows) {
                    const long long hyp = pr.GetIntOr(0, 0);
                    const double mass   = pr.GetFloatOr(1, 0.0);
                    b.prior.mass[hyp] = mass;
                }
            }

            auto postRs = pool.Query(
                "SELECT hypothesis_id, mass FROM ElleHeart.dbo.belief_posterior WHERE domain_id = "
                + std::to_string(id) + ";");
            if (postRs.success) {
                for (auto& pr : postRs.rows) {
                    const long long hyp = pr.GetIntOr(0, 0);
                    const double mass   = pr.GetFloatOr(1, 0.0);
                    b.posterior.mass[hyp] = mass;
                }
            }

            out.push_back(std::move(b));
        }
        return out;
    }

private:
    long long resolveDomainIdLocked(const std::string& domain) {
        auto it = m_domainIds.find(domain);
        if (it != m_domainIds.end()) return it->second;

        auto& pool = ElleSQLPool::Instance();
        std::ostringstream sql;
        sql << "SELECT TOP 1 domain_id FROM ElleHeart.dbo.belief_domain WHERE domain_code = N'"
            << escape(domain) << "';";
        auto rs = pool.Query(sql.str());
        if (rs.success && !rs.rows.empty()) {
            const long long id = rs.rows[0].GetIntOr(0, 0);
            if (id > 0) m_domainIds[domain] = id;
            return id;
        }
        return 0;
    }

    static std::string escape(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 4);
        for (char c : s) {
            if (c == '\'') out += "''";
            else out.push_back(c);
        }
        return out;
    }

    mutable std::mutex                          m_mutex;
    std::unordered_map<std::string, long long>  m_domainIds;
};

}
