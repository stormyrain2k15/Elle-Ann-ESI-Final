from __future__ import annotations

import argparse
import csv
import sys
from collections import defaultdict
from pathlib import Path

KNOWN_POS = {"NOUN", "VERB", "ADJ", "ADV", "PRON", "DET", "PREP",
             "CONJ", "INTERJ", "AUX", "PROPN", "NUM", "PUNCT", "OTHER"}
KNOWN_RELATIONS = {"SYNONYM", "ANTONYM", "HOMONYM", "HOMOPHONE",
                   "HOMOGRAPH", "HETERONYM", "PARAPHRASE", "HYPERNYM",
                   "HYPONYM", "MERONYM", "HOLONYM", "RELATED_CONCEPT",
                   "CONTRAST", "CAUSE", "EFFECT"}
KNOWN_EMOTIONS = {"VALENCE", "ANGER", "FEAR", "SADNESS", "JOY", "TRUST",
                  "TENDERNESS", "COMFORT", "SHAME", "CURIOSITY",
                  "POS_DRAW", "NEG_DRAW"}

def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser()
    ap.add_argument("--input-dir",
                    default=str(Path(__file__).resolve().parent / "output"))
    return ap.parse_args()

def in_range(v: str) -> bool:
    try:
        f = float(v)
        return -1.0 <= f <= 1.0
    except (ValueError, TypeError):
        return False

def read_csv(path: Path):
    with open(path, "r", encoding="utf-8", newline="") as fp:
        for r in csv.DictReader(fp):
            yield r

