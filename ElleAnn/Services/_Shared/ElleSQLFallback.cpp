#include "ElleSQLFallback.h"
#include "ElleLogger.h"
#include "ElleSQLConn.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>

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

void ElleSQLFallback::Initialize(bool enabled) {
    if (!enabled) {
        m_enabled.store(false, std::memory_order_release);
        return;
    }
    fs::path dir = fs::path(ExeDirectory()) / "sqllogs";
    std::error_code ec;
    fs::create_directories(dir, ec);
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

bool ElleSQLFallback::Enqueue(Kind kind, const std::string& sqlOrProc,
                              const std::vector<std::string>& params) {
    if (!m_enabled.load(std::memory_order_acquire)) return false;

    std::string line;
    line.reserve(64 + sqlOrProc.size());
    line += "{\"ts\":";
    {
        uint64_t ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%llu", (unsigned long long)ms);
        line += buf;
    }
    line += ",\"kind\":\"";
    switch (kind) {
        case Kind::Exec:        line += "Exec";        break;
        case Kind::QueryParams: line += "QueryParams"; break;
        case Kind::CallProc:    line += "CallProc";    break;
    }
    line += "\",\"sql\":";
    AppendJsonEscaped(line, sqlOrProc);
    line += ",\"params\":[";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i) line += ",";
        AppendJsonEscaped(line, params[i]);
    }
    line += "]}\n";

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
    }
}

bool ElleSQLFallback::ReplayLine(const std::string& jsonLine, std::string& outErr) {

    size_t p = jsonLine.find("\"kind\":");
    if (p == std::string::npos) { outErr = "missing kind"; return false; }
    p += 7;
    while (p < jsonLine.size() && jsonLine[p] != '"') ++p;
    std::string kind;
    if (!ParseJsonString(jsonLine, p, kind)) { outErr = "bad kind"; return false; }

    p = jsonLine.find("\"sql\":");
    if (p == std::string::npos) { outErr = "missing sql"; return false; }
    p += 6;
    while (p < jsonLine.size() && jsonLine[p] != '"') ++p;
    std::string sql;
    if (!ParseJsonString(jsonLine, p, sql)) { outErr = "bad sql"; return false; }

    std::vector<std::string> params;
    p = jsonLine.find("\"params\":[");
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
        for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
            if (!ent.is_regular_file()) continue;
            const auto& p = ent.path();
            if (p.extension() == ".txt") files.push_back(p);
        }
    }
    std::sort(files.begin(), files.end());

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
        for (const auto& line : lines) {
            std::string err;
            if (ReplayLine(line, err)) {
                replayed++;
            } else {

                retained.push_back(line);

                auto it = std::find(lines.begin(), lines.end(), line);
                if (it != lines.end()) {
                    for (auto jt = it + 1; jt != lines.end(); ++jt)
                        retained.push_back(*jt);
                }
                break;
            }
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
                    f.put('\n');
                }
            }
            std::error_code ec;
            fs::rename(tmp, path, ec);

            if (ec) {
                fs::copy_file(tmp, path, fs::copy_options::overwrite_existing, ec);
                fs::remove(tmp, ec);
            }

            break;
        }
    }

    return replayed;
}

uint64_t ElleSQLFallback::PendingBytes() const {
    uint64_t total = 0;
    std::error_code ec;
    if (!fs::exists(m_dir, ec)) return 0;
    for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
        if (ent.is_regular_file() && ent.path().extension() == ".txt") {
            total += (uint64_t)ent.file_size(ec);
        }
    }
    return total;
}

uint32_t ElleSQLFallback::FileCount() const {
    uint32_t n = 0;
    std::error_code ec;
    if (!fs::exists(m_dir, ec)) return 0;
    for (const auto& ent : fs::directory_iterator(m_dir, ec)) {
        if (ent.is_regular_file() && ent.path().extension() == ".txt") ++n;
    }
    return n;
}
