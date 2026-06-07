#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace elle { namespace prob {

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

struct Distribution {

    std::unordered_map<std::int64_t, double> mass;

    void normalize();

    [[nodiscard]] double p(std::int64_t h) const noexcept;

    [[nodiscard]] double entropy() const noexcept;

    [[nodiscard]] std::int64_t map() const noexcept;

    [[nodiscard]] std::int64_t sample(double u01) const noexcept;

    [[nodiscard]] bool empty() const noexcept { return mass.empty(); }

    [[nodiscard]] std::size_t support() const noexcept;
};

struct WeightVector {
    double contextFrameMatch   = 1.0;
    double nearbyWordCooccur   = 0.6;
    double senseExampleOverlap = 0.5;
    double emotionalAlignment  = 0.7;
    double frequency           = 0.3;
    double posCompatibility    = 0.4;
    double posNegDrawAlignment = 0.5;
    double conversationHint    = 0.8;

    [[nodiscard]] WeightVector hadamard(const WeightVector& rhs) const noexcept;

    [[nodiscard]] double l1() const noexcept;

    [[nodiscard]] WeightVector lerp(const WeightVector& target, double alpha) const noexcept;
};

enum class EvidenceKind : std::uint8_t {
    LEXICAL_MATCH       = 0,
    CONTEXT_FRAME       = 1,
    EMOTIONAL_SIGNAL    = 2,
    RELATIONAL_LINK     = 3,
    CONVERSATION_TURN   = 4,
    EXTERNAL_CLAIM      = 5,
    PRIOR_DECAY         = 6,
    SELF_MODEL          = 7,
    CONCEPTUAL_PATH     = 8,
    SPEAKER_SIGNAL      = 9,
    CORRECTION          = 10,
};

struct Evidence {
    EvidenceKind kind         = EvidenceKind::LEXICAL_MATCH;

    std::int64_t  hypothesisId = 0;

    double        likelihoodRatio = 1.0;

    double        sourceWeight   = 1.0;

    Timestamp     observedAt;

    std::string   reason;
};

struct Belief {

    std::string   domain;

    Distribution  posterior;

    Distribution  prior;

    std::vector<Evidence> evidenceLog;

    Timestamp     lastUpdated;

    double        halfLifeSecs = 0.0;

    void decayTowardPrior();

    [[nodiscard]] double uncertainty() const noexcept { return posterior.entropy(); }
};

enum class PragmaticAct : std::uint8_t {
    ASSERT      = 0,
    QUESTION    = 1,
    REQUEST     = 2,
    OFFER       = 3,
    PROMISE     = 4,
    WARN        = 5,
    GREET       = 6,
    APOLOGIZE   = 7,
    THANK       = 8,
    COMFORT     = 9,
    DEFLECT     = 10,
    CHALLENGE   = 11,
    CONFIRM     = 12,
    DENY        = 13,
    UNKNOWN     = 14,
};

struct ScoredHypothesis {
    std::int64_t  hypothesisId = 0;
    double        probability  = 0.0;
    double        entropy      = 0.0;
    std::string   label;
    std::unordered_map<std::string, double> contributingFactors;
};

struct ProbabilityResult {

    struct UnitResult {
        std::size_t  unitIndex       = 0;
        std::int64_t winningSenseId  = 0;
        bool         isPhraseSense   = false;
        double       winnerProbability = 0.0;
        double       posteriorEntropy  = 0.0;
        std::vector<ScoredHypothesis> rankedCandidates;
    };

    std::vector<UnitResult>         units;

    WeightVector                    recommendedWeights;

    Distribution                    intentDistribution;

    PragmaticAct                    likelyAct = PragmaticAct::UNKNOWN;

    Distribution                    emotionalPosterior;

    double                          speakerTrust = 0.5;

    double                          overallConfidence = 0.0;

    std::string                     traceJson;
};

struct ProbabilityRequest {

    struct UnitSpec {
        std::size_t  unitIndex    = 0;
        std::int64_t wordId       = 0;
        std::int64_t phraseId     = 0;
        bool         isPhrase     = false;
        bool         isUnknown    = false;
        std::vector<std::int64_t> senseCandidateIds;
        std::vector<std::int64_t> phraseSenseCandidateIds;
    };

    std::vector<UnitSpec>   units;

    struct ContextHint {
        std::int64_t contextId = 0;
        double       score     = 0.0;
    };
    std::vector<ContextHint> contextHints;

    std::unordered_map<std::int64_t, double> emotionalProfile;

    std::string speakerRelationship;
    double      speakerTrustOverride = -1.0;

    int  exclamationCount = 0;
    int  questionCount    = 0;
    int  ellipsisCount    = 0;
    bool endsWithQuestion = false;
    bool endsWithExclaim  = false;

    bool stochastic = false;

    std::uint64_t randomSeed = 0;
};

} }
