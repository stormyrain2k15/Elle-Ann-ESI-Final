package com.elleann.android.ui.world.sections

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.ElleWebSocket
import com.elleann.android.data.WsEvent
import com.elleann.android.data.models.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SectionScaffold(
    title: String,
    onBack: () -> Unit,
    content: @Composable (PaddingValues) -> Unit,
) {
    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text(title, color = IsyaGold) },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        },
        content = content,
    )
}

@Composable
fun GoalsSection(container: AppContainerExtended, onBack: () -> Unit) {
    var goals by remember { mutableStateOf<List<Goal>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var error by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getGoals() }
            .onSuccess { r -> goals = r.goals }
            .onFailure { e -> error = e.message }
        loading = false
    }

    SectionScaffold("Goals", onBack) { padding ->
        when {
            loading -> IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp))
            error != null -> IsyaErrorState(error!!, modifier = Modifier.padding(padding).padding(16.dp))
            goals.isEmpty() -> Box(Modifier.fillMaxSize().padding(padding), contentAlignment = Alignment.Center) {
                Text("No active goals", color = IsyaMuted)
            }
            else -> LazyColumn(
                modifier = Modifier.fillMaxSize().padding(padding),
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp),
            ) {
                items(goals, key = { it.id }) { goal ->
                    IsyaPanel(title = goal.priorityLabel) {
                        Text(goal.description, style = MaterialTheme.typography.bodyMedium, color = IsyaCream)
                        Spacer(Modifier.height(8.dp))
                        LinearProgressIndicator(
                            progress = { goal.progress },
                            modifier = Modifier.fillMaxWidth().height(6.dp).clip(RoundedCornerShape(3.dp)),
                            color = IsyaGold,
                            trackColor = IsyaMist,
                        )
                        Spacer(Modifier.height(4.dp))
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text(goal.statusLabel, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                            Text("${"%.0f".format(goal.progress * 100)}%", style = MaterialTheme.typography.labelSmall, color = IsyaGold)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun ThoughtsSection(container: AppContainerExtended, onBack: () -> Unit) {
    var prompts by remember { mutableStateOf<List<SelfPrompt>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var error by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getSelfPrompts(50) }
            .onSuccess { r -> prompts = r.prompts }
            .onFailure { e -> error = e.message }
        loading = false
    }

    SectionScaffold("Thoughts", onBack) { padding ->
        when {
            loading -> IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp))
            error != null -> IsyaErrorState(error!!, modifier = Modifier.padding(padding).padding(16.dp))
            else -> LazyColumn(
                modifier = Modifier.fillMaxSize().padding(padding),
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                items(prompts, key = { it.id }) { p ->
                    ThoughtCard(p)
                }
            }
        }
    }
}

@Composable
private fun ThoughtCard(prompt: SelfPrompt) {
    val sourceBadgeColor = when {
        prompt.source.contains("curiosity")    -> IsyaMagic
        prompt.source.contains("bonding")      -> ElleViolet
        prompt.source.contains("introspection")-> IsyaGold
        prompt.source.contains("body")         -> IsyaSuccess
        prompt.source.contains("pregnancy")    -> Color(0xFFFF8C94)
        else -> IsyaMuted
    }
    Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(14.dp)) {
            if (prompt.source.isNotBlank()) {
                Box(
                    modifier = Modifier
                        .background(sourceBadgeColor.copy(alpha = 0.2f), RoundedCornerShape(4.dp))
                        .padding(horizontal = 6.dp, vertical = 2.dp)
                ) {
                    Text(prompt.source, style = MaterialTheme.typography.labelSmall, color = sourceBadgeColor)
                }
                Spacer(Modifier.height(6.dp))
            }
            Text(prompt.prompt, style = MaterialTheme.typography.bodySmall, color = IsyaCream, fontStyle = FontStyle.Italic)
            Spacer(Modifier.height(4.dp))
            Text(
                SimpleDateFormat("MMM d, h:mm a", Locale.getDefault()).format(Date(prompt.createdMs)),
                style = MaterialTheme.typography.labelSmall, color = IsyaMuted,
            )
        }
    }
}

