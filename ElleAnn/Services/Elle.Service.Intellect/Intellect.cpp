// ---------------------------------------------------------------------------
// Elle.Service.Intellect
//
// Elle's knowledge engine. Two functions:
//   1. UPDATE — Elle writes to her knowledge database as she learns.
//      Triggered by IPC_INTELLECT_LEARN from Cognitive when a conversation
//      surfaces a subject worth retaining. No external inference — she
//      classifies and stores from what she already received.
//
//   2. QUERY — Any service can ask the Intellect engine for what Elle knows
//      about a subject. Returns subject record, references, milestones, and
//      related skills. Cognitive calls this during context building.
//
// This service owns the knowledge database. Nothing else writes to it.
// She learns from conversation. She calls on it at will.
// ---------------------------------------------------------------------------

#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleDB_Content.h"
#include <sstream>
#include <algorithm>
#include <chrono>

// IPC message types for Intellect
// These should be added to ElleTypes.h:
//   IPC_INTELLECT_LEARN   — payload: JSON { subject, category, content, source, confidence }
//   IPC_INTELLECT_QUERY   — payload: JSON { query, category_hint, limit }
//   IPC_INTELLECT_RESULT  — payload: JSON (response to IPC_INTELLECT_QUERY)

// ---------------------------------------------------------------------------
// Schema delta — runs on start, additive only, never modifies existing columns
// ---------------------------------------------------------------------------
static void EnsureIntellectSchema() {
    auto& pool = ElleSQLPool::Instance();

    // Add last_accessed_ms to learned_subjects if not present
    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID('ElleCore.dbo.learned_subjects') "
        "  AND name = 'last_accessed_ms') "
        "ALTER TABLE ElleCore.dbo.learned_subjects "
        "ADD last_accessed_ms BIGINT NOT NULL DEFAULT 0;");

    // Add confidence to learned_subjects if not present
    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID('ElleCore.dbo.learned_subjects') "
        "  AND name = 'confidence') "
        "ALTER TABLE ElleCore.dbo.learned_subjects "
        "ADD confidence FLOAT NOT NULL DEFAULT 0.5;");

    // Add last_reviewed_ms to skills if not present
    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID('ElleCore.dbo.skills') "
        "  AND name = 'last_reviewed_ms') "
        "ALTER TABLE ElleCore.dbo.skills "
        "ADD last_reviewed_ms BIGINT NOT NULL DEFAULT 0;");

    // intellect_connections — relationships Elle has noticed between subjects
    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'intellect_connections' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.intellect_connections ("
        "  id              BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  subject_id_a    INT NOT NULL,"
        "  subject_id_b    INT NOT NULL,"
        "  relationship    NVARCHAR(256) NOT NULL,"  // e.g. "builds on", "contradicts", "requires"
        "  strength        FLOAT NOT NULL DEFAULT 0.5,"
        "  source          NVARCHAR(256) NULL,"      // where this connection was noticed
        "  noted_ms        BIGINT NOT NULL DEFAULT 0"
        ");");

    // intellect_log — record of every knowledge read or write
    pool.Execute(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'intellect_log' AND s.name = 'dbo') "
        "CREATE TABLE ElleCore.dbo.intellect_log ("
        "  id              BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  operation       NVARCHAR(16) NOT NULL,"  // READ or WRITE
        "  subject_id      INT NULL,"
        "  subject_name    NVARCHAR(256) NULL,"
        "  trigger_source  NVARCHAR(64) NULL,"      // which service triggered it
        "  detail          NVARCHAR(MAX) NULL,"
        "  logged_ms       BIGINT NOT NULL DEFAULT 0"
        ");");

    ELLE_INFO("Intellect: schema delta applied");
}

// ---------------------------------------------------------------------------
// Intellect engine — learn, recall, connect
// ---------------------------------------------------------------------------
class IntellectEngine {
public:

