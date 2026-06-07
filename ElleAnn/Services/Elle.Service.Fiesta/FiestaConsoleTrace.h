#pragma once
#ifndef ELLE_FIESTA_CONSOLE_TRACE_H
#define ELLE_FIESTA_CONSOLE_TRACE_H

#include "FiestaProtoTable.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace Fiesta {
namespace Trace {

inline std::atomic<bool>& EnabledFlag() {
    static std::atomic<bool> g{false};
    return g;
}

inline void SetEnabled(bool on) { EnabledFlag().store(on); }
inline bool IsEnabled()          { return EnabledFlag().load(std::memory_order_relaxed); }

inline std::mutex& TraceMutex() {
    static std::mutex m;
    return m;
}

inline void WriteTimestamp(char buf[16]) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t   = system_clock::to_time_t(now);
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::snprintf(buf, 16, "%02d:%02d:%02d.%03lld",
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long long>(ms));
}

inline std::string HexPreview(const std::vector<uint8_t>& bytes,
                              std::size_t max_bytes = 16) {
    std::string s;
    s.reserve(max_bytes * 3 + 4);
    static const char* hex = "0123456789ABCDEF";
    std::size_t shown = std::min(bytes.size(), max_bytes);
    for (std::size_t i = 0; i < shown; ++i) {
        s += hex[bytes[i] >> 4];
        s += hex[bytes[i] & 0xF];
        if (i + 1 < shown) s += ' ';
    }
    if (bytes.size() > shown) s += " …";
    return s;
}

inline const char* RX_PFX() { return "\x1b[36mRX\x1b[0m"; }
inline const char* TX_PFX() { return "\x1b[33mTX\x1b[0m"; }
inline const char* ST_PFX() { return "\x1b[32m>>\x1b[0m"; }
inline const char* HI_PFX() { return "\x1b[35m★\x1b[0m"; }
inline const char* ER_PFX() { return "\x1b[31m!\x1b[0m"; }

inline void OnRx(uint16_t opcode, const std::vector<uint8_t>& payload) {
    if (!IsEnabled()) return;
    std::string_view name = OpcodeName(opcode);
    char ts[16]; WriteTimestamp(ts);
    std::string hex = HexPreview(payload);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s  0x%04X  %-30.*s (%4zu B)  %s\n",
                ts, RX_PFX(), opcode,
                static_cast<int>(name.size()), name.data(),
                payload.size(), hex.c_str());
    std::fflush(stdout);
}

inline void OnTx(uint16_t opcode, const std::vector<uint8_t>& payload) {
    if (!IsEnabled()) return;
    std::string_view name = OpcodeName(opcode);
    char ts[16]; WriteTimestamp(ts);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s  0x%04X  %-30.*s (%4zu B)\n",
                ts, TX_PFX(), opcode,
                static_cast<int>(name.size()), name.data(),
                payload.size());
    std::fflush(stdout);
}

inline void OnStateChange(std::string_view prev, std::string_view next) {
    if (!IsEnabled()) return;
    char ts[16]; WriteTimestamp(ts);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s   state %.*s -> \x1b[1m%.*s\x1b[0m\n",
                ts, ST_PFX(),
                static_cast<int>(prev.size()), prev.data(),
                static_cast<int>(next.size()), next.data());
    std::fflush(stdout);
}

inline void OnChat(std::string_view sender, std::string_view text,
                   const char* kind = "chat") {
    if (!IsEnabled()) return;
    char ts[16]; WriteTimestamp(ts);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s   %-7s \"%.*s\" -> \"%.*s\"\n",
                ts, HI_PFX(), kind,
                static_cast<int>(sender.size()), sender.data(),
                static_cast<int>(text.size()), text.data());
    std::fflush(stdout);
}

inline void OnMove(uint16_t handle,
                   uint32_t fromX, uint32_t fromY,
                   uint32_t toX,   uint32_t toY,
                   uint8_t  movetype) {
    if (!IsEnabled()) return;
    char ts[16]; WriteTimestamp(ts);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s   move    h=0x%04X (%u,%u) -> (%u,%u) type=0x%02X\n",
                ts, HI_PFX(), handle, fromX, fromY, toX, toY, movetype);
    std::fflush(stdout);
}

inline void OnError(std::string_view what) {
    if (!IsEnabled()) return;
    char ts[16]; WriteTimestamp(ts);
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("%s  %s   %.*s\n",
                ts, ER_PFX(),
                static_cast<int>(what.size()), what.data());
    std::fflush(stdout);
}

inline void Banner(std::string_view who) {
    if (!IsEnabled()) return;
    std::lock_guard<std::mutex> lk(TraceMutex());
    std::printf("\x1b[1;36m"
                "════════════════════════════════════════════════════════════════════\n"
                " Elle.Service.Fiesta — live console trace\n"
                " %.*s\n"
                "════════════════════════════════════════════════════════════════════"
                "\x1b[0m\n",
                static_cast<int>(who.size()), who.data());
    std::printf("Legend:  \x1b[36mRX\x1b[0m server->Elle   "
                "\x1b[33mTX\x1b[0m Elle->server   "
                "\x1b[32m>>\x1b[0m state   "
                "\x1b[35m★\x1b[0m decoded   "
                "\x1b[31m!\x1b[0m error\n\n");
    std::fflush(stdout);
}

#ifdef _WIN32
inline void EnsureWindowsConsole() {

    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {

        AllocConsole();
        SetConsoleTitleA("Elle.Service.Fiesta — live trace");
    }

    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$",  "r", stdin);

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#else
inline void EnsureWindowsConsole() {  }
#endif

}
}

#endif
