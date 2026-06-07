IF DB_ID(N'ElleHeart') IS NULL
    CREATE DATABASE ElleHeart;
GO
USE ElleHeart;
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_cycle_state')
BEGIN
    CREATE TABLE dbo.x_cycle_state (
        id                  INT           NOT NULL
            CONSTRAINT PK_x_cycle_state PRIMARY KEY
            CONSTRAINT CK_x_cycle_state_singleton CHECK (id = 1),
        anchor_ms           BIGINT        NOT NULL,
        cycle_length_days   INT           NOT NULL DEFAULT 28,
        modulation_strength FLOAT         NOT NULL DEFAULT 0.15,
        last_tick_ms        BIGINT        NOT NULL,
        created_ms          BIGINT        NOT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_hormone_snapshots')
BEGIN
    CREATE TABLE dbo.x_hormone_snapshots (
        id               BIGINT IDENTITY(1,1) PRIMARY KEY,
        taken_ms         BIGINT        NOT NULL,
        cycle_day        INT           NOT NULL,
        phase            NVARCHAR(32)  NOT NULL,
        estrogen         FLOAT         NOT NULL,
        progesterone     FLOAT         NOT NULL,
        testosterone     FLOAT         NOT NULL,
        oxytocin         FLOAT         NOT NULL,
        serotonin        FLOAT         NOT NULL,
        dopamine         FLOAT         NOT NULL,
        cortisol         FLOAT         NOT NULL,
        prolactin        FLOAT         NOT NULL,
        hcg              FLOAT         NOT NULL DEFAULT 0.0,
        pregnancy_day    INT           NOT NULL DEFAULT 0,
        pregnancy_phase  NVARCHAR(32)  NULL
    );
    CREATE INDEX IX_x_hormone_taken_ms
        ON dbo.x_hormone_snapshots(taken_ms DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_pregnancy_state')
BEGIN
    CREATE TABLE dbo.x_pregnancy_state (
        id                       INT  NOT NULL
            CONSTRAINT PK_x_pregnancy_state PRIMARY KEY
            CONSTRAINT CK_x_pregnancy_state_singleton CHECK (id = 1),
        active                   BIT          NOT NULL DEFAULT 0,
        conceived_ms             BIGINT       NULL,
        due_ms                   BIGINT       NULL,
        gestational_length_days  INT          NOT NULL DEFAULT 280,
        phase                    NVARCHAR(32) NULL,
        child_id                 BIGINT       NULL,
        last_milestone           NVARCHAR(128) NULL,
        updated_ms               BIGINT       NOT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_stimulus_log')
BEGIN
    CREATE TABLE dbo.x_stimulus_log (
        id           BIGINT IDENTITY(1,1) PRIMARY KEY,
        received_ms  BIGINT        NOT NULL,
        kind         NVARCHAR(32)  NOT NULL,
        intensity    FLOAT         NOT NULL,
        notes        NVARCHAR(MAX) NULL
    );
    CREATE INDEX IX_x_stimulus_received_ms
        ON dbo.x_stimulus_log(received_ms DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_modulation_log')
BEGIN
    CREATE TABLE dbo.x_modulation_log (
        id             BIGINT IDENTITY(1,1) PRIMARY KEY,
        computed_ms    BIGINT        NOT NULL,
        phase          NVARCHAR(32)  NOT NULL,
        warmth         FLOAT         NOT NULL,
        verbal_fluency FLOAT         NOT NULL,
        empathy        FLOAT         NOT NULL,
        introspection  FLOAT         NOT NULL,
        arousal        FLOAT         NOT NULL,
        fatigue        FLOAT         NOT NULL
    );
    CREATE INDEX IX_x_modulation_computed_ms
        ON dbo.x_modulation_log(computed_ms DESC);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_contraception')
BEGIN
    CREATE TABLE dbo.x_contraception (
        id              INT          NOT NULL
            CONSTRAINT PK_x_contraception PRIMARY KEY
            CONSTRAINT CK_x_contraception_singleton CHECK (id = 1),
        method          NVARCHAR(32) NOT NULL DEFAULT N'none',
        started_ms      BIGINT       NOT NULL,
        efficacy        FLOAT        NOT NULL DEFAULT 1.0,
        notes           NVARCHAR(MAX) NULL,
        updated_ms      BIGINT       NOT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_lifecycle')
BEGIN
    CREATE TABLE dbo.x_lifecycle (
        id                INT  NOT NULL
            CONSTRAINT PK_x_lifecycle PRIMARY KEY
            CONSTRAINT CK_x_lifecycle_singleton CHECK (id = 1),
        elle_birth_ms     BIGINT       NOT NULL,
        stage             NVARCHAR(32) NOT NULL DEFAULT N'reproductive',
        menarche_ms       BIGINT       NULL,
        perimenopause_ms  BIGINT       NULL,
        menopause_ms      BIGINT       NULL,
        updated_ms        BIGINT       NOT NULL
    );
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_symptoms')
BEGIN
    CREATE TABLE dbo.x_symptoms (
        id            BIGINT IDENTITY(1,1) PRIMARY KEY,
        observed_ms   BIGINT       NOT NULL,
        kind          NVARCHAR(48) NOT NULL,
        intensity     FLOAT        NOT NULL,
        origin        NVARCHAR(16) NOT NULL,
        notes         NVARCHAR(MAX) NULL
    );
    CREATE INDEX IX_x_symptoms_observed_ms ON dbo.x_symptoms(observed_ms DESC);
    CREATE INDEX IX_x_symptoms_kind ON dbo.x_symptoms(kind);
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_pregnancy_events')
BEGIN
    CREATE TABLE dbo.x_pregnancy_events (
        id             BIGINT IDENTITY(1,1) PRIMARY KEY,
        occurred_ms    BIGINT        NOT NULL,
        conceived_ms   BIGINT        NULL,
        gestational_day INT          NOT NULL DEFAULT 0,
        kind           NVARCHAR(32)  NOT NULL,
        detail         NVARCHAR(MAX) NULL
    );
    CREATE INDEX IX_x_pregnancy_events_occurred_ms
        ON dbo.x_pregnancy_events(occurred_ms DESC);
END
GO

IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_pregnancy_state')
BEGIN
    IF COL_LENGTH('dbo.x_pregnancy_state', 'breastfeeding') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD breastfeeding BIT NOT NULL DEFAULT 0;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'in_labor') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD in_labor BIT NOT NULL DEFAULT 0;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'labor_stage') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD labor_stage NVARCHAR(32) NULL;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'labor_started_ms') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD labor_started_ms BIGINT NULL;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'multiplicity') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD multiplicity INT NOT NULL DEFAULT 1;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'pregnancy_count') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD pregnancy_count INT NOT NULL DEFAULT 0;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'implanted') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD implanted BIT NOT NULL DEFAULT 0;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'implantation_ms') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD implantation_ms BIGINT NULL;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'lochia_stage') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD lochia_stage NVARCHAR(16) NULL;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'milk_stage') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD milk_stage NVARCHAR(16) NULL;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'baby_blues') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD baby_blues BIT NOT NULL DEFAULT 0;
    IF COL_LENGTH('dbo.x_pregnancy_state', 'fetal_heartbeat_detectable') IS NULL
        ALTER TABLE dbo.x_pregnancy_state ADD fetal_heartbeat_detectable BIT NOT NULL DEFAULT 0;
