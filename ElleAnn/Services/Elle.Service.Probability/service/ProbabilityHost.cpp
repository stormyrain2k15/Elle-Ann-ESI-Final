#include "service/ProbabilityHost.h"

#ifdef ELLE_HAVE_ODBC
#include "elle/SqlServerAccessLayer.hpp"
#include "service/OdbcBeliefPersistence.hpp"
#endif

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <utility>

#ifndef ELLE_HOST_LOG_ERROR
#define ELLE_HOST_LOG_ERROR(...) do { fprintf(stderr, "[ERROR] " __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#endif
#ifndef ELLE_HOST_LOG_WARN
#define ELLE_HOST_LOG_WARN(...)  do { fprintf(stderr, "[WARN]  " __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#endif

namespace elleann { namespace prob {

ProbabilityHost::ProbabilityHost() = default;

ProbabilityHost::~ProbabilityHost() {
    teardownPipeline();
}

bool ProbabilityHost::start(const HostConfig& cfg) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_cfg   = cfg;
    m_ready = false;
    if (!cfg.autoLoadOnStart) {
        return true;
    }
    return buildPipeline();
}

void ProbabilityHost::stop() {
    std::lock_guard<std::mutex> lk(m_mutex);
    teardownPipeline();
}

bool ProbabilityHost::reload() {
    std::lock_guard<std::mutex> lk(m_mutex);
    teardownPipeline();
    return buildPipeline();
}

bool ProbabilityHost::ready() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_ready;
}

bool ProbabilityHost::buildPipeline() {
    try {
        elle::EngineConfig ec = m_cfg.engineConfigPath.empty()
            ? elle::EngineConfig::defaults()
            : elle::EngineConfig::loadFromFile(m_cfg.engineConfigPath);

        if (m_cfg.useInMemoryLanguage) {
            m_db.reset(elle::makeInMemoryAccessLayer().release());
        } else {
#ifdef ELLE_HAVE_ODBC
            m_db = std::make_shared<elle::SqlServerAccessLayer>(ec.database);
#else
            const char* override_env = std::getenv("ELLE_PROBABILITY_ALLOW_INMEMORY");
            if (override_env && std::string(override_env) == "1") {
                ELLE_HOST_LOG_WARN("ProbabilityHost: ELLE_HAVE_ODBC not defined — "
                          "falling back to in-memory because "
                          "ELLE_PROBABILITY_ALLOW_INMEMORY=1 is set. "
                          "This is NOT durable; learned beliefs will be lost on restart.");
                m_db.reset(elle::makeInMemoryAccessLayer().release());
            } else {
                ELLE_HOST_LOG_ERROR("ProbabilityHost: SQL backend requested but ODBC support "
                           "(ELLE_HAVE_ODBC) was not compiled in. Refusing to silently "
                           "fall back to in-memory. Set ELLE_PROBABILITY_ALLOW_INMEMORY=1 "
                           "to opt in, or rebuild with ODBC.");
                teardownPipeline();
                return false;
            }
#endif
        }

        m_language = std::make_unique<elle::Engine>(m_db, ec);

        elle::prob::ProbabilityEngineConfig pcfg =
            m_cfg.probabilityConfigPath.empty()
                ? elle::prob::ProbabilityEngineConfig::defaults()
                : elle::prob::ProbabilityEngineConfig::loadFromFile(
                      m_cfg.probabilityConfigPath);

        m_engine = std::make_shared<elle::prob::ProbabilityEngine>(std::move(pcfg));
        m_engine->seedWeights(elle::prob::Bridge::fromScoringWeights(ec.weights));

        m_bridge = std::make_unique<elle::prob::Bridge>(m_engine);

        wireBeliefBackendLocked();

        m_ready = true;
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_ERROR("ProbabilityHost::buildPipeline failed: %s", e.what());
        teardownPipeline();
        return false;
    } catch (...) {
        ELLE_HOST_LOG_ERROR("ProbabilityHost::buildPipeline failed with unknown exception");
        teardownPipeline();
        return false;
    }
}

void ProbabilityHost::teardownPipeline() {
    m_bridge.reset();
    m_engine.reset();
    m_language.reset();
    m_db.reset();
    m_ready = false;
}

