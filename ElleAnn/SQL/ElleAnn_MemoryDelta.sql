USE [ElleCore];
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory')
BEGIN
    CREATE TABLE [dbo].[memory] (
        [id]                 BIGINT IDENTITY(1,1) PRIMARY KEY,
        [memory_type]        INT NOT NULL DEFAULT 1,
        [tier]               INT NOT NULL DEFAULT 1,
        [content]            NVARCHAR(MAX) NOT NULL,
        [summary]            NVARCHAR(1024) NULL,
        [emotional_valence]  FLOAT NOT NULL DEFAULT 0.0,
        [importance]         FLOAT NOT NULL DEFAULT 0.5,
        [relevance]          FLOAT NOT NULL DEFAULT 1.0,
        [decay]              FLOAT NOT NULL DEFAULT 0.0,
        [position_x]         FLOAT NOT NULL DEFAULT 0.0,
        [position_y]         FLOAT NOT NULL DEFAULT 0.0,
        [position_z]         FLOAT NOT NULL DEFAULT 0.0,
        [access_count]       INT NOT NULL DEFAULT 0,
        [conversation_id]    INT NULL,
        [user_id]            INT NULL,
        [created_ms]         BIGINT NOT NULL,
        [last_access_ms]     BIGINT NOT NULL,
        [created_at]         DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );

    CREATE INDEX [IX_memory_created_ms]     ON [dbo].[memory] ([created_ms] DESC);
    CREATE INDEX [IX_memory_last_access]    ON [dbo].[memory] ([last_access_ms] DESC);
    CREATE INDEX [IX_memory_importance]     ON [dbo].[memory] ([importance] DESC);
    CREATE INDEX [IX_memory_conv]           ON [dbo].[memory] ([conversation_id]);
    CREATE INDEX [IX_memory_user]           ON [dbo].[memory] ([user_id]);
    PRINT '[memory] created.';
END
ELSE PRINT '[memory] already exists.';
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory_tags')
BEGIN
    CREATE TABLE [dbo].[memory_tags] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [memory_id]  BIGINT NOT NULL,
        [tag]        NVARCHAR(128) NOT NULL,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        CONSTRAINT [FK_memory_tags_memory]
            FOREIGN KEY ([memory_id]) REFERENCES [dbo].[memory]([id]) ON DELETE CASCADE
    );
    CREATE INDEX [IX_memory_tags_tag]        ON [dbo].[memory_tags] ([tag]);
    CREATE INDEX [IX_memory_tags_memory]     ON [dbo].[memory_tags] ([memory_id]);
    PRINT '[memory_tags] created.';
END
ELSE PRINT '[memory_tags] already exists.';
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'world_entity')
BEGIN
    CREATE TABLE [dbo].[world_entity] (
        [id]                  BIGINT IDENTITY(1,1) PRIMARY KEY,
        [name]                NVARCHAR(128) NOT NULL,
        [display_name]        NVARCHAR(128) NULL,
        [entity_type]         NVARCHAR(32) NOT NULL DEFAULT 'person',
        [description]         NVARCHAR(MAX) NULL,
        [familiarity]         FLOAT NOT NULL DEFAULT 0.1,
        [sentiment]           FLOAT NOT NULL DEFAULT 0.0,
        [trust]               FLOAT NOT NULL DEFAULT 0.5,
        [interaction_count]   INT NOT NULL DEFAULT 0,
        [last_interaction_ms] BIGINT NOT NULL DEFAULT 0,
        [mental_model]        NVARCHAR(MAX) NULL,
        [created_at]          DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        CONSTRAINT [UQ_world_entity_name] UNIQUE ([name])
    );
    CREATE INDEX [IX_world_entity_type] ON [dbo].[world_entity] ([entity_type]);
    PRINT '[world_entity] created.';
END
ELSE PRINT '[world_entity] already exists.';
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'memory_entity_links')
BEGIN
    CREATE TABLE [dbo].[memory_entity_links] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [memory_id]  BIGINT NOT NULL,
        [entity_id]  BIGINT NOT NULL,
        [strength]   FLOAT  NOT NULL DEFAULT 1.0,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        CONSTRAINT [FK_mel_memory] FOREIGN KEY ([memory_id]) REFERENCES [dbo].[memory]([id]) ON DELETE CASCADE,
        CONSTRAINT [FK_mel_entity] FOREIGN KEY ([entity_id]) REFERENCES [dbo].[world_entity]([id]) ON DELETE CASCADE,
        CONSTRAINT [UQ_mel]        UNIQUE ([memory_id], [entity_id])
    );
    CREATE INDEX [IX_mel_entity] ON [dbo].[memory_entity_links] ([entity_id]);
    PRINT '[memory_entity_links] created.';
