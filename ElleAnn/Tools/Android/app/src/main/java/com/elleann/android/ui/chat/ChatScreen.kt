package com.elleann.android.ui.chat

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.content.ContextCompat
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.colorcode.*
import com.elleann.android.colorcode.TtsPlayState
import com.elleann.android.ui.chat.SttController
import com.elleann.android.ui.chat.SttState
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.ChatCacheManager
import com.elleann.android.data.ElleWebSocket
import com.elleann.android.data.WsEvent
import com.elleann.android.data.models.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

data class ChatState(
    val messages: List<Message>      = emptyList(),
    val inputText: String            = "",
    val loading: Boolean             = true,
    val cacheLoaded: Boolean         = false,
    val serverVerified: Boolean      = false,
    val sending: Boolean             = false,
    val error: String?               = null,
    val streamingResponse: String    = "",
    val streamingRequestId: String   = "",
    val colorCodeMode: ColorCodeMode = ColorCodeMode.OFF,
    val ttsWordIndex: Int            = -1,
    val ttsTargetMessageId: Long     = -1L,
    val pendingText: String          = "", // preserved on failed send
)

class ChatViewModel(
    private val conversationId: Long,
    private val container: AppContainerExtended,
    private val webSocket: ElleWebSocket,
    private val cacheManager: ChatCacheManager,
) : ViewModel() {

    private val _state = MutableStateFlow(ChatState())
    val state: StateFlow<ChatState> = _state.asStateFlow()

    init {
        // Track messages in memory as they change — but DON'T flush to disk here
        viewModelScope.launch {
            _state.collect { st ->
                cacheManager.track(conversationId, st.messages)
            }
        }
        loadCacheThenVerify()
        observeWebSocket()
    }

    override fun onCleared() {
        // Untrack when VM is destroyed — the app-level flush handles disk write
        cacheManager.untrack(conversationId)
        super.onCleared()
    }

    private fun loadCacheThenVerify() {
        viewModelScope.launch {
            // Load from disk first — fast, no network
            val cached = cacheManager.loadCache(conversationId)
            if (cached.isNotEmpty()) {
                _state.update { it.copy(messages = cached, cacheLoaded = true, loading = false) }
            }

            // Then verify against server in background
            val api = container.pairedExtendedApi() ?: run {
                _state.update { it.copy(loading = false, error = "Not signed in") }
                return@launch
            }
            when (val result = cacheManager.verifyWithServer(conversationId, cached, api)) {
                is ChatCacheManager.VerifyResult.Match -> {
                    _state.update { it.copy(serverVerified = true, loading = false) }
                }
                is ChatCacheManager.VerifyResult.Updated -> {
                    _state.update { it.copy(
                        messages       = result.messages,
                        serverVerified = true,
                        loading        = false,
                    )}
                    // Update tracked state but don't flush — will flush on close
                    cacheManager.track(conversationId, result.messages)
                }
                is ChatCacheManager.VerifyResult.Error -> {
                    _state.update { it.copy(serverVerified = false, loading = false) }
                }
            }
        }
    }

    private fun observeWebSocket() {
        viewModelScope.launch {
            webSocket.events.collect { event ->
                when (event) {
                    is WsEvent.ChatResponse -> {
                        if (event.requestId == _state.value.streamingRequestId) {
                            _state.update { it.copy(
                                streamingResponse = event.response,
                                sending = false,
                            )}
                            refreshMessages()
                        }
                    }
                    else -> Unit
                }
            }
        }
    }

    private fun refreshMessages() {
        viewModelScope.launch {
            runCatching {
                container.extendedApi.getMessages(conversationId)
            }.onSuccess { r ->
                _state.update { it.copy(messages = r.messages, streamingResponse = "") }
                // Keep in-memory state current — flush happens at app close/logout
                cacheManager.track(conversationId, r.messages)
            }
        }
    }

    fun onInput(text: String) = _state.update { it.copy(inputText = text, pendingText = "") }

    fun send() {
        val text = _state.value.inputText.trim()
        if (text.isBlank() || _state.value.sending) return
        val requestId = UUID.randomUUID().toString()

        val optimisticMsg = Message(
            messageId      = -System.nanoTime(),
            conversationId = conversationId,
            role           = 1,
            content        = text,
            timestampMs    = System.currentTimeMillis(),
        )
        val withOptimistic = _state.value.messages + optimisticMsg

        _state.update { it.copy(
            inputText          = "",
            pendingText        = "",
            sending            = true,
            streamingRequestId = requestId,
            streamingResponse  = "",
            messages           = withOptimistic,
        )}

        viewModelScope.launch {
            val sent = webSocket.sendChat(text, requestId, conversationId)
            if (!sent) {
                // Restore input text so user doesn't lose their message
                _state.update { s ->
                    s.copy(
                        messages    = s.messages.filterNot { it.messageId < 0 },
                        sending     = false,
                        inputText   = text, // put it back
                        pendingText = text,
                        error       = "Send failed — WebSocket not connected. Your message is restored.",
                    )
                }
            }
        }
    }

    fun cycleColorCode() {
        val modes = ColorCodeMode.entries
        val next = modes[(modes.indexOf(_state.value.colorCodeMode) + 1) % modes.size]
        _state.update { it.copy(colorCodeMode = next) }
    }

    fun playMessage(messageId: Long, text: String, tts: TtsController) {
        _state.update { it.copy(ttsTargetMessageId = messageId) }
        tts.speak(text)
    }

    fun pauseTts(tts: TtsController) {
        tts.pause()
    }

    fun stopTts(tts: TtsController) {
        tts.stop()
        _state.update { it.copy(ttsTargetMessageId = -1L, ttsWordIndex = -1) }
    }

    fun updateTtsWord(idx: Int) = _state.update { it.copy(ttsWordIndex = idx) }

    fun dismissError() = _state.update { it.copy(error = null) }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatScreen(
    conversationId: Long,
    containerExtended: AppContainerExtended,
    onBack: () -> Unit,
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val tts = remember { TtsController(context) }
    val stt = remember { SttController(context) }
    val sttState by stt.state.collectAsState()
    val appearancePrefs = remember {
        context.getSharedPreferences("elle_appearance", Context.MODE_PRIVATE)
    }

    val bubbleFontSize = remember {
        appearancePrefs.getInt("bubble_font_size", 14)
    }

    LaunchedEffect(Unit) {
        tts.speed = appearancePrefs.getFloat("tts_speed", 1.0f)
        tts.pitch = appearancePrefs.getFloat("tts_pitch", 1.0f)
    }

    val cacheManager = remember {
        (context.applicationContext as com.elleann.android.ElleApp).chatCacheManager
    }

    val ws = containerExtended.webSocketOrNull
    if (ws == null) {
        Box(
            modifier = Modifier.fillMaxSize().background(IsyaNight),
            contentAlignment = Alignment.Center,
        ) {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Text("Live connection not ready", color = IsyaMuted, style = MaterialTheme.typography.bodyMedium)
                Spacer(Modifier.height(8.dp))
                Text("Return to Elle home and wait for connection",
                    color = IsyaMuted.copy(alpha = 0.6f), style = MaterialTheme.typography.bodySmall)
            }
        }
        return
    }

    val vm: ChatViewModel = viewModel(
        key     = "chat_$conversationId",
        factory = object : ViewModelProvider.Factory {
            @Suppress("UNCHECKED_CAST")
            override fun <T : ViewModel> create(modelClass: Class<T>): T =
                ChatViewModel(conversationId, containerExtended, ws, cacheManager) as T
        }
    )

    DisposableEffect(lifecycleOwner) {
        val observer = object : DefaultLifecycleObserver {
            // DO NOT flush cache on pause/stop — only on explicit close/logout
            override fun onDestroy(owner: LifecycleOwner) { tts.shutdown(); stt.destroy() }
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
    }

    val state by vm.state.collectAsState()
    val ttsWord     by tts.currentWordIndex.collectAsState()
    val ttsPlayState by tts.playState.collectAsState()
    val listState = rememberLazyListState()

    fun beginSpeechInput() {
        stt.startListening { recognized ->
            val current = vm.state.value.inputText
            val appended = if (current.isBlank()) recognized else "$current $recognized"
            vm.onInput(appended)
        }
    }

    val micPermissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestPermission(),
    ) { granted ->
        if (granted) beginSpeechInput()
    }

    LaunchedEffect(ttsWord) { vm.updateTtsWord(ttsWord) }
    LaunchedEffect(state.messages.size, state.streamingResponse) {
        if (state.messages.isNotEmpty()) listState.animateScrollToItem(state.messages.size - 1)
    }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = {
                    Column {
                        Text("Conversation #$conversationId", color = IsyaCream, style = MaterialTheme.typography.titleSmall)
                        Text(
                            when {
                                state.sending        -> "Elle is responding…"
                                state.serverVerified -> "Synced with server"
                                state.cacheLoaded    -> "Loaded from cache"
                                else                 -> ""
                            },
                            style = MaterialTheme.typography.labelSmall,
                            color = when {
                                state.sending        -> IsyaMagic
                                state.serverVerified -> IsyaSuccess
                                else                 -> IsyaMuted
                            },
                        )
                    }
                },
                navigationIcon = {
                    IconButton(onClick = { onBack() }) {
                        Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted)
                    }
                },
                actions = {
                    IconButton(onClick = { vm.cycleColorCode() }) {
                        Icon(Icons.Rounded.Palette, "ColorCode", tint = when (state.colorCodeMode) {
                            ColorCodeMode.OFF      -> IsyaMuted
                            ColorCodeMode.GRAMMAR  -> IsyaMagic
                            ColorCodeMode.SEMANTIC -> IsyaGold
                            ColorCodeMode.MANUAL   -> ElleViolet
                            ColorCodeMode.CYCLING  -> IsyaGoldBright
                        })
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        },
        bottomBar = {
            Column {
                state.error?.let { err ->
                    Surface(color = Color(0xFF2A0D0D), modifier = Modifier.fillMaxWidth()) {
                        Row(
                            modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                            verticalAlignment = Alignment.CenterVertically,
                        ) {
                            Text(err, color = IsyaError, style = MaterialTheme.typography.bodySmall, modifier = Modifier.weight(1f))
                            IconButton(onClick = vm::dismissError, modifier = Modifier.size(24.dp)) {
                                Icon(Icons.Rounded.Close, "Dismiss", tint = IsyaError, modifier = Modifier.size(16.dp))
                            }
                        }
                    }
                }
                ChatInputBar(
                    text      = state.inputText,
                    onChange  = vm::onInput,
                    onSend    = vm::send,
                    sending   = state.sending,
                    sttState  = sttState,
                    onMic     = {
                        if (sttState == SttState.LISTENING) {
                            stt.stopListening()
                        } else if (ContextCompat.checkSelfPermission(
                                context,
                                Manifest.permission.RECORD_AUDIO,
                            ) == PackageManager.PERMISSION_GRANTED
                        ) {
                            beginSpeechInput()
                        } else {
                            micPermissionLauncher.launch(Manifest.permission.RECORD_AUDIO)
                        }
                    },
                )
            }
        }
    ) { padding ->
        if (state.loading && !state.cacheLoaded) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) { IsyaLoadingIndicator() }
            return@Scaffold
        }

        LazyColumn(
            state          = listState,
            modifier       = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(horizontal = 12.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            items(state.messages, key = { it.messageId }) { msg ->
                MessageBubble(
                    message         = msg,
                    colorCodeMode   = state.colorCodeMode,
                    ttsWordIndex    = if (msg.isElle && state.ttsTargetMessageId == msg.messageId)
                                          state.ttsWordIndex else -1,
                    ttsPlayState    = if (state.ttsTargetMessageId == msg.messageId)
                                          ttsPlayState else TtsPlayState.IDLE,
                    bubbleFontSize  = bubbleFontSize,
                    onPlay          = { vm.playMessage(msg.messageId, msg.content, tts) },
                    onPause         = { vm.pauseTts(tts) },
                    onStop          = { vm.stopTts(tts) },
                )
            }
            if (state.streamingResponse.isNotBlank()) {
                item {
                    StreamingBubble(text = state.streamingResponse, colorCodeMode = state.colorCodeMode)
                }
            }
        }
    }
}

