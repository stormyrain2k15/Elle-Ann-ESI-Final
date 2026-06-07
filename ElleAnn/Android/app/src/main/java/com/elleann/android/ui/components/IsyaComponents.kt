package com.elleann.android.ui.components

import androidx.compose.animation.core.*
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Close
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.elleann.android.R
import com.elleann.android.ui.theme.*

@Composable
fun IsyaPanel(
    title: String,
    modifier: Modifier = Modifier,
    headerIcon: ImageVector? = null,
    onClose: (() -> Unit)? = null,

    flowingBorder: Boolean = false,

    decoratedCorners: Boolean = false,

    fiestaBackground: Boolean = true,
    content: @Composable ColumnScope.() -> Unit,
) {
    IsyaAnimatedBorderBox(
        modifier     = modifier.fillMaxWidth(),
        cornerRadius = 12.dp,
        strokeWidth  = 1.5.dp,
        glowWidth    = 6.dp,
        animated     = flowingBorder,
    ) {
        Box(modifier = Modifier.fillMaxWidth()) {

            if (fiestaBackground) {
                IsyaFiestaPanelBackground(cornerRadius = 12.dp)
            } else {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(IsyaDusk, RoundedCornerShape(12.dp))
                )
            }

            Column(modifier = Modifier.fillMaxWidth()) {

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .background(
                            IsyaHeader.copy(alpha = 0.85f),
                            RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp),
                        )
                        .padding(horizontal = 12.dp, vertical = 10.dp),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    if (headerIcon != null) {
                        Icon(headerIcon, null, tint = IsyaMagic, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                    }
                    Text(
                        text       = title,
                        style      = MaterialTheme.typography.titleSmall,
                        color      = IsyaCream,
                        fontWeight = FontWeight.Medium,
                        modifier   = Modifier.weight(1f),
                    )
                    if (onClose != null) {
                        IconButton(onClick = onClose, modifier = Modifier.size(24.dp)) {
                            Icon(
                                imageVector        = Icons.Rounded.Close,
                                contentDescription = "Close",
                                tint               = IsyaMuted,
                                modifier           = Modifier.size(16.dp),
                            )
                        }
                    }
                }

                Box(modifier = Modifier
                    .fillMaxWidth()
                    .height(1.dp)
                    .background(IsyaMagic.copy(alpha = 0.3f)))

                Column(modifier = Modifier.padding(16.dp), content = content)
            }

            if (decoratedCorners) {
                IsyaCornerOrnaments(notchSize = 8.dp)
            }
        }
    }
}

enum class IsyaButtonVariant {
    PRIMARY_GOLD,
    SECONDARY_TEAL,

    SILVER_BLUE,
    DANGER,
    GHOST,
}

@Composable
fun IsyaButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    variant: IsyaButtonVariant = IsyaButtonVariant.SECONDARY_TEAL,
    enabled: Boolean = true,
    icon: ImageVector? = null,
    loading: Boolean = false,
) {
    val (bgBrush, textColor) = when (variant) {
        IsyaButtonVariant.PRIMARY_GOLD   -> Brush.horizontalGradient(listOf(IsyaGoldMid, IsyaGoldDeep)) to IsyaCream
        IsyaButtonVariant.SECONDARY_TEAL -> Brush.horizontalGradient(listOf(IsyaMagicMid, IsyaMagicDeep)) to IsyaCream
        IsyaButtonVariant.SILVER_BLUE    -> Brush.verticalGradient(
            listOf(
                IsyaSilverButtonBlue.copy(alpha = 0.95f),
                IsyaSilverButtonBlue.copy(alpha = 0.55f),
                IsyaSilverButtonBlue.copy(alpha = 0.35f),
            )
        ) to IsyaSilver
        IsyaButtonVariant.DANGER         -> Brush.horizontalGradient(listOf(Color(0xFFA01A1A), Color(0xFF6A0A0A))) to IsyaCream
        IsyaButtonVariant.GHOST          -> Brush.horizontalGradient(listOf(Color.Transparent, Color.Transparent)) to IsyaGold
    }
    val borderColor = when (variant) {
        IsyaButtonVariant.GHOST       -> IsyaGold
        IsyaButtonVariant.SILVER_BLUE -> IsyaSilverMid
        else                          -> null
    }

    Button(
        onClick  = onClick,
        enabled  = enabled && !loading,
        modifier = modifier.height(48.dp),
        shape    = RoundedCornerShape(8.dp),
        colors   = ButtonDefaults.buttonColors(
            containerColor         = Color.Transparent,
            contentColor           = textColor,
            disabledContainerColor = IsyaMist,
            disabledContentColor   = IsyaMuted,
        ),
        contentPadding = PaddingValues(0.dp),
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(bgBrush, RoundedCornerShape(8.dp))
                .let { m ->
                    if (borderColor != null) {
                        m.then(
                            Modifier
                                .border(
                                    width  = 1.dp,
                                    brush  = Brush.verticalGradient(
                                        listOf(IsyaSilver, borderColor, IsyaSilverDeep),
                                    ),
                                    shape  = RoundedCornerShape(8.dp),
                                )
                                .clip(RoundedCornerShape(8.dp))
                        )
                    } else m
                },
            contentAlignment = Alignment.Center,
        ) {
            if (loading) {
                CircularProgressIndicator(Modifier.size(20.dp), strokeWidth = 2.dp, color = textColor)
            } else {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    if (icon != null) {
                        Icon(icon, null, modifier = Modifier.size(18.dp))
                        Spacer(Modifier.width(6.dp))
                    }
                    Text(text, fontWeight = FontWeight.Medium)
                }
            }
        }
    }
}

