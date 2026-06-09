USE ElleHeart;
GO
SET NOCOUNT ON;
GO

IF OBJECT_ID('dbo.belief_evidence','U')   IS NOT NULL DROP TABLE dbo.belief_evidence;
IF OBJECT_ID('dbo.belief_posterior','U')  IS NOT NULL DROP TABLE dbo.belief_posterior;
IF OBJECT_ID('dbo.belief_prior','U')      IS NOT NULL DROP TABLE dbo.belief_prior;
IF OBJECT_ID('dbo.belief_audit','U')      IS NOT NULL DROP TABLE dbo.belief_audit;
IF OBJECT_ID('dbo.belief_domain','U')     IS NOT NULL DROP TABLE dbo.belief_domain;
GO

CREATE TABLE dbo.belief_domain (
    domain_id        BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    domain_code      NVARCHAR(128) NOT NULL UNIQUE,
    description      NVARCHAR(512) NULL,
    half_life_secs   FLOAT         NOT NULL DEFAULT 0.0,
    created_ms       BIGINT        NOT NULL,
    last_updated_ms  BIGINT        NOT NULL
);
GO

CREATE TABLE dbo.belief_prior (
    domain_id        BIGINT       NOT NULL,
    hypothesis_id    BIGINT       NOT NULL,
    mass             FLOAT        NOT NULL,
    CONSTRAINT PK_belief_prior PRIMARY KEY (domain_id, hypothesis_id),
    CONSTRAINT FK_belief_prior_dom FOREIGN KEY (domain_id)
        REFERENCES dbo.belief_domain(domain_id) ON DELETE CASCADE,
    CONSTRAINT CK_belief_prior_mass CHECK (mass >= 0.0 AND mass <= 1.0)
);
GO

CREATE TABLE dbo.belief_posterior (
    domain_id        BIGINT       NOT NULL,
    hypothesis_id    BIGINT       NOT NULL,
    mass             FLOAT        NOT NULL,
    CONSTRAINT PK_belief_posterior PRIMARY KEY (domain_id, hypothesis_id),
    CONSTRAINT FK_belief_posterior_dom FOREIGN KEY (domain_id)
        REFERENCES dbo.belief_domain(domain_id) ON DELETE CASCADE,
    CONSTRAINT CK_belief_posterior_mass CHECK (mass >= 0.0 AND mass <= 1.0)
);
GO

CREATE TABLE dbo.belief_evidence (
    evidence_id      BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    domain_id        BIGINT       NOT NULL,
    kind             TINYINT      NOT NULL,
    hypothesis_id    BIGINT       NOT NULL,
    likelihood_ratio FLOAT        NOT NULL,
    source_weight    FLOAT        NOT NULL,
    observed_ms      BIGINT       NOT NULL,
    reason           NVARCHAR(256) NULL,
    CONSTRAINT FK_belief_evidence_dom FOREIGN KEY (domain_id)
        REFERENCES dbo.belief_domain(domain_id) ON DELETE CASCADE,
    CONSTRAINT CK_belief_evidence_lr CHECK (likelihood_ratio > 0.0)
);
GO

CREATE INDEX IX_belief_evidence_domain_time
    ON dbo.belief_evidence(domain_id, observed_ms DESC);
GO

CREATE TABLE dbo.belief_audit (
    audit_id          BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
    domain_id         BIGINT       NOT NULL,
    operation         NVARCHAR(32) NOT NULL,
    evidence_count    INT          NOT NULL,
    entropy_before    FLOAT        NULL,
    entropy_after     FLOAT        NULL,
    map_hypothesis_id BIGINT       NULL,
    map_probability   FLOAT        NULL,
    recorded_ms       BIGINT       NOT NULL,
    detail            NVARCHAR(1024) NULL,
    CONSTRAINT FK_belief_audit_dom FOREIGN KEY (domain_id)
        REFERENCES dbo.belief_domain(domain_id) ON DELETE CASCADE
);
GO

CREATE INDEX IX_belief_audit_domain_time
    ON dbo.belief_audit(domain_id, recorded_ms DESC);
GO

IF OBJECT_ID('dbo.vw_BeliefSnapshot','V') IS NOT NULL DROP VIEW dbo.vw_BeliefSnapshot;
GO

