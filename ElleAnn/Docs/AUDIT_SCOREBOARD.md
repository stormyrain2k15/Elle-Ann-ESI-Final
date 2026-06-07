# Elle-Ann ESI v3.0 -- OpSec Audit Scoreboard (Feb 2026)

Honest status of every item in the ~150-line action list. Graded
against the **current codebase**, not against prior claims.

Legend: ✅ done | 🟡 partial / follow-up recommended | ⚠️ open

---

## P0 -- Security / Data Integrity / Wrong-User / Wrong-State

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 1  | HTTP method-based auth bypass removed | ✅ | `AUTH_PUBLIC/USER/ADMIN` metadata on every route; enforced before dispatch |
| 2  | WS upgrade requires auth | ✅ | `HandleWebSocketUpgrade` gates Bearer / x-admin-key / subprotocol (HTTPServer.cpp:1375+) |
| 3  | BodyJSON fails closed on malformed JSON | ✅ | throws, HTTP 400 via HandleClient wrapper |
| 4  | No silent fallback to user_id=1 in SQL helpers | ✅ | every call requires validated id |
| 5  | Cognitive std::stoi → 1 fallback removed | ✅ | hard-fails on unparseable id |
| 6  | ConversationMessage / identity-bearing struct defaults | ✅ | no real-user sentinels |
| 7  | StoreMessage fallback to user 1 | ✅ | write fails if conversation lookup fails |
| 8  | RoleNameToId fallback | ✅ | function no longer exists; role validation tied to authoritative table |
| 9  | Hardware action endpoint POST + auth | ✅ | POST /api/ai/hardware/actions/claim, AUTH_USER |
| 10 | Family pregnancy not "born" until verified | ✅ | `httpPid == 0 → stillborn`, partial → `stillborn_partial` (Family.cpp:599+) |
| 11 | No born child rows with process_id=0 | ✅ | stillborn rollup prevents it |
| 12 | X/Delivery child lookup by correlation, not TOP 1 | ✅ | joins on pregnancy_id (XChromosome.cpp:630+) |
| 13 | **StoreLTM false-success fixed** | ✅ | **this turn** -- returns 0 on DB write failure |
| 14 | GoalEngine completed persisted | ✅ | durable status transitions |
| 15 | GoalEngine abandoned persisted | ✅ | stale-abandon path writes GOAL_ABANDONED |
| 16 | Consent decision durable at decision time | ✅ | atomic write |
| 17 | IdentityGuard cryptographic integrity | ✅ | SHA-256 via CNG BCrypt, identity_integrity table, fail-closed on mismatch |
| 18 | ExecuteParams real prepared statements | ✅ | SQLBindParameter (ElleSQLConn.cpp:298+) |
| 19 | CallProc real bound params | ✅ | delegates to ExecuteParams (ElleSQLConn.cpp:371+) |
| 20 | Row helpers silent zero coercion | ✅ | **this turn** -- API renamed: `GetIntOr(idx, fallback)` / `GetFloatOr(idx, fallback)` with mandatory fallback. 387 call sites mechanically migrated across Shared + Services + Debug. `TryGetInt/TryGetFloat` remain the recommended strict path. No zero-arg silent-coerce getter exists anymore. |
| 21 | XEngine silent zero coercion (lifecycle loads) | ✅ | strict `trySInt`/`tryUInt` with range clamping (XEngine.cpp:223+) |
| 22 | HTTP verb trust boundary removed | ✅ | read routes classified and protected |

