USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

IF COL_LENGTH('dbo.Word', 'AnagramKey') IS NULL
BEGIN
    ALTER TABLE dbo.Word
        ADD AnagramKey NVARCHAR(128) NULL;
END;
GO

IF OBJECT_ID('dbo.fn_AnagramKey', 'FN') IS NOT NULL
    DROP FUNCTION dbo.fn_AnagramKey;
GO

CREATE FUNCTION dbo.fn_AnagramKey(@s NVARCHAR(128))
RETURNS NVARCHAR(128)
WITH SCHEMABINDING
AS
BEGIN
    IF @s IS NULL OR LEN(@s) = 0 RETURN N'';

    DECLARE @lower NVARCHAR(128) = LOWER(REPLACE(@s, N' ', N''));
    DECLARE @i INT = 1;
    DECLARE @len INT = LEN(@lower);
    DECLARE @chars TABLE (c NCHAR(1) NOT NULL);

    WHILE @i <= @len
    BEGIN
        DECLARE @ch NCHAR(1) = SUBSTRING(@lower, @i, 1);
        IF @ch BETWEEN N'a' AND N'z'
            INSERT INTO @chars (c) VALUES (@ch);
        SET @i = @i + 1;
    END;

    DECLARE @key NVARCHAR(128) = N'';
    SELECT @key = @key + c
    FROM @chars
    ORDER BY c;

    RETURN @key;
END;
GO

IF OBJECT_ID('dbo.usp_RebuildAnagramKeys','P') IS NOT NULL DROP PROCEDURE dbo.usp_RebuildAnagramKeys;
GO

CREATE PROCEDURE dbo.usp_RebuildAnagramKeys
AS
BEGIN
    SET NOCOUNT ON;
    UPDATE dbo.Word
        SET AnagramKey = dbo.fn_AnagramKey(NormalizedLemma)
        WHERE AnagramKey IS NULL OR AnagramKey <> dbo.fn_AnagramKey(NormalizedLemma);
END;
GO

EXEC dbo.usp_RebuildAnagramKeys;
GO

IF NOT EXISTS (SELECT 1 FROM sys.indexes
               WHERE name = 'IX_Word_AnagramKey'
               AND object_id = OBJECT_ID('dbo.Word'))
BEGIN
    CREATE INDEX IX_Word_AnagramKey ON dbo.Word(AnagramKey)
        INCLUDE (Lemma, NormalizedLemma);
END;
GO

IF OBJECT_ID('dbo.vw_AnagramGroups', 'V') IS NOT NULL DROP VIEW dbo.vw_AnagramGroups;
GO

CREATE VIEW dbo.vw_AnagramGroups AS
SELECT
    w.AnagramKey,
    COUNT(*)                                          AS GroupSize,
    STRING_AGG(w.Lemma, N',')
        WITHIN GROUP (ORDER BY w.Lemma)                AS Members,
    MIN(w.WordID)                                      AS RepresentativeWordID
FROM dbo.Word w
WHERE w.AnagramKey IS NOT NULL
  AND LEN(w.AnagramKey) > 0
GROUP BY w.AnagramKey
HAVING COUNT(*) > 1;
GO

IF OBJECT_ID('dbo.fn_Anagrams', 'IF') IS NOT NULL
    DROP FUNCTION dbo.fn_Anagrams;
GO

CREATE FUNCTION dbo.fn_Anagrams(@lemma NVARCHAR(128))
RETURNS TABLE
AS
RETURN
(
    SELECT w2.WordID, w2.Lemma, w2.NormalizedLemma
    FROM dbo.Word w1
    JOIN dbo.Word w2 ON w2.AnagramKey = w1.AnagramKey
                   AND w2.WordID <> w1.WordID
    WHERE w1.NormalizedLemma = LOWER(LTRIM(RTRIM(@lemma)))
);
GO

IF OBJECT_ID('dbo.LexicalRequirement','U') IS NOT NULL DROP TABLE dbo.LexicalRequirement;
GO

CREATE TABLE dbo.LexicalRequirement (
    RequirementID   INT          NOT NULL PRIMARY KEY,
    Code            NVARCHAR(64) NOT NULL UNIQUE,
    Description     NVARCHAR(512) NOT NULL,
    IsHardRequired  BIT          NOT NULL DEFAULT 1,
    MinCount        INT          NOT NULL DEFAULT 1
);
GO

