#pragma once
#ifndef ELLE_SQL_CONN_H
#define ELLE_SQL_CONN_H

#include "ElleTypes.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <unordered_map>

typedef void* SQLHENV;
typedef void* SQLHDBC;
typedef void* SQLHSTMT;
typedef short SQLRETURN;

struct SQLColumn {
    std::string name;
    int32_t     type;
    uint32_t    size;
};

struct SQLRow {
    std::vector<std::string> values;

    const std::string& operator[](size_t idx) const { return values[idx]; }

    [[nodiscard]] int64_t     GetIntOr(size_t idx, int64_t fallback) const;
    [[nodiscard]] double      GetFloatOr(size_t idx, double fallback) const;

    bool        TryGetInt(size_t idx, int64_t& outVal) const;
    bool        TryGetFloat(size_t idx, double& outVal) const;

    bool        IsNull(size_t idx) const;
};

struct SQLResultSet {
    std::vector<SQLColumn>  columns;
    std::vector<SQLRow>     rows;
    int64_t                 rows_affected = 0;
    bool                    success = false;
    std::string             error;

    bool        Empty() const { return rows.empty(); }
    size_t      RowCount() const { return rows.size(); }
    size_t      ColCount() const { return columns.size(); }
    int         ColIndex(const std::string& name) const;
};

class SQLConnection {
public:
    SQLConnection();
    ~SQLConnection();

    bool Connect(const std::string& connectionString);
    void Disconnect();
    bool IsConnected() const;
    bool Ping();

    SQLResultSet Execute(const std::string& sql);
    SQLResultSet ExecuteParams(const std::string& sql, const std::vector<std::string>& params);
    SQLResultSet CallProc(const std::string& proc, const std::vector<std::string>& params);

    int64_t ExecuteScalar(const std::string& sql);
    bool    ExecuteNonQuery(const std::string& sql);

    bool BeginTransaction();
    bool Commit();
    bool Rollback();

    const std::string& LastError() const { return m_lastError; }
    uint64_t LastUsedMs() const { return m_lastUsed; }

private:
    SQLHENV     m_hEnv = nullptr;
    SQLHDBC     m_hDbc = nullptr;
    bool        m_connected = false;
    bool        m_inTransaction = false;
    std::string m_connStr;
    std::string m_lastError;
    uint64_t    m_lastUsed = 0;

    bool AllocHandles();
    void FreeHandles();
    std::string GetDiagnostics(int16_t handleType, void* handle);

    SQLResultSet CollectStatementResults(SQLHSTMT hStmt, SQLRETURN execRet);
};

class ElleSQLPool {
public:
    static ElleSQLPool& Instance();

    bool Initialize(const std::string& connectionString, uint32_t poolSize = 8);
    void Shutdown();

    bool Reinitialize(const std::string& connectionString, uint32_t poolSize = 8);

    std::shared_ptr<SQLConnection> Acquire(uint32_t timeoutMs = 5000);
    void Release(std::shared_ptr<SQLConnection> conn);

    SQLResultSet Query(const std::string& sql);
    SQLResultSet QueryParams(const std::string& sql, const std::vector<std::string>& params);
    SQLResultSet CallProc(const std::string& proc, const std::vector<std::string>& params);
    bool         Exec(const std::string& sql);
    int64_t      Scalar(const std::string& sql);

    uint32_t AvailableConnections() const;
    uint32_t TotalConnections() const { return m_poolSize; }
    uint64_t TotalQueries() const { return m_totalQueries; }

private:
    ElleSQLPool() = default;
    ~ElleSQLPool() = default;

    std::string     m_connStr;
    uint32_t        m_poolSize = 0;
    bool            m_initialized = false;

    std::queue<std::shared_ptr<SQLConnection>>  m_available;
    std::mutex                                   m_mutex;
    std::condition_variable                      m_cv;
    uint64_t                                     m_totalQueries = 0;

    bool CreateConnection(std::shared_ptr<SQLConnection>& conn);
};

namespace ElleDB {