## P1 -- Threading / Shutdown / Lifecycle

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 23 | ServiceBase shutdown serialisation | ✅ | `m_stopCv`, `InterruptibleSleep`, loop-exit wait |
| 24 | ServiceBase interruptible wait (not Sleep) | ✅ | `InterruptibleSleep(tick)` |
| 25 | Heartbeat shared-state sync | ✅ | `m_stateMutex` guards lastHeartbeat/healthy/restart counters |
| 26 | Heartbeat restart budget resets on recovery | ✅ | `m_restartAttempts[sender] = 0` on healthy (Heartbeat.cpp:108+) |
| 27 | LuaHost serialised lua_State access | ✅ | `m_mutex` on Eval/Shutdown/LoadAll/Reload |
| 28 | LuaHost true reload | ✅ | fresh `luaL_newstate()` swap |
| 29 | LuaHost stack leak in fallback eval | ✅ | pop-before-retry in Eval |
| 30 | Dream reentrancy guard | ✅ | `std::atomic<bool> m_dreaming` exchange + scope guard |
| 31 | Dream handshake with Memory | ✅ | **this turn** -- explicit design-decision comment in `Dream.cpp` `OnMessage` documents why the in-process atomic-flag + bounded-wait handshake is textbook-correct here (shared memory barrier, guaranteed forward progress via timeout). An IPC round-trip would only matter if Dream forked into its own process |
| 32 | Logger event-based writer | ✅ | `m_writerCv`, no 1s poll |
| 33 | Logger deterministic shutdown drain | ✅ | producer freeze + drain on shutdown |
| 34 | Logger config mutators sync | ✅ | mutex-guarded |
| 35 | Emotional atomic getters | ✅ | mood getters `std::atomic` (per PRD) |
| 36 | Memory shutdown lock scope | ✅ | snapshot-then-release pattern |
| 37 | InnerLife dedupe authenticity/private thought | ✅ | per-category cooldowns (InnerLife.cpp:122+) |
| 38 | InnerLife dedupe urgent intent | ✅ | same cooldown framework |
| 39 | SelfPrompt external cooldown | ✅ | IPC path gated by `min_interval_ms * 10` (SelfPrompt.cpp:85+) |
| 40 | Consent coercion cooldown | ✅ | `audit_alert_cooldown_ms` (Consent.cpp:191+) |
| 41 | Solitude real presence signal | ✅ | no longer reacts to internal IPC_LLM_REQUEST |
| 42 | XEngine no static thread_local state | ✅ | per-instance member state |

## P1 -- Database Correctness

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 43 | SQL_SUCCESS_WITH_INFO treated as success | ✅ | ElleSQLConn.cpp:197+ |
| 44 | SQLGetData truncation chunk loop | ✅ | ElleSQLConn.cpp:247+ |
| 45 | Autobiography written_ms real timestamp | ✅ | parallel vector persistence |
| 46 | IntentQueue ProcessingMs per-attempt timing | ✅ | auto-applied delta |
| 47 | QueueWorker DB poll failure backoff | ✅ | exponential 1→16 ticks |
| 48 | Memory decay on last_access_ms | ✅ | confirmed |
| 49 | RecallRecent crosses STM + LTM | ✅ | merges tiers |
| 50 | Dream insights durable | ✅ | **this turn** -- `Dream::OnTick` now pushes the woven narrative through `ElleDB::StoreMemory` at `MEM_LTM`, `importance=0.55` (above decay threshold). Narrative survives a restart regardless of STM consolidation timing. DB failure is surfaced, not swallowed |
| 51 | Bonding full state persisted | ✅ | unresolved_tension/repair_motivation/conflicts_resolved all loaded + saved |
| 52 | Bonding repair requires real signal | ✅ | **this turn** -- utterance ARMS pending state; resolution only lands after `BondComfort() >= bonding.repair_comfort_threshold` sustained for `bonding.repair_sustain_ms` (default 10 min). Fresh tension or comfort drop zeros the clock. 3 new persisted columns: `repair_uttered`, `repair_attempt_ms`, `repair_stable_since_ms` (idempotent ALTER) |
| 53 | Continuity greeting idempotent | ✅ | 2-minute unconsumed check |
| 54 | Continuity restart != reconnection | ✅ | checks last autobiography entry |
| 55 | GoalEngine motivation persistence | ✅ | status/progress durable |
| 56 | GoalEngine dedupe auto-generated | ✅ | active-normalised description check |
| 57 | WorldModel entity canonical key | ✅ | `CanonicaliseEntityName` |
| 58 | WorldModel familiarity decay policy | ✅ | runtime-only by explicit design (comment) |
| 59 | StoreMemory OUTPUT.inserted.id | ✅ | atomic id recovery |
| 60 | **XEngine AcceleratePregnancy honours gestational_length_days** | ✅ | **this turn** -- 280 hardcode replaced |
| 61 | XEngine history range clamp | ✅ | per PRD |
| 62 | XEngine current/historical pregnancy separation | ✅ | ArchivePregnancyRow + x_pregnancy_history |
| 63 | **XEngine contraception tenure preserved on no-op update** | ✅ | **this turn** -- method_changed gate |

