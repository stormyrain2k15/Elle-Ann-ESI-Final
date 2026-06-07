package com.elleann.android.ui.chat

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Add
import androidx.compose.material.icons.rounded.Chat
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.models.Conversation
import com.elleann.android.data.models.NewConversationRequest
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

data class ConvListState(
    val conversations: List<Conversation> = emptyList(),
    val loading: Boolean = true,
    val error: String? = null,
    val creating: Boolean = false,
)

class ConversationListViewModel(
    private val container: AppContainerExtended,
) : ViewModel() {
    private val _state = MutableStateFlow(ConvListState())
    val state: StateFlow<ConvListState> = _state.asStateFlow()

    init { load() }

    private fun load() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            runCatching { container.extendedApi.getConversations() }
                .onSuccess { r -> _state.update { it.copy(conversations = r.conversations, loading = false) } }
                .onFailure { e -> _state.update { it.copy(error = e.message, loading = false) } }
        }
    }

    fun newConversation(onCreated: (Long) -> Unit) {
        viewModelScope.launch {
            _state.update { it.copy(creating = true) }
            runCatching {
                container.extendedApi.createConversation(NewConversationRequest())
            }.onSuccess { conv ->
                _state.update { it.copy(creating = false) }
                onCreated(conv.id)
            }.onFailure {
                _state.update { it.copy(creating = false) }
            }
        }
    }

    fun retry() = load()
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ConversationListScreen(
    containerExtended: AppContainerExtended,
    onOpenConversation: (Long) -> Unit,
    onSettings: () -> Unit,
) {
    val vm: ConversationListViewModel = viewModel(
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(modelClass: Class<T>): T =
                    ConversationListViewModel(containerExtended) as T
            }
        }
    )
    val state by vm.state.collectAsState()

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("Chat", color = IsyaGold) },
                actions = {
                    IconButton(onClick = onSettings) {
                        Icon(Icons.Rounded.Settings, "Settings", tint = IsyaMuted)
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        },
        floatingActionButton = {
            FloatingActionButton(
                onClick          = { vm.newConversation(onOpenConversation) },
                containerColor   = IsyaGold,
                contentColor     = IsyaNight,
            ) {
                if (state.creating) {
                    CircularProgressIndicator(Modifier.size(20.dp), strokeWidth = 2.dp, color = IsyaNight)
                } else {
                    Icon(Icons.Rounded.Add, "New conversation")
                }
            }
        }
    ) { padding ->
        when {
            state.loading -> Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                IsyaLoadingIndicator()
            }
            state.error != null -> Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                IsyaErrorState(state.error!!, onRetry = { vm.retry() })
            }
            state.conversations.isEmpty() -> Box(
                Modifier.fillMaxSize().padding(padding), contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Icon(Icons.Rounded.Chat, null, tint = IsyaMuted, modifier = Modifier.size(48.dp))
                    Spacer(Modifier.height(12.dp))
                    Text("No conversations yet", color = IsyaMuted, style = MaterialTheme.typography.bodyMedium)
                    Text("Tap + to start talking to Elle", color = IsyaSubtle, style = MaterialTheme.typography.bodySmall)
                }
            }
            else -> LazyColumn(
                modifier            = Modifier.fillMaxSize().padding(padding),
                contentPadding      = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                items(state.conversations, key = { it.id }) { conv ->
                    ConversationItem(conv, onClick = { onOpenConversation(conv.id) })
                }
            }
        }
    }
}

@Composable
private fun ConversationItem(conv: Conversation, onClick: () -> Unit) {
    val dateStr = conv.lastMessageMs?.let {
        SimpleDateFormat("MMM d, h:mm a", Locale.getDefault()).format(Date(it))
    } ?: ""

    Surface(
        modifier      = Modifier.fillMaxWidth().clickable(onClick = onClick),
        shape         = androidx.compose.foundation.shape.RoundedCornerShape(12.dp),
        color         = IsyaDusk,
        tonalElevation = 0.dp,
    ) {
        Row(
            modifier          = Modifier.padding(16.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Icon(Icons.Rounded.Chat, null, tint = IsyaMagic, modifier = Modifier.size(24.dp))
            Spacer(Modifier.width(12.dp))
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text     = conv.title ?: "Conversation #${conv.id}",
                    style    = MaterialTheme.typography.bodyMedium,
                    color    = IsyaCream,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                if (dateStr.isNotBlank()) {
                    Text(dateStr, style = MaterialTheme.typography.labelSmall, color = IsyaMuted)
                }
            }
            Text(
                "${conv.messageCount}",
                style = MaterialTheme.typography.labelSmall,
                color = IsyaGold,
            )
        }
    }
}

private val IsyaSubtle = androidx.compose.ui.graphics.Color(0xFF4A6A80)
