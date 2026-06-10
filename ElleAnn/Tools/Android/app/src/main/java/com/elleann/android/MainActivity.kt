package com.elleann.android

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.navigation.ElleNavHost
import com.elleann.android.ui.theme.ElleAnnTheme

class MainActivity : ComponentActivity() {

    private lateinit var containerExtended: AppContainerExtended

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        val app = application as ElleApp

        containerExtended = AppContainerExtended(
            context       = this,
            baseContainer = app.container,
            onReauthRequired = {

            },
        )

        containerExtended.initWebSocket()

        setContent {
            ElleAnnTheme {
                ElleNavHost(
                    container         = app.container,
                    containerExtended = containerExtended,
                    onUnpair          = { },
                )
            }
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
    }

    override fun onStart() {
        super.onStart()
        containerExtended.reconnectWebSocketIfNeeded()
    }

    override fun onStop() {
        super.onStop()
    }

    override fun onDestroy() {
        super.onDestroy()
        containerExtended.disconnectWebSocket()
    }
}
