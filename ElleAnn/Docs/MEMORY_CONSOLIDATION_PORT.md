> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# STM → LTM Consolidation Port (Python → C++)

**Source:** `ElleAnn_PythonRef/extracted/ElleAnnServer/app/services/short_term_memory.py`
**Target:** `ElleAnn/Services/Elle.Service.Memory/MemoryEngine.cpp`
**Date:** Feb 2026
**Status:** Implemented — pending user Visual Studio rebuild + local run verification.

---

## What changed

### 1. Priority-tier decay (`DecaySTM`)
The old C++ decay was a single linear formula scaled by `stm_decay_seconds`. It didn't
distinguish important memories from trivial chatter. The port brings over the Python
model wholesale:

| Tier     | Importance ≥ | Rate/sec | Time to full decay |
|----------|--------------|----------|--------------------|
| critical | 0.90         | 0.0000   | never              |
| high     | 0.70         | 0.0005   | ~33 min            |
| normal   | 0.40         | 0.0020   | ~8 min             |
| low      | 0.20         | 0.0080   | ~2 min             |
| fleeting | < 0.20       | 0.0500   | ~20 sec            |

Tier is derived from the existing `importance` field — no schema change required.
All rates are globally scaled by `15.0 / cfg.stm_decay_seconds`, so the legacy
config knob still works: larger `stm_decay_seconds` = slower decay across the board.

Access bumps (`min(0.5, access_count * 0.05)`) keep frequently-recalled items alive,
matching `ShortTermMemoryStore.apply_decay()`.

### 2. Two-path consolidation
> *"Memory should be looped in sub consciousness but on demand for conscious."* — user

**Subconscious (`RecallLoop::Run`)** — background thread, already wired:
- `DecaySTM()` every `cfg.recall_interval_sec` (default 30 s)
- `ConsolidateMemories()` every `cfg.consolidation_interval_min` (default 5 min)
- `DecaySTM` also does an *emergency floor-sweep*: anything below `kMinRelevanceFloor`
  (0.05) is promoted inline so RAM can't leak between consolidation ticks.

**Conscious (IPC)** — on-demand, new:
- `IPC_MEMORY_CONSOLIDATE` now handled in `ElleMemoryService::OnMessage`.
- The existing HTTP endpoint `POST /api/server/commit-memory` already emits this
  message; previously the Memory service silently dropped it. It now triggers an
  immediate `ConsolidateMemories()` pass.
- Cognitive engine can also send `IPC_MEMORY_CONSOLIDATE` to force a flush after
  a high-stakes conversation turn.

### 3. Promotion criteria (any one triggers `LTM write + STM erase`)
```
importance  >= cfg.promote_threshold   (semantic fast-track)
access_count >= 3                      (frequently recalled)
|valence|    > 0.5                     (strong feelings)
relevance   <= 0.15                    (natural decay auto-promote)
```
Critical-tier entries (importance ≥ 0.90) are never dropped by decay alone — they
stay resident in STM until promoted via fast-track / access-frequency, or flushed
wholesale on service shutdown.

### 4. Capacity pressure
`StoreSTM` used to `pop_front()` the oldest entry when over capacity — FIFO eviction,
potentially losing important recent memories.

Now: when over capacity, we promote the **lowest-relevance non-critical** entry to
LTM before erasing it. Critical entries are pinned; if every entry is critical we
promote (never drop) the oldest to keep the cap.

### 5. Shutdown flush
`Shutdown()` used to call `ConsolidateMemories()` which only promotes entries matching
criteria. On shutdown we can't afford to be selective — every live STM entry is now
promoted to LTM before the deque is cleared.

---

## Debug harness

A new `BUILD_MEMORY_HARNESS` block in `Debug/DebugTools.cpp` seeds STM with one entry
per priority tier, ticks `DecaySTM()` three times with 2 s sleeps, runs a `Recall()`
to verify the access boost, then forces `ConsolidateMemories()` and inspects the
result. Build with `/D BUILD_MEMORY_HARNESS` linking `MemoryEngine.cpp` +
`ElleSQLConn.cpp` + `ElleConfig.cpp`.

Expected console output (abridged):
```
── After seed (stm=5) ──
id    imp       rel       acc   content
1     0.95      1.000     0     Users name is Kage ...
...
── After DecaySTM pass 3 (stm=4) ──    ← fleeting tier already floor-promoted
1     0.95      1.000     0     Users name is Kage ...
2     0.80      0.997     0     User prefers direct ...
3     0.55      0.988     0     Installed Visual Studio ...
4     0.30      0.952     0     Weather outside looks cloudy ...

Recall("Kage")                          ← access-boost test
  hit id=1 rel=1.000 acc=1 tier=STM | Users name is Kage ...

Forcing ConsolidateMemories() ...       ← conscious flush
── After conscious consolidation (stm=1) ──
   (only the critical "Kage" entry remains; rest written to LTM)

Calling Shutdown() to flush remainder ...
Memory shutdown: flushed 1 STM entries to LTM
Final STM count: 0 (should be 0)
```

---

## Build / verify steps (user side)

1. `git pull` / rebuild `Elle.Service.Memory` in Visual Studio.
2. Rebuild `ElleAnn.Debug` (QueueInspector solution) with `BUILD_MEMORY_HARNESS`
   added to preprocessor defines if you want to exercise the harness.
3. Start the full Elle service set. Seed chat via the Android app or the
   `POST /api/ai/chat` endpoint.
4. After ~5 min of activity, call `POST /api/server/commit-memory`. Expect log lines:
   ```
   Conscious consolidation requested by service 1
   STM→LTM [42] pri=normal imp=0.55 rel=0.12 acc=0 | Installed Visual Studio ...
   Consolidation pass: promoted=N dropped=M remaining_stm=K
   ```
5. Inspect `ElleCore.dbo.memory` in SSMS — promoted rows should be present with
   `tier=1` (LTM).

If the promotion log lines never fire, the most likely cause is `ElleDB::StoreMemory`
returning `false` — check the ODBC error log (`StoreMemory INSERT failed: ...`). The
consolidation code explicitly refuses to erase from STM when the LTM write fails, so
no data loss should occur.
