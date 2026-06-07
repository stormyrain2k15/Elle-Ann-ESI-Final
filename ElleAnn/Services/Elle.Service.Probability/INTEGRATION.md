# Elle Probability Engine — Integrated into Elle-Ann

The `elle-probability-engine` Bayesian reasoning substrate ships as a
**self-contained C++17 static library** under this directory. It is the
upstream of the future "live weights" replacement for the language
engine's static `ScoringWeights` struct.

## Why it lives here

This engine is a peer to the existing Elle Windows-service mesh but
does **not** depend on it. It compiles standalone via CMake (FetchContent
pulls `nlohmann/json` and `doctest`). Putting it under `ElleAnn/Engines/`
keeps it next to the rest of the codebase so a single solution build
will pick it up, but it does **not** wire into `ElleAnn.sln` yet — the
README's "Integration Point A/B" requires `Bridge.cpp` to be written
against the language engine headers (see `include/elle/prob/Bridge.hpp`),
and that's the next pass.

## Build (Linux, container)

```bash
cd /app/ElleAnn/Engines/elle-probability
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/prob_heartbeat_demo
```

## Build (Windows, your local box)

```bat
cd ElleAnn\Engines\elle-probability
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\Release\prob_heartbeat_demo.exe
ctest --test-dir build -C Release --output-on-failure
```

## Status (2026-02-07)

- **Library**: `elle_probability_core` builds clean with `-Wall -Wextra
  -Wpedantic -Wshadow`. 0 warnings, 0 errors.
- **Tests**: **43/43 PASS** via doctest + ctest (BeliefStore, Bayesian
  updater, sense resolver, intent analyzer, emotional posterior, speaker
  trust, engine integration).
- **Smoke**: `prob_heartbeat_demo` runs 5 scenarios cleanly (neutral,
  withdrawn, hostile, comforted, neutral-followup). 26 beliefs stored
  by end-of-demo. Exit 0.

## Fixes applied during integration

The drop arrived with a handful of compile/link issues that I patched
on the way in. See `/app/memory/CHANGELOG.md` for the verbose log.

| File | Issue | Fix |
|---|---|---|
| `src/Types.cpp` | `mass.rbegin()` on `std::unordered_map` — unordered maps have no reverse iterator. | Track `lastKey` in the loop and return it on the floating-point edge case. |
| `include/elle/prob/SpeakerTrustModel.hpp` | `m_domain` declared before `m_speakerId` but initialiser list initialises `m_speakerId` first → `-Wreorder`. | Swapped declaration order so it matches the initialiser. |
| `src/SenseProbabilityResolver.cpp` | `emotionalAlignmentScore(int64_t candidateId, ...)` unused param → `-Wunused-parameter`. | Commented out the param name. |
| `tests/test_engine_integration.cpp:212` | `CHECK(a || b)` rejected by doctest's expression decomposer. | Bound the `||` expression into a `const bool` and `CHECK`ed the bool. |
| `tests/test_engine_integration.cpp` | Used `std::set` without including `<set>`. | Added `#include <set>`. |
| `tests/test_engine_integration.cpp` | 4 calls to `engine.analyze(...)` ignoring the `[[nodiscard]]` result. | Cast to `(void)`. |
| `src/IntentAnalyzer.cpp` | Syntax prior for QUESTION (4× base, ≈ 0.211) was diluted below 0.2 by neutral-trust likelihoods, breaking `test_intent_analyzer "question syntax boosts QUESTION act"`. | Strengthened syntax prior to 6× base for QUESTION and 3× for CONFIRM so the syntax signal survives downstream dilution. |

## What's still TODO upstream (per README.md)

- ~~`Bridge.cpp` — full impl of `fromMeaningObject()`~~ **DONE 2026-02-07**:
  Bridge.cpp shipped, `bridge_smoke_demo` runs A+B+C+D end-to-end,
  9 new doctest cases (52/52 total). See "Bridge Integration" section
  below.
- Per-sense frequency priors from the language engine's SQL.
- Multi-dimensional weight uncertainty (current is two-bucket).
- SQL persistence layer (mirror language engine's `persistAnalysisTrace`).

## Bridge Integration — language engine ⇄ probability engine

The bridge between the two engines is now **live**. To build with the
bridge enabled, the probability engine's CMakeLists auto-detects a
sibling `elle-language/` directory and adds `src/Bridge.cpp` +
`apps/bridge_smoke_demo.cpp` + `tests/test_bridge.cpp` to the build.

| Knob | Default | Meaning |
|---|---|---|
| `ELLE_PROB_WITH_LANGUAGE_ENGINE` | `AUTO` | `ON` = require bridge; `OFF` = skip; `AUTO` = enable if headers found |
| `ELLE_LANGUAGE_DIR` | `../elle-language` | Path to the language-engine source tree |
| compile def `ELLE_PROB_HAS_LANGUAGE_BRIDGE=1` | set when bridge active | Consumers can `#ifdef` against it |

### What the bridge provides

- **Integration Point A** (weight translation):
  `WeightVector ⇄ elle::ScoringWeights`. Bijective; round-trip tested.
- **Integration Point B** (request construction):
  `(MeaningObject, ConversationContext) → ProbabilityRequest`.
  Carries all units, sense candidates, phrase candidates, context
  frame scores, emotional profile (by EmotionID), conversation hints,
  punctuation signals.
- **Engine surface**: thin wrappers — `analyze`, `feedback`,
  `recordTrust`, `injectHormonalState`, plus mutable/const access to
  the underlying `ProbabilityEngine`.

### Run the end-to-end smoke

```bash
cd /app/ElleAnn/Engines/elle-probability
cmake --build build
./build/bridge_smoke_demo
```

Outputs 4 stages: weight round-trip → request construction → cold
analyze → re-analyze after feedback + trust + hormonal injection.

## What's NOT integrated yet (deliberate)

- **Not added to `ElleAnn.sln`** — would force MSBuild over CMake.
  Recommend leaving as a CMake-built static lib that the future
  `Elle.Service.Probability` or `Elle.Service.Cognitive` consumes via
  linker reference, or keep CMake build path explicit.
- **No IPC wiring** — no `SVC_PROBABILITY` service id, no
  `IPC_PROBABILITY_*` message types. Add when the Bridge lands so the
  IPC surface tracks the actual API.
