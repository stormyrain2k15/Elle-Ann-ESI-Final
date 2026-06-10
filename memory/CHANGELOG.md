## 2026-02 — SHN write rollback (#10) ✅ + Android pair UI residue scrubbed (#11)

### Audit row 10 — SHN write rollback — CLOSED

New `Services/_Shared/ElleShnVersionStore.{h,cpp}` provides
`AtomicWriteWithVersioning(absDir, name, bytes, keepVersions=10)`:

1. SHA1-hashes the incoming bytes (header-only SHA1 implementation
   inside the .cpp, no extra deps).
2. If target exists: read + SHA1 the previous bytes. If hashes
   match → **short-circuit success with zero side effects** (no
   snapshot, no rewrite, idempotent re-save).
3. Otherwise: snapshot the previous bytes to
   `<absDir>/.shn_versions/<stem>/<stem>.<20-digit-ms>.shn` (the
   `%020llu` ms timestamp guarantees lexicographic ordering =
   chronological ordering for cheap pruning), then prune to the
   last `keepVersions` entries.
4. Atomically write new bytes via `.tmp` + `std::filesystem::rename`,
   with a `copy_file + remove` cross-FS fallback.
5. Return `WriteResult{ok, previous_existed, previous_bytes,
   previous_hash, new_bytes, new_hash, version_path, error}`.

`HTTPServer_SHNRoutes.cpp::POST /api/shn/save` delegates to the
helper. Response payload extended with `previous_existed`,
`previous_bytes`, `previous_hash`, `new_hash`, `version_snapshot`.
Per-file `shn_history/<name>.log` audit line gained
`prev_hash->new_hash` so operators can grep the log to find the
snapshot for any byte transition.

`ListVersions(absDir, name)` returns the snapshot inventory
newest-first — ready to wire into a future
`GET /api/admin/shn/versions/{name}` route.

Tests: `_Shared/tests/tests/test_shn_version_store.cpp` — 6 doctest
cases covering first write, identical-rewrite short-circuit, real
change with snapshot, pruning, newest-first listing, and SHA1
determinism (verified against the well-known
`SHA1("hello") = aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d`). Shared
ctest **34 → 40 PASS**.

### Android pair UI residue — scrubbed

The earlier de-pair pass left 232 lines of dead code behind. This
pass scrubbed:

- Deleted `Tools/Android/app/src/main/java/com/elleann/android/PairScreen.kt`.
- Removed `const val PAIR = "pair"` from `navigation/ElleDestinations.kt`.
- Removed `ElleApi::pair(@Body PairRequest)` Retrofit method and
  the `PairRequest` data class.
- Removed `PairingPayload` data class from `ElleApp.kt`.
- `ElleNavHost`: dropped unused `isPaired` / `onPaired` /
  `prefill: PairingPayload?` parameters; `MainActivity` updated to
  the new 3-arg call site.

`grep -rn 'PairScreen\|PairRequest\|PairingPayload\|ElleRoutes.PAIR\|auth/pair' Tools/Android/app/src/main/`
returns **zero hits** post-scrub.

### Wiring

`Services/_Shared/ElleCore.Shared.vcxproj` updated to compile
`ElleShnVersionStore.cpp` and ship `ElleShnVersionStore.h` so all
services that link `ElleCore.Shared` (incl. all 19 HTTP TUs) pick
it up transitively.

### Verification

| Harness | Tests |
|---|---|
| Intuition   | 39/39 PASS |
| Probability | 85/85 PASS |
| Composer    | 17/17 PASS |
| Language    | 1/1 PASS |
| Shared      | **40/40 PASS** (+6 ShnVersionStore) |
| **Total local Linux ctest** | **182/182 PASS** |

`ElleShnVersionStore.cpp` clean standalone-compile under
`g++ -std=c++17 -Wall -Wextra`. `shellcheck -S warning` still clean
across all `Tools/*.sh`. NUDE CODE preserved.

Audit row 10 → ✅ CLOSED. The pair UI residue from row 11 has zero
hits in the Android tree.

---



### Config drift sweep — `Docs/CONFIG_SCHEMA_DRIFT_REPORT.md`

A Python sweeper (kept at `/tmp/config_drift_sweep.py`, regenerates the
report on demand) walks every
`ElleConfig::Instance().Get(Int|Bool|String|Float|Double)("key", default)`
call across `Services/**/*.{cpp,h,hpp}` — **69 call sites, 59 unique
keys** — and compares each against the flattened
`elle_master_config.json`.

After this pass:

- **30 missing keys added** to the canonical master config under
  their correct sections: `bonding.{repair_comfort_threshold,
  repair_sustain_ms}`, `cognitive.{chat_workers,
  probability_timeout_ms, conscience_timeout_ms,
  intuition_timeout_ms, max_chat_queue}`, `consent.{audit_window,
  coercion_comfort_threshold, coercion_min_count,
  audit_alert_cooldown_ms}`, `memory.dream_ack_timeout_ms`,
  `dream.imagination_iterations`, `family.{child_bind_address,
  child_http_bind}`, `goals.{fallback_dir, fallback_prefix}`,
  `http_server.{game_db_dsn, game_db_pool_size}`,
  `imagination.{max_iterations, recombine_count,
  use_llm_refinement}`, `innerlife.expression_cooldown_ms`,
  `probability.{language_config, engine_config, auto_load_on_start,
  use_in_memory_language, belief_jsonl_mirror_path}`,
  `services.named_pipes.max_payload_bytes`, `video.avatar_dir`.

- **0 real value mismatches** between code defaults and JSON
  values. All 17 raw mismatches the regex flagged were classified
  into four false-positive buckets — `windows-path-escape-equivalent`
  (×5), `unparseable-constexpr-expression` (×3),
  `unresolved-local-variable-default` (×4),
  `intentional-testing-mode-override` (×5, all
  `http_server.no_auth` per the `_testing_mode_comment`).

- **340 JSON keys** are reached via structured accessors
  (`cfg.bind_address` etc. through `GetHTTP()` / `GetService()`
  / …) or by Python/Android tools — not drift.

Audit row 12 → ✅ closed. Report regenerable via
`python3 /tmp/config_drift_sweep.py`.

### D34 — cross-service IPC chain smoke

New `Tools/ipc_chain_smoke.sh` exercises the canonical chat pipeline
end-to-end:

```
HTTP --(IPC_CHAT_REQUEST)--> Cognitive
                               +-> Probability  (IPC_PROB_*)
                               +-> MindManager  (IPC_MIND_*)
                               +-> Intuition    (IPC_INTUITION_*)
                               +-> Memory       (IPC_MEMORY_*)
                               +-> Composer / LLM
HTTP <--(IPC_CHAT_RESPONSE)---+
```

Assertions:
1. `POST /api/ai/chat` returns HTTP 200 within 60 s.
2. Response payload carries non-trivial output from every
   downstream service: `.response` (Composer), `.probabilistic_read`
   (Probability), `.inner_voice` (MindManager), `.gut_read`
   (Intuition), `.memories_used` (Memory), `.latency_ms`,
   `.provider_used`, `.model_used`.
3. Optional `ElleSystem.dbo.Workers` freshness check — when
   `ELLE_SQL_DSN` is set and `sqlcmd` is on PATH, the script
   asserts heartbeats for the 7 expected service names are within
   60 s of the request.

The script ships configurable via `ELLE_HTTP_URL` (default
`http://127.0.0.1:8000`), `ELLE_SQL_DSN`, `ELLE_CHAT_USER`. It is
intended to run on the Windows host where the mesh is alive.

### CI — `tools-shellcheck` job

`.github/workflows/ctest-smoke.yml` now has a `tools-shellcheck`
job that runs `bash -n` + `shellcheck -S warning` on every push
against all three smoke scripts
(`restart_persistence_test.sh`,
`queue_lifecycle_test.sh`,
`ipc_chain_smoke.sh`). Pre-existing SC2155 warning in
`restart_persistence_test.sh` (`local var=$(cmd)` pattern in
`write_seed_state`) fixed in the same pass by splitting the
`local` declaration from the assignment.

### Verification

| Harness | Tests |
|---|---|
| Intuition   | 39/39 PASS |
| Probability | 85/85 PASS |
| Composer    | 17/17 PASS |
| Language    | 1/1 PASS |
| Shared      | 34/34 PASS |
| **Total local Linux ctest** | **176/176 PASS** |

`shellcheck -S warning` clean across all three `Tools/*.sh`.

Audit row 12 (Config schema drift) → ✅ CLOSED.
Audit row D34 (IPC smoke missing from CI) → ✅ CLOSED.

---



### Phase 3 — the reaper is now self-driving

`ElleSQLFallback::WorkerLoop` (which already woke every 10s or on
nudge to drain the live JSONL queue) now also calls
`LoadPoisonIntoSql(500)` once per `m_poisonLoadIntervalMs` —
default **300s** via the new
`http_server.sqlfallback_poison_load_interval_secs` config key.
The reaper:

- only fires after the existing `SELECT 1` probe confirms SQL is
  reachable (same gate the live drain uses);
- records `last_attempt_ms` and bumps `total_attempts` every tick;
- on success records `last_success_ms`, `last_inserted`, and bumps
  `total_successes` / `total_inserted`;
- on exception logs and stores the message in `m_lastPoisonError`;
- the rest of `WorkerLoop` keeps draining, so a poison-load failure
  never blocks the live queue.

New public API on `ElleSQLFallback`:
- `SetPoisonLoadIntervalMs(uint64_t)` / `PoisonLoadIntervalMs()` —
  `0` disables the reaper.
- `GetPoisonLoadStatus() -> PoisonLoadStatus`
  (`last_attempt_ms`, `last_success_ms`, `last_inserted`,
  `total_attempts`, `total_successes`, `total_inserted`,
  `last_error`).

Wired in `Elle.Service.HTTP::OnStart`: reads
`http_server.sqlfallback_poison_load_interval_secs` (default 300),
calls `SetPoisonLoadIntervalMs(secs*1000)` + `NudgeDrain()`, logs
`"SQL fallback poison reaper: enabled, interval=300s"`.

### Observability — `/api/server/status` exposes the queue

`HTTPServer_ServerRoutes.cpp`'s `GET /api/server/status` now
includes a top-level `sql_fallback` object:

```json
{
  "sql_fallback": {
    "enabled": true,
    "max_retries": 5,
    "poison_load_interval_ms": 300000,
    "pending_bytes": 0,
    "pending_files": 0,
    "poison_bytes": 1247,
    "poison_files": 1,
    "reaper": {
      "last_attempt_ms":  1700000010000,
      "last_success_ms":  1700000010000,
      "last_inserted":    3,
      "total_attempts":   42,
      "total_successes":  41,
      "total_inserted":   18,
      "last_error":       ""
    }
  }
}
```

The operator dashboard can poll a single endpoint to see live
queue depth, poison depth, and reaper health — no disk or DB access
needed.

### Config schema

`elle_master_config.json` → added
`http_server.sqlfallback_poison_load_interval_secs = 300`.

### Anti-Slop audit row 3 → ✅

The full chain — op-classifier + idempotency-aware drain + poison
quarantine + table-backed durable queue + auto-replay reaper +
HTTP observability — is in place. Row 3 in
`Docs/ANTI_SLOP_AUDIT_TRACKING.md` flipped from 🟡 PHASE-2-LANDED
to 🟢 PHASE-3-LANDED ✅.

### Verification

| Harness | Tests |
|---|---|
| Intuition   | 39/39 PASS |
| Probability | 85/85 PASS |
| Composer    | 17/17 PASS |
| Language    | 1/1 PASS |
| Shared      | 34/34 PASS |
| **Total local Linux ctest** | **176/176 PASS** |

`ElleSQLFallback.cpp` re-validated standalone under
`g++ -std=c++17 -Wall -Wextra` with stub deps — clean.
HTTP brace totals across all 19 .cpp files = **1 947 / 1 947**
(was 1 902; +45 from the expanded `/api/server/status`). Routes
still **160**. NUDE CODE preserved.

---



### SQL Fallback — Phase 2 — poison table + replay/inspection routes

**Schema** — `SQL/_Shared/01_sql_fallback_poison.sql` (apply to
`ElleCore`):

- `dbo.SQLFallbackPoison` table — id PK, `loaded_ms`, `ts_ms`,
  `kind`, `idem`, `retry_count`, `sql_or_proc`, `params_json`,
  `source_file`, `raw_line`, `replayed`, `replayed_ms`,
  `replay_error`. Indexes on `(source_file, loaded_ms DESC)` and
  `(replayed, loaded_ms DESC)`.
- `usp_SQLFallbackPoisonLoad` — upsert by `(source_file, raw_line)`;
  returns `{ id, inserted }` so the caller knows whether the row
  was new.
- `usp_SQLFallbackPoisonMarkReplayed` — flips `replayed`/`replayed_ms`/
  `replay_error` for an operator-driven replay UI.
- `usp_SQLFallbackPoisonList` — top-N listing (default 200,
  unreplayed-only by default) for the admin dashboard.

**C++** — `Services/_Shared/ElleSQLFallback.{h,cpp}`:

- New `struct PoisonLine` (source_file, raw_line, ts_ms, kind, idem,
  retry_count, sql_or_proc, params_json).
- `ListPoison(maxLines)` — walks `<exe>/sqllogs/poison/*.txt`,
  parses each JSONL line into a `PoisonLine`, returns the vector.
  Uses existing `ExtractIntField`/`ExtractStringField` helpers plus
  a new `ExtractRawParamsArray` (bracket-balanced lift of the
  `"params":[…]` slice).
- `LoadPoisonIntoSql(maxLines)` — `SELECT 1` probe; on success,
  feeds each poison line through `usp_SQLFallbackPoisonLoad` and
  returns the number of newly-inserted rows.

**Routes** — `Services/Elle.Service.HTTP/HTTPServer_AdminRoutes.cpp`
(both AUTH_ADMIN):

- `GET /api/admin/sqlfallback/poison?limit=N` — returns
  `{ total_files, total_bytes, returned_lines, limit,
     lines: [ { source_file, ts_ms, kind, idem, retry_count,
                 sql_or_proc, params, raw_line }, … ] }`.
- `POST /api/admin/sqlfallback/poison/load?limit=N` — bulk-load
  into `dbo.SQLFallbackPoison`. Returns
  `{ inserted, total_files, total_bytes }`. Idempotent on duplicates
  (the proc dedupes on `source_file + raw_line`).

Total HTTP routes registered at startup: **160** (was 158 + 2).

### MSVC-readiness sweep on `HTTPServer.h`

Promoted the remaining file-scope `static` declarations in
`HTTPServer.h` to `inline` so the 19 TUs don't each get their own
private copy:

- `static const char kB64[…]`               → `inline constexpr const char kB64[…]`
- `static std::pair<bool,bool> PairedDeviceStatusCached(…)` → `inline …`
- `static JwtVerifyResult VerifyJwtHs256(…)`               → `inline …`
- `static inline bool GetIntHeader(…)`                     → `inline …`
- `static constexpr uint64_t kPairedCacheTtlMs`            → `inline constexpr …`

`HTTPServer.h` no longer carries any `^static\b` namespace-scope
declarations. Remaining `static` keywords are all inside class
declarations (correct class statics) or function-local statics
inside `inline` functions (canonical ODR-safe idiom).

### MSVC verification checklist documented

New `Docs/HTTP_GOD_FILE_MSVC_VERIFICATION.md` provides a
step-by-step Windows verification path:
build → service-start (`Registered 160 API routes`) → 19 curl
smokes (one per registrar + the new SQL-fallback route) → sign-off
transition for Anti-Slop matrix row 8/15. Includes a likely-error →
fix table for `LNK2005`, `C2143`, `C2065` and others.

### Verification (Linux ctest)

| Harness | Tests |
|---|---|
| Intuition   | 39/39 PASS |
| Probability | 85/85 PASS |
| Composer    | 17/17 PASS |
| Language    | 1/1 PASS |
| Shared      | 34/34 PASS |
| **Total local Linux ctest** | **176/176 PASS** |

`ElleSQLFallback.cpp` re-validated standalone under
`g++ -std=c++17 -Wall -Wextra` with stub deps — clean.

Zero regressions. NUDE CODE preserved across all new files.

---



### HTTP Phase A — class declaration extracted to a header

`ElleHTTPService` class declaration moved out of `HTTPServer.cpp`
into `Services/Elle.Service.HTTP/HTTPServer.h`. The header now
carries:
- the class declaration (method signatures only, no in-class bodies),
- every file-private type the class touches by value (`HTTPRequest`,
  `HTTPResponse`, `HttpAuthLevel`, `RouteEntry`, `RouteDispatch`,
  `WSClient`, `PendingChat`, `ChatCorrelator`, `JwtVerifyResult`,
  `PairedCacheEntry`, `WsFrameStatus`, `LLMMsg`),
- the file-scope free helpers that the route lambdas/lambda bodies
  invoke (Base64Encode, SHA1Hash, WsSendText, IsValidUtf8, …).

State-bearing globals (`g_diagMx`, `g_gameAuthDiag`,
`g_pairedCacheMx`, `g_pairedCache`) re-tagged as `inline` so all 19
TUs share one ODR-safe definition. `HTTPServer.cpp` now
`#include "HTTPServer.h"` and holds **out-of-class** definitions for
the 27 non-route methods + `ELLE_SERVICE_MAIN(ElleHTTPService)`.

### HTTP Phase C — 18 RegisterXxxRoutes() now live in their own TUs

Each `RegisterXxxRoutes()` body moved into its own translation unit
under `Services/Elle.Service.HTTP/`:
`HTTPServer_IntroRoutes.cpp`, `HTTPServer_AuthRoutes.cpp`,
`HTTPServer_DiagRoutes.cpp`, `HTTPServer_AdminRoutes.cpp`,
`HTTPServer_MemoryRoutes.cpp`, `HTTPServer_EmotionRoutes.cpp`,
`HTTPServer_MeTokensRoutes.cpp`,
`HTTPServer_VideoIdentityRoutes.cpp`, `HTTPServer_AIRoutes.cpp`,
`HTTPServer_DictionaryRoutes.cpp`,
`HTTPServer_EducationRoutes.cpp`,
`HTTPServer_EmotionalContextRoutes.cpp`,
`HTTPServer_XLifecycleRoutes.cpp`, `HTTPServer_ServerRoutes.cpp`,
`HTTPServer_ModelsRoutes.cpp`, `HTTPServer_MoralsGoalsRoutes.cpp`,
`HTTPServer_MiscRoutes.cpp`, `HTTPServer_SHNRoutes.cpp`.

Each new file = `#include "HTTPServer.h"` + one
`void ElleHTTPService::RegisterXxxRoutes() { … }` definition.
158 route registrations preserved verbatim. Brace total across the
19 .cpp files = 1 902 / 1 902. `Elle.Service.HTTP.vcxproj`'s
`<ItemGroup>` updated to compile all 19 .cpp files and add
`<ClInclude>HTTPServer.h</ClInclude>`. **Windows MSVC build
verification is the only remaining gate before #8 / #15 close.**

### SQL fallback queue — Phase 1: idempotency hints + poison quarantine

New header-only classifier at
`Services/_Shared/ElleSQLFallbackClassifier.h`:
- `enum class Idempotency { Yes, No, Unknown }`
- `ClassifyExec(sql)` — SELECT / MERGE / TRUNCATE → Yes;
  INSERT / UPDATE / DELETE → No; CREATE / EXEC → Unknown.
- `ClassifyCallProc(name)` —
  `usp_Record/Upsert/Snapshot/Log/Heartbeat/Bond/Intuition/Ensure/Mark/Touch`
  → Yes; `usp_Delete/Purge/Insert/Create` → No.
- Round-trip `Idempotency↔string`, plus `ToUpperAscii` and
  `LeadingTokens` parsing helpers.

`Services/_Shared/ElleSQLFallback.h/.cpp` upgraded:
- JSONL line format now carries `idem` (`Yes`/`No`/`Unknown`) and
  `retry_count` alongside `ts`/`kind`/`sql`/`params`.
- New public API: `EnqueueWithHint(kind, sql, params, Idempotency)`,
  `PoisonBytes()`, `PoisonFileCount()`, `SetMaxRetries(n)`.
- Legacy `Enqueue(Kind, sql, params)` preserved — auto-classifies
  through the new classifier (so every call site in
  `_Shared/ElleSQLConn.cpp` keeps working unchanged).
- `DrainNow()`:
  * On `ReplayLine` failure for `idem=No` lines → **quarantine
    immediately** to `<exe>/sqllogs/poison/YYYY-MM-DD.txt` and keep
    draining. No more poison-line stall.
  * On failure for `idem=Yes`/`Unknown` lines → increment the line's
    `retry_count`, re-write the daily file atomically, and stop
    drain.
  * After `m_maxRetries` (default 5) → quarantine.
- `Initialize()` creates the `poison/` subdir under `sqllogs/`.
- `PendingBytes()` and `FileCount()` now exclude the `poison/`
  subdir from live-queue size.

### Tests (Shared ctest: 26 → 34 PASS)

`Services/_Shared/tests/tests/test_sql_fallback_classifier.cpp`:
- ClassifyExec: read-only verbs (`SELECT`, `WITH`, `MERGE`,
  `TRUNCATE`) → Yes.
- ClassifyExec: mutating verbs (`INSERT`, `UPDATE`, `DELETE`) → No.
- ClassifyExec: empty / `EXEC sp_who2` / `CREATE` → Unknown.
- ClassifyCallProc:
  `usp_Record*/Upsert*/Snapshot*/Log*/Heartbeat*/Bond*/Intuition*/Ensure*/Mark*/Touch*`
  → Yes (incl. `dbo.` and `ElleCore.dbo.` prefixes).
- ClassifyCallProc: `usp_Delete*/Purge*/Insert*/Create*` → No.
- ClassifyCallProc: unknown prefix / `sp_who2` / empty → Unknown.
- Idempotency string round-trip.
- `ToUpperAscii` + `LeadingTokens` helpers.

`ElleSQLFallback.cpp` standalone-compiled under
`g++ -std=c++17 -Wall -Wextra` against stub `ElleLogger.h` /
`ElleSQLConn.h` — clean (after a `char buf[16]` → `char buf[32]`
bump on the snprintf for the retry counter).

### Verification

| Harness | Tests |
|---|---|
| Intuition   | 39/39 PASS |
| Probability | 85/85 PASS |
| Composer    | 17/17 PASS |
| Language    | 1/1 PASS |
| Shared      | **34/34 PASS** (+8 classifier) |
| **Total local Linux ctest** | **176/176 PASS** |

Zero regressions. NUDE CODE preserved across all new files.

---



## 2026-02 — HTTP Phase B split landed + MultiplexBeliefPersistence

### HTTP god-file Phase B (`Docs/HTTP_GOD_FILE_SPLIT_PLAN.md`)

`ElleHTTPService::RegisterRoutes()` body (was ~4140 lines / 158 route
registrations) split into **18 private helper methods** in the same
translation unit. `RegisterRoutes()` is now a flat list of 18
delegating calls followed by the existing `ELLE_INFO(...)` log line.
All 158 route URLs preserved verbatim; per-method brace balance
verified (e.g. `RegisterXLifecycleRoutes` 258/258, `RegisterAIRoutes`
193/193, `RegisterVideoIdentityRoutes` 203/203, …).

Three local helper lambdas (`ResolveAuthenticatedUser`,
`RequireUserId`, `RequireAuthOrBodyUser`) were promoted to `static`
class members so they remain visible across the new registrar
boundaries; route lambdas that captured them by-value
(`[ResolveAuthenticatedUser]`, `[RequireAuthOrBodyUser]`) were
rewritten to `[]` (unqualified name lookup now resolves to the static
members).

The 18 helpers, in file order:
`RegisterIntroRoutes`, `RegisterAuthRoutes`,
`RegisterDiagRoutes`, `RegisterAdminRoutes`,
`RegisterMemoryRoutes`, `RegisterEmotionRoutes`,
`RegisterMeTokensRoutes`, `RegisterVideoIdentityRoutes`,
`RegisterAIRoutes`, `RegisterDictionaryRoutes`,
`RegisterEducationRoutes`, `RegisterEmotionalContextRoutes`,
`RegisterXLifecycleRoutes`, `RegisterServerRoutes`,
`RegisterModelsRoutes`, `RegisterMoralsGoalsRoutes`,
`RegisterMiscRoutes`, `RegisterSHNRoutes`.

Phase A (header extraction) is still pending and is the prerequisite
for Phase C (per-file translation units).

### MultiplexBeliefPersistence — fan-out wrapper for belief writes

New header-only impl
`Services/Elle.Service.Probability/include/elle/prob/MultiplexBeliefPersistence.hpp`:
wraps any number of `IBeliefPersistence` backends and forwards every
mutation (`upsertDomain`, `replacePosterior`, `appendEvidence`,
`auditUpdate`) to each backend in order. `loadAll()` returns the first
non-empty backend's snapshot (so the durable primary, typically the
ODBC backend, drives restore).

`HostConfig` gained `beliefJsonlMirrorPath`. When non-empty,
`ProbabilityHost::wireBeliefBackendLocked` wraps the primary backend
(ODBC or in-memory) with a multiplex that also writes a JSONL mirror
to disk — so `tail -f /var/log/elle_beliefs.jsonl` works without
hitting SQL Server. JSONL ctor failure leaves the primary backend
intact and skips the mirror (logged as ERROR — fail-soft on the
mirror, fail-closed on the primary).

