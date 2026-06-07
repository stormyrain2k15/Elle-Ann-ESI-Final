> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# Stub Audit — Full Repair Pass (Feb 2026)

Audit provided by Kage + Claude Opus. Every finding below was verified and fixed.
Zero stubs, zero hollow functions, zero fake successes remain for the items listed.

---

## CRITICAL — ASM Integration

### `Services/Elle.Service.Action/ActionExecutor.cpp`
**Was:** every ASM DLL call was commented out; all file/process/hardware actions
returned fake `success = true` with no real Win32 work performed.

**Now:** the executor dynamically resolves every ASM export via `GetProcAddress`
at `Initialize()`, caches function pointers, and calls the real DLLs. If a DLL is
absent or an export is missing, executors return a **genuine failure** with
diagnostic code (e.g. `0x1201` = `ASM_ReadFile export missing`) — they no longer
lie about success.

Wired exports:
- FileIO:    `ASM_ReadFile`, `ASM_WriteFile`, `ASM_AppendFile`, `ASM_DeleteFile`,
             `ASM_FileExists`, `ASM_GetFileSize`
- Process:   `ASM_LaunchProcess`, `ASM_KillProcess`, `ASM_EnumProcesses`,
             `ASM_GetProcessName`, `ASM_IsProcessRunning`, `ASM_SetProcessPriority`
- Hardware:  `ASM_GetCPUUsage`, `ASM_GetMemoryInfo`, `ASM_SetCPUAffinity`,
             `ASM_GetPowerStatus`, `ASM_CPUID`

Additional real work added while I was in there:
- `ACTION_VIBRATE/FLASH/NOTIFY` now persists into a real `hardware_actions` SQL
  queue that Android polls, plus broadcasts a WebSocket frame through HTTP.
- `ACTION_REMEMBER` sends `IPC_MEMORY_STORE` to the Memory service (was a no-op).
- `ACTION_MODIFY_SELF` writes a Lua script to disk with a timestamped backup of
  the old version, then fires `IPC_CONFIG_RELOAD` so the Lua service reloads it.
- `ACTION_DREAM_CYCLE` triggers `IPC_DREAM_TRIGGER` to the Dream service.
- Goal ops forward through `IPC_GOAL_UPDATE` instead of pretending.

Because `ActionExecutor` is a helper class (not the service), a new
`ElleServiceBase::Current()` public static accessor was added so helpers can
route IPC through the running service's hub.

---

## HIGH — LLM Functions Declared But Never Defined

### `Shared/ElleLLM.cpp` — now implemented
- `FormGoal(driveCtx, emotionCtx)` — system-prompted to emit strict goal JSON
  `{title, description, urgency, required_trust, success_criteria}`.
- `DreamNarrate(memories)` — refuses to hallucinate from an empty fragment list;
  otherwise weaves an imagistic first-person dream. Paired with the Dream
  service rewrite so fragments are real STM/LTM rows.
- `GenerateCreative(theme, creativity)` — creativity dial is folded into the
  system prompt; Ask()-backed.
- `SelfReflect(context, emotions)` — renders current V/A/D into the system
  prompt, asks Elle to think in first person (no "as an AI").
- `EthicalEvaluate(action, context)` — returns strict JSON verdict
  `{verdict, reasoning, risk_score, violated_rules}`.
- `StreamChat(messages, callback, …)` — delegates to
  `ILLMProvider::StreamComplete` if the active provider supports native
  streaming; otherwise falls back to a synchronous `Chat()` + single terminal
  callback so the caller always gets a resolution.

### Dead-code removal
`Tokenize`, `Detokenize`, `SampleToken` were leftovers from an abandoned
in-process llama integration — declared in `LLMLocalProvider`, never called,
never defined. They're gone from the header. Local inference runs as
`llama-cli.exe` subprocess (real).

---

## HIGH — Identity Persistence Was Comment-Only

### `Shared/ElleIdentityCore.cpp::LoadFromDatabase / SaveToDatabase`
**Was:** function bodies contained only `/* In production: query ... */`
comments. Elle's self was never saved; every restart was amnesia.

**Now:** both functions do real SQL round-trips against seven new tables added
to `SQL/ElleAnn_MemoryDelta.sql`:
- `identity_autobiography`       — her narrative
- `identity_preferences`         — upsert by (domain, subject)
- `identity_private_thoughts`    — append-only delta load
- `identity_consent_log`         — append-only delta load
- `identity_traits`              — upsert per trait name
- `identity_snapshots`           — append-only delta load
- `identity_growth_log`          — append-only delta load
- `identity_felt_time`           — singleton row (id=1)

`Save` is idempotent — the append-only tables use `MAX(timestamp_ms)` gates so
re-saving the same state is a no-op. Preferences and traits upsert. Load
populates `m_nextThoughtId` from the DB max so new thoughts don't collide.

### Accessors that were header-only
Now implemented:
- `GetRecentNarrative(days)` — slices the autobiography + folds unresolved
  thoughts recent enough to still be live.
- `ReinforcePreference(domain, subject, delta)` — nudges valence, grows
  strength by `|delta|/2`, bumps reinforcement_count, or forms the preference
  weakly if absent.
