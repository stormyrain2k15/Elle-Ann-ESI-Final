from __future__ import annotations

import argparse
import csv
import sys
import time
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Tuple

try:
    import pyodbc
except ImportError:
    sys.stderr.write("Missing pyodbc. Install with: pip install pyodbc\n")
    sys.exit(2)

try:
    from tqdm import tqdm
except ImportError:
    def tqdm(it, **_):
        return it

def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(description="Load Elle ETL CSVs into SQL Server.")
    ap.add_argument("--server", required=True)
    ap.add_argument("--database", default="EllesLanguage")
    ap.add_argument("--trusted", action="store_true",
                    help="Use Windows integrated auth (Trusted_Connection=yes).")
    ap.add_argument("--user")
    ap.add_argument("--password")
    ap.add_argument("--driver", default="ODBC Driver 17 for SQL Server")
    ap.add_argument("--input-dir", default=str(Path(__file__).resolve().parent / "output"))
    ap.add_argument("--batch-size", type=int, default=5000)
    ap.add_argument("--skip", default="", help="Comma-sep list of stages to skip "
                    "(words, phrases, senses, examples, emotions, relations, "
                    "concepts, phrase_senses).")
    return ap.parse_args()

def connect(args: argparse.Namespace) -> pyodbc.Connection:
    if args.trusted:
        cs = (f"DRIVER={{{args.driver}}};SERVER={args.server};"
              f"DATABASE={args.database};Trusted_Connection=yes;")
    else:
        if not args.user or not args.password:
            sys.exit("Must supply --trusted OR (--user AND --password)")
        cs = (f"DRIVER={{{args.driver}}};SERVER={args.server};"
              f"DATABASE={args.database};UID={args.user};PWD={args.password};")
    conn = pyodbc.connect(cs, autocommit=False)
    conn.cursor().fast_executemany = True
    return conn

def cursor(conn: pyodbc.Connection) -> pyodbc.Cursor:
    cur = conn.cursor()
    cur.fast_executemany = True
    return cur

def iter_csv(path: Path) -> Iterator[Dict[str, str]]:
    with open(path, "r", encoding="utf-8", newline="") as fp:
        reader = csv.DictReader(fp)
        for row in reader:
            yield row

def in_batches(it: Iterable, n: int) -> Iterator[list]:
    batch: list = []
    for row in it:
        batch.append(row)
        if len(batch) >= n:
            yield batch
            batch = []
    if batch:
        yield batch

def load_words(conn: pyodbc.Connection, csv_path: Path, batch_size: int) -> None:
    print(f"[stage:words] loading {csv_path.name}")
    cur = cursor(conn)
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingWord') IS NOT NULL DROP TABLE #StagingWord;
        CREATE TABLE #StagingWord (
            Lemma           NVARCHAR(128) NOT NULL,
            NormalizedLemma NVARCHAR(128) NOT NULL,
            IsPalindrome    BIT           NOT NULL DEFAULT 0,
            Frequency       BIGINT        NOT NULL DEFAULT 0
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(csv_path), batch_size):
        rows = [(r["Lemma"], r["NormalizedLemma"],
                 int(r.get("IsPalindrome", 0) or 0),
                 int(r.get("Frequency", 0) or 0)) for r in batch]
        cur.executemany(
            "INSERT INTO #StagingWord (Lemma, NormalizedLemma, IsPalindrome, Frequency) "
            "VALUES (?, ?, ?, ?)", rows)
        n += len(rows)
    print(f"[stage:words]   staged {n:,} rows; calling usp_LoadStagingWords ...")
    cur.execute("EXEC dbo.usp_LoadStagingWords")
    conn.commit()
    print(f"[stage:words]   done.")

