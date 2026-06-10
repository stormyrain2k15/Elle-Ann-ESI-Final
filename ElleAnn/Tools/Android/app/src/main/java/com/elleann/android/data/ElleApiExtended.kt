package com.elleann.android.data

import com.elleann.android.data.models.*
import okhttp3.MultipartBody
import okhttp3.RequestBody
import retrofit2.http.*

interface ElleApiExtended {

    @GET("/api/auth/me")
    suspend fun getMe(): MeResponse

    @GET("/api/auth/me/recap")
    suspend fun getRecap(): RecapResponse

    @GET("/api/auth/session/greeting")
    suspend fun getSessionGreeting(): SessionGreeting

    @POST("/api/auth/session/greeting/{id}/ack")
    suspend fun ackSessionGreeting(@Path("id") id: Long): OkResponse

    @POST("/api/auth/logout")
    suspend fun logout(): OkResponse

    @GET("/api/memory/")
    suspend fun getMemories(): MemoryListResponse

    @POST("/api/memory/")
    suspend fun createMemory(@Body request: CreateMemoryRequest): Memory

    @GET("/api/memory/self-image/current")
    suspend fun getSelfImage(): SelfImageResponse

    @GET("/api/memory/{id}")
    suspend fun getMemory(@Path("id") id: Long): Memory

    @PUT("/api/memory/{id}")
    suspend fun updateMemory(@Path("id") id: Long, @Body memory: Memory): Memory

    @DELETE("/api/memory/{id}")
    suspend fun deleteMemory(@Path("id") id: Long): OkResponse

    @POST("/api/memory/{id}/files")
    suspend fun attachFileToMemory(
        @Path("id") id: Long,
        @Body body: AttachFileRequest,
    ): OkResponse

    @GET("/api/emotions/dimensions")
    suspend fun getEmotionDimensions(): EmotionDimensionsResponse

    @GET("/api/emotions/dimensions/{name}")
    suspend fun getEmotionDimension(@Path("name") name: String): EmotionDimension

    @PUT("/api/emotions/dimensions/{name}")
    suspend fun setEmotionDimension(
        @Path("name") name: String,
        @Body body: SetEmotionDimensionRequest,
    ): OkResponse

    @GET("/api/emotions/weights")
    suspend fun getEmotionWeights(): EmotionWeightsResponse

    @GET("/api/emotional-context/history")
    suspend fun getEmotionHistory(
        @Query("hours") hours: Int = 24,
        @Query("points") points: Int = 500,
    ): EmotionHistoryResponse

    @GET("/api/emotional-context/dimensions")
    suspend fun getEmotionDimensionsAtTime(
        @Query("t") timestampMs: Long = 0L,
        @Query("top") topN: Int = 5,
    ): EmotionDimensionsResponse

    @GET("/api/emotional-context/patterns")
    suspend fun getEmotionalPatterns(): List<EmotionalPattern>

    @GET("/api/emotional-context/vocabulary")
    suspend fun getEmotionalVocabulary(): List<VocabularyTerm>

    @GET("/api/emotional-context/growth")
    suspend fun getEmotionalGrowth(): EmotionGrowthResponse

    @GET("/api/tokens/conversations")
    suspend fun getConversations(): ConversationListResponse

    @POST("/api/tokens/conversations")
    suspend fun createConversation(@Body request: NewConversationRequest): Conversation

    @GET("/api/tokens/conversations/{id}")
    suspend fun getConversation(@Path("id") id: Long): Conversation

    @POST("/api/tokens/conversations/{id}/messages")
    suspend fun sendMessageThreaded(
        @Path("id") conversationId: Long,
        @Body request: SendMessageRequest,
    ): Message

    @GET("/api/tokens/conversations/{id}/messages")
    suspend fun getMessages(@Path("id") conversationId: Long): MessageListResponse

    @POST("/api/tokens/interactions")
    suspend fun logInteraction(@Body request: InteractionRequest): OkResponse

    @POST("/api/ai/chat")
    suspend fun chat(@Body request: ChatRequest): ChatResponse

    @GET("/api/ai/status")
    suspend fun getAiStatus(): AiStatus

