package com.elleann.android.data

import android.content.Context
import android.util.Log
import com.elleann.android.data.models.Message
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import java.io.File
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicReference

class ChatCacheManager(private val context: Context) {

    companion object {
        private const val TAG          = "ChatCacheManager"
        private const val CACHE_PREFIX = "elle_chat_"
        private const val CACHE_EXT    = ".txt"

        // Maximum messages to cache — last 50 from user and Elle only
        const val MAX_CACHED_MESSAGES = 50

        private val instanceRef = AtomicReference<ChatCacheManager?>(null)

        fun installAsGlobal(mgr: ChatCacheManager) {
            instanceRef.set(mgr)
        }

        // Called on app crash — flush everything to disk
        fun crashFlush() {
            instanceRef.get()?.flushAllSync()
        }
    }

    private val json = Json {
        ignoreUnknownKeys = true
        isLenient         = true
        encodeDefaults    = true
    }

    // Track which conversations are currently open so we know what to flush
    private val openConversations = ConcurrentHashMap<Long, List<Message>>()

    private fun cacheFile(conversationId: Long): File =
        File(context.filesDir, "$CACHE_PREFIX$conversationId$CACHE_EXT")

    private fun pathLockFor(conversationId: Long): Any =
        ("ChatCache_$conversationId").intern()

    // Register a conversation as open — keeps its messages tracked for flush
    fun track(conversationId: Long, messages: List<Message>) {
        openConversations[conversationId] = messages
    }

    // Unregister — called when navigating AWAY (but NOT flushing yet)
    // Cache is flushed on logout/close, not on navigation
    fun untrack(conversationId: Long) {
        openConversations.remove(conversationId)
    }

    suspend fun loadCache(conversationId: Long): List<Message> =
        withContext(Dispatchers.IO) { loadCacheSync(conversationId) }

    fun loadCacheSync(conversationId: Long): List<Message> {
        val file = cacheFile(conversationId)
        if (!file.exists()) return emptyList()
        return runCatching {
            val raw = file.readText()
            if (raw.isBlank()) emptyList()
            else json.decodeFromString<List<Message>>(raw)
        }.onFailure { e ->
            Log.w(TAG, "Cache read failed for conv $conversationId: ${e.message}")
            file.delete()
        }.getOrElse { emptyList() }
    }

    suspend fun writeCache(conversationId: Long, messages: List<Message>) =
        withContext(Dispatchers.IO) { writeCacheSync(conversationId, messages) }

    fun writeCacheSync(conversationId: Long, messages: List<Message>) {
        synchronized(pathLockFor(conversationId)) {
            runCatching {
                // Keep only user and Elle messages (roles 1 and 2), last 50
                val toCache = messages
                    .filter { it.isUser || it.isElle }
                    .takeLast(MAX_CACHED_MESSAGES)

                val encoded = json.encodeToString(toCache)
                val target  = cacheFile(conversationId)
                val tmp     = File(target.parentFile, target.name + ".tmp")
                tmp.writeText(encoded)

                // Atomic rename
                if (!tmp.renameTo(target)) {
                    tmp.copyTo(target, overwrite = true)
                    tmp.delete()
                }

                Log.d(TAG, "Cached ${toCache.size} messages for conv $conversationId")
            }.onFailure { e ->
                Log.e(TAG, "Cache write failed for conv $conversationId: ${e.message}")
            }
        }
    }

    // Called when a new message is sent — update tracked state but don't flush to disk yet
    fun trackMessage(conversationId: Long, messages: List<Message>) {
        openConversations[conversationId] = messages
    }

    // Flush to disk — called on logout, app close, or crash
    // NOT called on page navigation
    fun flushAllSync() {
        for ((id, msgs) in openConversations) {
            runCatching { writeCacheSync(id, msgs) }
                .onFailure { Log.w(TAG, "flushAllSync conv $id: ${it.message}") }
        }
    }

    suspend fun flushAll() = withContext(Dispatchers.IO) {
        val snapshot = openConversations.toMap()
        for ((id, msgs) in snapshot) writeCacheSync(id, msgs)
    }

    fun flushAllBlocking() {
        flushAllSync()
    }

    // Verify cached messages against server — returns updated list if mismatch
    sealed class VerifyResult {
        data object Match : VerifyResult()
        data class Updated(val messages: List<Message>) : VerifyResult()
        data class Error(val message: String) : VerifyResult()
    }

    suspend fun verifyWithServer(
        conversationId: Long,
        cachedMessages: List<Message>,
        api: ElleApiExtended,
    ): VerifyResult = withContext(Dispatchers.IO) {
        runCatching {
            val server = api.getMessages(conversationId).messages
                .filter { it.isUser || it.isElle }

            data class MsgKey(val id: Long, val role: Int, val content: String, val ts: Long)
            fun List<Message>.keys() =
                map { MsgKey(it.messageId, it.role, it.content, it.timestampMs) }.toSet()

            if (cachedMessages.keys() == server.keys() &&
                cachedMessages.size == server.size) {
                VerifyResult.Match
            } else {
                Log.d(TAG, "Cache mismatch conv $conversationId — " +
                    "cached: ${cachedMessages.size}, server: ${server.size}")
                VerifyResult.Updated(server)
            }
        }.getOrElse { e ->
            Log.w(TAG, "Server verify failed conv $conversationId: ${e.message}")
            VerifyResult.Error(e.message ?: "Verify failed")
        }
    }

    fun clearCache(conversationId: Long) {
        synchronized(pathLockFor(conversationId)) { cacheFile(conversationId).delete() }
        openConversations.remove(conversationId)
    }

    fun clearAll() {
        context.filesDir
            .listFiles { f -> f.name.startsWith(CACHE_PREFIX) && f.name.endsWith(CACHE_EXT) }
            ?.forEach { it.delete() }
        openConversations.clear()
    }

    fun cachedConversationIds(): List<Long> =
        context.filesDir
            .listFiles { f -> f.name.startsWith(CACHE_PREFIX) && f.name.endsWith(CACHE_EXT) }
            ?.mapNotNull {
                it.name.removePrefix(CACHE_PREFIX).removeSuffix(CACHE_EXT).toLongOrNull()
            } ?: emptyList()
}
