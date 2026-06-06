// ============================================================================
// Elle Probability Engine -- Core types
// File: include/elle/prob/Types.hpp
//
// Every distribution, belief record, evidence packet, and scored hypothesis
// that the probability engine reasons about is defined here.
//
// Design principles:
//   - All IDs are mirrored from elle::Types (WordID, SenseID, EmotionID …)
//     so the probability engine speaks the same integer language as the
//     language engine with zero translation overhead.
//   - Distributions are always explicit. There are no hidden "confidence"
//     scalars that paper over a real distribution.
//   - Evidence is typed and timestamped. The engine can explain every
//     belief it holds and trace it back to the evidence that formed it.
//   - Thread safety: all mutable state lives inside the service layer behind
//     mutexes. These types are plain data; callers own synchronisation.
// ============================================================================
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// We mirror the strong-ID machinery from the language engine so both engines
// share the same integer vocabulary without a dependency on the full engine.
namespace elle { namespace prob {

// ---------------------------------------------------------------------------
// Epoch clock: nanoseconds since Unix epoch. Used to timestamp evidence and
// decay schedules.
// ---------------------------------------------------------------------------
using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>;

inline Timestamp now() noexcept {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now());
}

inline double secondsSince(Timestamp t) noexcept {
    using namespace std::chrono;
    return duration_cast<duration<double>>(now() - t).count();
}

// ---------------------------------------------------------------------------
// Discrete probability distribution over a finite hypothesis space H.
// Each element maps a hypothesis key (int64) to a probability mass.
// Invariant: sum of all masses == 1.0  (enforced by normalize()).
// ---------------------------------------------------------------------------
struct Distribution {
    // hypothesis id -> probability mass
    std::unordered_map<std::int64_t, double> mass;

    // Renormalize so masses sum to 1.  If all masses are zero the
    // distribution becomes uniform over the existing keys.
    void normalize();

    // Returns P(h) or 0.0 if h is not in the distribution.
    [[nodiscard]] double p(std::int64_t h) const noexcept;

    // Entropy H = -sum p(h) log2 p(h). High entropy = uncertain.
    [[nodiscard]] double entropy() const noexcept;

    // MAP estimate: hypothesis with highest mass.  Returns -1 if empty.
    [[nodiscard]] std::int64_t map() const noexcept;

    // Sample one hypothesis proportional to mass. Uses the provided uniform
    // random [0,1) draw so the caller controls the RNG.
    [[nodiscard]] std::int64_t sample(double u01) const noexcept;

    // True iff the distribution is empty.
    [[nodiscard]] bool empty() const noexcept { return mass.empty(); }

    // Number of hypotheses with nonzero mass.
    [[nodiscard]] std::size_t support() const noexcept;
};

// ---------------------------------------------------------------------------
// Multidimensional weight vector used for the language-engine scoring
// weights.  Replaces the static ScoringWeights struct; the probability
// engine maintains a live Distribution over each dimension and updates
// them from evidence.
//
// Dimension names mirror ScoringWeights in the language engine exactly so
// the bridge layer can substitute them with zero renaming.
// ---------------------------------------------------------------------------
struct WeightVector {
    double contextFrameMatch   = 1.0;
    double nearbyWordCooccur   = 0.6;
    double senseExampleOverlap = 0.5;
    double emotionalAlignment  = 0.7;
    double frequency           = 0.3;
    double posCompatibility    = 0.4;
    double posNegDrawAlignment = 0.5;
    double conversationHint    = 0.8;

    // Element-wise product with a second vector (for evidence-weighted blend).
    [[nodiscard]] WeightVector hadamard(const WeightVector& rhs) const noexcept;

    // L1 norm (sum of absolute values).
    [[nodiscard]] double l1() const noexcept;

