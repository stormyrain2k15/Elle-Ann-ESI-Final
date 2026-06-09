#pragma once

#include "../_Shared/json.hpp"

#include <string>
#include <vector>

namespace ElleBeliefAdmin {

struct BeliefAuditRow {
    long long   auditId;
    long long   domainId;
    std::string domainCode;
    std::string operation;
    int         evidenceCount;
    double      entropyBefore;
    double      entropyAfter;
    long long   mapHypothesisId;
    double      mapProbability;
    long long   recordedMs;
    std::string detail;
};

struct BeliefSnapshotRow {
    long long   domainId;
    std::string domainCode;
    double      halfLifeSecs;
    long long   lastUpdatedMs;
    long long   hypothesisId;
    double      posteriorMass;
    double      priorMass;
};

bool FetchBeliefAudit(std::vector<BeliefAuditRow>& out,
                      const std::string& domainCode,
                      long long sinceMs,
                      int        limit);

bool FetchBeliefSnapshot(std::vector<BeliefSnapshotRow>& out,
                         const std::string& domainCode);

nlohmann::json BeliefAuditToJson(const std::vector<BeliefAuditRow>& rows);
nlohmann::json BeliefSnapshotToJson(const std::vector<BeliefSnapshotRow>& rows);

}
