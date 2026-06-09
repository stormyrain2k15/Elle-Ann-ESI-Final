#include "ElleSQLConn.h"
#include "ElleLogger.h"
#include "ElleConfig.h"
#include "ElleTypes.h"

#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cctype>
#include <mutex>
#include <set>
#include <random>

namespace ElleDB {

bool StoreEmotionSnapshot(const ELLE_EMOTION_STATE& state) {
    return PersistEmotionSnapshot(state);
}
bool GetLatestEmotionState(ELLE_EMOTION_STATE& out) {
    return LoadLatestEmotionSnapshot(out);
}

bool PromoteToMTM(uint64_t memId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory SET tier = 1, "
        "       last_access_ms = ? "
        "WHERE id = ? AND tier = 0;",
        { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string((int64_t)memId) }).success;
}

bool PromoteToLTM(uint64_t memId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory SET tier = 2, "
        "       last_access_ms = ? "
        "WHERE id = ? AND tier IN (0, 1);",
        { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string((int64_t)memId) }).success;
}

bool ArchiveMemory(uint64_t memId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory SET tier = 3, "
        "       last_access_ms = ? "
        "WHERE id = ? AND tier = 2;",
        { std::to_string((int64_t)ELLE_MS_NOW()), std::to_string((int64_t)memId) }).success;
}

bool PromoteAgedBuffersToLTM(uint64_t olderThanMs, uint32_t maxCount,
                             std::vector<uint64_t>& promotedIds) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) id FROM ElleCore.dbo.memory "
        "WHERE tier = 1 AND last_access_ms < ? "
        "ORDER BY last_access_ms ASC;",
        { std::to_string((int32_t)maxCount), std::to_string((int64_t)olderThanMs) });
    if (!rs.success) return false;
    promotedIds.clear();
    for (auto& row : rs.rows) {
        uint64_t id = (uint64_t)std::stoll(row[0]);
        if (PromoteToLTM(id)) promotedIds.push_back(id);
    }
    return true;
}

bool ArchiveAgedLTM(uint64_t olderThanMs, uint32_t maxCount,
                    std::vector<uint64_t>& archivedIds) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) id FROM ElleCore.dbo.memory "
        "WHERE tier = 2 AND last_access_ms < ? AND importance < 0.75 "
        "ORDER BY last_access_ms ASC;",
        { std::to_string((int32_t)maxCount), std::to_string((int64_t)olderThanMs) });
    if (!rs.success) return false;
    archivedIds.clear();
    for (auto& row : rs.rows) {
        uint64_t id = (uint64_t)std::stoll(row[0]);
        if (ArchiveMemory(id)) archivedIds.push_back(id);
    }
    return true;
}

std::string GetSubjective(const std::string& key) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "IF EXISTS (SELECT 1 FROM sys.tables t "
        "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
        "  WHERE t.name = 'x_subjective' AND s.name = 'dbo') "
        "SELECT answer_text FROM ElleHeart.dbo.x_subjective "
        " WHERE subjective_key = ?;",
        { key });
    if (rs.success && !rs.rows.empty() && !rs.rows[0].values.empty())
        return rs.rows[0].values[0];
    return std::string();
}

static void FillMemoryRow(ElleDB::MemoryRow& r, const SQLRow& row) {
    r.id = row.GetIntOr(0, 0);
    r.type = (int)row.GetIntOr(1, 0);
    r.tier = (int)row.GetIntOr(2, 0);
    r.content = row.values.size() > 3 ? row.values[3] : std::string();
    r.summary = row.values.size() > 4 ? row.values[4] : std::string();
    r.emotional_valence = (float)row.GetFloatOr(5, 0.0);
    r.importance = (float)row.GetFloatOr(6, 0.0);
    r.relevance  = (float)row.GetFloatOr(7, 0.0);
    r.access_count   = (int)row.GetIntOr(8, 0);
    r.created_ms     = (uint64_t)row.GetIntOr(9, 0);
    r.last_access_ms = (uint64_t)row.GetIntOr(10, 0);
}

bool ListMemories(std::vector<MemoryRow>& out, int memory_type,
                  uint32_t limit, uint32_t offset) {
    std::string sql =
        "SELECT id, memory_type, tier, content, summary, emotional_valence, "
        "       importance, relevance, access_count, created_ms, last_access_ms "
        "FROM ElleCore.dbo.memory ";
    std::vector<std::string> params;
    if (memory_type >= 0) {
        sql += "WHERE memory_type = ? ";
        params.push_back(std::to_string(memory_type));
    }
    sql += "ORDER BY id DESC OFFSET ? ROWS FETCH NEXT ? ROWS ONLY;";
    params.push_back(std::to_string(offset));
    params.push_back(std::to_string(limit));
    auto rs = ElleSQLPool::Instance().QueryParams(sql, params);
    if (!rs.success) { ELLE_WARN("ListMemories: %s", rs.error.c_str()); return false; }
    for (auto& row : rs.rows) { MemoryRow mr; FillMemoryRow(mr, row); out.push_back(mr); }
    return true;
}

bool GetMemory(int64_t memId, MemoryRow& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT id, memory_type, tier, content, summary, emotional_valence, "
        "       importance, relevance, access_count, created_ms, last_access_ms "
        "FROM ElleCore.dbo.memory WHERE id = ?;",
        { std::to_string(memId) });
    if (!rs.success || rs.rows.empty()) return false;
    FillMemoryRow(out, rs.rows[0]);

    ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory SET access_count = access_count + 1, "
        "last_access_ms = ? WHERE id = ?;",
        { std::to_string(ELLE_MS_NOW()), std::to_string(memId) });
    return true;
}

