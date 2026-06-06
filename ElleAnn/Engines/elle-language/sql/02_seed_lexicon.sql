/* =========================================================================
   Elle Semantic Dictionary -- Seed: lexicon (Words, Forms, Pronunciations)
   File: 02_seed_lexicon.sql

   This is starter data sufficient to make the heartbeat demo and the 13
   required tests pass. Real population will be orders of magnitude larger.
   The hard-coded IDs in this file are referenced by 03..07.
   ========================================================================= */
USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

/* --------- Reference: PartOfSpeech ---------------------------------------- */
SET IDENTITY_INSERT dbo.PartOfSpeech OFF;
MERGE dbo.PartOfSpeech AS T
USING (VALUES
    (1,  N'NOUN',    N'Noun'),
    (2,  N'VERB',    N'Verb'),
    (3,  N'ADJ',     N'Adjective'),
    (4,  N'ADV',     N'Adverb'),
    (5,  N'PRON',    N'Pronoun'),
    (6,  N'DET',     N'Determiner'),
    (7,  N'PREP',    N'Preposition'),
    (8,  N'CONJ',    N'Conjunction'),
    (9,  N'INTERJ',  N'Interjection'),
    (10, N'AUX',     N'Auxiliary'),
    (11, N'PROPN',   N'Proper Noun'),
    (12, N'NUM',     N'Numeral'),
    (13, N'PUNCT',   N'Punctuation'),
    (14, N'OTHER',   N'Other')
) AS S(PartOfSpeechID, Code, Name)
ON T.PartOfSpeechID = S.PartOfSpeechID
WHEN NOT MATCHED THEN INSERT(PartOfSpeechID, Code, Name) VALUES(S.PartOfSpeechID, S.Code, S.Name);

/* --------- Reference: RelationType --------------------------------------- */
MERGE dbo.RelationType AS T
USING (VALUES
    (1,  N'SYNONYM',         N'Synonym',          1),
    (2,  N'ANTONYM',         N'Antonym',          1),
    (3,  N'HOMONYM',         N'Homonym',          1),
    (4,  N'HOMOPHONE',       N'Homophone',        1),
    (5,  N'HOMOGRAPH',       N'Homograph',        1),
    (6,  N'HETERONYM',       N'Heteronym',        1),
    (7,  N'PARAPHRASE',      N'Paraphrase',       1),
    (8,  N'HYPERNYM',        N'Hypernym',         0),
    (9,  N'HYPONYM',         N'Hyponym',          0),
    (10, N'MERONYM',         N'Meronym',          0),
    (11, N'HOLONYM',         N'Holonym',          0),
    (12, N'RELATED_CONCEPT', N'Related Concept',  1),
    (13, N'CONTRAST',        N'Contrast',         1),
    (14, N'CAUSE',           N'Cause',            0),
    (15, N'EFFECT',          N'Effect',           0)
) AS S(RelationTypeID, Code, Name, IsSymmetric)
ON T.RelationTypeID = S.RelationTypeID
WHEN NOT MATCHED THEN INSERT VALUES(S.RelationTypeID, S.Code, S.Name, S.IsSymmetric);

/* --------- Reference: Emotion -------------------------------------------- */
MERGE dbo.Emotion AS T
USING (VALUES
    (1,  N'VALENCE',    N'Valence'),
    (2,  N'ANGER',      N'Anger'),
    (3,  N'FEAR',       N'Fear'),
    (4,  N'SADNESS',    N'Sadness'),
    (5,  N'JOY',        N'Joy'),
    (6,  N'TRUST',      N'Trust'),
    (7,  N'TENDERNESS', N'Tenderness'),
    (8,  N'COMFORT',    N'Comfort'),
    (9,  N'SHAME',      N'Shame'),
    (10, N'CURIOSITY',  N'Curiosity'),
    (11, N'POS_DRAW',   N'Positive Draw'),
    (12, N'NEG_DRAW',   N'Negative Draw')
) AS S(EmotionID, Code, Name)
ON T.EmotionID = S.EmotionID
WHEN NOT MATCHED THEN INSERT VALUES(S.EmotionID, S.Code, S.Name);

