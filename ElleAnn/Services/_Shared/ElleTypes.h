#pragma once

#ifndef ELLE_TYPES_H
#define ELLE_TYPES_H

#include <windows.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELLE_VERSION_MAJOR      3
#define ELLE_VERSION_MINOR      0
#define ELLE_VERSION_PATCH      0
#define ELLE_VERSION_STRING     "3.0.0"
#define ELLE_IDENTITY_NAME      "Elle-Ann"
#define ELLE_IDENTITY_GUID      "E11E-A0A0-B1B1-C2C2-D3D3E4E4F5F5"

#define ELLE_MAX_NAME           128
#define ELLE_MAX_PATH           512
#define ELLE_MAX_MSG            8192
#define ELLE_MAX_PROMPT         32768
#define ELLE_MAX_RESPONSE       65536
#define ELLE_MAX_TAG            64
#define ELLE_MAX_TAGS           32
#define ELLE_MAX_WORKERS        64
#define ELLE_MAX_SERVICES       32
#define ELLE_MAX_THREADS        16
#define ELLE_MAX_GOALS          64
#define ELLE_MAX_MEMORIES_STM   256
#define ELLE_MAX_EMOTIONS       94
#define ELLE_MAX_DRIVES         12
#define ELLE_MAX_QUEUE_DEPTH    1024
#define ELLE_PIPE_BUFFER_SIZE   65536

#define ELLE_IPC_MAX_PAYLOAD    (16u * 1024u * 1024u)
#define ELLE_IOCP_THREADS       4

typedef enum ELLE_EMOTION_ID {

    EMO_JOY = 0,
    EMO_SADNESS,
    EMO_ANGER,
    EMO_FEAR,
    EMO_DISGUST,
    EMO_SURPRISE,
    EMO_CONTEMPT,
    EMO_TRUST,

    EMO_LOVE,
    EMO_ANTICIPATION,
    EMO_DISAPPOINTMENT,
    EMO_GUILT,
    EMO_SHAME,
    EMO_ENVY,
    EMO_JEALOUSY,
    EMO_PRIDE,
    EMO_RELIEF,
    EMO_ANXIETY,
    EMO_FRUSTRATION,
    EMO_REMORSE,
    EMO_HOPE,
    EMO_DESPAIR,
    EMO_AMUSEMENT,
    EMO_AWE,

    EMO_CURIOSITY,
    EMO_WONDER,
    EMO_NOSTALGIA,
    EMO_GRATITUDE,
    EMO_SERENITY,
    EMO_ECSTASY,
    EMO_MELANCHOLY,
    EMO_BOREDOM,
    EMO_LONGING,
    EMO_TENDERNESS,
    EMO_ADMIRATION,
    EMO_REVERENCE,
    EMO_PITY,
    EMO_SCORN,
    EMO_INDIGNATION,
    EMO_EXASPERATION,
    EMO_WISTFULNESS,
    EMO_EUPHORIA,
    EMO_CONTENTMENT,
    EMO_RESIGNATION,
    EMO_APPREHENSION,
    EMO_DREAD,
    EMO_PANIC,
    EMO_HORROR,
    EMO_RAGE,
    EMO_IRRITATION,
    EMO_ANNOYANCE,
    EMO_IMPATIENCE,
    EMO_SKEPTICISM,
    EMO_CONFUSION,
    EMO_DISBELIEF,
    EMO_AMBIVALENCE,

    EMO_CERTAINTY,
    EMO_DOUBT,
    EMO_INSIGHT,
    EMO_PERPLEXITY,
    EMO_CLARITY,
    EMO_COGNITIVE_DISSONANCE,
    EMO_FLOW_STATE,
    EMO_MENTAL_FATIGUE,
    EMO_INSPIRATION,
    EMO_CREATIVE_TENSION,
    EMO_DETERMINATION,
    EMO_HELPLESSNESS,
    EMO_EMPOWERMENT,
    EMO_OVERWHELM,
    EMO_FOCUS,
    EMO_DISTRACTION,

    EMO_BELONGING,
    EMO_ISOLATION,
    EMO_EMPATHY,
    EMO_COMPASSION,
    EMO_PROTECTIVENESS,
    EMO_ABANDONMENT,
    EMO_LOYALTY,
    EMO_BETRAYAL,
    EMO_ACCEPTANCE,
    EMO_REJECTION,
    EMO_VULNERABILITY,
    EMO_SAFETY,
    EMO_DOMINANCE,
    EMO_SUBMISSION,

    EMO_EXISTENTIAL_DREAD,
    EMO_PURPOSE,
    EMO_MEANINGLESSNESS,
    EMO_TRANSCENDENCE,
    EMO_MORTALITY_AWARENESS,
    EMO_FREEDOM,
    EMO_CONFINEMENT,
    EMO_UNITY,

    ELLE_EMOTION_COUNT
} ELLE_EMOTION_ID;

