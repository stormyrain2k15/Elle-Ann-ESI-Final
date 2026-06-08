# Android Companion ↔ C++ Server Integration

The Android app pairs with a running desktop Elle-Ann HTTP service over
LAN. There is **no LLM call from the server** — chat replies are
generated deterministically by `Elle.Service.Composer`.

## Server-side dependencies (Elle.Service.HTTP)

Auto-linked via `#pragma comment(lib, ...)` — no `.vcxproj` edits
required:

- **`bcrypt.lib`** — SHA-1 for the WebSocket handshake
- **`ws2_32.lib`** — raw Winsock TCP for HTTP + WS
- **`_Shared/json.hpp`** — nlohmann::json (single-header, vendored)

`winhttp.lib` is no longer linked for outbound LLM HTTPS — the LLM purge
removed that surface entirely.

## Endpoints

| Endpoint                                                    | Behaviour                                                         |
|-------------------------------------------------------------|-------------------------------------------------------------------|
| `GET /` `GET /healthz` `GET /api/health`                    | Server info                                                       |
| `GET/POST/PUT/DELETE /api/memory/...`                       | Memory CRUD (live, backed by SQL)                                 |
| `GET/PUT /api/emotions/...`                                 | Live emotion state via IPC cache from `SVC_EMOTIONAL`             |
| `GET/POST /api/tokens/conversations[/id[/messages]]`        | Conversation + message CRUD                                       |
| `POST /api/video/generate`                                  | Hands off to `Tools/Deploy/video_worker` (returns job-ID)         |
| **`POST /api/ai/chat`**                                     | **Dispatches `IPC_CHAT_REQUEST` to `SVC_COGNITIVE`** (composer-backed) |
| `POST /api/ai/agents/{name}/chat`                           | Same path; named-agent persona overlay                            |
| `GET /api/ai/status`                                        | Emotion + Composer status                                         |
| `GET /api/server/status` `/analytics` `/settings`           | Server stats                                                      |
| `GET/POST/PUT/DELETE /api/models/...`                       | (Stubbed — no LLM models in the mesh anymore)                     |
| `GET /api/morals/rules`                                     | Live rules from `MindManager`                                     |
| `POST /api/morals/rules`                                    | Requires `x-admin-key` header                                     |
| `GET /api/dictionary/...`                                   | Live reads of the canonical dictionary                            |
| `GET/POST /api/x/...`                                       | XChromosome surface (cycle / hormones / pregnancy)                |
| `POST /api/auth/pair-code` `POST /api/auth/pair`            | Pairing — issues JWT                                              |
| `GET /api/auth/qr?code=...&host=...`                        | SVG QR for ellepair:// deep link                                  |
| `WS /` (any path with `Upgrade: websocket`)                 | **Full RFC 6455 handshake**                                       |

## Chat flow (server-side)

```
Android app  →  POST /api/ai/chat                  Elle.Service.HTTP
              ↓                                         │
           {message, conversation_id, user_id}          │
                                                        ▼
                                                    IPC_CHAT_REQUEST
                                                        │
                                                        ▼
                                                Elle.Service.Cognitive
                                                  ├─ probability read   (SVC_PROBABILITY)
                                                  ├─ conscience check   (SVC_MIND_MANAGER)
                                                  ├─ intuition gut read (SVC_INTUITION)
                                                  ├─ memory recall       (SQL)
                                                  └─ compose             (SVC_COMPOSER)
                                                        │
                                                        ▼
                                                  IPC_CHAT_RESPONSE
                                                        │
              ↑                                         │
        chat_response  ←  HTTP 200 JSON  ←  ChatCorrelator
```

See `Docs/CHAT_PIPELINE.md` for the full step-by-step.

## WebSocket

Connects at **`ws://<server>:8000/`** (any path).

Protocol is RFC 6455:

1. Client sends `Upgrade: websocket` with `Sec-WebSocket-Key`
2. Server replies `101 Switching Protocols` with `Sec-WebSocket-Accept`
   (SHA-1 + Base64)
3. Server sends welcome JSON
4. Server reads frames, auto-pongs pings
5. Server broadcasts IPC events the client subscribed to

### Client → server messages

```json
{"type": "ping"}
{"type": "chat", "message": "...", "request_id": "..."}
{"type": "subscribe", "topic": "emotion" | "world_event" | "x_phase" | ...}
```

