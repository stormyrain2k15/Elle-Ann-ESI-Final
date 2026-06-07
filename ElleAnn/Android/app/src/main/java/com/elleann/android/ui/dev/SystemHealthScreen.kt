package com.elleann.android.ui.dev

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.ArrowBack
import androidx.compose.material.icons.rounded.Refresh
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.models.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

private fun AppContainerExtended.adminApiOrFallback(): com.elleann.android.data.ElleApiExtended =
    adminApi ?: extendedApi

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SystemHealthScreen(
    container: AppContainerExtended,
    onBack: () -> Unit,
) {
    var health by remember { mutableStateOf<DiagHealthResponse?>(null) }
    var wires by remember { mutableStateOf<List<DiagWireRow>>(emptyList()) }
    var heartbeats by remember { mutableStateOf<List<DiagHeartbeatRow>>(emptyList()) }
    var queues by remember { mutableStateOf<List<DiagQueue>>(emptyList()) }
    var lastError by remember { mutableStateOf<String?>(null) }
    var refreshTick by remember { mutableStateOf(0) }
    val scope = rememberCoroutineScope()

    LaunchedEffect(Unit) {
        while (true) {
            val api = container.adminApiOrFallback()
            runCatching { api.getDiagHealth() }
                .onSuccess { health = it; lastError = null }
                .onFailure { lastError = it.message ?: it::class.java.simpleName }
            runCatching { api.getDiagWires() }.onSuccess { wires = it.wires }
            runCatching { api.getDiagHeartbeats() }.onSuccess { heartbeats = it.heartbeats }
            runCatching { api.getDiagQueues() }.onSuccess { queues = it.queues }
            refreshTick++
            delay(10_000)
        }
    }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("System Health", color = Color(0xFF00FF88)) },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted)
                    }
                },
                actions = {
                    IconButton(onClick = {
                        scope.launch {
                            val api = container.adminApiOrFallback()
                            runCatching { api.getDiagHealth() }
                                .onSuccess { health = it; lastError = null }
                                .onFailure { lastError = it.message }
                            runCatching { api.getDiagWires() }.onSuccess { wires = it.wires }
                            runCatching { api.getDiagHeartbeats() }.onSuccess { heartbeats = it.heartbeats }
                            runCatching { api.getDiagQueues() }.onSuccess { queues = it.queues }
                            refreshTick++
                        }
                    }) {
                        Icon(Icons.Rounded.Refresh, "Refresh", tint = Color(0xFF00FF88))
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = Color(0xFF0A1A0A)),
            )
        },
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(12.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),

            userScrollEnabled = true,
        ) {
            item("issues") {
                IssuesPanel(
                    health = health,
                    error = lastError,
                    refreshTick = refreshTick,
                )
            }

            item("llm") { LLMHealthPanel(health) }

            item("ipc-summary") {
                MetricRow(
                    label = "IPC wires (in-process)",
                    detail = "${health?.wiresUp ?: 0} up / ${health?.wiresTotal ?: 0} total",
                    healthy = (health?.let { it.wiresUp == it.wiresTotal && it.wiresTotal > 0 } == true),
                )
            }
            items(wires, key = { "w-${it.serviceId}" }) { w -> WireRow(w) }

            item("hb-summary") {
                MetricRow(
                    label = "Heartbeats (cross-process)",
                    detail = "${health?.heartbeatsUp ?: 0} up / ${health?.heartbeatsTotal ?: 0} total",
                    healthy = (health?.let { it.heartbeatsUp == it.heartbeatsTotal && it.heartbeatsTotal > 0 } == true),
                )
            }
            items(heartbeats, key = { "hb-${it.serviceId}" }) { hb -> HeartbeatRow(hb) }

            item("queues-summary") {
                Text(
                    "Queues",
                    color = IsyaGold,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(top = 8.dp, start = 4.dp),
                )
            }
            item("queues-totals") {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    MetricRow(
                        label = "Intent queue pending",
                        detail = (health?.intentPending ?: 0L).toString(),
                        healthy = ((health?.intentPending ?: 0L) <= 1000),
                    )
                    MetricRow(
                        label = "Action queue pending",
                        detail = (health?.actionPending ?: 0L).toString(),
                        healthy = ((health?.actionPending ?: 0L) <= 1000),
                    )
                    MetricRow(
                        label = "Memory rows",
                        detail = (health?.memoryCount ?: 0L).toString(),
                        healthy = true,
                    )
                }
            }
            items(queues, key = { "q-${it.service}" }) { q -> QueueRow(q) }
        }
    }
}

