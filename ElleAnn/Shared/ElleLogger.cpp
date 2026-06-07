#include "ElleLogger.h"
#include "ElleSQLConn.h"
#include "ElleQueueIPC.h"
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>

ElleLogger& ElleLogger::Instance() {
    static ElleLogger inst;
    return inst;
}

ElleLogger::~ElleLogger() { Shutdown(); }

bool ElleLogger::Initialize(ELLE_SERVICE_ID sourceService, uint32_t targets,
                             ELLE_LOG_LEVEL minLevel) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_sourceService = sourceService;
    m_targets = targets;
    m_minLevel.store(minLevel, std::memory_order_release);

    if (targets & ELLE_LOG_TARGET_CONSOLE) {
        m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if (targets & ELLE_LOG_TARGET_DATABASE) {
        m_dbRunning = true;
        m_dbThread = std::thread(&ElleLogger::DatabaseWriterThread, this);
    }

    if ((targets & ELLE_LOG_TARGET_FILE) && m_logPath.empty()) {
        std::lock_guard<std::mutex> fileLock(m_fileMutex);
        m_currentDateYmd    = TodayYmd();
        m_currentDateSuffix = 0;
        m_currentLineCount  = 0;
        m_currentFileSize   = 0;
        OpenDatedDebugFile();
    }

    m_initialized = true;
    return true;
}

void ElleLogger::Shutdown() {
    m_initialized = false;

    m_dbClosing.store(true, std::memory_order_release);
    m_dbRunning.store(false, std::memory_order_release);
    m_dbCv.notify_all();
    if (m_dbThread.joinable()) m_dbThread.join();
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_logFile.is_open()) m_logFile.close();
}

void ElleLogger::SetLogFile(const std::string& path, uint32_t maxSizeMB, uint32_t maxFiles) {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    m_logPath = path;
    m_maxFileSizeMB = maxSizeMB;
    m_maxFiles = maxFiles;

    std::filesystem::path p(path);
    std::filesystem::create_directories(p.parent_path());

    m_logFile.open(path, std::ios::app);
    if (m_logFile.is_open()) {
        m_currentFileSize = (uint64_t)std::filesystem::file_size(path);
    }
}

void ElleLogger::SetWebSocketBroadcaster(std::function<void(const std::string&)> broadcaster) {
    std::lock_guard<std::mutex> lock(m_wsMutex);
    m_wsBroadcaster = std::move(broadcaster);
}

void ElleLogger::SetMinLevel(ELLE_LOG_LEVEL level) {
    m_minLevel.store(level, std::memory_order_release);
}

void ElleLogger::Log(ELLE_LOG_LEVEL level, const char* fmt, ...) {
    if (!m_initialized || level < m_minLevel.load(std::memory_order_acquire)) return;

    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string formatted = FormatEntry(level, buffer);

    if (m_targets & ELLE_LOG_TARGET_CONSOLE) WriteConsole(level, formatted);
    if (m_targets & ELLE_LOG_TARGET_FILE)    WriteFile(formatted);
    if (m_targets & ELLE_LOG_TARGET_WEBSOCKET) {

        std::function<void(const std::string&)> cb;
        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            cb = m_wsBroadcaster;
        }
        if (cb) cb(formatted);
    }

    if (m_targets & ELLE_LOG_TARGET_DATABASE) {
        ELLE_LOG_ENTRY entry;
        entry.level = level;
        entry.source_svc = m_sourceService;
        entry.timestamp_ms = ELLE_MS_NOW();
        strncpy_s(entry.message, buffer, ELLE_MAX_MSG - 1);

        std::lock_guard<std::mutex> lock(m_dbMutex);
        m_dbQueue.push(entry);
    }

    m_totalEntries++;
    if (level >= LOG_ERROR) m_errorCount++;
}

void ElleLogger::Trace(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_TRACE, "%s", buffer);
}

void ElleLogger::Debug(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_DEBUG, "%s", buffer);
}

void ElleLogger::Info(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_INFO, "%s", buffer);
}

void ElleLogger::Warn(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_WARN, "%s", buffer);
}

