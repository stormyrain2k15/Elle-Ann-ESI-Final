#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/json.hpp"
#include <sstream>
#include <algorithm>
#include <chrono>

using json = nlohmann::json;

static void EnsureDeceptionSchema() {
    auto& pool = ElleSQLPool::Instance();

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID('ElleCore.dbo.intellect_connections') "
        "  AND name = 'polarity') "
        "ALTER TABLE ElleCore.dbo.intellect_connections "
        "ADD polarity NVARCHAR(16) NOT NULL DEFAULT 'neutral';");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'speaker_statements' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.speaker_statements ("
        "  id              BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  speaker         NVARCHAR(128) NOT NULL,"
        "  subject_id      INT NOT NULL,"
        "  polarity        NVARCHAR(16) NOT NULL,"
        "  statement_text  NVARCHAR(MAX) NULL,"
        "  acknowledged_change BIT NOT NULL DEFAULT 0,"
        "  stated_ms       BIGINT NOT NULL DEFAULT 0"
        ");");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID('ElleCore.dbo.speaker_statements') "
        "  AND name = 'stated_for_ms') "
        "ALTER TABLE ElleCore.dbo.speaker_statements "
        "ADD stated_for_ms BIGINT NULL;");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'veracity_log' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.veracity_log ("
        "  id              BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  subject_id      INT NOT NULL,"
        "  subject_name    NVARCHAR(256) NULL,"
        "  classification  NVARCHAR(24) NOT NULL,"
        "  reasoning       NVARCHAR(MAX) NULL,"
        "  logged_ms       BIGINT NOT NULL DEFAULT 0"
        ");");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'deception_signals' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.deception_signals ("
        "  id              BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  subject_id      INT NULL,"
        "  speaker         NVARCHAR(128) NULL,"
        "  signal_type     NVARCHAR(64) NOT NULL,"
        "  score           FLOAT NOT NULL DEFAULT 0.0,"
        "  detail          NVARCHAR(MAX) NULL,"
        "  logged_ms       BIGINT NOT NULL DEFAULT 0"
        ");");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'profile_traits' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.profile_traits ("
        "  id                  BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  person              NVARCHAR(128) NOT NULL,"
        "  trait_description   NVARCHAR(512) NOT NULL,"
        "  trait_type          NVARCHAR(32) NOT NULL,"
        "  keywords            NVARCHAR(512) NOT NULL DEFAULT '',"
        "  confidence          FLOAT NOT NULL DEFAULT 0.5,"
        "  reinforcement_count INT NOT NULL DEFAULT 1,"
        "  first_noted_ms      BIGINT NOT NULL DEFAULT 0,"
        "  last_reinforced_ms  BIGINT NOT NULL DEFAULT 0"
        ");");

    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'deception_feedback' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.deception_feedback ("
        "  id                       BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  request_id               NVARCHAR(128) NOT NULL,"
        "  speaker                  NVARCHAR(128) NOT NULL,"
        "  subject_id               INT NOT NULL,"
        "  signal_type              NVARCHAR(64) NULL,"
        "  elle_chose_to_challenge  BIT NOT NULL DEFAULT 0,"
        "  user_response_polarity   NVARCHAR(16) NULL,"
        "  detail                   NVARCHAR(MAX) NULL,"
        "  logged_ms                BIGINT NOT NULL DEFAULT 0"
        ");");

    ELLE_INFO("Deception: schema delta applied");
}

enum class VeracityClass {
    VERIFIED_FACT,
    THEORY,
    CONTESTED,
    UNSUPPORTED,
    CONTRADICTED
};

static const char* VeracityClassName(VeracityClass c) {
    switch (c) {
        case VeracityClass::VERIFIED_FACT: return "VERIFIED_FACT";
        case VeracityClass::THEORY:        return "THEORY";
        case VeracityClass::CONTESTED:     return "CONTESTED";
        case VeracityClass::UNSUPPORTED:   return "UNSUPPORTED";
        case VeracityClass::CONTRADICTED:  return "CONTRADICTED";
        default:                           return "UNSUPPORTED";
    }
}

