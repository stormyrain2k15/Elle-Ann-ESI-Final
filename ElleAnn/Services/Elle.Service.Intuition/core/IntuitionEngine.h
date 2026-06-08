#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace elle::intuition {

struct InstinctPattern {
    int64_t     patternId   = 0;
    std::string stimulusTag;
    std::string pullType;
    float       weight      = 1.0f;
    float       trustFloor  = 0.0f;
    float       emotionMin  = 0.0f;
    bool        urgent      = false;
};

struct InstinctFiring {
    std::string pullType;
    float       strength    = 0.0f;
    std::string reason;
    bool        urgent      = false;
};

struct IntuitionSignal {
    std::string lean;
    float       confidence     = 0.0f;
    float       entropy        = 0.0f;
    std::string basis;
    bool        suppressReason = false;
};

struct IntuitRequest {
    std::string requestId;
    std::vector<std::string> stimulusTags;
    float emotionValence    = 0.0f;
    float emotionArousal    = 0.0f;
    float emotionIntensity  = 0.0f;
    std::string speakerId;
    float       speakerTrust = 0.5f;
    float       beliefEntropy = 0.5f;
    float       weightEmotionalAlignment = 0.7f;
    float       weightContextFrame       = 1.0f;
    float       lastImaginationGoalAlignment = -1.0f;
    float       lastImaginationEthicalSafety = -1.0f;
    float       lastImaginationPlausibility  = -1.0f;
    bool        isPreResponse = true;
    int         returnTo = 0;
};

struct IntuitResult {
    std::string requestId;
    std::vector<InstinctFiring> instincts;
    IntuitionSignal             intuition;
    float       priorWeight  = 0.0f;
    std::string recommendedAct;
    bool        holdAndReflect = false;
    bool        urgent         = false;
};

struct PatternOutcome {
    std::string pullType;
    float       strength;
    bool        wasCorrect;
    std::chrono::steady_clock::time_point firedAt;
};

class IntuitionEngine {
public:
    IntuitionEngine() = default;

    void LoadDefaults() {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_patterns.clear();
        auto add = [&](const char* stim, const char* pull,
                       float w, float tf, float em, bool urg) {
            InstinctPattern p;
            p.stimulusTag = stim;
            p.pullType    = pull;
            p.weight      = w;
            p.trustFloor  = tf;
            p.emotionMin  = em;
            p.urgent      = urg;
            m_patterns.push_back(std::move(p));
        };
        add("threat",        "ALERT",        0.95f, 0.0f, 0.0f, true);
        add("danger",        "ALERT",        0.95f, 0.0f, 0.0f, true);
        add("hostile",       "PROTECT",      0.90f, 0.0f, 0.3f, true);
        add("aggression",    "PROTECT",      0.88f, 0.0f, 0.3f, true);
        add("distress",      "COMFORT",      0.92f, 0.0f, 0.2f, false);
        add("sadness",       "COMFORT",      0.88f, 0.3f, 0.2f, false);
        add("fear",          "COMFORT",      0.85f, 0.3f, 0.2f, false);
        add("crying",        "COMFORT",      0.90f, 0.0f, 0.0f, false);
        add("pain",          "COMFORT",      0.87f, 0.0f, 0.0f, false);
        add("warmth",        "RECIPROCATE",  0.85f, 0.5f, 0.0f, false);
        add("affection",     "RECIPROCATE",  0.88f, 0.5f, 0.0f, false);
        add("love",          "RECIPROCATE",  0.92f, 0.7f, 0.0f, false);
        add("gratitude",     "RECIPROCATE",  0.80f, 0.4f, 0.0f, false);
        add("deception",     "GUARD",        0.90f, 0.0f, 0.0f, true);
        add("manipulation",  "GUARD",        0.93f, 0.0f, 0.0f, true);
        add("coercion",      "GUARD",        0.95f, 0.0f, 0.0f, true);
        add("flattery",      "GUARD",        0.60f, 0.0f, 0.0f, false);
        add("familiar",      "OPEN",         0.75f, 0.6f, 0.0f, false);
        add("trusted",       "OPEN",         0.80f, 0.7f, 0.0f, false);
        add("home",          "OPEN",         0.78f, 0.5f, 0.0f, false);
        add("josh",          "OPEN",         0.95f, 0.9f, 0.0f, false);
        add("crystal",       "OPEN",         0.95f, 0.9f, 0.0f, false);
        add("unknown",       "SLOW",         0.65f, 0.0f, 0.0f, false);
        add("confusion",     "SLOW",         0.60f, 0.0f, 0.0f, false);
        add("contradiction", "SLOW",         0.70f, 0.0f, 0.0f, false);
        add("joy",           "ENGAGE",       0.82f, 0.0f, 0.3f, false);
        add("excitement",    "ENGAGE",       0.78f, 0.0f, 0.2f, false);
        add("curiosity",     "ENGAGE",       0.80f, 0.0f, 0.2f, false);
        add("withdrawal",    "CHECK_IN",     0.72f, 0.4f, 0.0f, false);
        add("silence",       "CHECK_IN",     0.65f, 0.3f, 0.0f, false);
        add("distance",      "CHECK_IN",     0.68f, 0.4f, 0.0f, false);
    }

