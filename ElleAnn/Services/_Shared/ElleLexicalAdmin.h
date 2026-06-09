#pragma once

#include "../_Shared/json.hpp"

#include <string>
#include <vector>

namespace ElleLanguage {

struct IncompleteWordRow {
    long long   wordId;
    std::string lemma;
    std::string normalizedLemma;
    bool        isCognitivelyComplete;
    double      completenessScore;
    std::string missingRequirements;
    int         senseCount;
    int         sensesWithUsage;
    int         sensesWithContext;
    int         sensesWithEmotion;
    int         wordRelCount;
    int         senseRelCount;
    int         conceptCount;
    std::string anagramKey;
};

struct AuditSummary {
    long long totalWords;
    long long completeWords;
    long long incompleteWords;
    double    avgCompletenessScore;
};

struct LexicalAuditReport {
    std::vector<IncompleteWordRow> rows;
    AuditSummary                   summary;
};

bool FetchLexicalAuditReport(LexicalAuditReport& outReport,
                             double minScore  = 0.0,
                             int    maxRows   = 500);

nlohmann::json LexicalAuditReportToJson(const LexicalAuditReport& report);

}
