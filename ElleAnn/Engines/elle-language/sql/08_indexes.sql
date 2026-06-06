/* =========================================================================
   Elle Semantic Dictionary -- Indexes for the hot paths the engine uses
   File: 08_indexes.sql
   ========================================================================= */
USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

/* Lookups: normalized lemma / form -- the engine hits these every word. */
CREATE INDEX IX_Word_NormalizedLemma     ON dbo.Word(NormalizedLemma);
CREATE INDEX IX_WordForm_NormalizedForm  ON dbo.WordForm(NormalizedForm);
CREATE INDEX IX_WordForm_WordID          ON dbo.WordForm(WordID);

/* Phrase scanner: hits by normalized form, descending word count for
   longest-match-first. */
CREATE INDEX IX_Phrase_Norm_WordCount    ON dbo.Phrase(NormalizedForm, WordCount DESC);
CREATE INDEX IX_PhraseWord_Phrase_Pos    ON dbo.PhraseWord(PhraseID, Position);

/* Sense lookups */
CREATE INDEX IX_Sense_WordID             ON dbo.Sense(WordID);
CREATE INDEX IX_PhraseSense_PhraseID     ON dbo.PhraseSense(PhraseID);

/* Sense examples and emotions */
CREATE INDEX IX_SUE_SenseID              ON dbo.SenseUsageExample(SenseID)         WHERE SenseID       IS NOT NULL;
CREATE INDEX IX_SUE_PhraseSenseID        ON dbo.SenseUsageExample(PhraseSenseID)   WHERE PhraseSenseID IS NOT NULL;
CREATE INDEX IX_SCE_SenseID              ON dbo.SenseContextExample(SenseID)       WHERE SenseID       IS NOT NULL;
CREATE INDEX IX_SCE_PhraseSenseID        ON dbo.SenseContextExample(PhraseSenseID) WHERE PhraseSenseID IS NOT NULL;
CREATE INDEX IX_SE_SenseID               ON dbo.SenseEmotion(SenseID)              WHERE SenseID       IS NOT NULL;
CREATE INDEX IX_SE_PhraseSenseID         ON dbo.SenseEmotion(PhraseSenseID)        WHERE PhraseSenseID IS NOT NULL;

/* Relation lookups */
CREATE INDEX IX_WordRelation_From        ON dbo.WordRelation(FromWordID, RelationTypeID);
CREATE INDEX IX_WordRelation_To          ON dbo.WordRelation(ToWordID, RelationTypeID);
CREATE INDEX IX_SenseRelation_From       ON dbo.SenseRelation(FromSenseID, RelationTypeID);
CREATE INDEX IX_SenseRelation_To         ON dbo.SenseRelation(ToSenseID, RelationTypeID);
CREATE INDEX IX_SemanticRelation_From    ON dbo.SemanticRelation(FromNodeID, RelationTypeID);
CREATE INDEX IX_SemanticRelation_To      ON dbo.SemanticRelation(ToNodeID, RelationTypeID);

/* Context keyword lookups */
CREATE INDEX IX_CFK_WordID               ON dbo.ContextFrameKeyword(WordID)   WHERE WordID   IS NOT NULL;
CREATE INDEX IX_CFK_PhraseID             ON dbo.ContextFrameKeyword(PhraseID) WHERE PhraseID IS NOT NULL;
CREATE INDEX IX_CFK_Context              ON dbo.ContextFrameKeyword(ContextID);

/* Concept / node */
CREATE INDEX IX_ConceptMember_Sense      ON dbo.ConceptMember(SenseID)       WHERE SenseID       IS NOT NULL;
CREATE INDEX IX_ConceptMember_PSense     ON dbo.ConceptMember(PhraseSenseID) WHERE PhraseSenseID IS NOT NULL;
CREATE INDEX IX_SemanticNode_Concept     ON dbo.SemanticNode(ConceptID);

/* Analysis trace persistence */
CREATE INDEX IX_AnalysisTrace_CreatedUtc ON dbo.AnalysisTrace(CreatedUtc DESC);
GO