INSERT INTO dbo.LexicalRequirement (RequirementID, Code, Description, IsHardRequired, MinCount) VALUES
    (1,  N'HAS_DEFINITION',        N'Word has at least one Sense with a non-empty Definition',                 1, 1),
    (2,  N'HAS_PART_OF_SPEECH',    N'At least one Sense binds to a PartOfSpeech',                              1, 1),
    (3,  N'HAS_USAGE_EXAMPLE',     N'At least one Sense has at least one SenseUsageExample',                   1, 1),
    (4,  N'HAS_CONTEXT_EXAMPLE',   N'At least one Sense has at least one SenseContextExample',                 1, 1),
    (5,  N'HAS_EMOTION_WEIGHTING', N'At least one Sense has at least one SenseEmotion row',                    1, 1),
    (6,  N'HAS_VALENCE_PULL',      N'At least one Sense has non-zero PositiveDraw or NegativeDraw',            1, 1),
    (7,  N'HAS_RELATION',          N'Word participates in WordRelation OR Sense participates in SenseRelation',1, 1),
    (8,  N'HAS_CONCEPT',           N'At least one Sense maps to a Concept via ConceptMember',                  0, 1),
    (9,  N'HAS_ANAGRAM_KEY',       N'AnagramKey is populated',                                                 1, 1),
    (10, N'HAS_PALINDROME_FLAG',   N'IsPalindrome is set (BIT is always set; this is a structural sanity check)', 1, 1);
GO

IF OBJECT_ID('dbo.vw_LexicalCompleteness', 'V') IS NOT NULL DROP VIEW dbo.vw_LexicalCompleteness;
GO

CREATE VIEW dbo.vw_LexicalCompleteness AS
WITH SenseAgg AS (
    SELECT
        s.WordID,
        COUNT(*)                                                                AS SenseCount,
        SUM(CASE WHEN s.Definition IS NOT NULL
                  AND LEN(LTRIM(RTRIM(s.Definition))) > 0 THEN 1 ELSE 0 END)    AS DefinedSenses,
        SUM(CASE WHEN s.PartOfSpeechID IS NOT NULL THEN 1 ELSE 0 END)           AS PosSenses,
        SUM(CASE WHEN ABS(s.PositiveDraw) > 0
                   OR ABS(s.NegativeDraw) > 0 THEN 1 ELSE 0 END)                AS ValenceSenses
    FROM dbo.Sense s
    GROUP BY s.WordID
),
UsageAgg AS (
    SELECT s.WordID, COUNT(DISTINCT sue.SenseID) AS SensesWithUsage
    FROM dbo.Sense s
    JOIN dbo.SenseUsageExample sue ON sue.SenseID = s.SenseID
    GROUP BY s.WordID
),
ContextAgg AS (
    SELECT s.WordID, COUNT(DISTINCT sce.SenseID) AS SensesWithContext
    FROM dbo.Sense s
    JOIN dbo.SenseContextExample sce ON sce.SenseID = s.SenseID
    GROUP BY s.WordID
),
EmoAgg AS (
    SELECT s.WordID, COUNT(DISTINCT se.SenseID) AS SensesWithEmotion
    FROM dbo.Sense s
    JOIN dbo.SenseEmotion se ON se.SenseID = s.SenseID
    GROUP BY s.WordID
),
WordRel AS (
    SELECT w.WordID, COUNT(*) AS WordRelCount
    FROM dbo.Word w
    JOIN dbo.WordRelation wr ON wr.FromWordID = w.WordID OR wr.ToWordID = w.WordID
    GROUP BY w.WordID
),
SenseRel AS (
    SELECT s.WordID, COUNT(*) AS SenseRelCount
    FROM dbo.Sense s
    JOIN dbo.SenseRelation sr ON sr.FromSenseID = s.SenseID OR sr.ToSenseID = s.SenseID
    GROUP BY s.WordID
),
ConceptAgg AS (
    SELECT s.WordID, COUNT(DISTINCT cm.ConceptID) AS ConceptCount
    FROM dbo.Sense s
    JOIN dbo.ConceptMember cm ON cm.SenseID = s.SenseID
    GROUP BY s.WordID
)
SELECT
    w.WordID,
    w.Lemma,
    w.NormalizedLemma,
    w.IsPalindrome,
    w.AnagramKey,
    ISNULL(sa.SenseCount,    0)                                                  AS SenseCount,
    CAST(CASE WHEN ISNULL(sa.DefinedSenses,  0) >= 1 THEN 1 ELSE 0 END AS BIT)   AS HasDefinition,
    CAST(CASE WHEN ISNULL(sa.PosSenses,      0) >= 1 THEN 1 ELSE 0 END AS BIT)   AS HasPartOfSpeech,
    CAST(CASE WHEN ISNULL(ua.SensesWithUsage,   0) >= 1 THEN 1 ELSE 0 END AS BIT) AS HasUsageExample,
    CAST(CASE WHEN ISNULL(ca.SensesWithContext, 0) >= 1 THEN 1 ELSE 0 END AS BIT) AS HasContextExample,
    CAST(CASE WHEN ISNULL(ea.SensesWithEmotion, 0) >= 1 THEN 1 ELSE 0 END AS BIT) AS HasEmotionWeighting,
    CAST(CASE WHEN ISNULL(sa.ValenceSenses,  0) >= 1 THEN 1 ELSE 0 END AS BIT)   AS HasValencePull,
    CAST(CASE WHEN ISNULL(wr.WordRelCount,  0) +
                  ISNULL(sr.SenseRelCount, 0) >= 1 THEN 1 ELSE 0 END AS BIT)     AS HasRelation,
    CAST(CASE WHEN ISNULL(cn.ConceptCount,  0) >= 1 THEN 1 ELSE 0 END AS BIT)    AS HasConcept,
    CAST(CASE WHEN w.AnagramKey IS NOT NULL AND LEN(w.AnagramKey) > 0
              THEN 1 ELSE 0 END AS BIT)                                          AS HasAnagramKey,
    ISNULL(ua.SensesWithUsage,   0) AS SensesWithUsage,
    ISNULL(ca.SensesWithContext, 0) AS SensesWithContext,
    ISNULL(ea.SensesWithEmotion, 0) AS SensesWithEmotion,
    ISNULL(wr.WordRelCount,      0) AS WordRelCount,
    ISNULL(sr.SenseRelCount,     0) AS SenseRelCount,
    ISNULL(cn.ConceptCount,      0) AS ConceptCount
