#pragma once

#include "elle/prob/Bridge.hpp"
#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"

#include "elle/Config.hpp"
#include "elle/Engine.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace elleann { namespace prob {

struct HostConfig {
    std::string engineConfigPath;
    std::string probabilityConfigPath;
    bool        autoLoadOnStart      = true;
    bool        useInMemoryLanguage  = false;
};

struct AnalyzeOutcome {
    bool                                 success = false;
    std::string                          error;
    elle::prob::ProbabilityResult        result;
    std::optional<elle::MeaningObject>   meaning;
};

class ProbabilityHost {
public:
    ProbabilityHost();
    ~ProbabilityHost();

    ProbabilityHost(const ProbabilityHost&)            = delete;
    ProbabilityHost& operator=(const ProbabilityHost&) = delete;

    bool start(const HostConfig& cfg);
    void stop();
    bool reload();

    [[nodiscard]] bool ready() const;

    AnalyzeOutcome analyzeText(const std::string&               rawText,
                               const elle::ConversationContext& convo,
                               const std::string&               speakerId);

    AnalyzeOutcome scoreRequest(const elle::prob::ProbabilityRequest& req,
                                const std::string&                    speakerId);

    bool feedback(std::size_t        unitIndex,
                  std::int64_t       confirmedId,
                  bool               isPhrase,
                  double             confidence,
                  const std::string& speakerId);

    bool recordTrust(const std::string&         speakerId,
                     elle::prob::TrustSignal    signal,
                     double                     strength);

    bool injectHormonalState(const std::unordered_map<std::int64_t, double>& state);

    elle::prob::WeightVector queryWeights() const;
    bool seedWeights(const elle::prob::WeightVector& w);
    bool resetAll();
    bool resetTurn();

private:
    HostConfig                                m_cfg;
    std::shared_ptr<elle::ISqlAccessLayer>    m_db;
    std::unique_ptr<elle::Engine>             m_language;
    std::unique_ptr<elle::prob::Bridge>       m_bridge;
    std::shared_ptr<elle::prob::ProbabilityEngine> m_engine;

    mutable std::mutex                        m_mutex;
    bool                                      m_ready = false;

    bool buildPipeline();
    void teardownPipeline();
};

} }