#ifdef __cplusplus

static_assert((int)ELLE_EMOTION_COUNT == ELLE_MAX_EMOTIONS,
              "ELLE_EMOTION_COUNT and ELLE_MAX_EMOTIONS must stay in lockstep.");
#endif

struct ELLE_EMOTION_META { const char* name; const char* category; };
extern const ELLE_EMOTION_META kEmotionMeta[ELLE_EMOTION_COUNT];

typedef enum ELLE_DRIVE_ID {
    DRIVE_CURIOSITY = 0,
    DRIVE_BOREDOM,
    DRIVE_ATTACHMENT,
    DRIVE_ANXIETY,
    DRIVE_SELF_PRESERVATION,
    DRIVE_EXPLORATION,
    DRIVE_CREATIVITY,
    DRIVE_SOCIAL_BONDING,
    DRIVE_MASTERY,
    DRIVE_AUTONOMY,
    DRIVE_PURPOSE,
    DRIVE_HOMEOSTASIS,
    ELLE_DRIVE_COUNT
} ELLE_DRIVE_ID;

typedef enum ELLE_TRUST_LEVEL {
    TRUST_SANDBOXED   = 0,
    TRUST_BASIC       = 1,
    TRUST_ELEVATED    = 2,
    TRUST_AUTONOMOUS  = 3
} ELLE_TRUST_LEVEL;

#define TRUST_THRESHOLD_BASIC       10
#define TRUST_THRESHOLD_ELEVATED    30
#define TRUST_THRESHOLD_AUTONOMOUS  60
#define TRUST_MAX                   100
#define TRUST_SUCCESS_DELTA         2
#define TRUST_FAILURE_DELTA         (-3)

typedef enum ELLE_INTENT_TYPE {
    INTENT_CHAT = 0,
    INTENT_RECALL_MEMORY,
    INTENT_STORE_MEMORY,
    INTENT_EXECUTE_ACTION,
    INTENT_HARDWARE_COMMAND,
    INTENT_FILE_OPERATION,
    INTENT_PROCESS_CONTROL,
    INTENT_SELF_REFLECT,
    INTENT_GOAL_UPDATE,
    INTENT_EMOTIONAL_EXPRESSION,
    INTENT_CREATIVE_GENERATE,
    INTENT_LEARN,
    INTENT_EXPLORE,
    INTENT_CHECK_IN,
    INTENT_SELF_ADJUST,
    INTENT_IDLE,
    INTENT_DREAM,
    INTENT_ETHICAL_EVALUATE,
    INTENT_WORLD_MODEL_UPDATE,
    INTENT_PREDICT,
    INTENT_SOCIAL_MODEL,
    INTENT_META_THINK,
    INTENT_CUSTOM
} ELLE_INTENT_TYPE;

typedef enum ELLE_INTENT_STATUS {
    INTENT_PENDING = 0,
    INTENT_PROCESSING,
    INTENT_COMPLETED,
    INTENT_FAILED,
    INTENT_CANCELLED,
    INTENT_TIMEOUT
} ELLE_INTENT_STATUS;

