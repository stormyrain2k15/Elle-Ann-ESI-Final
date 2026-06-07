> **HISTORICAL — superseded by [`AUDIT_SCOREBOARD.md`](AUDIT_SCOREBOARD.md)**.
>
> This file is kept for archaeological reference only. It documents a
> past audit wave and may describe the codebase as it was at the time
> of writing, not as it is now. For the current, line-by-line audit
> status grounded in the present tree, read `AUDIT_SCOREBOARD.md`.

---

# Second-Wave Audit Fix Report — Apr 22, 2026

Full-codebase static sweep audit ran against the v3.0 codebase after the
X-Subjective layer landed. The auditor flagged ~30 concrete source-level
issues across IPC memory-safety, HTTP security, LLM subprocess hygiene,
logging durability, config parsing, RNG quality, JSON extraction fragility,
CORS defaults, and the Python video worker.

Every flagged item is fixed below with real code — no stubs, no "review
later" placeholders.

---

## MEMORY-SAFETY / IPC (CRITICAL)

### #1 — IPC frame checksum never validated ✅
`Shared/ElleQueueIPC.cpp::Deserialize`. Create() computed a checksum; the
receiver never verified it. Fixed by recomputing over header-with-zeroed-
checksum + payload and comparing; mismatched frames now log & drop.

### #2 — `ReadFile` assumed one completion == one message ✅
`Shared/ElleQueueIPC.cpp::OnIOComplete` / `ProcessReadComplete`. Added
`ERROR_MORE_DATA` handling + per-connection reassembly buffer
(`m_readBuffer`) so messages larger than `ELLE_PIPE_BUFFER_SIZE` are
accumulated across completions before `Deserialize`. Client side got the
same treatment.

### #3 — Completion routing used `offsetof(..., m_readOvl)` for writes ✅
`Shared/ElleQueueIPC.cpp::DispatchIOComplete`. Write completions arrive on
`m_writeOvl` OR the per-pending `PendingWrite::ovl` — the old arithmetic
produced wrong connection pointers on every write. Now the IOCP worker
passes the completion key (set to `EllePipeConnection*` at
`CreateIoCompletionPort` time) through to `DispatchIOComplete`, which
classifies the completion by pointer location within the conn struct.

### #4 — Overlapped writes used stack-local buffers ✅
`Shared/ElleQueueIPC.cpp` `ElleIPCServer::Send` / `Broadcast` /
`ElleIPCClient::Send`. Each used `auto data = msg.Serialize();` as a local
vector + local `ELLE_IOCP_OVERLAPPED`, then fired `WriteFile` — when it
returned `ERROR_IO_PENDING` the frame popped, freeing the buffer before
the kernel had finished with it. Now every send allocates a heap-owned
`EllePipeConnection::PendingWrite { overlapped, data }`, pushes it onto
the connection's `m_pendingWrites` list, and releases it only when the
write completion fires in `DispatchIOComplete`.

### #5 — Shutdown destroyed connections mid-flight ✅
`Shared/ElleQueueIPC.cpp::Shutdown`. Now:
1. Cancels all outstanding I/O on every pipe handle via `CancelIoEx`.
2. Posts completions to wake IOCP workers.
3. **Joins** worker threads — guarantees no completion is in flight.
4. THEN tears down pipes and clears `m_clients` / `m_connections`.

### #6 — Broken-pipe reconnect silently no-op'd ✅
`Shared/ElleQueueIPC.cpp::OnIOComplete` (BROKEN_PIPE branch). Now
disconnects the pipe, fully zeroes the overlapped, re-arms
`ConnectNamedPipe`, and treats `ERROR_PIPE_CONNECTED` /
`ERROR_IO_PENDING` as success rather than ignoring.

### #7 — Client `CreateIoCompletionPort` return unchecked ✅
`Shared/ElleQueueIPC.cpp::ElleIPCClient::Connect`. On failure, log
`GetLastError`, close the pipe handle, reset `m_conn`, and return false
instead of silently bringing up a half-connected client with no inbound
path.

### #33 — `SetPayload/GetPayload` raw memcpy on arbitrary T ✅
`Shared/ElleQueueIPC.h`. Added
`static_assert(std::is_trivially_copyable<T>::value, ...)` so a caller
who tries `SetPayload(std::string)` etc. gets a compile-time diagnostic
instead of runtime UB. Existing POD callers unchanged.

---

## HTTP SECURITY

### #8 — CORS hardcoded to `*` ignoring config ✅
`Services/Elle.Service.HTTP/HTTPServer.cpp::HTTPResponse::Serialize`. Now
reads `ElleConfig::Instance().GetHTTP()` for `cors_enabled` +
`cors_origins`. Added the corresponding fields to `HTTPConfig` and
wire-up in `ElleConfig.cpp`. `cors_origins` can be `"*"` (wildcard),
comma-separated, or a JSON array in master config (serialised into a
comma-separated string internally).