void ElleLogger::Error(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_ERROR, "%s", buffer);
}

void ElleLogger::Fatal(const char* fmt, ...) {
    char buffer[4096];
    va_list args; va_start(args, fmt); vsnprintf(buffer, sizeof(buffer), fmt, args); va_end(args);
    Log(LOG_FATAL, "%s", buffer);
}

void ElleLogger::LogEmotion(const ELLE_EMOTION_STATE& state) {

    struct EmoPair { int idx; float val; };
    std::vector<EmoPair> sorted;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        if (state.dimensions[i] > 0.01f) {
            sorted.push_back({i, state.dimensions[i]});
        }
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const EmoPair& a, const EmoPair& b) { return a.val > b.val; });

    std::ostringstream ss;
    ss << "Emotion[V:" << std::fixed << std::setprecision(2) << state.valence
       << " A:" << state.arousal << " D:" << state.dominance << "] Top: ";

    int shown = 0;
    for (auto& e : sorted) {
        if (shown >= 5) break;
        if (shown > 0) ss << ", ";
        ss << e.idx << "=" << std::setprecision(2) << e.val;
        shown++;
    }

    Info("%s", ss.str().c_str());
}

void ElleLogger::LogTrustChange(int32_t oldScore, int32_t newScore, const std::string& reason) {
    Info("Trust: %d -> %d (%+d) [%s]", oldScore, newScore, newScore - oldScore, reason.c_str());
}

void ElleLogger::LogLLM(const ELLE_LLM_REQUEST& req, const ELLE_LLM_RESPONSE& resp) {
    if (resp.success) {
        Info("LLM[%s] %d+%d=%d tokens, %.0fms",
             req.model_name, resp.tokens_prompt, resp.tokens_completion,
             resp.tokens_total, resp.latency_ms);
    } else {
        Error("LLM[%s] FAILED: %s", req.model_name, resp.error);
    }
}

void ElleLogger::LogWithContext(ELLE_LOG_LEVEL level, const char* context, const char* fmt, ...) {
    if (!m_initialized || level < m_minLevel.load(std::memory_order_acquire)) return;

    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string formatted = FormatEntry(level, buffer, context ? context : "");

    if (m_targets & ELLE_LOG_TARGET_CONSOLE) WriteConsole(level, formatted);
    if (m_targets & ELLE_LOG_TARGET_FILE)    WriteFile(formatted);
    if (m_targets & ELLE_LOG_TARGET_WEBSOCKET) {
        std::function<void(const std::string&)> cb;
        {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            cb = m_wsBroadcaster;
        }
        if (cb) cb(formatted);
    }

    if (m_targets & ELLE_LOG_TARGET_DATABASE) {
        ELLE_LOG_ENTRY entry;
        entry.level = level;
        entry.source_svc = m_sourceService;
        entry.timestamp_ms = ELLE_MS_NOW();
        strncpy_s(entry.message, buffer, ELLE_MAX_MSG - 1);
        if (context) strncpy_s(entry.context, context, sizeof(entry.context) - 1);
        else         entry.context[0] = '\0';
        std::lock_guard<std::mutex> lock(m_dbMutex);
        m_dbQueue.push(entry);
    }

    m_totalEntries++;
    if (level >= LOG_ERROR) m_errorCount++;
}

void ElleLogger::LogIntent(const ELLE_INTENT_RECORD& intent) {
    Info("Intent[type=%u urg=%.2f status=%u]: %s",
         (unsigned)intent.type,
         intent.urgency,
         (unsigned)intent.status,
         intent.description);
}

void ElleLogger::LogAction(const ELLE_ACTION_RECORD& action) {
    Info("Action[type=%u status=%u intent=%llu]: %s",
         (unsigned)action.type,
         (unsigned)action.status,
         (unsigned long long)action.intent_id,
         action.command);
}

void ElleLogger::LogIPC(const ELLE_IPC_HEADER& header, bool incoming) {
    Trace("IPC %s [type=%u src=%u dst=%u payload=%u]",
          incoming ? "RX" : "TX",
          (unsigned)header.msg_type,
          (unsigned)header.source_svc,
          (unsigned)header.dest_svc,
          (unsigned)header.payload_size);
}

