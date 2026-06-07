SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

USE ElleCore;
GO

DECLARE @rowCount BIGINT;
DECLARE @msg NVARCHAR(400);

IF OBJECT_ID('dbo.Users', 'U') IS NOT NULL
BEGIN
    SELECT @rowCount = COUNT_BIG(*) FROM dbo.Users;
    IF @rowCount = 0
    BEGIN
        DROP TABLE dbo.Users;
        PRINT 'dropped dbo.Users (was empty)';
    END
    ELSE
    BEGIN
        SET @msg = N'WARNING: dbo.Users has ' + CAST(@rowCount AS NVARCHAR(20))
                 + N' row(s).  Auth identity should come from Account.dbo.tUser, '
                 + N'NOT from this table.  Inspect rows, copy any nUserNo '
                 + N'mapping you need to ElleCore.dbo.UserContinuity, then '
                 + N'manually DROP TABLE dbo.Users.';
        PRINT @msg;
    END
END
GO

DECLARE @legacy TABLE (name SYSNAME);
INSERT INTO @legacy(name) VALUES
    ('Conversations'),
    ('Messages'),
    ('VoiceCalls'),
    ('IntentQueue_Legacy'),
    ('ActionQueue'),
    ('Goals'),
    ('Memories'),
    ('MemoryTags'),
    ('MemoryClusters'),
    ('MemoryLinks'),
    ('MemoryPositions'),
    ('EmotionHistory'),
    ('Dictionary'),
    ('Education'),
    ('Morals'),
    ('Triggers'),
    ('WorldEntities'),
    ('Predictions'),
    ('Workers'),
    ('Models'),
    ('Analytics'),
    ('Logs'),
    ('Backups'),
    ('TrustState'),
    ('TrustAudit'),
    ('AttachmentProfiles'),
    ('Bonds'),
    ('LoveScore');

DECLARE @t SYSNAME;
DECLARE legacy_cur CURSOR LOCAL FAST_FORWARD FOR
    SELECT name FROM @legacy;
OPEN legacy_cur;
FETCH NEXT FROM legacy_cur INTO @t;
WHILE @@FETCH_STATUS = 0
BEGIN
    IF OBJECT_ID('dbo.' + QUOTENAME(@t), 'U') IS NOT NULL
    BEGIN
        DECLARE @sql NVARCHAR(400);
        DECLARE @c   BIGINT;
        SET @sql = N'SELECT @c = COUNT_BIG(*) FROM dbo.' + QUOTENAME(@t) + N';';
        EXEC sp_executesql @sql, N'@c BIGINT OUTPUT', @c = @c OUTPUT;
        IF @c = 0
        BEGIN
            SET @sql = N'DROP TABLE dbo.' + QUOTENAME(@t) + N';';
            EXEC sp_executesql @sql;
            PRINT 'dropped dbo.' + @t + ' (was empty)';
        END
        ELSE
        BEGIN
            PRINT 'kept   dbo.' + @t + ' — has rows; review manually';
        END
    END
    FETCH NEXT FROM legacy_cur INTO @t;
END
CLOSE legacy_cur;
DEALLOCATE legacy_cur;
GO

IF EXISTS (SELECT 1 FROM sys.foreign_keys
            WHERE referenced_object_id = OBJECT_ID('dbo.Users')
               OR referenced_object_id = OBJECT_ID('dbo.Conversations'))
BEGIN
    DECLARE @fkSql NVARCHAR(400);
    DECLARE fk_cur CURSOR LOCAL FAST_FORWARD FOR
        SELECT N'ALTER ' + N'TABLE dbo.' + QUOTENAME(OBJECT_NAME(parent_object_id))
             + N' DROP CONSTRAINT ' + QUOTENAME(name) + N';'
          FROM sys.foreign_keys
         WHERE referenced_object_id = OBJECT_ID('dbo.Users')
            OR referenced_object_id = OBJECT_ID('dbo.Conversations');
    OPEN fk_cur;
    FETCH NEXT FROM fk_cur INTO @fkSql;
    WHILE @@FETCH_STATUS = 0
    BEGIN
        EXEC sp_executesql @fkSql;
        PRINT @fkSql;
        FETCH NEXT FROM fk_cur INTO @fkSql;
    END
    CLOSE fk_cur;
    DEALLOCATE fk_cur;
END
GO

PRINT 'ElleAnn_SchemaSync_FebPivot.sql applied OK';
PRINT 'NEXT STEP: run ElleAnn_Schema.sql to create the canonical tables.';
GO
