# Repository Layout

A directory-by-directory accounting of everything in this repo, what
each thing is for, and whether it is consumed at Elle-Ann runtime.

## Top-level directories under `/app`

| Path                  | Role                                                              | Runtime? |
|-----------------------|-------------------------------------------------------------------|----------|
| `ElleAnn/`            | The product — C++ services, MASM DLLs, SQL, deploy scripts        | yes      |
| `frontend/`           | Optional React control-surface for dev-time inspection            | no       |
| `backend/`            | Optional FastAPI dev scaffold for the same control surface        | no       |
| `memory/`             | Agent working memory (PRD, CHANGELOG, handoff notes)              | no       |
| `test_reports/`       | Output from `testing_agent_v3_fork` runs                          | no       |
| `.github/workflows/`  | CI — MSBuild + 8 static-audit jobs gating PRs into main           | gate     |
| `.git/` `.gitconfig`  | Standard git plumbing + agent commit-identity                     | no       |
| `.emergent/`          | Fork-platform metadata; not consumed by Elle-Ann                  | no       |
| `.fiesta_re/`         | Reverse-engineering artefacts for the Fiesta client opcode dump   | no       |
| `.ruff_cache/`        | Ruff lint cache for the optional Python scaffolds                 | no       |

## `ElleAnn/` — the product

```
ElleAnn/
├── ElleAnn.sln               Visual Studio 2022 solution, 32 buildable projects
├── Directory.Build.props     Shared MSBuild flags (MSVCRT mode, warnings, etc.)
├── elle_master_config.json   Runtime config: SQL conn string, pipe prefix, services
├── README.md                 Top-level architecture doc
├── Services/                 _Shared lib + 5 ASM DLLs + 25 services + 1 Lua host
│                             + 1 standalone CMake engine (Elle.Service.Language)
├── Tools/                    7 build / deploy / debug / data tools
├── SQL/                      Schemas + seeds split by owner
└── Docs/                     Narrative documentation
```

### `ElleAnn/Services/`

| Folder pattern         | Count | Purpose                                                            |
|------------------------|------:|--------------------------------------------------------------------|
| `_Shared/`             |     1 | `ElleCore.Shared` static library — depended on by every service    |
| `Elle.ASM.*/`          |     5 | MASM-x64 DLLs: Hardware, Process, FileIO, Memory, Crypto           |
| `Elle.Lua.Behavioral/` |     1 | Lua 5.4 host for behavioural scripts                               |
| `Elle.Service.*/`      |    26 | One folder per service in the mesh (Language is CMake-only)        |

The 26 service folders map 1:1 to `ELLE_SERVICE_ID` 0–25 in
`_Shared/ElleTypes.h` (except `Elle.Service.Language`, which has no
service-id — it is consumed in-process by `Elle.Service.Probability`
via the bridge).

| Service folder                  | ELLE_SERVICE_ID         |
|---------------------------------|-------------------------|
| `Elle.Service.QueueWorker`      | `SVC_QUEUE_WORKER`      |
| `Elle.Service.HTTP`             | `SVC_HTTP_SERVER`       |
| `Elle.Service.Emotional`        | `SVC_EMOTIONAL`         |
| `Elle.Service.Memory`           | `SVC_MEMORY`            |
| `Elle.Service.Cognitive`        | `SVC_COGNITIVE`         |
| `Elle.Service.Action`           | `SVC_ACTION`            |
| `Elle.Service.Identity`         | `SVC_IDENTITY`          |
| `Elle.Service.Heartbeat`        | `SVC_HEARTBEAT`         |
| `Elle.Service.SelfPrompt`       | `SVC_SELF_PROMPT`       |
| `Elle.Service.Dream`            | `SVC_DREAM`             |
| `Elle.Service.GoalEngine`       | `SVC_GOAL_ENGINE`       |
| `Elle.Service.WorldModel`       | `SVC_WORLD_MODEL`       |
| `Elle.Lua.Behavioral`           | `SVC_LUA_BEHAVIORAL`    |
| `Elle.Service.Bonding`          | `SVC_BONDING`           |
| `Elle.Service.Continuity`       | `SVC_CONTINUITY`        |
| `Elle.Service.InnerLife`        | `SVC_INNER_LIFE`        |
| `Elle.Service.Solitude`         | `SVC_SOLITUDE`          |
| `Elle.Service.Family`           | `SVC_FAMILY`            |
| `Elle.Service.XChromosome`      | `SVC_X_CHROMOSOME`      |
| `Elle.Service.Consent`          | `SVC_CONSENT`           |
| `Elle.Service.Fiesta`           | `SVC_FIESTA`            |
| `Elle.Service.Probability`      | `SVC_PROBABILITY`       |
| `Elle.Service.MindManager`      | `SVC_MIND_MANAGER`      |
| `Elle.Service.Imagination`      | `SVC_IMAGINATION`       |
| `Elle.Service.Composer`         | `SVC_COMPOSER`          |
| `Elle.Service.Intuition`        | `SVC_INTUITION`         |
| `Elle.Service.Language`         | (no IPC id — CMake lib) |