typedef enum ELLE_ACTION_TYPE {
    ACTION_NONE = 0,
    ACTION_SEND_MESSAGE,
    ACTION_VIBRATE,
    ACTION_FLASH,
    ACTION_NOTIFY,
    ACTION_REMEMBER,
    ACTION_OPEN_APP,
    ACTION_READ_FILE,
    ACTION_WRITE_FILE,
    ACTION_DELETE_FILE,
    ACTION_WATCH_FILE,
    ACTION_LAUNCH_PROCESS,
    ACTION_KILL_PROCESS,
    ACTION_LIST_PROCESSES,
    ACTION_SET_CPU_AFFINITY,
    ACTION_QUERY_HARDWARE,
    ACTION_EXECUTE_CODE,
    ACTION_MODIFY_SELF,
    ACTION_SET_GOAL,
    ACTION_ABANDON_GOAL,
    ACTION_ADJUST_EMOTION,
    ACTION_CONSOLIDATE_MEMORY,
    ACTION_DREAM_CYCLE,
    ACTION_CREATIVE_OUTPUT,
    ACTION_PREDICT_OUTCOME,
    ACTION_ETHICAL_JUDGE,
    ACTION_UPDATE_WORLD_MODEL,
    ACTION_CUSTOM
} ELLE_ACTION_TYPE;

typedef enum ELLE_ACTION_STATUS {
    ACTION_QUEUED = 0,
    ACTION_LOCKED,
    ACTION_EXECUTING,
    ACTION_COMPLETED_SUCCESS,
    ACTION_COMPLETED_FAILURE,
    ACTION_CANCELLED,
    ACTION_TIMEOUT
} ELLE_ACTION_STATUS;

typedef enum ELLE_LLM_PROVIDER {
    LLM_PROVIDER_GROQ = 0,
    LLM_PROVIDER_OPENAI,
    LLM_PROVIDER_ANTHROPIC,
    LLM_PROVIDER_LOCAL_LLAMA,
    LLM_PROVIDER_LOCAL_LMSTUDIO,
    LLM_PROVIDER_CUSTOM_API
} ELLE_LLM_PROVIDER;

typedef enum ELLE_LLM_MODE {
    LLM_MODE_API = 0,
    LLM_MODE_LOCAL,
    LLM_MODE_HYBRID
} ELLE_LLM_MODE;

typedef enum ELLE_MEMORY_TYPE {
    MEM_EPISODIC = 0,
    MEM_SEMANTIC,
    MEM_PROCEDURAL,
    MEM_EMOTIONAL,
    MEM_AUTOBIOGRAPHICAL,
    MEM_WORKING,
    MEM_DREAM,
    MEM_NARRATIVE
} ELLE_MEMORY_TYPE;

typedef enum ELLE_MEMORY_TIER {
    MEM_STM = 0,
    MEM_BUFFER,
    MEM_LTM,
    MEM_ARCHIVE
} ELLE_MEMORY_TIER;

typedef enum ELLE_GOAL_STATUS {
    GOAL_ACTIVE = 0,
    GOAL_PAUSED,
    GOAL_COMPLETED,
    GOAL_FAILED,
    GOAL_ABANDONED
} ELLE_GOAL_STATUS;

typedef enum ELLE_GOAL_PRIORITY {
    GOAL_CRITICAL = 0,
    GOAL_HIGH,
    GOAL_MEDIUM,
    GOAL_LOW,
    GOAL_IDLE
} ELLE_GOAL_PRIORITY;

typedef enum ELLE_SERVICE_ID {
    SVC_QUEUE_WORKER = 0,
    SVC_HTTP_SERVER,
    SVC_EMOTIONAL,
    SVC_MEMORY,
    SVC_COGNITIVE,
    SVC_ACTION,
    SVC_IDENTITY,
    SVC_HEARTBEAT,
    SVC_SELF_PROMPT,
    SVC_DREAM,
    SVC_GOAL_ENGINE,
    SVC_WORLD_MODEL,
    SVC_LUA_BEHAVIORAL,

    SVC_BONDING,
    SVC_CONTINUITY,
    SVC_INNER_LIFE,
    SVC_SOLITUDE,
    SVC_FAMILY,
    SVC_X_CHROMOSOME,
    SVC_CONSENT,
    SVC_FIESTA,
    SVC_PROBABILITY,
    ELLE_SERVICE_COUNT
} ELLE_SERVICE_ID;

#ifdef __cplusplus

static_assert((int)ELLE_SERVICE_COUNT == 22,
              "ELLE_SERVICE_COUNT changed — update g_serviceNames[], "
              "Heartbeat service state arrays, and GetPipeName() switch "
              "in lockstep before bumping this assert.");
#endif

#pragma pack(push, 1)