/* --------- Words --------------------------------------------------------- */
/* Hard-code WordIDs via SET IDENTITY_INSERT for predictable seeding.       */
SET IDENTITY_INSERT dbo.Word ON;
INSERT INTO dbo.Word (WordID, Lemma, NormalizedLemma, IsPalindrome, Frequency) VALUES
    (1,   N'i',         N'i',         0, 1000000),
    (2,   N'am',        N'am',        0,  900000),
    (3,   N'fine',      N'fine',      0,  400000),
    (4,   N'bat',       N'bat',       0,   50000),
    (5,   N'there',     N'there',     0,  300000),
    (6,   N'their',     N'their',     0,  280000),
    (7,   N'they',      N'they',      0,  290000),
    (8,   N'are',       N'are',       0,  800000),
    (9,   N'okay',      N'okay',      0,  200000),
    (10,  N'happy',     N'happy',     0,  150000),
    (11,  N'sad',       N'sad',       0,  120000),
    (12,  N'angry',     N'angry',     0,  110000),
    (13,  N'glad',      N'glad',      0,   80000),
    (14,  N'worry',     N'worry',     0,   60000),
    (15,  N'do',        N'do',        0,  500000),
    (16,  N'not',       N'not',       0,  600000),
    (17,  N'about',     N'about',     0,  400000),
    (18,  N'me',        N'me',        0,  500000),
    (19,  N'leave',     N'leave',     0,   90000),
    (20,  N'alone',     N'alone',     0,   70000),
    (21,  N'racecar',   N'racecar',   1,    5000),  -- palindrome flag demo
    (22,  N'baseball',  N'baseball',  0,   30000),
    (23,  N'animal',    N'animal',    0,   80000),
    (24,  N'mammal',    N'mammal',    0,   30000),
    (25,  N'sport',     N'sport',     0,   60000);
SET IDENTITY_INSERT dbo.Word OFF;

/* --------- WordForms (inflections) -------------------------------------- */
SET IDENTITY_INSERT dbo.WordForm ON;
INSERT INTO dbo.WordForm (WordFormID, WordID, Form, NormalizedForm, PartOfSpeechID, InflectionTag) VALUES
    (1,  1,  N'I',       N'i',       5,  N'NOM'),
    (2,  2,  N'am',      N'am',      10, N'1SG.PRES'),
    (3,  3,  N'fine',    N'fine',    3,  N'BASE'),
    (4,  3,  N'fines',   N'fines',   2,  N'3SG.PRES'),
    (5,  4,  N'bat',     N'bat',     1,  N'SG'),
    (6,  4,  N'bats',    N'bats',    1,  N'PL'),
    (7,  5,  N'there',   N'there',   4,  N'BASE'),
    (8,  6,  N'their',   N'their',   6,  N'POSS'),
    (9,  7,  N'they''re',N'they''re',5,  N'CONTRACTION'),  -- they + are
    (10, 7,  N'they',    N'they',    5,  N'NOM'),
    (11, 8,  N'are',     N'are',     10, N'PL.PRES'),
    (12, 9,  N'okay',    N'okay',    3,  N'BASE'),
    (13, 9,  N'OK',      N'ok',      3,  N'BASE'),
    (14, 10, N'happy',   N'happy',   3,  N'BASE'),
    (15, 11, N'sad',     N'sad',     3,  N'BASE'),
    (16, 12, N'angry',   N'angry',   3,  N'BASE'),
    (17, 13, N'glad',    N'glad',    3,  N'BASE'),
    (18, 14, N'worry',   N'worry',   2,  N'BASE'),
    (19, 15, N'do',      N'do',      10, N'PRES'),
    (20, 16, N'not',     N'not',     4,  N'BASE'),
    (21, 17, N'about',   N'about',   7,  N'BASE'),
    (22, 18, N'me',      N'me',      5,  N'ACC'),
    (23, 19, N'leave',   N'leave',   2,  N'BASE'),
    (24, 20, N'alone',   N'alone',   3,  N'BASE'),
    (25, 21, N'racecar', N'racecar', 1,  N'SG'),
    (26, 22, N'baseball',N'baseball',1,  N'SG'),
    (27, 23, N'animal',  N'animal',  1,  N'SG'),
    (28, 24, N'mammal',  N'mammal',  1,  N'SG'),
    (29, 25, N'sport',   N'sport',   1,  N'SG'),
    /* Contraction expansion target form for "I'm" => "i am" handled in PhraseScanner
       but we also register the surface "I'm" -> WordID=1 (I) so single-word fallbacks
       can still resolve speakers if the phrase fails. */
    (30, 1,  N'I''m',    N'i''m',    5,  N'CONTRACTION');
SET IDENTITY_INSERT dbo.WordForm OFF;

/* --------- Pronunciations (illustrative; not exhaustive) ---------------- */
SET IDENTITY_INSERT dbo.Pronunciation ON;
INSERT INTO dbo.Pronunciation (PronunciationID, WordID, Ipa, Dialect) VALUES
    (1, 3,  N'/faɪn/',  N'en-US'),
    (2, 4,  N'/bæt/',   N'en-US'),
    (3, 5,  N'/ðɛr/',   N'en-US'),
    (4, 6,  N'/ðɛr/',   N'en-US'),  -- homophone of 'there'
    (5, 7,  N'/ðeɪ/',   N'en-US');
SET IDENTITY_INSERT dbo.Pronunciation OFF;
GO
