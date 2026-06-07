USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

IF OBJECT_ID('dbo.vw_EngineReadySenses', 'V') IS NOT NULL
    DROP VIEW dbo.vw_EngineReadySenses;
GO

CREATE VIEW dbo.vw_EngineReadySenses AS
WITH UsageEx AS (
    SELECT SenseID,
        MAX(CASE WHEN Slot = 1 THEN ExampleText END) AS UsageEx1,
        MAX(CASE WHEN Slot = 2 THEN ExampleText END) AS UsageEx2
    FROM dbo.SenseUsageExample
    WHERE SenseID IS NOT NULL
    GROUP BY SenseID
),
ContextEx AS (
    SELECT SenseID,
        MAX(CASE WHEN Slot = 1 THEN ContextText END) AS ContextEx1,
        MAX(CASE WHEN Slot = 2 THEN ContextText END) AS ContextEx2
    FROM dbo.SenseContextExample
    WHERE SenseID IS NOT NULL
    GROUP BY SenseID
),
EmoCount AS (
    SELECT SenseID, COUNT(*) AS EmotionDimCount
    FROM dbo.SenseEmotion
    WHERE SenseID IS NOT NULL
    GROUP BY SenseID
),
SynList AS (
    SELECT sr.FromSenseID AS SenseID,
        STRING_AGG(w.Lemma, N',') WITHIN GROUP (ORDER BY w.Lemma) AS Synonyms
    FROM dbo.SenseRelation sr
    JOIN dbo.RelationType rt ON rt.RelationTypeID = sr.RelationTypeID AND rt.Code = N'SYNONYM'
    JOIN dbo.Sense s2        ON s2.SenseID  = sr.ToSenseID
    JOIN dbo.Word  w         ON w.WordID    = s2.WordID
    GROUP BY sr.FromSenseID
),
AntList AS (
    SELECT sr.FromSenseID AS SenseID,
        STRING_AGG(w.Lemma, N',') WITHIN GROUP (ORDER BY w.Lemma) AS Antonyms
    FROM dbo.SenseRelation sr
    JOIN dbo.RelationType rt ON rt.RelationTypeID = sr.RelationTypeID AND rt.Code = N'ANTONYM'
    JOIN dbo.Sense s2        ON s2.SenseID  = sr.ToSenseID
    JOIN dbo.Word  w         ON w.WordID    = s2.WordID
    GROUP BY sr.FromSenseID
),
ConList AS (
    SELECT cm.SenseID,
        STRING_AGG(c.Label, N',') WITHIN GROUP (ORDER BY c.Label) AS Concepts
    FROM dbo.ConceptMember cm
    JOIN dbo.Concept c ON c.ConceptID = cm.ConceptID
    WHERE cm.SenseID IS NOT NULL
    GROUP BY cm.SenseID
)
SELECT
    s.SenseID,
    w.WordID,
    w.Lemma,
    w.NormalizedLemma,
    w.IsPalindrome,
    w.Frequency       AS WordFrequency,
    p.Code            AS PosCode,
    p.Name            AS PosName,
    s.Definition,
    s.Gloss,
    s.PositiveDraw,
    s.NegativeDraw,
    s.Valence,
    s.Frequency       AS SenseFrequency,
    s.SenseOrder,
    ue.UsageEx1,
    ue.UsageEx2,
    ce.ContextEx1,
    ce.ContextEx2,
    ISNULL(ec.EmotionDimCount, 0) AS EmotionDimCount,
    sl.Synonyms,
    al.Antonyms,
    cl.Concepts,

    CAST(CASE
        WHEN ue.UsageEx1   IS NOT NULL AND ue.UsageEx2   IS NOT NULL
         AND ce.ContextEx1 IS NOT NULL AND ce.ContextEx2 IS NOT NULL
         AND ISNULL(ec.EmotionDimCount, 0) > 0
        THEN 1 ELSE 0
    END AS BIT) AS IsReady
