# Elle-Ann Deployment — Windows SCM Installer

## Contents
- `elle_service_manifest.json` — ordered list of all 20 executables (19 services + Lua Behavioral) with dependencies
- `Install-ElleServices.ps1`   — registers + starts every service
- `Uninstall-ElleServices.ps1` — stops + deletes them (reverse order)
- `Install.bat`   — double-click wrapper (auto-elevates)
- `Uninstall.bat` — same for removal

## Prerequisites
1. Visual Studio build produced `.exe` files for all 20 executables.
   By default the manifest expects them in `Release\x64\` relative to this folder
   (this matches `Directory.Build.props` → `<DeployDir>`).
   If your build output is elsewhere, edit `binary_root` in the JSON OR pass
   `-BinaryRoot "<abs path>"` to the PowerShell script.
2. SQL Server is reachable at the connection string in
   `elle_master_config.json` AND every delta in `SQL/` has been applied in order:
     - `ElleAnn_Schema.sql`             (base tables)
     - `ElleAnn_Identity_Schema.sql`    (identity/autobiography)
     - `ElleAnn_XChromosome_Schema.sql` (cycle/hormones/pregnancy)
     - `ElleAnn_MemoryDelta.sql`        (memory-tier columns)
     - `ElleAnn_QueueReaperDelta.sql`   (IntentQueue.ProcessingMs column)
   All deltas are idempotent (`IF NOT EXISTS`) so re-running is safe.
3. ASM DLLs (Elle.ASM.Hardware.dll etc.) sit next to the .exe files OR on the
   system PATH — the Action service dynamically loads them via LoadLibrary.

## Install
Double-click `Install.bat` (or right-click → Run as Administrator).

Manual PowerShell run if you prefer:
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

Already-registered services are skipped on create but still started if stopped.

## Uninstall
Double-click `Uninstall.bat` (or `.\Uninstall-ElleServices.ps1`).

Stops + deletes in reverse dependency order (HTTP first, Heartbeat last).
Your SQL data and logs are **not** touched — delete the DB manually if you
want a full wipe.

## Verifying the install
```powershell
sc.exe query ElleHTTPServer     # should report RUNNING
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
     └── (Memory + Emotional) ── Cognitive ── HTTP
```
