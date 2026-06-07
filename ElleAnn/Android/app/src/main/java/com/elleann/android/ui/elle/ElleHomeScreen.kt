package com.elleann.android.ui.elle

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.ElleWebSocket
import com.elleann.android.data.WsEvent
import com.elleann.android.EmotionsResponse
import com.elleann.android.data.models.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.components.IsyaTopBar
import com.elleann.android.ui.components.IsyaHudRow
import com.elleann.android.ui.components.IsyaHudStat
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

data class ElleHomeState(
    val greeting: SessionGreeting? = null,
    val emotions: EmotionsResponse? = null,
    val aiStatus: AiStatus? = null,
    val selfImage: SelfImageResponse? = null,
    val wordOfDay: DictionaryWord? = null,
    val brainStatus: String = "",
    val halStatus: String = "",

    val me: MeResponse? = null,

    val recap: RecapResponse? = null,
    val connectionState: ConnectionState = ConnectionState.DISCONNECTED,
    val loading: Boolean = true,
    val error: String? = null,
)

class ElleHomeViewModel(
    private val containerExtended: AppContainerExtended,
    private val webSocket: ElleWebSocket,
) : ViewModel() {

    private val _state = MutableStateFlow(ElleHomeState())
    val state: StateFlow<ElleHomeState> = _state.asStateFlow()

    init {
        load()
        observeWebSocket()
        startEmotionPolling()
        startHealthPolling()
    }

    private fun load() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            val api = containerExtended.pairedExtendedApi() ?: run {
                _state.update { it.copy(loading = false, error = "Not paired") }
                return@launch
            }

            val greetingJob = launch {
                runCatching { api.getSessionGreeting() }
                    .onSuccess { g ->
                        _state.update { it.copy(greeting = g) }

                        runCatching { api.ackSessionGreeting(g.id) }
                    }
            }

            val meJob = launch {
                runCatching { api.getMe() }
                    .onSuccess { m -> _state.update { it.copy(me = m) } }
            }
            val recapJob = launch {

                runCatching { api.getRecap() }
                    .onSuccess { r -> _state.update { it.copy(recap = r) } }
            }
            val aiJob = launch {
                runCatching { api.getAiStatus() }
                    .onSuccess { s -> _state.update { it.copy(aiStatus = s) } }
            }
            val selfImageJob = launch {
                runCatching { api.getSelfImage() }
                    .onSuccess { s -> _state.update { it.copy(selfImage = s) } }
            }
            val wordJob = launch {
                runCatching { api.getRandomWord() }
                    .onSuccess { w -> _state.update { it.copy(wordOfDay = w) } }
            }
            val brainJob = launch {
                runCatching { api.getBrainStatus() }
                    .onSuccess { b -> _state.update { it.copy(brainStatus = b["status"] ?: "") } }
            }
            val halJob = launch {
                runCatching { api.getHalStatus() }
                    .onSuccess { h -> _state.update { it.copy(halStatus = h.hardware) } }
            }

            greetingJob.join(); aiJob.join(); selfImageJob.join()
            wordJob.join(); brainJob.join(); halJob.join()

            _state.update { it.copy(loading = false) }
        }
    }

    private fun observeWebSocket() {
        viewModelScope.launch {
            webSocket.events.collect { event ->
                when (event) {
                    is WsEvent.Connected    -> _state.update { it.copy(connectionState = ConnectionState.CONNECTED) }
                    is WsEvent.EmotionUpdate -> _state.update { it.copy(emotions = event.emotion) }
                    is WsEvent.Disconnected -> _state.update { it.copy(connectionState = ConnectionState.DISCONNECTED) }
                    else -> Unit
                }
            }
        }
    }

    private fun startEmotionPolling() {
        viewModelScope.launch {
            while (isActive) {
                delay(5_000)
                runCatching { containerExtended.extendedApi.getEmotionDimensions() }
                    .onSuccess { dims ->

                        val dimMap = dims.dimensions.associate { it.name to it.value }
                        val current = _state.value.emotions
                        _state.update { it.copy(emotions = com.elleann.android.EmotionsResponse(
                            valence      = dimMap["valence"]      ?: current?.valence,
                            arousal      = dimMap["arousal"]      ?: current?.arousal,
                            dominance    = dimMap["dominance"]    ?: current?.dominance,
                            mood         = current?.mood,
                            joy          = dimMap["joy"]          ?: current?.joy,
                            sadness      = dimMap["sadness"]      ?: current?.sadness,
                            anger        = dimMap["anger"]        ?: current?.anger,
                            fear         = dimMap["fear"]         ?: current?.fear,
                            disgust      = dimMap["disgust"]      ?: current?.disgust,
                            trust        = dimMap["trust"]        ?: current?.trust,
                            surprise     = dimMap["surprise"]     ?: current?.surprise,
                            anticipation = dimMap["anticipation"] ?: current?.anticipation,
                        ))}
                    }
            }
        }
    }

    private fun startHealthPolling() {
        viewModelScope.launch {
            while (isActive) {
                delay(5_000)
                val start = System.currentTimeMillis()
                val ok = runCatching {
                    containerExtended.extendedApi.getBrainStatus()
                }.isSuccess
                val elapsed = System.currentTimeMillis() - start
                _state.update {
                    it.copy(
                        connectionState = when {
                            !ok     -> ConnectionState.DISCONNECTED
                            elapsed > 1000 -> ConnectionState.SLOW
                            else    -> ConnectionState.CONNECTED
                        }
                    )
                }
            }
        }
    }

    fun retry() = load()
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ElleHomeScreen(
    containerExtended: AppContainerExtended,
    webSocket: ElleWebSocket,
    onSettings: () -> Unit,
    onUnpair: () -> Unit,
) {
    val vm: ElleHomeViewModel = viewModel(
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(modelClass: Class<T>): T =
                    ElleHomeViewModel(containerExtended, webSocket) as T
            }
        }
    )

    val state by vm.state.collectAsState()
    val stored = containerExtended.tokenStore.load()
    val serverLabel = stored?.let { "${it.host}:${it.port}" } ?: "—"

    Scaffold(
        containerColor = IsyaNight,
        topBar = {

            IsyaTopBar(
                title = {
                    Text("ElleAnn", style = MaterialTheme.typography.titleMedium,
                        color = IsyaGold, fontWeight = androidx.compose.ui.text.font.FontWeight.Bold)
                },
                subtitle = {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        ConnectionDot(state.connectionState)
                        Spacer(Modifier.width(6.dp))

                        val ident = state.me?.let { "${it.username} · #${it.userId}" } ?: serverLabel
                        Text(ident, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                    }
                },
                actions = {
                    androidx.compose.material3.IconButton(onClick = onSettings) {
                        Icon(Icons.Rounded.Settings, "Settings", tint = IsyaMuted)
                    }
                },
            )
        }
    ) { padding ->
        if (state.loading) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                IsyaLoadingIndicator()
            }
            return@Scaffold
        }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .verticalScroll(rememberScrollState())
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {

            state.recap?.let { r -> RecapStrip(r) }

            HealthBanner(containerExtended)

            state.greeting?.let { greeting ->
                IsyaPanel(title = "✦ Elle speaks", flowingBorder = true) {
                    Text(
                        text      = greeting.greeting,
                        style     = MaterialTheme.typography.bodyMedium,
                        color     = IsyaCream,
                        fontStyle = FontStyle.Italic,
                    )
                }
            }

            IsyaPanel(title = "EMOTIONAL PULSE", flowingBorder = true) {
                state.emotions?.let { e ->
                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        e.valence?.let { EmotionBar("Valence", it, color = IsyaMagic) }
                        e.arousal?.let { EmotionBar("Arousal", it, color = ElleViolet) }
                        e.dominance?.let { EmotionBar("Dominance", it, color = IsyaGold) }
                        e.mood?.let { mood ->
                            Spacer(Modifier.height(4.dp))
                            Text(
                                text  = "Mood: $mood",
                                style = MaterialTheme.typography.bodySmall,
                                color = IsyaParchment,
                            )
                        }
                    }
                } ?: Text("Loading emotions…", color = IsyaMuted)
            }

            state.selfImage?.let { si ->
                if (si.description.isNotBlank()) {
                    IsyaPanel(title = "ELLE SEES HERSELF AS") {
                        Text(
                            text      = si.description,
                            style     = MaterialTheme.typography.bodySmall,
                            color = IsyaParchment,
                            fontStyle = FontStyle.Italic,
                        )
                    }
                }
            }

            state.wordOfDay?.let { word ->
                IsyaPanel(title = "✦ WORD OF ISYA") {
                    Text(
                        text       = word.word,
                        style      = MaterialTheme.typography.titleMedium,
                        color      = IsyaGold,
                        fontStyle  = FontStyle.Normal,
                    )
                    Spacer(Modifier.height(4.dp))
                    word.partOfSpeech?.let {
                        Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                    }
                    word.definition?.let {
                        Text(it, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                    }
                }
            }

            IsyaPanel(title = "SYSTEM") {
                state.aiStatus?.let { ai ->
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        StatusRow("AI", ai.modelStatus, IsyaSuccess)
                        StatusRow("Model", ai.modelName, IsyaCream)
                        StatusRow("Brain", state.brainStatus.ifBlank { "active" }, IsyaSuccess)
                        StatusRow("HAL", state.halStatus.ifBlank { "nominal" }, IsyaSuccess)
                    }
                } ?: Text("Loading status…", color = IsyaMuted)
            }

            state.error?.let { err ->
                IsyaErrorState(err, onRetry = { vm.retry() })
            }
        }
    }
}

