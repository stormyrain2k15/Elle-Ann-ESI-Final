# ElleAnn Android Companion — Integration Spec (Connection Points)

**Status**: Spec-only. Implementation deferred to a dedicated session.
**Target**: Android companion app for the XChromosome cycle/lifecycle/
pregnancy surface + future expansion to other service endpoints.

This folder holds **code-shaped contracts**, not prose. Each `.kt` file
documents a connection point exactly as the Android app will consume
it. Drop these into the real Android project's `data/` package when
development begins and fill in the function bodies — the contracts are
already aligned to the live C++ HTTP service (verified against the
actual route handlers in `Services/Elle.Service.HTTP/HTTPServer.cpp`
at the time of writing).

## Files in this folder

| File | Purpose |
|---|---|
| `README.md` | This index |
| `ConnectionPoints.kt` | The full map of **what connects where** — ports, auth model, request/response shapes |
| `XApi.kt` | Retrofit-style interface mirroring all 17 `/api/x/...` HTTP routes |
| `XModels.kt` | Kotlin data classes for every request/response body (one-for-one with JSON keys) |
| `IpcOpcodes.kt` | Read-only reference of the 20 `IPC_X_*` opcode IDs (app never speaks raw IPC — this is for debugging pipeline traces) |
| `Auth.kt` | Pairing + token storage contract |
| `WsCommand.kt` | WebSocket command channel contract (`/command`, RFC 6455) |
| `ErrorEnvelope.kt` | Error shape contract — what every non-2xx body looks like |
| `SyncPolicy.kt` | When to poll vs. push, offline behaviour, retry/back-off rules |

## Integration snapshot (TL;DR)

- **Transport**: HTTP/1.1 + WebSocket on a single port. No gRPC.
- **Default port**: `8000` (configurable in `elle_master_config.json`:
  `http.port`). Bind default is `127.0.0.1`. For remote device access
  user must change `http.bind_address` explicitly — we do **not** ship
  a discovery service.
- **Auth**: `Authorization: Bearer <jwt>` on every request to endpoints
  marked `AUTH_USER`. The JWT is issued by a pairing handshake (see
  `Auth.kt`). Admin endpoints use `x-admin-key` and the Android app
  does **not** call those.
- **CORS**: Configurable, not relevant for a native Android client
  (CORS applies only to browser origins).
- **Content type**: JSON in, JSON out. UTF-8.
- **Timestamps**: Unix epoch milliseconds in `uint64` fields (named
  `*_ms`). Never ISO strings from this service.
- **WebSocket path**: `/command` (RFC 6455, text frames, ≤ 1 MiB).
  Used for push events; if the app can't maintain a socket (app
  background / poor network), fall back to polling per `SyncPolicy.kt`.
- **Push events the app cares about**: `IPC_X_PHASE_TRANSITION`,
  `IPC_X_BIRTH`, `IPC_X_LH_SURGE`, `IPC_X_LABOR_STAGE`,
  `IPC_X_MISCARRIAGE`. Surfaced over the WS channel as JSON envelopes
  (see `WsCommand.kt` for the envelope shape).

## What this spec is NOT

- **Not a UI spec.** Screen list, flows, and visual design are a
  separate piece of work. The endpoint contracts here support any UI
  decision the user makes later.
- **Not a replacement for viewing the server code.** If a spec file
  here disagrees with the live server, **the server is authoritative**
  — update the spec. The HTTP server implementation is the contract;
  this folder is a Kotlin-shaped mirror of it.
- **Not a transport spec for the app → other services.** The Android
  app talks to `Elle.Service.HTTP` only. It never opens a named pipe,
  never speaks raw IPC. Everything routes through the HTTP service.

## Versioning

When the HTTP surface changes (route added, request/response field
added/removed), update the matching `.kt` file here in the **same
commit** that touches the C++ side. CI canary (`wx-pattern-canary` +
`embedding-novelty-test` precedent) — TODO: add a schema-drift canary
that fails if `/api/x/...` routes exist in the C++ but not in `XApi.kt`.