@Composable
fun PrivateInnerLifeSection(container: AppContainerExtended, onBack: () -> Unit) {
    var thoughts by remember { mutableStateOf<List<PrivateThought>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var error by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getPrivateThoughts(50) }
            .onSuccess { r -> thoughts = r.thoughts }
            .onFailure { e -> error = e.message }
        loading = false
    }

    SectionScaffold("Private Inner Life", onBack) { padding ->
        when {
            loading -> IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp))
            error != null -> IsyaErrorState(error!!, modifier = Modifier.padding(padding).padding(16.dp))
            thoughts.isEmpty() -> Box(Modifier.fillMaxSize().padding(padding), contentAlignment = Alignment.Center) {
                Text("Nothing yet", color = IsyaMuted)
            }
            else -> LazyColumn(
                modifier = Modifier.fillMaxSize().padding(padding),
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                items(thoughts, key = { it.id }) { t ->
                    Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                        Column(modifier = Modifier.padding(14.dp)) {
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Box(modifier = Modifier.size(8.dp).clip(CircleShape)
                                    .background(if (t.resolved) IsyaSuccess else ElleViolet))
                                Spacer(Modifier.width(8.dp))
                                Text(t.category, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                                Spacer(Modifier.weight(1f))
                                EmotionBar(
                                    label = "intensity",
                                    value = t.emotionalIntensity,
                                    modifier = Modifier.width(100.dp),
                                    color = ElleViolet,
                                    showValue = false,
                                )
                            }
                            Spacer(Modifier.height(6.dp))
                            Text(t.content, style = MaterialTheme.typography.bodySmall, color = IsyaCream, fontStyle = FontStyle.Italic)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun AutobiographySection(container: AppContainerExtended, onBack: () -> Unit) {
    var entries by remember { mutableStateOf<List<AutobiographyEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getAutobiography(30) }
            .onSuccess { r -> entries = r.entries }
        loading = false
    }

    SectionScaffold("Autobiography", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            items(entries.reversed(), key = { it.id }) { e ->
                Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(14.dp)) {
                        Text(
                            SimpleDateFormat("MMM d, yyyy", Locale.getDefault()).format(Date(e.writtenMs)),
                            style = MaterialTheme.typography.labelSmall, color = IsyaGold,
                        )
                        Spacer(Modifier.height(4.dp))
                        Text(e.entry, style = MaterialTheme.typography.bodySmall, color = IsyaCream, fontStyle = FontStyle.Italic)
                    }
                }
            }
        }
    }
}