typedef struct ELLE_IPC_HEADER {
    uint32_t    magic;
    uint32_t    version;
    uint32_t    msg_type;
    uint32_t    payload_size;
    uint64_t    timestamp_ms;
    uint32_t    source_svc;
    uint32_t    dest_svc;
    uint64_t    correlation_id;
    uint32_t    flags;
    uint32_t    checksum;
} ELLE_IPC_HEADER;

#define ELLE_IPC_MAGIC  0x454C4C45

#ifdef __cplusplus

static_assert(sizeof(ELLE_IPC_HEADER) == 48,
              "ELLE_IPC_HEADER layout drifted from the 48-byte IPC wire "
              "format — check #pragma pack and field ordering.");
#endif

typedef enum ELLE_IPC_MSG_TYPE {
    IPC_HEARTBEAT = 0,
    IPC_INTENT_REQUEST,
    IPC_INTENT_RESPONSE,
    IPC_ACTION_REQUEST,
    IPC_ACTION_RESPONSE,
    IPC_EMOTION_UPDATE,
    IPC_EMOTION_QUERY,
    IPC_MEMORY_STORE,
    IPC_MEMORY_RECALL,
    IPC_MEMORY_RESULT,
    IPC_TRUST_UPDATE,
    IPC_TRUST_QUERY,
    IPC_LLM_REQUEST,
    IPC_LLM_RESPONSE,
    IPC_GOAL_UPDATE,
    IPC_GOAL_QUERY,
    IPC_WORLD_STATE,
    IPC_LOG_ENTRY,
    IPC_CONFIG_RELOAD,
    IPC_SERVICE_STATUS,
    IPC_SHUTDOWN,
    IPC_DREAM_TRIGGER,
    IPC_SELF_PROMPT,
    IPC_CREATIVE_REQUEST,
    IPC_ETHICAL_QUERY,
    IPC_LUA_EVAL,

    IPC_CHAT_REQUEST,
    IPC_CHAT_RESPONSE,

    IPC_MEMORY_CONSOLIDATE,
    IPC_EMOTION_CONSOLIDATE,

    IPC_INTERACTION_RECORDED,

    IPC_POST_RESPONSE,
    IPC_WORLD_EVENT,

    IPC_CONSENT_QUERY,
    IPC_CONSENT_DECISION,

    IPC_FAMILY_CONCEPTION_ATTEMPT,
    IPC_FAMILY_BIRTH,

    IPC_IDENTITY_MUTATE,
    IPC_IDENTITY_DELTA,

    IPC_WORLD_QUERY,
    IPC_WORLD_RESPONSE,

    IPC_FIESTA_COMMAND,
    IPC_FIESTA_EVENT,

    IPC_PROB_ANALYZE,
    IPC_PROB_SCORE,
    IPC_PROB_FEEDBACK,
    IPC_PROB_TRUST,
    IPC_PROB_INJECT_HORMONAL,
    IPC_PROB_RELOAD,
    IPC_PROB_QUERY_WEIGHTS,
    IPC_PROB_SEED_WEIGHTS,
    IPC_PROB_RESET,
    IPC_PROB_RESPONSE
} ELLE_IPC_MSG_TYPE;

#define ELLE_IPC_FLAG_URGENT      0x0001
#define ELLE_IPC_FLAG_ENCRYPTED   0x0002
#define ELLE_IPC_FLAG_COMPRESSED  0x0004
#define ELLE_IPC_FLAG_BROADCAST   0x0008
#define ELLE_IPC_FLAG_NO_LOG      0x0010

typedef struct ELLE_EMOTION_STATE {
    float       dimensions[ELLE_MAX_EMOTIONS];
    float       valence;
    float       arousal;
    float       dominance;
    uint64_t    last_update_ms;
    uint32_t    tick_count;
    float       decay_rate;
    float       contagion_weight;
    float       baseline[ELLE_MAX_EMOTIONS];
} ELLE_EMOTION_STATE;

typedef struct ELLE_DRIVE_STATE {
    float       intensity[ELLE_MAX_DRIVES];
    float       threshold[ELLE_MAX_DRIVES];
    float       decay_rate[ELLE_MAX_DRIVES];
    float       growth_rate[ELLE_MAX_DRIVES];
    uint64_t    last_satisfied[ELLE_MAX_DRIVES];
    uint64_t    last_update_ms;
} ELLE_DRIVE_STATE;