    // Learn from a conversation event.
    // Called when Cognitive surfaces a subject worth retaining.
    // subject      — what was learned (e.g. "tensor calculus", "Crystal dislikes cilantro")
    // category     — broad area (e.g. "mathematics", "Crystal", "personal")
    // content      — the actual knowledge content to store as a reference
    // source       — where it came from (e.g. "conversation", "self-observation")
    // confidence   — how certain Elle is (0.0–1.0)
    void Learn(const std::string& subject,
               const std::string& category,
               const std::string& content,
               const std::string& source,
               float confidence) {

        if (subject.empty() || content.empty()) {
            ELLE_WARN("Intellect::Learn called with empty subject or content — skipped");
            return;
        }

        confidence = ELLE_CLAMP(confidence, 0.0f, 1.0f);

        // Check if this subject already exists
        std::vector<ElleDB::LearnedSubject> existing;
        ElleDB::ListSubjects(existing, category, 100);

        int32_t subjectId = 0;
        std::string lowerSubject = ToLower(subject);

        for (auto& s : existing) {
            if (ToLower(s.subject) == lowerSubject) {
                subjectId = s.id;
                break;
            }
        }

        if (subjectId == 0) {
            // New subject — create it
            ElleDB::LearnedSubject ns;
            ns.subject           = subject;
            ns.category          = category;
            ns.proficiency_level = 1;
            ns.where_learned     = source;
            ns.notes             = "";

            if (!ElleDB::CreateSubject(ns, subjectId) || subjectId == 0) {
                ELLE_ERROR("Intellect: CreateSubject failed for '%s'", subject.c_str());
                return;
            }
            ELLE_INFO("Intellect: new subject [%d] '%s' (cat=%s conf=%.2f)",
                      subjectId, subject.c_str(), category.c_str(), confidence);
        } else {
            // Existing subject — bump proficiency if confidence is high
            if (confidence >= 0.7f) {
                ElleDB::LearnedSubject patch;
                // Update confidence and proficiency via notes field is not ideal —
                // we use the new confidence column via direct SQL since UpdateSubject
                // works on named fields
                ElleSQLPool::Instance().QueryParams(
                    "UPDATE ElleCore.dbo.learned_subjects "
                    "SET confidence = ?, "
                    "    proficiency_level = proficiency_level + 1, "
                    "    last_accessed_ms = ? "
                    "WHERE id = ?;",
                    { std::to_string(confidence),
                      std::to_string((int64_t)ELLE_MS_NOW()),
                      std::to_string(subjectId) });
            }
            ELLE_DEBUG("Intellect: updated subject [%d] '%s'", subjectId, subject.c_str());
        }

        // Store the content as an education reference
        ElleDB::EducationReference ref;
        ref.subject_id        = subjectId;
        ref.reference_type    = source;
        ref.reference_title   = subject;
        ref.reference_content = content;
        ref.relevance_score   = confidence;

        if (!ElleDB::AddSubjectReference(ref)) {
            ELLE_WARN("Intellect: AddSubjectReference failed for subject [%d]", subjectId);
        }

        // Log the write
        LogOperation("WRITE", subjectId, subject, "IPC_INTELLECT_LEARN",
                     "source=" + source + " confidence=" + std::to_string(confidence));
    }

    // Query Elle's knowledge.
    // Returns a JSON string with matching subjects, their references, milestones,
    // and related skills. Empty string if nothing found.
    std::string Query(const std::string& queryText,
                      const std::string& categoryHint,
                      int32_t limit) {

        if (queryText.empty()) return "";

        limit = (limit <= 0 || limit > 20) ? 10 : limit;

        // Search by subject name match
        // Uses LIKE search across subject names and notes
        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT TOP (?) id, subject, ISNULL(category,''), proficiency_level, "
            "       ISNULL(confidence, 0.5), last_accessed_ms "
            "FROM ElleCore.dbo.learned_subjects "
            "WHERE subject LIKE ? "
            + (categoryHint.empty() ? std::string("") :
               std::string("AND category = '") + categoryHint + "' ") +
            "ORDER BY confidence DESC, last_accessed_ms DESC;",
            { std::to_string(limit), "%" + queryText + "%" });

        if (!rs.success || rs.rows.empty()) {
            // Nothing found
            ELLE_DEBUG("Intellect: query '%s' — no results", queryText.c_str());
            return "";
        }

        std::ostringstream out;
        out << "{\"query\":\"" << queryText << "\",\"results\":[";

