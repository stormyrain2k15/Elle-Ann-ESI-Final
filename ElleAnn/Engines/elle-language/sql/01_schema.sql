/* =========================================================================
   Elle Semantic Dictionary Database -- Schema (SQL Server / T-SQL)
   File:    01_schema.sql
   Purpose: Defines every integer-indexed table the engine reads at runtime.
            This is the substrate. WordID / SenseID / PhraseID / ConceptID
            are the foundation; the engine processes meaning by walking
            these integers, not by predicting tokens.

   Conventions:
     * All primary keys are BIGINT IDENTITY for headroom at population scale.
     * All text fields are NVARCHAR (UTF-16) so Unicode is first-class.
     * All emotion / weight columns are DECIMAL(6,4) clamped to [-1.0, 1.0].
     * Every FK is explicit; cascades are intentional.
     * All "lookup" tables (RelationType, PartOfSpeech, Emotion, RelationStrength)
       use stable small integer IDs so the C++ engine can hard-reference them.

   Run order:
     01_schema.sql       (this file)
     02_seed_lexicon.sql
     03_seed_phrases.sql
     04_seed_senses.sql
     05_seed_relations.sql
     06_seed_context_frames.sql
     07_seed_concepts.sql
     08_indexes.sql
   ========================================================================= */

IF DB_ID('EllesLanguage') IS NULL
    EXEC ('CREATE DATABASE EllesLanguage;');
GO
USE EllesLanguage;
GO

/* ----------------------------------------------------------------------- */
/* Drop in reverse-dependency order so this script is idempotent.          */
/* ----------------------------------------------------------------------- */
IF OBJECT_ID('dbo.SemanticRelation','U')      IS NOT NULL DROP TABLE dbo.SemanticRelation;
IF OBJECT_ID('dbo.SemanticNode','U')          IS NOT NULL DROP TABLE dbo.SemanticNode;
IF OBJECT_ID('dbo.ConceptMember','U')         IS NOT NULL DROP TABLE dbo.ConceptMember;
IF OBJECT_ID('dbo.Concept','U')               IS NOT NULL DROP TABLE dbo.Concept;
IF OBJECT_ID('dbo.ContextFrameKeyword','U')   IS NOT NULL DROP TABLE dbo.ContextFrameKeyword;
IF OBJECT_ID('dbo.ContextFrame','U')          IS NOT NULL DROP TABLE dbo.ContextFrame;
IF OBJECT_ID('dbo.SenseEmotion','U')          IS NOT NULL DROP TABLE dbo.SenseEmotion;
IF OBJECT_ID('dbo.SenseContextExample','U')   IS NOT NULL DROP TABLE dbo.SenseContextExample;
IF OBJECT_ID('dbo.SenseUsageExample','U')     IS NOT NULL DROP TABLE dbo.SenseUsageExample;
IF OBJECT_ID('dbo.WordRelation','U')          IS NOT NULL DROP TABLE dbo.WordRelation;
IF OBJECT_ID('dbo.SenseRelation','U')         IS NOT NULL DROP TABLE dbo.SenseRelation;
IF OBJECT_ID('dbo.PhraseSense','U')           IS NOT NULL DROP TABLE dbo.PhraseSense;
IF OBJECT_ID('dbo.PhraseWord','U')            IS NOT NULL DROP TABLE dbo.PhraseWord;
IF OBJECT_ID('dbo.Phrase','U')                IS NOT NULL DROP TABLE dbo.Phrase;
IF OBJECT_ID('dbo.Sense','U')                 IS NOT NULL DROP TABLE dbo.Sense;
IF OBJECT_ID('dbo.WordForm','U')              IS NOT NULL DROP TABLE dbo.WordForm;
IF OBJECT_ID('dbo.Pronunciation','U')         IS NOT NULL DROP TABLE dbo.Pronunciation;
IF OBJECT_ID('dbo.Word','U')                  IS NOT NULL DROP TABLE dbo.Word;
IF OBJECT_ID('dbo.AnalysisTrace','U')         IS NOT NULL DROP TABLE dbo.AnalysisTrace;
IF OBJECT_ID('dbo.Emotion','U')               IS NOT NULL DROP TABLE dbo.Emotion;
IF OBJECT_ID('dbo.RelationType','U')          IS NOT NULL DROP TABLE dbo.RelationType;
IF OBJECT_ID('dbo.PartOfSpeech','U')          IS NOT NULL DROP TABLE dbo.PartOfSpeech;
GO

