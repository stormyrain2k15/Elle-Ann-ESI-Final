package com.elleann.android.navigation

import androidx.compose.foundation.layout.*
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.NavHostController
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.elleann.android.AppContainer
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.PairingPayload
import com.elleann.android.ui.chat.ChatScreen
import com.elleann.android.ui.chat.ConversationListScreen
import com.elleann.android.ui.chat.VideoCallScreen
import com.elleann.android.ui.common.ConnectionNotReadyScreen
import com.elleann.android.ui.dev.*
import com.elleann.android.ui.components.*
import com.elleann.android.ui.elle.ElleHomeScreen
import com.elleann.android.ui.memory.MemoryBrowserScreen
import com.elleann.android.ui.memory.MemoryDetailScreen
import com.elleann.android.ui.shneditor.SHNScreen
import com.elleann.android.ui.settings.*
import com.elleann.android.ui.world.*
import com.elleann.android.ui.world.sections.*
import com.elleann.android.data.models.ShnSaveRequest
import kotlinx.coroutines.launch
import android.util.Base64 as AndroidBase64

@Composable
fun ElleNavHost(
    container: AppContainer,
    containerExtended: AppContainerExtended,
    isPaired: Boolean,
    onPaired: () -> Unit,
    onUnpair: () -> Unit,
    prefill: PairingPayload? = null,
) {
    val navController = rememberNavController()

    NavHost(
        navController    = navController,
        startDestination = ElleRoutes.ELLE,
    ) {

        composable(ElleRoutes.ELLE) {
            MainScaffold(
                container         = container,
                containerExtended = containerExtended,
                onUnpair          = onUnpair,
                onNavigateToSettings = {
                    navController.navigate(ElleRoutes.SETTINGS)
                },
            )
        }

        composable(ElleRoutes.SETTINGS) {
            SettingsScreen(
                containerExtended = containerExtended,
                onBack = { navController.popBackStack() },
                onNavigate = { route -> navController.navigate(route) },
            )
        }
        composable(ElleRoutes.SETTINGS_ADMIN_KEY) {
            AdminKeySettingsScreen(
                adminKeyStore = containerExtended.adminKeyStore,
                onBack = { navController.popBackStack() },
            )
        }
        composable(ElleRoutes.SETTINGS_COLOR_CODE) {
            ColorCodeSettingsScreen(onBack = { navController.popBackStack() })
        }
        composable(ElleRoutes.SETTINGS_APPEARANCE) {
            AppearanceScreen(onBack = { navController.popBackStack() })
        }
        composable(ElleRoutes.SETTINGS_NOTIFICATIONS) {
            NotificationsScreen(onBack = { navController.popBackStack() })
        }
        composable(ElleRoutes.SETTINGS_ABOUT) {
            SettingsAboutScreen(onBack = { navController.popBackStack() })
        }
    }
}

@Composable
private fun MainScaffold(
    container: AppContainer,
    containerExtended: AppContainerExtended,
    onUnpair: () -> Unit,
    onNavigateToSettings: () -> Unit,
) {
    val navController = rememberNavController()
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentDestination = navBackStackEntry?.destination

    var devUnlocked by remember { mutableStateOf(!containerExtended.hasDevPin()) }

    Scaffold(
        modifier = Modifier.fillMaxSize(),
        bottomBar = {
            val currentRoute = currentDestination?.route ?: ""
            IsyaBottomNav(
                items = TopLevelDestination.all.map { d ->
                    IsyaNavItem(label = d.label, icon = d.icon, route = d.route)
                },
                currentRoute    = currentRoute,
                onItemSelected  = { item ->
                    val dest = TopLevelDestination.all.first { it.route == item.route }
                    if (dest is TopLevelDestination.Dev && !devUnlocked) {
                        navController.navigate("${dest.route}_pin")
                    } else {
                        navController.navigate(dest.route) {
                            popUpTo(navController.graph.findStartDestination().id) {
                                saveState = true
                            }
                            launchSingleTop = true
                            restoreState    = true
                        }
                    }
                },
                modifier = Modifier.navigationBarsPadding(),
            )
        }
    ) { paddingValues ->
        TabNavHost(
            navController     = navController,
            container         = container,
            containerExtended = containerExtended,
            onUnpair          = onUnpair,
            onSettings        = onNavigateToSettings,
            devUnlocked       = devUnlocked,
            onDevUnlocked     = { devUnlocked = true },
            modifier          = Modifier.padding(paddingValues),
        )
    }
}