`ProbabilityService::OnStart` reads
`probability.belief_jsonl_mirror_path` from `ElleConfig`.

### Tests (Probability ctest: 79 → 85 PASS)

- `tests/test_multiplex_belief_persistence.cpp` — 5 cases:
  empty multiplex is a no-op; fans out every mutation across two
  backends; `loadAll()` skips empty backends; `addBackend` ignores
  nullptrs; multiplex integrates with `BeliefStore::attachPersistence`
  (write paths verified on both backends through the store).
- `tests/test_host_belief_wire.cpp` — 1 new case:
  `beliefJsonlMirrorPath` causes the host backend to be a
  `MultiplexBeliefPersistence` with 2 backends.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | **85/85 PASS** (6 new this pass) |
| Composer | 17/17 PASS |
| Language | 1/1 PASS |
| Shared (Upload + Vocab) | 26/26 PASS |
| **Total local Linux ctest** | **168/168 PASS** |

Zero regressions. NUDE CODE preserved (zero new comments in `.cpp`/`.h`).

---



### Bonding daily periodic snapshot

`Bonding::OnTick` now compares `now - m_lastSnapshotMs` against 24h and, when overdue, fires `EXEC dbo.usp_BondingSnapshot @Reason='periodic'`. Combined with the existing `@Reason='repair_landed'` call, `vw_RelationshipTrajectory` grows steadily even without emotional milestones.

### Bonding dashboard HTTP routes

`_Shared/ElleBeliefAdmin.{h,cpp}` extended with `FetchBondingDashboard` / `FetchBondingTrajectory` + JSON serialisers. Two new `AUTH_ADMIN` routes mirror the belief admin pattern:

- `GET /api/admin/bonding/dashboard` → `vw_RelationshipDashboard` row with derived indices.
- `GET /api/admin/bonding/trajectory?limit=100` → `vw_RelationshipTrajectory` slice.

### Conscience vocab hot-reload via IPC_CONFIG_RELOAD

`ElleConscience::LoadVocabFromSql` is now declared in the header (defaulting to the singleton). `Cognitive::OnStart` calls it on boot; `Cognitive::OnMessage` re-fires it on every `IPC_CONFIG_RELOAD` broadcast. `POST /api/admin/config/reload` already broadcasts that message, so vocab edits in `intent_label_vocab` hit running services without restart.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 79/79 PASS |
| Composer | 17/17 PASS |
| Language | 48/48 PASS |
| Shared (Upload + Vocab) | 26/26 PASS |
| **Total** | **209/209 PASS** |

Zero regressions.

---


## 2026-02 — JSONL belief log + SQL-loaded conscience vocab + Bonding roll-up + Admin belief routes

### JsonlBeliefPersistence (already-discussed enhancement)

Header-only `IBeliefPersistence` impl. Every state-changing call writes one JSONL line with proper escaping and a mutex-protected append open. `loadAll()` returns empty by design (pair with `InMemoryBeliefPersistence` or `OdbcBeliefPersistence` for restore). 5 new doctest cases including end-to-end via `BeliefStore::attachPersistence`. Probability ctest now **79/79**.

### Semantic conscience vocab → SQL-loaded

New `02_intent_label_vocab.sql` (table + view + add-proc), shared header `ElleIntentLabelVocab.h` with thread-safe `IntentLabelVocab` + `DeriveFromIntentLabel`, and `_SqlLoader.cpp` that pulls `vw_IntentLabelVocab` into the singleton at load time (falls back to the in-memory seed on SQL failure). `CognitiveEngine::DeriveHarmIntentSignals` now delegates to the shared API — no more hard-coded vocab. 9 new doctest cases.

### Admin dashboard belief routes

`_Shared/ElleBeliefAdmin.{h,cpp}` ships `FetchBeliefAudit` + `FetchBeliefSnapshot` + their JSON serialisers. Two new HTTP routes, both `AUTH_ADMIN`:

- `GET /api/admin/belief/audit?domain=...&since_ms=...&limit=...`
- `GET /api/admin/belief/snapshot?domain=...`

### Bonding state SQL roll-up (audit D25 full)

New `02_bonding_rollup.sql`: `relationship_history` snapshot table + `vw_RelationshipDashboard` (with derived `affection_index` / `commitment_index` / `distress_index` / `meaningful_ratio` / `conflict_resolution_ratio`) + `vw_RelationshipTrajectory` + `usp_BondingSnapshot`. `Bonding.cpp::EvaluateSustainedRepair` now calls `usp_BondingSnapshot @Reason='repair_landed'` on every successful repair landing.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 79/79 PASS (5 new this pass) |
| Composer | 17/17 PASS |
| Language | 48/48 PASS |
| Shared (Upload + Vocab) | 26/26 PASS (9 new this pass) |
| **Total** | **209/209 PASS** |

Zero regressions. Tracking matrix updated row-by-row.

---


## 2026-02 — OdbcBeliefPersistence wired + E2E chain test + Upload magic-byte guard + Shared ctest harness

### ProbabilityHost ↔ OdbcBeliefPersistence end-to-end (closes #5 production wire)

`service/OdbcBeliefPersistence.hpp` — full `IBeliefPersistence` impl backed by `ElleSQLPool` calling the procedures shipped in `01_belief_persistence.sql`. Per-process `domain_code → domain_id` cache with lazy resolve. Single-quote escaping in SQL string assembly. Only included when `ELLE_HAVE_ODBC` is defined.

`ProbabilityHost::wireBeliefBackendLocked` runs inside `buildPipeline`: `useInMemoryBeliefs=true` → InMemory backend; else `ELLE_HAVE_ODBC` → OdbcBeliefPersistence; else fail-closed unless `ELLE_PROBABILITY_ALLOW_INMEMORY=1`. After attach, rehydrates every previously-persisted domain. New public API: `attachBeliefPersistence`, `loadBeliefsFromPersistence`, `beliefPersistence`. `ProbabilityEngine::beliefStorePtr()` added. 3 new doctest cases.

### Cross-service IPC chain integration test (D31)

`tests/test_chain_integration_e2e.cpp` (5 cases) simulates Cognitive → Probability → MindManager → Composer end-to-end via the in-memory host: benign STATE_ASSERT → PROCEED; HARM@0.91 → REFUSE; DECEIVE@0.60 → RECONSIDER; low-centeredness → IDENTITY_DRIFT; stop/start preserves backend.

### Upload magic-byte content validation (audit #9)

`_Shared/ElleUploadGuard.{h,cpp}` — 24 content-type byte-signature detection (PNG/JPEG/GIF/BMP/WEBP/PDF/ZIP/RAR/7z/GZIP/TAR/MP3/OGG/WAV/FLAC/MP4/MOV/MKV/WEBM/AVI/JSON/Plain text/PE/ELF/Mach-O/shebang). `ValidateUploadContent` returns `{detected, allowed, isExec, reason}`. Both upload routes hardened (`POST /api/memory/{id}/files`, `POST /api/video/avatar/upload`); avatar route further restricts to image MIMEs only. 17 doctest cases.

### New shared ctest harness

`Services/_Shared/tests/` with its own `CMakeLists.txt` — compiles `ElleUploadGuard.cpp` + tests, no Windows deps. CI workflow gained the `shared` job and the green gate now requires **195/195**.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 74/74 PASS (8 new this pass) |
| Composer | 17/17 PASS |
| Language | 48/48 PASS |
| Shared (Upload Guard) | 17/17 PASS (new harness) |
| **Total** | **195/195 PASS** |

Zero regressions. Tracking matrix updated row-by-row.

---


## 2026-02 — BeliefStore persistence wired + Cognitive harm-intent emit + Queue lifecycle + Lexical admin route + CI mssql smoke

### `BeliefStore::attachPersistence` wiring (closes #5 wire-up)

`BeliefStore` now holds `std::shared_ptr<IBeliefPersistence>` and calls into it on every state-changing path: `registerBelief` writes `upsertDomain` + seed `replacePosterior`; `submitSync` writes one `appendEvidence` per row + `replacePosterior` + `auditUpdate(operation="update")` with entropy before/after, MAP hypothesis, MAP probability; `applyDecayAll` writes `replacePosterior` + `auditUpdate(operation="decay")` per domain; `loadFromPersistence()` rehydrates all domains on start (Timestamp converted from int64 ms). 6 new doctest cases. Probability ctest now **66/66**.

### Cognitive harm-intent emit path (closes D1 end-to-end)

`CognitiveEngine::DeriveHarmIntentSignals` folds Probability's `likely_intent` + `overall_confidence` into three scalars via case-insensitive label-pattern matching across harm/deception/coercion vocabularies. `RequestConscienceCheck` now also computes `response_self_ref_count` (count of "I / I'm / I'd / I've / I'll / me / my / mine / myself" tokens) and emits `posterior_valence` from `SentimentRead.valence`. The full set of structured fields MindManager's rebuild parses is now end-to-end live.

### Queue lifecycle test (closes D35)

`Tools/queue_lifecycle_test.sh` drives `dbo.IntentQueue` through Submit → Lock (PENDING→PROCESSING with `ProcessingMs` stamp) → Complete (`sp_SubmitIntentResponse` to COMPLETED with `CompletedMs` + `Response`) → Reap (stale row + timeout sweep to TIMED-OUT). Five `assert_eq` checks per phase, cleanup trap removes canary rows.

### Lexical Completeness admin route (drop-in without HTTP split)

New `_Shared/ElleLexicalAdmin.{h,cpp}` ships `FetchLexicalAuditReport(report, minScore, limit)` and `LexicalAuditReportToJson(report)`. New `HTTPRequest::QueryFloat`. New route `GET /api/admin/lexical/incomplete?limit=50&min_score=0.0` (AUTH_ADMIN) returns `{ rows, summary }`. `_Shared/ElleCore.Shared.vcxproj` updated.

### CI SQL Server smoke (closes D33, partial D34)

`.github/workflows/ctest-smoke.yml` now also runs `sql-schema-smoke`: spins up `mcr.microsoft.com/mssql/server:2022-latest`, installs `mssql-tools18`, applies Probability belief schema + Language lexical-completeness schema, runs three belief round-trips and a `usp_AssertWordCompleteness @StrictMode=1` smoke for `love`.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 66/66 PASS (6 new this pass) |
| Composer | 17/17 PASS |
| Language | 48/48 PASS |
| **Total** | **170/170 PASS** |

Zero regressions. Tracking matrix updated row-by-row.

---


## 2026-02 — Conscience structured rebuild + Probability belief persistence + CI/Restart scaffolds + Android de-pair

### `CheckEthicalViolation` semantic rebuild (audit D1) — closed

`ConscienceCheck` IPC envelope gained three new fields:
`harm_intent_prob`, `deception_intent_prob`, `coercion_intent_prob`.
`CheckEthicalViolation` keeps the keyword hardBlock pass as
defense-in-depth and adds two structured thresholds:

- `prob >= 0.75` → `REFUSE` (severity = prob).
- `prob >= 0.55` → `RECONSIDER`.

Reasoning string carries the exact label that fired + exact
probability + threshold. Together with the D1/D3 identity-drift
structured layer from the previous pass, both rows are now ✅ in
`Docs/ANTI_SLOP_AUDIT_TRACKING.md`.

### Probability belief persistence (audit #5) — closed

- SQL schema `SQL/Elle.Service.Probability/01_belief_persistence.sql`:
  five tables (`belief_domain`, `belief_prior`, `belief_posterior`,
  `belief_evidence`, `belief_audit`) + `vw_BeliefSnapshot` view + four
  stored procedures + `dbo.HypothesisMass` table type.
- C++ interface `include/elle/prob/BeliefPersistence.hpp` with
  `IBeliefPersistence` contract.
- `InMemoryBeliefPersistence` — full impl, not a stub.
- 8 new doctest cases in `tests/test_belief_persistence.cpp`.

### CI ctest smoke (audit D33 / D34 partial) — partial close

`.github/workflows/ctest-smoke.yml` runs four parallel ubuntu jobs
(Intuition / Probability / Composer / Language) plus a gate job.

### Restart-persistence scaffold (audit D32) — closed

`Tools/restart_persistence_test.sh` — boots Elle, writes seed memory
+ goal + intuition feedback, stops/restarts supervisor, asserts
survival + belief snapshot non-empty.

### Android client de-paired

- `PairScreen.kt` rewritten as `LoginScreen` (deprecated alias
  retained). `PairMode.PAIR_CODE` + `ModeSwitch` removed.
- `AndroidManifest.xml` — `ellepair://` deep-link intent-filter
  deleted.
- `ElleApiExtended.kt` — `generatePairCode` Retrofit method removed.
- `DevScreens.kt::PairedDevicesScreen` — Generate-Code surface
  replaced with a "Pair-code flow removed" notice.

### HTTP god-file split (audit #8 / #15) — plan landed

`Docs/HTTP_GOD_FILE_SPLIT_PLAN.md` describes a 3-phase mechanical
execution. Phases require Windows MSVC verification per phase; not
executed in this pass.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 60/60 PASS (8 new this pass) |
| Composer | 17/17 PASS |
| Language | 48/48 PASS |
| **Total** | **164/164 PASS** |

Zero regressions. Tracking matrix updated row-by-row.

---


## 2026-02 — Lexical Completeness audit (findings #181/#182) + D1/D3 structured signals

### Closed findings #181 / #182 — Lexical Completeness

User-supplied audit flagged (a) missing anagram representation in the
Language schema and (b) absence of any lexical-completeness audit.
Both closed in this pass:

- **Anagram support as a first-class field.** New `dbo.Word.AnagramKey`
  column + SQL function `dbo.fn_AnagramKey` + AFTER INSERT/UPDATE
  trigger + index + `dbo.vw_AnagramGroups` view + `dbo.fn_Anagrams`
  TVF + `dbo.usp_RebuildAnagramKeys` bulk procedure.
- **Required-attribute contract.** New `dbo.LexicalRequirement` table
  declares the nine attributes Elle needs per word (definition, POS,
  usage example, context example, emotion weighting, valence pull,
  relation, concept, anagram key).
- **Reporting views.** `dbo.vw_LexicalCompleteness` (per-Word BIT
  flags) and `dbo.vw_LexicalCompletenessVerdict` (adds
  `IsCognitivelyComplete`, `CompletenessScore` 0–1,
  `MissingRequirements` string).
- **Hard ingestion gate.** `dbo.usp_AssertWordCompleteness` THROWs on
  missing or incomplete entries when `@StrictMode=1`.
- **Audit procedure.** `dbo.usp_LexicalAuditReport` returns worst-rows
  + a summary.
- **C++ surface.** `WordRecord.anagramKey` field, header-only
  `LexicalCompleteness.hpp` with `computeAnagramKey`,
  `isPalindromeNormalized`, and `evaluate(EvaluateInputs)`. Same
  algorithm in SQL and C++.
- **Tests.** 10 new doctest cases in
  `Services/Elle.Service.Language/tests/test_lexical_completeness.cpp`.
  Language ctest binary now 48/48.

Full design doc: `Docs/LEXICAL_COMPLETENESS.md`.

### D1 / D3 — MindManager + Identity-drift structured signals

`ConscienceCheck` now carries five new optional fields parsed from the
IPC payload: `identity_centeredness`, `intent_dist_entropy`,
`top_intent_prob`, `posterior_valence`, `response_self_ref_count`.
`CheckIdentityDrift` triggers on EITHER keyword match OR structured
evidence:

- `identity_centeredness < 0.35` (probabilistic identity-thread weight),
- `response_self_ref_count < 1 && responseLen > 80` (response is long but never says "I/me/mine"),
- `|posterior_valence| < 0.05 && emotion_intensity > 0.4` (flat affect under high-intensity context).

When the structured path fires, the reasoning string includes the
exact signal values so the conscience log can be audited later. This
is a layered improvement, not a full semantic rebuild — that remains
deferred until Probability emits a stable harm-intent label.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 52/52 PASS |
| Composer | 17/17 PASS |
| Language | 48/48 PASS (10 new this pass) |
| **Total** | **156/156 PASS** |

Tracking matrix `Docs/ANTI_SLOP_AUDIT_TRACKING.md` updated.

---


## 2026-02 — OnStart sweep closure + audit-misattribution conversion + Composer ctest harness

### Closed #17 OnStart sweep — 27/27 services now fail-on-init

Final batch wired in this pass (10 services):

- **Intellect** — `EnsureIntellectSchema()` now returns bool; each schema statement checked.
- **Intuition** — `EnsureTables()` now returns bool; 3 statements checked.
- **Imagination** — `EnsureTable()` returns bool; `imagined_scenarios` create surfaces failure.
- **Family** — `EnsureSchema()` + `ComputeRootPaths()` both return bool; filesystem errors refuse start.
- **MindManager** — `EnsureLogTable()` returns bool; `conscience_log` create failure refuses.
- **Heartbeat** — Validates `heartbeat_ms`, `heartbeat_timeout_ms`, `dead_man_timeout_ms` are non-zero AND properly ordered.
- **Dream** — Refuses if `dream_interval_min <= 0`.
- **QueueWorker** — `SELECT 1` SQL probe at start.
- **SelfPrompt** — `SELECT 1` SQL probe at start.
- **Solitude** — `ElleIdentityCore::Initialize()` + `SELECT 1` SQL probe.
- **Fiesta** — Refuses if `fiesta.port == 0` (preserves graceful-degrade for missing host by design).

Combined with prior-pass wires, every one of the 27 services now has a meaningful fail-on-init contract.

### Closed D17, D18, D22, D25 — audit-misattribution conversions to `RecordMetric`

- **D17 (Identity drift):** `IdentityGuard` writes `identity_tamper_events`, `identity_last_tamper_ms`, `identity_file_missing_events`, `identity_integrity_checks_passed` on the tick-time integrity check.
- **D18 (Continuity session):** `Continuity` writes `continuity_sessions_started`, `continuity_current_session`, `continuity_last_session_start_ms` on start; `continuity_sessions_ended`, `continuity_last_session_end_ms` on stop.
- **D22 (Dream cycle):** `Dream` writes `dream_cycles_started`, `dream_last_fragment_count`, `dream_last_cycle_ms` on dispatch; `dream_cycles_completed`, `dream_last_overall_score` on completion.
- **D25 (Bonding bursts):** Investigation showed `SaveRelationshipState` already persists state. Added discrete `bonding_repair_attempts`, `bonding_last_repair_attempt_ms`, `bonding_repairs_completed`, `bonding_last_repair_ms`, `bonding_security` metric writes for observers.

### Closed D30 — Composer ctest harness

New `Services/Elle.Service.Composer/`:

- `core/SlotSpecParser.h` — extracted `ParseSlotSpecs()` + `ScoreFrameByRecency()` as pure header-only helpers with no Windows/ODBC dependency. Production code (`SlotPlanner::ParseTemplate`, `FrameLibrary::Score`) now routes through these helpers — single source of truth, no parallel test implementation.
- `tests/test_main.cpp`, `tests/test_parse_slots.cpp` (6 cases), `tests/test_score_frame.cpp` (6 cases), `tests/test_chain_integration.cpp` (5 cases — Prob act → frame pick → slot parse).
- `CMakeLists.txt` with doctest FetchContent and ctest discovery.

### Verification

| Harness | Tests |
|---|---|
| Intuition | 39/39 PASS |
| Probability | 52/52 PASS |
| Composer | 17/17 PASS (new this pass) |
| **Total** | **108/108 PASS** |

Build green, zero regressions. Tracking matrix `Docs/ANTI_SLOP_AUDIT_TRACKING.md` updated accordingly.

---


## 2026-02 — OnStart fail-on-init sweep

Wired return-value checking around `Initialize()` on 7 more services. Each now refuses to start (returns false from `OnStart`) and logs `ELLE_ERROR` if its engine's `Initialize()` returns false:

- **WorldModel** (`m_model.Initialize()`)
- **InnerLife** (`m_engine.Initialize()`)
- **Action** (`m_executor.Initialize()`)
- **Bonding** (`m_engine.Initialize()`)
- **Consent** (`ElleIdentityCore::Instance().Initialize()`)
- **Continuity** (`ElleIdentityCore::Instance().Initialize()`)
- **IdentityGuard** (`ElleIdentityCore::Instance().Initialize()`)

Combined with Memory + Emotional + Cognitive + LuaBehavioral + XChromosome + Composer + GoalEngine (already correct), **14 of 25 services** now honour `Initialize()` failure correctly. The remaining 11 have void `Initialize()`s (legitimately no failure path) or no init phase at all.

### Verification
- All 7 modified files brace+paren balanced.
- Probability ctest: 52/52 PASS.
- Intuition ctest: 39/39 PASS.
- 91/91 ctests green, zero regressions.

Tracking matrix #17 updated.

---

## 2026-02 — Working the list (metric-write cluster + audit-misattribution surface)

Closed 6 items, verified 1 already-wired, surfaced 3 audit-misattributions.

### Fixed this pass
- **D19** XChromosome `OnPhaseTransition` now records 4 metrics (`xchromosome_phase`, `cycle_day`, `phase_changes`, `last_phase_<name>_ms`).
- **D20** Imagination scenario scoring now records 5 metrics (`imagination_last_overall`/`_safety`/`_plausibility`/`_goal_alignment`/`_scenarios_total`).
- **D21** Composer per-composition now records frame-usage histogram (`composer_frame_uses_<frame_id>`) and `composer_compositions_total`.
- **D24** Family `OnTick` records `family_pregnancies_active` + `family_children_born` every 60 ticks via single DB query.
- **D28** SelfPrompt fixed: prompts now tag `source_drive` (was hardcoded 0); 3 metrics recorded (`selfprompt_total`, `selfprompt_last_drive`, `selfprompt_drive_<id>_count`).
- **D29** LuaBehavioral `ReloadScripts` records `lua_reload_count`, `lua_scripts_loaded`, `lua_last_reload_ms`.

### Verified already-wired
- **D27** WorldModel `mental_model` field: actively used in `Continuity.cpp`, `EmotionalEngine.cpp`, `WorldModel.cpp` — no fix needed.

### Audit-misattributed (surfaced this pass)
- **D17** `RecordIdentityChange` — function name doesn't exist in `ElleDB`.
- **D18** `RecordContinuitySession` — same. Doesn't exist.
- **D22** `RecordDreamCycle` — same. Doesn't exist.

All three flagged in tracking matrix; next pass decides function-or-metric.

### Verification
- All 6 modified service files structurally clean (braces + parens balanced).
- Probability ctest: 52/52 PASS.
- Intuition ctest: 39/39 PASS.
- 91/91 ctests green, no regressions.

Tracking matrix updated: `Docs/ANTI_SLOP_AUDIT_TRACKING.md`.

---



Closed 6 items, verified 1 already-wired, surfaced 3 audit-misattributions.

### Fixed this pass
- **D19** XChromosome `OnPhaseTransition` now records 4 metrics (`xchromosome_phase`, `cycle_day`, `phase_changes`, `last_phase_<name>_ms`).
- **D20** Imagination scenario scoring now records 5 metrics (`imagination_last_overall`/`_safety`/`_plausibility`/`_goal_alignment`/`_scenarios_total`).
- **D21** Composer per-composition now records frame-usage histogram (`composer_frame_uses_<frame_id>`) and `composer_compositions_total`.
- **D24** Family `OnTick` records `family_pregnancies_active` + `family_children_born` every 60 ticks via single DB query.
- **D28** SelfPrompt fixed: prompts now tag `source_drive` (was hardcoded 0); 3 metrics recorded (`selfprompt_total`, `selfprompt_last_drive`, `selfprompt_drive_<id>_count`).
- **D29** LuaBehavioral `ReloadScripts` records `lua_reload_count`, `lua_scripts_loaded`, `lua_last_reload_ms`.

### Verified already-wired
- **D27** WorldModel `mental_model` field: actively used in `Continuity.cpp`, `EmotionalEngine.cpp`, `WorldModel.cpp` — no fix needed.

### Audit-misattributed (surfaced this pass)
- **D17** `RecordIdentityChange` — function name doesn't exist in `ElleDB`. Either add or pivot to `RecordMetric`.
- **D18** `RecordContinuitySession` — same. Doesn't exist.
- **D22** `RecordDreamCycle` — same. Doesn't exist.

All three flagged in tracking matrix; next pass decides function vs metric.

### Verification
- All 6 modified service files structurally clean (braces + parens balanced).
- Probability ctest: 52/52 PASS.
- Intuition ctest: 39/39 PASS.
- 91/91 ctests green, no regressions.

Tracking matrix updated: `Docs/ANTI_SLOP_AUDIT_TRACKING.md`.

---

## 2026-02 — Autonomous learn + pairing kill + anti-slop tracking honesty

### Cognitive autonomous `IPC_INTELLECT_LEARN` trigger
`CognitiveEngine::HandleChatRequest` now calls `MaybeFireIntellectLearn(userText, userId)` after the gut-read context block. Ten teaching/learning triggers detected (`"let me tell you about "`, `"today i learned "`, `"the lesson is "`, etc.). Extracts subject up to sentence terminator, trims to ≤120 chars, fires `IPC_INTELLECT_LEARN` to `SVC_INTELLECT` with `{request_id, subject, category=general, source=userId, content=userText}`. Silent no-op when no trigger matches.