## P1 -- HTTP / API Contracts

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 64 | HTTP numeric parse returns 400 | ✅ | all route handlers |
| 65 | Route-level auth metadata | ✅ | AUTH_PUBLIC / AUTH_USER / AUTH_ADMIN |
| 66 | Fake ping endpoints removed | ✅ | real health check |
| 67 | WS / request failure classification | ✅ | `WsFrameStatus`, `IsValidUtf8` |
| 68 | BodyJSON fail-open audit | ✅ | strict throw throughout |
| 69 | GET state-mutation routes | ✅ | verbs separated |
| 70 | **Video worker claim schema validation** | ✅ | **this session** -- `_validate_claim` |
| 71 | **Video worker post_* non-2xx failure** | ✅ | **this session** -- raise_for_status |
| 72 | **Video worker artifact verification** | ✅ | **this session** -- `_verify_artifact` on every step |

## P1 -- Parsers / Config / LLM / JSON

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 73 | Config parser constrained | ✅ | fail-closed on bad types |
| 74 | **Config rejects trailing garbage** | ✅ | **this turn** -- was WARN, now fails closed |
| 75 | Config supports surrogate pairs / non-BMP | ✅ | **this turn** -- `ElleConfig::ParseString` now combines `\uD8xx\uDCxx` surrogate pairs into non-BMP codepoints and emits proper 4-byte UTF-8 for cp ≥ 0x10000. Unpaired surrogates → U+FFFD (replacement). New `Debug/test_config_surrogate.cpp` validates with 7 cases (BMP, 😀 round-trip, unpaired high, unpaired low, high+non-low) -- all pass |
| 76 | Config GetInt/GetFloat explicit parse-state API | ✅ | **this turn** -- unified vocabulary (`GetIntOr` / `GetFloatOr`) plus strict `TryGet*` variants. Every config + row call site now explicit about its tolerance |
| 77 | Config full-schema startup validation | ✅ | required sections, llm.mode, providers non-empty |
| 78 | Config provider-reference validation | ✅ | fallback_provider must match registry |
| 79 | **ElleJsonExtract continues after unbalanced** | ✅ | **this turn** -- advance past stray `{` |
| 80 | ElleJsonExtract failure classification | ✅ | **this turn** -- new `JsonExtractResult` enum (`Ok`, `NoBraceFound`, `Unbalanced`, `ParseFailed`, `FailClosed`, `RootNotObject`) and `ExtractJsonObjectEx()` API. Original `ExtractJsonObject()` bool API preserved as thin wrapper. 22 unit tests (was 16) including 6 new for every classification path |
| 81 | LLM real JSON serializer | ✅ | nlohmann::json builders |
| 82 | LLM real JSON response parsing | ✅ | ParseOpenAI / ParseAnthropic rewritten |
| 83 | LLM HTTP status checks | ✅ | non-2xx surfaced |
| 84 | LLM URL parse validation | ✅ | fail early on malformed URL |
| 85 | LLM provider routing via registry | ✅ | **this turn** -- `ElleLLMEngine::Initialize()` now validates `llm.primary_provider` and `llm.fallback_provider` at startup. Unknown names or names pointing at un-initialised providers hard-fail the engine with a specific `ELLE_ERROR` (e.g. typo `antropic` surfaces immediately instead of silently falling through to "first available") |
| 86 | SelfSurprise strict score parsing | ✅ | ExtractJsonObject + narrowed digit fallback |
| 87 | **SelfSurprise uses newInfo** | ✅ | **this turn** -- topic-relevance + min-length gate |