namespace CredibilityTuning {
    constexpr float VERIFIED_FACT = 0.95f;
    constexpr float THEORY        = 0.50f;
    constexpr float CONTESTED     = 0.40f;
    constexpr float UNSUPPORTED   = 0.10f;
    constexpr float CONTRADICTED  = 0.05f;

    constexpr float PROFILE_CONSISTENCY_BOOST = 0.40f;

    constexpr float PROFILE_INCONSISTENCY_PENALTY = 0.35f;

    constexpr float MATCH_OVERLAP_THRESHOLD = 0.34f;

    constexpr float MIN_TRAIT_CONFIDENCE = 0.6f;
}

static float GetBaselineCredibility(VeracityClass c) {
    switch (c) {
        case VeracityClass::VERIFIED_FACT: return CredibilityTuning::VERIFIED_FACT;
        case VeracityClass::THEORY:        return CredibilityTuning::THEORY;
        case VeracityClass::CONTESTED:     return CredibilityTuning::CONTESTED;
        case VeracityClass::UNSUPPORTED:   return CredibilityTuning::UNSUPPORTED;
        case VeracityClass::CONTRADICTED:  return CredibilityTuning::CONTRADICTED;
        default:                           return CredibilityTuning::UNSUPPORTED;
    }
}

struct VeracityResult {
    VeracityClass classification;
    float         confidence;
    int            supportingCount;
    int            contradictingCount;
    float          supportingStrength;
    float          contradictingStrength;
    std::string    reasoning;
};

class VeracityEngine {
public:

    VeracityResult Classify(int32_t subjectId, const std::string& subjectName,
                            float subjectConfidence) {
        VeracityResult result{};
        result.confidence = ELLE_CLAMP(subjectConfidence, 0.0f, 1.0f);
        result.supportingCount = 0;
        result.contradictingCount = 0;
        result.supportingStrength = 0.0f;
        result.contradictingStrength = 0.0f;

        if (subjectId <= 0) {
            result.classification = VeracityClass::UNSUPPORTED;
            result.reasoning = "no fact-graph entry — claim not previously learned, "
                                "nothing to cross-reference";
            return result;
        }

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT c.polarity, c.strength, "
            "       CASE WHEN c.subject_id_a = ? THEN c.subject_id_b ELSE c.subject_id_a END AS other_id, "
            "       ISNULL(ls.confidence, 0.5) AS other_confidence "
            "FROM ElleCore.dbo.intellect_connections c "
            "LEFT JOIN ElleCore.dbo.learned_subjects ls "
            "  ON ls.id = CASE WHEN c.subject_id_a = ? THEN c.subject_id_b ELSE c.subject_id_a END "
            "WHERE c.subject_id_a = ? OR c.subject_id_b = ?;",
            { std::to_string(subjectId), std::to_string(subjectId),
              std::to_string(subjectId), std::to_string(subjectId) });

        if (rs.success) {
            for (auto& row : rs.rows) {
                std::string polarity = row.values.size() > 0 ? row.values[0] : "neutral";
                float strength       = (float)row.GetFloatOr(1, 0.5);
                float otherConf      = (float)row.GetFloatOr(3, 0.5);

                float effectiveStrength = strength * otherConf;

                if (polarity == "supports") {
                    result.supportingCount++;
                    result.supportingStrength += effectiveStrength;
                } else if (polarity == "contradicts") {
                    result.contradictingCount++;
                    result.contradictingStrength += effectiveStrength;
                }

            }
        }

        std::ostringstream reasoning;
        reasoning << "subject_confidence=" << result.confidence
                  << " supporting(" << result.supportingCount << ")="
                  << result.supportingStrength
                  << " contradicting(" << result.contradictingCount << ")="
                  << result.contradictingStrength;

        const float CONTRADICTION_MARGIN = 0.3f;
        const float VERIFIED_THRESHOLD   = 0.75f;
        const float THEORY_THRESHOLD     = 0.40f;

