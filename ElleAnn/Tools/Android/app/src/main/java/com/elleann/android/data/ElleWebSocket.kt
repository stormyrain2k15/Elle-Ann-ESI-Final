package com.elleann.android.data

import android.util.Log
import com.elleann.android.EmotionsResponse
import com.elleann.android.data.models.XHormones
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.launch
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import okhttp3.WebSocket
import okhttp3.WebSocketListener
import java.util.concurrent.TimeUnit

sealed class WsEvent {

    data class Connected(
        val clientId: String,
        val server: String,
        val version: String,
    ) : WsEvent()

    data object Pong : WsEvent()

    data class ChatResponse(
        val requestId: String,
        val response: String,
    ) : WsEvent()

    data class EmotionUpdate(
        val emotion: EmotionsResponse,
    ) : WsEvent()

    data class HormoneTick(
        val hormones: XHormones,
        val phase: String,
    ) : WsEvent()

    data class PhaseTransition(
        val from: String,
        val to: String,
    ) : WsEvent()

    data object Birth : WsEvent()

    data object LhSurge : WsEvent()

    data class LaborStage(val stage: String) : WsEvent()

    data object Miscarriage : WsEvent()

    data class Disconnected(val reason: String) : WsEvent()

    data class Failure(val reason: String) : WsEvent()

    data class Unknown(val type: String, val raw: String) : WsEvent()
}

