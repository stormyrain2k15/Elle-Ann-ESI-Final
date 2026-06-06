// ============================================================================
// Elle Engine -- MeaningObjectBuilder implementation
// File: src/MeaningObjectBuilder.cpp
// ============================================================================
#include "elle/MeaningObjectBuilder.hpp"

#include <nlohmann/json.hpp>

#include <sstream>

namespace elle {

namespace {

double aggregateConfidence(const std::vector<ResolvedSense>& resolved) {
    if (resolved.empty()) return 0.0;
    double sum = 0.0;
    int    counted = 0;
    for (const auto& r : resolved) {
        if (!r.rankedCandidates.empty()) {
            // confidence = top score / (top + second) (margin)
            const double top = r.rankedCandidates.front().score;
            const double second = r.rankedCandidates.size() > 1
                                ? r.rankedCandidates[1].score
                                : 0.0;
            const double denom = std::max(top + std::abs(second), 1e-6);
            sum += std::clamp(top / denom, 0.0, 1.0);
            ++counted;
        }
    }
    return counted == 0 ? 0.0 : sum / counted;
}

std::string inferIntent(const std::vector<ContextFrameMatch>& matches,
                        const IntegerSequence& seq) {
    if (!matches.empty()) {
        const auto& top = matches.front();
        if (!top.code.empty()) return top.code;
    }
    if (seq.endsWithQuestion) return "QUESTION";
    if (seq.endsWithExclaim)  return "EXCLAMATION";
    return "STATEMENT";
}

std::string buildExplanationTrace(const std::vector<ResolvedSense>& resolved,
                                  const std::vector<ContextFrameMatch>& matches) {
    std::ostringstream oss;
    oss << "Context frames (ranked):\n";
    for (const auto& m : matches) {
        oss << "  - " << m.code << "  score=" << m.score << "\n";
    }
    oss << "Per-unit decisions:\n";
    for (const auto& r : resolved) {
        oss << "  unit[" << r.unitIndex << "]";
        if (r.chosenSenseId)       oss << "  -> SenseID="       << r.chosenSenseId->value();
        if (r.chosenPhraseSenseId) oss << "  -> PhraseSenseID=" << r.chosenPhraseSenseId->value();
        oss << "  conf=" << r.confidence << "\n";
        for (const auto& c : r.rankedCandidates) {
            oss << "      candidate score=" << c.score << "  " << c.reason << "\n";
            for (const auto& [k, v] : c.scoreBreakdown) {
                oss << "        " << k << " = " << v << "\n";
            }
        }
    }
    return oss.str();
}

} // namespace

MeaningObject MeaningObjectBuilder::build(std::string rawInput,
                                          std::string normalizedInput,
                                          IntegerSequence sequence,
                                          std::vector<ResolvedSense> resolved,
                                          std::vector<ContextFrameMatch> contextMatches,
                                          EmotionalProfile emotional,
                                          std::vector<ConceptPath> paths,
                                          const DebugTrace& /*trace*/) const {
    MeaningObject m;
    m.rawInput        = std::move(rawInput);
    m.normalizedInput = std::move(normalizedInput);
    m.sequence        = std::move(sequence);
    m.resolvedSenses  = std::move(resolved);
    m.contextFrames   = std::move(contextMatches);
    m.emotionalProfile= std::move(emotional);
    m.conceptPaths    = std::move(paths);
    m.likelyIntent    = inferIntent(m.contextFrames, m.sequence);
    m.overallConfidence = aggregateConfidence(m.resolvedSenses);
    m.explanationTrace  = buildExplanationTrace(m.resolvedSenses, m.contextFrames);

    for (const auto& u : m.sequence.units) {
        if (u.isUnknown) m.unresolvedWords.push_back(u.originalSpan);
    }
    return m;
}

std::string meaningObjectToJson(const MeaningObject& m, int indent) {
    nlohmann::json j;
    j["raw_input"]        = m.rawInput;
    j["normalized_input"] = m.normalizedInput;
    j["overall_confidence"] = m.overallConfidence;
    j["likely_intent"]    = m.likelyIntent;

    nlohmann::json seq = nlohmann::json::array();
    for (const auto& u : m.sequence.units) {
        nlohmann::json uj;
        uj["position"]   = u.positionInSentence;
        uj["normalized"] = u.normalized;
        if (u.wordId)       uj["word_id"]        = u.wordId->value();
        if (u.wordFormId)   uj["word_form_id"]   = u.wordFormId->value();
        if (u.phraseId)     uj["phrase_id"]      = u.phraseId->value();
        uj["is_unknown"] = u.isUnknown;
        seq.push_back(std::move(uj));
    }
    j["integer_sequence"] = std::move(seq);

    nlohmann::json res = nlohmann::json::array();
    for (const auto& r : m.resolvedSenses) {
        nlohmann::json rj;
        rj["unit_index"] = r.unitIndex;
        if (r.chosenSenseId)       rj["chosen_sense_id"]        = r.chosenSenseId->value();
        if (r.chosenPhraseSenseId) rj["chosen_phrase_sense_id"] = r.chosenPhraseSenseId->value();
        rj["confidence"] = r.confidence;
        nlohmann::json ranked = nlohmann::json::array();
        for (const auto& c : r.rankedCandidates) {
            nlohmann::json cj = {{"score", c.score}, {"reason", c.reason}};
            if (c.senseId)       cj["sense_id"]       = c.senseId->value();
            if (c.phraseSenseId) cj["phrase_sense_id"] = c.phraseSenseId->value();
            nlohmann::json bd;
            for (const auto& [k, v] : c.scoreBreakdown) bd[k] = v;
            cj["breakdown"] = std::move(bd);
            ranked.push_back(std::move(cj));
        }
        rj["ranked_candidates"] = std::move(ranked);
        res.push_back(std::move(rj));
    }
    j["resolved_senses"] = std::move(res);

    nlohmann::json frames = nlohmann::json::array();
    for (const auto& f : m.contextFrames) {
        frames.push_back({
            {"context_id", f.contextId.value()},
            {"code",       f.code},
            {"name",       f.name},
            {"score",      f.score}
        });
    }
    j["context_frames"] = std::move(frames);

    nlohmann::json emo;
    for (const auto& [eid, w] : m.emotionalProfile.byEmotionId) emo[std::to_string(eid)] = w;
    j["emotional_profile"] = std::move(emo);

    nlohmann::json paths = nlohmann::json::array();
    for (const auto& p : m.conceptPaths) {
        nlohmann::json pj;
        pj["total_strength"] = p.totalStrength;
        nlohmann::json nodes = nlohmann::json::array();
        for (auto n : p.nodes) nodes.push_back(n.value());
        pj["nodes"] = std::move(nodes);
        nlohmann::json edges = nlohmann::json::array();
        for (const auto& e : p.edges) {
            edges.push_back({
                {"from",       e.fromNode.value()},
                {"to",         e.toNode.value()},
                {"relation",   e.relationTypeId.value()},
                {"strength",   e.strength},
                {"confidence", e.confidence}
            });
        }
        pj["edges"] = std::move(edges);
        paths.push_back(std::move(pj));
    }
    j["concept_paths"] = std::move(paths);

    j["unresolved_words"] = m.unresolvedWords;
    j["explanation_trace"] = m.explanationTrace;
    return j.dump(indent);
}

} // namespace elle