    bool SubmitIntent(const ELLE_INTENT_RECORD& intent);
    bool GetPendingIntents(std::vector<ELLE_INTENT_RECORD>& out, uint32_t maxCount = 10);
    bool UpdateIntentStatus(uint64_t intentId, ELLE_INTENT_STATUS status, const std::string& response = "");

    uint32_t ReapStaleIntents(uint32_t defaultTimeoutMs = 120000, uint32_t maxRetries = 3);

    bool SubmitAction(const ELLE_ACTION_RECORD& action);
    bool GetPendingActions(std::vector<ELLE_ACTION_RECORD>& out, uint32_t maxCount = 10);
    bool UpdateActionStatus(uint64_t actionId, ELLE_ACTION_STATUS status, const std::string& result = "");

    uint32_t ReapStaleActions(uint32_t defaultTimeoutMs = 60000, uint32_t maxAttempts = 3);

    struct QueueSnapshot {
        uint32_t intent_pending        = 0;
        uint32_t intent_processing     = 0;
        uint32_t intent_completed_1h   = 0;
        uint32_t intent_failed_1h      = 0;
        uint32_t intent_stale_processing = 0;
        uint32_t action_queued         = 0;
        uint32_t action_locked         = 0;
        uint32_t action_executing      = 0;
        uint32_t action_success_1h     = 0;
        uint32_t action_failure_1h     = 0;
        uint32_t action_timeout_1h     = 0;
        uint32_t action_stale_locked   = 0;
        uint32_t hardware_pending      = 0;
        uint32_t hardware_dispatched   = 0;
    };
    bool GetQueueSnapshot(QueueSnapshot& out);

    bool StoreMessage(uint64_t convoId, uint32_t role, const std::string& content,
                      const ELLE_EMOTION_STATE& emotions, float sentiment);
    bool GetConversationHistory(uint64_t convoId, std::vector<ELLE_CONVERSATION_MSG>& out, uint32_t limit = 50);

    bool StoreMemory(const ELLE_MEMORY_RECORD& mem);
    bool RecallMemories(const std::string& query, std::vector<ELLE_MEMORY_RECORD>& out,
                        uint32_t maxCount = 10, float minRelevance = 0.3f);

    bool RecallRecentLTM(std::vector<ELLE_MEMORY_RECORD>& out, uint32_t maxCount = 10);
    bool UpdateMemoryAccess(uint64_t memId);
    bool PromoteToMTM(uint64_t memId);
    bool PromoteToLTM(uint64_t memId);
    bool ArchiveMemory(uint64_t memId);

    std::string GetSubjective(const std::string& key);

    bool StoreEmotionSnapshot(const ELLE_EMOTION_STATE& state);
    bool GetLatestEmotionState(ELLE_EMOTION_STATE& out);

    bool UpdateTrust(int32_t delta, const std::string& reason);
    bool GetTrustState(ELLE_TRUST_STATE& out);

    struct PairedDeviceRow {
        std::string device_id;
        std::string device_name;
        uint64_t    paired_at_ms  = 0;
        uint64_t    expires_ms    = 0;
        uint64_t    last_seen_ms  = 0;
        bool        revoked       = false;
        uint64_t    revoked_at_ms = 0;
        std::string jwt_fingerprint;
    };

    bool UpsertPairedDevice(const PairedDeviceRow& row);

    bool GetPairedDevice(const std::string& device_id, PairedDeviceRow& out);

    bool ListPairedDevices(std::vector<PairedDeviceRow>& out, uint32_t limit = 50);

    bool RevokePairedDevice(const std::string& device_id);

    bool TouchPairedDeviceLastSeen(const std::string& device_id);

    struct SessionRow {
        std::string token;
        int64_t     nUserNo      = 0;
        std::string sUserID;
        std::string sUserName;
        int32_t     nAuthID      = 0;
        uint64_t    created_ms   = 0;
        uint64_t    last_seen_ms = 0;
        std::string device_name;
        std::string peer_addr;
    };

    bool CreateSession(const SessionRow& row);

    bool GetSessionByToken(const std::string& token, SessionRow& out);

    bool TouchSessionLastSeen(const std::string& token);

    bool DeleteSession(const std::string& token);

    int  DeleteSessionsForUser(int64_t nUserNo);