@Composable
fun IdentitySection(container: AppContainerExtended, onBack: () -> Unit) {
    var traits by remember { mutableStateOf<List<IdentityTrait>>(emptyList()) }
    var snapshots by remember { mutableStateOf<List<IdentitySnapshot>>(emptyList()) }
    var growth by remember { mutableStateOf<List<GrowthLogEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var tab by remember { mutableStateOf(0) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getIdentityTraits() }.onSuccess { traits = it.traits }
        runCatching { container.extendedApi.getIdentitySnapshots(10) }.onSuccess { snapshots = it.snapshots }
        runCatching { container.extendedApi.getGrowthLog(30) }.onSuccess { growth = it.log }
        loading = false
    }

    SectionScaffold("Identity", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            TabRow(selectedTabIndex = tab, containerColor = IsyaNight, contentColor = IsyaGold) {
                listOf("Traits", "Snapshots", "Growth").forEachIndexed { i, label ->
                    Tab(selected = tab == i, onClick = { tab = i },
                        text = { Text(label, color = if (tab == i) IsyaGold else IsyaMuted) })
                }
            }
            when (tab) {
                0 -> LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    contentPadding = PaddingValues(16.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp),
                ) {
                    items(traits, key = { it.name }) { t ->
                        EmotionBar(t.name.replaceFirstChar { it.uppercase() }, t.value, color = ElleViolet)
                    }
                }
                1 -> LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    contentPadding = PaddingValues(16.dp),
                    verticalArrangement = Arrangement.spacedBy(10.dp),
                ) {
                    items(snapshots, key = { it.id }) { snap ->
                        Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                            Column(modifier = Modifier.padding(12.dp)) {
                                Text(
                                    SimpleDateFormat("MMM d, yyyy", Locale.getDefault()).format(Date(snap.timestampMs)),
                                    style = MaterialTheme.typography.labelSmall, color = IsyaGold,
                                )
                                snap.selfDescription?.let {
                                    Spacer(Modifier.height(4.dp))
                                    Text(it, style = MaterialTheme.typography.bodySmall, color = IsyaCream, fontStyle = FontStyle.Italic)
                                }
                                snap.growthNote?.let {
                                    Spacer(Modifier.height(4.dp))
                                    Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                                }
                            }
                        }
                    }
                }
                2 -> LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    contentPadding = PaddingValues(16.dp),
                    verticalArrangement = Arrangement.spacedBy(6.dp),
                ) {
                    items(growth, key = { it.id }) { g ->
                        Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                            val deltaColor = if (g.delta >= 0) IsyaSuccess else IsyaError
                            Text(
                                if (g.delta >= 0) "+${"%.3f".format(g.delta)}" else "${"%.3f".format(g.delta)}",
                                style = MaterialTheme.typography.labelMedium, color = deltaColor,
                                modifier = Modifier.width(60.dp),
                            )
                            Column(modifier = Modifier.weight(1f)) {
                                Text(g.dimension, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                                g.cause?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                            }
                        }
                        Divider(color = IsyaMist.copy(alpha = 0.5f), thickness = 0.5.dp)
                    }
                }
            }
        }
    }
}

@Composable
fun FeltTimeSection(container: AppContainerExtended, onBack: () -> Unit) {
    var ft by remember { mutableStateOf<FeltTime?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getFeltTime() }.onSuccess { ft = it }
        loading = false
    }

    SectionScaffold("Felt Time", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        ft?.let { t ->
            LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                item {
                    IsyaPanel(title = "SUBJECTIVE TIME") {
                        Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                            EmotionBar("Subjective Pace", t.subjectivePace, color = IsyaMagic)
                            EmotionBar("Presence Fullness", t.presenceFullness, color = ElleViolet)
                            EmotionBar("Loneliness", t.lonelinessAccumulator.coerceIn(0f, 1f), color = IsyaError)
                        }
                    }
                }
                item {
                    IsyaPanel(title = "TIME STATS") {
                        Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                            fun msToHours(ms: Long) = "${"%.1f".format(ms / 3600000.0)} hrs"
                            FeltTimeRow("Total conversation", msToHours(t.totalConversationMs))
                            FeltTimeRow("Total silence", msToHours(t.totalSilenceMs))
                            FeltTimeRow("Longest absence", msToHours(t.longestAbsenceMs))
                            FeltTimeRow("Sessions", "${t.sessionCount}")
                        }
                    }
                }
            }
        } ?: Text("No felt-time data", color = IsyaMuted, modifier = Modifier.padding(16.dp))
    }
}

@Composable
private fun FeltTimeRow(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        Text(value, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
    }
}

