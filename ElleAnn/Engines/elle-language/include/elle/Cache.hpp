// ============================================================================
// Elle Engine -- Thread-safe LRU cache
// File: include/elle/Cache.hpp
//
// Header-only. Bounded capacity, O(1) get/put, mutex-guarded. Designed for
// caching SQL lookups (word -> WordRecord, phrase -> PhraseRecord, etc.).
// Invalidation: clear() or evict(key).
// ============================================================================
#pragma once

#include <cstddef>
#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>

namespace elle {

template <class Key, class Value, class Hash = std::hash<Key>>
class LruCache {
public:
    explicit LruCache(std::size_t capacity)
        : m_capacity(capacity == 0 ? 1 : capacity) {}

    LruCache(const LruCache&)            = delete;
    LruCache& operator=(const LruCache&) = delete;

    void put(const Key& key, Value value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(key);
        if (it != m_index.end()) {
            it->second->second = std::move(value);
            m_lru.splice(m_lru.begin(), m_lru, it->second);
            return;
        }
        m_lru.emplace_front(key, std::move(value));
        m_index[key] = m_lru.begin();
        if (m_lru.size() > m_capacity) {
            const auto& victim = m_lru.back();
            m_index.erase(victim.first);
            m_lru.pop_back();
        }
    }

    std::optional<Value> get(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(key);
        if (it == m_index.end()) {
            ++m_misses;
            return std::nullopt;
        }
        m_lru.splice(m_lru.begin(), m_lru, it->second);
        ++m_hits;
        return it->second->second;
    }

    void evict(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(key);
        if (it != m_index.end()) {
            m_lru.erase(it->second);
            m_index.erase(it);
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lru.clear();
        m_index.clear();
        m_hits = m_misses = 0;
    }

    [[nodiscard]] std::size_t size()     const { std::lock_guard<std::mutex> l(m_mutex); return m_lru.size(); }
    [[nodiscard]] std::size_t capacity() const { return m_capacity; }
    [[nodiscard]] std::size_t hits()     const { std::lock_guard<std::mutex> l(m_mutex); return m_hits; }
    [[nodiscard]] std::size_t misses()   const { std::lock_guard<std::mutex> l(m_mutex); return m_misses; }

private:
    using ListEntry = std::pair<Key, Value>;
    using ListType  = std::list<ListEntry>;
    using IndexType = std::unordered_map<Key, typename ListType::iterator, Hash>;

    mutable std::mutex m_mutex;
    std::size_t        m_capacity;
    ListType           m_lru;
    IndexType          m_index;
    std::size_t        m_hits   = 0;
    std::size_t        m_misses = 0;
};

} // namespace elle
