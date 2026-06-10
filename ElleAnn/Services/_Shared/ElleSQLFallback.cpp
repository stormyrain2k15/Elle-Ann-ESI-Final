#include "ElleSQLFallback.h"
#include "ElleLogger.h"
#include "ElleSQLConn.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
#  include <windows.h>
#endif

namespace fs = std::filesystem;

ElleSQLFallback& ElleSQLFallback::Instance() {
    static ElleSQLFallback inst;
    return inst;
}

ElleSQLFallback::~ElleSQLFallback() { Shutdown(); }

std::string ElleSQLFallback::ExeDirectory() const {
#ifdef _WIN32
    char buf[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    fs::path p(buf);
    std::string s = p.parent_path().string();
    return s.empty() ? "." : s;
#else
    return ".";
#endif
}

std::string ElleSQLFallback::TodayYmd() const {
    std::time_t t = std::time(nullptr);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
    return buf;
}

std::string ElleSQLFallback::PoisonDir() const {
    return (fs::path(m_dir) / "poison").string();
}

void ElleSQLFallback::Initialize(bool enabled) {
    if (!enabled) {
        m_enabled.store(false, std::memory_order_release);
        return;
    }
    fs::path dir = fs::path(ExeDirectory()) / "sqllogs";
    std::error_code ec;
    fs::create_directories(dir, ec);
    fs::create_directories(dir / "poison", ec);
    m_dir = dir.string();
    m_enabled.store(true, std::memory_order_release);
}

void ElleSQLFallback::Shutdown() {
    bool wasRunning = m_running.exchange(false, std::memory_order_acq_rel);
    if (wasRunning) {
        m_workerCv.notify_all();
        if (m_worker.joinable()) m_worker.join();
    }
    m_enabled.store(false, std::memory_order_release);
}

static void AppendJsonEscaped(std::string& out, const std::string& in) {
    out.push_back('"');
    for (unsigned char c : in) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
                    out += buf;
                } else {
                    out.push_back((char)c);
                }
        }
    }
    out.push_back('"');
}

static bool ParseJsonString(const std::string& src, size_t& pos, std::string& out) {
    if (pos >= src.size() || src[pos] != '"') return false;
    ++pos;
    out.clear();
    while (pos < src.size() && src[pos] != '"') {
        char c = src[pos++];
        if (c == '\\' && pos < src.size()) {
            char e = src[pos++];
            switch (e) {
                case '"':  out.push_back('"');  break;
                case '\\': out.push_back('\\'); break;
                case '/':  out.push_back('/');  break;
                case 'n':  out.push_back('\n'); break;
                case 'r':  out.push_back('\r'); break;
                case 't':  out.push_back('\t'); break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'u': {
                    if (pos + 4 > src.size()) return false;
                    unsigned cp = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = src[pos++];
                        unsigned d;
                        if (h >= '0' && h <= '9') d = h - '0';
                        else if (h >= 'a' && h <= 'f') d = 10 + (h - 'a');
                        else if (h >= 'A' && h <= 'F') d = 10 + (h - 'A');
                        else return false;
                        cp = (cp << 4) | d;
                    }
                    if (cp < 0x80) out.push_back((char)cp);
                    else if (cp < 0x800) {
                        out.push_back((char)(0xC0 | (cp >> 6)));
                        out.push_back((char)(0x80 | (cp & 0x3F)));
                    } else {
                        out.push_back((char)(0xE0 | (cp >> 12)));
                        out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                        out.push_back((char)(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default: out.push_back(e); break;
            }
        } else {
            out.push_back(c);
        }
    }
    if (pos >= src.size()) return false;
    ++pos;
    return true;
}

std::string ElleSQLFallback::BuildLine(Kind kind,
                                       const std::string& sqlOrProc,
                                       const std::vector<std::string>& params,
                                       uint64_t ts_ms,
                                       Idempotency idem,
                                       uint32_t retry_count) {
    std::string line;
    line.reserve(96 + sqlOrProc.size());
    line += "{\"ts\":";
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%llu", (unsigned long long)ts_ms);
        line += buf;
    }
    line += ",\"kind\":\"";
    switch (kind) {
        case Kind::Exec:        line += "Exec";        break;
        case Kind::QueryParams: line += "QueryParams"; break;
        case Kind::CallProc:    line += "CallProc";    break;
    }
    line += "\",\"idem\":\"";
    line += ElleSQLFallbackClassifier::IdempotencyToString(idem);
    line += "\",\"retry_count\":";
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%u", retry_count);
        line += buf;
    }
    line += ",\"sql\":";
    AppendJsonEscaped(line, sqlOrProc);
    line += ",\"params\":[";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i) line += ",";
        AppendJsonEscaped(line, params[i]);
    }
    line += "]}\n";
    return line;
}