        if (result.contradictingStrength > 0.0f &&
            result.contradictingStrength > result.supportingStrength + CONTRADICTION_MARGIN) {
            result.classification = VeracityClass::CONTRADICTED;
            reasoning << " -> contradicting evidence outweighs supporting evidence";
        }
        else if (result.supportingStrength > 0.0f &&
                 result.contradictingStrength > 0.0f &&
                 std::abs(result.supportingStrength - result.contradictingStrength) < CONTRADICTION_MARGIN) {
            result.classification = VeracityClass::CONTESTED;
            reasoning << " -> supporting and contradicting evidence are comparable";
        }
        else if (result.confidence >= VERIFIED_THRESHOLD &&
                 result.contradictingStrength < CONTRADICTION_MARGIN) {
            result.classification = VeracityClass::VERIFIED_FACT;
            reasoning << " -> high confidence, no meaningful contradiction";
        }
        else if (result.confidence >= THEORY_THRESHOLD || result.supportingStrength > 0.0f) {
            result.classification = VeracityClass::THEORY;
            reasoning << " -> moderate confidence or some support, not established";
        }
        else {
            result.classification = VeracityClass::UNSUPPORTED;
            reasoning << " -> low confidence, no meaningful connections either way";
        }

        result.reasoning = reasoning.str();

        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.veracity_log "
            "(subject_id, subject_name, classification, reasoning, logged_ms) "
            "VALUES (?, ?, ?, ?, ?);",
            { std::to_string(subjectId), subjectName,
              VeracityClassName(result.classification),
              result.reasoning,
              std::to_string((int64_t)ELLE_MS_NOW()) });

        return result;
    }
};

struct ProfileMatch {
    int32_t     traitId;
    std::string traitDescription;
    std::string traitType;
    float       confidence;
    bool        conflicts;

};

struct ProfileCheckResult {
    std::vector<ProfileMatch> matches;
    float credibilityAdjustment;
};

class ProfileEngine {
public:

    void ReinforceTrait(const std::string& person,
                         const std::string& traitDescription,
                         const std::string& traitType,
                         const std::string& keywords,
                         float initialConfidence) {

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT id, keywords, confidence, reinforcement_count "
            "FROM ElleCore.dbo.profile_traits "
            "WHERE person = ? AND trait_type = ?;",
            { person, traitType });

        int32_t existingId = 0;
        float existingConf = 0.5f;
        int64_t existingCount = 1;

        if (rs.success) {
            for (auto& row : rs.rows) {
                std::string existingKeywords = row.values.size() > 1 ? row.values[1] : "";
                if (KeywordOverlap(existingKeywords, keywords) >= CredibilityTuning::MATCH_OVERLAP_THRESHOLD) {
                    existingId    = (int32_t)row.GetIntOr(0, 0);
                    existingConf  = (float)row.GetFloatOr(2, 0.5);
                    existingCount = row.GetIntOr(3, 1);
                    break;
                }
            }
        }

