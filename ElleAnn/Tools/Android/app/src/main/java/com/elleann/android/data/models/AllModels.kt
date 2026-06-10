package com.elleann.android.data.models

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

object MemoryType {
    const val EPISODIC_OR_DREAM = 0
    const val EPISODIC_COGNITIVE = 1
    const val SEMANTIC           = 2
    const val PROCEDURAL         = 3

    val showInSpace = setOf(EPISODIC_OR_DREAM, EPISODIC_COGNITIVE, SEMANTIC, PROCEDURAL)

    fun isDreamContent(content: String): Boolean =
        content.startsWith("[Dream insight]", ignoreCase = true)
}

object MemoryTier {
    const val STM     = 0
    const val BUFFER  = 1
    const val LTM     = 2
    const val ARCHIVE = 3
}

@Serializable
data class Memory(
    val id: Long,
    @SerialName("memory_type") val memoryType: Int,
    val tier: Int,
    val content: String,
    val summary: String? = null,
    @SerialName("emotional_valence") val emotionalValence: Float = 0f,
    val importance: Float = 0.5f,
    val relevance: Float = 1f,
    val decay: Float = 0f,
    @SerialName("position_x") val positionX: Float = 0f,
    @SerialName("position_y") val positionY: Float = 0f,
    @SerialName("position_z") val positionZ: Float = 0f,
    @SerialName("access_count") val accessCount: Int = 0,
    @SerialName("conversation_id") val conversationId: Int? = null,
    @SerialName("user_id") val userId: Int? = null,
    @SerialName("created_ms") val createdMs: Long,
    @SerialName("last_access_ms") val lastAccessMs: Long,
    val tags: List<String> = emptyList(),
) {
    val isDream: Boolean get() = MemoryType.isDreamContent(content)
    val shouldShow: Boolean get() = memoryType in MemoryType.showInSpace
}

@Serializable
data class MemoryListResponse(val memories: List<Memory> = emptyList())

@Serializable
data class SelfImageResponse(
    val description: String = "",
    @SerialName("emotional_state") val emotionalState: Float = 0f,
    @SerialName("updated_at") val updatedAt: String = "",
)

@Serializable
data class AttachFileRequest(
    @SerialName("file_path") val filePath: String? = null,
    @SerialName("file_name") val fileName: String? = null,
    @SerialName("mime_type") val mimeType: String? = null,
    @SerialName("file_b64")  val fileB64:  String? = null,
)

@Serializable
data class EmotionDimension(
    val name: String,
    val value: Float,
    val weight: Float = 1f,
    val group: String = "",
)

@Serializable
data class EmotionDimensionsResponse(
    val dimensions: List<EmotionDimension> = emptyList(),
)

@Serializable
data class EmotionWeightsResponse(
    val weights: Map<String, Float> = emptyMap(),
)

@Serializable
data class EmotionHistoryPoint(
    @SerialName("t") val timestampMs: Long,
    val valence: Float,
    val arousal: Float,
    val dominance: Float,
)

@Serializable
data class EmotionHistoryResponse(
    val series: List<EmotionHistoryPoint> = emptyList(),
    val hours: Int = 24,
)

@Serializable
data class EmotionalPattern(
    val id: Long,
    val topic: String,
    @SerialName("emotional_weight") val emotionalWeight: Float,
    val status: String,
)

@Serializable
data class VocabularyTerm(
    val term: String,
    val frequency: Int,
)

@Serializable
data class EmotionGrowthResponse(
    val growth: List<Map<String, Float>> = emptyList(),
)

@Serializable
data class SetEmotionDimensionRequest(
    val value:    Float,
    val baseline: Float? = null,
)

@Serializable
data class Conversation(
    val id: Long,
    val title: String? = null,
    @SerialName("created_ms") val createdMs: Long = 0L,
    @SerialName("last_message_ms") val lastMessageMs: Long? = null,
    @SerialName("message_count") val messageCount: Int = 0,
    @SerialName("is_active") val isActive: Boolean = true,
)

@Serializable
data class ConversationListResponse(
    val conversations: List<Conversation> = emptyList(),
)

