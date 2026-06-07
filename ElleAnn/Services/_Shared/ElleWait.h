#pragma once

#include <atomic>
#include <cstdint>
#include <windows.h>

namespace ElleWait {

inline void PollingSleep(uint32_t totalMs,
                         const std::atomic<bool>& runningFlag,
                         uint32_t stepMs = 50) {
    if (stepMs == 0) stepMs = 50;
    uint32_t slept = 0;
    while (slept < totalMs) {
        if (!runningFlag.load()) return;
        uint32_t chunk = (totalMs - slept) < stepMs ? (totalMs - slept) : stepMs;
        Sleep(chunk);
        slept += chunk;
    }
}

inline void PollingSleep(uint32_t totalMs,
                         const std::atomic<bool>* runningFlagPtr,
                         uint32_t stepMs = 50) {
    if (!runningFlagPtr) { Sleep(totalMs); return; }
    PollingSleep(totalMs, *runningFlagPtr, stepMs);
}

inline void PollingSleepUntilSet(uint32_t totalMs,
                                  const std::atomic<bool>& stopFlag,
                                  uint32_t stepMs = 50) {
    if (stepMs == 0) stepMs = 50;
    uint32_t slept = 0;
    while (slept < totalMs) {
        if (stopFlag.load()) return;
        uint32_t chunk = (totalMs - slept) < stepMs ? (totalMs - slept) : stepMs;
        Sleep(chunk);
        slept += chunk;
    }
}

}