@Composable
private fun TabNavHost(
    navController: NavHostController,
    container: AppContainer,
    containerExtended: AppContainerExtended,
    onUnpair: () -> Unit,
    onSettings: () -> Unit,
    devUnlocked: Boolean,
    onDevUnlocked: () -> Unit,
    modifier: Modifier = Modifier,
) {
    NavHost(
        navController    = navController,
        startDestination = ElleRoutes.ELLE,
        modifier         = modifier,
    ) {

        composable(ElleRoutes.ELLE) {
            val ws = containerExtended.webSocketOrNull
            if (ws == null) {

                ConnectionNotReadyScreen(onRetry = { containerExtended.reconnectWebSocketIfNeeded() })
                return@composable
            }
            ElleHomeScreen(
                containerExtended = containerExtended,
                webSocket         = ws,
                onSettings        = onSettings,
                onUnpair          = onUnpair,
            )
        }

        composable(ElleRoutes.CHAT) {
            ConversationListScreen(
                containerExtended = containerExtended,
                onOpenConversation = { convId ->
                    navController.navigate(ElleRoutes.chatScreen(convId))
                },
                onSettings = onSettings,
            )
        }
        composable(
            route     = ElleRoutes.CHAT_SCREEN,
            arguments = listOf(navArgument("conversationId") { type = NavType.LongType }),
        ) { entry ->
            val conversationId = entry.arguments?.getLong("conversationId") ?: 1L
            ChatScreen(
                conversationId    = conversationId,
                containerExtended = containerExtended,
                onBack            = { navController.popBackStack() },
                onStartVideoCall  = { callId ->
                    navController.navigate(ElleRoutes.videoCall(callId))
                },
            )
        }
        composable(
            route     = ElleRoutes.VIDEO_CALL,
            arguments = listOf(navArgument("callId") { type = NavType.LongType }),
        ) { entry ->
            val callId = entry.arguments?.getLong("callId") ?: 0L
            VideoCallScreen(
                callId            = callId,
                containerExtended = containerExtended,
                restBaseUrl       = containerExtended.restBaseUrl ?: "",
                onBack            = { navController.popBackStack() },
            )
        }

        composable(ElleRoutes.MEMORY) {
            MemoryBrowserScreen(
                containerExtended = containerExtended,
                onOpenMemory      = { memoryId ->
                    navController.navigate(ElleRoutes.memoryDetail(memoryId))
                },
                onBack = { navController.popBackStack() },
            )
        }
        composable(ElleRoutes.MEMORY_BROWSER) {
            MemoryBrowserScreen(
                containerExtended = containerExtended,
                onOpenMemory = { memoryId ->
                    navController.navigate(ElleRoutes.memoryDetail(memoryId))
                },
                onBack = { navController.popBackStack() },
            )
        }
        composable(
            route     = ElleRoutes.MEMORY_DETAIL,
            arguments = listOf(navArgument("memoryId") { type = NavType.LongType }),
        ) { entry ->
            val memoryId = entry.arguments?.getLong("memoryId") ?: 0L
            MemoryDetailScreen(
                memoryId          = memoryId,
                containerExtended = containerExtended,
                onBack            = { navController.popBackStack() },
            )
        }

        composable(ElleRoutes.WORLD) {
            WorldScreen(
                containerExtended = containerExtended,
                onNavigate        = { route -> navController.navigate(route) },
                onSettings        = onSettings,
            )
        }

        composable(ElleRoutes.WORLD_GOALS)         { GoalsSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_THOUGHTS)      { ThoughtsSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_INNER_LIFE)    { PrivateInnerLifeSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_AUTOBIOGRAPHY) { AutobiographySection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_IDENTITY)      { IdentitySection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_FELT_TIME)     { FeltTimeSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_CONSENT)       { ConsentLogSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_OBSERVATORY)   {
            val ws = containerExtended.webSocketOrNull
            if (ws == null) {
                ConnectionNotReadyScreen(onRetry = { containerExtended.reconnectWebSocketIfNeeded() })
                return@composable
            }
            ObservatorySection(containerExtended, ws, { navController.popBackStack() })
        }
        composable(ElleRoutes.WORLD_PATTERNS)      { PatternsSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_LEARNING)      { LearningSection(containerExtended, { id -> navController.navigate(ElleRoutes.worldSubjectDetail(id)) }, { navController.popBackStack() }) }
        composable(
            route     = ElleRoutes.WORLD_SUBJECT_DETAIL,
            arguments = listOf(navArgument("subjectId") { type = NavType.IntType }),
        ) { entry ->
            val subjectId = entry.arguments?.getInt("subjectId") ?: 0
            SubjectDetailSection(subjectId, containerExtended, { navController.popBackStack() })
        }
        composable(ElleRoutes.WORLD_CAPABILITIES)  { CapabilitiesSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_ETHICS)        { EthicsSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_AUTONOMY)      { AutonomySection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_SELF_IMAGE)    { SelfImageSection(containerExtended, { navController.popBackStack() }) }
        composable(ElleRoutes.WORLD_X_CHRONICLE)   {
            val ws = containerExtended.webSocketOrNull
            if (ws == null) {
                ConnectionNotReadyScreen(onRetry = { containerExtended.reconnectWebSocketIfNeeded() })
                return@composable
            }
            XChronicleSection(containerExtended, ws, { navController.popBackStack() })
        }

        composable("${ElleRoutes.DEV}_pin") {
            DevPinScreen(
                containerExtended = containerExtended,
                onUnlocked        = {
                    onDevUnlocked()
                    navController.navigate(ElleRoutes.DEV) {
                        popUpTo("${ElleRoutes.DEV}_pin") { inclusive = true }
                    }
                },
                onBack = { navController.popBackStack() },
            )
        }
        composable(ElleRoutes.DEV) {
            DevDashboardScreen(
                containerExtended = containerExtended,
                onNavigate        = { route -> navController.navigate(route) },
                onSettings        = onSettings,
            )
        }
        composable(ElleRoutes.DEV_LOGS)          { LogMonitorScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_HEALTH)        { SystemHealthScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_SERVICES)      { ServiceStatusScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_DIAGNOSTICS)   { DiagnosticsScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_API_EXPLORER)  { ApiExplorerScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_HARDWARE)      { HardwareScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_MODELS)        { ModelsScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_AGENTS)        { AgentsScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_TOOLS)         { ToolsScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_DICTIONARY)    { DictionaryAdminScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_MEMORY_ADMIN)  { MemoryAdminScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_BACKUPS)       { BackupsScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_CONFIG)        { ConfigScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_DEVICES)       { PairedDevicesScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_VIDEO_WORKERS) { VideoWorkersScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_LEARNING_ADMIN){ LearningAdminScreen(containerExtended) { navController.popBackStack() } }
        composable(ElleRoutes.DEV_ETHICS_ADMIN)  { EthicsAdminScreen(containerExtended) { navController.popBackStack() } }

        composable(ElleRoutes.DEV_SHN_EDITOR) {
            val scope = rememberCoroutineScope()
            SHNScreen(
                api = containerExtended.extendedApi,
                onSaveToServer = { root, name, bytes, onResult ->
                    scope.launch {
                        val result = runCatching {
                            val b64 = AndroidBase64.encodeToString(
                                bytes, AndroidBase64.NO_WRAP
                            )
                            containerExtended.extendedApi.saveSHN(
                                ShnSaveRequest(root = root, name = name, bytesB64 = b64)
                            )
                        }
                        result.fold(
                            onSuccess = { resp ->
                                onResult(resp.ok,
                                    if (resp.ok) "saved → ${resp.path} (${resp.bytes} bytes)"
                                    else "server refused save")
                            },
                            onFailure = { e ->
                                onResult(false, "save failed: ${e.message ?: e.javaClass.simpleName}")
                            }
                        )
                    }
                }
            )
        }
    }
}
