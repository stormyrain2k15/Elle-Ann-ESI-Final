# Elle-Ann Deployment — Windows SCM Installer

This folder installs every Elle-Ann service into the Windows Service
Control Manager in the correct dependency order.

## Contents

| File                            | Purpose                                                       |
|---------------------------------|---------------------------------------------------------------|
| `elle_service_manifest.json`    | Ordered list of every executable + its dependencies           |
| `Install-ElleServices.ps1`      | Registers + starts every service                              |
| `Uninstall-ElleServices.ps1`    | Stops + deletes them (reverse order)                          |
| `Install.bat`                   | Double-click wrapper (auto-elevates)                          |
| `Uninstall.bat`                 | Same for removal                                              |
| `Deploy.ps1`                    | One-shot wrapper that calls Install + reports status          |
| `Deploy-OneClick.cmd` / `.vbs`  | Friendly double-click deploy from a fresh checkout            |
| `Deploy-FixODBC.ps1`            | Installs / repairs the ODBC Driver 17/18 for SQL Server       |
| `gen_serverinfo_files.py`       | Generates `ServerInfo.txt` files for FiestaData root paths    |
| `Configs/`                      | Per-service config overlays (copied next to `.exe` at install)|
| `Patches/`                      | Hotfix scripts (NTSEC ACLs, registry, etc.)                   |
| `video_worker/`                 | External Python Wav2Lip / ffmpeg worker (optional GPU sidecar)|

## Prerequisites

1. **Visual Studio build** produced every `.exe` listed in
   `elle_service_manifest.json`. By default the manifest expects them
   under `Release\x64\` relative to this folder (matches
   `Directory.Build.props` → `<DeployDir>`). Pass
   `-BinaryRoot "<abs path>"` to override.
2. **SQL Server is reachable** at the connection string in
   `elle_master_config.json`, **and every delta in `SQL/Engine/` has
   been applied in order**:
     - `ElleAnn_Schema.sql`             (base tables)
     - `ElleAnn_Identity_Schema.sql`    (identity / autobiography)
     - `ElleAnn_XChromosome_Schema.sql` (cycle / hormones / pregnancy)
     - `ElleAnn_MemoryDelta.sql`        (memory-tier columns)
     - `ElleAnn_QueueReaperDelta.sql`   (`IntentQueue.ProcessingMs` column)
     - `ElleAnn_PairedDevicesDelta.sql` (Android pairing)
     - `ElleAnn_Sessions_Delta.sql`     (multi-session continuity)
     - `ElleAnn_System_Schema.sql`      (system audit tables)
     - `ElleAnn_SchemaSync_FebPivot.sql`(Feb pivot rollup)
     - `ElleAnn_GameUnification.sql`    (game-state surface for Fiesta)
     - `imagined_and_conscience.sql`    (Imagination + MindManager tables)

   Plus the Composer seed:
     - `SQL/Elle.Service.Composer/composer_schema_seed.sql`

   All deltas are idempotent (`IF NOT EXISTS`) — re-running is safe.

3. **ASM DLLs** (`Elle.ASM.Hardware.dll` …) sit next to the `.exe`
   files **or** on the system PATH — `Elle.Service.Action` dynamically
   loads them via `LoadLibrary`.

4. **Lua sources fetched once** via `Tools\Lua\Fetch-Lua.ps1` (so
   `Elle.Lua.Behavioral.exe` can compile against `lua54/src/*.c`).

## Install

Double-click `Install.bat` (or right-click → Run as Administrator).

Manual PowerShell:

```powershell
cd .\Deploy
.\Install-ElleServices.ps1                           # register + start
.\Install-ElleServices.ps1 -NoStart                  # register only
.\Install-ElleServices.ps1 -BinaryRoot "C:\Ann\bin"  # custom bin path
.\Install-ElleServices.ps1 -Force                    # reconfigure existing services' binPath after a rebuild
.\Install-ElleServices.ps1 -WhatIf                   # dry run
```

## What it does per service

```
sc.exe create <Name> binPath= "<exe>" DisplayName= "..." start= auto depend= <deps>
sc.exe description <Name> "Elle-Ann ESI — <display>"
sc.exe failure    <Name> reset= 86400 actions= restart/60000/restart/60000/restart/60000
sc.exe start      <Name>
```

Already-registered services are skipped on `create` but still started
if stopped.

## Uninstall

Double-click `Uninstall.bat` (or `.\Uninstall-ElleServices.ps1`).

Stops + deletes in reverse dependency order (HTTP first, Heartbeat
last). Your SQL data and logs are **not** touched — delete the DB
manually if you want a full wipe.

## Verifying the install

```powershell
sc.exe query ElleHTTPServer     # should report RUNNING
sc.exe query ElleComposer       # should report RUNNING (no Composer → blank chat replies)
sc.exe query ElleIntuition      # should report RUNNING (Feb 2026 addition)
curl http://localhost:8000/api/health
```

## Service order (dependency graph)

```
  Heartbeat
     ├── QueueWorker ─── Action
     ├── Memory      ─── WorldModel
     │                 ├── Identity
     │                 ├── Continuity
     │                 ├── GoalEngine
     │                 └── Dream
     ├── Emotional   ─── InnerLife
     │                 ├── Solitude
     │                 ├── Bonding
     │                 └── SelfPrompt
     ├── Probability ─── MindManager ─── Imagination ─── Composer ─── Intuition
     ├── XChromosome ─── Family
     ├── Consent
     ├── LuaBehavioral
     ├── Fiesta   (optional — only if attaching to a ShineEngine server)
     └── (every above) ── Cognitive ── HTTP
```

## video_worker (optional sidecar)

External Python process running `wav2lip` + `ffmpeg` for video
generation. Lives in `video_worker/`. Polls Elle's HTTP service for
queued video jobs. The C++ core runs to full functionality without it;
the video endpoint returns a "worker offline" status when no worker is
polling.

Run path (typically on a GPU box):

```powershell
cd Deploy\video_worker
.\install.ps1                   # one-time: venv + deps
.\run.ps1                       # poll loop
```

See `video_worker/README.md` for the deployment contract.