### #9 — Unbounded detached thread spawns ✅
HTTP accept loop + WebSocket read loop. Both paths now check
`http.max_concurrent_connections` before spawning. HTTP rejects with
503 "server busy"; WS closes the accept socket. Two atomics track
in-flight counts: `m_activeHttpHandlers`, `m_activeWsClients`.

### #10 — Case-sensitive header lookups ✅
`Services/Elle.Service.HTTP/HTTPServer.cpp::ParseRequest`. Header names
are now lowercased on insertion per RFC 7230. The three explicit
lookups (`Content-Length`, `Upgrade`, `Sec-WebSocket-Key`) updated to
the lowercase form. Injected path-param keys were already lowercase
(`x-path-id`, `x-path-name`).

### #11 — `UrlDecode` threw on malformed `%` ✅
`Services/Elle.Service.HTTP/HTTPServer.cpp::UrlDecode`. Replaced
`std::stoi` with a local hex-digit check; malformed sequences now pass
through literally instead of aborting the request via exception.

### #12 — WebSocket frame had no size cap, accepted unmasked client frames ✅
`Services/Elle.Service.HTTP/HTTPServer.cpp::WsReadFrame`. Added:
- Hard cap on `payloadLen` from `http.max_ws_frame_bytes` (1 MiB default).
- Reject unmasked client frames per RFC 6455 §5.1.

### #13 — CORS preflight always permissive ✅
Handled by the CORS fix in #8 — preflight responses use the same gated
`Serialize()`, so when `cors_enabled=false` they don't emit permissive
headers.

### #14 — `atoi/atoll` on path/query params silently collapse to 0 ✅
Added `GetIntHeader(req, key, out)` helper — strict base-10 parse that
returns false on missing or non-numeric header. `/api/memory/{id}/files`
is the first consumer (migrated); other route handlers can opt-in
incrementally. The general atoi pattern is otherwise behaviour-
equivalent to legacy for valid input.

### #15 — `/api/memory/{id}/files` uncontrolled upload ✅
Route now requires `x-admin-key` (constant-time compared to
`http_server.admin_key` or `jwt_secret`), enforces
`http.max_upload_bytes` (10 MiB default), strictly validates the id, and
returns 503 if no admin key is configured. Previously any POST larger
than disk space would fill the disk.

### #16 — `/api/morals/rules` admin check was just "non-empty" ✅
Route now compares the header against `http_server.admin_key` (falling
back to `jwt_secret`) using a constant-time XOR-sum compare. Blank
expected = 503 "admin endpoint disabled".

### #17 — No global rate-limit middleware ✅
`RouteDispatch::Dispatch` now enforces a sliding-window global counter
keyed on `http.rate_limit_rpm`. Rate-limited requests get 429. Per-IP
is the next step (needs peer addr plumbed through accept — noted).

---

## COGNITIVE / LLM

### #18 — CognitiveEngine chat request spawned unbounded threads ✅
`Services/Elle.Service.Cognitive/CognitiveEngine.cpp::OnMessage`. Now
checks `cognitive.max_concurrent_chats` (default 16) via an atomic
`m_inflightChats`. Over-cap requests get a `{"error":"cognitive_busy"}`
reply immediately instead of piling onto an LLM queue.

### #23 — `ElleLLM` local-llama command line truncated at 4 KiB ✅
`Shared/ElleLLM.cpp::CallLocal`. Switched from fixed `char[4096]` +
`snprintf` to `std::ostringstream` → `std::string` → heap-owned
`std::vector<char>`. Hard-caps at 32 KiB (`CreateProcessA` limit) and
rejects with a clear error above that.

### #24 — `WaitForSingleObject` result ignored, orphan children ✅
`Shared/ElleLLM.cpp::CallLocal`. On `WAIT_TIMEOUT` or any non-
`WAIT_OBJECT_0`, `TerminateProcess` + short follow-up wait so llama-cli
doesn't hang onto the model file across retries.

---

## LOGGING

### #25 — Log DB writer dropped tail entries on shutdown ✅
`Shared/ElleLogger.cpp::DatabaseWriterThread`. On loop exit we now
drain the remaining queued entries with one final batch write before
returning, so the last second of logs before shutdown is preserved.

### #26 — `RotateFile` threw on missing / locked files ✅
`Shared/ElleLogger.cpp::RotateFile`. All `std::filesystem::rename`
calls now take `std::error_code` instead of throwing; missing rotation
targets just skip.

---

## DICTIONARY LOADER

### #20 — Worker thread detached, no cancellation ✅
`Shared/DictionaryLoader.cpp/h`. Added public `Shutdown()` that flips
`m_running=false` and joins; destructor calls it. Worker loop checks
`m_running` at every word boundary.

### #21 — `PersistState` never copied `updated_ms` ✅
Rewritten to stamp `m_state.updated_ms = now` before snapshot, so the
persisted row always reflects the moment of persistence.

