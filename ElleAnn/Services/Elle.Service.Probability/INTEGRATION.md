# Elle Probability Engine — Integration Status

`elle_probability_core` is fully integrated into the Elle-Ann mesh as
`SVC_PROBABILITY`.

## What ships in this directory

- **Core library** (`src/`, `include/`) — Bayesian substrate.
  Standalone C++17 static lib, CMake-built.
- **IPC service wrapper** (`service/ProbabilityService.cpp`,
  `ProbabilityHost.cpp`, `ProbabilityProto.cpp`) — wires the engine into
  the named-pipe mesh as `SVC_PROBABILITY`. Speaks `IPC_PROB_ANALYZE` /
  `IPC_PROB_RESPONSE` / `IPC_PROB_TRUST` / `IPC_PROB_INJECT`.
- **Bridge** (`src/Bridge.cpp`) — translates `MeaningObject` ⇄
  `ProbabilityRequest` and `WeightVector` ⇄ `ScoringWeights` against
  the standalone `Elle.Service.Language` engine.
- **Apps** — `prob_heartbeat_demo`, `bridge_smoke_demo`,
  `prob_host_smoke`, `prob_proto_smoke`.
- **Tests** — `tests/test_*.cpp`, 43 doctest cases (run via ctest).

## Build (Linux container)

```bash
cd /app/ElleAnn/Services/Elle.Service.Probability
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure        # 43/43 PASS
./build/prob_heartbeat_demo
./build/bridge_smoke_demo                          # if bridge enabled
```

## Build (Windows MSVC)

```bat
cd ElleAnn\Services\Elle.Service.Probability
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Release\prob_heartbeat_demo.exe
```

The service is also built into `ElleAnn.sln` via its `.vcxproj` so a
full MSBuild produces `Elle.Service.Probability.exe` alongside every
other service.

## Status (Feb 2026)

| Surface                | State                                                          |
|------------------------|----------------------------------------------------------------|
| Library                | `elle_probability_core` builds clean `-Wall -Wextra -Wpedantic -Wshadow` |
| ctest                  | **43/43 PASS**                                                 |
| Smoke (`prob_heartbeat_demo`) | 5 scenarios green, 26 beliefs stored by end-of-demo     |
| Bridge                 | Live. `bridge_smoke_demo` runs A → B → C → D end-to-end        |
| IPC integration        | Wired. Cognitive sends `IPC_PROB_ANALYZE` per turn             |
| MSBuild integration    | In `ElleAnn.sln`. Builds alongside every other service         |

## Bridge build knobs

| Knob                                  | Default                      | Meaning                                                |
|---------------------------------------|------------------------------|--------------------------------------------------------|
| `ELLE_PROB_WITH_LANGUAGE_ENGINE`      | `AUTO`                       | `ON` = require bridge; `OFF` = skip; `AUTO` = enable if Language headers found |
| `ELLE_LANGUAGE_DIR`                   | `../Elle.Service.Language`   | Path to the Language engine source tree                |
| compile def `ELLE_PROB_HAS_LANGUAGE_BRIDGE=1` | set when bridge active| Consumers can `#ifdef` against it                      |

## What the bridge provides

- **Integration Point A** (weight translation):
  `WeightVector ⇄ elle::ScoringWeights`. Bijective; round-trip tested.
- **Integration Point B** (request construction):
  `(MeaningObject, ConversationContext) → ProbabilityRequest`. Carries
  units, sense candidates, phrase candidates, context frame scores,
  emotional profile (by `EmotionID`), conversation hints, punctuation
  signals.
- **Engine surface** — thin wrappers: `analyze`, `feedback`,
  `recordTrust`, `injectHormonalState`, plus mutable/const access to
  the underlying `ProbabilityEngine`.

## Run the end-to-end smoke

```bash
cd /app/ElleAnn/Services/Elle.Service.Probability
cmake --build build
./build/bridge_smoke_demo
```

Outputs 4 stages: weight round-trip → request construction → cold
analyze → re-analyze after feedback + trust + hormonal injection.

## Future passes

- Per-sense frequency priors from the Language engine's SQL
  (currently uniform).
- Multi-dimensional weight uncertainty (current is two-bucket).
- SQL persistence layer (mirror Language engine's
  `persistAnalysisTrace`) for belief continuity across service restarts.