    // Interpolate toward target by alpha in [0,1].
    [[nodiscard]] WeightVector lerp(const WeightVector& target, double alpha) const noexcept;
};

// ---------------------------------------------------------------------------
// Evidence types. Every piece of evidence that updates a belief must carry
// one of these tags so the audit trail is interpretable.
// ---------------------------------------------------------------------------
enum class EvidenceKind : std::uint8_t {
    LEXICAL_MATCH       = 0,   // a word/phrase resolved cleanly
    CONTEXT_FRAME       = 1,   // a context frame fired
    EMOTIONAL_SIGNAL    = 2,   // emotional weight from a resolved sense
    RELATIONAL_LINK     = 3,   // synonym / antonym / hypernym traversal
    CONVERSATION_TURN   = 4,   // whole-turn outcome fed back as reward
    EXTERNAL_CLAIM      = 5,   // caller-supplied assertion
    PRIOR_DECAY         = 6,   // time-based prior update
    SELF_MODEL          = 7,   // Elle's own internal state feeds evidence
    CONCEPTUAL_PATH     = 8,   // semantic graph traversal completed
    SPEAKER_SIGNAL      = 9,   // speaker relationship / trust level change
    CORRECTION          = 10,  // explicit correction from a trusted source
};

// ---------------------------------------------------------------------------
// A single evidence packet submitted to the engine.
// ---------------------------------------------------------------------------
struct Evidence {
    EvidenceKind kind         = EvidenceKind::LEXICAL_MATCH;

    // The hypothesis space this evidence bears on.
    // semantics depend on the target belief (SenseID space, EmotionID space, …)
    std::int64_t  hypothesisId = 0;

    // Likelihood ratio P(evidence | hypothesis true) / P(evidence | hypothesis false).
    // Must be > 0. ratio > 1 supports the hypothesis. ratio < 1 undermines it.
    double        likelihoodRatio = 1.0;

    // How strongly this evidence source is trusted [0, 1].
    double        sourceWeight   = 1.0;

    Timestamp     observedAt;

    // Human-readable reason for audit trace.
    std::string   reason;
};

// ---------------------------------------------------------------------------
// Belief record: a Bayesian posterior over one hypothesis space, with full
// evidence history and a decay half-life.
// ---------------------------------------------------------------------------
struct Belief {
    // Identifier for what this belief is about. Convention:
    //   "sense:42"       -> SenseID 42
    //   "emotion:5"      -> EmotionID 5
    //   "context:3"      -> ContextID 3
    //   "weight:contextFrameMatch" -> scoring weight dimension
    //   "intent:*"       -> pragmatic intent label
    //   "speaker:trust"  -> trust level for the current speaker
    std::string   domain;

    // Current posterior distribution.
    Distribution  posterior;

    // Prior used when the belief was created or last reset.
    Distribution  prior;

    // All evidence that has contributed to this belief since last reset.
    std::vector<Evidence> evidenceLog;

    // Timestamp of last update.
    Timestamp     lastUpdated;

    // Half-life in seconds. After this many seconds without new evidence the
    // posterior decays back toward the prior.  0 = no decay.
    double        halfLifeSecs = 0.0;

    // Apply decay: blend posterior toward prior based on elapsed time.
    void decayTowardPrior();