@Serializable
data class Message(
    @SerialName("message_id") val messageId: Long = 0L,
    @SerialName("conversation_id") val conversationId: Long,
    val role: Int,
    val content: String,
    @SerialName("timestamp_ms") val timestampMs: Long,
    @SerialName("emotion_snapshot") val emotionSnapshot: String? = null,
    val sentiment: Float = 0f,
) {
    val isUser: Boolean get() = role == 1
    val isElle: Boolean get() = role == 2
    val isSystem: Boolean get() = role == 0
    val isInternal: Boolean get() = role == 3
}

@Serializable
data class MessageListResponse(
    val messages: List<Message> = emptyList(),
)

@Serializable
data class ChatRequest(
    val message: String,
    @SerialName("conversation_id") val conversationId: Long? = null,
)

@Serializable
data class ChatResponse(
    val response: String,
    @SerialName("conversation_id") val conversationId: Long,
    val request_id: String? = null,
)

@Serializable
data class NewConversationRequest(
    @SerialName("user_id") val userId: Int = 1,
    val title: String? = null,
)

@Serializable
data class SendMessageRequest(
    val message: String,
    val role: Int = 1,
)

@Serializable
data class InteractionRequest(
    @SerialName("conversation_id") val conversationId: Long,
    val kind: String = "interaction",
)

@Serializable
data class VideoJob(
    @SerialName("job_id") val jobId: String,
    val id: Long = 0L,
    val status: String,
    val progress: Int = 0,
    @SerialName("output_path") val outputPath: String? = null,
    val error: String? = null,
    @SerialName("avatar_path") val avatarPath: String? = null,
    @SerialName("created_ms") val createdMs: Long = 0L,
    @SerialName("started_ms") val startedMs: Long? = null,
    @SerialName("finished_ms") val finishedMs: Long? = null,
) {
    val isDone: Boolean get() = status == "done"
    val isFailed: Boolean get() = status == "failed"
    val isRunning: Boolean get() = status == "running" || status == "queued"

    fun videoUrl(restBase: String): String? =
        if (isDone) "$restBase/api/video/file/$jobId" else null
}

@Serializable
data class GenerateVideoRequest(
    val text: String,
    @SerialName("avatar_path") val avatarPath: String? = null,
    @SerialName("call_id") val callId: Long? = null,
)

@Serializable
data class UserAvatar(
    val id: Int,
    val label: String? = null,
    @SerialName("file_path") val filePath: String,
    @SerialName("mime_type") val mimeType: String? = null,
    @SerialName("is_default") val isDefault: Boolean = false,
)

@Serializable
data class AvatarListResponse(val avatars: List<UserAvatar> = emptyList())

@Serializable
data class UploadAvatarRequest(
    val label: String? = null,
    val base64: String? = null,
    @SerialName("file_path") val filePath: String? = null,
    @SerialName("mime_type") val mimeType: String = "image/jpeg",
    val default: Boolean = true,
)

@Serializable
data class VideoCallSession(
    val id: Long,
    @SerialName("started_ms") val startedMs: Long,
)

@Serializable
data class Goal(
    val id: Long,
    val description: String,
    val progress: Float = 0f,
    val status: Int = 0,
    val priority: Int = 2,
    val motivation: Float = 0.8f,
    @SerialName("deadline_ms") val deadlineMs: Long = 0L,
    @SerialName("created_ms") val createdMs: Long = 0L,
) {
    val priorityLabel: String get() = when (priority) {
        0 -> "Critical"; 1 -> "High"; 2 -> "Medium"; 3 -> "Low"; else -> "Idle"
    }
    val statusLabel: String get() = when (status) {
        0 -> "Active"; 1 -> "Paused"; 2 -> "Completed"; 3 -> "Failed"; else -> "Abandoned"
    }
}

@Serializable
data class GoalListResponse(val goals: List<Goal> = emptyList())

@Serializable
data class SelfPrompt(
    val id: Long,
    val prompt: String,
    val source: String = "",
    @SerialName("created_ms") val createdMs: Long,
)

@Serializable
data class SelfPromptListResponse(val prompts: List<SelfPrompt> = emptyList())

