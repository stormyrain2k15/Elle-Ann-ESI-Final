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

    void SetMaxRetries(uint32_t n) { m_maxRetries.store(n, std::memory_order_release); }
    uint32_t MaxRetries() const { return m_maxRetries.load(std::memory_order_acquire); }

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