    @GET("/api/ai/self-prompts")
    suspend fun getSelfPrompts(@Query("limit") limit: Int = 50): SelfPromptListResponse

    @POST("/api/ai/self-prompts/generate")
    suspend fun generateSelfPrompt(): SelfPrompt

    @POST("/api/ai/analyze-emotion")
    suspend fun analyzeEmotion(@Body request: AnalyzeEmotionRequest): AnalyzeEmotionResponse

    @GET("/api/ai/memory-tracking")
    suspend fun getMemoryTracking(): MemoryTrackingResponse

    @GET("/api/ai/autonomy/status")
    suspend fun getAutonomyStatus(): AutonomyStatus

    @GET("/api/ai/hardware/info")
    suspend fun getHardwareInfo(): HardwareInfo

    @GET("/api/ai/hardware/actions/pending")
    suspend fun getPendingHardwareActions(): PendingActionsResponse

    @POST("/api/ai/hardware/actions/claim")
    suspend fun claimHardwareAction(@Body body: ClaimHardwareActionRequest): HardwareAction

    @POST("/api/ai/hardware/actions/{id}/ack")
    suspend fun ackHardwareAction(@Path("id") id: Long): OkResponse

    @POST("/api/ai/hardware/actions/{id}/result")
    suspend fun reportHardwareResult(
        @Path("id") id: Long,
        @Body body: CompleteHardwareActionRequest,
    ): OkResponse

    @GET("/api/ai/agents")
    suspend fun getAgents(): AgentListResponse

    @POST("/api/ai/agents")
    suspend fun createAgent(@Body request: CreateAgentRequest): Agent

    @DELETE("/api/ai/agents/{name}")
    suspend fun deleteAgent(@Path("name") name: String): OkResponse

    @POST("/api/ai/agents/{name}/chat")
    suspend fun chatWithAgent(
        @Path("name") agentName: String,
        @Body request: AgentChatRequest,
    ): ChatResponse

    @GET("/api/ai/tools")
    suspend fun getTools(): ToolListResponse

    @POST("/api/ai/tools")
    suspend fun createTool(@Body request: CreateToolRequest): AiTool

    @DELETE("/api/ai/tools/{name}")
    suspend fun deleteTool(@Path("name") name: String): OkResponse

    @GET("/api/video/avatars")
    suspend fun getAvatars(): AvatarListResponse

    @GET("/api/video/avatar")
    suspend fun getDefaultAvatar(): UserAvatar

    @POST("/api/video/avatar/upload")
    suspend fun uploadAvatar(@Body request: UploadAvatarRequest): UserAvatar

    @POST("/api/video/generate")
    suspend fun generateVideo(@Body request: GenerateVideoRequest): VideoJob

    @GET("/api/video/status/{job_id}")
    suspend fun getVideoStatus(@Path("job_id") jobId: String): VideoJob

    @GET("/api/goals")
    suspend fun getGoals(): GoalListResponse

    @GET("/api/education/subjects")
    suspend fun getSubjects(
        @Query("category") category: String? = null,
        @Query("limit") limit: Int = 50,
    ): SubjectListResponse

    @GET("/api/education/subjects/{id}")
    suspend fun getSubject(@Path("id") id: Int): LearnedSubject

    @POST("/api/education/subjects")
    suspend fun createSubject(@Body subject: CreateSubjectRequest): LearnedSubject

    @PUT("/api/education/subjects/{id}")
    suspend fun updateSubject(
        @Path("id") id: Int,
        @Body patch: UpdateSubjectRequest,
    ): OkResponse

    @POST("/api/education/subjects/{id}/milestones")
    suspend fun addMilestone(
        @Path("id") subjectId: Int,
        @Body milestone: CreateMilestoneRequest,
    ): Milestone

    @POST("/api/education/subjects/{id}/references")
    suspend fun addReference(
        @Path("id") subjectId: Int,
        @Body reference: CreateReferenceRequest,
    ): EducationReference

    @GET("/api/education/skills")
    suspend fun getSkills(): SkillListResponse

    @POST("/api/education/skills")
    suspend fun createSkill(@Body skill: CreateSkillRequest): Skill