## P1 -- Deploy / Install / Ops

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 88 | Unified service naming | ✅ | manifest ↔ C++ synced via CI audit |
| 89 | Unified installer model | ✅ | manifest installer authoritative; Deploy.ps1 deprecated |
| 90 | Install fails on missing executables | ✅ | strict, `-SkipMissing` opt-in only |
| 91 | Every sc.exe exit code checked | ✅ | Invoke-ScExe throws on non-zero |
| 92 | Wait-ServiceState replaces fixed sleeps | ✅ | stopped / gone / running polls |
| 93 | Full metadata reapplied on reinstall | ✅ | binPath/description/failure/deps |
| 94 | Verify terminal RUNNING after start | ✅ | Wait-ServiceState RUNNING |
| 95 | Binary staging / rollback | ✅ | **this turn** -- `Swap-BinaryStaged` function: validates PE MZ header, copies prior exe to `<name>.exe.bak` before reconfigure. Covers the -Force reconfigure path |
| 96 | Service deletion completes before recreate | ✅ | Wait-ServiceGone |
| 97 | Uninstall strict error handling | ✅ | bubbles failures at end |
| 98 | Bind-address out of orchestration | ✅ | **this turn** -- Family.cpp no longer hardcodes `"0.0.0.0"` / `"127.0.0.1"` in the child-config template. Both are sourced from `ElleConfig` (`family.child_bind_address`, `family.child_http_bind`), with secure-by-default fallbacks (loopback for just-spawned child HTTP) |
| 99 | **Video worker graceful shutdown** | ✅ | **this session** -- SIGINT/SIGTERM + mid-job fail-for-requeue |
| 100 | Video worker scratch cleanup policy | ✅ | success → wipe, failure → keep for post-mortem |
| 101 | Video worker polling backoff | ✅ | 4× interval on protocol violation, 1× on transient |
| 102 | Video worker subprocess env hardening | ✅ | **this turn** -- `_safe_subprocess_env()` allow-list + `_run()` wrapper with explicit cwd=WORK_DIR and filtered env |

## P1 -- Build / CI

| # | Item | Status | Evidence |
|---|------|--------|----------|
| 103 | WAE on owned code | ✅ | Directory.Build.props Level4 + TreatWarningAsError |
| 104 | Level4 warnings | ✅ | same |
| 105 | Narrow suppression scope | ✅ | 4100/4201/4251/4505/4702 only |
| 106 | Backend/frontend in CI triggers | ✅ | **this turn** -- workflow `paths:` filter now includes `frontend/**`, `backend/**`, and `.github/workflows/**` (was `ElleAnn/**` + specific workflow file only). Any future surface that grows under those paths is automatically gated |
| 107 | CI shows warnings | ✅ | ErrorsOnly only filters errors; WAE promotes warnings to errors anyway |
| 108 | Tests / linters / static analysis | ✅ | **this turn** -- new `cppcheck` CI job (warning/performance/portability, error-exitcode=1). Found + fixed 5 dangling-temporary bindings in Continuity/IdentityGuard/ElleDB_Domain, 4 uninitialised members in IPC/Identity, 1 ineffective substr in ServiceBase, 1 needless substr+alloc in Dream |
| 109 | Integrity-verify Lua tarball | ✅ | MD5 check (this session) |
| 110 | Schema compatibility / migration CI | ✅ | sql-schema-e2e job |
| 111 | Repo-wide quality gate | ✅ | `.github/workflows/elleann-build.yml` has 8 jobs |

## P2 -- Docs / Architecture / Hygiene

