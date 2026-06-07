#pragma once
#ifndef ELLE_SQL_FALLBACK_H
#define ELLE_SQL_FALLBACK_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

class ElleSQLFallback {
public:
    enum class Kind { Exec, QueryParams, CallProc };

    static ElleSQLFallback& Instance();

    void Initialize(bool enabled = true);

    void Shutdown();

    bool IsEnabled() const { return m_enabled.load(std::memory_order_acquire); }

    bool Enqueue(Kind kind, const std::string& sqlOrProc,
                 const std::vector<std::string>& params);

    void NudgeDrain();

    uint32_t DrainNow();

    uint64_t PendingBytes() const;
    uint32_t FileCount() const;

private:
    ElleSQLFallback() = default;
    ~ElleSQLFallback();
    ElleSQLFallback(const ElleSQLFallback&) = delete;
    ElleSQLFallback& operator=(const ElleSQLFallback&) = delete;

    std::string m_dir;

    std::mutex  m_fileMutex;

    std::atomic<bool> m_enabled{false};
    std::atomic<bool> m_running{false};

    std::thread             m_worker;
    std::mutex              m_workerMutex;
    std::condition_variable m_workerCv;
    std::atomic<bool>       m_pendingWork{false};

    void WorkerLoop();
    bool ReplayLine(const std::string& jsonLine, std::string& outErr);
    std::string TodayYmd() const;
    std::string ExeDirectory() const;
};

#endif
