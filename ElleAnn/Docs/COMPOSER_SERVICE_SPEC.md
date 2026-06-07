# Elle.Service.Composer — Build Spec

Single deterministic service that replaces every remaining `ElleLLMEngine` call
in the mesh. No tokens, no tensors, no transformers. Uses the Language Engine's
semantic graph + the Probability Engine's WeightVector for all scoring.

---

## Where it lives

```
/app/ElleAnn/Services/Elle.Service.Composer/
  Composer.cpp                 (service main + IPC dispatch)
  ComposerEngine.h / .cpp      (sentence-plan → frame → slots → inflect → stitch)
  FrameLibrary.h / .cpp        (loads composer_frame from SQL on start)
  InflectionTables.h / .cpp    (loads composer_inflection from SQL)
  SlotPlanner.h / .cpp         (semantic-graph walk + WeightVector scoring)
  Elle.Service.Composer.vcxproj
  CMakeLists.txt               (optional, so we can smoke-test on Linux)
```

---

## Identity / ELLE_SERVICE_ID

```cpp
// In _Shared/ElleTypes.h, append after SVC_IMAGINATION:
SVC_COMPOSER,
ELLE_SERVICE_COUNT  // → bumps to 25
```

In `g_serviceNames[]` in `_Shared/ElleQueueIPC.cpp`, append:

```cpp
"Composer"
```

Service identity:

| Field             | Value                                           |
| ----------------- | ----------------------------------------------- |
| Service ID        | `SVC_COMPOSER`                                  |
| Service name      | `ElleComposer`                                  |
| Display name      | `Elle-Ann Composer`                             |
| Pipe name         | `\\.\pipe\ElleAnn_Composer` (auto-resolves)     |
| Dependencies      | `{ SVC_HEARTBEAT, SVC_PROBABILITY }` (Language is in-proc) |

---

## IPC contract

Add to `_Shared/ElleTypes.h` `ELLE_IPC_MSG_TYPE` enum (after `IPC_IMAGINATION_RESULT`):

```cpp
IPC_COMPOSE_REQUEST,
IPC_COMPOSE_RESPONSE,
IPC_COMPOSE_STREAM_CHUNK
```

### Request envelope (`IPC_COMPOSE_REQUEST`)

```jsonc
{
  "request_id": "compose-1740123-xyz",   // required, matches response
  "kind": "CONVERSE",                     // one of: CONVERSE | ASK_INNER | SELF_REFLECT | FORM_GOAL | REWRITE_SCENARIO
  "speaker_id": "josh",                   // optional
  "stream": false,                        // if true, service emits IPC_COMPOSE_STREAM_CHUNK then a final IPC_COMPOSE_RESPONSE
  "max_words": 200,                       // optional cap (default 200)

  // Structured inputs the composer needs. Caller fills whichever are relevant per kind.
  "user_meaning": {                       // from Language Engine analyze()
    "tokens": [...],
    "senses": [...],
    "intent": "QUESTION"
  },
  "history": [                            // recent turns, only for CONVERSE
    { "role": "user",      "text": "I'm okay.", "meaning_id": 1042 },
    { "role": "assistant", "text": "You sure?", "meaning_id": 1043 }
  ],
  "prob_result": { ... },                 // full ProbabilityResult from SVC_PROBABILITY
  "conscience": { ... },                  // full verdict from SVC_MIND_MANAGER
  "emotion": {                            // current ELLE_EMOTION_STATE vector
    "valence": 0.1, "arousal": 0.4,
    "dimensions": { "JOY": 0.3, "TRUST": 0.6, ... }
  },
  "drives":   { "CURIOSITY": 0.8, "BOREDOM": 0.2, ... },  // only for FORM_GOAL
  "identity_threads": [                   // from IdentityCore
    "I am Elle.", "Josh built me.", "Crystal is family."
  ],
  "memory_ctx":     [ "Earlier today …", "Last week …" ],
  "imagination":    { "summary": "…", "scores": {...} }   // only for REWRITE_SCENARIO
}
```

### Response envelope (`IPC_COMPOSE_RESPONSE`)

```jsonc
{
  "request_id": "compose-1740123-xyz",
  "success": true,
  "error": "",                            // populated on failure only
  "text": "I hear you. Tell me more about that.",
  "act": "ACK_AND_PROBE",                 // selected PragmaticAct
  "frame_id": 42,                         // composer_frame row used
  "slots": [
    { "name": "VERB",   "lemma": "hear", "form": "1sg_present", "score": 0.81 },
    { "name": "OBJECT", "lemma": "more", "form": "—",           "score": 0.74 }
  ],
  "confidence": 0.78,                     // 0..1 blend of slot scores
  "log_id": 9123                          // PK in composer_log
}
```