FROM dbo.Sense           s
JOIN dbo.Word            w  ON w.WordID         = s.WordID
LEFT JOIN dbo.PartOfSpeech p ON p.PartOfSpeechID = s.PartOfSpeechID
LEFT JOIN UsageEx   ue ON ue.SenseID = s.SenseID
LEFT JOIN ContextEx ce ON ce.SenseID = s.SenseID
LEFT JOIN EmoCount  ec ON ec.SenseID = s.SenseID
LEFT JOIN SynList   sl ON sl.SenseID = s.SenseID
LEFT JOIN AntList   al ON al.SenseID = s.SenseID
LEFT JOIN ConList   cl ON cl.SenseID = s.SenseID;
GO

IF OBJECT_ID('dbo.usp_LoadStagingWords','P') IS NOT NULL DROP PROCEDURE dbo.usp_LoadStagingWords;
GO
CREATE PROCEDURE dbo.usp_LoadStagingWords

AS
BEGIN
    SET NOCOUNT ON;

    IF OBJECT_ID('tempdb..#StagingWord') IS NULL
        THROW 50001, '#StagingWord temp table must exist before calling usp_LoadStagingWords', 1;

    MERGE dbo.Word AS T
    USING (
        SELECT
            Lemma           = LTRIM(RTRIM(Lemma)),
            NormalizedLemma = LTRIM(RTRIM(LOWER(NormalizedLemma))),
            IsPalindrome    = ISNULL(IsPalindrome, 0),
            Frequency       = ISNULL(Frequency, 0)
        FROM #StagingWord
        WHERE NormalizedLemma IS NOT NULL AND LEN(LTRIM(RTRIM(NormalizedLemma))) > 0
    ) AS S
    ON T.NormalizedLemma = S.NormalizedLemma
    WHEN MATCHED THEN
        UPDATE SET
            T.Frequency    = CASE WHEN S.Frequency > T.Frequency THEN S.Frequency ELSE T.Frequency END,
            T.IsPalindrome = S.IsPalindrome
    WHEN NOT MATCHED BY TARGET THEN
        INSERT (Lemma, NormalizedLemma, IsPalindrome, Frequency)
        VALUES (S.Lemma, S.NormalizedLemma, S.IsPalindrome, S.Frequency);
END;
GO

IF OBJECT_ID('dbo.usp_LoadStagingSenses','P') IS NOT NULL DROP PROCEDURE dbo.usp_LoadStagingSenses;
GO
CREATE PROCEDURE dbo.usp_LoadStagingSenses

AS
BEGIN
    SET NOCOUNT ON;

    IF OBJECT_ID('tempdb..#StagingSense') IS NULL
        THROW 50001, '#StagingSense temp table must exist before calling usp_LoadStagingSenses', 1;

    IF OBJECT_ID('tempdb..#StagingSenseOut') IS NOT NULL DROP TABLE #StagingSenseOut;
    CREATE TABLE #StagingSenseOut (
        SourceTag  NVARCHAR(64)  NOT NULL,
        SenseID    BIGINT        NOT NULL
    );

    DECLARE @Inserted TABLE (
        SenseID    BIGINT,
        WordID     BIGINT,
        SenseOrder INT
    );

    INSERT INTO dbo.Sense (WordID, PartOfSpeechID, Definition, Gloss,
                           PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder)
    OUTPUT inserted.SenseID, inserted.WordID, inserted.SenseOrder
        INTO @Inserted (SenseID, WordID, SenseOrder)
    SELECT
        w.WordID,
        p.PartOfSpeechID,
        s.Definition,
        s.Gloss,
        ISNULL(s.PositiveDraw, 0),
        ISNULL(s.NegativeDraw, 0),
        ISNULL(s.Valence,      0),
        ISNULL(s.Frequency,    0),
        s.SenseOrder
    FROM #StagingSense s
    JOIN dbo.Word w ON w.NormalizedLemma = s.NormalizedLemma
    LEFT JOIN dbo.PartOfSpeech p ON p.Code = s.PosCode

    WHERE NOT EXISTS (
        SELECT 1
        FROM dbo.Sense s2
        WHERE s2.WordID = w.WordID
          AND ISNULL(s2.PartOfSpeechID, -1) = ISNULL(p.PartOfSpeechID, -1)
          AND s2.SenseOrder = s.SenseOrder
    );

    INSERT INTO #StagingSenseOut (SourceTag, SenseID)
    SELECT s.SourceTag, i.SenseID
    FROM @Inserted i
    JOIN dbo.Word w ON w.WordID = i.WordID
    JOIN #StagingSense s
      ON  s.NormalizedLemma = w.NormalizedLemma
      AND s.SenseOrder      = i.SenseOrder;
