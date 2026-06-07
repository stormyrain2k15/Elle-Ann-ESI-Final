#pragma once
#ifndef ELLE_FIESTA_BRIEFINFO_RING_H
#define ELLE_FIESTA_BRIEFINFO_RING_H

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Fiesta {

class BriefInfoRing {
public:

    static constexpr size_t kMaxEntries = 4096;

    void Insert(uint16_t handle, const std::string& name) {
        std::lock_guard<std::mutex> lk(m_mx);
        if (m_map.size() >= kMaxEntries && !m_map.count(handle)) {
            m_map.erase(m_map.begin());
        }
        m_map[handle] = name;
    }
    void Remove(uint16_t handle) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_map.erase(handle);
    }

    std::string Resolve(uint16_t handle) const {
        std::lock_guard<std::mutex> lk(m_mx);
        const auto it = m_map.find(handle);
        return (it == m_map.end()) ? std::string() : it->second;
    }
    void Clear() {
        std::lock_guard<std::mutex> lk(m_mx);
        m_map.clear();
    }
    size_t Size() const {
        std::lock_guard<std::mutex> lk(m_mx);
        return m_map.size();
    }

private:
    mutable std::mutex                        m_mx;
    std::unordered_map<uint16_t, std::string> m_map;
};

}
#endif