bool ElleSQLFallback::ExtractIntField(const std::string& jsonLine,
                                      const std::string& key,
                                      int64_t& out) {
    std::string needle = "\"" + key + "\":";
    size_t p = jsonLine.find(needle);
    if (p == std::string::npos) return false;
    p += needle.size();
    while (p < jsonLine.size() && (jsonLine[p] == ' ' || jsonLine[p] == '"')) ++p;
    if (p >= jsonLine.size()) return false;
    int64_t v = 0;
    bool neg = false;
    if (jsonLine[p] == '-') { neg = true; ++p; }
    if (p >= jsonLine.size() || jsonLine[p] < '0' || jsonLine[p] > '9') return false;
    while (p < jsonLine.size() && jsonLine[p] >= '0' && jsonLine[p] <= '9') {
        v = v * 10 + (jsonLine[p] - '0');
        ++p;
    }
    out = neg ? -v : v;
    return true;
}

std::string ElleSQLFallback::ExtractStringField(const std::string& jsonLine,
                                                const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t p = jsonLine.find(needle);
    if (p == std::string::npos) return std::string();
    p += needle.size();
    while (p < jsonLine.size() && jsonLine[p] == ' ') ++p;
    std::string out;
    if (!ParseJsonString(jsonLine, p, out)) return std::string();
    return out;
}

std::string ElleSQLFallback::ReencodeWithIncrementedRetry(const std::string& jsonLine) {
    std::string needle = "\"retry_count\":";
    size_t p = jsonLine.find(needle);
    if (p == std::string::npos) {
        std::string s = jsonLine;
        if (!s.empty() && s.back() == '\n') s.pop_back();
        if (!s.empty() && s.back() == '}') s.pop_back();
        s += ",\"retry_count\":1}\n";
        return s;
    }
    size_t value_start = p + needle.size();
    while (value_start < jsonLine.size() && jsonLine[value_start] == ' ') ++value_start;
    size_t value_end = value_start;
    while (value_end < jsonLine.size() && jsonLine[value_end] >= '0' && jsonLine[value_end] <= '9') ++value_end;
    if (value_end == value_start) return jsonLine;
    int64_t cur = 0;
    for (size_t i = value_start; i < value_end; ++i) cur = cur * 10 + (jsonLine[i] - '0');
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lld", (long long)(cur + 1));
    return jsonLine.substr(0, value_start) + std::string(buf) + jsonLine.substr(value_end);
}

void ElleSQLFallback::QuarantineLine(const std::string& path,
                                     const std::string& jsonLine) {
    fs::path poison_dir = PoisonDir();
    std::error_code ec;
    fs::create_directories(poison_dir, ec);
    fs::path poison_file = poison_dir / fs::path(path).filename();
    std::ofstream f(poison_file, std::ios::app | std::ios::binary);
    if (!f.is_open()) {
        ELLE_ERROR("ElleSQLFallback: cannot open poison file %s",
                   poison_file.string().c_str());
        return;
    }
    f.write(jsonLine.data(), (std::streamsize)jsonLine.size());
    if (!jsonLine.empty() && jsonLine.back() != '\n') f.put('\n');
    ELLE_ERROR("ElleSQLFallback: line quarantined to %s",
               poison_file.string().c_str());
}

