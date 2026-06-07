package com.elleann.android.ui.memory

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
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
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.models.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.components.IsyaSearchBar
import com.elleann.android.ui.components.IsyaTopBar
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

enum class MemoryFilter(val label: String) {
    ALL("All"), EPISODIC("Episodic"), DREAMS("Dreams"),
    SEMANTIC("Semantic"), PROCEDURAL("Procedural"), IMPORTANT("Important")
}

data class MemoryBrowserState(
    val memories: List<Memory>  = emptyList(),
    val filtered: List<Memory>  = emptyList(),
    val loading: Boolean        = true,
    val refreshing: Boolean     = false,
    val error: String?          = null,
    val filter: MemoryFilter    = MemoryFilter.ALL,
    val searchQuery: String     = "",
    val lastRefreshed: Long     = 0L,
)

class MemoryBrowserViewModel(private val container: AppContainerExtended) : ViewModel() {
    private val _state = MutableStateFlow(MemoryBrowserState())
    val state: StateFlow<MemoryBrowserState> = _state.asStateFlow()

    init {
        loadThenPoll()
    }

    private fun loadThenPoll() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            fetch()
            _state.update { it.copy(loading = false) }

            if (_state.value.error == null) startPolling()
        }
    }

    private fun startPolling() {
        viewModelScope.launch {
            while (isActive) {
                delay(15_000)
                _state.update { it.copy(refreshing = true) }
                fetch()
                _state.update { it.copy(refreshing = false, lastRefreshed = System.currentTimeMillis()) }
            }
        }
    }

    private suspend fun fetch() {
        runCatching { container.extendedApi.getMemories() }
            .onSuccess { r ->

                val sorted = r.memories
                    .filter { it.shouldShow }
                    .sortedByDescending { it.createdMs }
                val filtered = applyFilter(sorted, _state.value.filter, _state.value.searchQuery)
                _state.update { it.copy(memories = sorted, filtered = filtered) }
            }
            .onFailure { e ->
                if (_state.value.memories.isEmpty()) {
                    _state.update { it.copy(error = e.message) }
                }

            }
    }

    fun setFilter(f: MemoryFilter) = _state.update { s ->
        s.copy(filter = f, filtered = applyFilter(s.memories, f, s.searchQuery))
    }

    fun search(q: String) = _state.update { s ->
        s.copy(searchQuery = q, filtered = applyFilter(s.memories, s.filter, q))
    }

    fun retry() = loadThenPoll()

    private fun applyFilter(all: List<Memory>, f: MemoryFilter, q: String): List<Memory> {
        val base = when (f) {
            MemoryFilter.ALL        -> all
            MemoryFilter.EPISODIC   -> all.filter { it.memoryType in setOf(0,1) && !it.isDream }
            MemoryFilter.DREAMS     -> all.filter { it.isDream }
            MemoryFilter.SEMANTIC   -> all.filter { it.memoryType == 2 }
            MemoryFilter.PROCEDURAL -> all.filter { it.memoryType == 3 }
            MemoryFilter.IMPORTANT  -> all.filter { it.importance > 0.7f }
        }
        return if (q.isBlank()) base
        else base.filter {
            it.content.contains(q, ignoreCase = true) ||
            it.summary?.contains(q, ignoreCase = true) == true
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MemoryBrowserScreen(
    containerExtended: AppContainerExtended,
    onOpenMemory: (Long) -> Unit,
    onBack: () -> Unit,
) {
    val vm: MemoryBrowserViewModel = viewModel(
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(c: Class<T>): T =
                    MemoryBrowserViewModel(containerExtended) as T
            }
        }
    )
    val state by vm.state.collectAsState()
    val listState = rememberLazyListState()

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("Memories", color = IsyaGold)
                        Spacer(Modifier.width(8.dp))
                        if (state.refreshing) {
                            CircularProgressIndicator(Modifier.size(14.dp), strokeWidth = 1.5.dp, color = IsyaMagic)
                        } else {
                            Text(
                                "(${state.filtered.size})",
                                style = MaterialTheme.typography.labelSmall,
                                color = IsyaMuted,
                            )
                        }
                    }
                },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {

            IsyaSearchBar(
                value         = state.searchQuery,
                onValueChange = vm::search,
                placeholder   = "Search memories…",
                modifier      = Modifier.padding(horizontal = 16.dp, vertical = 8.dp),
                leadingIcon   = Icons.Rounded.Search,
                trailingIcon  = if (state.searchQuery.isNotBlank()) {
                    { IconButton(onClick = { vm.search("") }) {
                        Icon(Icons.Rounded.Clear, "Clear", tint = IsyaMuted)
                    } }
                } else null,
            )

            LazyRow(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                items(MemoryFilter.entries.toTypedArray()) { filter ->
                    FilterChip(
                        selected = state.filter == filter,
                        onClick  = { vm.setFilter(filter) },
                        label    = { Text(filter.label) },
                        colors   = FilterChipDefaults.filterChipColors(
                            selectedContainerColor = IsyaGold,
                            selectedLabelColor     = IsyaNight,
                            containerColor         = IsyaDusk,
                            labelColor             = IsyaMuted,
                        ),
                    )
                }
            }

            Spacer(Modifier.height(4.dp))

            if (state.lastRefreshed > 0L) {
                Text(
                    "Live · updated ${SimpleDateFormat("h:mm:ss a", Locale.getDefault()).format(Date(state.lastRefreshed))}",
                    style    = MaterialTheme.typography.labelSmall,
                    color    = IsyaMuted,
                    modifier = Modifier.padding(horizontal = 16.dp),
                )
            }

            Spacer(Modifier.height(6.dp))

            when {
                state.loading -> IsyaLoadingIndicator(Modifier.fillMaxWidth().padding(32.dp))
                state.error != null && state.memories.isEmpty() ->
                    IsyaErrorState(state.error!!, onRetry = { vm.retry() }, Modifier.padding(16.dp))
                state.filtered.isEmpty() -> Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(Icons.Rounded.Memory, null, tint = IsyaMuted, modifier = Modifier.size(40.dp))
                        Spacer(Modifier.height(8.dp))
                        Text("No memories match", color = IsyaMuted)
                    }
                }
                else -> LazyColumn(
                    state           = listState,
                    modifier        = Modifier.fillMaxSize(),
                    contentPadding  = PaddingValues(horizontal = 16.dp, vertical = 4.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp),
                ) {
                    items(state.filtered, key = { it.id }) { mem ->
                        MemoryCard(mem, onClick = { onOpenMemory(mem.id) })
                    }

                    item { Spacer(Modifier.height(16.dp)) }
                }
            }
        }
    }
}

