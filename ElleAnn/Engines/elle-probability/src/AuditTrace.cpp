// ============================================================================
// Elle Probability Engine -- Audit Trace implementation
// File: src/AuditTrace.cpp
// ============================================================================
#include "elle/prob/AuditTrace.hpp"
#include "elle/prob/EmotionalPosteriorBuilder.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <iomanip>
#include <sstream>

namespace elle { namespace prob {

// ---------------------------------------------------------------------------
// Helper: PragmaticAct name
// ---------------------------------------------------------------------------

static const char* actName(PragmaticAct act) {
    switch (act) {
        case PragmaticAct::ASSERT:    return "ASSERT";
        case PragmaticAct::QUESTION:  return "QUESTION";
        case PragmaticAct::REQUEST:   return "REQUEST";
        case PragmaticAct::OFFER:     return "OFFER";
        case PragmaticAct::PROMISE:   return "PROMISE";
        case PragmaticAct::WARN:      return "WARN";
        case PragmaticAct::GREET:     return "GREET";
        case PragmaticAct::APOLOGIZE: return "APOLOGIZE";
        case PragmaticAct::THANK:     return "THANK";
        case PragmaticAct::COMFORT:   return "COMFORT";
        case PragmaticAct::DEFLECT:   return "DEFLECT";
        case PragmaticAct::CHALLENGE: return "CHALLENGE";
        case PragmaticAct::CONFIRM:   return "CONFIRM";
        case PragmaticAct::DENY:      return "DENY";
        case PragmaticAct::UNKNOWN:   return "UNKNOWN";
        default:                      return "UNKNOWN";
    }
}

static const char* evidenceKindName(EvidenceKind k) {
    switch (k) {
        case EvidenceKind::LEXICAL_MATCH:          return "LEXICAL_MATCH";
        case EvidenceKind::CONTEXT_FRAME:          return "CONTEXT_FRAME";
        case EvidenceKind::EMOTIONAL_SIGNAL:       return "EMOTIONAL_SIGNAL";
        case EvidenceKind::RELATIONAL_LINK:        return "RELATIONAL_LINK";
        case EvidenceKind::CONVERSATION_TURN:      return "CONVERSATION_TURN";
        case EvidenceKind::EXTERNAL_CLAIM:         return "EXTERNAL_CLAIM";
        case EvidenceKind::PRIOR_DECAY:            return "PRIOR_DECAY";
        case EvidenceKind::SELF_MODEL:             return "SELF_MODEL";
        case EvidenceKind::CONCEPTUAL_PATH:        return "CONCEPTUAL_PATH";
        case EvidenceKind::SPEAKER_SIGNAL:         return "SPEAKER_SIGNAL";
        case EvidenceKind::CORRECTION:             return "CORRECTION";
        default:                                   return "UNKNOWN";
    }
}

// ---------------------------------------------------------------------------
// toJson
// ---------------------------------------------------------------------------

std::string AuditTrace::toJson(const ProbabilityResult& result, int indent) {
    nlohmann::json j;

    // Decisions (per-unit sense resolutions).
    nlohmann::json decisions = nlohmann::json::array();
    for (const auto& u : result.units) {
        nlohmann::json unit;
        unit["unitIndex"]          = u.unitIndex;
        unit["winningSenseId"]     = u.winningSenseId;
        unit["isPhraseSense"]      = u.isPhraseSense;
        unit["winnerProbability"]  = u.winnerProbability;
        unit["posteriorEntropy"]   = u.posteriorEntropy;

        nlohmann::json candidates = nlohmann::json::array();
        for (const auto& sh : u.rankedCandidates) {
            nlohmann::json c;
            c["hypothesisId"]  = sh.hypothesisId;
            c["probability"]   = sh.probability;
            c["label"]         = sh.label;
            nlohmann::json factors;
            for (const auto& [k, v] : sh.contributingFactors) factors[k] = v;
            c["factors"] = factors;
            candidates.push_back(c);
        }
        unit["candidates"] = candidates;
        decisions.push_back(unit);
    }
    j["decisions"] = decisions;

    // Weights.
    {
        nlohmann::json w;
        w["contextFrameMatch"]   = result.recommendedWeights.contextFrameMatch;
        w["nearbyWordCooccur"]   = result.recommendedWeights.nearbyWordCooccur;
        w["senseExampleOverlap"] = result.recommendedWeights.senseExampleOverlap;
        w["emotionalAlignment"]  = result.recommendedWeights.emotionalAlignment;
        w["frequency"]           = result.recommendedWeights.frequency;
        w["posCompatibility"]    = result.recommendedWeights.posCompatibility;
        w["posNegDrawAlignment"] = result.recommendedWeights.posNegDrawAlignment;
        w["conversationHint"]    = result.recommendedWeights.conversationHint;
        j["weights"] = w;
    }

    // Intent.
    {
        nlohmann::json intent;
        intent["likelyAct"] = actName(result.likelyAct);
        nlohmann::json dist;
        for (const auto& [k, v] : result.intentDistribution.mass) {
            dist[std::to_string(k)] = v;
        }
        intent["distribution"] = dist;
        j["intent"] = intent;
    }

    // Emotional posterior.
    {
        nlohmann::json emo;
        nlohmann::json dist;
        for (const auto& [k, v] : result.emotionalPosterior.mass) {
            dist[std::to_string(k)] = v;
        }
        emo["distribution"]  = dist;
        emo["entropy"]       = result.emotionalPosterior.entropy();
        j["emotions"] = emo;
    }

    // Trust.
    j["speakerTrust"]      = result.speakerTrust;
    j["overallConfidence"] = result.overallConfidence;

    return indent > 0 ? j.dump(indent) : j.dump();
}

// ---------------------------------------------------------------------------
// beliefsToJson
// ---------------------------------------------------------------------------

std::string AuditTrace::beliefsToJson(const BeliefStore&              store,
                                       const std::vector<std::string>& domains,
                                       int                             indent)
{
    nlohmann::json j = nlohmann::json::object();
    for (const auto& d : domains) {
        auto b = store.getBelief(d);
        if (!b) continue;
        nlohmann::json bj;
        bj["domain"]       = b->domain;
        bj["uncertainty"]  = b->uncertainty();
        bj["halfLifeSecs"] = b->halfLifeSecs;
        nlohmann::json posterior;
        for (const auto& [k, v] : b->posterior.mass) {
            posterior[std::to_string(k)] = v;
        }
        bj["posterior"] = posterior;
        bj["evidenceCount"] = static_cast<int>(b->evidenceLog.size());
        j[d] = bj;
    }
    return indent > 0 ? j.dump(indent) : j.dump();
}

// ---------------------------------------------------------------------------
// weightsToJson
// ---------------------------------------------------------------------------

std::string AuditTrace::weightsToJson(const WeightVector& w, int indent) {
    nlohmann::json j;
    j["contextFrameMatch"]   = w.contextFrameMatch;
    j["nearbyWordCooccur"]   = w.nearbyWordCooccur;
    j["senseExampleOverlap"] = w.senseExampleOverlap;
    j["emotionalAlignment"]  = w.emotionalAlignment;
    j["frequency"]           = w.frequency;
    j["posCompatibility"]    = w.posCompatibility;
    j["posNegDrawAlignment"] = w.posNegDrawAlignment;
    j["conversationHint"]    = w.conversationHint;
    return indent > 0 ? j.dump(indent) : j.dump();
}

// ---------------------------------------------------------------------------
// emotionToJson
// ---------------------------------------------------------------------------

std::string AuditTrace::emotionToJson(const EmotionalPosteriorBuilder& builder,
                                       int indent) {
    const auto vad = builder.currentVAD();
    nlohmann::json j;
    j["valence"]   = vad.valence;
    j["arousal"]   = vad.arousal;
    j["dominance"] = vad.dominance;
    const auto post = builder.currentPosterior();
    nlohmann::json dist;
    for (const auto& [k, v] : post.mass) dist[std::to_string(k)] = v;
    j["distribution"] = dist;
    j["entropy"] = post.entropy();
    return indent > 0 ? j.dump(indent) : j.dump();
}

// ---------------------------------------------------------------------------
// distributionToJson
// ---------------------------------------------------------------------------

std::string AuditTrace::distributionToJson(const Distribution& d, int indent) {
    nlohmann::json j;
    for (const auto& [k, v] : d.mass) j[std::to_string(k)] = v;
    nlohmann::json wrap;
    wrap["mass"]    = j;
    wrap["entropy"] = d.entropy();
    wrap["map"]     = d.map();
    wrap["support"] = static_cast<int>(d.support());
    return indent > 0 ? wrap.dump(indent) : wrap.dump();
}

// ---------------------------------------------------------------------------
// evidenceSummary
// ---------------------------------------------------------------------------

std::string AuditTrace::evidenceSummary(const Belief& belief) {
    std::ostringstream oss;
    oss << "Belief[" << belief.domain << "] "
        << "uncertainty=" << std::fixed << std::setprecision(4) << belief.uncertainty()
        << " evidence_count=" << belief.evidenceLog.size() << "\n";
    for (const auto& ev : belief.evidenceLog) {
        oss << "  [" << evidenceKindName(ev.kind) << "]"
            << " h=" << ev.hypothesisId
            << " lr=" << std::setprecision(3) << ev.likelihoodRatio
            << " sw=" << ev.sourceWeight
            << " : " << ev.reason << "\n";
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// Simple JSON helpers (unused externally but kept for completeness)
// ---------------------------------------------------------------------------

std::string AuditTrace::jsonString(const std::string& s) {
    nlohmann::json j = s;
    return j.dump();
}

std::string AuditTrace::jsonDouble(double v) {
    nlohmann::json j = v;
    return j.dump();
}

std::string AuditTrace::jsonInt(std::int64_t v) {
    nlohmann::json j = v;
    return j.dump();
}

} } // namespace elle::prob
