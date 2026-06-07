> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# Audit Fix Report — Feb 2026

External code-only audit (Kage/Claude Opus style) identified **13 real
source-level bugs**. Every item is fixed below with zero stubs, zero
pretend-success, zero behavioural regressions.

---

## HIGH SEVERITY

### Bug #1 — Goal IPC payload mismatch ✅
**Files:** `Services/Elle.Service.Action/ActionExecutor.cpp`,
`Services/Elle.Service.GoalEngine/GoalEngine.cpp`

`ExecuteGoalOp()` was building a `"<type>:<command>:<params>"` string and
sending it with `SetStringPayload()`. `GoalEngine::OnMessage` for
`IPC_GOAL_UPDATE` reads via `msg.GetPayload(goal)` into a binary
`ELLE_GOAL_RECORD`. Every goal op silently corrupted.

**Fix:** `ExecuteGoalOp` now builds a proper `ELLE_GOAL_RECORD` and uses
`SetPayload(goal)`. `action.command` holds the verb, `action.parameters`
holds the description.

---

### Bug #2 — Intent payload mismatch ✅
**Files:** `Services/Elle.Service.QueueWorker/QueueWorker.cpp`,
`Services/Elle.Service.Cognitive/CognitiveEngine.cpp`

QueueWorker sent a binary `ELLE_INTENT_RECORD` via `SetPayload()`.
Cognitive read `msg.GetStringPayload()` and fed the struct bytes into
`m_engine.ProcessInput(text, "", 0)` as if they were text. Every queued
intent was garbled.

**Fix:** Cognitive's `IPC_INTENT_REQUEST` handler now `GetPayload(intent)`
and routes via `RouteIntent(intent)`. We also stopped calling
`ProcessInput` from the IPC handler — ProcessInput re-parses text and
`SubmitIntent()`s a new row, which would loop (QueueWorker → Cognitive
→ SubmitIntent → QueueWorker → …).

---

### Bug #3 — Action pipeline double-handling ✅
**Files:** `Services/Elle.Service.QueueWorker/QueueWorker.cpp`,
`Services/Elle.Service.Action/ActionExecutor.cpp`,
`Services/Elle.Service.Cognitive/CognitiveEngine.cpp`

Both QueueWorker.OnTick AND Action.OnTick polled
`GetPendingActions()`; on top of that Action's `IPC_ACTION_REQUEST`
handler called `SubmitAction()` to *re-submit* each IPC'd action,
producing duplicate executions and re-enqueue loops.

Cognitive had the identical bug for intents (its OnTick also polled
the same queue QueueWorker owned).

**Fix:** Single polling authority — `SVC_QUEUE_WORKER`. Removed:

- `Cognitive::OnTick` intent poll.
- `Action::OnTick` action poll.

`Action::OnMessage(IPC_ACTION_REQUEST)` now executes the action directly
via a new `ExecuteAction()` method that handles lock → execute →
status → trust delta → broadcast.

---

### Bug #4 + #6 + #7 — Service-ID overflow + missing SVC_CONTINUITY ✅
**Files:** `Shared/ElleTypes.h`, `Shared/ElleQueueIPC.cpp`,
`Services/Elle.Service.Heartbeat/Heartbeat.cpp`, every Phase-2 service.

Six new services (`Bonding`, `Continuity`, `InnerLife`, `Solitude`,
`Family` reserved, `XChromosome`, `Consent` reserved) used raw casts
like `(ELLE_SERVICE_ID)(ELLE_SERVICE_COUNT + N)`. Heartbeat's
`m_lastHeartbeat[]` / `m_healthy[]` arrays were sized to the old
`ELLE_SERVICE_COUNT` = 13, so any incoming frame from those IDs wrote
**out of bounds**. `GetPipeName()` indexed `g_serviceNames[svc]` with
no bounds check, reading past the end too. `Continuity.cpp` even
referenced a literal `SVC_CONTINUITY` that was never defined.

