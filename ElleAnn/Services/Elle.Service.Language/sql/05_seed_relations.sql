USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

INSERT INTO dbo.WordRelation (FromWordID, ToWordID, RelationTypeID, Strength) VALUES

    (4, 4, 3, 1.0000),

    (5, 6, 4, 1.0000),
    (6, 5, 4, 1.0000),
    (5, 7, 4, 1.0000),
    (7, 5, 4, 1.0000),
    (6, 7, 4, 1.0000),
    (7, 6, 4, 1.0000),

    (3, 9, 1,  0.7500),
    (9, 3, 1,  0.7500),
    (10, 13, 1, 0.7000),
    (13, 10, 1, 0.7000),
    (10, 11, 2, 0.9000),
    (11, 10, 2, 0.9000),
    (12, 13, 2, 0.6500),
    (13, 12, 2, 0.6500);

INSERT INTO dbo.SenseRelation (FromSenseID, ToSenseID, RelationTypeID, Strength) VALUES

    (1, 10, 1, 0.85),
    (10, 1, 1, 0.85),

    (4, 20, 9, 1.0),
    (20, 4, 8, 1.0),

    (20, 19, 9, 1.0),
    (19, 20, 8, 1.0),

    (5, 18, 12, 0.9),
    (18, 5, 12, 0.9),

    (18, 21, 9, 1.0),
    (21, 18, 8, 1.0),

    (11, 12, 2, 0.95),
    (12, 11, 2, 0.95),

    (1, 3, 13, 0.50),
    (3, 1, 13, 0.50),

    (5, 6, 5, 1.0),
    (6, 5, 5, 1.0);

INSERT INTO dbo.WordRelation (FromWordID, ToWordID, RelationTypeID, Strength) VALUES

    (3, 9, 7, 0.80),
    (9, 3, 7, 0.80);
GO