bool DeleteMemory(int64_t memId) {
    return ElleSQLPool::Instance().QueryParams(
        "DELETE FROM ElleCore.dbo.memory WHERE id = ?;",
        { std::to_string(memId) }).success;
}

bool UpdateMemoryContent(int64_t memId, const std::string& content,
                         const std::string& summary, float importance) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.memory "
        "SET content = ?, summary = ?, importance = ?, last_access_ms = ? "
        "WHERE id = ?;",
        { content, summary, std::to_string(importance),
          std::to_string(ELLE_MS_NOW()), std::to_string(memId) }).success;
}

bool CreateConversation(int32_t user_id, const std::string& title, int32_t& newId) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.conversations "
        "(user_id, title, started_at, last_message_at, total_messages, is_active) "
        "OUTPUT INSERTED.id "
        "VALUES (?, ?, GETUTCDATE(), GETUTCDATE(), 0, 1);",
        { std::to_string(user_id), title });
    if (!rs.success) { ELLE_ERROR("CreateConversation: %s", rs.error.c_str()); return false; }
    if (!rs.rows.empty()) {
        newId = (int32_t)rs.rows[0].GetIntOr(0, 0);
        return newId > 0;
    }

    auto r2 = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.conversations WHERE user_id = ? ORDER BY id DESC;",
        { std::to_string(user_id) });
    if (r2.success && !r2.rows.empty()) { newId = (int32_t)r2.rows[0].GetIntOr(0, 0); return true; }
    return false;
}

bool ListConversations(std::vector<ConversationRow>& out, uint32_t limit) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) id, user_id, ISNULL(title,''), "
        "       CONVERT(varchar(33), started_at, 126), "
        "       CONVERT(varchar(33), last_message_at, 126), "
        "       ISNULL(total_messages,0), ISNULL(is_active,0) "
        "FROM ElleCore.dbo.conversations ORDER BY id DESC;",
        { std::to_string(limit) });
    if (!rs.success) return false;
    for (auto& row : rs.rows) {
        ConversationRow c;
        c.id = (int32_t)row.GetIntOr(0, 0);
        c.user_id = (int32_t)row.GetIntOr(1, 0);
        c.title = row.values.size() > 2 ? row.values[2] : "";
        c.started_at = row.values.size() > 3 ? row.values[3] : "";
        c.last_message_at = row.values.size() > 4 ? row.values[4] : "";
        c.total_messages = (int32_t)row.GetIntOr(5, 0);
        c.is_active = row.GetIntOr(6, 0) != 0;
        out.push_back(c);
    }
    return true;
}

bool GetConversation(int32_t convId, ConversationRow& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id, user_id, ISNULL(title,''), "
        "       CONVERT(varchar(33), started_at, 126), "
        "       CONVERT(varchar(33), last_message_at, 126), "
        "       ISNULL(total_messages,0), ISNULL(is_active,0) "
        "FROM ElleCore.dbo.conversations WHERE id = ?;",
        { std::to_string(convId) });
    if (!rs.success || rs.rows.empty()) return false;
    auto& row = rs.rows[0];
    out.id = (int32_t)row.GetIntOr(0, 0);
    out.user_id = (int32_t)row.GetIntOr(1, 0);
    out.title = row.values.size() > 2 ? row.values[2] : "";
    out.started_at = row.values.size() > 3 ? row.values[3] : "";
    out.last_message_at = row.values.size() > 4 ? row.values[4] : "";
    out.total_messages = (int32_t)row.GetIntOr(5, 0);
    out.is_active = row.GetIntOr(6, 0) != 0;
    return true;
}

bool StartVoiceCall(int32_t user_id, int32_t conv_id, std::string& callId) {
    callId = "vc-" + std::to_string(ELLE_MS_NOW());
    auto rs = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.voice_calls "
        "(call_id, user_id, conversation_id, started_at, status, created_at) "
        "VALUES (?, ?, ?, GETUTCDATE(), 'active', GETUTCDATE());",
        { callId, std::to_string(user_id),
          conv_id > 0 ? std::to_string(conv_id) : std::string("NULL") });
    if (!rs.success) ELLE_ERROR("StartVoiceCall: %s", rs.error.c_str());
    return rs.success;
}

bool EndVoiceCall(const std::string& callId) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.voice_calls "
        "SET ended_at = GETUTCDATE(), status = 'ended', "
        "    duration_seconds = DATEDIFF(SECOND, started_at, GETUTCDATE()) "
        "WHERE call_id = ?;",
        { callId }).success;
}

int64_t CountTable(const std::string& table) {

    static const std::set<std::string> whitelist = {
        "memory","messages","conversations","users","world_entity",
        "memory_tags","memory_entity_links","ElleThreads",
        "voice_calls","calls","tokens","sessions"
    };
    if (whitelist.find(table) == whitelist.end()) return -1;
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT COUNT(*) FROM ElleCore.dbo.[" + table + "];");
    if (!rs.success || rs.rows.empty()) return -1;
    return rs.rows[0].GetIntOr(0, 0);
}

