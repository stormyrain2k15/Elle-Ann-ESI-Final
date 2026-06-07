#pragma once
#include "../_Shared/ElleTypes.h"
#include "../_Shared/json.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

using json = nlohmann::json;

struct ComposerFrame {
    int64_t     frameId     = 0;
    std::string kind;
    std::string act;
    std::string posPattern;
    std::string templateStr;
    float       weight      = 1.0f;
    int64_t     lastUsedMs  = 0;
};

class FrameLibrary {
public:
    bool Load();

    // Returns best-scoring frame for (kind, act).
    // Applies recency penalty so Elle doesn't repeat herself.
    const ComposerFrame* Pick(const std::string& kind,
                              const std::string& act) const;

    // Mark a frame as just used (updates last_used_ms in memory + SQL).
    void MarkUsed(int64_t frameId);

    size_t Count() const;

private:
    float Score(const ComposerFrame& f) const;

    mutable std::mutex              m_mutex;
    std::vector<ComposerFrame>      m_frames;
};