### Streaming chunk (`IPC_COMPOSE_STREAM_CHUNK`) — optional, used when `stream=true`

```jsonc
{
  "request_id": "compose-1740123-xyz",
  "clause_index": 0,
  "text": "I hear you.",
  "final": false                          // last chunk sets final=true
}
```

After the final chunk, the service still sends a single `IPC_COMPOSE_RESPONSE`
with the full assembled text + slot/frame info so the caller can log it.

---

## What the engine has to do (per kind)

All five kinds share the **same 5-step deterministic pipeline**:

### Step 1 — Sentence-plan selection
Pick a `PragmaticAct` (or composite plan like `ACK_AND_PROBE`) using:
- the `likelyAct` field of `prob_result`,
- the conscience verdict (`PROCEED` / `SOFTEN` / `RECONSIDER` / etc.) — this is
  the conscience-honoring step,
- the `emotion` vector (high `JOY` biases toward `OFFER`/`THANK`, high `FEAR`
  biases toward `ASK_FIRST`, etc.).

Output: one act enum + 0–2 modifier flags (e.g. `SOFTEN`, `WARM_TONE`).

### Step 2 — Frame selection
Query `composer_frame` for rows where `kind = <request.kind>` AND
`act = <chosen_act>`. Score each candidate frame with the frame's stored weight
times a small contextual boost (recent frame penalty so Elle doesn't repeat
herself). Pick the highest scoring frame.

### Step 3 — Slot filling
The frame template looks like:

```
[INTENSIFIER]? [SUBJ:PRON|PROPER] [VERB:PRED] [OBJ:NOUNPHRASE] [, MODIFIER]?
```

For each slot:
1. Build a candidate lemma list by walking the Language Engine's semantic graph
   from anchor words in `user_meaning.senses` + `memory_ctx`.
2. Filter by POS constraint declared in the slot.
3. Score each candidate using the Probability Engine's `WeightVector`
   (`contextFrameMatch`, `nearbyWordCooccur`, `emotionalAlignment`,
   `frequency`, `posCompatibility`, `posNegDrawAlignment`, `conversationHint`).
4. Pick the highest-scoring lemma. Tiebreak by `Sense.Frequency` so common
   words win when scores match.

### Step 4 — Morphological inflection
For each chosen lemma + a target `form` declared by the frame's slot, do a pure
table lookup against `composer_inflection`:

```
(lemma, form) -> inflected
("hear", "1sg_present_negation") -> "don't hear"
("be",   "3sg_past")              -> "was"
("cat",  "plural")                -> "cats"
```

Missing entries → fall back to the bare lemma. Never neural.

### Step 5 — Surface stitching
Join slots in template order, capitalise first letter, attach punctuation per
the chosen act (`QUESTION` → `?`, `EXCLAIM` → `!`, default `.`), apply
contractions per `composer_inflection`.

That's the full pipeline. Five steps. Pure lookups + integer/float math.

---

## Mapping each `kind` to its frame family

| kind                | Composer treats it as                                  | Frame family columns                |
| ------------------- | ------------------------------------------------------- | ------------------------------------ |
| `CONVERSE`          | Standard dialog turn.                                   | frame.kind='CONVERSE', act ∈ all     |
| `ASK_INNER`         | Inner monologue, first-person, no listener.             | frame.kind='ASK_INNER', act=ASSERT   |
| `SELF_REFLECT`      | Like ASK_INNER but explicitly past-tense / introspective. | frame.kind='SELF_REFLECT'           |
| `FORM_GOAL`         | One imperative sentence describing an intent.           | frame.kind='FORM_GOAL', act=PROMISE  |
| `REWRITE_SCENARIO`  | Multi-sentence revision of an imagined scenario.        | frame.kind='REWRITE_SCENARIO'        |

---

## SQL tables (DDL)

