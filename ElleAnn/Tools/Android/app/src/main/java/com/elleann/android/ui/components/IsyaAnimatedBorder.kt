package com.elleann.android.ui.components

import androidx.compose.animation.core.*
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.*
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.elleann.android.ui.theme.*
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin

@Composable
fun IsyaAnimatedBorderBox(
    modifier: Modifier = Modifier,
    cornerRadius: Dp = 12.dp,
    strokeWidth: Dp = 1.5.dp,
    glowWidth: Dp = 6.dp,
    animated: Boolean = true,
    content: @Composable BoxScope.() -> Unit,
) {
    val infiniteTransition = rememberInfiniteTransition(label = "isya_border")

    val dashPhase by if (animated) {
        infiniteTransition.animateFloat(
            initialValue  = 0f,
            targetValue   = 1000f,
            animationSpec = infiniteRepeatable(
                animation  = tween(50_000, easing = LinearEasing),
                repeatMode = RepeatMode.Restart,
            ),
            label = "dash_phase",
        )
    } else { remember { mutableStateOf(0f) } }

    val hueFraction by if (animated) {
        infiniteTransition.animateFloat(
            initialValue  = 0f,
            targetValue   = 1f,
            animationSpec = infiniteRepeatable(
                animation  = tween(40_000, easing = LinearEasing),
                repeatMode = RepeatMode.Restart,
            ),
            label = "hue_fraction",
        )
    } else { remember { mutableStateOf(0f) } }

    val borderColor = isyaHueCycle(hueFraction)

    Box(modifier = modifier) {
        if (animated) {

            Canvas(
                modifier = Modifier
                    .matchParentSize()
                    .clip(RoundedCornerShape(cornerRadius))
            ) {
                drawIsyaBorder(
                    color       = borderColor.copy(alpha = 0.20f),
                    strokePx    = glowWidth.toPx(),
                    cornerPx    = cornerRadius.toPx(),
                    dashPhase   = dashPhase,
                    dashed      = false,
                )
            }

            Canvas(
                modifier = Modifier
                    .matchParentSize()
                    .clip(RoundedCornerShape(cornerRadius))
            ) {
                drawIsyaBorder(
                    color     = borderColor,
                    strokePx  = strokeWidth.toPx(),
                    cornerPx  = cornerRadius.toPx(),
                    dashPhase = dashPhase,
                    dashed    = true,
                )
            }
        } else {

            Canvas(
                modifier = Modifier
                    .matchParentSize()
                    .clip(RoundedCornerShape(cornerRadius))
            ) {
                val sw = strokeWidth.toPx()
                val cr = cornerRadius.toPx()

                drawIsyaSilverBevel(
                    strokePx    = sw,
                    cornerPx    = cr,
                    highlight   = IsyaSilver,
                    mid         = IsyaSilverMid,
                    deep        = IsyaSilverDeep,
                )

                drawIsyaSilverBevel(
                    strokePx    = sw * 0.4f,
                    cornerPx    = cr,
                    highlight   = IsyaSilver.copy(alpha = 0.85f),
                    mid         = IsyaSilver.copy(alpha = 0.0f),
                    deep        = IsyaSilver.copy(alpha = 0.0f),
                    insetPx     = sw * 0.4f,
                )
            }
        }

        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(strokeWidth),
            content = content,
        )
    }
}

private fun DrawScope.drawIsyaBorder(
    color: Color,
    strokePx: Float,
    cornerPx: Float,
    dashPhase: Float,
    dashed: Boolean,
) {
    val halfStroke = strokePx / 2f
    val rect = androidx.compose.ui.geometry.Rect(
        left   = halfStroke,
        top    = halfStroke,
        right  = size.width  - halfStroke,
        bottom = size.height - halfStroke,
    )

    val pathEffect = if (dashed) {
        PathEffect.dashPathEffect(
            intervals = floatArrayOf(18f, 10f),
            phase     = -dashPhase,
        )
    } else null

    drawRoundRect(
        color       = color,
        topLeft     = Offset(rect.left, rect.top),
        size        = Size(rect.width, rect.height),
        cornerRadius = CornerRadius(cornerPx, cornerPx),
        style       = Stroke(
            width      = strokePx,
            pathEffect = pathEffect,
        ),
    )
}

private fun isyaHueCycle(fraction: Float): Color {
    return when {
        fraction < 0.33f -> {

            val t = fraction / 0.33f
            lerpColor(IsyaSilverMid, IsyaGold, t)
        }
        fraction < 0.66f -> {

            val t = (fraction - 0.33f) / 0.33f
            lerpColor(IsyaGold, IsyaMagic, t)
        }
        else -> {

            val t = (fraction - 0.66f) / 0.34f
            lerpColor(IsyaMagic, IsyaSilverMid, t)
        }
    }
}

private fun lerpColor(a: Color, b: Color, t: Float): Color = Color(
    red   = a.red   + (b.red   - a.red)   * t,
    green = a.green + (b.green - a.green) * t,
    blue  = a.blue  + (b.blue  - a.blue)  * t,
    alpha = 1f,
)

private fun DrawScope.drawIsyaSilverBevel(
    strokePx:  Float,
    cornerPx:  Float,
    highlight: Color,
    mid:       Color,
    deep:      Color,
    insetPx:   Float = 0f,
) {
    val half = strokePx / 2f + insetPx
    val rect = androidx.compose.ui.geometry.Rect(
        left   = half,
        top    = half,
        right  = size.width  - half,
        bottom = size.height - half,
    )
    val brush = Brush.verticalGradient(
        colors = listOf(highlight, mid, deep),
        startY = rect.top,
        endY   = rect.bottom,
    )
    drawRoundRect(
        brush        = brush,
        topLeft      = Offset(rect.left, rect.top),
        size         = Size(rect.width, rect.height),
        cornerRadius = CornerRadius(cornerPx, cornerPx),
        style        = Stroke(width = strokePx),
    )
}
