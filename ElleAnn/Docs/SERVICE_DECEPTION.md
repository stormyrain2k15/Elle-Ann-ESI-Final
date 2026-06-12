# Elle.Service.Deception — Veracity & Deception Engine

Two related jobs in one Windows service:

1. **Veracity classification** (fully built) — given a subject Elle has
   learned about, classify it as one of
   `VERIFIED_FACT | THEORY | CONTESTED | UNSUPPORTED | CONTRADICTED`
   based on the subject's own confidence and the polarity/strength of
   its connections to other subjects in Intellect's knowledge graph.
2. **Deception detection** (baseline built, extension point marked) —
   given a statement made by a speaker about a subject, check:
   - does it contradict a `VERIFIED_FACT`? (factual contradiction)
   - does it contradict something the **same speaker** said before about
     the same subject, with no acknowledgment of the change?
     (self-contradiction over time)
   - optionally, an externally-supplied deception-intent probability
     from `Probability` (semantic signal, not lexical)

The mechanics for additional deception signals plug into
`AnalyzeStatement()` and the `deception_signals` table. That table and
the signal-combination function are the explicitly-marked extension
point. Everything else in the file is real, working logic — no stubs.

## Schema deltas (idempotent on service start via `EnsureDeceptionSchema()`)

### `intellect_connections.polarity NVARCHAR(16) DEFAULT 'neutral'`
Without polarity, a "connection" between two facts can't tell us whether
they agree or disagree — which is the whole point of cross-referencing.
Existing rows default to `neutral` rather than presuming agreement.
Values: `supports | contradicts | neutral`.

### `speaker_statements`
Tracks what a given speaker has asserted about a given subject over
time, and with what polarity. Self-contradiction detection runs against
this table. If a speaker says "X happened" and three weeks later says
"X never happened" without flagging the change, that's a signal — not a
verdict.

| column | type | notes |
|---|---|---|
| `id` | `BIGINT IDENTITY PK` | |
| `speaker` | `NVARCHAR(128)` | |
| `subject_id` | `INT` | |
| `polarity` | `NVARCHAR(16)` | `affirms` / `denies` |
| `statement_text` | `NVARCHAR(MAX)` | |
| `acknowledged_change` | `BIT DEFAULT 0` | 1 = speaker flagged they changed their mind, suppresses self-contradiction signal |
| `stated_ms` | `BIGINT` | epoch ms |

