# Elle.Service.Probability — Windows Service Wrapper

The Probability service exposes the integrated **Bayesian Probability Engine + Language Engine**
over Elle's standard IOCP named-pipe IPC. It is the deterministic, integer-math replacement for
LLM/Transformer-based intent and sense inference inside the Elle service mesh.

## Identity

| Field                | Value                                       |
| -------------------- | ------------------------------------------- |
| `ELLE_SERVICE_ID`    | `SVC_PROBABILITY` (= 21)                    |
| Service name         | `ElleProbability`                           |
| Display name         | `Elle-Ann Probability Engine`               |
| Named pipe           | `\\.\pipe\ElleAnn_Probability` (configurable prefix) |
| Source layout        | `/app/ElleAnn/Services/Elle.Service.Probability/` |

## Architecture

```
+--------------------+        IOCP named pipe         +----------------------+
| Elle.Service.*     |  <---------------------------> | Elle.Service.Probability |
|   (Cognitive,      |       IPC_PROB_* / RESP        |                      |
|    XChromosome,    |                                |  ProbabilityHost     |
|    HTTPServer ...) |                                |  ├── Language Engine |
+--------------------+                                |  └── Bridge → ProbEng|
                                                      +----------------------+
```

- `service/ProbabilityHost.{h,cpp}` — thread-safe lifecycle owner of the Language `Engine`
  and the `Bridge` around `ProbabilityEngine`. Auto-loads on `start()` or rebuilds via
  `reload()`.
- `service/ProbabilityProto.{h,cpp}` — pure JSON ↔ engine-type marshalling helpers.
  Cross-platform; testable on Linux.
- `service/ProbabilityService.cpp` — the `ElleServiceBase` skeleton + IPC dispatch.

## IPC envelope

All messages use length-prefixed JSON via `SetStringPayload` (matching the existing
`Elle.Service.XChromosome` and `Elle.Service.WorldModel` pattern). Every response is sent
back as `IPC_PROB_RESPONSE` with the original `correlation_id` and a `request_id` field
in the body for in-flight matching.

### Requests / Responses

| Request type                  | Body shape                                                                   | Response body                                                |
| ----------------------------- | ----------------------------------------------------------------------------- | ------------------------------------------------------------ |
| `IPC_PROB_ANALYZE`            | `{ request_id, text, speaker_id, convo:{speakerRelationship, activeContextHints, recentWordIds, prefersBaseball} }` | `{ request_id, success, error, result, likely_intent, overall_confidence, unresolved_words }` |
| `IPC_PROB_SCORE`              | `{ request_id, speaker_id, request:{units, contextHints, emotionalProfile, speakerRelationship, ...} }` | `{ request_id, success, error, result }`                     |
| `IPC_PROB_FEEDBACK`           | `{ request_id, unit_index, confirmed_sense_id, is_phrase, confidence, speaker_id }` | `{ request_id, success }`                                    |
| `IPC_PROB_TRUST`              | `{ request_id, speaker_id, signal, strength }`                                | `{ request_id, success }`                                    |
| `IPC_PROB_INJECT_HORMONAL`    | `{ request_id, state:{ "<emotionId>": <weight>, ... } }`                      | `{ request_id, success, applied }`                           |
| `IPC_PROB_RELOAD`             | `{ request_id }`                                                              | `{ request_id, success, ready }`                             |
| `IPC_PROB_QUERY_WEIGHTS`      | `{ request_id }`                                                              | `{ request_id, success, weights }`                           |
| `IPC_PROB_SEED_WEIGHTS`       | `{ request_id, weights:{...} }`                                               | `{ request_id, success }`                                    |
| `IPC_PROB_RESET`              | `{ request_id, scope:"turn"|"all" }`                                          | `{ request_id, success, scope }`                             |

### Result shape (`result` field)

