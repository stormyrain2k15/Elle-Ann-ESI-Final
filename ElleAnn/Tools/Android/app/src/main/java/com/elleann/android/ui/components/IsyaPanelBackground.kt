package com.elleann.android.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin
import kotlin.random.Random

val FiestaPanelBase   = Color(0xFF0D1B44)
val FiestaStreakLite  = Color(0xFF3C5FA8)
val FiestaStreakDark  = Color(0xFF040C28)

@Composable
fun IsyaFiestaPanelBackground(
    modifier:    Modifier = Modifier,
    cornerRadius: Dp     = 12.dp,
    streakCount:  Int    = 6,
    tiltDegrees:  Float  = 12f,
) {

    val speckles = remember {
        val rng = Random(0xE11EA117)
        Array(140) { rng.nextFloat() to rng.nextFloat() }
    }

    Canvas(
        modifier = modifier
            .fillMaxSize()
            .clip(RoundedCornerShape(cornerRadius))
    ) {

        drawRect(color = FiestaPanelBase, size = size)

        val w     = size.width
        val h     = size.height
        val tilt  = (tiltDegrees * PI / 180.0).toFloat()
        val cosT  = cos(tilt.toDouble()).toFloat()
        val sinT  = sin(tilt.toDouble()).toFloat()

        val period = w / streakCount
        val bandW  = period * 0.65f

        val span = (w + h * sinT / cosT).coerceAtLeast(w)
        val startX = -bandW
        var cx = startX
        while (cx < span) {
            val gradient = Brush.linearGradient(
                colors = listOf(
                    FiestaStreakDark,
                    FiestaStreakLite,
                    FiestaStreakDark,
                ),
                start = Offset(cx - bandW / 2, 0f),
                end   = Offset(cx + bandW / 2, h),
            )

            val path = Path().apply {
                moveTo(cx - bandW / 2, 0f)
                lineTo(cx + bandW / 2, 0f)
                lineTo(cx + bandW / 2 - h * sinT / cosT, h)
                lineTo(cx - bandW / 2 - h * sinT / cosT, h)
                close()
            }
            drawPath(path = path, brush = gradient, alpha = 0.55f)
            cx += period
        }

        val grain = Color(0xFFC8D6F0).copy(alpha = 0.06f)
        for ((u, v) in speckles) {
            drawCircle(
                color  = grain,
                radius = 0.6f,
                center = Offset(u * w, v * h),
            )
        }
    }
}

@Composable
fun IsyaCornerOrnaments(
    modifier: Modifier = Modifier,
    notchSize: Dp = 8.dp,
) {
    Canvas(modifier = modifier.fillMaxSize()) {
        val n = notchSize.toPx()
        val w = size.width
        val h = size.height
        val brush = Brush.linearGradient(
            colors = listOf(
                Color(0xFFD2D7DC),
                Color(0xFFA5AFB4),
                Color(0xFF6E7378),
            )
        )

        drawPath(Path().apply {
            moveTo(0f, n);  lineTo(n, 0f);  lineTo(n, n);  close()
        }, brush)

        drawPath(Path().apply {
            moveTo(w - n, 0f); lineTo(w, n); lineTo(w - n, n); close()
        }, brush)

        drawPath(Path().apply {
            moveTo(0f, h - n); lineTo(n, h - n); lineTo(n, h); close()
        }, brush)

        drawPath(Path().apply {
            moveTo(w - n, h - n); lineTo(w, h - n); lineTo(w - n, h); close()
        }, brush)
    }
}
