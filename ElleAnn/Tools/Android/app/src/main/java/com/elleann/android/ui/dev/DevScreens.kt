package com.elleann.android.ui.dev

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import kotlinx.coroutines.withContext
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.ElleApiExtended
import com.elleann.android.data.models.*
import com.elleann.android.navigation.ElleRoutes
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

private fun AppContainerExtended.api(admin: Boolean = false): ElleApiExtended =
    if (admin) (adminApi ?: extendedApi) else extendedApi

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun DevScaffold(title: String, onBack: () -> Unit, content: @Composable (PaddingValues) -> Unit) {
    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text(title, color = Color(0xFF00FF88)) },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = Color(0xFF0A1A0A)),
            )
        },
        content = content,
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DevPinScreen(
    containerExtended: AppContainerExtended,
    onUnlocked: () -> Unit,
    onBack: () -> Unit,
) {
    var pin by remember { mutableStateOf("") }
    var error by remember { mutableStateOf(false) }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("Dev Access", color = IsyaError) },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(32.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center,
        ) {
            Icon(Icons.Rounded.Lock, null, tint = IsyaError, modifier = Modifier.size(48.dp))
            Spacer(Modifier.height(24.dp))
            Text("Enter Dev PIN", style = MaterialTheme.typography.titleMedium, color = IsyaCream)
            Spacer(Modifier.height(24.dp))
            OutlinedTextField(
                value = pin,
                onValueChange = { if (it.length <= 8) pin = it },
                label = { Text("PIN", color = IsyaMuted) },
                visualTransformation = PasswordVisualTransformation(),
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.NumberPassword),
                isError = error,
                modifier = Modifier.fillMaxWidth(),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedTextColor = IsyaCream, unfocusedTextColor = IsyaCream,
                    focusedBorderColor = IsyaError, unfocusedBorderColor = IsyaMist,
                    focusedContainerColor = IsyaDusk, unfocusedContainerColor = IsyaDusk,
                    cursorColor = IsyaError,
                ),
            )
            if (error) {
                Spacer(Modifier.height(8.dp))
                Text("Incorrect PIN", color = IsyaError, style = MaterialTheme.typography.bodySmall)
            }
            Spacer(Modifier.height(24.dp))
            IsyaButton(
                text    = "Unlock Dev",
                onClick = {
                    if (!containerExtended.hasDevPin() || containerExtended.verifyDevPin(pin)) {
                        onUnlocked()
                    } else {
                        error = true; pin = ""
                    }
                },
                variant  = IsyaButtonVariant.DANGER,
                modifier = Modifier.fillMaxWidth(),
            )
        }
    }
}

