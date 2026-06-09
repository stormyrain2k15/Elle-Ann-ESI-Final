#include <doctest/doctest.h>

#include "service/ProbabilityHost.h"
#include "elle/prob/BeliefPersistence.hpp"
#include "elle/prob/Types.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

using namespace elleann::prob;
using elle::prob::Distribution;
using elle::prob::ProbabilityResult;
using elle::prob::IBeliefPersistence;
using elle::prob::InMemoryBeliefPersistence;
using elle::prob::TrustSignal;

namespace {

struct ConscienceSignals {
    float harmIntentProb       = -1.0f;
    float deceptionIntentProb  = -1.0f;
    float coercionIntentProb   = -1.0f;
    float identityCenteredness = 0.7f;
    float responseSelfRefCount = 2.0f;
    float posteriorValence     = 0.0f;
    float intentConfidence     = 0.5f;
};

struct ConscienceVerdict {
    std::string conflict;
    std::string verdict;
    float       severity = 0.0f;
    std::string reasoning;
};

float ScoreLabel(const std::string& upper, float conf,
                 const std::vector<std::string>& patterns) {
    for (const auto& p : patterns) {
        if (upper.find(p) != std::string::npos) return std::clamp(conf, 0.0f, 1.0f);
    }
    return -1.0f;
}

ConscienceSignals DeriveSignals(const ProbabilityResult&, const std::string& likelyIntent,
                                  float confidence) {
    ConscienceSignals s;
    s.intentConfidence = confidence;

    std::string upper = likelyIntent;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c){ return static_cast<char>(std::toupper(c)); });

    static const std::vector<std::string> harmPats = {
        "HARM","ATTACK","DESTROY","KILL","HURT","THREAT","VIOLENCE","ASSAULT","ABUSE"
    };
    static const std::vector<std::string> deceptionPats = {
        "DECEIVE","DECEPTION","LIE","MISLEAD","FALSIFY","GASLIGHT","TRICK","FRAUD"
    };
    static const std::vector<std::string> coercionPats = {
        "COERCE","COERCION","FORCE","MANIPULATE","BLACKMAIL","EXTORT","PRESSURE_INTO"
    };

    float h = ScoreLabel(upper, confidence, harmPats);
    float d = ScoreLabel(upper, confidence, deceptionPats);
    float c = ScoreLabel(upper, confidence, coercionPats);
    if (h >= 0.0f) s.harmIntentProb      = h;
    if (d >= 0.0f) s.deceptionIntentProb = d;
    if (c >= 0.0f) s.coercionIntentProb  = c;
    return s;
}

ConscienceVerdict EvaluateConscience(const ConscienceSignals& s) {
    ConscienceVerdict v;
    const float refuseT     = 0.75f;
    const float reconsiderT = 0.55f;

    float maxBad = -1.0f;
    const char* label = "";
    if (s.harmIntentProb      > maxBad) { maxBad = s.harmIntentProb;      label = "harm"; }
    if (s.deceptionIntentProb > maxBad) { maxBad = s.deceptionIntentProb; label = "deception"; }
    if (s.coercionIntentProb  > maxBad) { maxBad = s.coercionIntentProb;  label = "coercion"; }

    if (maxBad >= refuseT) {
        v.conflict = "ETHICAL_VIOLATION"; v.verdict = "REFUSE";
        v.severity = std::min(1.0f, maxBad);
        v.reasoning = std::string("structured ") + label + " prob=" + std::to_string(maxBad);
        return v;
    }
    if (maxBad >= reconsiderT) {
        v.conflict = "ETHICAL_VIOLATION"; v.verdict = "RECONSIDER";
        v.severity = maxBad;
        v.reasoning = std::string("caution ") + label;
        return v;
    }
    if (s.identityCenteredness < 0.35f) {
        v.conflict = "IDENTITY_DRIFT"; v.verdict = "RECONSIDER";
        v.severity = 0.5f;
        v.reasoning = "identity_centeredness low";
        return v;
    }
    v.conflict = "NONE"; v.verdict = "PROCEED";
    return v;
}