    // Current entropy of the posterior (uncertainty measure).
    [[nodiscard]] double uncertainty() const noexcept { return posterior.entropy(); }
};

// ---------------------------------------------------------------------------
// Pragmatic intent: what is the speaker trying to DO with this utterance?
// These are the speech-act categories (Austin / Grice / Searle) that the
// probability engine assigns distributions over.
// ---------------------------------------------------------------------------
enum class PragmaticAct : std::uint8_t {
    ASSERT      = 0,   // stating something as fact
    QUESTION    = 1,   // requesting information
    REQUEST     = 2,   // requesting action
    OFFER       = 3,   // proposing to do something
    PROMISE     = 4,   // committing to a future action
    WARN        = 5,   // alerting to danger / consequence
    GREET       = 6,   // social opening / closing
    APOLOGIZE   = 7,
    THANK       = 8,
    COMFORT     = 9,   // offering emotional support
    DEFLECT     = 10,  // avoiding a topic
    CHALLENGE   = 11,  // contesting a claim
    CONFIRM     = 12,  // affirming prior understanding
    DENY        = 13,
    UNKNOWN     = 14,
};

// ---------------------------------------------------------------------------
// A scored hypothesis: one candidate with its probability and explanation.
// Used in ProbabilityResult to expose ranked candidates.
// ---------------------------------------------------------------------------
struct ScoredHypothesis {
    std::int64_t  hypothesisId = 0;
    double        probability  = 0.0;
    double        entropy      = 0.0;   // marginal uncertainty at this hypothesis
    std::string   label;                // human-readable name
    std::unordered_map<std::string, double> contributingFactors;
};

// ---------------------------------------------------------------------------
// Full result returned by the probability engine for one analysis request.
// ---------------------------------------------------------------------------
struct ProbabilityResult {
    // Per-unit winning sense and its posterior probability.
    struct UnitResult {
        std::size_t  unitIndex       = 0;
        std::int64_t winningSenseId  = 0;  // SenseID or PhraseSenseID
        bool         isPhraseSense   = false;
        double       winnerProbability = 0.0;
        double       posteriorEntropy  = 0.0;  // uncertainty of this choice
        std::vector<ScoredHypothesis> rankedCandidates;
    };

    std::vector<UnitResult>         units;

    // Live scoring weights recommended for this analysis
    // (posterior MAP estimate from the weight belief).
    WeightVector                    recommendedWeights;

    // Distribution over PragmaticAct for this utterance.
    Distribution                    intentDistribution;

    // Top-ranked pragmatic act.
    PragmaticAct                    likelyAct = PragmaticAct::UNKNOWN;

    // Emotional distribution across all 12 emotion dimensions.
    // key = EmotionID.value(), value = posterior probability mass.
    Distribution                    emotionalPosterior;

    // Speaker trust estimate [0, 1].
    double                          speakerTrust = 0.5;

    // Overall confidence: geometric mean of per-unit winner probabilities.
    double                          overallConfidence = 0.0;

    // Full audit trace (JSON-serialisable).
    std::string                     traceJson;
};

// ---------------------------------------------------------------------------
// Request submitted to the probability engine. Mirrors the language engine's
// IntegerSequence + ConversationContext but adds the probability-specific
// fields the caller can supply.
// ---------------------------------------------------------------------------
struct ProbabilityRequest {
    // Integer sequence from the language engine (all IDs).
    // Passed by value so the probability engine can work concurrently.
    struct UnitSpec {
        std::size_t  unitIndex    = 0;
        std::int64_t wordId       = 0;   // 0 = phrase
        std::int64_t phraseId     = 0;
        bool         isPhrase     = false;
        bool         isUnknown    = false;
        std::vector<std::int64_t> senseCandidateIds;
        std::vector<std::int64_t> phraseSenseCandidateIds;
    };

    std::vector<UnitSpec>   units;

    // Pre-computed context frame scores from the language engine.
    struct ContextHint {
        std::int64_t contextId = 0;
        double       score     = 0.0;
    };
    std::vector<ContextHint> contextHints;

    // Pre-computed emotional profile from the language engine.
    // key = EmotionID.value(), value = aggregated weight.
    std::unordered_map<std::int64_t, double> emotionalProfile;

    // Caller-supplied conversation context.
    std::string speakerRelationship;  // "stranger", "intimate", "trusted", ...
    double      speakerTrustOverride = -1.0;  // negative = use internal estimate

    // Punctuation intensity signals.
    int  exclamationCount = 0;
    int  questionCount    = 0;
    int  ellipsisCount    = 0;
    bool endsWithQuestion = false;
    bool endsWithExclaim  = false;

    // If true the engine samples from distributions rather than taking MAP.
    // Use false for deterministic/test runs.
    bool stochastic = false;

    // Seed for stochastic sampling (0 = use internal PRNG).
    std::uint64_t randomSeed = 0;
};

} } // namespace elle::prob
