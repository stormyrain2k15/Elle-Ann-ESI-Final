> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# Second-Wave Audit — Completion Report

All items from the user's second-wave audit are now addressed.
File and line refs below match the post-fix code.

---

## 1. Cognitive intent router broken
`Services/Elle.Service.Cognitive/CognitiveEngine.cpp::RouteIntent`
Every branch now speaks the *receiver's* native opcode:
- Memory ← `IPC_MEMORY_STORE` / `IPC_MEMORY_RECALL`
- Emotional ← `IPC_EMOTION_UPDATE`
- Goal ← `IPC_GOAL_UPDATE`
- World ← `IPC_WORLD_STATE`
- SelfPrompt ← `IPC_SELF_PROMPT`
- Dream ← `IPC_DREAM_TRIGGER`
- Action ← `IPC_ACTION_REQUEST`

## 2. SelfPrompt wrong payload type
`Services/Elle.Service.SelfPrompt/SelfPrompt.cpp`
Autonomous thoughts are now `ElleDB::SubmitIntent(ELLE_INTENT_RECORD{…})`
rows that QueueWorker hands to Cognitive as proper binary records.

## 3. Bonding proactive path wrong destination
`Services/Elle.Service.Bonding/Bonding.cpp:OnTick`
Proactive impulses now submit a real `INTENT_CHAT` row AND emit
`IPC_WORLD_EVENT` (JSON) to `SVC_HTTP_SERVER` for live WS fan-out.

## 4. InnerLife expression wrong destination
`Services/Elle.Service.InnerLife/InnerLife.cpp:OnTick`
Inner expressions now submit a real `INTENT_EMOTIONAL_EXPRESSION`
row AND emit `IPC_SELF_PROMPT` to `SVC_SELF_PROMPT` (which owns that
opcode). Cognitive is no longer the target — it doesn't handle that opcode.

## 5. Bonding.ProcessInteraction unreachable
Cognitive emits `IPC_INTERACTION_RECORDED` at the end of every chat turn
with the full turn context; Bonding's `OnMessage` parses it and calls
`ProcessInteraction`.

## 6. Bonding relationship state not durable
`LoadRelationshipState` / `SaveRelationshipState` now do real SQL
round-trips against `ElleHeart.dbo.relationship_state` (lazy-created),
saved after every ProcessInteraction().

## 7. Bonding dead helpers
- `GetRelationshipContext` — now has a real caller: written to
  `ElleHeart.dbo.bonding_context` via `PersistContextToDatabase()` every
  interaction, and consumed by Cognitive's chat system prompt builder.
- `AddSharedReference`, `RecordSignificantMoment` — **deleted** (no
  meaningful call sites; DRY-NO-STUB).

## 8. Goal ID mismatch
`Services/Elle.Service.GoalEngine/GoalEngine.cpp::CreateGoal`
Now uses `ElleDB::StoreGoalReturningId` so in-memory `goal.id` always
matches the DB IDENTITY. Dead `m_nextId` field removed.

## 9. UpdateProgress dead path
`GoalEngine.Tick(drives)` now auto-credits progress when the source
drive's intensity drops below `threshold * 0.8`, giving
`UpdateProgress` a real caller. Tick is wired to drive state pulled
from SQL each tick.

## 10. WorldModel dead service
`CognitiveEngine.HandleChatRequest` now emits `IPC_WORLD_STATE` to
`SVC_WORLD_MODEL` for every discovered/observed entity, routing all
world-entity SQL writes through WorldModel (the authoritative owner).

## 11. WorldModel no DB hydration
`WorldModel.Initialize()` calls `ElleDB::GetAllEntities(rows)`
(newly added to `Shared/ElleSQLConn`).

## 12. WorldModel dead functions
`PredictBehavior` and `GetWorldSummary` — **deleted**.

## 13. Cognitive stranded paths
`ProcessInput`, `Reason`, `Metacognize`, `AllocateThread`, the
`CognitiveThread` struct, and their backing members — **deleted**.
The chat pipeline (`ElleCognitiveService::HandleChatRequest`)
fully supersedes them.

## 14. Cognitive fragile first-{/last-} JSON slicing
Replaced with `Elle::ExtractJsonObject` (brace-balanced extractor from
`Shared/ElleJsonExtract.h`), tolerant to fences, prose, nested braces.