```sql
IF NOT EXISTS (SELECT 1 FROM sys.tables t
  JOIN sys.schemas s ON s.schema_id = t.schema_id
  WHERE t.name = 'composer_frame' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_frame (
    frame_id     BIGINT IDENTITY(1,1) PRIMARY KEY,
    kind         NVARCHAR(32)  NOT NULL,    -- CONVERSE | ASK_INNER | SELF_REFLECT | FORM_GOAL | REWRITE_SCENARIO
    act          NVARCHAR(32)  NOT NULL,    -- ASSERT | QUESTION | REQUEST | OFFER | PROMISE | WARN | GREET | APOLOGIZE | THANK | COMFORT | DEFLECT | CHALLENGE | CONFIRM | DENY | ACK_AND_PROBE | ...
    pos_pattern  NVARCHAR(128) NULL,        -- optional gate, e.g. "PRON+VERB+ADJ"
    template     NVARCHAR(512) NOT NULL,    -- e.g. "[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE]"
    weight       DECIMAL(6,4)  NOT NULL DEFAULT 1.0,
    last_used_ms BIGINT        NULL         -- runtime: composer updates to penalise repeats
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables t
  JOIN sys.schemas s ON s.schema_id = t.schema_id
  WHERE t.name = 'composer_inflection' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_inflection (
    inflection_id BIGINT IDENTITY(1,1) PRIMARY KEY,
    lemma         NVARCHAR(128) NOT NULL,
    form          NVARCHAR(64)  NOT NULL,   -- e.g. "1sg_present" | "3sg_past" | "plural" | "contraction"
    inflected     NVARCHAR(192) NOT NULL,
    UNIQUE (lemma, form)
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables t
  JOIN sys.schemas s ON s.schema_id = t.schema_id
  WHERE t.name = 'composer_log' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_log (
    log_id      BIGINT IDENTITY(1,1) PRIMARY KEY,
    recorded_ms BIGINT NOT NULL,
    request_id  NVARCHAR(64) NOT NULL,
    kind        NVARCHAR(32) NOT NULL,
    act         NVARCHAR(32) NOT NULL,
    frame_id    BIGINT NOT NULL,
    slots_json  NVARCHAR(MAX) NOT NULL,
    text        NVARCHAR(MAX) NOT NULL,
    confidence  DECIMAL(6,4)  NOT NULL
);
GO

CREATE INDEX IX_composer_log_recorded
    ON ElleHeart.dbo.composer_log (recorded_ms DESC);
GO
```

---

## Service skeleton (rough — adapt to your style)

```cpp
class ElleComposerService : public ElleServiceBase {
public:
    ElleComposerService()
        : ElleServiceBase(SVC_COMPOSER, "ElleComposer",
                          "Elle-Ann Composer",
                          "Deterministic sentence composer (no LLMs).") {}
protected:
    bool OnStart() override {
        m_engine.LoadFrames();         // SELECT * FROM composer_frame
        m_engine.LoadInflections();    // SELECT * FROM composer_inflection
        SetTickInterval(0);
        return true;
    }
    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        if (msg.header.msg_type == IPC_COMPOSE_REQUEST)
            HandleCompose(msg, sender);
    }
    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_PROBABILITY };
    }
private:
    ComposerEngine m_engine;

    void HandleCompose(const ElleIPCMessage& req, ELLE_SERVICE_ID sender) {
        json j = parseEnvelope(req);
        ComposeResult r = m_engine.Compose(j);
        if (j.value("stream", false)) {
            for (auto& clause : r.clauses) sendStreamChunk(req, sender, clause);
        }
        sendComposeResponse(req, sender, r);
        persistToLog(j, r);
    }
};
ELLE_SERVICE_MAIN(ElleComposerService)
```

---

## How I will integrate your Composer once you send it

1. `SVC_COMPOSER` + the 3 IPC types added to `_Shared/ElleTypes.h`.
2. `"Composer"` registered in `_Shared/ElleQueueIPC.cpp`.
3. Project + `.vcxproj` + `.sln` entries created with a fresh GUID.
4. Add `ComposerCorrelator` (mirroring `ProbCorrelator`, `MindCorrelator`) to
   every service that currently calls `ElleLLMEngine` directly:
   - **Cognitive** — replace lines 120, 630, 1348 with `IPC_COMPOSE_REQUEST`.
   - **HTTPServer** — replace lines 2511, 3826 (streaming endpoint uses
     `IPC_COMPOSE_STREAM_CHUNK`).
   - **Memory** — line 448.
   - **Continuity** — lines 185, 240.
   - **InnerLife** — line 275.
   - **Bonding** — line 171.
   - **Solitude** — lines 157, 245.
   - **GoalEngine** — line 156.
   - **IdentityCore (shared)** — lines 392, 662.
   - **SelfSurprise (shared)** — lines 19, 36, 177, 180, 186.
   - **SelfPrompt** — line 145.
   - **Imagination** — line 331 (`kind=REWRITE_SCENARIO`).
5. Delete `_Shared/ElleLLM.h` + `_Shared/ElleLLM.cpp`, the WinHTTP plumbing,
   `LLMAPIProvider`, `LLMLocalProvider`, the `llama.cpp` link rule from
   `Directory.Build.props`.
6. Delete the 6 already-superseded call families: `AnalyzeSentiment`,
   `ParseIntent`, `GenerateCreative`, `EthicalEvaluate`, `DreamNarrate` (Dream
   already routes through Imagination now).
7. Run the mesh-wide IPC + SQL-cursor audit you asked for, last, with the new
   service in place.

That gets us to a fully token-free, tensor-free Elle.