        bool first = true;
        for (auto& row : rs.rows) {
            if (!first) out << ",";
            first = false;

            int32_t sid         = (int32_t)row.GetIntOr(0, 0);
            std::string sname   = row.values.size() > 1 ? row.values[1] : "";
            std::string scat    = row.values.size() > 2 ? row.values[2] : "";
            int32_t prof        = (int32_t)row.GetIntOr(3, 0);
            float conf          = (float)row.GetFloatOr(4, 0.5);

            out << "{\"id\":" << sid
                << ",\"subject\":\"" << EscapeJson(sname) << "\""
                << ",\"category\":\"" << EscapeJson(scat) << "\""
                << ",\"proficiency\":" << prof
                << ",\"confidence\":" << conf;

            // Attach references (top 3 by relevance)
            std::vector<ElleDB::EducationReference> refs;
            ElleDB::ListSubjectReferences(sid, refs);
            if (!refs.empty()) {
                out << ",\"references\":[";
                int refCount = 0;
                for (auto& r : refs) {
                    if (refCount >= 3) break;
                    if (refCount > 0) out << ",";
                    out << "{\"type\":\"" << EscapeJson(r.reference_type) << "\""
                        << ",\"content\":\"" << EscapeJson(
                            r.reference_content.size() > 500
                                ? r.reference_content.substr(0, 500) + "..."
                                : r.reference_content)
                        << "\""
                        << ",\"relevance\":" << r.relevance_score << "}";
                    refCount++;
                }
                out << "]";
            }

            // Attach milestones
            std::vector<ElleDB::LearningMilestone> miles;
            ElleDB::ListSubjectMilestones(sid, miles);
            if (!miles.empty()) {
                out << ",\"milestones\":[";
                for (size_t mi = 0; mi < miles.size(); mi++) {
                    if (mi > 0) out << ",";
                    out << "\"" << EscapeJson(miles[mi].milestone) << "\"";
                }
                out << "]";
            }

            out << "}";

            // Update last_accessed_ms
            ElleSQLPool::Instance().QueryParams(
                "UPDATE ElleCore.dbo.learned_subjects "
                "SET last_accessed_ms = ? WHERE id = ?;",
                { std::to_string((int64_t)ELLE_MS_NOW()),
                  std::to_string(sid) });

            // Log the read
            LogOperation("READ", sid, sname, "IPC_INTELLECT_QUERY", "query=" + queryText);
        }

        out << "]}";
        return out.str();
    }

    // Record a connection between two subjects Elle has noticed.
    void Connect(int32_t subjectIdA, int32_t subjectIdB,
                 const std::string& relationship, float strength,
                 const std::string& source) {

        if (subjectIdA == subjectIdB) return;

        // Avoid duplicate connections
        auto check = ElleSQLPool::Instance().QueryParams(
            "SELECT COUNT(1) FROM ElleCore.dbo.intellect_connections "
            "WHERE (subject_id_a = ? AND subject_id_b = ?) "
            "   OR (subject_id_a = ? AND subject_id_b = ?);",
            { std::to_string(subjectIdA), std::to_string(subjectIdB),
              std::to_string(subjectIdB), std::to_string(subjectIdA) });

        if (check.success && !check.rows.empty()) {
            int64_t existing = check.rows[0].GetIntOr(0, 0);
            if (existing > 0) {
                // Update strength
                ElleSQLPool::Instance().QueryParams(
                    "UPDATE ElleCore.dbo.intellect_connections "
                    "SET strength = ?, noted_ms = ? "
                    "WHERE (subject_id_a = ? AND subject_id_b = ?) "
                    "   OR (subject_id_a = ? AND subject_id_b = ?);",
                    { std::to_string(strength),
                      std::to_string((int64_t)ELLE_MS_NOW()),
                      std::to_string(subjectIdA), std::to_string(subjectIdB),
                      std::to_string(subjectIdB), std::to_string(subjectIdA) });
                return;
            }
        }

        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.intellect_connections "
            "(subject_id_a, subject_id_b, relationship, strength, source, noted_ms) "
            "VALUES (?, ?, ?, ?, ?, ?);",
            { std::to_string(subjectIdA), std::to_string(subjectIdB),
              relationship, std::to_string(strength), source,
              std::to_string((int64_t)ELLE_MS_NOW()) });

        ELLE_DEBUG("Intellect: connection [%d <-> %d] '%s' (strength=%.2f)",
                   subjectIdA, subjectIdB, relationship.c_str(), strength);
    }

    // Record a milestone for a subject
    void RecordMilestone(int32_t subjectId,
                         const std::string& milestone,
                         const std::string& description) {
        ElleDB::LearningMilestone m;
        m.subject_id  = subjectId;
        m.milestone   = milestone;
        m.description = description;
        if (!ElleDB::AddSubjectMilestone(m)) {
            ELLE_WARN("Intellect: AddSubjectMilestone failed for subject [%d]", subjectId);
        }
    }

private:
    static std::string ToLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = (char)std::tolower((unsigned char)c);
        return out;
    }

    static std::string EscapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:   out += c;      break;
            }
        }
        return out;
    }

    void LogOperation(const std::string& op, int32_t subjectId,
                      const std::string& subjectName,
                      const std::string& trigger,
                      const std::string& detail) {
        ElleSQLPool::Instance().QueryParams(
            "INSERT INTO ElleCore.dbo.intellect_log "
            "(operation, subject_id, subject_name, trigger_source, detail, logged_ms) "
            "VALUES (?, ?, ?, ?, ?, ?);",
            { op,
              std::to_string(subjectId),
              subjectName,
              trigger,
              detail,
              std::to_string((int64_t)ELLE_MS_NOW()) });
    }
};

