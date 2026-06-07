from __future__ import annotations

import argparse
import csv
import sys
from collections import defaultdict
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parents[1] / "output"
DATA_DIR   = Path(__file__).resolve().parents[1] / "data"

NRC_TO_ELLE = {
    "anger":        "ANGER",
    "fear":         "FEAR",
    "sadness":      "SADNESS",
    "joy":          "JOY",
    "trust":        "TRUST",
    "anticipation": "CURIOSITY",
    "disgust":      "ANGER",
    "surprise":     "CURIOSITY",
    "positive":     "POS_DRAW",
    "negative":     "NEG_DRAW",
}

ELLE_EMOTION_CODES = {
    "VALENCE", "ANGER", "FEAR", "SADNESS", "JOY", "TRUST",
    "TENDERNESS", "COMFORT", "SHAME", "CURIOSITY", "POS_DRAW", "NEG_DRAW",
}

DEFAULT_WEIGHT = 0.45


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(
        description="Build sense_emotions augmentation from NRC EmoLex.")
    ap.add_argument("--lexicon",
                    default=str(DATA_DIR / "NRC-Emotion-Lexicon-Wordlevel-v0.92.txt"))
    ap.add_argument("--output-dir", default=str(OUTPUT_DIR))
    ap.add_argument("--senses-csv",
                    default=str(OUTPUT_DIR / "senses.csv"))
    ap.add_argument("--weight", type=float, default=DEFAULT_WEIGHT)
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
        sys.stderr.write(f"ERROR: lexicon file not found at {path}.\n")
        sys.stderr.write("Download NRC-Emotion-Lexicon-Wordlevel-v0.92.txt from "
                         "https://saifmohammad.com/WebPages/NRC-Emotion-Lexicon.htm "
                         "and place it under Tools/ETL/data/.\n")
        sys.exit(2)
    with open(path, "r", encoding="utf-8") as fp:
        for line in fp:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) != 3:
                continue
            word, emotion, assoc = parts[0].lower(), parts[1].lower(), parts[2]
            if assoc != "1":
                continue
            elle = NRC_TO_ELLE.get(emotion)
            if not elle:
                continue
            yield word, elle


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    sense_index = load_sense_index(Path(args.senses_csv))
    out_path = out_dir / "sense_emotions_nrc.csv"

    written = 0
    unmatched: set[str] = set()
    seen_pair: set[tuple[str, str]] = set()

    with open(out_path, "w", encoding="utf-8", newline="") as fp:
        w = csv.writer(fp)
        w.writerow(["SourceTag", "EmotionCode", "Weight"])
        for word, elle_code in iter_lexicon(Path(args.lexicon)):
            tags = sense_index.get(word, [])
            if not tags:
                unmatched.add(word)
                continue
            if elle_code not in ELLE_EMOTION_CODES:
                continue
            for tag in tags:
                key = (tag, elle_code)
                if key in seen_pair:
                    continue
                seen_pair.add(key)
                w.writerow([tag, elle_code, f"{args.weight:.2f}"])
                written += 1

    sys.stdout.write(
        f"[nrc_emolex] wrote {written} sense_emotions_nrc.csv rows; "
        f"{len(unmatched)} lexicon words had no matching WordNet sense.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
