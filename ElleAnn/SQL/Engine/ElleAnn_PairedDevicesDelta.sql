USE ElleCore;
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.objects
    WHERE object_id = OBJECT_ID(N'dbo.PairedDevices') AND type = N'U'
)
BEGIN
    CREATE TABLE dbo.PairedDevices (
        DeviceId        NVARCHAR(128) NOT NULL PRIMARY KEY,
        DeviceName      NVARCHAR(256) NOT NULL,
        PairedAtMs      BIGINT        NOT NULL,
        ExpiresMs       BIGINT        NOT NULL,
        LastSeenMs      BIGINT        NULL,
        Revoked         BIT           NOT NULL DEFAULT 0,
        RevokedAtMs     BIGINT        NULL,
        JwtFingerprint  NVARCHAR(64)  NOT NULL
    );
END
GO

IF NOT EXISTS (
    SELECT 1 FROM sys.indexes
    WHERE name = 'IX_PairedDevices_Active_PairedAtMs'
      AND object_id = OBJECT_ID(N'dbo.PairedDevices')
)
BEGIN
    CREATE INDEX IX_PairedDevices_Active_PairedAtMs
        ON dbo.PairedDevices (PairedAtMs DESC)
        INCLUDE (DeviceName, ExpiresMs, LastSeenMs)
        WHERE Revoked = 0;
END
GO
