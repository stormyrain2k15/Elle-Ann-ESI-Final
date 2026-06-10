package com.elleann.android.ui.login

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Lock
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.AppContainer
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.data.models.LoginRequest
import com.elleann.android.ui.components.IsyaButton
import com.elleann.android.ui.components.IsyaButtonVariant
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class LoginState(
    val host: String     = "",
    val port: String     = "8000",
    val username: String = "",
    val password: String = "",
    val loading: Boolean = false,
    val error: String?   = null,
)

class LoginViewModel(
    private val context: android.content.Context,
    private val container: AppContainer,
    private val containerExtended: AppContainerExtended,
) : ViewModel() {

    private val _state = MutableStateFlow(LoginState())
    val state: StateFlow<LoginState> = _state.asStateFlow()

    fun onHost(v: String)     = _state.update { it.copy(host = v, error = null) }
    fun onPort(v: String)     = _state.update { it.copy(port = v, error = null) }
    fun onUsername(v: String) = _state.update { it.copy(username = v, error = null) }
    fun onPassword(v: String) = _state.update { it.copy(password = v, error = null) }

    fun login(onSuccess: (String, Int) -> Unit) {
        val s = _state.value
        val host = s.host.trim()
        val port = s.port.trim().toIntOrNull() ?: 8000
        if (host.isBlank()) { _state.update { it.copy(error = "Server address is required") }; return }
        if (s.username.isBlank()) { _state.update { it.copy(error = "Username is required") }; return }
        if (s.password.isBlank()) { _state.update { it.copy(error = "Password is required") }; return }

        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            runCatching {
                container.apiFor(host, port).login(
                    LoginRequest(
                        username   = s.username.trim(),
                        password   = s.password,
                        deviceId   = android.provider.Settings.Secure.getString(
                            context.contentResolver,
                            android.provider.Settings.Secure.ANDROID_ID,
                        ),
                        deviceName = android.os.Build.MODEL,
                    )
                )
            }.onSuccess { resp ->
                containerExtended.onLoginSuccess(resp.jwt, host, port)
                _state.update { it.copy(loading = false) }
                onSuccess(host, port)
            }.onFailure { e ->
                _state.update { it.copy(loading = false, error = e.message ?: "Login failed") }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun LoginScreen(
    container: AppContainer,
    containerExtended: AppContainerExtended,
    onLoginSuccess: (String, Int) -> Unit,
) {
    val context = LocalContext.current
    val vm: LoginViewModel = viewModel(
        factory = remember(context) {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(c: Class<T>): T =
                    LoginViewModel(context.applicationContext, container, containerExtended) as T
            }
        }
    )
    val state by vm.state.collectAsState()

    Scaffold(containerColor = IsyaNight) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(32.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center,
        ) {
            Icon(Icons.Rounded.Lock, null, tint = IsyaGold, modifier = Modifier.size(48.dp))
            Spacer(Modifier.height(24.dp))
            Text("Sign in to ElleAnn", style = MaterialTheme.typography.titleMedium, color = IsyaCream)
            Spacer(Modifier.height(32.dp))

            OutlinedTextField(
                value = state.host,
                onValueChange = vm::onHost,
                label = { Text("Server address", color = IsyaMuted) },
                placeholder = { Text("192.168.x.x", color = IsyaMuted.copy(alpha = 0.5f)) },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                colors = elleTextFieldColors(),
            )
            Spacer(Modifier.height(12.dp))

            OutlinedTextField(
                value = state.port,
                onValueChange = vm::onPort,
                label = { Text("Port", color = IsyaMuted) },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                colors = elleTextFieldColors(),
            )
            Spacer(Modifier.height(12.dp))

            OutlinedTextField(
                value = state.username,
                onValueChange = vm::onUsername,
                label = { Text("Username", color = IsyaMuted) },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                colors = elleTextFieldColors(),
            )
            Spacer(Modifier.height(12.dp))

            OutlinedTextField(
                value = state.password,
                onValueChange = vm::onPassword,
                label = { Text("Password", color = IsyaMuted) },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                visualTransformation = PasswordVisualTransformation(),
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
                colors = elleTextFieldColors(),
            )
            Spacer(Modifier.height(8.dp))

            state.error?.let {
                Text(it, color = IsyaError, style = MaterialTheme.typography.bodySmall)
                Spacer(Modifier.height(8.dp))
            }

            Spacer(Modifier.height(16.dp))

            IsyaButton(
                text = if (state.loading) "Signing in…" else "Sign In",
                onClick = { vm.login(onLoginSuccess) },
                variant = IsyaButtonVariant.PRIMARY_GOLD,
                modifier = Modifier.fillMaxWidth(),
                enabled = !state.loading,
            )
        }
    }
}

@Composable
private fun elleTextFieldColors() = OutlinedTextFieldDefaults.colors(
    focusedTextColor        = IsyaCream,
    unfocusedTextColor      = IsyaCream,
    focusedBorderColor      = IsyaGold,
    unfocusedBorderColor    = IsyaMist,
    focusedContainerColor   = IsyaDusk,
    unfocusedContainerColor = IsyaDusk,
    cursorColor             = IsyaGoldBright,
)