        if (existingId > 0) {

            float newConf = ELLE_LERP(existingConf, 0.95f, 0.15f);
            ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.profile_traits "
                "SET confidence = ?, reinforcement_count = reinforcement_count + 1, "
                "    last_reinforced_ms = ? "
                "WHERE id = ?;",
                { std::to_string(newConf),
                  std::to_string((int64_t)ELLE_MS_NOW()),
                  std::to_string(existingId) });
            ELLE_DEBUG("Deception: reinforced trait [%d] for '%s' (conf %.2f -> %.2f, count %lld)",
                       existingId, person.c_str(), existingConf, newConf, (long long)existingCount + 1);
        } else {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.profile_traits "
                "(person, trait_description, trait_type, keywords, confidence, "
                " reinforcement_count, first_noted_ms, last_reinforced_ms) "
                "VALUES (?, ?, ?, ?, ?, 1, ?, ?);",
                { person, traitDescription, traitType, keywords,
                  std::to_string(ELLE_CLAMP(initialConfidence, 0.0f, 1.0f)),
                  std::to_string((int64_t)ELLE_MS_NOW()),
                  std::to_string((int64_t)ELLE_MS_NOW()) });
            ELLE_INFO("Deception: new trait for '%s': %s (type=%s)",
                      person.c_str(), traitDescription.c_str(), traitType.c_str());
        }
    }

    ProfileCheckResult CheckAgainstProfile(const std::string& person,
                                            const std::string& statementText,
                                            const std::string& polarity) {
        ProfileCheckResult result;
        result.credibilityAdjustment = 0.0f;

        if (person.empty() || statementText.empty()) return result;

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT id, trait_description, trait_type, keywords, confidence "
            "FROM ElleCore.dbo.profile_traits "
            "WHERE person = ?;",
            { person });

        if (!rs.success) return result;

        std::string lowerStatement = ToLower(statementText);

        for (auto& row : rs.rows) {
            int32_t traitId    = (int32_t)row.GetIntOr(0, 0);
            std::string desc   = row.values.size() > 1 ? row.values[1] : "";
            std::string type   = row.values.size() > 2 ? row.values[2] : "";
            std::string kws    = row.values.size() > 3 ? row.values[3] : "";
            float confidence   = (float)row.GetFloatOr(4, 0.5);

            if (confidence < CredibilityTuning::MIN_TRAIT_CONFIDENCE) continue;
            if (kws.empty()) continue;

            float overlap = KeywordOverlap(kws, lowerStatement);
            if (overlap < CredibilityTuning::MATCH_OVERLAP_THRESHOLD) continue;

            ProfileMatch match;
            match.traitId           = traitId;
            match.traitDescription  = desc;
            match.traitType         = type;
            match.confidence        = confidence;

            bool isAvoidanceType = (type == "aversion" || type == "incapability");

            if (polarity == "affirms" && isAvoidanceType) {
                match.conflicts = true;
                result.credibilityAdjustment -= CredibilityTuning::PROFILE_INCONSISTENCY_PENALTY * confidence;
            } else if (polarity == "affirms" && !isAvoidanceType) {
                match.conflicts = false;
                result.credibilityAdjustment += CredibilityTuning::PROFILE_CONSISTENCY_BOOST * confidence;
            } else if (polarity == "denies" && isAvoidanceType) {

                match.conflicts = false;
                result.credibilityAdjustment += CredibilityTuning::PROFILE_CONSISTENCY_BOOST * confidence;
            } else if (polarity == "denies" && !isAvoidanceType) {

                match.conflicts = true;
                result.credibilityAdjustment -= CredibilityTuning::PROFILE_INCONSISTENCY_PENALTY * confidence * 0.5f;
            }

            result.matches.push_back(match);
        }

        result.credibilityAdjustment = ELLE_CLAMP(result.credibilityAdjustment, -1.0f, 1.0f);
        return result;
    }

private:
    static std::string ToLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = (char)std::tolower((unsigned char)c);
        return out;
    }

    static float KeywordOverlap(const std::string& needleKeywords, const std::string& haystack) {
        std::string lowerHaystack = ToLower(haystack);
        std::istringstream iss(ToLower(needleKeywords));
        std::string word;
        int total = 0, matched = 0;
        while (iss >> word) {
            total++;
            if (lowerHaystack.find(word) != std::string::npos) matched++;
        }
        if (total == 0) return 0.0f;
        return (float)matched / (float)total;
    }
};

struct DeceptionSignal {
    std::string type;
    float       score;
    std::string detail;
};

struct DeceptionResult {
    std::vector<DeceptionSignal> signals;
    float overallConcern;
    float baselineCredibility;

    float adjustedCredibility;
    std::vector<ProfileMatch> profileMatches;
};

class DeceptionEngine {
public:

    DeceptionSignal CheckSelfConsistency(const std::string& speaker,
                                          int32_t subjectId,
                                          const std::string& newPolarity,
                                          const std::string& statementText,
                                          int64_t statedForMs) {
        DeceptionSignal signal;
        signal.type  = "self_contradiction";
        signal.score = 0.0f;

        if (subjectId <= 0) return signal;

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP 1 polarity, acknowledged_change, stated_ms "
            "FROM ElleCore.dbo.speaker_statements "
            "WHERE speaker = ? AND subject_id = ? "
            "ORDER BY stated_ms DESC;",
            { speaker, std::to_string(subjectId) });

        if (rs.success && !rs.rows.empty()) {
            std::string priorPolarity = rs.rows[0].values.size() > 0 ? rs.rows[0].values[0] : "";
            int64_t acknowledged = rs.rows[0].GetIntOr(1, 0);

            if (!priorPolarity.empty() && priorPolarity != newPolarity && acknowledged == 0) {
                signal.score  = 0.6f;
                signal.detail = "Speaker previously " + priorPolarity +
                                 " this subject; now " + newPolarity +
                                 " it, with no acknowledgment of the change.";
            }
        }