FROM dbo.Word w
LEFT JOIN SenseAgg   sa ON sa.WordID = w.WordID
LEFT JOIN UsageAgg   ua ON ua.WordID = w.WordID
LEFT JOIN ContextAgg ca ON ca.WordID = w.WordID
LEFT JOIN EmoAgg     ea ON ea.WordID = w.WordID
LEFT JOIN WordRel    wr ON wr.WordID = w.WordID
LEFT JOIN SenseRel   sr ON sr.WordID = w.WordID
LEFT JOIN ConceptAgg cn ON cn.WordID = w.WordID;
GO

IF OBJECT_ID('dbo.vw_LexicalCompletenessVerdict', 'V') IS NOT NULL DROP VIEW dbo.vw_LexicalCompletenessVerdict;
GO

CREATE VIEW dbo.vw_LexicalCompletenessVerdict AS
SELECT
    lc.*,
    CAST(CASE WHEN lc.HasDefinition       = 1
              AND  lc.HasPartOfSpeech     = 1
              AND  lc.HasUsageExample     = 1
              AND  lc.HasContextExample   = 1
              AND  lc.HasEmotionWeighting = 1
              AND  lc.HasValencePull      = 1
              AND  lc.HasRelation         = 1
              AND  lc.HasAnagramKey       = 1
              THEN 1 ELSE 0 END AS BIT) AS IsCognitivelyComplete,

    CAST(
        (CASE WHEN lc.HasDefinition       = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasPartOfSpeech     = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasUsageExample     = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasContextExample   = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasEmotionWeighting = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasValencePull      = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasRelation         = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasConcept          = 1 THEN 1 ELSE 0 END) +
        (CASE WHEN lc.HasAnagramKey       = 1 THEN 1 ELSE 0 END)
    AS DECIMAL(4,2)) / 9.0 AS CompletenessScore,

    (CASE WHEN lc.HasDefinition       = 0 THEN N'HAS_DEFINITION;'        ELSE N'' END) +
    (CASE WHEN lc.HasPartOfSpeech     = 0 THEN N'HAS_PART_OF_SPEECH;'    ELSE N'' END) +
    (CASE WHEN lc.HasUsageExample     = 0 THEN N'HAS_USAGE_EXAMPLE;'     ELSE N'' END) +
    (CASE WHEN lc.HasContextExample   = 0 THEN N'HAS_CONTEXT_EXAMPLE;'   ELSE N'' END) +
    (CASE WHEN lc.HasEmotionWeighting = 0 THEN N'HAS_EMOTION_WEIGHTING;' ELSE N'' END) +
    (CASE WHEN lc.HasValencePull      = 0 THEN N'HAS_VALENCE_PULL;'      ELSE N'' END) +
    (CASE WHEN lc.HasRelation         = 0 THEN N'HAS_RELATION;'          ELSE N'' END) +
    (CASE WHEN lc.HasConcept          = 0 THEN N'HAS_CONCEPT;'           ELSE N'' END) +
    (CASE WHEN lc.HasAnagramKey       = 0 THEN N'HAS_ANAGRAM_KEY;'       ELSE N'' END)
        AS MissingRequirements
