from __future__ import annotations

import csv
import gzip
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

THIS_DIR  = Path(__file__).resolve().parent
ETL_DIR   = THIS_DIR.parent
SOURCES   = ETL_DIR / "sources"

SAMPLE_SENSES = [
    ["NormalizedLemma", "PosCode", "SenseOrder", "Definition", "Gloss",
     "PositiveDraw", "NegativeDraw", "Valence", "Frequency", "SourceTag"],
    ["happy",   "ADJ", 0, "feeling joy",     "feeling joy",     "0.0", "0.0", "0.0", 0, "WN:happy.a.01"],
    ["sad",     "ADJ", 0, "feeling sorrow",  "feeling sorrow",  "0.0", "0.0", "0.0", 0, "WN:sad.a.01"],
    ["anger",   "NOUN",0, "strong displeasure","strong displeasure","0.0","0.0","0.0",0,"WN:anger.n.01"],
    ["calm",    "ADJ", 0, "not agitated",    "not agitated",    "0.0", "0.0", "0.0", 0, "WN:calm.a.01"],
    ["unknown1","NOUN",0, "rare placeholder","rare placeholder","0.0","0.0","0.0",0,"WN:unknown1.n.01"],
]

SAMPLE_WORDS = [
    ["Lemma", "NormalizedLemma", "IsPalindrome", "Frequency", "SourceTag"],
    ["happy",   "happy",   0, 0, "WN:lemma:happy"],
    ["sad",     "sad",     0, 0, "WN:lemma:sad"],
    ["anger",   "anger",   0, 0, "WN:lemma:anger"],
    ["calm",    "calm",    0, 0, "WN:lemma:calm"],
    ["unknown1","unknown1",0, 0, "WN:lemma:unknown1"],
]

EMOLEX_INPUT = """\
happy\tjoy\t1
happy\tpositive\t1
sad\tsadness\t1
sad\tnegative\t1
anger\tanger\t1
calm\ttrust\t1
notfound\tjoy\t1
"""

VAD_INPUT = """\
Word\tValence\tArousal\tDominance
happy\t0.96\t0.74\t0.71
sad\t0.06\t0.41\t0.32
anger\t0.12\t0.81\t0.66
calm\t0.85\t0.20\t0.45
notfound\t0.50\t0.50\t0.50
"""


def make_wiktionary_jsonl() -> str:
    rows = [
        {"word": "happy",   "pos": "adjective",
         "senses": [{"glosses": ["feeling joy or pleasure"]},
                    {"glosses": ["a fortunate state"]}],
         "synonyms": [{"word": "joyful"}, {"word": "glad"}]},
        {"word": "newbie",  "pos": "noun",
         "senses": [{"glosses": ["a newcomer"]}]},
        {"word": "anger",   "pos": "noun",
         "senses": [{"glosses": ["strong displeasure"]}],
         "antonyms": [{"word": "calm"}]},
    ]
    return "\n".join(json.dumps(r) for r in rows) + "\n"


def write_csv(path: Path, rows):
    with open(path, "w", encoding="utf-8", newline="") as fp:
        w = csv.writer(fp)
        for r in rows:
            w.writerow(r)


def run(cmd: list[str]) -> tuple[int, str, str]:
    p = subprocess.run(cmd, capture_output=True, text=True)
    return p.returncode, p.stdout, p.stderr


