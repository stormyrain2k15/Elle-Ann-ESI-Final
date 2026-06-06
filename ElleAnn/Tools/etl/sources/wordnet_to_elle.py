"""
WordNet → Elle Semantic Dictionary ETL
======================================

Generates UTF-8 CSV files in `output/` that match the canonical
`elle-language/sql/01_schema.sql` schema.  The CSVs are designed to be
loaded into SQL Server via the staging-table procs in
`09_validation_and_loaders.sql` (Word, Sense, Phrase) plus direct
BULK INSERTs for the example / emotion / relation tables.

Output files (UTF-8, header row, no BOM):

    output/words.csv
        Lemma, NormalizedLemma, IsPalindrome, Frequency, SourceTag

    output/senses.csv
        NormalizedLemma, PosCode, SenseOrder, Definition, Gloss,
        PositiveDraw, NegativeDraw, Valence, Frequency, SourceTag

    output/sense_usage_examples.csv
        SourceTag, Slot, ExampleText

    output/sense_context_examples.csv
        SourceTag, Slot, ContextText

    output/sense_emotions.csv
        SourceTag, EmotionCode, Weight

    output/sense_relations.csv
        FromSourceTag, ToSourceTag, RelationCode, Strength

    output/word_relations.csv
        FromNormalizedLemma, ToNormalizedLemma, RelationCode, Strength

    output/concepts.csv
        ConceptLabel, Description

    output/concept_members.csv
        ConceptLabel, SourceTag, Strength      -- SourceTag refers to senses

    output/phrases.csv
        Surface, NormalizedForm, WordCount, Frequency, SourceTag

    output/phrase_words.csv
        SourceTag, Position, SurfaceText, NormalizedLemma

    output/phrase_senses.csv
        SourceTag, PhraseSourceTag, SenseOrder, Definition, Gloss,
        PositiveDraw, NegativeDraw, Valence, Frequency

`SourceTag` is the stable identity we use to wire things together
*across* CSV files before the database assigns the real BIGINT IDs.
For WordNet senses it is the synset name (e.g. `dog.n.01`).
"""

from __future__ import annotations

import csv
import re
import string
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, Set, Tuple

try:
    import nltk
    from nltk.corpus import wordnet as wn
    from tqdm import tqdm
except ImportError as e:
    sys.stderr.write(f"Missing dependency: {e}. Run: pip install nltk tqdm\n")
    sys.exit(2)

# Make sure WordNet is downloaded (quiet, idempotent).
for pkg in ("wordnet", "omw-1.4"):
    try:
        nltk.data.find(f"corpora/{pkg}")
    except LookupError:
        nltk.download(pkg, quiet=True)

# --------------------------------------------------------------------------- #
# Paths + constants                                                           #
# --------------------------------------------------------------------------- #
OUTPUT_DIR = Path(__file__).resolve().parents[1] / "output"
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# WordNet POS → Elle PartOfSpeech.Code
POS_MAP = {
    wn.NOUN: "NOUN",
    wn.VERB: "VERB",
    wn.ADJ:  "ADJ",
    wn.ADJ_SAT: "ADJ",
    wn.ADV:  "ADV",
}