bool ElleSQLFallback::EnqueueWithHint(Kind kind, const std::string& sqlOrProc,
                                      const std::vector<std::string>& params,
                                      Idempotency idem) {
    if (!m_enabled.load(std::memory_order_acquire)) return false;

    uint64_t ts_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string line = BuildLine(kind, sqlOrProc, params, ts_ms, idem, 0);

    {
        std::lock_guard<std::mutex> lk(m_fileMutex);
        fs::path path = fs::path(m_dir) / (TodayYmd() + ".txt");
        std::ofstream f(path, std::ios::app | std::ios::binary);
        if (!f.is_open()) {
            ELLE_ERROR("ElleSQLFallback: open(%s) failed — offline queue lost this line",
                       path.string().c_str());
            return false;
        }
        f.write(line.data(), (std::streamsize)line.size());
        if (!f.good()) {
            ELLE_ERROR("ElleSQLFallback: write to %s failed", path.string().c_str());
            return false;
        }
    }

    bool expected = false;
    if (m_running.compare_exchange_strong(expected, true,
                                          std::memory_order_acq_rel)) {
        m_worker = std::thread(&ElleSQLFallback::WorkerLoop, this);
    }
    m_pendingWork.store(true, std::memory_order_release);
    m_workerCv.notify_all();
    return true;
}

bool ElleSQLFallback::Enqueue(Kind kind, const std::string& sqlOrProc,
                              const std::vector<std::string>& params) {
    Idempotency idem = (kind == Kind::CallProc)
        ? ElleSQLFallbackClassifier::ClassifyCallProc(sqlOrProc)
        : ElleSQLFallbackClassifier::ClassifyExec(sqlOrProc);
    return EnqueueWithHint(kind, sqlOrProc, params, idem);
}

void ElleSQLFallback::NudgeDrain() {
    if (!m_enabled.load(std::memory_order_acquire)) return;
    m_pendingWork.store(true, std::memory_order_release);
    m_workerCv.notify_all();
}

void ElleSQLFallback::WorkerLoop() {
    while (m_running.load(std::memory_order_acquire)) {
        {
            std::unique_lock<std::mutex> lk(m_workerMutex);
            m_workerCv.wait_for(lk, std::chrono::seconds(10), [this] {
                return !m_running.load(std::memory_order_acquire) ||
                       m_pendingWork.load(std::memory_order_acquire);
            });
        }
        if (!m_running.load(std::memory_order_acquire)) break;
        m_pendingWork.store(false, std::memory_order_release);

        SQLResultSet probe = ElleSQLPool::Instance().Query("SELECT 1");
        if (!probe.success) continue;

        uint32_t replayed = DrainNow();
        if (replayed) {
            ELLE_INFO("ElleSQLFallback: drained %u queued queries", replayed);
        }

        const uint64_t interval = m_poisonLoadIntervalMs.load(std::memory_order_acquire);
        if (interval > 0) {
            uint64_t now_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            uint64_t last = m_lastPoisonAttemptMs.load(std::memory_order_acquire);
            if (last == 0 || now_ms - last >= interval) {
                m_lastPoisonAttemptMs.store(now_ms, std::memory_order_release);
                m_totalPoisonAttempts.fetch_add(1, std::memory_order_acq_rel);
                try {
                    uint32_t inserted = LoadPoisonIntoSql(500);
                    m_lastPoisonSuccessMs.store(now_ms, std::memory_order_release);
                    m_lastPoisonInserted.store(inserted, std::memory_order_release);
                    m_totalPoisonSuccesses.fetch_add(1, std::memory_order_acq_rel);
                    m_totalPoisonInserted.fetch_add(inserted, std::memory_order_acq_rel);
                    {
                        std::lock_guard<std::mutex> lk(m_lastPoisonErrorMx);
                        m_lastPoisonError.clear();
                    }
                    if (inserted) {
                        ELLE_INFO("ElleSQLFallback: reaper loaded %u poison lines into SQL",
                                  inserted);
                    }
                } catch (const std::exception& e) {
                    std::lock_guard<std::mutex> lk(m_lastPoisonErrorMx);
                    m_lastPoisonError = e.what();
                    ELLE_ERROR("ElleSQLFallback: poison reaper threw: %s", e.what());
                }
            }
        }
    }
}