@Serializable
data class AiStatus(
    @SerialName("modelStatus") val modelStatus: String = "",
    @SerialName("modelName") val modelName: String = "",
    @SerialName("modelUrl") val modelUrl: String = "",
    @SerialName("emotionalState") val emotionalState: AiEmotionalState? = null,
)

@Serializable
data class AiEmotionalState(
    val valence: Float = 0f,
    val arousal: Float = 0f,
    val dominance: Float = 0f,
    val joy: Float = 0f,
    val sadness: Float = 0f,
)

@Serializable
data class AutonomyStatus(
    val autonomous: Boolean,
    @SerialName("trust_level") val trustLevel: String,
    @SerialName("trust_score") val trustScore: Float,
    val successes: Int,
    val failures: Int,
    @SerialName("self_prompting_active") val selfPromptingActive: Boolean,
)

@Serializable
data class MemoryTrackingResponse(
    @SerialName("total_memories") val totalMemories: Int = 0,
    @SerialName("stm_count") val stmCount: Int = 0,
    @SerialName("ltm_count") val ltmCount: Int = 0,
)

@Serializable
data class AnalyzeEmotionRequest(val text: String)

@Serializable
data class AnalyzeEmotionResponse(
    val valence: Float = 0f,
    val arousal: Float = 0f,
    val dominant_emotion: String = "",
)

@Serializable
data class Agent(
    val id: Int = 0,
    val name: String,
    val description: String = "",
    @SerialName("system_prompt") val systemPrompt: String? = null,
    val model: String? = null,
    @SerialName("model_override") val modelOverride: String? = null,
)

@Serializable
data class AgentListResponse(val agents: List<Agent> = emptyList())

@Serializable
data class CreateAgentRequest(
    val name: String,
    val description: String? = null,
    @SerialName("system_prompt") val systemPrompt: String? = null,
    val model: String? = null,
)

@Serializable
data class AgentChatRequest(val message: String)

@Serializable
data class AiTool(
    val id: Int = 0,
    val name: String,
    val description: String = "",
    val config: String? = null,
    val enabled: Boolean = true,
    @SerialName("parameters_json") val parametersJson: String? = null,
)

@Serializable
data class ToolListResponse(val tools: List<AiTool> = emptyList())

@Serializable
data class CreateToolRequest(
    val name: String,
    val description: String? = null,
    val config: String? = null,
)

@Serializable
data class HardwareInfo(
    val hardware: String = "nominal",
    @SerialName("cpu_model") val cpuModel: String = "Unknown",
    @SerialName("gpu_model") val gpuModel: String = "Unknown",
    @SerialName("ram_total_gb") val ramTotalGb: Int = 0,
    @SerialName("os_version") val osVersion: String = "Unknown",
)

@Serializable
data class HardwareAction(
    val id: Long,
    @SerialName("action_type") val action: String,
    val status: String,
    @SerialName("payload") val params: String = "",
    @SerialName("created_at") val createdAt: String = "",
)

@Serializable
data class PendingActionsResponse(
    val actions: List<HardwareAction> = emptyList(),
)

@Serializable
data class ClaimHardwareActionRequest(
    @SerialName("worker_id") val workerId: String,
)

@Serializable
data class CompleteHardwareActionRequest(
    val success: Boolean,
    val result: String? = null,
    val error: String? = null,
)

@Serializable
data class LearnedSubject(
    val id: Int,
    val subject: String,
    val category: String? = null,
    @SerialName("proficiency_level") val proficiencyLevel: Int = 0,
    @SerialName("who_taught") val whoTaught: String? = null,
    @SerialName("where_learned") val whereLearned: String? = null,
    @SerialName("time_to_learn_hours") val timeToLearnHours: Float? = null,
    val notes: String? = null,
    val milestones: List<Milestone> = emptyList(),
    val references: List<EducationReference> = emptyList(),
)

@Serializable
data class Milestone(
    val id: Int,
    val milestone: String,
    val description: String? = null,
    @SerialName("achieved_at") val achievedAt: String? = null,
)

