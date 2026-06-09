#include "elle/prob/BeliefStore.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace elle { namespace prob {

BeliefStore::BeliefStore(std::size_t workerCount) {
    const std::size_t n = (workerCount == 0)
        ? std::max(std::size_t(1), static_cast<std::size_t>(std::thread::hardware_concurrency()))
        : workerCount;

    m_workers.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        m_workers.emplace_back([this]{ workerLoop(); });
    }

    m_decayThread = std::thread([this]{ decayLoop(); });
}

BeliefStore::~BeliefStore() {

    {
        std::lock_guard<std::mutex> lk(m_queueMutex);
        m_shutdown.store(true, std::memory_order_release);
    }
    m_queueCv.notify_all();

    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }

    {
        std::lock_guard<std::mutex> lk(m_decayMutex);
    }
    m_decayCv.notify_all();
    if (m_decayThread.joinable()) m_decayThread.join();
}

void BeliefStore::registerBelief(const std::string& domain,
                                  Distribution       prior,
                                  double             halfLifeSecs)
{
    {
        std::unique_lock<std::shared_mutex> wlock(m_rwLock);
        if (m_beliefs.count(domain)) return;

        Belief b;
        b.domain       = domain;
        b.prior        = prior;
        b.posterior    = prior;
        b.halfLifeSecs = halfLifeSecs;
        b.lastUpdated  = now();
        m_beliefs.emplace(domain, std::move(b));
    }

    std::shared_ptr<IBeliefPersistence> backend;
    {
        std::lock_guard<std::mutex> lk(m_persistenceMutex);
        backend = m_persistence;
    }
    if (backend) {
        try {
            backend->upsertDomain(domain, prior, halfLifeSecs);
            const std::int64_t whenMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                now().time_since_epoch()).count();
            backend->replacePosterior(domain, prior, whenMs);
        } catch (const std::exception& e) {
            (void)e;
        }
    }
}

void BeliefStore::upsertBelief(Belief belief) {
    std::unique_lock<std::shared_mutex> wlock(m_rwLock);
    m_beliefs[belief.domain] = std::move(belief);
}

std::optional<Belief> BeliefStore::getBelief(const std::string& domain) const {
    std::shared_lock<std::shared_mutex> rlock(m_rwLock);
    auto it = m_beliefs.find(domain);
    if (it == m_beliefs.end()) return std::nullopt;
    Belief copy = it->second;
    return copy;
}

Distribution BeliefStore::getPosterior(const std::string& domain) const {
    auto b = getBelief(domain);
    return b ? b->posterior : Distribution{};
}

std::int64_t BeliefStore::mapEstimate(const std::string& domain) const {
    auto b = getBelief(domain);
    return b ? b->posterior.map() : -1;
}

double BeliefStore::probability(const std::string& domain,
                                 std::int64_t       hypothesisId) const
{
    auto b = getBelief(domain);
    return b ? b->posterior.p(hypothesisId) : 0.0;
}

void BeliefStore::submitAsync(UpdateJob job) {
    {
        std::lock_guard<std::mutex> lk(m_queueMutex);
        m_queue.push_back(std::move(job));
        m_inFlight.fetch_add(1, std::memory_order_relaxed);
    }
    m_queueCv.notify_one();
}

void BeliefStore::submitSync(const std::string& domain,
                              const std::vector<Evidence>& ev)
{
    std::unique_lock<std::shared_mutex> wlock(m_rwLock);
    applyUpdateLocked(domain, ev);
}

void BeliefStore::applyUpdateLocked(const std::string& domain,
                                     const std::vector<Evidence>& ev)
{
    auto it = m_beliefs.find(domain);
    if (it == m_beliefs.end()) return;

    const double entropyBefore = it->second.posterior.entropy();

    it->second.decayTowardPrior();

    m_updater.update(it->second, ev);

    const double entropyAfter  = it->second.posterior.entropy();
    const std::int64_t map     = it->second.posterior.map();
    const double mapProb       = (map >= 0) ? it->second.posterior.p(map) : 0.0;
    const std::int64_t whenMs  = std::chrono::duration_cast<std::chrono::milliseconds>(
        it->second.lastUpdated.time_since_epoch()).count();
    const Distribution posterior = it->second.posterior;

    std::shared_ptr<IBeliefPersistence> backend;
    {
        std::lock_guard<std::mutex> lk(m_persistenceMutex);
        backend = m_persistence;
    }
    if (backend) {
        try {
            for (const auto& e : ev) backend->appendEvidence(domain, e);
            backend->replacePosterior(domain, posterior, whenMs);
            backend->auditUpdate(domain, "update", ev.size(),
                                 entropyBefore, entropyAfter,
                                 map, mapProb, "");
        } catch (const std::exception&) {
        }
    }
}

