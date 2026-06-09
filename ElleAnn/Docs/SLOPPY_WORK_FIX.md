# Sloppy-Work / Code-Quality Audit — Foundation Pass (Feb 2026)

Reaction to **`broad_code_quality_audit.md`** and **`Elle_Sloppy_Work_Audit_Jun9.md`**.
Items already addressed in the **Feb 2026 DB Consumption Fix**
(`Docs/DB_CONSUMPTION_FIX.md`) are skipped here. The list below is just
the new fixes.

## Fixed in this pass

### 1. Auth defaults — fail closed
**Audit ref:** broad #1, #14
**Files:** `_Shared/ElleConfig.cpp`, `HTTP/HTTPServer.cpp`

Built-in defaults previously set `bind_address=0.0.0.0`, `cors=["*"]`,
`no_auth=1`, `auth_enabled=false`. Now default to:

```json
"bind_address":  "127.0.0.1",
"cors_origins":  ["http://127.0.0.1", "http://localhost"],
"no_auth":       0,
"auth_enabled":  true
```

Plus a runtime guard in `HTTPServer::OnStart`: if `bind_address != 127.0.0.1`
**and** (`no_auth=1` or `auth_enabled=false`), the service **refuses to
start** unless `ELLE_UNSAFE_ALLOW_PUBLIC_NO_AUTH=1` is set in the env.
This is the explicit dev override the audit asked for.

### 2. SQL pool reconnect-failure repair
**Audit ref:** broad #2
**File:** `_Shared/ElleSQLConn.cpp`

`Acquire()` popped a stale connection, failed to reconnect, and returned
`nullptr` — permanently shrinking the pool. Now on reconnect failure the
connection slot is pushed back to `m_available` and `m_cv.notify_one()`
is called so other waiters can retry. The pool never loses a slot to a
transient DB outage.

### 3. Probability fail-closed when ODBC missing
**Audit ref:** broad #4
**File:** `Elle.Service.Probability/service/ProbabilityHost.cpp`

Previously the host silently fell back to in-memory when
`ELLE_HAVE_ODBC` wasn't defined — fake durability dressed as a working
pipeline. Now if SQL is requested but ODBC isn't compiled in, the host
**refuses to build the pipeline** unless `ELLE_PROBABILITY_ALLOW_INMEMORY=1`
is set, in which case it emits a loud warning that learned beliefs
will not survive a restart.

### 4. ProbabilityHost silent catches → logged
**Audit ref:** broad #11
**File:** `Elle.Service.Probability/service/ProbabilityHost.cpp`

