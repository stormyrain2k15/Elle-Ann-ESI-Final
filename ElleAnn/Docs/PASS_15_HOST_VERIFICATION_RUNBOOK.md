# Pass 15 — Host-side verification runbook

Three operations the Linux container cannot run for you. Each section
lists what was pre-validated in-container, the exact commands to run
on the host, and the expected pass signal.

## 1. Apply `SQL/_Shared/01_sql_fallback_poison.sql` to `ElleCore`

### Pre-validated in-container
- Parsed cleanly into **8 GO-delimited batches** (`USE`, `SET NOCOUNT`,
  table create, 2× index create, 3× CREATE OR ALTER PROCEDURE).
- Paren balance **33 / 33**; the apparent BEGIN/END asymmetry (8/9) is
  the `CASE … END` keyword in `usp_SQLFallbackPoisonMarkReplayed`,
  not a real imbalance.
- `sqlfluff lint --dialect tsql` reports zero structural errors —
  only cosmetic `CP02` capitalisation warnings on `varchar(MAX)` etc.

### On the Windows / SQL host
```cmd
:: From repo root
sqlcmd -S "%ELLE_SQL_DSN%" -d ElleCore ^
       -i SQL\_Shared\01_sql_fallback_poison.sql -b
```

### Pass signal
- `sqlcmd` exits 0.
- `SELECT OBJECT_ID('dbo.SQLFallbackPoison','U')` returns non-NULL.
- `SELECT name FROM sys.procedures WHERE name LIKE 'usp_SQLFallbackPoison%'`
  returns 3 rows: `usp_SQLFallbackPoisonLoad`,
  `usp_SQLFallbackPoisonMarkReplayed`, `usp_SQLFallbackPoisonList`.

### Roll-back if needed
```sql
DROP PROCEDURE IF EXISTS dbo.usp_SQLFallbackPoisonList;
DROP PROCEDURE IF EXISTS dbo.usp_SQLFallbackPoisonMarkReplayed;
DROP PROCEDURE IF EXISTS dbo.usp_SQLFallbackPoisonLoad;
DROP TABLE     IF EXISTS dbo.SQLFallbackPoison;
```

## 2. Run `Tools/ipc_chain_smoke.sh`

### Pre-validated in-container
- `bash -n` clean.
- `shellcheck -S warning` clean.
- **End-to-end logic exercised against a Python stub HTTP server** that
  returns a fully-shaped `/api/ai/chat` payload — all 9 chain
  assertions fired correctly (Composer/Probability/MindManager/
  Intuition/Memory/latency/provider/model), script exit 0.

### On the Windows host (after Elle service mesh is running)
```cmd
set ELLE_HTTP_URL=http://127.0.0.1:8000
set ELLE_SQL_DSN=YOUR_DSN_HERE
set ELLE_CHAT_USER=default

bash Tools\ipc_chain_smoke.sh
```

(Use the Git-for-Windows or WSL bash. The script needs `curl` + `jq`,
both standard on Windows 10 1809+; `sqlcmd` only needed for the
optional heartbeat freshness check.)

### Pass signal
- Final line `[ipc-chain] === All assertions passed for req_id … ===`
- Exit 0.
- (Optional) Each of the 7 expected service heartbeats within 60 s.

## 3. Android `./gradlew lintDebug`

### Pre-validated in-container (no Android SDK)
- `grep -rn 'PairScreen\|PairRequest\|PairingPayload\|ElleRoutes\.PAIR\|"pair"\|pair('
  Tools/Android/app/src/main/` → **zero hits**.
- `grep -rnE 'import .*\.PairScreen$|import .*\.PairRequest$|import .*\.PairingPayload$|import .*\.PairCodeScreen$'`
  → zero orphaned imports.
- `ElleNavHost.kt` no longer references `isPaired`/`onPaired`/`prefill`.
- `MainActivity.kt` ElleNavHost call site matches the new 3-arg
  signature (`container`, `containerExtended`, `onUnpair`).
- `AndroidManifest.xml` has no `pair`/`ellepair` references.

### On the host with Android SDK
```sh
cd Tools/Android
./gradlew lintDebug
```

### Pass signal
- BUILD SUCCESSFUL.
- `app/build/reports/lint-results-debug.html` shows no new warnings
  vs the previous run.

### If you hit lint warnings from the scrub
Likely candidates:
- `Unused import` in any file that imported `PairScreen` or
  `PairingPayload` indirectly — should not occur per the grep above,
  but Kotlin sometimes pulls unused imports through.
- `Unused parameter` from the `onUnpair: () -> Unit` parameter in
  `ElleNavHost` if you never wire a logout/unpair flow.

Both are trivially fixable and don't block the build.
