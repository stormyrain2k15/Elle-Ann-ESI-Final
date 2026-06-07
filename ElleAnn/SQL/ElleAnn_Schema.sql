SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'ElleCore')
    CREATE DATABASE ElleCore;
GO

USE ElleCore;
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'conversations')
BEGIN
    CREATE TABLE dbo.conversations (
        id              INT IDENTITY(1,1) PRIMARY KEY,
        user_id         INT NULL,
        title           NVARCHAR(256) NULL,
        started_at      DATETIME2(3) NOT NULL DEFAULT GETUTCDATE(),
        last_message_at DATETIME2(3) NULL,
        total_messages  INT NOT NULL DEFAULT 0,
        is_active       BIT NOT NULL DEFAULT 1
    );
    CREATE INDEX IX_conversations_user_id
        ON dbo.conversations(user_id, last_message_at DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'messages')
BEGIN
    CREATE TABLE dbo.messages (
        id                 BIGINT IDENTITY(1,1) PRIMARY KEY,
        conversation_id    INT NULL,
        user_id            INT NULL,
        role               INT NOT NULL,
        content            NVARCHAR(MAX) NOT NULL,
        emotion_detected   NVARCHAR(64) NULL,
        emotion_intensity  FLOAT NULL,
        Direction          INT NOT NULL DEFAULT 0,
        created_at         DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_messages_conv_created
        ON dbo.messages(conversation_id, created_at);
    CREATE INDEX IX_messages_user_created
        ON dbo.messages(user_id, created_at);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'voice_calls')
BEGIN
    CREATE TABLE dbo.voice_calls (
        id                INT IDENTITY(1,1) PRIMARY KEY,
        call_id           NVARCHAR(64) NOT NULL UNIQUE,
        user_id           INT NULL,
        conversation_id   INT NULL,
        started_at        DATETIME2(3) NULL,
        ended_at          DATETIME2(3) NULL,
        duration_seconds  INT NULL,
        status            NVARCHAR(16) NOT NULL DEFAULT 'active',
        created_at        DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'user_avatars')
BEGIN
    CREATE TABLE dbo.user_avatars (
        id          INT IDENTITY(1,1) PRIMARY KEY,
        user_id     INT NOT NULL,
        label       NVARCHAR(128) NULL,
        file_path   NVARCHAR(512) NOT NULL,
        mime_type   NVARCHAR(64) NULL,
        is_default  BIT NOT NULL DEFAULT 0,
        created_at  DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_user_avatars_user
        ON dbo.user_avatars(user_id, is_default DESC, id DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'IntentQueue')
BEGIN
    CREATE TABLE dbo.IntentQueue (
        Id              BIGINT IDENTITY(1,1) PRIMARY KEY,
        IntentType      INT NOT NULL,
        Status          INT NOT NULL DEFAULT 0,
        SourceDrive     INT NOT NULL DEFAULT 0,
        Urgency         FLOAT NOT NULL DEFAULT 0.5,
        Confidence      FLOAT NOT NULL DEFAULT 0.5,
        Description     NVARCHAR(MAX) NULL,
        Parameters      NVARCHAR(MAX) NULL,
        RequiredTrust   INT NOT NULL DEFAULT 0,
        CreatedMs       BIGINT NOT NULL,
        TimeoutMs       BIGINT NOT NULL DEFAULT 30000,
        ProcessingMs    BIGINT NULL,
        Result          NVARCHAR(MAX) NULL,
        ErrorCode       INT NOT NULL DEFAULT 0
    );
    CREATE INDEX IX_IntentQueue_Status
        ON dbo.IntentQueue(Status, CreatedMs);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'action_queue')
BEGIN
    CREATE TABLE dbo.action_queue (
        id              BIGINT IDENTITY(1,1) PRIMARY KEY,
        intent_id       BIGINT NULL,
        action_type     INT NOT NULL,
        status          INT NOT NULL DEFAULT 0,
        command         NVARCHAR(MAX) NULL,
        parameters      NVARCHAR(MAX) NULL,
        result          NVARCHAR(MAX) NULL,
        required_trust  INT NOT NULL DEFAULT 0,
        trust_delta     INT NOT NULL DEFAULT 0,
        created_ms      BIGINT NOT NULL,
        started_ms      BIGINT NULL,
        completed_ms    BIGINT NULL,
        timeout_ms      BIGINT NOT NULL DEFAULT 30000,
        error_code      INT NOT NULL DEFAULT 0
    );
    CREATE INDEX IX_action_queue_status
        ON dbo.action_queue(status, created_ms);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'goals')