    bool ListSessions(std::vector<SessionRow>& out, uint32_t limit = 50);

    bool StoreGoal(const ELLE_GOAL_RECORD& goal);

    uint64_t StoreGoalReturningId(const ELLE_GOAL_RECORD& goal);
    bool UpdateGoalProgress(uint64_t goalId, float progress);
    bool UpdateGoalStatus(uint64_t goalId, uint32_t status);
    bool GetActiveGoals(std::vector<ELLE_GOAL_RECORD>& out);

    bool StoreEntity(const ELLE_WORLD_ENTITY& entity);
    bool GetEntity(const std::string& name, ELLE_WORLD_ENTITY& out);
    bool UpdateEntityInteraction(uint64_t entityId);

    bool GetAllEntities(std::vector<ELLE_WORLD_ENTITY>& out);

    struct MemoryRow {
        int64_t id; int type; int tier;
        std::string content; std::string summary;
        float emotional_valence; float importance; float relevance;
        int access_count; uint64_t created_ms; uint64_t last_access_ms;
    };
    bool ListMemories(std::vector<MemoryRow>& out, int memory_type ,
                      uint32_t limit, uint32_t offset);
    bool GetMemory(int64_t memId, MemoryRow& out);
    bool DeleteMemory(int64_t memId);
    bool UpdateMemoryContent(int64_t memId, const std::string& content,
                              const std::string& summary, float importance);

    struct ConversationRow {
        int32_t id; int32_t user_id; std::string title;
        std::string started_at; std::string last_message_at;
        int32_t total_messages; bool is_active;
    };
    bool CreateConversation(int32_t user_id, const std::string& title, int32_t& newId);
    bool ListConversations(std::vector<ConversationRow>& out, uint32_t limit = 50);
    bool GetConversation(int32_t convId, ConversationRow& out);

    bool StartVoiceCall(int32_t user_id, int32_t conv_id, std::string& callId);
    bool EndVoiceCall(const std::string& callId);

    int64_t CountTable(const std::string& table);

    struct CrystalProfile {
        bool        found = false;
        std::string traits;
        std::string vulnerability_patterns;
        std::string comfort_patterns;
        std::string trigger_patterns;
        std::string preferred_tone;
        float       trust_level    = 0.0f;
        float       intimacy_level = 0.0f;
    };
    bool GetCrystalProfile(int32_t user_id, CrystalProfile& out);

    struct ElleThread {
        int32_t     id = 0;
        std::string topic;
        std::string status;
        float       emotional_weight = 0.0f;
        float       intensity = 0.0f;
        std::string summary;
        std::string unresolved_questions;
    };

    bool GetOpenThreads(std::vector<ElleThread>& out, uint32_t limit = 5);

    struct UserPresence {
        bool        found = false;
        int32_t     silence_minutes = 0;
        int32_t     threshold_minutes = 0;
        std::string silence_interpretation;
        int32_t     abnormal_silence_count = 0;
    };
    bool GetUserPresence(int32_t user_id, UserPresence& out);
    bool UpdateUserPresenceOnInteraction(int32_t user_id);

    bool RegisterWorker(ELLE_SERVICE_ID svc, const std::string& name);
    bool UpdateWorkerHeartbeat(ELLE_SERVICE_ID svc);
    bool GetWorkerStatuses(std::vector<ELLE_SERVICE_STATUS>& out);

    bool WriteLog(ELLE_LOG_LEVEL level, ELLE_SERVICE_ID svc, const std::string& msg);
    bool GetRecentLogs(std::vector<ELLE_LOG_ENTRY>& out, uint32_t count = 100,
                       ELLE_LOG_LEVEL minLevel = LOG_INFO);

    bool RecordMetric(const std::string& name, double value);

