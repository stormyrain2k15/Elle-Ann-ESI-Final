#pragma once

#include "elle/prob/Types.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace elle::prob {

struct PersistedBelief {
    std::string  domain;
    Distribution prior;
    Distribution posterior;
    double       halfLifeSecs = 0.0;
    std::int64_t lastUpdatedMs = 0;
};

class IBeliefPersistence {
public:
    virtual ~IBeliefPersistence() = default;

    virtual void upsertDomain(const std::string& domain,
                              const Distribution& prior,
                              double halfLifeSecs) = 0;

    virtual void replacePosterior(const std::string& domain,
                                  const Distribution& posterior,
                                  std::int64_t whenMs) = 0;

    virtual void appendEvidence(const std::string& domain,
                                const Evidence& ev) = 0;

    virtual void auditUpdate(const std::string& domain,
                             const std::string& operation,
                             std::size_t evidenceCount,
                             double entropyBefore,
                             double entropyAfter,
                             std::int64_t mapHypothesisId,
                             double mapProbability,
                             const std::string& detail) = 0;

    virtual std::vector<PersistedBelief> loadAll() = 0;
};

class InMemoryBeliefPersistence final : public IBeliefPersistence {
public:
    void upsertDomain(const std::string& domain,
                      const Distribution& prior,
                      double halfLifeSecs) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& row = m_rows[domain];
        row.domain        = domain;
        row.prior         = prior;
        if (row.posterior.empty()) row.posterior = prior;
        row.halfLifeSecs  = halfLifeSecs;
    }

    void replacePosterior(const std::string& domain,
                          const Distribution& posterior,
                          std::int64_t whenMs) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_rows.find(domain);
        if (it == m_rows.end()) {
            PersistedBelief row;
            row.domain        = domain;
            row.prior         = posterior;
            row.posterior     = posterior;
            row.halfLifeSecs  = 0.0;
            row.lastUpdatedMs = whenMs;
            m_rows.emplace(domain, std::move(row));
            return;
        }
        it->second.posterior     = posterior;
        it->second.lastUpdatedMs = whenMs;
    }

    void appendEvidence(const std::string& domain,
                        const Evidence& ev) override {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_evidence[domain].push_back(ev);
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
        AuditRow row;
        row.domain          = domain;
        row.operation       = operation;
        row.evidenceCount   = evidenceCount;
        row.entropyBefore   = entropyBefore;
        row.entropyAfter    = entropyAfter;
        row.mapHypothesisId = mapHypothesisId;
        row.mapProbability  = mapProbability;
        row.detail          = detail;
        m_audit.push_back(std::move(row));
    }

    std::vector<PersistedBelief> loadAll() override {
        std::lock_guard<std::mutex> lk(m_mutex);
        std::vector<PersistedBelief> out;
        out.reserve(m_rows.size());
        for (const auto& [k, v] : m_rows) out.push_back(v);
        return out;
    }

    std::vector<Evidence> evidenceFor(const std::string& domain) const {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_evidence.find(domain);
        return it == m_evidence.end() ? std::vector<Evidence>{} : it->second;
    }

    struct AuditRow {
        std::string  domain;
        std::string  operation;
        std::size_t  evidenceCount   = 0;
        double       entropyBefore   = 0.0;
        double       entropyAfter    = 0.0;
        std::int64_t mapHypothesisId = 0;
        double       mapProbability  = 0.0;
        std::string  detail;
    };

    std::vector<AuditRow> auditTrail() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_audit;
    }

    std::size_t domainCount() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_rows.size();
    }

private:
    mutable std::mutex                                m_mutex;
    std::unordered_map<std::string, PersistedBelief>  m_rows;
    std::unordered_map<std::string, std::vector<Evidence>> m_evidence;
    std::vector<AuditRow>                             m_audit;
};

inline std::shared_ptr<IBeliefPersistence> makeInMemoryBeliefPersistence() {
    return std::make_shared<InMemoryBeliefPersistence>();
}

}