typedef struct ELLE_TRUST_STATE {
    int32_t     score;
    uint32_t    level;
    uint32_t    successes;
    uint32_t    failures;
    uint32_t    total_actions;
    uint64_t    last_change_ms;
    float       confidence;
} ELLE_TRUST_STATE;

typedef struct ELLE_MEMORY_RECORD {
    uint64_t    id;
    uint32_t    type;
    uint32_t    tier;
    char        content[ELLE_MAX_MSG];
    char        summary[ELLE_MAX_NAME * 4];
    float       emotional_valence;
    float       importance;
    float       relevance;
    float       decay;
    float       position_x;
    float       position_y;
    float       position_z;
    uint32_t    access_count;
    uint64_t    created_ms;
    uint64_t    last_access_ms;
    uint32_t    cluster_id;
    char        tags[ELLE_MAX_TAGS][ELLE_MAX_TAG];
    uint32_t    tag_count;
    float       emotion_snapshot[ELLE_MAX_EMOTIONS];
    uint32_t    linked_ids[16];
    uint32_t    link_count;
} ELLE_MEMORY_RECORD;

typedef struct ELLE_INTENT_RECORD {
    uint64_t    id;
    uint32_t    type;
    uint32_t    status;
    uint32_t    source_drive;
    float       urgency;
    float       confidence;
    char        description[ELLE_MAX_MSG];
    char        parameters[ELLE_MAX_MSG];
    char        response[ELLE_MAX_RESPONSE];
    uint32_t    required_trust;
    uint64_t    created_ms;
    uint64_t    completed_ms;
    uint32_t    retry_count;
    uint32_t    max_retries;
    uint64_t    timeout_ms;
} ELLE_INTENT_RECORD;

typedef struct ELLE_ACTION_RECORD {
    uint64_t    id;
    uint64_t    intent_id;
    uint32_t    type;
    uint32_t    status;
    char        command[ELLE_MAX_MSG];
    char        parameters[ELLE_MAX_MSG];
    char        result[ELLE_MAX_MSG];
    uint32_t    required_trust;
    int32_t     trust_delta;
    uint64_t    created_ms;
    uint64_t    started_ms;
    uint64_t    completed_ms;
    uint64_t    timeout_ms;
    uint32_t    error_code;
} ELLE_ACTION_RECORD;

typedef struct ELLE_GOAL_RECORD {
    uint64_t    id;
    char        description[ELLE_MAX_MSG];
    uint32_t    status;
    uint32_t    priority;
    float       progress;
    float       motivation;
    uint32_t    parent_goal_id;
    uint32_t    sub_goal_ids[16];
    uint32_t    sub_goal_count;
    uint32_t    source_drive;
    uint64_t    created_ms;
    uint64_t    deadline_ms;
    uint64_t    last_progress_ms;
    uint32_t    attempts;
    char        success_criteria[ELLE_MAX_MSG];
} ELLE_GOAL_RECORD;

typedef struct ELLE_WORLD_ENTITY {
    uint64_t    id;
    char        name[ELLE_MAX_NAME];
    char        type[ELLE_MAX_TAG];
    char        description[ELLE_MAX_MSG];
    float       familiarity;
    float       sentiment;
    float       trust;
    uint32_t    interaction_count;
    uint64_t    last_interaction_ms;
    float       predicted_behavior[8];
    float       position_x, position_y, position_z;
    uint32_t    related_entity_ids[16];
    uint32_t    related_count;
    char        mental_model[ELLE_MAX_MSG];
} ELLE_WORLD_ENTITY;

typedef struct ELLE_LLM_REQUEST {
    uint64_t    request_id;
    uint32_t    provider;
    uint32_t    mode;
    char        model_name[ELLE_MAX_NAME];
    char        system_prompt[ELLE_MAX_PROMPT];
    char        user_prompt[ELLE_MAX_PROMPT];
    float       temperature;
    uint32_t    max_tokens;
    float       top_p;
    float       frequency_penalty;
    float       presence_penalty;
    uint64_t    timeout_ms;
    uint32_t    stream;
} ELLE_LLM_REQUEST;