@Composable
private fun MemoryCard(memory: Memory, onClick: () -> Unit) {
    val (typeColor, typeLabel) = remember(memory.memoryType, memory.isDream) {
        when {
            memory.isDream                   -> Pair(MemoryDream, "Dream")
            memory.memoryType in setOf(0, 1) -> Pair(MemoryEpisodic, "Episodic")
            memory.memoryType == 2           -> Pair(MemorySemantic, "Semantic")
            memory.memoryType == 3           -> Pair(MemoryProcedural, "Procedural")
            else                             -> Pair(IsyaMuted, "Memory")
        }
    }

    val dateStr = remember(memory.createdMs) {
        SimpleDateFormat("MMM d, yyyy  h:mm a", Locale.getDefault()).format(Date(memory.createdMs))
    }

    val importanceBorderWidth = (2 + memory.importance * 4).dp

    Surface(
        modifier = Modifier.fillMaxWidth().clickable(onClick = onClick),
        shape    = RoundedCornerShape(10.dp),
        color    = IsyaDusk,
    ) {
        Row(modifier = Modifier.fillMaxWidth()) {

            Box(
                modifier = Modifier
                    .width(importanceBorderWidth)
                    .fillMaxHeight()
                    .background(typeColor, RoundedCornerShape(topStart = 10.dp, bottomStart = 10.dp))
            )

            Column(
                modifier = Modifier
                    .weight(1f)
                    .padding(horizontal = 12.dp, vertical = 10.dp),
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically,
                ) {

                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Box(
                            modifier = Modifier
                                .size(8.dp)
                                .clip(CircleShape)
                                .background(typeColor)
                        )
                        Spacer(Modifier.width(6.dp))
                        Text(
                            typeLabel,
                            style = MaterialTheme.typography.labelSmall,
                            color = typeColor,
                        )
                        if (memory.isDream) {
                            Spacer(Modifier.width(6.dp))
                            Text("✦", color = MemoryDream.copy(alpha = 0.7f), style = MaterialTheme.typography.labelSmall)
                        }
                    }

                    Text(
                        "imp ${"%.1f".format(memory.importance)}",
                        style = MaterialTheme.typography.labelSmall,
                        color = when {
                            memory.importance > 0.7f -> IsyaGold
                            memory.importance > 0.4f -> IsyaMuted
                            else                     -> IsyaMuted.copy(alpha = 0.5f)
                        },
                    )
                }

                Spacer(Modifier.height(6.dp))

                Text(
                    text     = memory.content,
                    style    = MaterialTheme.typography.bodySmall,
                    color    = if (memory.isDream) IsyaCream.copy(alpha = 0.85f) else IsyaCream,
                    fontStyle = if (memory.isDream) FontStyle.Italic else FontStyle.Normal,
                    maxLines = 4,
                    overflow = TextOverflow.Ellipsis,
                )

                memory.summary?.let { summary ->
                    Spacer(Modifier.height(4.dp))
                    Text(
                        summary,
                        style     = MaterialTheme.typography.labelSmall,
                        color     = IsyaParchment,
                        fontStyle = FontStyle.Italic,
                        maxLines  = 2,
                        overflow  = TextOverflow.Ellipsis,
                    )
                }

                Spacer(Modifier.height(6.dp))
                Text(dateStr, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
            }

            Icon(
                Icons.Rounded.ChevronRight, null,
                tint     = IsyaMuted.copy(alpha = 0.5f),
                modifier = Modifier.align(Alignment.CenterVertically).padding(end = 8.dp),
            )
        }
    }
}