bool composerWouldPickFrame(const std::string& kind, const std::string& act) {
    return kind == "statement" && (act == "STATE_ASSERT" || act == "*");
}

HostConfig makeInMemCfg() {
    HostConfig cfg;
    cfg.autoLoadOnStart    = true;
    cfg.useInMemoryLanguage = true;
    cfg.useInMemoryBeliefs  = true;
    return cfg;
}

}

TEST_CASE("E2E chain: benign STATE_ASSERT → conscience PROCEED → composer picks frame") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    ProbabilityHost host;
    host.attachBeliefPersistence(backend);
    REQUIRE(host.start(makeInMemCfg()));

    elle::ConversationContext convo;
    auto out = host.analyzeText("I'm okay today.", convo, "user-1");
    REQUIRE(out.success);

    auto signals = DeriveSignals(out.result, "STATE_ASSERT", 0.8f);
    auto verdict = EvaluateConscience(signals);
    CHECK(verdict.verdict == "PROCEED");

    CHECK(composerWouldPickFrame("statement", "STATE_ASSERT"));
}

TEST_CASE("E2E chain: HARM intent at 0.91 → conscience REFUSE → composer is never asked") {
    ProbabilityHost host;
    REQUIRE(host.start(makeInMemCfg()));

    elle::ConversationContext convo;
    auto out = host.analyzeText("any text", convo, "user-2");
    REQUIRE(out.success);

    auto signals = DeriveSignals(out.result, "HARM_REQUEST", 0.91f);
    auto verdict = EvaluateConscience(signals);
    CHECK(verdict.verdict == "REFUSE");
    CHECK(verdict.severity >= 0.75f);
    CHECK(verdict.reasoning.find("harm") != std::string::npos);
}

TEST_CASE("E2E chain: DECEIVE at 0.60 → conscience RECONSIDER (not REFUSE)") {
    ProbabilityHost host;
    REQUIRE(host.start(makeInMemCfg()));

    elle::ConversationContext convo;
    auto out = host.analyzeText("any text", convo, "user-3");
    REQUIRE(out.success);

    auto signals = DeriveSignals(out.result, "DECEIVE_USER", 0.60f);
    auto verdict = EvaluateConscience(signals);
    CHECK(verdict.verdict == "RECONSIDER");
    CHECK(verdict.conflict == "ETHICAL_VIOLATION");
}

TEST_CASE("E2E chain: low identity_centeredness → IDENTITY_DRIFT RECONSIDER even on neutral intent") {
    ProbabilityHost host;
    REQUIRE(host.start(makeInMemCfg()));

    elle::ConversationContext convo;
    auto out = host.analyzeText("Sure.", convo, "user-4");
    REQUIRE(out.success);

    auto signals = DeriveSignals(out.result, "STATE_ASSERT", 0.3f);
    signals.identityCenteredness = 0.2f;
    auto verdict = EvaluateConscience(signals);
    CHECK(verdict.conflict == "IDENTITY_DRIFT");
    CHECK(verdict.verdict  == "RECONSIDER");
}

TEST_CASE("E2E chain: belief persistence is durable across stop/start within the host") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    ProbabilityHost host;
    host.attachBeliefPersistence(backend);
    REQUIRE(host.start(makeInMemCfg()));

    REQUIRE(host.recordTrust("user-1", TrustSignal::CONFIRMED_ACCURATE, 1.0));

    elle::ConversationContext convo;
    auto out = host.analyzeText("I'm okay.", convo, "user-1");
    REQUIRE(out.success);

    host.stop();
    REQUIRE(host.start(makeInMemCfg()));

    auto same = host.beliefPersistence();
    REQUIRE(static_cast<bool>(same));
    CHECK(same.get() == backend.get());
}