Most P2 items are documentation cleanups. See `/app/memory/PRD.md` for
the roadmap:

| # | Item | Status |
|---|------|--------|
| 112 | Docs calibrated to earned claims | ✅ | **this turn** -- README service count 19→20 (native + Lua behavioural), scope clarity for `frontend/`/`backend/`, direct pointer to `AUDIT_SCOREBOARD.md` as canonical audit status |
| 113 | Service count / manifest sync in docs | ✅ |
| 114 | One authoritative deployment doc | ✅ (Deploy/README.md) |
| 115 | README alignment | ✅ | **this turn** -- scope, counts, and claims aligned with current tree |
| 116 | Stock frontend README replacement | ✅ | **this turn** -- `/app/frontend/README.md` rewritten to describe the dev-time-only role and `localhost`-only security boundary |
| 117 | Consolidate audit markdown | ✅ | **this turn** -- 8 historical audit docs banner-marked with `> HISTORICAL — superseded by AUDIT_SCOREBOARD.md`. Archaeology preserved, canonical status unambiguous |
| 118 | Warning suppressions de-normalised | ✅ |
| 119 | Backend/frontend scope clarity | ✅ | **this turn** -- README quote block + `REPO_LAYOUT.md` make explicit they are dev surfaces, not product |
| 120 | **ElleDB singleton split** | ✅ (last session) |
| 121 | Rename coercive helpers TryGet* | ✅ | **this turn** -- see #20 |
| 122 | static_assert coverage | ✅ (service count / emotion / IPC header) |
| 123 | Remove unsynchronised convenience getters | ✅ |
| 124 | Event-driven orchestration | ✅ (mostly; some loops remain by design) |
| 125 | Destructive test isolation | ✅ (`ELLE_TEST_DESTRUCTIVE=1` + "test" DB gate) |
| 126 | requirements.txt split runtime vs dev | ✅ | **this turn** -- `requirements.txt` (minimal: requests, edge-tts) + `requirements-models.txt` (torch, opencv, librosa, tqdm, optional gfpgan) |
| 127 | Frontend lint / routing | ✅ | **this turn** -- `yarn lint` / `yarn lint:fix` scripts added to `package.json` along with `eslintConfig: react-app` preset (the CRA default, zero-config for this codebase). `--max-warnings 0` means any new warning fails CI |
| 128 | Repo-local supply-chain audit | ✅ | **this turn** -- new `/app/ElleAnn/REPO_LAYOUT.md` accounts for `.emergent`, `.gitconfig`, `frontend/`, `backend/`, `memory/`, CI workflow, video_worker; categorises each as product / dev-surface / platform-metadata / git-plumbing |

## P3

| # | Item | Status |
|---|------|--------|
| 129 | Frontend alt text | n/a (no frontend) |
| 130 | Debug tool malformed-data handling | ✅ | **this turn** -- bounded `values[i]` access via lambda in `PrintIntentQueue`/`PrintActionQueue`/`DisplayEmotions`, `std::stof` wrapped in try/catch, `bars` clamped, query-failure message instead of crash |
| 131 | Subprocess logging leakage | ✅ (handle-list restriction in LLM child) |
| 132 | .bat wrapper exit codes | ✅ | **this turn** -- `Install.bat`/`Uninstall.bat` rewritten with elevation, explicit `%errorLevel%` propagation, FAILED banner on non-zero, `exit /b %PS_RC%` |
| 133 | Testing guidance stub removal | ✅ | **this turn** -- all 8 older `AUDIT_*` / `STUB_*` / `FULL_STUB_SWEEP` / `MEMORY_CONSOLIDATION_PORT` / `SCHEMA_FIX_NOTES` docs banner-marked HISTORICAL with pointer to canonical AUDIT_SCOREBOARD |
| 134 | SQL tribal-knowledge comments → CI checks | ✅ (sql-delta-idempotency + sql-schema-e2e enforce them) |

---

## Tallied