std::string ElleLogger::FormatEntry(ELLE_LOG_LEVEL level, const std::string& message,
                                     const std::string& context) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    std::ostringstream ss;
    ss << "[" << std::setfill('0')
       << std::setw(4) << st.wYear << "-"
       << std::setw(2) << st.wMonth << "-"
       << std::setw(2) << st.wDay << " "
       << std::setw(2) << st.wHour << ":"
       << std::setw(2) << st.wMinute << ":"
       << std::setw(2) << st.wSecond << "."
       << std::setw(3) << st.wMilliseconds << "]"
       << " [" << LevelString(level) << "]"
       << " [" << ElleIPC::GetServiceName(m_sourceService) << "]";

    if (!context.empty()) ss << " [" << context << "]";
    ss << " " << message;

    return ss.str();
}

const char* ElleLogger::LevelString(ELLE_LOG_LEVEL level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "?????";
    }
}

uint16_t ElleLogger::LevelColor(ELLE_LOG_LEVEL level) {
    switch (level) {
        case LOG_TRACE: return 8;
        case LOG_DEBUG: return 7;
        case LOG_INFO:  return 10;
        case LOG_WARN:  return 14;
        case LOG_ERROR: return 12;
        case LOG_FATAL: return 79;
        default:        return 7;
    }
}

void ElleLogger::WriteConsole(ELLE_LOG_LEVEL level, const std::string& formatted) {
    if (m_hConsole) {
        SetConsoleTextAttribute(m_hConsole, LevelColor(level));
    }
    std::cout << formatted << std::endl;
    if (m_hConsole) {
        SetConsoleTextAttribute(m_hConsole, 7);
    }
}

void ElleLogger::WriteFile(const std::string& formatted) {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (!m_logFile.is_open()) return;

    int today = TodayYmd();
    if (today != m_currentDateYmd) {
        m_currentDateYmd    = today;
        m_currentDateSuffix = 0;
        m_currentLineCount  = 0;
        m_logFile.close();
        OpenDatedDebugFile();
        if (!m_logFile.is_open()) return;
    }

    m_logFile << formatted << "\n";
    m_logFile.flush();
    m_currentFileSize += formatted.size() + 1;
    m_currentLineCount++;

    if (m_currentLineCount >= m_maxLinesPerFile ||
        m_currentFileSize > (uint64_t)m_maxFileSizeMB * 1024 * 1024) {
        RotateFile();
    }
}

void ElleLogger::RotateFile() {

    m_logFile.close();
    m_currentDateSuffix++;
    m_currentLineCount = 0;
    m_currentFileSize  = 0;
    OpenDatedDebugFile();
}

int ElleLogger::TodayYmd() const {
    time_t t = time(nullptr);
    struct tm lt;
    localtime_s(&lt, &t);
    return (lt.tm_year + 1900) * 10000 + (lt.tm_mon + 1) * 100 + lt.tm_mday;
}

