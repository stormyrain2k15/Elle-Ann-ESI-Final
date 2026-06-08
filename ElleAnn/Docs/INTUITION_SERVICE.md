# Elle.Service.Intuition

The Intuition service is a two-tier pre-cognition pipeline that runs before
heavy reasoning in the Cognitive engine. It is fully deterministic, has zero
LLM dependencies, and is wired into the existing IOCP Named-Pipe mesh.

## Service identity

- `ELLE_SERVICE_ID`: `SVC_INTUITION` (id 25, last before `ELLE_SERVICE_COUNT`).
- Pipe / human name: `Intuition`.
- Dependencies (declared in `OnStart`): `SVC_HEARTBEAT`, `SVC_PROBABILITY`,
  `SVC_EMOTIONAL`.

## Tiers

### Tier 1 — Instinct (microsecond, pattern-matched)

Loaded from `ElleHeart.dbo.intuition_pattern`. Each row is a weighted
stimulus → pull mapping:

```
stimulus_tag  → pull_type (COMFORT|PROTECT|RETREAT|ALERT|GUARD|OPEN|
                            RECIPROCATE|SLOW|ENGAGE|CHECK_IN)
weight        in [0.1, 1.0]
trust_floor   minimum speaker trust required to fire
emotion_min   minimum emotional intensity required to fire
urgent        bypasses normal IPC queue priority when true
```

If the DB table is absent, the service falls back to ~30 factory-installed
baseline instincts covering threat / distress / warmth / deception /
familiarity / confusion / joy / withdrawal vectors.

### Tier 2 — Intuition (synthesized gut signal)

Aggregates fired instincts plus live state into a single directional lean:

```
DANGER | DOUBT | SAFE | REACH_OUT | ENGAGE | UNCERTAIN
```

The signal carries `confidence` (clamped against belief entropy),
`entropy`, `suppress_reason` (true = Elle knows but can't yet explain why),
and an audit `basis` string.

### Combined output for Cognitive

```
prior_weight       in [0, 0.85]  — how much weight Cognitive should give
                                    this signal before full reasoning.
                                    Capped at 0.65 on pre-response queries
                                    so the gut never overrides reason.
recommended_act    WARN | QUESTION | ASSERT | COMFORT | ACK_AND_PROBE
hold_and_reflect   true = slow down, something is off
urgent             true = fast-path the MindManager ping
```

## IPC interface

| direction | type                       | payload                                                |
|-----------|----------------------------|--------------------------------------------------------|
| in        | `IPC_INTUITION_REQUEST`    | JSON (see request schema below)                        |
| in        | `IPC_EMOTION_UPDATE`       | binary `ELLE_EMOTION_STATE` (broadcast from Emotional) |
| in        | `IPC_PROB_RESPONSE`        | JSON (broadcast — used as cache fallback only)         |
| in        | `IPC_INTUITION_FEEDBACK`   | JSON `{pull_type, was_correct, strength}`              |
| out       | `IPC_INTUITION_RESULT`     | JSON (see response schema below)                       |

### Request schema (Cognitive → Intuition)

```json
{
  "request_id":         "intu-pre-<ms>-<thread>",
  "stimulus_tags":      ["distress", "fear", "Josh"],
  "emotion_valence":    -0.4,
  "emotion_arousal":    0.6,
  "emotion_intensity":  0.5,
  "speaker_id":         "default",
  "speaker_trust":      0.5,
  "belief_entropy":     0.3,
  "is_pre_response":    true,
  "return_to":          24
}
```

### Response schema (Intuition → Cognitive)

```json
{
  "request_id":        "intu-pre-...",
  "prior_weight":      0.42,
  "recommended_act":   "COMFORT",
  "hold_and_reflect":  false,
  "urgent":            false,
  "intuition": {
    "lean":            "REACH_OUT",
    "confidence":      0.61,
    "entropy":         0.30,
    "basis":           "lean=REACH_OUT conf=0.61 instincts=3",
    "suppress_reason": false
  },
  "instincts": [
    { "pull_type": "COMFORT", "strength": 0.86, "reason": "pattern:distress -> COMFORT", "urgent": false }
  ]
}
```

## Cognitive integration

`ElleCognitiveService::HandleChatRequest` calls intuition after probability
and conscience, before memory recall:

```
prob ─┐
mind ─┼─► intuition ─► prompt ctx ─► response ─► feedback
emo  ─┘
```

After the chat reply ships, `SendIntuitionFeedback` pings
`IPC_INTUITION_FEEDBACK` so per-pattern weights can decay or strengthen in
`ElleHeart.dbo.intuition_pattern`.

`FormatIntuitionContext` emits a `"Gut read"` block into the system prompt
only when `prior_weight >= 0.25`, `hold_and_reflect`, or `urgent` is set —
so trivial gut reads stay silent.

A 150 ms timeout is enforced via
`cognitive.intuition_timeout_ms` in `elle_master_config.json`. On timeout
or send failure the pipeline degrades cleanly (returns `{}` and the prompt
block is suppressed).

## SQL footprint

- `ElleHeart.dbo.intuition_pattern` — pattern table (auto-created on boot).
- `ElleHeart.dbo.intuition_log`     — per-firing audit log with covering
  index `IX_intuition_log_recorded (recorded_ms DESC)`.

## Service mesh registration

- `Services/_Shared/ElleTypes.h`: `SVC_INTUITION` in `ELLE_SERVICE_ID`,
  `ELLE_SERVICE_COUNT = 26`, plus `IPC_INTUITION_REQUEST / RESULT /
  FEEDBACK` in `ELLE_IPC_MSG_TYPE`.
- `Services/_Shared/ElleQueueIPC.cpp`: `g_serviceNames[]` includes
  `"Intuition"`, guarded by a `static_assert` against `ELLE_SERVICE_COUNT`.
- `ElleAnn.sln`: project entry `{B1000000-0000-0000-0000-000000000019}` in
  the `Services` folder, with Debug|x64 + Release|x64 configs.
- `Services/Elle.Service.Intuition/Elle.Service.Intuition.vcxproj`:
  references the shared `ElleCore.Shared.vcxproj`.

## Nude code policy

`Intuition.cpp` is fully stripped of comments and block banners — only
source code remains. Documentation lives here in `Docs/`.
