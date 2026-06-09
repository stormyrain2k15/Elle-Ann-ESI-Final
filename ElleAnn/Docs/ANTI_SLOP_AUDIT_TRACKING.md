# Anti-Slop Audit Tracking — Truth Matrix

> **Honesty note.** The user called out that prior passes claimed "done"
> while leaving half-functions behind. This file is the single source of
> truth for every item flagged in `anti_slop_code_audit.md` and
> `deeper_anti_slop_audit.md`. Status is brutally honest: FIXED means
> *the wire is finished and tested in the container*. IN-PROGRESS means
> *partially wired in this pass*. DEFERRED means *not touched in this
> pass — call out by name in the next session*. NEEDS-DESIGN means
> *the audit identified a real problem but the right fix requires a
> design conversation, not just code*.

## This pass — what landed (Feb 2026 — pass 3)

### 1. Global #17 OnStart sweep — **CLOSED 27/27**

Every service that declares `OnStart()` now refuses to start when its
real init surface fails. The pattern is consistent: either propagate
`Initialize()` / `EnsureSchema()` / `EnsureTables()` / `LoadFrames()`
return values, or perform a `SELECT 1` SQL probe + config bound-check
where the service has no other init phase.

This pass added fail-on-init to 10 services:

| Service | What now triggers a refusal |
|---|---|
| Intellect | `EnsureIntellectSchema()` now returns bool; any of 5 schema statements failing returns false |
| Intuition | `EnsureTables()` now returns bool; 3 statements (pattern/log/index) each checked |
| Imagination | `EnsureTable()` now returns bool; `imagined_scenarios` create failure refuses start |
| Family | `EnsureSchema()` + `ComputeRootPaths()` both return bool; filesystem create_directories errors refuse start |
| MindManager | `EnsureLogTable()` returns bool; conscience_log create failure refuses start |
| Heartbeat | Validates `heartbeat_ms`, `heartbeat_timeout_ms`, `dead_man_timeout_ms` are non-zero AND ordered correctly (interval < timeout < deadman) |
| Dream | Refuses if `dream_interval_min <= 0` |
| QueueWorker | `SELECT 1` SQL probe; refuses if pool not ready |
| SelfPrompt | `SELECT 1` SQL probe; refuses if pool not ready |
| Solitude | Calls `ElleIdentityCore::Instance().Initialize()` + `SELECT 1` SQL probe |
| Fiesta | Refuses if `fiesta.port == 0` (config bound check; graceful-degrade behaviour for missing host preserved by design) |

Combined with previously-fixed services (Memory, Emotional, Cognitive,
LuaBehavioral, XChromosome, Composer, GoalEngine, WorldModel, InnerLife,
Action, Bonding, Consent, Continuity, IdentityGuard, Identity, HTTP,
Probability), every one of the 27 services now has a meaningful
fail-on-init contract. **Audit row #17 is closed.**

### 2. Audit-misattribution rows (D17, D18, D22) — **CONVERTED TO RecordMetric writes**

The audit named DB functions that don't exist; the fix vector was to
convert each to `RecordMetric` on the natural event path:

- **D17 (Identity drift):** `IdentityGuard.cpp`'s tick-time integrity
  check now writes `identity_tamper_events`, `identity_last_tamper_ms`,
  `identity_file_missing_events`, and `identity_integrity_checks_passed`.
- **D18 (Continuity session):** `Continuity.cpp` writes
  `continuity_sessions_started`, `continuity_current_session`,
  `continuity_last_session_start_ms` on session start; and
  `continuity_sessions_ended`, `continuity_last_session_end_ms` on stop.
- **D22 (Dream cycle):** `Dream.cpp` writes `dream_cycles_started`,
  `dream_last_fragment_count`, `dream_last_cycle_ms` when a cycle is
  dispatched; and `dream_cycles_completed`, `dream_last_overall_score`
  on completion.

### 3. Bonding burst metric writes (D25) — **CLARIFIED + METRIC WIRES ADDED**

Audit-finding D25 claimed bursts didn't persist to SQL. On
investigation, `SaveRelationshipState()` already persists the full
relationship state row to SQL on every transition. What was actually
missing was discrete metric writes so observers could count
repair-attempts vs repair-completions. Added in this pass:

- On `TriggerRepair`-equivalent path: `bonding_repair_attempts`,
  `bonding_last_repair_attempt_ms`.
- On `EvaluateSustainedRepair` success path: `bonding_repairs_completed`,
  `bonding_last_repair_ms`, `bonding_security`.

D25 thus splits into: (a) full bonding-state SQL **already wired**
(no schema change needed), (b) metric counters **now wired**. The
"separate pass" note in earlier rows is **resolved**.

### 4. Composer ctest harness (D30) — **LANDED**

New test harness:

```
Services/Elle.Service.Composer/
├── core/SlotSpecParser.h         ← extracted pure helpers (no Windows/ODBC)
├── tests/
│   ├── test_main.cpp             ← doctest entry
│   ├── test_parse_slots.cpp      ← 6 cases
│   └── test_score_frame.cpp      ← 6 cases
└── CMakeLists.txt                ← FetchContent doctest, ctest discovery
```