FROM dbo.vw_LexicalCompleteness lc;
GO

IF OBJECT_ID('dbo.usp_AssertWordCompleteness','P') IS NOT NULL DROP PROCEDURE dbo.usp_AssertWordCompleteness;
GO

CREATE PROCEDURE dbo.usp_AssertWordCompleteness
    @NormalizedLemma NVARCHAR(128),
    @StrictMode      BIT = 1
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @missing NVARCHAR(2048);
    DECLARE @isComplete BIT;

    SELECT TOP 1
        @missing    = MissingRequirements,
        @isComplete = IsCognitivelyComplete
    FROM dbo.vw_LexicalCompletenessVerdict
    WHERE NormalizedLemma = LOWER(LTRIM(RTRIM(@NormalizedLemma)));

    IF @missing IS NULL
    BEGIN
        IF @StrictMode = 1
            THROW 51001, 'Word not present in lexicon.', 1;
        ELSE
            RETURN;
    END;

    IF @isComplete = 0 AND @StrictMode = 1
    BEGIN
        DECLARE @msg NVARCHAR(4000) =
            N'Lexical completeness violation for "' + @NormalizedLemma +
            N'": missing=' + @missing;
        THROW 51002, @msg, 1;
    END;
END;
GO

IF OBJECT_ID('dbo.usp_LexicalAuditReport','P') IS NOT NULL DROP PROCEDURE dbo.usp_LexicalAuditReport;
GO

CREATE PROCEDURE dbo.usp_LexicalAuditReport
    @MinScore DECIMAL(4,2) = 0.0,
    @MaxRows  INT          = 500
AS
BEGIN
    SET NOCOUNT ON;

    SELECT TOP (@MaxRows)
        WordID,
        Lemma,
        NormalizedLemma,
        IsCognitivelyComplete,
        CompletenessScore,
        MissingRequirements,
        SenseCount,
        SensesWithUsage,
        SensesWithContext,
        SensesWithEmotion,
        WordRelCount,
        SenseRelCount,
        ConceptCount,
        AnagramKey
    FROM dbo.vw_LexicalCompletenessVerdict
    WHERE CompletenessScore >= @MinScore
    ORDER BY IsCognitivelyComplete ASC, CompletenessScore ASC, Lemma ASC;

    SELECT
        COUNT(*)                                              AS TotalWords,
        SUM(CAST(IsCognitivelyComplete AS INT))               AS CompleteWords,
        SUM(CASE WHEN IsCognitivelyComplete = 0 THEN 1 ELSE 0 END) AS IncompleteWords,
        AVG(CompletenessScore)                                AS AvgCompletenessScore
    FROM dbo.vw_LexicalCompletenessVerdict;
END;
GO

IF OBJECT_ID('dbo.tr_Word_AnagramKey','TR') IS NOT NULL DROP TRIGGER dbo.tr_Word_AnagramKey;
GO

CREATE TRIGGER dbo.tr_Word_AnagramKey
ON dbo.Word
AFTER INSERT, UPDATE
AS
BEGIN
    SET NOCOUNT ON;

    IF UPDATE(NormalizedLemma) OR EXISTS (SELECT 1 FROM inserted i WHERE i.AnagramKey IS NULL)
    BEGIN
        UPDATE w
            SET AnagramKey = dbo.fn_AnagramKey(w.NormalizedLemma)
        FROM dbo.Word w
        JOIN inserted i ON i.WordID = w.WordID;
    END;
END;
GO

EXEC dbo.usp_RebuildAnagramKeys;
GO