/* =========================================================================
   1. Reference / lookup tables
   ========================================================================= */
CREATE TABLE dbo.PartOfSpeech (
    PartOfSpeechID  INT          NOT NULL PRIMARY KEY,
    Code            NVARCHAR(16) NOT NULL UNIQUE,
    Name            NVARCHAR(64) NOT NULL
);

CREATE TABLE dbo.RelationType (
    RelationTypeID  INT          NOT NULL PRIMARY KEY,
    Code            NVARCHAR(32) NOT NULL UNIQUE,
    Name            NVARCHAR(64) NOT NULL,
    IsSymmetric     BIT          NOT NULL DEFAULT 0
);

CREATE TABLE dbo.Emotion (
    EmotionID       INT          NOT NULL PRIMARY KEY,
    Code            NVARCHAR(32) NOT NULL UNIQUE,
    Name            NVARCHAR(64) NOT NULL
);

/* =========================================================================
   2. Lexicon: Word, Pronunciation, WordForm
   ========================================================================= */
CREATE TABLE dbo.Word (
    WordID            BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    Lemma             NVARCHAR(128) NOT NULL,
    NormalizedLemma   NVARCHAR(128) NOT NULL,
    IsPalindrome      BIT           NOT NULL DEFAULT 0,
    Frequency         BIGINT        NOT NULL DEFAULT 0,
    CreatedUtc        DATETIME2(3)  NOT NULL DEFAULT SYSUTCDATETIME(),
    CONSTRAINT UQ_Word_NormalizedLemma UNIQUE (NormalizedLemma)
);

CREATE TABLE dbo.Pronunciation (
    PronunciationID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    WordID          BIGINT       NOT NULL,
    Ipa             NVARCHAR(128) NOT NULL,
    Dialect         NVARCHAR(32)  NULL,
    CONSTRAINT FK_Pron_Word FOREIGN KEY (WordID) REFERENCES dbo.Word(WordID) ON DELETE CASCADE
);

CREATE TABLE dbo.WordForm (
    WordFormID      BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    WordID          BIGINT        NOT NULL,
    Form            NVARCHAR(128) NOT NULL,
    NormalizedForm  NVARCHAR(128) NOT NULL,
    PartOfSpeechID  INT           NULL,
    InflectionTag   NVARCHAR(32)  NULL,
    CONSTRAINT FK_WF_Word FOREIGN KEY (WordID) REFERENCES dbo.Word(WordID) ON DELETE CASCADE,
    CONSTRAINT FK_WF_Pos  FOREIGN KEY (PartOfSpeechID) REFERENCES dbo.PartOfSpeech(PartOfSpeechID),
    CONSTRAINT UQ_WF_Form UNIQUE (NormalizedForm, PartOfSpeechID)
);

/* =========================================================================
   3. Sense layer
   ========================================================================= */
CREATE TABLE dbo.Sense (
    SenseID         BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    WordID          BIGINT         NOT NULL,
    PartOfSpeechID  INT            NULL,
    Definition      NVARCHAR(1024) NOT NULL,
    Gloss           NVARCHAR(256)  NULL,
    PositiveDraw    DECIMAL(6,4)   NOT NULL DEFAULT 0,
    NegativeDraw    DECIMAL(6,4)   NOT NULL DEFAULT 0,
    Valence         DECIMAL(6,4)   NOT NULL DEFAULT 0,
    Frequency       BIGINT         NOT NULL DEFAULT 0,
    SenseOrder      INT            NOT NULL DEFAULT 0,
    CONSTRAINT FK_Sense_Word FOREIGN KEY (WordID) REFERENCES dbo.Word(WordID) ON DELETE CASCADE,
    CONSTRAINT FK_Sense_Pos  FOREIGN KEY (PartOfSpeechID) REFERENCES dbo.PartOfSpeech(PartOfSpeechID),
    CONSTRAINT CK_Sense_PositiveDraw CHECK (PositiveDraw BETWEEN -1.0 AND 1.0),
    CONSTRAINT CK_Sense_NegativeDraw CHECK (NegativeDraw BETWEEN -1.0 AND 1.0),
    CONSTRAINT CK_Sense_Valence      CHECK (Valence      BETWEEN -1.0 AND 1.0)
);