### `ElleAnn/Tools/`

| Folder        | What it is                                                              |
|---------------|-------------------------------------------------------------------------|
| `Android/`    | Kotlin companion app — pair → JWT → authenticated reads                 |
| `Debug/`      | Linux test harness with `windows.h` stubs for portable syntax checks    |
| `Deploy/`     | SCM installer (PowerShell + batch), service manifest, video worker      |
| `ETL/`        | Lexicon pipeline (WordNet + NRC-EmoLex + NRC-VAD + Wiktionary) → CSV → SQL |
| `FiestaData/` | Overlay tree mirroring `9Data/Hero/LuaScript/ElleLua/` for deployment   |
| `Lua/`        | Lua 5.4 source-fetcher (`Fetch-Lua.ps1`) — vendored at build time       |
| `SHN/`        | Fiesta `.shn` (table format) parser                                     |

### `ElleAnn/SQL/`

| Folder                          | Purpose                                                          |
|---------------------------------|------------------------------------------------------------------|
| `SQL/Engine/`                   | Mesh schemas: identity, memory delta, queue reaper, x-chromosome, sessions, full schema sync |
| `SQL/Elle.Service.Composer/`    | Composer frames + inflection tables + 90+ seed frames            |
| `SQL/Elle.Service.Language/`    | Canonical dictionary (01..09) + WordNet/NRC/Wiktionary loaders   |

### `ElleAnn/Docs/`

Narrative-only. Code files are nude-code; all explanation lives here.

| File                         | Topic                                                        |
|------------------------------|--------------------------------------------------------------|
| `REPO_LAYOUT.md`             | this file                                                    |
| `CHAT_PIPELINE.md`           | end-to-end chat path (Prob → Mind → Intuition → Composer)    |
| `LLM_AUDIT.md`               | the 19-site LLM purge — complete                             |
| `COMPOSER_SERVICE_SPEC.md`   | generative path internals                                    |
| `PROBABILITY_SERVICE.md`     | Bayesian substrate internals                                 |
| `INTUITION_SERVICE.md`       | two-tier instinct + gut signal internals                     |
| `BUILD_NOTES.md`             | current build steps + prerequisites                          |
| `BUILD_VS.md`                | Visual Studio 2022 walk-through                              |
| `ANDROID_INTEGRATION.md`     | Kotlin companion app integration                             |
| `MEMORY_CONSOLIDATION_PORT.md` | how STM → LTM works in Memory service                      |
| `AUDIT_*.md` `STUB_*.md` `SCHEMA_FIX_NOTES.md` `FULL_STUB_SWEEP.md` | historical audit reports — snapshots in time, not living docs |

## `frontend/`

Optional. React dev-time control surface. Not part of the shipped
product. See `frontend/README.md` if you want to bring it up.

## `backend/`

Optional. FastAPI scaffold paired with `frontend/`. Not part of the
shipped product — the canonical backend is the 26-service C++ mesh
under `ElleAnn/`.

## `memory/`

Notes written by fork agents for cross-session continuity.
**Not consumed at runtime.** Safe to delete or ignore on any local
clone.

- `PRD.md` — product requirements + backlog (living doc)
- `CHANGELOG.md` — dated implementation log (append-only)
- `test_credentials.md` — admin/test creds for testing-agent runs

## `.github/workflows/elleann-build.yml`

CI pipeline. Runs on every push to `main` or PR:

1. SLN integrity check
2. C++ balance audit (braces / parens balanced across the tree)
3. SQL delta idempotency
4. SQL schema end-to-end coherence
5. PowerShell syntax check
6. Service-name manifest consistency (g_serviceNames ↔ ELLE_SERVICE_COUNT)
7. cppcheck static analysis
8. Windows MSBuild against Lua 5.4 (MD5-verified download)

All jobs must pass for green ✓.

## What is NOT runtime-consumed

`.emergent/`, `.gitconfig`, `.git/`, `.github/`, `.ruff_cache/`,
`memory/`, `test_reports/`, `frontend/`, `backend/` — none of these
are read by any `Elle.Service.*` process. Safe to ignore on a local
build / deploy box.

## What IS runtime-consumed

- `ElleAnn/elle_master_config.json` (config)
- `ElleAnn/Services/**/*.exe` and `*.dll` (after MSBuild)
- `ElleAnn/Tools/Deploy/elle_service_manifest.json` (install order)
- `ElleAnn/Tools/FiestaData/9Data/Hero/LuaScript/ElleLua/*.lua` (loaded by `Elle.Lua.Behavioral`)
- SQL Server: schemas applied from `ElleAnn/SQL/`
