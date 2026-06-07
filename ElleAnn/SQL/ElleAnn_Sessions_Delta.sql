SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

USE ElleCore;
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Sessions' AND schema_id = SCHEMA_ID('dbo'))
BEGIN
    CREATE TABLE dbo.Sessions (
        Token       NVARCHAR(64)   NOT NULL PRIMARY KEY,
        nUserNo     BIGINT         NOT NULL,
        sUserID     NVARCHAR(30)   NOT NULL,
        sUserName   NVARCHAR(60)   NULL,
        nAuthID     INT            NOT NULL DEFAULT 0,
        CreatedMs   BIGINT         NOT NULL,
        LastSeenMs  BIGINT         NOT NULL,
        DeviceName  NVARCHAR(128)  NULL,
        PeerAddr    NVARCHAR(64)   NULL
    );
    CREATE INDEX IX_Sessions_nUserNo ON dbo.Sessions(nUserNo);
    PRINT 'ElleCore.dbo.Sessions created';
END
ELSE
BEGIN
    PRINT 'ElleCore.dbo.Sessions already exists';
END
GO

PRINT 'ElleAnn_Sessions_Delta.sql applied OK';
GO
