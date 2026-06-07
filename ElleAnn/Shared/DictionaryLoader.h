#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

class DictionaryLoader {
public:
    static DictionaryLoader& Instance();

    bool StartLoad(uint32_t startIdx = 0, uint32_t limit = 0);

    bool FetchOneAndStore(const std::string& word);

    struct State {
        std::string status;
        uint32_t    loaded = 0;
        uint32_t    failed = 0;
        uint32_t    skipped = 0;
        std::string last_word;
        std::string error;
        uint64_t    started_ms = 0;
        uint64_t    updated_ms = 0;
    };
    State GetState() const;

    void Shutdown();

    ~DictionaryLoader();

private:
    DictionaryLoader() = default;

    std::atomic<bool>     m_running{false};
    mutable std::mutex    m_stateMutex;
    State                 m_state;
    std::thread           m_worker;

    void WorkerRun(uint32_t startIdx, uint32_t limit);
    bool FetchDefinition(const std::string& word, std::string& outJson);
    bool ParseAndInsert(const std::string& word, const std::string& apiJson,
                        uint32_t& entriesAdded);
    void PersistState();
};
