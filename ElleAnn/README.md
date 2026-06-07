# Elle-Ann ESI v3.0

Massively autonomous agentic Emotional Synthetic Intelligence.

## Repository Layout

```
ElleAnn/
├── Services/        29 C++/MASM/Lua projects (the runtime mesh)
├── Tools/            7 build / deploy / debug / data tools
├── SQL/              2 SQL script collections (engine + Language dictionary)
├── Docs/            14 narrative documents
├── ElleAnn.sln
├── Directory.Build.props
└── README.md         (this file)
```

## Build

```bat
cd ElleAnn
"%ProgramFiles%\Microsoft Visual Studio\2022\<edition>\MSBuild\Current\Bin\MSBuild.exe" ElleAnn.sln /p:Configuration=Release /p:Platform=x64
```

For the standalone CMake engines (Language + Probability):

```bash
cd ElleAnn\Services\Elle.Service.Language
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

cd ..\Elle.Service.Probability
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release
```

## Services

29 projects total under `Services/`:

| Group | Members | Purpose |
|---|---|---|
| `_Shared`              | 1 | Cross-service C++ static library (`ElleCore.Shared`) |
| `Elle.ASM.*`           | 5 | MASM-backed DLLs (Crypto, FileIO, Hardware, Memory, Process) |
| `Elle.Service.*`       | 22 | Windows services for the 21-service mesh + Fiesta headless client |
| `Elle.Service.Language`    | 1 | Integer-indexed non-LLM language engine |
| `Elle.Service.Probability` | 1 | Bayesian belief / sense / intent / trust substrate |
| `Elle.Lua.Behavioral`  | 1 | Lua runtime + scripted behavior hooks |

## Tools

7 tooling collections under `Tools/`:

| Folder        | Purpose |
|---------------|---------|
| `Android/`    | Kotlin companion phone app |
| `Debug/`      | Linux test harness (Windows.h stubs for portable compile-checks) |
| `Deploy/`     | PowerShell / CMD installers + service manifest + patches |
| `ETL/`        | WordNet → SQL Server pipeline (Python) |
| `FiestaData/` | Fiesta private-server data + reverse-engineering artifacts |
| `Lua/`        | Vendored Lua 5.4 interpreter + fetch script |
| `SHN/`        | Fiesta `.shn` file parser |

## SQL

| Folder                  | Purpose |
|-------------------------|---------|
| `Elle.Service.Language` | Canonical dictionary schema (01..09) + WordNet loader procs |
| `Engine`                | Engine-mesh schemas (Identity, XChromosome, Memory, Sessions, etc.) |

## Docs

14 narrative files: build notes, audit reports, pipeline designs, schema fix logs. Read these for context — code files contain code only.

## Coding Standards

- **Source files contain only code.** No comments, no banners, no docstrings, no commented-out experiments. Explanation lives in `Docs/` and per-service `README.md` files.
- **One service per folder under `Services/`.** Project name = folder name = solution name.
- **One tool per folder under `Tools/`.** Each carries its own README.
- **All `.vcxproj` and `CMakeLists.txt` paths are relative.** `_Shared` is reached from any service as `..\_Shared\`.

## Status (alpha test)

- Build: green (MSBuild on Windows; CMake on Linux for the two engine projects)
- Tests: `Elle.Service.Probability` 52/52 PASS via ctest
- Comment-cleanup: 1.6 MB of comment text stripped across 349 source files
- Schema: unified, no dead tables, validation view + bulk loaders in place
- ETL: WordNet → 12 schema-aligned CSVs (~100 MB), validator 0-error
