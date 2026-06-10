# HTTP god-file split — MSVC verification checklist

> Status: Phase A + B + C have all landed in the Linux container.
> The 19-file split is structurally sound (brace totals 1 902 / 1 902,
> 158 + 2 routes accounted for, NUDE CODE preserved). The only step
> that **cannot run inside the Linux container** is the actual MSVC
> compile + start-up smoke. Follow this checklist on Windows.

## 0. Pre-flight

- All 19 `HTTPServer*.cpp` files and `HTTPServer.h` are present in
  `Services/Elle.Service.HTTP/`.
- `Elle.Service.HTTP.vcxproj` lists all 19 `<ClCompile>` entries +
  `<ClInclude Include="HTTPServer.h" />`.
- Apply `SQL/_Shared/01_sql_fallback_poison.sql` against `ElleCore`
  before exercising the two new poison admin routes
  (`/api/admin/sqlfallback/poison`,
  `/api/admin/sqlfallback/poison/load`).

## 1. Build

```cmd
:: From repo root
msbuild ElleAnn.sln /p:Configuration=Debug /p:Platform=x64 /m
```

**Expected:** clean compile. Likely issue classes if MSVC complains:

| Symptom | Most likely cause | Fix |
|---|---|---|
| `LNK2005 ...g_diagMx already defined` | A global is still tagged `static` somewhere in `HTTPServer.h` | grep `HTTPServer.h` for `^static`; promote to `inline` |
| `error C2143: syntax error` in one of the route .cpp files | A method body in the corresponding `RegisterXxxRoutes` chunk had an unclosed brace before the split | The Linux brace audit ran clean — most likely an MSVC-only macro (e.g. `#pragma warning`) crossed a split boundary. Search the chunk for unmatched `#pragma`/`#if`. |
| `error C2065: 'XxxResponse': undeclared identifier` in a split file | The type was left in `HTTPServer.cpp` instead of `HTTPServer.h` | grep `HTTPServer.cpp` for `^struct\|^class`; move missing type into `HTTPServer.h` |
| `error C4150: deletion of pointer to incomplete type` in any file | A member uses an incomplete forward-declared type by value | All file-private types (`HTTPRequest`, `HTTPResponse`, `RouteDispatch`, `WSClient`, `PendingChat`, `ChatCorrelator`) are already in the header — should not trigger |

## 2. Service-start smoke

```cmd
sc start Elle.Service.HTTP
```

Then `Get-EventLog -LogName Application -Source Elle.Service.HTTP -Newest 20` or tail `logs/elle_http.log`. Expected log lines (both must appear):

```
INFO  Registered 160 API routes
INFO  SQL fallback poison reaper: enabled, interval=300s
```

(`158` original routes + the 2 new poison admin routes. The reaper line confirms Phase 3 wiring — to disable set `http_server.sqlfallback_poison_load_interval_secs = 0`.)

## 3. Route smoke — at least one per registrar

Each curl below targets one route from each of the 18 + new
`sqlfallback/*` registrars. Status `200 OK` (or `401 Unauthorized`
for the AUTH-gated routes — which still proves the route exists and
the dispatcher reached it) is success.

| Registrar | Smoke route |
|---|---|
| Intro                | `curl -s http://localhost:8082/api/health` |
| Auth                 | `curl -s http://localhost:8082/api/auth/qr` |
| Diag                 | `curl -s http://localhost:8082/api/diag/queues` |
| Admin                | `curl -s -H "x-auth-admin: 1" http://localhost:8082/api/admin/lexical/incomplete?limit=5` |
| Memory               | `curl -s "http://localhost:8082/api/memory/why?limit=5"` |
| Emotion              | `curl -s http://localhost:8082/api/emotions` |
| MeTokens             | `curl -s -X POST -H "Content-Type: application/json" -d '{}' http://localhost:8082/api/me` |
| VideoIdentity        | `curl -s http://localhost:8082/api/identity/active` |
| AI                   | `curl -s http://localhost:8082/api/ai/status` |
| Dictionary           | `curl -s http://localhost:8082/api/dictionary/stats` |
| Education            | `curl -s "http://localhost:8082/api/education/skills?limit=5"` |
| EmotionalContext     | `curl -s http://localhost:8082/api/emotional-context/dimensions` |
| XLifecycle           | `curl -s http://localhost:8082/api/x/state` |
| Server               | `curl -s -H "x-auth-admin: 1" http://localhost:8082/api/server/status` — also assert the new `sql_fallback` block: `\| python3 -c "import sys,json; d=json.load(sys.stdin); print(d['sql_fallback']['reaper'])"` |
| Models               | `curl -s http://localhost:8082/api/models/slots` |
| MoralsGoals          | `curl -s http://localhost:8082/api/morals/rules` |
| Misc                 | `curl -s http://localhost:8082/api/brain/status` |
| SHN                  | `curl -s "http://localhost:8082/api/shn/list?limit=5"` |
| **NEW** SQL-fallback | `curl -s -H "x-auth-admin: 1" "http://localhost:8082/api/admin/sqlfallback/poison?limit=5"` |

(Adjust port to match `http.bind_port` in `elle_master_config.json`.)

## 4. Sign-off

When every smoke returns `200 OK` (or `401` for AUTH-gated, with no
`404`/`500`), flip the row in `Docs/ANTI_SLOP_AUDIT_TRACKING.md`:

```
| 8  | HTTP god-file (6 000 LOC monolith) | 🟢 SPLIT-LANDED | … |
| 15 | SQL access logic duplicated across services | ⏸ | Same as #8 — cosmetic restructuring |
```

→

```
| 8  | HTTP god-file (6 000 LOC monolith) | ✅ CLOSED | … MSVC-verified <date>, 160 routes registered, 19 smokes green |
| 15 | SQL access logic duplicated across services | ✅ CLOSED | Header `HTTPServer.h` now the single source of truth for HTTP-side SQL helpers |
```
