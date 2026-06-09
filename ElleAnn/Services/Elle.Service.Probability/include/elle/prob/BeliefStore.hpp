#pragma once

#include "elle/prob/BayesianUpdater.hpp"
#include "elle/prob/BeliefPersistence.hpp"
#include "elle/prob/Types.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace elle { namespace prob {

struct UpdateJob {
    std::string           domain;
    std::vector<Evidence> evidence;

    std::function<void(const Belief&)> onComplete;
};

class BeliefStore {
public:

    explicit BeliefStore(std::size_t workerCount = 0);
    ~BeliefStore();

    BeliefStore(const BeliefStore&)            = delete;
    BeliefStore& operator=(const BeliefStore&) = delete;

    void registerBelief(const std::string& domain,
                        Distribution       prior,
                        double             halfLifeSecs = 0.0);

    void upsertBelief(Belief belief);

    [[nodiscard]] std::optional<Belief> getBelief(const std::string& domain) const;

    [[nodiscard]] Distribution getPosterior(const std::string& domain) const;

    [[nodiscard]] std::int64_t mapEstimate(const std::string& domain) const;

    [[nodiscard]] double probability(const std::string& domain,
                                     std::int64_t       hypothesisId) const;

    void submitAsync(UpdateJob job);

    void submitSync(const std::string& domain, const std::vector<Evidence>& ev);

    void applyDecayAll();

    void resetBelief(const std::string& domain);

    void resetAll();

    [[nodiscard]] std::size_t domainCount() const;

    [[nodiscard]] std::vector<std::string> domains() const;

    void flush();

    void attachPersistence(std::shared_ptr<IBeliefPersistence> backend);

    std::size_t loadFromPersistence();

private:

    void applyUpdateLocked(const std::string& domain,
                           const std::vector<Evidence>& ev);

    void workerLoop();

    void decayLoop();

    mutable std::shared_mutex               m_rwLock;
    std::unordered_map<std::string, Belief> m_beliefs;

    std::vector<std::thread>  m_workers;
    std::vector<UpdateJob>    m_queue;
    std::mutex                m_queueMutex;
    std::condition_variable   m_queueCv;
    std::atomic<bool>         m_shutdown { false };
    std::atomic<std::size_t>  m_inFlight { 0 };
    std::condition_variable   m_idleCv;
    std::mutex                m_idleMutex;

    std::thread               m_decayThread;
    std::condition_variable   m_decayCv;
    std::mutex                m_decayMutex;

    BayesianUpdater           m_updater;

    std::shared_ptr<IBeliefPersistence> m_persistence;
    mutable std::mutex                  m_persistenceMutex;
};

} }