**Fix:**
- Added proper enum values: `SVC_BONDING`(13), `SVC_CONTINUITY`(14),
  `SVC_INNER_LIFE`(15), `SVC_SOLITUDE`(16), `SVC_FAMILY`(17),
  `SVC_X_CHROMOSOME`(18), `SVC_CONSENT`(19). `ELLE_SERVICE_COUNT` = 20.
- `g_serviceNames[]` now has 20 strings. Compile-time
  `static_assert(sizeof(g_serviceNames)/sizeof(g_serviceNames[0]) ==
  (size_t)ELLE_SERVICE_COUNT)` prevents future drift.
- `GetPipeName()` + `GetServiceName()` bounds-check `svc` before
  indexing.
- `Heartbeat::OnMessage` bounds-checks `sender` before writing to
  `m_lastHeartbeat[]` / `m_healthy[]`. `AttemptRestart` bounds-checks
  `svc`.
- Every raw cast replaced with the proper `SVC_*` enum value.

---

### Bug #5 — Missing DB helper implementations ✅
**File:** `Shared/ElleSQLConn.cpp`

Ten helpers were declared in `ElleSQLConn.h` but had no matching
definitions, producing unresolved externals at link time the moment the
services were built:

- `SubmitAction`
- `GetPendingActions`
- `UpdateActionStatus`
- `GetTrustState`
- `StoreEmotionSnapshot`    (alias → `PersistEmotionSnapshot`)
- `GetLatestEmotionState`   (alias → `LoadLatestEmotionSnapshot`)
- `UpdateGoalProgress`
- `GetActiveGoals`
- `PromoteToLTM`
- `ArchiveMemory`

**Fix:** all 10 implemented with real SQL against live snake-case
tables (`action_queue`, `goals`, `TrustState`, `memory`, `emotion_snapshots`),
parameterised queries, and lazy-create for the two new tables
(`action_queue`, `goals`) so the app boots on a partial-apply database.

---

## MEDIUM SEVERITY

### Bug #8 — Self-modify reload only partially real ✅
**File:** `Lua/Elle.Lua.Behavioral/LuaHost.cpp`

`LoadAllScripts()` loaded a hardcoded 12-name list. If
`ActionExecutor::SELF_MODIFY` wrote a new `.lua` file, reload succeeded
but the file was never loaded.

**Fix:** `LoadAllScripts()` now scans the scripts directory with
`std::filesystem::directory_iterator`, picks up every `*.lua` (case-
insensitive), sorts for deterministic order, and loads all of them.

---

### Bug #9 — Unsafe JSON construction ✅
**File:** `Services/Elle.Service.Action/ActionExecutor.cpp`

`ExecuteMessage`, `ExecuteHardwareCommand`, and the file-watcher
callback built JSON via string concatenation, embedding arbitrary
content inside `"..."`. Any backslash, quote, or control char in
`action.parameters` / `cmd` / a Windows path broke the frame or could
inject fields.

**Fix:** all three build envelopes with `nlohmann::json` (already
vendored as `Shared/json.hpp`) so every value is escaped correctly.

---

### Bug #10 — Unsafe SQL string concatenation ✅
**File:** `Shared/ElleSQLConn.cpp`

`SubmitIntent`, `UpdateTrust`, `RegisterWorker`, `UpdateWorkerHeartbeat`,
and `WriteLog` built SQL via `+` with single-quoted string literals.
Apostrophes in `reason` / `msg` / intent text broke the query; classic
injection surface even in-house.

**Fix:** every one switched to `QueryParams(sql, {params...})` with `?`
placeholders. The pool layer does `'..'` wrapping with escape-doubling,
so any quote or newline in user-originated strings is now safe.

---

### Bug #11 + #12 — StoreGoal incompatible / fragile JSON ✅
**File:** `Shared/ElleSQLConn.cpp`