CREATE TABLE dbo.SenseUsageExample (
    SenseUsageExampleID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    SenseID             BIGINT         NOT NULL,
    ExampleText         NVARCHAR(1024) NOT NULL,
    Slot                TINYINT        NOT NULL,        -- 1 or 2 (two per sense)
    CONSTRAINT FK_SUE_Sense FOREIGN KEY (SenseID) REFERENCES dbo.Sense(SenseID) ON DELETE CASCADE,
    CONSTRAINT CK_SUE_Slot  CHECK (Slot IN (1,2))
);

CREATE TABLE dbo.SenseContextExample (
    SenseContextExampleID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    SenseID               BIGINT         NOT NULL,
    ContextText           NVARCHAR(1024) NOT NULL,
    Slot                  TINYINT        NOT NULL,
    CONSTRAINT FK_SCE_Sense FOREIGN KEY (SenseID) REFERENCES dbo.Sense(SenseID) ON DELETE CASCADE,
    CONSTRAINT CK_SCE_Slot  CHECK (Slot IN (1,2))
);

CREATE TABLE dbo.SenseEmotion (
    SenseID    BIGINT       NOT NULL,
    EmotionID  INT          NOT NULL,
    Weight     DECIMAL(6,4) NOT NULL,
    CONSTRAINT PK_SenseEmotion PRIMARY KEY (SenseID, EmotionID),
    CONSTRAINT FK_SE_Sense   FOREIGN KEY (SenseID)   REFERENCES dbo.Sense(SenseID)   ON DELETE CASCADE,
    CONSTRAINT FK_SE_Emotion FOREIGN KEY (EmotionID) REFERENCES dbo.Emotion(EmotionID),
    CONSTRAINT CK_SE_Weight  CHECK (Weight BETWEEN -1.0 AND 1.0)
);

/* =========================================================================
   4. Phrase layer (multi-word expressions; matched BEFORE single words)
   ========================================================================= */
CREATE TABLE dbo.Phrase (
    PhraseID        BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    Surface         NVARCHAR(256) NOT NULL,
    NormalizedForm  NVARCHAR(256) NOT NULL,
    WordCount       INT           NOT NULL,
    Frequency       BIGINT        NOT NULL DEFAULT 0,
    CONSTRAINT UQ_Phrase_Norm UNIQUE (NormalizedForm)
);

CREATE TABLE dbo.PhraseWord (
    PhraseID    BIGINT NOT NULL,
    Position    INT    NOT NULL,
    WordID      BIGINT NULL,                            -- nullable: covers tokens not in lexicon
    SurfaceText NVARCHAR(128) NOT NULL,
    CONSTRAINT PK_PhraseWord PRIMARY KEY (PhraseID, Position),
    CONSTRAINT FK_PW_Phrase FOREIGN KEY (PhraseID) REFERENCES dbo.Phrase(PhraseID) ON DELETE CASCADE,
    CONSTRAINT FK_PW_Word   FOREIGN KEY (WordID)   REFERENCES dbo.Word(WordID)
);

CREATE TABLE dbo.PhraseSense (
    PhraseSenseID   BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    PhraseID        BIGINT         NOT NULL,
    Definition      NVARCHAR(1024) NOT NULL,
    Gloss           NVARCHAR(256)  NULL,
    PositiveDraw    DECIMAL(6,4)   NOT NULL DEFAULT 0,
    NegativeDraw    DECIMAL(6,4)   NOT NULL DEFAULT 0,
    Valence         DECIMAL(6,4)   NOT NULL DEFAULT 0,
    Frequency       BIGINT         NOT NULL DEFAULT 0,
    SenseOrder      INT            NOT NULL DEFAULT 0,
    CONSTRAINT FK_PS_Phrase FOREIGN KEY (PhraseID) REFERENCES dbo.Phrase(PhraseID) ON DELETE CASCADE,
    CONSTRAINT CK_PS_Pos    CHECK (PositiveDraw BETWEEN -1.0 AND 1.0),
    CONSTRAINT CK_PS_Neg    CHECK (NegativeDraw BETWEEN -1.0 AND 1.0),
    CONSTRAINT CK_PS_Val    CHECK (Valence      BETWEEN -1.0 AND 1.0)
);

/* Phrase senses get their own usage/context/emotion rows -- reuse Sense tables
   by introducing SenseID == NULL-friendly variants would be messier than
   simply giving phrase senses parallel tables, kept here intentionally. */
