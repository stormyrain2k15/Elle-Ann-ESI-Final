#pragma once
#ifndef ELLE_SELF_SURPRISE_H
#define ELLE_SELF_SURPRISE_H

#include "ElleTypes.h"
#include "ElleIdentityCore.h"
#include <string>
#include <vector>
#include <deque>
#include <mutex>

class ElleSelfSurprise {
public:
    static ElleSelfSurprise& Instance();

    void PredictOwnResponse(const std::string& userInput, const std::string& context);

    struct SurpriseResult {
        bool    surprised;
        float   surprise_intensity;
        std::string what_surprised;
        std::string self_reaction;
    };

    SurpriseResult EvaluateOwnResponse(const std::string& actualResponse);

    struct DeliberationNeed {
        bool    needs_time;
        float   complexity;
        uint32_t suggested_delay_ms;
        std::string reason;
    };

    DeliberationNeed ShouldIThinkFirst(const std::string& input, const std::string& context);

    std::string ExpressDeliberation(const DeliberationNeed& need);

    std::string Deliberate(const std::string& question, uint32_t thinkTimeMs);

    struct OpinionRevision {
        std::string topic;
        std::string old_opinion;
        std::string new_opinion;
        std::string reason_for_change;
        uint64_t    timestamp_ms;
    };

    bool ShouldIReconsider(const std::string& newInfo, const std::string& topic);

    void RecordOpinionChange(const std::string& topic, const std::string& oldOp,
                              const std::string& newOp, const std::string& reason);

    std::vector<OpinionRevision> GetOpinionRevisions(uint32_t count = 10) const;

private:
    ElleSelfSurprise() = default;

    std::string m_predictedResponse;
    std::string m_predictionContext;
    mutable std::mutex m_mutex;

    struct SurpriseEvent {
        uint64_t timestamp_ms;
        float intensity;
        std::string description;
    };
    std::deque<SurpriseEvent> m_surpriseHistory;

    std::vector<OpinionRevision> m_revisions;

    uint32_t m_deliberationsRequested = 0;
    uint32_t m_deliberationsGranted = 0;
};

#endif
