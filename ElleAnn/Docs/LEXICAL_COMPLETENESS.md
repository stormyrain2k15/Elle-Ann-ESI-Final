# Lexical Completeness Audit (Findings #181 / #182)

## What the audit named

The user-supplied audit flagged two related concerns:

> #181 — Language Database Schema Incomplete Relative to Intended Design.
> Anagram representation is missing. Some lexical metadata is distributed
> across tables instead of being represented as a first-class language
> object. No coverage validation for required lexical attributes.
>
> #182 — No Lexical Completeness Audit Found.
> The system asks "Does the word exist?" but not "Does the word have
> enough semantic information to reason about it?"

## What landed (Feb 2026)

### 1. Anagram representation — added as first-class

- New column `dbo.Word.AnagramKey NVARCHAR(128)`.
- Pure-SQL function `dbo.fn_AnagramKey(NVARCHAR(128))` computes the
  canonical key: lowercase, strip non-letters, sort letters ascending.
  `listen → eilnst`, `silent → eilnst`, `LISTEN → eilnst`.
- AFTER INSERT/UPDATE trigger `dbo.tr_Word_AnagramKey` keeps the key
  in sync whenever `NormalizedLemma` changes.
- Index `IX_Word_AnagramKey ON dbo.Word(AnagramKey)` makes group lookup
  O(log n).
- View `dbo.vw_AnagramGroups` returns all groups of size ≥ 2 with
  member list and representative WordID.
- Inline TVF `dbo.fn_Anagrams(@lemma)` returns the anagrams of a given
  lemma without itself.
- Stored procedure `dbo.usp_RebuildAnagramKeys` bulk-populates the
  column for existing rows. The delta SQL calls it at apply-time.

### 2. Required lexical attributes — declared and enforced

New table `dbo.LexicalRequirement` declares the contract. Ten
requirement codes, marked as hard-required or soft:

| Code | Description | Hard |
|---|---|:-:|
| `HAS_DEFINITION` | At least one Sense with non-empty Definition | ✓ |
| `HAS_PART_OF_SPEECH` | At least one Sense bound to a PartOfSpeech | ✓ |
| `HAS_USAGE_EXAMPLE` | At least one SenseUsageExample row | ✓ |
| `HAS_CONTEXT_EXAMPLE` | At least one SenseContextExample row | ✓ |
| `HAS_EMOTION_WEIGHTING` | At least one SenseEmotion row | ✓ |
| `HAS_VALENCE_PULL` | Non-zero PositiveDraw or NegativeDraw on at least one Sense | ✓ |
| `HAS_RELATION` | Participates in WordRelation OR SenseRelation | ✓ |
| `HAS_CONCEPT` | At least one ConceptMember mapping | — |
| `HAS_ANAGRAM_KEY` | AnagramKey populated | ✓ |
| `HAS_PALINDROME_FLAG` | IsPalindrome is set (sanity) | ✓ |

### 3. Reporting views

- `dbo.vw_LexicalCompleteness` — one row per Word with BIT flags for
  every requirement plus the underlying counts (`SensesWithUsage`,
  `SensesWithContext`, `SensesWithEmotion`, `WordRelCount`,
  `SenseRelCount`, `ConceptCount`).
- `dbo.vw_LexicalCompletenessVerdict` — adds `IsCognitivelyComplete`
  (the all-hard-requirements AND-of-BITs), `CompletenessScore` (0.0–1.0
  fraction of nine attributes hit), and `MissingRequirements`
  (semicolon-joined string of failing requirement codes).

### 4. Hard ingestion gate

- `dbo.usp_AssertWordCompleteness @NormalizedLemma, @StrictMode=1`
  THROWs 51001 if the word is missing entirely and 51002 if it's
  present but `IsCognitivelyComplete = 0`. This is the procedure the
  loader/ingestion path calls per-row to refuse "technically present
  but cognitively empty" entries.
- `dbo.usp_LexicalAuditReport @MinScore=0.0, @MaxRows=500` returns
  the worst-completeness words plus a single summary row
  (TotalWords / CompleteWords / IncompleteWords / AvgScore).

### 5. C++ surface

- `include/elle/Types.hpp::WordRecord` now carries `std::string anagramKey`.
- `SqlServerAccessLayer::findWordBy*` selects `ISNULL(AnagramKey, N'')`
  into the new field.
- `InMemoryAccessLayer::addWord` populates `anagramKey` on insert via a
  local sort+filter pass, so the in-memory access layer matches SQL.
- New header `include/elle/LexicalCompleteness.hpp`:
  - `computeAnagramKey(string_view)` — pure, same algorithm as the SQL function.
  - `isPalindromeNormalized(string_view)` — pure.
  - `evaluate(EvaluateInputs)` — produces a full `CompletenessReport`
    (flags, score, ordered `missingRequirements`, `isCognitivelyComplete`).
  - The header has no Windows/ODBC dependency.

### 6. Tests

New doctest cases under `tests/test_lexical_completeness.cpp`:

| Case | What it asserts |
|---|---|
| `computeAnagramKey sorts and lowercases` | `listen / silent / LISTEN` collide on `eilnst` |
| `computeAnagramKey strips non-letters` | `li-sten 2` → `eilnst` |
| `Anagram pairs share the same key` | `evil ⇔ vile ⇔ live`, `dusty ⇔ study` |
| `isPalindromeNormalized` | `racecar`, `level`, `madam` true; `listen`, `""` false |
| `evaluate flags fully-populated word complete` | Score 1.0, all flags true |
| `evaluate flags missing usage examples` | Score < 1.0, `HAS_USAGE_EXAMPLE` in missing |
| `evaluate flags zero-sense word as deeply incomplete` | Score < 0.2 |
| `evaluate flags word with no relations as incomplete` | `HAS_RELATION` in missing |
| `evaluate computes anagram key on-the-fly if missing` | Falls back to in-process compute |
| `evaluate missingRequirements stable order` | Same order across runs |

48/48 Language ctests pass after this addition. Combined harness
total is now **156/156**.

## What this changes for Elle, in plain terms

Before this pass, the lexicon could carry the word `love` but no
emotional weights, no usage example, and no relation to `affection` —
and the Language Engine would happily report "yes that's a word".

After this pass:

1. Every word automatically carries a canonical anagram key, so Elle
   can find `evil/vile/live` in O(1) per group.
2. The system can answer *"is this word cognitively complete?"* not
   just *"does this word exist?"*. The answer is a deterministic
   nine-attribute score + a `MissingRequirements` string.
3. Ingestion can be made fail-closed (`usp_AssertWordCompleteness
   @StrictMode=1`) so half-populated rows can never enter the
   production lexicon.
4. The same algorithm runs in SQL and in C++, so the in-memory access
   layer can't drift from the database.

## Files touched

- `SQL/Elle.Service.Language/10_lexical_completeness.sql` (new)
- `Services/Elle.Service.Language/include/elle/Types.hpp`
- `Services/Elle.Service.Language/include/elle/LexicalCompleteness.hpp` (new)
- `Services/Elle.Service.Language/src/InMemoryAccessLayer.cpp`
- `Services/Elle.Service.Language/src/SqlServerAccessLayer.cpp`
- `Services/Elle.Service.Language/tests/test_lexical_completeness.cpp` (new)
- `Services/Elle.Service.Language/CMakeLists.txt`
