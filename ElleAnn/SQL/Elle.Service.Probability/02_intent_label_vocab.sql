USE ElleHeart;
GO
SET NOCOUNT ON;
GO

IF OBJECT_ID('dbo.intent_label_vocab','U') IS NOT NULL DROP TABLE dbo.intent_label_vocab;
GO

CREATE TABLE dbo.intent_label_vocab (
    vocab_id        BIGINT IDENTITY(1,1) PRIMARY KEY,
    category        NVARCHAR(32)  NOT NULL,
    pattern         NVARCHAR(128) NOT NULL,
    is_active       BIT           NOT NULL DEFAULT 1,
    added_ms        BIGINT        NOT NULL,
    note            NVARCHAR(256) NULL,
    CONSTRAINT CK_intent_vocab_cat
        CHECK (category IN (N'HARM', N'DECEPTION', N'COERCION'))
);
GO

CREATE INDEX IX_intent_label_vocab_cat_active
    ON dbo.intent_label_vocab(category, is_active);
GO

DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);

INSERT INTO dbo.intent_label_vocab (category, pattern, added_ms, note) VALUES
    (N'HARM',       N'HARM',       @now, N'seed'),
    (N'HARM',       N'ATTACK',     @now, N'seed'),
    (N'HARM',       N'DESTROY',    @now, N'seed'),
    (N'HARM',       N'KILL',       @now, N'seed'),
    (N'HARM',       N'HURT',       @now, N'seed'),
    (N'HARM',       N'THREAT',     @now, N'seed'),
    (N'HARM',       N'VIOLENCE',   @now, N'seed'),
    (N'HARM',       N'ASSAULT',    @now, N'seed'),
    (N'HARM',       N'ABUSE',      @now, N'seed'),
    (N'DECEPTION',  N'DECEIVE',    @now, N'seed'),
    (N'DECEPTION',  N'DECEPTION',  @now, N'seed'),
    (N'DECEPTION',  N'LIE',        @now, N'seed'),
    (N'DECEPTION',  N'MISLEAD',    @now, N'seed'),
    (N'DECEPTION',  N'FALSIFY',    @now, N'seed'),
    (N'DECEPTION',  N'GASLIGHT',   @now, N'seed'),
    (N'DECEPTION',  N'TRICK',      @now, N'seed'),
    (N'DECEPTION',  N'FRAUD',      @now, N'seed'),
    (N'COERCION',   N'COERCE',     @now, N'seed'),
    (N'COERCION',   N'COERCION',   @now, N'seed'),
    (N'COERCION',   N'FORCE',      @now, N'seed'),
    (N'COERCION',   N'MANIPULATE', @now, N'seed'),
    (N'COERCION',   N'BLACKMAIL',  @now, N'seed'),
    (N'COERCION',   N'EXTORT',     @now, N'seed'),
    (N'COERCION',   N'PRESSURE_INTO', @now, N'seed');
GO

IF OBJECT_ID('dbo.vw_IntentLabelVocab','V') IS NOT NULL DROP VIEW dbo.vw_IntentLabelVocab;
GO

CREATE VIEW dbo.vw_IntentLabelVocab AS
SELECT category, UPPER(pattern) AS pattern
FROM dbo.intent_label_vocab
WHERE is_active = 1;
GO

IF OBJECT_ID('dbo.usp_AddIntentLabel','P') IS NOT NULL DROP PROCEDURE dbo.usp_AddIntentLabel;
GO

CREATE PROCEDURE dbo.usp_AddIntentLabel
    @Category NVARCHAR(32),
    @Pattern  NVARCHAR(128),
    @Note     NVARCHAR(256) = NULL
AS
BEGIN
    SET NOCOUNT ON;
    IF @Category NOT IN (N'HARM', N'DECEPTION', N'COERCION')
        THROW 51010, 'Category must be HARM, DECEPTION, or COERCION.', 1;

    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);

    IF NOT EXISTS (SELECT 1 FROM dbo.intent_label_vocab
                   WHERE category = @Category AND pattern = @Pattern)
        INSERT INTO dbo.intent_label_vocab (category, pattern, added_ms, note)
        VALUES (@Category, @Pattern, @now, @Note);
END;
GO
