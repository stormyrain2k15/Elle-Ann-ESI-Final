#pragma once
#ifndef ELLE_SQL_FALLBACK_H
#define ELLE_SQL_FALLBACK_H

#include "ElleSQLFallbackClassifier.h"

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

class ElleSQLFallback {
public:
    enum class Kind { Exec, QueryParams, CallProc };

    using Idempotency = ElleSQLFallbackClassifier::Idempotency;

    static ElleSQLFallback& Instance();

    void Initialize(bool enabled = true);

    void Shutdown();

    bool IsEnabled() const { return m_enabled.load(std::memory_order_acquire); }

    bool Enqueue(Kind kind, const std::string& sqlOrProc,
                 const std::vector<std::string>& params);

    bool EnqueueWithHint(Kind kind, const std::string& sqlOrProc,
                         const std::vector<std::string>& params,
                         Idempotency idem);

    void NudgeDrain();

    uint32_t DrainNow();

    uint64_t PendingBytes() const;
    uint32_t FileCount() const;

    uint64_t PoisonBytes() const;
    uint32_t PoisonFileCount() const;

    struct PoisonLine {
        std::string source_file;
        std::string raw_line;
        int64_t     ts_ms       = 0;
        std::string kind;
        std::string idem;
        int64_t     retry_count = 0;
        std::string sql_or_proc;
        std::string params_json;
    };

    std::vector<PoisonLine> ListPoison(uint32_t maxLines = 200) const;

    uint32_t LoadPoisonIntoSql(uint32_t maxLines = 500);

    void SetMaxRetries(uint32_t n) { m_maxRetries.store(n, std::memory_order_release); }
    uint32_t MaxRetries() const { return m_maxRetries.load(std::memory_order_acquire); }

    void SetPoisonLoadIntervalMs(uint64_t ms) {
        m_poisonLoadIntervalMs.store(ms, std::memory_order_release);
    }
    uint64_t PoisonLoadIntervalMs() const {
        return m_poisonLoadIntervalMs.load(std::memory_order_acquire);
    }

    struct PoisonLoadStatus {
        uint64_t last_attempt_ms   = 0;
        uint64_t last_success_ms   = 0;
        uint32_t last_inserted     = 0;
        uint32_t total_attempts    = 0;
        uint32_t total_successes   = 0;
        uint32_t total_inserted    = 0;
        std::string last_error;
    };

    PoisonLoadStatus GetPoisonLoadStatus() const;

private:
    ElleSQLFallback() = default;
    ~ElleSQLFallback();
    ElleSQLFallback(const ElleSQLFallback&) = delete;
    ElleSQLFallback& operator=(const ElleSQLFallback&) = delete;

    std::string m_dir;

    std::mutex  m_fileMutex;

    std::atomic<bool> m_enabled{false};
    std::atomic<bool> m_running{false};
    std::atomic<uint32_t> m_maxRetries{5};
    std::atomic<uint64_t> m_poisonLoadIntervalMs{0};

    std::atomic<uint64_t> m_lastPoisonAttemptMs{0};
    std::atomic<uint64_t> m_lastPoisonSuccessMs{0};
    std::atomic<uint32_t> m_lastPoisonInserted{0};
    std::atomic<uint32_t> m_totalPoisonAttempts{0};
    std::atomic<uint32_t> m_totalPoisonSuccesses{0};
    std::atomic<uint32_t> m_totalPoisonInserted{0};
    mutable std::mutex    m_lastPoisonErrorMx;
    std::string           m_lastPoisonError;

    std::thread             m_worker;
    std::mutex              m_workerMutex;
    std::condition_variable m_workerCv;
    std::atomic<bool>       m_pendingWork{false};

    void WorkerLoop();
    bool ReplayLine(const std::string& jsonLine, std::string& outErr);
    std::string TodayYmd() const;
    std::string ExeDirectory() const;
    std::string PoisonDir() const;
    void QuarantineLine(const std::string& path, const std::string& jsonLine);

    static bool ExtractIntField(const std::string& jsonLine,
                                const std::string& key,
                                int64_t& out);
    static std::string ReencodeWithIncrementedRetry(const std::string& jsonLine);
    static std::string ExtractStringField(const std::string& jsonLine,
                                          const std::string& key);
    static std::string BuildLine(Kind kind,
                                 const std::string& sqlOrProc,
                                 const std::vector<std::string>& params,
                                 uint64_t ts_ms,
                                 Idempotency idem,
                                 uint32_t retry_count);
};

#endif