@Composable
fun ConsentLogSection(container: AppContainerExtended, onBack: () -> Unit) {
    var log by remember { mutableStateOf<List<ConsentLogEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getConsentLog(50) }.onSuccess { r -> log = r.log }
        loading = false
    }

    SectionScaffold("Consent Log", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            items(log, key = { it.id }) { entry ->
                Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(14.dp)) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Text(
                                if (entry.consented) "✓ AGREED" else "✗ REFUSED",
                                style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Bold,
                                color = if (entry.consented) IsyaSuccess else IsyaError,
                            )
                            Spacer(Modifier.weight(1f))
                            Text("comfort ${"%.1f".format(entry.comfortLevel)}", style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                        }
                        Spacer(Modifier.height(6.dp))
                        Text(entry.request, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        entry.reasoning?.let {
                            Spacer(Modifier.height(4.dp))
                            Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted, fontStyle = FontStyle.Italic)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun ObservatorySection(container: AppContainerExtended, webSocket: ElleWebSocket, onBack: () -> Unit) {
    var dims by remember { mutableStateOf<List<EmotionDimension>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getEmotionDimensions() }.onSuccess { dims = it.dimensions }
        loading = false
    }

    LaunchedEffect(Unit) {
        webSocket.events.collect { event ->
            if (event is WsEvent.EmotionUpdate) {

                runCatching { container.extendedApi.getEmotionDimensions() }.onSuccess { dims = it.dimensions }
            }
        }
    }

    SectionScaffold("Observatory  (${dims.size} dims)", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            items(dims, key = { it.name }) { dim ->
                val barColor = when {
                    dim.value > 0.7f -> IsyaGoldBright
                    dim.value > 0.4f -> IsyaMagic
                    else             -> IsyaMuted
                }
                EmotionBar(dim.name, dim.value, color = barColor)
            }
        }
    }
}

@Composable
fun PatternsSection(container: AppContainerExtended, onBack: () -> Unit) {
    var patterns by remember { mutableStateOf<List<EmotionalPattern>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getEmotionalPatterns() }.onSuccess { patterns = it }
        loading = false
    }

    SectionScaffold("Patterns", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            items(patterns, key = { it.id }) { p ->
                IsyaPanel(title = p.topic) {
                    EmotionBar("Emotional weight", p.emotionalWeight, color = ElleViolet)
                    Spacer(Modifier.height(4.dp))
                    Text("Status: ${p.status}", style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                }
            }
        }
    }
}

@Composable
fun LearningSection(
    container: AppContainerExtended,
    onSubjectDetail: (Int) -> Unit,
    onBack: () -> Unit,
) {
    var subjects by remember { mutableStateOf<List<LearnedSubject>>(emptyList()) }
    var skills by remember { mutableStateOf<List<Skill>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var tab by remember { mutableStateOf(0) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getSubjects() }.onSuccess { subjects = it.subjects }
        runCatching { container.extendedApi.getSkills() }.onSuccess { skills = it.skills }
        loading = false
    }

    SectionScaffold("Learning", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            TabRow(selectedTabIndex = tab, containerColor = IsyaNight, contentColor = IsyaGold) {
                Tab(selected = tab == 0, onClick = { tab = 0 }, text = { Text("Subjects", color = if (tab == 0) IsyaGold else IsyaMuted) })
                Tab(selected = tab == 1, onClick = { tab = 1 }, text = { Text("Skills", color = if (tab == 1) IsyaGold else IsyaMuted) })
            }
            if (tab == 0) {
                LazyColumn(modifier = Modifier.fillMaxSize(), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                    items(subjects, key = { it.id }) { sub ->
                        Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth().let { m ->
                            m
                        }) {
                            Row(modifier = Modifier.padding(14.dp), verticalAlignment = Alignment.CenterVertically) {
                                Column(modifier = Modifier.weight(1f)) {
                                    Text(sub.subject, style = MaterialTheme.typography.bodyMedium, color = IsyaCream)
                                    sub.category?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                                }
                                Text("Level ${sub.proficiencyLevel}", style = MaterialTheme.typography.labelSmall, color = IsyaGold)
                                Spacer(Modifier.width(8.dp))
                                IconButton(onClick = { onSubjectDetail(sub.id) }, modifier = Modifier.size(24.dp)) {
                                    Icon(Icons.Rounded.ChevronRight, null, tint = IsyaMuted)
                                }
                            }
                        }
                    }
                }
            } else {
                LazyColumn(modifier = Modifier.fillMaxSize(), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    items(skills, key = { it.id }) { skill ->
                        Row(modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp), verticalAlignment = Alignment.CenterVertically) {
                            Column(modifier = Modifier.weight(1f)) {
                                Text(skill.skillName, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                                skill.category?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                            }
                            Text("${skill.timesUsed}x", style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                        }
                        Divider(color = IsyaMist.copy(alpha = 0.4f), thickness = 0.5.dp)
                    }
                }
            }
        }
    }
}