def load_phrases(conn: pyodbc.Connection, dir_: Path, batch_size: int) -> None:
    print("[stage:phrases] loading phrases.csv + phrase_words.csv")
    cur = cursor(conn)
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingPhrase') IS NOT NULL DROP TABLE #StagingPhrase;
        CREATE TABLE #StagingPhrase (
            Surface         NVARCHAR(256) NOT NULL,
            NormalizedForm  NVARCHAR(256) NOT NULL,
            WordCount       INT           NOT NULL,
            Frequency       BIGINT        NOT NULL,
            SourceTag       NVARCHAR(64)  NOT NULL
        );
        IF OBJECT_ID('tempdb..#StagingPhraseWord') IS NOT NULL DROP TABLE #StagingPhraseWord;
        CREATE TABLE #StagingPhraseWord (
            SourceTag       NVARCHAR(64)  NOT NULL,
            Position        INT           NOT NULL,
            SurfaceText     NVARCHAR(128) NOT NULL,
            NormalizedLemma NVARCHAR(128) NOT NULL
        );
    """)
    np_ = 0
    for batch in in_batches(iter_csv(dir_ / "phrases.csv"), batch_size):
        rows = [(r["Surface"], r["NormalizedForm"],
                 int(r["WordCount"] or 0),
                 int(r.get("Frequency", 0) or 0),
                 r["SourceTag"]) for r in batch]
        cur.executemany(
            "INSERT INTO #StagingPhrase (Surface, NormalizedForm, WordCount, "
            "Frequency, SourceTag) VALUES (?, ?, ?, ?, ?)", rows)
        np_ += len(rows)
    nw = 0
    for batch in in_batches(iter_csv(dir_ / "phrase_words.csv"), batch_size):
        rows = [(r["SourceTag"], int(r["Position"]),
                 r["SurfaceText"], r["NormalizedLemma"]) for r in batch]
        cur.executemany(
            "INSERT INTO #StagingPhraseWord (SourceTag, Position, SurfaceText, "
            "NormalizedLemma) VALUES (?, ?, ?, ?)", rows)
        nw += len(rows)
    print(f"[stage:phrases]   staged {np_:,} phrases, {nw:,} phrase-words")
    cur.execute("EXEC dbo.usp_LoadStagingPhrases")
    conn.commit()
    print("[stage:phrases]   done.")

def load_senses(conn: pyodbc.Connection, csv_path: Path,
                batch_size: int) -> Dict[str, int]:
    print(f"[stage:senses] loading {csv_path.name}")
    cur = cursor(conn)
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingSense') IS NOT NULL DROP TABLE #StagingSense;
        CREATE TABLE #StagingSense (
            NormalizedLemma NVARCHAR(128) NOT NULL,
            PosCode         NVARCHAR(16)  NULL,
            SenseOrder      INT           NOT NULL,
            Definition      NVARCHAR(1024) NOT NULL,
            Gloss           NVARCHAR(256) NULL,
            PositiveDraw    DECIMAL(6,4)  NULL,
            NegativeDraw    DECIMAL(6,4)  NULL,
            Valence         DECIMAL(6,4)  NULL,
            Frequency       BIGINT        NULL,
            SourceTag       NVARCHAR(64)  NOT NULL
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(csv_path), batch_size):
        rows = [(r["NormalizedLemma"], r.get("PosCode") or None,
                 int(r["SenseOrder"]),
                 (r["Definition"] or "")[:1024],
                 (r.get("Gloss") or None) if r.get("Gloss") else None,
                 float(r.get("PositiveDraw") or 0.0),
                 float(r.get("NegativeDraw") or 0.0),
                 float(r.get("Valence") or 0.0),
                 int(r.get("Frequency") or 0),
                 r["SourceTag"]) for r in batch]
        cur.executemany(
            "INSERT INTO #StagingSense (NormalizedLemma, PosCode, SenseOrder, "
            "Definition, Gloss, PositiveDraw, NegativeDraw, Valence, "
            "Frequency, SourceTag) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", rows)
        n += len(rows)
    print(f"[stage:senses]   staged {n:,} rows; calling usp_LoadStagingSenses ...")
    cur.execute("EXEC dbo.usp_LoadStagingSenses")
    cur.execute("SELECT SourceTag, SenseID FROM #StagingSenseOut")
    mapping = {tag: int(sid) for tag, sid in cur.fetchall()}
    conn.commit()
    print(f"[stage:senses]   resolved {len(mapping):,} SourceTag -> SenseID mappings.")
    return mapping

def load_examples_and_emotions(conn: pyodbc.Connection, dir_: Path,
                               sense_map: Dict[str, int],
                               batch_size: int) -> None:
    cur = cursor(conn)

    def _stream_rows(csv_path: Path, slot_field: str, text_field: str,
                     table: str, slot_col: str, text_col: str) -> int:
        n = 0
        for batch in in_batches(iter_csv(csv_path), batch_size):
            rows = []
            for r in batch:
                sid = sense_map.get(r["SourceTag"])
                if sid is None:
                    continue
                rows.append((sid, int(r[slot_field]), r[text_field][:1024]))
            if rows:
                cur.executemany(
                    f"INSERT INTO dbo.{table} (SenseID, {slot_col}, {text_col}) "
                    f"VALUES (?, ?, ?)", rows)
                n += len(rows)
        return n

    print("[stage:examples] sense_usage_examples ...")
    n = _stream_rows(dir_ / "sense_usage_examples.csv",
                     "Slot", "ExampleText",
                     "SenseUsageExample", "Slot", "ExampleText")
    print(f"[stage:examples]   inserted {n:,} usage rows.")

    print("[stage:examples] sense_context_examples ...")
    n = _stream_rows(dir_ / "sense_context_examples.csv",
                     "Slot", "ContextText",
                     "SenseContextExample", "Slot", "ContextText")
    print(f"[stage:examples]   inserted {n:,} context rows.")

    print("[stage:emotions] sense_emotions ...")

    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingEmo') IS NOT NULL DROP TABLE #StagingEmo;
        CREATE TABLE #StagingEmo (
            SenseID     BIGINT       NOT NULL,
            EmotionCode NVARCHAR(32) NOT NULL,
            Weight      DECIMAL(6,4) NOT NULL
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(dir_ / "sense_emotions.csv"), batch_size):
        rows = []
        for r in batch:
            sid = sense_map.get(r["SourceTag"])
            if sid is None:
                continue
            rows.append((sid, r["EmotionCode"], float(r["Weight"])))
        if rows:
            cur.executemany(
                "INSERT INTO #StagingEmo (SenseID, EmotionCode, Weight) "
                "VALUES (?, ?, ?)", rows)
            n += len(rows)
    cur.execute("""
        INSERT INTO dbo.SenseEmotion (SenseID, EmotionID, Weight)
        SELECT se.SenseID, e.EmotionID, se.Weight
        FROM #StagingEmo se
        JOIN dbo.Emotion e ON e.Code = se.EmotionCode
        WHERE NOT EXISTS (
            SELECT 1 FROM dbo.SenseEmotion x
            WHERE x.SenseID = se.SenseID AND x.EmotionID = e.EmotionID
        );
    """)
    conn.commit()
    print(f"[stage:emotions]   staged {n:,} rows; merged into SenseEmotion.")

def load_relations(conn: pyodbc.Connection, dir_: Path,
                   sense_map: Dict[str, int], batch_size: int) -> None:
    cur = cursor(conn)

    print("[stage:relations] sense_relations ...")
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingSR') IS NOT NULL DROP TABLE #StagingSR;
        CREATE TABLE #StagingSR (
            FromSenseID    BIGINT       NOT NULL,
            ToSenseID      BIGINT       NOT NULL,
            RelationCode   NVARCHAR(32) NOT NULL,
            Strength       DECIMAL(6,4) NOT NULL
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(dir_ / "sense_relations.csv"), batch_size):
        rows = []
        for r in batch:
            f = sense_map.get(r["FromSourceTag"])
            t = sense_map.get(r["ToSourceTag"])
            if f is None or t is None or f == t:
                continue
            rows.append((f, t, r["RelationCode"], float(r["Strength"])))
        if rows:
            cur.executemany(
                "INSERT INTO #StagingSR (FromSenseID, ToSenseID, RelationCode, "
                "Strength) VALUES (?, ?, ?, ?)", rows)
            n += len(rows)
    cur.execute("""
        INSERT INTO dbo.SenseRelation (FromSenseID, ToSenseID, RelationTypeID, Strength)
        SELECT sr.FromSenseID, sr.ToSenseID, rt.RelationTypeID, sr.Strength
        FROM #StagingSR sr
        JOIN dbo.RelationType rt ON rt.Code = sr.RelationCode
        WHERE NOT EXISTS (
            SELECT 1 FROM dbo.SenseRelation x
            WHERE x.FromSenseID = sr.FromSenseID
              AND x.ToSenseID   = sr.ToSenseID
              AND x.RelationTypeID = rt.RelationTypeID
        );
    """)
    conn.commit()
    print(f"[stage:relations]   staged {n:,} sense-relation rows; merged.")

    print("[stage:relations] word_relations ...")
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingWR') IS NOT NULL DROP TABLE #StagingWR;
        CREATE TABLE #StagingWR (
            FromNorm     NVARCHAR(128) NOT NULL,
            ToNorm       NVARCHAR(128) NOT NULL,
            RelationCode NVARCHAR(32)  NOT NULL,
            Strength     DECIMAL(6,4)  NOT NULL
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(dir_ / "word_relations.csv"), batch_size):
        rows = [(r["FromNormalizedLemma"], r["ToNormalizedLemma"],
                 r["RelationCode"], float(r["Strength"])) for r in batch]
        cur.executemany(
            "INSERT INTO #StagingWR (FromNorm, ToNorm, RelationCode, Strength) "
            "VALUES (?, ?, ?, ?)", rows)
        n += len(rows)
    cur.execute("""
        INSERT INTO dbo.WordRelation (FromWordID, ToWordID, RelationTypeID, Strength)
        SELECT w1.WordID, w2.WordID, rt.RelationTypeID, wr.Strength
        FROM #StagingWR wr
        JOIN dbo.Word w1         ON w1.NormalizedLemma = wr.FromNorm
        JOIN dbo.Word w2         ON w2.NormalizedLemma = wr.ToNorm
        JOIN dbo.RelationType rt ON rt.Code = wr.RelationCode
        WHERE w1.WordID <> w2.WordID
          AND NOT EXISTS (
              SELECT 1 FROM dbo.WordRelation x
              WHERE x.FromWordID = w1.WordID
                AND x.ToWordID   = w2.WordID
                AND x.RelationTypeID = rt.RelationTypeID
          );
    """)
    conn.commit()
    print(f"[stage:relations]   staged {n:,} word-relation rows; merged.")

def load_concepts(conn: pyodbc.Connection, dir_: Path,
                  sense_map: Dict[str, int], batch_size: int) -> None:
    cur = cursor(conn)

    print("[stage:concepts] concepts ...")
    n = 0
    for batch in in_batches(iter_csv(dir_ / "concepts.csv"), batch_size):
        rows = [(r["ConceptLabel"], r.get("Description", "")) for r in batch]
        cur.executemany(
            "MERGE dbo.Concept AS T USING (VALUES (?, ?)) AS S(Label, Description) "
            "ON T.Label = S.Label "
            "WHEN NOT MATCHED THEN INSERT (Label, Description) "
            "VALUES (S.Label, S.Description);", rows)
        n += len(rows)
    conn.commit()
    print(f"[stage:concepts]   inserted/merged {n:,} concepts.")

    print("[stage:concepts] concept_members ...")
    cur.execute("""
        IF OBJECT_ID('tempdb..#StagingCM') IS NOT NULL DROP TABLE #StagingCM;
        CREATE TABLE #StagingCM (
            ConceptLabel NVARCHAR(128) NOT NULL,
            SenseID      BIGINT        NOT NULL,
            Strength     DECIMAL(6,4)  NOT NULL
        );
    """)
    n = 0
    for batch in in_batches(iter_csv(dir_ / "concept_members.csv"), batch_size):
        rows = []
        for r in batch:
            sid = sense_map.get(r["SourceTag"])
            if sid is None:
                continue
            rows.append((r["ConceptLabel"], sid, float(r["Strength"])))
        if rows:
            cur.executemany(
                "INSERT INTO #StagingCM (ConceptLabel, SenseID, Strength) "
                "VALUES (?, ?, ?)", rows)
            n += len(rows)
    cur.execute("""
        INSERT INTO dbo.ConceptMember (ConceptID, SenseID, Strength)
        SELECT c.ConceptID, cm.SenseID, cm.Strength
        FROM #StagingCM cm
        JOIN dbo.Concept c ON c.Label = cm.ConceptLabel
        WHERE NOT EXISTS (
            SELECT 1 FROM dbo.ConceptMember x
            WHERE x.ConceptID = c.ConceptID AND x.SenseID = cm.SenseID
        );
    """)
    conn.commit()
    print(f"[stage:concepts]   staged {n:,} member rows; merged.")

def main() -> None:
    args = parse_args()
    input_dir = Path(args.input_dir)
    skip = {s.strip() for s in args.skip.split(",") if s.strip()}

    if not input_dir.exists():
        sys.exit(f"Input dir does not exist: {input_dir}")

    t0 = time.time()
    print(f"[run] connecting to {args.server}/{args.database} ...")
    conn = connect(args)

    if "words" not in skip:
        load_words(conn, input_dir / "words.csv", args.batch_size)
    if "phrases" not in skip:
        load_phrases(conn, input_dir, args.batch_size)
    sense_map: Dict[str, int] = {}
    if "senses" not in skip:
        sense_map = load_senses(conn, input_dir / "senses.csv", args.batch_size)
    if "examples" not in skip and sense_map:
        load_examples_and_emotions(conn, input_dir, sense_map, args.batch_size)
    if "relations" not in skip and sense_map:
        load_relations(conn, input_dir, sense_map, args.batch_size)
    if "concepts" not in skip and sense_map:
        load_concepts(conn, input_dir, sense_map, args.batch_size)

    conn.close()
    print(f"[run] ALL DONE in {time.time() - t0:.1f}s")

if __name__ == "__main__":
    main()