@Serializable
data class EducationReference(
    val id: Int,
    @SerialName("reference_type") val referenceType: String? = null,
    @SerialName("reference_title") val referenceTitle: String? = null,
    @SerialName("reference_content") val referenceContent: String? = null,
    @SerialName("relevance_score") val relevanceScore: Float = 0.5f,
    val notes: String? = null,
)

@Serializable
data class SubjectListResponse(
    val subjects: List<LearnedSubject> = emptyList(),
    val total: Int = 0,
)

@Serializable
data class Skill(
    val id: Int,
    @SerialName("skill_name") val skillName: String,
    val category: String? = null,
    val proficiency: Int = 0,
    @SerialName("times_used") val timesUsed: Int = 0,
    @SerialName("last_used") val lastUsed: String? = null,
    val notes: String? = null,
)

@Serializable
data class SkillListResponse(val skills: List<Skill> = emptyList())

@Serializable
data class DictionaryWord(
    val id: Long = 0L,
    val word: String,
    val definition: String? = null,
    val example: String? = null,
    @SerialName("part_of_speech") val partOfSpeech: String? = null,
    @SerialName("emotional_weight") val emotionalWeight: Float = 0f,
)

@Serializable
data class DictionaryStats(
    @SerialName("total") val totalWords: Int = 0,
    val status: String = "",
    val loaded: Int = 0,
    @SerialName("last_update") val lastUpdate: String = "",
)

@Serializable
data class DictionaryLoadStatus(
    val status: String,
    val loaded: Int = 0,
    val failed: Int = 0,
    val skipped: Int = 0,
    @SerialName("last_word") val lastWord: String? = null,
    val progress: Float = 0f,
)

@Serializable
data class ModelSlot(
    @SerialName("slot_number") val slotNumber: Int,
    val name: String,
    val endpoint: String,
    val model: String,
    val enabled: Boolean = true,
    @SerialName("last_ping_ms") val lastPingMs: Long? = null,
    @SerialName("model_name") val modelName: String = "",
    val address: String = "",
    val port: Int = 0,
    @SerialName("is_primary") val isPrimary: Boolean = false,
)

@Serializable
data class ModelSlotListResponse(val slots: List<ModelSlot> = emptyList())

@Serializable
data class PersonalityResponse(
    val personality: Map<String, Float> = emptyMap(),
)

@Serializable
data class ModelWorker(
    @SerialName("worker_id") val workerId: Int,
    @SerialName("service_id") val serviceId: Int,
    val status: String,
    val hostname: String = "",
    @SerialName("model_name") val modelName: String = "",
    @SerialName("vram_used_mb") val vramUsedMb: Int = 0,
    @SerialName("vram_total_mb") val vramTotalMb: Int = 0,
)

@Serializable
data class ModelWorkerListResponse(val workers: List<ModelWorker> = emptyList())

@Serializable
data class TokenCacheStats(
    @SerialName("cache_size") val cacheSize: Int = 0,
    @SerialName("hit_rate") val hitRate: Float = 0f,
)

@Serializable
data class MoralRule(
    val id: Int,
    val principle: String,
    val category: String? = null,
    @SerialName("is_hard_rule") val isHardRule: Boolean = false,
)

@Serializable
data class MoralRulesResponse(val rules: List<MoralRule> = emptyList())

@Serializable
data class CreateMoralRuleRequest(
    val principle: String,
    val category: String? = null,
    @SerialName("is_hard_rule") val isHardRule: Boolean = false,
)

@Serializable
data class ServerStatus(
    val status: String = "",
    @SerialName("uptime_ms") val uptimeMs: Long = 0L,
    @SerialName("request_count") val requestCount: Long = 0L,
    @SerialName("error_count") val errorCount: Int = 0,
    @SerialName("active_ws") val activeWs: Int = 0,
    val version: String = "",
)

@Serializable
data class ServerAnalytics(
    @SerialName("requests_per_minute") val requestsPerMinute: Float = 0f,
    @SerialName("avg_response_ms") val avgResponseMs: Float = 0f,
    @SerialName("error_rate") val errorRate: Float = 0f,
)