ElleSQLFallback::PoisonLoadStatus ElleSQLFallback::GetPoisonLoadStatus() const {
    PoisonLoadStatus s;
    s.last_attempt_ms = m_lastPoisonAttemptMs.load(std::memory_order_acquire);
    s.last_success_ms = m_lastPoisonSuccessMs.load(std::memory_order_acquire);
    s.last_inserted   = m_lastPoisonInserted.load(std::memory_order_acquire);
    s.total_attempts  = m_totalPoisonAttempts.load(std::memory_order_acquire);
    s.total_successes = m_totalPoisonSuccesses.load(std::memory_order_acquire);
    s.total_inserted  = m_totalPoisonInserted.load(std::memory_order_acquire);
    {
        std::lock_guard<std::mutex> lk(m_lastPoisonErrorMx);
        s.last_error = m_lastPoisonError;
    }
    return s;
}

bool ElleSQLFallback::ReplayLine(const std::string& jsonLine, std::string& outErr) {
    std::string kind = ExtractStringField(jsonLine, "kind");
    if (kind.empty()) { outErr = "missing kind"; return false; }

    std::string sql = ExtractStringField(jsonLine, "sql");
    if (sql.empty()) { outErr = "missing sql"; return false; }

    std::vector<std::string> params;
    size_t p = jsonLine.find("\"params\":[");
    if (p != std::string::npos) {
        p += 10;
        while (p < jsonLine.size() && jsonLine[p] != ']') {
            while (p < jsonLine.size() && jsonLine[p] != '"' && jsonLine[p] != ']') ++p;
            if (p >= jsonLine.size() || jsonLine[p] == ']') break;
            std::string s;
            if (!ParseJsonString(jsonLine, p, s)) { outErr = "bad param"; return false; }
            params.push_back(std::move(s));
            while (p < jsonLine.size() && (jsonLine[p] == ',' || jsonLine[p] == ' ')) ++p;
        }
    }

    auto& pool = ElleSQLPool::Instance();
    if (kind == "CallProc") {
        auto rs = pool.CallProc(sql, params);
        if (!rs.success) { outErr = "CallProc failed: " + rs.error; return false; }
        return true;
    }
    if (kind == "QueryParams") {
        auto rs = pool.QueryParams(sql, params);
        if (!rs.success) { outErr = "QueryParams failed: " + rs.error; return false; }
        return true;
    }

    if (!pool.Exec(sql)) { outErr = "Exec failed"; return false; }
    return true;
}