ALTER TABLE dbo.SenseUsageExample   ADD PhraseSenseID BIGINT NULL
    CONSTRAINT FK_SUE_PS FOREIGN KEY (PhraseSenseID) REFERENCES dbo.PhraseSense(PhraseSenseID) ON DELETE CASCADE;
ALTER TABLE dbo.SenseContextExample ADD PhraseSenseID BIGINT NULL
    CONSTRAINT FK_SCE_PS FOREIGN KEY (PhraseSenseID) REFERENCES dbo.PhraseSense(PhraseSenseID) ON DELETE CASCADE;
ALTER TABLE dbo.SenseEmotion        ADD PhraseSenseID BIGINT NULL
    CONSTRAINT FK_SE_PS  FOREIGN KEY (PhraseSenseID) REFERENCES dbo.PhraseSense(PhraseSenseID);
GO

/* Either SenseID or PhraseSenseID must be set, never both. */
ALTER TABLE dbo.SenseUsageExample   ADD CONSTRAINT CK_SUE_OneOwner CHECK (
    (SenseID IS NOT NULL AND PhraseSenseID IS NULL) OR
    (SenseID IS NULL     AND PhraseSenseID IS NOT NULL));
ALTER TABLE dbo.SenseContextExample ADD CONSTRAINT CK_SCE_OneOwner CHECK (
    (SenseID IS NOT NULL AND PhraseSenseID IS NULL) OR
    (SenseID IS NULL     AND PhraseSenseID IS NOT NULL));
GO

/* =========================================================================
   5. Relations: word-level and sense-level
   ========================================================================= */
CREATE TABLE dbo.WordRelation (
    WordRelationID  BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    FromWordID      BIGINT       NOT NULL,
    ToWordID        BIGINT       NOT NULL,
    RelationTypeID  INT          NOT NULL,
    Strength        DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    CONSTRAINT FK_WR_From  FOREIGN KEY (FromWordID)     REFERENCES dbo.Word(WordID),
    CONSTRAINT FK_WR_To    FOREIGN KEY (ToWordID)       REFERENCES dbo.Word(WordID),
    CONSTRAINT FK_WR_Type  FOREIGN KEY (RelationTypeID) REFERENCES dbo.RelationType(RelationTypeID),
    CONSTRAINT CK_WR_Range CHECK (Strength BETWEEN -1.0 AND 1.0),
    CONSTRAINT UQ_WR       UNIQUE (FromWordID, ToWordID, RelationTypeID)
);

CREATE TABLE dbo.SenseRelation (
    SenseRelationID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    FromSenseID     BIGINT       NOT NULL,
    ToSenseID       BIGINT       NOT NULL,
    RelationTypeID  INT          NOT NULL,
    Strength        DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    CONSTRAINT FK_SR_From  FOREIGN KEY (FromSenseID)    REFERENCES dbo.Sense(SenseID),
    CONSTRAINT FK_SR_To    FOREIGN KEY (ToSenseID)      REFERENCES dbo.Sense(SenseID),
    CONSTRAINT FK_SR_Type  FOREIGN KEY (RelationTypeID) REFERENCES dbo.RelationType(RelationTypeID),
    CONSTRAINT CK_SR_Range CHECK (Strength BETWEEN -1.0 AND 1.0),
    CONSTRAINT UQ_SR       UNIQUE (FromSenseID, ToSenseID, RelationTypeID)
);

/* =========================================================================
   6. Concept layer (groups of senses that mean "the same thing")
   ========================================================================= */
CREATE TABLE dbo.Concept (
    ConceptID   BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    Label       NVARCHAR(128)  NOT NULL,
    Description NVARCHAR(1024) NULL,
    CONSTRAINT UQ_Concept_Label UNIQUE (Label)
);

CREATE TABLE dbo.ConceptMember (
    ConceptID     BIGINT       NOT NULL,
    SenseID       BIGINT       NULL,
    PhraseSenseID BIGINT       NULL,
    Strength      DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    CONSTRAINT FK_CM_Concept FOREIGN KEY (ConceptID)     REFERENCES dbo.Concept(ConceptID) ON DELETE CASCADE,
    CONSTRAINT FK_CM_Sense   FOREIGN KEY (SenseID)       REFERENCES dbo.Sense(SenseID),
    CONSTRAINT FK_CM_PS      FOREIGN KEY (PhraseSenseID) REFERENCES dbo.PhraseSense(PhraseSenseID),
    CONSTRAINT CK_CM_One     CHECK ((SenseID IS NULL) <> (PhraseSenseID IS NULL))
);
CREATE UNIQUE INDEX UX_CM_Sense   ON dbo.ConceptMember(ConceptID, SenseID)
    WHERE SenseID IS NOT NULL;