CREATE VIEW dbo.vw_BeliefSnapshot AS
SELECT
    d.domain_id,
    d.domain_code,
    d.half_life_secs,
    d.last_updated_ms,
    bp.hypothesis_id,
    bp.mass               AS posterior_mass,
    bpr.mass              AS prior_mass
FROM dbo.belief_domain     d
JOIN dbo.belief_posterior  bp  ON bp.domain_id  = d.domain_id
LEFT JOIN dbo.belief_prior bpr ON bpr.domain_id = d.domain_id
                              AND bpr.hypothesis_id = bp.hypothesis_id;
GO

IF OBJECT_ID('dbo.usp_BeliefUpsertDomain','P') IS NOT NULL DROP PROCEDURE dbo.usp_BeliefUpsertDomain;
GO

CREATE PROCEDURE dbo.usp_BeliefUpsertDomain
    @DomainCode    NVARCHAR(128),
    @Description   NVARCHAR(512) = NULL,
    @HalfLifeSecs  FLOAT         = 0.0,
    @DomainID      BIGINT OUTPUT
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);

    MERGE dbo.belief_domain AS T
    USING (SELECT @DomainCode AS code) AS S
       ON T.domain_code = S.code
    WHEN MATCHED THEN
        UPDATE SET T.description    = ISNULL(@Description, T.description),
                   T.half_life_secs = @HalfLifeSecs,
                   T.last_updated_ms = @now
    WHEN NOT MATCHED THEN
        INSERT (domain_code, description, half_life_secs, created_ms, last_updated_ms)
        VALUES (@DomainCode, @Description, @HalfLifeSecs, @now, @now);

    SELECT @DomainID = domain_id FROM dbo.belief_domain WHERE domain_code = @DomainCode;
END;
GO

IF OBJECT_ID('dbo.usp_BeliefReplacePosterior','P') IS NOT NULL DROP PROCEDURE dbo.usp_BeliefReplacePosterior;
GO

CREATE TYPE dbo.HypothesisMass AS TABLE (
    hypothesis_id BIGINT NOT NULL,
    mass          FLOAT  NOT NULL
);
GO

CREATE PROCEDURE dbo.usp_BeliefReplacePosterior
    @DomainID BIGINT,
    @Rows     dbo.HypothesisMass READONLY
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);
    BEGIN TRAN;
        DELETE FROM dbo.belief_posterior WHERE domain_id = @DomainID;
        INSERT INTO dbo.belief_posterior (domain_id, hypothesis_id, mass)
            SELECT @DomainID, hypothesis_id, mass FROM @Rows;
        UPDATE dbo.belief_domain SET last_updated_ms = @now WHERE domain_id = @DomainID;
    COMMIT;
END;
GO

IF OBJECT_ID('dbo.usp_BeliefAppendEvidence','P') IS NOT NULL DROP PROCEDURE dbo.usp_BeliefAppendEvidence;
GO

CREATE PROCEDURE dbo.usp_BeliefAppendEvidence
    @DomainID         BIGINT,
    @Kind             TINYINT,
    @HypothesisID     BIGINT,
    @LikelihoodRatio  FLOAT,
    @SourceWeight     FLOAT,
    @Reason           NVARCHAR(256) = NULL
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);
    INSERT INTO dbo.belief_evidence
        (domain_id, kind, hypothesis_id, likelihood_ratio, source_weight, observed_ms, reason)
    VALUES (@DomainID, @Kind, @HypothesisID, @LikelihoodRatio, @SourceWeight, @now, @Reason);
END;
GO

IF OBJECT_ID('dbo.usp_BeliefAudit','P') IS NOT NULL DROP PROCEDURE dbo.usp_BeliefAudit;
GO

CREATE PROCEDURE dbo.usp_BeliefAudit
    @DomainID         BIGINT,
    @Operation        NVARCHAR(32),
    @EvidenceCount    INT,
    @EntropyBefore    FLOAT = NULL,
    @EntropyAfter     FLOAT = NULL,
    @MapHypothesisID  BIGINT = NULL,
    @MapProbability   FLOAT = NULL,
    @Detail           NVARCHAR(1024) = NULL
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);
    INSERT INTO dbo.belief_audit
        (domain_id, operation, evidence_count, entropy_before, entropy_after,
         map_hypothesis_id, map_probability, recorded_ms, detail)
    VALUES (@DomainID, @Operation, @EvidenceCount, @EntropyBefore, @EntropyAfter,
            @MapHypothesisID, @MapProbability, @now, @Detail);
END;
GO
