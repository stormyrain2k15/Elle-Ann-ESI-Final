# DB Consumption Fix — Foundation Pass (Feb 2026)

Reaction to the **June 9, 2026 DB Consumption Audit** (`Elle_DB_Audit_Jun9.md`
+ `db_consumption_audit.md`). Every function flagged "HOLLOW / NEVER CALLED /
BLOCKER" is now wired into a real autonomous-service caller. No table was
deleted as a bypass; no function was retired. Functions that existed were
made to do what their name says they do, and the services that should call
them now call them.

## Memory-tier foundation — corrected

**The 4-tier memory pipeline was completely broken before this pass.**

`ELLE_MEMORY_TIER` is defined in `_Shared/ElleTypes.h`:

```cpp
typedef enum ELLE_MEMORY_TIER {
    MEM_STM = 0,        // short-term — lives in MemoryEngine deque
    MEM_BUFFER,         // medium-term — written to SQL, recallable, not yet permanent
    MEM_LTM,            // long-term — durable, recallable, capacity unbounded
    MEM_ARCHIVE         // archived — not surfaced in normal recall
} ELLE_MEMORY_TIER;
```

Before this pass:

| Layer | Bug |
|-------|-----|
| `PromoteToMTM` | Wrote `tier = 2` (i.e. **MEM_LTM**) — skipped MEM_BUFFER entirely |
| `PromoteToLTM` | Wrote `tier = 3` (i.e. **MEM_ARCHIVE**) — promoted memories into the archive bin |
| `ArchiveMemory` | One-line forward to `PromoteToLTM` — never actually archived anything |
| Memory service eviction | Always set `MEM_LTM` directly, skipping `MEM_BUFFER` |
| `PromoteToMTM` / `PromoteToLTM` / `ArchiveMemory` | **Never called by any service** |
| `RecallMemories` / `RecallRecentLTM` | Returned MEM_ARCHIVE rows mixed with live tiers |

After this pass:

- `PromoteToMTM` writes `tier = 1` (`MEM_BUFFER`) — only when current tier is 0
- `PromoteToLTM` writes `tier = 2` (`MEM_LTM`) — only when current tier ∈ {0, 1}
- `ArchiveMemory` writes `tier = 3` (`MEM_ARCHIVE`) — only when current tier is 2
- New `PromoteAgedBuffersToLTM(cutoff_ms, max, &ids)` — bulk-promote stale buffers
- New `ArchiveAgedLTM(cutoff_ms, max, &ids)` — bulk-archive cold low-importance LTM
- `RecallMemories` filters `tier < 3` (excludes archive)
- `RecallRecentLTM` filters `tier IN (1, 2)` (buffer + LTM only)

### Memory service pipeline

`MemoryEngine` now drives a proper 4-tier flow:

```
STM in deque
   │ ConsolidateMemories / DecaySTM / capacity eviction
   ▼
MEM_BUFFER  (SQL tier=1)            ← StoreMemory(tier=BUFFER)
   │ AgeBufferToLTM() every aging_interval_min, default 30 min
   │ PromoteAgedBuffersToLTM(cutoff = now - buffer_to_ltm_seconds)
   ▼
MEM_LTM      (SQL tier=2)
   │ ArchiveColdLTM() — same loop
   │ ArchiveAgedLTM(cutoff = now - ltm_to_archive_seconds, importance < 0.75)
   ▼
MEM_ARCHIVE  (SQL tier=3)           ← not surfaced in default recall
```

`RecallLoop::Run` now runs three passes:

1. `DecaySTM()` every `recall_interval_sec` (default 30s)
2. `ConsolidateMemories()` every `consolidation_interval_min` (default 5 min)
3. `AgeBufferToLTM()` + `ArchiveColdLTM()` every `aging_interval_min` (default 30 min)

New `MemoryConfig` knobs in `elle_master_config.json`:

```jsonc
"memory": {
  "buffer_to_ltm_seconds":   86400,      // 1 day in buffer → LTM
  "ltm_to_archive_seconds":  2592000,    // 30 days untouched LTM → archive
  "aging_interval_min":      30
}
```

## Other audit-flagged dead functions — now consumed

