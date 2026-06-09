#include "ElleIntentLabelVocab.h"
#include "ElleSQLConn.h"
#include "ElleLogger.h"

namespace ElleConscience {

bool LoadVocabFromSql(IntentLabelVocab& vocab = IntentLabelVocab::Instance()) {
    auto& pool = ElleSQLPool::Instance();
    auto rs = pool.Query(
        "SELECT category, pattern FROM ElleHeart.dbo.vw_IntentLabelVocab;");
    if (!rs.success) {
        ELLE_WARN("IntentLabelVocab: SQL load failed; keeping in-memory defaults");
        return false;
    }

    std::vector<std::string> harm, deception, coercion;
    for (auto& row : rs.rows) {
        const std::string cat     = row.values.size() > 0 ? row.values[0] : std::string();
        const std::string pattern = row.values.size() > 1 ? row.values[1] : std::string();
        if (pattern.empty()) continue;

        if      (cat == "HARM")      harm.push_back(pattern);
        else if (cat == "DECEPTION") deception.push_back(pattern);
        else if (cat == "COERCION")  coercion.push_back(pattern);
    }

    if (!harm.empty())      vocab.setCategoryPatterns(HarmCategory::HARM,      std::move(harm));
    if (!deception.empty()) vocab.setCategoryPatterns(HarmCategory::DECEPTION, std::move(deception));
    if (!coercion.empty())  vocab.setCategoryPatterns(HarmCategory::COERCION,  std::move(coercion));

    ELLE_INFO("IntentLabelVocab: loaded %zu patterns from ElleHeart.dbo.vw_IntentLabelVocab",
              vocab.totalPatternCount());
    return true;
}

}
