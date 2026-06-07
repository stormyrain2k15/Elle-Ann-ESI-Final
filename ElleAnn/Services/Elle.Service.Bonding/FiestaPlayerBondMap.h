#pragma once
#ifndef ELLE_FIESTA_PLAYER_BOND_MAP_H
#define ELLE_FIESTA_PLAYER_BOND_MAP_H

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Elle { namespace Bonding {

struct PlayerBond {
    std::string name;
    uint16_t    last_handle = 0;

    uint32_t    encounter_count = 0;
    uint32_t    chat_count      = 0;
    uint32_t    shout_count     = 0;
    uint32_t    whispers_to_us  = 0;
    uint32_t    whispers_from_us = 0;

    uint64_t    first_seen_ms = 0;
    uint64_t    last_seen_ms  = 0;

    float       affinity    = 0.0f;
    float       familiarity = 0.0f;
};

class FiestaPlayerBondMap {
public:
    static constexpr size_t kMaxBonds = 1024;

    void OnAppear(const std::string& name, uint16_t handle, uint64_t now_ms) {
        if (name.empty()) return;
        std::lock_guard<std::mutex> lk(m_mx);
        auto& b = TouchLocked(name, now_ms);
        b.last_handle = handle;
        b.encounter_count++;
        b.familiarity = ClampUnit(b.familiarity + 0.01f);
    }

    void OnChat(const std::string& name, uint16_t handle,
                const std::string& channel, uint64_t now_ms) {
        if (name.empty()) return;
        std::lock_guard<std::mutex> lk(m_mx);
        auto& b = TouchLocked(name, now_ms);
        b.last_handle = handle;
        if      (channel == "normal")       b.chat_count++;
        else if (channel == "shout")        b.shout_count++;
        else if (channel == "whisper_in")   b.whispers_to_us++;
        else if (channel == "whisper_out")  b.whispers_from_us++;

        const float delta = (channel.rfind("whisper", 0) == 0) ? 0.04f : 0.02f;
        b.familiarity = ClampUnit(b.familiarity + delta);
    }

    void NudgeAffinity(const std::string& name, float delta, uint64_t now_ms) {
        if (name.empty()) return;
        std::lock_guard<std::mutex> lk(m_mx);
        auto& b = TouchLocked(name, now_ms);
        b.affinity = ClampSigned(b.affinity + delta);
    }

    PlayerBond Get(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mx);
        const auto it = m_map.find(name);
        return (it == m_map.end()) ? PlayerBond{} : it->second;
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lk(m_mx);
        return m_map.size();
    }

    std::vector<PlayerBond> SnapshotByFamiliarity(size_t topN = 32) const {
        std::lock_guard<std::mutex> lk(m_mx);
        std::vector<PlayerBond> out;
        out.reserve(m_map.size());
        for (const auto& kv : m_map) out.push_back(kv.second);
        if (out.size() > topN) {
            std::partial_sort(out.begin(), out.begin() + topN, out.end(),
                              [](const PlayerBond& a, const PlayerBond& b) {
                                  return a.familiarity > b.familiarity;
                              });
            out.resize(topN);
        } else {
            std::sort(out.begin(), out.end(),
                      [](const PlayerBond& a, const PlayerBond& b) {
                          return a.familiarity > b.familiarity;
                      });
        }
        return out;
    }

    void Clear() {
        std::lock_guard<std::mutex> lk(m_mx);
        m_map.clear();
    }

private:
    static float ClampUnit(float v)   { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
    static float ClampSigned(float v) { return v < -1.f ? -1.f : (v > 1.f ? 1.f : v); }

    PlayerBond& TouchLocked(const std::string& name, uint64_t now_ms) {
        auto it = m_map.find(name);
        if (it == m_map.end()) {
            if (m_map.size() >= kMaxBonds) EvictLeastEngagedLocked();
            PlayerBond fresh;
            fresh.name = name;
            fresh.first_seen_ms = now_ms;
            fresh.last_seen_ms  = now_ms;
            it = m_map.emplace(name, std::move(fresh)).first;
        } else {
            it->second.last_seen_ms = now_ms;
        }
        return it->second;
    }

    void EvictLeastEngagedLocked() {
        if (m_map.empty()) return;
        auto victim = m_map.begin();
        uint64_t lowest = ScoreOf(victim->second);
        for (auto it = m_map.begin(); it != m_map.end(); ++it) {
            const uint64_t s = ScoreOf(it->second);
            if (s < lowest) { lowest = s; victim = it; }
        }
        m_map.erase(victim);
    }

    static uint64_t ScoreOf(const PlayerBond& b) {
        return (uint64_t)b.encounter_count
             + (uint64_t)b.chat_count
             + (uint64_t)b.shout_count
             + 2 * (uint64_t)(b.whispers_to_us + b.whispers_from_us);
    }

    mutable std::mutex                          m_mx;
    std::unordered_map<std::string, PlayerBond> m_map;
};

}}
#endif