@Composable
fun IsyaInputField(
    value: String,
    onValueChange: (String) -> Unit,
    label: String,
    modifier: Modifier = Modifier,
    placeholder: String = "",
    singleLine: Boolean = true,
    keyboardOptions: androidx.compose.foundation.text.KeyboardOptions =
        androidx.compose.foundation.text.KeyboardOptions.Default,
    enabled: Boolean = true,
    isError: Boolean = false,
    visualTransformation: androidx.compose.ui.text.input.VisualTransformation =
        androidx.compose.ui.text.input.VisualTransformation.None,
) {
    Column(modifier = modifier) {
        Text(
            text     = label,
            style    = MaterialTheme.typography.labelMedium,
            color    = if (isError) IsyaError else IsyaParchment,
            fontStyle = FontStyle.Italic,
        )
        Spacer(Modifier.height(4.dp))
        OutlinedTextField(
            value = value, onValueChange = onValueChange,
            placeholder = { Text(placeholder, color = IsyaMuted) },
            singleLine = singleLine, enabled = enabled, isError = isError,
            keyboardOptions = keyboardOptions,
            visualTransformation = visualTransformation,
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(8.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedTextColor      = IsyaCream, unfocusedTextColor    = IsyaCream,
                disabledTextColor     = IsyaMuted,
                focusedBorderColor    = IsyaMagic, unfocusedBorderColor  = IsyaGold.copy(alpha = 0.6f),
                errorBorderColor      = IsyaError,
                focusedContainerColor = IsyaInput, unfocusedContainerColor = IsyaInput,
                cursorColor           = IsyaMagicBright,
            ),
        )
    }
}

enum class ConnectionState { CONNECTED, SLOW, DISCONNECTED }

@Composable
fun ConnectionDot(state: ConnectionState, modifier: Modifier = Modifier) {
    val color = when (state) {
        ConnectionState.CONNECTED    -> IsyaSuccess
        ConnectionState.SLOW         -> IsyaWarn
        ConnectionState.DISCONNECTED -> IsyaError
    }
    val infiniteTransition = rememberInfiniteTransition(label = "dot")
    val alpha by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue  = if (state == ConnectionState.CONNECTED) 0.5f else 0.2f,
        animationSpec = infiniteRepeatable(tween(1200), RepeatMode.Reverse),
        label = "dot_alpha",
    )
    Box(modifier = modifier.size(10.dp).clip(CircleShape).background(color.copy(alpha = alpha)))
}

@Composable
fun EmotionBar(
    label: String,
    value: Float,
    modifier: Modifier = Modifier,
    color: Color = IsyaMagic,
    showValue: Boolean = true,
) {
    val animated by animateFloatAsState(
        targetValue   = value.coerceIn(0f, 1f),
        animationSpec = tween(600, easing = CubicBezierEasing(0.33f, 1f, 0.68f, 1f)),
        label         = "bar_$label",
    )
    Row(modifier = modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
        Text(label, style = MaterialTheme.typography.bodySmall, color = IsyaParchment, modifier = Modifier.width(90.dp))
        Box(modifier = Modifier.weight(1f).height(6.dp).clip(RoundedCornerShape(3.dp)).background(IsyaMist)) {
            Box(modifier = Modifier
                .fillMaxWidth(animated).fillMaxHeight()
                .clip(RoundedCornerShape(3.dp))
                .background(Brush.horizontalGradient(listOf(color.copy(alpha = 0.7f), color))))
        }
        if (showValue) {
            Text(
                "%.2f".format(value),
                style = MaterialTheme.typography.labelSmall,
                color = IsyaMuted,
                modifier = Modifier.width(40.dp).padding(start = 8.dp),
            )
        }
    }
}

@Composable
fun IsyaIdentityRing(modifier: Modifier = Modifier) {
    Image(
        painter            = painterResource(id = R.drawable.isya_identity_ring),
        contentDescription = "Identity ring",
        contentScale       = ContentScale.Fit,
        modifier           = modifier,
    )
}

@Composable
fun IsyaSectionHeader(title: String, modifier: Modifier = Modifier, subtitle: String? = null) {
    Column(modifier = modifier) {
        Text(title, style = MaterialTheme.typography.headlineSmall, color = IsyaGold, fontWeight = FontWeight.SemiBold)
        subtitle?.let { Text(it, style = MaterialTheme.typography.bodySmall, color = IsyaMuted) }
    }
}

@Composable
fun IsyaLoadingIndicator(modifier: Modifier = Modifier) {
    Box(modifier = modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
        CircularProgressIndicator(color = IsyaGold, strokeWidth = 2.dp)
    }
}

@Composable
fun IsyaErrorState(message: String, onRetry: (() -> Unit)? = null, modifier: Modifier = Modifier) {
    Column(
        modifier = modifier.fillMaxWidth(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text("⚠ $message", style = MaterialTheme.typography.bodyMedium, color = IsyaError)
        if (onRetry != null) IsyaButton("Retry", onRetry, variant = IsyaButtonVariant.GHOST)
    }
}

private val IsyaGoldMid   = Color(0xFFC8922A)
private val IsyaGoldDeep  = Color(0xFFA0701A)
private val IsyaMagicMid  = Color(0xFF1A8A9A)
private val IsyaMagicDeep = Color(0xFF0D6070)
