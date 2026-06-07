USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

SET IDENTITY_INSERT dbo.Phrase ON;
INSERT INTO dbo.Phrase (PhraseID, Surface, NormalizedForm, WordCount, Frequency) VALUES
    (1, N'I''m fine',         N'i am fine',         3,  85000),
    (2, N'leave me alone',    N'leave me alone',    3,  20000),
    (3, N'don''t worry about me',
                              N'do not worry about me', 5, 9000);
SET IDENTITY_INSERT dbo.Phrase OFF;

INSERT INTO dbo.PhraseWord (PhraseID, Position, WordID, SurfaceText) VALUES

    (1, 0, 1, N'I'),
    (1, 1, 2, N'am'),
    (1, 2, 3, N'fine'),

    (2, 0, 19, N'leave'),
    (2, 1, 18, N'me'),
    (2, 2, 20, N'alone'),

    (3, 0, 15, N'do'),
    (3, 1, 16, N'not'),
    (3, 2, 14, N'worry'),
    (3, 3, 17, N'about'),
    (3, 4, 18, N'me');
GO
