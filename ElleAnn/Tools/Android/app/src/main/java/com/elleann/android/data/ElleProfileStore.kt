package com.elleann.android.data

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.util.Base64
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.ByteArrayOutputStream
import java.io.File

/**
 * ElleProfileStore — manages Elle's locked profile picture.
 *
 * Design:
 * - Once a picture is set it is LOCKED. Cannot be changed or deleted.
 * - The image is uploaded to the server via /api/video/avatar/upload
 *   and stored there permanently. This means it survives uninstall/reinstall.
 * - A local cache copy is kept in filesDir for fast loading.
 * - On fresh install, the app fetches the locked avatar from the server.
 *
 * The lock is enforced both locally (EncryptedSharedPreferences flag) and
 * server-side (the default avatar is never changed after first set).
 */
class ElleProfileStore(
    private val context: Context,
    private val prefs: android.content.SharedPreferences,
) {
    companion object {
        private const val TAG            = "ElleProfileStore"
        private const val KEY_LOCKED     = "elle_profile_locked"
        private const val KEY_AVATAR_ID  = "elle_profile_avatar_id"
        private const val LOCAL_FILENAME = "elle_profile.jpg"
        private const val MAX_DIMENSION  = 512 // resize before upload
    }

    private val _bitmap = MutableStateFlow<Bitmap?>(null)
    val bitmap: StateFlow<Bitmap?> = _bitmap.asStateFlow()

    private val _isLockedFlow = MutableStateFlow(prefs.getBoolean(KEY_LOCKED, false))
    val isLockedFlow: StateFlow<Boolean> = _isLockedFlow.asStateFlow()

    val isLocked: Boolean get() = _isLockedFlow.value
    val localFile: File get() = File(context.filesDir, LOCAL_FILENAME)

    // Load from local cache — fast, no network
    suspend fun loadLocal() = withContext(Dispatchers.IO) {
        _isLockedFlow.value = prefs.getBoolean(KEY_LOCKED, false)
        if (localFile.exists()) {
            runCatching {
                _bitmap.value = BitmapFactory.decodeFile(localFile.absolutePath)
            }.onFailure { Log.w(TAG, "Failed to load local profile: ${it.message}") }
        }
    }

    // Fetch from server — called on fresh install or if local cache is missing.
    // This restores the lock flag after uninstall/reinstall, then best-effort
    // downloads the image if filePath is exposed as a reachable URL/path.
    suspend fun fetchFromServer(
        api: ElleApiExtended,
        downloadHttpClient: OkHttpClient? = null,
        restBaseUrl: String? = null,
    ) = withContext(Dispatchers.IO) {
        runCatching {
            val avatar = api.getDefaultAvatar()
            Log.i(TAG, "Server avatar: id=${avatar.id} path=${avatar.filePath}")

            prefs.edit()
                .putBoolean(KEY_LOCKED, true)
                .putInt(KEY_AVATAR_ID, avatar.id)
                .apply()
            _isLockedFlow.value = true

            val url = avatar.filePath.toDownloadUrl(restBaseUrl)
            if (downloadHttpClient != null && url != null) {
                downloadAndCache(downloadHttpClient, url)
            }
        }.onFailure {
            Log.d(TAG, "No server avatar yet: ${it.message}")
        }
    }

    // Upload and lock — can only be called once
    suspend fun uploadAndLock(uri: Uri, api: ElleApiExtended): Result<Unit> =
        withContext(Dispatchers.IO) {
            if (isLocked) {
                return@withContext Result.failure(IllegalStateException("Profile picture is locked"))
            }

            runCatching {
                // Decode and resize
                val stream = context.contentResolver.openInputStream(uri)
                    ?: throw IllegalArgumentException("Cannot open image")
                val original = stream.use { inputStream ->
                    BitmapFactory.decodeStream(inputStream)
                } ?: throw IllegalArgumentException("Cannot decode image")

                val resized = resizeBitmap(original, MAX_DIMENSION)

                // Encode to JPEG base64
                val baos = ByteArrayOutputStream()
                resized.compress(Bitmap.CompressFormat.JPEG, 85, baos)
                val bytes = baos.toByteArray()
                val base64 = Base64.encodeToString(bytes, Base64.NO_WRAP)

                // Upload to server as default avatar
                val response = api.uploadAvatar(
                    com.elleann.android.data.models.UploadAvatarRequest(
                        label    = "Elle profile",
                        base64   = base64,
                        mimeType = "image/jpeg",
                        default  = true,
                    )
                )

                // Save locally
                localFile.writeBytes(bytes)

                // Lock it
                prefs.edit()
                    .putBoolean(KEY_LOCKED, true)
                    .putInt(KEY_AVATAR_ID, response.id)
                    .apply()
                _isLockedFlow.value = true

                _bitmap.value = resized

                Log.i(TAG, "Profile picture set and locked (avatar id=${response.id})")
            }
        }

    private fun downloadAndCache(client: OkHttpClient, url: String) {
        runCatching {
            val request = Request.Builder().url(url).build()
            client.newCall(request).execute().use { response ->
                if (!response.isSuccessful) {
                    Log.d(TAG, "Avatar download failed: HTTP ${response.code} for $url")
                    return
                }
                val bytes = response.body?.bytes()
                if (bytes == null || bytes.isEmpty()) return

                val bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
                if (bitmap != null) {
                    localFile.writeBytes(bytes)
                    _bitmap.value = bitmap
                }
            }
        }.onFailure {
            Log.d(TAG, "Avatar download unavailable: ${it.message}")
        }
    }

    private fun String.toDownloadUrl(restBaseUrl: String?): String? {
        if (isBlank()) return null
        if (startsWith("http://") || startsWith("https://")) return this
        val base = restBaseUrl?.takeIf { it.isNotBlank() }?.trimEnd('/') ?: return null
        return "$base/${trimStart('/')}"
    }

    private fun resizeBitmap(src: Bitmap, maxDim: Int): Bitmap {
        val w = src.width
        val h = src.height
        if (w <= maxDim && h <= maxDim) return src
        val scale = maxDim.toFloat() / maxOf(w, h)
        return Bitmap.createScaledBitmap(src, (w * scale).toInt(), (h * scale).toInt(), true)
    }
}