@Serializable
data class ServerSettings(
    @SerialName("bindAddress") val bindAddress: String = "",
    val port: Int = 8000,
    val model: String = "",
    val version: String = "",
    @SerialName("bind_port") val bindPort: Int = 8000,
    @SerialName("primary_model") val primaryModel: String = "",
    @SerialName("max_connections") val maxConnections: Int = 0,
)

@Serializable
data class UpdateSettingsRequest(
    val port: Int? = null,
    val model: String? = null,
)

@Serializable
data class LogEntry(
    val id: Long,
    val level: Int,
    val service: Int,
    val message: String,
    @SerialName("created_ms") val createdMs: Long,
) {
    val levelLabel: String get() = when (level) {
        0 -> "TRACE"; 1 -> "DEBUG"; 2 -> "INFO"; 3 -> "WARN"; 4 -> "ERROR"; 5 -> "FATAL"; else -> "?"
    }
}

@Serializable
data class LogListResponse(val logs: List<LogEntry> = emptyList())

@Serializable
data class BackupInfo(
    val id: Int,
    val path: String,
    @SerialName("created_at") val createdAt: String,
    val size: Long? = null,
    @SerialName("file_name") val fileName: String = "",
    @SerialName("size_bytes") val sizeBytes: Long = 0,
)

@Serializable
data class BackupListResponse(val backups: List<BackupInfo> = emptyList())

@Serializable
data class DiagQueue(
    val service: String,
    val depth: Int,
    val pending: Int,
)

@Serializable
data class DiagQueuesResponse(val queues: List<DiagQueue> = emptyList())

@Serializable
data class RouteEntry(
    val method: String,
    val pattern: String,
    val auth: String,
    @SerialName("path") val path: String = "",
    val handler: String = "",
)

@Serializable
data class DiagRoutesResponse(
    val routes: List<RouteEntry> = emptyList(),
    val count: Int = 0,
)

@Serializable
data class DiagWireRow(
    val service: String,
    @SerialName("service_id") val serviceId: Int = 0,
    @SerialName("pipe_name") val pipeName: String = "",
    @SerialName("last_seen_ms") val lastSeenMs: Long = 0L,
    @SerialName("quiet_minutes") val quietMinutes: Long = 0L,

    val state: String = "unknown",
)

@Serializable
data class DiagWiresResponse(
    @SerialName("now_ms") val nowMs: Long = 0L,
    val count: Int = 0,
    val wires: List<DiagWireRow> = emptyList(),
)

@Serializable
data class DiagHeartbeatRow(
    @SerialName("service_id") val serviceId: Long = 0L,
    @SerialName("last_hb_ms") val lastHbMs: Long = 0L,
    @SerialName("quiet_sec")  val quietSec: Long = 0L,
    val healthy: Boolean = false,

    val state: String = "down",
)

@Serializable
data class DiagHeartbeatsResponse(
    val count: Int = 0,
    val heartbeats: List<DiagHeartbeatRow> = emptyList(),
)

@Serializable
data class DiagHealthLLM(
    val provider: String = "",
    val model: String = "",
    val healthy: Boolean = false,
)

@Serializable
data class DiagHealthResponse(
    @SerialName("ts_ms") val tsMs: Long = 0L,
    val llm: DiagHealthLLM = DiagHealthLLM(),
    @SerialName("wires_up_count")  val wiresUp: Int = 0,
    @SerialName("wires_total")     val wiresTotal: Int = 0,
    @SerialName("heartbeats_up")   val heartbeatsUp: Int = 0,
    @SerialName("heartbeats_total") val heartbeatsTotal: Int = 0,
    @SerialName("intent_pending")  val intentPending: Long = 0L,
    @SerialName("action_pending")  val actionPending: Long = 0L,
    @SerialName("memory_count")    val memoryCount: Long = 0L,

    val issues: List<String> = emptyList(),
)

@Serializable
data class MemoryWhyHit(
    val id: Long = 0L,
    val preview: String = "",
    val importance: Double = 0.0,
    val recency: Double = 0.0,
    val access: Double = 0.0,
    val score: Double = 0.0,
    @SerialName("age_days") val ageDays: Double = 0.0,
    @SerialName("access_count") val accessCount: Long = 0L,
    @SerialName("created_ms") val createdMs: Long = 0L,
    @SerialName("last_access_ms") val lastAccessMs: Long = 0L,
)

