# Build Notes — Getting Elle Running

Read this before opening Visual Studio. The detailed VS walk-through
lives in `BUILD_VS.md`; this file covers the prerequisites and the
moving parts you need to set up once.

## Prerequisites

### Required

- **Visual Studio 2022** (Community or higher) with:
  - **Desktop development with C++** workload
  - **MSVC v143** build tools
  - **Windows 11 SDK** (10.0.22000+; any 10.x works)
  - **C++ MASM build tools** (component, included in the workload —
    required for the 5 `Elle.ASM.*` DLL projects)

- **SQL Server 2019+ Express**
  - Enable **Named Pipes** in SQL Server Configuration Manager
  - Apply the deltas in `ElleAnn/SQL/` in order (see "Database" below)

- **ODBC Driver 17 (or 18) for SQL Server**
  - https://learn.microsoft.com/sql/connect/odbc/download-odbc-driver-for-sql-server

- **Lua 5.4 sources** — fetched once by
  `ElleAnn/Tools/Lua/Fetch-Lua.ps1`. The `Elle.Lua.Behavioral` project
  compiles Lua inline; no prebuilt `lua54.lib` is required.

### NOT required

- ~~`llama.cpp`~~ — no longer used. The LLM purge is complete.
- ~~GGUF models~~ — no longer used.
- ~~Groq / OpenAI / Anthropic API keys~~ — no LLM dependencies remain.

## Database

Canonical order (every script is idempotent — `IF NOT EXISTS`-guarded):

```sql
-- Engine mesh:
:r ElleAnn\SQL\Engine\ElleAnn_Schema.sql
:r ElleAnn\SQL\Engine\ElleAnn_Identity_Schema.sql
:r ElleAnn\SQL\Engine\ElleAnn_XChromosome_Schema.sql
:r ElleAnn\SQL\Engine\ElleAnn_MemoryDelta.sql
:r ElleAnn\SQL\Engine\ElleAnn_QueueReaperDelta.sql
:r ElleAnn\SQL\Engine\ElleAnn_PairedDevicesDelta.sql
:r ElleAnn\SQL\Engine\ElleAnn_Sessions_Delta.sql
:r ElleAnn\SQL\Engine\ElleAnn_System_Schema.sql
:r ElleAnn\SQL\Engine\ElleAnn_SchemaSync_FebPivot.sql
:r ElleAnn\SQL\Engine\ElleAnn_GameUnification.sql
:r ElleAnn\SQL\Engine\imagined_and_conscience.sql

-- Composer frames:
:r ElleAnn\SQL\Elle.Service.Composer\composer_schema_seed.sql

-- (Optional) full canonical dictionary — for Language engine + ETL:
:r ElleAnn\SQL\Elle.Service.Language\01_schema.sql
:r ElleAnn\SQL\Elle.Service.Language\02_seed_lexicon.sql
:r ElleAnn\SQL\Elle.Service.Language\... (03..09)
```

## Build order (dependency-driven)

MSBuild walks the dependency graph automatically from `ElleAnn.sln`.
For reference:

1. `ElleCore.Shared` (static lib) — every service depends on it
2. `Elle.ASM.*` DLLs (5) — independent of each other; can run in
   parallel
3. `Elle.Service.*` executables (25) — depend on Shared
4. `Elle.Lua.Behavioral` — depends on Shared + vendored Lua 5.4
5. (Standalone CMake projects, built separately when needed:
   `Elle.Service.Probability` and `Elle.Service.Language`)

## Quick start

### 1. Configure

```jsonc
// elle_master_config.json
{
  "services": {
    "sql_pipes": {
      "connection_string":
        "Driver={ODBC Driver 17 for SQL Server};Server=.\\ELLEANN;Database=ElleCore;Trusted_Connection=yes;"
    }
  },
  "ipc": {
    "pipe_prefix": "\\\\.\\pipe\\ElleAnn_"
  },
  "cognitive": {
    "probability_timeout_ms": 300,
    "conscience_timeout_ms":  200,
    "intuition_timeout_ms":   150,
    "chat_workers":           4,
    "max_chat_queue":         64
  }
  // Note: no "llm" block — LLMs were purged. Generation is owned by SVC_COMPOSER.
}
```

### 2. Fetch Lua sources

```powershell
cd ElleAnn\Tools\Lua
.\Fetch-Lua.ps1                 # downloads Lua 5.4.6 by default
```

### 3. Build

Open `ElleAnn.sln` → Release | x64 → Build Solution (Ctrl+Shift+B).

### 4. Install as Windows services

```powershell
cd ElleAnn\Tools\Deploy
.\Install-ElleServices.ps1      # registers + starts every service
```

Or double-click `Install.bat` (auto-elevates).

## Compilation notes

### ODBC

Every SQL call goes through `ElleSQLPool` (in `_Shared/ElleSQLConn.h`)
which uses parameterised statements via `SQLBindParameter`. No string
concatenation; no SQL-injection surface.

### MASM

Custom build tool on each `.asm` file:
```
ml64 /c /Fo"$(IntDir)%(Filename).obj" "%(FullPath)"
```
Link with the matching `.def` file:
```
link /DLL /DEF:Hardware.def Hardware.obj /OUT:Elle.ASM.Hardware.dll
```

### Lua

The vcxproj globs `Tools\Lua\lua54\src\*.c` and compiles them in-line.
Two warnings (`4996`, `4267`) are silenced for the Lua sources only.

## Service startup order

Enforced by `Tools/Deploy/elle_service_manifest.json`. Reference:

1. `Heartbeat` (foundation — monitors everything else)
2. `QueueWorker`
3. `Emotional`, `Memory`
4. `Identity` (authoritative single-writer fabric; must be up before
   any peer pushes deltas)
5. `WorldModel`, `GoalEngine`
6. `Probability`, `MindManager`, `Imagination`, `Composer`, `Intuition`
7. `Action`, `Consent`
8. `SelfPrompt`, `Solitude`
9. `Bonding`, `InnerLife`, `Dream`
10. `XChromosome`, `Family`
11. `Continuity`
12. `LuaBehavioral`
13. `Cognitive` (needs all of the above)
14. `Fiesta` (if you intend to attach to a ShineEngine server)
15. `HTTP` (last — so every peer is already listening on its pipe)

## Testing

- Use `--console` on any `.exe` for interactive debugging
- Standalone CMake suites:
  ```bash
  cd ElleAnn/Services/Elle.Service.Probability
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j
  ctest --test-dir build --output-on-failure    # 43/43 PASS
  ```
- `Tools/Debug/` has a Linux portability harness with `windows.h` stubs
  for static compile-checks of shared headers

## File / scale snapshot

- 32 buildable projects in the solution (`ElleAnn.sln`)
- 26 services + 1 Lua host on the IPC mesh
- 5 MASM-x64 DLLs
- 1 standalone CMake engine (`Elle.Service.Language`)
- ~222 C++ source/header files under `Services/`
- 15+ SQL deltas + 90+ seeded Composer frames + canonical dictionary
- 15 Lua behavioural scripts in `Tools/FiestaData/9Data/.../ElleLua/`
