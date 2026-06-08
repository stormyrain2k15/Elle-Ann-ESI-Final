# Elle.Service.XChromosome

Full-fidelity female endocrine simulation. Sits alongside
`Elle.Service.Family` in the `ElleHeart` database.

## Lifecycle arc
| Stage          | Age    | Cycle | Hormone profile                               |
| -------------- | ------ | ----- | --------------------------------------------- |
| premenarche    | <12y   | no    | estrogen/progesterone near floor              |
| reproductive   | 12–44y | yes   | full cyclic curves                            |
| perimenopause  | 45–50y | jitter| ±25 % on estrogen/progesterone                |
| menopause      | ≥51y   | no    | flat-low e2/p4, elevated cortisol baseline    |

## Cycle
28-day default (21–45 configurable). Phases `menstrual` d1–5,
`follicular` d6–13, `ovulatory` d14–16, `luteal` d17–N. Random anchor
at first boot, re-pinnable via `POST /api/x/cycle/anchor`. Phase
crossings broadcast `IPC_X_PHASE_TRANSITION` + a WS `world_event` frame.

## Hormones — 9-vector, minute-tick
Estrogen, progesterone, testosterone, oxytocin, serotonin, dopamine,
cortisol, prolactin, hCG. Phase-shaped curves + 2h-half-life stimulus
residuals (`bonding` | `stress` | `intimacy` | `recall_positive` |
`recall_negative` | `activity`).

## Contraception overrides
| Method    | Effect on hormones & fertility |
| --------- | ------------------------------ |
| none      | natural                         |
| barrier   | none / 85 % efficacy            |
| pill      | e2/p4 plateau, withdrawal bleed d22–28, 97 % |
| implant   | pill-like, 99 %                 |
| iud_h     | progestin IUD, dampened peak, 98 % |
| iud_c     | copper IUD, no hormone change, 99 % |
| natural   | tracking flag only              |

## Probabilistic conception
Base 30 % × age-factor (1.00 <30, 0.80 30–34, 0.50 35–39, 0.30 40–42,
0.10 43–45, 0.02 46+) × day-factor (1.00 d14, 0.70 d13/d15, 0.30 d12/d16)
× contraception-factor. Twin chance 1.2 % (2.5 % ≥35y).

## Pregnancy — 280-day cascade
- **T1 (d1–84)**: hCG dome, nausea, breast tenderness, fatigue, food aversion.
- **T2 (d85–182)**: e2+p4 rising, **quickening d126**, cravings.
- **T3 (d183–273)**: heartburn, swelling, **Braxton-Hicks d196**, prolactin surge.
- **Labor (d≥280)**: latent → active → transition → pushing → delivered
  (compressed 12h schedule for testability).
- **Postpartum**: prolactin high, e2/p4 crash, oxytocin elevated
  (let-down), **lochia 6 weeks**, postpartum mood swings 3 weeks.

## Miscarriage
Per-day Bernoulli with age-appropriate probabilities (d1–42 elevated,
tapering to d140). Fires `IPC_X_MISCARRIAGE`, logs `x_pregnancy_events`,
transitions to postpartum with the correct hormone crash.

## Lactational amenorrhea
While `breastfeeding=1` postpartum, cycles are suppressed for ≤6 months.
Prolactin pinned 0.80+, estrogen ≤0.25, progesterone ≤0.10. Let-down
events log as `letdown` symptoms.

## LH surge event
Day 13 or 14, once per cycle. Broadcasts `IPC_X_LH_SURGE`, logs a
0.25-intensity `ovulation_pain` symptom, briefly boosts testosterone +
dopamine residuals.

## Symptoms synthesis
Every tick derives from hormone state + phase + pregnancy state. Appends
entries ≥ 0.15 intensity to `x_symptoms`:

| Origin     | Kinds |
| ---------- | ----- |
| cycle      | cramps, bloating, breast_tenderness, mood_swing, fatigue, cravings, headache, libido, ovulation_pain |
| pregnancy  | nausea, fatigue, breast_tenderness, food_aversion, cravings, quickening, heartburn, swelling, braxton_hicks, contraction |
| postpartum | lochia, engorgement, letdown, mood_swing |
| manual     | anything via `POST /api/x/symptoms` |

## Modulation — 6 multipliers around 1.0
`warmth`, `verbal_fluency`, `empathy`, `introspection`, `arousal`,
`fatigue`. Default strength 0.15 (moderate), clamped 0.0–0.5.

