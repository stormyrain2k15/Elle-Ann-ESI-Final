#pragma once

#include "elle/prob/BeliefPersistence.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace elle::prob {

class MultiplexBeliefPersistence final : public IBeliefPersistence {
public:
    MultiplexBeliefPersistence() = default;

    explicit MultiplexBeliefPersistence(
            std::vector<std::shared_ptr<IBeliefPersistence>> backends)
        : m_backends(std::move(backends)) {}

    void addBackend(std::shared_ptr<IBeliefPersistence> backend) {
        if (backend) m_backends.push_back(std::move(backend));
    }

    std::size_t backendCount() const { return m_backends.size(); }

    std::shared_ptr<IBeliefPersistence> primaryBackend() const {
        return m_backends.empty() ? nullptr : m_backends.front();
    }

    void upsertDomain(const std::string& domain,
                      const Distribution& prior,
                      double halfLifeSecs) override {
        for (auto& b : m_backends) {
            if (b) b->upsertDomain(domain, prior, halfLifeSecs);
        }
    }

    void replacePosterior(const std::string& domain,
                          const Distribution& posterior,
                          std::int64_t whenMs) override {
        for (auto& b : m_backends) {
            if (b) b->replacePosterior(domain, posterior, whenMs);
        }
    }

    void appendEvidence(const std::string& domain,
                        const Evidence& ev) override {
        for (auto& b : m_backends) {
            if (b) b->appendEvidence(domain, ev);
        }
    }

    void auditUpdate(const std::string& domain,
                     const std::string& operation,
                     std::size_t evidenceCount,
                     double entropyBefore,
                     double entropyAfter,
                     std::int64_t mapHypothesisId,
                     double mapProbability,
                     const std::string& detail) override {
        for (auto& b : m_backends) {
            if (b) b->auditUpdate(domain, operation, evidenceCount,
                                  entropyBefore, entropyAfter,
                                  mapHypothesisId, mapProbability, detail);
        }
    }

    std::vector<PersistedBelief> loadAll() override {
        for (auto& b : m_backends) {
            if (!b) continue;
            auto rows = b->loadAll();
            if (!rows.empty()) return rows;
        }
        return {};
    }

private:
    std::vector<std::shared_ptr<IBeliefPersistence>> m_backends;
};

inline std::shared_ptr<IBeliefPersistence>
makeMultiplexBeliefPersistence(
        std::vector<std::shared_ptr<IBeliefPersistence>> backends) {
    return std::make_shared<MultiplexBeliefPersistence>(std::move(backends));
}

}