BEGIN
    CREATE TABLE dbo.goals (
        id                BIGINT IDENTITY(1,1) PRIMARY KEY,
        description       NVARCHAR(MAX) NOT NULL,
        status            INT NOT NULL DEFAULT 0,
        priority          INT NOT NULL DEFAULT 2,
        progress          FLOAT NOT NULL DEFAULT 0.0,
        motivation        FLOAT NOT NULL DEFAULT 0.7,
        source_drive      INT NOT NULL DEFAULT 0,
        parent_goal_id    BIGINT NULL,
        success_criteria  NVARCHAR(MAX) NULL,
        created_ms        BIGINT NOT NULL,
        deadline_ms       BIGINT NULL,
        last_progress_ms  BIGINT NULL,
        attempts          INT NOT NULL DEFAULT 0
    );
    CREATE INDEX IX_goals_status_priority
        ON dbo.goals(status, priority DESC, created_ms);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory')
BEGIN
    CREATE TABLE dbo.memory (
        id                  BIGINT IDENTITY(1,1) PRIMARY KEY,
        memory_type         INT NOT NULL DEFAULT 0,
        tier                INT NOT NULL DEFAULT 1,
        content             NVARCHAR(MAX) NOT NULL,
        summary             NVARCHAR(1024) NULL,
        emotional_valence   FLOAT NOT NULL DEFAULT 0.0,
        importance          FLOAT NOT NULL DEFAULT 0.5,
        relevance           FLOAT NOT NULL DEFAULT 0.5,
        position_x          FLOAT NULL,
        position_y          FLOAT NULL,
        position_z          FLOAT NULL,
        created_ms          BIGINT NOT NULL,
        last_access_ms      BIGINT NOT NULL
    );
    CREATE INDEX IX_memory_tier_created
        ON dbo.memory(tier, created_ms DESC);
    CREATE INDEX IX_memory_last_access
        ON dbo.memory(last_access_ms DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory_tags')
BEGIN
    CREATE TABLE dbo.memory_tags (
        id          BIGINT IDENTITY(1,1) PRIMARY KEY,
        memory_id   BIGINT NOT NULL,
        tag         NVARCHAR(128) NOT NULL
    );
    CREATE INDEX IX_memory_tags_memory ON dbo.memory_tags(memory_id);
    CREATE INDEX IX_memory_tags_tag    ON dbo.memory_tags(tag);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory_entity_links')
BEGIN
    CREATE TABLE dbo.memory_entity_links (
        id          BIGINT IDENTITY(1,1) PRIMARY KEY,
        memory_id   BIGINT NOT NULL,
        entity_id   BIGINT NOT NULL
    );
    CREATE UNIQUE INDEX UX_mel_pair
        ON dbo.memory_entity_links(memory_id, entity_id);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'world_entity')
BEGIN
    CREATE TABLE dbo.world_entity (
        id           BIGINT IDENTITY(1,1) PRIMARY KEY,
        name         NVARCHAR(256) NOT NULL UNIQUE,
        kind         NVARCHAR(64) NULL,
        properties   NVARCHAR(MAX) NULL,
        first_seen_ms BIGINT NULL,
        last_seen_ms  BIGINT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'emotion_snapshots')
BEGIN
    CREATE TABLE dbo.emotion_snapshots (
        id          BIGINT IDENTITY(1,1) PRIMARY KEY,
        valence     FLOAT NOT NULL,
        arousal     FLOAT NOT NULL,
        dominance   FLOAT NOT NULL,
        dimensions  NVARCHAR(MAX) NOT NULL,
        taken_ms    BIGINT NOT NULL,
        created_at  DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_emotion_taken ON dbo.emotion_snapshots(taken_ms DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'dictionary_words')
BEGIN
    CREATE TABLE dbo.dictionary_words (
        id              BIGINT IDENTITY(1,1) PRIMARY KEY,
        word            NVARCHAR(128) NOT NULL,
        definition      NVARCHAR(MAX) NULL,
        example         NVARCHAR(MAX) NULL,
        part_of_speech  NVARCHAR(32)  NULL
    );
    CREATE INDEX IX_dictionary_words_word ON dbo.dictionary_words(word);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'dictionary_loader_state')
BEGIN
    CREATE TABLE dbo.dictionary_loader_state (
        id              INT IDENTITY(1,1) PRIMARY KEY,
        last_offset     BIGINT NOT NULL DEFAULT 0,
        last_word       NVARCHAR(128) NULL,
        words_loaded    BIGINT NOT NULL DEFAULT 0,
        updated_at      DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'learned_subjects')
BEGIN
    CREATE TABLE dbo.learned_subjects (
        id                    INT IDENTITY(1,1) PRIMARY KEY,
        subject               NVARCHAR(256) NOT NULL,
        category              NVARCHAR(128) NULL,
        proficiency_level     INT NOT NULL DEFAULT 0,
        who_taught            NVARCHAR(256) NULL,
        where_learned         NVARCHAR(256) NULL,
        time_to_learn_hours   FLOAT NULL,
        notes                 NVARCHAR(MAX) NULL,
        created_at            DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'education_references')