data class DevSection(val label: String, val subtitle: String, val route: String)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DevDashboardScreen(
    containerExtended: AppContainerExtended,
    onNavigate: (String) -> Unit,
    onSettings: () -> Unit,
) {
    var serverStatus by remember { mutableStateOf<ServerStatus?>(null) }
    LaunchedEffect(Unit) {
        runCatching { containerExtended.extendedApi.getServerStatus() }.onSuccess { serverStatus = it }
    }

    val sections = listOf(
        DevSection("Dashboard",     "Server status, analytics",         ElleRoutes.DEV_DASHBOARD),
        DevSection("System Health", "LLM, IPC wires, heartbeats, queues", ElleRoutes.DEV_HEALTH),
        DevSection("Logs",          "Console log stream",               ElleRoutes.DEV_LOGS),
        DevSection("Services",      "19 Windows service status",        ElleRoutes.DEV_SERVICES),
        DevSection("Diagnostics",   "IPC queues, route manifest",       ElleRoutes.DEV_DIAGNOSTICS),
        DevSection("API Explorer",  "Live REST endpoint browser",       ElleRoutes.DEV_API_EXPLORER),
        DevSection("Hardware",      "HAL actions queue",                ElleRoutes.DEV_HARDWARE),
        DevSection("Models",        "Slots, workers, token cache",      ElleRoutes.DEV_MODELS),
        DevSection("Agents",        "Agent personas",                   ElleRoutes.DEV_AGENTS),
        DevSection("Tools",         "Registered AI tools",              ElleRoutes.DEV_TOOLS),
        DevSection("Dictionary",    "Word database, load status",       ElleRoutes.DEV_DICTIONARY),
        DevSection("Memory Admin",  "Create/delete memories",          ElleRoutes.DEV_MEMORY_ADMIN),
        DevSection("Backups",       "SQL backups, trigger backup",      ElleRoutes.DEV_BACKUPS),
        DevSection("Config",        "Server settings",                  ElleRoutes.DEV_CONFIG),
        DevSection("Paired Devices","Revoke, generate pair code",      ElleRoutes.DEV_DEVICES),
        DevSection("Video Workers", "Job queue, worker status",        ElleRoutes.DEV_VIDEO_WORKERS),
        DevSection("Learning Admin","Add subjects, skills",             ElleRoutes.DEV_LEARNING_ADMIN),
        DevSection("Ethics Admin",  "Add / remove moral rules",         ElleRoutes.DEV_ETHICS_ADMIN),
        DevSection("SHN Editor",    "Edit Fiesta .shn data tables",     ElleRoutes.DEV_SHN_EDITOR),
    )

    Scaffold(
        containerColor = Color(0xFF0A1A0A),
        topBar = {
            TopAppBar(
                title = { Text("Dev Panel  ⚡", color = Color(0xFF00FF88)) },
                actions = {
                    IconButton(onClick = onSettings) { Icon(Icons.Rounded.Settings, null, tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = Color(0xFF0A1A0A)),
            )
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(6.dp),
        ) {

            item {
                serverStatus?.let { ss ->
                    Surface(shape = RoundedCornerShape(10.dp), color = Color(0xFF0D2B0D)) {
                        Row(modifier = Modifier.fillMaxWidth().padding(12.dp), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Uptime: ${ss.uptimeMs / 1000}s", style = MaterialTheme.typography.labelSmall, color = Color(0xFF00FF88), fontFamily = FontFamily.Monospace)
                            Text("Req: ${ss.requestCount}", style = MaterialTheme.typography.labelSmall, color = Color(0xFF00FF88), fontFamily = FontFamily.Monospace)
                            Text("Err: ${ss.errorCount}", style = MaterialTheme.typography.labelSmall, color = if (ss.errorCount > 0) IsyaError else Color(0xFF00FF88), fontFamily = FontFamily.Monospace)
                            Text("WS: ${ss.activeWs}", style = MaterialTheme.typography.labelSmall, color = Color(0xFF00FF88), fontFamily = FontFamily.Monospace)
                        }
                    }
                }
            }
            items(sections.drop(1)) { sec ->
                Surface(
                    modifier = Modifier.fillMaxWidth().clickable { onNavigate(sec.route) },
                    shape = RoundedCornerShape(8.dp),
                    color = Color(0xFF0D2B0D),
                ) {
                    Row(modifier = Modifier.padding(14.dp), verticalAlignment = Alignment.CenterVertically) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(sec.label, style = MaterialTheme.typography.bodySmall, color = Color(0xFF00FF88), fontWeight = FontWeight.Medium)
                            Text(sec.subtitle, style = MaterialTheme.typography.labelSmall, color = Color(0xFF3A7A3A))
                        }
                        Icon(Icons.Rounded.ChevronRight, null, tint = Color(0xFF3A7A3A))
                    }
                }
            }
        }
    }
}