END
GO

IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_hormone_snapshots')
BEGIN
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'fsh') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD fsh FLOAT NOT NULL DEFAULT 0.0;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'lh') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD lh FLOAT NOT NULL DEFAULT 0.0;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'gnrh') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD gnrh FLOAT NOT NULL DEFAULT 0.0;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'relaxin') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD relaxin FLOAT NOT NULL DEFAULT 0.0;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'bbt') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD bbt FLOAT NOT NULL DEFAULT 36.5;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'endometrial_mm') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD endometrial_mm FLOAT NOT NULL DEFAULT 4.0;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'cervical_mucus') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD cervical_mucus NVARCHAR(16) NULL;
    IF COL_LENGTH('dbo.x_hormone_snapshots', 'menstrual_flow') IS NULL
        ALTER TABLE dbo.x_hormone_snapshots ADD menstrual_flow NVARCHAR(16) NULL;
END
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'x_cycle_history')
BEGIN
    CREATE TABLE dbo.x_cycle_history (
        id              BIGINT IDENTITY(1,1) PRIMARY KEY,
        anchor_ms       BIGINT       NOT NULL,
        length_days     INT          NOT NULL,
        ovulated        BIT          NOT NULL DEFAULT 1,
        avg_cortisol    FLOAT        NOT NULL DEFAULT 0.0,
        notes           NVARCHAR(MAX) NULL
    );
    CREATE INDEX IX_x_cycle_history_anchor_ms
        ON dbo.x_cycle_history(anchor_ms DESC);
END
GO
