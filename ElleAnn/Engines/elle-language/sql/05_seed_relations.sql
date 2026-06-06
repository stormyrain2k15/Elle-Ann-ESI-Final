/* =========================================================================
   Elle Semantic Dictionary -- Seed: relations
   File: 05_seed_relations.sql

   Covers:
     * Synonym/antonym for "happy/sad/glad" etc.
     * Homonym for "bat"
     * Homophone for there/their/they're
     * Heteronym (we reuse "bat" verb vs noun as a stand-in heteronym example)
     * Paraphrase relations for "I'm fine" -> related expressions
   ========================================================================= */
USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

/* --------- WordRelations (lexical-level) --------------------------------- */
INSERT INTO dbo.WordRelation (FromWordID, ToWordID, RelationTypeID, Strength) VALUES
    /* Homonym: bat (4) is homonym to itself across senses; we still log a
       word-level homonym pair for tests that ask "are these homonyms?" */
    (4, 4, 3, 1.0000),

    /* Homophones */
    (5, 6, 4, 1.0000),
    (6, 5, 4, 1.0000),
    (5, 7, 4, 1.0000),  -- there ~ they're
    (7, 5, 4, 1.0000),
    (6, 7, 4, 1.0000),
    (7, 6, 4, 1.0000),

    /* Synonyms / antonyms across word lemmas */
    (3, 9, 1,  0.7500),     -- fine ~ okay
    (9, 3, 1,  0.7500),
    (10, 13, 1, 0.7000),    -- happy ~ glad
    (13, 10, 1, 0.7000),
    (10, 11, 2, 0.9000),    -- happy <-> sad
    (11, 10, 2, 0.9000),
    (12, 13, 2, 0.6500),    -- angry <-> glad
    (13, 12, 2, 0.6500);

/* --------- SenseRelations (sense-level, precise) ------------------------- */
INSERT INTO dbo.SenseRelation (FromSenseID, ToSenseID, RelationTypeID, Strength) VALUES
    /* fine#1 (acceptable adj) -- synonym -- okay#1 */
    (1, 10, 1, 0.85),
    (10, 1, 1, 0.85),

    /* bat#1 (mammal) -- hyponym of mammal sense #20 */
    (4, 20, 9, 1.0),  -- bat IS_A mammal (hyponym->hypernym is asymmetric; here we mark hyponym direction)
    (20, 4, 8, 1.0),  -- mammal hypernym of bat

    /* mammal#20 -- hyponym of animal#19 */
    (20, 19, 9, 1.0),
    (19, 20, 8, 1.0),

    /* bat#2 (baseball club) -- related concept -- baseball#18 */
    (5, 18, 12, 0.9),
    (18, 5, 12, 0.9),

    /* baseball -- hyponym of sport */
    (18, 21, 9, 1.0),
    (21, 18, 8, 1.0),

    /* happy <-> sad antonym at sense level */
    (11, 12, 2, 0.95),
    (12, 11, 2, 0.95),

    /* fine#1 acceptable vs fine#3 penalty -- contrast (different semantic field) */
    (1, 3, 13, 0.50),
    (3, 1, 13, 0.50),

    /* Heteronym: bat noun#5 (baseball club) vs bat verb#6 (to strike) share spelling
       but differ in role; we surface this as a homograph relation. */
    (5, 6, 5, 1.0),
    (6, 5, 5, 1.0);

/* --------- Paraphrase relations between PhraseSenses / Senses ------------ */
/* The C++ engine can also issue these against PhraseSense IDs by joining
   ConceptMember + SenseRelation; but for direct paraphrase lookup we just
   record them via the sense-level table using the negative-sentinel mapping
   would be ugly. Instead we expose paraphrase relations through
   SenseRelation rows where the SenseID column references a phrase sense
   via a dedicated mapping: NOT done here. Real paraphrase between
   PhraseSenses is encoded in ConceptMember (07) -- this is the proper
   data-model answer. We still keep word-level paraphrase below: */
INSERT INTO dbo.WordRelation (FromWordID, ToWordID, RelationTypeID, Strength) VALUES
    /* "fine" paraphrase to "okay" at the word level */
    (3, 9, 7, 0.80),
    (9, 3, 7, 0.80);
GO
