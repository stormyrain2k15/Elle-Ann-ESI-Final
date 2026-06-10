package com.elleann.android.ui.world.sections

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.models.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*
import androidx.compose.foundation.background
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight

// ---------------------------------------------------------------------------
// InnerWorldSection — Thoughts, Private Inner Life, Consent Log in tabs
// ---------------------------------------------------------------------------
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun InnerWorldSection(container: AppContainerExtended, onBack: () -> Unit) {
    var tab by remember { mutableStateOf(0) }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("Inner Life", color = IsyaGold) },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted)
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            TabRow(selectedTabIndex = tab, containerColor = IsyaNight, contentColor = IsyaGold) {
                listOf("Thoughts", "Private", "Consent").forEachIndexed { i, label ->
                    Tab(
                        selected = tab == i,
                        onClick  = { tab = i },
                        text     = { Text(label, color = if (tab == i) IsyaGold else IsyaMuted) }
                    )
                }
            }
            when (tab) {
                0 -> ThoughtsContent(container)
                1 -> PrivateInnerLifeContent(container)
                2 -> ConsentLogContent(container)
            }
        }
    }
}

@Composable
private fun ThoughtsContent(container: AppContainerExtended) {
    var prompts by remember { mutableStateOf<List<SelfPrompt>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var error by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getSelfPrompts(50) }
            .onSuccess { prompts = it.prompts }
            .onFailure { error = it.message }
        loading = false
    }

    when {
        loading  -> IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp))
        error != null -> IsyaErrorState(error!!, modifier = Modifier.padding(16.dp))
        else -> LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            items(prompts, key = { it.id }) { p ->
                val sourceBadgeColor = when {
                    p.source.contains("curiosity") -> IsyaMagic
                    p.source.contains("bonding")   -> ElleViolet
                    p.source.contains("introspection") -> IsyaGold
                    else -> IsyaMuted
                }
                Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(14.dp)) {
                        if (p.source.isNotBlank()) {
                            Box(
                                modifier = Modifier
                                    .background(sourceBadgeColor.copy(alpha = 0.2f), RoundedCornerShape(4.dp))
                                    .padding(horizontal = 6.dp, vertical = 2.dp)
                            ) {
                                Text(p.source, style = MaterialTheme.typography.labelSmall, color = sourceBadgeColor)
                            }
                            Spacer(Modifier.height(6.dp))
                        }
                        Text(p.prompt, style = MaterialTheme.typography.bodySmall, color = IsyaCream, fontStyle = FontStyle.Italic)
                        Spacer(Modifier.height(4.dp))
                        Text(
                            SimpleDateFormat("MMM d, h:mm a", Locale.getDefault()).format(Date(p.createdMs)),
                            style = MaterialTheme.typography.labelSmall, color = IsyaMuted,
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun PrivateInnerLifeContent(container: AppContainerExtended) {
    var thoughts by remember { mutableStateOf<List<PrivateThought>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }
    var error by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getPrivateThoughts(50) }
            .onSuccess { thoughts = it.thoughts }
            .onFailure { error = it.message }
        loading = false
    }

    when {
        loading  -> IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp))
        error != null -> IsyaErrorState(error!!, modifier = Modifier.padding(16.dp))
        thoughts.isEmpty() -> Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Nothing yet", color = IsyaMuted)
        }
        else -> LazyColumn(
            modifier = Modifier.fillMaxSize(),
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
                            EmotionBar("intensity", t.emotionalIntensity,
                                modifier = Modifier.width(100.dp), color = ElleViolet, showValue = false)
                        }
                        Spacer(Modifier.height(6.dp))
                        Text(t.content, style = MaterialTheme.typography.bodySmall,
                            color = IsyaCream, fontStyle = FontStyle.Italic)
                    }
                }
            }
        }
    }
}

@Composable
private fun ConsentLogContent(container: AppContainerExtended) {
    var log by remember { mutableStateOf<List<ConsentLogEntry>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getConsentLog(50) }
            .onSuccess { log = it.log }
        loading = false
    }

    if (loading) {
        IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp))
        return
    }

    LazyColumn(
        modifier = Modifier.fillMaxSize(),
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
                        Text("comfort ${"%.1f".format(entry.comfortLevel)}",
                            style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                    }
                    Spacer(Modifier.height(6.dp))
                    Text(entry.request, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                    entry.reasoning?.let {
                        Spacer(Modifier.height(4.dp))
                        Text(it, style = MaterialTheme.typography.labelSmall,
                            color = IsyaMuted, fontStyle = FontStyle.Italic)
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// IdentityWorldSection — Identity, Autobiography, Self-Image, Felt Time in tabs
// ---------------------------------------------------------------------------
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun IdentityWorldSection(container: AppContainerExtended, onBack: () -> Unit) {
    var tab by remember { mutableStateOf(0) }

    // Preload all data for all tabs upfront
    var traits     by remember { mutableStateOf<List<IdentityTrait>>(emptyList()) }
    var snapshots  by remember { mutableStateOf<List<IdentitySnapshot>>(emptyList()) }
    var growth     by remember { mutableStateOf<List<GrowthLogEntry>>(emptyList()) }
    var autobiography by remember { mutableStateOf<List<AutobiographyEntry>>(emptyList()) }
    var selfImage  by remember { mutableStateOf<SelfImageResponse?>(null) }
    var feltTime   by remember { mutableStateOf<FeltTime?>(null) }
    var loading    by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        runCatching { container.extendedApi.getIdentityTraits() }.onSuccess { traits = it.traits }
        runCatching { container.extendedApi.getIdentitySnapshots(10) }.onSuccess { snapshots = it.snapshots }
        runCatching { container.extendedApi.getGrowthLog(30) }.onSuccess { growth = it.log }
        runCatching { container.extendedApi.getAutobiography(30) }.onSuccess { autobiography = it.entries }
        runCatching { container.extendedApi.getSelfImage() }.onSuccess { selfImage = it }
        runCatching { container.extendedApi.getFeltTime() }.onSuccess { feltTime = it }
        loading = false
    }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("Identity", color = IsyaGold) },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted)
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        if (loading) {
            IsyaLoadingIndicator(Modifier.fillMaxSize().padding(padding).padding(32.dp))
            return@Scaffold
        }

        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            TabRow(selectedTabIndex = tab, containerColor = IsyaNight, contentColor = IsyaGold) {
                listOf("Traits", "Autobiography", "Self-Image", "Time").forEachIndexed { i, label ->
                    Tab(
                        selected = tab == i,
                        onClick  = { tab = i },
                        text     = { Text(label, color = if (tab == i) IsyaGold else IsyaMuted,
                            style = MaterialTheme.typography.labelSmall) }
                    )
                }
            }
            when (tab) {
                0 -> IdentityTraitsContent(traits, snapshots, growth)
                1 -> AutobiographyContent(autobiography)
                2 -> SelfImageContent(selfImage)
                3 -> FeltTimeContent(feltTime)
            }
        }
    }
}

