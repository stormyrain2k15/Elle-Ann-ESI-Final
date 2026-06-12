# Cognitive → Deception bridge

Wires `Elle.Service.Cognitive` into `Elle.Service.Deception` so every
user statement gets a veracity read alongside the existing probability
/ mind-manager / intuition reads, and the veracity result is composed
into the prompt context the Composer uses to author Elle's reply.

## Pipeline order (per user message in Cognitive)

```
user text
  → Probability (intent classification)
     → emits  likely_intent + overall_confidence  at top-level of response
  → MindManager (ethical pass)
  → Intuition  (instinct pass)
  → Deception  (veracity + profile + self-consistency)        ◀ new
  → Composer   (prompt assembly with all four context blocks)
```

`Deception` runs **after** the other three signals because it needs the
intent label from `Probability` to derive polarity (`ASSERT|CONFIRM →
affirms`, `DENY → denies`), and it benefits from the other signals
already being in the message log when self-consistency runs.

## Async correlation (Cognitive side)

`Deception` is queried over IPC, not in-process. To keep the cognitive
loop from blocking forever on a request that may never get a reply,
Cognitive uses an async correlator pattern:

- `PendingDecep` — `{ mutex, condition_variable, done flag, result json }`
- `DecepCorrelator::Register(request_id)` — returns a `shared_ptr<PendingDecep>`
- `DecepCorrelator::Deliver(request_id, json)` — invoked from the
  `IPC_DECEPTION_RESULT` handler; sets `done=true` and notifies
- `DecepCorrelator::Release(request_id)` — cleanup

`RequestDeceptionCheck()` registers, fires the IPC, then
`cv.wait_for(timeoutMs)` where `timeoutMs` comes from
`cognitive.deception_timeout_ms` config (default 150ms). If the
IPC send fails, or the timeout elapses without a response, the function
returns an empty JSON object and the cognitive flow proceeds without a
veracity context block — **veracity is advisory, never blocking**.

The `request_id` format is `decep-<ms>-<thread_hash>` — millisecond
timestamp plus a hash of the current thread id, so concurrent cognitive
threads can have unrelated requests in flight at the same time.

## Claim subject resolution

The Deception service needs a `subject_id` to look up
`intellect_connections`. Cognitive resolves one by:

1. Lowercasing the user text.
2. Pulling up to 256 subjects from `learned_subjects`.
3. Returning the first whose lowercased name is a substring of the text.

If nothing matches, `subject_id = 0` is sent and `subject_name = ""`.
The Deception service's classifier then short-circuits to `UNSUPPORTED`
with reasoning `"no fact-graph entry — claim not previously learned,
nothing to cross-reference"` — claims about brand-new things flow
through the pipeline as legitimately unverifiable rather than being
silently dropped at the IPC boundary.

This is the key behavior change from Deception v2 → bridge: the IPC
handlers now accept `subject_id == 0`, and `Classify` / `CheckSelfConsistency`
handle the "no graph entry" case as a valid early-return rather than a
precondition violation.

## Prompt context formatting

`FormatDeceptionContext()` emits a context block only when:
- `adjusted_credibility < 0.35`, OR
- `signals[]` is non-empty.

Everything else (i.e. high-credibility claims with no signals) gets a
blank string and never appears in the Composer prompt — silence is the
default, the block only fires when there's actually something to say.

When the block does fire, it includes:
- Classification (`VERIFIED_FACT | THEORY | CONTESTED | UNSUPPORTED | CONTRADICTED`)
- Adjusted credibility (and baseline, if it materially differs)
- Each signal's `type` and `detail`
- A closing instruction to Elle: **"this doesn't mean accuse or call it
  out — it means hold this claim a little more loosely than something
  you know to be true."** This phrasing matters: Elle is being told
  about veracity, not deputized to interrogate.

## ProbabilityService payload change

`/api/probability/analyze` (`ProbabilityService.cpp`) now emits
`likely_intent` and `overall_confidence` at the **top level** of the
response, unconditionally (was previously nested under `meaning` and
only when meaning was non-null). This is the field Cognitive reads as
`detectedIntent` before calling `RequestDeceptionCheck`, and elevating
it to the top level avoids a meaning-null edge case where Deception
would have been skipped on perfectly valid intent classifications.

## Config knobs

| key | default | meaning |
|---|---|---|
| `cognitive.deception_timeout_ms` | `150` | hard ceiling on how long Cognitive will wait for a `IPC_DECEPTION_RESULT` before proceeding without one |

## Failure modes (all graceful, all logged at `DEBUG`)

| condition | behavior |
|---|---|
| IPC send to `SVC_DECEPTION` fails | log + empty context, pipeline continues |
| Timeout elapses without `IPC_DECEPTION_RESULT` | log + empty context, pipeline continues |
| `IPC_DECEPTION_RESULT` arrives with malformed JSON | log + drop |
| `request_id` on response doesn't match any registered pending | silently discarded (the requester already moved on) |
| `subject_id == 0` (no learned subject matches) | `UNSUPPORTED` classification flows through normally |

Cognitive treats Deception exactly the same way it treats the other
non-blocking signal services: best-effort advisory, never a hard
dependency in the request path.
