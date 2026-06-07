package com.elleann.android.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.material3.Typography
import androidx.compose.material3.Shapes
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.foundation.shape.RoundedCornerShape

private val IsyaDarkColorScheme = darkColorScheme(
    primary            = IsyaGold,
    onPrimary          = IsyaNight,
    primaryContainer   = IsyaGoldDeep,
    onPrimaryContainer = IsyaCream,
    secondary          = IsyaMagic,
    onSecondary        = IsyaNight,
    secondaryContainer = IsyaMagicDeep,
    onSecondaryContainer = IsyaCream,
    tertiary           = ElleViolet,
    onTertiary         = IsyaNight,
    tertiaryContainer  = ElleVioletDeep,
    onTertiaryContainer = IsyaCream,
    background         = IsyaNight,
    onBackground       = IsyaCream,
    surface            = IsyaDusk,
    onSurface          = IsyaCream,
    surfaceVariant     = IsyaMist,
    onSurfaceVariant   = IsyaParchment,
    outline            = IsyaSilverMid,
    outlineVariant     = IsyaSilverDeep,
    error              = IsyaError,
    onError            = IsyaCream,
    errorContainer     = Color(0xFF4A0A0AuL),
    onErrorContainer   = IsyaCream,
    inverseSurface     = IsyaCream,
    inverseOnSurface   = IsyaNight,
    inversePrimary     = IsyaGoldDeep,
    scrim              = Color(0x99000000uL),
)

private val IsyaTypography = Typography(
    displayLarge = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.Bold,
        fontSize   = 57.sp,
        lineHeight = 64.sp,
        letterSpacing = (-0.25).sp,
        color = IsyaCream
    ),
    displayMedium = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.SemiBold,
        fontSize   = 45.sp,
        lineHeight = 52.sp,
        color = IsyaCream
    ),
    displaySmall = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.SemiBold,
        fontSize   = 36.sp,
        lineHeight = 44.sp,
        color = IsyaCream
    ),
    headlineLarge = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.SemiBold,
        fontSize   = 32.sp,
        lineHeight = 40.sp,
        color = IsyaGold
    ),
    headlineMedium = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.Medium,
        fontSize   = 28.sp,
        lineHeight = 36.sp,
        color = IsyaGold
    ),
    headlineSmall = TextStyle(
        fontFamily = FontFamily.Serif,
        fontWeight = FontWeight.Medium,
        fontSize   = 24.sp,
        lineHeight = 32.sp,
        color = IsyaGold
    ),
    titleLarge = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.SemiBold,
        fontSize   = 22.sp,
        lineHeight = 28.sp,
        color = IsyaCream
    ),
    titleMedium = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Medium,
        fontSize   = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.15.sp,
        color = IsyaCream
    ),
    titleSmall = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Medium,
        fontSize   = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
        color = IsyaParchment
    ),
    bodyLarge = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Normal,
        fontSize   = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.5.sp,
        color = IsyaCream
    ),
    bodyMedium = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Normal,
        fontSize   = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.25.sp,
        color = IsyaCream
    ),
    bodySmall = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Normal,
        fontSize   = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.4.sp,
        color = IsyaParchment
    ),
    labelLarge = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Medium,
        fontSize   = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
        color = IsyaCream
    ),
    labelMedium = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Medium,
        fontSize   = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
        color = IsyaParchment
    ),
    labelSmall = TextStyle(
        fontFamily = FontFamily.SansSerif,
        fontWeight = FontWeight.Medium,
        fontSize   = 11.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
        color = IsyaMuted
    ),
)

private val IsyaShapes = Shapes(
    extraSmall = RoundedCornerShape(4.dp),
    small      = RoundedCornerShape(8.dp),
    medium     = RoundedCornerShape(12.dp),
    large      = RoundedCornerShape(16.dp),
    extraLarge = RoundedCornerShape(24.dp),
)

data class IsyaExtendedColors(
    val gold: Color,
    val goldBright: Color,
    val magic: Color,
    val magicBright: Color,
    val elleViolet: Color,
    val cream: Color,
    val parchment: Color,
    val muted: Color,
    val night: Color,
    val dusk: Color,
    val mist: Color,
    val header: Color,
    val input: Color,
    val silver: Color,
    val silverMid: Color,
    val silverDeep: Color,
    val error: Color,
    val success: Color,
    val warn: Color,
)

private val LocalIsyaColors = staticCompositionLocalOf {
    IsyaExtendedColors(
        gold         = IsyaGold,
        goldBright   = IsyaGoldBright,
        magic        = IsyaMagic,
        magicBright  = IsyaMagicBright,
        elleViolet   = ElleViolet,
        cream        = IsyaCream,
        parchment    = IsyaParchment,
        muted        = IsyaMuted,
        night        = IsyaNight,
        dusk         = IsyaDusk,
        mist         = IsyaMist,
        header       = IsyaHeader,
        input        = IsyaInput,
        silver       = IsyaSilver,
        silverMid    = IsyaSilverMid,
        silverDeep   = IsyaSilverDeep,
        error        = IsyaError,
        success      = IsyaSuccess,
        warn         = IsyaWarn,
    )
}

object IsyaTheme {
    val colors: IsyaExtendedColors
        @Composable get() = LocalIsyaColors.current
}

@Composable
fun ElleAnnTheme(
    content: @Composable () -> Unit
) {
    val isyaColors = IsyaExtendedColors(
        gold         = IsyaGold,
        goldBright   = IsyaGoldBright,
        magic        = IsyaMagic,
        magicBright  = IsyaMagicBright,
        elleViolet   = ElleViolet,
        cream        = IsyaCream,
        parchment    = IsyaParchment,
        muted        = IsyaMuted,
        night        = IsyaNight,
        dusk         = IsyaDusk,
        mist         = IsyaMist,
        header       = IsyaHeader,
        input        = IsyaInput,
        silver       = IsyaSilver,
        silverMid    = IsyaSilverMid,
        silverDeep   = IsyaSilverDeep,
        error        = IsyaError,
        success      = IsyaSuccess,
        warn         = IsyaWarn,
    )
    CompositionLocalProvider(LocalIsyaColors provides isyaColors) {
        MaterialTheme(
            colorScheme = IsyaDarkColorScheme,
            typography  = IsyaTypography,
            shapes      = IsyaShapes,
            content     = content,
        )
    }
}
