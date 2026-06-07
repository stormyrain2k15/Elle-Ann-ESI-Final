#include "FrameLibrary.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleLogger.h"
#include <algorithm>
#include <chrono>
#include <cmath>

bool FrameLibrary::Load() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_frames.clear();

    auto rs = ElleSQLPool::Instance().Query(
        "SELECT frame_id, kind, act, ISNULL(pos_pattern,''), template, weight, "
        "ISNULL(last_used_ms,0) "
        "FROM ElleHeart.dbo.composer_frame ORDER BY kind, act, weight DESC");

    if (!rs.success) {
        ELLE_ERROR("FrameLibrary: failed to load composer_frame");
        return false;
    }

    for (auto& row : rs.rows) {
        ComposerFrame f;
        f.frameId    = std::stoll(row[0]);
        f.kind       = row[1];
        f.act        = row[2];
        f.posPattern = row[3];
        f.templateStr= row[4];
        f.weight     = std::stof(row[5]);
        f.lastUsedMs = std::stoll(row[6]);
        m_frames.push_back(std::move(f));
    }

    ELLE_INFO("FrameLibrary: loaded %zu frames", m_frames.size());
    return true;
}

float FrameLibrary::Score(const ComposerFrame& f) const {
    float s = f.weight;

    // Recency penalty: exponential decay, half-life 5 minutes.
    if (f.lastUsedMs > 0) {
        using namespace std::chrono;
        int64_t nowMs = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
        double elapsedSec = static_cast<double>(nowMs - f.lastUsedMs) / 1000.0;
        double halfLife   = 300.0;
        double penalty    = std::pow(0.5, elapsedSec / halfLife);
        s *= static_cast<float>(1.0 - 0.6 * penalty);
    }

    return s;
}

const ComposerFrame* FrameLibrary::Pick(const std::string& kind,
                                         const std::string& act) const
{
    std::lock_guard<std::mutex> lk(m_mutex);
    const ComposerFrame* best = nullptr;
    float bestScore = -1.0f;

    for (const auto& f : m_frames) {
        if (f.kind != kind) continue;
        if (f.act  != act  && f.act != "*") continue;
        float s = Score(f);
        if (s > bestScore) {
            bestScore = s;
            best = &f;
        }
    }
    return best;
}

void FrameLibrary::MarkUsed(int64_t frameId) {
    using namespace std::chrono;
    int64_t nowMs = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count();

    {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& f : m_frames) {
            if (f.frameId == frameId) {
                f.lastUsedMs = nowMs;
                break;
            }
        }
    }

    ElleSQLPool::Instance().Exec(
        "UPDATE ElleHeart.dbo.composer_frame SET last_used_ms=" +
        std::to_string(nowMs) +
        " WHERE frame_id=" + std::to_string(frameId));
}

size_t FrameLibrary::Count() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_frames.size();
}
