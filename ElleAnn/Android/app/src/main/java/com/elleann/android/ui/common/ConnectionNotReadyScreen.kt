package com.elleann.android.ui.common

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp

@Composable
fun ConnectionNotReadyScreen(
    onRetry: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Surface(
        modifier = modifier.fillMaxSize(),
        color = Color(0xFF06140A),
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(32.dp),
            verticalArrangement   = Arrangement.spacedBy(16.dp, Alignment.CenterVertically),
            horizontalAlignment   = Alignment.CenterHorizontally,
        ) {
            CircularProgressIndicator(
                color = Color(0xFF00FF88),
                strokeWidth = 2.dp,
            )
            Text(
                text  = "Live connection not ready",
                color = Color(0xFF00FF88),
                style = MaterialTheme.typography.titleMedium,
                textAlign = TextAlign.Center,
            )
            Text(
                text  = "Reconnecting to the Elle-Ann companion service. " +
                        "If this persists, check your LAN address and that " +
                        "the desktop service is running.",
                color = Color(0xFF7BB6BC),
                style = MaterialTheme.typography.bodySmall,
                textAlign = TextAlign.Center,
            )
            Button(onClick = onRetry) {
                Text("Retry")
            }
        }
    }
}