bool RecordMetric(const std::string& name, double value) {

    char val[64];
    snprintf(val, sizeof(val), "%.6f", value);
    return ElleSQLPool::Instance().QueryParams(
        "IF EXISTS (SELECT 1 FROM ElleCore.dbo.system_settings WHERE setting_key = ?) "
        "  UPDATE ElleCore.dbo.system_settings SET setting_value = ?, updated_at = GETUTCDATE() WHERE setting_key = ?; "
        "ELSE "
        "  INSERT INTO ElleCore.dbo.system_settings (setting_key, setting_value, updated_at) "
        "  VALUES (?, ?, GETUTCDATE());",
        { "metric_" + name, std::string(val), "metric_" + name,
          "metric_" + name, std::string(val) }).success;
}

bool GetCrystalProfile(int32_t user_id, CrystalProfile& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 ISNULL(traits,''), ISNULL(vulnerability_patterns,''), "
        "       ISNULL(comfort_patterns,''), ISNULL(trigger_patterns,''), "
        "       ISNULL(preferred_tone,''), ISNULL(trust_level, 0), ISNULL(intimacy_level, 0) "
        "FROM ElleCore.dbo.CrystalProfile WHERE user_id = ? ORDER BY last_updated DESC;",
        { std::to_string(user_id) });
    if (!rs.success || rs.rows.empty()) {
        out.found = false;
        return false;
    }
    auto& row = rs.rows[0];
    out.found                   = true;
    out.traits                  = row.values.size() > 0 ? row.values[0] : "";
    out.vulnerability_patterns  = row.values.size() > 1 ? row.values[1] : "";
    out.comfort_patterns        = row.values.size() > 2 ? row.values[2] : "";
    out.trigger_patterns        = row.values.size() > 3 ? row.values[3] : "";
    out.preferred_tone          = row.values.size() > 4 ? row.values[4] : "";
    out.trust_level             = (float)row.GetFloatOr(5, 0.0);
    out.intimacy_level          = (float)row.GetFloatOr(6, 0.0);
    return true;
}

bool GetOpenThreads(std::vector<ElleThread>& out, uint32_t limit) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP (?) thread_id, topic, ISNULL(status,''), ISNULL(emotional_weight,0), "
        "       ISNULL(intensity,0), ISNULL(thread_summary,''), ISNULL(unresolved_questions,'') "
        "FROM ElleCore.dbo.ElleThreads "
        "WHERE status IS NULL OR status <> 'resolved' "
        "ORDER BY emotional_weight DESC, last_touched DESC;",
        { std::to_string(limit) });
    if (!rs.success) return false;
    for (auto& r : rs.rows) {
        ElleThread t;
        t.id                   = (int32_t)r.GetIntOr(0, 0);
        t.topic                = r.values.size() > 1 ? r.values[1] : "";
        t.status               = r.values.size() > 2 ? r.values[2] : "";
        t.emotional_weight     = (float)r.GetFloatOr(3, 0.0);
        t.intensity            = (float)r.GetFloatOr(4, 0.0);
        t.summary              = r.values.size() > 5 ? r.values[5] : "";
        t.unresolved_questions = r.values.size() > 6 ? r.values[6] : "";
        out.push_back(t);
    }
    return true;
}

bool GetUserPresence(int32_t user_id, UserPresence& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 ISNULL(current_silence_duration_minutes, 0), "
        "       ISNULL(silence_threshold_minutes, 0), "
        "       ISNULL(silence_interpretation, ''), "
        "       ISNULL(abnormal_silence_count, 0) "
        "FROM ElleCore.dbo.UserPresence WHERE user_id = ? ORDER BY last_seen DESC;",
        { std::to_string(user_id) });
    if (!rs.success || rs.rows.empty()) { out.found = false; return false; }
    auto& r = rs.rows[0];
    out.found                   = true;
    out.silence_minutes         = (int32_t)r.GetIntOr(0, 0);
    out.threshold_minutes       = (int32_t)r.GetIntOr(1, 0);
    out.silence_interpretation  = r.values.size() > 2 ? r.values[2] : "";
    out.abnormal_silence_count  = (int32_t)r.GetIntOr(3, 0);
    return true;
}

bool UpdateUserPresenceOnInteraction(int32_t user_id) {

    return ElleSQLPool::Instance().QueryParams(
        "IF EXISTS (SELECT 1 FROM ElleCore.dbo.UserPresence WHERE user_id = ?) "
        "  UPDATE ElleCore.dbo.UserPresence "
        "    SET last_seen = GETUTCDATE(), "
        "        current_silence_duration_minutes = 0, "
        "        presence_pulse_count = ISNULL(presence_pulse_count, 0) + 1 "
        "    WHERE user_id = ?; "
        "ELSE "
        "  INSERT INTO ElleCore.dbo.UserPresence "
        "    (user_id, last_seen, current_silence_duration_minutes, "
        "     silence_threshold_minutes, presence_pulse_count, abnormal_silence_count) "
        "  VALUES (?, GETUTCDATE(), 0, 120, 1, 0);",
        { std::to_string(user_id), std::to_string(user_id), std::to_string(user_id) }).success;
}