BEGIN
    CREATE TABLE dbo.education_references (
        id                INT IDENTITY(1,1) PRIMARY KEY,
        subject_id        INT NULL,
        reference_type    NVARCHAR(64) NULL,
        reference_title   NVARCHAR(512) NULL,
        reference_content NVARCHAR(MAX) NULL,
        file_path         NVARCHAR(512) NULL,
        relevance_score   FLOAT NULL,
        notes             NVARCHAR(MAX) NULL,
        created_at        DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'learning_milestones')
BEGIN
    CREATE TABLE dbo.learning_milestones (
        id           INT IDENTITY(1,1) PRIMARY KEY,
        subject_id   INT NOT NULL,
        milestone    NVARCHAR(256) NOT NULL,
        description  NVARCHAR(MAX) NULL,
        achieved_at  DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'skills')
BEGIN
    CREATE TABLE dbo.skills (
        id                       INT IDENTITY(1,1) PRIMARY KEY,
        skill_name               NVARCHAR(256) NOT NULL UNIQUE,
        category                 NVARCHAR(128) NULL,
        proficiency              INT NOT NULL DEFAULT 0,
        learned_from_subject_id  INT NULL,
        notes                    NVARCHAR(MAX) NULL,
        created_at               DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'system_settings')
BEGIN
    CREATE TABLE dbo.system_settings (
        setting_key     NVARCHAR(128) NOT NULL PRIMARY KEY,
        setting_value   NVARCHAR(MAX) NULL,
        updated_at      DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'hardware_actions')
BEGIN
    CREATE TABLE dbo.hardware_actions (
        id           BIGINT IDENTITY(1,1) PRIMARY KEY,
        action_type  NVARCHAR(64) NOT NULL,
        target       NVARCHAR(256) NULL,
        result       NVARCHAR(MAX) NULL,
        created_at   DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'video_jobs')
BEGIN
    CREATE TABLE dbo.video_jobs (
        id            BIGINT IDENTITY(1,1) PRIMARY KEY,
        job_uuid      NVARCHAR(64) NOT NULL UNIQUE,
        text          NVARCHAR(MAX) NOT NULL,
        avatar_path   NVARCHAR(512) NULL,
        call_id       INT NULL,
        status        NVARCHAR(16) NOT NULL DEFAULT 'queued',
        progress      FLOAT NOT NULL DEFAULT 0.0,
        result_path   NVARCHAR(512) NULL,
        error         NVARCHAR(MAX) NULL,
        created_ms    BIGINT NOT NULL,
        completed_ms  BIGINT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'UserPresence')
BEGIN
    CREATE TABLE dbo.UserPresence (
        user_id                            INT NOT NULL PRIMARY KEY,
        last_seen                          DATETIME2(3) NOT NULL DEFAULT GETUTCDATE(),
        current_silence_duration_minutes   INT NOT NULL DEFAULT 0,
        silence_threshold_minutes          INT NOT NULL DEFAULT 120,
        presence_pulse_count               INT NOT NULL DEFAULT 0,
        abnormal_silence_count             INT NOT NULL DEFAULT 0
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'CrystalProfile')
BEGIN
    CREATE TABLE dbo.CrystalProfile (
        id                       INT IDENTITY(1,1) PRIMARY KEY,
        user_id                  INT NOT NULL,
        traits                   NVARCHAR(MAX) NULL,
        vulnerability_patterns   NVARCHAR(MAX) NULL,
        comfort_patterns         NVARCHAR(MAX) NULL,
        trigger_patterns         NVARCHAR(MAX) NULL,
        preferred_tone           NVARCHAR(64)  NULL,
        trust_level              FLOAT NOT NULL DEFAULT 0.0,
        intimacy_level           FLOAT NOT NULL DEFAULT 0.0,
        last_updated             DATETIME2(3) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_CrystalProfile_user
        ON dbo.CrystalProfile(user_id, last_updated DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'ElleThreads')
BEGIN
    CREATE TABLE dbo.ElleThreads (
        id                INT IDENTITY(1,1) PRIMARY KEY,
        topic             NVARCHAR(512) NOT NULL,
        summary           NVARCHAR(MAX) NULL,
        emotional_weight  FLOAT NOT NULL DEFAULT 0.0,
        last_touched      DATETIME2(3) NOT NULL DEFAULT GETUTCDATE(),
        status            NVARCHAR(32) NULL
    );
    CREATE INDEX IX_ElleThreads_status_weight
        ON dbo.ElleThreads(status, emotional_weight DESC, last_touched DESC);
END
GO

PRINT 'ElleAnn_Schema.sql applied OK';
GO
