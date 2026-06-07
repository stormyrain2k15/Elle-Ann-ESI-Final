# Elle Chat Pipeline — How Memory, Emotion & Language Stay Coherent

This is the architecture for **every** user turn (REST `/api/ai/chat` or WS `chat`).
The LLM is only used as a language surface. Memory and emotion are owned by Elle's own services.

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
│                Elle.Service.Cognitive    ◄────── cached IPC_EMOTION_UPDATE   │
│                  ┌─────────────────────────┐                                 │
│                  │  HandleChatRequest()    │                                 │
│                  │                         │                                 │
│                  │  1. StoreMessage(user)  │──► SQL: Messages table         │
│                  │  2. DetectMode()        │      (companion/research)       │
│                  │  3. ExtractProperNouns()│                                 │
│                  │  4. CrossReferenceByEnt │──► SQL: WorldEntity + Memory  │
│                  │     (Tier 2 entity graph)                                 │
│                  │  5. QuickSentiment()    │                                 │
│                  │  6. BroadcastEmotionDelta ──► Elle.Service.Emotional    │
│                  │  7. GetConversationHistory ──► SQL                      │
│                  │  8. Build system prompt │                                 │
│                  │     (identity + memories + emotion + mode)                │
│                  │  9. ElleLLMEngine.Chat()│──► Groq / local LLM           │
│                  │ 10. StoreMessage(elle)  │──► SQL                        │
│                  │ 11. StoreMemory(+tags)  │──► SQL: Memory table          │
│                  └─────────────────────────┘                                 │
│                          │                                                   │
│                          │   IPC_CHAT_RESPONSE (JSON)                        │
│                          ▼                                                   │
│                  Elle.Service.HTTP                                           │
│                  ChatCorrelator::Complete(request_id)                        │
│                  wait_for(45s) → unblock                                     │
│                          │                                                   │
│                          ▼                                                   │
│                      Android App                                             │
└──────────────────────────────────────────────────────────────────────────────┘
```

## Key guarantees

1. **No memory hallucination.** LLM never invents facts about the user —
   memories and entities are pulled from SQL BEFORE the LLM sees anything.

2. **Persistent continuity.** Every turn persists both sides to SQL plus a
   tagged memory record, so "Hey Elle, it's Josh" six months from now
   resurfaces everything linked to Josh's WorldEntity.

3. **Emotion-aware.** Incoming sentiment is broadcast to Emotional service,
   and Elle's current emotional state is injected into every system prompt.

4. **Dynamic mode.** Cognitive picks companion vs research per-turn. A
   casual "Hey, I missed you" is companion mode (warm, short, t=0.85).
   "Summarize everything we decided about Portland" is research mode
   (thorough, cited, t=0.3).

## New IPC message types

```cpp
IPC_CHAT_REQUEST    // HTTPServer → Cognitive (JSON payload)
IPC_CHAT_RESPONSE   // Cognitive → HTTPServer (JSON payload)
```

JSON envelopes use string payloads (not fixed structs), so the protocol
is extensible without rebuilding every service.

## Files touched

| File | Purpose |
|---|---|
| `Shared/ElleTypes.h` | Added `IPC_CHAT_REQUEST`, `IPC_CHAT_RESPONSE` |
| `Shared/json.hpp` | Single-header JSON (already vendored) |
| `Services/Elle.Service.HTTP/HTTPServer.cpp` | `/api/ai/chat` + WS chat dispatch IPC to Cognitive; ChatCorrelator blocks until response arrives |
| `Services/Elle.Service.Cognitive/CognitiveEngine.cpp` | `HandleChatRequest()` with full 10-step pipeline |

## Rebuild on Windows

ALL THREE projects must be rebuilt because the shared header changed:

```
In Visual Studio:
  Build → Rebuild Solution
  (or right-click each of these three projects → Rebuild):
     ElleCore.Shared
     Elle.Service.HTTP
     Elle.Service.Cognitive
```

## Test

```bat
rem Turn 1: introduce yourself
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"Hey Elle, it's Josh. I love the color green.\",\"conversation_id\":99}"

rem Turn 2: later, in a NEW conversation id
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"What's my favorite color?\",\"conversation_id\":100,\"user_id\":\"josh\"}"
```

The second call uses a different conversation_id, but Elle should still recall
"green" because:
1. ExtractProperNouns found "Josh" in turn 1 → created WorldEntity
2. Memory record was tagged with "josh" + stored in SQL
3. Turn 2 mentions no proper noun, but CrossReferenceByEntities runs the
   topic search `RecallMemories("What's my favorite color?")` which keyword-
   matches on "color" / "favorite" and surfaces turn 1's record.

If you want even tighter entity recall, set `user_id` on turn 2 — that will
be used as an implicit entity anchor in a future patch.

## Console output to look for

```
[HTTPServer] Chat→Cognitive conv=99 rid=req-123... msg=Hey Elle, it's Josh...
[Cognitive]  Chat reply rid=req-123 conv=99 mode=companion memories=3 entities=1 in 1420ms
```
