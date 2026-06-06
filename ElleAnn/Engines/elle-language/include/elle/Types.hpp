// ============================================================================
// Elle Engine -- Core integer-indexed types
// File:    include/elle/Types.hpp
//
// Every meaningful unit the engine reasons about has a strongly-typed integer
// ID. We deliberately do NOT use raw int / std::uint64_t for IDs at the API
// boundary because a SenseID is not interchangeable with a WordID and the
// type system should enforce that.
//
// All IDs are 64-bit so we never have to migrate at population scale.
// ============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace elle {

// ---------------------------------------------------------------------------
// Strong ID type. Phantom tag distinguishes WordID from SenseID at the type
// system level even though both wrap std::int64_t.
// ---------------------------------------------------------------------------
template <class Tag>
class Id {
public:
    using value_type = std::int64_t;

    constexpr Id() noexcept = default;
    constexpr explicit Id(value_type v) noexcept : m_value(v) {}

    [[nodiscard]] constexpr value_type value() const noexcept { return m_value; }
    [[nodiscard]] constexpr bool       valid() const noexcept { return m_value > 0; }

    friend constexpr bool operator==(Id a, Id b) noexcept { return a.m_value == b.m_value; }
    friend constexpr bool operator!=(Id a, Id b) noexcept { return a.m_value != b.m_value; }
    friend constexpr bool operator< (Id a, Id b) noexcept { return a.m_value <  b.m_value; }

private:
    value_type m_value = 0;
};

// Phantom tags ---------------------------------------------------------------
struct WordTag        {};
struct WordFormTag    {};
struct SenseTag       {};
struct PhraseTag      {};
struct PhraseSenseTag {};
struct ConceptTag     {};
struct ContextTag     {};
struct NodeTag        {};
struct RelationTag    {};
struct PosTag         {};
struct EmotionTag     {};

using WordID         = Id<WordTag>;
using WordFormID     = Id<WordFormTag>;
using SenseID        = Id<SenseTag>;
using PhraseID       = Id<PhraseTag>;
using PhraseSenseID  = Id<PhraseSenseTag>;
using ConceptID      = Id<ConceptTag>;
using ContextID      = Id<ContextTag>;
using SemanticNodeID = Id<NodeTag>;
using RelationTypeID = Id<RelationTag>;
using PartOfSpeechID = Id<PosTag>;
using EmotionID      = Id<EmotionTag>;

// ---------------------------------------------------------------------------
// Stable RelationType integers -- must match 02_seed_lexicon.sql
// ---------------------------------------------------------------------------
namespace rel {
    inline constexpr RelationTypeID SYNONYM         { 1};
    inline constexpr RelationTypeID ANTONYM         { 2};
    inline constexpr RelationTypeID HOMONYM         { 3};
    inline constexpr RelationTypeID HOMOPHONE       { 4};
    inline constexpr RelationTypeID HOMOGRAPH       { 5};
    inline constexpr RelationTypeID HETERONYM       { 6};
    inline constexpr RelationTypeID PARAPHRASE      { 7};
    inline constexpr RelationTypeID HYPERNYM        { 8};
    inline constexpr RelationTypeID HYPONYM         { 9};
    inline constexpr RelationTypeID MERONYM         {10};
    inline constexpr RelationTypeID HOLONYM         {11};
    inline constexpr RelationTypeID RELATED_CONCEPT {12};
    inline constexpr RelationTypeID CONTRAST        {13};
    inline constexpr RelationTypeID CAUSE           {14};
    inline constexpr RelationTypeID EFFECT          {15};
} // namespace rel

// ---------------------------------------------------------------------------
// Stable EmotionID integers -- must match 02_seed_lexicon.sql
// ---------------------------------------------------------------------------
namespace emo {
    inline constexpr EmotionID VALENCE    { 1};
    inline constexpr EmotionID ANGER      { 2};
    inline constexpr EmotionID FEAR       { 3};
    inline constexpr EmotionID SADNESS    { 4};
    inline constexpr EmotionID JOY        { 5};
    inline constexpr EmotionID TRUST      { 6};
    inline constexpr EmotionID TENDERNESS { 7};
    inline constexpr EmotionID COMFORT    { 8};
    inline constexpr EmotionID SHAME      { 9};
    inline constexpr EmotionID CURIOSITY  {10};
    inline constexpr EmotionID POS_DRAW   {11};
    inline constexpr EmotionID NEG_DRAW   {12};
} // namespace emo

// ===========================================================================
// Data records returned by the SQL access layer
// ===========================================================================
struct WordRecord {
    WordID       wordId;
    std::string  lemma;
    std::string  normalizedLemma;
    bool         isPalindrome  = false;
    std::int64_t frequency     = 0;
};

struct WordFormRecord {
    WordFormID    wordFormId;
    WordID        wordId;
    std::string   form;
    std::string   normalizedForm;
    std::optional<PartOfSpeechID> partOfSpeechId;
    std::string   inflectionTag;
};

struct SenseRecord {
    SenseID       senseId;
    WordID        wordId;
    std::optional<PartOfSpeechID> partOfSpeechId;
    std::string   definition;
    std::string   gloss;
    double        positiveDraw = 0.0;
    double        negativeDraw = 0.0;
    double        valence      = 0.0;
    std::int64_t  frequency    = 0;
    int           senseOrder   = 0;
};

struct PhraseRecord {
    PhraseID      phraseId;
    std::string   surface;
    std::string   normalizedForm;
    int           wordCount = 0;
    std::int64_t  frequency = 0;
    std::vector<WordID> wordSequence;   // positions 0..N-1
};