void ProbabilityHost::wireBeliefBackendLocked() {
    if (!m_engine) return;

    if (!m_beliefBackend) {
        if (m_cfg.useInMemoryBeliefs) {
            m_beliefBackend = elle::prob::makeInMemoryBeliefPersistence();
        } else {
#ifdef ELLE_HAVE_ODBC
            try {
                m_beliefBackend = std::make_shared<elleann::prob::OdbcBeliefPersistence>();
            } catch (const std::exception& e) {
                ELLE_HOST_LOG_ERROR("ProbabilityHost: OdbcBeliefPersistence ctor failed: %s",
                                    e.what());
                m_beliefBackend.reset();
            }
#else
            const char* override_env = std::getenv("ELLE_PROBABILITY_ALLOW_INMEMORY");
            if (override_env && std::string(override_env) == "1") {
                ELLE_HOST_LOG_WARN("ProbabilityHost: ODBC not compiled — "
                                   "using in-memory belief persistence (NOT durable).");
                m_beliefBackend = elle::prob::makeInMemoryBeliefPersistence();
            } else {
                ELLE_HOST_LOG_ERROR("ProbabilityHost: SQL belief persistence requested but "
                                    "ODBC support not compiled in. Refusing to silently fall "
                                    "back. Set ELLE_PROBABILITY_ALLOW_INMEMORY=1 to opt in.");
                return;
            }
#endif
        }
    }

    if (!m_beliefBackend) return;

    auto store = m_engine->beliefStorePtr();
    if (!store) return;

    store->attachPersistence(m_beliefBackend);
    const std::size_t restored = store->loadFromPersistence();
    if (restored > 0) {
        ELLE_HOST_LOG_WARN("ProbabilityHost: restored %zu belief domains from persistence",
                           restored);
    }
}

AnalyzeOutcome ProbabilityHost::analyzeText(const std::string&               rawText,
                                            const elle::ConversationContext& convo,
                                            const std::string&               speakerId) {
    AnalyzeOutcome out{};
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_language || !m_bridge) {
        out.error = "probability_host_not_ready";
        return out;
    }
    try {
        auto analysis = m_language->analyze(rawText, convo);
        auto req      = elle::prob::Bridge::fromMeaningObject(analysis.meaning, convo);
        out.result    = m_bridge->analyze(req, speakerId);
        out.meaning   = std::move(analysis.meaning);
        out.success   = true;
    } catch (const std::exception& ex) {
        out.error = ex.what();
    }
    return out;
}

AnalyzeOutcome ProbabilityHost::scoreRequest(const elle::prob::ProbabilityRequest& req,
                                             const std::string&                    speakerId) {
    AnalyzeOutcome out{};
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_bridge) {
        out.error = "probability_host_not_ready";
        return out;
    }
    try {
        out.result  = m_bridge->analyze(req, speakerId);
        out.success = true;
    } catch (const std::exception& ex) {
        out.error = ex.what();
    }
    return out;
}

bool ProbabilityHost::feedback(std::size_t        unitIndex,
                               std::int64_t       confirmedId,
                               bool               isPhrase,
                               double             confidence,
                               const std::string& speakerId) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_bridge) return false;
    try {
        m_bridge->feedback(unitIndex, confirmedId, isPhrase, confidence, speakerId);
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::feedback failed: %s", e.what());
        return false;
    }
}

bool ProbabilityHost::recordTrust(const std::string&         speakerId,
                                  elle::prob::TrustSignal    signal,
                                  double                     strength) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_bridge) return false;
    try {
        m_bridge->recordTrust(speakerId, signal, strength);
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::recordTrust failed: %s", e.what());
        return false;
    }
}

bool ProbabilityHost::injectHormonalState(
    const std::unordered_map<std::int64_t, double>& state) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_bridge) return false;
    try {
        m_bridge->injectHormonalState(state);
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::injectHormonalState failed: %s", e.what());
        return false;
    }
}

elle::prob::WeightVector ProbabilityHost::queryWeights() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return {};
    return m_engine->currentWeights();
}

bool ProbabilityHost::seedWeights(const elle::prob::WeightVector& w) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return false;
    try {
        m_engine->seedWeights(w);
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::seedWeights failed: %s", e.what());
        return false;
    }
}

bool ProbabilityHost::resetAll() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return false;
    try {
        m_engine->resetAll();
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::resetAll failed: %s", e.what());
        return false;
    }
}

bool ProbabilityHost::resetTurn() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return false;
    try {
        m_engine->resetTurn();
        return true;
    } catch (const std::exception& e) {
        ELLE_HOST_LOG_WARN("ProbabilityHost::resetTurn failed: %s", e.what());
        return false;
    }
}

void ProbabilityHost::attachBeliefPersistence(
    std::shared_ptr<elle::prob::IBeliefPersistence> backend) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_beliefBackend = std::move(backend);
    if (m_engine) {
        auto store = m_engine->beliefStorePtr();
        if (store) store->attachPersistence(m_beliefBackend);
    }
}

std::size_t ProbabilityHost::loadBeliefsFromPersistence() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_engine) return 0;
    auto store = m_engine->beliefStorePtr();
    if (!store) return 0;
    return store->loadFromPersistence();
}

std::shared_ptr<elle::prob::IBeliefPersistence>
ProbabilityHost::beliefPersistence() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_beliefBackend;
}

} }
