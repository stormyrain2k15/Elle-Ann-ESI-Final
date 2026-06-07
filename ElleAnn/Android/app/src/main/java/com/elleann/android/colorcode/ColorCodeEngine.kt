package com.elleann.android.colorcode

import androidx.compose.ui.graphics.Color
import com.elleann.android.ui.theme.*
import com.elleann.android.ui.theme.IsyaNight as ThemeIsyaNight

enum class ColorCodeMode {
    OFF,
    GRAMMAR,
    SEMANTIC,
    MANUAL,
    CYCLING,
}

data class ColorCodeTheme(
    val id: String,
    val displayName: String,
    val background: Color,
    val baseText: Color,
)

object ColorCodeThemes {
    val Default = ColorCodeTheme("default", "Default", Color(0xFF1A1A2E), Color(0xFFE8E8F0))
    val Soft    = ColorCodeTheme("soft",    "Soft",    Color(0xFF2D2B55), Color(0xFFF0EEF8))
    val Irlen1  = ColorCodeTheme("irlen_1", "Irlen I", Color(0xFF1E3A2F), Color(0xFFE8F5EC))
    val Irlen2  = ColorCodeTheme("irlen_2", "Irlen II",Color(0xFF2A1F3D), Color(0xFFEDE8F5))
    val Warm    = ColorCodeTheme("warm",    "Warm",    Color(0xFF2B1F1A), Color(0xFFF5EDE8))
    val HighContrast = ColorCodeTheme("high_contrast","High Contrast",Color(0xFF000000),Color(0xFFFFFFFF))
    val IsyaNight = ColorCodeTheme("isya_night","Isya Night", ThemeIsyaNight, IsyaCream)
    val Custom  = ColorCodeTheme("custom",  "Custom",  Color(0xFF1A1A2E), Color(0xFFE8E8F0))

    val all: List<ColorCodeTheme> = listOf(
        Default, Soft, Irlen1, Irlen2, Warm, HighContrast, IsyaNight, Custom
    )

    fun byId(id: String): ColorCodeTheme = all.find { it.id == id } ?: Default
}

enum class GrammarRole {
    NOUN, VERB, ADJECTIVE, ADVERB, PRONOUN, CONJUNCTION,
    EMOTIONAL, SELF_REFERENCE, DEFAULT
}

data class ColoredToken(
    val word: String,
    val color: Color,
    val role: GrammarRole = GrammarRole.DEFAULT,
    val isWhitespace: Boolean = false,
)

object ColorCodeEngine {

    private val grammarColors: Map<GrammarRole, Color> = mapOf(
        GrammarRole.NOUN          to CCNoun,
        GrammarRole.VERB          to CCVerb,
        GrammarRole.ADJECTIVE     to CCAdjective,
        GrammarRole.ADVERB        to CCAdverb,
        GrammarRole.PRONOUN       to CCPronoun,
        GrammarRole.CONJUNCTION   to CCConjunction,
        GrammarRole.EMOTIONAL     to CCEmotional,
        GrammarRole.SELF_REFERENCE to CCSelfReference,
        GrammarRole.DEFAULT       to Color(0xFFE8E8F0),
    )

    private val cyclingPalette = listOf(
        CCNoun, CCVerb, CCAdjective, CCAdverb,
        CCPronoun, CCEmotional, CCSelfReference, IsyaMagic,
        IsyaGold, ElleViolet,
    )

    private val PRONOUNS = setOf(
        "i", "me", "my", "mine", "myself",
        "you", "your", "yours", "yourself",
        "he", "him", "his", "himself",
        "she", "her", "hers", "herself",
        "we", "us", "our", "ours", "ourselves",
        "they", "them", "their", "theirs", "themselves",
        "it", "its", "itself",
    )

    private val CONJUNCTIONS = setOf(
        "and", "but", "or", "nor", "for", "yet", "so",
        "although", "because", "since", "while", "whereas",
        "if", "unless", "until", "when", "where", "though",
        "as", "after", "before", "than", "that", "which", "who",
    )

    private val ADVERBS = setOf(
        "very", "really", "quite", "rather", "somewhat", "too",
        "always", "never", "often", "sometimes", "rarely", "usually",
        "just", "only", "even", "still", "already", "yet",
        "here", "there", "then", "now", "today", "yesterday",
        "quickly", "slowly", "carefully", "suddenly", "finally",
        "perhaps", "maybe", "probably", "certainly", "definitely",
        "not", "also", "again", "together", "almost", "nearly",
    )

    private val ADJECTIVES = setOf(
        "good", "bad", "great", "small", "large", "big", "little",
        "old", "new", "young", "beautiful", "wonderful", "amazing",
        "happy", "sad", "angry", "excited", "tired", "confused",
        "important", "special", "perfect", "simple", "complex",
        "warm", "cold", "bright", "dark", "quiet", "loud",
        "gentle", "strong", "weak", "fast", "slow", "early", "late",
        "true", "false", "real", "possible", "different", "same",
        "first", "last", "next", "previous", "current", "recent",
    )