    @PUT("/api/education/skills/{name}/use")
    suspend fun useSkill(@Path("name") name: String): OkResponse

    @GET("/api/dictionary/lookup/{word}")
    suspend fun lookupWord(@Path("word") word: String): DictionaryWord

    @GET("/api/dictionary/search")
    suspend fun searchDictionary(@Query("q") query: String): List<DictionaryWord>

    @GET("/api/dictionary/random")
    suspend fun getRandomWord(): DictionaryWord

    @GET("/api/dictionary/stats")
    suspend fun getDictionaryStats(): DictionaryStats

    @GET("/api/dictionary/load/status")
    suspend fun getDictionaryLoadStatus(): DictionaryLoadStatus

    @POST("/api/dictionary/load")
    suspend fun loadDictionary(@Body body: LoadDictionaryRequest = LoadDictionaryRequest()): OkResponse

    @GET("/api/models/personality")
    suspend fun getPersonality(): PersonalityResponse

    @GET("/api/models/slots")
    suspend fun getModelSlots(): ModelSlotListResponse

    @POST("/api/models/slots")
    suspend fun createModelSlot(@Body slot: ModelSlot): ModelSlot

    @DELETE("/api/models/slots/{slot_number}")
    suspend fun deleteModelSlot(@Path("slot_number") slotNumber: Int): OkResponse

    @POST("/api/models/slots/{slot_number}/ping")
    suspend fun pingModelSlot(@Path("slot_number") slotNumber: Int): OkResponse

    @GET("/api/models/token-cache/stats")
    suspend fun getTokenCacheStats(): TokenCacheStats

    @GET("/api/models/workers")
    suspend fun getModelWorkers(): ModelWorkerListResponse

    @POST("/api/models/workers")
    suspend fun createModelWorker(@Body worker: CreateModelWorkerRequest): ModelWorker

    @PUT("/api/models/workers/{worker_id}/decommission")
    suspend fun decommissionWorker(@Path("worker_id") workerId: Int): OkResponse

    @GET("/api/morals/rules")
    suspend fun getMoralRules(): MoralRulesResponse

    @POST("/api/morals/rules")
    suspend fun createMoralRule(@Body request: CreateMoralRuleRequest): MoralRule

    @GET("/api/server/status")
    suspend fun getServerStatus(): ServerStatus

    @GET("/api/server/analytics")
    suspend fun getServerAnalytics(): ServerAnalytics

    @GET("/api/server/settings")
    suspend fun getServerSettings(): ServerSettings

    @PUT("/api/server/settings")
    suspend fun updateServerSettings(@Body request: UpdateSettingsRequest): OkResponse

    @GET("/api/server/console")
    suspend fun getConsoleLogs(
        @Query("limit") limit: Int = 100,
        @Query("level") level: String? = null,
    ): LogListResponse

    @DELETE("/api/server/console")
    suspend fun clearConsoleLogs(): OkResponse

    @GET("/api/server/backups")
    suspend fun getBackups(): BackupListResponse

    @POST("/api/server/backup")
    suspend fun createBackup(): OkResponse

    @POST("/api/server/commit-memory")
    suspend fun commitMemory(): OkResponse

    @POST("/api/server/commit-emotional-memory")
    suspend fun commitEmotionalMemory(): OkResponse

    @GET("/api/diag/queues")
    suspend fun getDiagQueues(): DiagQueuesResponse

    @GET("/api/diag/routes")
    suspend fun getDiagRoutes(): DiagRoutesResponse

    @GET("/api/diag/wires")
    suspend fun getDiagWires(): DiagWiresResponse

    @GET("/api/diag/heartbeats")
    suspend fun getDiagHeartbeats(): DiagHeartbeatsResponse

    @GET("/api/diag/health")
    suspend fun getDiagHealth(): DiagHealthResponse

    @GET("/api/memory/why")
    suspend fun getMemoryWhy(@Query("entities") entities: String): MemoryWhyResponse

    @GET("/api/identity/private-thoughts")
    suspend fun getPrivateThoughts(
        @Query("limit") limit: Int = 50,
        @Query("resolved") resolved: Boolean? = null,
    ): PrivateThoughtsResponse

