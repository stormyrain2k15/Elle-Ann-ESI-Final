#include "service/ProbabilityProto.h"

#include <exception>

namespace elleann { namespace prob {

nlohmann::json distributionToJson(const elle::prob::Distribution& d) {
    nlohmann::json o = nlohmann::json::object();
    for (const auto& [h, p] : d.mass) {
        o[std::to_string(h)] = p;
    }
    return o;
}

nlohmann::json weightsToJson(const elle::prob::WeightVector& w) {
    return nlohmann::json{
        {"contextFrameMatch",   w.contextFrameMatch},
        {"nearbyWordCooccur",   w.nearbyWordCooccur},
        {"senseExampleOverlap", w.senseExampleOverlap},
        {"emotionalAlignment",  w.emotionalAlignment},
        {"frequency",           w.frequency},
        {"posCompatibility",    w.posCompatibility},
        {"posNegDrawAlignment", w.posNegDrawAlignment},
        {"conversationHint",    w.conversationHint}
    };
}

elle::prob::WeightVector weightsFromJson(const nlohmann::json& j) {
    elle::prob::WeightVector w{};
    w.contextFrameMatch   = j.value("contextFrameMatch",   w.contextFrameMatch);
    w.nearbyWordCooccur   = j.value("nearbyWordCooccur",   w.nearbyWordCooccur);
    w.senseExampleOverlap = j.value("senseExampleOverlap", w.senseExampleOverlap);
    w.emotionalAlignment  = j.value("emotionalAlignment",  w.emotionalAlignment);
    w.frequency           = j.value("frequency",           w.frequency);
    w.posCompatibility    = j.value("posCompatibility",    w.posCompatibility);
    w.posNegDrawAlignment = j.value("posNegDrawAlignment", w.posNegDrawAlignment);
    w.conversationHint    = j.value("conversationHint",    w.conversationHint);
    return w;
}

const char* pragmaticActName(elle::prob::PragmaticAct a) {
    using PA = elle::prob::PragmaticAct;
    switch (a) {
        case PA::ASSERT:    return "ASSERT";
        case PA::QUESTION:  return "QUESTION";
        case PA::REQUEST:   return "REQUEST";
        case PA::OFFER:     return "OFFER";
        case PA::PROMISE:   return "PROMISE";
        case PA::WARN:      return "WARN";
        case PA::GREET:     return "GREET";
        case PA::APOLOGIZE: return "APOLOGIZE";
        case PA::THANK:     return "THANK";
        case PA::COMFORT:   return "COMFORT";
        case PA::DEFLECT:   return "DEFLECT";
        case PA::CHALLENGE: return "CHALLENGE";
        case PA::CONFIRM:   return "CONFIRM";
        case PA::DENY:      return "DENY";
        case PA::UNKNOWN:   return "UNKNOWN";
    }
    return "UNKNOWN";
}

nlohmann::json resultToJson(const elle::prob::ProbabilityResult& r) {
    nlohmann::json units = nlohmann::json::array();
    for (const auto& u : r.units) {
        nlohmann::json ranked = nlohmann::json::array();
        for (const auto& cand : u.rankedCandidates) {
            ranked.push_back({
                {"hypothesisId", cand.hypothesisId},
                {"probability",  cand.probability},
                {"entropy",      cand.entropy},
                {"label",        cand.label}
            });
        }
        units.push_back({
            {"unitIndex",         u.unitIndex},
            {"winningSenseId",    u.winningSenseId},
            {"isPhraseSense",     u.isPhraseSense},
            {"winnerProbability", u.winnerProbability},
            {"posteriorEntropy",  u.posteriorEntropy},
            {"rankedCandidates",  ranked}
        });
    }
    return nlohmann::json{
        {"units",              units},
        {"recommendedWeights", weightsToJson(r.recommendedWeights)},
        {"intentDistribution", distributionToJson(r.intentDistribution)},
        {"likelyAct",          pragmaticActName(r.likelyAct)},
        {"emotionalPosterior", distributionToJson(r.emotionalPosterior)},
        {"speakerTrust",       r.speakerTrust},
        {"overallConfidence",  r.overallConfidence}
    };
}

elle::ConversationContext convoFromJson(const nlohmann::json& j) {
    elle::ConversationContext c{};
    if (j.contains("speakerRelationship")) {
        c.speakerRelationship = j.value("speakerRelationship", std::string());
    }
    if (j.contains("activeContextHints") && j["activeContextHints"].is_array()) {
        for (const auto& h : j["activeContextHints"]) {
            if (h.is_number_integer()) {
                c.activeContextHints.push_back(
                    elle::ContextID{h.get<std::int64_t>()});
            }
        }
    }
    if (j.contains("recentWordIds") && j["recentWordIds"].is_array()) {
        for (const auto& w : j["recentWordIds"]) {
            if (w.is_number_integer()) {
                c.recentWordIds.push_back(
                    elle::WordID{w.get<std::int64_t>()});
            }
        }
    }
    c.prefersBaseball = j.value("prefersBaseball", false);
    return c;
}

elle::prob::ProbabilityRequest requestFromJson(const nlohmann::json& j) {
    elle::prob::ProbabilityRequest req{};

    if (j.contains("units") && j["units"].is_array()) {
        for (const auto& uj : j["units"]) {
            elle::prob::ProbabilityRequest::UnitSpec u{};
            u.unitIndex = uj.value("unitIndex", (std::size_t)0);
            u.wordId    = uj.value("wordId",   (std::int64_t)0);
            u.phraseId  = uj.value("phraseId", (std::int64_t)0);
            u.isPhrase  = uj.value("isPhrase",  false);
            u.isUnknown = uj.value("isUnknown", false);
            if (uj.contains("senseCandidateIds") && uj["senseCandidateIds"].is_array()) {
                for (const auto& s : uj["senseCandidateIds"]) {
                    if (s.is_number_integer()) {
                        u.senseCandidateIds.push_back(s.get<std::int64_t>());
                    }
                }
            }
            if (uj.contains("phraseSenseCandidateIds") &&
                uj["phraseSenseCandidateIds"].is_array()) {
                for (const auto& s : uj["phraseSenseCandidateIds"]) {
                    if (s.is_number_integer()) {
                        u.phraseSenseCandidateIds.push_back(s.get<std::int64_t>());
                    }
                }
            }
            req.units.push_back(std::move(u));
        }
    }

    if (j.contains("contextHints") && j["contextHints"].is_array()) {
        for (const auto& ch : j["contextHints"]) {
            elle::prob::ProbabilityRequest::ContextHint h{};
            h.contextId = ch.value("contextId", (std::int64_t)0);
            h.score     = ch.value("score",     0.0);
            req.contextHints.push_back(h);
        }
    }

    if (j.contains("emotionalProfile") && j["emotionalProfile"].is_object()) {
        for (auto it = j["emotionalProfile"].begin();
             it != j["emotionalProfile"].end(); ++it) {
            try {
                std::int64_t k = std::stoll(it.key());
                req.emotionalProfile[k] = it.value().get<double>();
            } catch (const std::exception&) {
            }
        }
    }

    req.speakerRelationship  = j.value("speakerRelationship",  std::string());
    req.speakerTrustOverride = j.value("speakerTrustOverride", -1.0);
    req.exclamationCount     = j.value("exclamationCount",     0);
    req.questionCount        = j.value("questionCount",        0);
    req.ellipsisCount        = j.value("ellipsisCount",        0);
    req.endsWithQuestion     = j.value("endsWithQuestion",     false);
    req.endsWithExclaim      = j.value("endsWithExclaim",      false);
    req.stochastic           = j.value("stochastic",           false);
    req.randomSeed           = j.value("randomSeed",           (std::uint64_t)0);
    return req;
}

elle::prob::TrustSignal trustSignalFromString(const std::string& s) {
    using TS = elle::prob::TrustSignal;
    if (s == "CONFIRMED_ACCURATE")        return TS::CONFIRMED_ACCURATE;
    if (s == "KEPT_PROMISE")              return TS::KEPT_PROMISE;
    if (s == "CONSISTENT_WITH_HISTORY")   return TS::CONSISTENT_WITH_HISTORY;
    if (s == "CORRECTION_NEEDED")         return TS::CORRECTION_NEEDED;
    if (s == "CONTRADICTED")              return TS::CONTRADICTED;
    if (s == "HOSTILE_FRAMING")           return TS::HOSTILE_FRAMING;
    if (s == "IDENTITY_CONFIRMED")        return TS::IDENTITY_CONFIRMED;
    return TS::CONFIRMED_ACCURATE;
}

std::unordered_map<std::int64_t, double> hormonalStateFromJson(const nlohmann::json& j) {
    std::unordered_map<std::int64_t, double> state;
    if (!j.is_object()) return state;
    for (auto it = j.begin(); it != j.end(); ++it) {
        try {
            std::int64_t k = std::stoll(it.key());
            state[k] = it.value().get<double>();
        } catch (const std::exception&) {
        }
    }
    return state;
}

} }