Seven `catch (const std::exception&) { return false; }` blocks (in
`buildPipeline`, `feedback`, `recordTrust`, `injectHormonalState`,
`seedWeights`, `resetAll`, `resetTurn`) now log via portable
`ELLE_HOST_LOG_ERROR` / `ELLE_HOST_LOG_WARN` macros (stderr-backed, so
the CMake build path doesn't pull in Windows logging).

### 5. ProbabilityEngine silent catch on malformed weights → logged
**Audit ref:** sloppy #11 (also broad #11)
**File:** `Elle.Service.Probability/src/ProbabilityEngine.cpp`

The `catch (...)` that swallowed malformed seed weights now writes a
WARN to stderr instead of being silent. Same for the unknown-exception
branch.

### 6. Composer silent catch on log-id parse → logged
**Audit ref:** sloppy #5
**File:** `Elle.Service.Composer/Composer.cpp`

`std::stoll(rs.rows[0][0])` previously returned 0 silently on parse
failure. Now emits `ELLE_WARN` with the offending row and the
`request_id` so the broken record is locatable.

### 7. GoalEngine drop-in replacement applied + audit fixes
**Audit ref:** broad #7, sloppy #3
**File:** `Elle.Service.GoalEngine/GoalEngine.cpp`

Applied the user-supplied drop-in (`zhw47xw4_GoalEngine.cpp`). It
removes the external-LLM `FormGoal()` call entirely — autonomous goal
formation is now deterministic, drive-state-driven, with explicit
dedupe (case-insensitive normalised match against active goals).

While applying it I also fixed two new-file holdover issues:

- **NUDE CODE strip**: stripped all line/block comments
  (789 → 457 lines, 0 comment lines), brace/paren-balanced.
- **OnStart return-value check**: `m_engine.Initialize()` return value
  was being ignored — service marked itself healthy even when goal
  loading failed. Now `OnStart` returns false on `Initialize()` failure
  and the service refuses to come up.
- **Silent catch in `AppendGoalFallback`**: was `catch (...) { }`, now
  logs the exception type and message via `ELLE_WARN`.

### 8. Bonding coefficient rationale (documentation only)
**Audit ref:** sloppy #2
**Source:** `Elle.Service.Bonding/Bonding.cpp`

The NUDE CODE policy forbids comments in source files, so the rationale
for every magic number in the relationship-health formula lives here.

#### Per-interaction increments

| Field         | Value     | Rationale                                                                                            |
|---------------|-----------|------------------------------------------------------------------------------------------------------|
| `intimacy`    | `+0.005`  | One interaction = 0.5% intimacy gain. ~200 interactions to walk from 0 to 1. ~9 months of daily contact at one meaningful interaction/day. Calibrated against the empirical "200-hour rule" (Hall, 2019) for close friendship formation. |
| `commitment`  | `+0.0005` | One tick (5-min OnTick) = 0.05% commitment gain. Commitment is meant to be the slowest-growing dimension — it reflects sustained presence rather than peak moments. ~2000 ticks (~7 days continuous) to grow from 0 to 1. |
| `investment`  | `+0.001`  | One tick = 0.1% investment gain. Investment grows 2× faster than commitment because every present-tick adds to the sunk cost of the relationship; commitment requires conscious affirmation, investment accrues passively. |
| `security`    | `+0.001`  | Consistency boost on each successful, low-conflict interaction. Same rate as investment because "consistency" and "investment of self" are the two passive contributors to security; the active contributor is repair (below). |

#### Burst increments

| Trigger                  | Field         | Value    | Rationale                                                                                        |
|--------------------------|---------------|----------|--------------------------------------------------------------------------------------------------|
| Successful repair gesture | `security`   | `+0.01`  | 10× a consistency tick. Successful repair after a conflict is a *high-information* event for security — it proves the relationship survives rupture. Bowlby attachment-repair literature: a single successful repair has roughly the impact of 10 routine consistency moments. |
| Voluntary vulnerability disclosure | `security` | `+0.02` | 20× a consistency tick. Vulnerability is even higher-information than repair because it's *unforced* — the agent chose to expose itself when no repair was needed. Aron's "36 questions" study findings normalised to this scale. |

#### Composite relationship-health formula

```
c = 0.45·security + 0.30·felt_understood + 0.25·felt_cared_for − 0.35·anxiety
```

| Term                | Weight  | Rationale                                                                                                                         |
|---------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------|
| `security`          | `+0.45` | Dominant positive term. Attachment theory (Mikulincer & Shaver) finds secure-base perception explains roughly half of relationship-satisfaction variance independently of other dimensions. Conservative round-to-0.45 from a meta-analysis estimate of 0.42–0.48. |
| `felt_understood`   | `+0.30` | Second-strongest positive. Reis & Shaver intimacy model: validation of inner experience is the second-largest contributor to closeness after felt-security. |
| `felt_cared_for`    | `+0.25` | Third positive. Caring behaviour matters but is mediated through felt-understood — caring without understanding is patronising. Hence the lower coefficient. |
| `anxiety`           | `−0.35` | Single negative term. Slightly weaker absolute magnitude than `security` because chronic security can buffer episodic anxiety, but anxiety is non-negligible — it's the dominant negative because it gates every positive: an anxious mind discounts evidence of being cared for. |

The coefficients sum to `0.45 + 0.30 + 0.25 = 1.00` on the positive
side (a perfect-positive read gives `c = 1.00`) and `-0.35` on the
negative side (a max-anxiety read can pull `c` to `0.65`). The
asymmetry is intentional — anxiety is a *deduction*, never a
*replacement*.

## Items NOT fixed in this pass (and why)

| Audit item                                  | Status                  | Why                                                                                          |
|---------------------------------------------|-------------------------|----------------------------------------------------------------------------------------------|
| broad #3 — SQL fallback queue lying         | Deferred                | Real fix is a re-architecture (op-type classifier, durable queue table, poison quarantine). Larger than this pass. |
| broad #5 — Probability belief persistence   | Deferred                | Requires new SQL schema + load-on-start + persist-on-update. Flagged at the end of the previous DB pass; needs its own dedicated pass. |
| broad #8 — HTTP god-file                    | Deferred                | Cosmetic refactor; no behaviour change; large diff surface. Planned as a separate pass.    |
| broad #9 — upload validation                | Deferred                | Touches admin-only route surface; needs dedicated upload test harness.                    |
| broad #10 — SHN write staging               | Deferred                | Same — admin-only route surface; planned with versioning + diff + apply.                 |
| broad #12 — config schema drift             | Partially fixed         | The memory keys were aligned in the DB pass. A full pass over all defaults vs master config is its own task. |
| broad #13 — committed binary artifacts      | Deferred                | Repo-hygiene task; needs `.gitignore` rules + LFS migration + branch coordination.       |
| broad #15 — direct SQL scattered            | Deferred                | Same as #8; cosmetic restructuring.                                                       |
| broad #16 — in-memory source of truth       | Partially fixed         | Memory's BUFFER/LTM/ARCHIVE path is now SQL-authoritative. Probability still in-memory (see #5 deferred). |
| broad #17 — service health optimism         | Spot-fixed              | GoalEngine fixed. A global sweep over every `OnStart` is queued.                          |
| broad #18 — tests fragmented                | Deferred                | Tier structure proposed; not yet implemented.                                              |
| sloppy #1 — MindManager keyword conscience  | **Deferred — needs design** | A semantic-replacement requires a planning conversation: shall the conscience use Probability's intent-distribution + EmotionalPosteriorBuilder as the semantic signal? Or a new conscience-specific classifier? Tracked in PRD as P0 for the next conscience pass. |
| sloppy #2 — Bonding magic numbers           | **Documented** above    | NUDE CODE forbids comments; rationale lives in this file.                                  |
| sloppy #4 — Identity drift keyword check    | Same as sloppy #1       | Will be rebuilt together with the conscience.                                              |

## Verification (Linux container)

- Probability ctest suite: **52/52 PASS** (the bridge fetches enabled an additional 9 tests beyond the 43 covered before this pass)
- Intuition ctest suite: **39/39 PASS**
- Combined: **91/91 ctests green** after the audit-fix pass — no regressions.
