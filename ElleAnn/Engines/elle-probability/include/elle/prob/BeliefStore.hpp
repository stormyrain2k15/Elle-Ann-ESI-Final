// ============================================================================
// Elle Probability Engine -- Belief Store
// File: include/elle/prob/BeliefStore.hpp
//
// Thread-safe registry of all active beliefs. The store holds the full
// doxastic state of the probability engine: what it believes about
// word senses, emotional state, pragmatic intent, scoring weights,
// speaker trust, and any domain the engine reasons about.
//
// Architecture:
//   - Beliefs are keyed by domain string ("sense:42", "weight:contextFrameMatch", …)
//   - A fixed thread pool of writer workers processes update requests from a
//     lock-free queue so callers never block on belief updates.
//   - Read access is lock-free via shared_ptr snapshots; writers hold a
//     short exclusive lock only when swapping the snapshot pointer.
//   - Decay is applied lazily on read and eagerly by a background decay thread.
// ============================================================================
#pragma once

#include "elle/prob/BayesianUpdater.hpp"
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

// A single pending belief update job submitted to the worker pool.
struct UpdateJob {
    std::string           domain;
    std::vector<Evidence> evidence;
    // Optional callback invoked after the update completes.
    std::function<void(const Belief&)> onComplete;
};

class BeliefStore {
public:
    // Construct with a worker thread count. The default (0) uses
    // std::thread::hardware_concurrency().
    explicit BeliefStore(std::size_t workerCount = 0);
    ~BeliefStore();

    // Non-copyable, non-moveable (owns threads).
    BeliefStore(const BeliefStore&)            = delete;
    BeliefStore& operator=(const BeliefStore&) = delete;

    // ---- Belief lifecycle ------------------------------------------------

    // Register a new belief domain with the given prior.  No-op if the domain
    // already exists.  Thread-safe.
    void registerBelief(const std::string& domain,
                        Distribution       prior,
                        double             halfLifeSecs = 0.0);

    // Register or replace a belief. Always overwrites.
    void upsertBelief(Belief belief);

    // Return a snapshot of the current belief for the given domain.
    // Returns std::nullopt if the domain is not registered.
    [[nodiscard]] std::optional<Belief> getBelief(const std::string& domain) const;

    // Return the current posterior for a domain, or an empty distribution.
    [[nodiscard]] Distribution getPosterior(const std::string& domain) const;

    // Return the MAP hypothesis for a domain, or -1.
    [[nodiscard]] std::int64_t mapEstimate(const std::string& domain) const;

    // Return P(h) for a specific hypothesis in a domain, or 0.0.
    [[nodiscard]] double probability(const std::string& domain,
                                     std::int64_t       hypothesisId) const;

    // ---- Evidence submission ---------------------------------------------

    // Enqueue an evidence update for async processing by the worker pool.
    // Returns immediately; the update is applied on a worker thread.
    void submitAsync(UpdateJob job);

    // Apply evidence synchronously (blocks until the update is complete).
    // Use for unit tests or when the caller needs the updated belief before
    // continuing.
    void submitSync(const std::string& domain, const std::vector<Evidence>& ev);

    // ---- Decay -----------------------------------------------------------

    // Apply temporal decay to all beliefs whose half-life has elapsed.
    // Called by the background decay thread; also callable manually.
    void applyDecayAll();

    // ---- Reset -----------------------------------------------------------

    // Reset one belief to its prior.
    void resetBelief(const std::string& domain);

    // Reset all beliefs.
    void resetAll();

    // ---- Introspection ---------------------------------------------------

    // Return the number of registered domains.
    [[nodiscard]] std::size_t domainCount() const;

    // Return all domain keys.
    [[nodiscard]] std::vector<std::string> domains() const;

    // Drain the async queue and wait for all workers to be idle.
    void flush();

private:
    // Internal: apply updates directly under the write lock.
    void applyUpdateLocked(const std::string& domain,
                           const std::vector<Evidence>& ev);

    // Worker thread main loop.
    void workerLoop();

    // Background decay thread main loop.
    void decayLoop();

    // ---- State -----------------------------------------------------------
    mutable std::shared_mutex               m_rwLock;
    std::unordered_map<std::string, Belief> m_beliefs;

    // ---- Worker pool -------------------------------------------------
    std::vector<std::thread>  m_workers;
    std::vector<UpdateJob>    m_queue;
    std::mutex                m_queueMutex;
    std::condition_variable   m_queueCv;
    std::atomic<bool>         m_shutdown { false };
    std::atomic<std::size_t>  m_inFlight { 0 };
    std::condition_variable   m_idleCv;
    std::mutex                m_idleMutex;

    // ---- Decay thread ------------------------------------------------
    std::thread               m_decayThread;
    std::condition_variable   m_decayCv;
    std::mutex                m_decayMutex;

    BayesianUpdater           m_updater;
};

} } // namespace elle::prob