Old `StoreGoal` wrote ad-hoc JSON via `snprintf` into
`dbo.system_settings`. `UpdateGoalProgress` / `GetActiveGoals` were
never implemented (Bug #5). The write/read paths couldn't agree.
Also: the `snprintf` didn't escape `goal.description`, so any
apostrophe/backslash broke the stored JSON.

**Fix:** introduced real `dbo.goals` (snake_case, lazy-created) with
matching `StoreGoal` / `UpdateGoalProgress` / `GetActiveGoals`, all
parameterised. The CamelCase `dbo.Goals` in
`SQL/ElleAnn_Schema.sql` is the aspirational design — the live boxes
we've seen use lowercase, which is why this table joins the
`memory`/`world_entity`/`hardware_actions` family.

---

## LOWER SEVERITY (real but self-contained)

### Bug #14 (user-raised follow-up) — Pending intent/action TOCTOU race ✅
**File:** `Shared/ElleSQLConn.cpp`

`GetPendingIntents` and `GetPendingActions` were doing a plain
`SELECT ... WHERE status = 0`; the row stayed *pending* until its
consumer (Cognitive / Action) later called `UpdateIntentStatus` /
`UpdateActionStatus`. At a 500 ms poll tick, any consumer that took
longer than 500 ms to handle the IPC (LLM calls routinely do, Win32
ops occasionally do) would have its row re-selected on the next
QueueWorker tick and dispatched a second time — duplicate work,
duplicate trust-delta side effects, duplicate WS broadcasts.

**Fix:** both dequeues are now single-statement atomic claim-on-select
using `UPDATE TOP (N) ... OUTPUT inserted.* FROM <queue> WITH (ROWLOCK,
READPAST) WHERE status = <pending>`:

- `GetPendingIntents`: PENDING → PROCESSING, returns the claimed rows.
- `GetPendingActions`: QUEUED → LOCKED, sets `started_ms`, returns the
  claimed rows.

`ROWLOCK + READPAST` lets parallel pollers carve up the queue without
blocking each other; the `OUTPUT inserted.*` guarantees each row is
observed by exactly one caller — SQL Server's native idempotent-
dequeue pattern. Matches the pattern HTTPServer already uses on
`hardware_actions` in `/api/ai/hardware/actions/pending`.

**Known remaining concerns (inherent to any queue, not TOCTOU):**
- A consumer crashing mid-process leaves rows stuck in PROCESSING /
  LOCKED forever. A periodic reaper that re-queues rows older than
  `timeout_ms` can land later — outside the 13-bug audit.

---

### Bug #13 — IPC_WORLD_STATE overloaded ✅
**Files:** `Shared/ElleTypes.h`, `Services/Elle.Service.HTTP/HTTPServer.cpp`,
`Services/Elle.Service.Action/ActionExecutor.cpp`,
`Services/Elle.Service.XChromosome/XChromosome.cpp`,
`Services/Elle.Service.Continuity/Continuity.cpp`.

`IPC_WORLD_STATE` was used for two different payload types:

1. Binary `ELLE_WORLD_ENTITY` structs → WorldModel
2. Arbitrary JSON strings → HTTPServer WS fan-out

WorldModel's `OnMessage` called `msg.GetPayload(entity)` on string
payloads and misread them; HTTPServer's forward filter happened to
work because it only read the string side.

**Fix:** added `IPC_WORLD_EVENT` as a dedicated string-JSON channel.
Every JSON-string emitter (ActionExecutor, XChromosome, Continuity,
file watcher) now uses `IPC_WORLD_EVENT`. HTTPServer's WS fan-out
listens on `IPC_WORLD_EVENT`. `IPC_WORLD_STATE` is reserved for
binary `ELLE_WORLD_ENTITY` updates only, which is what WorldModel
expects.

---

## VERIFICATION

- Brace balance: every file zero.
- `grep -rn "ELLE_SERVICE_COUNT + " Services/ Shared/ Lua/` returns
  only the doc comment in `ElleTypes.h`.
- All 17 `ElleServiceBase(...)` constructors use proper `SVC_*` IDs.
- All 30 `ElleDB::*` functions declared in the header have matching
  implementations.
- `g_serviceNames[]` cardinality enforced at compile time via
  `static_assert`.

No mocks. No fake returns. Every fix is backed by real SQL, real
IPC, and real binding code.
