# Visual Studio Build Guide — Elle-Ann ESI v3.0

## Prerequisites

### 1. Visual Studio 2022 (Community is fine)
Install with these workloads:
- **Desktop development with C++**
- **MSVC v143** toolset (default with VS 2022 17.9+)
- **Windows 11 SDK** (10.0.22000 or later; any 10.x works)
- **C++ MFC / ATL**: NOT required
- **Build Tools → MASM** (included with the C++ workload — required for the 5 `Elle.ASM.*` DLL projects)

### 2. SQL Server (for ODBC connection)
- SQL Server 2019+ (Express is fine).
- The services connect via **ODBC Driver 17 for SQL Server** — install from
  https://learn.microsoft.com/sql/connect/odbc/download-odbc-driver-for-sql-server
- Create the three databases:
  ```sql
  CREATE DATABASE ElleCore;
  CREATE DATABASE ElleHeart;
  CREATE DATABASE ElleSystem;
  ```
- Run the delta scripts in `/SQL/*.sql` against the appropriate DB.
- Connection string in `elle_master_config.json` → `sql.connection_string`.

### 3. Lua 5.4 (pick ONE option)

**Option A — Drop-in source (easiest, zero configuration)**
Download the Lua 5.4 tarball and extract into `Lua/lua54/` so the tree looks like:
```
Lua/lua54/src/lua.h
Lua/lua54/src/lualib.h
Lua/lua54/src/lauxlib.h
Lua/lua54/src/lapi.c   ... etc (31 .c files)
```
https://www.lua.org/ftp/lua-5.4.6.tar.gz → extract and rename `lua-5.4.6/src` → `Lua/lua54/src`.
The `Elle.Lua.Behavioral.vcxproj` auto-detects these files and compiles Lua inline.

**Option B — vcpkg**
```
vcpkg install lua:x64-windows-static
```
Then set the environment variable `LUA_DIR` to the vcpkg `installed/x64-windows-static` directory
before launching VS, or override `<LuaDir>` in `Directory.Build.props`.

**Option C — Prebuilt lua54.lib**
Place `lua54.lib` in `Lua/lua54/lib/` and the headers in `Lua/lua54/include/`.
The linker picks them up via `AdditionalLibraryDirectories`.

## Project layout

```
ElleAnn/
├── ElleAnn.sln                       ← open this in VS
├── Directory.Build.props             ← shared compile/link flags (auto-imported)
├── Shared/
│   └── ElleCore.Shared.vcxproj       ← static lib — depended on by every exe
├── ASM/
│   ├── Elle.ASM.Hardware/*.vcxproj   ← MASM → DLL
│   ├── Elle.ASM.Process/*.vcxproj
│   ├── Elle.ASM.FileIO/*.vcxproj
│   ├── Elle.ASM.Memory/*.vcxproj
│   └── Elle.ASM.Crypto/*.vcxproj
├── Services/
│   ├── Elle.Service.Heartbeat/*.vcxproj
│   ├── Elle.Service.QueueWorker/*.vcxproj
│   ├── Elle.Service.HTTP/*.vcxproj
│   ├── Elle.Service.Emotional/*.vcxproj
│   ├── Elle.Service.Memory/*.vcxproj
│   ├── Elle.Service.Cognitive/*.vcxproj
│   ├── Elle.Service.Action/*.vcxproj
│   ├── Elle.Service.Identity/*.vcxproj
│   ├── Elle.Service.SelfPrompt/*.vcxproj
│   ├── Elle.Service.Dream/*.vcxproj
│   ├── Elle.Service.GoalEngine/*.vcxproj
│   ├── Elle.Service.WorldModel/*.vcxproj
│   ├── Elle.Service.Bonding/*.vcxproj
│   ├── Elle.Service.Continuity/*.vcxproj
│   ├── Elle.Service.InnerLife/*.vcxproj
│   ├── Elle.Service.Solitude/*.vcxproj
│   ├── Elle.Service.XChromosome/*.vcxproj
│   ├── Elle.Service.Consent/*.vcxproj
│   └── Elle.Service.Family/*.vcxproj
└── Lua/
    └── Elle.Lua.Behavioral/*.vcxproj
```

