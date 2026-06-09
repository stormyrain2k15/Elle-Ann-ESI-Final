package com.elleann.android

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class LoginState(
    val host:     String   = "",
    val port:     String   = "8000",
    val username: String   = "",
    val password: String   = "",
    val loading:  Boolean  = false,
    val error:    String?  = null,
    val success:  Boolean  = false,
)

class LoginViewModel(
    private val container: AppContainer,
    prefill: PairingPayload? = null,
) : ViewModel() {

    private val _state = MutableStateFlow(
        LoginState(
            host = prefill?.host ?: "",
            port = prefill?.port?.toString() ?: "8000",
        )
    )
    val state: StateFlow<LoginState> = _state.asStateFlow()

    fun onHost(v: String)     = _state.update { it.copy(host = v, error = null) }
    fun onPort(v: String)     = _state.update { it.copy(port = v, error = null) }
    fun onUsername(v: String) = _state.update { it.copy(username = v, error = null) }
    fun onPassword(v: String) = _state.update { it.copy(password = v, error = null) }

    fun submit(deviceId: String, onSuccess: () -> Unit) {
        val s = _state.value
        val port = s.port.toIntOrNull() ?: run {
            _state.update { it.copy(error = "Invalid port number") }; return
        }
        if (s.host.isBlank()) {
            _state.update { it.copy(error = "Server host is required") }; return
        }
        if (s.username.isBlank() || s.password.isBlank()) {
            _state.update { it.copy(error = "Username and password are required") }
            return
        }

        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            runCatching {
                val api = container.apiFor(s.host, port)

                val body = com.elleann.android.data.models.LoginRequest(
                    username   = s.username,
                    password   = s.password,
                    deviceId   = deviceId,
                    deviceName = android.os.Build.MODEL ?: "ElleAnn Android",
                )
                val resp = api.login(body)
                container.tokenStore.save(
                    StoredToken(
                        jwt       = resp.jwt,
                        host      = s.host,
                        port      = port,
                        expiresMs = Long.MAX_VALUE,
                    )
                )
            }.onSuccess {
                _state.update { it.copy(loading = false, success = true) }
                onSuccess()
            }.onFailure { e ->

                val msg = when (e) {
                    is retrofit2.HttpException -> {
                        val raw = runCatching { e.response()?.errorBody()?.string() }.getOrNull()
                        if (!raw.isNullOrBlank()) "${e.code()}: $raw" else "${e.code()}: ${e.message()}"
                    }
                    else -> e.message ?: "Request failed"
                }
                _state.update { it.copy(loading = false, error = msg) }
            }
        }
    }
}

@Composable
fun LoginScreen(
    container: AppContainer,
    prefill: PairingPayload? = null,
    onSignedIn: () -> Unit,
) {
    val vm: LoginViewModel = viewModel(
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(modelClass: Class<T>): T =
                    LoginViewModel(container, prefill) as T
            }
        }
    )
    val state by vm.state.collectAsState()
    val context = androidx.compose.ui.platform.LocalContext.current

    val deviceId = remember {
        @Suppress("HardwareIds")
        android.provider.Settings.Secure.getString(
            context.contentResolver,
            android.provider.Settings.Secure.ANDROID_ID,
        ) ?: java.util.UUID.randomUUID().toString()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(IsyaNight),
        contentAlignment = Alignment.Center,
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth(0.9f)
                .verticalScroll(rememberScrollState()),
            horizontalAlignment  = Alignment.CenterHorizontally,
            verticalArrangement  = Arrangement.spacedBy(20.dp),
        ) {
            Spacer(Modifier.height(48.dp))

            Text(
                text       = "ElleAnn",
                style      = MaterialTheme.typography.displaySmall,
                color      = IsyaGold,
                fontWeight = FontWeight.Bold,
                textAlign  = TextAlign.Center,
            )
            Text(
                text      = "ESI v3 Companion",
                style     = MaterialTheme.typography.titleSmall,
                color     = IsyaMuted,
                textAlign = TextAlign.Center,
            )

            Spacer(Modifier.height(4.dp))

            IsyaPanel(title = "CONNECT TO SERVER", flowingBorder = true) {
                Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {
                    IsyaInputField(
                        value         = state.host,
                        onValueChange = vm::onHost,
                        label         = "Server host or IP",
                        placeholder   = "192.168.1.x",
                    )
                    IsyaInputField(
                        value         = state.port,
                        onValueChange = vm::onPort,
                        label         = "Port",
                        placeholder   = "8000",
                        keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                            keyboardType = KeyboardType.Number,
                        ),
                    )
                    IsyaInputField(
                        value         = state.username,
                        onValueChange = vm::onUsername,
                        label         = "Username",
                        placeholder   = "same as your in-game account",
                    )
                    IsyaInputField(
                        value         = state.password,
                        onValueChange = vm::onPassword,
                        label         = "Password",
                        placeholder   = "••••••••",
                        keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                            keyboardType = KeyboardType.Password,
                        ),
                        visualTransformation = PasswordVisualTransformation(),
                    )

                    state.error?.let { err ->
                        Text(err, color = IsyaError, style = MaterialTheme.typography.bodySmall)
                    }

                    IsyaButton(
                        text     = if (state.loading) "Connecting…" else "SIGN IN",
                        onClick  = { vm.submit(deviceId, onSignedIn) },
                        loading  = state.loading,
                        variant  = IsyaButtonVariant.PRIMARY_GOLD,
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }

            Text(
                text  = "Uses your game account (Account.dbo.tUser).\nNo separate Elle account needed.",
                style = MaterialTheme.typography.bodySmall,
                color = IsyaMuted,
                textAlign = TextAlign.Center,
            )

            Spacer(Modifier.height(32.dp))
        }
    }
}

@Deprecated(
    message    = "Pair-code flow removed. Use LoginScreen.",
    replaceWith = ReplaceWith("LoginScreen(container, prefill, onSignedIn)"),
)
@Composable
fun PairScreen(
    container: AppContainer,
    prefill: PairingPayload? = null,
    onPaired: () -> Unit,
) = LoginScreen(container, prefill, onPaired)
