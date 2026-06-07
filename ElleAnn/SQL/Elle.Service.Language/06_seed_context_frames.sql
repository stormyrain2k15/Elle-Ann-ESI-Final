USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

SET IDENTITY_INSERT dbo.ContextFrame ON;
INSERT INTO dbo.ContextFrame (ContextID, Code, Name, Description, ToneHint, IntentHint, Valence) VALUES
    (1, N'CASUAL_STATUS_CHECK',
        N'Casual status check',
        N'Light social check-in: "how are you", small talk about feelings.',
        N'neutral',   N'inform',     0.10),
    (2, N'EMOTIONAL_WITHDRAWAL',
        N'Emotional withdrawal',
        N'Speaker is sad or shut down and signaling they do not want to engage.',
        N'sad',       N'withdraw',  -0.55),
    (3, N'DISMISSIVE_HOSTILE',
        N'Dismissive / hostile',
        N'Speaker is angry and rejecting concern; often punctuated with !, !!, !!!.',
        N'angry',     N'dismiss',   -0.65),
    (4, N'REASSURANCE',
        N'Reassurance',
        N'Speaker reassures the listener that they need not worry.',
        N'tender',    N'reassure',   0.50),
    (5, N'BASEBALL_CONTEXT',
        N'Baseball context',
        N'Sports / baseball discussion: bats, balls, pitchers, swings.',
        N'neutral',   N'inform',     0.10),
    (6, N'WILDLIFE_CONTEXT',
        N'Wildlife / nature context',
        N'Discussion of animals, caves, nocturnal life.',
        N'neutral',   N'inform',     0.00);
SET IDENTITY_INSERT dbo.ContextFrame OFF;

INSERT INTO dbo.ContextFrameKeyword (ContextID, WordID, PhraseID, Weight) VALUES

    (1, 3,  NULL, 0.50),
    (1, 9,  NULL, 0.60),
    (1, NULL, 1, 0.80),

    (2, 11, NULL, 0.85),
    (2, 20, NULL, 0.55),
    (2, NULL, 2, 0.85),
    (2, NULL, 1, 0.30),

    (3, 12, NULL, 0.90),
    (3, NULL, 1, 0.30),

    (4, 14, NULL, 0.60),
    (4, 13, NULL, 0.55),
    (4, NULL, 3, 0.95),
    (4, NULL, 1, 0.40),

    (5, 22, NULL, 0.90),
    (5, 25, NULL, 0.50),
    (5, 4,  NULL, 0.55),

    (6, 23, NULL, 0.70),
    (6, 24, NULL, 0.85),
    (6, 4,  NULL, 0.55);
GO