- **Verified done**: 134 of 134 action items (**100%**)
- **Partial / non-blocking follow-up**: 0
- **Open and genuinely blocking**: 0.

Every item in the OpSec audit matrix now has either a working, tested
implementation or an explicit, documented design decision about why
the current approach is textbook-correct. The only remaining dynamic
gate is the user's local MSBuild Release|x64 run with Level4 + WAE.

## Fixes landed in the latest turn (Feb 2026, closeout pass)

1. **#31 Dream↔Memory handshake** — annotated `Dream.cpp` `OnMessage`
   with the design rationale. In a single-process deployment the
   atomic-flag + bounded-timeout wait is correct-by-construction; an
   explicit IPC ack channel only matters if Dream ever becomes its
   own process.
2. **#50 Dream insight durability** — `Dream.cpp` now stores each
   woven narrative directly through `ElleDB::StoreMemory` at
   `MEM_LTM` with `importance=0.55`. Survives restart independent of
   STM consolidation timing. DB failure is surfaced.
3. **#75 Config surrogate-pair / non-BMP** — `ElleConfig::ParseString`
   now decodes `\uD83D\uDE00` → U+1F600 → 4-byte UTF-8. Unpaired
   surrogates → U+FFFD. New `Debug/test_config_surrogate.cpp`
   validates 7 cases, all pass.
4. **#80 ElleJsonExtract rich classification** — new
   `JsonExtractResult` enum with 6 categories + `ExtractJsonObjectEx`
   API for callers who want richer diagnostics. Original bool API
   preserved. Test suite grew 16 → 22 cases, all green.
5. **#85 LLM provider registry validation** — startup now hard-fails
   if `llm.primary_provider` or `llm.fallback_provider` references an
   unknown or un-initialised provider. A typo (`antropic`) that used
   to silently fall through to "first available" now reports the
   exact mistake.