class ElleWebSocket(
    private val host: String,
    private val port: Int,
    private val jwt: String,
    private val client: OkHttpClient,
) {
    companion object {
        private const val TAG = "ElleWebSocket"
        private const val WS_PATH = "/command"

        private const val BACKOFF_INITIAL_MS  = 2_000L
        private const val BACKOFF_MAX_MS      = 60_000L
        private const val BACKOFF_MULTIPLIER  = 2.0
        private const val PING_INTERVAL_SECONDS = 30L
    }

    private val _events = MutableSharedFlow<WsEvent>(replay = 0, extraBufferCapacity = 64)
    val events: SharedFlow<WsEvent> = _events.asSharedFlow()

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    private var webSocket: WebSocket? = null
    @Volatile private var isConnected  = false
    @Volatile private var isConnecting = false
    @Volatile private var shouldReconnect = true
    private var reconnectJob: Job? = null

    private val json = Json { ignoreUnknownKeys = true; isLenient = true }

    private val wsClient = client.newBuilder()
        .pingInterval(PING_INTERVAL_SECONDS, TimeUnit.SECONDS)
        .build()

    fun connect() {
        if (isConnected || isConnecting || webSocket != null) return
        shouldReconnect = true
        openConnection()
    }

    fun disconnect() {
        shouldReconnect = false
        reconnectJob?.cancel()
        reconnectJob = null
        webSocket?.close(1000, "Client disconnecting")
        webSocket = null
        isConnected  = false
        isConnecting = false
        scope.cancel()
    }

    fun isConnected(): Boolean = isConnected

    private fun openConnection(attempt: Int = 0) {
        isConnecting = true

        if (host.isBlank() || port <= 0) {
            isConnecting = false
            shouldReconnect = false
            scope.launch {
                _events.emit(WsEvent.Failure(
                    "WebSocket not opened: no host paired yet. " +
                    "Visit Pair screen first."))
            }
            return
        }
        val url = "ws://$host:$port$WS_PATH"
        val builder = Request.Builder().url(url)

        if (jwt.isNotBlank()) {
            builder.addHeader("Authorization", "Bearer $jwt")
        }
        val request = builder.build()

        webSocket = wsClient.newWebSocket(request, object : WebSocketListener() {

            override fun onOpen(ws: WebSocket, response: Response) {
                isConnecting = false
                isConnected  = true
                Log.i(TAG, "WebSocket connected to $url")
            }

            override fun onMessage(ws: WebSocket, text: String) {
                parseAndEmit(text)
            }

            override fun onFailure(ws: WebSocket, t: Throwable, response: Response?) {

                if (webSocket == ws) webSocket = null
                isConnecting = false
                isConnected  = false
                Log.w(TAG, "WebSocket failure (attempt $attempt): ${t.message}")
                _events.tryEmit(WsEvent.Disconnected(t.message ?: "Unknown error"))
                scheduleReconnect(attempt)
            }

            override fun onClosed(ws: WebSocket, code: Int, reason: String) {
                if (webSocket == ws) webSocket = null
                isConnecting = false
                isConnected  = false
                Log.i(TAG, "WebSocket closed: $code $reason")
                _events.tryEmit(WsEvent.Disconnected(reason))

                if (shouldReconnect && code != 1000) scheduleReconnect(attempt)
            }
        })
    }

    private fun scheduleReconnect(attemptNumber: Int = 0) {
        if (!shouldReconnect) return
        reconnectJob?.cancel()
        reconnectJob = scope.launch {
            val delayMs = minOf(
                (BACKOFF_INITIAL_MS * Math.pow(BACKOFF_MULTIPLIER, attemptNumber.toDouble())).toLong(),
                BACKOFF_MAX_MS,
            )
            Log.i(TAG, "Reconnect attempt ${attemptNumber + 1} in ${delayMs}ms")
            delay(delayMs)
            if (shouldReconnect) openConnection(attemptNumber + 1)
        }
    }

    private fun parseAndEmit(text: String) {
        runCatching {
            val obj: JsonObject = json.parseToJsonElement(text).jsonObject
            val type = obj["type"]?.jsonPrimitive?.content ?: return

            val event: WsEvent = when (type) {
                "connected" -> WsEvent.Connected(
                    clientId = obj["client_id"]?.jsonPrimitive?.content ?: "",
                    server   = obj["server"]?.jsonPrimitive?.content ?: "",
                    version  = obj["version"]?.jsonPrimitive?.content ?: "",
                )
                "pong" -> WsEvent.Pong
                "chat_response" -> WsEvent.ChatResponse(
                    requestId = obj["request_id"]?.jsonPrimitive?.content ?: "",
                    response  = obj["response"]?.jsonPrimitive?.content ?: "",
                )
                "ipc_broadcast" -> {
                    val emotionObj = obj["emotion"]?.jsonObject
                    fun f(key: String) = emotionObj?.get(key)?.jsonPrimitive?.content?.toFloatOrNull()
                    WsEvent.EmotionUpdate(
                        emotion = EmotionsResponse(
                            valence      = f("valence"),
                            arousal      = f("arousal"),
                            dominance    = f("dominance"),
                            mood         = emotionObj?.get("mood")?.jsonPrimitive?.content,
                            joy          = f("joy"),
                            sadness      = f("sadness"),
                            anger        = f("anger"),
                            fear         = f("fear"),
                            disgust      = f("disgust"),
                            trust        = f("trust"),
                            surprise     = f("surprise"),
                            anticipation = f("anticipation"),
                        )
                    )
                }
                "hormone_tick" -> WsEvent.HormoneTick(
                    hormones = parseHormones(obj),
                    phase    = obj["phase"]?.jsonPrimitive?.content ?: "",
                )
                "phase_transition" -> WsEvent.PhaseTransition(
                    from = obj["from"]?.jsonPrimitive?.content ?: "",
                    to   = obj["to"]?.jsonPrimitive?.content ?: "",
                )
                "birth"       -> WsEvent.Birth
                "lh_surge"    -> WsEvent.LhSurge
                "labor_stage" -> WsEvent.LaborStage(obj["stage"]?.jsonPrimitive?.content ?: "")
                "miscarriage" -> WsEvent.Miscarriage
                else          -> WsEvent.Unknown(type, text)
            }

            _events.tryEmit(event)
        }.onFailure { e ->
            Log.e(TAG, "Failed to parse WS message: $text", e)
        }
    }

    private fun parseHormones(obj: JsonObject): XHormones {
        fun float(key: String) = obj[key]?.jsonPrimitive?.content?.toFloatOrNull() ?: 0f
        return XHormones(
            estradiol    = float("estradiol"),
            progesterone = float("progesterone"),
            testosterone = float("testosterone"),
            lh           = float("lh"),
            fsh          = float("fsh"),
            prolactin    = float("prolactin"),
            oxytocin     = float("oxytocin"),
            cortisol     = float("cortisol"),
            hcg          = float("hcg"),
        )
    }

    fun sendPing() {
        webSocket?.send("""{"type":"ping"}""")
    }

    fun sendChat(message: String, requestId: String, conversationId: Long = 1L): Boolean {
        val ws = webSocket ?: return false
        val encoded = kotlinx.serialization.json.Json.encodeToString(
            kotlinx.serialization.json.JsonPrimitive.serializer(),
            kotlinx.serialization.json.JsonPrimitive(message),
        )
        val payload = """{"type":"chat","message":$encoded,"request_id":"$requestId","conversation_id":$conversationId}"""
        return ws.send(payload)
    }

    fun subscribe(topic: String) {
        webSocket?.send("""{"type":"subscribe","topic":"$topic"}""")
    }
}
