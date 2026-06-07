/*******************************************************************************
 * BUILD_NOTES.md — Compilation Guide for ELLE-ANN ESI v3.0
 * 
 * Read this before opening Visual Studio.
 ******************************************************************************/

# Build Notes — Getting Elle Running

## Prerequisites

### Required
- **Visual Studio 2022** (Community or higher) with:
  - Desktop development with C++
  - MSVC v143 build tools
  - Windows SDK 10.0.22621.0+
  - C++ MASM support (ml64.exe — included with C++ workload)

- **SQL Server Express** (2019 or 2022)
  - Enable Named Pipes protocol in SQL Server Configuration Manager
  - Create databases by running `SQL/ElleAnn_Schema.sql`
  - Then run `SQL/ElleAnn_Identity_Schema.sql`

### Optional (for full functionality)
- **Lua 5.4** — headers + lib (lua54.lib)
  - Download from lua.org or vcpkg: `vcpkg install lua:x64-windows`
  
- **llama.cpp** — for local LLM inference
  - Clone: `git clone https://github.com/ggerganov/llama.cpp`
  - Build: `cmake -B build && cmake --build build --config Release`
  - Copy: llama.lib + headers to a `deps/llama/` folder
  
- **GGUF Model** — for local inference
  - Download from huggingface.co (e.g., Llama-3-8B-Instruct.Q4_K_M.gguf)
  - Update `elle_master_config.json` → `llm.providers.local_llama.model_path`

## Build Order (Dependencies)

1. **ElleCore.Shared** (Static Library) — build first, everything depends on it
2. **ASM DLLs** (5 DLLs) — no dependencies on each other
3. **Services** (all) — depend on Shared, optionally load ASM DLLs at runtime
4. **Lua.Behavioral** — depends on Shared + Lua 5.4 lib
5. **Debug tools** — depend on Shared

## Quick Start

### Step 1: Create .vcxproj files
The .sln references projects that need .vcxproj files created. For each:
- Project Type: Console Application (services) or Static Library (Shared) or DLL (ASM)
- Platform: x64
- Include Dirs: Add `$(SolutionDir)Shared`
- Lib Dirs: Add output of ElleCore.Shared
- For services: Link against `ElleCore.Shared.lib`, `ws2_32.lib`, `odbc32.lib`, `winhttp.lib`
- For MASM projects: Custom Build Tool → ml64.exe /c /Fo"$(IntDir)%(Filename).obj" "%(FullPath)"

### Step 2: Configure
Edit `elle_master_config.json`:
```json
{
    "llm": {
        "providers": {
            "groq": {
                "api_key": "YOUR_GROQ_KEY_HERE"
            }
        }
    },
    "services": {
        "sql_pipes": {
            "connection_string": "Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-IIREG0M\\ELLEANN;Database=ElleCore;Trusted_Connection=yes;"
        }
    }
}
```

### Step 3: Database
```sql
-- In SSMS or sqlcmd:
:r SQL\ElleAnn_Schema.sql
:r SQL\ElleAnn_Identity_Schema.sql
```

### Step 4: Build & Run
1. Build ElleCore.Shared
2. Build all ASM DLLs
3. Build Elle.Service.Heartbeat first (it's the foundation)
4. Build remaining services
5. Copy all DLLs and `elle_master_config.json` to the same output directory
6. Double-click Elle.Service.Heartbeat.exe → Install as service
7. Repeat for other services (or use `--console` for debugging)

## Compilation Notes

### ODBC
The SQL connection uses ODBC. Ensure `ODBC Driver 17 for SQL Server` is installed.
Download: https://learn.microsoft.com/en-us/sql/connect/odbc/download-odbc-driver-for-sql-server

### MASM
MASM files use x64 calling convention (RCX, RDX, R8, R9 for first 4 args).
Build command: `ml64 /c /Fo"Hardware.obj" Hardware.asm`
Link with: `link /DLL /DEF:Hardware.def Hardware.obj /OUT:Elle.ASM.Hardware.dll`

### llama.cpp Integration
The `LLMLocalProvider` in ElleLLM.cpp has scaffolded integration points.
To activate:
1. `#include "llama.h"` in ElleLLM.cpp
2. Link against llama.lib
3. Uncomment the llama_* calls in `LLMLocalProvider::Initialize()` and `Generate()`

### Lua Integration
1. `#include "lua.hpp"` (or the separate headers as currently done)
2. Link against lua54.lib
3. Ensure scripts directory path in config matches actual location

## Service Startup Order
1. Heartbeat (foundation — monitors everything else)
2. QueueWorker (SQL queue bridge)
3. Emotional (needs to be running for emotion queries)
4. Memory (needs emotional state for tagging)
5. Cognitive (needs emotional + memory)
6. Action (needs cognitive for intent routing)
7. Identity (file monitoring)
8. HTTP (API server — needs everything)
9. SelfPrompt, Dream, GoalEngine, WorldModel (autonomous services)
10. LuaBehavioral (behavioral scripts)
11. Bonding, Continuity, InnerLife, Solitude (soul layer)

## Testing
- Use `--console` flag on any service for interactive debugging
- Debug tools in the Debug solution provide live monitoring
- Start with Heartbeat + one service at a time to verify IPC

## File Count
- 60 source files
- ~16,000 lines of C++/MASM/Lua/SQL
- 16 services (12 core + 4 soul layer)
- 5 MASM DLLs
- 6 Lua behavioral scripts
- 2 SQL schema files
- 1 central configuration document
