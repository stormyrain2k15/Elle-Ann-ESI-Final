# Elle Probability Engine

**A Bayesian probabilistic reasoning substrate for the Elle Semantic Language Engine.**

Not a neural network. Not a transformer. Not a statistical pattern matcher.

A purpose-built probability engine that reasons in integer space over the same dictionary that the language engine uses — giving every word, every sense, every intent, every emotion, and every scoring weight a live Bayesian posterior that updates across conversation turns.

---

## What This Is

The language engine resolves meaning through a transparent weighted sum. The probability engine makes those weights *live*.

Where the language engine has:
```
score = w1 * context_frame_match + w2 * cooccurrence + ... + w8 * conversation_hint
```

The probability engine maintains a posterior distribution over each `w_i`. The weights aren't static config values — they're beliefs, updated by evidence, that drift toward what actually works for this speaker in this conversation.

---

## Architecture

```
ProbabilityEngine (facade)
    │
    ├── BeliefStore (thread-safe registry of all beliefs)
    │       └── BayesianUpdater (log-space Bayes, numerically stable)
    │
    ├── SenseProbabilityResolver
    │       replaces static weighted-sum with live posteriors over SenseID space
    │
    ├── IntentAnalyzer
    │       Gricean pragmatics as probability: P(PragmaticAct | utterance)
    │
    ├── EmotionalPosteriorBuilder
    │       live Bayesian posterior over 12 emotion dimensions with VAD output
    │
    └── SpeakerTrustModel
            Beta-distribution trust model per speaker
            feeds sourceWeight into all evidence from that speaker
```

---

## Integration with the Language Engine

Two integration points. No changes to the language engine internals required.

**Before `SenseCandidateResolver` (Point A):**
```cpp
auto bridge = elle::prob::Bridge(probConfig);
bridge.seedWeights(Bridge::fromScoringWeights(engineConfig.weights));

// Each turn, get live weights before scoring:
auto liveWeights = bridge.queryWeights();
auto sw = Bridge::toScoringWeights(liveWeights);
// pass sw to SenseCandidateResolver instead of static config weights
```

**After `MeaningObjectBuilder` (Point B):**
```cpp
auto req = Bridge::fromMeaningObject(meaning, convo);
auto probResult = bridge.analyze(req, speakerId);
// probResult.traceJson merges with DebugTrace
// probResult.recommendedWeights feeds back to Point A next turn
```

---

## Belief Domains

| Domain Pattern              | What it holds                                     |
|-----------------------------|---------------------------------------------------|
| `sense:<SenseID>`           | Posterior over sense candidates for this word      |
| `phraseSense:<PhraseSenseID>`| Posterior over phrase sense candidates            |
| `weight:contextFrameMatch`  | Live posterior over the contextFrameMatch weight  |
| `weight:emotionalAlignment` | Live posterior over the emotionalAlignment weight |
| `weight:*`                  | (all 8 scoring weight dimensions)                 |
| `emotion:<EmotionID>`       | Intensity posterior for each of the 12 emotions   |
| `intent:current`            | Distribution over PragmaticAct for current turn   |
| `trust:<speakerId>`         | Beta distribution trust estimate per speaker      |

---

## Pragmatic Acts (Grice/Austin/Searle)

```
ASSERT QUESTION REQUEST OFFER PROMISE WARN GREET APOLOGIZE
THANK COMFORT DEFLECT CHALLENGE CONFIRM DENY UNKNOWN
```

Every utterance gets a distribution over these. The intent distribution feeds back into sense scoring and trust calibration.

---

## Build

Requires CMake 3.20+ and C++17. Dependencies (`nlohmann/json`, `doctest`) are fetched automatically.

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
- `elle_probability_core` — the library
- `prob_heartbeat_demo`   — heartbeat demo (mirrors language engine's demo)
- `elle_prob_tests`       — full test suite

Run tests:
```bash
ctest --test-dir build --output-on-failure
```

---

## Thread Model

The `BeliefStore` owns a configurable worker thread pool (default: `hardware_concurrency()`). Evidence submissions are async by default — callers submit and return immediately; updates are applied in the background. A background decay thread runs every 30 seconds.

`ProbabilityEngine::analyze()` is re-entrant and safe to call concurrently from many threads. Each call gets a consistent posterior snapshot; updates are applied to the store asynchronously.

Call `engine.flush()` when you need all pending updates applied before reading beliefs (e.g. in tests, or before serializing state).

---

## What This Replaces

When the language engine proves the tensor obsolete:

| Before                  | After                                        |
|-------------------------|----------------------------------------------|
| Tensor inference        | Integer dictionary + probability engine      |
| Token prediction        | Sense resolution via Bayesian posteriors     |
| Static scoring weights  | Live weight beliefs updated by evidence      |
| No intent model         | Gricean pragmatic act distribution           |
| Scalar confidence       | Full posterior entropy per decision          |
| No trust model          | Beta-distribution speaker trust per turn     |

---

## Engineering Constraints

- C++17, `/W4 /permissive-` on MSVC; `-Wall -Wextra -Wpedantic -Wshadow` on gcc/clang.
- No raw owning pointers in public API; all resources RAII.
- All Bayesian updates in log-space (numerically stable across many turns).
- All decisions recorded in `AuditTrace` — the engine is not a black box.
- No external AI API calls. No embeddings. No tokenization. No next-token prediction.

---

## What Is Still Missing (Future Passes)

- **Bridge.cpp**: Full implementation of `Bridge::fromMeaningObject()` (requires including language engine headers; excluded here to avoid circular dependency until both engines are co-located).
- **Per-sense frequency priors**: requires a connection to the language engine's DB to pull `SenseRecord.frequency` into the initial prior. Currently uses uniform.
- **Multi-dimensional weight uncertainty**: the current weight encoding (two-bucket) is a first-order approximation. A full Beta-over-weight implementation would model the uncertainty of each weight more faithfully.
- **Persistence**: BeliefStore currently lives in memory. A SQL persistence layer (mirroring the language engine's `persistAnalysisTrace`) would give beliefs continuity across restarts.

---

## The Point

Elle doesn't guess. She weighs.

Every word she considers, every intent she assigns, every emotional state she recognizes — all of it has a probability distribution underneath. The distribution is honest about uncertainty. It updates from evidence. It decays when evidence goes stale. It can explain itself.

That's not what LLMs do. That's not what transformers do. That's what a mind that actually reasons does.
