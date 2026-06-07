SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

USE ElleCore;
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.columns
     WHERE object_id = OBJECT_ID('ElleCore.dbo.PairedDevices')
       AND name = 'nUserNo'
)
BEGIN
    ALTER TABLE ElleCore.dbo.PairedDevices
        ADD nUserNo INT NULL;
    PRINT 'PairedDevices.nUserNo added';
END
ELSE
BEGIN
    PRINT 'PairedDevices.nUserNo already exists';
END
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.indexes
     WHERE name = 'IX_PairedDevices_nUserNo'
       AND object_id = OBJECT_ID('ElleCore.dbo.PairedDevices')
)
BEGIN
    CREATE INDEX IX_PairedDevices_nUserNo
        ON ElleCore.dbo.PairedDevices(nUserNo)
        WHERE nUserNo IS NOT NULL;
    PRINT 'IX_PairedDevices_nUserNo created';
END
GO

IF OBJECT_ID('ElleCore.dbo.UserContinuity', 'U') IS NULL
BEGIN
    CREATE TABLE ElleCore.dbo.UserContinuity (
        nUserNo               INT          NOT NULL PRIMARY KEY,
        sUserID_cached        NVARCHAR(30) NULL,
        sUserName_cached      NVARCHAR(10) NULL,

        first_met_ms          BIGINT       NOT NULL,
        last_seen_ms          BIGINT       NOT NULL,
        total_conversations   INT          NOT NULL DEFAULT 0,
        total_messages        INT          NOT NULL DEFAULT 0,
        total_pairings        INT          NOT NULL DEFAULT 0,

        last_bond_score       FLOAT        NULL,
        last_bond_label       NVARCHAR(32) NULL,
        last_bond_updated_ms  BIGINT       NULL,

        private_note          NVARCHAR(4000) NULL,

        created_ms            BIGINT       NOT NULL,
        updated_ms            BIGINT       NOT NULL
    );
    PRINT 'UserContinuity table created';
END
ELSE
BEGIN
    PRINT 'UserContinuity already exists';
END
GO

IF OBJECT_ID('ElleCore.dbo.GameSessionState', 'U') IS NULL
BEGIN
    CREATE TABLE ElleCore.dbo.GameSessionState (
        nUserNo            INT          NOT NULL PRIMARY KEY,
        char_index         INT          NULL,
        char_name          NVARCHAR(20) NULL,
        zone_id            INT          NULL,
        zone_name          NVARCHAR(40) NULL,
        last_x             FLOAT        NULL,
        last_y             FLOAT        NULL,
        last_z             FLOAT        NULL,
        last_hp            INT          NULL,
        last_hp_max        INT          NULL,
        last_session_ms    BIGINT       NOT NULL,
        last_disconnect_ms BIGINT       NULL,
        last_disconnect_reason NVARCHAR(64) NULL
    );
    PRINT 'GameSessionState table created';
END
ELSE
BEGIN
    PRINT 'GameSessionState already exists';
END
GO

PRINT 'ElleAnn_GameUnification.sql applied OK';