    struct LearnedSubject {
        int32_t     id = 0;
        std::string subject;
        std::string category;
        int32_t     proficiency_level = 0;
        std::string who_taught;
        std::string where_learned;
        float       time_to_learn_hours = 0.0f;
        std::string notes;
        std::string date_started;
        std::string date_completed;
    };
    struct EducationReference {
        int32_t     id = 0;
        int32_t     subject_id = 0;
        std::string reference_type;
        std::string reference_title;
        std::string reference_content;
        std::string file_path;
        float       relevance_score = 0.5f;
        std::string notes;
    };
    struct LearningMilestone {
        int32_t     id = 0;
        int32_t     subject_id = 0;
        std::string milestone;
        std::string description;
        std::string achieved_at;
    };
    struct Skill {
        int32_t     id = 0;
        std::string skill_name;
        std::string category;
        int32_t     proficiency = 0;
        int32_t     learned_from_subject_id = 0;
        int32_t     times_used = 0;
        std::string last_used;
        std::string notes;
    };
    bool ListSubjects(std::vector<LearnedSubject>& out,
                      const std::string& category , uint32_t limit = 50);
    bool GetSubject(int32_t subject_id, LearnedSubject& out);
    bool CreateSubject(const LearnedSubject& in, int32_t& newId);
    bool UpdateSubject(int32_t subject_id, const LearnedSubject& patch,
                       const std::vector<std::string>& fieldsToUpdate);
    bool ListSubjectReferences(int32_t subject_id, std::vector<EducationReference>& out);
    bool AddSubjectReference(const EducationReference& in);
    bool ListSubjectMilestones(int32_t subject_id, std::vector<LearningMilestone>& out);
    bool AddSubjectMilestone(const LearningMilestone& in);
    bool ListSkills(std::vector<Skill>& out, const std::string& category );
    bool CreateSkill(const Skill& in, int32_t& newId);
    bool RecordSkillUse(const std::string& skill_name);

    struct VideoJob {
        int64_t     id = 0;
        std::string job_uuid;
        std::string text;
        std::string avatar_path;
        int64_t     call_id = 0;
        std::string status;
        int32_t     progress = 0;
        std::string output_path;
        std::string error;
        int64_t     created_ms = 0;
        int64_t     started_ms = 0;
        int64_t     finished_ms = 0;
    };
    bool CreateVideoJob(const std::string& text, const std::string& avatar_path,
                        int64_t call_id, VideoJob& out);
    bool GetVideoJob(const std::string& job_uuid, VideoJob& out);
    bool ClaimNextVideoJob(VideoJob& out);
    bool UpdateVideoJobProgress(const std::string& job_uuid, int32_t progress);
    bool CompleteVideoJob(const std::string& job_uuid, const std::string& output_path);
    bool FailVideoJob(const std::string& job_uuid, const std::string& error);

    struct UserAvatar {
        int32_t     id = 0;
        int32_t     user_id = 0;
        std::string label;
        std::string file_path;
        std::string mime_type;
        bool        is_default = false;
    };
    bool RegisterAvatar(const UserAvatar& in, int32_t& newId);
    bool GetDefaultAvatar(int32_t user_id, UserAvatar& out);
    bool ListAvatars(int32_t user_id, std::vector<UserAvatar>& out);

    struct DictionaryLoaderState {
        std::string status;
        int32_t     loaded = 0;
        int32_t     failed = 0;
        int32_t     skipped = 0;
        std::string last_word;
        std::string error;
        int64_t     started_ms = 0;
        int64_t     updated_ms = 0;
    };
    bool GetDictionaryLoaderState(DictionaryLoaderState& out);
    bool UpsertDictionaryLoaderState(const DictionaryLoaderState& in);
    bool InsertDictionaryWord(const std::string& word,
                              const std::string& part_of_speech,
                              const std::string& definition,
                              const std::string& example);
    int64_t CountDictionaryWords();

    bool DeriveDriveState(ELLE_DRIVE_STATE& out);

    bool PersistEmotionSnapshot(const ELLE_EMOTION_STATE& state);
    bool LoadLatestEmotionSnapshot(ELLE_EMOTION_STATE& out);

    struct EmotionHistoryPoint {
        int64_t     taken_ms = 0;
        float       valence  = 0.0f;
        float       arousal  = 0.0f;
        float       dominance= 0.0f;
    };

    bool GetEmotionHistory(uint32_t hours,
                           std::vector<EmotionHistoryPoint>& out,
                           uint32_t maxPoints = 500);
}

#endif