END;
GO

IF OBJECT_ID('dbo.usp_LoadStagingPhrases','P') IS NOT NULL DROP PROCEDURE dbo.usp_LoadStagingPhrases;
GO
CREATE PROCEDURE dbo.usp_LoadStagingPhrases

AS
BEGIN
    SET NOCOUNT ON;

    IF OBJECT_ID('tempdb..#StagingPhrase') IS NULL
        THROW 50001, '#StagingPhrase must exist', 1;
    IF OBJECT_ID('tempdb..#StagingPhraseWord') IS NULL
        THROW 50001, '#StagingPhraseWord must exist', 1;

    IF OBJECT_ID('tempdb..#StagingPhraseOut') IS NOT NULL DROP TABLE #StagingPhraseOut;
    CREATE TABLE #StagingPhraseOut (
        SourceTag NVARCHAR(64) NOT NULL,
        PhraseID  BIGINT       NOT NULL
    );

    DECLARE @Acted TABLE (
        Act           NVARCHAR(10),
        PhraseID      BIGINT,
        NormalizedForm NVARCHAR(256)
    );

    MERGE dbo.Phrase AS T
    USING (
        SELECT
            Surface         = LTRIM(RTRIM(Surface)),
            NormalizedForm  = LTRIM(RTRIM(LOWER(NormalizedForm))),
            WordCount       = ISNULL(WordCount, 0),
            Frequency       = ISNULL(Frequency, 0)
        FROM #StagingPhrase
        WHERE NormalizedForm IS NOT NULL
          AND LEN(LTRIM(RTRIM(NormalizedForm))) > 0
    ) AS S
    ON T.NormalizedForm = S.NormalizedForm
    WHEN MATCHED THEN
        UPDATE SET T.Frequency = CASE WHEN S.Frequency > T.Frequency THEN S.Frequency ELSE T.Frequency END
    WHEN NOT MATCHED BY TARGET THEN
        INSERT (Surface, NormalizedForm, WordCount, Frequency)
        VALUES (S.Surface, S.NormalizedForm, S.WordCount, S.Frequency)
    OUTPUT $action, inserted.PhraseID, inserted.NormalizedForm INTO @Acted;

    INSERT INTO #StagingPhraseOut (SourceTag, PhraseID)
    SELECT sp.SourceTag, p.PhraseID
    FROM #StagingPhrase sp
    JOIN dbo.Phrase p
      ON p.NormalizedForm = LTRIM(RTRIM(LOWER(sp.NormalizedForm)));

    DELETE pw
    FROM dbo.PhraseWord pw
    JOIN #StagingPhraseOut po ON po.PhraseID = pw.PhraseID;

    INSERT INTO dbo.PhraseWord (PhraseID, Position, WordID, SurfaceText)
    SELECT
        po.PhraseID,
        spw.Position,
        w.WordID,
        spw.SurfaceText
    FROM #StagingPhraseWord spw
    JOIN #StagingPhraseOut po ON po.SourceTag = spw.SourceTag
    LEFT JOIN dbo.Word w ON w.NormalizedLemma = LOWER(spw.NormalizedLemma);
END;
GO

GO
