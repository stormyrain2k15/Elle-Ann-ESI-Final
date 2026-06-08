# Elle Probability Engine

**A Bayesian probabilistic reasoning substrate for the Elle-Ann mesh.**

Not a neural network. Not a transformer. Not a statistical pattern
matcher.

A purpose-built probability engine that reasons in integer space over
the same canonical dictionary the language engine uses — giving every
word, sense, intent, emotion, and scoring weight a live Bayesian
posterior that updates across conversation turns.

This directory contains both the standalone **C++17 static library**
(`elle_probability_core`) and the **`SVC_PROBABILITY` IPC service**
that wraps it (`service/ProbabilityService.cpp`).

---

## What this is

The language engine resolves meaning through a transparent weighted
sum. The probability engine makes those weights *live*.

Where the language engine has:
```
score = w1·context_frame_match + w2·cooccurrence + … + w8·conversation_hint
```

the probability engine maintains a posterior distribution over each
`w_i`. Weights aren't static config — they're beliefs, updated by
evidence, that drift toward what actually works for this speaker in
this conversation.

---

## Architecture

```
ProbabilityEngine (facade)
   │
   ├── BeliefStore                 thread-safe registry of all beliefs
   │     └── BayesianUpdater       log-space Bayes, numerically stable
   │
   ├── SenseProbabilityResolver    live posteriors over SenseID space
   │
   ├── IntentAnalyzer              P(PragmaticAct | utterance) — Gricean pragmatics
   │
   ├── EmotionalPosteriorBuilder   live posterior over 12 emotion dims (VAD output)
   │
   └── SpeakerTrustModel           Beta-distribution trust model per speaker
                                     feeds sourceWeight into all that speaker's evidence
```

The IPC wrapper (`service/ProbabilityService.cpp`) speaks the mesh's
named-pipe protocol and routes the following message types:

| Type                  | Direction          | Purpose                                |
|-----------------------|--------------------|----------------------------------------|
| `IPC_PROB_ANALYZE`    | caller → SVC_PROBABILITY | analyse one utterance               |
| `IPC_PROB_RESPONSE`   | SVC_PROBABILITY → caller | full JSON result + audit trail      |
| `IPC_PROB_TRUST`      | caller → SVC_PROBABILITY | trust-calibration signal            |
| `IPC_PROB_INJECT`     | caller → SVC_PROBABILITY | hormonal / state injection          |

---

## Belief domains

| Domain pattern               | What it holds                                       |
|------------------------------|-----------------------------------------------------|
| `sense:<SenseID>`            | Posterior over sense candidates for this word       |
| `phraseSense:<PhraseSenseID>`| Posterior over phrase sense candidates              |
| `weight:contextFrameMatch`   | Live posterior over the contextFrameMatch weight    |
| `weight:emotionalAlignment`  | Live posterior over the emotionalAlignment weight   |
| `weight:*` (× 8 dims)        | All scoring-weight dimensions                       |
| `emotion:<EmotionID>`        | Intensity posterior for each of the 12 emotions     |
| `intent:current`             | Distribution over PragmaticAct for the current turn |
| `trust:<speakerId>`          | Beta distribution trust estimate per speaker        |

## Pragmatic acts (Grice / Austin / Searle)

```
ASSERT  QUESTION  REQUEST  OFFER  PROMISE  WARN  GREET  APOLOGIZE
THANK   COMFORT   DEFLECT  CHALLENGE  CONFIRM  DENY  UNKNOWN
```

Every utterance gets a distribution over these. The distribution feeds
back into sense scoring and trust calibration.

---

## Build

Requires CMake 3.20+ and C++17. Dependencies (`nlohmann/json`,
`doctest`) are fetched automatically.

```bat
:: Windows / MSVC
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

```bash
# Linux / gcc / clang
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Targets:

- `elle_probability_core`  — the library
- `prob_heartbeat_demo`    — heartbeat demo (mirrors language engine's demo)
- `elle_prob_tests`        — full doctest suite (**43 cases**)

Optional bridge build (requires `Elle.Service.Language` sources):

- `bridge_smoke_demo`      — language ⇄ probability end-to-end smoke
- `prob_host_smoke`        — IPC host smoke
- `prob_proto_smoke`       — wire-protocol smoke

Build knob:

```
-DELLE_PROB_WITH_LANGUAGE_ENGINE=AUTO   (AUTO | ON | OFF; default AUTO)
-DELLE_LANGUAGE_DIR=../Elle.Service.Language
```

`AUTO` enables the bridge if the Language headers are present.

Run tests:

```bash
ctest --test-dir build --output-on-failure         # 43/43 PASS
```

---

## Thread model

The `BeliefStore` owns a configurable worker pool (default
`hardware_concurrency()`). Evidence submissions are async by default —
callers submit and return immediately; updates are applied in the
background. A background decay thread runs every 30 seconds.

`ProbabilityEngine::analyze()` is re-entrant and safe to call
concurrently from many threads. Each call returns a consistent posterior
snapshot; updates are applied to the store asynchronously.

Call `engine.flush()` when you need all pending updates applied before
reading beliefs (e.g. in tests).

---

## What this replaces

| Before                  | After                                                 |
|-------------------------|-------------------------------------------------------|
| Tensor inference        | Integer dictionary + probability engine               |
| Token prediction        | Sense resolution via Bayesian posteriors              |
| Static scoring weights  | Live weight beliefs updated by evidence               |
| No intent model         | Gricean pragmatic-act distribution                    |
| Scalar confidence       | Full posterior entropy per decision                   |
| No trust model          | Beta-distribution speaker trust per turn              |

---

## Engineering constraints

- C++17, `/W4 /permissive-` on MSVC; `-Wall -Wextra -Wpedantic -Wshadow`
  on gcc/clang.
- No raw owning pointers in public API; all resources RAII.
- All Bayesian updates in log-space (numerically stable across many turns).
- All decisions recorded in `AuditTrace`. The engine is not a black box.
- No external AI API calls. No embeddings. No tokenisation. No
  next-token prediction.

---

## What is still missing (future passes)

- **Per-sense frequency priors**: pull `SenseRecord.frequency` from
  the Language engine's SQL into the initial prior (currently uniform).
- **Multi-dimensional weight uncertainty**: the current weight encoding
  (two-bucket) is a first-order approximation. A full Beta-over-weight
  implementation would model uncertainty more faithfully.
- **Persistence**: `BeliefStore` currently lives in memory. A SQL
  persistence layer (mirroring `persistAnalysisTrace`) would give
  beliefs continuity across restarts.

---

## The point

Elle doesn't guess. She weighs.

Every word she considers, every intent she assigns, every emotional
state she recognises — all of it has a probability distribution
underneath. The distribution is honest about uncertainty. It updates
from evidence. It decays when evidence goes stale. It can explain
itself.