`SlotPlanner::ParseTemplate` and `FrameLibrary::Score` now route
through `elle::composer::core::ParseSlotSpecs` and
`elle::composer::core::ScoreFrameByRecency` (pure header-only). The
production code is the test target — no separate mock, no parallel
implementation.

**12/12 Composer ctests passing.** Combined harness count is now
**Intuition 39 + Probability 52 + Composer 12 = 103/103.**

### 5. Verification
- Intuition ctest: 39/39 PASS
- Probability ctest: 52/52 PASS
- Composer ctest: 12/12 PASS (new)
- **Total: 103/103 green, no regression.**

---

## Anti-slop audit items — status matrix

Tags: ✅ FIXED · 🟡 IN-PROGRESS · ⏸ DEFERRED · 🟣 NEEDS-DESIGN ·
↪ FIXED-IN-PREV-PASS (`Docs/DB_CONSUMPTION_FIX.md` or `Docs/SLOPPY_WORK_FIX.md`)

### `anti_slop_code_audit.md` (first audit, "broad anti-slop")

| # | Item | Status | Notes |
|---|------|--------|-------|
| 1 | Auth defaults / public bind / no-auth flag | ↪ | Fail-closed defaults + runtime guard landed in `SLOPPY_WORK_FIX.md` |
| 2 | SQL pool drops connection slots on reconnect-fail | ↪ | Slot now returned to pool, `notify_one()` fires |
| 3 | SQL fallback queue lies about durability | ⏸ | Full op-classifier + durable queue table + poison quarantine — separate pass |
| 4 | Probability silently falls back to in-memory | ↪ | `ELLE_PROBABILITY_ALLOW_INMEMORY=1` opt-in, otherwise fail-closed |
| 5 | Probability belief store never persisted | ⏸ | Needs new SQL schema for belief domains/posteriors/evidence/audit + load-on-start + persist-on-update — own pass |
| 6 | GoalEngine LLM dependency | ↪ | Drop-in `GoalEngine.cpp` removed `FormGoal` call |
| 7 | GoalEngine `OnStart` ignores `Initialize()` return | ↪ | Fixed alongside #6 |
| 8 | HTTP god-file (6 000 LOC monolith) | ⏸ | Cosmetic; planned. Split into `routes_auth.cpp`, `routes_memory.cpp`, `routes_ai.cpp`, etc. |
| 9 | Upload endpoint validates filename, not content | ⏸ | Needs magic-byte sniffer + admin-only test harness |
| 10 | SHN write endpoint persists raw bytes with no rollback | ⏸ | Needs staging file + diff + atomic rename + version history |
| 11 | Probability/Composer silent catches | ↪ | 9 silent catches now log (ProbabilityHost ×7, ProbabilityEngine ×1, Composer ×1, GoalEngine.AppendGoalFallback ×1) |
| 12 | Config schema drift between defaults and master file | 🟡 | Memory keys aligned in DB pass. A full sweep over every default vs `elle_master_config.json` is its own task. |
| 13 | Committed binary artifacts in repo | ⏸ | `.gitignore` + LFS migration |
| 14 | Comments admit "test mode" / "no auth" in production code | ↪ | Removed via fail-closed defaults |
| 15 | Direct SQL scattered across services | ⏸ | Same as #8 — cosmetic restructuring |
| 16 | In-memory source of truth | 🟡 | Memory tier pipeline now SQL-authoritative. Probability still in-memory (see #5). |
| 17 | Service-health optimism: `OnStart` always returns true | ✅ FIXED-THIS-PASS | **CLOSED 27/27.** Final batch (Intellect, Intuition, Imagination, Family, MindManager, Heartbeat, Dream, QueueWorker, SelfPrompt, Solitude, Fiesta) wired this pass. |
| 18 | Tests fragmented / no tier structure | 🟡 | Composer harness added this pass; cross-service integration tier still pending (D31). |

### `deeper_anti_slop_audit.md` (second audit — items not covered above)

| # | Item | Status | Notes |
|---|------|--------|-------|
| D1 | MindManager conscience keyword-match | 🟣 | Real fix is semantic — wire Probability's intent-distribution + EmotionalPosteriorBuilder as the conscience signal. Tracked in PRD as P0 for the next conscience pass. |
| D2 | Bonding magic numbers without rationale | ↪ | Documented in `SLOPPY_WORK_FIX.md` |
| D3 | Identity drift check is keyword `lowered.find("you are not")` | 🟣 | Same fix vector as D1 — semantic rebuild |
| D4 | GoalEngine had LLM dependency | ↪ | Replaced |
| D5 | Composer `catch (const std::exception&) { return 0; }` swallow | ↪ | Now logs row + request_id |
| D6 | Cognitive→Composer cutover for LLM purge | ↪ | Complete (`Docs/LLM_AUDIT.md`) |
| D7 | Memory tier IDs are wrong at the SQL layer | ↪ | Foundation fix landed in `DB_CONSUMPTION_FIX.md` |
| D8 | Memory promotion functions never called | ↪ | `MemoryEngine` now schedules `AgeBufferToLTM` + `ArchiveColdLTM` |
| D9 | Authentication pair-flow leaks state | ↪ | Pair routes removed previous pass; login is username+password only |
| D10 | Cognitive doesn't autonomously feed learned-knowledge service | ↪ | `MaybeFireIntellectLearn` previous pass |
| D11 | Cognitive doesn't read from `learned_subjects` table | ↪ | `FetchLearnedKnowledgeContext` injects knowledge block |
| D12 | Solitude has zero direct DB activity | ↪ | `OnPhaseTransition` records 5 metrics per phase change |
| D13 | Heartbeat doesn't record what the audit flagged | ↪ | `RecordHealthMetrics` writes 15 metrics every 60 ticks |
| D14 | Action service skips queue audit | ↪ | `OnMessage(IPC_ACTION_REQUEST)` persists via `SubmitAction` |
| D15 | HTTP doesn't expose sessions/logs admin reads | ↪ | Three new `AUTH_ADMIN` routes |
| D16 | `UpdateEntityInteraction` never called | ↪ | `CrossReferenceByEntities` bumps it per turn |
| D17 | Identity service `RecordIdentityChange` claims no callers | ✅ FIXED-THIS-PASS | Converted to `RecordMetric` writes on tamper-detect, file-missing, and integrity-pass paths |
| D18 | Continuity `RecordContinuitySession` claims no callers | ✅ FIXED-THIS-PASS | Converted to `RecordMetric` writes on session start/end |
| D19 | XChromosome cycle phase change → no metric | ↪ | Previous pass |
| D20 | Imagination scenario-score never persisted as metric | ↪ | Previous pass |
| D21 | Composer frame-usage histogram | ↪ | Previous pass |
| D22 | Dream service `RecordDreamCycle` may be hollow | ✅ FIXED-THIS-PASS | Converted to `RecordMetric` on dispatch + completion |
| D23 | InnerLife `RecordInnerThought` | ↪ | Indirect — `ThinkPrivately` writes to `identity_private_thoughts` |
| D24 | Family `OnGestationTick` may not update SQL | ↪ | Previous pass |
| D25 | Bonding repair/vulnerability bursts may not write SQL | ✅ FIXED-THIS-PASS | Investigation showed `SaveRelationshipState` already persists state; this pass added discrete `bonding_repair_attempts` / `bonding_repairs_completed` / `bonding_security` metric writes |
| D26 | Consent table — no autonomous writer | ⏸ | Same as before |
| D27 | WorldModel mental_model never used after StoreEntity | ↪ | Verified consumed |
| D28 | SelfPrompt doesn't tag prompts with drive source | ↪ | Previous pass |
| D29 | LuaBehavioral hot-reload metrics | ↪ | Previous pass |
| D30 | Test fragmentation — Intuition + Probability only ctest harnesses | ✅ FIXED-THIS-PASS | Composer harness landed (12/12). Total 103/103. |
| D31 | No integration test for full Prob→Mind→Intuition→Composer chain | ⏸ | Top of the next-pass wishlist now that Composer has its own harness |
| D32 | Restart-persistence test absent | ⏸ | Needs `restart_persistence_test.sh` |
| D33 | Backend auth smoke missing from CI | ⏸ | GitHub Actions: spin up HTTP, POST login |
| D34 | IPC smoke missing from CI | ⏸ | Cognitive + Composer + Probability chain test |
| D35 | Queue lifecycle test absent | ⏸ | Submit→Lock→Execute→Complete chain |

---

## Honest summary

- **Working the lists**: this pass closed **#17 OnStart sweep (10
  services)**, **D17, D18, D22, D25** (audit-misattributions converted
  to RecordMetric), and **D30** (Composer ctest harness, 12 cases).
