#include "elle/prob/Bridge.hpp"
#include "elle/prob/ProbabilityEngine.hpp"
#include "elle/prob/Types.hpp"

#include "elle/Config.hpp"
#include "elle/Types.hpp"

#include <cstdio>
#include <iostream>
#include <string>

namespace {

elle::MeaningObject makeImFineMeaning() {
    using namespace elle;
    MeaningObject m{};
    m.rawInput        = "I'm fine.";
    m.normalizedInput = "i'm fine.";

    {
        WordUnit u{};
        u.originalSpan = "I'm";
        u.normalized   = "i'm";
        u.wordId       = WordID{101};
        u.positionInSentence = 0;
        u.senseCandidates.push_back(SenseID{1001});
        m.sequence.units.push_back(std::move(u));
    }
    {
        WordUnit u{};
        u.originalSpan = "fine";
        u.normalized   = "fine";
        u.wordId       = WordID{202};
        u.positionInSentence = 1;
        u.senseCandidates.push_back(SenseID{2001});
        u.senseCandidates.push_back(SenseID{2002});
        u.senseCandidates.push_back(SenseID{2003});
        m.sequence.units.push_back(std::move(u));
    }

    m.sequence.endsWithQuestion = false;
    m.sequence.endsWithExclaim  = false;
    m.sequence.questionCount    = 0;
    m.sequence.exclamationCount = 0;

    ContextFrameMatch cfm{};
    cfm.contextId = ContextID{77};
    cfm.code      = "EMOTIONAL_REGULATION";
    cfm.name      = "Emotional state self-report";
    cfm.score     = 1.2;
    m.contextFrames.push_back(cfm);

    m.emotionalProfile.byEmotionId[2] = 0.3;
    m.emotionalProfile.byEmotionId[5] = 0.5;
    m.emotionalProfile.valence        = -0.2;
    m.emotionalProfile.positiveDraw   = 0.1;
    m.emotionalProfile.negativeDraw   = 0.4;

    m.likelyIntent      = "STATE_ASSERT";
    m.overallConfidence = 0.62;
    return m;
}

elle::ConversationContext makeConvo() {
    elle::ConversationContext c{};
    c.speakerRelationship = "intimate";
    c.activeContextHints.push_back(elle::ContextID{77});
    return c;
}

void dumpResult(const elle::prob::ProbabilityResult& r) {
    std::cout << "  Overall confidence:  " << r.overallConfidence  << "\n";
    std::cout << "  Speaker trust:       " << r.speakerTrust       << "\n";
    std::cout << "  Likely act:          "
              << static_cast<int>(r.likelyAct) << "\n";
    std::cout << "  Per-unit results:    " << r.units.size() << " unit(s)\n";
    for (const auto& u : r.units) {
        std::cout << "    unit[" << u.unitIndex << "]: winner="
                  << u.winningSenseId
                  << "  p=" << u.winnerProbability
                  << "  H=" << u.posteriorEntropy
                  << "  candidates=" << u.rankedCandidates.size() << "\n";
    }
    std::cout << "  Intent dist entropy: "
              << r.intentDistribution.entropy() << "\n";
    std::cout << "  Emotion posterior:   "
              << r.emotionalPosterior.support() << " non-zero emotions\n";
    std::cout << "  Recommended weights: contextFrame="
              << r.recommendedWeights.contextFrameMatch
              << "  emoAlign="
              << r.recommendedWeights.emotionalAlignment << "\n";
}

}

int main() {
    using namespace elle::prob;

    std::cout << "================================================\n";
    std::cout << "  Elle Probability Bridge -- End-to-end Smoke    \n";
    std::cout << "================================================\n\n";

    elle::ScoringWeights langWeights{};
    langWeights.contextFrameMatch   = 1.3;
    langWeights.emotionalAlignment  = 0.9;
    langWeights.conversationHint    = 1.1;

    const WeightVector wv = Bridge::fromScoringWeights(langWeights);
    std::cout << "[A] Lang ScoringWeights -> WeightVector:\n";
    std::cout << "    contextFrameMatch:   " << wv.contextFrameMatch  << "\n";
    std::cout << "    emotionalAlignment:  " << wv.emotionalAlignment << "\n";
    std::cout << "    conversationHint:    " << wv.conversationHint   << "\n";

    const elle::ScoringWeights roundtrip = Bridge::toScoringWeights(wv);
    if (roundtrip.contextFrameMatch != langWeights.contextFrameMatch ||
        roundtrip.conversationHint  != langWeights.conversationHint) {
        std::cerr << "[A] FAIL: round-trip mismatch\n";
        return 2;
    }
    std::cout << "    round-trip:          OK\n\n";

    const elle::MeaningObject       meaning = makeImFineMeaning();
    const elle::ConversationContext convo   = makeConvo();
    const ProbabilityRequest req = Bridge::fromMeaningObject(meaning, convo);

    std::cout << "[B] MeaningObject (\"I'm fine.\", intimate) -> ProbabilityRequest:\n";
    std::cout << "    units:                " << req.units.size() << "\n";
    std::cout << "    contextHints:         " << req.contextHints.size() << "\n";
    std::cout << "    emotionalProfile:     " << req.emotionalProfile.size()
              << " emotions\n";
    std::cout << "    speakerRelationship:  " << req.speakerRelationship << "\n\n";

    Bridge bridge;
    bridge.engine().seedWeights(wv);

    const ProbabilityResult r1 = bridge.analyze(req, "user_demo");

    std::cout << "[C] Pass 1 result (cold engine, intimate speaker):\n";
    dumpResult(r1);
    std::cout << "\n";

    bridge.feedback(1,
                    2001,
                    false,
                    0.9,
                    "user_demo");

    bridge.recordTrust("user_demo", TrustSignal::CONFIRMED_ACCURATE, 1.0);

    bridge.injectHormonalState({{2, 0.6}, {5, 0.4}});

    const ProbabilityResult r2 = bridge.analyze(req, "user_demo");
    std::cout << "[D] Pass 2 result (after feedback + trust + hormonal):\n";
    dumpResult(r2);
    std::cout << "\n";

    std::cout << "================================================\n";
    std::cout << "  Smoke OK: A + B + C + D all green             \n";
    std::cout << "================================================\n";
    return 0;
}