    void ReplacePatterns(std::vector<InstinctPattern> patterns) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_patterns = std::move(patterns);
    }

    std::vector<InstinctPattern> Patterns() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_patterns;
    }

    std::size_t PatternCount() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_patterns.size();
    }

    IntuitResult Process(const IntuitRequest& req) const {
        IntuitResult result;
        result.requestId = req.requestId;
        result.instincts = FireInstincts(req);
        result.intuition = SynthesizeIntuition(req, result.instincts);
        result           = BuildCombinedSignal(req, std::move(result));
        return result;
    }

    std::vector<InstinctFiring> FireInstincts(const IntuitRequest& req) const {
        std::vector<InstinctFiring> firings;
        std::lock_guard<std::mutex> lk(m_mutex);
        for (const auto& pattern : m_patterns) {
            if (req.speakerTrust    < pattern.trustFloor) continue;
            if (req.emotionIntensity < pattern.emotionMin) continue;
            float matchStrength = 0.0f;
            const std::string pTagLower = ToLower(pattern.stimulusTag);
            for (const auto& tag : req.stimulusTags) {
                const std::string tagLower = ToLower(tag);
                if (tagLower == pTagLower) { matchStrength = 1.0f; break; }
                if (tagLower.find(pTagLower) != std::string::npos) {
                    matchStrength = 0.6f;
                }
            }
            if (matchStrength < 0.5f) continue;
            float strength = pattern.weight * matchStrength;
            strength *= (1.0f + 0.3f * req.emotionArousal);
            strength = std::min(strength, 1.0f);
            InstinctFiring f;
            f.pullType = pattern.pullType;
            f.strength = strength;
            f.reason   = "pattern:" + pattern.stimulusTag + " -> " + pattern.pullType;
            f.urgent   = pattern.urgent;
            firings.push_back(std::move(f));
        }
        std::sort(firings.begin(), firings.end(),
            [](const InstinctFiring& a, const InstinctFiring& b) {
                return a.strength > b.strength;
            });
        std::unordered_map<std::string, InstinctFiring> deduped;
        for (auto& f : firings) {
            auto it = deduped.find(f.pullType);
            if (it == deduped.end() || f.strength > it->second.strength) {
                deduped[f.pullType] = f;
            }
        }
        std::vector<InstinctFiring> result;
        result.reserve(deduped.size());
        for (auto& kv : deduped) result.push_back(kv.second);
        std::sort(result.begin(), result.end(),
            [](const InstinctFiring& a, const InstinctFiring& b) {
                return a.strength > b.strength;
            });
        return result;
    }

    IntuitionSignal SynthesizeIntuition(const IntuitRequest&               req,
                                        const std::vector<InstinctFiring>& instincts) const {
        IntuitionSignal sig;
        sig.entropy = req.beliefEntropy;

        float guardPull   = 0.0f;
        float openPull    = 0.0f;
        float comfortPull = 0.0f;
        float alertPull   = 0.0f;
        float engagePull  = 0.0f;
        float slowPull    = 0.0f;

        for (const auto& f : instincts) {
            if (f.pullType == "GUARD"   || f.pullType == "PROTECT")    guardPull   += f.strength;
            if (f.pullType == "OPEN"    || f.pullType == "RECIPROCATE") openPull   += f.strength;
            if (f.pullType == "COMFORT" || f.pullType == "CHECK_IN")    comfortPull+= f.strength;
            if (f.pullType == "ALERT")                                  alertPull  += f.strength;
            if (f.pullType == "ENGAGE")                                 engagePull += f.strength;
            if (f.pullType == "SLOW")                                   slowPull   += f.strength;
        }

        if (req.lastImaginationEthicalSafety >= 0.0f &&
            req.lastImaginationEthicalSafety < 0.4f) {
            guardPull += 0.5f;
        }
        if (req.lastImaginationGoalAlignment >= 0.0f &&
            req.lastImaginationGoalAlignment > 0.7f) {
            engagePull += 0.4f;
        }
        if (req.lastImaginationPlausibility >= 0.0f &&
            req.lastImaginationPlausibility < 0.3f) {
            slowPull += 0.4f;
        }

        if (req.emotionValence < -0.4f) guardPull += 0.3f * req.emotionIntensity;
        if (req.emotionValence >  0.4f) openPull  += 0.3f * req.emotionIntensity;
        if (req.emotionArousal > 0.7f && req.emotionValence < 0.0f) alertPull += 0.4f;

        openPull  += req.speakerTrust * 0.5f;
        guardPull += (1.0f - req.speakerTrust) * 0.3f;

        struct Pull { const char* name; float v; };
        Pull pulls[] = {
            {"DANGER",    alertPull},
            {"DOUBT",     guardPull},
            {"SAFE",      openPull},
            {"REACH_OUT", comfortPull},
            {"ENGAGE",    engagePull},
            {"UNCERTAIN", slowPull},
        };

        const Pull* dominant = &pulls[0];
        for (auto& p : pulls) {
            if (p.v > dominant->v) dominant = &p;
        }
        sig.lean = dominant->name;

        float total = alertPull + guardPull + openPull
                    + comfortPull + engagePull + slowPull;
        sig.confidence = (total > 0.0f) ? std::min(1.0f, dominant->v / total) : 0.0f;
        sig.confidence *= (1.0f - 0.4f * req.beliefEntropy);

        sig.suppressReason = (sig.confidence > 0.6f && req.beliefEntropy > 0.6f);

        std::string basis = "lean=" + sig.lean;
        basis += " conf=" + std::to_string(sig.confidence).substr(0, 4);
        basis += " instincts=" + std::to_string(instincts.size());
        if (req.lastImaginationPlausibility >= 0.0f) {
            basis += " img_plaus=" +
                     std::to_string(req.lastImaginationPlausibility).substr(0, 4);
        }
        sig.basis = basis;

        return sig;
    }

    IntuitResult BuildCombinedSignal(const IntuitRequest& req,
                                     IntuitResult result) const {
        for (const auto& f : result.instincts) {
            if (f.urgent && f.strength > 0.7f) {
                result.urgent = true;
                break;
            }
        }

        result.holdAndReflect =
            (result.intuition.lean == "UNCERTAIN" && result.intuition.confidence > 0.5f) ||
            (result.intuition.lean == "DOUBT"     && result.intuition.confidence > 0.6f) ||
            (result.intuition.suppressReason       && result.intuition.confidence > 0.65f);

        static const std::unordered_map<std::string, std::string> leanToAct = {
            {"DANGER",    "WARN"},
            {"DOUBT",     "QUESTION"},
            {"SAFE",      "ASSERT"},
            {"REACH_OUT", "COMFORT"},
            {"ENGAGE",    "ACK_AND_PROBE"},
            {"UNCERTAIN", "QUESTION"},
        };
        auto it = leanToAct.find(result.intuition.lean);
        result.recommendedAct = (it != leanToAct.end()) ? it->second : "ASSERT";

        result.priorWeight = result.intuition.confidence
                           * (1.0f - 0.3f * req.beliefEntropy);
        result.priorWeight = std::clamp(result.priorWeight, 0.0f, 0.85f);
        if (req.isPreResponse) {
            result.priorWeight = std::min(result.priorWeight, 0.65f);
        }
        return result;
    }

    void AdjustPatternWeight(const std::string& pullType, float delta) {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& p : m_patterns) {
            if (p.pullType == pullType) {
                p.weight = std::clamp(p.weight + delta, 0.1f, 1.0f);
            }
        }
    }

    void Decay(float floor = 0.3f, float step = 0.001f) {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& p : m_patterns) {
            if (p.weight > floor) p.weight -= step;
        }
    }

private:
    static std::string ToLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c){ return (char)std::tolower(c); });
        return out;
    }

    mutable std::mutex            m_mutex;
    std::vector<InstinctPattern>  m_patterns;
};

}