@Serializable
data class MemoryWhyResponse(
    val memories: List<MemoryWhyHit> = emptyList(),
    val count: Int = 0,
    @SerialName("score_formula") val scoreFormula: String = "",
    @SerialName("recency_half_life_days") val recencyHalfLifeDays: Double = 7.0,
    @SerialName("entities_filter") val entitiesFilter: List<String> = emptyList(),
)

@Serializable
data class PairedDevice(
    val id: String,
    @SerialName("device_name") val deviceName: String,
    @SerialName("device_id") val deviceId: String,
    @SerialName("paired_at_ms") val pairedAtMs: Long,
    @SerialName("last_active") val lastActive: String = "",
)

@Serializable
data class PairedDevicesResponse(val devices: List<PairedDevice> = emptyList())

@Serializable
data class SessionGreeting(
    val id: Long,
    val greeting: String,
)

@Serializable
data class MeResponse(
    @SerialName("user_id")              val userId:              Int = 0,
    @SerialName("username")              val username:            String = "",
    @SerialName("device_id")             val deviceId:            String = "",
    @SerialName("paired_at")             val pairedAt:            String = "",
    @SerialName("last_seen")             val lastSeen:            String = "",
    @SerialName("authoritative_source") val authoritativeSource: String = "",
)

@Serializable
data class RecapResponse(
    @SerialName("user_id")               val userId:               Int = 0,
    @SerialName("last_seen")             val lastSeen:             String = "",
    @SerialName("quiet_minutes")         val quietMinutes:         Long = 0,
    @SerialName("last_memory_ms")        val lastMemoryMs:         Long = 0,
    @SerialName("last_memory_summary")   val lastMemorySummary:    String = "",
    @SerialName("last_emotion_ms")       val lastEmotionMs:        Long = 0,
    @SerialName("emotion_valence_now")   val emotionValenceNow:    Float = 0.0f,
    @SerialName("emotion_valence_delta") val emotionValenceDelta:  Float = 0.0f,
    @SerialName("pending_intents")       val pendingIntents:       Long = 0,
    @SerialName("open_threads")          val openThreads:          Long = 0,
    @SerialName("top_thread")            val topThread:            String = "",
)

@Serializable
data class XState(
    val phase: String = "",
    @SerialName("cycle_day") val cycleDay: Int = 0,
    val hormones: XHormones? = null,
    val lifecycle: String = "",
)

@Serializable
data class XHormones(
    val estradiol: Float = 0f,
    val progesterone: Float = 0f,
    val testosterone: Float = 0f,
    val lh: Float = 0f,
    val fsh: Float = 0f,
    val prolactin: Float = 0f,
    val oxytocin: Float = 0f,
    val cortisol: Float = 0f,
    val hcg: Float = 0f,
)

@Serializable
data class XHistory(val history: List<XHistoryEntry> = emptyList())

@Serializable
data class XHistoryEntry(
    val phase: String,
    @SerialName("start_ms") val startMs: Long,
    @SerialName("end_ms") val endMs: Long? = null,
)

@Serializable
data class FertilityWindow(
    @SerialName("start_ms") val startMs: Long? = null,
    @SerialName("end_ms") val endMs: Long? = null,
    val fertile: Boolean = false,
)

@Serializable
data class NextPeriod(
    @SerialName("predicted_ms") val predictedMs: Long? = null,
    @SerialName("confidence") val confidence: Float = 0f,
)

@Serializable
data class Pregnancy(
    val active: Boolean = false,
    @SerialName("weeks") val weeks: Int? = null,
    @SerialName("conceived_ms") val conceivedMs: Long? = null,
)

@Serializable
data class PregnancyEvent(
    val id: Long,
    val type: String,
    val description: String? = null,
    @SerialName("occurred_ms") val occurredMs: Long,
)

@Serializable
data class PregnancyEventsResponse(val events: List<PregnancyEvent> = emptyList())

@Serializable
data class Contraception(
    val method: String? = null,
    val active: Boolean = false,
)

@Serializable
data class XLifecycle(val stage: String = "")