@Composable
private fun MessageBubble(
    message: Message,
    colorCodeMode: ColorCodeMode,
    ttsWordIndex: Int,
    ttsPlayState: TtsPlayState,
    bubbleFontSize: Int = 14,
    onPlay: () -> Unit,
    onPause: () -> Unit,
    onStop: () -> Unit,
) {
    val isUser = message.isUser
    val timeStr = remember(message.timestampMs) {
        SimpleDateFormat("h:mm a", Locale.getDefault()).format(Date(message.timestampMs))
    }
    val clipboardManager = androidx.compose.ui.platform.LocalClipboardManager.current

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = if (isUser) Arrangement.End else Arrangement.Start,
    ) {
        Column(
            modifier = Modifier.widthIn(max = 300.dp),
            horizontalAlignment = if (isUser) Alignment.End else Alignment.Start,
        ) {
            // Message bubble
            Box(
                modifier = Modifier
                    .background(
                        if (isUser) IsyaMist else IsyaDusk,
                        RoundedCornerShape(
                            topStart    = if (isUser) 16.dp else 4.dp,
                            topEnd      = if (isUser) 4.dp else 16.dp,
                            bottomStart = 16.dp, bottomEnd = 16.dp,
                        )
                    )
                    .padding(horizontal = 14.dp, vertical = 10.dp)
            ) {
                if (colorCodeMode == ColorCodeMode.OFF || isUser) {
                    Text(message.content, style = MaterialTheme.typography.bodyMedium.copy(fontSize = bubbleFontSize.sp), color = IsyaCream)
                } else {
                    ColorCodedText(message.content, colorCodeMode, ttsWordIndex, bubbleFontSize)
                }
            }

            // Timestamp
            Text(
                timeStr,
                style = MaterialTheme.typography.labelSmall,
                color = IsyaMuted,
                modifier = Modifier.padding(horizontal = 4.dp, vertical = 2.dp),
            )

            // Action bar — Copy always shown, Play/Pause/Stop on Elle messages only
            Row(
                modifier = Modifier.padding(horizontal = 2.dp, vertical = 0.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(0.dp),
            ) {
                // Copy — both user and Elle messages
                IconButton(
                    onClick = {
                        clipboardManager.setText(
                            androidx.compose.ui.text.AnnotatedString(message.content)
                        )
                    },
                    modifier = Modifier.size(32.dp),
                ) {
                    Icon(Icons.Rounded.ContentCopy, "Copy",
                        tint = IsyaMuted, modifier = Modifier.size(16.dp))
                }

                // Play / Pause / Stop — Elle messages only
                if (!isUser) {
                    // Play (or Resume if paused)
                    IconButton(
                        onClick  = onPlay,
                        enabled  = ttsPlayState != TtsPlayState.PLAYING,
                        modifier = Modifier.size(32.dp),
                    ) {
                        Icon(
                            Icons.Rounded.PlayArrow, "Play",
                            tint = if (ttsPlayState == TtsPlayState.PLAYING) IsyaMuted
                                   else IsyaMagic,
                            modifier = Modifier.size(16.dp),
                        )
                    }
                    // Pause
                    IconButton(
                        onClick  = onPause,
                        enabled  = ttsPlayState == TtsPlayState.PLAYING,
                        modifier = Modifier.size(32.dp),
                    ) {
                        Icon(
                            Icons.Rounded.Pause, "Pause",
                            tint = if (ttsPlayState == TtsPlayState.PLAYING) IsyaGold
                                   else IsyaMuted,
                            modifier = Modifier.size(16.dp),
                        )
                    }
                    // Stop
                    IconButton(
                        onClick  = onStop,
                        enabled  = ttsPlayState != TtsPlayState.IDLE,
                        modifier = Modifier.size(32.dp),
                    ) {
                        Icon(
                            Icons.Rounded.Stop, "Stop",
                            tint = if (ttsPlayState != TtsPlayState.IDLE) IsyaError
                                   else IsyaMuted,
                            modifier = Modifier.size(16.dp),
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun StreamingBubble(text: String, colorCodeMode: ColorCodeMode) {
    Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.Start) {
        Box(
            modifier = Modifier
                .widthIn(max = 300.dp)
                .background(IsyaDusk, RoundedCornerShape(4.dp, 16.dp, 16.dp, 16.dp))
                .padding(horizontal = 14.dp, vertical = 10.dp)
        ) {
            if (colorCodeMode == ColorCodeMode.OFF) {
                Text(text, style = MaterialTheme.typography.bodyMedium, color = IsyaCream.copy(alpha = 0.7f))
            } else {
                ColorCodedText(text, colorCodeMode, -1)
            }
        }
    }
}

@Composable
private fun ColorCodedText(text: String, mode: ColorCodeMode, ttsWordIndex: Int, fontSize: Int = 14) {
    val tokens = remember(text, mode) { ColorCodeEngine.tokenize(text, mode) }
    var wordIdx = 0
    val annotated = buildAnnotatedString {
        tokens.forEach { token ->
            if (token.isWhitespace) { append(token.word) }
            else {
                val i = wordIdx++
                val color = if (ttsWordIndex == i) IsyaGoldBright else token.color
                withStyle(SpanStyle(color = color, fontSize = fontSize.sp)) { append(token.word) }
            }
        }
    }
    Text(annotated)
}

@Composable
private fun ChatInputBar(
    text: String,
    onChange: (String) -> Unit,
    onSend: () -> Unit,
    sending: Boolean,
    sttState: SttState = SttState.IDLE,
    onMic: () -> Unit = {},
) {
    Surface(color = IsyaHeader, tonalElevation = 8.dp) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp, vertical = 8.dp)
                .navigationBarsPadding()
                .imePadding(),
            verticalAlignment = Alignment.Bottom,
        ) {
            // Microphone button — standard Android mic icon
            IconButton(
                onClick  = onMic,
                modifier = Modifier.size(48.dp),
            ) {
                Icon(
                    imageVector = if (sttState == SttState.LISTENING)
                        Icons.Rounded.MicOff else Icons.Rounded.Mic,
                    contentDescription = if (sttState == SttState.LISTENING) "Stop" else "Speak",
                    tint = when (sttState) {
                        SttState.LISTENING -> IsyaError       // red while recording
                        SttState.ERROR     -> IsyaWarn
                        SttState.IDLE      -> IsyaMuted
                    },
                )
            }
            OutlinedTextField(
                value = text, onValueChange = onChange,
                placeholder = { Text("Say something to Elle…", color = IsyaMuted, fontSize = 14.sp) },
                modifier = Modifier.weight(1f),
                maxLines = 5,
                shape = RoundedCornerShape(20.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedTextColor = IsyaCream, unfocusedTextColor = IsyaCream,
                    focusedBorderColor = if (sttState == SttState.LISTENING) IsyaError else IsyaMagic,
                    unfocusedBorderColor = IsyaMist,
                    focusedContainerColor = IsyaDusk, unfocusedContainerColor = IsyaDusk,
                    cursorColor = IsyaMagicBright,
                ),
            )
            Spacer(Modifier.width(8.dp))
            IconButton(
                onClick  = onSend,
                enabled  = text.isNotBlank() && !sending,
                modifier = Modifier
                    .size(48.dp)
                    .background(
                        if (text.isNotBlank() && !sending) Color(0xFF1A8A9A) else IsyaMist,
                        RoundedCornerShape(24.dp),
                    ),
            ) {
                if (sending) CircularProgressIndicator(Modifier.size(20.dp), strokeWidth = 2.dp, color = IsyaCream)
                else Icon(Icons.Rounded.Send, "Send", tint = IsyaCream)
            }
        }
    }
}
