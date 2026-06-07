# Elle Semantic Language Engine

**An integer-indexed, SQL-Server-backed semantic language engine in modern C++17. Not an LLM. Not a token model. No embeddings.**

The core unit is the integer-indexed dictionary database. Every meaningful unit — `WordID`, `WordFormID`, `PhraseID`, `PhraseSenseID`, `SenseID`, `ConceptID`, `ContextID`, `SemanticNodeID`, `RelationTypeID`, `EmotionID` — is a 64-bit integer. The engine processes meaning by walking those integers.

---

## Architecture

```
raw text
   │
   ▼
┌──────────────────────────┐
│ 1. InputNormalizer       │  preserves contractions, quotes, "?!", "...", "!!!"
├──────────────────────────┤
│ 2. PhraseScanner         │  longest-match-first; emits PhraseID + PhraseSense candidates
├──────────────────────────┤
│ 3. WordFormResolver      │  WordID -> WordFormID -> unknown
├──────────────────────────┤
│ 4. IntegerSequenceBuilder│  builds the ordered, integer-backed sequence
├──────────────────────────┤
│ 6. ContextComparator     │  scores ContextFrame matches (situation/tone/intent)
├──────────────────────────┤
│ 5. SenseCandidateResolver│  transparent weighted scoring; per-candidate breakdown
├──────────────────────────┤
│ 7. EmotionalWeightProc.  │  per-sentence emotional profile across 12 dimensions
├──────────────────────────┤
│ 8. SemanticGraphWalker   │  bounded BFS over the concept-node graph
├──────────────────────────┤
│ 9. MeaningObjectBuilder  │  emits final MeaningObject + explanation
└──────────────────────────┘
        │
        ▼
   MeaningObject + DebugTrace (renderable to JSON; persistable to dbo.AnalysisTrace)
```

Two `ISqlAccessLayer` implementations ship in-box:

| Implementation          | Backend                                                         | Use                            |
|-------------------------|-----------------------------------------------------------------|--------------------------------|
| `SqlServerAccessLayer`  | ODBC Driver 18 for SQL Server, parameterized via `SQLBindParameter` | Production                     |
| `InMemoryAccessLayer`   | Hand-seeded fixture mirroring `sql/*.sql`                       | Unit tests, smoke tests, demos |

Both implement the same interface; the engine is unaware of which one is loaded.

---

## Repository Layout

```
elle-engine/
├── CMakeLists.txt
├── README.md
├── config/engine.config.json
├── include/elle/                     # public headers
├── src/                              # implementation
├── apps/
│   ├── heartbeat_demo.cpp           # "I'm fine." with four contextual interpretations
│   └── smoke_test.cpp                # canary that exercises all layers
├── tests/                            # doctest suite (13 cases per spec)
└── sql/
    ├── 01_schema.sql                 # T-SQL DDL
    ├── 02_seed_lexicon.sql           # Words, Forms, PartOfSpeech, RelationType, Emotion
    ├── 03_seed_phrases.sql           # multi-word expressions
    ├── 04_seed_senses.sql            # Sense + PhraseSense + examples + emotion weights
    ├── 05_seed_relations.sql         # synonym/antonym/homonym/homophone/heteronym/paraphrase
    ├── 06_seed_context_frames.sql    # ContextFrame + keyword weights
    ├── 07_seed_concepts.sql          # Concept, ConceptMember, SemanticNode, SemanticRelation
    └── 08_indexes.sql                # hot-path indexes
```

---

## Build

Requires CMake 3.20+ and a C++17-capable compiler (MSVC 2019/2022, gcc 9+, clang 10+).

```bat
:: Windows / MSVC
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

```bash
# Linux / macOS (syntax + InMemory backend; ODBC layer requires unixODBC if you want the SQL Server path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

`nlohmann/json` and `doctest` are pulled in via `FetchContent` — no manual third-party setup. Targets:

- `elle_core`        — the layered engine (no ODBC dep, links only nlohmann/json)
- `elle_odbc`        — ODBC + `SqlServerAccessLayer`
- `heartbeat_demo`   — the canonical "I'm fine." showcase
- `smoke_test`       — canary, exits non-zero on first failure
- `elle_tests`       — full doctest suite

CMake option `-DELLE_WITH_ODBC=OFF` disables the ODBC target if you only need the in-memory path.

---

## Database setup

Run the SQL scripts in order against a SQL Server 2019+ instance:

```sql
:r sql\01_schema.sql
:r sql\02_seed_lexicon.sql
:r sql\03_seed_phrases.sql
:r sql\04_seed_senses.sql
:r sql\05_seed_relations.sql
:r sql\06_seed_context_frames.sql
:r sql\07_seed_concepts.sql
:r sql\08_indexes.sql
```

Then point `config/engine.config.json` at your server:

```jsonc
"database": {
    "driver":                 "ODBC Driver 18 for SQL Server",
    "server":                 "tcp:my-sqlserver.local,1433",
    "database":               "EllesLanguage",
    "user":                   "elle_engine",
    "password":               "<from secret store>",
    "trustedConnection":      false,
    "encrypt":                true,
    "trustServerCertificate": false
}
```

---

## Heartbeat demo: "I'm fine."

Running `heartbeat_demo` against the in-memory backend produces four DIFFERENT integer outcomes for the same surface string:

| Conversation hint            | Winning PhraseSenseID | Gloss                |
|------------------------------|-----------------------|----------------------|
| (none)                       | 1                     | `neutral_okay`       |
| `EMOTIONAL_WITHDRAWAL` (2)   | 2                     | `sad_withdrawn`      |
| `DISMISSIVE_HOSTILE` (3)     | 3                     | `angry_dismissive`   |
| `REASSURANCE` (4)            | 4                     | `reassuring`         |

Every decision is rendered as JSON via `DebugTrace::toJson()` with:
- chosen SenseID / PhraseSenseID
- rejected candidate IDs
- per-candidate score breakdown (frequency, context-frame match, draw alignment, …)
- the context frames that influenced the decision and their contributing keywords

---

## Tests (13)

```
elle_tests
  ├── test_input_normalizer.cpp
  ├── test_homonym.cpp              ← "bat" animal vs baseball
  ├── test_homophone.cpp            ← there / their / they're
  ├── test_phrase_resolution.cpp    ← "I'm fine"
  ├── test_emotional_context.cpp    ← neutral / sad / angry / reassuring
  ├── test_synonym_antonym.cpp
  ├── test_paraphrase.cpp
  ├── test_unknown_word.cpp
  ├── test_phrase_first.cpp
  ├── test_context_scoring.cpp
  ├── test_semantic_graph.cpp
  └── test_debug_trace.cpp
```

Run via `ctest --test-dir build --output-on-failure` after build.

---

## Scoring transparency

`SenseCandidateResolver` is the most opinionated layer. Its scoring is a transparent weighted sum, never a black box:

```
score = w1 * context_frame_match
      + w2 * nearby_word_cooccurrence
      + w3 * sense_example_overlap
      + w4 * emotional_alignment
      + w5 * frequency
      + w6 * pos_compatibility
      + w7 * positive_negative_draw_alignment
      + w8 * conversation_hint
```

Weights live in `config/engine.config.json` (`weights` block). Each contribution is recorded in `ScoredSense::scoreBreakdown` and is rendered in both `DebugTrace::toJson()` and `MeaningObject::explanationTrace`.

---

## Caching

All hot lookups are LRU-cached with thread-safe, capacity-bounded `LruCache<K, V>`. Caches:

- WordID by normalized lemma
- WordFormID by normalized form
- Phrases-by-first-word
- Sense candidates per Word/Phrase
- Context frames + per-frame keyword sets
- Sense / phrase-sense emotion weights

Cache sizes are configurable in `config/engine.config.json` (`cache` block) and can be flushed at runtime via `Cache::clear()` / `Cache::evict(key)`.

---

## What is intentionally NOT here

- No transformer, no tokenizer, no embedding vectors, no next-token prediction.
- No external AI API calls.
- No reliance on text similarity heuristics beyond explicit example-overlap.
- No fuzzy string matching beyond `normalizeForLookup`. Misspellings fall through to the unknown-word handler.

---

## What is still missing (future passes)

- **Multi-sentence discourse context**: each call is currently independent. The `ConversationContext` struct is the seed for cross-turn memory.
- **Speaker model**: the engine consumes `speakerRelationship` but does not yet apply it.
- **Real population data**: the SQL seed covers the heartbeat demo. Populating against a full lexicon (CMU dict, WordNet relations, frequency lists) is a separate import pipeline.
- **Connection pooling**: `SqlServerAccessLayer` holds one connection. A pool layer in front is straightforward to add and not part of this pass.
- **Lua scripting hook**: the design allows for per-rule Lua hooks in `SenseCandidateResolver`, but the embed (`lua_State`) is reserved for a later pass.

---

## Engineering constraints honored

- C++17, `/W4 /permissive-` on MSVC; `-Wall -Wextra -Wpedantic -Wshadow` on gcc/clang.
- All ODBC calls go through RAII handles; every SQL parameter goes through `SQLBindParameter` — no string concatenation, no SQL injection surface.
- Every SQL_C boundary uses `SQL_C_WCHAR` for `NVARCHAR` types; engine-internal strings are UTF-8.
- No raw owning pointers in the public API; every owning resource is a `unique_ptr` / `shared_ptr` / RAII wrapper.
- Strong ID types prevent accidentally swapping `WordID` and `SenseID` at the type system.
- Every decision is recorded in `DebugTrace`. The engine is not a black box.