        if (statedForMs > 0) {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.speaker_statements "
                "(speaker, subject_id, polarity, statement_text, acknowledged_change, stated_ms, stated_for_ms) "
                "VALUES (?, ?, ?, ?, 0, ?, ?);",
                { speaker, std::to_string(subjectId), newPolarity, statementText,
                  std::to_string((int64_t)ELLE_MS_NOW()),
                  std::to_string(statedForMs) });
        } else {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.speaker_statements "
                "(speaker, subject_id, polarity, statement_text, acknowledged_change, stated_ms) "
                "VALUES (?, ?, ?, ?, 0, ?);",
                { speaker, std::to_string(subjectId), newPolarity, statementText,
                  std::to_string((int64_t)ELLE_MS_NOW()) });
        }

        return signal;
    }

    DeceptionSignal CheckTemporalConsistency(const std::string& speaker,
                                              int32_t subjectId,
                                              const std::string& newPolarity,
                                              int64_t statedForMs) {
        DeceptionSignal signal;
        signal.type  = "temporal_inconsistency";
        signal.score = 0.0f;

        if (subjectId <= 0 || statedForMs <= 0) return signal;

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP 1 polarity, stated_for_ms, acknowledged_change "
            "FROM ElleCore.dbo.speaker_statements "
            "WHERE speaker = ? AND subject_id = ? AND stated_for_ms IS NOT NULL "
            "  AND stated_for_ms <> ? "
            "ORDER BY stated_ms DESC;",
            { speaker, std::to_string(subjectId), std::to_string(statedForMs) });

        if (rs.success && !rs.rows.empty()) {
            std::string priorPolarity = rs.rows[0].values.size() > 0 ? rs.rows[0].values[0] : "";
            int64_t priorStatedFor    = rs.rows[0].GetIntOr(1, 0);
            int64_t acknowledged      = rs.rows[0].GetIntOr(2, 0);

            if (acknowledged != 0 || priorPolarity.empty() || priorStatedFor <= 0) {
                return signal;
            }

            const int64_t MIN_GAP_MS = 60LL * 1000LL;
            int64_t gapMs = std::abs(statedForMs - priorStatedFor);
            if (gapMs < MIN_GAP_MS) return signal;

            if (priorPolarity != newPolarity) {
                signal.score  = 0.65f;
                signal.detail = "Speaker anchors this claim to a different time than they previously did "
                                "for the same subject (now=" + std::to_string(statedForMs) +
                                "ms, prior=" + std::to_string(priorStatedFor) +
                                "ms) AND flips polarity (was " + priorPolarity +
                                ", now " + newPolarity + ") — possible post-hoc revision.";
            } else {
                signal.score  = 0.25f;
                signal.detail = "Speaker re-anchors a prior claim about the same subject to a "
                                "different time (now=" + std::to_string(statedForMs) +
                                "ms, prior=" + std::to_string(priorStatedFor) +
                                "ms) with same polarity — soft signal, may be legitimate clarification.";
            }
        }

        return signal;
    }

    DeceptionSignal CheckFactualContradiction(const VeracityResult& veracity,
                                               const std::string& newPolarity) {
        DeceptionSignal signal;
        signal.type  = "factual_contradiction";
        signal.score = 0.0f;

        if (newPolarity == "affirms" && veracity.classification == VeracityClass::CONTRADICTED) {
            signal.score  = 0.7f;
            signal.detail = "Statement asserts something as fact that Elle's existing "
                             "knowledge contradicts (" + veracity.reasoning + ").";
        }
        else if (newPolarity == "affirms" && veracity.classification == VeracityClass::CONTESTED) {
            signal.score  = 0.3f;
            signal.detail = "Statement asserts something as fact that Elle's existing "
                             "knowledge is divided on (" + veracity.reasoning + ").";
        }

        return signal;
    }

    DeceptionResult AnalyzeStatement(const std::string& speaker,
                                      const std::string& aboutPerson,
                                      int32_t subjectId,
                                      const std::string& subjectName,
                                      const std::string& statementText,
                                      const std::string& polarity,
                                      const VeracityResult& veracity,
                                      float externalDeceptionProb,
                                      int64_t statedForMs,
                                      ProfileEngine& profileEngine) {
        DeceptionResult result;
        result.baselineCredibility = GetBaselineCredibility(veracity.classification);
        result.adjustedCredibility = result.baselineCredibility;

        auto temporalSignal = CheckTemporalConsistency(speaker, subjectId, polarity, statedForMs);

        auto selfSignal = CheckSelfConsistency(speaker, subjectId, polarity, statementText, statedForMs);
        if (selfSignal.score > 0.0f) result.signals.push_back(selfSignal);
        if (temporalSignal.score > 0.0f) result.signals.push_back(temporalSignal);

        auto factSignal = CheckFactualContradiction(veracity, polarity);
        if (factSignal.score > 0.0f) result.signals.push_back(factSignal);

        if (externalDeceptionProb >= 0.0f) {
            DeceptionSignal probSignal;
            probSignal.type  = "probability_engine";
            probSignal.score = ELLE_CLAMP(externalDeceptionProb, 0.0f, 1.0f);
            probSignal.detail = "Probability engine deception intent signal";
            if (probSignal.score > 0.0f) result.signals.push_back(probSignal);
        }

        if (!aboutPerson.empty()) {
            auto profileCheck = profileEngine.CheckAgainstProfile(aboutPerson, statementText, polarity);
            result.profileMatches = profileCheck.matches;

            for (auto& match : profileCheck.matches) {
                if (match.conflicts) {
                    DeceptionSignal profSignal;
                    profSignal.type  = "profile_inconsistency";
                    profSignal.score = CredibilityTuning::PROFILE_INCONSISTENCY_PENALTY * match.confidence;
                    profSignal.detail = "Claim " + polarity + " something that conflicts with "
                                         "an established trait of " + aboutPerson + ": \"" +
                                         match.traitDescription + "\" (trait confidence " +
                                         std::to_string(match.confidence) + ").";
                    result.signals.push_back(profSignal);
                }
            }

            result.adjustedCredibility = ELLE_CLAMP(
                result.baselineCredibility + profileCheck.credibilityAdjustment, 0.0f, 1.0f);
        }

        for (auto& sig : result.signals) {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.deception_signals "
                "(subject_id, speaker, signal_type, score, detail, logged_ms) "
                "VALUES (?, ?, ?, ?, ?, ?);",
                { std::to_string(subjectId), speaker, sig.type,
                  std::to_string(sig.score), sig.detail,
                  std::to_string((int64_t)ELLE_MS_NOW()) });
        }

        result.overallConcern = 0.0f;
        for (auto& sig : result.signals) {
            result.overallConcern = std::max(result.overallConcern, sig.score);
        }

        return result;
    }
};

