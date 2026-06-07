IF NOT EXISTS (SELECT 1 FROM sys.tables t
  JOIN sys.schemas s ON s.schema_id = t.schema_id
  WHERE t.name = 'imagined_scenarios' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.imagined_scenarios (
    id BIGINT IDENTITY(1,1) PRIMARY KEY,
    scenario_id NVARCHAR(64) NOT NULL,
    summary NVARCHAR(MAX) NOT NULL,
    score_json NVARCHAR(MAX) NOT NULL,
    iteration_count INT NOT NULL,
    created_ms BIGINT NOT NULL,
    source_memory_ids_json NVARCHAR(MAX) NOT NULL,
    refined NVARCHAR(MAX) NULL
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_imagined_created'
               AND object_id = OBJECT_ID('ElleHeart.dbo.imagined_scenarios'))
CREATE INDEX IX_imagined_created
    ON ElleHeart.dbo.imagined_scenarios (created_ms DESC);
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables t
  JOIN sys.schemas s ON s.schema_id = t.schema_id
  WHERE t.name = 'conscience_log' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.conscience_log (
    id BIGINT IDENTITY(1,1) PRIMARY KEY,
    recorded_ms BIGINT NOT NULL,
    request_id NVARCHAR(64) NOT NULL,
    speaker_id NVARCHAR(64) NOT NULL,
    conflict NVARCHAR(64) NOT NULL,
    verdict NVARCHAR(64) NOT NULL,
    severity FLOAT NOT NULL,
    reasoning NVARCHAR(MAX) NULL,
    is_post_action BIT NOT NULL,
    entry_json NVARCHAR(MAX) NOT NULL
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.indexes WHERE name = 'IX_conscience_recorded'
               AND object_id = OBJECT_ID('ElleHeart.dbo.conscience_log'))
CREATE INDEX IX_conscience_recorded
    ON ElleHeart.dbo.conscience_log (recorded_ms DESC);
GO