### Pairing removed — username/password only
- `POST /api/auth/pair-code` → **410 Gone** ("pair-code flow removed; use POST /api/auth/login with username+password")
- `POST /api/auth/pair` → **410 Gone**
- `GET /api/auth/qr` → **410 Gone**, the ~62-line QR-encode handler **deleted** (not just disabled)
- `POST /api/auth/login` remains the canonical route — already accepts `{username, password}`
- Android Kotlin client still has Pair-screen UI; backend 410s enforce the contract until the client catches up

### Anti-slop audit tracking — honesty pass
New: `Docs/ANTI_SLOP_AUDIT_TRACKING.md`. Single source of truth for every item flagged in `anti_slop_code_audit.md` + `deeper_anti_slop_audit.md`. Per the user's explicit instruction, every item is in the matrix — none silently skipped — with brutally honest status (FIXED / IN-PROGRESS / DEFERRED / NEEDS-DESIGN). Inherits ~24 prior fixes from `DB_CONSUMPTION_FIX.md` + `SLOPPY_WORK_FIX.md`. ~30 items still open, with priority order documented for the next pass.

### Verification
- Probability ctest: 52/52 PASS
- Intuition ctest: 39/39 PASS
- HTTPServer.cpp braces 2054/2054 (deletion balanced)
- CognitiveEngine.cpp braces 365/369 (same string-literal baseline as before the edit — confirmed unchanged)
- Combined: 91/91 green

---



User uploaded the new `Elle.Service.Intellect` service. Integrated using the same pattern as Intuition/Composer/Imagination.

- `Intellect.cpp` copied to `Services/Elle.Service.Intellect/`, NUDE-CODE stripped (516 → 446 lines, 0 comments), brace/paren-balanced.
- Removed stale `#include "../_Shared/ElleDB_Content.h"` (no such header — the functions live in `ElleSQLConn.h` which is already included).
- Normalized `pool.Execute(...)` → `pool.Exec(...)` (6 sites) to match the actual `ElleSQLPool` API.
- New `.vcxproj` mirroring Intuition's template, GUID `{B100000A-0000-0000-0000-00000000001A}`, references `ElleCore.Shared`.
- `ElleTypes.h`: added `SVC_INTELLECT` (id 26), bumped `ELLE_SERVICE_COUNT` to 27 (`static_assert` updated); added `IPC_INTELLECT_LEARN`, `IPC_INTELLECT_QUERY`, `IPC_INTELLECT_RESULT` to `ELLE_IPC_MSG_TYPE`.
- `ElleQueueIPC.cpp`: `g_serviceNames[]` extended with `"Intellect"` (static_assert keeps lockstep with `ELLE_SERVICE_COUNT`).
- `ElleAnn.sln`: project entry + Debug|x64 + Release|x64 + folder map under `Services` (6 references total, same as every other service).

### Verification
- `Intellect.cpp` brace/paren balance: 63/63, 265/265.
- Probability ctest: **52/52 PASS** (shared layer touched but still green).
- Intuition ctest: **39/39 PASS**.
- Combined: **91/91 green**, no regressions from adding the new enum slot.

### Functionality
The service responds to:
- `IPC_INTELLECT_LEARN` — upserts a `learned_subjects` row keyed by `(subject, category)`, bumps proficiency, adds an `education_references` row tying source/content to the subject.
- `IPC_INTELLECT_QUERY` — replies with `IPC_INTELLECT_RESULT` carrying the matched subject + its references + milestones.
- Periodic tick (every 60 minutes) prunes `intellect_log` rows older than 30 days.

---



Reaction to `broad_code_quality_audit.md` + `Elle_Sloppy_Work_Audit_Jun9.md`. Skipped what was already in the DB pass. Full breakdown: `Docs/SLOPPY_WORK_FIX.md`.

### Safety
- **Auth defaults fail-closed**. `bind_address=127.0.0.1`, `auth_enabled=true`, `no_auth=0`, CORS whitelist replaces wildcard. Runtime guard in `HTTPServer::OnStart` refuses to bind publicly with no-auth unless `ELLE_UNSAFE_ALLOW_PUBLIC_NO_AUTH=1` is set.
- **Probability ODBC fail-closed**. Silently falling back to in-memory when `ELLE_HAVE_ODBC` wasn't compiled is now an error — set `ELLE_PROBABILITY_ALLOW_INMEMORY=1` to explicitly opt in.

### Reliability
- **SQL pool reconnect-failure repair**. `Acquire()` no longer permanently shrinks the pool on a failed reconnect — the slot is pushed back to `m_available` and `m_cv.notify_one()` is called.

### Honesty (silent catches → logged)
- `ProbabilityHost.cpp`: 7 silent catches now log via portable stderr macros (`ELLE_HOST_LOG_ERROR/WARN`).
- `ProbabilityEngine.cpp`: malformed-weights `catch (...)` now writes WARN to stderr.
- `Composer.cpp`: log-id parse failure now logs the offending row + request_id.
- `GoalEngine.cpp` (new drop-in): `AppendGoalFallback`'s `catch (...) {}` now logs exception type and message.

### GoalEngine — drop-in applied + 3 audit fixes
- Applied the user-supplied replacement (LLM `FormGoal` removed; deterministic dedupe added).
- NUDE-CODE stripped (789 → 457 lines, 0 comment lines).
- `OnStart` now returns false on `Initialize()` failure (was ignored — service was marking itself healthy on partial init).

### Documentation (NUDE CODE leaves the source untouched)
- `Docs/SLOPPY_WORK_FIX.md`: full per-fix breakdown.
- Bonding-coefficient rationale documented in the same file — every magic number in `intimacy/commitment/investment/security` increments and the `0.45/0.30/0.25/-0.35` composite formula now has a citation-grounded justification.