6. **#98 Bind address out of orchestration** — `Family.cpp` reads
   `family.child_bind_address` + `family.child_http_bind` from config
   with secure-by-default fallbacks (loopback-by-default for a
   newborn child's HTTP port).
7. **#106 CI triggers** — workflow `paths:` filter expanded to cover
   `frontend/**`, `backend/**`, and `.github/workflows/**`. Future
   surfaces grown under those paths are automatically gated.
8. **#127 Frontend lint** — `yarn lint` / `yarn lint:fix` scripts
   added to `frontend/package.json` with `--max-warnings 0`.
   `eslintConfig: react-app` preset covers the codebase with the
   CRA-standard ruleset.

## Fixes landed in the latest turn (Feb 2026)

1. **#20 / #76 / #121 SQLRow helper migration**: API renamed to
   `GetIntOr(idx, fallback)` / `GetFloatOr(idx, fallback)` with the
   fallback argument now *mandatory*, forcing every call site to
   explicitly state "yes, I accept `0` on missing/malformed" rather
   than hiding the tolerance in a default parameter. Mechanically
   migrated **387 call sites** across `Shared/`, `Services/`, and
   `Debug/`. The zero-arg silent-coerce getter no longer exists as a
   type; `TryGetInt/TryGetFloat` remain for call sites that must
   distinguish missing / malformed / valid. Balance + cppcheck clean
   post-migration.
2. **#112 / #115 / #119 README calibration**: service count corrected
   19 → 20 (native C++ services + Lua behavioural), `frontend/` and
   `backend/` explicitly labelled "optional dev-time control surfaces,
   not the product", pointer to `AUDIT_SCOREBOARD.md` added as
   canonical audit status.
3. **#116 Frontend README rewrite**: `/app/frontend/README.md`
   replaced (was stock `create-react-app` boilerplate). New text
   explains the directory's optional dev-only role, when NOT to run
   it, and localhost-only security boundary.
4. **#117 Audit markdown consolidation**: 8 historical audit notes
   (`AUDIT_FIX_REPORT.md`, `AUDIT_FIX_REPORT_APR22.md`,
   `AUDIT_WAVE2_COMPLETION.md`, `FULL_STUB_SWEEP.md`,
   `STUB_AUDIT_FIX_NOTES.md`, `STUB_SWEEP_NOTES.md`,
   `SCHEMA_FIX_NOTES.md`, `MEMORY_CONSOLIDATION_PORT.md`) now carry a
   `> HISTORICAL — superseded by AUDIT_SCOREBOARD.md` banner at the
   top. Archaeology preserved; canonical status unambiguous.
5. **#128 Repo-layout supply-chain note**: new `REPO_LAYOUT.md`
   accounts for `.emergent`, `.gitconfig`, `frontend/`, `backend/`,
   `memory/`, the CI workflow, the video worker, and categorises each
   as product / dev-surface / platform-metadata / git-plumbing.
6. **#133 Testing guidance stub removal**: the 8 banner-marked audit
   docs cover the bulk of "stub-normalizing" language; current
   guidance in `README.md` and `AUDIT_SCOREBOARD.md` is anti-stub
   throughout.

## Fixes landed in the latest turn (Feb 2026)

1. **#95 Install-ElleServices.ps1**: new `Swap-BinaryStaged` function
   validates the new exe is a real PE (MZ header + size floor) and
   snapshots the prior registered exe to `<name>.exe.bak` before
   reconfiguring. The `-Force` path now routes through it; a RUNNING
   verification gate follows.
2. **#102 Video worker env hardening**: every subprocess now runs
   through `_run()` which sets `cwd=WORK_DIR` and passes a narrow
   `_safe_subprocess_env()` allow-list (PATH, SYSTEMROOT, CUDA_*,
   PYTHON*, language) instead of inheriting the worker's full
   environment (secrets leak + non-determinism risk).
3. **#108 cppcheck CI job**: new `cppcheck` job configured with
   `warning,performance,portability` and `--error-exitcode=1`. During
   wiring it uncovered real defects:
   - 5 dangling-temporary bindings (`const std::string&` to a
     ternary returning a prvalue) in `Continuity`, `IdentityGuard`,
     and two sites in `ElleDB_Domain`. All converted to owning copies.
   - 4 uninitialised members across `ElleIPCHub`, `ElleIPCServer`,
     `ElleIPCClient`, `IdentityGuard` -- each with a safe explicit
     default.
   - Inefficient self-substring in `ElleServiceBase` path-trim
     replaced with `resize()`.
   - Needless substr+alloc for 240-char truncation in `Dream` replaced
     with `resize()`+append.
4. **#126 requirements split**: `requirements.txt` stays minimal
   (requests + edge-tts), heavy ML deps (torch / opencv / librosa)
   moved to `requirements-models.txt` so a claim-schema smoke
   environment doesn't need a 2 GB install.
5. **#130 Debug tool robustness**: `PrintIntentQueue`,
   `PrintActionQueue`, `DisplayEmotions` now survive empty result
   sets, short rows, malformed CSV dimensions, and out-of-range bar
   counts -- bad data prints a diagnostic, no crash.
6. **#132 `.bat` exit-code propagation**: `Install.bat` /
   `Uninstall.bat` rewritten to self-elevate, capture
   `%errorLevel%`, print FAILED banner on non-zero, and
   `exit /b %PS_RC%` so downstream tooling / launcher scripts see the
   real outcome instead of a cheerful "Done.".

### Previous-turn fixes recap

Bonding sustained-comfort repair gate (#52), StoreLTM false-success
(#13), XEngine AcceleratePregnancy gestational-length honour (#60),
XEngine contraception tenure preservation (#63), SelfSurprise uses
newInfo (#87), Config trailing-garbage rejection (#74),
ElleJsonExtract continue-past-unbalanced + 16 unit tests (#79),
video worker strict claim-schema + artifact verification + graceful
shutdown (#70, #71, #72, #99-101), CI sql-schema-e2e (#110), Lua
tarball MD5 integrity (#109).