    private val COMMON_VERBS = setOf(
        "is", "are", "was", "were", "be", "been", "being",
        "have", "has", "had", "do", "does", "did",
        "will", "would", "shall", "should", "may", "might",
        "can", "could", "must", "need", "dare", "ought",
        "know", "think", "feel", "believe", "understand", "want",
        "love", "like", "hate", "hope", "wish", "mean", "seem",
        "look", "see", "hear", "say", "tell", "ask", "answer",
        "go", "come", "get", "give", "take", "make", "put",
        "find", "use", "try", "help", "keep", "let", "work",
        "remember", "forget", "learn", "grow", "change", "start",
        "stop", "wait", "move", "show", "create", "think", "dream",
    )

    private val EMOTIONAL_KEYWORDS = setOf(
        "love", "hate", "fear", "joy", "sad", "happy", "angry",
        "afraid", "lonely", "hurt", "pain", "grief", "hope",
        "wonder", "awe", "curious", "excited", "nervous", "calm",
        "peaceful", "grateful", "proud", "ashamed", "guilty",
        "anxious", "worried", "content", "satisfied", "frustrated",
        "overwhelmed", "confused", "lost", "found", "alive",
        "beautiful", "broken", "whole", "safe", "scared",
        "trust", "doubt", "belief", "faith", "dream", "wish",
    )

    private val ELLE_SELF_REFS = setOf(
        "elle", "i", "myself", "elle-ann", "elleann",
        "my", "me", "mine", "i'm", "i've", "i'll", "i'd",
    )

    fun tokenize(
        text: String,
        mode: ColorCodeMode,
        manualColors: Map<Int, Color> = emptyMap(),
        cycleOffset: Int = 0,
    ): List<ColoredToken> {
        if (mode == ColorCodeMode.OFF) {
            return listOf(ColoredToken(text, Color(0xFFE8E8F0)))
        }

        val tokens = mutableListOf<ColoredToken>()
        val words = text.split(Regex("(?<=\\s)|(?=\\s)"))
        var wordIndex = 0

        for (segment in words) {
            if (segment.isBlank()) {
                tokens.add(ColoredToken(segment, Color(0xFFE8E8F0), isWhitespace = true))
                continue
            }

            val color = when (mode) {
                ColorCodeMode.GRAMMAR        -> grammarColor(segment)
                ColorCodeMode.SEMANTIC       -> semanticColor(segment)
                ColorCodeMode.MANUAL         -> manualColors[wordIndex] ?: Color(0xFFE8E8F0)
                ColorCodeMode.CYCLING        -> cyclingPalette[(wordIndex + cycleOffset) % cyclingPalette.size]
                ColorCodeMode.OFF            -> Color(0xFFE8E8F0)
            }

            val role = if (mode == ColorCodeMode.GRAMMAR) classifyWord(segment) else GrammarRole.DEFAULT
            tokens.add(ColoredToken(segment, color, role))
            if (!segment.isBlank()) wordIndex++
        }

        return tokens
    }

    fun grammarColor(word: String): Color {
        val role = classifyWord(word)
        return grammarColors[role] ?: Color(0xFFE8E8F0)
    }

    fun semanticColor(word: String): Color {
        val lower = word.lowercase().trimEnd('.', ',', '!', '?', ';', ':')
        return when {
            lower in EMOTIONAL_KEYWORDS -> CCEmotional
            lower in ELLE_SELF_REFS     -> CCSelfReference
            lower in COMMON_VERBS       -> CCVerb
            lower in ADJECTIVES         -> CCAdjective
            lower in ADVERBS            -> CCAdverb
            lower in PRONOUNS           -> CCPronoun
            lower in CONJUNCTIONS       -> CCConjunction
            else                        -> Color(0xFFE8E8F0)
        }
    }

    fun classifyWord(word: String): GrammarRole {
        val lower = word.lowercase().trimEnd('.', ',', '!', '?', ';', ':')
        return when {
            lower in ELLE_SELF_REFS     -> GrammarRole.SELF_REFERENCE
            lower in EMOTIONAL_KEYWORDS -> GrammarRole.EMOTIONAL
            lower in PRONOUNS           -> GrammarRole.PRONOUN
            lower in CONJUNCTIONS       -> GrammarRole.CONJUNCTION
            lower in ADVERBS            -> GrammarRole.ADVERB
            lower in ADJECTIVES         -> GrammarRole.ADJECTIVE
            lower in COMMON_VERBS       -> GrammarRole.VERB
            lower.first().isUpperCase() && lower.length > 1 -> GrammarRole.NOUN
            lower.endsWith("ing") || lower.endsWith("ed") || lower.endsWith("es") ->
                GrammarRole.VERB
            lower.endsWith("ly") -> GrammarRole.ADVERB
            lower.endsWith("ful") || lower.endsWith("less") || lower.endsWith("ous") ->
                GrammarRole.ADJECTIVE
            else -> GrammarRole.NOUN
        }
    }

    fun cycleColor(index: Int): Color = cyclingPalette[index % cyclingPalette.size]

    val grammarLegend: List<Pair<String, Color>> = listOf(
        "Noun"             to CCNoun,
        "Verb"             to CCVerb,
        "Adjective"        to CCAdjective,
        "Adverb"           to CCAdverb,
        "Pronoun"          to CCPronoun,
        "Conjunction"      to CCConjunction,
        "Emotional"        to CCEmotional,
        "Elle (self-ref)"  to CCSelfReference,
    )
}