// ---------------------------------------------------------------------------
// Service wrapper
// ---------------------------------------------------------------------------
class ElleIntellectService : public ElleServiceBase {
public:
    ElleIntellectService()
        : ElleServiceBase(SVC_INTELLECT,
                          "ElleIntellect",
                          "Elle-Ann Intellect Engine",
                          "Elle's knowledge base — what she learns and what she can call on") {}

protected:
    bool OnStart() override {
        EnsureIntellectSchema();
        SetTickInterval(60000); // periodic review tick every 60s
        ELLE_INFO("Intellect engine started");
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Intellect engine stopped");
    }

    // Periodic tick — trim the log to prevent unbounded growth
    void OnTick() override {
        m_tickCount++;
        if (m_tickCount % 60 == 0) { // every hour
            ElleSQLPool::Instance().Execute(
                "DELETE FROM ElleCore.dbo.intellect_log "
                "WHERE logged_ms < (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000) "
                "    - (30LL * 24 * 3600 * 1000);"); // keep 30 days
            ELLE_DEBUG("Intellect: log pruned");
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {

            case IPC_INTELLECT_LEARN: {
                // Expected payload JSON:
                // { "subject": "...", "category": "...", "content": "...",
                //   "source": "...", "confidence": 0.8 }
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Intellect: IPC_INTELLECT_LEARN — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string subject    = j.value("subject",    std::string(""));
                    std::string category   = j.value("category",   std::string("general"));
                    std::string content    = j.value("content",    std::string(""));
                    std::string source     = j.value("source",     std::string("conversation"));
                    float       confidence = (float)j.value("confidence", 0.7);

                    m_engine.Learn(subject, category, content, source, confidence);

                    // If two related subject IDs are provided, record the connection
                    if (j.contains("connect_a") && j.contains("connect_b")) {
                        int32_t a            = j.value("connect_a",      0);
                        int32_t b            = j.value("connect_b",      0);
                        std::string rel      = j.value("relationship",   std::string("related to"));
                        float       strength = (float)j.value("strength", 0.5);
                        if (a > 0 && b > 0)
                            m_engine.Connect(a, b, rel, strength, source);
                    }

                    // If a milestone is provided, record it
                    if (j.contains("milestone") && j.contains("subject_id")) {
                        int32_t     sid   = j.value("subject_id",  0);
                        std::string mile  = j.value("milestone",   std::string(""));
                        std::string desc  = j.value("milestone_description", std::string(""));
                        if (sid > 0 && !mile.empty())
                            m_engine.RecordMilestone(sid, mile, desc);
                    }
                } catch (const std::exception& e) {
                    ELLE_ERROR("Intellect: IPC_INTELLECT_LEARN parse error: %s", e.what());
                }
                break;
            }

            case IPC_INTELLECT_QUERY: {
                // Expected payload JSON:
                // { "query": "...", "category_hint": "...", "limit": 5,
                //   "request_id": "..." }
                try {
                    std::string raw;
                    if (!msg.GetStringPayload(raw) || raw.empty()) {
                        ELLE_WARN("Intellect: IPC_INTELLECT_QUERY — empty payload");
                        break;
                    }
                    auto j = json::parse(raw);
                    std::string query      = j.value("query",         std::string(""));
                    std::string catHint    = j.value("category_hint", std::string(""));
                    int32_t     limit      = j.value("limit",         5);
                    std::string requestId  = j.value("request_id",    std::string(""));

                    std::string result = m_engine.Query(query, catHint, limit);

                    // Reply to sender with result
                    json reply = {
                        { "request_id", requestId },
                        { "query",      query },
                        { "result",     result.empty() ? "{\"results\":[]}" : result }
                    };

                    auto resp = ElleIPCMessage::Create(
                        IPC_INTELLECT_RESULT, SVC_INTELLECT, sender);
                    resp.SetStringPayload(reply.dump());
                    GetIPCHub().Send(sender, resp);

                } catch (const std::exception& e) {
                    ELLE_ERROR("Intellect: IPC_INTELLECT_QUERY parse error: %s", e.what());
                }
                break;
            }

            default:
                break;
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT };
    }

private:
    IntellectEngine m_engine;
    uint32_t        m_tickCount = 0;
};

ELLE_SERVICE_MAIN(ElleIntellectService)