@Composable
private fun StatusRow(label: String, value: String, valueColor: Color) {
    Row(
        modifier              = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Text(label, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        Text(value, style = MaterialTheme.typography.bodySmall, color = valueColor)
    }
}

@Composable
private fun RecapStrip(r: com.elleann.android.data.models.RecapResponse) {
    val items = mutableListOf<String>()
    if (r.quietMinutes > 5) {
        val human = when {
            r.quietMinutes > 60 * 24 -> "${r.quietMinutes / (60 * 24)}d quiet"
            r.quietMinutes > 60      -> "${r.quietMinutes / 60}h quiet"
            else                     -> "${r.quietMinutes}m quiet"
        }
        items += human
    }
    if (r.lastMemorySummary.isNotBlank()) {
        val s = r.lastMemorySummary.take(80) + if (r.lastMemorySummary.length > 80) "…" else ""
        items += "last memory: $s"
    }
    val d = r.emotionValenceDelta
    if (kotlin.math.abs(d) > 0.05f) {
        val arrow = if (d > 0) "↑" else "↓"
        items += "mood $arrow ${"%.2f".format(kotlin.math.abs(d))}"
    }
    if (r.pendingIntents > 0) items += "${r.pendingIntents} pending intent${if (r.pendingIntents == 1L) "" else "s"}"
    if (r.topThread.isNotBlank()) items += "open thread: ${r.topThread.take(60)}"
    if (items.isEmpty()) return

    androidx.compose.material3.Card(
        colors = androidx.compose.material3.CardDefaults.cardColors(containerColor = IsyaSlot),
        modifier = Modifier.fillMaxWidth(),
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(
                text = "WHILE YOU WERE AWAY",
                style = MaterialTheme.typography.labelSmall,
                color = IsyaGold.copy(alpha = 0.7f),
            )
            Spacer(Modifier.height(6.dp))
            items.forEach { line ->
                Text(
                    text = "·  $line",
                    style = MaterialTheme.typography.bodySmall,
                    color = IsyaCream.copy(alpha = 0.85f),
                    modifier = Modifier.padding(vertical = 1.dp),
                )
            }
        }
    }
}