# Minimal emotion lexicon: words → (EmotionCode, weight).
# Wired into SenseEmotion so the engine has *some* signal beyond zero
# until NRC-EmoLex / VAD lexicon is ingested.  Weights are intentionally
# small (≤ 0.6) so they get refined, not dominated.
EMOTION_LEX: Dict[str, Tuple[str, float]] = {
    # ---- POSITIVE / JOY ----
    "happy":    ("JOY", 0.6),
    "joy":      ("JOY", 0.6),
    "joyful":   ("JOY", 0.55),
    "delight":  ("JOY", 0.5),
    "love":     ("JOY", 0.5),
    "pleasure": ("JOY", 0.4),
    "smile":    ("JOY", 0.3),
    # ---- COMFORT / TENDERNESS ----
    "comfort":  ("COMFORT", 0.5),
    "soothe":   ("COMFORT", 0.45),
    "warm":     ("TENDERNESS", 0.35),
    "gentle":   ("TENDERNESS", 0.4),
    "kind":     ("TENDERNESS", 0.4),
    # ---- ANGER ----
    "anger":    ("ANGER", 0.6),
    "angry":    ("ANGER", 0.6),
    "rage":     ("ANGER", 0.6),
    "furious":  ("ANGER", 0.55),
    "hate":     ("ANGER", 0.5),
    # ---- FEAR ----
    "fear":     ("FEAR", 0.6),
    "afraid":   ("FEAR", 0.55),
    "scared":   ("FEAR", 0.5),
    "terror":   ("FEAR", 0.6),
    "anxious":  ("FEAR", 0.4),
    # ---- SADNESS ----
    "sad":      ("SADNESS", 0.6),
    "grief":    ("SADNESS", 0.55),
    "sorrow":   ("SADNESS", 0.55),
    "cry":      ("SADNESS", 0.4),
    "weep":     ("SADNESS", 0.4),
    "loss":     ("SADNESS", 0.3),
    # ---- SHAME ----
    "shame":    ("SHAME", 0.55),
    "guilt":    ("SHAME", 0.5),
    "embarrass":("SHAME", 0.4),
    # ---- CURIOSITY ----
    "curious":  ("CURIOSITY", 0.5),
    "wonder":   ("CURIOSITY", 0.4),
    "explore":  ("CURIOSITY", 0.35),
    # ---- TRUST ----
    "trust":    ("TRUST", 0.6),
    "honest":   ("TRUST", 0.5),
    "loyal":    ("TRUST", 0.5),
    "faithful": ("TRUST", 0.4),
    # ---- NEGATIVE DRAW (catch-all) ----
    "bad":      ("NEG_DRAW", 0.4),
    "evil":     ("NEG_DRAW", 0.55),
    "wrong":    ("NEG_DRAW", 0.35),
    "harm":     ("NEG_DRAW", 0.5),
    # ---- POSITIVE DRAW (catch-all) ----
    "good":     ("POS_DRAW", 0.4),
    "great":    ("POS_DRAW", 0.4),
    "right":    ("POS_DRAW", 0.3),
    "help":     ("POS_DRAW", 0.4),
}

# Lemmas we deliberately skip from WordNet's "_" suffix
# (these come back to bite the engine: bad heuristics, super-rare entries).
_SKIP_LEMMA_PREFIXES = ("'",)

# --------------------------------------------------------------------------- #
# Helpers                                                                     #
# --------------------------------------------------------------------------- #

_NORM_TRANSLATE = str.maketrans({c: " " for c in string.punctuation if c not in "-'"})


def normalize(s: str) -> str:
    """Lower, strip, collapse whitespace, keep hyphens/apostrophes."""
    s = s.replace("_", " ").lower().strip()
    s = s.translate(_NORM_TRANSLATE)
    s = re.sub(r"\s+", " ", s)
    return s.strip()


def is_palindrome(s: str) -> bool:
    t = re.sub(r"[^a-z0-9]", "", s.lower())
    return len(t) > 1 and t == t[::-1]


def derive_valence(definition: str) -> Tuple[float, float, float]:
    """Cheap valence / pos-draw / neg-draw from definition tokens.
    Intentionally simple; the probability engine refines these online."""
    tokens = re.findall(r"[a-z']+", definition.lower())
    pos = 0
    neg = 0
    for t in tokens:
        if t in EMOTION_LEX:
            code, w = EMOTION_LEX[t]
            if code in ("JOY", "COMFORT", "TENDERNESS", "TRUST", "CURIOSITY",
                        "POS_DRAW"):
                pos += w
            elif code in ("ANGER", "FEAR", "SADNESS", "SHAME", "NEG_DRAW"):
                neg += w
    total = pos + neg
    if total < 1e-6:
        return 0.0, 0.0, 0.0
    valence = max(min((pos - neg) / total, 1.0), -1.0)
    pos_draw = min(pos, 1.0)
    neg_draw = min(neg, 1.0)
    return round(pos_draw, 4), round(neg_draw, 4), round(valence, 4)


