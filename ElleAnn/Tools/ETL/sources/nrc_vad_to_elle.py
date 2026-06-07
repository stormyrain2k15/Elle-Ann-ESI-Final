from __future__ import annotations

import argparse
import csv
import sys
from collections import defaultdict
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parents[1] / "output"
DATA_DIR   = Path(__file__).resolve().parents[1] / "data"


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(
        description="Build sense valence overrides from NRC VAD lexicon.")
    ap.add_argument("--lexicon",
                    default=str(DATA_DIR / "NRC-VAD-Lexicon.txt"))
    ap.add_argument("--output-dir", default=str(OUTPUT_DIR))
    ap.add_argument("--senses-csv",
                    default=str(OUTPUT_DIR / "senses.csv"))
    return ap.parse_args()


def load_sense_index(senses_csv: Path) -> dict[str, list[str]]:
    by_lemma: dict[str, list[str]] = defaultdict(list)
    if not senses_csv.exists():
        sys.stderr.write(
            f"ERROR: {senses_csv} does not exist — run wordnet_to_elle.py first.\n")
        sys.exit(2)
    with open(senses_csv, "r", encoding="utf-8", newline="") as fp:
        for row in csv.DictReader(fp):
            lemma = row["NormalizedLemma"].strip().lower()
            tag   = row["SourceTag"].strip()
            if lemma and tag:
                by_lemma[lemma].append(tag)
    return by_lemma


def iter_lexicon(path: Path):
    if not path.exists():
        sys.stderr.write(f"ERROR: VAD lexicon not found at {path}.\n")
        sys.stderr.write("Download NRC-VAD-Lexicon.txt from "
                         "https://saifmohammad.com/WebPages/nrc-vad.html "
                         "and place it under Tools/ETL/data/.\n")
        sys.exit(2)
    with open(path, "r", encoding="utf-8") as fp:
        first = True
        for line in fp:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 4:
                continue
            if first:
                first = False
                try:
                    float(parts[1])
                except ValueError:
                    continue
            try:
                word    = parts[0].strip().lower()
                valence = float(parts[1])
                arousal = float(parts[2])
                dom     = float(parts[3])
            except ValueError:
                continue
            yield word, valence, arousal, dom


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    sense_index = load_sense_index(Path(args.senses_csv))
    out_path    = out_dir / "sense_valence_vad.csv"

    written  = 0
    unmatched: set[str] = set()

    with open(out_path, "w", encoding="utf-8", newline="") as fp:
        w = csv.writer(fp)
        w.writerow(["SourceTag", "Valence", "PositiveDraw", "NegativeDraw", "Arousal", "Dominance"])
        for word, valence, arousal, dom in iter_lexicon(Path(args.lexicon)):
            tags = sense_index.get(word, [])
            if not tags:
                unmatched.add(word)
                continue
            centered = max(-1.0, min(1.0, (valence - 0.5) * 2.0))
            pos_draw = max(0.0, centered)
            neg_draw = max(0.0, -centered)
            for tag in tags:
                w.writerow([
                    tag,
                    f"{centered:.4f}",
                    f"{pos_draw:.4f}",
                    f"{neg_draw:.4f}",
                    f"{arousal:.4f}",
                    f"{dom:.4f}",
                ])
                written += 1

    sys.stdout.write(
        f"[nrc_vad] wrote {written} sense_valence_vad.csv rows; "
        f"{len(unmatched)} VAD words had no matching WordNet sense.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
