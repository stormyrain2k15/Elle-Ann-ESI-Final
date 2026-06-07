#include "service/ProbabilityHost.h"

#ifdef ELLE_HAVE_ODBC
#include "elle/SqlServerAccessLayer.hpp"
#endif

#include <exception>
#include <utility>

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
            m_db.reset(elle::makeInMemoryAccessLayer().release());
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

        m_ready = true;
        return true;
    } catch (const std::exception&) {
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
    } catch (const std::exception&) {
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
    } catch (const std::exception&) {
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
    } catch (const std::exception&) {
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
    } catch (const std::exception&) {
        return false;
    }
}

bool ProbabilityHost::resetAll() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return false;
    try {
        m_engine->resetAll();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ProbabilityHost::resetTurn() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_ready || !m_engine) return false;
    try {
        m_engine->resetTurn();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} }
