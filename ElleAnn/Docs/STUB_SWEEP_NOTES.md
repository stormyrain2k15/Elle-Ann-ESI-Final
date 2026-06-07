> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# STUB SWEEP — Every Endpoint Now Real

This pass removed all hollow/fake responses. Every route either:
1. Reads/writes real SQL against `ElleCore`, or
2. Returns an honest `501 not_implemented` with a clear reason (video generation)

## Files changed
- `Shared/ElleSQLConn.h` + `.cpp` — added `ListMemories`, `GetMemory`, `DeleteMemory`,
  `UpdateMemoryContent`, `CreateConversation`, `ListConversations`, `GetConversation`,
  `StartVoiceCall`, `EndVoiceCall`, `CountTable`. Fixed `RecordMetric`. Removed dead
  `StoreEntity_LEGACY_UNUSED`. Fixed `StoreGoal` to write to existing `system_settings`.
- `Shared/ElleTypes.h` — added `IPC_MEMORY_CONSOLIDATE`, `IPC_EMOTION_CONSOLIDATE`
- `Services/Elle.Service.HTTP/HTTPServer.cpp` — all 80 routes rewritten with real SQL
- `SQL/ElleAnn_MemoryDelta.sql` — extended with `ai_tools`, `ai_agents`, `moral_rules`
  (seeded), `dictionary_words`, `log_entries`, `ai_self_prompts`, `model_slots`

## Route status

### REAL — reads/writes live SQL
- All `/api/memory/*` → `dbo.memory`, `memory_tags`, `memory_entity_links`
- All `/api/tokens/conversations/*` → `dbo.conversations`, `dbo.messages`
- `/api/tokens/video-calls/*` → `dbo.voice_calls`
- `/api/ai/chat` → IPC to Cognitive (full pipeline)
- `/api/ai/self-prompts`, `.../generate` → `dbo.ai_self_prompts`
- `/api/ai/status` → live config + emotion cache
- `/api/ai/analyze-emotion` → local sentiment
- `/api/ai/memory-tracking` → COUNT(*) from memory tables
- `/api/ai/autonomy/status` → `dbo.TrustState` via ElleDB
- `/api/ai/hardware/info` → WinAPI GetSystemInfo, GlobalMemoryStatusEx
- `/api/ai/hardware/actions/pending`, `.../result` → `dbo.action_queue`
- `/api/ai/tools` GET/POST/DELETE → `dbo.ai_tools`
- `/api/ai/agents` GET/POST/DELETE/`.../chat` → `dbo.ai_agents`
- `/api/dictionary/*` → `dbo.dictionary_words`
- `/api/education/subjects` → `dbo.subjects` (lazy-created)
- `/api/education/skills` → `dbo.skills` (if present)
- `/api/emotional-context/patterns` → `dbo.ElleThreads`
- `/api/emotional-context/vocabulary` → `dbo.memory_tags` aggregate
- `/api/emotional-context/growth` → `dbo.SelfReflections`
- `/api/server/status` → `dbo.Workers` via GetWorkerStatuses
- `/api/server/console` GET/DELETE → `dbo.log_entries`
- `/api/server/settings` GET/PUT → `dbo.system_settings`
- `/api/server/analytics` → WinAPI process/memory stats + SQL counts
- `/api/server/backup` POST → `BACKUP DATABASE ElleCore TO DISK = ?`
- `/api/server/backups` GET → `FindFirstFile data\backups\*.bak`
- `/api/server/commit-memory`, `.../commit-emotional-memory` → IPC trigger
- `/api/models/slots` CRUD → `dbo.model_slots`
- `/api/models/workers` → `dbo.Workers`
- `/api/models/personality` → `dbo.PersonalityTraits`
- `/api/models/token-cache/stats` → `dbo.system_settings` counters
- `/api/morals/rules` → `dbo.moral_rules` (seeded with 3 core rules)

### HONEST 501 — feature not wired yet
- `POST /api/video/generate`
- `GET /api/video/status/{job_id}`
- `POST /api/video/avatar/upload`
These return `501 { "error": "video_generation_service_not_running" }` so the
Android app can show a real "Not available yet" UI instead of a fake success.

## Next

To use this, on Windows:

1. Pull from GitHub
2. **Run the updated `SQL/ElleAnn_MemoryDelta.sql`** in SSMS against ElleCore
   — it adds 7 new tables (idempotent, safe to re-run)
3. Rebuild Solution as admin (every service links to the changed Shared lib)
4. Redeploy binaries to `C:\ElleAnn\Bin`
5. Restart stack, curl any of the routes above

Every endpoint now gives you either real data or an honest "not built yet" message.