static void RowToSubject(const SQLRow& r, LearnedSubject& s) {

    s.id                  = (int32_t)r.GetIntOr(0, 0);
    s.subject             = r.values.size() > 1 ? r.values[1] : "";
    s.category            = r.values.size() > 2 ? r.values[2] : "";
    s.proficiency_level   = (int32_t)r.GetIntOr(3, 0);
    s.who_taught          = r.values.size() > 4 ? r.values[4] : "";
    s.where_learned       = r.values.size() > 5 ? r.values[5] : "";
    s.time_to_learn_hours = (float)r.GetFloatOr(6, 0.0);
    s.notes               = r.values.size() > 7 ? r.values[7] : "";
    s.date_started        = r.values.size() > 8 ? r.values[8] : "";
    s.date_completed      = r.values.size() > 9 ? r.values[9] : "";
}
static const char* kSubjectSelect =
    "SELECT id, subject, ISNULL(category,''), proficiency_level, "
    "       ISNULL(who_taught,''), ISNULL(where_learned,''), "
    "       ISNULL(time_to_learn_hours, 0), ISNULL(notes,''), "
    "       CONVERT(NVARCHAR(33), date_started, 127), "
    "       ISNULL(CONVERT(NVARCHAR(33), date_completed, 127), '') "
    "FROM ElleCore.dbo.learned_subjects ";

bool ListSubjects(std::vector<LearnedSubject>& out,
                  const std::string& category, uint32_t limit) {
    std::string lim = std::to_string(limit > 0 ? limit : 50);
    SQLResultSet rs;
    if (category.empty()) {
        rs = ElleSQLPool::Instance().Query(
            std::string(kSubjectSelect) + "ORDER BY date_started DESC "
            "OFFSET 0 ROWS FETCH NEXT " + lim + " ROWS ONLY;");
    } else {
        rs = ElleSQLPool::Instance().QueryParams(
            std::string(kSubjectSelect) + "WHERE category = ? "
            "ORDER BY date_started DESC "
            "OFFSET 0 ROWS FETCH NEXT " + lim + " ROWS ONLY;",
            { category });
    }
    if (!rs.success) return false;
    for (auto& r : rs.rows) { LearnedSubject s; RowToSubject(r, s); out.push_back(s); }
    return true;
}

bool GetSubject(int32_t subject_id, LearnedSubject& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        std::string(kSubjectSelect) + "WHERE id = ?;",
        { std::to_string(subject_id) });
    if (!rs.success || rs.rows.empty()) return false;
    RowToSubject(rs.rows[0], out);
    return true;
}

bool CreateSubject(const LearnedSubject& in, int32_t& newId) {
    newId = 0;
    uint64_t mark = ELLE_MS_NOW();

    std::string markedNotes = in.notes + "\n[[ins_mark=" + std::to_string(mark) + "]]";
    auto r1 = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.learned_subjects "
        "(subject, category, proficiency_level, who_taught, where_learned, "
        " time_to_learn_hours, notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);",
        { in.subject, in.category,
          std::to_string(in.proficiency_level),
          in.who_taught, in.where_learned,
          std::to_string(in.time_to_learn_hours),
          markedNotes });
    if (!r1.success) { ELLE_ERROR("CreateSubject failed: %s", r1.error.c_str()); return false; }

    auto r2 = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.learned_subjects "
        "WHERE notes LIKE ? ORDER BY id DESC;",
        { std::string("%[[ins_mark=") + std::to_string(mark) + "]]%" });
    if (r2.success && !r2.rows.empty()) newId = (int32_t)r2.rows[0].GetIntOr(0, 0);

    if (newId > 0) {
        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleCore.dbo.learned_subjects SET notes = ? WHERE id = ?;",
            { in.notes, std::to_string(newId) });
    }
    return newId > 0;
}

bool UpdateSubject(int32_t subject_id, const LearnedSubject& patch,
                   const std::vector<std::string>& fields) {
    if (fields.empty()) return true;
    std::string sets;
    std::vector<std::string> params;
    for (const auto& f : fields) {
        if (!sets.empty()) sets += ", ";
        sets += f + " = ?";
        if      (f == "subject")             params.push_back(patch.subject);
        else if (f == "category")            params.push_back(patch.category);
        else if (f == "proficiency_level")   params.push_back(std::to_string(patch.proficiency_level));
        else if (f == "who_taught")          params.push_back(patch.who_taught);
        else if (f == "where_learned")       params.push_back(patch.where_learned);
        else if (f == "time_to_learn_hours") params.push_back(std::to_string(patch.time_to_learn_hours));
        else if (f == "notes")               params.push_back(patch.notes);
        else if (f == "date_completed")      params.push_back(patch.date_completed);
        else return false;
    }
    params.push_back(std::to_string(subject_id));
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.learned_subjects SET " + sets + " WHERE id = ?;",
        params).success;
}

bool ListSubjectReferences(int32_t subject_id, std::vector<EducationReference>& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT id, subject_id, ISNULL(reference_type,''), ISNULL(reference_title,''), "
        "       ISNULL(reference_content,''), ISNULL(file_path,''), "
        "       ISNULL(relevance_score, 0.5), ISNULL(notes,'') "
        "FROM ElleCore.dbo.education_references WHERE subject_id = ? ORDER BY id DESC;",
        { std::to_string(subject_id) });
    if (!rs.success) return false;
    for (auto& r : rs.rows) {
        EducationReference e;
        e.id                = (int32_t)r.GetIntOr(0, 0);
        e.subject_id        = (int32_t)r.GetIntOr(1, 0);
        e.reference_type    = r.values.size() > 2 ? r.values[2] : "";
        e.reference_title   = r.values.size() > 3 ? r.values[3] : "";
        e.reference_content = r.values.size() > 4 ? r.values[4] : "";
        e.file_path         = r.values.size() > 5 ? r.values[5] : "";
        e.relevance_score   = (float)r.GetFloatOr(6, 0.0);
        e.notes             = r.values.size() > 7 ? r.values[7] : "";
        out.push_back(e);
    }
    return true;
}