    @GET("/api/identity/autobiography")
    suspend fun getAutobiography(@Query("limit") limit: Int = 30): AutobiographyResponse

    @GET("/api/identity/preferences")
    suspend fun getIdentityPreferences(
        @Query("domain") domain: String? = null,
    ): IdentityPreferencesResponse

    @GET("/api/identity/traits")
    suspend fun getIdentityTraits(): IdentityTraitsResponse

    @GET("/api/identity/snapshots")
    suspend fun getIdentitySnapshots(@Query("limit") limit: Int = 20): IdentitySnapshotsResponse

    @GET("/api/identity/growth-log")
    suspend fun getGrowthLog(@Query("limit") limit: Int = 50): GrowthLogResponse

    @GET("/api/identity/felt-time")
    suspend fun getFeltTime(): FeltTime

    @GET("/api/identity/consent-log")
    suspend fun getConsentLog(@Query("limit") limit: Int = 50): ConsentLogResponse

    @retrofit2.http.Streaming
    @GET("/api/video/file/{job_id}")
    suspend fun getVideoFile(
        @retrofit2.http.Path("job_id") jobId: String,
    ): retrofit2.Response<okhttp3.ResponseBody>

    @POST("/api/admin/reload")
    suspend fun reloadConfig(): OkResponse

    @GET("/api/brain/status")
    suspend fun getBrainStatus(): Map<String, String>

    @GET("/api/hal/status")
    suspend fun getHalStatus(): HardwareInfo

    @GET("/api/auth/devices")
    suspend fun getPairedDevices(): PairedDevicesResponse

    @DELETE("/api/auth/devices/{id}")
    suspend fun revokeDevice(@Path("id") deviceId: String): OkResponse

    @GET("/api/x/state")
    suspend fun getXState(): XState

    @GET("/api/x/history")
    suspend fun getXHistory(): XHistory

    @GET("/api/x/fertility_window")
    suspend fun getFertilityWindow(): FertilityWindow

    @GET("/api/x/next_period")
    suspend fun getNextPeriod(): NextPeriod

    @GET("/api/x/pregnancy")
    suspend fun getPregnancy(): Pregnancy

    @GET("/api/x/pregnancy/events")
    suspend fun getPregnancyEvents(): PregnancyEventsResponse

    @GET("/api/x/contraception")
    suspend fun getContraception(): Contraception

    @POST("/api/x/contraception")
    suspend fun setContraception(@Body contraception: Contraception): OkResponse

    @GET("/api/x/lifecycle")
    suspend fun getXLifecycle(): XLifecycle

    @GET("/api/x/modulation")
    suspend fun getXModulation(): XModulation

    @GET("/api/x/symptoms")
    suspend fun getXSymptoms(): XSymptomsResponse

    @POST("/api/x/symptoms")
    suspend fun logSymptom(@Body request: LogSymptomRequest): OkResponse

    @POST("/api/x/cycle/anchor")
    suspend fun setCycleAnchor(@Body request: CycleAnchorRequest): OkResponse

    @POST("/api/x/conception/attempt")
    suspend fun attemptConception(@Body request: AttemptConceptionRequest = AttemptConceptionRequest()): OkResponse

    @POST("/api/x/lifecycle")
    suspend fun setLifecycle(@Body lifecycle: XLifecycle): OkResponse

    @POST("/api/x/pregnancy/accelerate")
    suspend fun acceleratePregnancy(@Body request: AcceleratePregnancyRequest = AcceleratePregnancyRequest()): OkResponse

    @POST("/api/shn/save")
    suspend fun saveSHN(@Body request: ShnSaveRequest): ShnSaveResponse

    @GET("/api/shn/list")
    suspend fun listSHN(@Query("root") root: String = "Hero"): ShnListResponse

    @GET("/api/shn/get")
    suspend fun getSHN(
        @Query("root") root: String,
        @Query("name") name: String
    ): ShnGetResponse

    @GET("/api/shn/history")
    suspend fun historySHN(
        @Query("name") name: String,
        @Query("limit") limit: Int = 20
    ): ShnHistoryResponse
}