## 15. HTTP detached thread shutdown hazard
`HTTPServer::OnStop` now drain-fences `m_activeHttpHandlers` and
`m_activeWsClients` (with 10s cap) before tearing down, so detached
request handlers and WS readers finish touching member state before
destruction.

## 16. Cognitive detached chat-thread shutdown hazard
`ElleCognitiveService::OnStop` drain-fences `m_inflightChats` (5s cap).

## 17. Lua live placeholder bindings
- `elle.get_emotion(name)` → `ElleDB::LoadLatestEmotionSnapshot`
  (valence/arousal/dominance/indexed dimensions).
- `elle.get_trust()` → `ElleDB::GetTrustState().score`.

## 18. Lua dead wrappers
`LuaHost::Call`, `ShapeResponse`, `ScoreIntent` — **deleted**. The
service's public surface is now just `Eval` + `ReloadScripts`.

## 19. InnerLife half-built
- `PostResponseCheck` — wired to `IPC_POST_RESPONSE` from Cognitive.
- `GetInnerLifeContext` — now written to `ElleHeart.dbo.innerlife_context`
  each tick and consumed by Cognitive's chat system prompt.
- `HowDoIFeelAboutThis` — **deleted** (consent surface lives in
  `ElleIdentityCore::EvaluateConsent`, no call sites for this duplicate).

## 20. XChromosome emitting to uncompiled SVC_FAMILY
Delivery attempts are now persisted to `ElleHeart.dbo.x_conception_attempts`
(durable). The IPC send is kept for forward-compat — when SVC_FAMILY
comes online it will drain the backlog from that table.

## 21. Continuity manual fragile JSON
`Continuity.cpp::GenerateReconnectionGreeting` — `awayDesc` is now
passed through `EscapeJson()` before being embedded in the context
JSON bundle, so embedded quotes/newlines can never corrupt it.

## 22. Fragmented ElleIdentityCore (the big architectural one)
`Shared/ElleIdentityCore`:
- Added `RefreshFromDatabase(min_interval_ms)` that pulls the
  persisted state from SQL while preserving the current process's
  in-flight session counters.
- `Bonding::OnTick`, `InnerLife::OnTick`, `Solitude::OnTick`,
  `Dream::OnTick` each call `RefreshFromDatabase()` at the top so
  they see cross-process writes (autobiography, private thoughts,
  traits, preferences) on their next cycle.
- `Continuity::OnTick` now calls `SaveToDatabase()` every tick
  (previously only on Shutdown), so peer processes actually have
  something fresh to pull.

This gives every process an eventually-consistent view of the one
unified mind without adding a new IPC fabric. Stronger options
(single-writer + event stream) remain possible later; this is the
minimum correct fix that doesn't require new infrastructure.

---

## Files touched
- `Shared/ElleIdentityCore.h` / `.cpp`
- `Shared/ElleSQLConn.h` / `.cpp` (added `GetAllEntities`)
- `Services/Elle.Service.Cognitive/CognitiveEngine.cpp`
- `Services/Elle.Service.Bonding/Bonding.cpp`
- `Services/Elle.Service.InnerLife/InnerLife.cpp`
- `Services/Elle.Service.WorldModel/WorldModel.cpp`
- `Services/Elle.Service.GoalEngine/GoalEngine.cpp`
- `Services/Elle.Service.Continuity/Continuity.cpp`
- `Services/Elle.Service.Solitude/Solitude.cpp`
- `Services/Elle.Service.Dream/Dream.cpp`
- `Services/Elle.Service.XChromosome/XChromosome.cpp`
- `Services/Elle.Service.HTTP/HTTPServer.cpp`
- `Lua/Elle.Lua.Behavioral/LuaHost.cpp`

## New SQL tables (lazy-created on first write)
- `ElleHeart.dbo.bonding_context` (singleton id=1, Bonding writer)
- `ElleHeart.dbo.innerlife_context` (singleton id=1, InnerLife writer)
- `ElleHeart.dbo.x_conception_attempts` (append-only, XChromosome writer)

## Verification
Brace-balance check (comments + strings stripped) passes on every
modified file. Local compile on the user's Windows machine is required
for full validation — no Windows toolchain available in this
environment.
