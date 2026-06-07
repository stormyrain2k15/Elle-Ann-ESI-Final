# Elle Semantic Dictionary — ETL Pipeline

Builds the integer-indexed knowledge graph that the Elle language engine
reasons over. Sources are pluggable (WordNet today, NRC-EmoLex /
Wiktionary / your-own next), output is a stable set of CSVs aligned to
`elle-language/sql/01_schema.sql`, and the loader is a single Python
script you run on the Windows box that hosts SQL Server.

## Quick start

```bash
# Container side (Linux) — generate CSVs
pip install nltk tqdm
python3 /app/ElleAnn/Tools/etl/sources/wordnet_to_elle.py
python3 /app/ElleAnn/Tools/etl/validate_csvs.py
# → 12 CSVs in /app/ElleAnn/Tools/etl/output/  (~100 MB, 0 errors)

# Copy /app/ElleAnn/Tools/etl/output/  to your Windows box, e.g. C:\Elle\etl\output\

# Windows side — load into SQL Server
pip install pyodbc tqdm
# 1) Create DB + schema (canonical run order):
#    sqlcmd -S localhost -d master -i sql\00_create_database.sql   (if you have one)
#    for /R sql %f in (*.sql) do sqlcmd -S localhost -d EllesLanguage -i %f
# 2) Stream the CSVs:
python load_to_sqlserver.py ^
    --server localhost ^
    --database EllesLanguage ^
    --trusted ^
    --input-dir C:\Elle\etl\output
```

## Why this exists

The language engine is integer-indexed. Every `Word`, `Sense`, `Phrase`,
`Concept`, `ContextFrame`, and `Emotion` has a `BIGINT` primary key, and
the C++ code reasons exclusively over those IDs. The ETL's job is to
populate the SQL store from whatever lexical source we choose, mapping
external IDs (e.g. WordNet synset names) to our canonical primary keys
without losing fidelity.

## Layout

```
Tools/etl/
├── README.md              this file
├── sources/
│   └── wordnet_to_elle.py     WordNet → CSV (117 k synsets → 12 CSVs)
├── validate_csvs.py           Container-side schema + FK validator
├── load_to_sqlserver.py       Windows-side bulk loader (pyodbc + staging procs)
└── output/                    CSVs land here (gitignored)
```

## CSV contract

The 12 output files use **SourceTag** as a portable cross-file foreign key
*before* the DB assigns the real `BIGINT` IDs. Format:

| Tag prefix         | Meaning                              | Example                  |
|--------------------|--------------------------------------|--------------------------|
| `WN:lemma:<norm>`  | A single-word lemma                  | `WN:lemma:dog`           |
| `WN:phrase:<norm>` | A multi-word phrase (idiom, MWE)     | `WN:phrase:hot dog`      |
| `WN:<synset>`      | A WordNet sense (synset)             | `WN:dog.n.01`            |

`NormalizedLemma` and `NormalizedForm` are the *natural keys* the DB uses
for de-duplication (`Word.NormalizedLemma` is `UNIQUE`).

| File                          | Cols                                                                      | Purpose                                       |
|-------------------------------|---------------------------------------------------------------------------|-----------------------------------------------|
| `words.csv`                   | Lemma, NormalizedLemma, IsPalindrome, Frequency, SourceTag                | `dbo.Word` rows                               |
| `phrases.csv`                 | Surface, NormalizedForm, WordCount, Frequency, SourceTag                  | `dbo.Phrase` rows                             |
| `phrase_words.csv`            | SourceTag, Position, SurfaceText, NormalizedLemma                         | `dbo.PhraseWord` rows                         |
| `senses.csv`                  | NormalizedLemma, PosCode, SenseOrder, Definition, Gloss, …, SourceTag     | `dbo.Sense` rows (single-word senses)         |
| `phrase_senses.csv`           | SourceTag, PhraseSourceTag, SenseOrder, Definition, Gloss, …              | `dbo.PhraseSense` rows                        |
| `sense_usage_examples.csv`    | SourceTag, Slot, ExampleText                                              | `dbo.SenseUsageExample` (Slot ∈ {1,2})        |
| `sense_context_examples.csv`  | SourceTag, Slot, ContextText                                              | `dbo.SenseContextExample` (Slot ∈ {1,2})      |
| `sense_emotions.csv`          | SourceTag, EmotionCode, Weight                                            | `dbo.SenseEmotion` (FK by `Emotion.Code`)     |
| `sense_relations.csv`         | FromSourceTag, ToSourceTag, RelationCode, Strength                        | `dbo.SenseRelation`                           |
| `word_relations.csv`          | FromNormalizedLemma, ToNormalizedLemma, RelationCode, Strength            | `dbo.WordRelation`                            |
| `concepts.csv`                | ConceptLabel, Description                                                 | `dbo.Concept`                                 |
| `concept_members.csv`         | ConceptLabel, SourceTag, Strength                                         | `dbo.ConceptMember`                           |