### `veracity_log`
Audit trail of every classification with the reasoning string. Used for
Elle's own self-consistency and for debugging the classifier. Pruned to
30 days on every 60th tick (same retention policy as Intellect's log).

### `deception_signals` — **EXTENSION POINT**
One row per fired signal of any type. `signal_type` is open-ended
(`NVARCHAR(64)`, not an enum) so new signal types defined later land
here without another schema migration.

Baseline signal types written today:
- `factual_contradiction` — statement conflicts with `VERIFIED_FACT`
- `self_contradiction` — speaker conflicts with own prior statement
- `probability_engine` — externally-supplied `deceptionIntentProbability`
- `profile_inconsistency` — claim conflicts with an established trait

Future signal types (tone shift, narrative inconsistency, motive
analysis, whatever it turns out to be) get a new `signal_type` value
and slot into `AnalyzeStatement()` without schema changes.

### `profile_traits`
Behavioral/personality facts about specific people, learned over time
and reinforced with repetition. This is the **second axis of judgment**:
not fact-vs-fiction, but claim-vs-known-person.

`trait_type` values:
- `aversion` — strong dislike or avoidance
- `incapability` — known inability
- `preference` — likes / tends toward
- `behavioral_pattern` — repeated observed behavior
- `capability` — known ability
- `value` — held belief / principle

`keywords` are space-separated tokens used to match statements against
the trait (deterministic substring overlap, no embeddings).

`confidence` grows toward 1.0 with `reinforcement_count` via simple
lerp toward a high target (`0.95`, step `0.15`). A trait mentioned
once carries less weight than one confirmed across many conversations.

## Tuning knobs (`namespace CredibilityTuning`)

| constant | value | meaning |
|---|---|---|
| `VERIFIED_FACT` | `0.95` | baseline credibility for verified claims |
| `THEORY` | `0.50` | |
| `CONTESTED` | `0.40` | |
| `UNSUPPORTED` | `0.10` | the **10% rule** floor — believe nothing of what you hear |
| `CONTRADICTED` | `0.05` | |
| `PROFILE_CONSISTENCY_BOOST` | `+0.40` | max raise from a single matched-trait |
| `PROFILE_INCONSISTENCY_PENALTY` | `−0.35` | max drop from a single conflicting trait |
| `MATCH_OVERLAP_THRESHOLD` | `0.34` | fraction of trait keywords that must appear in the statement for a match |
| `MIN_TRAIT_CONFIDENCE` | `0.60` | a trait below this can't move credibility on its own |

These are explicit tuning knobs and need live calibration. The ordering
matters more than the exact values: `VERIFIED_FACT` near the top,
`CONTRADICTED` near the bottom, `UNSUPPORTED` deliberately low (the
"10% rule") rather than at a neutral midpoint.

## Classifier decision logic (`VeracityEngine::Classify`)

Connections to other subjects are weighted by the **connected subject's
own confidence** — a `contradicts` link from something Elle is only
30% sure of doesn't outweigh something she's 90% sure of.

- `CONTRADICTED` — contradicting evidence outweighs supporting by at
  least `CONTRADICTION_MARGIN = 0.3`, and at least one high-confidence
  contradicting fact exists.
- `CONTESTED` — supporting and contradicting strengths are within
  `CONTRADICTION_MARGIN` of each other and both non-zero.
- `VERIFIED_FACT` — subject confidence ≥ `0.75` and no meaningful
  contradiction.
- `THEORY` — confidence in `[0.40, 0.75)` or some support exists, no
  meaningful contradiction.
- `UNSUPPORTED` — low confidence, no meaningful connections either way.

`neutral` connections never factor into the classification — they're
related-but-non-committal links.

## IPC contract

| opcode | direction | payload |
|---|---|---|
| `IPC_VERACITY_CLASSIFY` | in | `{subject_id, subject_name, confidence, request_id}` |
| `IPC_VERACITY_RESULT` | out | `{request_id, subject_id, classification, confidence, reasoning, supporting_count, contradicting_count}` |
| `IPC_DECEPTION_CHECK` | in | `{speaker, about_person, subject_id, subject_name, subject_confidence, statement_text, polarity, external_deception_prob, request_id}` |
| `IPC_DECEPTION_RESULT` | out | `{request_id, overall_concern, classification, baseline_credibility, adjusted_credibility, signals[], profile_matches[]}` |
| `IPC_PROFILE_LEARN` | in | `{person, trait_description, trait_type, keywords, confidence}` — fire-and-forget |
| `IPC_PROFILE_QUERY` | in | `{person, statement_text, polarity, request_id}` |
| `IPC_PROFILE_RESULT` | out | `{request_id, person, credibility_adjustment, matches[]}` |

`about_person` defaults to `speaker` when not provided.
`external_deception_prob = -1` means "not supplied" (this service does
not compute it itself, only incorporates it when given).
`polarity` is one of `affirms | denies`.

## Dependencies & tick

- `GetDependencies() → { SVC_HEARTBEAT, SVC_INTELLECT }` — Deception
  walks `intellect_connections` and `learned_subjects`, so Intellect
  must be alive first.
- `SetTickInterval(60000)` (60s). Every 60th tick (~1 hour) the
  retention pruner runs against `veracity_log` and `deception_signals`,
  dropping rows older than 30 days.

## Adding a new deception-signal type

1. Write a function returning a `DeceptionSignal` with a new `type`
   string.
2. Call it in `AnalyzeStatement` and push the result into
   `result.signals`.
3. Persistence to `deception_signals` is automatic — the loop at the
   end of `AnalyzeStatement` writes every signal regardless of type.
4. Decide how it factors into `overallConcern`. Currently
   max-of-signals; this will likely become a weighted combination once
   more signal types exist and their relative reliability is known.
   That's a decision for **when those signals are actually defined**,
   not before.

Nothing in the existing logic needs to change to add a new signal —
the architecture is additive by design.

## Why credibility is "additive on top of veracity"

`baseline_credibility` is set from the veracity classification alone —
this is "how much should I believe a claim of this type by default."
`adjusted_credibility` then adds the profile check on top, clamped to
`[0,1]`. The two values are reported separately in the IPC result so
downstream consumers (e.g. `MindManager`, `Cognitive`) can see both
"what the facts say" and "what knowing the person changes" without the
service collapsing those two into a single opaque number.

`VERIFIED_FACT` and `CONTRADICTED` claims don't strictly need profile
help — the facts already decided them. But the profile check is run
regardless because it's cheap and the matched-traits list is itself
informative output, even when it can't move the credibility number
much further.
