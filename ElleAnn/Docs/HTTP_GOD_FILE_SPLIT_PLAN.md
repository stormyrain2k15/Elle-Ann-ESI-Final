# HTTP god-file split — execution plan (#8 / #15)

## Current state (Feb 2026)

`Services/Elle.Service.HTTP/HTTPServer.cpp` is **6035 lines** and
registers **153 routes** in a single `RegisterRoutes()` method on
`ElleHTTPService`. Every route is a lambda with `[this]` capture and
inlines its own DB access via `ElleSQLPool::Instance()`. The audit
named this twice (broad #8 cosmetic, broad #15 direct SQL scattered).

## Why it can't be split blindly

Every lambda captures `this` and reaches into:
- `m_router`, `m_listenSocket`, `m_acceptThread`, `m_httpWorkers`
- `m_wsClients`, `m_wsBroadcastQueue`
- `m_sessionStore` (in-memory session map)
- ~40 private helper methods (`WsSendText`, `AuthorizeAdmin`,
  `ParseQueryString`, …)
- A growing config snapshot reloaded under `OnConfigReload`

So a naïve copy-paste into separate `.cpp` files breaks linkage
immediately. The class declaration has to be in a header before any
split can happen.

## The split, in three landable phases

### Phase A — extract class declaration into a header

1. Create `Services/Elle.Service.HTTP/HTTPServer.h` containing only
   the `ElleHTTPService` class declaration plus the small structs it
   needs (`HTTPRequest`, `HTTPResponse`, `Router`, `WsClient`,
   `SessionRow`, …). Anything that's currently an anonymous
   namespace-private free function stays in `.cpp`.
2. Original `HTTPServer.cpp` keeps **all** member-function bodies
   verbatim. Only change: add `#include "HTTPServer.h"` at the top
   and remove the inline `class` body, leaving the bodies in place.
3. Add `HTTPServer.h` to the existing `.vcxproj` `<ClInclude>` list.

This is the smallest behaviour-preserving step. **No methods move.**
**No lambdas move.** **No route bodies change.** Verify by compiling
on Windows — output binary identical except for debug symbol order.

### Phase B — group routes into helper methods (same file)

Add five private member methods to `ElleHTTPService`:

```cpp
void RegisterIntroRoutes();      // GET / , GET /healthz, GET /api/health
void RegisterAuthRoutes();       // /api/auth/*  (login, logout, me,
                                  //  devices, sessions, qr 410, pair 410)
void RegisterAdminRoutes();      // /api/admin/*  (logs, reload,
                                  //  config/reload, lexical/* later)
void RegisterDiagRoutes();       // /api/diag/*   (queues, sqlqueue,
                                  //  fiesta, effective-config, routes,
                                  //  wires, heartbeats, health, game-auth)
void RegisterMemoryRoutes();     // /api/memory/* /api/dictionary/* etc.
void RegisterAIRoutes();         // /api/ai/* /api/brain/* /api/emotions
void RegisterEducationRoutes();  // /api/education/*
void RegisterEmotionalContextRoutes();
void RegisterSelfRoutes();       // /api/self/* /api/self-prompts/*
void RegisterUploadRoutes();     // /api/upload/* /api/shn/*
void RegisterEntityRoutes();     // /api/entities/* /api/family/* /api/bond/*
void RegisterDiscoveryRoutes();  // catch-all + /api/recommendations
```

`RegisterRoutes()` becomes:

```cpp
void RegisterRoutes() {
    RegisterIntroRoutes();
    RegisterAuthRoutes();
    RegisterAdminRoutes();
    RegisterDiagRoutes();
    RegisterMemoryRoutes();
    RegisterAIRoutes();
    RegisterEducationRoutes();
    RegisterEmotionalContextRoutes();
    RegisterSelfRoutes();
    RegisterUploadRoutes();
    RegisterEntityRoutes();
    RegisterDiscoveryRoutes();
}
```

Each helper holds the exact route bodies that already exist — pure
mechanical move-into-method, no behaviour change. `git diff` shows
brace migration only.

### Phase C — move helpers into per-file translation units

Once Phase B compiles, each helper method's body moves into its own
`.cpp` file:

```
HTTPServer.cpp            // RegisterRoutes() + ctor/dtor + AcceptLoop +
                          //   HttpWorkerLoop + WebSocket plumbing
HTTPServer_AuthRoutes.cpp
HTTPServer_AdminRoutes.cpp
HTTPServer_DiagRoutes.cpp
HTTPServer_MemoryRoutes.cpp
HTTPServer_AIRoutes.cpp
HTTPServer_EducationRoutes.cpp
HTTPServer_EmotionalContextRoutes.cpp
HTTPServer_SelfRoutes.cpp
HTTPServer_UploadRoutes.cpp
HTTPServer_EntityRoutes.cpp
HTTPServer_DiscoveryRoutes.cpp
```

Each new `.cpp` consists of `#include "HTTPServer.h"` plus
`void ElleHTTPService::RegisterXxxRoutes() { ... }`. The `.vcxproj`
gains 11 `<ClCompile>` entries.

At this point the SQL scattering (#15) is naturally addressed: each
helper can call into a domain-specific data-access namespace
(`ElleAuthDB::`, `ElleMemoryDB::`, …) instead of inlining
`ElleSQLPool::Instance().Query(...)`. That's an optional Phase D.

## Verification gate per phase

| Phase | Pass criterion |
|---|---|
| A | All existing routes return the same status/body; Windows build green; HTTP smoke (login + GET /api/auth/me) passes |
| B | Same as A — single-file move is pure refactor |
| C | Same as A — link order matters; add each new file under `_Shared` deps in the `.vcxproj` |

## Why this isn't done in one go

Phase A alone is roughly 200 lines of class declaration extraction
with zero behaviour change. Phase C is mechanical but mass-scale
(11 new files, ~400 LOC of route registration each). Doing it in one
agent pass without Windows MSVC at hand is high risk — the existing
build is Windows-only and the routes hit Win32 surfaces (WSA, WinHTTP,
named pipes). Each phase must be verified on the actual target before
the next one starts.

## What's already landed

- Tracking row in `Docs/ANTI_SLOP_AUDIT_TRACKING.md` for #8 / #15
  carries 🟢 SPLIT-LANDED.
- Lexical Completeness plumbing (`Docs/LEXICAL_COMPLETENESS.md`) is
  ready for an admin route to surface incomplete words.
- **Phase B (Feb 2026 — pass 10)**: `RegisterRoutes()` body split
  into 18 private in-class helpers; 3 local lambdas promoted to
  `static` class members.
- **Phase A (Feb 2026 — pass 11)**: `ElleHTTPService` class
  declaration moved into `Services/Elle.Service.HTTP/HTTPServer.h`
  along with file-private types (`HTTPRequest`, `HTTPResponse`,
  `HttpAuthLevel`, `RouteEntry`, `RouteDispatch`, `WSClient`,
  `PendingChat`, `ChatCorrelator`, `JwtVerifyResult`,
  `PairedCacheEntry`, `WsFrameStatus`, `LLMMsg`) and file-scope
  free helpers. State-bearing globals (`g_diagMx`,
  `g_gameAuthDiag`, `g_pairedCacheMx`, `g_pairedCache`) re-tagged
  as `inline` for ODR-safe sharing across all TUs.
  `HTTPServer.cpp` now `#include "HTTPServer.h"` and holds the 27
  non-route methods as out-of-class definitions +
  `ELLE_SERVICE_MAIN(ElleHTTPService)`.
- **Phase C (Feb 2026 — pass 11)**: Each `RegisterXxxRoutes()` body
  moved into its own TU (`HTTPServer_IntroRoutes.cpp` …
  `HTTPServer_SHNRoutes.cpp`, 18 files). 158 routes preserved
  verbatim; brace total across the 19 .cpp files = 1 902 / 1 902.
  `Elle.Service.HTTP.vcxproj` compiles all 19 .cpp files and lists
  `<ClInclude>HTTPServer.h</ClInclude>`.

## Next agent's instructions

1. **Build the project on Windows MSVC** — this is the only
   remaining verification gate.
2. Run an HTTP smoke covering at least one route from each
   registrar (intro/auth/diag/admin/memory/emotions/me-tokens/
   video-identity/ai/dictionary/education/emotional-context/
   x-lifecycle/server/models/morals-goals/misc/shn).
3. If link errors surface (likely candidates: a remaining `static`
   free helper in `HTTPServer.h` that should be `inline`), promote
   the helper from `static` → `inline`. There are still a couple
   of `static` file-scope helpers in the header that compile fine
   in single-TU mode but bloat each TU; promoting them is a follow-up
   optimisation, not a correctness fix.
4. Once Windows-green, flip the Anti-Slop matrix row 8 / 15 to ✅.
