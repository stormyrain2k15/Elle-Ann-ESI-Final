SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'ElleSystem')
    CREATE DATABASE ElleSystem;
GO

USE ElleSystem;
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Workers')
BEGIN
    CREATE TABLE dbo.Workers (
        ServiceId           INT     NOT NULL PRIMARY KEY,
        ServiceName         NVARCHAR(64) NULL,
        LastHeartbeatMs     BIGINT  NOT NULL DEFAULT 0,
        Healthy             BIT     NOT NULL DEFAULT 0,
        StartedMs           BIGINT  NOT NULL DEFAULT 0,
        Pid                 INT     NULL,
        HostName            NVARCHAR(128) NULL
    );
    CREATE INDEX IX_Workers_LastHeartbeat
        ON dbo.Workers(LastHeartbeatMs DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Logs')
BEGIN
    CREATE TABLE dbo.Logs (
        Id                  BIGINT  IDENTITY(1,1) PRIMARY KEY,
        Level               INT     NOT NULL,
        ServiceId           INT     NOT NULL,
        Message             NVARCHAR(MAX) NOT NULL,
        TimestampMs         BIGINT  NOT NULL,
        CorrelationId       NVARCHAR(64) NULL
    );
    CREATE INDEX IX_Logs_TimestampMs ON dbo.Logs(TimestampMs DESC);
    CREATE INDEX IX_Logs_Service     ON dbo.Logs(ServiceId, TimestampMs DESC);
    CREATE INDEX IX_Logs_Level       ON dbo.Logs(Level, TimestampMs DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Analytics')
BEGIN
    CREATE TABLE dbo.Analytics (
        Id                  BIGINT  IDENTITY(1,1) PRIMARY KEY,
        Bucket              NVARCHAR(128) NOT NULL,
        Value               FLOAT   NOT NULL,
        Tags                NVARCHAR(512) NULL,
        TimestampMs         BIGINT  NOT NULL
    );
    CREATE INDEX IX_Analytics_Bucket_Time
        ON dbo.Analytics(Bucket, TimestampMs DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'TrustState')
BEGIN
    CREATE TABLE dbo.TrustState (
        UserId              INT     NOT NULL PRIMARY KEY,
        TrustScore          FLOAT   NOT NULL DEFAULT 0.5,
        LastUpdatedMs       BIGINT  NOT NULL,
        ActionCount         BIGINT  NOT NULL DEFAULT 0
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'TrustAudit')
BEGIN
    CREATE TABLE dbo.TrustAudit (
        Id                  BIGINT  IDENTITY(1,1) PRIMARY KEY,
        UserId              INT     NOT NULL,
        Delta               FLOAT   NOT NULL,
        Reason              NVARCHAR(256) NULL,
        ActionType          NVARCHAR(64)  NULL,
        TimestampMs         BIGINT  NOT NULL
    );
    CREATE INDEX IX_TrustAudit_User_Time
        ON dbo.TrustAudit(UserId, TimestampMs DESC);
END
GO

PRINT 'ElleAnn_System_Schema.sql applied OK';
GO