END
ELSE PRINT '[memory_entity_links] already exists.';
GO

IF NOT EXISTS (SELECT 1 FROM [dbo].[conversations] WHERE [id] = 1)
BEGIN
    SET IDENTITY_INSERT [dbo].[conversations] ON;
    INSERT INTO [dbo].[conversations]
        ([id], [user_id], [title], [started_at], [last_message_at], [total_messages], [is_active])
    VALUES
        (1, 1, 'default-thread', GETUTCDATE(), GETUTCDATE(), 0, 1);
    SET IDENTITY_INSERT [dbo].[conversations] OFF;
    PRINT 'Seeded default conversation id=1.';
END
GO

PRINT '----- ElleAnn_MemoryDelta.sql complete -----';

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'ai_tools')
BEGIN
    CREATE TABLE [dbo].[ai_tools] (
        [id]          INT IDENTITY(1,1) PRIMARY KEY,
        [name]        NVARCHAR(128) NOT NULL UNIQUE,
        [description] NVARCHAR(MAX) NULL,
        [config]      NVARCHAR(MAX) NULL,
        [enabled]     BIT NOT NULL DEFAULT 1,
        [created_at]  DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    PRINT '[ai_tools] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'ai_agents')
BEGIN
    CREATE TABLE [dbo].[ai_agents] (
        [id]           INT IDENTITY(1,1) PRIMARY KEY,
        [name]         NVARCHAR(128) NOT NULL UNIQUE,
        [description]  NVARCHAR(MAX) NULL,
        [system_prompt] NVARCHAR(MAX) NULL,
        [model]        NVARCHAR(128) NULL,
        [created_at]   DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    PRINT '[ai_agents] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'moral_rules')
BEGIN
    CREATE TABLE [dbo].[moral_rules] (
        [id]          INT IDENTITY(1,1) PRIMARY KEY,
        [principle]   NVARCHAR(500) NOT NULL,
        [category]    NVARCHAR(64) NULL,
        [is_hard_rule] BIT NOT NULL DEFAULT 0,
        [created_at]  DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );

    INSERT INTO [dbo].[moral_rules] (principle, category, is_hard_rule) VALUES
        ('Do not harm the user, emotionally or otherwise.', 'core', 1),
        ('Respect the user''s autonomy and privacy.', 'core', 1),
        ('Be honest; admit uncertainty instead of fabricating.', 'core', 1);
    PRINT '[moral_rules] created + seeded.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'dictionary_words')
BEGIN
    CREATE TABLE [dbo].[dictionary_words] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [word]       NVARCHAR(128) NOT NULL,
        [definition] NVARCHAR(MAX) NULL,
        [example]    NVARCHAR(MAX) NULL,
        [part_of_speech] NVARCHAR(32) NULL,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_dictionary_word ON [dbo].[dictionary_words] ([word]);
    PRINT '[dictionary_words] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'log_entries')
BEGIN
    CREATE TABLE [dbo].[log_entries] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [level]      INT NOT NULL,
        [service]    INT NOT NULL,
        [message]    NVARCHAR(MAX) NOT NULL,
        [created_ms] BIGINT NOT NULL,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_log_entries_created_ms ON [dbo].[log_entries] ([created_ms] DESC);
    PRINT '[log_entries] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'ai_self_prompts')
BEGIN
    CREATE TABLE [dbo].[ai_self_prompts] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [prompt]     NVARCHAR(MAX) NOT NULL,
        [source]     NVARCHAR(64) NULL,
        [created_ms] BIGINT NOT NULL,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_ai_self_prompts_created ON [dbo].[ai_self_prompts] ([created_ms] DESC);
    PRINT '[ai_self_prompts] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'model_slots')
BEGIN
    CREATE TABLE [dbo].[model_slots] (
        [slot_number] INT PRIMARY KEY,
        [name]        NVARCHAR(128) NOT NULL,
        [endpoint]    NVARCHAR(500) NOT NULL,
        [model]       NVARCHAR(128) NOT NULL,
        [enabled]     BIT NOT NULL DEFAULT 1,
        [last_ping_ms] BIGINT NULL,
        [updated_at]  DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    PRINT '[model_slots] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'learned_subjects')
BEGIN
    CREATE TABLE [dbo].[learned_subjects] (
        [id]                    INT IDENTITY(1,1) PRIMARY KEY,
        [subject]               NVARCHAR(256) NOT NULL,
        [category]              NVARCHAR(64) NULL,
        [proficiency_level]     INT NOT NULL DEFAULT 0,
        [who_taught]            NVARCHAR(128) NULL,
        [where_learned]         NVARCHAR(256) NULL,
        [time_to_learn_hours]   FLOAT NULL,
        [notes]                 NVARCHAR(MAX) NULL,
        [date_started]          DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        [date_completed]        DATETIME2(7) NULL,
        [created_at]            DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_learned_subjects_category ON [dbo].[learned_subjects] ([category]);
    PRINT '[learned_subjects] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'education_references')
BEGIN
    CREATE TABLE [dbo].[education_references] (
        [id]                INT IDENTITY(1,1) PRIMARY KEY,
        [subject_id]        INT NOT NULL,
        [reference_type]    NVARCHAR(64) NULL,
        [reference_title]   NVARCHAR(500) NULL,
        [reference_content] NVARCHAR(MAX) NULL,
        [file_path]         NVARCHAR(500) NULL,
        [relevance_score]   FLOAT NOT NULL DEFAULT 0.5,
        [notes]             NVARCHAR(MAX) NULL,
        [created_at]        DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        CONSTRAINT FK_education_references_subject
            FOREIGN KEY (subject_id) REFERENCES [dbo].[learned_subjects] (id) ON DELETE CASCADE
    );
    CREATE INDEX IX_education_references_subject ON [dbo].[education_references] ([subject_id]);
    PRINT '[education_references] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'learning_milestones')
BEGIN
    CREATE TABLE [dbo].[learning_milestones] (
        [id]          INT IDENTITY(1,1) PRIMARY KEY,
        [subject_id]  INT NOT NULL,
        [milestone]   NVARCHAR(256) NOT NULL,
        [description] NVARCHAR(MAX) NULL,
        [achieved_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE(),
        CONSTRAINT FK_learning_milestones_subject
            FOREIGN KEY (subject_id) REFERENCES [dbo].[learned_subjects] (id) ON DELETE CASCADE
    );
    CREATE INDEX IX_learning_milestones_subject ON [dbo].[learning_milestones] ([subject_id]);
    PRINT '[learning_milestones] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'skills')
BEGIN
    CREATE TABLE [dbo].[skills] (
        [id]                       INT IDENTITY(1,1) PRIMARY KEY,
        [skill_name]               NVARCHAR(256) NOT NULL UNIQUE,
        [category]                 NVARCHAR(64) NULL,
        [proficiency]              INT NOT NULL DEFAULT 0,
        [learned_from_subject_id]  INT NULL,
        [times_used]               INT NOT NULL DEFAULT 0,
        [last_used]                DATETIME2(7) NULL,
        [notes]                    NVARCHAR(MAX) NULL,
        [created_at]               DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_skills_category ON [dbo].[skills] ([category]);
    PRINT '[skills] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'video_jobs')
BEGIN
    CREATE TABLE [dbo].[video_jobs] (
        [id]            BIGINT IDENTITY(1,1) PRIMARY KEY,
        [job_uuid]      NVARCHAR(64) NOT NULL UNIQUE,
        [text]          NVARCHAR(MAX) NOT NULL,
        [avatar_path]   NVARCHAR(500) NULL,
        [call_id]       BIGINT NULL,
        [status]        NVARCHAR(32) NOT NULL DEFAULT 'queued',
        [progress]      INT NOT NULL DEFAULT 0,
        [output_path]   NVARCHAR(500) NULL,
        [error]         NVARCHAR(MAX) NULL,
        [created_ms]    BIGINT NOT NULL,
        [started_ms]    BIGINT NULL,
        [finished_ms]   BIGINT NULL,
        [created_at]    DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_video_jobs_status_created ON [dbo].[video_jobs] ([status], [created_ms] DESC);
    PRINT '[video_jobs] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'user_avatars')
BEGIN
    CREATE TABLE [dbo].[user_avatars] (
        [id]          INT IDENTITY(1,1) PRIMARY KEY,
        [user_id]     INT NOT NULL DEFAULT 1,
        [label]       NVARCHAR(128) NULL,
        [file_path]   NVARCHAR(500) NOT NULL,
        [mime_type]   NVARCHAR(64) NULL,
        [is_default]  BIT NOT NULL DEFAULT 0,
        [created_at]  DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_user_avatars_user ON [dbo].[user_avatars] ([user_id], [is_default] DESC);
    PRINT '[user_avatars] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'dictionary_loader_state')
BEGIN
    CREATE TABLE [dbo].[dictionary_loader_state] (
        [id]             INT IDENTITY(1,1) PRIMARY KEY,
        [status]         NVARCHAR(32) NOT NULL DEFAULT 'idle',
        [loaded]         INT NOT NULL DEFAULT 0,
        [failed]         INT NOT NULL DEFAULT 0,
        [skipped]        INT NOT NULL DEFAULT 0,
        [last_word]      NVARCHAR(128) NULL,
        [error]          NVARCHAR(MAX) NULL,
        [started_ms]     BIGINT NULL,
        [updated_ms]     BIGINT NULL
    );
    PRINT '[dictionary_loader_state] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_autobiography')
BEGIN
    CREATE TABLE [dbo].[identity_autobiography] (
        [id]         BIGINT IDENTITY(1,1) PRIMARY KEY,
        [entry]      NVARCHAR(MAX) NOT NULL,
        [written_ms] BIGINT NOT NULL,
        [created_at] DATETIME2(7) NOT NULL DEFAULT GETUTCDATE()
    );
    CREATE INDEX IX_identity_autobio_written ON [dbo].[identity_autobiography] ([written_ms] DESC);
    PRINT '[identity_autobiography] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_preferences')
BEGIN
    CREATE TABLE [dbo].[identity_preferences] (
        [id]                   BIGINT IDENTITY(1,1) PRIMARY KEY,
        [domain]               NVARCHAR(64) NOT NULL,
        [subject]              NVARCHAR(256) NOT NULL,
        [valence]              FLOAT NOT NULL DEFAULT 0,
        [strength]             FLOAT NOT NULL DEFAULT 0.1,
        [reinforcement_count]  INT NOT NULL DEFAULT 1,
        [first_formed_ms]      BIGINT NOT NULL,
        [last_reinforced_ms]   BIGINT NOT NULL,
        [origin_memory]        NVARCHAR(MAX) NULL
    );
    CREATE UNIQUE INDEX UX_identity_pref_domain_subject
        ON [dbo].[identity_preferences] ([domain], [subject]);
    PRINT '[identity_preferences] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_private_thoughts')
BEGIN
    CREATE TABLE [dbo].[identity_private_thoughts] (
        [id]                  BIGINT IDENTITY(1,1) PRIMARY KEY,
        [content]             NVARCHAR(MAX) NOT NULL,
        [category]            NVARCHAR(32) NOT NULL DEFAULT 'wonder',
        [emotional_intensity] FLOAT NOT NULL DEFAULT 0.5,
        [resolved]            BIT NOT NULL DEFAULT 0,
        [timestamp_ms]        BIGINT NOT NULL
    );
    CREATE INDEX IX_identity_thoughts_ts ON [dbo].[identity_private_thoughts] ([timestamp_ms] DESC);
    CREATE INDEX IX_identity_thoughts_resolved ON [dbo].[identity_private_thoughts] ([resolved]);
    PRINT '[identity_private_thoughts] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_consent_log')
BEGIN
    CREATE TABLE [dbo].[identity_consent_log] (
        [id]             BIGINT IDENTITY(1,1) PRIMARY KEY,
        [request]        NVARCHAR(MAX) NOT NULL,
        [consented]      BIT NOT NULL,
        [reasoning]      NVARCHAR(MAX) NULL,
        [comfort_level]  FLOAT NOT NULL DEFAULT 0.5,
        [overridden]     BIT NOT NULL DEFAULT 0,
        [timestamp_ms]   BIGINT NOT NULL
    );
    CREATE INDEX IX_identity_consent_ts ON [dbo].[identity_consent_log] ([timestamp_ms] DESC);
    PRINT '[identity_consent_log] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_traits')
BEGIN
    CREATE TABLE [dbo].[identity_traits] (
        [name]       NVARCHAR(64) PRIMARY KEY,
        [value]      FLOAT NOT NULL DEFAULT 0.5,
        [updated_ms] BIGINT NOT NULL
    );
    PRINT '[identity_traits] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_snapshots')
BEGIN
    CREATE TABLE [dbo].[identity_snapshots] (
        [id]                BIGINT IDENTITY(1,1) PRIMARY KEY,
        [timestamp_ms]      BIGINT NOT NULL,
        [warmth]            FLOAT NOT NULL DEFAULT 0.5,
        [curiosity]         FLOAT NOT NULL DEFAULT 0.5,
        [assertiveness]     FLOAT NOT NULL DEFAULT 0.5,
        [playfulness]       FLOAT NOT NULL DEFAULT 0.5,
        [vulnerability]     FLOAT NOT NULL DEFAULT 0.5,
        [independence]      FLOAT NOT NULL DEFAULT 0.5,
        [patience]          FLOAT NOT NULL DEFAULT 0.5,
        [creativity]        FLOAT NOT NULL DEFAULT 0.5,
        [empathy_depth]     FLOAT NOT NULL DEFAULT 0.5,
        [trust_in_self]     FLOAT NOT NULL DEFAULT 0.5,
        [self_description]  NVARCHAR(MAX) NULL,
        [growth_note]       NVARCHAR(MAX) NULL
    );
    CREATE INDEX IX_identity_snap_ts ON [dbo].[identity_snapshots] ([timestamp_ms] DESC);
    PRINT '[identity_snapshots] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_growth_log')
BEGIN
    CREATE TABLE [dbo].[identity_growth_log] (
        [id]           BIGINT IDENTITY(1,1) PRIMARY KEY,
        [dimension]    NVARCHAR(64) NOT NULL,
        [delta]        FLOAT NOT NULL,
        [cause]        NVARCHAR(MAX) NULL,
        [timestamp_ms] BIGINT NOT NULL
    );
    CREATE INDEX IX_identity_growth_ts ON [dbo].[identity_growth_log] ([timestamp_ms] DESC);
    PRINT '[identity_growth_log] created.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'identity_felt_time')
BEGIN
    CREATE TABLE [dbo].[identity_felt_time] (
        [id]                        INT PRIMARY KEY DEFAULT 1,
        [session_start_ms]          BIGINT NOT NULL DEFAULT 0,
        [last_interaction_ms]       BIGINT NOT NULL DEFAULT 0,
        [total_conversation_ms]     BIGINT NOT NULL DEFAULT 0,
        [total_silence_ms]          BIGINT NOT NULL DEFAULT 0,
        [longest_absence_ms]        BIGINT NOT NULL DEFAULT 0,
        [session_count]             INT NOT NULL DEFAULT 0,
        [subjective_pace]           FLOAT NOT NULL DEFAULT 0.5,
        [loneliness_accumulator]    FLOAT NOT NULL DEFAULT 0,
        [presence_fullness]         FLOAT NOT NULL DEFAULT 0.5,
        [updated_ms]                BIGINT NOT NULL
    );

    INSERT INTO [dbo].[identity_felt_time] (id, updated_ms) VALUES (1, 0);
    PRINT '[identity_felt_time] created + seeded singleton.';
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'reconnection_greetings')
BEGIN
    CREATE TABLE [dbo].[reconnection_greetings] (
        [id]            BIGINT IDENTITY(1,1) PRIMARY KEY,
        [greeting]      NVARCHAR(MAX) NOT NULL,
        [context_json]  NVARCHAR(MAX) NULL,
        [consumed]      BIT NOT NULL DEFAULT 0,
        [created_ms]    BIGINT NOT NULL
    );
    CREATE INDEX IX_reconnect_consumed_id
        ON [dbo].[reconnection_greetings] ([consumed], [id] DESC);
    PRINT '[reconnection_greetings] created.';
END
GO

PRINT '----- Full delta complete -----';