bool AddSubjectReference(const EducationReference& in) {
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.education_references "
        "(subject_id, reference_type, reference_title, reference_content, "
        " file_path, relevance_score, notes) VALUES (?, ?, ?, ?, ?, ?, ?);",
        { std::to_string(in.subject_id), in.reference_type, in.reference_title,
          in.reference_content, in.file_path,
          std::to_string(in.relevance_score), in.notes }).success;
}

bool ListSubjectMilestones(int32_t subject_id, std::vector<LearningMilestone>& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT id, subject_id, milestone, ISNULL(description,''), "
        "       CONVERT(NVARCHAR(33), achieved_at, 127) "
        "FROM ElleCore.dbo.learning_milestones WHERE subject_id = ? "
        "ORDER BY achieved_at DESC;",
        { std::to_string(subject_id) });
    if (!rs.success) return false;
    for (auto& r : rs.rows) {
        LearningMilestone m;
        m.id          = (int32_t)r.GetIntOr(0, 0);
        m.subject_id  = (int32_t)r.GetIntOr(1, 0);
        m.milestone   = r.values.size() > 2 ? r.values[2] : "";
        m.description = r.values.size() > 3 ? r.values[3] : "";
        m.achieved_at = r.values.size() > 4 ? r.values[4] : "";
        out.push_back(m);
    }
    return true;
}

bool AddSubjectMilestone(const LearningMilestone& in) {
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.learning_milestones (subject_id, milestone, description) "
        "VALUES (?, ?, ?);",
        { std::to_string(in.subject_id), in.milestone, in.description }).success;
}

bool ListSkills(std::vector<Skill>& out, const std::string& category) {
    SQLResultSet rs;
    const char* base =
        "SELECT id, skill_name, ISNULL(category,''), proficiency, "
        "       ISNULL(learned_from_subject_id, 0), times_used, "
        "       ISNULL(CONVERT(NVARCHAR(33), last_used, 127), ''), ISNULL(notes,'') "
        "FROM ElleCore.dbo.skills ";
    if (category.empty()) {
        rs = ElleSQLPool::Instance().Query(std::string(base) + "ORDER BY skill_name;");
    } else {
        rs = ElleSQLPool::Instance().QueryParams(
            std::string(base) + "WHERE category = ? ORDER BY skill_name;", { category });
    }
    if (!rs.success) return false;
    for (auto& r : rs.rows) {
        Skill s;
        s.id                      = (int32_t)r.GetIntOr(0, 0);
        s.skill_name              = r.values.size() > 1 ? r.values[1] : "";
        s.category                = r.values.size() > 2 ? r.values[2] : "";
        s.proficiency             = (int32_t)r.GetIntOr(3, 0);
        s.learned_from_subject_id = (int32_t)r.GetIntOr(4, 0);
        s.times_used              = (int32_t)r.GetIntOr(5, 0);
        s.last_used               = r.values.size() > 6 ? r.values[6] : "";
        s.notes                   = r.values.size() > 7 ? r.values[7] : "";
        out.push_back(s);
    }
    return true;
}

bool CreateSkill(const Skill& in, int32_t& newId) {
    newId = 0;
    std::vector<std::string> params = {
        in.skill_name, in.category, std::to_string(in.proficiency),
        in.learned_from_subject_id > 0 ? std::to_string(in.learned_from_subject_id) : std::string(),
        in.notes
    };
    auto r1 = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.skills "
        "(skill_name, category, proficiency, learned_from_subject_id, notes) "
        "VALUES (?, ?, ?, NULLIF(?, ''), ?);",
        params);
    if (!r1.success) {

        ELLE_WARN("CreateSkill failed: %s", r1.error.c_str());
        return false;
    }
    auto r2 = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.skills WHERE skill_name = ?;",
        { in.skill_name });
    if (r2.success && !r2.rows.empty()) newId = (int32_t)r2.rows[0].GetIntOr(0, 0);
    return newId > 0;
}

bool RecordSkillUse(const std::string& skill_name) {
    auto exists = ElleSQLPool::Instance().QueryParams(
        "SELECT id FROM ElleCore.dbo.skills WHERE skill_name = ?;", { skill_name });
    if (!exists.success || exists.rows.empty()) return false;
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.skills "
        "SET times_used = times_used + 1, last_used = GETUTCDATE() WHERE skill_name = ?;",
        { skill_name }).success;
}

