# Linux/

**This folder is dev & test scaffolding only — NOT part of the Elle-Ann system.**

The Elle-Ann runtime is pure C++ Windows services with MS ODBC SQL Server.
Nothing here runs in production. Nothing here gets shipped. Everything in
this folder exists exclusively to make the Linux ctest harness work on a
clean dev container so the four extractable header-only cores
(`Intuition`, `Probability`, `Composer`, `Language`) and the `_Shared`
helper tests can be exercised without needing a Windows host.

## What lives here

| File | Purpose |
|---|---|
| `setup_dev_env.sh` | One-shot apt install for everything CI uses. Idempotent. |
| `run_all_ctests.sh` | Configure + build + run all five Linux ctest harnesses in sequence. |

## Quick start

```bash
sudo ElleAnn/Linux/setup_dev_env.sh
ElleAnn/Linux/run_all_ctests.sh
```

Expected end state: all five harnesses green
(`Intuition`, `Probability`, `Composer`, `Language`, `_Shared`).

## What is NOT here (and why)

- **Service `CMakeLists.txt` files** stay inside each service directory
  (`Services/Elle.Service.Intuition/CMakeLists.txt`, etc.) — they declare
  *what* gets tested for that service, which is a service-local concern.
- **`Tools/*.sh` scripts** (`ipc_chain_smoke.sh`, `pass15_host_doctor.sh`,
  etc.) stay under `Tools/` — those are operational scripts that run
  against the live system (or simulate doing so), not env-prep.
- **CI workflow** (`.github/workflows/ctest-smoke.yml`) stays where GitHub
  expects it. The scripts here mirror what CI runs so local results
  match CI results.

If you find yourself wanting to put something *runtime-related* in this
folder, you're in the wrong folder — that belongs in `Services/`, `SQL/`,
or `Tools/`.