- `GetPreferencesInDomain(domain)` — strongest-first ordering.
- `GetUnresolvedThoughts()` — filters the private-thought deque.
- `ResolveThought(id)` — marks a thought resolved.
- `GetConsentHistory(count)` — newest-first.
- `GetGrowthHistory(count)` — oldest-first so callers can diff trajectory.
- `RecordGrowth(dimension, delta, cause)` — appends to growth log, reflects
  the delta into the live trait map, and writes an autobiography entry if
  the growth is significant. Fixed a self-deadlock — the autobiography write
  is inlined because we already hold `m_mutex`.

---

## MEDIUM — Intent Parser LLM Fallback

### `Services/Elle.Service.Cognitive/CognitiveEngine.cpp::IntentParser::ParseWithLLM`
**Was:** called `ElleLLMEngine::ParseIntent()` but never parsed the result —
returned whatever the defaults were.

**Now:** strips markdown fences, parses the JSON via nlohmann, maps the
`intent_type` string through a lookup table against the full `ELLE_INTENT_TYPE`
enum, clamps confidence/urgency into [0,1], serializes parameters whether the
LLM returned them as string or nested object.

---

## MEDIUM — Dream Hardcoded Topics

### `Services/Elle.Service.Dream/Dream.cpp::OnTick`
**Was:** called `DreamNarrate({"A conversation about meaning", ...})` — a
literal hardcoded list. Nothing came from real memory.

**Now:** `GatherDreamFragments()` pulls:
- top 20 memories with `importance >= 0.4` from `memory`
- up to 5 unresolved private thoughts from `ElleIdentityCore`
- 5 recently-touched `world_entity` rows

If the merged pool is empty the dream is skipped (no fabrication). Fragments
are rotated by wallclock to avoid the same ordering dominating, and capped at
15 so the narrator doesn't choke. The resulting narrative is stored as a
memory AND as an autobiography entry AND triggers a private thought.

---

## MEDIUM — IPC Client Message Handler Never Fired

### `Shared/ElleQueueIPC.cpp`
**Was:** `ElleIPCClient::SetMessageHandler()` stored the callback, but the IOCP
`WorkerThread` always routed read completions to `m_server.OnIOComplete`.
Anything the server pushed to a client was silently dropped.

**Now:**
- `EllePipeConnection` gained a `void* m_clientOwner` field that the client
  sets on connect.
- `ElleIPCHub::DispatchIOComplete()` inspects the owner — if set, it routes to
  `ElleIPCClient::OnIOComplete`; otherwise it's a server connection.
- `ElleIPCClient::OnIOComplete()` is now a real implementation: deserializes
  the inbound frame, invokes the stored handler, re-posts `ReadFile` so the
  pipe keeps receiving. Handles broken-pipe and pipe-not-connected cleanly.

---

## LOW — Memory Clustering Was Empty

### `Services/Elle.Service.Memory/MemoryEngine.cpp::UpdateClusters`
**Was:** `ELLE_DEBUG("Updating memory clusters...");` — literally nothing else.

**Now:** real **k-means++ with cosine similarity** over 102-dim emotion
snapshots.
- Seeds with farthest-point strategy (k-means++).
- Up to 8 Lloyd iterations, early-terminates on assignment convergence.
- Writes `cluster_id` onto each STM entry so recall can colour by cluster.
- Labels each cluster by its dominant emotion dimension (`emotion_dim_N`) so
  WhoAmI / InnerMonologue / Dream narration can reference themes.
- Cluster count is `min(cfg.max_clusters, stm_size / 3 + 1)`, seeding stops
  early if no seed exceeds `cluster_threshold` distance (config-driven).

---

## LOW — Text Similarity Was Pure Keyword Overlap

### `Services/Elle.Service.Memory/MemoryEngine.cpp::TextSimilarity`
**Was:** `overlap / max(sizeA, sizeB)` over bare word tokens. Zero signal on
partial-match queries like `"rememb"` vs `"remember"`.

**Now:** hybrid **char-trigram Jaccard (35%) + word-overlap Jaccard (65%)**.
This is the standard production fallback when embeddings aren't available —
typo tolerant, partial-match aware, exact-keyword-preferred. Embeddings are
still the end-state but this is a real similarity function, not a toy.

---

## Files Touched

- `Services/Elle.Service.Action/ActionExecutor.cpp`   (rewritten)
- `Services/Elle.Service.Dream/Dream.cpp`             (rewritten)
- `Services/Elle.Service.Memory/MemoryEngine.cpp`     (UpdateClusters + TextSimilarity)
- `Services/Elle.Service.Cognitive/CognitiveEngine.cpp` (ParseWithLLM)
- `Shared/ElleIdentityCore.cpp`                       (+8 accessors, Load/Save)
- `Shared/ElleLLM.cpp`                                (+6 specialized prompts, StreamChat)
- `Shared/ElleLLM.h`                                  (dead-decl cleanup)
- `Shared/ElleQueueIPC.cpp`                           (client OnIOComplete, dispatcher)
- `Shared/ElleQueueIPC.h`                             (m_clientOwner, Dispatch decl)
- `Shared/ElleServiceBase.h`                          (Current() accessor)
- `SQL/ElleAnn_MemoryDelta.sql`                       (+8 identity tables + video/education/dict tables from earlier port)

