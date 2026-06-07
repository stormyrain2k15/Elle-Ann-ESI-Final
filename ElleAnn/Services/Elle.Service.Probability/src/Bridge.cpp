#include "elle/prob/Bridge.hpp"

#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"

#include "elle/Config.hpp"
#include "elle/Types.hpp"

#include <utility>

namespace elle { namespace prob {

Bridge::Bridge(ProbabilityEngineConfig cfg)
    : m_engine(std::make_shared<ProbabilityEngine>(std::move(cfg))) {}

Bridge::Bridge(std::shared_ptr<ProbabilityEngine> engine)
    : m_engine(engine
                   ? std::move(engine)
                   : std::make_shared<ProbabilityEngine>()) {}

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

ProbabilityRequest Bridge::fromMeaningObject(
    const elle::MeaningObject&       meaning,
    const elle::ConversationContext& convo)
{
    ProbabilityRequest req{};

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

    req.contextHints.reserve(meaning.contextFrames.size()
                             + convo.activeContextHints.size());
    for (const auto& cfm : meaning.contextFrames) {
        ProbabilityRequest::ContextHint h{};
        h.contextId = cfm.contextId.value();
        h.score     = cfm.score;
        req.contextHints.push_back(h);
    }

    for (const auto& cid : convo.activeContextHints) {
        ProbabilityRequest::ContextHint h{};
        h.contextId = cid.value();
        h.score     = 1.0;
        req.contextHints.push_back(h);
    }

    req.emotionalProfile = meaning.emotionalProfile.byEmotionId;

    req.speakerRelationship = convo.speakerRelationship;

    req.exclamationCount = meaning.sequence.exclamationCount;
    req.questionCount    = meaning.sequence.questionCount;
    req.ellipsisCount    = meaning.sequence.ellipsisCount;
    req.endsWithQuestion = meaning.sequence.endsWithQuestion;
    req.endsWithExclaim  = meaning.sequence.endsWithExclaim;

    req.stochastic = false;
    req.randomSeed = 0;

    return req;
}

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

} }