def main() -> int:
    failures = 0
    tmp = Path(tempfile.mkdtemp(prefix="etl_test_"))
    try:
        output = tmp / "output"
        data   = tmp / "data"
        output.mkdir()
        data.mkdir()

        write_csv(output / "senses.csv", SAMPLE_SENSES)
        write_csv(output / "words.csv",  SAMPLE_WORDS)

        emo_path = data / "NRC-Emotion-Lexicon-Wordlevel-v0.92.txt"
        emo_path.write_text(EMOLEX_INPUT, encoding="utf-8")
        rc, out, err = run([sys.executable, str(SOURCES / "nrc_emolex_to_elle.py"),
                            "--lexicon",    str(emo_path),
                            "--output-dir", str(output),
                            "--senses-csv", str(output / "senses.csv")])
        if rc != 0:
            sys.stderr.write(f"FAIL nrc_emolex rc={rc}\n{err}\n"); failures += 1

        rows = list(csv.DictReader(open(output / "sense_emotions_nrc.csv")))
        codes = {r["EmotionCode"] for r in rows}
        tags  = {r["SourceTag"]   for r in rows}
        if not {"JOY", "POS_DRAW", "SADNESS", "NEG_DRAW", "ANGER", "TRUST"}.issubset(codes):
            sys.stderr.write(f"FAIL emolex codes={codes}\n"); failures += 1
        if "WN:happy.a.01" not in tags:
            sys.stderr.write(f"FAIL emolex missing happy tag\n"); failures += 1
        print(f"[emolex] {len(rows)} rows / codes={sorted(codes)}")

        vad_path = data / "NRC-VAD-Lexicon.txt"
        vad_path.write_text(VAD_INPUT, encoding="utf-8")
        rc, out, err = run([sys.executable, str(SOURCES / "nrc_vad_to_elle.py"),
                            "--lexicon",    str(vad_path),
                            "--output-dir", str(output),
                            "--senses-csv", str(output / "senses.csv")])
        if rc != 0:
            sys.stderr.write(f"FAIL nrc_vad rc={rc}\n{err}\n"); failures += 1

        rows = list(csv.DictReader(open(output / "sense_valence_vad.csv")))
        by_tag = {r["SourceTag"]: r for r in rows}
        if "WN:happy.a.01" not in by_tag or float(by_tag["WN:happy.a.01"]["Valence"]) < 0.5:
            sys.stderr.write(f"FAIL vad happy valence\n"); failures += 1
        if "WN:sad.a.01" not in by_tag or float(by_tag["WN:sad.a.01"]["NegativeDraw"]) < 0.5:
            sys.stderr.write(f"FAIL vad sad neg_draw\n"); failures += 1
        print(f"[vad] {len(rows)} rows")

        wikt_path = data / "kaikki-en.jsonl"
        wikt_path.write_text(make_wiktionary_jsonl(), encoding="utf-8")
        rc, out, err = run([sys.executable, str(SOURCES / "wiktionary_to_elle.py"),
                            "--dump",       str(wikt_path),
                            "--output-dir", str(output),
                            "--words-csv",  str(output / "words.csv"),
                            "--senses-csv", str(output / "senses.csv")])
        if rc != 0:
            sys.stderr.write(f"FAIL wiktionary rc={rc}\n{err}\n"); failures += 1

        new_words   = list(csv.DictReader(open(output / "words_wikt.csv")))
        new_senses  = list(csv.DictReader(open(output / "senses_wikt.csv")))
        new_rel     = list(csv.DictReader(open(output / "word_relations_wikt.csv")))
        norms = {w["NormalizedLemma"] for w in new_words}
        if "newbie" not in norms:
            sys.stderr.write(f"FAIL wiktionary newbie missing\n"); failures += 1
        if "happy" in norms:
            sys.stderr.write(f"FAIL wiktionary should skip known word 'happy'\n"); failures += 1
        if not new_senses:
            sys.stderr.write(f"FAIL wiktionary no senses emitted\n"); failures += 1
        sample_rels = {(r["FromNormalizedLemma"], r["ToNormalizedLemma"], r["RelationCode"])
                       for r in new_rel}
        if ("anger", "calm", "ANTONYM") not in sample_rels:
            sys.stderr.write(f"FAIL wiktionary antonym missing\n"); failures += 1
        print(f"[wiktionary] words={len(new_words)} senses={len(new_senses)} rels={len(new_rel)}")

        if failures == 0:
            print("\n==== ETL aug PASS — 0 failures ====")
            return 0
        print(f"\n==== ETL aug FAIL — {failures} failures ====")
        return 1
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
