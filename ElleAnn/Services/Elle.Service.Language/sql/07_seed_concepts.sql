USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

SET IDENTITY_INSERT dbo.Concept ON;
INSERT INTO dbo.Concept (ConceptID, Label, Description) VALUES
    (1, N'STATE_ACCEPTABLE',     N'The state of being okay / acceptable / fine.'),
    (2, N'STATE_SADNESS',        N'Sorrow, withdrawal, low mood.'),
    (3, N'STATE_ANGER',          N'Anger, hostility, displeasure.'),
    (4, N'ACT_REASSURE',         N'Reassuring another person, asking them not to worry.'),
    (5, N'ENTITY_BAT_MAMMAL',    N'The flying mammal called "bat".'),
    (6, N'ENTITY_BAT_EQUIPMENT', N'The baseball implement called "bat".'),
    (7, N'DOMAIN_BASEBALL',      N'Sport of baseball and its equipment.'),
    (8, N'DOMAIN_WILDLIFE',      N'Animals and natural ecosystems.');
SET IDENTITY_INSERT dbo.Concept OFF;

INSERT INTO dbo.ConceptMember (ConceptID, SenseID, PhraseSenseID, Strength) VALUES

    (1, 1,    NULL, 1.0),
    (1, 10,   NULL, 1.0),
    (1, NULL, 1,    1.0),

    (2, 12,   NULL, 1.0),
    (2, NULL, 2,    1.0),
    (2, NULL, 5,    0.8),

    (3, 13,   NULL, 1.0),
    (3, NULL, 3,    1.0),

    (4, NULL, 4,    1.0),
    (4, NULL, 6,    1.0),
    (4, 14,   NULL, 0.6),

    (5, 4,    NULL, 1.0),

    (6, 5,    NULL, 1.0),
    (6, 6,    NULL, 0.7),

    (7, 18,   NULL, 1.0),
    (7, 21,   NULL, 0.8),
    (7, 5,    NULL, 0.9),

    (8, 19,   NULL, 1.0),
    (8, 20,   NULL, 1.0),
    (8, 4,    NULL, 0.9);

SET IDENTITY_INSERT dbo.SemanticNode ON;
INSERT INTO dbo.SemanticNode (SemanticNodeID, ConceptID, NodeLabel) VALUES
    (1, 1, N'state.acceptable'),
    (2, 2, N'state.sadness'),
    (3, 3, N'state.anger'),
    (4, 4, N'act.reassure'),
    (5, 5, N'entity.bat.mammal'),
    (6, 6, N'entity.bat.equipment'),
    (7, 7, N'domain.baseball'),
    (8, 8, N'domain.wildlife');
SET IDENTITY_INSERT dbo.SemanticNode OFF;

INSERT INTO dbo.SemanticRelation (FromNodeID, ToNodeID, RelationTypeID, Strength, Confidence) VALUES

    (2, 3, 12, 0.6, 0.9),
    (3, 2, 12, 0.6, 0.9),

    (1, 2, 13, 0.7, 0.95),
    (2, 1, 13, 0.7, 0.95),

    (1, 3, 13, 0.7, 0.95),
    (3, 1, 13, 0.7, 0.95),

    (4, 1, 15, 0.7, 0.85),

    (7, 6, 11, 0.9, 0.95),
    (6, 7, 10, 0.9, 0.95),

    (8, 5, 11, 0.9, 0.95),
    (5, 8, 10, 0.9, 0.95);
GO
