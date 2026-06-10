package com.elleann.android.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.AutoAwesome
import androidx.compose.material.icons.rounded.Build
import androidx.compose.material.icons.rounded.Chat
import androidx.compose.material.icons.rounded.Explore
import androidx.compose.material.icons.rounded.Memory
import androidx.compose.ui.graphics.vector.ImageVector

sealed class TopLevelDestination(
    val route: String,
    val icon: ImageVector,
    val label: String,
) {
    data object Elle : TopLevelDestination("elle",   Icons.Rounded.AutoAwesome, "Elle")
    data object Chat : TopLevelDestination("chat",   Icons.Rounded.Chat,        "Chat")
    data object Memory : TopLevelDestination("memory", Icons.Rounded.Memory,    "Memory")
    data object World : TopLevelDestination("world", Icons.Rounded.Explore,     "World")
    data object Dev : TopLevelDestination("dev",     Icons.Rounded.Build,       "Dev")

    companion object {
        val all: List<TopLevelDestination> = listOf(Elle, Chat, Memory, World, Dev)
    }
}

object ElleRoutes {
    const val ELLE   = "elle"
    const val CHAT   = "chat"
    const val MEMORY = "memory"
    const val WORLD  = "world"
    const val DEV    = "dev"

    // Chat
    const val CHAT_SCREEN = "chat/conversation/{conversationId}"
    fun chatScreen(conversationId: Long) = "chat/conversation/$conversationId"

    // Memory
    const val MEMORY_BROWSER = "memory/browser"
    const val MEMORY_DETAIL  = "memory/detail/{memoryId}"
    fun memoryDetail(memoryId: Long) = "memory/detail/$memoryId"

    // World — grouped sections
    const val WORLD_GOALS         = "world/goals"
    const val WORLD_INNER         = "world/inner"         // Thoughts + Private Inner Life + Consent
    const val WORLD_IDENTITY      = "world/identity"      // Identity + Autobiography + Self-Image + Felt Time
    const val WORLD_OBSERVATORY   = "world/observatory"
    const val WORLD_PATTERNS      = "world/patterns"
    const val WORLD_LEARNING      = "world/learning"
    const val WORLD_SUBJECT_DETAIL = "world/learning/subject/{subjectId}"
    const val WORLD_X_CHRONICLE   = "world/x-chronicle"
    fun worldSubjectDetail(subjectId: Int) = "world/learning/subject/$subjectId"

    // Dev
    const val DEV_HEALTH        = "dev/health"
    const val DEV_LOGS          = "dev/logs"
    const val DEV_SERVICES      = "dev/services"
    const val DEV_DIAGNOSTICS   = "dev/diagnostics"
    const val DEV_API_EXPLORER  = "dev/api-explorer"
    const val DEV_HARDWARE      = "dev/hardware"
    const val DEV_MODELS        = "dev/models"
    const val DEV_AGENTS        = "dev/agents"
    const val DEV_TOOLS         = "dev/tools"
    const val DEV_DICTIONARY    = "dev/dictionary"
    const val DEV_MEMORY_ADMIN  = "dev/memory"
    const val DEV_BACKUPS       = "dev/backups"
    const val DEV_CONFIG        = "dev/config"
    const val DEV_DEVICES       = "dev/devices"
    const val DEV_LEARNING_ADMIN = "dev/learning"
    const val DEV_ETHICS_ADMIN  = "dev/ethics"
    const val DEV_SHN_EDITOR    = "dev/shn-editor"

    // Settings
    const val SETTINGS              = "settings"
    const val SETTINGS_ADMIN_KEY    = "settings/admin-key"
    const val SETTINGS_COLOR_CODE   = "settings/color-code"
    const val SETTINGS_APPEARANCE   = "settings/appearance"
    const val SETTINGS_NOTIFICATIONS = "settings/notifications"
    const val SETTINGS_ABOUT        = "settings/about"
}