## Run order (DB load)

Hard dependency:
1. `words.csv`     →  populates `Word`              (MERGE on `NormalizedLemma`)
2. `phrases.csv` + `phrase_words.csv` →  `Phrase` + `PhraseWord`  (MERGE on `NormalizedForm`)
3. `senses.csv`    →  `Sense`, returns `SourceTag→SenseID` map in `#StagingSenseOut`
4. `sense_usage_examples.csv`, `sense_context_examples.csv`, `sense_emotions.csv`  → joined by the map
5. `sense_relations.csv`, `word_relations.csv` → joined by the map
6. `concepts.csv`, `concept_members.csv` → joined by the map

`load_to_sqlserver.py` does all six in one pass, with `pyodbc` + the
staging-table stored procs in `sql/09_validation_and_loaders.sql`.

## Idempotency

Re-running the loader is safe:
- `Word`, `Phrase`, `Concept`: MERGE on natural key, update Frequency to the higher value.
- `Sense`: skipped on `(WordID, PartOfSpeechID, SenseOrder)` match.
- `SenseEmotion`, `SenseRelation`, `WordRelation`, `ConceptMember`:
  `INSERT … WHERE NOT EXISTS` on the natural key.

## Validation (before you push to your DB)

```bash
python3 validate_csvs.py
```

Catches:
- Required fields missing
- `SourceTag` foreign-keys that don't resolve
- Decimals out of `[-1.0, 1.0]`
- `WordCount` mismatched against `phrase_words` row count
- Unknown `PosCode` / `EmotionCode` / `RelationCode`
- `Slot` values not in `{1,2}`
- Senses missing usage/context examples

Exit code is `0` on green, `1` on any ERROR. Warnings are informational.

## Adding a new source

1. Drop a new file under `sources/`, e.g. `sources/nrc_emolex.py`.
2. Write **the same 12 CSVs** with `SourceTag` prefixed by your source
   (e.g. `NRC:lemma:hate`).
3. The validator + loader will pick it up automatically — they don't care
   which source produced the rows, only that the SourceTag prefixes are
   distinct and the schema fields match.

## What's intentionally NOT in the ETL

- **NRC-EmoLex / VAD lexicon** — license-clean drop next. WordNet alone
  gives ~2.5 k emotion-tagged senses (definition-keyword heuristic); the
  real lexicon will lift that to all senses with a real word-level emotion
  vector.
- **SUBTLEX / Google n-gram frequencies** — currently every Frequency
  defaults to 0. Loader's MERGE clamps to MAX, so dropping a frequency
  source later just adds the missing column.
- **Context frames** — handcrafted seeds live in
  `elle-language/config/context_frames.json`. We will eventually grow
  these from a tagged corpus, not from WordNet.

## Current scale (WordNet only)

```
words.csv                       83,045 unique lemmas
phrases.csv                     64,243 phrases
phrase_words.csv               140,740 phrase-word positions
senses.csv                      88,429 senses
phrase_senses.csv               29,229 phrase senses
sense_usage_examples.csv       235,316 rows
sense_context_examples.csv     235,316 rows
sense_emotions.csv               3,280 rows  (will explode w/ NRC-EmoLex)
sense_relations.csv            198,186 rows
word_relations.csv             170,313 rows
concepts.csv                   137,666 concepts
concept_members.csv            206,746 memberships
```

Total: ~100 MB CSV, all SQL-load-ready, validated 0 ERR / 1 WARN.
