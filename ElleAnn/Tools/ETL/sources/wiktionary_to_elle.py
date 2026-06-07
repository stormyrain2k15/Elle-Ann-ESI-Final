from __future__ import annotations

import argparse
import csv
import gzip
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parents[1] / "output"
DATA_DIR   = Path(__file__).resolve().parents[1] / "data"

WIKT_POS_TO_ELLE = {
    "noun":         "NOUN",
    "verb":         "VERB",
    "adj":          "ADJ",
    "adjective":    "ADJ",
    "adv":          "ADV",
    "adverb":       "ADV",
    "pron":         "PRON",
    "pronoun":      "PRON",
    "det":          "DET",
    "determiner":   "DET",
    "prep":         "PREP",
    "preposition":  "PREP",
    "conj":         "CONJ",
    "conjunction":  "CONJ",
    "intj":         "INTERJ",
    "interjection": "INTERJ",
    "num":          "NUM",
    "numeral":      "NUM",
    "name":         "PROPN",
    "proper noun":  "PROPN",
    "aux":          "AUX",
    "punct":        "PUNCT",
}

WIKT_REL_TO_ELLE = {
    "synonyms":  "SYNONYM",
    "antonyms":  "ANTONYM",
    "hypernyms": "HYPERNYM",
    "hyponyms":  "HYPONYM",
    "meronyms":  "MERONYM",
    "holonyms":  "HOLONYM",
    "related":   "RELATED_CONCEPT",
}

NORM_RE = re.compile(r"[^a-z0-9'\-]+")


def normalize(s: str) -> str:
    return NORM_RE.sub("", s.lower().strip())


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(
        description="Augment Elle lexicon with Wiktionary (kaikki.org JSONL) data.")
    ap.add_argument("--dump",
                    default=str(DATA_DIR / "kaikki-en.jsonl"),
                    help="Path to kaikki English JSONL dump (.jsonl or .jsonl.gz)")
    ap.add_argument("--output-dir", default=str(OUTPUT_DIR))
    ap.add_argument("--words-csv",  default=str(OUTPUT_DIR / "words.csv"))
    ap.add_argument("--senses-csv", default=str(OUTPUT_DIR / "senses.csv"))
    ap.add_argument("--max-entries", type=int, default=0,
                    help="If >0, cap the number of dump entries processed.")
    return ap.parse_args()


def existing_words(words_csv: Path) -> set[str]:
    out: set[str] = set()
    if not words_csv.exists():
        return out
    with open(words_csv, "r", encoding="utf-8", newline="") as fp:
        for row in csv.DictReader(fp):
            n = row.get("NormalizedLemma", "").strip().lower()
            if n:
                out.add(n)
    return out


def existing_senses(senses_csv: Path) -> set[tuple[str, str]]:
    out: set[tuple[str, str]] = set()
    if not senses_csv.exists():
        return out
    with open(senses_csv, "r", encoding="utf-8", newline="") as fp:
        for row in csv.DictReader(fp):
            n = row.get("NormalizedLemma", "").strip().lower()
            d = row.get("Definition", "").strip().lower()
            if n and d:
                out.add((n, d[:200]))
    return out


def open_dump(path: Path):
    if not path.exists():
        sys.stderr.write(f"ERROR: Wiktionary dump not found at {path}.\n")
        sys.stderr.write("Download from https://kaikki.org/dictionary/English/ "
                         "(kaikki.org-dictionary-English.jsonl[.gz]) and place "
                         "under Tools/ETL/data/.\n")
        sys.exit(2)
    if str(path).endswith(".gz"):
        return gzip.open(path, "rt", encoding="utf-8")
    return open(path, "r", encoding="utf-8")


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    known_words   = existing_words(Path(args.words_csv))
    known_senses  = existing_senses(Path(args.senses_csv))
    sys.stdout.write(
        f"[wiktionary] starting: {len(known_words)} known words, "
        f"{len(known_senses)} known senses\n")

    words_out     = out_dir / "words_wikt.csv"
    senses_out    = out_dir / "senses_wikt.csv"
    relations_out = out_dir / "word_relations_wikt.csv"

    new_words: dict[str, tuple[str, str]] = {}
    new_senses_rows: list[list[str]] = []
    new_relations:   list[list[str]] = []

    processed = 0
    with open_dump(Path(args.dump)) as fp, \
         open(senses_out, "w", encoding="utf-8", newline="") as sf:
        sw = csv.writer(sf)
        sw.writerow(["NormalizedLemma", "PosCode", "SenseOrder", "Definition",
                     "Gloss", "PositiveDraw", "NegativeDraw", "Valence",
                     "Frequency", "SourceTag"])

        for line in fp:
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except json.JSONDecodeError:
                continue

            processed += 1
            if args.max_entries and processed > args.max_entries:
                break

            word = rec.get("word", "").strip()
            if not word:
                continue
            norm = normalize(word)
            if not norm:
                continue
            pos_raw = rec.get("pos", "").lower()
            pos     = WIKT_POS_TO_ELLE.get(pos_raw, "OTHER")

            if norm not in known_words and norm not in new_words:
                new_words[norm] = (word, pos)

            senses = rec.get("senses", []) or []
            for idx, sense in enumerate(senses):
                glosses = sense.get("glosses", []) or []
                if not glosses:
                    continue
                definition = " ".join(g.strip() for g in glosses if g)
                if not definition:
                    continue
                key = (norm, definition.lower()[:200])
                if key in known_senses:
                    continue
                known_senses.add(key)
                tag = f"WIKT:{norm}:{pos.lower()}:{idx}"
                sw.writerow([norm, pos, idx, definition, definition,
                             "0.0", "0.0", "0.0", 0, tag])
                new_senses_rows.append([tag])

            for k, rel in WIKT_REL_TO_ELLE.items():
                for entry in rec.get(k, []) or []:
                    target = entry.get("word") if isinstance(entry, dict) else None
                    if not target:
                        continue
                    n2 = normalize(target)
                    if not n2 or n2 == norm:
                        continue
                    new_relations.append([norm, n2, rel, "0.5"])

    with open(words_out, "w", encoding="utf-8", newline="") as wf:
        ww = csv.writer(wf)
        ww.writerow(["Lemma", "NormalizedLemma", "IsPalindrome", "Frequency", "SourceTag"])
        for norm, (lemma, _pos) in new_words.items():
            palindrome = "1" if norm == norm[::-1] and len(norm) > 1 else "0"
            ww.writerow([lemma, norm, palindrome, 0, f"WIKT:lemma:{norm}"])

    with open(relations_out, "w", encoding="utf-8", newline="") as rf:
        rw = csv.writer(rf)
        rw.writerow(["FromNormalizedLemma", "ToNormalizedLemma",
                     "RelationCode", "Strength"])
        seen: set[tuple[str, str, str]] = set()
        for from_n, to_n, rel, strength in new_relations:
            k = (from_n, to_n, rel)
            if k in seen:
                continue
            seen.add(k)
            rw.writerow([from_n, to_n, rel, strength])

    sys.stdout.write(
        f"[wiktionary] processed {processed} entries — "
        f"{len(new_words)} new words, {len(new_senses_rows)} new senses, "
        f"{len(new_relations)} relation rows (deduped on write).\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
