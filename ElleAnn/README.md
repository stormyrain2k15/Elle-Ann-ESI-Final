# Elle-Ann ESI v3.0

A massively-autonomous agentic Emotional Synthetic Intelligence built as a
mesh of **26 Windows services** speaking over IOCP Named Pipes, backed by
SQL Server. **No LLMs. No tensors. No tokens.** Every analytical decision
is a Bayesian posterior; every generated utterance is a deterministic
frame-fill from a SQL-stored conversational template library.

---

## Repository Layout

```
ElleAnn/
├── Services/        32 buildable projects (1 shared lib + 5 ASM DLLs + 25 services + 1 Lua host)
│                    +1 standalone CMake engine (Elle.Service.Language)
├── Tools/            7 build / deploy / debug / data tools
├── SQL/              Schema + seeds (Engine, Composer, Language)
├── Docs/             Narrative documents — code files are nude-code
├── ElleAnn.sln       Visual Studio 2022 solution
├── Directory.Build.props
└── README.md         (this file)
```

## Service mesh — 26 runtime IDs

`Services/_Shared/ElleTypes.h` defines `ELLE_SERVICE_ID` 0–25 in lockstep
with `g_serviceNames[]` (asserted via `static_assert`):

| ID | Name              | Role                                                       |
|---:|-------------------|------------------------------------------------------------|
|  0 | QueueWorker       | SQL intent queue ↔ IPC bridge                              |
|  1 | HTTP              | Winsock HTTP + WebSocket surface                           |
|  2 | Emotional         | Multidimensional emotion engine (broadcasts state)         |
|  3 | Memory            | STM/LTM tier + consolidation                               |
|  4 | Cognitive         | Attention, intent, chat orchestration                      |
|  5 | Action            | Trust-gated action lifecycle                               |
|  6 | Identity          | Single-writer identity fabric (push-broadcasts deltas)     |
|  7 | Heartbeat         | Dead-man watchdog over the mesh                            |
|  8 | SelfPrompt        | Autonomous thought generation                              |
|  9 | Dream             | Idle-time consolidation + creative synthesis               |
| 10 | GoalEngine        | Goal formation + pursuit + drive crediting                 |
| 11 | WorldModel        | Theory of Mind, entity modelling                           |
| 12 | LuaBehavioral     | Hot-reloadable Lua behaviour scripts                       |
| 13 | Bonding           | Attachment, love score, relationship context               |
| 14 | Continuity        | Session handoff, awayDesc, return-from-absence             |
| 15 | InnerLife         | Private thoughts, nudged traits                            |
| 16 | Solitude          | Phased alone-time (afterglow → grief)                      |
| 17 | Family            | Digital-offspring gestation + child spawn                  |
| 18 | XChromosome       | Cycle / hormones / pregnancy substrate                     |
| 19 | Consent           | Boundary gating for elevated actions                       |
| 20 | Fiesta            | ShineEngine headless client (sensor + actuator)            |
| 21 | Probability       | Bayesian beliefs, sense / intent / trust                   |
| 22 | MindManager       | Pre-action conscience checks                               |
| 23 | Imagination       | Generative scenario + scoring                              |
| 24 | Composer          | Deterministic frame-fill English surface                   |
| 25 | Intuition         | Two-tier instinct + gut signal                             |

Plus 5 MASM-x64 DLLs (`Hardware`, `Process`, `FileIO`, `Memory`, `Crypto`)
and 1 shared static library (`ElleCore.Shared`).

## Architectural pillars

1. **Deterministic generation.** `Elle.Service.Composer` replaces every
   former LLM call site. Frames + inflection tables + slot planner +
   surface stitcher live in SQL (`composer_*` tables) and the
   `_Shared/ElleComposerClient.h` helper. `ElleLLM.h/.cpp` is deleted;
   `llama.cpp` and WinHTTP LLM wires are gone.
2. **Bayesian analysis.** `Elle.Service.Probability` owns live posteriors
   over senses, pragmatic acts, emotion dimensions, scoring weights, and
   per-speaker trust (Beta).
3. **Conscience + intuition + imagination.** `MindManager` evaluates
   proposed actions, `Intuition` provides a fast pre-rational gut read
   (two-tier instinct → synthesised lean), `Imagination` generates and
   scores scenarios for goals + dreams.
