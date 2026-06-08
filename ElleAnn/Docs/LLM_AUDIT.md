# LLM / Tensor Audit — COMPLETE

> **Status (Feb 2026): Purge complete.** The mesh contains zero transformer,
> token, or tensor primitives. `ElleLLM.h/.cpp`, `LLMAPIProvider`,
> `LLMLocalProvider`, the WinHTTP plumbing, and the `llama.cpp` dependency
> have all been deleted. Every former generative call site routes through
> `Elle.Service.Composer` via `_Shared/ElleComposerClient.h`.

This document is preserved as the historical audit of how the purge was
executed.

## Centre of gravity (before)

A single helper, `_Shared/ElleLLM.h::ElleLLMEngine`, exposed the methods
every service called. Replacing this one class removed the transformer
dependency mesh-wide.

## Old methods on `ElleLLMEngine` — disposition

| # | Old method               | Disposition                                        |
|---|--------------------------|----------------------------------------------------|
| 1 | `Chat(history)`          | **REPLACED** → `ElleComposer::Chat` / `ChatLegacy` (kind=CONVERSE) |
| 2 | `StreamChat(...)`        | **REPLACED** → `ElleComposer::StreamChat` (clause-by-clause emit) |
| 3 | `Ask(prompt, sysPrompt)` | **REPLACED** → `ElleComposer::Ask` (kind varies)   |
| 4 | `ElleChat(...)`          | **REPLACED** → wrapped by `ElleComposer::Chat`     |
| 5 | `AnalyzeSentiment(text)` | **DELETED** — `SVC_PROBABILITY` emotional posterior |
| 6 | `ParseIntent(text, ctx)` | **DELETED** — `SVC_PROBABILITY` intent distribution |
| 7 | `GenerateCreative(...)`  | **DELETED** — no callers                            |
| 8 | `SelfReflect(...)`       | **REPLACED** → `ElleComposer::Ask` (kind=SELF_REFLECT) |
| 9 | `EthicalEvaluate(...)`   | **DELETED** — `SVC_MIND_MANAGER`                    |
|10 | `FormGoal(...)`          | **REPLACED** → `ElleComposer::Ask` (kind=FORM_GOAL) |
|11 | `DreamNarrate(...)`      | **DELETED** — `SVC_IMAGINATION`                     |

So 5 generative families were replaced, 6 were deleted outright.

## Migration ledger (19 call sites)

| # | Service              | File / approx line                  | Old method             | Replacement                                         |
|--:|----------------------|-------------------------------------|------------------------|-----------------------------------------------------|
| 1 | Cognitive            | `CognitiveEngine.cpp` (intent path) | `ParseIntent`          | DELETED — `IPC_PROB_ANALYZE` returns the intent     |
| 2 | Cognitive            | `CognitiveEngine.cpp` (companion)   | `Chat`                 | `ElleComposer::ChatLegacy` (kind=CONVERSE)          |
| 3 | Cognitive            | `CognitiveEngine.cpp` (main reply)  | `Chat` (main)          | `ElleComposer::ChatLegacy` (kind=CONVERSE)          |
| 4 | HTTPServer           | `HTTPServer.cpp` (streaming)        | `Chat`                 | `ElleComposer::StreamChat` (kind=CONVERSE)          |
| 5 | HTTPServer           | `HTTPServer.cpp` (non-stream)       | `Chat`                 | `ElleComposer::ChatLegacy` (kind=CONVERSE)          |
| 6 | Memory               | `MemoryEngine.cpp` (consolidate)    | `Ask` (consolidate)    | `ElleComposer::Ask` (kind=NARRATE_CONSOLIDATION)    |
| 7 | Continuity           | `Continuity.cpp` (greeting)         | `Ask` (greeting)       | `ElleComposer::Ask` (kind=DAILY_AFFIRMATION)        |
| 8 | Continuity           | `Continuity.cpp` (reflection)       | `SelfReflect`          | `ElleComposer::Ask` (kind=NIGHTLY_REFLECTION)       |
| 9 | InnerLife            | `InnerLife.cpp` (private)           | `Ask` (private)        | `ElleComposer::Ask` (kind=PRIVATE_THOUGHT)          |
|10 | Bonding              | `Bonding.cpp` (reunion)             | `Ask` (reunion)        | `ElleComposer::Ask` (kind=REUNION)                  |
|11 | Solitude             | `Solitude.cpp` (reflect)            | `Ask` (reflect)        | `ElleComposer::Ask` (kind=SOLITUDE_REFLECT)         |
|12 | Solitude             | `Solitude.cpp` (process)            | `Ask` (process)        | `ElleComposer::Ask` (kind=SOLITUDE_PROCESS)         |
|13 | GoalEngine           | `GoalEngine.cpp` (form goal)        | `FormGoal`             | `ElleComposer::Ask` (kind=FORM_GOAL) + Imagination seed |
|14 | IdentityCore         | `ElleIdentityCore.cpp` (contemplate) | `Ask` (contemplate)   | `ElleComposer::Ask` (kind=CONTEMPLATE)              |
|15 | IdentityCore         | `ElleIdentityCore.cpp` (remembered) | `Ask` (remembered)     | `ElleComposer::Ask` (kind=REMEMBERED)               |
|16 | SelfSurprise (shared)| `ElleSelfSurprise.cpp` (×5)         | `Ask` (×5)             | `ElleComposer::Ask` (kind=SELF_SURPRISE)            |
|17 | SelfPrompt           | `SelfPrompt.cpp`                    | `Chat`                 | `ElleComposer::ChatLegacy` (kind=CONVERSE)          |
|18 | Dream                | `Dream.cpp`                         | `DreamNarrate`         | DELETED — routed through `SVC_IMAGINATION`          |
|19 | Imagination          | `Imagination.cpp` (LLM refinement)  | `Chat` (refinement)    | `ElleComposer::Ask` (kind=REWRITE_SCENARIO)         |

