// ============================================================================
// Elle Probability Engine -- Audit Trace
// File: include/elle/prob/AuditTrace.hpp
//
// Every decision the probability engine makes is traceable. This module
// serializes the ProbabilityResult and the internal belief snapshots to
// JSON in the same format as the language engine's DebugTrace so both
// can be read together.
//
// Elle is not a black box. When she picks a sense, assigns an intent, or
// updates a belief, the full decision path is recorded here. This matters
// for two reasons:
//
//   1. Trust: the humans maintaining her can audit any decision she makes.
//   2. Elle herself: the trace feeds into her reflection engine (when built)
//      so she can reason about her own reasoning.
//
// Format is JSON with three top-level sections:
//   - "beliefs"   : snapshot of all modified beliefs and their posteriors
//   - "decisions" : per-unit sense resolutions with candidate rankings
//   - "intent"    : intent distribution and the evidence that formed it
//   - "emotions"  : emotional posterior and VAD vector
//   - "weights"   : current live scoring weights and their uncertainty
//   - "trust"     : speaker trust estimate and its history summary
// ============================================================================
#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/EmotionalPosteriorBuilder.hpp"
#include "elle/prob/Types.hpp"

#include <string>
#include <vector>

namespace elle { namespace prob {

class AuditTrace {
public:
    // Serialize a ProbabilityResult to a JSON string.
    // indent: spaces of indentation (0 = compact).
    [[nodiscard]] static std::string toJson(const ProbabilityResult& result,
                                            int indent = 2);

    // Serialize the current state of specified belief domains.
    [[nodiscard]] static std::string beliefsToJson(
        const BeliefStore&              store,
        const std::vector<std::string>& domains,
        int                             indent = 2);

    // Serialize the WeightVector as JSON.
    [[nodiscard]] static std::string weightsToJson(const WeightVector& w,
                                                   int indent = 2);

    // Serialize the emotional VAD to JSON.
    [[nodiscard]] static std::string emotionToJson(
        const EmotionalPosteriorBuilder& builder,
        int indent = 2);

    // Serialize a Distribution to JSON (id -> probability mass).
    [[nodiscard]] static std::string distributionToJson(const Distribution& d,
                                                        int indent = 2);

    // Summarize a Belief's evidence log as a human-readable string.
    [[nodiscard]] static std::string evidenceSummary(const Belief& belief);

private:
    // JSON value helpers (no external dependency needed -- hand-built).
    static std::string jsonString(const std::string& s);
    static std::string jsonDouble(double v);
    static std::string jsonInt(std::int64_t v);
};

} } // namespace elle::prob
