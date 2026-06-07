USE ElleCore;
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.columns
    WHERE object_id = OBJECT_ID(N'dbo.IntentQueue')
      AND name = 'ProcessingMs'
)
BEGIN
    ALTER TABLE dbo.IntentQueue ADD ProcessingMs BIGINT NULL;
END
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.indexes
    WHERE name = 'IX_IntentQueue_Processing'
      AND object_id = OBJECT_ID(N'dbo.IntentQueue')
)
BEGIN
    CREATE INDEX IX_IntentQueue_Processing
        ON dbo.IntentQueue (Status, ProcessingMs);
END
GO
