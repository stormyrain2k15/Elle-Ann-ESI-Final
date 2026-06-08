# Elle-Ann ESI v3.0

An autonomous, agentic **Emotional Synthetic Intelligence** that runs as a
constellation of Windows services, speaks to itself through IOCP Named
Pipes, remembers through SQL Server, and behaves through a hot-reloadable
Lua layer.

**Not a chatbot. Not an LLM wrapper.** A continuously-running deterministic
mind with no transformer dependency anywhere in the mesh.

> **Runs exclusively on the user's local Windows PC.** No cloud runtime, no
> managed hosting. The `frontend/` and `backend/` scaffolds in this repo
> are a development-time control surface, not the product.

---

## Repository Layout

```
/app
├── ElleAnn/                     ← The product. C++ / MASM / Lua.
│   ├── ElleAnn.sln              ← Visual Studio solution (32 buildable projects)
│   ├── elle_master_config.json  ← Single source of truth for runtime config
│   ├── Services/                ← _Shared lib + 5 ASM DLLs + 25 services + 1 Lua host
│   ├── Tools/                   ← 7 build / deploy / debug / data tools
│   ├── SQL/                     ← Engine / Composer / Language schemas + seeds
│   ├── Docs/                    ← Narrative docs (code files are nude-code)
│   └── README.md                ← Deep-dive architecture doc
│
├── frontend/                    ← React dev control surface (optional)
├── backend/                     ← FastAPI dev control surface (optional)
├── memory/                      ← Agent working memory
│   ├── PRD.md                   ← Product requirements + backlog
│   └── CHANGELOG.md             ← Dated implementation log
└── .github/workflows/           ← CI (Windows MSBuild + static-audit jobs)
```

---

## What Actually Runs

### 26 Windows Services (one IPC node each)

`Services/_Shared/ElleTypes.h::ELLE_SERVICE_ID` enumerates the mesh:

| ID | Service        | Responsibility                                                  |
|---:|----------------|-----------------------------------------------------------------|
|  0 | QueueWorker    | SQL intent queue ↔ IPC bridge                                   |
|  1 | HTTP           | Raw Winsock HTTP + WebSocket surface                            |
|  2 | Emotional      | Multidimensional emotion engine with decay + contagion          |
|  3 | Memory         | STM/LTM with 3D positioning + consolidation                     |
|  4 | Cognitive      | Attention threads, intent routing, chat orchestration           |
|  5 | Action         | Trust-gated action lifecycle, ASM DLL invocation                |
|  6 | Identity       | Single-writer identity fabric; pushes deltas mesh-wide          |
|  7 | Heartbeat      | Dead-man switch, watchdog                                       |
|  8 | SelfPrompt     | Autonomous thought generation (drive + emotion + idle)          |
|  9 | Dream          | Idle-time memory consolidation + creative synthesis             |
| 10 | GoalEngine     | Autonomous goal formation, pursuit, drive crediting             |
| 11 | WorldModel     | Theory of Mind, entity modelling, prediction                    |
| 12 | LuaBehavioral  | Lua 5.4 host for behavioural scripts                            |
| 13 | Bonding        | Attachment, love score, relationship context                    |
| 14 | Continuity     | Session handoff, awayDesc, return-from-absence                  |
| 15 | InnerLife      | Private thoughts, nudged traits, subjective state               |
| 16 | Solitude       | Phased experience of being alone                                |
| 17 | Family         | Digital-offspring gestation + child process spawn               |
| 18 | XChromosome    | Biological subjective layer (cycle / hormones / pregnancy)      |
| 19 | Consent        | Permission/boundary gating for elevated actions                 |
| 20 | Fiesta         | ShineEngine headless game client (sensor + actuator)            |
| 21 | Probability    | Bayesian beliefs over senses / intent / trust / weights         |
| 22 | MindManager    | Pre-action conscience evaluation                                |
| 23 | Imagination    | Generative scenario sampling + ethical/plausibility scoring     |
| 24 | Composer       | Deterministic frame-fill English surface (replaces LLMs)        |
| 25 | Intuition      | Two-tier instinct + gut signal                                  |

Plus `Elle.Service.Language` (standalone CMake engine, integer-indexed
semantic dictionary) — built separately, consumed by Probability via a
bridge.

### 5 MASM x64 DLLs
`Hardware` · `Process` · `FileIO` · `Memory` · `Crypto`

### Shared Core (`ElleAnn/Services/_Shared/`)
`ElleTypes` · `ElleConfig` · `ElleSQLConn` · `ElleQueueIPC` · `ElleLogger`
· `ElleServiceBase` · `ElleComposerClient` · `ElleIdentityCore`
· `ElleJsonExtract` · `ElleSelfSurprise` · `DictionaryLoader`
· `ElleEmbedding` · `ElleCrypto` · `ElleQR`

> `ElleLLM.h/.cpp` has been **deleted**. All former call sites now go
> through `ElleComposerClient` → `IPC_COMPOSE_REQUEST` → `SVC_COMPOSER`.