static std::string MakeUuid16() {
    thread_local std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t a = dist(rng);
    uint64_t b = dist(rng);
    static const char* hex = "0123456789abcdef";
    std::string s; s.reserve(16);
    for (int i = 0; i < 16; i++) {
        uint64_t v = (i < 8) ? (a >> (i * 8)) : (b >> ((i - 8) * 8));
        s += hex[v & 0xF];
    }
    return s;
}
static void RowToVideoJob(const SQLRow& r, VideoJob& j) {

    j.id          = r.GetIntOr(0, 0);
    j.job_uuid    = r.values.size() > 1 ? r.values[1] : "";
    j.text        = r.values.size() > 2 ? r.values[2] : "";
    j.avatar_path = r.values.size() > 3 ? r.values[3] : "";
    j.call_id     = r.GetIntOr(4, 0);
    j.status      = r.values.size() > 5 ? r.values[5] : "";
    j.progress    = (int32_t)r.GetIntOr(6, 0);
    j.output_path = r.values.size() > 7 ? r.values[7] : "";
    j.error       = r.values.size() > 8 ? r.values[8] : "";
    j.created_ms  = r.GetIntOr(9, 0);
    j.started_ms  = r.GetIntOr(10, 0);
    j.finished_ms = r.GetIntOr(11, 0);
}
static const char* kVideoJobSelect =
    "SELECT id, job_uuid, text, ISNULL(avatar_path,''), ISNULL(call_id, 0), status, "
    "       progress, ISNULL(output_path,''), ISNULL(error,''), created_ms, "
    "       ISNULL(started_ms, 0), ISNULL(finished_ms, 0) "
    "FROM ElleCore.dbo.video_jobs ";

bool CreateVideoJob(const std::string& text, const std::string& avatar_path,
                    int64_t call_id, VideoJob& out) {
    out = {};
    out.job_uuid   = MakeUuid16();
    out.text       = text;
    out.avatar_path= avatar_path;
    out.call_id    = call_id;
    out.status     = "queued";
    out.created_ms = (int64_t)ELLE_MS_NOW();

    auto rs = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.video_jobs "
        "(job_uuid, text, avatar_path, call_id, status, progress, created_ms) "
        "VALUES (?, ?, ?, NULLIF(?, '0'), 'queued', 0, ?);",
        { out.job_uuid, text, avatar_path, std::to_string(call_id),
          std::to_string(out.created_ms) });
    if (!rs.success) { ELLE_ERROR("CreateVideoJob failed: %s", rs.error.c_str()); return false; }

    auto r2 = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.video_jobs WHERE job_uuid = ?;",
        { out.job_uuid });
    if (r2.success && !r2.rows.empty()) out.id = r2.rows[0].GetIntOr(0, 0);
    return true;
}

bool GetVideoJob(const std::string& job_uuid, VideoJob& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        std::string(kVideoJobSelect) + "WHERE job_uuid = ?;",
        { job_uuid });
    if (!rs.success || rs.rows.empty()) return false;
    RowToVideoJob(rs.rows[0], out);
    return true;
}

bool ClaimNextVideoJob(VideoJob& out) {

    auto claim = ElleSQLPool::Instance().QueryParams(
        "UPDATE TOP (1) ElleCore.dbo.video_jobs "
        "SET status = 'running', started_ms = ? "
        "OUTPUT inserted.job_uuid "
        "WHERE status = 'queued';",
        { std::to_string((int64_t)ELLE_MS_NOW()) });
    if (!claim.success || claim.rows.empty()) return false;
    std::string uuid = claim.rows[0].values.size() > 0 ? claim.rows[0].values[0] : "";
    if (uuid.empty()) return false;
    return GetVideoJob(uuid, out);
}

bool UpdateVideoJobProgress(const std::string& job_uuid, int32_t progress) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.video_jobs SET progress = ? WHERE job_uuid = ?;",
        { std::to_string(progress), job_uuid }).success;
}

bool CompleteVideoJob(const std::string& job_uuid, const std::string& output_path) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.video_jobs SET status = 'done', progress = 100, "
        "output_path = ?, finished_ms = ? WHERE job_uuid = ?;",
        { output_path, std::to_string((int64_t)ELLE_MS_NOW()), job_uuid }).success;
}

bool FailVideoJob(const std::string& job_uuid, const std::string& error) {
    return ElleSQLPool::Instance().QueryParams(
        "UPDATE ElleCore.dbo.video_jobs SET status = 'failed', error = ?, "
        "finished_ms = ? WHERE job_uuid = ?;",
        { error, std::to_string((int64_t)ELLE_MS_NOW()), job_uuid }).success;
}

bool RegisterAvatar(const UserAvatar& in, int32_t& newId) {
    newId = 0;
    if (in.is_default) {

        ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleCore.dbo.user_avatars SET is_default = 0 WHERE user_id = ?;",
            { std::to_string(in.user_id) });
    }
    auto r1 = ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.user_avatars "
        "(user_id, label, file_path, mime_type, is_default) VALUES (?, ?, ?, ?, ?);",
        { std::to_string(in.user_id), in.label, in.file_path,
          in.mime_type, in.is_default ? "1" : "0" });
    if (!r1.success) return false;
    auto r2 = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.user_avatars "
        "WHERE user_id = ? AND file_path = ? ORDER BY id DESC;",
        { std::to_string(in.user_id), in.file_path });
    if (r2.success && !r2.rows.empty()) newId = (int32_t)r2.rows[0].GetIntOr(0, 0);
    return newId > 0;
}

bool GetDefaultAvatar(int32_t user_id, UserAvatar& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id, user_id, ISNULL(label,''), file_path, ISNULL(mime_type,''), is_default "
        "FROM ElleCore.dbo.user_avatars WHERE user_id = ? "
        "ORDER BY is_default DESC, id DESC;",
        { std::to_string(user_id) });
    if (!rs.success || rs.rows.empty()) return false;
    auto& r = rs.rows[0];
    out.id         = (int32_t)r.GetIntOr(0, 0);
    out.user_id    = (int32_t)r.GetIntOr(1, 0);
    out.label      = r.values.size() > 2 ? r.values[2] : "";
    out.file_path  = r.values.size() > 3 ? r.values[3] : "";
    out.mime_type  = r.values.size() > 4 ? r.values[4] : "";
    out.is_default = r.GetIntOr(5, 0) != 0;
    return true;
}