@Composable
fun LogMonitorScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var logs by remember { mutableStateOf<List<LogEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var levelFilter by remember { mutableStateOf<String?>(null) }

    fun reload() {
        loading = true

    }

    LaunchedEffect(levelFilter) {
        loading = true
        runCatching { container.extendedApi.getConsoleLogs(200, levelFilter) }.onSuccess { logs = it.logs }
        loading = false
    }

    DevScaffold("Log Monitor", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            Row(modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp, vertical = 4.dp), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                listOf(null, "INFO", "WARN", "ERROR").forEach { level ->
                    FilterChip(
                        selected = levelFilter == level,
                        onClick = { levelFilter = level },
                        label = { Text(level ?: "ALL", fontSize = 11.sp) },
                        colors = FilterChipDefaults.filterChipColors(
                            selectedContainerColor = Color(0xFF00FF88),
                            selectedLabelColor = Color.Black,
                            containerColor = Color(0xFF0D2B0D),
                            labelColor = Color(0xFF00AA55),
                        ),
                    )
                }
            }
            if (loading) IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp))
            else LazyColumn(modifier = Modifier.fillMaxSize(), contentPadding = PaddingValues(8.dp), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                items(logs, key = { it.id }) { log ->
                    val levelColor = when (log.level) { 4 -> IsyaError; 3 -> IsyaWarn; 2 -> Color(0xFF00FF88); 1 -> Color(0xFF6699CC); else -> Color(0xFF3A7A3A) }
                    Row(modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 3.dp)) {
                        Text("[${log.levelLabel}]", style = MaterialTheme.typography.labelSmall, color = levelColor, fontFamily = FontFamily.Monospace, modifier = Modifier.width(52.dp))
                        Text(log.message, style = MaterialTheme.typography.labelSmall, color = Color(0xFFCCFFCC), fontFamily = FontFamily.Monospace, maxLines = 2, overflow = TextOverflow.Ellipsis)
                    }
                }
            }
        }
    }
}

@Composable
fun ServiceStatusScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var queues by remember { mutableStateOf<List<DiagQueue>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getDiagQueues() }.onSuccess { queues = it.queues }
        loading = false
    }

    DevScaffold("Services / IPC Queues", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            items(queues) { q ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Row(modifier = Modifier.padding(12.dp), horizontalArrangement = Arrangement.SpaceBetween) {
                        Text(q.service, style = MaterialTheme.typography.bodySmall, color = Color(0xFF00FF88), fontFamily = FontFamily.Monospace, modifier = Modifier.weight(1f))
                        Text("depth: ${q.depth}  pending: ${q.pending}", style = MaterialTheme.typography.labelSmall, color = Color(0xFF3A7A3A), fontFamily = FontFamily.Monospace)
                    }
                }
            }
        }
    }
}