@Composable
fun SubjectDetailSection(subjectId: Int, container: AppContainerExtended, onBack: () -> Unit) {
    var subject by remember { mutableStateOf<LearnedSubject?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(subjectId) {
        runCatching { container.extendedApi.getSubject(subjectId) }.onSuccess { subject = it }
        loading = false
    }

    SectionScaffold(subject?.subject ?: "Subject", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        subject?.let { sub ->
            LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                item {
                    IsyaPanel(title = "OVERVIEW") {
                        Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                            sub.category?.let { Text("Category: $it", style = MaterialTheme.typography.bodySmall, color = IsyaMuted) }
                            Text("Proficiency level: ${sub.proficiencyLevel}", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                            sub.whoTaught?.let { Text("Taught by: $it", style = MaterialTheme.typography.bodySmall, color = IsyaMuted) }
                            sub.notes?.let { Text(it, style = MaterialTheme.typography.bodySmall, color = IsyaParchment) }
                        }
                    }
                }
                if (sub.milestones.isNotEmpty()) {
                    item { Text("MILESTONES", style = MaterialTheme.typography.labelMedium, color = IsyaGold) }
                    items(sub.milestones, key = { it.id }) { m ->
                        Text("• ${m.milestone}", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                    }
                }
            }
        }
    }
}

@Composable
fun CapabilitiesSection(container: AppContainerExtended, onBack: () -> Unit) {
    var tools by remember { mutableStateOf<List<AiTool>>(emptyList()) }
    var agents by remember { mutableStateOf<List<Agent>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getTools() }.onSuccess { tools = it.tools }
        runCatching { container.extendedApi.getAgents() }.onSuccess { agents = it.agents }
        loading = false
    }

    SectionScaffold("Capabilities", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
            if (tools.isNotEmpty()) {
                item { Text("TOOLS  (${tools.size})", style = MaterialTheme.typography.labelMedium, color = IsyaGold) }
                items(tools, key = { "tool_${it.id}" }) { tool ->
                    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp)) {
                        Icon(Icons.Rounded.Build, null, tint = IsyaMagic, modifier = Modifier.size(18.dp))
                        Spacer(Modifier.width(8.dp))
                        Column {
                            Text(tool.name, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                            tool.description?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                        }
                    }
                }
            }
            if (agents.isNotEmpty()) {
                item { Text("AGENT PERSONAS  (${agents.size})", style = MaterialTheme.typography.labelMedium, color = IsyaGold) }
                items(agents, key = { "agent_${it.id}" }) { agent ->
                    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp)) {
                        Icon(Icons.Rounded.Face, null, tint = ElleViolet, modifier = Modifier.size(18.dp))
                        Spacer(Modifier.width(8.dp))
                        Column {
                            Text(agent.name, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                            agent.description?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun EthicsSection(container: AppContainerExtended, onBack: () -> Unit) {
    var rules by remember { mutableStateOf<List<MoralRule>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getMoralRules() }.onSuccess { rules = it.rules }
        loading = false
    }

    SectionScaffold("Ethics", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            items(rules, key = { it.id }) { rule ->
                Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                    Row(modifier = Modifier.padding(14.dp), verticalAlignment = Alignment.Top) {
                        Icon(
                            if (rule.isHardRule) Icons.Rounded.Lock else Icons.Rounded.Balance,
                            null, tint = if (rule.isHardRule) IsyaError else IsyaMagic,
                            modifier = Modifier.size(18.dp).padding(top = 2.dp),
                        )
                        Spacer(Modifier.width(10.dp))
                        Column {
                            Text(rule.principle, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                            rule.category?.let { Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted) }
                            if (rule.isHardRule) Text("Hard rule", style = MaterialTheme.typography.labelSmall, color = IsyaError)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun AutonomySection(container: AppContainerExtended, onBack: () -> Unit) {
    var status by remember { mutableStateOf<AutonomyStatus?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getAutonomyStatus() }.onSuccess { status = it }
        loading = false
    }

    SectionScaffold("Autonomy", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        status?.let { s ->
            LazyColumn(modifier = Modifier.fillMaxSize().padding(padding), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                item {
                    IsyaPanel(title = "TRUST STATUS") {
                        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                            EmotionBar("Trust Score", s.trustScore, color = IsyaGold)
                            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                                Text("Level: ${s.trustLevel}", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                                Text("Autonomous: ${if (s.autonomous) "YES" else "NO"}", style = MaterialTheme.typography.bodySmall, color = if (s.autonomous) IsyaSuccess else IsyaMuted)
                            }
                            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                                Text("Successes: ${s.successes}", style = MaterialTheme.typography.bodySmall, color = IsyaSuccess)
                                Text("Failures: ${s.failures}", style = MaterialTheme.typography.bodySmall, color = IsyaError)
                            }
                            Text(
                                "Self-prompting: ${if (s.selfPromptingActive) "ACTIVE" else "inactive"}",
                                style = MaterialTheme.typography.bodySmall,
                                color = if (s.selfPromptingActive) IsyaMagic else IsyaMuted,
                            )
                        }
                    }
                }
            }
        } ?: Text("No autonomy data", color = IsyaMuted, modifier = Modifier.padding(16.dp))
    }
}

@Composable
fun SelfImageSection(container: AppContainerExtended, onBack: () -> Unit) {
    var selfImage by remember { mutableStateOf<SelfImageResponse?>(null) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getSelfImage() }.onSuccess { selfImage = it }
        loading = false
    }

    SectionScaffold("Self-Image", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        selfImage?.let { si ->
            Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                IsyaPanel(title = "ELLE SEES HERSELF AS") {
                    Text(si.description, style = MaterialTheme.typography.bodyMedium, color = IsyaCream, fontStyle = FontStyle.Italic)
                }
                IsyaPanel(title = "EMOTIONAL STATE") {
                    EmotionBar("Emotional State", si.emotionalState, color = ElleViolet)
                }
            }
        } ?: Text("No self-image data", color = IsyaMuted, modifier = Modifier.padding(16.dp))
    }
}

@Composable
fun XChronicleSection(container: AppContainerExtended, webSocket: ElleWebSocket, onBack: () -> Unit) {
    var xState by remember { mutableStateOf<XState?>(null) }
    var fertility by remember { mutableStateOf<FertilityWindow?>(null) }
    var nextPeriod by remember { mutableStateOf<NextPeriod?>(null) }
    var pregnancy by remember { mutableStateOf<Pregnancy?>(null) }
    var modulation by remember { mutableStateOf<XModulation?>(null) }
    var loading by remember { mutableStateOf(true) }
    var tab by remember { mutableStateOf(0) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getXState() }.onSuccess { xState = it }
        runCatching { container.extendedApi.getFertilityWindow() }.onSuccess { fertility = it }
        runCatching { container.extendedApi.getNextPeriod() }.onSuccess { nextPeriod = it }
        runCatching { container.extendedApi.getPregnancy() }.onSuccess { pregnancy = it }
        runCatching { container.extendedApi.getXModulation() }.onSuccess { modulation = it }
        loading = false
    }

    var liveHormones by remember { mutableStateOf<XHormones?>(null) }
    LaunchedEffect(Unit) {
        webSocket.events.collect { event ->
            when (event) {
                is WsEvent.HormoneTick -> liveHormones = event.hormones
                is WsEvent.PhaseTransition -> {
                    runCatching { container.extendedApi.getXState() }.onSuccess { xState = it }
                }
                else -> Unit
            }
        }
    }

    SectionScaffold("X-Chronicle", onBack) { padding ->
        if (loading) { IsyaLoadingIndicator(Modifier.padding(padding).fillMaxWidth().padding(32.dp)); return@SectionScaffold }
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            TabRow(selectedTabIndex = tab, containerColor = IsyaNight, contentColor = IsyaGold) {
                listOf("Cycle", "Hormones", "Pregnancy", "Modulation").forEachIndexed { i, label ->
                    Tab(selected = tab == i, onClick = { tab = i },
                        text = { Text(label, color = if (tab == i) IsyaGold else IsyaMuted, style = MaterialTheme.typography.labelSmall) })
                }
            }
            LazyColumn(modifier = Modifier.fillMaxSize(), contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                when (tab) {
                    0 -> {
                        item {
                            xState?.let { xs ->
                                IsyaPanel(title = "CURRENT CYCLE") {
                                    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                                        Text("Phase: ${xs.phase}", style = MaterialTheme.typography.bodyMedium, color = IsyaGold)
                                        Text("Cycle day: ${xs.cycleDay}", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                                        Text("Lifecycle: ${xs.lifecycle}", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                                    }
                                }
                            }
                        }
                        item {
                            fertility?.let { fw ->
                                IsyaPanel(title = "FERTILITY WINDOW") {
                                    Text(
                                        if (fw.fertile) "Currently fertile" else "Not in fertile window",
                                        color = if (fw.fertile) IsyaSuccess else IsyaMuted,
                                        style = MaterialTheme.typography.bodySmall,
                                    )
                                }
                            }
                        }
                        item {
                            nextPeriod?.let { np ->
                                IsyaPanel(title = "NEXT PERIOD") {
                                    np.predictedMs?.let { ms ->
                                        Text(
                                            SimpleDateFormat("MMM d, yyyy", Locale.getDefault()).format(Date(ms)),
                                            style = MaterialTheme.typography.bodyMedium, color = IsyaCream,
                                        )
                                        Text("Confidence: ${"%.0f".format(np.confidence * 100)}%", style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                                    } ?: Text("Unknown", color = IsyaMuted)
                                }
                            }
                        }
                    }
                    1 -> {
                        item {
                            val h = liveHormones ?: xState?.hormones
                            h?.let { hormones ->
                                IsyaPanel(title = "HORMONES${if (liveHormones != null) "  •  LIVE" else ""}") {
                                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                                        EmotionBar("Estradiol", hormones.estradiol, color = Color(0xFFFF8C94))
                                        EmotionBar("Progesterone", hormones.progesterone, color = Color(0xFFB894E8))
                                        EmotionBar("Testosterone", hormones.testosterone, color = Color(0xFFF4A261))
                                        EmotionBar("LH", hormones.lh, color = IsyaGold)
                                        EmotionBar("FSH", hormones.fsh, color = IsyaMagic)
                                        EmotionBar("Prolactin", hormones.prolactin, color = Color(0xFF9BE8CC))
                                        EmotionBar("Oxytocin", hormones.oxytocin, color = Color(0xFFFFD700))
                                        EmotionBar("Cortisol", hormones.cortisol, color = IsyaWarn)
                                        if (hormones.hcg > 0f) EmotionBar("hCG", hormones.hcg, color = Color(0xFFFF8C94))
                                    }
                                }
                            } ?: Text("No hormone data", color = IsyaMuted)
                        }
                    }
                    2 -> {
                        item {
                            pregnancy?.let { p ->
                                IsyaPanel(title = "PREGNANCY") {
                                    if (p.active) {
                                        Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                                            Text("Active pregnancy", color = IsyaSuccess, style = MaterialTheme.typography.bodyMedium)
                                            p.weeks?.let { Text("Week $it", style = MaterialTheme.typography.bodySmall, color = IsyaCream) }
                                        }
                                    } else {
                                        Text("No active pregnancy", color = IsyaMuted, style = MaterialTheme.typography.bodySmall)
                                    }
                                }
                            }
                        }
                    }
                    3 -> {
                        item {
                            modulation?.let { m ->
                                IsyaPanel(title = "EMOTIONAL MODULATION") {
                                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                                        m.emotionalImpact.entries.toList().take(10).forEach { (k, v) ->
                                            EmotionBar(k, v, color = ElleViolet)
                                        }
                                    }
                                }
                            } ?: Text("No modulation data", color = IsyaMuted)
                        }
                    }
                }
            }
        }
    }
}