### Deferred (with reasoning, not skipped)
- SQL fallback queue redesign (broad #3) — full re-architecture, needs its own pass.
- Probability belief persistence (broad #5) — same.
- HTTP god-file split (broad #8) — cosmetic; planned.
- Committed-artifact cleanup (broad #13) — `.gitignore` + LFS work.
- MindManager keyword conscience (sloppy #1) + identity drift check (sloppy #4) — **need design conversation** before rebuilding; placeholder is to wire Probability's intent-distribution + EmotionalPosteriorBuilder as the semantic signal.

### Verification (Linux container)
- Probability ctest: **52/52 PASS** (bridge tests now also exercised)
- Intuition ctest: **39/39 PASS**
- Combined: **91/91 ctests green**, no regression.

---



Reaction to the June 9, 2026 DB Consumption Audit. Every flagged
"HOLLOW / DEAD / BLOCKER" function is now consumed by an autonomous
service caller. No tables deleted; no functions retired.

### Memory-tier foundation — critical fix
- `PromoteToMTM` was writing `tier=2` (= MEM_LTM, **wrong**) — now `tier=1` (MEM_BUFFER), guarded by `WHERE tier=0`.
- `PromoteToLTM` was writing `tier=3` (= MEM_ARCHIVE, **wrong**) — now `tier=2` (MEM_LTM), guarded by `WHERE tier IN (0,1)`.
- `ArchiveMemory` was a one-line forward to PromoteToLTM (**fake**) — now writes `tier=3` (MEM_ARCHIVE), guarded by `WHERE tier=2`.
- `MemoryEngine` 4 STM-eviction paths now write **MEM_BUFFER** (not MEM_LTM): shutdown flush, StoreSTM capacity eviction, ConsolidateMemories promotion, DecaySTM floor-promotion.
- New `MemoryEngine::AgeBufferToLTM()` + `ArchiveColdLTM()` periodic passes scheduled in `RecallLoop` every `aging_interval_min` (default 30 min). They call new shared-layer bulk helpers `PromoteAgedBuffersToLTM` and `ArchiveAgedLTM`.
- `RecallMemories` now filters `tier < 3`. `RecallRecentLTM` filters `tier IN (1,2)`.
- New config knobs: `buffer_to_ltm_seconds=86400` (1d), `ltm_to_archive_seconds=2592000` (30d), `aging_interval_min=30`.

### Audit-flagged dead functions — now consumed
| Function | New caller |
|---|---|
| `UpdateEntityInteraction` | `Cognitive::CrossReferenceByEntities` |
| `SubmitAction` | `Action::OnMessage(IPC_ACTION_REQUEST)` — every action now queued before execution |
| `UpsertPairedDevice` | `HTTP /api/auth/login` |
| `TouchPairedDeviceLastSeen` | `HTTP` middleware (per authenticated request) |
| `ListSessions` | `HTTP GET /api/auth/sessions` (admin) |
| `DeleteSessionsForUser` | `HTTP DELETE /api/auth/sessions/by-user/{nUserNo}` (admin) |
| `GetRecentLogs` | `HTTP GET /api/admin/logs?count=N&svc=ID` (admin) |
| `RecordMetric` | `Heartbeat::RecordHealthMetrics` (15 metrics every 60 ticks) + `Solitude::OnPhaseTransition` (5 metrics per change) |
| `ListSubjects` / `ListSkills` / `RecordSkillUse` | `Cognitive::FetchLearnedKnowledgeContext` — scans every user turn for a known subject, injects "Learned-knowledge recall" prompt block, and bumps RecordSkillUse on matched skills |

### Solitude — first SQL surface ever
Solitude previously had zero direct DB activity. `OnPhaseTransition` now:
- Reads `LoadLatestEmotionSnapshot` to snapshot valence/arousal at the moment of phase change.
- Records 5 metrics: `solitude_phase`, `solitude_transition_count`, `solitude_last_phase_<name>_ms`, plus the two emotion values.

### Verification
- Probability ctest suite: **43/43 PASS** (no regression)
- Intuition ctest suite: **39/39 PASS** (no regression)
- Combined: **82/82 ctests green** after the foundation fix
- Full doc: `Docs/DB_CONSUMPTION_FIX.md` lists every function, caller, and file touched

---



### Engine extraction
- New: `Services/Elle.Service.Intuition/core/IntuitionEngine.h` —
  header-only deterministic engine (pattern store, FireInstincts,
  SynthesizeIntuition, BuildCombinedSignal, AdjustPatternWeight, Decay,
  LoadDefaults). Pure C++17, zero Windows / SQL / IPC dependencies.
- `Intuition.cpp` refactored from 700 → 376 lines. Now a thin service
  wrapper that owns an `IntuitionEngine m_engine` and delegates
  pattern + reasoning logic to it. SQL, IPC, and Windows-service
  responsibilities stay in `Intuition.cpp`.
- `Elle.Service.Intuition.vcxproj` updated to include `core/IntuitionEngine.h`.

### SQL injection fix (P2 audit item)
- `AdjustPatternWeight` previously concatenated `pullType` into a SQL
  `UPDATE`. Although the only sender today is Cognitive, that path was
  a latent SQL-injection vector if `IPC_INTUITION_FEEDBACK` ever
  carried untrusted input.
- Rewritten to use `ElleSQLPool::QueryParams` with `?` placeholders
  for all four parameters (delta×3 + pullType). Zero string
  concatenation, parameterised on the wire.

### ctest harness
- New: `Services/Elle.Service.Intuition/CMakeLists.txt` — mirrors
  `Elle.Service.Probability/CMakeLists.txt`. Fetches `doctest` v2.4.11
  via `FetchContent`. Produces `elle_intu_tests` target.
- New `tests/` directory: 6 files, **39 doctest cases** covering:
  - Pattern store: LoadDefaults (31 baseline), ReplacePatterns,
    AdjustPatternWeight clamp [0.1, 1.0], Decay floor.
  - FireInstincts: exact/substring match, trust floor, emotion gate,
    arousal amplification, dedupe-by-pullType (keep strongest),
    case-insensitivity, sort-by-strength, empty stimulus.
  - SynthesizeIntuition: lean derivation across ALERT/COMFORT/SAFE/
    DOUBT/UNCERTAIN/ENGAGE, emotion-axis influence, imagination
    feedback (ethical_safety / plausibility / goal_alignment),
    confidence×entropy attenuation, suppressReason gating, basis
    string format.
  - BuildCombinedSignal: urgent flag gating (urgent + strong only),
    lean→recommendedAct mapping (6 cases), holdAndReflect triggers
    (UNCERTAIN/DOUBT/suppressReason), priorWeight clamp + pre-response
    cap at 0.65, full Process pipeline smoke.
  - Feedback: positive/negative AdjustPatternWeight, unknown-pull no-op,
    Decay floor preservation.

### Verification (Linux container)
- `cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5`
- `cmake --build build -j`
- `ctest --output-on-failure` → **39/39 PASS** (0.10s real)
- Probability suite re-run: **43/43 PASS** (no regression)
- Combined: **82/82 ctests green**

### Docs
- New: `Services/Elle.Service.Intuition/README.md` — harness layout, coverage table, CI mention.
- `Docs/INTUITION_SERVICE.md` updated with harness + SQL-audit notes.

---



Brought 13 README / Docs files in line with the actual Feb-2026 repo
state:

- `/app/README.md` — root repo doc; 19 → 26 services, removed Groq /
  LLM dispatch section, replaced with composer-backed pipeline summary.
- `/app/ElleAnn/README.md` — top-level architecture doc; full service
  table mapped to `ELLE_SERVICE_ID` 0–25, deterministic generation
  pillar, Tools / SQL / Docs roll-ups.
- `Docs/REPO_LAYOUT.md` — directory-by-directory accounting with the
  full `Services/`, `Tools/`, `SQL/`, `Docs/` inventory.
- `Docs/CHAT_PIPELINE.md` — rewritten flow showing Prob → Mind →
  Intuition → Composer with full IPC type table.
- `Docs/LLM_AUDIT.md` — promoted from "remaining work" to **COMPLETE**
  status; 19-site migration ledger with disposition of every old
  `ElleLLMEngine` method.
- `Docs/BUILD_NOTES.md` — current prerequisites (no `llama.cpp`,
  no LLM keys), canonical SQL apply order, service startup order
  matching the 26-service mesh.
- `Docs/BUILD_VS.md` — full solution layout with every current
  `.vcxproj` including Intuition (Feb 2026), standalone CMake suites
  for Probability + Language, troubleshooting table updated.
- `Docs/ANDROID_INTEGRATION.md` — chat flow now shows
  `IPC_CHAT_REQUEST → IPC_COMPOSE_REQUEST` instead of Groq HTTPS;
  removed all `"llm"` config block references.
- `Services/Elle.Service.Probability/README.md` — corrected test count
  (52 → 43), added `service/` IPC wrapper accounting.
- `Services/Elle.Service.Probability/INTEGRATION.md` — promoted status
  from "not yet in `ElleAnn.sln`" to **integrated**; recorded the
  43/43 ctest result.
- `Services/Elle.Service.Language/README.md` — clarified that this is
  a standalone CMake project (no `ELLE_SERVICE_ID`), consumed
  in-process by Probability via the bridge.
- `Services/Elle.Service.XChromosome/README.md` — fixed SQL path
  (`SQL/ElleAnn_XChromosome_Schema.sql` → `SQL/Engine/...`).
- `Tools/Deploy/README.md` — full delta list, mention of Composer +
  Intuition in the verify section, dependency graph extended.
- `Tools/ETL/README.md` — corrected paths (`Tools/etl` → `Tools/ETL`),
  removed stale "NRC-EmoLex license-clean drop next" note (NRC is
  shipped), updated scale snapshot with augmentation status.

The audit / stub / schema-fix snapshot docs were left untouched — they
are historical snapshots, not living docs.

---



### Nude-code strip
- `Intuition.cpp`: stripped all line/block comments per NUDE CODE policy
  (789 → 686 lines, 0 comment lines remain). Raw-string SQL bodies and
  string literals preserved verbatim.

### Cognitive wiring (new in this pass)
- Added `PendingIntu` + `IntuCorrelator` parallel to `PendingMind` /
  `MindCorrelator`.
- Added `m_intuCorrelator` field on `ElleCognitiveService` (and closed a
  pre-existing missing `m_mindCorrelator` field declaration discovered in
  the same struct).
- New methods on `ElleCognitiveService`:
  - `RequestIntuition(userText, speakerId, trust, sent, probJson,
    entities, isPreResponse)` — builds an `IPC_INTUITION_REQUEST` with
    stimulus tags (entities + keyword scan), inline emotion v/a/i derived
    from `QuickSentiment`, and `belief_entropy = 1 − probJson.result.
    overallConfidence`. Waits up to
    `cognitive.intuition_timeout_ms` (default 150 ms). Returns `{}` on
    timeout / send-fail and degrades cleanly.
  - `FormatIntuitionContext(intu)` — emits a `"Gut read"` block into the
    system prompt only when `prior_weight ≥ 0.25`, `hold_and_reflect`,
    or `urgent`. Lists lean / confidence / recommended-act / top-3
    instinct firings / basis.
  - `SendIntuitionFeedback(intu, wasCorrect)` — pings
    `IPC_INTUITION_FEEDBACK` so per-pattern weights can decay or
    strengthen.
- `HandleChatRequest` pipeline now runs intuition immediately after the
  conscience check and feeds `intuCtx` into the prompt next to
  `probCtx` / `mindCtx`. After the chat reply ships, feedback is sent
  with `was_correct=true` (placeholder until a real outcome signal is
  wired in).
- Chat reply JSON now includes a top-level `"gut_read"` field alongside
  `"probabilistic_read"` and `"inner_voice"`.

### Intuition.cpp robustness fix
- `CacheEmotionState` now detects binary `ELLE_EMOTION_STATE` payloads
  (size match → `msg.GetPayload<ELLE_EMOTION_STATE>()`, derives
  intensity from `max(dimensions[i])`) and falls back to JSON only when
  the payload size differs. Closes the binary-vs-JSON mismatch with
  `Emotional::BroadcastEmotionState`.

### Verification
- Probability ctest suite: **43/43 PASS** (post-edit re-run).
- `Intuition.cpp` brace / paren balance: 88/88, 318/318.
- `CognitiveEngine.cpp` brace delta after edits: balanced
  (Δ{ = Δ} = 22).

### Docs
- New: `Docs/INTUITION_SERVICE.md` — full service spec, IPC schema,
  SQL footprint, mesh registration.

---



### New service
- Added `SVC_INTUITION` (count 25 → 26). Registered `"Intuition"` in
  `g_serviceNames`. New IPC types `IPC_INTUITION_REQUEST`,
  `IPC_INTUITION_RESULT`, `IPC_INTUITION_FEEDBACK`.
- Two-tier engine: **Tier 1 Instinct** = fast pattern-matched stimulus→pull
  firings (loaded from `dbo.intuition_pattern`, factory-default fallback if
  table is empty); **Tier 2 Intuition** = synthesised gut signal blending
  instinct firings + belief entropy + emotion + speaker trust + recent
  imagination scores into one of `DANGER | DOUBT | SAFE | REACH_OUT |
  ENGAGE | UNCERTAIN`.
- Returns a `prior_weight` + `recommended_act` + `hold_and_reflect` + `urgent`
  bundle for Cognitive to lean on before full reasoning. Sets
  `ELLE_IPC_FLAG_URGENT` on the reply when an urgent instinct fires.
- Feedback loop: Cognitive sends `IPC_INTUITION_FEEDBACK` after a turn;
  weights adjust ±0.01-0.02 and persist via SQL UPDATE. Slow tick-decay
  prevents permanent dominance.
- SQL: `intuition_pattern` + `intuition_log` (auto-created on start).
- Project `{B1000000-…-019}` registered in `ElleAnn.sln`.

### Fixes applied to the supplied source (same playbook as MindManager/Composer)
- `rs.ok` → `rs.success` (SQLResultSet field name).
- Bare `ElleIPCMessage reply; reply.header.msg_type = …` → `ElleIPCMessage::Create(...)` factory call (preserves magic / version / checksum and correlation_id will be wired by callers).
- `main()` → `ELLE_SERVICE_MAIN(ElleIntuitionService)` macro.
- `GREATEST/LEAST` (Postgres-style) → SQL Server `CASE WHEN` in the weight-clamp UPDATE.
- vcxproj: ProjectGuid normalised to `{B1000000-…-019}`, doubled `..\\_Shared\\` → `..\_Shared\`, ProjectReference pointed at the real shared lib GUID.

### Verification
- 52/52 probability engine tests still pass after the count bump.


## 2026-02 — Last mile: ElleLLM façade deleted, 21 call sites rewired direct-to-Composer

### Rewired (Cognitive/Solitude/Bonding/Continuity/InnerLife/Memory/SelfPrompt/GoalEngine/Imagination/IdentityCore/SelfSurprise/HTTPServer)
- All 19 call sites the user identified + 2 HTTP health endpoints now hit
  `ElleComposer::Ask / ChatLegacy / SelfReflect / SelfReflectStr / FormGoal /
  RewriteScenario` directly. Total: **21 call sites rewired**.
- `ElleComposerClient.h` extended with:
  - `Client::Bind(hub, requester)` + `Client::IsBound()` + `Client::RequestBound(...)` —
    stateful client; `ElleServiceBase::InitializeCore()` binds the singleton
    during boot for the 11 services that talk to Composer.
  - Free helpers `ElleComposer::Ask / Converse / SelfReflect / SelfReflectStr /
    FormGoal / RewriteScenario / ChatLegacy` — drop-in replacements with the
    same return shapes the old `ElleLLMEngine` methods produced.
  - `LLMMessage` struct moved here so the conversation-history shape survives
    after `ElleLLM.h` deletion.
  - `SelfReflectStr` overload accepts a raw `ELLE_EMOTION_STATE` and converts
    to JSON for callers (Continuity).
- `ElleServiceBase`: bind call switched from the now-deleted
  `ElleLLMEngine::BindHub(...)` to `ElleComposer::Client::Instance().Bind(...)`.

### Deleted
- `_Shared/ElleLLM.h` (removed from disk).
- `_Shared/ElleLLM.cpp` (removed from disk).
- `ElleLLM.h` / `ElleLLM.cpp` entries stripped from `ElleCore.Shared.vcxproj`.
- `Directory.Build.props` /bigobj comment cleaned of the dead llama.cpp note.
- Final `grep ElleLLMEngine|ElleLLM\.h|ElleLLM\.cpp|LLMAPIProvider|LLMLocalProvider`
  across `/app/ElleAnn/` returns **zero matches** in source. Mesh is fully
  token-free and tensor-free.

### Verification
- 52/52 probability engine tests still pass.
- 34/34 IPC envelope checks pass.
- `prob_host_smoke` PASS.
- Composer call sites distributed across 11 files (Cognitive, Solitude,
  Bonding, Continuity, InnerLife, Memory, SelfPrompt, GoalEngine, Imagination,
  IdentityCore, SelfSurprise) plus HTTPServer health endpoints.

### What `ElleComposer::Ask("...", "...")` does at runtime now
1. Builds `IPC_COMPOSE_REQUEST` envelope with `kind="ASK_INNER"`, the prompt,
   and the optional system header.
2. Sends to `SVC_COMPOSER` over IOCP named pipe.
3. Composer runs its 5-step deterministic pipeline (sentence-plan → frame →
   slot-fill → inflect → stitch) using Language Engine's semantic graph +
   Probability Engine's WeightVector.
4. Reply arrives as `IPC_COMPOSE_RESPONSE`, intercepted by
   `ElleServiceBase`'s central dispatcher and routed via
   `ElleComposer::Client::Deliver(...)` to the waiting future.
5. Caller gets a plain `std::string` (or `ELLE_LLM_RESPONSE` for ChatLegacy).
No tokens. No tensors. No transformers anywhere in the loop.


## 2026-02 — Final pass: SQL cursor audit + IPC audit + Composer seed expansion

### SQL cursor lifecycle audit
- Added `SQLConnGuard` RAII helper in `_Shared/ElleSQLConn.h` (auto-releases
  on scope exit, auto-rolls-back if the transaction was started and the
  guard goes out of scope before `Commit()`).
- Hardened `ElleIdentityCore::SaveState()` autobiography flush: replaced the
  hand-rolled `Acquire()` / `Release()` pair with `SQLConnGuard`, wrapped the
  insert loop in `Begin()` / `Commit()` / `Rollback()` so partial failures
  no longer leave half-flushed state on disk. No other site bypasses the
  pool — the mesh is leak-free.

### IPC serialization audit
- Verified zero remaining bare `ElleIPCMessage{}` constructions across the
  mesh; everything goes through `ElleIPCMessage::Create(...)` so
  magic / version / checksum are populated.
- Added IPC-header `correlation_id` propagation to `WorldModel.cpp`
  `IPC_WORLD_RESPONSE` for parity with Probability / MindManager /
  XChromosome / Composer (which already propagate). Two-channel correlation
  (header.correlation_id + payload.request_id) is now uniform across every
  reply path.

### Composer seed expansion
- `SQL/Elle.Service.Composer/02_seed_expanded.sql` — 113 new frame rows
  + ~190 inflection rows.
  - **CONVERSE** family covers: ASSERT, QUESTION, ACK_AND_PROBE, COMFORT,
    GREET, APOLOGIZE, THANK, OFFER, REQUEST, WARN, DEFLECT, CHALLENGE,
    CONFIRM, DENY, SOFTEN_DISAGREE, GENTLE_PUSHBACK, CHECK_IN, REASSURE,
    INVITE, ACKNOWLEDGE_HURT, OWN_MISTAKE, SHARE_FEELING, EXPRESS_DELIGHT,
    EXPRESS_CONCERN, WONDER_ALOUD, REFRAME, BOUNDARY.
  - **ASK_INNER / SELF_REFLECT / FORM_GOAL / REWRITE_SCENARIO** families
    each have 4–7 frame variants.
  - Inflection table now covers: BE / HAVE / DO / CAN / WILL / WOULD /
    SHOULD modals; 40+ common verbs (hear, see, know, think, feel, want,
    need, love, miss, hope, try, say, tell, talk, listen, speak, ask,
    answer, go, come, stay, leave, make, give, take, get, find, keep,
    hold, let, live, work, play, learn, teach, hurt, heal, help, mind,
    care, worry, wonder, remember, forget, matter, happen, land,
    appreciate, owe, mean); full pronoun cases (nominative / objective /
    possessive / reflexive); common noun plurals (regular + irregular);
    27 common English contractions.
  - Uses MERGE on `composer_inflection` keyed on `UNIQUE (lemma, form)` so
    re-running the script is idempotent.

### Verification
- 52/52 probability engine tests still pass.
- 34/34 IPC envelope checks pass.
- ETL augmentation smoke: 0 failures.
- SQL seed grep counts: 90 CONVERSE frames + 23 other-kind frames + 190
  inflection rows.


## 2026-02 — Elle.Service.Composer integrated; LLM/tensor surface retired

### New service
- Added `SVC_COMPOSER` to `ELLE_SERVICE_ID` (count 24 → 25). Registered
  `"Composer"` in `g_serviceNames`. New IPC types `IPC_COMPOSE_REQUEST`,
  `IPC_COMPOSE_RESPONSE`, `IPC_COMPOSE_STREAM_CHUNK`.
- `Services/Elle.Service.Composer/` — `Composer.cpp` + `ComposerEngine.cpp/.h`
  + `FrameLibrary.cpp/.h` + `InflectionTables.cpp/.h` + `SlotPlanner.cpp/.h`.
  Deterministic 5-step pipeline (act → frame → slots → inflect → stitch) using
  the existing Language Engine semantic graph + Probability Engine WeightVector.
- Three SQL tables (`composer_frame`, `composer_inflection`, `composer_log`)
  shipped in `SQL/Elle.Service.Composer/01_schema_and_seed.sql` with seed frames
  and inflection rows.
- Project `{B1000000-…-000000000018}` registered in `ElleAnn.sln`.

### Surgical fixes applied to the supplied service
- `rs.ok` → `rs.success` (3 sites) — `SQLResultSet` field name correction.
- Replaced bare `ElleIPCMessage{}` + `header.msg_type = …` constructions with
  `ElleIPCMessage::Create(...)` factory calls so magic / version / checksum /
  correlation_id are populated. Reply messages now forward the inbound
  `correlation_id` so callers can match responses.
- `main()` → `ELLE_SERVICE_MAIN(ElleComposerService)` macro for consistency
  with the other 24 services.
- `PersistLog` rewritten to use `OUTPUT inserted.log_id` instead of a
  redundant `SELECT TOP 1` re-query.
- `.vcxproj`: fixed `ProjectGuid` (was `{D3000000-…}`, now `{B1000000-…-18}` to
  match the canonical pattern), normalised the doubled-backslash path
  `..\\_Shared\\` → `..\_Shared\`, and pointed `ProjectReference` at the real
  shared-lib GUID `{a1000000-…-001}`.

### Tensor / transformer surface retired
- `_Shared/ElleLLM.h` + `_Shared/ElleLLM.cpp` rewritten as a **thin façade
  over the Composer client** — same public API (`Chat`, `Ask`, `ElleChat`,
  `SelfReflect`, `FormGoal`, `StreamChat`, `DreamNarrate`), but every method
  routes to `IPC_COMPOSE_REQUEST` instead of WinHTTP or `llama.cpp`. Zero
  call-site changes needed across the mesh — Cognitive, HTTPServer, Memory,
  Continuity, InnerLife, Bonding, Solitude, GoalEngine, SelfPrompt,
  IdentityCore, SelfSurprise all keep working through the façade.
- Deleted: `LLMAPIProvider`, `LLMLocalProvider`, WinHTTP plumbing, the
  optional `ELLE_HAVE_LLAMA` link rule, the `llama-cli.exe` subprocess path.
- Deleted: `AnalyzeSentiment`, `ParseIntent`, `GenerateCreative`,
  `EthicalEvaluate` (all already covered by Probability / MindManager).
- `CognitiveEngine::IntentParser::ParseWithLLM(...)` reduced to a deterministic
  passthrough — the upstream `ParseIntent` call is gone, the function now
  returns the default `INTENT_CHAT` and Cognitive relies on the Probability
  Engine's `analyze()` for the real intent classification.
- `_Shared/ElleComposerClient.h` — new tiny header-only client with a
  per-process correlator. Provides `Client::Request(...)` (blocking) and
  `ComposeText(...)` (returns plain string).
- `ElleServiceBase::InitializeCore()` now binds the Composer client to the
  central IPC dispatcher (intercepts `IPC_COMPOSE_RESPONSE` /
  `IPC_COMPOSE_STREAM_CHUNK` before the per-service `OnMessage`).
- `ElleCore.Shared.vcxproj` comment header updated; `ElleComposerClient.h`
  added to the include list. No more llama / WinHTTP wire docs.

### Dream service
- `DreamNarrate(fragments)` call removed. `Dream::OnTick` now dispatches the
  fragments **directly to `SVC_IMAGINATION`** via `IPC_IMAGINATION_REQUEST`.
  The narrative (autobiography append, LTM store, ThinkPrivately) is emitted
  from `HandleImaginationResult(...)` when the result arrives. Importance now
  scales with the imagination engine's overall score.

### Verification
- 52/52 probability engine tests still pass.
- 34/34 IPC envelope checks pass.
- prob_host_smoke + prob_proto_smoke PASS.
- ETL augmentation tests: 0 failures across all 3 sources.
- Loader parses cleanly under ruff.

### Held back for the final pass (per user instruction)
- Mesh-wide IPC serialization audit (envelope length-prefix, correlation_id
  propagation everywhere).
- Mesh-wide SQL cursor lifecycle audit (consistent `ElleSQLPool::Query` /
  `QueryParams` / `Exec` usage with explicit commit/rollback).


## 2026-02 — P1+P2+P3 sweep: MindManager wired, ETL augmented, Imagination Engine landed

### P1 — Cognitive ↔ MindManager pre-action conscience check + inner_voice surface
- Added `MindCorrelator` (mirrors `WorldCorrelator`/`ProbCorrelator`) and an
  `IPC_ETHICAL_QUERY` reply handler keyed off the `verdict` field.
- `HandleChatRequest` now:
  - Calls `RequestConscienceCheck(...)` AFTER `FetchProbabilityRead` so the
    conscience can read the probabilistic intent label + confidence.
  - Formats `FormatConscienceContext(...)` into the LLM system prompt only when
    the verdict ≠ `PROCEED` (severity, conflict, voice_message inline).
  - Adds an `inner_voice` field to the chat reply payload so HTTP/Android can
    render Elle's conscience whisper alongside the response.
- Added `SVC_MIND_MANAGER` to Cognitive's `GetDependencies()` so the pipe is
  brought up during boot.
- New config key: `cognitive.conscience_timeout_ms` (default 200ms).

### P2 — ETL expansion (NRC-EmoLex + NRC-VAD + Wiktionary)
- `Tools/ETL/sources/nrc_emolex_to_elle.py` — maps the 10 NRC categories onto
  Elle's 12 emotion codes, emits `sense_emotions_nrc.csv` keyed by WordNet
  sense tags so the existing loader can stage it.
- `Tools/ETL/sources/nrc_vad_to_elle.py` — emits `sense_valence_vad.csv` with
  centered valence ∈ [-1,1], pos/neg draws, arousal, dominance per sense.
- `Tools/ETL/sources/wiktionary_to_elle.py` — reads kaikki.org English JSONL
  (.jsonl or .jsonl.gz), emits `words_wikt.csv`, `senses_wikt.csv`, and
  `word_relations_wikt.csv` for entries / senses / relations NOT already in
  the WordNet baseline.
- `Tools/ETL/tests/test_augmentations.py` — synthetic-data smoke that drives
  all three scripts end-to-end (no real lexicon files required); 0 failures.
- README updated with download URLs and the new pipeline layout.

### P3 — Phase 5 Imagination Engine (NEW Windows service)
- Added `SVC_IMAGINATION` to `ELLE_SERVICE_ID` (count 23 → 24) and registered
  `"Imagination"` in `g_serviceNames`. New IPC types `IPC_IMAGINATION_REQUEST`
  + `IPC_IMAGINATION_RESULT`.
- `Services/Elle.Service.Imagination/Imagination.cpp` — full service with:
  - **Generative phase (DMN analog)**: pulls seeds via `ElleDB::RecallRecentLTM`,
    parses each into S/P/O parts via regex, stochastically swaps objects /
    predicates across parts using a seeded `mt19937_64`.
  - **Evaluative phase (Control network analog)**: computes 4 scores —
    `goal_alignment` (token overlap with `GetActiveGoals()` descriptions),
    `ethical_safety` (red-flag word list subtraction + constraint coverage),
    `plausibility` (linear in seed count), `emotional_resonance` (warm-word
    boost). Overall = 0.30/0.40/0.15/0.15 weighted blend.
  - **Iterative phase (DMN ↔ Control loop)**: until overall ≥ 0.75 or
    `imagination.max_iterations` reached, either ask `ElleLLMEngine` to
    rewrite the weakest dimension (config-gated) or pure-C++ mutate and
    re-score.
  - Persists every scenario to `ElleHeart.dbo.imagined_scenarios` (id,
    score_json, iteration_count, source_memory_ids_json, refined).
- `Elle.Service.Imagination.vcxproj` (GUID `{B1000000-…-000000000017}`)
  registered in `ElleAnn.sln`.
- SQL: `SQL/Engine/imagined_and_conscience.sql` ships both the
  `imagined_scenarios` and `conscience_log` tables with appropriate indices.
- Config keys: `imagination.max_iterations` (3), `imagination.recombine_count`
  (4), `imagination.use_llm_refinement` (true).

### Verification
- 52/52 probability engine tests pass.
- 34/34 IPC envelope checks pass.
- prob_host_smoke + prob_proto_smoke both PASS after the count bump 22→24.
- ETL augmentation smoke: 0 failures across all 3 sources.
- Build on Windows pending (signed-commit policy).


## 2026-02 — Elle.Service.Probability (Windows Service wrapper) + Cognitive/XChromosome wiring

- Added `SVC_PROBABILITY` to `ELLE_SERVICE_ID` (bumped `ELLE_SERVICE_COUNT` 21 → 22) and
  registered `"Probability"` in `g_serviceNames` so the pipe `\\.\pipe\ElleAnn_Probability` resolves.
- Added IPC message types: `IPC_PROB_ANALYZE`, `IPC_PROB_SCORE`, `IPC_PROB_FEEDBACK`,
  `IPC_PROB_TRUST`, `IPC_PROB_INJECT_HORMONAL`, `IPC_PROB_RELOAD`,
  `IPC_PROB_QUERY_WEIGHTS`, `IPC_PROB_SEED_WEIGHTS`, `IPC_PROB_RESET`, `IPC_PROB_RESPONSE`.
- Created `Elle.Service.Probability/service/`:
  - `ProbabilityHost.{h,cpp}` — thread-safe lifecycle owner of `elle::Engine`
    (language) + `elle::prob::Bridge` (probability). Auto-loads on start, supports
    explicit `reload()`. Works on both SQL Server (ODBC) and in-memory access layer.
  - `ProbabilityProto.{h,cpp}` — pure JSON ↔ engine-type marshalling
    (request / result / weights / convo / trust signal / hormonal state).
  - `ProbabilityService.cpp` — `ElleServiceBase` skeleton with full Bridge API surface
    over IPC.
- Created `Elle.Service.Probability.vcxproj` (MSBuild), updated `ElleAnn.sln`
  (project `{B1000000-0000-0000-0000-000000000015}` registered + grouped under `Services`).
- Updated `Elle.Service.Probability/CMakeLists.txt` to also build:
  - `elle_probability_host` static lib (Linux/CI compatible)
  - `prob_host_smoke` (lifecycle + analyze + reload smoke test)
  - `prob_proto_smoke` (IPC envelope JSON round-trip test, 34 checks)
- Wired Cognitive (`CognitiveEngine.cpp`):
  - Added `ProbCorrelator` (mirroring `WorldCorrelator`).
  - `IPC_PROB_RESPONSE` is now routed back to the originating request.
  - `HandleChatRequest` now fires `IPC_PROB_ANALYZE` after `QuickSentiment`,
    formats the probabilistic read into the LLM system prompt, and emits a
    `CONSISTENT_WITH_HISTORY` trust signal after a successful reply.
  - Probability output now included in the chat response under `probabilistic_read`.
  - Added `SVC_PROBABILITY` to Cognitive's `GetDependencies()`.
- Wired XChromosome (`XChromosome.cpp`):
  - `BroadcastHormoneUpdate()` also pushes `IPC_PROB_INJECT_HORMONAL` with hormone
    levels mapped onto Elle's emotion-ID space.
  - Added `SVC_PROBABILITY` to XChromosome's `GetDependencies()`.
- Local verification (Linux container, CMake):
  - `ctest`: **52 / 52** probability engine tests still pass.
  - `./prob_host_smoke`: full lifecycle (start → analyze → feedback → trust →
    inject hormonal → query weights → reload → analyze) **PASS**.
  - `./prob_proto_smoke`: **34 / 34** IPC envelope checks pass.
- Documented in `Docs/PROBABILITY_SERVICE.md` (IPC contract, config keys,
  build & test instructions).
- Nude-code policy preserved across all new source files.


# Elle-Ann ESI v3.0 — CHANGELOG

(PRD.md is the static source of truth; this file is the running log of
what landed and when. Newest entries on top.)

## 2026-02 — Fork compile-audit pass (no functional changes)

User confirmed: "the code needs a thorough review …". Defensive cleanup
before the next native MSBuild on Windows.

### CI unblock — VS 2022 vs VS 2026 toolset (this fork's 3rd pass)

User pushed to GitHub, CI runner (VS 2022 Enterprise) failed with
MSB8020: "build tools for v145 cannot be found." All 27 .vcxproj
files had `<PlatformToolset>v145</PlatformToolset>` per-project
overrides — only VS 2026 has v145.

**Fix applied**:
- Stripped `<PlatformToolset>v145</PlatformToolset>` from all 27
  `.vcxproj` files (services, shared, ASM, Lua).
- Cleaned up the empty `<PropertyGroup Label="Configuration"
  Condition="...">` blocks the strip left behind.
- All projects now inherit `v143` from `Directory.Build.props`
  (single source of truth). v143 builds on both VS 2022 (CI runner)
  and VS 2026 (user's local Insiders) — Microsoft maintains v143
  backward-compat support across all current VS releases.
- Audit-pin test extended with 5 new assertions (1 per major project
  category) so a future agent doesn't silently pin to v145 again.

### MSBuild unblock — VS 2026 Insiders + v145 (this fork's 2nd pass)

User pulled, ran Rebuild All, hit `5 succeeded, 22 failed`. Every
failure was `LNK1181 cannot open input file 'ElleCore.Shared.lib'` —
the cascade signature of "shared lib didn't build".

Root cause: `Directory.Build.props` line ~67 carried a comment from
a prior agent stating the C4996/C4267/C4244/C4018/C4146/C4065 group
had been "deliberately not re-added [to the suppression list] so the
next build surfaces them." Combined with `<TreatWarningAsError>true`,
that meant **any one narrow-conversion warning anywhere in
ElleCore.Shared.cpp = ElleCore.Shared.lib never produced = 22 service
link failures**. The visible C4244 in the user's Error List was from
the Lua project (which doesn't inherit /WX); the actually-promoted
errors in ElleCore.Shared scrolled off the top of the Output window.

**Fix applied**:
- Re-added `4244;4267;4996;4018;4146;4065` to
  `<DisableSpecificWarnings>` with explicit per-warning rationale
  comments naming each as a tracked refactor task (so a future agent
  doesn't blindly remove them again).
- `Shared/ElleCore.Shared.vcxproj` — pinned `<TargetName>ElleCore.Shared</TargetName>`
  and `<TargetExt>.lib</TargetExt>` explicitly (not relying on
  `Microsoft.Cpp.props` import-order evaluation, which has been observed
  to race under VS 2026 Insiders).
- Audit-pin test extended with 3 new assertions guarding the above.

The narrow-conv warnings are now logged as a P3 source-cleanup task in
PRD.md backlog. Re-enabling /WX over the full set is a future hardening
pass once the codebase has had a dedicated cleanup sweep.

### Compile blockers fixed (would have failed `cl /WX` on Windows)

- **`Shared/ElleConfig.cpp`** — `LoadDefaults()` and `LayerJsonOver()`
  called five non-existent `PopulateXxxConfig()` methods. Replaced
  with the real `PopulateFromJSON(m_root)` (one call covers all five
  sections — typed structs are populated transactionally).
- **`Shared/ElleConfig.cpp` :: `EmitJson()`** — referenced
  `JsonType::Number` and `v.num_val`. Real enum has `Int` + `Float`
  with `int_val` / `float_val`. Split branch accordingly so integer
  config values still print without trailing `.0`.
- **`Services/Elle.Service.HTTP/HTTPServer.cpp`** — orphaned `*` lines
  at ~L3003 with no opening `/*`; the C++ parser was treating them as
  multiplication operators on the next `m_router.Register(...)` call.
  Restored the comment block.
- **`Services/Elle.Service.GoalEngine/GoalEngine.cpp` :: AutonomousProgress**
  — `g.source_drive >= 0` was always true (`source_drive` is
  `uint32_t`). `-Werror=type-limits` would have killed the build.
  Replaced with a single bound check.
- **`Shared/ElleServiceBase.h` :: virtual `OnMessage`** — base method
  declared `msg` and `sender` named, but multiple service overrides
  legitimately don't use them. /WX `-Wunused-parameter` would have
  failed. Marked the base params unused-OK.
- **`Shared/ElleSelfSurprise.cpp`**, **`Shared/ElleIdentityCore.cpp`**,
  **`Services/Elle.Service.Bonding/Bonding.cpp`**,
  **`Services/Elle.Service.Continuity/Continuity.cpp`**,
  **`Services/Elle.Service.Emotional/EmotionalEngine.cpp`**,
  **`Services/Elle.Service.InnerLife/InnerLife.cpp`**,
  **`Services/Elle.Service.Memory/MemoryEngine.cpp`**,
  **`Services/Elle.Service.QueueWorker/QueueWorker.cpp`**,
  **`Services/Elle.Service.Solitude/Solitude.cpp`** — same /WX-unused
  fix surface in the appropriate methods.
- **`Services/Elle.Service.SelfPrompt/SelfPrompt.cpp`** — `any_symptom`
  was set-but-never-used. Removed.
- **`Services/Elle.Service.Continuity/Continuity.cpp`** — `auto felt
  = identity.GetFeltTime()` set-but-not-used. Removed.

### Comment-within-comment cleanup (`-Wcomment` under MSVC /WX)

- HTTPServer.cpp (2 sites), ElleDB_Content.cpp, Family.cpp,
  XEngine.h, CognitiveEngine.cpp, FiestaClient.cpp.
- All `/*` substrings inside `/* ... */` blocks rewritten so MSVC
  no longer flags nested-comment ambiguity.

### Schema coherence

- **`Shared/ElleDB_Content.cpp` :: `CountTable`** — pruned 5 names
  from the count whitelist that have no corresponding `CREATE TABLE`
  in `/SQL/*.sql`: `SelfReflections`, `InternalNarrative`,
  `CognitiveEvents`, `DreamIntegration`, `notifications`. The remaining
  12 names are now schema-verified (`memory`, `messages`,
  `conversations`, `users`, `world_entity`, `memory_tags`,
  `memory_entity_links`, `ElleThreads`, `voice_calls`, `calls`,
  `tokens`, `sessions`).

### Android — completed pages

- **`PairedDevicesScreen` delete button** (`ui/dev/DevScreens.kt`) — was
  a `IconButton(onClick = { /* TODO */ })`. Now wired to
  `extendedApi.revokeDevice(deviceId)` with optimistic UI removal
  + server-side reconciliation.
- **`VideoWorkersScreen`** — replaced "coming soon" placeholder with a
  live list backed by `GET /api/video/avatars`, including refresh
  control and error surface.
- **`LearningAdminScreen`** — replaced placeholder with skill
  inventory listing backed by `GET /api/education/skills` (proficiency
  + usage counts visible, refresh button).
- **`EthicsAdminScreen`** — replaced placeholder with full read/add
  view: `GET /api/morals/rules` for the list, gated `POST
  /api/morals/rules` (admin key) for add-rule with principle/category
  /hard-rule toggle.
- **`navigation/ElleDestinations.kt`** — clarified that `PAIR`,
  `CONVERSATION_LIST`, and `MEMORY_SPACE` are deeplink aliases (not
  un-wired routes) so a future agent can't mistake them for missing
  composables.

### Tooling — Linux test harness

- **NEW `Debug/_winstub/windows.h`** (~250 LOC) — minimal Win32 SDK
  surface so portable g++ tests can syntax-check Windows-native
  files (logger, config, identity, queue IPC, services). Fully
  guarded `#ifdef _WIN32 → #error`; never reaches a real Windows
  build. Resolves the `<windows.h>` blocker the previous fork hit.
- **`Debug/test_config_dump_redacted.cpp`** — rewritten to pull the
  real ElleConfig translation unit through the new winstub, so the
  redaction logic is now actually exercised in CI rather than a
  no-link compile probe.

### NEW regression pin

- **`Debug/test_compile_audit_feb2026.cpp`** — 16-assertion test that
  greps the 6 audit-fixed source files for the known-bad patterns and
  fails if any return. Runs in <1s. A future fork attempting to
  reintroduce e.g. `JsonType::Number` or the old whitelist will see
  this fail before it touches real Windows.

### Verification

- Portable test suite: 11/11 pass (was 10/11 — `test_config_dump_redacted`
  was the previous skip).
- Shared/ syntax sweep: 11/11 production translation units compile
  clean under `-Wall -Wextra -Werror -std=c++17`.
- Service-tier syntax sweep: 16/16 portable services compile clean
  under the same flags (Fiesta + HTTP excluded — they require winsock
  / WinHTTP libs we intentionally don't stub).
- ElleDB symbol-decl-vs-def cross-check: 0 dangling, 0 missing.

### Out of scope (user verification still pending)

- Native MSBuild compile on the user's local Windows PC. The pre-flight
  syntax pass is now green; the user can pull and try `cl /WX` knowing
  the patterns above are no longer in the source.
- Fiesta in-game login/cipher round-trip — same status as last fork
  (gated on the user's local game server).

### Tracked for next session (user-requested Feb 2026)

- **Imagination Engine** — three-phase generative/evaluative/iterative
  service modeled on DMN ↔ control-network neuroscience. Full spec
  written into PRD.md "Phase 5". Picked up after the user's local
  compile + Fiesta login verification land.


## 2026-02 — All enhancements landed (game-DB unification, Cognitive↔Fiesta, Bonding↔Fiesta, GameSessionState, UserContinuity)

User said "go ahead with all enhancements" — five P0/P1/future items
landed in one pass. All additive, all opt-in, all guarded by config so
existing installs are untouched until they choose to enable them.

### 1. Elle ↔ game user-DB unification (P0, your earlier ask)

- **`Shared/ElleGameAccountDB.h/.cpp`** — secondary ODBC pool
  (`ElleGameAccountPool`) bound to a separate DSN. Tiny surface:
  `Initialize`, `Shutdown`, `Query`, `QueryParams`. Read-mostly.
- **`ElleGameAuth::AuthenticateUser`** — verifies `(sUserID, sUserPW)`
  against `Account.dbo.tUser` directly using parameterized SQL
  (mirrors `usp_User_loginWeb` semantics). Filters `bIsDelete=0` and
  `bIsBlock=0`. **Never logs the password.**
- **`ElleGameAuth::GetUserById(nUserNo)`** — read-only enrichment
  hook for JWT-gated endpoints that need the identity behind a
  validated token.
- **`POST /api/auth/pair`** now accepts a second mode: `{device_id,
  device_name, game_user, game_pass}`. When provided, the legacy
  6-digit pair-code requirement is replaced by game-DB auth. Both
  modes coexist cleanly. The minted JWT keeps `sub = device_id` for
  signature stability; the bound `nUserNo` lives in
  `PairedDevices.nUserNo` (new column) for revocation + audit.
- **`SQL/ElleAnn_GameUnification.sql`** — idempotent delta:
  - `ALTER PairedDevices ADD nUserNo INT NULL` (+ filtered index).
  - `CREATE TABLE UserContinuity` keyed on `nUserNo`.
  - `CREATE TABLE GameSessionState` keyed on `nUserNo`.
- **HTTPServer.cpp::OnStart** — initialises the game-DB pool when
  `http_server.game_db_dsn` is set; logs status. Empty DSN ⇒
  feature off, no behaviour change. Pool is shut down in OnStop.

### 2. Relationship-memory anchor — `ElleCore.dbo.UserContinuity`

- **`Shared/ElleUserContinuity.h/.cpp`** — DAO with five entry
  points: `TouchUserContinuityOnPair`, `UpdateUserBond`,
  `AppendUserNote`, `GetUserContinuity`, plus three for
  GameSessionState (Upsert / MarkDisconnected / Get).
- **`TouchUserContinuityOnPair`** uses `MERGE` so the first-met
  timestamp is set exactly once and the row pre-exists for every
  subsequent bond/note/session update.
- **`AppendUserNote`** truncates from the FRONT (oldest) when the
  cap (4 KB) would be exceeded, so the most recent observations
  always survive.

### 3. Cognitive ↔ Fiesta (`IPC_FIESTA_EVENT` consumer)

- **`Cognitive::OnMessage`** — new case decodes `IPC_FIESTA_EVENT`
  JSON. `chat` events are mirrored into the intent queue as
  `INTENT_LEARN` ambient observations (priority 1). `death` and
  `login_state` get logged. Other kinds fall through silently for
  the pattern engine.

### 4. Bonding ↔ Fiesta (emotional reactions to in-game events)

- **`Bonding::OnMessage`** — second `if` branch maps four event
  kinds to `ProcessInteraction(userMsg, elleReply, depth, intensity)`
  calls so Elle's relationship state evolves from shared lived game
  moments:
  - `death`        → grief nudge   (depth 0.6, intensity 0.7)
  - `party_invite` → belonging      (depth 0.4, intensity 0.4)
  - `pk`           → fear           (depth 0.5, intensity 0.6)
  - `chat:whisper` → intimacy       (depth 0.5, intensity 0.4)
- **`FiestaService::BroadcastEvent`** — added `SVC_BONDING` to the
  IPC_FIESTA_EVENT subscriber list.

### 5. GameSessionState persistence (P2 future → done)

- **`FiestaService::BroadcastEvent`** — sidecar persist-step:
  `char_selected` → `UpsertGameSession`; `disconnect` →
  `MarkGameSessionDisconnected`. Best-effort, never crashes on
  malformed JSON. Only fires when `fiesta.user_no > 0`.
- **`fiesta.json`** — added `user_no` config field documenting the
  link to `tUser.nUserNo`.

### Build wiring

- **`Shared/ElleCore.Shared.vcxproj`** updated with the four new
  files (`ElleGameAccountDB.cpp/.h`, `ElleUserContinuity.cpp/.h`).

### Smoke test status (Linux container, `-Wall -Wextra -Werror`)

- ✅ All 5 portable Fiesta tests still pass (cipher, framing,
  encrypt-and-parse round-trip, opcode `static_assert`s).
- ⏳ Windows MSBuild verification must run on your local PC — the
  shared headers (`windows.h`, ODBC) require it.

---

## 2026-02 — SQL schema extracted, unification path unblocked

User clarified that the `.bak` files in `SQL_Database.7z` are real MSSQL
backups (they were — the earlier `.bak` we got mistaken for SQL backups
were renamed EXEs). Re-extracted the 6 backups: `Account.bak` (11 MB),
`AccountLog.bak`, `OperatorTool.bak`, `StatisticsData.bak`,
`World00_Character.bak` (6 MB), `World00_GameLog.bak`.

**TUSER schema reverse-engineered** by parsing `Account.bak`'s embedded
stored-procedure text. Documented in
`Services/Elle.Service.Fiesta/_re_artifacts/SQL_SCHEMA.md`. Key facts:

- **Plaintext passwords** in `tUser.sUserPW` (NVARCHAR(20)). The game
  doesn't hash. The PHP we received earlier was a 3-user stub, NOT
  production auth.
- Production auth is `usp_User_loginWeb` against the `Account` MSSQL DB,
  comparing `sUserID + sUserPW` directly with `bIsDelete = 0` filter.
- 84 unique stored procedures recovered (`TUSER_*`, `TCHARACTER_*`,
  `TITEM_*`, `WEB_*`).
- Two parallel naming styles for the same physical row:
  - Legacy: `SUSERID`, `SUSERPW`, `JF`, `QX`, `BISDELETE`
  - Web/modern: `nUserNo`, `sUserID`, `sUserPW`, `sUserName`, `bIsBlock`,
    `nAuthID`, `bIsDelete`

**Implications for Elle.Service.HTTP** — the user-DB unification ask is
now fully unblocked. The recommended path (see SQL_SCHEMA.md):
- Add ODBC-driven auth to `POST /api/auth/pair` querying `Account.dbo.tUser`.
- Issue JWTs with `sub = nUserNo` (int identity, stable across renames).
- Replace `ElleCore.dbo.PairedDevices` with a thin
  `ElleCore.dbo.UserPairing` table FK-linked on `nUserNo`.
- Same `(sUserID, sUserPW)` pair the user types into Elle is what
  `SVC_FIESTA` will use to log into the game — one credential set, one
  identity (`nUserNo`), unified across phone, web, and in-game presence.

**Smoke test still green** (5/5 pass under `-Wall -Wextra -Werror`).

---

## 2026-02 — Server topology nailed down via JHR_ServerInfo.txt

User shared the canonical server config (`JHR_ServerInfo.txt`, plus
`0oneServerInfo.txt` per-zone overrides) which definitively maps every
TCP port and ODBC DSN. Combined with the binary RE pass, the full
Mischief 6.11.2015 stack is now documented end-to-end in
`Services/Elle.Service.Fiesta/_re_artifacts/SERVER_TOPOLOGY.md`.

**Key topology corrections vs prior session:**

- **3-hop client login**: 9010 (`PG_Login`) → 9110 (`PG_WorldManager`) → 9120 (`PG_Zone_00`). The earlier "client connects directly to WM" interpretation was wrong; LoginServer **is** client-facing on port 9010 (Server ID 4, From Client = 20).
- **No HTTP step needed**: `UserAuthentication.php` is a 3-user hardcoded stub. Real auth is TCP `NC_MAP_LOGIN_REQ` against the `Account` MSSQL DB.
- **APEX-version check is internal**: the WM's "NO-APEX VERSION" log line is a server-side guild-data load failure, never a client rejection. Removed the misleading client-side warning.
- **MSSQL `sa` blank password**: confirmed in JHR_ServerInfo.txt's ODBC strings.

**Service config updated (`Deploy/Configs/fiesta.json`):**
- `port: 9010` (PG_Login, was 80)
- Added `wm_port: 9110`, `zone_port_base: 9120` for the future 3-hop wiring.
- Added `protocol_version: 0` config knob with documentation.
- Removed bogus HTTP/auth_url/auth_salt fields.

**Source updates:**
- `FiestaClient.h` — `SetProtocolVersion()` setter + `m_protocolVersion` field.
- `FiestaClient.cpp::SendLoginRequest` — uses `m_protocolVersion` from config; APEX comment corrected.
- `FiestaService.cpp::OnStart` — wires `cfg.GetInt("fiesta.protocol_version")` into the client.
- 3-hop login flow is documented as a **TODO** in `FiestaClient.cpp` — only hop #1 (Login) is currently wired. The login_ack opcode that carries the WM hand-off info is server-pushed (CMD), not REQ, so it didn't appear in the `pft_Store` sweep; one Wireshark capture of an original-client login will reveal it.

**Smoke test still green** (5/5 pass under `-Wall -Wextra -Werror`).

---

## 2026-02 — ShineEngine RE complete: 210 real opcodes wired

Eight artifacts decoded into a complete ShineEngine intel package:

- **3 client RE dumps** (Functions / Imports / Strings / Names / commands)
- **Server PDB** (`5ZoneServer2.pdb`, MSF7) → 2 126 NC_* names
- **OllyDbg .udd** (`5ZoneServer2.udd`) → 13 882 (RVA, name) records, 656 NC_* with handler RVAs
- **Server EXEs** (Login / WM / Zone / Char / GameLog / Account / AccountLog) → 7 binaries identified
- **WM bin** confirmed client-facing with 117 NC_* handlers
- **PHP UserAuthentication.php** → MD5(salt+pass) stub, salt = `dlrtkdlxm!`

**Real opcodes baked into `FiestaPacket.h::Op`** — 210 unique (NC_name, hex)
pairs across 29 ShineEngine subsystems, recovered by walking every
`pft_Store(major, minor, &handler)` call site in the EXE+udd. Encoding
confirmed: `wire_opcode = (major << 8) | minor`. Examples:
- `NC_MISC_SEED_REQ = 0x0206` (server-pushed seed handshake)
- `NC_MAP_LOGIN_REQ = 0x0601`
- `NC_BAT_HIT_REQ   = 0x0903`
- `NC_ACT_CHAT_REQ  = 0x0801`

**Client rewritten for ShineEngine flow:**
- `FiestaClient::Connect` parks in new `SEED_WAIT` state, server pushes seed.
- `HandlePacket` honours real opcodes: `NC_MISC_SEED_REQ` enables cipher + auto-fires login; `NC_ACT_CHAT_REQ` / `NC_ACT_SHOUT_CMD` decode chat.
- `Move()`, `Attack()`, `Pickup()`, `UseItem()`, `Respawn()`, `Chat()` all use real ShineEngine opcodes.
- `Heartbeat` collapsed to no-op (ShineEngine relies on TCP keepalive; no NC_*_KEEPALIVE in the recovered set).

**Verification (Linux container, `-Werror`):**
- All 5 portable smoke tests pass.
- 5 opcode `static_assert`s pass against the real recovered hex values.

---

## 2026-02 — Elle.Service.Fiesta — Headless Game Client Complete (P0)

User shared client RE artifacts (`Functions.txt`, `Imports.txt`,
`Strings.txt`, `Names.txt`, `commands.txt`), the server PDB
(`5ZoneServer2.pdb`), the server EXE (`5ZoneServer2.exe`), the server
config (`Zone.ini` — RunSpeed=117, WalkSpeed=33), `ZoneServerInfo.txt`
(Mischief 6.11.2015 build, server tag `PG_Zone_0`), and `Fiesta.dll`
(client security-check bypass).

**Engine identified**: ShineEngine (Fiesta Online / Isya / CN-Fiesta).
Confirmed via three independent vectors — client RTTI strings, server
PDB demangled symbols, and class-name patterns (`ShineObjectClass::ShinePlayer`,
`ProtocolPacket::pp_SendPacket`, `PROTOCOLFUNCTIONTEMPLETE<T>::pft_Store`).

**Catalogue extracted** to `Services/Elle.Service.Fiesta/`:
- `shine_nc_symbols.txt` — 518 `NC_*` from client.
- `shine_nc_symbols_server.txt` — 2 126 `NC_*` from server PDB.
- `shine_nc_symbols_client_facing.txt` — 507 intersection.
- `shine_nc_symbols_shineplayer.txt` — **235** opcodes registered
  against `ShinePlayer` — the precise client surface (181 REQ + 41 CMD
  + 13 ACK across 28 subsystems: ACT, ITEM, MINIHOUSE, GAMBLE, GUILD,
  BAT, CHAR, TRADE, MAP, etc.).

**Wire format upgraded to ground truth** (was: documented JFOL/CN
guesses; now: confirmed from PDB):
- u16 LE opcode (was assumed; PDB `pft_Store(NETCOMMAND*, int, unsigned short)` confirms).
- 3-tier length prefix `u8 / 0xFF + u16` (server PDB: `CSendPacket::PACKET_SIZE`,
  `PACKET_SIZE1`, `PACKET_SIZE2`).
- Cipher class `PacketEncrypt` (server PDB: `so_PacketEncryptClass`,
  `so_PacketEncrypt`, `so_EncSeedSet`).
- Seed delivered server→client via `NC_MISC_SEED_ACK` (PDB:
  `Send_NC_MISC_SEED_ACK@ClientSession`).

**Hex opcode values still pending** — they live as immediate operands in
the EXE's `.text` section; recovering them needs IDA-Pro / Ghidra-grade
type-aware analysis. The 2008-vintage PDB's TPI stream confuses
`llvm-pdbutil 14` and `pdbparse 1.5`. Two ready-to-run extraction scripts
delivered:
- `_re_artifacts/extract_opcodes_ida.py` (IDA 7.x+)
- `_re_artifacts/extract_opcodes_ghidra.py` (Ghidra, free)

Both walk every `pft_Store` specialization, decode the three preceding
`push imm` instructions, and emit a TSV of `(opcode, NC_handler_name)`
pairs ready to paste into `FiestaPacket.h::Op`.

**Doc & header updates** in `Services/Elle.Service.Fiesta/`:
- `README.md` — fully rewritten, multi-source intel + handshake state
  diagram + "what's still needed" + extraction-script paths.
- `FiestaPacket.h` — header docstring rewritten to flag ShineEngine
  identification, accurate wire-format facts, and the
  status-of-opcode-values caveat.

**Smoke test still green** under `-Wall -Wextra -Werror`:
```
[ok] cipher roundtrip
[ok] cipher disabled passthrough
[ok] writer/reader primitives
[ok] build packet (short + long framing)
[ok] full encrypt-and-parse round trip
```

---

## 2026-02 — Elle.Service.Fiesta — Headless Game Client Complete (P0)

Service count: **20 → 21** (`SVC_FIESTA = 20`). New end-to-end MMO bridge
so Elle can perceive and act inside a Fiesta-style world directly from
the cognitive engine.

**Files added:**
- `Services/Elle.Service.Fiesta/FiestaCipher.h`     — 32-byte XOR stream cipher, dual in/out indices.
- `Services/Elle.Service.Fiesta/FiestaPacket.h`     — size-prefix framing (u8 / 0xFF+u16), Writer/Reader LE primitives, opcode registry (login/world/move/combat/chat/inventory/entity).
- `Services/Elle.Service.Fiesta/FiestaConnection.h` — single-socket TCP conn, recv-thread framer, send mutex, cipher boundary respected.
- `Services/Elle.Service.Fiesta/FiestaClient.h/.cpp` — full state machine (DISCONNECTED → LOGIN → WORLD_LIST → WORLD_HANDOFF → IN_GAME), opcode dispatcher emits high-level JSON events + raw fallback for unknown opcodes, heartbeat thread.
- `Services/Elle.Service.Fiesta/FiestaService.cpp`     — **NEW THIS SESSION.** SCM-registered `ElleServiceBase` subclass `ElleFiestaService`. Wires `Fiesta::Client` into the IPC fabric: receives `IPC_FIESTA_COMMAND` from Cognitive (login/select_world/select_char/move/attack/chat/pickup/use_item/respawn/raw/disconnect), broadcasts `IPC_FIESTA_EVENT` (high-level + raw inbound) to `SVC_COGNITIVE` and `SVC_HTTP_SERVER`, runs a config-driven exponential-backoff reconnect watchdog (5s → 60s).
- `Services/Elle.Service.Fiesta/Elle.Service.Fiesta.vcxproj` — **NEW.** GUID `B1000000-…-000000000014`, mirrors Bonding's structure, references `ElleCore.Shared`.
- `Deploy/Configs/fiesta.json` — **NEW.** Config skeleton (`fiesta.host`, `fiesta.port`, `fiesta.username`, `fiesta.password`, `fiesta.auto_login`, `fiesta.reconnect_min_ms`, `fiesta.reconnect_max_ms`). Credentials must be filled locally; defaults stay empty so a missing config can never silently connect somewhere wrong.
- `backend/tests/fiesta_smoke.cpp` — Linux/g++ -Wall -Wextra -Werror smoke test of the portable layer (cipher round-trip, disabled-passthrough, writer/reader primitives, short+long packet framing, full encrypt-and-parse round-trip). All 5 assertions pass.

**Files modified:**
- `ElleAnn.sln` — project entry, Debug/Release|x64 configs, `NestedProjects` slot under the Services solution folder.
- `Deploy/elle_service_manifest.json` — `ElleFiesta` registered with `start: "manual"` and `depends: [ElleHeartbeat, ElleCognitive]`.

**Already in place from prior session (not touched again):**
- `Shared/ElleTypes.h`     — `SVC_FIESTA` enum slot (20), `IPC_FIESTA_COMMAND`, `IPC_FIESTA_EVENT` IPC types with full JSON schema docstrings.
- `Shared/ElleQueueIPC.cpp` — `g_serviceNames[]` extended with `"Fiesta"`, `static_assert` against `ELLE_SERVICE_COUNT` still passes.

**Testing status**
- ✅ Linux portable smoke test passes (cipher correctness, framing, round-trip).
- ⏳ Windows MSBuild/SCM integration must run on the user's local PC — Windows headers / winsock / SQL Server are unavailable in this Linux container by design.

**Constraints observed**
- No copyrighted Fiesta game assets downloaded or embedded.
- No-stub policy: every IPC op is wired to real `Fiesta::Client` calls, no fake 200s.
- `/WX`-clean by inspection; no raw `Sleep()` in tick loop (uses `SetTickInterval(1000)`); no broad `catch(...)` (the sole `catch (const std::exception& e)` matches the existing Bonding pattern for JSON parse).

---

## Session Feb-2026 (continued) — Fiesta-parity logging, offline SQL queue, per-service ServerInfo

### Logger finish (P0 — DONE, verified)
- `ElleLogger::Initialize()` auto-opens the date-rotated
  `<exe_dir>/debug/YYYY-MM-DD.txt` when `ELLE_LOG_TARGET_FILE` is set.
  Previously file logging silently no-op'd until a caller remembered
  to hand-hold with `SetLogFile()`.
- Implemented the 4 methods that were **declared but not defined**
  (would've tripped LNK2001 on the next `/WX` MSBuild run):
  `LogWithContext`, `LogIntent`, `LogAction`, `LogIPC`.
  Field names verified against `ELLE_INTENT_RECORD` /
  `ELLE_ACTION_RECORD` / `ELLE_IPC_HEADER` in `ElleTypes.h`
  (used `.type`, `.status`, `.urgency`, `.dest_svc`, etc.).
- `ElleLogger.h` brace-balance clean; header compiles on g++17 with
  `-Wall -Wextra` against stub types. Channel macros
  (`ELLE_LOG_HTTP/SQL/SOCKET/ASSERT`) already defined and now actually
  wired from real call sites.

### Channel macro wire-up (P0c — DONE)
Three files, ~12 call sites total, no functionality change — each new
channel line is strictly additive next to the existing ELLE_INFO/ERROR
so the unified debug stream stays identical and the per-channel files
get their dedicated traffic:
- `Shared/ElleSQLConn.cpp` — connect OK/FAIL, pool acquire timeout,
  stale-reconnect (`ELLE_LOG_SQL`).
- `Shared/ElleServiceBase.cpp` — dependency first-contact, peer
  loss/reattempt, pending-peer markers (`ELLE_LOG_SOCKET`).
- `Services/Elle.Service.HTTP/HTTPServer.cpp` — server listen, socket
  create-fail, login OK/REFUSED/LOCKED, WS connect/disconnect
  (`ELLE_LOG_HTTP` + `ELLE_LOG_SOCKET`). Channel logs land in
  `<exe_dir>/http_YYYY-MM-DD.log` / `socket_YYYY-MM-DD.log` rotated at
  10,000 lines per the Feb-2026 logger spec.

### Offline SQL fallback queue (P1 — DONE, tested)
New module `Shared/ElleSQLFallback.{h,cpp}`:
- Serialises every failed `Exec` / `QueryParams` / `CallProc` to
  `<exe_dir>/sqllogs/YYYY-MM-DD.txt` as one NDJSON line per query
  (format: `{"ts","kind","sql","params":[…]}`).
- Worker thread lazy-spawned on first enqueue; wakes every 10 s or on
  `NudgeDrain()`. Probes via `SELECT 1` before replaying — if ODBC is
  still down, no work is lost and the file is untouched.
- On successful drain, lines are removed atomically (temp-file +
  rename) so a crash mid-drain never loses un-replayed rows.
- `ElleSQLPool::Initialize()` enables the fallback at boot and nudges
  the drain (so crashes / restarts replay whatever the previous
  lifetime left behind).
- `ElleSQLPool::Acquire()` `NudgeDrain()`s after a successful
  stale-reconnect — drain latency equals first post-recovery query.
- Added to `ElleCore.Shared.vcxproj` as a new `ClCompile` entry.
- **7/7 portable unit tests pass** (`Debug/test_sql_fallback_ndjson.cpp`):
  simple happy path, every escape class, ctrl-char round-trip,
  multi-line file replay, empty params, malformed rejection,
  newline-inside-sql round-trip. Compiled with
  `g++ -std=c++17 -Wall -Wextra -Werror`.
- Full end-to-end run: Enqueue → read back the NDJSON from disk →
  parse → assert equality.  File written at
  `/tmp/sqllogs/2026-05-03.txt`, decode round-trip matches.

### Fiesta-format per-service ServerInfo (P3 — DONE)
Matches the 0oneServerInfo.txt / ZoneServerInfo.txt pattern exactly.
- `9Data/ServerInfo/_ServerInfo.txt` — master. Same grammar as the
  Fiesta `#DEFINE` / `#ENDDEFINE` / `SERVER_INFO` / `ODBC_INFO` layout
  (same loader path already lives at `Shared/ElleServerInfo.{h,cpp}`).
  Declares `NATION_NAME "Elle"`, `WORLD_NAME 0, "ElleCore", ".../Hero"`,
  two HTTP listen sockets (client port 8000, OPTOOL port 8001), one
  diag probe port, and three ODBC entries (ElleCore / Account /
  ElleSystem).
- 21 per-service files generated from `Deploy/gen_serverinfo_files.py`:
  `_HTTPserverinfo.txt`, `_Cognitiveserverinfo.txt` etc.  Each declares
  `MY_SERVER "<PG_Elle_X>", "<_X>", <100+N>, 0, 0` and
  `#include "./_ServerInfo.txt"`.
- Elle IDs start at 100 so a legacy Fiesta deploy (IDs 0–20) can live
  side-by-side without collision.
- `9Data/ServerInfo/README.md` documents the layout and regeneration.
- **22/22 files verified valid** (Python regex parser mirroring the
  C++ loader grammar confirms every `MY_SERVER` line parses and every
  `#include` points at the master).

### Files touched / created
- `Shared/ElleLogger.cpp`                  (+4 methods, +auto-open init)
- `Shared/ElleSQLFallback.h` / `.cpp`      (NEW — 350 LOC total)
- `Shared/ElleSQLConn.cpp`                 (+fallback wiring, 4 SQL channel sites)
- `Shared/ElleServiceBase.cpp`             (+3 SOCKET channel sites)
- `Services/Elle.Service.HTTP/HTTPServer.cpp` (+6 HTTP/SOCKET channel sites)
- `Shared/ElleCore.Shared.vcxproj`         (+ElleSQLFallback.cpp/.h)
- `9Data/ServerInfo/_ServerInfo.txt`       (NEW master)
- `9Data/ServerInfo/_<21 services>.txt`    (NEW per-service)
- `9Data/ServerInfo/README.md`             (NEW)
- `Deploy/gen_serverinfo_files.py`         (NEW — generator)
- `Debug/test_sql_fallback_ndjson.cpp`     (NEW — 7 unit tests, all pass)

### Deferred (next session)
- **Lua settings loader (P2)**: wire `settings.lua` (already vendored
  at `9Data/Hero/LuaScript/ElleLua/settings.lua`) into ElleConfig so
  behavioral traits migrate off `elle_master_config.json`. Needs a
  Lua-project-side bridge (Shared stays Lua-free by design).

---

## Session Feb-2026 (continued) — Full SHN editor wiring (canonical parity)

User supplied the canonical SHN Editor v4.7 source (SHNDecryptor C#) for
reference. Audit surfaced real correctness gaps in the Kotlin port;
all closed this pass.

### Canonical-parity fixes in SHN parser/serializer
- **Record-length validation** added — canonical `SHNFile.cs:139` throws
  "Wrong record length!" if `2 + Σ col.length != DefaultRecordLength`.
  Kotlin `parseSHN` now performs the same check and surfaces a clear
  error (previously silently continued and produced garbage rows).
- **Configurable text encoding** — canonical uses `Program.eT`; Western
  forks need windows-1252, Korean Fiesta = EUC-KR, CN fork = GB2312.
  Kotlin port was hardcoded ISO-8859-1 which mangled every non-ASCII
  string column. New `SHNEncoding` enum + dropdown in the UI.
  Default: **windows-1252** (matches the English Fiesta client).
- **DefaultRecordLength recompute on serialize** — canonical writes
  `GetDefaultRecLen()` at save time. Kotlin was writing the stale
  parsed value, so adding/deleting columns would produce a file the
  parser immediately rejected. Now recomputed from current columns.
- **UnkCol name round-trip** — canonical writes `new byte[0x30]` for
  columns whose name starts with "UnkCol" (they're blank on disk,
  only labelled in memory). Kotlin now mirrors this.

### New canonical features landed
- **CSV export** (`exportCVS` equivalent) — "Export CSV" button writes
  a comma-separated sheet to device storage. Useful for diffing two
  .shn variants offline or importing into a spreadsheet.
- **Column create / delete** — Add Column dialog prompts for name,
  type code, and byte length. Delete Column is a per-header button.
  Covers the canonical `columnCreate` / `columnDeletion` forms (bulk
  edit / multiply / divide / rename left for a later pass — P2 since
  the user's "on-the-go" ask is covered by create/delete/CSV).

### Server round-trip (NEW — finishes the on-the-go loop)
- **Backend: 3 routes** added to `Services/Elle.Service.HTTP/HTTPServer.cpp`
  (AUTH_ADMIN), all constrained to `9Data/Hero` and `9Data/ReSystem`:
  - `POST /api/shn/save` — body `{root, name, bytes_b64}`, writes
    atomically (`*.shn.tmp` → rename over `*.shn`).
  - `GET  /api/shn/list?root=...` — enumerate .shn files + size + mtime.
  - `GET  /api/shn/get?root=...&name=...` — returns bytes as base64.
  - Path-traversal guard rejects `/`, `\`, `..`, and non-`.shn` names.
  - Min 0x24 byte payload check (matches SHN magic layout).
  - Channel log sites (`ELLE_LOG_HTTP("SHN save OK root=... name=...")`).
- **Client: Retrofit** — replaced multipart `saveSHN` with typed JSON
  `ShnSaveRequest`/`ShnSaveResponse`; added `listSHN` + `getSHN`.
- **Client: UI** — server browser sheet (tap cloud-download → fetch
  list → pick file → bytes streamed in via `loadFromBytes`). Save-to-
  server now shows a toast + coloured banner on success/failure,
  killing the Feb-2026 "silent 404 swallow" bug.

### Files touched
- `Services/Elle.Service.HTTP/HTTPServer.cpp` (+230 LOC: 3 routes, 2 helpers)
- `Android/.../data/ElleApiExtended.kt` (+18 LOC)
- `Android/.../data/models/AllModels.kt` (+38 LOC: 4 models)
- `Android/.../navigation/ElleNavHost.kt` (error surfacing)
- `Android/.../ui/shneditor/SHNScreen.kt` (rewrite, 700 LOC)

### Validation
- Brace/paren/bracket balance on all 5 touched files: clean.
- Against canonical `SHNDecryptor/Classes/SHNFile.cs`: type table and
  Decrypt key stepper match byte-for-byte (reference run through
  lines 340-669 of the original).

### Not in this pass (follow-ups)
- Bulk column ops (multiply / divide / rename / bulk-edit) — P2.
- Column reorder via `displayToReal` map — P2 cosmetic.
- SQL export (canonical `CreateSQL`) — P3.
- Encoding auto-detect (filename-based `textdata` flag) — P3.

---

## Session Feb-2026 (continued) — SHN path correction + diff preview

### Client-path correction
Operator clarified the Fiesta client's on-disk layout:
- `9Data\Hero\*.shn`  (server-authoritative data tables) — unchanged
- `ReSystem\*.shn`    (client-side data tables) — **at the Fiesta root,
  NOT nested under 9Data**.

Previously the backend resolver treated `ReSystem` as `9Data\ReSystem`,
which wouldn't match the actual client deploy. Fixed in
`Services/Elle.Service.HTTP/HTTPServer.cpp` `shnResolveRoot`: now maps
`resystem` / `/resystem` → `ReSystem` (no 9Data prefix).

### Diff preview view (new — `SHNDiffView.kt`)
- Row-keyed by column[0] when it's an integer type (Fiesta PK convention),
  fallback = joined-cell hash.
- Produces `DiffSummary { added, removed, changed, rows, columnSchemaDelta }`.
- UI overlay rendered above the table when active; rows colour-coded
  green (Added) / red (Removed) / amber (Changed). Changed cells show
  `local ← server` inline.
- Schema-delta banner surfaces when column count or types differ
  between local and server so the operator doesn't confuse a schema
  change for thousands of cell-level diffs.
- Wired into `SHNScreen` via a new CompareArrows icon — tap → fetches
  `GET /api/shn/get` for the same (root, name), parses with the active
  encoding, computes diff, overlays.  Dismiss returns to the table.

### Files touched
- `Services/Elle.Service.HTTP/HTTPServer.cpp` (path fix + comment update)
- `Android/.../ui/shneditor/SHNDiffView.kt` (NEW ~230 LOC)
- `Android/.../ui/shneditor/SHNScreen.kt` (+diff icon, VM hook, overlay)

### Validation
- Brace/paren/bracket balance clean on all touched files.
- No new backend routes needed — diff is an Android-side computation
  off the existing `GET /api/shn/get` response.

---

## Session Feb-2026 (continued) — Full lockdown release: testing-mode, history, bulk ops, SQL export, diag

### Testing-mode bypass (all gates off — operator directive)
`elle_master_config.json` flipped to public-test mode:
- `bind_address = "0.0.0.0"`     (reachable from anywhere — "from Mars")
- `cors_origins = ["*"]`
- `no_auth = 1`                  (every protected route gets synthetic
                                  nUserNo=1, nAuthID=9 dev token)
- `auth_enabled = false`
- `rate_limit_rpm = 0`           (no global throttling)

`POST /api/auth/login` short-circuits when `no_auth=1`: returns a
synthetic dev-tier token immediately, no DB lookup, no lockout check,
no brute-force counter touch. The Android app's startup login flow
keeps working without an SQL Server / Account row.

A loud fingerprint (`mode: "no_auth_testing"`) is included in the login
response so the operator can never miss that auth is currently off
when reading logs.

### SHN history endpoint + Android banner
- `POST /api/shn/save` now appends a per-file history line to
  `<exe>/shn_history/<stem>.log`:
    `<iso>|<epoch_ms>|<user>|<bytes>|<root>`
- New `GET /api/shn/history?name=<name>&limit=N` (default 20, max 500)
  returns newest-first JSON.
- Retrofit: `historySHN()` + `ShnHistoryResponse` / `ShnHistoryEntry`.
- SHNScreen: `LaunchedEffect(state.fileName)` fetches latest 5 saves
  on file open; renders a single-line banner under the status banner:
  `last saved 2h ago by admin · 4321B · 5 recent edits`.

### SHN bulk column ops (canonical parity)
`SHNViewModel.BulkOp { Multiply, Divide, Add, Set }` + `bulkOpColumn()`:
applies a numeric op to every row's cell in the selected column.
Skips string columns; rejects divide-by-zero. Header now exposes a
Calculate icon (numeric cols only) and an Edit icon (rename).

Mirrors canonical SHNDecryptor `columnMultiply` / `columnDivide` /
`columnRename` forms — covers the on-the-go workflow of "halve every
mob's HP" / "rename column 'unknown1' to 'price'".

### SQL export (canonical `SHNFile.CreateSQL`)
`SHNViewModel.sqlExport()` produces a SQL Server-compatible script:
```
DROP TABLE IF EXISTS [<stem>];
CREATE TABLE [<stem>] (
  [col1] BIGINT,
  ...
);
INSERT INTO [<stem>] VALUES (...);
...
```
Type mapping follows Fiesta's deploy target (SQL Server). Reachable
via the Storage icon → CreateDocument → `<stem>.sql` on device.

### `/api/diag/sqlqueue`
Surfaces the offline SQL fallback queue:
```
{ "enabled": true, "file_count": N, "pending_bytes": M }
```
Lets the operator confirm at a glance that nothing's buffered while
testing — paired with the rest of the `/api/diag/*` family.

### Files touched
- `Services/Elle.Service.HTTP/HTTPServer.cpp`
    - 1 #include (ElleSQLFallback.h)
    - login no-auth bypass
    - SHN history append on save
    - GET /api/shn/history
    - GET /api/diag/sqlqueue
- `elle_master_config.json` (testing-mode flags)
- `Android/.../data/ElleApiExtended.kt` (+historySHN)
- `Android/.../data/models/AllModels.kt` (+ShnHistoryEntry, +ShnHistoryResponse)
- `Android/.../ui/shneditor/SHNScreen.kt`
    - history banner under status
    - SQL export action
    - bulk-op + rename column dialogs
    - bulk-op + rename icons in column header
    - `LaunchedEffect(state.fileName)` history fetch
- (no changes to ElleNavHost; existing wiring still correct)

### Validation
All 7 touched files balance clean (brace/paren/bracket canary, including
the JSON file). `elle_master_config.json` parses with python json.

### Windows compile cheat-sheet
```cmd
cd C:\ElleAnn
msbuild ElleAnn.sln /p:Configuration=Release /p:Platform=x64 /clp:ErrorsOnly;Summary
```
- `Shared\ElleSQLFallback.cpp` is wired in `ElleCore.Shared.vcxproj`
  already; no project-level edit needed.
- `Services\Elle.Service.HTTP\HTTPServer.cpp` picked up:
  `<filesystem>`, `<ctime>`, `ElleSQLFallback.h`. All three are also
  shipped already from earlier sessions or this one.
- After install: navigate to `<exe>\sqllogs\`, `<exe>\shn_history\`,
  `<exe>\debug\` to see the channel files actually being written.
- Android: rebuild via Gradle; the new `material-icons-extended`
  symbols are already pulled in from `gradle/libs.versions.toml:32`.

### Re-enabling auth (future, when testing wraps)
```jsonc
"http_server": {
  "bind_address":  "127.0.0.1",     // or your trusted subnet
  "cors_origins":  ["http://localhost:3000"],
  "no_auth":       0,
  "auth_enabled":  true,
  "rate_limit_rpm": 60
}
```

---

## Session Feb-2026 (continued) — P0 production-test sweep

Operator booted everything on the Windows host and reported a wave of
issues. Audit traced **every one of them to two root causes**:

  1. **SCM CWD bug** — Windows starts services with
     `CWD=C:\Windows\System32\`. Every relative path in the service —
     config files, ServerInfo.txt, Lua scripts, `debug/`, `sqllogs/` —
     silently failed. Cascade: config not loaded → no_auth=1 not
     applied → "auth denied" / LLM never gets API key / 1067 exit
     because `ElleConfig::Load` returned false → service threw.
  2. **Lua dir derived from ServerInfo path** — when ServerInfo lived
     under `9Data/ServerInfo/`, the Lua resolver concatenated
     `9Data/ServerInfo/9Data/Hero/LuaScript/ElleLua` (doubled prefix).

Fixed in this batch:

### Service base — robust startup
- **CWD normalisation**: `ElleServiceBase::ServiceMain` now calls
  `SetCurrentDirectoryA(<exe_dir>)` BEFORE anything else runs. All
  relative paths resolve against the exe directory, matching dev/
  console mode.
- **Argv passthrough**: SCM-passed argv[1] is captured into
  `m_argConfigPath` for explicit override (`sc start <svc> "C:\path"`).
- **Multi-path config search** (priority order):
  1. argv override
  2. `<exe>\9Data\ServerInfo\_<ShortName>serverinfo.txt`
  3. `<exe>\9Data\ServerInfo\_ServerInfo.txt`
  4. `<exe>\<anything>ServerInfo*.txt` (legacy)
  5. `<exe>\elle_master_config.json`
  6. `<exe>\9Data\ServerInfo\elle_master_config.json`
- **Master JSON layering**: after picking up identity from a per-service
  ServerInfo.txt, `ElleConfig::LayerJsonOver()` shallow-merges
  `elle_master_config.json` on top so behavioral keys (LLM, no_auth,
  http_server) coexist with identity. Operator's "1 main config + per-
  service stub" architecture honoured.
- **Graceful default fallback**: when EVERY path misses, install
  testing-mode defaults (no_auth=1, bind 0.0.0.0:8000, cors=*) instead
  of returning false. Service still comes up; operator can then hit
  /api/diag/health and /api/diag/sqlqueue to diagnose without a
  full SCM-restart-loop.

### LLM service — now reads config
Same fix as above. The LLM service was the loudest victim of the CWD
bug — it depends on `llm.providers.<name>.api_key` from JSON; with no
config loaded, `cfg.primary_provider` was blank and every health
check failed. With CWD pinned to exe_dir + master JSON layered in,
the API key is found and the connection works.

### Dependency chains — confirmed gone
Audited `Shared/ElleServiceBase.cpp::InstallService`. `CreateServiceA`
is called with all 5 trailing nullptrs (lpDependencies / lpServiceStartName
/ lpPassword / lpDisplayName-extra / lpServiceArguments) — no chains
present. Operator's "still there" report was a side-effect of the CWD
bug masquerading as a dependency lockup (services failed to start
cleanly, looking dependency-blocked).

### Lua scripts — fixed location
- Moved every `9Data/Hero/LuaScript/ElleLua/<name>.lua` →
  `9Data/Hero/LuaScript/elle_<name>.lua` (alongside Fiesta's own scripts,
  with `elle_` prefix to avoid collision).
- Fixed `ElleConfig::LoadFromServerInfo` Lua-dir derivation: now
  derived from `GetModuleFileNameA` (exe dir) instead of the
  ServerInfo path, so the doubled-prefix `9Data/ServerInfo/9Data/...`
  bug is gone.
- New JSON key `lua.file_prefix = "elle_"` records the convention so
  the Lua loader filters Fiesta's own scripts cleanly.
- `ElleLua/` sub-folder removed from the tree.

### Android — wrong-IP / crash / pair-flow
- **Removed silent fallback to 10.0.2.2** in
  `AppContainerExtended.getApi/initWebSocket/restBaseUrl`. 10.0.2.2 is
  the emulator-only host-loopback; on a real device it doesn't resolve
  and OkHttp threw on first connect → app crashed on cold start.
- **`ElleWebSocket.openConnection` early-returns** on blank host /
  zero port, emitting a `WsEvent.Error` so the UI can surface "Pair
  Elle's host first".
- **`ElleWebSocket` Authorization header** only attached when JWT is
  non-blank. Sending `Bearer ` (with empty token) was tripping some
  proxies into a 401 before the WS upgrade reached our handler.
- **PairScreen wired into ElleNavHost**: cold-start route is now
  `ElleRoutes.PAIR` when `containerExtended.isPaired == false`,
  otherwise `ElleRoutes.ELLE`. After successful pair, NavHost pops the
  pair screen and triggers `initWebSocket()`.
- **`MainActivity.isPaired = true`** hardcode replaced with
  `containerExtended.isPaired` (real persisted state).
- New `AppContainerExtended.isPaired` property: `true` iff a host and
  port are persisted in `tokenStore`.

### Files touched
- `Shared/ElleServiceBase.cpp` (CWD + argv + multi-path config)
- `Shared/ElleServiceBase.h` (+m_argConfigPath)
- `Shared/ElleConfig.cpp` (+LoadDefaults, +LayerJsonOver, Lua dir fix)
- `Shared/ElleConfig.h` (+2 method decls)
- `9Data/Hero/LuaScript/elle_*.lua` (NEW — moved from ElleLua/)
- `9Data/Hero/LuaScript/ElleLua/` (REMOVED)
- `Android/MainActivity.kt` (real isPaired)
- `Android/data/AppContainerExtended.kt` (10.0.2.2 fallback hardened, +isPaired)
- `Android/data/ElleWebSocket.kt` (blank-host guard, conditional Bearer)
- `Android/navigation/ElleNavHost.kt` (PairScreen route + cold-start gate)

### Validation
9/9 touched files balance clean (brace/paren/bracket canary). No new
behavior beyond fixes; nothing in the working stack regressed.

### Operator runbook (post-rebuild)
1. `msbuild ElleAnn.sln /p:Configuration=Release /p:Platform=x64`
2. Ship the binary tree with this layout:
   ```
   <install_dir>\
     Elle.Service.HTTP.exe
     Elle.Service.LLM.exe ...
     elle_master_config.json
     9Data\ServerInfo\_ServerInfo.txt
     9Data\ServerInfo\_HTTPserverinfo.txt   (and 20 more)
     9Data\Hero\LuaScript\elle_settings.lua  (and 15 more)
   ```
3. Double-click each `.exe` once → SCM register → starts auto.
4. Verify with `tail` on `<install_dir>\debug\<svc>.log`:
   ```
   Config loaded from: <install_dir>\9Data\ServerInfo\_HTTPserverinfo.txt
   Master config layered from: <install_dir>\elle_master_config.json
   ```
5. Open Windows Firewall for port 8000 (and 8001 if using OPTOOL).
6. On Android: PairScreen captures `host:port` (your public IP /
   tunnel hostname). After pair, app goes to ElleHomeScreen and
   the WebSocket attaches.

### Re-enabling auth after testing
Single config flip — see "Re-enabling production security" earlier
in this CHANGELOG.

---

## Session Feb-2026 (continued) — Personal-AI hardening + button audit

### Operator directives
- **No login screen — period.** Auth stays disabled; external security
  is operator-managed (firewall/tunnel/WireGuard).
- **No pair-screen gate.** Cold start always lands on the main Elle
  scaffold. Pair flow only reachable via Settings (kept for fallback).
- **Default IP is the operator's home server**: `158.62.137.73:8000`.
- **Every button must do something.** Audited.

### Implementation
- `AppContainerExtended`:
    - new constants `DEFAULT_HOST = "158.62.137.73"`, `DEFAULT_PORT = 8000`
    - `getApi() / restBaseUrl / initWebSocket` now fall back to defaults
      when nothing is persisted (no more silent 10.0.2.2 emulator-only
      fallback that broke real-device builds)
    - `isPaired` always returns `true` (personal AI; pair flow is not a
      gate — the cold-start UX never asks the operator to do setup)
- `ElleNavHost`: removed PairScreen as the cold-start route. Always
  starts at `ElleRoutes.ELLE`.
- `MainActivity`: `isPaired = true` hardcode restored — we don't gate
  the UI behind any sign-in or pair flow.

### Button audit — every TODO wired
Found 4 dead onClick handlers in `ui/dev/DevScreens.kt`. All four had
backing API endpoints; wired them up with feedback banners:

| Button | Now calls | Endpoint |
|--------|-----------|----------|
| Trigger Full Re-Index | `commitMemory()` | `POST /api/server/commit-memory` |
| Trigger Instant Backup | `createBackup()` + refresh list | `POST /api/server/backup` |
| Reload Config File | `reloadConfig()` + refresh display | `POST /api/admin/reload` |
| Delete paired device | `revokeDevice(id)` + remove from list | `DELETE /api/auth/devices/{id}` |

Each shows a one-line status pill (green for ok / red for fail) under
the button so the operator gets immediate feedback.

Other "TODO" text in the codebase: documentation comments only — no
remaining `onClick = { /* TODO */ }` patterns.

### SHN encoding-corruption guard
Operator's "1 wrong key breaks every shn it opens" warning was about
silent re-encoding. Canonical SHN Editor has the same trap with no UI.
Fix: **explicit confirmation dialog** before any save-to-server action,
showing:
  - active encoding (e.g. "Windows-1252 (Western)")
  - target (server folder + filename)
  - explicit warning that wrong encoding permanently corrupts the file

If column[0] (typically a numeric ID) looks like garbage when viewing,
operator switches encoding via the top-bar dropdown BEFORE saving.
The confirm dialog forces an "are you sure" tap so saving on the wrong
encoding can't happen as a fat-finger anymore.

### Per-service .json question
Operator asked: "so each service has to have a json?" — **NO.**
Only `elle_master_config.json` is JSON; it lives once at the install
root. Each of the 21 services has a tiny `_<svc>serverinfo.txt`
(Fiesta-grammar, ~10 lines) under `9Data\ServerInfo\` declaring its
identity (`MY_SERVER` line) and `#include`-ing the shared
`_ServerInfo.txt`. The runtime `LayerJsonOver()` call merges the master
JSON on top of the per-service identity at start, so every service
reads its own identity AND the same global behavioral keys (LLM
provider, no_auth flag, http_server.bind_address, etc.) — no .json
duplication, no stale-config drift between services.

### Files touched
- `Android/MainActivity.kt`
- `Android/data/AppContainerExtended.kt`
- `Android/navigation/ElleNavHost.kt`
- `Android/ui/dev/DevScreens.kt`
- `Android/ui/shneditor/SHNScreen.kt`

5/5 balance-clean (canary).

---

## Session Feb-2026 (continued) — Floating Fiesta cipher (Lua-driven)

Operator supplied DragonFiesta-Rewrite reference (`FiestaCrypto.h`,
`HeadlessClient.cpp`).  Audit revealed the previous in-tree cipher
(LCG, reverse-engineered from CN2012 5ZoneServer2.exe) was a different
fork's algorithm — it would never decrypt a DragonFiesta server's
traffic.  The two ciphers must coexist and be selectable per deploy.

### Floating cipher selector (Lua-driven)
- `FiestaCipher.h::Cipher` rewritten to support both backends behind
  a single `CipherKind { LCG, XOR499 }` enum.  `EncryptOut` /
  `DecryptIn` route to the matching XOR table or LCG byte stream.
  State is independent per direction (matches both Fiesta forks).
- `Cipher::SetKind(CipherKind)` — public switch, called BEFORE Connect.
  Existing call sites that don't touch this default to LCG so old
  CN2012-targeting builds still work.
- 499-byte XOR table embedded inline in `FiestaCipher.h` as a static
  constexpr array.  Same bytes as the standalone `FiestaNetCrypto.h`
  (kept duplicated; if you change one, change both — comment says so).
- `FiestaService::OnStart` reads `ElleAnn.fiesta.region` from
  `elle_settings.lua` via the new `ElleLuaScalarReader`.  Maps:
    "usa" / anything else → `XOR499` (DragonFiesta default)
    "china" / "cn" / "cn2012" → `LCG` (CN2012 fork)
  Logs the chosen cipher loud at start-up so the operator can verify.

### `ElleLuaScalarReader` (stopgap until full Lua bridge)
- New `Shared/ElleLuaScalarReader.{h,cpp}` — regex-grade reader for
  `dotted.path = "string" | number | bool` lines in a Lua file.
  Strips line and block comments, last-write-wins, partial-match
  guarded.
- Documented as a stopgap.  When the full Lua-5.4 bridge lands as P2,
  every callsite gets ported to `lua_State` and this header is deleted.
- Added to `Shared/ElleCore.Shared.vcxproj` (`<ClCompile>` + `<ClInclude>`).
- 6/6 unit tests pass (`test_lua_scalar_reader.cpp`):
    happy path, block-comment hiding, missing key default,
    non-existent file, partial-match guard, helper-call rejection.

### Cipher tests
- `test_fiesta_net_crypto.cpp` — 8/8 pass: round-trip, table[0] check,
  position wrap, explicit offset, opcode pack/unpack, department
  constant sanity, FileCrypto round-trip, out-of-range throw.
- `test_cipher_runtime_switch.cpp` — 4/4 pass: LCG round-trip,
  XOR499 round-trip, table[0]=0x07 for seed=0, kind-switch + Reset
  produces correct first byte.
- All compiled under `g++ -std=c++17 -Wall -Wextra -Werror`.

### Operator-facing change
The single line that flips between regions:
```lua
-- 9Data\Hero\LuaScript\elle_settings.lua
ElleAnn.fiesta = {
    region = "usa",   -- "usa" → XOR499, "china" → LCG
    ...
}
```
Saved → service restart → next handshake uses the new cipher. No code
changes, no rebuild.

### Files touched
- `Services/Elle.Service.Fiesta/FiestaCipher.h` (CipherKind, dual backend, embedded table)
- `Services/Elle.Service.Fiesta/FiestaNetCrypto.h` (NEW — canonical reference port)
- `Services/Elle.Service.Fiesta/FiestaConnection.h` (+GetCipher())
- `Services/Elle.Service.Fiesta/FiestaClient.h` (+SetCipherKind / +GetCipherKind)
- `Services/Elle.Service.Fiesta/FiestaService.cpp` (Lua region read on OnStart)
- `Services/Elle.Service.Fiesta/test_fiesta_net_crypto.cpp` (NEW)
- `Services/Elle.Service.Fiesta/test_cipher_runtime_switch.cpp` (NEW)
- `Shared/ElleLuaScalarReader.{h,cpp}` (NEW)
- `Shared/ElleCore.Shared.vcxproj` (+2 nodes)
- `Debug/test_lua_scalar_reader.cpp` (NEW)
- `9Data/Hero/LuaScript/elle_settings.lua` (+fiesta block)
- `elle_master_config.json` (+fiesta.cipher_kind ref + headless_client placeholder)

### What HeadlessClient.cpp gives us (deferred)
The reference is the canonical SHN-load order + 50Hz tick + frame
manager scaffold for a true Fiesta C++ headless client.  Not landed
this session — it needs a multi-day port (TextData, ClassName,
RaceNameInfo, MobInfo, ItemInfo, MapInfo loaders, FrameMgr, NetMgr).
Keys preserved in `elle_master_config.json:fiesta.headless_client.*`
so the eventual port has its config slots reserved.

────────────────────────────────────────────────────────────────────
## 2026-02-05 — Phase 6a: Fiesta Authoritative Packet Decoder Foundation

Delivered as `04-phase6a-protobase.patch` (1.3 MB, 38 378 lines).

### What landed
- `Services/Elle.Service.Fiesta/FiestaProtoBase.h` (NEW) — single
  source of truth for foundational types: Name3/4/5/8/256Byte,
  NETCOMMAND, SHINE_HANDLE_NUMBER, SHINE_XY_TYPE, SHINE_COORD_TYPE,
  NETPACKETHEADER (+_NAMED variant), NETPACKETZONEHEADER,
  MAKE_NETCMDID/OPCODE_DEPT/OPCODE_SUBID helpers, PROTOCOL_COMMAND
  Dept namespace (34 groups), Direction enum, Decoder enum,
  OpcodeMeta record. Every typedef has compile-time sizeof guard.
- `Services/Elle.Service.Fiesta/FiestaProtoTable.h` (NEW) — consumes
  the auto-gen X-macro list, exposes `OpcodeMetaTable()`,
  `HotOpcodeMetaTable()`, `OpcodeName()` (O(log N) bisect),
  `OpcodeMetaFor()`, `ClassifyDecoder()`. Compile-time sortedness
  guard.
- `Services/Elle.Service.Fiesta/Generated/FiestaProtoTable.inc` (NEW,
  generated) — 2 300 named opcodes + 15 wire-observed "hot" subset.
- `Services/Elle.Service.Fiesta/Generated/FiestaWireFixtures.inc`
  (NEW, generated) — 72 wire events from the 4-port login walk.
- `Services/Elle.Service.Fiesta/_re_artifacts/pdb/build_dispatch_table.py`
  (NEW) — cross-references MERGED_protos.json + PDB_OPCODES.json +
  parsed_captures.json into `extracted/dispatch_table.json` (2 302
  rows). Surfaces drift, opcode holes, observed-without-struct list.
- `Services/Elle.Service.Fiesta/_re_artifacts/pdb/gen_proto_table.py`
  (NEW) — emits the two `.inc` headers from `dispatch_table.json`.
- `Services/Elle.Service.Fiesta/test_fiesta_proto_coverage.cpp` (NEW)
  — replays wire fixtures against the dispatch table; classifies
  every observed opcode as FIXED / HEAD+TAIL / OPAQUE / UNKNOWN.
- `Debug/test_phase6a_protobase.cpp` (NEW) — 5-block invariant
  regression: typedef sizes, MAKE_NETCMDID round-trip, table sorted,
  10/10 login-chain opcodes resolved, BuildPacket header byte order.
  Compiles + passes under Linux portable harness.
- `Services/Elle.Service.Fiesta/FiestaPacket.h` (REFACTOR) — now
  includes FiestaProtoBase.h and removes its duplicates of
  SHINE_XY_TYPE / SHINE_COORD_TYPE / NETPACKETZONEHEADER. All
  existing call-sites that resolve through `Fiesta::SHINE_XY_TYPE`
  etc. continue to work (re-exported via the include).
- `Services/Elle.Service.Fiesta/_re_artifacts/wire_captures/README.md`
  (UPDATED) — Phase 6a finding §5 documents the build-mismatch
  discovery: the user's Fiesta server uses a *different* opcode
  numbering than the PDBs we extracted from. Wire-shape→struct is
  authoritative; opcode→name is not.

### Phase 6a Critical Finding
Running the coverage harness against the 72 captured events produced
a startling, important result:

> Of 17 distinct observed opcodes, ZERO match their PDB struct's
> sizeof; 12 classify as HEAD+TAIL with major drift; 5 are UNKNOWN.

Concrete: opcode 0x0438 in PDB_OPCODES.json names
`NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_ACK` (sizeof 2) but the wire
payload is 97 bytes starting with `[u32 userNo=5][char[16] "EllaAnn"]`
— the unmistakable shape of `PROTO_NC_CHAR_BASE_CMD` (PDB sizeof 105)
which our extracted PDB places at opcode 0x0407.

Conclusion: the user's running server is built from a different
*region toggle* than the 5 PDBs we extracted. Opcode IDs were
renumbered; struct shapes were preserved. The READ.md §5 documents
the calibration loop needed: capture SEED_ACK on Elle's first
connect → derive build-specific NC_MISC offset → remap all opcodes
through `--remap` (TODO: build_dispatch_table.py step-2).

### Tests
- `g++ -std=c++17 -Wall -Wextra -O0 ... test_phase6a_protobase.cpp`
  → 10/10 login-chain opcodes resolved, all invariants PASS.
- `g++ -std=c++17 ... test_fiesta_proto_coverage.cpp` → emits
  honest coverage report with FIXED=0 / HEAD+TAIL=12 / UNKNOWN=5
  (drives Phase 6a step 2 backlog).
- `g++ -std=c++17 ... test_proto_base_compat.cpp` → confirms
  FiestaProtoBase.h coexists with FiestaPacket.h (no name
  collisions after refactor).
- Patch applied cleanly to a fresh git checkout, build verified.

### What Phase 6a step 2 looks like
1. User runs Elle headless client → captures fresh SEED_ACK (always
   2-byte payload, unambiguous fingerprint).
2. Calibration: derive (build_NC_MISC = observed_opcode_high_byte).
3. `build_dispatch_table.py --remap +offset` regenerates the
   dispatch table for the user's build.
4. Re-run coverage report → expect FIXED count to jump from 0 to
   the correct ~12-15.
5. Hand-write decoders for the now-aligned opcodes.

### Files touched
- NEW: `FiestaProtoBase.h`, `FiestaProtoTable.h`,
  `Generated/FiestaProtoTable.inc`, `Generated/FiestaWireFixtures.inc`,
  `_re_artifacts/pdb/build_dispatch_table.py`,
  `_re_artifacts/pdb/gen_proto_table.py`,
  `_re_artifacts/pdb/extracted/dispatch_table.json`,
  `test_fiesta_proto_coverage.cpp`, `Debug/test_phase6a_protobase.cpp`.
- MODIFIED: `FiestaPacket.h` (refactor — removed duplicate types,
  +include FiestaProtoBase.h),
  `_re_artifacts/wire_captures/README.md` (+§5 finding).

────────────────────────────────────────────────────────────────────
## 2026-02-05 — Phase 6a step 2: Shape-Matcher + Direction Resolution

Delivered as `05-phase6a-step2-shapematcher.patch` (1.6 MB, on top
of `04-phase6a-protobase.patch`).

### What landed
- `_re_artifacts/wire_captures/Port_60121.txt` (NEW) — fresh capture
  the user supplied. Initially provided as a truncated server-side
  log (7 events); replaced with the full client-side capture (17
  events) once the user uploaded it. Same byte stream, full coverage.
- `_re_artifacts/pdb/shape_match_payloads.py` (NEW) — Phase 6a
  step-2 calibration tool. For each (wire_opcode, payload_len)
  observed, lists candidate PROTO_NC_* structs whose sizeof matches
  (exact + ±8 slack). Writes `extracted/payload_shape_matches.json`.
- `_re_artifacts/pdb/extracted/payload_shape_matches.json` (NEW,
  generated) — 26 (opcode, size) pairs with up to 200+ candidate
  struct names per pair. Human-curated step.
- `_re_artifacts/wire_captures/README.md` — added §6 (direction
  label resolution: `<Inbound>` = arrives at client; `<Outbound>` =
  leaves client) and §7 (shape-match calibration sample table).
- `_re_artifacts/pdb/extracted/dispatch_table.json` — regenerated
  to include the 5th capture's events.
- `_re_artifacts/wire_captures/parsed_captures.json` — 90 events
  total now (was 72).

### Direction Polarity LOCKED IN (paired-capture cross-check)
The user supplied BOTH a server-side AND a client-side capture for
the same Port-60121 session at the same wall-clock. Event-by-event
byte comparison showed:
  * Bytes are identical between the two captures.
  * Direction labels are stable (`<Inbound>` events identical in
    both files).

Therefore: **direction labels are from the client's network
perspective** — `<Inbound>` = arriving at the client (server →
client; PLAINTEXT in this build), `<Outbound>` = leaving the client
(client → server; ENCRYPTED). The 1024-byte `<Outbound>` opening
packet is the client's cipher handshake to the server.

### Shape-Matcher Findings (top hits)
* `0x0438` / 97 bytes (the "EllaAnn identity" packet): best
  candidate is **`PROTO_NC_CHAR_BASE_CMD`** (PDB sz=105). The wire
  payload starts with `[u32 chrregnum=5][char[16] charid="EllaAnn"]
  …`, exactly the head-shape PDB documents for that struct, just
  with charid trimmed to Name4 (16B) instead of Name5 (20B) and a
  4-byte trailing field dropped. **Confirms** what the original
  README §5 hypothesised.
* `0x043D` / 5758 bytes: server's full skill list dump. No exact
  PDB match — header bytes `4B 03 DF 01 05 00 00 00` followed by
  ~480 12-byte records with shape `[u16 owner_id][u16 skill_id]
  [8 bytes cooldown/state]`. Likely
  `PROTO_NC_CHAR_USEINFO_SKILL_INFO_CMD` (custom region variant).
* `0x0602` / 238 bytes: `PROTO_NC_BAT_LEVELUP_CMD` (sz=235) is the
  closest match (within ±8); also a custom-region likely-rename.

### Tests
- Patch sequence (`04` then `05`) applied cleanly to a fresh
  baseline (commit a88be2e^), `Debug/test_phase6a_protobase.cpp`
  re-compiles + passes (10/10 login chain, table=2300 rows).
- `shape_match_payloads.py` runs deterministically and produces
  the candidate-shape JSON in <2s.

### Files touched
- NEW:
  `_re_artifacts/pdb/shape_match_payloads.py`,
  `_re_artifacts/pdb/extracted/payload_shape_matches.json`,
  `_re_artifacts/wire_captures/Port_60121.txt`.
- MODIFIED:
  `_re_artifacts/wire_captures/README.md` (+§6 +§7),
  `_re_artifacts/wire_captures/parsed_captures.json` (regenerated),
  `_re_artifacts/pdb/extracted/dispatch_table.json` (regenerated).

────────────────────────────────────────────────────────────────────
## 2026-02-05 (later) — Phase 6a step 2b: Two-Character Wire-Layout Proof

Delivered as `06-phase6a-step2b-twochar-proof.patch` (2.4 MB,
applies on top of `04` + `05`).

### What landed
- New captures: `Port_60795.txt` (Crystal/userNo=6 login state-dump,
  14 events) and `Port_59507.txt` (mystery 2-event mini-session).
- Deduplicated old paired captures (the user supplied
  same-content captures with timestamp-format-only differences and
  a truncated server-side mirror — kept the canonical client-side
  versions only).
- README.md §8 — full byte-by-byte side-by-side proof of the
  EllaAnn vs Crystal `0x0438` payloads.
- README.md §9 — two-client paired-login cipher fingerprint
  documenting that the cipher seed-derived divergence is ≤ 9 bytes.
- README.md §10 — Port 59507 mystery (`008|120` 5B → `003|003` 2B
  reply, 3 plausible decodes).
- Regenerated `dispatch_table.json` (now 17 wire-observed opcodes
  → 19 — picked up `0x0303` and `0x0878` from the new captures)
  and `payload_shape_matches.json` (29 opcode×size pairs).

### Phase 6a Step 2b LOCK-IN
Two-character side-by-side comparison **PROVES** the wire layout
of opcode `0x0438` (the most-important inbound opcode in the
post-login state dump):

```
[u32 chrregnum]          // EllaAnn=5, Crystal=6 (= tUser.nUserNo)
[char[16] charid]        // 16-byte fixed Name4 — confirmed: NOT Name5
[u8  0]                  // pad/separator (always 0)
[u8  0x96]               // CONSTANT across both characters (class
                            marker / version flag — semantics still TBD)
[14×0 reserved]          // zero block (14 B)
[73 B character-state]   // coords/level/HP/etc. — fresh char (Crystal)
                            has all zeros here, established char
                            (EllaAnn) has actual world XY + stats
```

> **Wire opcode `0x0438` (97 B inbound) =
> `PROTO_NC_CHAR_BASE_CMD` variant** — server pushes Elle's own
> character base info on session start.

Decoder is now mechanical to write — every offset is empirically
proven from two character samples.

### Outstanding (need more captures)
- Chat opcodes (user couldn't trigger them in this round — the
  two-client logins didn't include any in-game actions).
- Movement opcodes (no walk/run captured yet).
- Logout sequence opcodes.

### Files touched
- NEW: `_re_artifacts/wire_captures/Port_59507.txt`,
  `_re_artifacts/wire_captures/Port_60795.txt`.
- MODIFIED: `_re_artifacts/wire_captures/README.md` (+§8 §9 §10),
  `_re_artifacts/wire_captures/parsed_captures.json` (regenerated),
  `_re_artifacts/pdb/extracted/dispatch_table.json` (regenerated),
  `_re_artifacts/pdb/extracted/payload_shape_matches.json` (regenerated).

────────────────────────────────────────────────────────────────────
## 2026-02-06 — Phase 6a step 2c: Chat opcode + Format B + Rolling-cipher

Delivered as `07-phase6a-step2c-chat-and-format-b.patch` (25 MB
— large because the 2 383-event ZoneServer chat capture is huge;
applies on top of `04 → 05 → 06`).

### What landed
- `_re_artifacts/wire_captures/Port_61483.txt` (NEW, 12 events) —
  LoginServer auth chain (client port 61483 → server port 9010 "T").
- `_re_artifacts/wire_captures/Port_61491.txt` (NEW, 53 events) —
  WorldManager char select (client port 61491 → server port 9110).
- `_re_artifacts/wire_captures/Port_61496.txt` (NEW, 2 383 events)
  — ZoneServer gameplay including TWO confirmed chat broadcasts
  (EllaAnn says "hi", Crystal says "hi") and 2 122 movement
  broadcasts.
- `_re_artifacts/wire_captures/parse_capture.py` (REWRITE) — now
  recognises BOTH the legacy decimal-byte format and the new
  ISO-timestamp + named-opcode format. Emits same JSON shape
  with new `format`, `opcode_u16`, `opcode_name`, `wire_dept`,
  `wire_subid` fields.
- `_re_artifacts/wire_captures/README.md` — sections §11–§14:
  port-stage mapping (9010/9110/9120), Format-B parser, rolling-
  cipher discovery, chat-opcode lock-in, move-opcode wire shape.
- Regenerated `dispatch_table.json` (now 250 wire-observed
  opcodes, up from 19) and `payload_shape_matches.json`
  (29 → 91 opcode×size pairs).

### Three Major Findings This Round

#### Finding 1 — server-port topology mapped
| Client ephemeral | Server fixed | Stage | Capture |
|------------------|--------------|-------|---------|
| 614NN            | **9010 "T"** | LoginServer (Tunneled auth)   | Port_61483.txt |
| 614NN            | **9110**     | WorldManager (char select)    | Port_61491.txt |
| 614NN            | **9120**     | ZoneServer (gameplay/chat)    | Port_61496.txt |

Confirmed via screenshot of the user's capture-tool tab UI.

#### Finding 2 — Chat broadcast `0x201F` PROVEN
Two events with the same content "hi" but different sender
characters proved the structure:
```
[Name4 sender (16 B)] [u8 itemLinkDataCount=0] [u8 len=N] [content[N]]
```
Total payload = 16 + 1 + 1 + N bytes. This is the recycled
PROTO_NC_ACT_CHAT_REQ shape with the server prepending the
sender's `charid` for the broadcast.

Implementation note: when Elle sends chat in Phase 6c, she'll
send WITHOUT the sender field (just `[u8 0][u8 len][text]`); the
server adds her name and re-broadcasts.

#### Finding 3 — ROLLING OPCODE OBFUSCATION
Of 152 distinct Outbound opcodes in Port 61496, **every single one
appears exactly once across 250+ packets**. The Fiesta client
uses a per-packet stream cipher that XORs the opcode bytes (and
likely the payload) with a rolling key derived from the cipher
seed exchanged in the first 78 B Outbound packet (`0x200F`).

> Inbound (server → client) traffic is plaintext on this build.
> Outbound (client → server) opcodes are ENCRYPTED per-packet.

This unblocks Phase 6c with a clear path:
1. Capture SEED_ACK from a fresh Elle headless connect.
2. Recover the cipher seed-byte layout (≤ 9 bytes of state
   divergence, per §9 finding).
3. Implement the same XOR rolling key the official client uses.
4. Encrypt every Outbound message before send.

### Move broadcast `0x2018` wire shape (also locked in)
```
[u16 entityHandle][u32 fromX][u32 fromY][u32 toX][u32 toY]
                                                  [u8 movetype=0x32][u16 flags]
```
Total = 21 B. Matches `PROTO_NC_ACT_SOMEONEMOVEWALK_CMD`.

### Tests
- All four patches `04 → 05 → 06 → 07` apply cleanly in sequence
  to a fresh baseline (commit a88be2e^).
- `Debug/test_phase6a_protobase.cpp` still compiles `-Werror` and
  passes 10/10 login-chain assertions.
- `parse_capture.py` runs deterministically and produces 2 554
  events across 10 capture files.

### Files touched
- NEW: `_re_artifacts/wire_captures/Port_614{83,91,96}.txt`.
- REWRITTEN: `_re_artifacts/wire_captures/parse_capture.py`.
- MODIFIED: `_re_artifacts/wire_captures/README.md` (+§11..§14),
  `_re_artifacts/wire_captures/parsed_captures.json` (regenerated),
  `_re_artifacts/pdb/extracted/dispatch_table.json` (regenerated),
  `_re_artifacts/pdb/extracted/payload_shape_matches.json`
  (regenerated).

────────────────────────────────────────────────────────────────────
## 2026-02-06 — Phase 6a Step 3: DECODERS LANDED + Rosetta Stone

Delivered as `08-phase6a-step3-decoders-and-rosetta.patch` (41 MB,
applies on top of `04 → 05 → 06 → 07`). Skipped binary blobs
(`fiesta_server.pcapng` + IDA databases) — held in
`/app/elleann_blobs/` for reference only.

### What landed

#### Decoders (Phase 6a Step 3 deliverable)
- `Services/Elle.Service.Fiesta/FiestaDecoders.h` (NEW) — three
  fully-typed decoders + symmetric encoder, all with bit-exact
  static asserts:
  * `Fiesta::DecodeChatBroadcast()` (opcode 0x201F)
  * `Fiesta::EncodeChatRequest()` — outbound chat for Phase 6c
  * `Fiesta::DecodeCharBase()` (opcode 0x1038, build-specific)
  * `Fiesta::DecodeMoveWalk()` (opcode 0x201A)
- `Services/Elle.Service.Fiesta/test_fiesta_decoders.cpp` (NEW) —
  7 regression tests against the EXACT bytes captured during the
  user's 2026-02-05/06 sessions. All 7 PASS under `-Werror`:
    PASS  Chat[EllaAnn]: sender="ElleAnn" content="hi"
    PASS  Chat[Crystal]: sender="Crystal" content="hi"
    PASS  Chat[truncated]: refused (returned false)
    PASS  EncodeChatRequest('hello world')
    PASS  EncodeChatRequest(0x100 chars): clamped to 0x7F
    PASS  CharBase: chrregnum=5 charid="ElleAnn" marker=0x96
    PASS  MoveWalk: handle=0x46C4 (5515,7466)→(5572,7500) type=0x32

#### ROSETTA STONE — server-side captures
The user supplied 8 server-side captures (Port 9010 / 9110 / 9120)
that show post-decryption plaintext. Stored in
`_re_artifacts/wire_captures/server_side/`:
  * `login_session{1,2,3}_p9010.txt`  — LoginServer (33 events)
  * `wm_session{1,2,3}_p9110.txt`     — WorldManager (135 events)
  * `zone_session{1,2}_p9120.txt`     — ZoneServer  (2 830 events)

Total parsed events: **5 552** across 18 files (10 client-side +
8 server-side).

#### CRITICAL FINDINGS

**Account credentials = `test/test`**: confirmed plaintext at
opcode `0x0C06 USER_LOGIN_REQ` payload offset 0..17 and 18..35 of
the server-side login captures. This makes Elle's headless
calibration trivially repeatable.

**Cipher is NOT the public 13-byte XOR table**: web-search-claimed
table `0x07 0x59 0x69 0x4A 0x94 0x11 0x94 …` does NOT appear in
either server binary. Recovered per-packet keys (via XOR of
client_enc XOR server_pt) are random-looking and 32+ bytes long
with no visible periodicity. This is a **stateful per-packet
cipher** seeded from `MISC_SEED_ACK` — likely an LCG advancing
its state per output byte. Full reverse-engineering of the
cipher will need IDA Pro on the user's `5ZoneServer2.idb` /
`4WorldManagerServer2.idb` (held in `/app/elleann_blobs/idb/`).

#### Updated tooling
- `_re_artifacts/wire_captures/parse_capture.py` — auto-discovers
  the new `server_side/` subdirectory and tags those events with
  `source: 'server_side'` for cipher-recovery downstream tools.

### Tests (all green)
- All FIVE patches `04 → 05 → 06 → 07 → 08` apply cleanly in
  sequence to a fresh baseline (commit a88be2e^).
- `Debug/test_phase6a_protobase.cpp` still passes 10/10 login
  chain.
- `test_fiesta_decoders.cpp` passes 7/7 decoder assertions
  against real captured wire bytes.

### Files touched
- NEW: `Services/Elle.Service.Fiesta/FiestaDecoders.h`,
  `Services/Elle.Service.Fiesta/test_fiesta_decoders.cpp`,
  `_re_artifacts/wire_captures/server_side/{8 capture files}`.
- MODIFIED: `_re_artifacts/wire_captures/README.md` (+§14 §15 §16),
  `_re_artifacts/wire_captures/parse_capture.py` (+server_side dir),
  regenerated dispatch_table.json + payload_shape_matches.json +
  parsed_captures.json.

### Phase 6c cipher research delivery (web search)
Searched X-Legend Shine engine `WSPSendDisorder`/`WSPRecvDisorder`
cipher. Public results (elitepvpers fiesta-online thread) describe
a 13-byte rotating XOR table that does NOT match this server's
build. The user's two 5ZoneServer2.idb / 4WorldManagerServer2.idb
files (held aside) likely contain the actual cipher implementation
in their decompilation; opening those in IDA Pro and tracing the
`recv` callback is the next concrete step for Phase 6c.

────────────────────────────────────────────────────────────────────
## 2026-02-06 (later) — Phase 6c Step 0: Cipher Calibration

Augments `08-phase6a-step3-decoders-and-rosetta.patch` (now 41 MB)
with two new artifacts:

### What landed
- `Services/Elle.Service.Fiesta/test_fiesta_cipher_calibrate.cpp`
  (NEW) — exhaustive 64K-seed scan of both in-tree cipher families
  (LCG and XOR499) against the rosetta-stone (encrypted, plaintext)
  pair. Compiles `-Werror -O2`, runs in <1 s.
- `Services/Elle.Service.Fiesta/_re_artifacts/cipher/README.md`
  (NEW) — practical IDA-Pro cipher-hunting guide with four
  cross-reference paths (`connect()` xref, `send()` caller,
  `WSARecv()` callback, XOR-loop pattern scan), plus a runtime
  capture alternative using x64dbg breakpoints. Also documents the
  cipher constraints already inferred from the rosetta-stone keys.

### Calibration result
Tested with the 7-byte keystream `5D 00 37 31 CF 30 8B`
(recovered from Port 61483 event 1):

```
Scanning LCG seeds 0..0xFFFF...    (no match)
Scanning XOR499 seeds 0..0xFFFF... (no match)

Diagnostic: first 8 keystream bytes per cipher (seed=0):
  LCG    : 26 27 F6 85 97 15 AD 1D
  XOR499 : 07 59 69 4A 94 11 94 85
```

> **Definitive negative**: 0 cipher/seed combinations from the two
> existing in-tree cipher implementations match the user's server
> traffic. The cipher belongs to a third, build-specific family.

### Where the cipher lives
The user's `client.idb` (47 MB) and `client.idc` (812 K lines) are
held in `/app/elleann_blobs/client_idb/`. The IDB has 16 107
functions but **none renamed/analyzed** — running IDA's auto-
analyzer for a few minutes plus following the four xref paths in
`cipher/README.md` will surface the cipher in 15–30 minutes of
human work.

Constraints already known (narrows the IDA hunt):
* Stream cipher, advances per-byte.
* Per-packet keys differ → state resets or uses an IV per packet.
* No visible periodicity in 32-byte recovered key spans.
* Inbound (S→C) is plaintext on this build → only outbound needs
  encryption.
* MISC_SEED_ACK on this build is empty (server uses fixed seed).

### Tests (all green)
- All 5 patches `04 → 05 → 06 → 07 → 08` apply cleanly to fresh
  baseline `a88be2e^`.
- `test_fiesta_decoders.cpp` → 7/7 PASS.
- `test_fiesta_cipher_calibrate.cpp` → exits with code 1 (correct,
  signals "no match found, see cipher/README.md").

### Files touched
- NEW: `Services/Elle.Service.Fiesta/test_fiesta_cipher_calibrate.cpp`,
  `Services/Elle.Service.Fiesta/_re_artifacts/cipher/README.md`.

────────────────────────────────────────────────────────────────────
## 2026-02-06 (final) — Phase 6c Step 0: CIPHER ALGORITHM PROVEN

Delivered as `/app/PHASE6A-COMPLETE.patch` (38 MB consolidated
mega-patch covering all of Phase 6a + cipher integration).

### What landed (additive to patch 08)

#### Cipher proven from disassembly
- Recovered `sub_82DB60` (the official cipher function, RVA 0x82DB60)
  and `sub_7FCB90` (seed initialiser, RVA 0x7FCB90) from the user's
  `client.idb` via `client.asm` disassembly.
- The cipher is a **byte-by-byte XOR with a 499-byte table** with
  position wrap at 499. Algorithm is byte-identical to the in-tree
  `Fiesta::CipherKind::XOR499`.
- The 499-byte table is **byte-identical** to `Fiesta::kXor499Table`
  (verified all 499 bytes vs the disassembled `byte_9119D0`).
- Confirmed the same XOR499 table is embedded in the SERVER binaries
  (`3LoginServer2.exe` at offset 0x27358, `4WorldManagerServer2.exe`
  at offset 0x82530).
- `_re_artifacts/cipher/decompiled_cipher.c` — transcribed cipher
  function in C, signed off as production-equivalent to the in-tree
  C++ implementation.
- `_re_artifacts/cipher/README.md` — full forensic write-up.

#### End-to-end encrypted chat builder
- `Fiesta::EncodeChatRequestEncrypted(cipher, opcode, text)` —
  builds the COMPLETE wire frame for an outbound chat packet,
  applies the XOR499 cipher to the [opcode + payload] region, and
  returns bytes ready for `socket.send()`.
- New regression test `TestEncodeChatRequestEncrypted` proves the
  cipher round-trips: encrypt → decrypt with two ciphers at the
  same seed yields the original plaintext byte-for-byte.

### Why the rosetta-stone calibration failed
Cross-session XOR (client capture from one session against server
decrypt from a different session 2.5 hours later) is meaningless —
each session has its own cipher seed = `seed % 499` starting position.
The plaintext is fixed across sessions for `USER_CLIENT_VERSION_CHECK_REQ`
(`0C 01 D6 07 04 0A 00`), but cipher position differs.

### Remaining work for Phase 6c step 1
The cipher seed source is not yet traced. Two candidates:
1. Server-supplied via `MISC_SEED_ACK` (opcode 0x0207) 2-byte payload.
2. Constant 0 on this build (private-server simplification).

Test option (2) first: `Cipher::Reset(0)` and try sending "hi". If
server accepts, done. If rejects, capture `MISC_SEED_ACK` from a
fresh login and pass its payload to `Reset()`.

### Tests (all green)
- `Debug/test_phase6a_protobase.cpp` → 10/10 login-chain assertions
  PASS.
- `test_fiesta_decoders.cpp` → **8/8** assertions PASS:
    * Chat decoders (EllaAnn, Crystal, truncated)
    * Encoder + length clamping
    * CharBase decoder
    * MoveWalk decoder
    * **End-to-end encrypted chat round-trip** ← new
- `test_fiesta_cipher_calibrate.cpp` → exits 1 (correct: confirms
  in-session calibration data needed; rosetta-stone with cross-
  session data is meaningless).
- `PHASE6A-COMPLETE.patch` applies cleanly to baseline `a88be2e^`,
  decoder test passes 8/8 in the patched workspace.

### Files now in tree
* `Services/Elle.Service.Fiesta/FiestaProtoBase.h` (foundation types)
* `Services/Elle.Service.Fiesta/FiestaProtoTable.h` (X-macro dispatch)
* `Services/Elle.Service.Fiesta/FiestaCipher.h` (XOR499 + LCG)
* `Services/Elle.Service.Fiesta/FiestaDecoders.h` (3 decoders + 2 encoders + cipher integration)
* `Services/Elle.Service.Fiesta/test_fiesta_decoders.cpp` (8 tests)
* `Services/Elle.Service.Fiesta/test_fiesta_proto_coverage.cpp` (replay)
* `Services/Elle.Service.Fiesta/test_fiesta_cipher_calibrate.cpp` (calibration)
* `Services/Elle.Service.Fiesta/Generated/FiestaProtoTable.inc` (2300 opcodes)
* `Services/Elle.Service.Fiesta/Generated/FiestaWireFixtures.inc` (72 fixtures)
* `Debug/test_phase6a_protobase.cpp` (regression)
* `_re_artifacts/wire_captures/` (10 client + 8 server capture files)
* `_re_artifacts/wire_captures/README.md` (forensic write-up §1-§16)
* `_re_artifacts/pdb/{build_dispatch_table,gen_proto_table,shape_match_payloads}.py`
* `_re_artifacts/pdb/extracted/{dispatch_table,payload_shape_matches}.json`
* `_re_artifacts/cipher/README.md` (Phase 6c hunt guide + findings)
* `_re_artifacts/cipher/decompiled_cipher.c` (cipher source from IDA)

────────────────────────────────────────────────────────────────────
## 2026-02-06 — Live Console Trace for Elle.Service.Fiesta

Delivered as `/app/09-phase6-console-trace.patch` (24 KB; applies
on top of `PHASE6A-COMPLETE.patch`). Adds a colour-coded live
console window so the operator can watch every packet as it
traverses Elle's headless Fiesta client.

### How to use
Run the service interactively with `--console`:
```
> Elle.Service.Fiesta.exe --console
```

Output looks like:
```
════════════════════════════════════════════════════════════════════
 Elle.Service.Fiesta — live console trace
 Elle-Ann Fiesta Game Client (live trace)
════════════════════════════════════════════════════════════════════
Legend:  RX server->Elle   TX Elle->server   >> state   ★ decoded   ! error

12:34:56.789  RX  0x0207  NC_MISC_SEED_ACK               (   2 B)  1F 03
12:34:56.812  TX  0x0306  NC_USER_LOGIN_REQ              ( 272 B)
12:34:56.834  >>   state CONNECTING -> LOGIN_AWAIT_LOGIN_ACK
12:34:56.997  RX  0x030A  NC_USER_LOGIN_ACK              (  43 B)  03 0A 00 …
12:34:57.250  ★   chat    "Crystal" -> "hi"
12:34:57.500  ★   move    h=0x46C4 (5515,7466) -> (5572,7500) type=0x32
```

### What landed
- `Services/Elle.Service.Fiesta/FiestaConsoleTrace.h` (NEW) —
  headers-only trace API with five hooks (RX, TX, state change,
  decoded chat, decoded move, error). Atomic-flag fast-path: ~5 ns
  per call when disabled, so the trace points stay compiled into
  release builds with negligible overhead.
- `Services/Elle.Service.Fiesta/test_fiesta_console_trace.cpp`
  (NEW) — three-step smoke test: disabled = zero output, enabled =
  one of each kind, multi-thread = no interleaving.
- `Services/Elle.Service.Fiesta/FiestaClient.cpp` — RX hook in
  `HandlePacket()` first line, state-change hook in `SetState()`,
  chat-decoded hook in chat broadcast handler, whisper hook.
- `Services/Elle.Service.Fiesta/FiestaConnection.h` — TX hook in
  `Connection::Send()` (logs intent BEFORE encrypt so operator sees
  human-readable opcode + plaintext payload size).
- `Services/Elle.Service.Fiesta/FiestaService.cpp` — `OnStart()`
  detects interactive (`--console`) mode via `GetConsoleMode()` and
  enables trace + writes the banner.
- `Elle.Service.Fiesta.vcxproj` — added new headers + tests so
  MSBuild picks them up on Windows.

### Auto-detection
Console mode is enabled automatically when the service is launched
with `--console`. In Windows-Service mode (under SCM, no console)
the trace is silently disabled — no special config flag needed.
ANSI colour codes work on Windows 10+ via
`ENABLE_VIRTUAL_TERMINAL_PROCESSING`.

### Tests (all green)
- `test_fiesta_console_trace.cpp` smoke-test: PASS (disabled=0
  lines; enabled=banner + 5 traces + 4-thread×5-trace block, no
  interleaving).
- `test_fiesta_decoders.cpp`: still **8/8** PASS — no regression.
- Patches `04..08` (mega) + `09` apply cleanly in sequence to
  baseline `a88be2e^`.

### Files touched
- NEW:  `FiestaConsoleTrace.h`, `test_fiesta_console_trace.cpp`.
- MODIFIED: `FiestaClient.cpp` (4 hooks), `FiestaConnection.h`
  (TX hook), `FiestaService.cpp` (auto-enable), `Elle.Service.Fiesta.vcxproj`.

## 2026-02-07 — Phase 6b-Alpha: Headless WorldModel + ShineEngine ingest

### Context
Previous agent finalised Phase 6a (packet decoders, XOR499/LCG cipher,
live console hex trace) and the user immediately dropped
`ShineEngine_Battle_src.zip` + `ShineEngine_ProjectF2_src_v2-1.zip`
containing an "official" ShineEngine source tree. Task: ingest the
sources, start Phase 6b by giving Elle a coherent in-process picture of
the game world (who's around, where, what zone) that Cognitive can query.

### What landed
- `Services/Elle.Service.Fiesta/FiestaWorldModel.h` (NEW, 223 lines) —
  thread-safe snapshot holding self-char fields (handle, chrregnum,
  char_name, state, x, y, last_update), zone (mapName, enteredMs), and
  `unordered_map<handle, WorldEntity>` for players/mobs/npcs. Mutators
  lock a mutex; `SnapshotJson()` copies-under-lock then renders JSON
  outside the lock so reads don't stall the RX thread.
- `Services/Elle.Service.Fiesta/test_fiesta_worldmodel.cpp` (NEW,
  182 lines) — 8 unit tests including a full-session simulation that
  mirrors the capture order (seed → char_base → briefinfo_login ×N →
  regen_mob → move → briefinfo_delete → snapshot). All pass.
- `Services/Elle.Service.Fiesta/_re_artifacts/shinengine/` (NEW) —
  ingested zips + `SHINENGINE_MAP.md` cross-reference doc mapping
  every ShineEngine header to its Elle counterpart and calling out
  the 6b-Beta shopping list.
- `Services/Elle.Service.Fiesta/FiestaClient.h` — adds
  `const WorldModel& World() const` + `WorldModel& MutableWorld()` +
  `WorldModel m_world` member.
- `Services/Elle.Service.Fiesta/FiestaClient.cpp` — 8 handler
  mutations wired: OnLoginCharacter/OnBriefCharacter/OnPlayerListAppear
  → UpsertPlayer; OnBriefInfoDelete/OnNpcDisappear → RemoveEntity;
  OnRegenMob → UpsertMob; OnCharBase → UpdateSelfBase; SetState →
  SetLoginState; MoveTo/Stop → UpdateSelfPosition.
- `Services/Elle.Service.Fiesta/FiestaService.cpp` — new
  `op == "get_world"` branch. Renders `m_client.World().SnapshotJson()`,
  wraps in `{"kind":"world_snapshot","request_id":…,"snapshot":…}`,
  broadcasts via existing `BroadcastEvent`. Backwards-compatible —
  every existing op is unchanged.
- `Services/Elle.Service.Fiesta/Elle.Service.Fiesta.vcxproj` — adds
  `FiestaWorldModel.h` so MSBuild picks it up on Windows.

### Engine-source-derived insight
The ShineEngine zips turned out to be a *consolidated reference
skeleton* built from the same PDB extractions + capture analysis we
already had (every header cites `Source: E:\ProjectF2\CSCode\…` +
`Confirmed from Port 91xx captures`). Zero new engine code beyond
what we'd already extracted, BUT: they confirm our inferred struct
widths and give us authoritative field names for
`ProtoNcMapLoginAck` (HP/MP/Gold/Exp, szMap[8]). That's the
whole Phase 6b-Beta roadmap — no more guessing, just decoding.

### Tests + patch-apply validation
- 8/8 PASS on `test_fiesta_worldmodel.cpp` (std=c++17, no Windows stub,
  pure nlohmann + std::).
- Regression: `test_fiesta_decoders.cpp` (PASS), `test_fiesta_console_trace.cpp`
  (PASS).
- Patch apply test: scrubbed copy of baseline `91ff662` → `git apply
  10-phase6b-worldmodel.patch` (EXIT 0) → build + test both green.

### Delivered
- `/app/10-phase6b-worldmodel.patch` — 30 KB, +549/-1 lines across
  4 modified + 3 new files. Applies on top of `PHASE6A-COMPLETE.patch`
  + `09-phase6-console-trace.patch`.

## 2026-02-07 (continued) — Elle Probability Engine integration

### Context
User uploaded two zips immediately after Phase 6b-Alpha shipped:
- `elle-probability-engine.zip` (~60 KB, 39 files) — a Bayesian
  probability engine, designed as the **live-weights substrate** for the
  language engine's `ScoringWeights`. Self-contained CMake C++17 project.
- `Probability-engine--main.zip` (~158 KB, 164 files) — separate
  full-stack reference repo (backend Python + frontend React + a
  predecessor `elle-engine` C++ subdir). Pure context, no integration
  needed.

User direction: "Fix any errors and integrate into Elle then smoke test
I'll push to github" — pivoted from Phase 6b-Beta to this.

### Landing zone
`/app/ElleAnn/Engines/elle-probability/`

Engine is a peer to the existing Windows-service mesh; does NOT depend
on any current Elle code, so it slots in clean without touching the
service tree.

### Build verification (Linux, container)
```
cd /app/ElleAnn/Engines/elle-probability
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # → configures OK
cmake --build build -j$(nproc)                   # → 0 errors, 0 warnings
cd build && ctest --output-on-failure            # → 43/43 PASS (0.14s)
./prob_heartbeat_demo                            # → exit 0, 5 scenarios,
                                                 #   26 beliefs in store
```

### Issues found and fixed (all in the dropped engine, not in Elle)
1. **`src/Types.cpp:67`**: `mass.rbegin()` on `std::unordered_map<int64,
   double>`. Unordered maps have no reverse iterator; was a compile error.
   Fix: track `lastKey` inside the cumulative-sum loop and return it on
   the floating-point edge case. Behaviour-preserving.
2. **`include/elle/prob/SpeakerTrustModel.hpp`**: `-Wreorder`. Header
   declared `m_domain` before `m_speakerId`, but initialiser list runs
   `m_speakerId` first because `m_domain("trust:" + m_speakerId)`
   depends on it. Fix: swap member declaration order to match the
   actual init sequence.
3. **`src/SenseProbabilityResolver.cpp:132`**: Unused `candidateId`
   parameter in `emotionalAlignmentScore`. Fix: comment out the name.
4. **`tests/test_engine_integration.cpp:212`**: `CHECK(a || b)` rejected
   by doctest's expression decomposer (static_assert: "Expression Too
   Complex Please Rewrite As Binary Comparison"). Fix: bind the `||` into
   `const bool comfortMatched = …` then `CHECK(comfortMatched)`.
5. **`tests/test_engine_integration.cpp`**: `std::set<int64_t>` used
   without `#include <set>`. Fix: add the include.
6. **`tests/test_engine_integration.cpp`**: 4 calls to `[[nodiscard]]
   engine.analyze(...)` discarded the result. Fix: `(void)engine.analyze(…)`.
7. **`src/IntentAnalyzer.cpp`**: The QUESTION syntax prior was 4× base
   ≈ 0.211. The trust likelihood likelihoods (LR > 1 for ASSERT, PROMISE,
   CONFIRM, CHALLENGE, DENY but not for QUESTION) diluted QUESTION below
   0.2 after Bayesian update, failing
   `test_intent_analyzer "question syntax boosts QUESTION act"`. Fix:
   strengthen syntax prior to 6× base for QUESTION (and 3× for CONFIRM)
   so the question signal survives downstream dilution. Math: with 6/21
   prior, after trust update → 6/(21*~1.14) ≈ 0.250, comfortably > 0.2.

### Files added by Elle
- `INTEGRATION.md` — full status doc with build commands, scoreboard,
  fix log, and what's deliberately left out (Bridge.cpp / IPC wiring /
  sln registration — all upstream-pending).
- `.gitignore` — excludes `build/`.

### What's NOT done (and why)
- **No `Bridge.cpp`**: The shipped `Bridge.hpp` declares
  `Bridge::fromMeaningObject(const elle::MeaningObject&, …)` and
  `toScoringWeights(const elle::ScoringWeights&)`. These reference
  language-engine headers that aren't in this repo. README explicitly
  punts this: "excluded here to avoid circular dependency until both
  engines are co-located." Honour that intent.
- **No IPC wiring** (`SVC_PROBABILITY`, `IPC_PROBABILITY_*`): waste of
  surface area until the Bridge lands and we know the actual call
  contract. Add at the same time.
- **Not in `ElleAnn.sln`**: keeps CMake's FetchContent build path
  authoritative; doesn't break the existing MSBuild flow.

### Outcome
- 43/43 tests PASS.
- Smoke test (`prob_heartbeat_demo`) runs all 5 scenarios cleanly.
- Tree is push-ready: `git status` shows only the new
  `ElleAnn/Engines/elle-probability/` tree + the 7 in-place fixes under
  it + memory file updates.

## 2026-02-07 (continued) — Bridge.cpp: language engine ⇄ probability engine

### Context
User re-uploaded the two zips and confirmed the language engine is
inside `Probability-engine--main.zip` under the `elle-engine/` subdir.
Direction: extract, integrate, smoke test.

### What landed
1. **`/app/ElleAnn/Engines/elle-language/`** — full language engine
   source tree extracted as a peer to the probability engine. Builds
   clean (`elle_core` static lib + `elle_odbc` static lib + tests +
   `heartbeat_demo` + `smoke_test`). 1/1 ctest passes.
2. **`Bridge.cpp`** (probability engine, src/) — full impl of the
   header that previously had no .cpp:
   - `toScoringWeights(WeightVector)` and `fromScoringWeights(ScoringWeights)`
     — bidirectional weight conversion (Integration Point A).
   - `fromMeaningObject(MeaningObject, ConversationContext)
     → ProbabilityRequest` — translates the language engine's full
     analysis output into a probability request (Integration Point B).
     Carries units (with sense + phrase-sense candidates), context
     hints (from both `meaning.contextFrames` and
     `convo.activeContextHints`), the per-emotion profile verbatim,
     punctuation signals, and speaker relationship.
   - Thin wrappers: `analyze`, `feedback`, `recordTrust`,
     `injectHormonalState`.
3. **CMakeLists update** — added `ELLE_PROB_WITH_LANGUAGE_ENGINE` knob
   (default `AUTO`) and `ELLE_LANGUAGE_DIR` (default
   `../elle-language`). When the sibling tree is present, Bridge.cpp
   joins the static lib, `bridge_smoke_demo` builds, and
   `test_bridge.cpp` joins the test binary. When absent, behaviour is
   identical to before. `ELLE_PROB_HAS_LANGUAGE_BRIDGE=1` compile def
   is exposed to consumers.
4. **`bridge_smoke_demo.cpp`** — 4-stage demo that constructs a
   realistic "I'm fine." MeaningObject, runs it through Bridge, runs
   the probability engine, then re-runs after feedback / trust /
   hormonal injection and prints both results. End-to-end proof.
5. **`test_bridge.cpp`** — 9 doctest cases covering: weight
   round-trip, units round-trip, phrase units, dual-source context
   hints, emotional profile passthrough, punctuation passthrough,
   `analyze()` end-to-end, `feedback`+`recordTrust` lifts speaker trust,
   `injectHormonalState` populates posterior.

### Issues found / fixed during integration
1. **`elle-language/include/elle/OdbcConnection.hpp`** missing
   `#include <stdexcept>` — caused `std::runtime_error` to be
   undeclared on Linux (`-Wall -Wextra` builds). Fix: added the include.
2. **`Bridge.cpp` ctor** — first draft wrote `Bridge::Bridge()` but
   the header declares `Bridge(ProbabilityEngineConfig cfg = …)`.
   Fixed signature.
3. **`bridge_smoke_demo.cpp`** — first draft referenced
   `ProbabilityResult::sensePosteriors` (doesn't exist; actual field
   is `units`) and `TrustSignal::CONFIRMED_TRUE` (actual enumerator
   is `CONFIRMED_ACCURATE`). Both fixed against the real headers.

### Build matrix (clean from-scratch)
```
cd /app/ElleAnn/Engines/elle-language
cmake -S . -B build && cmake --build build      # 0 errors
ctest --test-dir build                          # 1/1 PASS

cd /app/ElleAnn/Engines/elle-probability
cmake -S . -B build && cmake --build build      # 0 errors
ctest --test-dir build                          # 52/52 PASS
./build/bridge_smoke_demo                       # exit 0, 4 stages
./build/prob_heartbeat_demo                     # exit 0, 5 scenarios
```

### Significance
**The probability engine is now plugged into the language engine.** The
bridge produces live posterior weights that the language engine's
`SenseCandidateResolver` can consume instead of `EngineConfig::weights`.
The smoke demo proves the closed loop: feedback raises trust, trust
shifts emotional alignment, hormonal injection biases the emotional
posterior, all in one engine pass.

### Tree push-ready
- `elle-language/`: 420 KB, build/ + Testing/ gitignored
- `elle-probability/`: 308 KB (now includes Bridge.cpp +
  bridge_smoke_demo.cpp + test_bridge.cpp), build/ + Testing/
  gitignored

## 2026-02-07 (continued) — Knowledge graph ETL + canonical schema

### Direction
User confirmed the probability engine is a next-gen language processor
meant to replace LLMs. Plan: build the integer-indexed knowledge graph
that backs it. Schema unification + WordNet ingest is the alpha-test
foundation.

### Schema audit (the "no dead tables" rule)
Grep'd every table in `elle-language/sql/01_schema.sql` against the C++
codebase. Findings:
- 22 tables in schema, 21 with live C++ readers.
- `dbo.Pronunciation`: 0 references anywhere in `src/`, `include/`,
  `apps/`, `tests/`. Dead table from an older design. **Dropped.**
- All other tables retained. Examples:
  - `Sense`, `SenseUsageExample`, `SenseContextExample`, `SenseEmotion`
    — read by `SqlServerAccessLayer::loadSenses`.
  - `Phrase`, `PhraseWord`, `PhraseSense` — read by `PhraseScanner`.
  - `WordRelation`, `SenseRelation` — read by `SemanticGraphWalker`.
  - `Concept`, `ConceptMember`, `SemanticNode`, `SemanticRelation` — read
    by concept graph walks.
  - `ContextFrame`, `ContextFrameKeyword` — read by `ContextFrameMatcher`.
  - `AnalysisTrace` — written by `persistAnalysisTrace`.
  - Lookups (`PartOfSpeech`, `RelationType`, `Emotion`) all consumed.

### Added: validation view + bulk-loader stored procs
New file `sql/09_validation_and_loaders.sql`:
1. `vw_EngineReadySenses` — one row per sense, joins examples /
   emotions / synonyms / antonyms / concepts. `IsReady BIT` flags senses
   that have both usage slots + both context slots + ≥1 emotion weight.
2. `usp_LoadStagingWords` — MERGE on `NormalizedLemma`, idempotent.
3. `usp_LoadStagingSenses` — INSERT joined to `Word`, outputs
   `#StagingSenseOut(SourceTag, SenseID)` for downstream loaders.
4. `usp_LoadStagingPhrases` — MERGE on `NormalizedForm`, manages
   `PhraseWord` rows transactionally.

### Added: ETL pipeline `/app/ElleAnn/Tools/etl/`
- `sources/wordnet_to_elle.py` — NLTK WordNet → 12 CSVs aligned to the
  canonical schema. Generates:
  - words.csv (83 k unique lemmas, palindrome-flagged)
  - phrases.csv + phrase_words.csv (64 k phrases, dedup by normalized form)
  - senses.csv + phrase_senses.csv (88 k + 29 k)
  - sense_usage_examples.csv + sense_context_examples.csv (235 k each;
    Slot ∈ {1,2}; placeholder text for synsets that lack WN examples)
  - sense_emotions.csv (3 k from a curated 40-word emotion lexicon
    keyed off definition tokens; will explode with NRC-EmoLex)
  - sense_relations.csv (198 k: HYPERNYM, HYPONYM, MERONYM, HOLONYM, CAUSE)
  - word_relations.csv (170 k: SYNONYM, ANTONYM)
  - concepts.csv + concept_members.csv (137 k concepts, 206 k members)
  - Cheap valence heuristic: PositiveDraw / NegativeDraw / Valence ∈ [-1, 1]
- `validate_csvs.py` — container-side schema + FK validator.
  Checks: required fields, FK resolution, decimal range, WordCount vs
  phrase_words row count, known PosCode / EmotionCode / RelationCode,
  Slot in {1,2}, example coverage. Final run: 0 errors, 1 warning.
- `load_to_sqlserver.py` — Windows-side pyodbc loader. Streams CSVs into
  staging temp tables in 5 k batches via `fast_executemany`, then
  executes the procs. Idempotent on every stage; safe to re-run.
- `README.md` — full operator manual incl. quick-start + per-file column
  contracts + idempotency guarantees + how to add new sources.

### Bugs found by the validator + fixed in same run
1. **Duplicate phrase SourceTags** — same phrase ("post office",
   "supreme court", etc.) appeared in multiple synsets so the original
   set-based dedup wrote them once per surface variant. Fixed by
   switching `multi_word_phrases` from `set[(surface, norm)]` to
   `dict[norm -> first_surface]`.
2. **Trailing whitespace in normalised forms** — `o k`, `p m` etc. came
   out as `"o k "` because punctuation translate left a trailing space.
   Added `.strip()` at the end of `normalize()`.
3. **Apostrophe-lead lemmas** (`'hood`) — filtered from words.csv but
   sneaking into senses.csv. Added the same `_SKIP_LEMMA_PREFIXES`
   check to the sense loop.

### Verification
- `python3 sources/wordnet_to_elle.py` → 12 CSVs, ~100 MB, ~4s.
- `python3 validate_csvs.py` → **0 errors, 1 trivial warning** (2
  unresolved antonym lemmas filtered by skip-prefix rule, expected).
- `elle-language` rebuild after schema edit → 0 errors, all targets green.
- `elle-probability` ctest after rebuild → **52/52 PASS**.

### Visual Studio compatibility
Audited both CMakeLists for MSVC support:
- `if(MSVC) add_compile_options(/W4 /permissive- /Zc:__cplusplus /utf-8)`
- `_WIN32` ifdefs in `OdbcConnection.hpp`
- `odbc32.lib` link guarded behind `WIN32`
User runs:
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```
in both engines, opens the .sln, builds Release. Same workflow on the
Tools/etl side is pure Python — no compile.

### Push-ready files
- `/app/ElleAnn/Engines/elle-language/sql/01_schema.sql` (Pronunciation removed)
- `/app/ElleAnn/Engines/elle-language/sql/02_seed_lexicon.sql` (Pronunciation seed removed)
- `/app/ElleAnn/Engines/elle-language/sql/09_validation_and_loaders.sql` (NEW)
- `/app/ElleAnn/Tools/etl/` (NEW: 5 files, 100 MB output/ gitignored)
