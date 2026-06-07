package com.elleann.android.ui.chat

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
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.colorcode.*
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

        viewModelScope.launch {
            _state.collect { st ->
                cacheManager.track(conversationId, st.messages)
            }
        }
        loadCacheThenVerify()
        observeWebSocket()
    }

    override fun onCleared() {

        cacheManager.writeCacheSync(conversationId, _state.value.messages)
        cacheManager.untrack(conversationId)
        super.onCleared()
    }

    private fun loadCacheThenVerify() {
        viewModelScope.launch {

            val cached = cacheManager.loadCache(conversationId)
            if (cached.isNotEmpty()) {
                _state.update { it.copy(messages = cached, cacheLoaded = true, loading = false) }
            }

            val api = container.pairedExtendedApi() ?: run {
                _state.update { it.copy(loading = false, error = "Not paired") }
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
                    cacheManager.writeCache(conversationId, result.messages)
                }
                is ChatCacheManager.VerifyResult.Error -> {

                    _state.update { it.copy(serverVerified = false, loading = false) }
                }
            }
        }
    }

    fun flushCache() {
        viewModelScope.launch {
            cacheManager.writeCache(conversationId, _state.value.messages)
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
                cacheManager.writeCache(conversationId, r.messages)
            }
        }
    }

    fun onInput(text: String) = _state.update { it.copy(inputText = text) }

    fun send() {
        val text = _state.value.inputText.trim()
        if (text.isBlank() || _state.value.sending) return
        val requestId = java.util.UUID.randomUUID().toString()

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
            sending            = true,
            streamingRequestId = requestId,
            streamingResponse  = "",
            messages           = withOptimistic,
        )}

        viewModelScope.launch {
            cacheManager.writeAfterMessage(conversationId, withOptimistic)
            val sent = webSocket.sendChat(text, requestId, conversationId)
            if (!sent) {

                _state.update { s ->
                    s.copy(
                        messages  = s.messages.filterNot { it.messageId < 0 },
                        sending   = false,
                        error     = "Message failed to send — WebSocket not connected.",
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

    fun toggleTts(text: String, tts: TtsController) {
        if (_state.value.ttsWordIndex >= 0) {
            tts.stop(); _state.update { it.copy(ttsWordIndex = -1) }
        } else {
            tts.speak(text)
        }
    }

    fun updateTtsWord(idx: Int) = _state.update { it.copy(ttsWordIndex = idx) }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatScreen(
    conversationId: Long,
    containerExtended: AppContainerExtended,
    onBack: () -> Unit,
    onStartVideoCall: (Long) -> Unit,
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val tts = remember { TtsController(context) }

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
                Text("Return to Elle home and wait for WebSocket to connect",
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
            override fun onPause(owner: LifecycleOwner)  { vm.flushCache() }
            override fun onStop(owner: LifecycleOwner)   { vm.flushCache() }
            override fun onDestroy(owner: LifecycleOwner) { tts.shutdown() }
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
    }

    val state by vm.state.collectAsState()
    val ttsWord by tts.currentWordIndex.collectAsState()
    val listState = rememberLazyListState()

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
                    IconButton(onClick = { vm.flushCache(); onBack() }) {
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
            ChatInputBar(
                text    = state.inputText,
                onChange = vm::onInput,
                onSend  = vm::send,
                sending = state.sending,
            )
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
                    message       = msg,
                    colorCodeMode = state.colorCodeMode,
                    ttsWordIndex  = if (msg.isElle) state.ttsWordIndex else -1,
                    onTts         = { vm.toggleTts(msg.content, tts) },
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
    onTts: () -> Unit,
) {
    val isUser = message.isUser
    val timeStr = remember(message.timestampMs) {
        SimpleDateFormat("h:mm a", Locale.getDefault()).format(Date(message.timestampMs))
    }

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = if (isUser) Arrangement.End else Arrangement.Start,
    ) {
        Column(
            modifier = Modifier.widthIn(max = 300.dp),
            horizontalAlignment = if (isUser) Alignment.End else Alignment.Start,
        ) {
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
                    Text(message.content, style = MaterialTheme.typography.bodyMedium, color = IsyaCream)
                } else {
                    ColorCodedText(message.content, colorCodeMode, ttsWordIndex)
                }
            }
            Row(
                modifier = Modifier.padding(horizontal = 4.dp, vertical = 2.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(4.dp),
            ) {
                Text(timeStr, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                if (!isUser) {
                    IconButton(onClick = onTts, modifier = Modifier.size(20.dp)) {
                        Icon(Icons.Rounded.VolumeUp, "Speak", tint = IsyaMuted, modifier = Modifier.size(14.dp))
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
private fun ColorCodedText(text: String, mode: ColorCodeMode, ttsWordIndex: Int) {
    val tokens = remember(text, mode) { ColorCodeEngine.tokenize(text, mode) }
    var wordIdx = 0
    val annotated = buildAnnotatedString {
        tokens.forEach { token ->
            if (token.isWhitespace) { append(token.word) }
            else {
                val i = wordIdx++
                val color = if (ttsWordIndex == i) IsyaGoldBright else token.color
                withStyle(SpanStyle(color = color, fontSize = 14.sp)) { append(token.word) }
            }
        }
    }
    Text(annotated)
}

@Composable
private fun ChatInputBar(text: String, onChange: (String) -> Unit, onSend: () -> Unit, sending: Boolean) {
    Surface(color = IsyaHeader, tonalElevation = 8.dp) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp, vertical = 8.dp)
                .navigationBarsPadding()
                .imePadding(),
            verticalAlignment = Alignment.Bottom,
        ) {
            OutlinedTextField(
                value = text, onValueChange = onChange,
                placeholder = { Text("Say something to Elle…", color = IsyaMuted, fontSize = 14.sp) },
                modifier = Modifier.weight(1f),
                maxLines = 5,
                shape = RoundedCornerShape(20.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedTextColor = IsyaCream, unfocusedTextColor = IsyaCream,
                    focusedBorderColor = IsyaMagic, unfocusedBorderColor = IsyaMist,
                    focusedContainerColor = IsyaDusk, unfocusedContainerColor = IsyaDusk,
                    cursorColor = IsyaMagicBright,
                ),
            )
            Spacer(Modifier.width(8.dp))
            IconButton(
                onClick = onSend,
                enabled = text.isNotBlank() && !sending,
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
