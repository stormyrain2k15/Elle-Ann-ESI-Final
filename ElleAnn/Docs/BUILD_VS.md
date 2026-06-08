# Visual Studio Build Guide — Elle-Ann ESI v3.0

## Prerequisites

### 1. Visual Studio 2022 (Community is fine)

Install with these workloads:

- **Desktop development with C++**
- **MSVC v143** toolset (default with VS 2022 17.9+)
- **Windows 11 SDK** (10.0.22000 or later; any 10.x works)
- **C++ MFC / ATL**: NOT required
- **MASM build tools** — Individual Components → tick *MSVC v143 - VS 2022
  C++ x64/x86 MASM build tools* (required for the 5 `Elle.ASM.*` DLL
  projects)

### 2. SQL Server (for ODBC connection)

- SQL Server 2019+ (Express is fine).
- The services connect via **ODBC Driver 17 (or 18) for SQL Server**:
  https://learn.microsoft.com/sql/connect/odbc/download-odbc-driver-for-sql-server
- Create the canonical databases (names referenced in the SQL deltas):
  ```sql
  CREATE DATABASE ElleCore;
  CREATE DATABASE ElleHeart;
  CREATE DATABASE ElleSystem;
  ```
- Apply the deltas under `ElleAnn\SQL\Engine\*.sql` and the seed under
  `ElleAnn\SQL\Elle.Service.Composer\composer_schema_seed.sql`. See
  `BUILD_NOTES.md` for canonical order.

### 3. Lua 5.4 (pick ONE option)

**Option A — Drop-in source (easiest, zero configuration; the CI uses
this)**

```powershell
cd ElleAnn\Tools\Lua
.\Fetch-Lua.ps1                 # downloads 5.4.6 by default
.\Fetch-Lua.ps1 -Version 5.4.7  # or pin a specific version
```

Tree after fetch:
```
ElleAnn\Tools\Lua\lua54\src\lua.h
ElleAnn\Tools\Lua\lua54\src\lualib.h
ElleAnn\Tools\Lua\lua54\src\lauxlib.h
ElleAnn\Tools\Lua\lua54\src\lapi.c   ... (~35 .c files total)
```

`Elle.Lua.Behavioral.vcxproj` auto-detects these and compiles Lua
inline. No prebuilt `lua54.lib` is required.

**Option B — vcpkg**

```
vcpkg install lua:x64-windows-static
```

Set the `LUA_DIR` environment variable to the vcpkg
`installed/x64-windows-static` directory before launching VS, or
override `<LuaDir>` in `Directory.Build.props`.

**Option C — Prebuilt `lua54.lib`**