@Composable
private fun HealthBanner(container: AppContainerExtended) {
    var health by remember { mutableStateOf<com.elleann.android.data.models.DiagHealthResponse?>(null) }
    LaunchedEffect(Unit) {
        while (true) {

            val api = container.adminApi ?: return@LaunchedEffect
            runCatching { api.getDiagHealth() }.onSuccess { health = it }
            delay(30_000)
        }
    }

    val h = health ?: return
    val llmDown = !h.llm.healthy
    val hasIssues = h.issues.isNotEmpty()
    if (!llmDown && !hasIssues) return

    val severe = llmDown
    val bg = if (severe) Color(0xFF3A0D0DuL)
             else        Color(0xFF3A2A0DuL)
    val accent = if (severe) Color(0xFFFF6F6FuL)
                 else        IsyaGold

    androidx.compose.material3.Surface(
        shape = androidx.compose.foundation.shape.RoundedCornerShape(10.dp),
        color = bg,
        modifier = Modifier.fillMaxWidth(),
    ) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp),
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    if (llmDown) "Elle's brain is offline" else "System needs attention",
                    color = accent,
                    fontWeight = androidx.compose.ui.text.font.FontWeight.Bold,
                    style = MaterialTheme.typography.labelLarge,
                )
            }
            if (llmDown) {
                Text(
                    "LLM provider \"${h.llm.provider.ifBlank { "—" }}\" is not responding. " +
                        "Add an api_key in elle_master_config.json or wait for the fallback to come up.",
                    color = Color(0xFFFFE8B0uL),
                    style = MaterialTheme.typography.bodySmall,
                )
            }
            h.issues.forEach { iss ->
                Text(
                    "·  $iss",
                    color = Color(0xFFFFE8B0uL),
                    style = MaterialTheme.typography.bodySmall,
                )
            }
        }
    }
}
