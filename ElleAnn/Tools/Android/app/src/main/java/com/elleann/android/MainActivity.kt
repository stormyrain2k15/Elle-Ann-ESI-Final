package com.elleann.android

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.runtime.*
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.navigation.ElleNavHost
import com.elleann.android.ui.theme.ElleAnnTheme
import com.elleann.android.ui.login.LoginScreen

class MainActivity : ComponentActivity() {

    private lateinit var containerExtended: AppContainerExtended

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        val app = application as ElleApp

        containerExtended = AppContainerExtended(
            context          = this,
            baseContainer    = app.container,
            onReauthRequired = {
                // Token expired or invalid — clear it and show login
                runOnUiThread { triggerReauth() }
            },
        )

        setContent {
            ElleAnnTheme {
                var showLogin by remember {
                    mutableStateOf(!containerExtended.isPaired)
                }

                if (showLogin) {
                    LoginScreen(
                        container = app.container,
                        containerExtended = containerExtended,
                        onLoginSuccess = { host, port ->
                            containerExtended.initWebSocket()
                            showLogin = false
                        }
                    )
                } else {
                    LaunchedEffect(Unit) {
                        containerExtended.initWebSocket()
                    }
                    ElleNavHost(
                        container         = app.container,
                        containerExtended = containerExtended,
                        onUnpair          = {
                            containerExtended.logout()
                            showLogin = true
                        },
                    )
                }
            }
        }
    }

    private fun triggerReauth() {
        // Re-trigger recomposition by clearing the stored token
        // The setContent block will react to isPaired becoming false
        containerExtended.logout()
        recreate()
    }

    override fun onStart() {
        super.onStart()
        if (containerExtended.isPaired) {
            containerExtended.reconnectWebSocketIfNeeded()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        containerExtended.disconnectWebSocket()
    }
}
