# Android App ↔ C++ Server Integration

## What was just rewritten
`Services/Elle.Service.HTTP/HTTPServer.cpp` has been completely rewritten to match
the Kotlin `ElleApiService.kt` contract exported from the Elle-Ann Android app.

### New dependencies added (auto-linked via `#pragma comment(lib, ...)`)
- **`winhttp.lib`** — direct HTTPS calls to Groq (bypasses IPC hang for chat)
- **`bcrypt.lib`** — SHA-1 for WebSocket handshake
- **`Shared/json.hpp`** — nlohmann::json (single-header, already vendored)

No changes are required to the `.vcxproj` — `#pragma comment(lib, "xxx")` pulls the
libraries in automatically.

## All endpoints now implemented (60+)

All routes listed in `ElleApiService.kt` respond with the correct JSON shapes
(stubs where server-side data does not yet exist; real behavior where it does).

| Endpoint | Behavior |
|---|---|
| `GET /`, `GET /healthz`, `GET /api/health` | Server info |
| `GET/POST/PUT/DELETE /api/memory/...` | CRUD stubs (id echo) |
| `GET/PUT /api/emotions/...` | Live emotion state from IPC cache |
| `GET/POST /api/tokens/conversations[/id[/messages]]` | CRUD stubs |
| `POST /api/video/generate` etc. | Job-ID stubs |
| **`POST /api/ai/chat`** | **Direct Groq call via WinHTTP** |
| `POST /api/ai/agents/{name}/chat` | Direct Groq call |
| `GET /api/ai/status` | Emotion + model info |
| `GET /api/server/status`, `/analytics`, `/settings` | Server stats |
| `GET/POST/PUT/DELETE /api/models/...` | Stubs |
| `GET /api/morals/rules` | Stub |
| `POST /api/morals/rules` | Requires `x-admin-key` header |
| `GET /api/dictionary/...` | Stubs |
| `WS /` (any path with `Upgrade: websocket`) | **Full RFC 6455 handshake** |

## WebSocket

Connects at **`ws://<server>:8000/`** (any path).
Protocol is RFC 6455:
1. Browser/Android sends `Upgrade: websocket` with `Sec-WebSocket-Key`
2. Server replies `101 Switching Protocols` with `Sec-WebSocket-Accept` (SHA-1+Base64)
3. Server sends welcome JSON: `{"type":"connected","client_id":"...",...}`
4. Server reads frames; pings auto-pong
5. Server broadcasts `{"type":"ipc_broadcast","emotion":{...}}` on emotion updates

### Supported client → server messages
```json
{"type": "ping"}
{"type": "chat", "message": "...", "request_id": "..."}
{"type": "subscribe", "topic": "..."}
```

### Server → client
```json
{"type": "connected", "client_id": "...", "server": "Elle-Ann", "version": "3.0.0"}
{"type": "pong"}
{"type": "chat_response", "request_id": "...", "response": "..."}
{"type": "ipc_broadcast", "msg_type": 1, "emotion": {...}}
```

## Critical configuration

`elle_master_config.json` must contain the Groq key under the `llm.providers.groq` map:
```json
{
  "llm": {
    "providers": {
      "groq": {
        "enabled": true,
        "api_url": "https://api.groq.com/openai/v1/chat/completions",
        "api_key": "gsk_YOUR_KEY_HERE",
        "model": "llama-3.3-70b-versatile"
      }
    }
  }
}
```
The server auto-appends `/chat/completions` if missing.

## Build steps

1. Pull the latest code from this repo into your Visual Studio solution folder.
2. In Visual Studio Solution Explorer → `Elle.Service.HTTP` project →
   right-click → **Rebuild**.
   - No other project changes are needed.
   - Linker will auto-pull in `winhttp.lib` and `bcrypt.lib`.
3. Run `Elle.Service.HTTP.exe --console`.
4. You should see: `HTTP server listening on 0.0.0.0:8000 (62 routes)`

## What's Next (this commit)

**Memory now works end-to-end:**
- `/api/ai/chat` and WS `chat` messages now:
  1. Pull relevant long-term memories (SQL `StoreMemory` / `RecallMemories`) and inject into the system prompt
  2. Pull the last 20 conversation turns from SQL (`GetConversationHistory`) and replay them to Groq
  3. Include the current live emotion state in the system prompt
  4. Persist both the user turn and Elle's reply via `StoreMessage` after each call
  5. Store a summarized episodic memory record for future recall
- Accepts `conversation_id` in the request body (default `1` if omitted) — the app's chat screen should send the current conversation's id to keep continuity.
- Return payload now includes `{"conversation_id": N}` so the app can track the thread.

**To test memory**:
```bat
rem Turn 1
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"My name is Bug.\",\"conversation_id\":99}"

rem Turn 2 — Elle should remember
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"What's my name?\",\"conversation_id\":99}"
```

If SQL Server is reachable and the schemas were loaded, Elle will say "Bug" in turn 2.

## Test from the command line (Windows `curl.exe`)

```bat
rem Health
curl http://localhost:8000/api/health

rem Chat (will hit Groq)
curl -X POST http://localhost:8000/api/ai/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"message\":\"Hello Elle, who are you?\"}"

rem Emotions (cached from IPC)
curl http://localhost:8000/api/emotions

rem WebSocket (using wscat or the Android app)
wscat -c ws://localhost:8000/
```

## What if chat still hangs?

1. Verify `api_key` is set in `elle_master_config.json`.
2. Watch the console — on error, the server now logs:
   `Groq call failed: <reason>`
3. The response body will also include `{"error":"LLM failed","details":"..."}`
   so the Android app can display it.

## Next steps after you verify

- P1 issues from backlog: hardware/device command push via WebSocket
- P2: installer/service registration (double-click install)