@Composable
fun DiagnosticsScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var routes by remember { mutableStateOf<List<RouteEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var search by remember { mutableStateOf("") }

    LaunchedEffect(Unit) {
        runCatching { container.api(true).getDiagRoutes() }.onSuccess { routes = it.routes }
        loading = false
    }

    DevScaffold("Route Manifest", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            OutlinedTextField(
                value = search,
                onValueChange = { search = it },
                modifier = Modifier.fillMaxWidth().padding(12.dp),
                placeholder = { Text("Filter routes...", color = Color(0xFF3A7A3A)) },
                leadingIcon = { Icon(Icons.Rounded.Search, null, tint = Color(0xFF3A7A3A)) },
                colors = OutlinedTextFieldDefaults.colors(
                    focusedTextColor = Color(0xFF00FF88), unfocusedTextColor = Color(0xFF00FF88),
                    focusedBorderColor = Color(0xFF00FF88), unfocusedBorderColor = Color(0xFF3A7A3A),
                    focusedContainerColor = Color(0xFF0A1A0A), unfocusedContainerColor = Color(0xFF0A1A0A),
                ),
                shape = RoundedCornerShape(10.dp)
            )

            if (loading) { IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp)) }
            else {
                LazyColumn(contentPadding = PaddingValues(bottom = 16.dp)) {
                    item {
                        Text("${routes.size} active routes", color = Color(0xFF3A7A3A), style = MaterialTheme.typography.labelSmall, modifier = Modifier.padding(start = 16.dp, bottom = 8.dp))
                    }
                    val filtered = routes.filter { it.path.contains(search, true) || it.method.contains(search, true) }
                    items(filtered) { r ->
                        val methodColor = when (r.method) {
                            "GET" -> Color(0xFF00FF88)
                            "POST" -> Color(0xFF00AAFF)
                            "PUT" -> IsyaWarn
                            "DELETE" -> IsyaError
                            else -> IsyaMuted
                        }
                        Column(modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 6.dp)) {
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Text(r.method, color = methodColor, fontWeight = FontWeight.Bold, style = MaterialTheme.typography.labelSmall, modifier = Modifier.width(48.dp))
                                Text(r.path, color = Color(0xFFCCFFCC), fontFamily = FontFamily.Monospace, style = MaterialTheme.typography.bodySmall)
                            }
                            Text(r.handler, color = Color(0xFF3A7A3A), style = MaterialTheme.typography.labelSmall, fontSize = 9.sp, modifier = Modifier.padding(start = 48.dp))
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun ApiExplorerScreen(container: AppContainerExtended, onBack: () -> Unit) {
    DevScaffold("API Explorer", onBack) { padding ->
        Box(modifier = Modifier.fillMaxSize().padding(padding), contentAlignment = Alignment.Center) {
            Text("Interactive swagger-style explorer coming soon.", color = Color(0xFF3A7A3A))
        }
    }
}

@Composable
fun HardwareScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var info by remember { mutableStateOf<HardwareInfo?>(null) }
    var actions by remember { mutableStateOf<List<HardwareAction>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getHardwareInfo() }.onSuccess { info = it }
        runCatching { container.api(true).getPendingHardwareActions() }.onSuccess { actions = it.actions }
        loading = false
    }

    DevScaffold("Hardware / HAL", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            item {
                Text("System Info", color = Color(0xFF00FF88), style = MaterialTheme.typography.labelLarge)
                info?.let { i ->
                    Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                        Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                            Text("CPU: ${i.cpuModel}", color = Color(0xFFCCFFCC), style = MaterialTheme.typography.bodySmall)
                            Text("GPU: ${i.gpuModel}", color = Color(0xFFCCFFCC), style = MaterialTheme.typography.bodySmall)
                            Text("RAM: ${i.ramTotalGb} GB", color = Color(0xFFCCFFCC), style = MaterialTheme.typography.bodySmall)
                            Text("Kernel: ${i.osVersion}", color = Color(0xFFCCFFCC), style = MaterialTheme.typography.bodySmall)
                        }
                    }
                }
            }
            item {
                Text("Pending Action Queue", color = Color(0xFF00FF88), style = MaterialTheme.typography.labelLarge, modifier = Modifier.padding(top = 8.dp))
            }
            if (actions.isEmpty()) {
                item { Text("Queue empty", color = Color(0xFF3A7A3A), style = MaterialTheme.typography.bodySmall) }
            }
            items(actions) { action ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Row(horizontalArrangement = Arrangement.SpaceBetween, modifier = Modifier.fillMaxWidth()) {
                            Text(action.action, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                            Text("ID: ${action.id}", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                        }
                        Text(action.params, color = Color(0xFFCCFFCC), fontSize = 11.sp, fontFamily = FontFamily.Monospace)
                        Text("Created: ${action.createdAt}", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                    }
                }
            }
        }
    }
}

@Composable
fun ModelsScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var workers by remember { mutableStateOf<List<ModelWorker>>(emptyList()) }
    var slots by remember { mutableStateOf<List<ModelSlot>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.api(true).getModelWorkers() }.onSuccess { workers = it.workers }
        runCatching { container.api(true).getModelSlots() }.onSuccess { slots = it.slots }
        loading = false
    }

    DevScaffold("Models / Slots", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            item { Text("Model Slots", color = Color(0xFF00FF88), style = MaterialTheme.typography.labelLarge) }
            items(slots) { slot ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Row(modifier = Modifier.padding(12.dp), horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically) {
                        Column {
                            Text("Slot ${slot.slotNumber}: ${slot.modelName}", color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                            Text("Address: ${slot.address}:${slot.port}", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                        }
                        Text(if (slot.isPrimary) "PRIMARY" else "BACKUP", color = if (slot.isPrimary) IsyaGold else Color(0xFF3A7A3A), style = MaterialTheme.typography.labelSmall)
                    }
                }
            }
            item { Text("Workers", color = Color(0xFF00FF88), style = MaterialTheme.typography.labelLarge, modifier = Modifier.padding(top = 8.dp)) }
            items(workers) { worker ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Row(horizontalArrangement = Arrangement.SpaceBetween, modifier = Modifier.fillMaxWidth()) {
                            Text(worker.hostname, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                            Text(worker.status, color = if (worker.status == "ONLINE") Color(0xFF00FF88) else IsyaError, style = MaterialTheme.typography.labelSmall)
                        }
                        Text("Model: ${worker.modelName}", color = Color(0xFFCCFFCC), fontSize = 11.sp)
                        Text("VRAM: ${worker.vramUsedMb}/${worker.vramTotalMb} MB", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                    }
                }
            }
        }
    }
}

