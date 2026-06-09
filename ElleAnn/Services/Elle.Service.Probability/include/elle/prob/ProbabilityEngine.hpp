#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/EmotionalPosteriorBuilder.hpp"
#include "elle/prob/IntentAnalyzer.hpp"
#include "elle/prob/SenseProbabilityResolver.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"
#include "elle/prob/Types.hpp"

#include <memory>
#include <string>

namespace elle { namespace prob {

struct ProbabilityEngineConfig {

    std::size_t workerThreads = 0;

    EmotionalPosteriorConfig emotionConfig;

    BetaParams defaultTrustPrior { 2.0, 2.0 };

    double trustHalfLifeSecs = 86400.0;

    double senseHalfLifeSecs = 600.0;

    double intentHalfLifeSecs = 120.0;

    std::string weightsJson;

    bool stochastic = false;

    [[nodiscard]] static ProbabilityEngineConfig loadFromFile(const std::string& path);
    [[nodiscard]] static ProbabilityEngineConfig loadFromString(const std::string& json);
    [[nodiscard]] static ProbabilityEngineConfig defaults();
};

class ProbabilityEngine {
public:
    explicit ProbabilityEngine(ProbabilityEngineConfig config = ProbabilityEngineConfig::defaults());
    ~ProbabilityEngine();

    ProbabilityEngine(const ProbabilityEngine&)            = delete;
    ProbabilityEngine& operator=(const ProbabilityEngine&) = delete;

    [[nodiscard]] ProbabilityResult analyze(const ProbabilityRequest& req,
                                            const std::string& speakerId = "default");

    [[nodiscard]] WeightVector currentWeights() const;

    void feedback(std::size_t  unitIndex,
                  std::int64_t correctSenseId,
                  bool         isPhrase,
                  double       confidence,
                  const std::string& speakerId = "default");

    void recordTrustSignal(const std::string& speakerId,
                           TrustSignal        signal,
                           double             strength = 1.0);

    void seedWeights(const WeightVector& w);

    void seedEmotionalPrior(const std::unordered_map<std::int64_t, double>& priorWeights);

    void resetAll();

    void resetTurn();

    [[nodiscard]] const ProbabilityEngineConfig& config() const noexcept { return m_config; }

    [[nodiscard]] const BeliefStore& beliefStore() const noexcept { return *m_store; }

    [[nodiscard]] std::shared_ptr<BeliefStore> beliefStorePtr() const noexcept { return m_store; }

    void flush();

private:

    SpeakerTrustModel& getSpeakerModel(const std::string& speakerId);

    ProbabilityEngineConfig                           m_config;
    std::shared_ptr<BeliefStore>                      m_store;
    std::unique_ptr<SenseProbabilityResolver>         m_senseResolver;
    std::unique_ptr<IntentAnalyzer>                   m_intentAnalyzer;
    std::unique_ptr<EmotionalPosteriorBuilder>        m_emotionBuilder;

    mutable std::mutex                                        m_speakerMutex;
    std::unordered_map<std::string,
                       std::unique_ptr<SpeakerTrustModel>>   m_speakerModels;
};

} }