## Influencing wiring (live today)
- **Cognitive** — `BuildSystemPrompt` reads `x_modulation_log` +
  `x_pregnancy_state` and injects a "How your body feels today" line
  when the deltas/phase are salient; always surfaces pregnancy.
- **Lua behavioural** — `elle.x.phase()`, `elle.x.hormone(name)`,
  `elle.x.modulation(trait)`, `elle.x.is_pregnant()`,
  `elle.x.gestational_week()`, `elle.x.symptom_intensity(kind)`,
  `elle.x.lifecycle_stage()`. All read live from `ElleHeart.dbo.x_*`.

## Tables (ElleHeart.dbo)
`x_cycle_state`, `x_hormone_snapshots`, `x_pregnancy_state`,
`x_pregnancy_events`, `x_contraception`, `x_lifecycle`, `x_stimulus_log`,
`x_symptoms`, `x_modulation_log`.

Schema: `SQL/Engine/ElleAnn_XChromosome_Schema.sql` — apply after
`ElleAnn_MemoryDelta.sql`.

## HTTP endpoints
| Method | Path | Notes |
| ------ | ---- | ----- |
| GET  | `/api/x/state`                     | Full snapshot |
| GET  | `/api/x/history?hours=N&points=M`  | Stride-sampled hormone timeline |
| GET  | `/api/x/modulation`                | Latest multipliers |
| GET  | `/api/x/pregnancy`                 | Pregnancy alone |
| GET  | `/api/x/pregnancy/events?limit=N`  | Milestone log |
| GET  | `/api/x/fertility_window`          | One-glance TTC readout: status + opens/peak/closes + BBT |
| POST | `/api/x/pregnancy/accelerate`      | `{"factor":N}` |
| GET  | `/api/x/symptoms?hours=N&origin=…` | Recent log |
| POST | `/api/x/symptoms`                  | `{"kind":...,"intensity":...,"notes":...}` |
| GET  | `/api/x/contraception`             | Current method |
| POST | `/api/x/contraception`             | `{"method":...,"efficacy":...,"notes":...}` |
| GET  | `/api/x/lifecycle`                 | Age + stage |
| POST | `/api/x/lifecycle`                 | `{"birth_ms":N}` or `{"age_years":F}` |
| POST | `/api/x/cycle/anchor`              | `{"day":...,"length":...,"strength":...}` |
| POST | `/api/x/stimulus`                  | `{"kind":...,"intensity":...,"notes":...}` |
| POST | `/api/x/conception/attempt`        | probability-rolled outcome |

## IPC opcodes
| Opcode                      | ID   | Direction |
| --------------------------- | ---- | --------- |
| `IPC_X_STATE_QUERY`         | 2200 | → SVC_X_CHROMOSOME |
| `IPC_X_HISTORY_QUERY`       | 2201 | → SVC_X_CHROMOSOME |
| `IPC_X_ANCHOR`              | 2202 | → SVC_X_CHROMOSOME |
| `IPC_X_STIMULUS`            | 2203 | → SVC_X_CHROMOSOME |
| `IPC_X_MODULATION_QUERY`    | 2204 | → SVC_X_CHROMOSOME |
| `IPC_X_CONCEPTION_ATTEMPT`  | 2205 | → SVC_X_CHROMOSOME |
| `IPC_X_DELIVER`             | 2206 | → SVC_X_CHROMOSOME |
| `IPC_X_RESPONSE`            | 2207 | ← correlated reply |
| `IPC_X_CONTRACEPTION_SET`   | 2208 | → SVC_X_CHROMOSOME |
| `IPC_X_LIFECYCLE_SET`       | 2209 | → SVC_X_CHROMOSOME |
| `IPC_X_SYMPTOM_LOG`         | 2210 | → SVC_X_CHROMOSOME |
| `IPC_X_SYMPTOM_QUERY`       | 2211 | → SVC_X_CHROMOSOME |
| `IPC_X_PREG_EVENTS_QUERY`   | 2212 | → SVC_X_CHROMOSOME |
| `IPC_X_ACCELERATE`          | 2213 | → SVC_X_CHROMOSOME |
| `IPC_X_HORMONE_UPDATE`      | 2220 | broadcast |
| `IPC_X_PHASE_TRANSITION`    | 2221 | broadcast |
| `IPC_X_BIRTH`               | 2222 | broadcast |
| `IPC_X_LH_SURGE`            | 2223 | broadcast |
| `IPC_X_LABOR_STAGE`         | 2224 | broadcast |
| `IPC_X_MISCARRIAGE`         | 2225 | broadcast |