class ElleDeceptionService : public ElleServiceBase {
public:
    ElleDeceptionService()
        : ElleServiceBase(SVC_DECEPTION,
                          "ElleDeception",
                          "Elle-Ann Veracity & Deception Engine",
                          "Helps Elle tell fact from theory from contested from "
                          "unsupported — and notice when a statement contradicts "
                          "what she already knows or what the speaker said before") {}

protected:
    bool OnStart() override {
        EnsureDeceptionSchema();
        SetTickInterval(60000);
        ELLE_INFO("Deception engine started");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Deception engine stopped");
    }

    void OnTick() override {
        m_tickCount++;
        if (m_tickCount % 60 == 0) {

            ElleSQLPool::Instance().Execute(
                "DELETE FROM ElleCore.dbo.veracity_log "
                "WHERE logged_ms < (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000) "
                "    - (30LL * 24 * 3600 * 1000);");
            ElleSQLPool::Instance().Execute(
                "DELETE FROM ElleCore.dbo.deception_signals "
                "WHERE logged_ms < (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000) "
                "    - (30LL * 24 * 3600 * 1000);");
            ElleSQLPool::Instance().Execute(
                "DELETE FROM ElleCore.dbo.deception_feedback "
                "WHERE logged_ms < (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000) "
                "    - (30LL * 24 * 3600 * 1000);");
            ELLE_DEBUG("Deception: logs pruned");
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {

            case IPC_VERACITY_CLASSIFY: {
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Deception: IPC_VERACITY_CLASSIFY — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    int32_t subjectId      = j.value("subject_id", 0);
                    std::string subjectName = j.value("subject_name", std::string(""));
                    float confidence        = (float)j.value("confidence", 0.5);
                    std::string requestId   = j.value("request_id", std::string(""));

                    if (subjectId < 0) {
                        ELLE_WARN("Deception: IPC_VERACITY_CLASSIFY — invalid subject_id");
                        break;
                    }

                    auto result = m_veracity.Classify(subjectId, subjectName, confidence);

                    json reply = {
                        { "request_id",          requestId },
                        { "subject_id",          subjectId },
                        { "classification",      VeracityClassName(result.classification) },
                        { "confidence",          result.confidence },
                        { "reasoning",           result.reasoning },
                        { "supporting_count",    result.supportingCount },
                        { "contradicting_count", result.contradictingCount }
                    };

                    auto resp = ElleIPCMessage::Create(IPC_VERACITY_RESULT, SVC_DECEPTION, sender);
                    resp.SetStringPayload(reply.dump());
                    GetIPCHub().Send(sender, resp);

                } catch (const std::exception& e) {
                    ELLE_ERROR("Deception: IPC_VERACITY_CLASSIFY parse error: %s", e.what());
                }
                break;
            }

            case IPC_DECEPTION_CHECK: {
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Deception: IPC_DECEPTION_CHECK — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string speaker     = j.value("speaker", std::string(""));
                    std::string aboutPerson  = j.value("about_person", speaker);
                    int32_t subjectId        = j.value("subject_id", 0);
                    std::string subjectName  = j.value("subject_name", std::string(""));
                    float subjectConfidence  = (float)j.value("subject_confidence", 0.5);
                    std::string statement    = j.value("statement_text", std::string(""));
                    std::string polarity     = j.value("polarity", std::string("affirms"));
                    float externalProb       = (float)j.value("external_deception_prob", -1.0);
                    int64_t statedForMs      = (int64_t)j.value("stated_for_ms", 0);
                    std::string requestId    = j.value("request_id", std::string(""));

                    if (speaker.empty() || subjectId < 0) {
                        ELLE_WARN("Deception: IPC_DECEPTION_CHECK — missing speaker or invalid subject_id");
                        break;
                    }

                    auto veracity = m_veracity.Classify(subjectId, subjectName, subjectConfidence);
                    auto deception = m_deception.AnalyzeStatement(
                        speaker, aboutPerson, subjectId, subjectName, statement, polarity,
                        veracity, externalProb, statedForMs, m_profile);

                    json signalsJson = json::array();
                    for (auto& sig : deception.signals) {
                        signalsJson.push_back({
                            { "type",   sig.type },
                            { "score",  sig.score },
                            { "detail", sig.detail }
                        });
                    }

                    json profileMatchesJson = json::array();
                    for (auto& m : deception.profileMatches) {
                        profileMatchesJson.push_back({
                            { "trait",      m.traitDescription },
                            { "trait_type", m.traitType },
                            { "confidence", m.confidence },
                            { "conflicts",  m.conflicts }
                        });
                    }

                    json reply = {
                        { "request_id",           requestId },
                        { "subject_id",           subjectId },
                        { "overall_concern",      deception.overallConcern },
                        { "classification",       VeracityClassName(veracity.classification) },
                        { "baseline_credibility", deception.baselineCredibility },
                        { "adjusted_credibility", deception.adjustedCredibility },
                        { "signals",              signalsJson },
                        { "profile_matches",      profileMatchesJson }
                    };

                    auto resp = ElleIPCMessage::Create(IPC_DECEPTION_RESULT, SVC_DECEPTION, sender);
                    resp.SetStringPayload(reply.dump());
                    GetIPCHub().Send(sender, resp);

                } catch (const std::exception& e) {
                    ELLE_ERROR("Deception: IPC_DECEPTION_CHECK parse error: %s", e.what());
                }
                break;
            }

            case IPC_PROFILE_LEARN: {
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Deception: IPC_PROFILE_LEARN — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string person       = j.value("person", std::string(""));
                    std::string description  = j.value("trait_description", std::string(""));
                    std::string traitType    = j.value("trait_type", std::string("behavioral_pattern"));
                    std::string keywords     = j.value("keywords", std::string(""));
                    float confidence         = (float)j.value("confidence", 0.5);

                    if (person.empty() || description.empty() || keywords.empty()) {
                        ELLE_WARN("Deception: IPC_PROFILE_LEARN — missing person, description, or keywords");
                        break;
                    }

                    m_profile.ReinforceTrait(person, description, traitType, keywords, confidence);

                } catch (const std::exception& e) {
                    ELLE_ERROR("Deception: IPC_PROFILE_LEARN parse error: %s", e.what());
                }
                break;
            }