uint32_t ElleSQLFallback::DrainNow() {
    if (!m_enabled.load(std::memory_order_acquire)) return 0;

    std::vector<fs::path> files;
    {
        std::lock_guard<std::mutex> lk(m_fileMutex);
        std::error_code ec;
        if (!fs::exists(m_dir, ec)) return 0;
        fs::path poison_dir = PoisonDir();
        for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
            if (!ent.is_regular_file()) continue;
            const auto& p = ent.path();
            if (p.extension() != ".txt") continue;
            if (p.parent_path() == poison_dir) continue;
            files.push_back(p);
        }
    }
    std::sort(files.begin(), files.end());

    const uint32_t maxRetries = m_maxRetries.load(std::memory_order_acquire);
    uint32_t replayed = 0;

    for (const auto& path : files) {
        std::vector<std::string> lines;
        {
            std::lock_guard<std::mutex> lk(m_fileMutex);
            std::ifstream f(path, std::ios::binary);
            if (!f.is_open()) continue;
            std::string line;
            while (std::getline(f, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (!line.empty()) lines.push_back(std::move(line));
            }
        }
        if (lines.empty()) {
            std::lock_guard<std::mutex> lk(m_fileMutex);
            std::error_code ec;
            fs::remove(path, ec);
            continue;
        }

        std::vector<std::string> retained;
        bool stalled = false;
        for (size_t idx = 0; idx < lines.size(); ++idx) {
            const std::string& line = lines[idx];
            std::string err;
            if (ReplayLine(line, err)) {
                replayed++;
                continue;
            }

            int64_t retry_count = 0;
            ExtractIntField(line, "retry_count", retry_count);
            std::string idemField = ExtractStringField(line, "idem");
            ElleSQLFallbackClassifier::Idempotency idem =
                ElleSQLFallbackClassifier::IdempotencyFromString(idemField);

            bool quarantine_now = (idem == ElleSQLFallbackClassifier::Idempotency::No)
                               || ((uint32_t)retry_count + 1 >= maxRetries);

            if (quarantine_now) {
                QuarantineLine(path.string(), line);
                ELLE_ERROR("ElleSQLFallback: poison line %s after %u retries (idem=%s) — %s",
                           path.filename().string().c_str(),
                           (unsigned)retry_count,
                           idemField.c_str(),
                           err.c_str());
                continue;
            }

            std::string bumped = ReencodeWithIncrementedRetry(line);
            retained.push_back(bumped);
            for (size_t j = idx + 1; j < lines.size(); ++j) {
                retained.push_back(lines[j]);
            }
            stalled = true;
            break;
        }

        std::lock_guard<std::mutex> lk(m_fileMutex);
        if (retained.empty()) {
            std::error_code ec;
            fs::remove(path, ec);
        } else {
            fs::path tmp = path; tmp += ".tmp";
            {
                std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
                for (const auto& l : retained) {
                    f.write(l.data(), (std::streamsize)l.size());
                    if (l.empty() || l.back() != '\n') f.put('\n');
                }
            }
            std::error_code ec;
            fs::rename(tmp, path, ec);
            if (ec) {
                fs::copy_file(tmp, path, fs::copy_options::overwrite_existing, ec);
                fs::remove(tmp, ec);
            }
            if (stalled) break;
        }
    }

    return replayed;
}

uint64_t ElleSQLFallback::PendingBytes() const {
    uint64_t total = 0;
    std::error_code ec;
    if (!fs::exists(m_dir, ec)) return 0;
    fs::path poison_dir = fs::path(m_dir) / "poison";
    for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
        if (!ent.is_regular_file()) continue;
        if (ent.path().extension() != ".txt") continue;
        if (ent.path().parent_path() == poison_dir) continue;
        total += (uint64_t)ent.file_size(ec);
    }
    return total;
}

uint32_t ElleSQLFallback::FileCount() const {
    uint32_t n = 0;
    std::error_code ec;
    if (!fs::exists(m_dir, ec)) return 0;
    fs::path poison_dir = fs::path(m_dir) / "poison";
    for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
        if (!ent.is_regular_file()) continue;
        if (ent.path().extension() != ".txt") continue;
        if (ent.path().parent_path() == poison_dir) continue;
        ++n;
    }
    return n;
}

uint64_t ElleSQLFallback::PoisonBytes() const {
    uint64_t total = 0;
    std::error_code ec;
    fs::path poison_dir = fs::path(m_dir) / "poison";
    if (!fs::exists(poison_dir, ec)) return 0;
    for (const auto& ent : fs::directory_iterator(poison_dir, ec)) {
        if (ent.is_regular_file() && ent.path().extension() == ".txt") {
            total += (uint64_t)ent.file_size(ec);
        }
    }
    return total;
}