CREATE UNIQUE INDEX UX_CM_PSense  ON dbo.ConceptMember(ConceptID, PhraseSenseID)
    WHERE PhraseSenseID IS NOT NULL;

/* =========================================================================
   7. Semantic graph (concept-to-concept and node-to-node typed edges)
   ========================================================================= */
CREATE TABLE dbo.SemanticNode (
    SemanticNodeID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    ConceptID      BIGINT NOT NULL,
    NodeLabel      NVARCHAR(128) NULL,
    CONSTRAINT FK_SN_Concept FOREIGN KEY (ConceptID) REFERENCES dbo.Concept(ConceptID) ON DELETE CASCADE
);

CREATE TABLE dbo.SemanticRelation (
    SemanticRelationID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    FromNodeID         BIGINT       NOT NULL,
    ToNodeID           BIGINT       NOT NULL,
    RelationTypeID     INT          NOT NULL,
    Strength           DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    Confidence         DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    CONSTRAINT FK_SemR_From FOREIGN KEY (FromNodeID)     REFERENCES dbo.SemanticNode(SemanticNodeID),
    CONSTRAINT FK_SemR_To   FOREIGN KEY (ToNodeID)       REFERENCES dbo.SemanticNode(SemanticNodeID),
    CONSTRAINT FK_SemR_Type FOREIGN KEY (RelationTypeID) REFERENCES dbo.RelationType(RelationTypeID),
    CONSTRAINT CK_SemR_S    CHECK (Strength   BETWEEN -1.0 AND 1.0),
    CONSTRAINT CK_SemR_C    CHECK (Confidence BETWEEN  0.0 AND 1.0)
);

/* =========================================================================
   8. Context frames
   ========================================================================= */
CREATE TABLE dbo.ContextFrame (
    ContextID   BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    Code        NVARCHAR(64)  NOT NULL UNIQUE,
    Name        NVARCHAR(128) NOT NULL,
    Description NVARCHAR(1024) NULL,
    ToneHint    NVARCHAR(64)  NULL,
    IntentHint  NVARCHAR(64)  NULL,
    Valence     DECIMAL(6,4)  NOT NULL DEFAULT 0,
    CONSTRAINT CK_CF_Valence CHECK (Valence BETWEEN -1.0 AND 1.0)
);

CREATE TABLE dbo.ContextFrameKeyword (
    ContextFrameKeywordID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    ContextID             BIGINT       NOT NULL,
    WordID                BIGINT       NULL,
    PhraseID              BIGINT       NULL,
    Weight                DECIMAL(6,4) NOT NULL DEFAULT 1.0,
    CONSTRAINT FK_CFK_Ctx  FOREIGN KEY (ContextID) REFERENCES dbo.ContextFrame(ContextID) ON DELETE CASCADE,
    CONSTRAINT FK_CFK_Word FOREIGN KEY (WordID)    REFERENCES dbo.Word(WordID),
    CONSTRAINT FK_CFK_Phr  FOREIGN KEY (PhraseID)  REFERENCES dbo.Phrase(PhraseID),
    CONSTRAINT CK_CFK_One  CHECK ((WordID IS NULL) <> (PhraseID IS NULL)),
    CONSTRAINT CK_CFK_W    CHECK (Weight BETWEEN -1.0 AND 1.0)
);

/* =========================================================================
   9. Runtime analysis persistence (optional but supported)
   ========================================================================= */
CREATE TABLE dbo.AnalysisTrace (
    AnalysisTraceID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    CreatedUtc      DATETIME2(3) NOT NULL DEFAULT SYSUTCDATETIME(),
    RawInput        NVARCHAR(MAX) NOT NULL,
    NormalizedInput NVARCHAR(MAX) NOT NULL,
    MeaningJson     NVARCHAR(MAX) NOT NULL,
    TraceJson       NVARCHAR(MAX) NOT NULL,
    ConfidenceScore DECIMAL(6,4)  NOT NULL DEFAULT 0,
    EngineVersion   NVARCHAR(32)  NOT NULL
);
GO
