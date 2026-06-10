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

enum class TtsPlayState { IDLE, PLAYING, PAUSED }

class TtsController(context: Context) {

    companion object {
        private const val TAG = "TtsController"
    }

    private val _currentWordIndex = MutableStateFlow(-1)
    val currentWordIndex: StateFlow<Int> = _currentWordIndex.asStateFlow()

    private val _playState = MutableStateFlow(TtsPlayState.IDLE)
    val playState: StateFlow<TtsPlayState> = _playState.asStateFlow()

    private val _isReady = MutableStateFlow(false)
    val isReady: StateFlow<Boolean> = _isReady.asStateFlow()

    private var tts: TextToSpeech? = null
    private var currentText: String = ""
    private var wordRanges: List<IntRange> = emptyList()
    private var pausedWordIndex: Int = -1

    var speed: Float = 1.0f
        set(value) { field = value.coerceIn(0.5f, 2.0f); tts?.setSpeechRate(field) }

    var pitch: Float = 1.0f
        set(value) { field = value.coerceIn(0.5f, 2.0f); tts?.setPitch(field) }

    var locale: Locale = Locale.US
        set(value) { field = value; tts?.language = value }

    enum class HighlightMode { WORD, SENTENCE }
    var highlightMode: HighlightMode = HighlightMode.WORD

    init {
        tts = TextToSpeech(context) { status ->
            if (status == TextToSpeech.SUCCESS) {
                tts?.language = locale
                tts?.setSpeechRate(speed)
                tts?.setPitch(pitch)
                setupUtteranceListener()
                _isReady.value = true
                Log.i(TAG, "TTS initialized")
            } else {
                Log.e(TAG, "TTS init failed: $status")
            }
        }
    }

    private fun setupUtteranceListener() {
        tts?.setOnUtteranceProgressListener(object : UtteranceProgressListener() {
            override fun onStart(utteranceId: String?) {
                _playState.value = TtsPlayState.PLAYING
                _currentWordIndex.value = 0
            }
            override fun onDone(utteranceId: String?) {
                _playState.value = TtsPlayState.IDLE
                _currentWordIndex.value = -1
                pausedWordIndex = -1
            }
            @Deprecated("Deprecated in Java")
            override fun onError(utteranceId: String?) {
                _playState.value = TtsPlayState.IDLE
                _currentWordIndex.value = -1
            }
            override fun onRangeStart(utteranceId: String?, start: Int, end: Int, frame: Int) {
                val idx = wordRanges.indexOfFirst { range -> start in range }
                if (idx >= 0) _currentWordIndex.value = idx
            }
        })
    }

    // Play from beginning — or resume from pause point if same text
    fun speak(text: String) {
        if (!_isReady.value) return
        if (_playState.value == TtsPlayState.PAUSED && text == currentText && pausedWordIndex > 0) {
            // Resume from paused word
            val resumeFrom = wordRanges.getOrNull(pausedWordIndex)?.let { range ->
                text.substring(range.first.coerceAtMost(text.length))
            } ?: text
            tts?.stop()
            _currentWordIndex.value = pausedWordIndex
            tts?.speak(resumeFrom, TextToSpeech.QUEUE_FLUSH, null, UUID.randomUUID().toString())
            _playState.value = TtsPlayState.PLAYING
        } else {
            // Fresh play
            currentText = text
            wordRanges = computeWordRanges(text)
            pausedWordIndex = -1
            tts?.stop()
            _currentWordIndex.value = 0
            tts?.speak(text, TextToSpeech.QUEUE_FLUSH, null, UUID.randomUUID().toString())
        }
    }

    // Pause — saves word position for resume
    fun pause() {
        if (_playState.value == TtsPlayState.PLAYING) {
            pausedWordIndex = _currentWordIndex.value
            tts?.stop()
            _playState.value = TtsPlayState.PAUSED
        }
    }

    // Stop — clears everything
    fun stop() {
        tts?.stop()
        _playState.value = TtsPlayState.IDLE
        _currentWordIndex.value = -1
        pausedWordIndex = -1
        currentText = ""
    }

    fun shutdown() {
        tts?.stop()
        tts?.shutdown()
        tts = null
    }

    private fun computeWordRanges(text: String): List<IntRange> {
        val ranges = mutableListOf<IntRange>()
        Regex("""\b\w+\b""").findAll(text).forEach { ranges.add(it.range) }
        return ranges
    }
}