void BeliefStore::applyDecayAll() {
    struct DecaySnap {
        std::string  domain;
        Distribution posterior;
        std::int64_t whenMs;
        double       entropyBefore;
        double       entropyAfter;
    };
    std::vector<DecaySnap> snapshots;
    {
        std::unique_lock<std::shared_mutex> wlock(m_rwLock);
        for (auto& [k, b] : m_beliefs) {
            const double before = b.posterior.entropy();
            b.decayTowardPrior();
            const double after  = b.posterior.entropy();
            const std::int64_t whenMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                b.lastUpdated.time_since_epoch()).count();
            snapshots.push_back(DecaySnap{ k, b.posterior, whenMs, before, after });
        }
    }

    std::shared_ptr<IBeliefPersistence> backend;
    {
        std::lock_guard<std::mutex> lk(m_persistenceMutex);
        backend = m_persistence;
    }
    if (!backend) return;

    for (auto& s : snapshots) {
        try {
            backend->replacePosterior(s.domain, s.posterior, s.whenMs);
            const std::int64_t map = s.posterior.map();
            const double mapProb   = (map >= 0) ? s.posterior.p(map) : 0.0;
            backend->auditUpdate(s.domain, "decay", 0,
                                 s.entropyBefore, s.entropyAfter, map, mapProb, "");
        } catch (const std::exception&) {
        }
    }
}

void BeliefStore::resetBelief(const std::string& domain) {
    std::unique_lock<std::shared_mutex> wlock(m_rwLock);
    auto it = m_beliefs.find(domain);
    if (it != m_beliefs.end()) {
        BayesianUpdater::reset(it->second);
    }
}

void BeliefStore::resetAll() {
    std::unique_lock<std::shared_mutex> wlock(m_rwLock);
    for (auto& [k, b] : m_beliefs) {
        BayesianUpdater::reset(b);
    }
}

std::size_t BeliefStore::domainCount() const {
    std::shared_lock<std::shared_mutex> rlock(m_rwLock);
    return m_beliefs.size();
}

std::vector<std::string> BeliefStore::domains() const {
    std::shared_lock<std::shared_mutex> rlock(m_rwLock);
    std::vector<std::string> keys;
    keys.reserve(m_beliefs.size());
    for (const auto& [k, v] : m_beliefs) keys.push_back(k);
    return keys;
}

void BeliefStore::flush() {

    std::unique_lock<std::mutex> lk(m_idleMutex);
    m_idleCv.wait(lk, [this]{
        std::lock_guard<std::mutex> qlk(m_queueMutex);
        return m_queue.empty() && m_inFlight.load(std::memory_order_acquire) == 0;
    });
}

void BeliefStore::workerLoop() {
    while (true) {
        UpdateJob job;
        {
            std::unique_lock<std::mutex> lk(m_queueMutex);
            m_queueCv.wait(lk, [this]{
                return !m_queue.empty() || m_shutdown.load(std::memory_order_acquire);
            });

            if (m_shutdown.load(std::memory_order_acquire) && m_queue.empty()) {
                return;
            }

            if (m_queue.empty()) continue;

            job = std::move(m_queue.front());
            m_queue.erase(m_queue.begin());
        }

        {
            std::unique_lock<std::shared_mutex> wlock(m_rwLock);
            applyUpdateLocked(job.domain, job.evidence);
        }

        if (job.onComplete) {
            auto b = getBelief(job.domain);
            if (b) job.onComplete(*b);
        }

        if (m_inFlight.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::lock_guard<std::mutex> lk(m_idleMutex);
            m_idleCv.notify_all();
        }
    }
}

void BeliefStore::decayLoop() {
    using namespace std::chrono_literals;
    while (!m_shutdown.load(std::memory_order_acquire)) {
        std::unique_lock<std::mutex> lk(m_decayMutex);
        m_decayCv.wait_for(lk, 30s, [this]{
            return m_shutdown.load(std::memory_order_acquire);
        });
        if (!m_shutdown.load(std::memory_order_acquire)) {
            applyDecayAll();
        }
    }
}

void BeliefStore::attachPersistence(std::shared_ptr<IBeliefPersistence> backend) {
    std::lock_guard<std::mutex> lk(m_persistenceMutex);
    m_persistence = std::move(backend);
}

std::size_t BeliefStore::loadFromPersistence() {
    std::shared_ptr<IBeliefPersistence> backend;
    {
        std::lock_guard<std::mutex> lk(m_persistenceMutex);
        backend = m_persistence;
    }
    if (!backend) return 0;

    auto rows = backend->loadAll();
    std::unique_lock<std::shared_mutex> wlock(m_rwLock);
    std::size_t restored = 0;
    for (const auto& row : rows) {
        Belief b;
        b.domain       = row.domain;
        b.prior        = row.prior;
        b.posterior    = row.posterior.empty() ? row.prior : row.posterior;
        b.halfLifeSecs = row.halfLifeSecs;
        if (row.lastUpdatedMs > 0) {
            b.lastUpdated = Timestamp(std::chrono::milliseconds(row.lastUpdatedMs));
        } else {
            b.lastUpdated = now();
        }
        m_beliefs[row.domain] = std::move(b);
        ++restored;
    }
    return restored;
}

} }
