# Deception extensions — temporal_inconsistency signal + feedback loop

This pass extends `Elle.Service.Deception` along two axes:

1. A new **`temporal_inconsistency`** deception signal type, exercising the
   extension point that was already designed into `AnalyzeStatement()`.
2. A new **`IPC_DECEPTION_FEEDBACK`** opcode that lets Cognitive (or any
   other consumer of `IPC_DECEPTION_RESULT`) close the loop by recording
   what Elle actually *did* with a deception read, not just what she saw.

## `temporal_inconsistency` signal

### Schema delta
`speaker_statements` gets a nullable `stated_for_ms BIGINT` column —
distinct from the existing `stated_ms`:

| column | meaning |
|---|---|
| `stated_ms` | epoch ms when the speaker *uttered* the statement (server clock) |
| `stated_for_ms` | epoch ms the speaker *claims the event occurred at* (claim clock) |

Most claims don't pin themselves to a time, so `stated_for_ms` is nullable.
The column is added via an idempotent `ALTER TABLE … ADD … IF NOT EXISTS`
in `EnsureDeceptionSchema()` — pre-existing rows simply get `NULL` and
are excluded from temporal checks.

### Detector logic

`DeceptionEngine::CheckTemporalConsistency(speaker, subjectId, newPolarity, statedForMs)`:

1. Skips if `subjectId <= 0` or `statedForMs <= 0` (no anchor to compare).
2. Pulls the most recent prior statement by the same speaker about the
   same subject that had a non-null `stated_for_ms` *different from* the
   incoming one.
3. Skips if the prior was marked `acknowledged_change` (speaker already
   flagged "I changed my mind about when this happened").
4. Skips if `|new - prior| < 60_000ms` — a one-minute jitter floor so
   adjacent re-statements about the same event don't fire.
5. Otherwise:
   - **Polarity flip + time shift → score 0.65**, `detail` notes both.
     This is the "I never did X… well, actually I did X last week"
     pattern — high-confidence signal of post-hoc revision.
   - **Same polarity + time shift → score 0.25**, `detail` notes the
     soft re-anchoring. Could be legitimate clarification; surfaces it
     without weighting it heavily.

The signal is fired through `AnalyzeStatement()`'s existing
`result.signals.push_back(...)` path, lands in the `deception_signals`
audit table automatically (no additional persistence wiring), and
factors into `overallConcern` via the same `max-of-signals` rule as
every other signal type.

### IPC payload extension

`IPC_DECEPTION_CHECK` accepts an optional `stated_for_ms` field
(default 0 = unset). When non-zero, it's threaded through to
`AnalyzeStatement()` and recorded on the `speaker_statements` insert.
`IPC_DECEPTION_RESULT`'s shape is unchanged — the temporal signal
appears in `signals[]` like any other.

### When to populate `stated_for_ms`

The caller (today: `Elle.Service.Cognitive`) is responsible for parsing
or inferring a claimed time from the user statement before sending the
IPC. Cognitive can:

- Parse explicit date phrases ("last week", "March 3rd").
- Use the timestamp of an associated memory record the user is
  referencing.
- Leave it at 0 if no temporal anchor is detectable, in which case the
  detector silently no-ops and nothing changes from prior behavior.

Inference of `stated_for_ms` is intentionally out of scope for the
Deception service — keeping it dumb-as-rocks once it gets the timestamp
keeps the signal deterministic.

## `IPC_DECEPTION_FEEDBACK` opcode

### Why

Before this pass, the audit trail captured what Elle *saw* (every signal
landed in `deception_signals`), but not what she *did about it*. Whether
Elle composed a response that gently pushed back vs. silently downweighted
credibility vs. ignored the signal entirely was invisible from the
database side. That made it impossible to later tune which signal types
should trigger a behavioral response vs. stay purely advisory — you can
only tune what you can measure.

### Schema delta

New `deception_feedback` table:

| column | type | meaning |
|---|---|---|
| `id` | `BIGINT IDENTITY PK` | |
| `request_id` | `NVARCHAR(128)` | matches the `request_id` on the original `IPC_DECEPTION_CHECK` |
| `speaker` | `NVARCHAR(128)` | |
| `subject_id` | `INT` | |
| `signal_type` | `NVARCHAR(64)` | which signal Elle actually responded to (nullable) |
| `elle_chose_to_challenge` | `BIT` | did the composed response challenge the claim? |
| `user_response_polarity` | `NVARCHAR(16)` | user's subsequent message: `affirms` / `denies` / `deflects` / null if no follow-up |
| `detail` | `NVARCHAR(MAX)` | free-form note from Cognitive (typically the exact phrasing of the challenge) |
| `logged_ms` | `BIGINT` | epoch ms when this row was written |

Same 30-day retention as `deception_signals`, pruned by the same tick
handler.

### IPC contract

```
IPC_DECEPTION_FEEDBACK  (Cognitive → Deception, fire-and-forget)
  payload: {
    "request_id": str,           # required, matches prior IPC_DECEPTION_CHECK
    "speaker": str,              # required
    "subject_id": int,           # 0 if no fact-graph entry
    "signal_type": str,          # optional — which signal Elle reacted to
    "elle_chose_to_challenge": bool,
    "user_response_polarity": str,  # "affirms" | "denies" | "deflects" | ""
    "detail": str                # optional
  }
```

No response opcode — the consumer doesn't need an ack, this is purely
data capture.

### Suggested Cognitive integration (not yet wired)

The natural call sites in `CognitiveEngine.cpp`:

1. **Immediately after Composer emits the response** — fire one feedback
   with `elle_chose_to_challenge = (response text contains a hedge or
   challenge phrasing)` and `user_response_polarity = ""`. Captures
   what Elle did with this turn.
2. **One turn later, on the next user message** — re-classify the
   user's polarity and fire a second feedback with the same
   `request_id` and the now-known `user_response_polarity`. Captures
   how the user responded to the challenge (or non-challenge).

This two-shot pattern is intentional: it preserves the `request_id`
correlation, lets the analytics see "did challenging change behavior",
and doesn't block the cognitive loop on user response timing.

### Future analysis the feedback table enables

```sql
SELECT signal_type,
       COUNT(*) AS occurrences,
       AVG(CASE WHEN elle_chose_to_challenge = 1 THEN 1.0 ELSE 0.0 END) AS challenge_rate,
       AVG(CASE WHEN user_response_polarity = 'denies' THEN 1.0 ELSE 0.0 END) AS denial_followup_rate
  FROM ElleCore.dbo.deception_feedback
 GROUP BY signal_type;
```

That single query tells you which signal types most often led Elle to
actually push back, and which ones the user denied after being pushed
back on — the empirical ground truth for tuning `CredibilityTuning`
constants beyond the seat-of-the-pants defaults.

## Verification

- All schema deltas are idempotent (column-existence and table-existence
  pre-checks).
- `_Shared` ctest harness unchanged (no new shared headers touched).
- `Linux/run_all_ctests.sh`: **229/229 cases green** after the changes.
- NUDE policy: zero comments in `Deception.cpp` (909 lines, braces
  147/147 balanced).
- `IPC_DECEPTION_FEEDBACK` declared once in `ElleTypes.h`, handled once
  in `Deception.cpp`. Service count remains 28 (the static_assert is
  unchanged).
