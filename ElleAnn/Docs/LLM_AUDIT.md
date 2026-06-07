# LLM / Tensor Audit — what still needs replacing

Goal: Elle's mesh contains **zero** transformer / token / tensor primitives.
The Language Engine + Probability Engine + MindManager + Imagination Engine
already cover the *analytical* side. The *generative* side (turning structured
state into Elle's English voice) is the last surface that still touches
`ElleLLMEngine`. This file lists every remaining call so the user can design a
deterministic replacement service.

## Centre of gravity

A single helper, `_Shared/ElleLLM.h::ElleLLMEngine`, exposes the methods every
service calls. Replacing this one class removes the transformer dependency
mesh-wide.

## Surviving methods on `ElleLLMEngine`

| # | Method                   | Purpose                                                      | Status                            |
| - | ------------------------ | ------------------------------------------------------------ | --------------------------------- |
| 1 | `Chat(history)`          | Produce Elle's next utterance from a conversation prefix.    | **Replace** (generative)          |
| 2 | `StreamChat(...)`        | Streaming version of (1).                                    | **Replace** (generative)          |
| 3 | `Ask(prompt, sysPrompt)` | One-shot inner monologue / introspection.                    | **Replace** (generative)          |
| 4 | `ElleChat(...)`          | Wrapper that pre-bakes emotion + memory + goal into (1).     | **Replace** (generative)          |
| 5 | `AnalyzeSentiment(text)` | Valence / arousal / dominant emotion.                        | **DELETE** — Probability Engine    |
| 6 | `ParseIntent(text, ctx)` | Structured intent extraction.                                | **DELETE** — Probability Engine    |
| 7 | `GenerateCreative(...)`  | Creative riff on a theme.                                    | **DELETE** — no callers           |
| 8 | `SelfReflect(...)`       | Introspective response over recent state.                    | **Replace** (generative)          |
| 9 | `EthicalEvaluate(...)`   | Score a proposed action ethically.                           | **DELETE** — MindManager          |
| 10| `FormGoal(...)`          | Pick a goal sentence given drives + emotions.                | **Replace** (generative)          |
| 11| `DreamNarrate(...)`      | Weave memory fragments into a dream paragraph.               | **DELETE** — Imagination Engine    |

So we have **5 actually-generative call families** (`Chat`, `StreamChat`,
`Ask`, `SelfReflect`, `FormGoal`) and 6 calls that can simply be deleted.

## Call sites still using LLM (as of 2026-02)

| Service                | File / line                              | Method               | Replacement plan                                            |
| ---------------------- | ----------------------------------------- | -------------------- | ------------------------------------------------------------ |
| Cognitive              | `CognitiveEngine.cpp:120`                 | `ParseIntent`        | DELETE — already covered by Probability `analyze`            |
| Cognitive              | `CognitiveEngine.cpp:630`                 | `Chat`               | REPLACE — composer service                                   |
| Cognitive              | `CognitiveEngine.cpp:1348`                | `Chat` (main)        | REPLACE — composer service                                   |
| HTTPServer             | `HTTPServer.cpp:2511`                     | `Chat`               | REPLACE — composer service (streaming endpoint)              |
| HTTPServer             | `HTTPServer.cpp:3826`                     | `Chat`               | REPLACE — composer service                                   |
| Memory                 | `MemoryEngine.cpp:448`                    | `Ask` (consolidate)  | REPLACE — composer service (kind=NARRATE_CONSOLIDATION)      |
| Continuity             | `Continuity.cpp:185`                      | `Ask` (greeting)     | REPLACE — composer service (kind=DAILY_AFFIRMATION)          |
| Continuity             | `Continuity.cpp:240`                      | `SelfReflect`        | REPLACE — composer service (kind=NIGHTLY_REFLECTION)         |
| InnerLife              | `InnerLife.cpp:275`                       | `Ask` (private)      | REPLACE — composer service (kind=PRIVATE_THOUGHT)            |
| Bonding                | `Bonding.cpp:171`                         | `Ask` (reunion)      | REPLACE — composer service (kind=REUNION)                    |
| Solitude               | `Solitude.cpp:157`                        | `Ask` (reflect)      | REPLACE — composer service (kind=SOLITUDE_REFLECT)           |
| Solitude               | `Solitude.cpp:245`                        | `Ask` (process)      | REPLACE — composer service (kind=SOLITUDE_PROCESS)           |
| GoalEngine             | `GoalEngine.cpp:156`                      | `FormGoal`           | REPLACE — composer service + Imagination for candidate seed  |
| IdentityCore (shared)  | `ElleIdentityCore.cpp:392`                | `Ask` (contemplate)  | REPLACE — composer service                                   |
| IdentityCore (shared)  | `ElleIdentityCore.cpp:662`                | `Ask` (remembered)   | REPLACE — composer service                                   |
| SelfSurprise (shared)  | `ElleSelfSurprise.cpp:19, 36, 177, 180, 186` | `Ask` (×5)        | REPLACE — composer service                                   |
| SelfPrompt             | `SelfPrompt.cpp:145`                      | `Chat`               | REPLACE — composer service                                   |
| Dream                  | `Dream.cpp:63`                            | `DreamNarrate`       | DELETE — already routes through Imagination service          |
| Imagination            | `Imagination.cpp:331`                     | `Chat` (refinement)  | REPLACE OR DISABLE — config flag `imagination.use_llm_refinement=false` already disables it; final replacement is composer service |

## What the Composer service has to compute (no tokens, no tensors)

Each `IPC_COMPOSE_REQUEST` carries a `kind` and a structured payload. The
composer's job is to convert that into Elle's English in 5 deterministic
steps:

1. **Sentence-plan selection** — pick a `PragmaticAct` from the already-
   computed `intentDistribution` (Probability Engine output).
2. **Frame selection** — pick a frame template keyed by `(kind, act, POS pattern)`
   from a `composer_frame` SQL table.
3. **Slot filling** — for each `[SLOT:POS+constraint]` in the frame, walk the
   Language Engine's semantic graph for candidate lemmas, then score them
   with the Probability Engine's existing `WeightVector`. Pick the winner.
4. **Morphological inflection** — verb conjugation / pluralization /
   contraction via lookup tables (English).
5. **Surface stitching** — paste lemmas in slot order, fix capitalisation,
   apply punctuation per the chosen `PragmaticAct` (`QUESTION` → "?", etc.).

## Interface the Composer service needs to expose

```text
SVC_COMPOSER (new ELLE_SERVICE_ID)

IPC types:
  IPC_COMPOSE_REQUEST       — { request_id, kind, payload, identity, emotion,
                                memory_ctx, conscience_ctx, prob_ctx }
  IPC_COMPOSE_RESPONSE      — { request_id, text, act, frame_id,
                                slots:[...], confidence }
  IPC_COMPOSE_STREAM_CHUNK  — { request_id, clause_index, text, final }

kind ∈ {
  CONVERSE,            // (A) — replaces Chat/ElleChat (main reply path)
  ASK_INNER,           // (B) — replaces Ask used for inner monologue
  SELF_REFLECT,        // (E) — replaces SelfReflect
  FORM_GOAL,           // (F) — replaces FormGoal
  REWRITE_SCENARIO     // (G) — replaces Imagination's LLM refinement
}
```

SQL tables the Composer service needs:

- `composer_frame(frame_id, kind, act, pos_pattern, template, weight)`
- `composer_inflection(lemma, form, inflected)`
- `composer_log(id, request_id, kind, frame_id, slots_json, text, scored, recorded_ms)`

## What I will do once the user ships `Elle.Service.Composer`

1. Add `SVC_COMPOSER` to `ELLE_SERVICE_ID` (count 24 → 25).
2. Add the three IPC message types and register `"Composer"` in `g_serviceNames`.
3. Route every cell in the "Call sites still using LLM" table above through
   `IPC_COMPOSE_REQUEST` + a `ComposerCorrelator` (mirroring `ProbCorrelator`,
   `MindCorrelator`).
4. Delete `_Shared/ElleLLM.h` + `ElleLLM.cpp`, the WinHTTP plumbing, the
   `LLMAPIProvider`/`LLMLocalProvider` classes, and remove the `llama.cpp`
   dependency from `Directory.Build.props`.
5. Drop the 6 to-be-deleted call families: `AnalyzeSentiment`, `ParseIntent`,
   `GenerateCreative`, `EthicalEvaluate`, `DreamNarrate`, and any wrapper code.
6. Update PRD + CHANGELOG.
7. Final pass on **IPC serialization** and **SQL cursor lifecycles** as the
   user requested — done last, mesh-wide, so all envelope shapes match
   (length-prefixed JSON via `SetStringPayload`) and every cursor uses the
   same `ElleSQLPool::QueryParams` / `Exec` lifecycle pattern with explicit
   `commit()` / `rollback()` placement.
