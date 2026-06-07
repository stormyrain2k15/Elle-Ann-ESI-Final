# Schema Pivot — Feb 2026

> **This is the current authoritative schema document.** Older versions of
> `SCHEMA_FIX_NOTES.md` and the original `ElleAnn_Schema.sql` declared
> PascalCase tables (`Conversations`, `Messages`, `Memories`, `Goals`, …)
> that the operational C++ code never wrote to. The result: operators ran
> the schema, the code wrote into a different (lowercase) shape, and most
> tables stayed empty. That is now fixed.

---

## Root cause (concrete)

`ElleAnn_Schema.sql` said:

```sql
CREATE TABLE dbo.Conversations (ConversationId INT, UserId INT, Title NVARCHAR …)
CREATE TABLE dbo.Messages      (MessageId BIGINT, ConversationId INT, Role NVARCHAR …)
CREATE TABLE dbo.Memories      (MemoryId BIGINT, MemoryType NVARCHAR, Content …)
CREATE TABLE dbo.Goals         (GoalId BIGINT, Description NVARCHAR …)
```

But `Shared/ElleDB_*.cpp` wrote:

```sql
INSERT INTO ElleCore.dbo.conversations (user_id, title, started_at, …)
INSERT INTO ElleCore.dbo.messages      (conversation_id, user_id, role, content, …)
INSERT INTO ElleCore.dbo.memory        (memory_type, tier, content, importance, …)
INSERT INTO ElleCore.dbo.goals         (description, status, priority, …)
```

Different table names, different column names, different types. The code
self-creates some of these at runtime (see `EnsureGoalsTable` in
`ElleDB_Domain.cpp`) which is why Elle worked at all — but only on logins
that had `CREATE TABLE` rights, and only after the first miss. Plus a
bunch of inter-table FKs that never resolved because the referenced
PascalCase tables had a different shape than expected.

Plus: there was a `dbo.Users` table in ElleCore. **There should not be.**
The game's `Account.dbo.tUser` is the only source of truth for user
identity. ElleCore side tables key on `nUserNo INT` (the value of
`Account.dbo.tUser.nUserNo`) without an FK — cross-DB FKs are brittle and
a delete in `Account` should never cascade into Elle's lived memory.

## The fix (three files)

### 1. `SQL/ElleAnn_SchemaSync_FebPivot.sql`  (corrective delta)

Run **first** on any existing install. Drops the legacy PascalCase shells
*if empty*, drops `dbo.Users` *if empty*, surgically removes FKs to those
shells. Idempotent — safe to re-run.

### 2. `SQL/ElleAnn_Schema.sql`  (canonical, rewritten)

Re-run after the delta. Now declares the actual lowercase tables the code
writes to (`conversations`, `messages`, `memory`, `goals`, `voice_calls`,
`user_avatars`, `action_queue`, `emotion_snapshots`, …) plus the few
PascalCase tables that genuinely match code (`PairedDevices`,
`IntentQueue`, `ElleThreads`, `CrystalProfile`, `UserPresence`).

### 3. Code  (already shipped this pivot)

- `HTTPServer.cpp` gains `ResolveAuthenticatedUser` — JWT → device → `nUserNo`.
- `RequireAuthOrBodyUser` — JWT-first, falls back to body for admin tools.
- New `GET /api/me` — "who am I" endpoint, hits `Account.dbo.tUser` for
  the username so the dev panel never needs to look the username up
  itself.
- `/api/tokens/conversations`, `/api/tokens/video-calls`, and the avatar
  reads now JWT-bind by default; body `user_id` is honored only when
  there's no JWT-bound device on the request (admin/CLI path).

## Run order

**Fresh install:**

```
1. ElleAnn_Schema.sql
2. ElleAnn_System_Schema.sql           ← ElleSystem.dbo.Workers/Logs/...
3. ElleAnn_PairedDevicesDelta.sql
4. ElleAnn_QueueReaperDelta.sql
5. ElleAnn_MemoryDelta.sql
6. ElleAnn_GameUnification.sql
7. ElleAnn_Identity_Schema.sql
8. ElleAnn_XChromosome_Schema.sql
```

**Upgrade from older install (anything before Feb 2026):**

```
1. ElleAnn_SchemaSync_FebPivot.sql      ← THIS FIRST
2. ElleAnn_Schema.sql
3. ElleAnn_System_Schema.sql            ← ElleSystem.dbo.Workers/Logs/...
4. ElleAnn_PairedDevicesDelta.sql
5. ElleAnn_QueueReaperDelta.sql
6. ElleAnn_MemoryDelta.sql
7. ElleAnn_GameUnification.sql
8. ElleAnn_Identity_Schema.sql
9. ElleAnn_XChromosome_Schema.sql
```

If `ElleAnn_SchemaSync_FebPivot.sql` PRINTs warnings about non-empty
legacy tables, copy any rows you care about into the canonical lowercase
tables manually, then drop the legacy table. (In practice these have
always been empty.)

## What this pivot does NOT do

- **Doesn't rename `int32_t user_id` parameters in C++ to `nUserNo`.**
  That's a much wider refactor with no functional benefit — the value
  stored is already `nUserNo`. The next pass can add a `using nUserNo_t
  = int32_t;` alias if naming-clarity is wanted.
- **Doesn't migrate row data from legacy → canonical tables automatically.**
  Operators on a polluted DB do that step manually. The schema sync
  refuses to drop a non-empty legacy table specifically so we never
  silently lose data.