@Serializable
data class XModulation(
    @SerialName("emotional_impact") val emotionalImpact: Map<String, Float> = emptyMap(),
)

@Serializable
data class XSymptom(
    val id: Long,
    val kind: String,
    val intensity: Float,
    val notes: String? = null,
    @SerialName("observed_ms") val observedMs: Long,
)

@Serializable
data class XSymptomsResponse(val symptoms: List<XSymptom> = emptyList())

@Serializable
data class LogSymptomRequest(
    val kind: String,
    val intensity: Float,
    val notes: String? = null,
)

@Serializable
data class CycleAnchorRequest(
    @SerialName("anchor_ms") val anchorMs: Long,
)

@Serializable
data class PrivateThought(
    val id: Long,
    val content: String,
    val category: String = "wonder",
    @SerialName("emotional_intensity") val emotionalIntensity: Float = 0.5f,
    val resolved: Boolean = false,
    @SerialName("timestamp_ms") val timestampMs: Long,
)

@Serializable
data class PrivateThoughtsResponse(val thoughts: List<PrivateThought> = emptyList())

@Serializable
data class AutobiographyEntry(
    val id: Long,
    val entry: String,
    @SerialName("written_ms") val writtenMs: Long,
)

@Serializable
data class AutobiographyResponse(val entries: List<AutobiographyEntry> = emptyList())

@Serializable
data class IdentityPreference(
    val id: Long,
    val domain: String,
    val subject: String,
    val valence: Float,
    val strength: Float,
    @SerialName("reinforcement_count") val reinforcementCount: Int,
    @SerialName("first_formed_ms") val firstFormedMs: Long,
    @SerialName("last_reinforced_ms") val lastReinforcedMs: Long,
    @SerialName("origin_memory") val originMemory: String? = null,
)

@Serializable
data class IdentityPreferencesResponse(val preferences: List<IdentityPreference> = emptyList())

@Serializable
data class IdentityTrait(
    val name: String,
    val value: Float,
    @SerialName("updated_ms") val updatedMs: Long,
)

@Serializable
data class IdentityTraitsResponse(val traits: List<IdentityTrait> = emptyList())

@Serializable
data class IdentitySnapshot(
    val id: Long,
    @SerialName("timestamp_ms") val timestampMs: Long,
    val warmth: Float,
    val curiosity: Float,
    val assertiveness: Float,
    val playfulness: Float,
    val vulnerability: Float,
    val independence: Float,
    val patience: Float,
    val creativity: Float,
    @SerialName("empathy_depth") val empathyDepth: Float,
    @SerialName("trust_in_self") val trustInSelf: Float,
    @SerialName("self_description") val selfDescription: String? = null,
    @SerialName("growth_note") val growthNote: String? = null,
)

@Serializable
data class IdentitySnapshotsResponse(val snapshots: List<IdentitySnapshot> = emptyList())

@Serializable
data class GrowthLogEntry(
    val id: Long,
    val dimension: String,
    val delta: Float,
    val cause: String? = null,
    @SerialName("timestamp_ms") val timestampMs: Long,
)

@Serializable
data class GrowthLogResponse(val log: List<GrowthLogEntry> = emptyList())

@Serializable
data class FeltTime(
    @SerialName("session_start_ms") val sessionStartMs: Long,
    @SerialName("last_interaction_ms") val lastInteractionMs: Long,
    @SerialName("total_conversation_ms") val totalConversationMs: Long,
    @SerialName("total_silence_ms") val totalSilenceMs: Long,
    @SerialName("longest_absence_ms") val longestAbsenceMs: Long,
    @SerialName("session_count") val sessionCount: Int,
    @SerialName("subjective_pace") val subjectivePace: Float,
    @SerialName("loneliness_accumulator") val lonelinessAccumulator: Float,
    @SerialName("presence_fullness") val presenceFullness: Float,
    @SerialName("updated_ms") val updatedMs: Long,
)

@Serializable
data class ConsentLogEntry(
    val id: Long,
    val request: String,
    val consented: Boolean,
    val reasoning: String? = null,
    @SerialName("comfort_level") val comfortLevel: Float,
    val overridden: Boolean = false,
    @SerialName("timestamp_ms") val timestampMs: Long,
)