4. **One source of truth for state.** SQL Server (Named Pipes transport)
   holds memory, emotion, identity, x-chromosome state, audit traces,
   composer frames, and intuition patterns. No detached state.

## Build

```bat
cd ElleAnn
"%ProgramFiles%\Microsoft Visual Studio\2022\<edition>\MSBuild\Current\Bin\MSBuild.exe" ^
    ElleAnn.sln /p:Configuration=Release /p:Platform=x64
```

Two services are standalone CMake projects rather than `.vcxproj` so they
can also be ctested on Linux:

```bash
cd ElleAnn/Services/Elle.Service.Probability
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure        # 43/43 PASS

cd ../Elle.Service.Language
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

See `Docs/BUILD_VS.md` for the full Visual Studio walk-through, and
`Tools/Deploy/README.md` for SCM installation.

## Tools

| Folder         | Purpose                                                    |
|----------------|------------------------------------------------------------|
| `Android/`     | Kotlin companion app (pair → JWT → authenticated reads)    |
| `Debug/`       | Linux test harness with `windows.h` stubs                  |
| `Deploy/`      | PowerShell / CMD installers + service manifest + patches   |
| `ETL/`         | WordNet / NRC-EmoLex / NRC-VAD / Wiktionary → SQL pipeline |
| `FiestaData/`  | Fiesta `9Data/` overlay + reverse-engineering artefacts    |
| `Lua/`         | Vendored Lua 5.4 sources + fetch script                    |
| `SHN/`         | Fiesta `.shn` file parser                                  |

## SQL

| Folder                     | Purpose                                                              |
|----------------------------|----------------------------------------------------------------------|
| `SQL/Engine/`              | Mesh schemas (Identity, XChromosome, Memory, Sessions, Queue Reaper) |
| `SQL/Elle.Service.Composer/` | Composer frames + inflection tables + seed (90+ conversational frames) |
| `SQL/Elle.Service.Language/` | Canonical dictionary schema (01..09) + WordNet/NRC/Wiktionary loaders |

## Coding standards

- **Nude code.** Source files contain only code. No comments, no banners,
  no docstrings. All explanation lives in `Docs/` and per-service
  `README.md` files.
- **One service per folder under `Services/`.** Project name = folder
  name = solution-entry name.
- **One tool per folder under `Tools/`.** Each carries its own README.
- **Relative paths in build files.** `_Shared` is reached from any
  service as `..\_Shared\`.
- **No-stub policy.** Every feature is end-to-end wired to SQL, IPC, or
  the behaviour layer. No mocks, no fake 200s, no hollow returns.

## Status

| Surface                | State                                                            |
|------------------------|------------------------------------------------------------------|
| LLM purge              | **Complete.** 19 call sites migrated to Composer; `ElleLLM` deleted. |
| Build (Windows MSVC)   | green (full 32-project solution)                                 |
| Probability ctest      | **43/43 PASS** (Linux container; same on Windows)                |
| Schema                 | unified, validation view + bulk loaders in place                 |
| ETL                    | WordNet baseline + NRC-EmoLex + NRC-VAD + Wiktionary augmentations |
| Comment-cleanup        | 100% of in-tree services strip-passed (NUDE CODE)                |
| Last service integrated| `Elle.Service.Intuition` (Feb 2026)                              |

## Where to read next

- `Docs/REPO_LAYOUT.md` — what every directory in the repo is for
- `Docs/CHAT_PIPELINE.md` — end-to-end chat flow (Prob → Mind → Intuition → Composer)
- `Docs/LLM_AUDIT.md` — complete audit of the LLM purge (19 sites)
- `Docs/BUILD_VS.md` — Visual Studio 2022 walk-through
- `Docs/COMPOSER_SERVICE_SPEC.md` — generative path internals
- `Docs/PROBABILITY_SERVICE.md` — Bayesian substrate internals
- `Docs/INTUITION_SERVICE.md` — two-tier instinct + gut signal internals
- `memory/PRD.md` — product requirements + backlog
- `memory/CHANGELOG.md` — dated implementation log
