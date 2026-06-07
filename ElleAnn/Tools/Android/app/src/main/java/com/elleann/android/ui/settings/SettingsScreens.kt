package com.elleann.android.ui.settings

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import com.elleann.android.data.AdminKeyStore
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.navigation.ElleRoutes
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SettingsScaffold(title: String, onBack: () -> Unit, content: @Composable (PaddingValues) -> Unit) {
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
fun SettingsScreen(
    containerExtended: AppContainerExtended,
    onBack: () -> Unit,
    onNavigate: (String) -> Unit,
) {
    val stored = containerExtended.tokenStore.load()

    SettingsScaffold("Settings", onBack) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {

            item {
                IsyaPanel(title = "CONNECTION") {
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        SettingsRow("Server", stored?.let { "${it.host}:${it.port}" } ?: "Not paired")
                        SettingsRow("Admin key", if (containerExtended.adminKeyStore.hasKey()) "Configured ✓" else "Not set")
                    }
                }
            }
            item { SettingsNavItem("Admin Key", "Configure x-admin-key for Dev access", Icons.Rounded.Lock) { onNavigate(ElleRoutes.SETTINGS_ADMIN_KEY) } }
            item { SettingsNavItem("ColorCode", "Text accessibility settings", Icons.Rounded.Palette) { onNavigate(ElleRoutes.SETTINGS_COLOR_CODE) } }
            item { SettingsNavItem("Appearance", "Theme, font size", Icons.Rounded.FormatSize) { onNavigate(ElleRoutes.SETTINGS_APPEARANCE) } }
            item { SettingsNavItem("Notifications", "Alerts and push settings", Icons.Rounded.Notifications) { onNavigate(ElleRoutes.SETTINGS_NOTIFICATIONS) } }
            item { SettingsNavItem("About", "Version info", Icons.Rounded.Info) { onNavigate(ElleRoutes.SETTINGS_ABOUT) } }
        }
    }
}

@Composable
private fun SettingsRow(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        Text(value, style = MaterialTheme.typography.bodySmall, color = IsyaCream)
    }
}

@Composable
private fun SettingsNavItem(label: String, subtitle: String, icon: androidx.compose.ui.graphics.vector.ImageVector, onClick: () -> Unit) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(10.dp),
        color = IsyaDusk,
        onClick = onClick,
    ) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Icon(icon, null, tint = IsyaMagic, modifier = Modifier.size(22.dp))
            Spacer(Modifier.width(12.dp))
            Column(modifier = Modifier.weight(1f)) {
                Text(label, style = MaterialTheme.typography.bodyMedium, color = IsyaCream)
                Text(subtitle, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
            }
            Icon(Icons.Rounded.ChevronRight, null, tint = IsyaMuted)
        }
    }
}

@Composable
fun AdminKeySettingsScreen(adminKeyStore: AdminKeyStore, onBack: () -> Unit) {
    var key by remember { mutableStateOf(adminKeyStore.getKey()) }
    var saved by remember { mutableStateOf(false) }

    SettingsScaffold("Admin Key", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(16.dp)) {
            IsyaPanel(title = "x-admin-key") {
                Text(
                    "Enter the x-admin-key shared secret from Josh's elle_master_config.json. " +
                    "Required for Dev tab admin operations.",
                    style = MaterialTheme.typography.bodySmall,
                    color = IsyaMuted,
                )
            }
            OutlinedTextField(
                value = key,
                onValueChange = { key = it; saved = false },
                label = { Text("Admin key", color = IsyaMuted) },
                modifier = Modifier.fillMaxWidth(),
                visualTransformation = PasswordVisualTransformation(),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedTextColor = IsyaCream, unfocusedTextColor = IsyaCream,
                    focusedBorderColor = IsyaGold, unfocusedBorderColor = IsyaMist,
                    focusedContainerColor = IsyaDusk, unfocusedContainerColor = IsyaDusk,
                    cursorColor = IsyaGoldBright,
                ),
            )
            IsyaButton(
                text = if (saved) "Saved ✓" else "Save Key",
                onClick = { adminKeyStore.setKey(key); saved = true },
                variant = IsyaButtonVariant.PRIMARY_GOLD,
                modifier = Modifier.fillMaxWidth(),
            )
            if (adminKeyStore.hasKey()) {
                IsyaButton(
                    text = "Clear Key",
                    onClick = { adminKeyStore.clearKey(); key = ""; saved = false },
                    variant = IsyaButtonVariant.GHOST,
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
fun ColorCodeSettingsScreen(onBack: () -> Unit) {
    SettingsScaffold("ColorCode", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
            IsyaPanel(title = "ABOUT COLORCODE") {
                Text(
                    "ColorCode colors text by grammatical role — nouns, verbs, adjectives, " +
                    "and more — or by semantic/emotional weight. Designed for Irlen syndrome " +
                    "and reading fatigue. Toggle mode in any chat via the palette icon.",
                    style = MaterialTheme.typography.bodySmall,
                    color = IsyaCream,
                )
            }
            IsyaPanel(title = "GRAMMAR LEGEND") {
                Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    com.elleann.android.colorcode.ColorCodeEngine.grammarLegend.forEach { (label, color) ->
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Box(
                                Modifier.size(12.dp).background(color, CircleShape)
                            )
                            Spacer(Modifier.width(10.dp))
                            Text(label, style = MaterialTheme.typography.bodySmall, color = color)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun AppearanceScreen(onBack: () -> Unit) {
    SettingsScaffold("Appearance", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
            IsyaPanel(title = "THEME") {
                Text("Isya Night — the only theme worthy of Elle.", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
            }
        }
    }
}

@Composable
fun NotificationsScreen(onBack: () -> Unit) {
    SettingsScaffold("Notifications", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp)) {
            Text("Notification settings will appear here.", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        }
    }
}

@Composable
fun SettingsAboutScreen(onBack: () -> Unit) {
    SettingsScaffold("About", onBack) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            IsyaPanel(title = "ElleAnn Companion") {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    SettingsRow("Version", "1.0.0")
                    SettingsRow("Server", "ESI v3.0")
                    SettingsRow("Services", "19 Windows services + 5 ASM DLLs")
                    SettingsRow("Emotion dims", "102")
                    SettingsRow("REST routes", "123")
                    SettingsRow("Apache endpoints", "10 (Named Pipes SQL)")
                }
            }
        }
    }
}
