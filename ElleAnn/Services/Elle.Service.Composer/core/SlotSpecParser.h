#pragma once
#include <cmath>
#include <regex>
#include <string>
#include <vector>

namespace elle::composer::core {

struct ParsedSlotSpec {
    std::string name;
    std::string posTag;
    std::string form;
    bool        optional = false;
};

inline std::vector<ParsedSlotSpec> ParseSlotSpecs(const std::string& tmpl) {
    std::vector<ParsedSlotSpec> slots;
    std::regex slotRe(R"(\[([A-Z_]+):([A-Z_]+)(?::([A-Za-z0-9_]+))?\](\?)?)");
    auto begin = std::sregex_iterator(tmpl.begin(), tmpl.end(), slotRe);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        ParsedSlotSpec s;
        s.name     = (*it)[1].str();
        s.posTag   = (*it)[2].str();
        s.form     = (*it)[3].matched ? (*it)[3].str() : "-";
        s.optional = (*it)[4].matched;
        slots.push_back(std::move(s));
    }
    return slots;
}

inline float ScoreFrameByRecency(float baseWeight,
                                 int64_t lastUsedMs,
                                 int64_t nowMs,
                                 double halfLifeSec = 300.0,
                                 double maxPenaltyFraction = 0.6) {
    float s = baseWeight;
    if (lastUsedMs > 0 && nowMs > lastUsedMs) {
        double elapsedSec = static_cast<double>(nowMs - lastUsedMs) / 1000.0;
        double penalty    = std::pow(0.5, elapsedSec / halfLifeSec);
        s *= static_cast<float>(1.0 - maxPenaltyFraction * penalty);
    }
    return s;
}

}
