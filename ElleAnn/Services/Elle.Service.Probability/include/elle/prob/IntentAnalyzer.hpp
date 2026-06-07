#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>

namespace elle { namespace prob {

class IntentAnalyzer {
public:
    explicit IntentAnalyzer(std::shared_ptr<BeliefStore> store);

    void analyze(const ProbabilityRequest& req,
                 ProbabilityResult&        result) const;

    [[nodiscard]] Distribution currentIntentPosterior() const;

    void resetIntent();

private:

    [[nodiscard]] Distribution syntaxPrior(const ProbabilityRequest& req) const;

    [[nodiscard]] std::unordered_map<std::int64_t, double>
    emotionLikelihoods(const Distribution& emotionPosterior) const;

    [[nodiscard]] std::unordered_map<std::int64_t, double>
    contextLikelihoods(
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const;

    [[nodiscard]] std::unordered_map<std::int64_t, double>
    trustLikelihoods(double speakerTrust) const;

    static std::int64_t actId(PragmaticAct act) noexcept {
        return static_cast<std::int64_t>(act);
    }

    static constexpr const char* DOMAIN_INTENT = "intent:current";

    std::shared_ptr<BeliefStore> m_store;
};

} }
