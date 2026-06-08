# Elle Intuition Engine — Test Harness

The Intuition service ships with a standalone CMake harness so the
deterministic engine logic can be ctested in CI (alongside Probability).

The engine is **header-only** (`core/IntuitionEngine.h`) — no Windows,
no SQL, no IPC. The `Elle.Service.Intuition.vcxproj` includes that
header, and `Intuition.cpp` (the Windows service wrapper) delegates to
an `IntuitionEngine` instance.

## Layout

```
Elle.Service.Intuition/
├── CMakeLists.txt                ← standalone CMake project (this harness)
├── Elle.Service.Intuition.vcxproj ← Windows service project (Visual Studio)
├── Intuition.cpp                 ← service wrapper (IPC + SQL + Windows)
├── core/
│   └── IntuitionEngine.h         ← pure C++17 deterministic engine
└── tests/
    ├── test_main.cpp             ← doctest main
    ├── test_patterns.cpp         ← LoadDefaults / ReplacePatterns / Decay / AdjustPatternWeight
    ├── test_fire_instincts.cpp   ← Tier-1 firing logic (trust floor, emotion gate, dedupe, etc.)
    ├── test_synthesize.cpp       ← Tier-2 SynthesizeIntuition (lean / confidence / basis)
    ├── test_combined_signal.cpp  ← BuildCombinedSignal + full Process pipeline
    └── test_feedback.cpp         ← Pattern-weight learning loop
```

## Build & run

```bash
cd ElleAnn/Services/Elle.Service.Intuition
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure       # 39/39 PASS
```

CMake fetches `doctest` v2.4.11 via `FetchContent`; no extra setup
required.

On systems with a CMake older than 3.5, pass
`-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to the configure step (workaround
for the FetchContent-pulled `doctest` policy bump).

## Coverage

| Test file                    | Cases | Focus                                           |
|------------------------------|------:|-------------------------------------------------|
| `test_patterns.cpp`          |     4 | LoadDefaults (31 baseline patterns), Replace, AdjustPatternWeight clamp, Decay |
| `test_fire_instincts.cpp`    |     9 | Exact/substring match, trust floor, emotion gate, arousal amplification, dedupe, case-insensitivity, sort order, empty stimulus |
| `test_synthesize.cpp`        |    11 | Lean derivation for ALERT/COMFORT/SAFE/DOUBT/UNCERTAIN/ENGAGE, valence+arousal, ethical_safety, plausibility, goal_alignment, suppressReason, basis string |
| `test_combined_signal.cpp`   |    11 | Urgent gating, lean→recommendedAct mapping, holdAndReflect triggers, priorWeight scaling with entropy/pre-response cap, full Process pipeline |
| `test_feedback.cpp`          |     4 | AdjustPatternWeight positive/negative, no-op on unknown pull, Decay floor |
| **Total**                    |  **39** | **100% PASS in Feb 2026**                      |

## What is NOT covered here

The harness covers the **pure engine**, not the service wrapper.
Out-of-scope for ctest:

- `LoadPatterns` (SQL ODBC path)
- `LogFiring` / `EnsureTables` (SQL DDL + bulk insert)
- `HandleIntuitRequest` / `SendResult` (IPC encode/decode)
- `CacheEmotionState` / `CacheProbState` (broadcast payload handling)

Those are exercised end-to-end by the Windows MSBuild + the live
service mesh, not by this harness.

## CI

`.github/workflows/elleann-build.yml` runs this harness alongside the
Probability harness on every push to `main` and every PR.