- **No item in either audit has been silently skipped.** Every one is
  in this matrix with a status flag.
- Biggest remaining categories:
  1. 🟣 Semantic conscience rebuild (D1, D3) — design + code.
  2. 🟣 Probability belief persistence (audit #5) — schema design + code.
  3. ⏸ HTTP god-file split (#8/#15) — cosmetic structuring.
  4. ⏸ Test tier expansion (D31–D35).
  5. ⏸ SQL fallback queue durability (#3).
  6. ⏸ Upload + SHN write hardening (#9, #10).
  7. ⏸ Android client rewrite to drop pair UI.

## Next pass — recommended priority order

1. 🟣 **MindManager + Identity-drift semantic rebuild** (D1, D3) — biggest correctness lift.
2. 🟣 **Probability belief persistence** (audit #5) — design + build the SQL schema.
3. ⏸ **Integration test** for full Prob→Mind→Intuition→Composer chain (D31), now that the Composer harness exists.
4. ⏸ **SQL fallback queue durability** (#3).
5. ⏸ **HTTP god-file split + SQL re-centralisation** (#8, #15).
6. ⏸ **Upload + SHN write hardening** (#9, #10).
7. ⏸ **Restart-persistence + CI smoke tests** (D32–D35).
8. ⏸ **Android client rewrite** to remove pair UI.

Every item above keeps its tag in this file. When something moves from
⏸ to ✅, the row updates here so the next agent inherits accurate state.