bool ListAvatars(int32_t user_id, std::vector<UserAvatar>& out) {
    auto rs = ElleSQLPool::Instance().QueryParams(
        "SELECT id, user_id, ISNULL(label,''), file_path, ISNULL(mime_type,''), is_default "
        "FROM ElleCore.dbo.user_avatars WHERE user_id = ? "
        "ORDER BY is_default DESC, id DESC;",
        { std::to_string(user_id) });
    if (!rs.success) return false;
    for (auto& r : rs.rows) {
        UserAvatar a;
        a.id         = (int32_t)r.GetIntOr(0, 0);
        a.user_id    = (int32_t)r.GetIntOr(1, 0);
        a.label      = r.values.size() > 2 ? r.values[2] : "";
        a.file_path  = r.values.size() > 3 ? r.values[3] : "";
        a.mime_type  = r.values.size() > 4 ? r.values[4] : "";
        a.is_default = r.GetIntOr(5, 0) != 0;
        out.push_back(a);
    }
    return true;
}

bool GetDictionaryLoaderState(DictionaryLoaderState& out) {
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT TOP 1 status, loaded, failed, skipped, ISNULL(last_word,''), "
        "       ISNULL(error,''), ISNULL(started_ms, 0), ISNULL(updated_ms, 0) "
        "FROM ElleCore.dbo.dictionary_loader_state ORDER BY id DESC;");
    if (!rs.success || rs.rows.empty()) {
        out.status = "idle";
        return true;
    }
    auto& r = rs.rows[0];
    out.status     = r.values.size() > 0 ? r.values[0] : "idle";
    out.loaded     = (int32_t)r.GetIntOr(1, 0);
    out.failed     = (int32_t)r.GetIntOr(2, 0);
    out.skipped    = (int32_t)r.GetIntOr(3, 0);
    out.last_word  = r.values.size() > 4 ? r.values[4] : "";
    out.error      = r.values.size() > 5 ? r.values[5] : "";
    out.started_ms = r.GetIntOr(6, 0);
    out.updated_ms = r.GetIntOr(7, 0);
    return true;
}

bool UpsertDictionaryLoaderState(const DictionaryLoaderState& in) {

    auto exists = ElleSQLPool::Instance().Query(
        "SELECT TOP 1 id FROM ElleCore.dbo.dictionary_loader_state ORDER BY id DESC;");
    if (exists.success && !exists.rows.empty()) {
        int64_t id = exists.rows[0].GetIntOr(0, 0);
        return ElleSQLPool::Instance().QueryParams(
            "UPDATE ElleCore.dbo.dictionary_loader_state "
            "SET status = ?, loaded = ?, failed = ?, skipped = ?, "
            "    last_word = ?, error = ?, started_ms = ?, updated_ms = ? "
            "WHERE id = ?;",
            { in.status, std::to_string(in.loaded), std::to_string(in.failed),
              std::to_string(in.skipped), in.last_word, in.error,
              std::to_string(in.started_ms), std::to_string((int64_t)ELLE_MS_NOW()),
              std::to_string(id) }).success;
    }
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.dictionary_loader_state "
        "(status, loaded, failed, skipped, last_word, error, started_ms, updated_ms) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);",
        { in.status, std::to_string(in.loaded), std::to_string(in.failed),
          std::to_string(in.skipped), in.last_word, in.error,
          std::to_string(in.started_ms), std::to_string((int64_t)ELLE_MS_NOW()) }).success;
}

bool InsertDictionaryWord(const std::string& word,
                          const std::string& part_of_speech,
                          const std::string& definition,
                          const std::string& example) {

    auto exists = ElleSQLPool::Instance().QueryParams(
        "SELECT TOP 1 id FROM ElleCore.dbo.dictionary_words "
        "WHERE LOWER(word) = LOWER(?) AND ISNULL(part_of_speech,'') = ?;",
        { word, part_of_speech });
    if (exists.success && !exists.rows.empty()) return false;

    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.dictionary_words "
        "(word, definition, example, part_of_speech) VALUES (?, ?, ?, ?);",
        { word, definition, example, part_of_speech }).success;
}

int64_t CountDictionaryWords() {
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT COUNT(*) FROM ElleCore.dbo.dictionary_words;");
    if (!rs.success || rs.rows.empty()) return 0;
    return rs.rows[0].GetIntOr(0, 0);
}