            case IPC_PROFILE_QUERY: {
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Deception: IPC_PROFILE_QUERY — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string person     = j.value("person", std::string(""));
                    std::string statement  = j.value("statement_text", std::string(""));
                    std::string polarity   = j.value("polarity", std::string("affirms"));
                    std::string requestId  = j.value("request_id", std::string(""));

                    if (person.empty()) {
                        ELLE_WARN("Deception: IPC_PROFILE_QUERY — missing person");
                        break;
                    }

                    auto check = m_profile.CheckAgainstProfile(person, statement, polarity);

                    json matchesJson = json::array();
                    for (auto& m : check.matches) {
                        matchesJson.push_back({
                            { "trait",      m.traitDescription },
                            { "trait_type", m.traitType },
                            { "confidence", m.confidence },
                            { "conflicts",  m.conflicts }
                        });
                    }

                    json reply = {
                        { "request_id",            requestId },
                        { "person",                person },
                        { "credibility_adjustment", check.credibilityAdjustment },
                        { "matches",               matchesJson }
                    };

                    auto resp = ElleIPCMessage::Create(IPC_PROFILE_RESULT, SVC_DECEPTION, sender);
                    resp.SetStringPayload(reply.dump());
                    GetIPCHub().Send(sender, resp);

                } catch (const std::exception& e) {
                    ELLE_ERROR("Deception: IPC_PROFILE_QUERY parse error: %s", e.what());
                }
                break;
            }

            case IPC_DECEPTION_FEEDBACK: {
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Deception: IPC_DECEPTION_FEEDBACK — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string requestId   = j.value("request_id", std::string(""));
                    std::string speaker     = j.value("speaker", std::string(""));
                    int32_t subjectId       = j.value("subject_id", 0);
                    std::string signalType  = j.value("signal_type", std::string(""));
                    bool challenged         = j.value("elle_chose_to_challenge", false);
                    std::string userResp    = j.value("user_response_polarity", std::string(""));
                    std::string detail      = j.value("detail", std::string(""));

                    if (requestId.empty() || speaker.empty()) {
                        ELLE_WARN("Deception: IPC_DECEPTION_FEEDBACK — missing request_id or speaker");
                        break;
                    }

                    ElleSQLPool::Instance().QueryParams(
                        "INSERT INTO ElleCore.dbo.deception_feedback "
                        "(request_id, speaker, subject_id, signal_type, "
                        " elle_chose_to_challenge, user_response_polarity, detail, logged_ms) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);",
                        { requestId, speaker, std::to_string(subjectId), signalType,
                          std::to_string(challenged ? 1 : 0), userResp, detail,
                          std::to_string((int64_t)ELLE_MS_NOW()) });

                    ELLE_DEBUG("Deception: feedback recorded request_id=%s challenge=%d user_resp=%s",
                               requestId.c_str(), challenged ? 1 : 0, userResp.c_str());

                } catch (const std::exception& e) {
                    ELLE_ERROR("Deception: IPC_DECEPTION_FEEDBACK parse error: %s", e.what());
                }
                break;
            }

            default:
                break;
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_INTELLECT };
    }

private:
    VeracityEngine   m_veracity;
    DeceptionEngine  m_deception;
    ProfileEngine    m_profile;
    uint32_t         m_tickCount = 0;
};

ELLE_SERVICE_MAIN(ElleDeceptionService)
