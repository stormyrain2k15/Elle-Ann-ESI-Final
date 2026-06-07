#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

class InflectionTables {
public:
    bool Load();

    // Returns inflected form for (lemma, form).
    // Falls back to bare lemma if the pair is not in the table.
    std::string Inflect(const std::string& lemma,
                        const std::string& form) const;

    size_t Count() const;

private:
    struct Key {
        std::string lemma;
        std::string form;
        bool operator==(const Key& o) const {
            return lemma == o.lemma && form == o.form;
        }
    };
    struct KeyHash {
        size_t operator()(const Key& k) const {
            size_t h = std::hash<std::string>{}(k.lemma);
            h ^= std::hash<std::string>{}(k.form) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    mutable std::mutex m_mutex;
    std::unordered_map<Key, std::string, KeyHash> m_table;
};