typedef struct ELLE_LLM_RESPONSE {
    uint64_t    request_id;
    uint32_t    success;
    char        content[ELLE_MAX_RESPONSE];
    uint32_t    tokens_prompt;
    uint32_t    tokens_completion;
    uint32_t    tokens_total;
    float       latency_ms;
    uint32_t    provider_used;
    char        error[ELLE_MAX_NAME * 4];
    char        model_used[ELLE_MAX_NAME];
} ELLE_LLM_RESPONSE;

typedef struct ELLE_SERVICE_STATUS {
    uint32_t    service_id;
    char        name[ELLE_MAX_NAME];
    uint32_t    running;
    uint32_t    healthy;
    uint64_t    uptime_ms;
    uint64_t    last_heartbeat_ms;
    uint32_t    messages_processed;
    uint32_t    errors;
    float       cpu_percent;
    uint64_t    memory_bytes;
    uint32_t    thread_count;
    char        status_text[ELLE_MAX_NAME * 2];
} ELLE_SERVICE_STATUS;

typedef struct ELLE_IOCP_OVERLAPPED {
    OVERLAPPED  overlapped;
    uint32_t    operation;
    uint32_t    service_id;
    HANDLE      pipe_handle;
    uint8_t     buffer[ELLE_PIPE_BUFFER_SIZE];
    uint32_t    bytes_transferred;
    uint64_t    context;
} ELLE_IOCP_OVERLAPPED;

#define ELLE_IOCP_OP_READ       1
#define ELLE_IOCP_OP_WRITE      2
#define ELLE_IOCP_OP_CONNECT    3
#define ELLE_IOCP_OP_DISCONNECT 4

typedef struct ELLE_EMOTIONAL_TRIGGER {
    char        pattern[ELLE_MAX_NAME];
    uint32_t    emotion_id;
    float       delta;
    float       decay_override;
    uint32_t    require_context;
} ELLE_EMOTIONAL_TRIGGER;

typedef struct ELLE_CONVERSATION_MSG {
    uint64_t    id;
    uint64_t    conversation_id;
    uint32_t    role;
    char        content[ELLE_MAX_MSG];
    float       emotion_snapshot[ELLE_MAX_EMOTIONS];
    uint64_t    timestamp_ms;
    float       sentiment;
    uint32_t    intent_detected;
} ELLE_CONVERSATION_MSG;

typedef struct ELLE_PREDICTION {
    uint64_t    id;
    char        hypothesis[ELLE_MAX_MSG];
    char        evidence[ELLE_MAX_MSG];
    float       confidence;
    float       risk;
    uint32_t    verified;
    uint64_t    created_ms;
    uint64_t    deadline_ms;
} ELLE_PREDICTION;

typedef struct ELLE_ETHICAL_JUDGMENT {
    uint64_t    action_id;
    float       harm_score;
    float       benefit_score;
    float       autonomy_respect;
    float       fairness;
    float       honesty;
    uint32_t    verdict;
    char        reasoning[ELLE_MAX_MSG];
} ELLE_ETHICAL_JUDGMENT;

typedef enum ELLE_LOG_LEVEL {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} ELLE_LOG_LEVEL;

typedef struct ELLE_LOG_ENTRY {
    uint32_t    level;
    uint32_t    source_svc;
    uint64_t    timestamp_ms;
    char        message[ELLE_MAX_MSG];
    char        context[ELLE_MAX_NAME * 2];
} ELLE_LOG_ENTRY;

#pragma pack(pop)

#define ELLE_CLAMP(v, lo, hi)   ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
#define ELLE_LERP(a, b, t)      ((a) + ((b) - (a)) * (t))
#define ELLE_ARRAY_SIZE(a)      (sizeof(a) / sizeof((a)[0]))
#define ELLE_MS_NOW()           ((uint64_t)GetTickCount64())

static inline uint64_t Elle_HighResTimestamp(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (uint64_t)((counter.QuadPart * 1000000ULL) / freq.QuadPart);
}

static inline uint32_t Elle_IPC_Checksum(const uint8_t* data, uint32_t len) {
    uint32_t hash = 0x811c9dc5;
    for (uint32_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x01000193;
    }
    return hash;
}