std::string ElleLogger::ExeDirectory() const {

    static std::string s_cached;
    static std::once_flag s_once;
    std::call_once(s_once, []() {
        char buf[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        std::filesystem::path p(buf);
        s_cached = p.parent_path().string();
        if (s_cached.empty()) s_cached = ".";
    });
    return s_cached;
}

void ElleLogger::OpenDatedDebugFile() {

    char ymdBuf[32];
    int y =  m_currentDateYmd / 10000;
    int mo = (m_currentDateYmd / 100) % 100;
    int d =  m_currentDateYmd % 100;
    if (m_currentDateSuffix == 0) {
        snprintf(ymdBuf, sizeof(ymdBuf), "%04d-%02d-%02d.txt", y, mo, d);
    } else {
        snprintf(ymdBuf, sizeof(ymdBuf), "%04d-%02d-%02d-%02u.txt", y, mo, d, m_currentDateSuffix);
    }

    std::filesystem::path dir(ExeDirectory());
    dir /= "debug";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    m_logPath = (dir / ymdBuf).string();

    m_logFile.open(m_logPath, std::ios::app);
    if (m_logFile.is_open()) {
        m_currentFileSize = (uint64_t)std::filesystem::file_size(m_logPath, ec);

        std::ifstream probe(m_logPath);
        m_currentLineCount = 0;
        for (std::string line; std::getline(probe, line); ++m_currentLineCount) {}
    }
}

ElleLogger::ChannelState& ElleLogger::GetOrOpenChannel(const std::string& name) {
    std::lock_guard<std::mutex> lk(m_channelsMutex);
    auto it = m_channels.find(name);
    if (it != m_channels.end()) return *it->second;

    auto st = std::make_unique<ChannelState>();
    m_channels.emplace(name, std::move(st));
    return *m_channels[name];
}

void ElleLogger::LogChannel(const char* channel, const char* fmt, ...) {
    if (!channel || !*channel || !fmt) return;

    char body[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(body, sizeof(body), fmt, args);
    va_end(args);

    auto& st = GetOrOpenChannel(channel);
    std::lock_guard<std::mutex> lk(st.mtx);

    int today = TodayYmd();
    bool needOpen = !st.file.is_open();
    if (today != st.dateYmd) {
        st.dateYmd    = today;
        st.dateSuffix = 0;
        st.lineCount  = 0;
        if (st.file.is_open()) st.file.close();
        needOpen = true;
    } else if (st.lineCount >= m_maxLinesPerFile) {

        st.dateSuffix++;
        st.lineCount = 0;
        st.file.close();
        needOpen = true;
    }

    if (needOpen) {
        char buf[64];
        int y  =  st.dateYmd / 10000;
        int mo = (st.dateYmd / 100) % 100;
        int d  =  st.dateYmd % 100;
        if (st.dateSuffix == 0)
            snprintf(buf, sizeof(buf), "%s_%04d-%02d-%02d.log", channel, y, mo, d);
        else
            snprintf(buf, sizeof(buf), "%s_%04d-%02d-%02d-%02u.log", channel, y, mo, d, st.dateSuffix);
        std::filesystem::path path = std::filesystem::path(ExeDirectory()) / buf;
        st.file.open(path.string(), std::ios::app);
        if (st.file.is_open() && st.dateSuffix == 0) {
            std::ifstream probe(path.string());
            st.lineCount = 0;
            for (std::string line; std::getline(probe, line); ++st.lineCount) {}
        }
    }

    if (!st.file.is_open()) return;

    time_t t = time(nullptr);
    struct tm lt;
    localtime_s(&lt, &t);
    char ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", &lt);

    st.file << "[" << ts << "] " << body << "\n";
    st.file.flush();
    st.lineCount++;
}

void ElleLogger::DatabaseWriterThread() {
    while (m_dbRunning.load()) {
        std::vector<ELLE_LOG_ENTRY> batch;
        {
            std::unique_lock<std::mutex> lock(m_dbMutex);

            m_dbCv.wait_for(lock, std::chrono::seconds(1), [this]{
                return !m_dbRunning.load() || !m_dbQueue.empty();
            });
            while (!m_dbQueue.empty() && batch.size() < 100) {
                batch.push_back(m_dbQueue.front());
                m_dbQueue.pop();
            }
        }

        for (auto& entry : batch) {
            ElleDB::WriteLog((ELLE_LOG_LEVEL)entry.level,
                            (ELLE_SERVICE_ID)entry.source_svc,
                            entry.message);
        }
    }

    std::vector<ELLE_LOG_ENTRY> tail;
    {
        std::lock_guard<std::mutex> lock(m_dbMutex);
        while (!m_dbQueue.empty()) {
            tail.push_back(m_dbQueue.front());
            m_dbQueue.pop();
        }
    }
    for (auto& entry : tail) {
        ElleDB::WriteLog((ELLE_LOG_LEVEL)entry.level,
                        (ELLE_SERVICE_ID)entry.source_svc,
                        entry.message);
    }
}
