#include "elle/EmotionalWeightProcessor.hpp"

#include <nlohmann/json.hpp>

namespace elle {

EmotionalWeightProcessor::EmotionalWeightProcessor(ISqlAccessLayer& db)
    : m_db(db) {}

EmotionalProfile EmotionalWeightProcessor::aggregate(const std::vector<ResolvedSense>& resolved,
                                                     const IntegerSequence& seq,
                                                     DebugTrace& trace) {
    EmotionalProfile p;

    auto addWeight = [&](EmotionID e, double w) {
        p.byEmotionId[e.value()] += w;
    };

    for (const auto& r : resolved) {
        if (r.chosenSenseId) {
            for (const auto& ew : m_db.getSenseEmotions(*r.chosenSenseId)) {
                addWeight(ew.emotionId, ew.weight);
            }
        }
        if (r.chosenPhraseSenseId) {
            for (const auto& ew : m_db.getPhraseSenseEmotions(*r.chosenPhraseSenseId)) {
                addWeight(ew.emotionId, ew.weight);
            }
        }
    }

    if (seq.endsWithExclaim) {
        addWeight(emo::ANGER,   0.10 * static_cast<double>(seq.exclamationCount));
        addWeight(emo::VALENCE,-0.10 * static_cast<double>(seq.exclamationCount));
    }
    if (seq.ellipsisCount > 0) {
        addWeight(emo::SADNESS, 0.15 * static_cast<double>(seq.ellipsisCount));
        addWeight(emo::VALENCE,-0.10);
    }

    p.valence      = p.get(emo::VALENCE);
    p.positiveDraw = p.get(emo::POS_DRAW);
    p.negativeDraw = p.get(emo::NEG_DRAW);

    nlohmann::json payload;
    for (const auto& [eid, w] : p.byEmotionId) {
        payload[std::to_string(eid)] = w;
    }
    trace.logJson("EmotionalWeightProcessor", "profile_aggregated",
                  "Per-sentence emotion profile.", std::move(payload));
    return p;
}

}