class Validator:
    def __init__(self, dir_: Path):
        self.dir = dir_
        self.errors: list[str] = []
        self.warnings: list[str] = []
        self.word_norms: set[str] = set()
        self.phrase_norms: set[str] = set()
        self.phrase_tags: set[str] = set()
        self.sense_tags: set[str] = set()

    def err(self, msg: str):
        self.errors.append(msg)

    def warn(self, msg: str):
        self.warnings.append(msg)

    def check_words(self):
        path = self.dir / "words.csv"
        if not path.exists():
            self.err(f"missing required file: {path.name}")
            return
        for i, r in enumerate(read_csv(path), 2):
            norm = r.get("NormalizedLemma", "").strip()
            if not norm:
                self.err(f"words.csv:{i}: NormalizedLemma empty")
                continue
            if " " in norm:
                self.warn(f"words.csv:{i}: NormalizedLemma contains space "
                          f"(should be in phrases.csv): {norm!r}")
            self.word_norms.add(norm)
        print(f"[check] words.csv          : {len(self.word_norms):>8,} unique lemmas")

    def check_phrases(self):
        path = self.dir / "phrases.csv"
        pw_path = self.dir / "phrase_words.csv"
        if not path.exists() or not pw_path.exists():
            self.err("phrases.csv or phrase_words.csv missing")
            return
        word_counts_by_tag: dict[str, int] = {}
        for i, r in enumerate(read_csv(path), 2):
            tag = r.get("SourceTag", "")
            norm = r.get("NormalizedForm", "").strip()
            if not tag or not norm:
                self.err(f"phrases.csv:{i}: SourceTag/NormalizedForm empty")
                continue
            if tag in self.phrase_tags:
                self.err(f"phrases.csv:{i}: duplicate SourceTag {tag!r}")
            self.phrase_tags.add(tag)
            self.phrase_norms.add(norm)
            try:
                word_counts_by_tag[tag] = int(r.get("WordCount", 0))
            except ValueError:
                self.err(f"phrases.csv:{i}: non-int WordCount {r.get('WordCount')!r}")

        actual_counts: dict[str, int] = defaultdict(int)
        for i, r in enumerate(read_csv(pw_path), 2):
            tag = r.get("SourceTag", "")
            if tag not in self.phrase_tags:
                self.err(f"phrase_words.csv:{i}: unknown phrase SourceTag {tag!r}")
            actual_counts[tag] += 1
        for tag, expected in word_counts_by_tag.items():
            if actual_counts.get(tag, 0) != expected:
                self.warn(f"phrase WordCount mismatch for {tag}: "
                          f"declared {expected}, got {actual_counts.get(tag, 0)}")
        print(f"[check] phrases.csv        : {len(self.phrase_tags):>8,} phrases")

    def check_senses(self):
        path = self.dir / "senses.csv"
        if not path.exists():
            self.err(f"missing required file: {path.name}")
            return
        unresolved = 0
        for i, r in enumerate(read_csv(path), 2):
            tag = r.get("SourceTag", "")
            norm = r.get("NormalizedLemma", "").strip()
            pos = r.get("PosCode", "").strip() or None
            if not tag or not norm:
                self.err(f"senses.csv:{i}: SourceTag/NormalizedLemma empty")
                continue
            if norm not in self.word_norms:
                unresolved += 1
                if unresolved <= 5:
                    self.warn(f"senses.csv:{i}: NormalizedLemma {norm!r} "
                              f"not in words.csv (FK orphan)")
            if pos and pos not in KNOWN_POS:
                self.err(f"senses.csv:{i}: unknown PosCode {pos!r}")
            for col in ("PositiveDraw", "NegativeDraw", "Valence"):
                v = r.get(col, "")
                if v not in ("", None) and not in_range(v):
                    self.err(f"senses.csv:{i}: {col}={v!r} out of [-1,1]")
            if tag in self.sense_tags:
                self.err(f"senses.csv:{i}: duplicate SourceTag {tag!r}")
            self.sense_tags.add(tag)
        if unresolved > 5:
            self.warn(f"senses.csv: {unresolved} senses had orphan NormalizedLemma "
                      f"(first 5 shown above)")
        print(f"[check] senses.csv         : {len(self.sense_tags):>8,} senses")

    def check_examples(self):
        usages_per_sense: dict[str, set[int]] = defaultdict(set)
        contexts_per_sense: dict[str, set[int]] = defaultdict(set)
        for label, fname, target, slot_field, text_field in [
            ("usage",   "sense_usage_examples.csv",   usages_per_sense,
             "Slot", "ExampleText"),
            ("context", "sense_context_examples.csv", contexts_per_sense,
             "Slot", "ContextText"),
        ]:
            path = self.dir / fname
            if not path.exists():
                self.err(f"missing required file: {fname}")
                continue
            for i, r in enumerate(read_csv(path), 2):
                tag = r.get("SourceTag", "")
                if tag not in self.sense_tags:

                    continue
                try:
                    s = int(r.get(slot_field, 0))
                except ValueError:
                    self.err(f"{fname}:{i}: non-int Slot")
                    continue
                if s not in (1, 2):
                    self.err(f"{fname}:{i}: Slot must be 1 or 2, got {s}")
                if not r.get(text_field, "").strip():
                    self.warn(f"{fname}:{i}: {text_field} empty")
                target[tag].add(s)

        partial_u = sum(1 for v in usages_per_sense.values()   if len(v) < 2)
        partial_c = sum(1 for v in contexts_per_sense.values() if len(v) < 2)
        no_u      = len(self.sense_tags) - len(usages_per_sense)
        no_c      = len(self.sense_tags) - len(contexts_per_sense)
        if partial_u: self.warn(f"{partial_u:,} senses have fewer than 2 usage examples")
        if partial_c: self.warn(f"{partial_c:,} senses have fewer than 2 context examples")
        if no_u:      self.warn(f"{no_u:,} senses have NO usage examples")
        if no_c:      self.warn(f"{no_c:,} senses have NO context examples")
        print(f"[check] usage examples     : {sum(len(v) for v in usages_per_sense.values()):>8,}")
        print(f"[check] context examples   : {sum(len(v) for v in contexts_per_sense.values()):>8,}")

    def check_emotions(self):
        path = self.dir / "sense_emotions.csv"
        if not path.exists():
            self.warn("sense_emotions.csv missing (engine will work but with zero emotion data)")
            return
        n = 0
        for i, r in enumerate(read_csv(path), 2):
            tag = r.get("SourceTag", "")
            code = r.get("EmotionCode", "")
            w = r.get("Weight", "")
            if tag not in self.sense_tags:
                continue
            if code not in KNOWN_EMOTIONS:
                self.err(f"sense_emotions.csv:{i}: unknown EmotionCode {code!r}")
                continue
            if not in_range(w):
                self.err(f"sense_emotions.csv:{i}: Weight={w!r} out of [-1,1]")
            n += 1
        print(f"[check] sense_emotions.csv : {n:>8,} rows")

    def check_relations(self):

        path = self.dir / "sense_relations.csv"
        if path.exists():
            n = bad = 0
            for i, r in enumerate(read_csv(path), 2):
                code = r.get("RelationCode", "")
                if code not in KNOWN_RELATIONS:
                    self.err(f"sense_relations.csv:{i}: unknown RelationCode {code!r}")
                    bad += 1
                    if bad > 10: break
                n += 1
            print(f"[check] sense_relations    : {n:>8,} rows")

        path = self.dir / "word_relations.csv"
        if path.exists():
            n = 0
            unresolved = 0
            for i, r in enumerate(read_csv(path), 2):
                code = r.get("RelationCode", "")
                if code not in KNOWN_RELATIONS:
                    self.err(f"word_relations.csv:{i}: unknown RelationCode {code!r}")
                    break
                fa = r.get("FromNormalizedLemma", "")
                fb = r.get("ToNormalizedLemma", "")
                if fa not in self.word_norms or fb not in self.word_norms:
                    unresolved += 1
                n += 1
            if unresolved:
                self.warn(f"word_relations.csv: {unresolved:,} rows with unresolved lemmas")
            print(f"[check] word_relations     : {n:>8,} rows")

    def check_concepts(self):
        path = self.dir / "concepts.csv"
        if path.exists():
            labels = set()
            for i, r in enumerate(read_csv(path), 2):
                l = r.get("ConceptLabel", "")
                if not l:
                    self.err(f"concepts.csv:{i}: empty ConceptLabel")
                    continue
                if l in labels:
                    self.err(f"concepts.csv:{i}: duplicate ConceptLabel {l!r}")
                labels.add(l)
            print(f"[check] concepts.csv       : {len(labels):>8,} unique labels")
            self._concept_labels = labels
        path = self.dir / "concept_members.csv"
        if path.exists():
            n = 0
            unresolved = 0
            for i, r in enumerate(read_csv(path), 2):
                l = r.get("ConceptLabel", "")
                t = r.get("SourceTag", "")
                if l not in getattr(self, "_concept_labels", set()):
                    unresolved += 1
                    continue
                if t not in self.sense_tags:
                    continue
                n += 1
            if unresolved:
                self.warn(f"concept_members.csv: {unresolved:,} rows reference "
                          f"unknown concept labels")
            print(f"[check] concept_members    : {n:>8,} matched rows")

    def report(self) -> int:
        print()
        for w in self.warnings:
            print(f"  WARN: {w}")
        for e in self.errors:
            print(f"  ERR : {e}")
        print()
        print(f"[summary] {len(self.errors)} errors, {len(self.warnings)} warnings")
        return 1 if self.errors else 0

def main() -> int:
    args = parse_args()
    d = Path(args.input_dir)
    if not d.exists():
        sys.exit(f"input dir not found: {d}")
    v = Validator(d)
    v.check_words()
    v.check_phrases()
    v.check_senses()
    v.check_examples()
    v.check_emotions()
    v.check_relations()
    v.check_concepts()
    return v.report()

if __name__ == "__main__":
    sys.exit(main())