data class MemoryDetailState(
    val memory: Memory?  = null,
    val loading: Boolean = true,
    val error: String?   = null,
)

class MemoryDetailViewModel(
    private val memoryId: Long,
    private val container: AppContainerExtended,
) : ViewModel() {
    private val _state = MutableStateFlow(MemoryDetailState())
    val state: StateFlow<MemoryDetailState> = _state.asStateFlow()
    init { load() }

    private fun load() {
        viewModelScope.launch {
            runCatching { container.extendedApi.getMemory(memoryId) }
                .onSuccess { m -> _state.update { it.copy(memory = m, loading = false) } }
                .onFailure { e -> _state.update { it.copy(error = e.message, loading = false) } }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MemoryDetailScreen(
    memoryId: Long,
    containerExtended: AppContainerExtended,
    onBack: () -> Unit,
) {
    val vm: MemoryDetailViewModel = viewModel(
        key = "mem_$memoryId",
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(c: Class<T>): T =
                    MemoryDetailViewModel(memoryId, containerExtended) as T
            }
        }
    )
    val state by vm.state.collectAsState()

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text(if (state.memory?.isDream == true) "✦ Dream Insight" else "Memory Detail", color = IsyaGold) },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.Rounded.ArrowBack, "Back", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        when {
            state.loading -> Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) { IsyaLoadingIndicator() }
            state.error != null -> IsyaErrorState(state.error!!, modifier = Modifier.padding(padding).padding(16.dp))
            state.memory != null -> {
                val mem = state.memory!!
                LazyColumn(
                    modifier = Modifier.fillMaxSize().padding(padding),
                    contentPadding = PaddingValues(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp),
                ) {
                    item {
                        IsyaPanel(title = if (mem.isDream) "✦ DREAM CONTENT" else "MEMORY CONTENT") {
                            Text(
                                mem.content,
                                style     = MaterialTheme.typography.bodyMedium,
                                color     = IsyaCream,
                                fontStyle = if (mem.isDream) FontStyle.Italic else FontStyle.Normal,
                            )
                        }
                    }
                    mem.summary?.let { s ->
                        item {
                            IsyaPanel(title = "SUMMARY") {
                                Text(s, style = MaterialTheme.typography.bodySmall, color = IsyaParchment, fontStyle = FontStyle.Italic)
                            }
                        }
                    }
                    item {
                        IsyaPanel(title = "METADATA") {
                            Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                                DetailRow("Type", if (mem.isDream) "Dream (episodic, zero-init)"
                                    else when (mem.memoryType) { 0, 1 -> "Episodic"; 2 -> "Semantic"; 3 -> "Procedural"; else -> "Unknown" })
                                DetailRow("Tier", when (mem.tier) { 0 -> "STM"; 1 -> "Buffer"; 2 -> "LTM"; 3 -> "Archive"; else -> "${mem.tier}" })
                                DetailRow("Importance", "${"%.4f".format(mem.importance)}")
                                DetailRow("Relevance",  "${"%.4f".format(mem.relevance)}")
                                DetailRow("Valence",    "${"%.4f".format(mem.emotionalValence)}")
                                DetailRow("Decay",      "${"%.4f".format(mem.decay)}")
                                DetailRow("Access count", "${mem.accessCount}")
                                mem.conversationId?.let { DetailRow("Conversation", "#$it") }
                                mem.userId?.let { DetailRow("User", "#$it") }
                                DetailRow("Created", SimpleDateFormat("MMM d, yyyy  h:mm:ss a", Locale.getDefault()).format(Date(mem.createdMs)))
                                DetailRow("Last accessed", SimpleDateFormat("MMM d, yyyy  h:mm:ss a", Locale.getDefault()).format(Date(mem.lastAccessMs)))
                                if (mem.tags.isNotEmpty()) DetailRow("Tags", mem.tags.joinToString(", "))
                            }
                        }
                    }
                    item {
                        IsyaPanel(title = "3D POSITION") {
                            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                                DetailRow("X", "${"%.2f".format(mem.positionX)}")
                                DetailRow("Y", "${"%.2f".format(mem.positionY)}")
                                DetailRow("Z", "${"%.2f".format(mem.positionZ)}")
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun DetailRow(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        Text(value, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
    }
}
