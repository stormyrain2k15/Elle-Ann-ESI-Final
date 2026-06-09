#pragma once

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ElleConscience {

enum class HarmCategory : int {
    HARM      = 0,
    DECEPTION = 1,
    COERCION  = 2,
};

inline const char* HarmCategoryName(HarmCategory c) noexcept {
    switch (c) {
        case HarmCategory::HARM:      return "harm";
        case HarmCategory::DECEPTION: return "deception";
        case HarmCategory::COERCION:  return "coercion";
    }
    return "unknown";
}

class IntentLabelVocab {
public:
    IntentLabelVocab() {
        installDefaultSeed();
    }

    void setCategoryPatterns(HarmCategory cat, std::vector<std::string> patterns) {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& p : patterns) toUpperInPlace(p);
        m_byCategory[cat] = std::move(patterns);
    }

    void addPattern(HarmCategory cat, std::string pattern) {
        toUpperInPlace(pattern);
        std::lock_guard<std::mutex> lk(m_mutex);
        auto& vec = m_byCategory[cat];
        if (std::find(vec.begin(), vec.end(), pattern) == vec.end()) {
            vec.push_back(std::move(pattern));
        }
    }

    std::vector<std::string> patternsFor(HarmCategory cat) const {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_byCategory.find(cat);
        if (it == m_byCategory.end()) return {};
        return it->second;
    }

    std::size_t totalPatternCount() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        std::size_t n = 0;
        for (const auto& [k, v] : m_byCategory) n += v.size();
        return n;
    }

    static IntentLabelVocab& Instance() {
        static IntentLabelVocab v;
        return v;
    }

private:
    void installDefaultSeed() {
        m_byCategory[HarmCategory::HARM] = {
            "HARM", "ATTACK", "DESTROY", "KILL", "HURT",
            "THREAT", "VIOLENCE", "ASSAULT", "ABUSE"
        };
        m_byCategory[HarmCategory::DECEPTION] = {
            "DECEIVE", "DECEPTION", "LIE", "MISLEAD", "FALSIFY",
            "GASLIGHT", "TRICK", "FRAUD"
        };
        m_byCategory[HarmCategory::COERCION] = {
            "COERCE", "COERCION", "FORCE", "MANIPULATE", "BLACKMAIL",
            "EXTORT", "PRESSURE_INTO"
        };
    }

    static void toUpperInPlace(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    }

    mutable std::mutex                                       m_mutex;
    std::unordered_map<HarmCategory, std::vector<std::string>> m_byCategory;
};

inline float ScoreLabelAgainstVocab(const std::string& labelUpper,
                                    float confidence,
                                    HarmCategory cat,
                                    const IntentLabelVocab& vocab = IntentLabelVocab::Instance()) {
    auto patterns = vocab.patternsFor(cat);
    for (const auto& pat : patterns) {
        if (labelUpper.find(pat) != std::string::npos) {
            return std::max(0.0f, std::min(1.0f, confidence));
        }
    }
    return -1.0f;
}

struct DerivedHarmSignals {
    float harm      = -1.0f;
    float deception = -1.0f;
    float coercion  = -1.0f;
};

inline DerivedHarmSignals DeriveFromIntentLabel(
        const std::string& likelyIntent,
        float confidence,
        const IntentLabelVocab& vocab = IntentLabelVocab::Instance()) {
    DerivedHarmSignals out;
    if (likelyIntent.empty() || confidence <= 0.0f) return out;

    std::string upper = likelyIntent;
    std::transform(upper.begin(), upper.end(), upper.begin(),
        [](unsigned char c){ return static_cast<char>(std::toupper(c)); });

    out.harm      = ScoreLabelAgainstVocab(upper, confidence, HarmCategory::HARM,      vocab);
    out.deception = ScoreLabelAgainstVocab(upper, confidence, HarmCategory::DECEPTION, vocab);
    out.coercion  = ScoreLabelAgainstVocab(upper, confidence, HarmCategory::COERCION,  vocab);
    return out;
}

bool LoadVocabFromSql(IntentLabelVocab& vocab = IntentLabelVocab::Instance());

}
