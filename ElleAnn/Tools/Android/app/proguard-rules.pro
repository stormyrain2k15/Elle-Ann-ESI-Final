# ElleAnn ProGuard rules

# Retrofit — keep all interfaces and annotations
-keep interface com.elleann.android.data.** { *; }
-keepattributes *Annotation*

# Kotlinx Serialization — keep all @Serializable classes
-keep @kotlinx.serialization.Serializable class com.elleann.android.** { *; }
-keepclassmembers class com.elleann.android.** {
    kotlinx.serialization.KSerializer serializer(...);
}

# OkHttp
-dontwarn okhttp3.**
-dontwarn okio.**
-keep class okhttp3.** { *; }
-keep class okio.** { *; }

# Retrofit
-dontwarn retrofit2.**
-keep class retrofit2.** { *; }

# Coroutines
-keepclassmembernames class kotlinx.** { volatile <fields>; }

# EncryptedSharedPreferences / Security Crypto
-keep class androidx.security.crypto.** { *; }

# ZXing QR scanner
-keep class com.journeyapps.** { *; }
-keep class com.google.zxing.** { *; }

# Coil
-dontwarn coil.**

# WebView / JavaScript bridge (PixelStreaming and video player)
-keepclassmembers class * {
    @android.webkit.JavascriptInterface <methods>;
}

# Keep model classes (all data classes for Retrofit/JSON)
-keep class com.elleann.android.data.models.** { *; }
-keep class com.elleann.android.ElleApp { *; }
-keep class com.elleann.android.StoredToken { *; }
-keep class com.elleann.android.PairResponse { *; }
-keep class com.elleann.android.EmotionsResponse { *; }
