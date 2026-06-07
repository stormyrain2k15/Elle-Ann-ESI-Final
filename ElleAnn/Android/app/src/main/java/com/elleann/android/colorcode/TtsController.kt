package com.elleann.android.colorcode

import android.content.Context
import android.speech.tts.TextToSpeech
import android.speech.tts.UtteranceProgressListener
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.util.Locale
import java.util.UUID

class TtsController(context: Context) {

    companion object {
        private const val TAG = "TtsController"
    }

    private val _currentWordIndex = MutableStateFlow(-1)
    val currentWordIndex: StateFlow<Int> = _currentWordIndex.asStateFlow()

    private val _isSpeaking = MutableStateFlow(false)
    val isSpeaking: StateFlow<Boolean> = _isSpeaking.asStateFlow()

    private val _isReady = MutableStateFlow(false)
    val isReady: StateFlow<Boolean> = _isReady.asStateFlow()

    private var tts: TextToSpeech? = null
    private var currentText: String = ""
    private var wordRanges: List<IntRange> = emptyList()

    var speed: Float = 1.0f
        set(value) { field = value.coerceIn(0.5f, 2.0f); tts?.setSpeechRate(field) }

    var pitch: Float = 1.0f
        set(value) { field = value.coerceIn(0.5f, 2.0f); tts?.setPitch(field) }

    var locale: Locale = Locale.US
        set(value) { field = value; tts?.language = value }

    var highlightMode: HighlightMode = HighlightMode.WORD

    enum class HighlightMode { WORD, SENTENCE }

    init {
        tts = TextToSpeech(context) { status ->
            if (status == TextToSpeech.SUCCESS) {
                tts?.language = locale
                tts?.setSpeechRate(speed)
                tts?.setPitch(pitch)
                setupUtteranceListener()
                _isReady.value = true
                Log.i(TAG, "TTS initialized successfully")
            } else {
                Log.e(TAG, "TTS initialization failed with status: $status")
            }
        }
    }

    private fun setupUtteranceListener() {
        tts?.setOnUtteranceProgressListener(object : UtteranceProgressListener() {

            override fun onStart(utteranceId: String?) {
                _isSpeaking.value = true
                _currentWordIndex.value = 0
            }

            override fun onDone(utteranceId: String?) {
                _isSpeaking.value = false
                _currentWordIndex.value = -1
            }

            @Deprecated("Deprecated in Java")
            override fun onError(utteranceId: String?) {
                _isSpeaking.value = false
                _currentWordIndex.value = -1
            }

            override fun onRangeStart(utteranceId: String?, start: Int, end: Int, frame: Int) {

                val idx = wordRanges.indexOfFirst { range -> start in range }
                if (idx >= 0) _currentWordIndex.value = idx
            }
        })
    }

    fun speak(text: String) {
        if (!_isReady.value) {
            Log.w(TAG, "TTS not ready — queuing speak after init")
            return
        }

        currentText = text
        wordRanges = computeWordRanges(text)

        tts?.stop()
        _currentWordIndex.value = 0

        val utteranceId = UUID.randomUUID().toString()
        tts?.speak(text, TextToSpeech.QUEUE_FLUSH, null, utteranceId)
    }

    fun stop() {
        tts?.stop()
        _isSpeaking.value = false
        _currentWordIndex.value = -1
    }

    fun pause() = stop()

    fun shutdown() {
        tts?.stop()
        tts?.shutdown()
        tts = null
    }

    private fun computeWordRanges(text: String): List<IntRange> {
        val ranges = mutableListOf<IntRange>()
        val pattern = Regex("""\b\w+\b""")
        pattern.findAll(text).forEach { match ->
            ranges.add(match.range)
        }
        return ranges
    }
}
