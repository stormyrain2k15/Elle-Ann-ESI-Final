USE ElleCore;
GO
SET NOCOUNT ON;
GO

IF OBJECT_ID('dbo.SQLFallbackPoison','U') IS NULL
BEGIN
    CREATE TABLE dbo.SQLFallbackPoison (
        id             BIGINT IDENTITY(1,1) PRIMARY KEY,
        loaded_ms      BIGINT       NOT NULL,
        ts_ms          BIGINT       NULL,
        kind           NVARCHAR(16) NULL,
        idem           NVARCHAR(16) NULL,
        retry_count    INT          NULL,
        sql_or_proc    NVARCHAR(MAX) NULL,
        params_json    NVARCHAR(MAX) NULL,
        source_file    NVARCHAR(256) NOT NULL,
        raw_line       NVARCHAR(MAX) NOT NULL,
        replayed       BIT          NOT NULL CONSTRAINT DF_SQLFallbackPoison_replayed DEFAULT(0),
        replayed_ms    BIGINT       NULL,
        replay_error   NVARCHAR(MAX) NULL
    );
END
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.indexes
    WHERE name = 'IX_SQLFallbackPoison_source_loaded'
      AND object_id = OBJECT_ID('dbo.SQLFallbackPoison')
)
BEGIN
    CREATE INDEX IX_SQLFallbackPoison_source_loaded
        ON dbo.SQLFallbackPoison(source_file, loaded_ms DESC);
END
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.indexes
    WHERE name = 'IX_SQLFallbackPoison_replayed'
      AND object_id = OBJECT_ID('dbo.SQLFallbackPoison')
)
BEGIN
    CREATE INDEX IX_SQLFallbackPoison_replayed
        ON dbo.SQLFallbackPoison(replayed, loaded_ms DESC);
END
GO

CREATE OR ALTER PROCEDURE dbo.usp_SQLFallbackPoisonLoad
    @loaded_ms     BIGINT,
    @source_file   NVARCHAR(256),
    @raw_line      NVARCHAR(MAX),
    @ts_ms         BIGINT       = NULL,
    @kind          NVARCHAR(16) = NULL,
    @idem          NVARCHAR(16) = NULL,
    @retry_count   INT          = NULL,
    @sql_or_proc   NVARCHAR(MAX) = NULL,
    @params_json   NVARCHAR(MAX) = NULL
AS
BEGIN
    SET NOCOUNT ON;

    IF NOT EXISTS (
        SELECT 1 FROM dbo.SQLFallbackPoison
        WHERE source_file = @source_file
          AND raw_line    = @raw_line
    )
    BEGIN
        INSERT INTO dbo.SQLFallbackPoison
            (loaded_ms, ts_ms, kind, idem, retry_count,
             sql_or_proc, params_json, source_file, raw_line)
        VALUES
            (@loaded_ms, @ts_ms, @kind, @idem, @retry_count,
             @sql_or_proc, @params_json, @source_file, @raw_line);

        SELECT CAST(SCOPE_IDENTITY() AS BIGINT) AS id, 1 AS inserted;
    END
    ELSE
    BEGIN
        SELECT
            (SELECT TOP 1 id FROM dbo.SQLFallbackPoison
              WHERE source_file = @source_file AND raw_line = @raw_line) AS id,
            0 AS inserted;
    END
END
GO

CREATE OR ALTER PROCEDURE dbo.usp_SQLFallbackPoisonMarkReplayed
    @id           BIGINT,
    @replayed_ms  BIGINT,
    @replay_error NVARCHAR(MAX) = NULL
AS
BEGIN
    SET NOCOUNT ON;
    UPDATE dbo.SQLFallbackPoison
       SET replayed     = CASE WHEN @replay_error IS NULL THEN 1 ELSE 0 END,
           replayed_ms  = @replayed_ms,
           replay_error = @replay_error
     WHERE id = @id;

    SELECT @@ROWCOUNT AS affected;
END
GO

CREATE OR ALTER PROCEDURE dbo.usp_SQLFallbackPoisonList
    @top  INT = 200,
    @only_unreplayed BIT = 1
AS
BEGIN
    SET NOCOUNT ON;
    SELECT TOP (@top)
        id, loaded_ms, ts_ms, kind, idem, retry_count,
        sql_or_proc, params_json, source_file, raw_line,
        replayed, replayed_ms, replay_error
    FROM dbo.SQLFallbackPoison
    WHERE (@only_unreplayed = 0) OR (replayed = 0)
    ORDER BY loaded_ms DESC, id DESC;
END
GO