@Composable
fun AgentsScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var agents by remember { mutableStateOf<List<Agent>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getAgents() }.onSuccess { agents = it.agents }
        loading = false
    }

    DevScaffold("Agent Personas", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            items(agents) { agent ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text(agent.name, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Text(agent.description, color = Color(0xFFCCFFCC), fontSize = 11.sp, maxLines = 2, overflow = TextOverflow.Ellipsis)
                        Text("Model Override: ${agent.modelOverride ?: "None"}", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                    }
                }
            }
        }
    }
}

@Composable
fun ToolsScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var tools by remember { mutableStateOf<List<AiTool>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getTools() }.onSuccess { tools = it.tools }
        loading = false
    }

    DevScaffold("AI Tools", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            items(tools) { tool ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text(tool.name, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Text(tool.description, color = Color(0xFFCCFFCC), fontSize = 11.sp)
                        Text("Parameters: ${tool.parametersJson?.take(100)}...", color = Color(0xFF3A7A3A), fontSize = 10.sp, fontFamily = FontFamily.Monospace)
                    }
                }
            }
        }
    }
}

@Composable
fun DictionaryAdminScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var stats by remember { mutableStateOf<DictionaryStats?>(null) }
    var status by remember { mutableStateOf<DictionaryLoadStatus?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getDictionaryStats() }.onSuccess { stats = it }
        runCatching { container.extendedApi.getDictionaryLoadStatus() }.onSuccess { status = it }
        loading = false
    }

    DevScaffold("Dictionary Admin", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(16.dp)) {
            stats?.let { s ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        Text("Word Count: ${s.totalWords}", color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Text("Last Updated: ${s.lastUpdate}", color = Color(0xFFCCFFCC), fontSize = 11.sp)
                    }
                }
            }
            status?.let { s ->
                Text("Load Status: ${s.status} (${(s.progress * 100).toInt()}%)", color = if (s.status == "READY") Color(0xFF00FF88) else IsyaGold)
                LinearProgressIndicator(progress = { s.progress }, modifier = Modifier.fillMaxWidth(), color = Color(0xFF00FF88), trackColor = Color(0xFF0A1A0A))
            }
            Spacer(Modifier.weight(1f))
            val scope = rememberCoroutineScope()
            var feedback by remember { mutableStateOf<String?>(null) }
            feedback?.let {
                Text(it, color = if (it.startsWith("ok")) Color(0xFF00FF88) else IsyaError,
                     fontSize = 11.sp)
            }
            IsyaButton(
                text = "Trigger Full Re-Index",
                onClick = {
                    scope.launch {
                        feedback = runCatching { container.extendedApi.commitMemory() }
                            .fold(
                                onSuccess = { "ok — memory consolidation queued" },
                                onFailure = { "fail: ${it.message ?: "unknown"}" }
                            )
                    }
                },
                variant = IsyaButtonVariant.PRIMARY_GOLD,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
fun MemoryAdminScreen(container: AppContainerExtended, onBack: () -> Unit) {
    DevScaffold("Memory Admin", onBack) { padding ->
        Box(modifier = Modifier.fillMaxSize().padding(padding), contentAlignment = Alignment.Center) {
            Text("Memory pruning and manual entry tools coming soon.", color = Color(0xFF3A7A3A))
        }
    }
}

@Composable
fun BackupsScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var backups by remember { mutableStateOf<List<BackupInfo>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.api(true).getBackups() }.onSuccess { backups = it.backups }
        loading = false
    }

    DevScaffold("SQL Backups", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        val scope = rememberCoroutineScope()
        var feedback by remember { mutableStateOf<String?>(null) }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            item {
                IsyaButton(
                    text = "Trigger Instant Backup",
                    onClick = {
                        scope.launch {
                            feedback = runCatching { container.extendedApi.createBackup() }
                                .fold(
                                    onSuccess = { "ok — backup written" },
                                    onFailure = { "fail: ${it.message ?: "unknown"}" }
                                )

                            runCatching { container.extendedApi.getBackups() }
                                .onSuccess { backups = it.backups }
                        }
                    },
                    variant = IsyaButtonVariant.PRIMARY_GOLD,
                    modifier = Modifier.fillMaxWidth()
                )
                feedback?.let {
                    Text(it,
                        color = if (it.startsWith("ok")) Color(0xFF00FF88) else IsyaError,
                        fontSize = 11.sp)
                }
            }
            items(backups) { b ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text(b.fileName, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Row(horizontalArrangement = Arrangement.SpaceBetween, modifier = Modifier.fillMaxWidth()) {
                            Text("${b.sizeBytes / 1024 / 1024} MB", color = Color(0xFFCCFFCC), fontSize = 11.sp)
                            Text(b.createdAt, color = Color(0xFF3A7A3A), fontSize = 10.sp)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun ConfigScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var settings by remember { mutableStateOf<ServerSettings?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getServerSettings() }.onSuccess { settings = it }
        loading = false
    }

    DevScaffold("Server Config", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
            settings?.let { s ->
                Text("Version: ${s.version}", color = Color(0xFF00FF88))
                Text("Bind Address: ${s.bindAddress}", color = Color(0xFFCCFFCC))
                Text("Bind Port: ${s.bindPort}", color = Color(0xFFCCFFCC))
                Text("Primary Model: ${s.primaryModel}", color = Color(0xFFCCFFCC))
                Text("Max Connections: ${s.maxConnections}", color = Color(0xFFCCFFCC))
            }
            Spacer(Modifier.weight(1f))
            val scope = rememberCoroutineScope()
            var feedback by remember { mutableStateOf<String?>(null) }
            feedback?.let {
                Text(it,
                    color = if (it.startsWith("ok")) Color(0xFF00FF88) else IsyaError,
                    fontSize = 11.sp)
            }
            IsyaButton(
                text = "Reload Config File",
                onClick = {
                    scope.launch {
                        feedback = runCatching { container.extendedApi.reloadConfig() }
                            .fold(
                                onSuccess = { "ok — config re-read from disk" },
                                onFailure = { "fail: ${it.message ?: "unknown"}" }
                            )

                        runCatching { container.extendedApi.getServerSettings() }
                            .onSuccess { settings = it }
                    }
                },
                variant = IsyaButtonVariant.PRIMARY_GOLD,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
fun PairedDevicesScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var devices by remember { mutableStateOf<List<PairedDevice>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    val scope = rememberCoroutineScope()

    LaunchedEffect(Unit) {
        runCatching { container.api(true).getPairedDevices() }.onSuccess { devices = it.devices }
        loading = false
    }

    DevScaffold("Paired Devices", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@DevScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            item {
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF1A1A0A), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                        Text("Pair-code flow removed", color = IsyaGold, style = MaterialTheme.typography.labelSmall)
                        Text("Sign-in uses username + password only.", color = IsyaCream, style = MaterialTheme.typography.bodySmall)
                    }
                }
            }
            items(devices) { d ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Row(modifier = Modifier.padding(12.dp), verticalAlignment = Alignment.CenterVertically) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(d.deviceName, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                            Text("ID: ${d.deviceId}", color = Color(0xFF3A7A3A), fontSize = 10.sp)
                            Text("Last Active: ${d.lastActive}", color = Color(0xFFCCFFCC), fontSize = 11.sp)
                        }
                        IconButton(onClick = {
                            scope.launch {
                                runCatching {
                                    container.extendedApi.revokeDevice(d.deviceId)
                                }.onSuccess {

                                    devices = devices.filterNot { it.deviceId == d.deviceId }
                                    runCatching { container.api(true).getPairedDevices() }
                                        .onSuccess { resp -> devices = resp.devices }
                                }
                            }
                        }) {
                            Icon(Icons.Rounded.Delete, null, tint = IsyaError)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun VideoWorkersScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var avatars by remember { mutableStateOf<List<UserAvatar>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var err by remember { mutableStateOf<String?>(null) }
    val scope = rememberCoroutineScope()

    fun reload() {
        loading = true; err = null
        scope.launch {
            runCatching { container.extendedApi.getAvatars() }
                .onSuccess { avatars = it.avatars }
                .onFailure { err = it.message ?: it.javaClass.simpleName }
            loading = false
        }
    }
    LaunchedEffect(Unit) { reload() }

    DevScaffold("Video Generation Workers", onBack) { padding ->
        if (loading) {
            IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp))
            return@DevScaffold
        }
        LazyColumn(
            modifier         = Modifier.fillMaxSize().padding(padding),
            contentPadding   = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            err?.let {
                item {
                    Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF2A0D0D), modifier = Modifier.fillMaxWidth()) {
                        Text("Worker queue unreachable: $it", color = IsyaError, modifier = Modifier.padding(12.dp))
                    }
                }
            }
            item {
                Text(
                    "${avatars.size} avatar${if (avatars.size == 1) "" else "s"} registered",
                    color = IsyaGold, fontWeight = FontWeight.Bold,
                )
            }
            items(avatars) { a ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text(a.label?.takeIf { it.isNotBlank() } ?: "(unnamed)",
                             color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Text("ID: ${a.id}${if (a.isDefault) "  (default)" else ""}",
                             color = Color(0xFF3A7A3A), fontSize = 10.sp)
                        Text("Path: ${a.filePath}",
                             color = Color(0xFFCCFFCC), fontSize = 11.sp,
                             maxLines = 1, overflow = TextOverflow.Ellipsis)
                    }
                }
            }
            item {
                IsyaButton(
                    text     = "Refresh",
                    onClick  = { reload() },
                    variant  = IsyaButtonVariant.PRIMARY_GOLD,
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
fun LearningAdminScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var skills by remember { mutableStateOf<List<Skill>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var err by remember { mutableStateOf<String?>(null) }
    val scope = rememberCoroutineScope()

    fun reload() {
        loading = true; err = null
        scope.launch {
            runCatching { container.extendedApi.getSkills() }
                .onSuccess { skills = it.skills }
                .onFailure { err = it.message ?: it.javaClass.simpleName }
            loading = false
        }
    }
    LaunchedEffect(Unit) { reload() }

    DevScaffold("Learning Admin", onBack) { padding ->
        if (loading) {
            IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp))
            return@DevScaffold
        }
        LazyColumn(
            modifier         = Modifier.fillMaxSize().padding(padding),
            contentPadding   = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            err?.let {
                item {
                    Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF2A0D0D), modifier = Modifier.fillMaxWidth()) {
                        Text("Could not load skills: $it", color = IsyaError, modifier = Modifier.padding(12.dp))
                    }
                }
            }
            item {
                Text("${skills.size} skill${if (skills.size == 1) "" else "s"} inventoried",
                     color = IsyaGold, fontWeight = FontWeight.Bold)
            }
            items(skills) { s ->
                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text(s.skillName, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                        Text("Category: ${s.category ?: "—"}", color = Color(0xFFCCFFCC), fontSize = 11.sp)
                        Text("Proficiency: ${s.proficiency}  ·  Used ${s.timesUsed}×",
                             color = Color(0xFF3A7A3A), fontSize = 10.sp)
                    }
                }
            }
            item {
                IsyaButton(
                    text     = "Refresh",
                    onClick  = { reload() },
                    variant  = IsyaButtonVariant.PRIMARY_GOLD,
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
fun EthicsAdminScreen(container: AppContainerExtended, onBack: () -> Unit) {
    var rules by remember { mutableStateOf<List<MoralRule>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var err by remember { mutableStateOf<String?>(null) }
    var newPrinciple by remember { mutableStateOf("") }
    var newCategory by remember { mutableStateOf("") }
    var newHard by remember { mutableStateOf(false) }
    var saving by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    fun reload() {
        loading = true; err = null
        scope.launch {
            runCatching { container.extendedApi.getMoralRules() }
                .onSuccess { rules = it.rules }
                .onFailure { err = it.message ?: it.javaClass.simpleName }
            loading = false
        }
    }
    LaunchedEffect(Unit) { reload() }

    DevScaffold("Ethics Admin", onBack) { padding ->
        LazyColumn(
            modifier         = Modifier.fillMaxSize().padding(padding),
            contentPadding   = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            err?.let {
                item {
                    Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF2A0D0D), modifier = Modifier.fillMaxWidth()) {
                        Text("Could not load rules: $it", color = IsyaError, modifier = Modifier.padding(12.dp))
                    }
                }
            }
            item {

                Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF1A1A0A), modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(12.dp)) {
                        Text("Add Rule (ADMIN)", color = IsyaGold, fontWeight = FontWeight.Bold)
                        Spacer(Modifier.height(6.dp))
                        OutlinedTextField(
                            value           = newPrinciple,
                            onValueChange   = { newPrinciple = it },
                            label           = { Text("Principle") },
                            singleLine      = false,
                            modifier        = Modifier.fillMaxWidth(),
                        )
                        Spacer(Modifier.height(6.dp))
                        OutlinedTextField(
                            value           = newCategory,
                            onValueChange   = { newCategory = it },
                            label           = { Text("Category (optional)") },
                            singleLine      = true,
                            modifier        = Modifier.fillMaxWidth(),
                        )
                        Spacer(Modifier.height(6.dp))
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Switch(checked = newHard, onCheckedChange = { newHard = it })
                            Spacer(Modifier.width(8.dp))
                            Text("Hard rule (un-overridable)", color = IsyaCream)
                        }
                        Spacer(Modifier.height(8.dp))
                        IsyaButton(
                            text     = if (saving) "Saving…" else "Add Rule",
                            onClick  = {
                                if (newPrinciple.isNotBlank() && !saving) {
                                    saving = true
                                    scope.launch {
                                        runCatching {
                                            container.api(true).createMoralRule(
                                                CreateMoralRuleRequest(
                                                    principle  = newPrinciple.trim(),
                                                    category   = newCategory.trim().ifEmpty { null },
                                                    isHardRule = newHard,
                                                )
                                            )
                                        }.onSuccess {
                                            newPrinciple = ""; newCategory = ""; newHard = false
                                            reload()
                                        }.onFailure { err = it.message ?: it.javaClass.simpleName }
                                        saving = false
                                    }
                                }
                            },
                            variant  = IsyaButtonVariant.PRIMARY_GOLD,
                            modifier = Modifier.fillMaxWidth(),
                        )
                    }
                }
            }
            if (loading) {
                item { IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(16.dp)) }
            } else {
                item {
                    Text("${rules.size} rule${if (rules.size == 1) "" else "s"} active",
                         color = IsyaGold, fontWeight = FontWeight.Bold)
                }
                items(rules) { r ->
                    Surface(shape = RoundedCornerShape(8.dp), color = Color(0xFF0D2B0D), modifier = Modifier.fillMaxWidth()) {
                        Column(modifier = Modifier.padding(12.dp)) {
                            Text(r.principle, color = Color(0xFF00FF88), fontWeight = FontWeight.Bold)
                            Text(
                                "Category: ${r.category ?: "—"}  ·  ${if (r.isHardRule) "HARD" else "soft"}",
                                color    = if (r.isHardRule) IsyaError else Color(0xFF3A7A3A),
                                fontSize = 10.sp,
                            )
                        }
                    }
                }
            }
        }
    }
}