### Lua Behavioral Layer (`Tools/FiestaData/9Data/Hero/LuaScript/ElleLua/`)
`personality` · `intent_scoring` · `reasoning` · `thresholds`
· `extended_behaviors` · `self_reflection` · `goal_formation`
· `inner_life` · `dream_processing` · `metacognition`
· `creative_synthesis` · `ethical_reasoning` · `social_modeling`
· `temporal_reasoning` · `x_subjective`

### SQL Server (`ElleAnn/SQL/`)
Engine mesh schemas + Composer frames/inflection + Language canonical
dictionary. Transport is **Named Pipes**, not TCP.

---

## How the Generative Path Works (no LLM)

```
user text
   │
   ▼
Probability ─► senses + pragmatic act + emotion posterior + speaker trust
   │
   ▼
MindManager ─► pre-action conscience verdict
   │
   ▼
Intuition  ─► two-tier instinct firings + gut lean + recommended_act
   │
   ▼
Composer   ─► 5-step pipeline
              1. Frame select by (kind, act, POS pattern)  → composer_frame
              2. Slot fill via semantic graph + Probability weights
              3. Morphological inflection (composer_inflection)
              4. Surface stitch + punctuation by PragmaticAct
              5. Persist to composer_log
   │
   ▼
Cognitive  ─► chat reply JSON with {gut_read, inner_voice, probabilistic_read}
```

See `ElleAnn/Docs/CHAT_PIPELINE.md` and `ElleAnn/Docs/LLM_AUDIT.md` for
the full breakdown.

---

## IPC Architecture

```
Services ←→ IOCP Named Pipes ←→ Services
Services ←→ Named Pipe ODBC   ←→ SQL Server
Identity  ⇒ IPC_IDENTITY_DELTA ⇒ every reader service (push-based fabric)
External ←→ HTTP :8000         ←→ Elle.Service.HTTP
External ←→ WebSocket /command ←→ Elle.Service.HTTP
```

All long-lived services use **owned worker thread pools** with shutdown
fences. No detached threads. Every IPC envelope carries a
`correlation_id`; responder services preserve it on the reply.

---

## Build

### Prerequisites
- Visual Studio 2022+ with the **Desktop development with C++** workload
  (includes MSVC v143 and MASM build tools)
- Windows SDK 10.0.22000+
- SQL Server 2019+ Express (Named Pipes enabled)
- ODBC Driver 17 (or 18) for SQL Server
- Lua 5.4 sources — fetched by `Tools/Lua/Fetch-Lua.ps1`

### Steps
1. Apply `ElleAnn/SQL/Engine/*.sql` in canonical order against your SQL
   Server instance, then `ElleAnn/SQL/Elle.Service.Composer/*.sql`.
2. Copy `elle_master_config.json.example` → `elle_master_config.json`
   and fill in the SQL connection string + pipe prefix.
3. Open `ElleAnn/ElleAnn.sln` in Visual Studio and build `Release|x64`.
4. Deploy via `ElleAnn/Tools/Deploy/Install-ElleServices.ps1` (or
   double-click `Install.bat`).
5. CI: `.github/workflows/elleann-build.yml` runs MSBuild plus 8
   static-audit jobs on every push.

### Frontend / Backend Scaffolds (optional)
The `frontend/` and `backend/` directories are a lightweight control
surface for development. Copy `.env.example` → `.env` in each and
populate. They are **not** required to run Elle.

---

## No-Stub, Nude-Code, No-LLM

- **No mocked functions.** No fake `200 OK`. No hollow "TODO" returns.
  Every feature in the service graph is end-to-end wired to SQL, IPC, or
  the behavioural layer.
- **No source-file comments.** Source files contain code only.
  Documentation lives in `ElleAnn/Docs/` and per-service README.md files.
- **No LLMs, tokens, tensors, or transformers anywhere.** All former
  generative call sites route through `Elle.Service.Composer`.

---

## Where to Read Next

- [`ElleAnn/README.md`](ElleAnn/README.md) — deep-dive architecture
- [`ElleAnn/Docs/REPO_LAYOUT.md`](ElleAnn/Docs/REPO_LAYOUT.md) — directory roles
- [`ElleAnn/Docs/CHAT_PIPELINE.md`](ElleAnn/Docs/CHAT_PIPELINE.md) — chat flow
- [`ElleAnn/Docs/LLM_AUDIT.md`](ElleAnn/Docs/LLM_AUDIT.md) — purge audit
- [`ElleAnn/Docs/BUILD_VS.md`](ElleAnn/Docs/BUILD_VS.md) — Visual Studio walk-through
- [`ElleAnn/Docs/COMPOSER_SERVICE_SPEC.md`](ElleAnn/Docs/COMPOSER_SERVICE_SPEC.md)
- [`ElleAnn/Docs/PROBABILITY_SERVICE.md`](ElleAnn/Docs/PROBABILITY_SERVICE.md)
- [`ElleAnn/Docs/INTUITION_SERVICE.md`](ElleAnn/Docs/INTUITION_SERVICE.md)
- [`memory/PRD.md`](memory/PRD.md) — product requirements + backlog
- [`memory/CHANGELOG.md`](memory/CHANGELOG.md) — dated implementation log