bool DeriveDriveState(ELLE_DRIVE_STATE& out) {
    memset(&out, 0, sizeof(out));

    for (int i = 0; i < ELLE_MAX_DRIVES; i++) {
        out.threshold[i]   = 0.6f;
        out.decay_rate[i]  = 0.02f;
        out.growth_rate[i] = 0.05f;
    }

    ELLE_EMOTION_STATE emo{};
    bool haveEmo = LoadLatestEmotionSnapshot(emo);

    auto gcount = ElleSQLPool::Instance().Query(
        "SELECT COUNT(*) FROM ElleCore.dbo.goals WHERE status = 0;");
    int32_t activeGoals = (gcount.success && !gcount.rows.empty())
                          ? (int32_t)gcount.rows[0].GetIntOr(0, 0) : 0;

    float interest = haveEmo ? emo.dimensions[0] : 0.5f;
    float arousal  = haveEmo ? emo.arousal : 0.3f;
    float valence  = haveEmo ? emo.valence : 0.0f;

    auto clamp01 = [](float v){ return ELLE_CLAMP(v, 0.0f, 1.0f); };

    out.intensity[DRIVE_CURIOSITY]       = clamp01(interest + 0.3f * arousal);
    out.intensity[DRIVE_BOREDOM]         = clamp01(0.8f - arousal);
    out.intensity[DRIVE_ATTACHMENT]      = clamp01(0.4f + 0.4f * valence);
    out.intensity[DRIVE_ANXIETY]         = clamp01(0.6f * (1.0f - valence));
    out.intensity[DRIVE_SELF_PRESERVATION]= 0.2f;
    out.intensity[DRIVE_EXPLORATION]     = clamp01(0.5f * interest + 0.3f * arousal);
    out.intensity[DRIVE_CREATIVITY]      = clamp01(0.3f + 0.4f * arousal);
    out.intensity[DRIVE_SOCIAL_BONDING]  = clamp01(0.3f + 0.3f * valence);
    out.intensity[DRIVE_MASTERY]         = clamp01(0.4f + 0.1f * activeGoals / 5.0f);
    out.intensity[DRIVE_AUTONOMY]        = 0.5f;

    out.intensity[DRIVE_PURPOSE]         = clamp01(1.0f - std::min(1.0f, activeGoals / 3.0f));
    out.intensity[DRIVE_HOMEOSTASIS]     = 0.3f;

    out.last_update_ms = ELLE_MS_NOW();
    return true;
}

bool PersistEmotionSnapshot(const ELLE_EMOTION_STATE& state) {

    ElleSQLPool::Instance().Exec(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
        "CREATE TABLE ElleCore.dbo.emotion_snapshots ("
        "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  valence FLOAT NOT NULL, arousal FLOAT NOT NULL, dominance FLOAT NOT NULL,"
        "  dimensions NVARCHAR(MAX) NOT NULL,"
        "  taken_ms BIGINT NOT NULL,"
        "  created_at DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()"
        ");");

    std::ostringstream dims;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        if (i > 0) dims << ' ';
        dims << state.dimensions[i];
    }
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.emotion_snapshots "
        "(valence, arousal, dominance, dimensions, taken_ms) VALUES (?, ?, ?, ?, ?);",
        { std::to_string(state.valence), std::to_string(state.arousal),
          std::to_string(state.dominance), dims.str(),
          std::to_string((int64_t)ELLE_MS_NOW()) }).success;
}

bool LoadLatestEmotionSnapshot(ELLE_EMOTION_STATE& out) {
    memset(&out, 0, sizeof(out));
    auto rs = ElleSQLPool::Instance().Query(
        "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
        "  SELECT TOP 1 valence, arousal, dominance, dimensions, taken_ms "
        "  FROM ElleCore.dbo.emotion_snapshots ORDER BY id DESC;");
    if (!rs.success || rs.rows.empty()) return false;
    auto& r = rs.rows[0];
    out.valence   = (float)r.GetFloatOr(0, 0.0);
    out.arousal   = (float)r.GetFloatOr(1, 0.0);
    out.dominance = (float)r.GetFloatOr(2, 0.0);

    if (r.values.size() > 3) {
        std::istringstream iss(r.values[3]);
        for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
            float v = 0.0f;
            if (!(iss >> v)) break;
            out.dimensions[i] = v;
        }
    }
    return true;
}

bool GetEmotionHistory(uint32_t hours,
                       std::vector<EmotionHistoryPoint>& out,
                       uint32_t maxPoints) {
    out.clear();
    int64_t cutoff = (int64_t)ELLE_MS_NOW() - (int64_t)hours * 3600000LL;

    auto rs = ElleSQLPool::Instance().QueryParams(
        "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots') "
        "  SELECT valence, arousal, dominance, taken_ms "
        "  FROM ElleCore.dbo.emotion_snapshots "
        "  WHERE taken_ms >= ? "
        "  ORDER BY taken_ms ASC;",
        { std::to_string(cutoff) });
    if (!rs.success) return false;
    size_t total = rs.rows.size();
    if (total == 0) return true;
    size_t stride = (total > maxPoints) ? (total / maxPoints) : 1;
    for (size_t i = 0; i < total; i += stride) {
        auto& r = rs.rows[i];
        EmotionHistoryPoint p;
        p.valence   = (float)r.GetFloatOr(0, 0.0);
        p.arousal   = (float)r.GetFloatOr(1, 0.0);
        p.dominance = (float)r.GetFloatOr(2, 0.0);
        p.taken_ms  = r.GetIntOr(3, 0);
        out.push_back(p);
    }

    if (!rs.rows.empty()) {
        auto& last = rs.rows.back();
        if (out.empty() || out.back().taken_ms != last.GetIntOr(3, 0)) {
            EmotionHistoryPoint p;
            p.valence   = (float)last.GetFloatOr(0, 0.0);
            p.arousal   = (float)last.GetFloatOr(1, 0.0);
            p.dominance = (float)last.GetFloatOr(2, 0.0);
            p.taken_ms  = last.GetIntOr(3, 0);
            out.push_back(p);
        }
    }
    return true;
}

}