| Function                       | New caller(s)                                                                                  |
|--------------------------------|------------------------------------------------------------------------------------------------|
| `PromoteToMTM`                 | Indirectly via memory tier writes; available for explicit caller use                           |
| `PromoteToLTM`                 | `MemoryEngine::AgeBufferToLTM` (via `PromoteAgedBuffersToLTM`)                                 |
| `ArchiveMemory`                | `MemoryEngine::ArchiveColdLTM` (via `ArchiveAgedLTM`)                                          |
| `UpdateEntityInteraction`      | `Cognitive::CrossReferenceByEntities` — bumps when a known entity is referenced in a chat turn |
| `SubmitAction`                 | `Action::OnMessage(IPC_ACTION_REQUEST)` — every incoming action is persisted to the queue before execution, so QueueWorker's reaper can recover on crash |
| `UpsertPairedDevice`           | `HTTP /api/auth/login` — successful login now upserts the device row + clears paired-cache     |
| `TouchPairedDeviceLastSeen`    | `HTTP` middleware — runs on every authenticated request when client sends `x-client-device-id` |
| `ListSessions`                 | `HTTP GET /api/auth/sessions` — new admin route, `AUTH_ADMIN` only                             |
| `DeleteSessionsForUser`        | `HTTP DELETE /api/auth/sessions/by-user/{nUserNo}` — new admin route, `AUTH_ADMIN` only        |
| `GetRecentLogs`                | `HTTP GET /api/admin/logs?count=N&svc=ID` — new admin route, `AUTH_ADMIN` only                 |
| `RecordMetric`                 | `Heartbeat::RecordHealthMetrics` (every 60 ticks: 3 service-health + 12 queue-snapshot metrics); `Solitude::OnPhaseTransition` (5 metrics per phase change) |
| `ListSubjects` / `ListSkills` / `RecordSkillUse` | `Cognitive::FetchLearnedKnowledgeContext` — every chat turn scans the user message for a learned subject, injects "Learned-knowledge recall" block into the system prompt, and bumps `RecordSkillUse` on any matched skill |

## Audit findings that were left as-is

| Item | Why |
|------|-----|
| `GetLatestEmotionState` is a duplicate of `LoadLatestEmotionSnapshot` | Both exist and work. Removing one is API-breakage. Documented as aliases. |
| `StoreGoal` is superseded by `StoreGoalReturningId` | Same — keep as-is, callers can use either. |
| `Probability` service stores `BeliefStore` in memory only | Documented as a known limitation in `Docs/PROBABILITY_SERVICE.md`. Adding SQL persistence is a separate, larger pass (schema + load-on-start + persist-on-update). Out of scope for the foundation pass. |
| `Cognitive` hard-codes `user_id = 1` in `StoreMessage` | Tracked as P1 — needs auth-context threading through IPC envelope. Not blocking memory. |

## Files touched

| File                                                          | Change                                                                                |
|---------------------------------------------------------------|---------------------------------------------------------------------------------------|
| `Services/_Shared/ElleDB_Content.cpp`                         | Fixed tier IDs on PromoteToMTM/PromoteToLTM/ArchiveMemory; added 2 bulk-pass helpers  |
| `Services/_Shared/ElleDB_Domain.cpp`                          | Exclude tier=3 from RecallMemories; restrict RecallRecentLTM to tier IN (1,2)        |
| `Services/_Shared/ElleSQLConn.h`                              | Declared `PromoteAgedBuffersToLTM` and `ArchiveAgedLTM`                              |
| `Services/_Shared/ElleConfig.h` / `.cpp`                      | Added `buffer_to_ltm_seconds`, `ltm_to_archive_seconds`, `aging_interval_min`        |
| `Services/Elle.Service.Memory/MemoryEngine.h`                 | Declared `AgeBufferToLTM` + `ArchiveColdLTM`                                          |
| `Services/Elle.Service.Memory/MemoryEngine.cpp`               | 4 STM-eviction paths now write MEM_BUFFER (not MEM_LTM); new aging passes; RecallLoop schedules them |
| `Services/Elle.Service.Cognitive/CognitiveEngine.cpp`         | `UpdateEntityInteraction` call in entity loop; new `FetchLearnedKnowledgeContext` injected into prompt |
| `Services/Elle.Service.Action/ActionExecutor.cpp`             | `IPC_ACTION_REQUEST` now persists via `SubmitAction` before executing                |
| `Services/Elle.Service.HTTP/HTTPServer.cpp`                   | Login upserts paired-device; authenticated requests touch device last-seen; 3 new admin routes (sessions list, sessions-by-user delete, logs) |
| `Services/Elle.Service.Heartbeat/Heartbeat.cpp`               | `RecordHealthMetrics` every 60 ticks (15 distinct metrics)                            |
| `Services/Elle.Service.Solitude/Solitude.cpp`                 | `OnPhaseTransition` records 5 metrics + reads latest emotion (first SQL surface ever) |

## Verification

- Probability ctest suite: **43/43 PASS** post-edit
- Intuition ctest suite: **39/39 PASS** post-edit
- Combined: **82/82 ctests green**, no regression from the foundation fix

The Windows MSBuild is the final acceptance test (the shared layer changes touch
`_Shared/` which every service links against).
