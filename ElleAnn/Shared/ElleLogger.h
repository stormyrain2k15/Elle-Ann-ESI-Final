#pragma once
#ifndef ELLE_LOGGER_H
#define ELLE_LOGGER_H

#include "ElleTypes.h"
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <fstream>
#include <memory>
#include <unordered_map>

#define ELLE_LOG_TARGET_CONSOLE     0x01
#define ELLE_LOG_TARGET_FILE        0x02
#define ELLE_LOG_TARGET_DATABASE    0x04
#define ELLE_LOG_TARGET_WEBSOCKET   0x08
#define ELLE_LOG_TARGET_ALL         0x0F

class ElleLogger {
public:
    static ElleLogger& Instance();

    bool Initialize(ELLE_SERVICE_ID sourceService,
                    uint32_t targets = ELLE_LOG_TARGET_ALL,
                    ELLE_LOG_LEVEL minLevel = LOG_INFO);
    void Shutdown();

    void SetMinLevel(ELLE_LOG_LEVEL level);
    void SetLogFile(const std::string& path, uint32_t maxSizeMB = 100, uint32_t maxFiles = 10);
    void SetWebSocketBroadcaster(std::function<void(const std::string&)> broadcaster);

    void Log(ELLE_LOG_LEVEL level, const char* fmt, ...);
    void LogWithContext(ELLE_LOG_LEVEL level, const char* context, const char* fmt, ...);

    void Trace(const char* fmt, ...);
    void Debug(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Warn(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Fatal(const char* fmt, ...);

    void LogEmotion(const ELLE_EMOTION_STATE& state);
    void LogIntent(const ELLE_INTENT_RECORD& intent);
    void LogAction(const ELLE_ACTION_RECORD& action);
    void LogTrustChange(int32_t oldScore, int32_t newScore, const std::string& reason);
    void LogIPC(const ELLE_IPC_HEADER& header, bool incoming);
    void LogLLM(const ELLE_LLM_REQUEST& req, const ELLE_LLM_RESPONSE& resp);

    void LogChannel(const char* channel, const char* fmt, ...);

    uint64_t TotalEntries() const { return m_totalEntries; }
    uint64_t ErrorCount() const { return m_errorCount; }

private:
    ElleLogger() = default;
    ~ElleLogger();
    ElleLogger(const ElleLogger&) = delete;
    ElleLogger& operator=(const ElleLogger&) = delete;

    ELLE_SERVICE_ID m_sourceService = SVC_HEARTBEAT;
    uint32_t        m_targets = 0;
    std::atomic<ELLE_LOG_LEVEL>  m_minLevel{LOG_INFO};
    bool            m_initialized = false;

    HANDLE m_hConsole = nullptr;
    void WriteConsole(ELLE_LOG_LEVEL level, const std::string& formatted);

    std::string     m_logPath;
    std::ofstream   m_logFile;
    uint32_t        m_maxFileSizeMB = 100;
    uint32_t        m_maxFiles      = 10;
    uint64_t        m_currentFileSize = 0;

    uint32_t        m_maxLinesPerFile = 10000;
    uint32_t        m_currentLineCount = 0;
    int             m_currentDateYmd   = 0;
    uint32_t        m_currentDateSuffix = 0;
    std::mutex      m_fileMutex;
    void WriteFile(const std::string& formatted);
    void RotateFile();

    int  TodayYmd() const;

    void OpenDatedDebugFile();

    struct ChannelState {
        std::ofstream   file;
        uint32_t        lineCount = 0;
        int             dateYmd   = 0;
        uint32_t        dateSuffix = 0;
        std::mutex      mtx;
    };
    std::mutex                                                  m_channelsMutex;
    std::unordered_map<std::string, std::unique_ptr<ChannelState>> m_channels;

    std::string ExeDirectory() const;
    ChannelState& GetOrOpenChannel(const std::string& name);

    std::queue<ELLE_LOG_ENTRY> m_dbQueue;
    std::mutex                  m_dbMutex;
    std::condition_variable     m_dbCv;
    std::thread                 m_dbThread;
    std::atomic<bool>           m_dbRunning{false};

    std::atomic<bool>           m_dbClosing{false};
    void DatabaseWriterThread();

    std::function<void(const std::string&)> m_wsBroadcaster;
    std::mutex                              m_wsMutex;

    std::string FormatEntry(ELLE_LOG_LEVEL level, const std::string& message,
                            const std::string& context = "");
    const char* LevelString(ELLE_LOG_LEVEL level);
    uint16_t LevelColor(ELLE_LOG_LEVEL level);

    std::atomic<uint64_t> m_totalEntries{0};
    std::atomic<uint64_t> m_errorCount{0};

    std::mutex m_logMutex;
};

#define ELLE_TRACE(fmt, ...)  ElleLogger::Instance().Trace(fmt, ##__VA_ARGS__)
#define ELLE_DEBUG(fmt, ...)  ElleLogger::Instance().Debug(fmt, ##__VA_ARGS__)
#define ELLE_INFO(fmt, ...)   ElleLogger::Instance().Info(fmt, ##__VA_ARGS__)
#define ELLE_WARN(fmt, ...)   ElleLogger::Instance().Warn(fmt, ##__VA_ARGS__)
#define ELLE_ERROR(fmt, ...)  ElleLogger::Instance().Error(fmt, ##__VA_ARGS__)
#define ELLE_FATAL(fmt, ...)  ElleLogger::Instance().Fatal(fmt, ##__VA_ARGS__)

#define ELLE_LOG_ASSERT(fmt, ...)  ElleLogger::Instance().LogChannel("asserts", fmt, ##__VA_ARGS__)
#define ELLE_LOG_SOCKET(fmt, ...)  ElleLogger::Instance().LogChannel("socket",  fmt, ##__VA_ARGS__)
#define ELLE_LOG_HTTP(fmt, ...)    ElleLogger::Instance().LogChannel("http",    fmt, ##__VA_ARGS__)
#define ELLE_LOG_SQL(fmt, ...)     ElleLogger::Instance().LogChannel("sql",     fmt, ##__VA_ARGS__)

#define ELLE_ASSERT(cond, fmt, ...)                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            ELLE_LOG_ASSERT("ASSERT FAILED (%s:%d) [%s]: " fmt,              \
                            __FILE__, __LINE__, #cond, ##__VA_ARGS__);       \
            ELLE_ERROR("ASSERT FAILED [%s]: " fmt, #cond, ##__VA_ARGS__);    \
        }                                                                    \
    } while (0)

#endif
