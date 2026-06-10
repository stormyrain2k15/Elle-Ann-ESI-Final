package com.elleann.android.ui.settings

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.ui.unit.sp
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
    val context = androidx.compose.ui.platform.LocalContext.current
    val prefs = remember {
        context.getSharedPreferences("elle_appearance", android.content.Context.MODE_PRIVATE)
    }

    var ttsSpeed by remember { mutableStateOf(prefs.getFloat("tts_speed", 1.0f)) }
    var ttsPitch  by remember { mutableStateOf(prefs.getFloat("tts_pitch", 1.0f)) }
    var bubbleFontSize by remember { mutableStateOf(prefs.getInt("bubble_font_size", 14)) }

    fun save() {
        prefs.edit()
            .putFloat("tts_speed", ttsSpeed)
            .putFloat("tts_pitch", ttsPitch)
            .putInt("bubble_font_size", bubbleFontSize)
            .apply()
    }

    SettingsScaffold("Appearance", onBack) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {
            IsyaPanel(title = "THEME") {
                Text("Isya Night — dark, minimal, built for Elle.",
                    style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
            }

            IsyaPanel(title = "TEXT TO SPEECH") {
                Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Column {
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Reading speed", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text("${"%.1f".format(ttsSpeed)}×", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                        Slider(
                            value = ttsSpeed,
                            onValueChange = { ttsSpeed = it; save() },
                            valueRange = 0.5f..2.0f,
                            steps = 5,
                            colors = SliderDefaults.colors(thumbColor = IsyaGold, activeTrackColor = IsyaGold),
                        )
                    }
                    Column {
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("Pitch", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                            Text("${"%.1f".format(ttsPitch)}×", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                        }
                        Slider(
                            value = ttsPitch,
                            onValueChange = { ttsPitch = it; save() },
                            valueRange = 0.5f..2.0f,
                            steps = 5,
                            colors = SliderDefaults.colors(thumbColor = IsyaGold, activeTrackColor = IsyaGold),
                        )
                    }
                }
            }

            IsyaPanel(title = "CHAT BUBBLES") {
                Column {
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                        Text("Message font size", style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                        Text("${bubbleFontSize}sp", style = MaterialTheme.typography.bodySmall, color = IsyaCream)
                    }
                    Slider(
                        value = bubbleFontSize.toFloat(),
                        onValueChange = { bubbleFontSize = it.toInt(); save() },
                        valueRange = 11f..20f,
                        steps = 8,
                        colors = SliderDefaults.colors(thumbColor = IsyaGold, activeTrackColor = IsyaGold),
                    )
                    Text(
                        "Preview: This is how Elle's responses will appear in chat.",
                        color = IsyaCream,
                        fontSize = bubbleFontSize.sp,
                        style = MaterialTheme.typography.bodySmall,
                    )
                }
            }
        }
    }
}

@Composable
fun NotificationsScreen(
    containerExtended: com.elleann.android.data.AppContainerExtended,
    onBack: () -> Unit,
) {
    val context = androidx.compose.ui.platform.LocalContext.current
    val prefs = remember {
        context.getSharedPreferences("elle_notifications", android.content.Context.MODE_PRIVATE)
    }

    var sessionGreeting by remember { mutableStateOf(prefs.getBoolean("session_greeting", true)) }
    var emotionAlerts   by remember { mutableStateOf(prefs.getBoolean("emotion_alerts", false)) }
    var thoughtNotifs   by remember { mutableStateOf(prefs.getBoolean("thought_notifs", false)) }

    fun save() {
        prefs.edit()
            .putBoolean("session_greeting", sessionGreeting)
            .putBoolean("emotion_alerts", emotionAlerts)
            .putBoolean("thought_notifs", thoughtNotifs)
            .apply()
    }

    SettingsScaffold("Notifications", onBack) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            IsyaPanel(title = "ELLE ACTIVITY") {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    NotifToggleRow(
                        label    = "Session greeting",
                        subtitle = "Show Elle's greeting when you open the app",
                        checked  = sessionGreeting,
                        onChange = { sessionGreeting = it; save() },
                    )
                    Divider(color = IsyaMist.copy(alpha = 0.3f), thickness = 0.5.dp)
                    NotifToggleRow(
                        label    = "Emotional shift alerts",
                        subtitle = "Notify when Elle's emotional state changes significantly",
                        checked  = emotionAlerts,
                        onChange = { emotionAlerts = it; save() },
                    )
                    Divider(color = IsyaMist.copy(alpha = 0.3f), thickness = 0.5.dp)
                    NotifToggleRow(
                        label    = "Autonomous thoughts",
                        subtitle = "Notify when Elle generates a self-prompt",
                        checked  = thoughtNotifs,
                        onChange = { thoughtNotifs = it; save() },
                    )
                }
            }
            Text(
                "Notifications require the app to be running in the background " +
                "and connected to the Elle server.",
                style = MaterialTheme.typography.bodySmall,
                color = IsyaMuted,
                modifier = Modifier.padding(horizontal = 4.dp),
            )
        }
    }
}

@Composable
private fun NotifToggleRow(label: String, subtitle: String, checked: Boolean, onChange: (Boolean) -> Unit) {
    Row(
        modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(label, style = MaterialTheme.typography.bodyMedium, color = IsyaCream)
            Text(subtitle, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
        }
        Switch(
            checked = checked,
            onCheckedChange = onChange,
            colors = SwitchDefaults.colors(
                checkedThumbColor = IsyaGold,
                checkedTrackColor = IsyaGold.copy(alpha = 0.4f),
            ),
        )
    }
}

@Composable
fun SettingsAboutScreen(
    containerExtended: com.elleann.android.data.AppContainerExtended,
    onBack: () -> Unit,
) {
    var serverStatus by remember { mutableStateOf<com.elleann.android.data.models.ServerStatus?>(null) }
    var routeCount   by remember { mutableStateOf<Int?>(null) }
    var hbCount      by remember { mutableStateOf<Int?>(null) }
    var dimCount     by remember { mutableStateOf<Int?>(null) }

    LaunchedEffect(Unit) {
        runCatching { containerExtended.extendedApi.getServerStatus() }
            .onSuccess { serverStatus = it }
        runCatching { containerExtended.extendedApi.getDiagRoutes() }
            .onSuccess { routeCount = it.count }
        runCatching { containerExtended.extendedApi.getDiagHeartbeats() }
            .onSuccess { hbCount = it.count }
        runCatching { containerExtended.extendedApi.getEmotionDimensions() }
            .onSuccess { dimCount = it.dimensions.size }
    }

    SettingsScaffold("About", onBack) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            IsyaPanel(title = "ElleAnn Companion") {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    SettingsRow("App version", "1.0.0")
                    SettingsRow("Server version", serverStatus?.version?.ifBlank { "—" } ?: "—")
                    SettingsRow("Services", hbCount?.toString() ?: "—")
                    SettingsRow("Emotion dims", dimCount?.toString() ?: "—")
                    SettingsRow("REST routes", routeCount?.toString() ?: "—")
                    serverStatus?.let { ss ->
                        SettingsRow("Uptime", "${ss.uptimeMs / 1000}s")
                        SettingsRow("Requests handled", ss.requestCount.toString())
                    }
                }
            }
        }
    }
}
