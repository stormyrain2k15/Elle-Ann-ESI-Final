// ============================================================================
// Elle Probability Engine -- Intent Analyzer
// File: include/elle/prob/IntentAnalyzer.hpp
//
// Assigns a probability distribution over PragmaticAct values for each
// utterance. Philosophy here is applied Gricean theory:
//
//   Grice's maxims as likelihood signals:
//     Quantity  -- how much was said relative to what was needed?
//     Quality   -- hedges, uncertainty markers, confidence signals
//     Relation  -- does this fit what was being discussed (context frames)?
//     Manner    -- phrasing clarity, directness, emotional loading
//
//   Austin / Searle speech act categories map directly to PragmaticAct.
//
// The analyzer maintains a belief over PragmaticAct in the BeliefStore
// under domain "intent:current". Each utterance updates that belief so
// multi-turn intent tracking is automatic.
//
// Evidence construction:
//   - Punctuation profile  (question -> QUESTION, exclaim -> ASSERT/WARN)
//   - Emotional posterior  (comfort -> COMFORT, anger -> CHALLENGE/DENY)
//   - Context frame scores (greeting frame -> GREET, etc.)
//   - Speaker trust level  (low trust -> skepticism weight on ASSERT)
//   - Syntactic signals    (interrogative word order not parsed here but
//                           flagged by question count from lexeme pass)
// ============================================================================
#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>

namespace elle { namespace prob {

class IntentAnalyzer {
public:
    explicit IntentAnalyzer(std::shared_ptr<BeliefStore> store);

    // Analyze intent from a full request and populate result.intentDistribution
    // and result.likelyAct.
    void analyze(const ProbabilityRequest& req,
                 ProbabilityResult&        result) const;

    // Return the current multi-turn intent posterior (domain "intent:current").
    [[nodiscard]] Distribution currentIntentPosterior() const;

    // Reset the multi-turn intent belief.
    void resetIntent();

private:
    // Build initial prior: which acts are apriori plausible given syntax alone?
    [[nodiscard]] Distribution syntaxPrior(const ProbabilityRequest& req) const;

    // Likelihood of each act given the emotional posterior.
    // Returns a map act_id -> likelihood multiplier.
    [[nodiscard]] std::unordered_map<std::int64_t, double>
    emotionLikelihoods(const Distribution& emotionPosterior) const;

    // Likelihood of each act given context frame scores.
    [[nodiscard]] std::unordered_map<std::int64_t, double>
    contextLikelihoods(
        const std::vector<ProbabilityRequest::ContextHint>& ctxHints) const;

    // Likelihood modifier from speaker trust.
    // Low trust boosts DENY/CHALLENGE; high trust boosts ASSERT/CONFIRM.
    [[nodiscard]] std::unordered_map<std::int64_t, double>
    trustLikelihoods(double speakerTrust) const;

    // Convert a PragmaticAct enum to its hypothesis integer ID.
    static std::int64_t actId(PragmaticAct act) noexcept {
        return static_cast<std::int64_t>(act);
    }

    static constexpr const char* DOMAIN_INTENT = "intent:current";

    std::shared_ptr<BeliefStore> m_store;
};

} } // namespace elle::prob