uint32_t ElleSQLFallback::PoisonFileCount() const {
    uint32_t n = 0;
    std::error_code ec;
    fs::path poison_dir = fs::path(m_dir) / "poison";
    if (!fs::exists(poison_dir, ec)) return 0;
    for (const auto& ent : fs::directory_iterator(poison_dir, ec)) {
        if (ent.is_regular_file() && ent.path().extension() == ".txt") ++n;
    }
    return n;
}

static std::string ExtractRawParamsArray(const std::string& jsonLine) {
    size_t p = jsonLine.find("\"params\":");
    if (p == std::string::npos) return "[]";
    p += 9;
    while (p < jsonLine.size() && jsonLine[p] == ' ') ++p;
    if (p >= jsonLine.size() || jsonLine[p] != '[') return "[]";
    size_t depth = 0;
    bool in_str = false;
    size_t start = p;
    for (; p < jsonLine.size(); ++p) {
        char c = jsonLine[p];
        if (c == '\\' && p + 1 < jsonLine.size()) { ++p; continue; }
        if (c == '"') in_str = !in_str;
        else if (!in_str) {
            if (c == '[') ++depth;
            else if (c == ']') {
                --depth;
                if (depth == 0) return jsonLine.substr(start, p - start + 1);
            }
        }
    }
    return "[]";
}

std::vector<ElleSQLFallback::PoisonLine>
ElleSQLFallback::ListPoison(uint32_t maxLines) const {
    std::vector<PoisonLine> out;
    std::error_code ec;
    fs::path poison_dir = fs::path(m_dir) / "poison";
    if (!fs::exists(poison_dir, ec)) return out;

    std::vector<fs::path> files;
    for (const auto& ent : fs::directory_iterator(poison_dir, ec)) {
        if (ent.is_regular_file() && ent.path().extension() == ".txt") {
            files.push_back(ent.path());
        }
    }
    std::sort(files.begin(), files.end());

    for (const auto& path : files) {
        if (out.size() >= maxLines) break;
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) continue;
        std::string line;
        std::string fname = path.filename().string();
        while (out.size() < maxLines && std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            PoisonLine pl;
            pl.source_file = fname;
            pl.raw_line    = line;
            int64_t v = 0;
            if (ExtractIntField(line, "ts", v))           pl.ts_ms       = v;
            if (ExtractIntField(line, "retry_count", v))  pl.retry_count = v;
            pl.kind        = ExtractStringField(line, "kind");
            pl.idem        = ExtractStringField(line, "idem");
            pl.sql_or_proc = ExtractStringField(line, "sql");
            pl.params_json = ExtractRawParamsArray(line);
            out.push_back(std::move(pl));
        }
    }
    return out;
}

uint32_t ElleSQLFallback::LoadPoisonIntoSql(uint32_t maxLines) {
    SQLResultSet probe = ElleSQLPool::Instance().Query("SELECT 1");
    if (!probe.success) {
        ELLE_ERROR("ElleSQLFallback::LoadPoisonIntoSql: SQL unreachable, aborting");
        return 0;
    }

    auto rows = ListPoison(maxLines);
    if (rows.empty()) return 0;

    uint64_t loaded_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    uint32_t inserted = 0;
    for (const auto& r : rows) {
        std::vector<std::string> params = {
            std::to_string(loaded_ms),
            r.source_file,
            r.raw_line,
            r.ts_ms > 0 ? std::to_string(r.ts_ms) : std::string(),
            r.kind,
            r.idem,
            std::to_string(r.retry_count),
            r.sql_or_proc,
            r.params_json
        };
        auto rs = ElleSQLPool::Instance().CallProc(
            "ElleCore.dbo.usp_SQLFallbackPoisonLoad", params);
        if (rs.success) {
            int64_t was = 0;
            if (!rs.rows.empty() && rs.rows[0].TryGetInt(1, was) && was == 1) {
                ++inserted;
            }
        } else {
            ELLE_ERROR("ElleSQLFallback::LoadPoisonIntoSql: row insert failed — %s",
                       rs.error.c_str());
        }
    }
    return inserted;
}