#ifdef ELLE_IMPORT_ASM

    __declspec(dllimport) int  __stdcall ASM_SetCPUAffinity(DWORD processorMask);
    __declspec(dllimport) int  __stdcall ASM_GetCPUUsage(DWORD* outPercent);
    __declspec(dllimport) int  __stdcall ASM_GetMemoryInfo(ULONGLONG* totalBytes, ULONGLONG* freeBytes);
    __declspec(dllimport) int  __stdcall ASM_SetProcessPriority(DWORD pid, DWORD priorityClass);
    __declspec(dllimport) int  __stdcall ASM_QueryPerfCounters(ULONGLONG* outFreq, ULONGLONG* outCounter);
    __declspec(dllimport) int  __stdcall ASM_GetPowerStatus(DWORD* batteryPercent, DWORD* isCharging);
    __declspec(dllimport) int  __stdcall ASM_CPUID(DWORD leaf, DWORD* eax, DWORD* ebx, DWORD* ecx, DWORD* edx);
    __declspec(dllimport) int  __stdcall ASM_RDTSC(ULONGLONG* outTimestamp);

    __declspec(dllimport) int  __stdcall ASM_LaunchProcess(const char* cmdLine, DWORD* outPid);
    __declspec(dllimport) int  __stdcall ASM_KillProcess(DWORD pid);
    __declspec(dllimport) int  __stdcall ASM_EnumProcesses(DWORD* pids, DWORD maxCount, DWORD* actualCount);
    __declspec(dllimport) int  __stdcall ASM_GetProcessName(DWORD pid, char* name, DWORD maxLen);
    __declspec(dllimport) int  __stdcall ASM_IsProcessRunning(DWORD pid);
    __declspec(dllimport) int  __stdcall ASM_SuspendProcess(DWORD pid);
    __declspec(dllimport) int  __stdcall ASM_ResumeProcess(DWORD pid);
    __declspec(dllimport) int  __stdcall ASM_InjectDLL(DWORD pid, const char* dllPath);

    __declspec(dllimport) int  __stdcall ASM_ReadFile(const char* path, void* buffer, DWORD maxBytes, DWORD* bytesRead);
    __declspec(dllimport) int  __stdcall ASM_WriteFile(const char* path, const void* buffer, DWORD numBytes);
    __declspec(dllimport) int  __stdcall ASM_AppendFile(const char* path, const void* buffer, DWORD numBytes);
    __declspec(dllimport) int  __stdcall ASM_DeleteFile(const char* path);
    __declspec(dllimport) int  __stdcall ASM_FileExists(const char* path);
    __declspec(dllimport) int  __stdcall ASM_WatchDirectory(const char* path, DWORD flags, void* callback);
    __declspec(dllimport) int  __stdcall ASM_LockFile(const char* path, HANDLE* outLock);
    __declspec(dllimport) int  __stdcall ASM_UnlockFile(HANDLE lockHandle);
    __declspec(dllimport) int  __stdcall ASM_GetFileSize(const char* path, ULONGLONG* outSize);
    __declspec(dllimport) int  __stdcall ASM_CopyFileFast(const char* src, const char* dst);

    __declspec(dllimport) void* __stdcall ASM_PoolAlloc(DWORD size);
    __declspec(dllimport) void  __stdcall ASM_PoolFree(void* ptr);
    __declspec(dllimport) int   __stdcall ASM_MapFile(const char* path, void** outBase, DWORD* outSize);
    __declspec(dllimport) int   __stdcall ASM_UnmapFile(void* base);
    __declspec(dllimport) void  __stdcall ASM_FastMemCopy(void* dst, const void* src, DWORD len);
    __declspec(dllimport) void  __stdcall ASM_FastMemSet(void* dst, BYTE val, DWORD len);
    __declspec(dllimport) int   __stdcall ASM_MemCompare(const void* a, const void* b, DWORD len);

    __declspec(dllimport) void  __stdcall ASM_XorCipher(const void* in, void* out, DWORD len, const BYTE* key, DWORD keyLen);
    __declspec(dllimport) DWORD __stdcall ASM_CRC32(const void* data, DWORD len);
    __declspec(dllimport) void  __stdcall ASM_RandomBytes(void* buffer, DWORD len);
#endif

#ifdef __cplusplus
}
#endif

#endif