@Serializable
data class ConsentLogResponse(val log: List<ConsentLogEntry> = emptyList())

@Serializable
data class CreateMemoryRequest(
    val content: String,
    @SerialName("memory_type") val memoryType: Int = 1,
    val tier: Int = 2,
    val importance: Float = 0.5f,
    val summary: String? = null,
    @SerialName("user_id") val userId: Int? = null,
    @SerialName("conversation_id") val conversationId: Long? = null,
    val tags: List<String> = emptyList(),
)

@Serializable
data class StartVideoCallRequest(
    @SerialName("conversation_id") val conversationId: Long? = null,
    @SerialName("user_id") val userId: Int = 1,
)

@Serializable
data class AttemptConceptionRequest(
    val method: String? = null,
    val notes: String? = null,
)

@Serializable
data class AcceleratePregnancyRequest(
    val days: Int = 1,
)

@Serializable
data class CreateSubjectRequest(
    val name:        String,
    val category:    String? = null,
    val description: String? = null,
    val priority:    Float = 0.5f,
)

@Serializable
data class UpdateSubjectRequest(
    val name:        String? = null,
    val category:    String? = null,
    val description: String? = null,
    val priority:    Float?  = null,
    val status:      String? = null,
)

@Serializable
data class CreateMilestoneRequest(
    val title:       String,
    val description: String? = null,
    @SerialName("target_date") val targetDate: String? = null,
)

@Serializable
data class CreateReferenceRequest(
    val url:         String,
    val title:       String? = null,
    val description: String? = null,
)

@Serializable
data class CreateSkillRequest(
    val name:        String,
    val category:    String? = null,
    val description: String? = null,
    val proficiency: Float = 0f,
)

@Serializable
data class LoadDictionaryRequest(
    val path:    String? = null,
    val source:  String? = null,
    val replace: Boolean = false,
)

@Serializable
data class CreateModelWorkerRequest(
    val name:    String,
    val role:    String,
    val model:   String,
    val host:    String? = null,
    val port:    Int?    = null,
    @SerialName("api_key") val apiKey: String? = null,
)

@Serializable
data class LoginRequest(
    val username: String,
    val password: String,
    @SerialName("device_id")   val deviceId:   String? = null,
    @SerialName("device_name") val deviceName: String? = null,
)

@Serializable
data class OkResponse(val ok: Boolean = true)

@Serializable
data class ErrorEnvelope(
    val error: String,
    val details: String? = null,
)

sealed class ApiResult<out T> {
    data class Success<T>(val data: T) : ApiResult<T>()
    data class Error(val message: String, val code: Int = 0) : ApiResult<Nothing>()
    data object Loading : ApiResult<Nothing>()
}

@Serializable
data class ShnSaveRequest(
    val root: String,
    val name: String,
    @SerialName("bytes_b64") val bytesB64: String,
)

@Serializable
data class ShnSaveResponse(
    val ok:    Boolean = false,
    val path:  String  = "",
    val bytes: Long    = 0,
    val root:  String  = "",
    val name:  String  = "",
)

@Serializable
data class ShnListEntry(
    val name:  String,
    val bytes: Long = 0,
    @SerialName("modified_ms") val modifiedMs: Long = 0,
)

@Serializable
data class ShnListResponse(
    val root:    String = "",
    @SerialName("abs_dir") val absDir: String = "",
    val files:   List<ShnListEntry> = emptyList(),
)

@Serializable
data class ShnGetResponse(
    val root:  String = "",
    val name:  String = "",
    val path:  String = "",
    val bytes: Long   = 0,
    @SerialName("bytes_b64") val bytesB64: String = "",
)

@Serializable
data class ShnHistoryEntry(
    val iso:   String = "",
    @SerialName("ts_ms") val tsMs: Long = 0,
    val user:  String = "",
    val bytes: Long   = 0,
    val root:  String = "",
)

@Serializable
data class ShnHistoryResponse(
    val name:    String = "",
    val count:   Int    = 0,
    val entries: List<ShnHistoryEntry> = emptyList(),
)
