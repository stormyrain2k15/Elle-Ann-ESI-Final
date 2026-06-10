package com.elleann.android.ui.chat

import android.content.Intent
import android.os.Bundle
import android.speech.RecognitionListener
import android.speech.RecognizerIntent
import android.speech.SpeechRecognizer
import android.util.Log
import androidx.compose.runtime.*
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

enum class SttState { IDLE, LISTENING, ERROR }

/**
 * SttController wraps Android's SpeechRecognizer.
 *
 * Usage:
 *   val stt = remember { SttController(context) }
 *   stt.startListening { text -> vm.onInput(vm.state.value.inputText + text) }
 *   stt.stopListening()
 *   DisposableEffect { onDispose { stt.destroy() } }
 */
class SttController(private val context: android.content.Context) {

    companion object {
        private const val TAG = "SttController"
    }

    private val _state = MutableStateFlow(SttState.IDLE)
    val state: StateFlow<SttState> = _state.asStateFlow()

    private var recognizer: SpeechRecognizer? = null
    private var onResult: ((String) -> Unit)? = null

    val isAvailable: Boolean get() = SpeechRecognizer.isRecognitionAvailable(context)

    fun startListening(onText: (String) -> Unit) {
        if (!isAvailable) {
            _state.value = SttState.ERROR
            return
        }
        onResult = onText
        recognizer?.destroy()
        recognizer = SpeechRecognizer.createSpeechRecognizer(context)
        recognizer?.setRecognitionListener(object : RecognitionListener {
            override fun onReadyForSpeech(params: Bundle?) { _state.value = SttState.LISTENING }
            override fun onBeginningOfSpeech() {}
            override fun onRmsChanged(rmsdB: Float) {}
            override fun onBufferReceived(buffer: ByteArray?) {}
            override fun onEndOfSpeech() { _state.value = SttState.IDLE }
            override fun onError(error: Int) {
                Log.w(TAG, "STT error: $error")
                _state.value = SttState.IDLE
            }
            override fun onResults(results: Bundle?) {
                val matches = results?.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION)
                val text = matches?.firstOrNull()?.trim() ?: ""
                if (text.isNotEmpty()) onResult?.invoke(text)
                _state.value = SttState.IDLE
            }
            override fun onPartialResults(partialResults: Bundle?) {}
            override fun onEvent(eventType: Int, params: Bundle?) {}
        })

        val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
            putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
            putExtra(RecognizerIntent.EXTRA_PARTIAL_RESULTS, false)
            putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 1)
        }
        recognizer?.startListening(intent)
    }

    fun stopListening() {
        recognizer?.stopListening()
        _state.value = SttState.IDLE
    }

    fun destroy() {
        recognizer?.destroy()
        recognizer = null
    }
}