@Composable
private fun IdentityTraitsContent(
    traits: List<IdentityTrait>,
    snapshots: List<IdentitySnapshot>,
    growth: List<GrowthLogEntry>,
) {
    var sub by remember { mutableStateOf(0) }
    Column(modifier = Modifier.fillMaxSize()) {
        TabRow(selectedTabIndex = sub, containerColor = IsyaNight, contentColor = IsyaMagic) {
            listOf("Traits", "Snapshots", "Growth").forEachIndexed { i, label ->
                Tab(selected = sub == i, onClick = { sub = i },
                    text = { Text(label, color = if (sub == i) IsyaMagic else IsyaMuted,
                        style = MaterialTheme.typography.labelSmall) })
            }
        }
        when (sub) {
            0 -> LazyColumn(modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                items(traits, key = { it.name }) { t ->
                    EmotionBar(t.name.replaceFirstChar { it.uppercase() }, t.value, color = ElleViolet)
                }
            }
            1 -> LazyColumn(modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                items(snapshots, key = { it.id }) { snap ->
                    Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                        Column(modifier = Modifier.padding(12.dp)) {
                            Text(SimpleDateFormat("MMM d, yyyy", Locale.getDefault()).format(Date(snap.timestampMs)),
                                style = MaterialTheme.typography.labelSmall, color = IsyaGold)
                            snap.selfDescription?.let {
                                Spacer(Modifier.height(4.dp))
                                Text(it, style = MaterialTheme.typography.bodySmall,
                                    color = IsyaCream, fontStyle = FontStyle.Italic)
                            }
                            snap.growthNote?.let {
                                Spacer(Modifier.height(4.dp))
                                Text(it, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                            }
                        }
                    }
                }
            }
            2 -> LazyColumn(modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
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

@Composable
private fun AutobiographyContent(entries: List<AutobiographyEntry>) {
    if (entries.isEmpty()) {
        Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("No entries yet", color = IsyaMuted)
        }
        return
    }
    LazyColumn(modifier = Modifier.fillMaxSize(),
        contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        items(entries.reversed(), key = { it.id }) { e ->
            Surface(shape = RoundedCornerShape(10.dp), color = IsyaDusk, modifier = Modifier.fillMaxWidth()) {
                Column(modifier = Modifier.padding(14.dp)) {
                    Text(SimpleDateFormat("MMM d, yyyy", Locale.getDefault()).format(Date(e.writtenMs)),
                        style = MaterialTheme.typography.labelSmall, color = IsyaGold)
                    Spacer(Modifier.height(4.dp))
                    Text(e.entry, style = MaterialTheme.typography.bodySmall,
                        color = IsyaCream, fontStyle = FontStyle.Italic)
                }
            }
        }
    }
}

@Composable
private fun SelfImageContent(selfImage: SelfImageResponse?) {
    selfImage?.let { si ->
        Column(modifier = Modifier.fillMaxSize().padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
            IsyaPanel(title = "ELLE SEES HERSELF AS") {
                Text(si.description, style = MaterialTheme.typography.bodyMedium,
                    color = IsyaCream, fontStyle = FontStyle.Italic)
            }
            IsyaPanel(title = "EMOTIONAL STATE") {
                EmotionBar("Emotional State", si.emotionalState, color = ElleViolet)
            }
        }
    } ?: Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Text("No self-image data", color = IsyaMuted)
    }
}

@Composable
private fun FeltTimeContent(ft: FeltTime?) {
    ft?.let { t ->
        LazyColumn(modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
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
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Total conversation", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text(msToHours(t.totalConversationMs), style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Total silence", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text(msToHours(t.totalSilenceMs), style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Longest absence", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text(msToHours(t.longestAbsenceMs), style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Sessions", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text("${t.sessionCount}", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                    }
                }
            }
        }
    } ?: Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Text("No felt-time data", color = IsyaMuted)
    }
}