```json
{
  "units": [
    {
      "unitIndex": 0,
      "winningSenseId": 2002,
      "isPhraseSense": false,
      "winnerProbability": 0.42,
      "posteriorEntropy":  1.13,
      "rankedCandidates":  [ { "hypothesisId": 2001, "probability": 0.31, ... }, ... ]
    }
  ],
  "recommendedWeights": { "contextFrameMatch": 0.99, ... },
  "intentDistribution": { "0": 0.41, "1": 0.32, ... },
  "likelyAct": "ASSERT|QUESTION|REQUEST|...",
  "emotionalPosterior": { "4": 0.62, "12": 0.41, ... },
  "speakerTrust": 0.50,
  "overallConfidence": 0.25
}
```

### TrustSignal strings (case-sensitive)

`CONFIRMED_ACCURATE`, `KEPT_PROMISE`, `CONSISTENT_WITH_HISTORY`, `CORRECTION_NEEDED`,
`CONTRADICTED`, `HOSTILE_FRAMING`, `IDENTITY_CONFIRMED`.

## Config keys (`/app/ElleAnn/config`)

| Key                                       | Default | Meaning                                                            |
| ----------------------------------------- | ------- | ------------------------------------------------------------------ |
| `probability.language_config`             | `""`    | Path to language `EngineConfig` JSON; empty → built-in defaults    |
| `probability.engine_config`               | `""`    | Path to probability `ProbabilityEngineConfig` JSON; empty → defaults |
| `probability.auto_load_on_start`          | `true`  | If false, the service starts but waits for an explicit `IPC_PROB_RELOAD` |
| `probability.use_in_memory_language`      | `false` | Use the in-memory access layer instead of SQL Server (test mode)   |
| `cognitive.probability_timeout_ms`        | `300`   | How long Cognitive waits for a probabilistic read before degrading |

## Lifecycle

- `start()` → builds Language `Engine` (SQL Server access via ODBC by default), builds
  Probability `Bridge`, and seeds `WeightVector` from the Language `ScoringWeights`.
- `reload()` → tears down the entire pipeline and rebuilds it. Triggered by
  `IPC_PROB_RELOAD` or by a missing-config recovery path.
- `stop()` → releases the language DB, the bridge, the engine.

The host serializes all calls with a single mutex; concurrent IPC requests are
fine because each call only touches the engines for a bounded amount of work.

## Wiring into the rest of the mesh

- **Cognitive (`Elle.Service.Cognitive`)** — `HandleChatRequest` now fires
  `IPC_PROB_ANALYZE` right after `QuickSentiment`, waits up to
  `cognitive.probability_timeout_ms` (default 300 ms) for the correlated
  `IPC_PROB_RESPONSE`, then injects a `Probabilistic read of the user's message`
  block into the LLM system prompt. On successful completion of the LLM reply,
  Cognitive fires `IPC_PROB_TRUST` with `CONSISTENT_WITH_HISTORY` (strength 0.5).
  Probability is also returned in the chat response under `probabilistic_read`.

- **XChromosome (`Elle.Service.XChromosome`)** — `BroadcastHormoneUpdate()` now
  also sends `IPC_PROB_INJECT_HORMONAL` to `SVC_PROBABILITY` with a deterministic
  mapping from hormone levels (estrogen, progesterone, oxytocin, cortisol,
  dopamine, serotonin) onto emotion IDs (joy=5, oxytocin → trust=6 / tenderness=7,
  serotonin → comfort=8, dopamine → curiosity=10 / pos_draw=11, cortisol →
  anger/fear/sadness/neg_draw).

Both services now list `SVC_PROBABILITY` in their `GetDependencies()` so the
reconnector keeps the pipe alive.

## Tests (Linux container, CMake)

```bash
cd /app/ElleAnn/Services/Elle.Service.Probability
mkdir -p build && cd build
cmake .. -DELLE_PROB_WITH_LANGUAGE_ENGINE=ON
cmake --build . -j4
ctest --output-on-failure          # 52/52 engine tests
./prob_host_smoke                   # End-to-end host lifecycle smoke
./prob_proto_smoke                  # IPC envelope JSON round-trip + lifecycle (34 checks)
./bridge_smoke_demo                 # Original bridge smoke
./prob_heartbeat_demo               # Original probability heartbeat demo
```