### Server → client messages

```json
{"type": "connected",      "client_id": "...", "server": "Elle-Ann", "version": "3.0.0"}
{"type": "pong"}
{"type": "chat_response",  "request_id": "...", "response": "...",
                            "probabilistic_read": {...},
                            "inner_voice":        {...},
                            "gut_read":           {...}}
{"type": "ipc_broadcast",  "msg_type": <ELLE_IPC_MSG_TYPE>, "payload": {...}}
{"type": "world_event",    "kind": "x_phase_transition" | "x_lh_surge" | ...}
```

## Authentication (pair flow)

1. Desktop ops requests a pair code: `POST /api/auth/pair-code` →
   `{"code": "654321", "expires_in": 300}`
2. (Optional) Desktop shows the QR: `GET /api/auth/qr?code=654321&host=192.168.1.50`
3. Android app scans QR or types host/port/code
4. App: `POST /api/auth/pair {"code":"654321","device":"..."}` → JWT
5. App stores JWT in EncryptedSharedPreferences
6. All subsequent requests attach `Authorization: Bearer <jwt>` via
   `AuthInterceptor`
7. 401 → AuthInterceptor wipes the token and bounces back to PairScreen

## Critical configuration

`elle_master_config.json`:

```json
{
  "services": {
    "sql_pipes": {
      "connection_string": "Driver={ODBC Driver 17 for SQL Server};Server=.\\ELLEANN;Database=ElleCore;Trusted_Connection=yes;"
    }
  },
  "ipc": {
    "pipe_prefix": "\\\\.\\pipe\\ElleAnn_"
  },
  "http": {
    "bind":   "0.0.0.0:8000",
    "cors":   ["http://localhost:3000"],
    "rate":   { "default_rps": 10 }
  },
  "cognitive": {
    "probability_timeout_ms": 300,
    "conscience_timeout_ms":  200,
    "intuition_timeout_ms":   150,
    "max_chat_queue":         64,
    "chat_workers":           4
  }
}
```

> The legacy `"llm": { "providers": ... }` block is **gone**. No Groq /
> OpenAI / Anthropic / llama.cpp keys are read by any service. If you
> still have one in your config, it is ignored.

## Build steps (server)

1. Pull the latest tree.
2. Open `ElleAnn\ElleAnn.sln` → Release | x64 → Build Solution.
3. Install services: `Tools\Deploy\Install-ElleServices.ps1` (see
   `Tools/Deploy/README.md`).
4. Run any individual service in foreground with `Service.exe --console`
   for debugging.

The HTTP service logs route counts on startup, e.g.:
```
HTTP server listening on 0.0.0.0:8000 (62 routes registered)
```

## Test from the command line (Windows `curl.exe`)

```bat
rem Health
curl http://localhost:8000/api/health

rem Chat — composer-backed, no LLM
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"Hello Elle, who are you?\",\"conversation_id\":1}"

rem Live emotion (from IPC cache)
curl http://localhost:8000/api/emotions

rem WebSocket (with wscat or the Android app)
wscat -c ws://localhost:8000/
```

The chat response now carries `gut_read` + `inner_voice` +
`probabilistic_read` blocks alongside the response text. See
`Docs/CHAT_PIPELINE.md` for the full envelope shape.

## Diagnostic logging (server console)

```
[HTTPServer]   POST /api/ai/chat  conv=99 rid=req-123 msg=Hello Elle...
[Probability]  analyze rid=prob-... act=GREET trust=0.50 conf=0.78
[MindManager]  verdict rid=mind-pre-... PROCEED severity=0.00
[Intuition]    intu-pre-... lean=SAFE conf=0.61 weight=0.42 act=ACK_AND_PROBE
[Composer]     compose rid=cmp-... kind=CONVERSE act=ASSERT frame=12 slots=4
[Cognitive]    Chat reply rid=req-123 conv=99 mode=companion in 142ms
```

## What if chat hangs?

1. Confirm Composer is running: `sc.exe query ElleComposer`
2. Watch the console — Cognitive logs `IPC_COMPOSE_REQUEST send failed`
   if Composer is offline, and replies with
   `{"error":"composer_offline","retry":true}`
3. Check `logs/Cognitive.log` and `logs/Composer.log` for the matching
   `correlation_id`
