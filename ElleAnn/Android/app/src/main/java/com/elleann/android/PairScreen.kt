package com.elleann.android

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
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

enum class PairMode { SIGN_IN, PAIR_CODE }

data class PairState(
    val mode:     PairMode = PairMode.SIGN_IN,
    val host:     String   = "",
    val port:     String   = "8000",
    val code:     String   = "",
    val username: String   = "",
    val password: String   = "",
    val loading:  Boolean  = false,
    val error:    String?  = null,
    val success:  Boolean  = false,
)

class PairViewModel(
    private val container: AppContainer,
    prefill: PairingPayload? = null,
) : ViewModel() {

    private val _state = MutableStateFlow(
        PairState(
            host = prefill?.host ?: "",
            port = prefill?.port?.toString() ?: "8000",
            code = prefill?.code ?: "",

            mode = if (!prefill?.code.isNullOrBlank()) PairMode.PAIR_CODE else PairMode.SIGN_IN,
        )
    )
    val state: StateFlow<PairState> = _state.asStateFlow()

    fun onMode(m: PairMode) = _state.update { it.copy(mode = m, error = null) }
    fun onHost(v: String)     = _state.update { it.copy(host = v, error = null) }
    fun onPort(v: String)     = _state.update { it.copy(port = v, error = null) }
    fun onCode(v: String)     = _state.update { it.copy(code = v, error = null) }
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
        when (s.mode) {
            PairMode.SIGN_IN -> {
                if (s.username.isBlank() || s.password.isBlank()) {
                    _state.update { it.copy(error = "Username and password are required") }
                    return
                }
            }
            PairMode.PAIR_CODE -> {
                if (s.code.length != 6 || !s.code.all { c -> c.isDigit() }) {
                    _state.update { it.copy(error = "Pair code must be 6 digits") }
                    return
                }
            }
        }

        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            runCatching {
                val api = container.apiFor(s.host, port)

                val body = com.elleann.android.data.models.LoginRequest(
                    username   = if (s.mode == PairMode.SIGN_IN) s.username else s.code,
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
fun PairScreen(
    container: AppContainer,
    prefill: PairingPayload? = null,
    onPaired: () -> Unit,
) {
    val vm: PairViewModel = viewModel(
        factory = remember {
            object : androidx.lifecycle.ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : ViewModel> create(modelClass: Class<T>): T =
                    PairViewModel(container, prefill) as T
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

                    ModeSwitch(
                        current = state.mode,
                        onChange = vm::onMode,
                    )

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

                    when (state.mode) {
                        PairMode.SIGN_IN -> {
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
                        }
                        PairMode.PAIR_CODE -> {
                            IsyaInputField(
                                value         = state.code,
                                onValueChange = { v ->
                                    if (v.length <= 6 && v.all { it.isDigit() }) vm.onCode(v)
                                },
                                label         = "6-digit pair code",
                                placeholder   = "123456",
                                keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                                    keyboardType = KeyboardType.NumberPassword,
                                ),
                            )
                        }
                    }

                    state.error?.let { err ->
                        Text(err, color = IsyaError, style = MaterialTheme.typography.bodySmall)
                    }

                    IsyaButton(
                        text     = if (state.loading) "Connecting…"
                                   else if (state.mode == PairMode.SIGN_IN) "SIGN IN"
                                   else "REDEEM CODE",
                        onClick  = { vm.submit(deviceId, onPaired) },
                        loading  = state.loading,
                        variant  = IsyaButtonVariant.PRIMARY_GOLD,
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }

            Text(
                text  = when (state.mode) {
                    PairMode.SIGN_IN   -> "Uses your game account (Account.dbo.tUser).\nNo separate Elle account needed."
                    PairMode.PAIR_CODE -> "Admin generates a 6-digit code via\nElle's device manager. Codes expire quickly."
                },
                style = MaterialTheme.typography.bodySmall,
                color = IsyaMuted,
                textAlign = TextAlign.Center,
            )

            Spacer(Modifier.height(32.dp))
        }
    }
}

@Composable
private fun ModeSwitch(current: PairMode, onChange: (PairMode) -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(10.dp))
            .background(IsyaSlot),
        horizontalArrangement = Arrangement.spacedBy(2.dp),
    ) {
        PairMode.values().forEach { m ->
            val selected = m == current
            val label = when (m) {
                PairMode.SIGN_IN   -> "Sign In"
                PairMode.PAIR_CODE -> "Pair Code"
            }
            Box(
                modifier = Modifier
                    .weight(1f)
                    .clip(RoundedCornerShape(8.dp))
                    .background(if (selected) IsyaGold.copy(alpha = 0.15f) else androidx.compose.ui.graphics.Color.Transparent)
                    .clickable { onChange(m) }
                    .padding(vertical = 8.dp),
                contentAlignment = Alignment.Center,
            ) {
                Text(
                    text       = label,
                    style      = MaterialTheme.typography.bodyMedium,
                    color      = if (selected) IsyaGold else IsyaMuted,
                    fontWeight = if (selected) FontWeight.SemiBold else FontWeight.Normal,
                )
            }
        }
    }
}