@Composable
private fun IssuesPanel(
    health: DiagHealthResponse?,
    error: String?,
    refreshTick: Int,
) {
    val issues = health?.issues ?: emptyList()
    val container = when {
        error != null -> Color(0xFF3A0D0D)
        issues.isEmpty() && health != null -> Color(0xFF0D2B0D)
        else -> Color(0xFF3A2A0D)
    }
    val tint = when {
        error != null -> Color(0xFFFF6F6F)
        issues.isEmpty() && health != null -> Color(0xFF00FF88)
        else -> IsyaGold
    }
    val title = when {
        error != null -> "/api/diag/health unreachable"
        issues.isEmpty() && health != null -> "All systems nominal"
        health == null -> "Loading…"
        else -> "${issues.size} issue${if (issues.size == 1) "" else "s"}"
    }
    Surface(
        shape = RoundedCornerShape(10.dp),
        color = container,
        modifier = Modifier.fillMaxWidth(),
    ) {
        Column(modifier = Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(title, color = tint, fontWeight = FontWeight.Bold)
                Spacer(Modifier.weight(1f))
                Text(
                    "tick ${refreshTick}",
                    color = IsyaMuted,
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace,
                )
            }
            error?.let {
                Text(
                    it,
                    color = Color(0xFFFFB3B3),
                    fontSize = 12.sp,
                    fontFamily = FontFamily.Monospace,
                )
            }
            issues.forEach { iss ->
                Text(
                    "• $iss",
                    color = Color(0xFFFFE8B0),
                    fontSize = 13.sp,
                    fontFamily = FontFamily.Monospace,
                )
            }
        }
    }
}

@Composable
private fun LLMHealthPanel(health: DiagHealthResponse?) {
    val llm = health?.llm
    val healthy = llm?.healthy == true
    val bg = if (healthy) Color(0xFF0D2B0D) else Color(0xFF3A0D0D)
    val accent = if (healthy) Color(0xFF00FF88) else Color(0xFFFF6F6F)
    Surface(
        shape = RoundedCornerShape(10.dp),
        color = bg,
        modifier = Modifier.fillMaxWidth(),
    ) {
        Row(modifier = Modifier.padding(14.dp), verticalAlignment = Alignment.CenterVertically) {
            Column(modifier = Modifier.weight(1f)) {
                Text("LLM", color = accent, fontWeight = FontWeight.Bold)
                Text(
                    if (llm == null) "—" else "${llm.provider}  ·  ${llm.model.ifBlank { "(no model)" }}",
                    color = Color(0xFFCCFFCC),
                    fontSize = 13.sp,
                    fontFamily = FontFamily.Monospace,
                )
            }
            Text(
                if (healthy) "READY" else "DOWN",
                color = accent,
                fontWeight = FontWeight.Bold,
                fontFamily = FontFamily.Monospace,
            )
        }
    }
}

@Composable
private fun MetricRow(label: String, detail: String, healthy: Boolean) {
    val accent = if (healthy) Color(0xFF00FF88) else IsyaGold
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF0D2B0D), RoundedCornerShape(6.dp))
            .padding(horizontal = 12.dp, vertical = 8.dp),
    ) {
        Text(label, color = Color(0xFFCCFFCC), modifier = Modifier.weight(1f), fontSize = 13.sp)
        Text(detail, color = accent, fontFamily = FontFamily.Monospace, fontSize = 13.sp)
    }
}

@Composable
private fun WireRow(w: DiagWireRow) {
    val accent = when (w.state) {
        "up" -> Color(0xFF00FF88)
        "stale" -> IsyaGold
        else -> IsyaMuted
    }
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF0A1A0A), RoundedCornerShape(4.dp))
            .padding(horizontal = 10.dp, vertical = 4.dp),
    ) {
        Text(
            w.service,
            color = Color(0xFFCCFFCC),
            modifier = Modifier.weight(1f),
            fontSize = 12.sp,
            fontFamily = FontFamily.Monospace,
        )
        val detail =
            if (w.lastSeenMs == 0L) "never"
            else "${w.quietMinutes}m quiet"
        Text(detail, color = accent, fontSize = 11.sp, fontFamily = FontFamily.Monospace)
        Spacer(Modifier.width(8.dp))
        Text(
            w.state.uppercase(),
            color = accent,
            fontSize = 10.sp,
            fontWeight = FontWeight.Bold,
            fontFamily = FontFamily.Monospace,
        )
    }
}

@Composable
private fun HeartbeatRow(hb: DiagHeartbeatRow) {
    val accent = when (hb.state) {
        "up" -> Color(0xFF00FF88)
        "stale" -> IsyaGold
        else -> Color(0xFFFF6F6F)
    }
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF0A1A0A), RoundedCornerShape(4.dp))
            .padding(horizontal = 10.dp, vertical = 4.dp),
    ) {
        Text(
            "svc#${hb.serviceId}",
            color = Color(0xFFCCFFCC),
            modifier = Modifier.weight(1f),
            fontSize = 12.sp,
            fontFamily = FontFamily.Monospace,
        )
        Text("${hb.quietSec}s", color = accent, fontSize = 11.sp, fontFamily = FontFamily.Monospace)
        Spacer(Modifier.width(8.dp))
        Text(
            hb.state.uppercase(),
            color = accent,
            fontSize = 10.sp,
            fontWeight = FontWeight.Bold,
            fontFamily = FontFamily.Monospace,
        )
    }
}

@Composable
private fun QueueRow(q: DiagQueue) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF0A1A0A), RoundedCornerShape(4.dp))
            .padding(horizontal = 10.dp, vertical = 4.dp),
    ) {
        Text(
            q.service,
            color = Color(0xFFCCFFCC),
            modifier = Modifier.weight(1f),
            fontSize = 12.sp,
            fontFamily = FontFamily.Monospace,
        )
        Text(
            "depth=${q.depth} pending=${q.pending}",
            color = Color(0xFF3A7A3A),
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
        )
    }
}