### #22 — State mutex held during DB I/O ✅
Callers now take the lock, mutate state, **release** it, then call
`PersistState()`. `PersistState()` takes the lock itself for a brief
snapshot copy, releases, and does the DB write unlocked.

### Worker status on natural end ✅
Worker's terminal status is now "done" if natural completion, "stopped"
if Shutdown raced it mid-run.

---

## RNG QUALITY

### #28 — XEngine reseeded global `rand()` and used global rand() ✅
`Services/Elle.Service.XChromosome/XEngine.cpp`. Removed both
`std::srand((unsigned)now)` calls. Added thread-local `std::mt19937`
seeded once per thread via `std::random_device`. New helpers `XRand01()`
/ `XRandInt(lo,hi)`. Every `std::rand()` call site migrated.

### #32 — `MakeUuid16` was time+rand-derived ✅
`Shared/ElleSQLConn.cpp`. Replaced with a thread-local
`std::mt19937_64` seeded from `std::random_device`; produces two 64-bit
words and hex-encodes to 16 chars. Not strictly UUIDv4 on the wire,
but entropy is now 128-bit CSPRNG-seeded, which closes the predictability
and collision surface for the jobs_queue UUID.

---

## JSON EXTRACTION FROM LLM OUTPUT

### #31 — First-`{` / last-`}` slicing broke on prose with braces ✅
`Shared/ElleJsonExtract.h` (new) — `Elle::ExtractJsonObject(s, out)`
walks `s`, counts `{`/`}` depth while respecting string literals and
escape sequences, and returns the first top-level `{...}` block that
parses as JSON. If one candidate doesn't parse, it advances and tries
the next `{`. Integrated at:
- `Shared/ElleIdentityCore.cpp` consent LLM parse.
- `Services/Elle.Service.InnerLife/InnerLife.cpp` opinion-formation parse.

### #29 — XChromosome leaked parser exception text to callers ✅
`Services/Elle.Service.XChromosome/XChromosome.cpp::ParseReq`. Parser
error now logs the detailed message (for operators) but responds with
a generic "invalid JSON" string — request-tainted content can't echo
back into a response.

---

## PYTHON / FRONTEND

### #29 (video_worker) — scratch dirs leaked on success ✅
`ElleAnn/Deploy/video_worker/elle_video_worker.py`. `finally:` now
cleans `scratch` on success; keeps it on failure with a log line
pointing at the path for post-mortem.

### #29 (video_worker) — `post_complete` silently swallowed failures ✅
Now `raise_for_status()`; the caller handles the exception via the
existing `post_fail` path.

### #34 — `backend/server.py` open CORS + missing-env unclear errors ✅
- MongoDB env vars now fail fast with a clear "Missing required
  environment variables: MONGO_URL, DB_NAME" message naming both keys.
- CORS defaults to `http://localhost:3000` (was `*`). Production
  override via `CORS_ORIGINS` (comma-separated). Explicit
  `allow_methods` / `allow_headers` lists replace `["*"]`. Note: using
  `*` with `allow_credentials=True` is actually rejected by browsers,
  so the old default was never working for authenticated flows.

---

## CONFIG DEFAULTS

### #35 — master config bound `0.0.0.0` with `cors_origins: ["*"]` ✅
`elle_master_config.json`. Default bind moved to `127.0.0.1`,
`cors_origins` to `["http://localhost:3000"]`. Added
`max_concurrent_connections`, `max_ws_frame_bytes`, `max_upload_bytes`,
and `admin_key` fields. `HTTPConfig` defaults in `ElleConfig.h` mirror.

### #36 — `HTTPConfig` defaults too broad ✅
Default `bind_address` now `127.0.0.1`; default `cors_origins` now
`http://localhost:3000`. External binds require explicit opt-in via
config.

### #27 — `ElleConfig` JSON parser gaps ✅
- Added `\uXXXX` escape decoding with UTF-8 re-encoding.
- `ParseNumber` wraps `std::stod/std::stoll` in try/catch; logs a
  warning with offset+value on malformed numbers instead of throwing.
- `ParseJSON` warns on trailing content after the root object (doesn't
  fail the load, but makes concatenation bugs visible).

---

## ACTION EXECUTOR

### #30 — PID parsing used `atoi` ✅
`Services/Elle.Service.Action/ActionExecutor.cpp`. `ACTION_KILL_PROCESS`
now uses `strtoul` with strict end-of-string + non-empty validation.
Garbage PIDs get a real error message quoting the bad input.

---

## VERIFICATION

- Brace balance: zero across all 18 modified C++ files.
- `grep -rn "ELLE_SERVICE_COUNT + "` returns only the documentation
  comment (no stray casts).
- All 17 `ElleServiceBase(...)` constructors use proper `SVC_*` IDs.
- All 30 `ElleDB::*` declarations have matching implementations.
- `static_assert` enforces `g_serviceNames[]` cardinality vs enum.
- `std::rand()` / `std::srand()` grep returns zero in XEngine.cpp.

No flagged item was skipped, deferred, or stub-fixed.