def emotion_signals(definition: str) -> Dict[str, float]:
    """Aggregate emotion code → weight from definition tokens."""
    agg: Dict[str, float] = defaultdict(float)
    for t in re.findall(r"[a-z']+", definition.lower()):
        if t in EMOTION_LEX:
            code, w = EMOTION_LEX[t]
            agg[code] = max(agg[code], w)  # take max, not sum (saturates)
    return agg


# --------------------------------------------------------------------------- #
# CSV writers                                                                 #
# --------------------------------------------------------------------------- #


class CsvWriter:
    """Thin context manager: opens file, writes header on enter, exposes writerow."""

    def __init__(self, path: Path, header: list[str]):
        self.path = path
        self.header = header
        self._fp = None
        self._writer = None
        self.rows = 0

    def __enter__(self):
        self._fp = open(self.path, "w", encoding="utf-8", newline="")
        self._writer = csv.writer(self._fp, quoting=csv.QUOTE_MINIMAL)
        self._writer.writerow(self.header)
        return self

    def writerow(self, row):
        self._writer.writerow(row)
        self.rows += 1

    def __exit__(self, *a):
        self._fp.close()


# --------------------------------------------------------------------------- #
# Main ETL                                                                    #
# --------------------------------------------------------------------------- #


def run() -> None:
    print(f"[etl] output dir: {OUTPUT_DIR}")

    # ------------------------------------------------------------------ #
    # Pass 1: collect unique lemmas (single-word) and synset definitions #
    # ------------------------------------------------------------------ #
    print("[etl] scanning WordNet synsets ...")
    all_synsets = list(wn.all_synsets())
    print(f"[etl]   {len(all_synsets):,} synsets")

    unique_lemmas: Dict[str, str] = {}  # normalized -> first-seen surface
    # normalized phrase form -> first-seen surface (collapses duplicates from
    # multiple synsets sharing the same phrase, e.g. "post office" senses).
    multi_word_phrases: Dict[str, str] = {}

    for ss in all_synsets:
        for lemma in ss.lemmas():
            name = lemma.name().replace("_", " ")
            if name.startswith(_SKIP_LEMMA_PREFIXES):
                continue
            norm = normalize(name)
            if not norm:
                continue
            if " " in norm:
                if norm not in multi_word_phrases:
                    multi_word_phrases[norm] = name
            elif norm not in unique_lemmas:
                unique_lemmas[norm] = name

    print(f"[etl]   {len(unique_lemmas):,} unique single-word lemmas")
    print(f"[etl]   {len(multi_word_phrases):,} multi-word phrases")

    # ------------------------------------------------------------------ #
    # Pass 2: write words.csv + (for phrases) phrases.csv / phrase_words #
    # ------------------------------------------------------------------ #
    print("[etl] writing words.csv ...")
    with CsvWriter(OUTPUT_DIR / "words.csv",
                   ["Lemma", "NormalizedLemma", "IsPalindrome",
                    "Frequency", "SourceTag"]) as wcsv:
        for norm, surface in sorted(unique_lemmas.items()):
            wcsv.writerow([
                surface,
                norm,
                1 if is_palindrome(norm) else 0,
                0,
                f"WN:lemma:{norm}",
            ])

    print("[etl] writing phrases.csv + phrase_words.csv ...")
    with CsvWriter(OUTPUT_DIR / "phrases.csv",
                   ["Surface", "NormalizedForm", "WordCount",
                    "Frequency", "SourceTag"]) as pcsv, \
         CsvWriter(OUTPUT_DIR / "phrase_words.csv",
                   ["SourceTag", "Position", "SurfaceText",
                    "NormalizedLemma"]) as pwcsv:
        for norm, surface in sorted(multi_word_phrases.items()):
            tag = f"WN:phrase:{norm}"
            tokens = norm.split(" ")
            pcsv.writerow([surface, norm, len(tokens), 0, tag])
            for pos, tok in enumerate(tokens):
                pwcsv.writerow([tag, pos, tok, tok])

    # ------------------------------------------------------------------ #
    # Pass 3: senses + examples + emotions + concept memberships         #
    # ------------------------------------------------------------------ #
    print("[etl] writing senses.csv + sense_*.csv + concepts.csv ...")

    # senseorder per (normalized lemma, pos) so engine sense ordering is stable
    sense_order_counter: Dict[Tuple[str, str], int] = defaultdict(int)
    # concept label -> set of sense source tags
    concept_to_senses: Dict[str, Set[str]] = defaultdict(set)

    with CsvWriter(OUTPUT_DIR / "senses.csv",
                   ["NormalizedLemma", "PosCode", "SenseOrder",
                    "Definition", "Gloss",
                    "PositiveDraw", "NegativeDraw", "Valence",
                    "Frequency", "SourceTag"]) as scsv, \
         CsvWriter(OUTPUT_DIR / "phrase_senses.csv",
                   ["SourceTag", "PhraseSourceTag", "SenseOrder",
                    "Definition", "Gloss",
                    "PositiveDraw", "NegativeDraw", "Valence",
                    "Frequency"]) as pscsv, \
         CsvWriter(OUTPUT_DIR / "sense_usage_examples.csv",
                   ["SourceTag", "Slot", "ExampleText"]) as uexcsv, \
         CsvWriter(OUTPUT_DIR / "sense_context_examples.csv",
                   ["SourceTag", "Slot", "ContextText"]) as ctxcsv, \
         CsvWriter(OUTPUT_DIR / "sense_emotions.csv",
                   ["SourceTag", "EmotionCode", "Weight"]) as ecsv:

        for ss in tqdm(all_synsets, desc="[etl] synsets"):
            primary_lemma = ss.lemmas()[0].name().replace("_", " ")
            if primary_lemma.startswith(_SKIP_LEMMA_PREFIXES):
                continue
            norm_primary = normalize(primary_lemma)
            if not norm_primary:
                continue

            pos_code = POS_MAP.get(ss.pos(), "OTHER")
            definition = ss.definition() or ""
            gloss = (definition[:253] + "...") if len(definition) > 256 else definition
            pos_draw, neg_draw, valence = derive_valence(definition)
            tag = f"WN:{ss.name()}"

            # ---- Examples (2 usage, 2 context) ---- #
            usages = list(ss.examples())[:2]
            while len(usages) < 2:
                usages.append(f"({primary_lemma}: usage example placeholder)")
            for slot, text in enumerate(usages, 1):
                uexcsv.writerow([tag, slot, text])

            ctx1 = (f"In context, '{primary_lemma}' is used to mean: "
                    f"{definition[:140]}")
            ctx2 = (f"Typical setting for '{primary_lemma}': hypernyms "
                    + ", ".join(h.lemmas()[0].name().replace("_", " ")
                                for h in ss.hypernyms()[:3])
                    + ".")
            ctxcsv.writerow([tag, 1, ctx1])
            ctxcsv.writerow([tag, 2, ctx2])

            # ---- Emotion signals ---- #
            for code, w in emotion_signals(definition).items():
                ecsv.writerow([tag, code, round(w, 4)])

            # ---- Concept membership ---- #
            # Every synset is its own concept (label = synset name).
            concept_to_senses[ss.name()].add(tag)
            # Hypernyms group sister senses together → cross-sense concepts.
            for h in ss.hypernyms():
                concept_to_senses[f"under:{h.name()}"].add(tag)

            # ---- Decide sense vs phrase-sense based on the primary lemma --- #
            if " " in norm_primary:
                # Multi-word lemma → phrase sense, joined to its phrase row.
                phrase_tag = f"WN:phrase:{norm_primary}"
                order = sense_order_counter[(norm_primary, pos_code)]
                sense_order_counter[(norm_primary, pos_code)] += 1
                pscsv.writerow([tag, phrase_tag, order, definition, gloss,
                                pos_draw, neg_draw, valence, 0])
            else:
                order = sense_order_counter[(norm_primary, pos_code)]
                sense_order_counter[(norm_primary, pos_code)] += 1
                scsv.writerow([norm_primary, pos_code, order,
                               definition, gloss,
                               pos_draw, neg_draw, valence, 0, tag])

    # ------------------------------------------------------------------ #
    # Pass 4: relations (synonym/antonym at sense level, hypernym/hyponym
    #         as SemanticRelation-style edges via SenseRelation)
    # ------------------------------------------------------------------ #
    print("[etl] writing sense_relations.csv + word_relations.csv ...")
    with CsvWriter(OUTPUT_DIR / "sense_relations.csv",
                   ["FromSourceTag", "ToSourceTag", "RelationCode", "Strength"]
                   ) as srcsv, \
         CsvWriter(OUTPUT_DIR / "word_relations.csv",
                   ["FromNormalizedLemma", "ToNormalizedLemma",
                    "RelationCode", "Strength"]) as wrcsv:

        synset_tag = lambda ss: f"WN:{ss.name()}"

        for ss in tqdm(all_synsets, desc="[etl] relations"):
            from_tag = synset_tag(ss)
            # Hypernyms
            for hyper in ss.hypernyms():
                srcsv.writerow([from_tag, synset_tag(hyper), "HYPERNYM", 1.0])
            for hypo in ss.hyponyms():
                srcsv.writerow([from_tag, synset_tag(hypo), "HYPONYM", 1.0])
            # Part / whole
            for mero in ss.part_meronyms() + ss.substance_meronyms():
                srcsv.writerow([from_tag, synset_tag(mero), "MERONYM", 1.0])
            for holo in ss.part_holonyms() + ss.substance_holonyms():
                srcsv.writerow([from_tag, synset_tag(holo), "HOLONYM", 1.0])
            # Cause / effect
            for c in ss.causes():
                srcsv.writerow([from_tag, synset_tag(c), "CAUSE", 1.0])

            # Word-level synonyms / antonyms within the synset.
            lemmas_norm = [normalize(l.name().replace("_", " "))
                           for l in ss.lemmas()]
            lemmas_norm = [l for l in lemmas_norm if l and " " not in l]
            for i, a in enumerate(lemmas_norm):
                for b in lemmas_norm[i + 1:]:
                    if a != b:
                        wrcsv.writerow([a, b, "SYNONYM", 1.0])
                        wrcsv.writerow([b, a, "SYNONYM", 1.0])
            for lemma in ss.lemmas():
                a_norm = normalize(lemma.name().replace("_", " "))
                if not a_norm or " " in a_norm:
                    continue
                for ant in lemma.antonyms():
                    b_norm = normalize(ant.name().replace("_", " "))
                    if not b_norm or " " in b_norm:
                        continue
                    wrcsv.writerow([a_norm, b_norm, "ANTONYM", 1.0])

    # ------------------------------------------------------------------ #
    # Pass 5: concepts.csv + concept_members.csv                         #
    # ------------------------------------------------------------------ #
    print("[etl] writing concepts.csv + concept_members.csv ...")
    with CsvWriter(OUTPUT_DIR / "concepts.csv",
                   ["ConceptLabel", "Description"]) as ccsv, \
         CsvWriter(OUTPUT_DIR / "concept_members.csv",
                   ["ConceptLabel", "SourceTag", "Strength"]) as cmcsv:
        for label, members in sorted(concept_to_senses.items()):
            ccsv.writerow([label, f"WordNet-derived concept ({label})"])
            for tag in sorted(members):
                cmcsv.writerow([label, tag, 1.0])

    # ------------------------------------------------------------------ #
    # Done. Print row counts.                                            #
    # ------------------------------------------------------------------ #
    print("[etl] DONE. Output rows:")
    for f in sorted(OUTPUT_DIR.glob("*.csv")):
        try:
            with open(f, "r", encoding="utf-8") as fp:
                n = sum(1 for _ in fp) - 1  # minus header
        except Exception:
            n = -1
        print(f"  {f.name:<32} {n:>10,} rows")


if __name__ == "__main__":
    run()