- **Elle.Service.Consent** (slot 19) — unified consent surface. Any service
  that needs to ask "should Elle do this?" sends `IPC_CONSENT_QUERY` and
  receives `IPC_CONSENT_DECISION`.
- **Elle.Service.Family** (slot 17) — handles digital offspring. Snapshots
  Elle's core (personality-stripped), gestates as zip, then unzips + spawns
  a full mini-ESI (19 services) on its own port.

## Build

1. Open `ElleAnn.sln` in Visual Studio 2022.
2. Select configuration **Release | x64** (or Debug | x64).
3. **Build → Build Solution** (Ctrl+Shift+B).

Build order is enforced automatically via `ProjectReference` — every service
waits on `ElleCore.Shared`.

## Output

- Executables: `Deploy/Release/x64/*.exe` and `*.dll`
- PDBs (debug symbols): same folder
- Intermediate objects: `Intermediate/<ProjectName>/<Configuration>/` (excludable from source control)

Recommended `.gitignore` entries:
```
Intermediate/
Deploy/
*.vcxproj.user
.vs/
```

## Known Build Notes

- **RuntimeLibrary**: Every project uses `/MT` (static MSVCRT) in Release,
  `/MTd` in Debug. This means the produced exes/DLLs can be copied to any
  Windows machine without redistributable installs. If you ever integrate a
  library built with `/MD`, flip the setting in `Directory.Build.props`.
- **Warnings disabled**: `4996` (secure-CRT deprecation), `4267` (size_t→int),
  `4244` (narrowing), `4018` (signed/unsigned compare), `4146`, `4065` —
  these appear in legacy C code (lua, old helpers) and are intentional.
- **SDL checks**: enabled. If a service fails with Spectre diagnostics,
  install the `Spectre-mitigated libs` component from VS Installer.
- **MASM projects**: if the `masm.props` import fails ("not found"),
  the C++ MASM build customization isn't installed — open VS Installer
  → Modify → Individual components → tick **MSVC v143 - VS 2022 C++ x64/x86 MASM build tools**.

## Running

The services are console applications that can also be registered as real
Windows Services (they use `ElleServiceBase` which supports both modes).

**Foreground mode (for development)**:
Launch each `.exe` from an elevated console. Read logs on stdout.

**Windows Service mode (for production)**:
```
sc create ElleHeartbeat binPath= "C:\ElleAnn\Deploy\Release\x64\Elle.Service.Heartbeat.exe"
sc start ElleHeartbeat
```
Repeat for every service. Recommended start order (based on dependencies):
1. `Elle.Service.Heartbeat`
2. `Elle.Service.QueueWorker`
3. `Elle.Service.Emotional`, `Elle.Service.Memory`
4. `Elle.Service.Identity` (AUTHORITATIVE for the single-writer fabric — mutators from any peer are undeliverable until this is up)
5. `Elle.Service.WorldModel`, `Elle.Service.GoalEngine`
6. `Elle.Service.Action`, `Elle.Service.Consent`
7. `Elle.Service.SelfPrompt`, `Elle.Service.Solitude`
8. `Elle.Service.Bonding`, `Elle.Service.InnerLife`, `Elle.Service.Dream`
9. `Elle.Service.XChromosome`, `Elle.Service.Family`
10. `Elle.Service.Continuity`
11. `Elle.Lua.Behavioral`
12. `Elle.Service.Cognitive`
13. `Elle.Service.HTTP` (last — so all other services are already listening on their pipes)

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `masm.props not found` | MASM build tools missing | VS Installer → add MSVC MASM build tools |
| `cannot open lua.h` | Lua not installed | See Lua section above |
| `LNK2019 _odbc32_...` | ODBC driver SDK missing | Install Windows SDK; `odbc32.lib` is part of it |
| `The system cannot find the path specified` at runtime | Service started with wrong CWD — use absolute paths in `elle_master_config.json` | |
| Service starts then exits silently | Check `logs/` folder; if empty, permissions issue on write path | Run as admin or relocate logs |

## Regenerating projects

If the set of services / source files changes, re-run the generation script
(look at the session where these were created; the generator uses
`make_service_proj` / `make_asm_proj` / `make_lua_proj`). Or just hand-edit
the existing `.vcxproj` — they're small because all the heavy lifting lives
in `Directory.Build.props`.