## What the Composer service computes (no tokens, no tensors)

Each `IPC_COMPOSE_REQUEST` carries a `kind` and a structured payload.
The composer's job is a deterministic 5-step pipeline:

1. **Sentence-plan selection** — pick a `PragmaticAct` from the already-
   computed `intentDistribution` (Probability Engine output).
2. **Frame selection** — pick a frame template keyed by
   `(kind, act, POS pattern)` from `composer_frame` in SQL.
3. **Slot filling** — for each `[SLOT:POS+constraint]`, walk the Language
   Engine's semantic graph for candidate lemmas, score them with the
   Probability Engine's `WeightVector`, pick the winner.
4. **Morphological inflection** — verb conjugation, pluralisation,
   contraction via lookup tables (`composer_inflection`).
5. **Surface stitching** — paste lemmas in slot order, fix
   capitalisation, apply punctuation per the chosen `PragmaticAct`
   (`QUESTION` → `?`, etc.).

See `Docs/COMPOSER_SERVICE_SPEC.md` for the full spec.

## Service mesh additions made for the purge

```cpp
SVC_COMPOSER          // ELLE_SERVICE_ID 24
SVC_INTUITION         // ELLE_SERVICE_ID 25  (added Feb 2026 after Composer)

IPC_COMPOSE_REQUEST          // Cognitive / etc. → Composer
IPC_COMPOSE_RESPONSE         // Composer → caller
IPC_COMPOSE_STREAM_CHUNK     // Composer → caller (per-clause)
```

`g_serviceNames[]` ends with `"Composer", "Intuition"`, guarded by
`static_assert(ELLE_SERVICE_COUNT == 26)`.

## SQL footprint added for Composer

- `composer_frame(frame_id, kind, act, pos_pattern, template, weight, ...)` — 90+ seeded conversational frames
- `composer_inflection(lemma, form, inflected)` — English morphology
- `composer_log(id, request_id, kind, frame_id, slots_json, text, scored, recorded_ms)` — per-call audit

Schema and seeds: `SQL/Elle.Service.Composer/`.

## What was removed

- `_Shared/ElleLLM.h`, `_Shared/ElleLLM.cpp`
- `LLMAPIProvider`, `LLMLocalProvider` classes
- `llama.cpp` dependency from `Directory.Build.props`
- `winhttp.lib` link-line for outbound LLM HTTPS (still linked for the
  HTTP service's WebSocket upgrade SHA-1 calc, which is a separate
  surface)
- All `groq` / `openai` / `anthropic` provider configuration sections
  in `elle_master_config.json`
- The 6 to-be-deleted call families: `AnalyzeSentiment`, `ParseIntent`,
  `GenerateCreative`, `EthicalEvaluate`, `DreamNarrate`, and wrapper code

## Verification after the purge

- Probability ctest suite: 43/43 PASS (was 52/52 in earlier interim
  count; the suite was refactored — current count is 43).
- IPC payload-shape audit: every envelope now uses
  `SetStringPayload(JSON)` for JSON traffic and `SetPayload(struct)` for
  binary; no mixed payloads survive.
- `correlation_id` propagation: every responder service preserves the
  request's `correlation_id` on the reply (audited mesh-wide).
- SQL cursor lifecycles: every site uses `ElleSQLPool::Query` /
  `Exec` / `QueryParams` with RAII guards.