struct PhraseSenseRecord {
    PhraseSenseID phraseSenseId;
    PhraseID      phraseId;
    std::string   definition;
    std::string   gloss;
    double        positiveDraw = 0.0;
    double        negativeDraw = 0.0;
    double        valence      = 0.0;
    std::int64_t  frequency    = 0;
    int           senseOrder   = 0;
};

struct ContextFrameRecord {
    ContextID    contextId;
    std::string  code;
    std::string  name;
    std::string  toneHint;
    std::string  intentHint;
    double       valence = 0.0;
};

struct ContextKeywordRecord {
    ContextID                  contextId;
    std::optional<WordID>      wordId;
    std::optional<PhraseID>    phraseId;
    double                     weight = 0.0;
};

struct EmotionWeight {
    EmotionID emotionId;
    double    weight = 0.0;
};

struct RelationRecord {
    std::int64_t fromId;
    std::int64_t toId;
    RelationTypeID relationTypeId;
    double         strength = 0.0;
};

struct SemanticEdge {
    SemanticNodeID fromNode;
    SemanticNodeID toNode;
    RelationTypeID relationTypeId;
    double         strength   = 1.0;
    double         confidence = 1.0;
};

struct ConceptMemberRecord {
    ConceptID                     conceptId;
    std::optional<SenseID>        senseId;
    std::optional<PhraseSenseID>  phraseSenseId;
    double                        strength = 1.0;
};

// ===========================================================================
// Engine-internal sentence structures
// ===========================================================================

// A single normalized input chunk (NOT an LLM token).
struct Lexeme {
    std::string  originalSpan;    // exactly as in the raw input
    std::string  normalized;      // lower-cased, contractions preserved
    std::size_t  startByte = 0;   // byte offsets in raw input
    std::size_t  endByte   = 0;
    bool         isPunctuation = false;
    bool         isQuotedRegion = false;
    int          emotionalPunctuationIntensity = 0;  // count of ! / ? / ...
};

// Output of phrase scan + lexeme lookup, one entry per consumed sentence span.
struct WordUnit {
    std::vector<Lexeme>      lexemes;          // lexemes this unit absorbs (1..N)
    std::string              originalSpan;
    std::string              normalized;
    std::optional<WordID>    wordId;
    std::optional<WordFormID> wordFormId;
    std::optional<PhraseID>  phraseId;
    std::vector<SenseID>       senseCandidates;
    std::vector<PhraseSenseID> phraseSenseCandidates;
    std::size_t              positionInSentence = 0;
    bool                     isUnknown = false;
};

// The integer-backed sequence the rest of the engine operates on.
struct IntegerSequence {
    std::vector<WordUnit> units;
    int                   exclamationCount  = 0;
    int                   questionCount     = 0;
    int                   ellipsisCount     = 0;
    bool                  endsWithQuestion  = false;
    bool                  endsWithExclaim   = false;
    bool                  containsQuoted    = false;
};

// One scored candidate after the sense-resolution pass.
struct ScoredSense {
    std::optional<SenseID>       senseId;
    std::optional<PhraseSenseID> phraseSenseId;
    double                       score = 0.0;
    std::unordered_map<std::string, double> scoreBreakdown;
    std::string                  reason;
};

// Final per-unit resolution.
struct ResolvedSense {
    std::size_t                  unitIndex = 0;
    std::optional<SenseID>       chosenSenseId;
    std::optional<PhraseSenseID> chosenPhraseSenseId;
    double                       confidence = 0.0;
    std::vector<ScoredSense>     rankedCandidates;
};

// Per-sentence context match.
struct ContextFrameMatch {
    ContextID   contextId;
    std::string code;
    std::string name;
    double      score = 0.0;
    std::unordered_map<std::string, double> contributingKeywords;  // keyword label -> weight contribution
};

// Per-sentence emotional profile.
struct EmotionalProfile {
    std::unordered_map<std::int64_t, double> byEmotionId;          // EmotionID.value() -> aggregated weight
    double valence      = 0.0;
    double positiveDraw = 0.0;
    double negativeDraw = 0.0;

    [[nodiscard]] double get(EmotionID e) const noexcept {
        auto it = byEmotionId.find(e.value());
        return it == byEmotionId.end() ? 0.0 : it->second;
    }
};

struct ConceptPath {
    std::vector<SemanticNodeID> nodes;
    std::vector<SemanticEdge>   edges;
    double                      totalStrength = 0.0;
};

struct MeaningObject {
    std::string                  rawInput;
    std::string                  normalizedInput;
    IntegerSequence              sequence;
    std::vector<ResolvedSense>   resolvedSenses;
    std::vector<std::string>     unresolvedWords;
    std::vector<ContextFrameMatch> contextFrames;
    EmotionalProfile             emotionalProfile;
    std::string                  likelyIntent;
    std::vector<ConceptPath>     conceptPaths;
    double                       overallConfidence = 0.0;
    std::string                  explanationTrace;       // human-readable, multi-line
};

// Caller-supplied conversation context that biases scoring.
struct ConversationContext {
    std::vector<ContextID> activeContextHints;            // e.g. WILDLIFE_CONTEXT
    std::vector<WordID>    recentWordIds;
    std::string            speakerRelationship;          // "stranger", "intimate", ...
    bool                   prefersBaseball = false;       // reserved hint
};

} // namespace elle

// ---------------------------------------------------------------------------
// Hash support so strong IDs are usable as keys in unordered_map / LruCache.
// ---------------------------------------------------------------------------
namespace std {
template <class Tag>
struct hash<::elle::Id<Tag>> {
    std::size_t operator()(const ::elle::Id<Tag>& id) const noexcept {
        return std::hash<std::int64_t>{}(id.value());
    }
};
} // namespace std