Place `lua54.lib` under `ElleAnn\Tools\Lua\lua54\lib\` and headers
under `ElleAnn\Tools\Lua\lua54\include\`.

## Solution layout

```
ElleAnn/
├── ElleAnn.sln                       ← open this in VS
├── Directory.Build.props             ← shared compile/link flags (auto-imported)
├── elle_master_config.json           ← runtime config
└── Services/
    ├── _Shared/
    │   └── ElleCore.Shared.vcxproj   ← static lib (every exe depends on it)
    ├── Elle.ASM.Crypto/*.vcxproj     ← MASM → DLL
    ├── Elle.ASM.FileIO/*.vcxproj
    ├── Elle.ASM.Hardware/*.vcxproj
    ├── Elle.ASM.Memory/*.vcxproj
    ├── Elle.ASM.Process/*.vcxproj
    ├── Elle.Lua.Behavioral/*.vcxproj ← Lua host (SVC_LUA_BEHAVIORAL)
    ├── Elle.Service.Action/*.vcxproj
    ├── Elle.Service.Bonding/*.vcxproj
    ├── Elle.Service.Cognitive/*.vcxproj
    ├── Elle.Service.Composer/*.vcxproj
    ├── Elle.Service.Consent/*.vcxproj
    ├── Elle.Service.Continuity/*.vcxproj
    ├── Elle.Service.Dream/*.vcxproj
    ├── Elle.Service.Emotional/*.vcxproj
    ├── Elle.Service.Family/*.vcxproj
    ├── Elle.Service.Fiesta/*.vcxproj
    ├── Elle.Service.GoalEngine/*.vcxproj
    ├── Elle.Service.Heartbeat/*.vcxproj
    ├── Elle.Service.HTTP/*.vcxproj
    ├── Elle.Service.Identity/*.vcxproj
    ├── Elle.Service.Imagination/*.vcxproj
    ├── Elle.Service.InnerLife/*.vcxproj
    ├── Elle.Service.Intuition/*.vcxproj   ← integrated Feb 2026
    ├── Elle.Service.Memory/*.vcxproj
    ├── Elle.Service.MindManager/*.vcxproj
    ├── Elle.Service.Probability/*.vcxproj (and standalone CMakeLists.txt)
    ├── Elle.Service.QueueWorker/*.vcxproj
    ├── Elle.Service.SelfPrompt/*.vcxproj
    ├── Elle.Service.Solitude/*.vcxproj
    ├── Elle.Service.WorldModel/*.vcxproj
    └── Elle.Service.XChromosome/*.vcxproj
```

`Elle.Service.Language` lives under `Services/` but is **not** in
`ElleAnn.sln` — it is a standalone CMake engine consumed by
`Elle.Service.Probability` via the bridge. Build it separately when
needed (see "Standalone CMake suites" below).

## Build

1. Open `ElleAnn.sln` in Visual Studio 2022.
2. Select configuration **Release | x64** (or Debug | x64).
3. **Build → Build Solution** (Ctrl+Shift+B).

Build order is enforced automatically via `ProjectReference` — every
service waits on `ElleCore.Shared`. ASM DLLs build in parallel with
services.

## Output

- Executables: `Deploy/Release/x64/*.exe` and `*.dll`
- PDBs (debug symbols): same folder
- Intermediate objects: `Intermediate/<ProjectName>/<Configuration>/`

Recommended `.gitignore` additions:
```
Intermediate/
Deploy/Release/
Deploy/Debug/
*.vcxproj.user
.vs/
```

## Known build notes

- **RuntimeLibrary**: every project uses `/MT` (static MSVCRT) in
  Release, `/MTd` in Debug. Produced exes / DLLs can be copied to any
  Windows machine without redistributable installs. Flip
  `Directory.Build.props` if you need `/MD` for a foreign library.
- **Warnings silenced**: `4996` (secure-CRT deprecation), `4267`
  (size_t→int), `4244` (narrowing), `4018` (signed/unsigned compare),
  `4146`, `4065` — these appear in legacy C code (Lua, old helpers).
- **SDL checks**: enabled. If a service fails with Spectre diagnostics,
  install the *Spectre-mitigated libs* component from VS Installer.
- **MASM**: if `masm.props` import fails ("not found"), the C++ MASM
  build customisation isn't installed — VS Installer → Modify →
  Individual components → tick *MSVC v143 - VS 2022 C++ x64/x86 MASM
  build tools*.

## Standalone CMake suites

Two services maintain CMake projects in addition to (or instead of)
their `.vcxproj` so they can be ctested:

```bash
# Probability — Bayesian substrate
cd ElleAnn/Services/Elle.Service.Probability
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure          # 43/43 PASS

# Language — semantic dictionary engine (no .vcxproj — CMake only)
cd ../Elle.Service.Language
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`Elle.Service.Probability` also has an `apps/bridge_smoke_demo` that
exercises the Language ↔ Probability bridge end-to-end when
`ELLE_PROB_WITH_LANGUAGE_ENGINE=ON` (or `AUTO` with the Language
headers present).

## Running

Services are console applications that can also be registered as real
Windows Services (they use `ElleServiceBase` which supports both modes).

**Foreground (development):** launch each `.exe --console` from an
elevated console. Read logs on stdout.

**Windows Service (production):**

```powershell
cd ElleAnn\Tools\Deploy
.\Install-ElleServices.ps1
```

The PowerShell installer reads `elle_service_manifest.json` and
registers + starts every service in dependency order. See
`Tools/Deploy/README.md`.

## Troubleshooting

| Symptom                                     | Cause                                  | Fix                                                                         |
|---------------------------------------------|----------------------------------------|-----------------------------------------------------------------------------|
| `masm.props not found`                      | MASM build tools missing               | VS Installer → add MSVC MASM build tools                                    |
| `cannot open lua.h`                         | Lua sources not fetched                | Run `Tools\Lua\Fetch-Lua.ps1`                                               |
| `LNK2019 _odbc32_...`                       | ODBC driver SDK missing                | Install Windows SDK; `odbc32.lib` is part of it                             |
| Service starts then exits silently          | Bad CWD / config path                  | Use absolute paths in `elle_master_config.json`; check `logs\` permissions  |
| Cognitive replies are blank                 | `Elle.Service.Composer` not running    | Composer is mandatory now — must be up before Cognitive                     |
| Intuition signals never appear              | `Elle.Service.Intuition` not running   | Start Intuition; Cognitive degrades cleanly if it's missing                 |
| `static_assert ELLE_SERVICE_COUNT changed`  | `g_serviceNames[]` and enum out of sync| Update both in lockstep — see `_Shared/ElleQueueIPC.cpp`                    |

## CI

`.github/workflows/elleann-build.yml` runs on every push to `main` and
on every PR:

1. SLN integrity
2. C++ balance audit (braces / parens balanced across the tree)
3. SQL delta idempotency
4. SQL schema end-to-end coherence
5. PowerShell syntax check
6. Service-name manifest consistency
7. cppcheck static analysis
8. Windows MSBuild against vendored Lua 5.4 (MD5-verified download)

All 8 jobs must pass for green ✓.