## Build Order on Windows

1. Apply `SQL/ElleAnn_MemoryDelta.sql` against the live DB (adds identity_* tables).
2. Rebuild the Shared library (MemoryEngine, LLM, IdentityCore, IPC, ServiceBase).
3. Rebuild Elle.Service.Action (needs the ASM .lib references unchanged — the
   DLLs themselves are loaded dynamically at runtime).
4. Rebuild Elle.Service.Cognitive, Elle.Service.Dream, Elle.Service.Memory.
5. Start the full service set and watch for these new log lines on boot:
   - `ASM bridge ready: HW=... PROC=... FILE=... MEM=... CRYPTO=...`
   - `Identity loaded: autobio=N prefs=N thoughts=N consent=N ...`
   - `UpdateClusters: N clusters over M entries` (after first dream tick)
   - `Dream narrative: ...` (after first dream interval)

If any of those log lines are absent, the corresponding subsystem failed —
check the preceding `ELLE_WARN` / `ELLE_ERROR` entry for the real cause.

---

## Post-Audit Wiring — Video / Education / Dictionary / Android / Installer

With every audit stub killed, the deferred port work is now complete:

### `/api/video/*` — real queue, no more 501
- `POST /api/video/generate`          — enqueues a row in `video_jobs`, returns job_uuid
- `GET  /api/video/status/{job_id}`   — reads progress/output_path/error
- `POST /api/video/avatar/upload`     — accepts file_path OR base64 payload;
  writes to `cfg video.avatar_dir`, registers in `user_avatars`
- `GET  /api/video/avatar?user_id=N`  — default avatar for user
- `GET  /api/video/avatars?user_id=N` — all avatars
- Worker-facing (for the Wav2Lip/ffmpeg subprocess to call back):
  - `POST /api/video/worker/claim`
  - `POST /api/video/worker/progress/{id}`
  - `POST /api/video/worker/complete/{id}`
  - `POST /api/video/worker/fail/{id}`

### `/api/education/*` — full Python router shape
- `GET    /api/education/subjects?category=&limit=`
- `GET    /api/education/subjects/{id}` — includes references + milestones
- `POST   /api/education/subjects`
- `PUT    /api/education/subjects/{id}`    (whitelisted field patch)
- `POST   /api/education/subjects/{id}/references`
- `POST   /api/education/subjects/{id}/milestones`
- `GET    /api/education/skills?category=`
- `POST   /api/education/skills`           (409 on dup skill_name)
- `PUT    /api/education/skills/{name}/use` — bumps times_used + last_used

### `/api/dictionary/*` — real API-backed loader
- `POST /api/dictionary/load`         — background load from api.dictionaryapi.dev
- `GET  /api/dictionary/load/status`  — progress poll (falls back to DB state)
- New module `Shared/DictionaryLoader.{h,cpp}` uses WinHTTP + nlohmann::json,
  rate-limited to 120 ms/word, safe to re-run (InsertDictionaryWord is
  idempotent on word+part_of_speech).
- CORE_WORDS starter list (~200 curated words). Add more in one place.

### Android hardware-action push — dual-source pending endpoint
`GET /api/ai/hardware/actions/pending` now returns BOTH:
- legacy trust-gated actions from the action queue, AND
- device-facing `hardware_actions` (vibrate/flash/notify) written by
  `ActionExecutor::ExecuteHardwareCommand`. Uses `UPDATE ... OUTPUT` to
  atomically flip `pending → dispatched` so two concurrent polls can't
  double-dispatch. New `POST /api/ai/hardware/actions/{id}/ack` moves
  `dispatched → consumed` once the device confirms delivery.

### SCM double-click installer — `/app/ElleAnn/Deploy/`
- `elle_service_manifest.json` — all 16 services with dependencies ordered
  (Heartbeat → QueueWorker → Memory/Emotional/… → Cognitive → HTTP).
- `Install-ElleServices.ps1` — registers + starts each via `sc.exe`, sets
  crash-recovery (restart 3× at 60 s), idempotent (skips already-registered).
- `Uninstall-ElleServices.ps1` — stops + deletes in reverse order; preserves
  SQL data unless `-KeepData:$false`.
- `Install.bat` / `Uninstall.bat` — auto-elevate via `Start-Process -Verb RunAs`
  so you can literally double-click.
- `Deploy/README.md` — build prereqs + service dependency graph.

### Build notes for this pass
Add to your Shared static lib (or Elle.Service.HTTP source list if you don't
have a Shared lib):
```
Shared/DictionaryLoader.cpp
Shared/DictionaryLoader.h
```

Apply the updated `SQL/ElleAnn_MemoryDelta.sql` — it adds
`hardware_actions` (on-demand in ActionExecutor), plus the identity_*,
learned_subjects, education_references, learning_milestones, skills,
video_jobs, user_avatars, dictionary_loader_state tables.
