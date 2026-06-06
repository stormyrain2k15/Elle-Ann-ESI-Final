// ============================================================================
// Elle Probability Engine -- Bridge: language-engine ⇄ probability-engine
// File: src/Bridge.cpp
//
// This is "Integration Point A + B" from include/elle/prob/Bridge.hpp.
// It translates structures owned by the language engine
// (elle::ScoringWeights, elle::MeaningObject, elle::ConversationContext)
// into the probability engine's request/weight types, and back.
//
// Linking: this TU depends on the language-engine headers (Config.hpp,
// Types.hpp) but NOT on any language-engine .cpp. It compiles whenever the
// elle-language tree is on the include path. The probability engine's
// CMakeLists guards adding this file to the build with
// `ELLE_PROB_WITH_LANGUAGE_ENGINE=ON`.
// ============================================================================

#include "elle/prob/Bridge.hpp"

#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"

#include "elle/Config.hpp"   // ScoringWeights
#include "elle/Types.hpp"    // MeaningObject, ConversationContext

#include <utility>

namespace elle { namespace prob {

// ----------------------------------------------------------------------------
// Construction
// ----------------------------------------------------------------------------
Bridge::Bridge(ProbabilityEngineConfig cfg)
    : m_engine(std::make_shared<ProbabilityEngine>(std::move(cfg))) {}

Bridge::Bridge(std::shared_ptr<ProbabilityEngine> engine)
    : m_engine(engine
                   ? std::move(engine)
                   : std::make_shared<ProbabilityEngine>()) {}

// ============================================================================
// Integration Point A — weight translation
// ============================================================================

ScoringWeights Bridge::toScoringWeights(const WeightVector& wv) {
    ScoringWeights sw{};
    sw.contextFrameMatch   = wv.contextFrameMatch;
    sw.nearbyWordCooccur   = wv.nearbyWordCooccur;
    sw.senseExampleOverlap = wv.senseExampleOverlap;
    sw.emotionalAlignment  = wv.emotionalAlignment;
    sw.frequency           = wv.frequency;
    sw.posCompatibility    = wv.posCompatibility;
    sw.posNegDrawAlignment = wv.posNegDrawAlignment;
    sw.conversationHint    = wv.conversationHint;
    return sw;
}

WeightVector Bridge::fromScoringWeights(const ScoringWeights& sw) {
    WeightVector wv{};
    wv.contextFrameMatch   = sw.contextFrameMatch;
    wv.nearbyWordCooccur   = sw.nearbyWordCooccur;
    wv.senseExampleOverlap = sw.senseExampleOverlap;
    wv.emotionalAlignment  = sw.emotionalAlignment;
    wv.frequency           = sw.frequency;
    wv.posCompatibility    = sw.posCompatibility;
    wv.posNegDrawAlignment = sw.posNegDrawAlignment;
    wv.conversationHint    = sw.conversationHint;
    return wv;
}

// ============================================================================
// Integration Point B — MeaningObject + ConversationContext → ProbabilityRequest
// ============================================================================
//
// The language engine's IntegerSequence already carries everything we need
// for unit-level resolution: word IDs, phrase IDs, sense candidates, position,
// and punctuation intensity. The MeaningObject's contextFrames and
// emotionalProfile are summary statistics computed during sentence
// processing. We pipe all of it through verbatim — no estimation, no padding.
//
// The probability engine treats senseCandidateIds and phraseSenseCandidateIds
// as int64_t lists; the language engine stores them as `std::vector<SenseID>`
// / `std::vector<PhraseSenseID>` where Id<Tag>::value() returns the int64.
// ----------------------------------------------------------------------------
ProbabilityRequest Bridge::fromMeaningObject(
    const elle::MeaningObject&       meaning,
    const elle::ConversationContext& convo)
{
    ProbabilityRequest req{};

    // ---- units (one per WordUnit) -----------------------------------------
    req.units.reserve(meaning.sequence.units.size());
    for (std::size_t i = 0; i < meaning.sequence.units.size(); ++i) {
        const auto& wu = meaning.sequence.units[i];

        ProbabilityRequest::UnitSpec u{};
        u.unitIndex = i;
        u.isPhrase  = wu.phraseId.has_value();
        u.isUnknown = wu.isUnknown;
        if (wu.wordId)   u.wordId   = wu.wordId->value();
        if (wu.phraseId) u.phraseId = wu.phraseId->value();

        u.senseCandidateIds.reserve(wu.senseCandidates.size());
        for (const auto& sid : wu.senseCandidates) {
            u.senseCandidateIds.push_back(sid.value());
        }
        u.phraseSenseCandidateIds.reserve(wu.phraseSenseCandidates.size());
        for (const auto& psid : wu.phraseSenseCandidates) {
            u.phraseSenseCandidateIds.push_back(psid.value());
        }

        req.units.push_back(std::move(u));
    }

    // ---- context hints (from MeaningObject.contextFrames) ------------------
    // We trust the language engine's per-sentence frame scores verbatim.
    req.contextHints.reserve(meaning.contextFrames.size()
                             + convo.activeContextHints.size());
    for (const auto& cfm : meaning.contextFrames) {
        ProbabilityRequest::ContextHint h{};
        h.contextId = cfm.contextId.value();
        h.score     = cfm.score;
        req.contextHints.push_back(h);
    }
    // Caller-supplied conversation hints arrive with no score; surface them
    // at a strong-but-not-overriding weight of 1.0 so they cannot drown out
    // sentence-level frame matches whose scores are typically in [0, 5].
    for (const auto& cid : convo.activeContextHints) {
        ProbabilityRequest::ContextHint h{};
        h.contextId = cid.value();
        h.score     = 1.0;
        req.contextHints.push_back(h);
    }

    // ---- emotional profile ------------------------------------------------
    req.emotionalProfile = meaning.emotionalProfile.byEmotionId;

    // ---- conversation context --------------------------------------------
    req.speakerRelationship = convo.speakerRelationship;

    // ---- punctuation signals (mirrored from sequence) --------------------
    req.exclamationCount = meaning.sequence.exclamationCount;
    req.questionCount    = meaning.sequence.questionCount;
    req.ellipsisCount    = meaning.sequence.ellipsisCount;
    req.endsWithQuestion = meaning.sequence.endsWithQuestion;
    req.endsWithExclaim  = meaning.sequence.endsWithExclaim;

    // Deterministic by default — the language engine drives the RNG when it
    // needs stochastic behaviour.
    req.stochastic = false;
    req.randomSeed = 0;

    return req;
}

// ============================================================================
// Engine surface — thin wrappers over ProbabilityEngine
// ============================================================================
ProbabilityResult Bridge::analyze(const ProbabilityRequest& req,
                                  const std::string& speakerId) {
    return m_engine->analyze(req, speakerId);
}

void Bridge::feedback(std::size_t       unitIndex,
                      std::int64_t      confirmedSenseId,
                      bool              isPhrase,
                      double            confidence,
                      const std::string& speakerId) {
    m_engine->feedback(unitIndex, confirmedSenseId, isPhrase, confidence,
                       speakerId);
}

void Bridge::recordTrust(const std::string& speakerId,
                         TrustSignal        signal,
                         double             strength) {
    m_engine->recordTrustSignal(speakerId, signal, strength);
}

void Bridge::injectHormonalState(
    const std::unordered_map<std::int64_t, double>& state) {
    m_engine->seedEmotionalPrior(state);
}

} } // namespace elle::prob
