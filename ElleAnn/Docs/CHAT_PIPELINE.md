# Elle Chat Pipeline

The end-to-end path for every user turn (REST `POST /api/ai/chat` or WS
`chat`). There is **no LLM** in this pipeline. Generation is performed by
`Elle.Service.Composer`, a deterministic frame-fill engine.

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                     Android / curl                                           │
│                          │                                                   │
│                          ▼                                                   │
│                  Elle.Service.HTTP                                           │
│                     POST /api/ai/chat                                        │
│                  {message, conversation_id, user_id}                         │
│                          │                                                   │
│                          │   IPC_CHAT_REQUEST (JSON envelope)                │
│                          ▼                                                   │
│                Elle.Service.Cognitive    ◄────── IPC_EMOTION_UPDATE broadcast│
│                  ┌─────────────────────────┐                                 │
│                  │  HandleChatRequest()    │                                 │
│                  │                         │                                 │
│                  │  1. StoreMessage(user)  │──► SQL: Messages table          │
│                  │  2. DetectMode()        │     (companion / research)      │
│                  │  3. ExtractProperNouns()│                                 │
│                  │  4. QuickSentiment()    │                                 │
│                  │  5. BroadcastEmotionDelta ──► SVC_EMOTIONAL              │
│                  │  6. FetchProbabilityRead ──►  SVC_PROBABILITY           │
│                  │     IPC_PROB_ANALYZE → IPC_PROB_RESPONSE                  │
│                  │  7. RequestConscienceCheck ──► SVC_MIND_MANAGER          │
│                  │     IPC_ETHICAL_QUERY → reply                             │
│                  │  8. RequestIntuition    ──► SVC_INTUITION                │
│                  │     IPC_INTUITION_REQUEST → IPC_INTUITION_RESULT          │
│                  │  9. CrossReferenceByEntities ──► SQL: WorldEntity + Memory│
│                  │ 10. GetConversationHistory   ──► SQL                     │
│                  │ 11. Build system prompt                                   │
│                  │     identity + memories + emotion + probCtx + mindCtx     │
│                  │     + intuCtx + mode                                      │
│                  │ 12. ElleComposer::Chat()   ──► SVC_COMPOSER             │
│                  │     IPC_COMPOSE_REQUEST kind=CONVERSE → IPC_COMPOSE_RESPONSE│
│                  │ 13. SendIntuitionFeedback  ──► SVC_INTUITION            │
│                  │ 14. StoreMessage(elle)     ──► SQL                        │
│                  │ 15. StoreMemory(+tags)     ──► SQL: Memory table          │
│                  └─────────────────────────┘                                 │
│                          │                                                   │
│                          │   IPC_CHAT_RESPONSE (JSON)                        │
│                          ▼                                                   │
│                  Elle.Service.HTTP                                           │
│                  ChatCorrelator::Complete(request_id)                        │
│                          │                                                   │
│                          ▼                                                   │
│                      Android / curl                                          │
└──────────────────────────────────────────────────────────────────────────────┘
```

## Key guarantees

1. **Zero LLM calls.** Generation is a deterministic 5-step pipeline in
   `Elle.Service.Composer`: frame select → slot fill → inflection →
   surface stitch → persist. No tokens, no tensors, no transformer.
2. **No memory hallucination.** Memories and entities are pulled from
   SQL *before* the composer is called. Memories cannot be invented.
3. **Persistent continuity.** Every turn persists both sides to SQL plus
   a tagged memory record. Six months later, "Hey Elle, it's Josh" will
   resurface every memory linked to Josh's WorldEntity.
4. **Emotion-aware.** Live `IPC_EMOTION_UPDATE` broadcasts from
   `SVC_EMOTIONAL` are cached in Cognitive *and* Intuition. Sentiment
   from the incoming turn is broadcast to Emotional, which updates
   its 102-dimension state.
5. **Probability-aware.** Every turn fetches a Bayesian read of the
   user's message — pragmatic act, speaker trust, top-3 emotional
   posterior, intent — via `IPC_PROB_ANALYZE`. The probabilistic_read
   becomes part of the response JSON.
6. **Conscience-checked.** `MindManager` evaluates the proposed action
   against safety/ethics rules before the composer sees the prompt.
   Non-`PROCEED` verdicts inject a "Inner-voice check" block into the
   system prompt.
7. **Gut-checked.** `Intuition` runs after conscience and returns a
   `prior_weight` + `recommended_act` + `hold_and_reflect` + `urgent`
   bundle. Only injected when the signal is strong enough to matter.
8. **Mode-aware.** Cognitive picks companion vs research mode per turn.

## IPC message types touched in one chat turn

| Type                          | Direction                                  |
|-------------------------------|--------------------------------------------|
| `IPC_CHAT_REQUEST`            | HTTP → Cognitive                           |
| `IPC_EMOTION_UPDATE`          | Cognitive → Emotional; Emotional broadcast |
| `IPC_PROB_ANALYZE`            | Cognitive → Probability                    |
| `IPC_PROB_RESPONSE`           | Probability → Cognitive                    |
| `IPC_ETHICAL_QUERY`           | Cognitive → MindManager (+ reply)          |
| `IPC_INTUITION_REQUEST`       | Cognitive → Intuition                      |
| `IPC_INTUITION_RESULT`        | Intuition → Cognitive                      |
| `IPC_INTUITION_FEEDBACK`      | Cognitive → Intuition (post-reply)         |
| `IPC_COMPOSE_REQUEST`         | Cognitive → Composer                       |
| `IPC_COMPOSE_RESPONSE`        | Composer → Cognitive                       |
| `IPC_WORLD_STATE`             | Cognitive → WorldModel (entity updates)    |
| `IPC_PROB_TRUST`              | Cognitive → Probability (trust calibration) |
| `IPC_CHAT_RESPONSE`           | Cognitive → HTTP                           |

All envelopes are JSON via `SetStringPayload(...)`, except
`IPC_EMOTION_UPDATE` which carries a binary `ELLE_EMOTION_STATE` struct.
Every envelope carries a `correlation_id`; responder services preserve
it on the reply.

## Chat reply JSON

```json
{
  "request_id":         "...",
  "response":           "Elle's English utterance",
  "conversation_id":    99,
  "mode":               "companion",
  "memories_used":      3,
  "entities":           ["Josh"],
  "latency_ms":         142,
  "provider_used":      "composer",
  "model_used":         "composer/frame-fill",
  "system_prompt_bytes": 4120,
  "probabilistic_read": { ... full Probability result ... },
  "inner_voice":        { ... MindManager verdict ... },
  "gut_read":           { ... Intuition lean/recommended_act/instincts ... }
}
```

## Timeouts (defaults from `elle_master_config.json`)

| Stage                | Knob                                  | Default |
|----------------------|---------------------------------------|--------:|
| Probability analyze  | `cognitive.probability_timeout_ms`    |    300  |
| Conscience check     | `cognitive.conscience_timeout_ms`     |    200  |
| Intuition request    | `cognitive.intuition_timeout_ms`      |    150  |
| Composer compose     | (per-frame; service-internal)         |     -   |

Any stage that times out degrades gracefully: the corresponding context
block is suppressed and the pipeline continues.

## Files touched in one chat turn

| File                                                          | Role                       |
|---------------------------------------------------------------|----------------------------|
| `Services/Elle.Service.HTTP/HTTPServer.cpp`                   | Route + ChatCorrelator     |
| `Services/Elle.Service.Cognitive/CognitiveEngine.cpp`         | `HandleChatRequest` pipeline |
| `Services/_Shared/ElleComposerClient.h`                       | Helper that wraps `IPC_COMPOSE_REQUEST` |
| `Services/Elle.Service.Probability/service/ProbabilityService.cpp` | Analyze handler       |
| `Services/Elle.Service.MindManager/MindManager.cpp`           | Conscience handler         |
| `Services/Elle.Service.Intuition/Intuition.cpp`               | Two-tier instinct + gut    |
| `Services/Elle.Service.Composer/Composer.cpp`                 | 5-step pipeline + persist  |
| `Services/Elle.Service.Emotional/EmotionalEngine.cpp`         | Emotion state + broadcast  |
| `Services/_Shared/ElleTypes.h`                                | All IPC opcodes / svc ids  |

## Quick e2e smoke

```bat
rem Turn 1 — introduce yourself
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"Hey Elle, it's Josh. I love the color green.\",\"conversation_id\":99,\"user_id\":\"josh\"}"

rem Turn 2 — in a NEW conversation_id
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"What's my favorite color?\",\"conversation_id\":100,\"user_id\":\"josh\"}"
```

Turn 2 should recall "green" because:
1. `ExtractProperNouns` found "Josh" in turn 1 → upserted to WorldEntity
2. Memory record tagged with "josh" + stored in SQL
3. Turn 2's `CrossReferenceByEntities` topic-searches for "color" + "favorite"
   and surfaces turn 1's record
4. The Composer's `CONVERSE` frame fills its `[SLOT:OBJECT]` from the recalled
   memory rather than inventing a colour

## Console output to look for

```
[HTTPServer] Chat→Cognitive conv=99 rid=req-123... msg=Hey Elle, it's Josh...
[Probability] analyze rid=prob-... act=GREET trust=0.50 conf=0.78
[MindManager] verdict rid=mind-pre-... PROCEED severity=0.00
[Intuition]   intu-pre-... lean=SAFE conf=0.61 weight=0.42 act=ACK_AND_PROBE
[Composer]    compose rid=cmp-... kind=CONVERSE act=ASSERT frame=12 slots=4
[Cognitive]   Chat reply rid=req-123 conv=99 mode=companion memories=3 entities=1 in 142ms
```
