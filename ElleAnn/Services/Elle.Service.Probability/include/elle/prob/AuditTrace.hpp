#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/EmotionalPosteriorBuilder.hpp"
#include "elle/prob/Types.hpp"

#include <string>
#include <vector>

namespace elle { namespace prob {

class AuditTrace {
public:

    [[nodiscard]] static std::string toJson(const ProbabilityResult& result,
                                            int indent = 2);

    [[nodiscard]] static std::string beliefsToJson(
        const BeliefStore&              store,
        const std::vector<std::string>& domains,
        int                             indent = 2);

    [[nodiscard]] static std::string weightsToJson(const WeightVector& w,
                                                   int indent = 2);

    [[nodiscard]] static std::string emotionToJson(
        const EmotionalPosteriorBuilder& builder,
        int indent = 2);

    [[nodiscard]] static std::string distributionToJson(const Distribution& d,
                                                        int indent = 2);

    [[nodiscard]] static std::string evidenceSummary(const Belief& belief);

private:

    static std::string jsonString(const std::string& s);
    static std::string jsonDouble(double v);
    static std::string jsonInt(std::int64_t v);
};

} }
