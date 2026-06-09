#include "ElleLexicalAdmin.h"
#include "ElleSQLConn.h"
#include "ElleLogger.h"

namespace ElleLanguage {

bool FetchLexicalAuditReport(LexicalAuditReport& outReport,
                             double minScore,
                             int    maxRows) {
    outReport.rows.clear();
    outReport.summary = AuditSummary{};

    auto& pool = ElleSQLPool::Instance();

    const int capped = (maxRows <= 0 || maxRows > 5000) ? 500 : maxRows;

    auto rs = pool.QueryParams(
        "SELECT TOP (" + std::to_string(capped) + ") "
        "    WordID, Lemma, NormalizedLemma, "
        "    IsCognitivelyComplete, CompletenessScore, MissingRequirements, "
        "    SenseCount, SensesWithUsage, SensesWithContext, SensesWithEmotion, "
        "    WordRelCount, SenseRelCount, ConceptCount, AnagramKey "
        "FROM EllesLanguage.dbo.vw_LexicalCompletenessVerdict "
        "WHERE CompletenessScore >= ? "
        "ORDER BY IsCognitivelyComplete ASC, CompletenessScore ASC, Lemma ASC;",
        { std::to_string(minScore) });

    if (!rs.success) {
        ELLE_ERROR("LexicalAdmin: failed to query vw_LexicalCompletenessVerdict");
        return false;
    }

    outReport.rows.reserve(rs.rows.size());
    for (auto& row : rs.rows) {
        IncompleteWordRow r;
        r.wordId                = row.GetIntOr(0, 0);
        r.lemma                 = row.values.size() > 1 ? row.values[1] : std::string();
        r.normalizedLemma       = row.values.size() > 2 ? row.values[2] : std::string();
        r.isCognitivelyComplete = row.GetIntOr(3, 0) != 0;
        r.completenessScore     = row.GetFloatOr(4, 0.0);
        r.missingRequirements   = row.values.size() > 5 ? row.values[5] : std::string();
        r.senseCount            = (int)row.GetIntOr(6, 0);
        r.sensesWithUsage       = (int)row.GetIntOr(7, 0);
        r.sensesWithContext     = (int)row.GetIntOr(8, 0);
        r.sensesWithEmotion     = (int)row.GetIntOr(9, 0);
        r.wordRelCount          = (int)row.GetIntOr(10, 0);
        r.senseRelCount         = (int)row.GetIntOr(11, 0);
        r.conceptCount          = (int)row.GetIntOr(12, 0);
        r.anagramKey            = row.values.size() > 13 ? row.values[13] : std::string();
        outReport.rows.push_back(std::move(r));
    }

    auto sum = pool.Query(
        "SELECT COUNT(*) AS TotalWords, "
        "       SUM(CAST(IsCognitivelyComplete AS INT)) AS CompleteWords, "
        "       SUM(CASE WHEN IsCognitivelyComplete = 0 THEN 1 ELSE 0 END) AS IncompleteWords, "
        "       AVG(CompletenessScore) AS AvgScore "
        "FROM EllesLanguage.dbo.vw_LexicalCompletenessVerdict;");
    if (sum.success && !sum.rows.empty()) {
        auto& row = sum.rows[0];
        outReport.summary.totalWords           = row.GetIntOr(0, 0);
        outReport.summary.completeWords        = row.GetIntOr(1, 0);
        outReport.summary.incompleteWords      = row.GetIntOr(2, 0);
        outReport.summary.avgCompletenessScore = row.GetFloatOr(3, 0.0);
    } else {
        ELLE_WARN("LexicalAdmin: summary roll-up query returned no rows");
    }

    return true;
}

nlohmann::json LexicalAuditReportToJson(const LexicalAuditReport& report) {
    nlohmann::json out;
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& r : report.rows) {
        nlohmann::json row;
        row["word_id"]                  = r.wordId;
        row["lemma"]                    = r.lemma;
        row["normalized_lemma"]         = r.normalizedLemma;
        row["is_cognitively_complete"]  = r.isCognitivelyComplete;
        row["completeness_score"]       = r.completenessScore;
        row["missing_requirements"]     = r.missingRequirements;
        row["sense_count"]              = r.senseCount;
        row["senses_with_usage"]        = r.sensesWithUsage;
        row["senses_with_context"]      = r.sensesWithContext;
        row["senses_with_emotion"]      = r.sensesWithEmotion;
        row["word_rel_count"]           = r.wordRelCount;
        row["sense_rel_count"]          = r.senseRelCount;
        row["concept_count"]            = r.conceptCount;
        row["anagram_key"]              = r.anagramKey;
        rows.push_back(std::move(row));
    }
    out["rows"] = std::move(rows);

    nlohmann::json summary;
    summary["total_words"]             = report.summary.totalWords;
    summary["complete_words"]          = report.summary.completeWords;
    summary["incomplete_words"]        = report.summary.incompleteWords;
    summary["avg_completeness_score"]  = report.summary.avgCompletenessScore;
    out["summary"] = std::move(summary);

    return out;
}

}
