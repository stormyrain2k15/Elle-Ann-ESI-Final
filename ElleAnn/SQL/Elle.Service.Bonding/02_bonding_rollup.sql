USE ElleHeart;
GO
SET NOCOUNT ON;
GO

IF OBJECT_ID('dbo.relationship_history','U') IS NOT NULL DROP TABLE dbo.relationship_history;
GO

CREATE TABLE dbo.relationship_history (
    history_id        BIGINT IDENTITY(1,1) PRIMARY KEY,
    snapshot_ms       BIGINT NOT NULL,
    intimacy          FLOAT  NOT NULL,
    passion           FLOAT  NOT NULL,
    commitment        FLOAT  NOT NULL,
    security          FLOAT  NOT NULL,
    anxiety           FLOAT  NOT NULL,
    avoidance         FLOAT  NOT NULL,
    felt_understood   FLOAT  NOT NULL,
    felt_cared_for    FLOAT  NOT NULL,
    investment        FLOAT  NOT NULL,
    total_interactions          INT NOT NULL,
    meaningful_conversations    INT NOT NULL,
    times_person_asked_about_her INT NOT NULL,
    conflicts_experienced       INT NOT NULL,
    unresolved_tension          BIT NOT NULL,
    repair_motivation           FLOAT NOT NULL,
    conflicts_resolved          INT NOT NULL,
    snapshot_reason   NVARCHAR(64) NOT NULL
);
GO

CREATE INDEX IX_relationship_history_time
    ON dbo.relationship_history(snapshot_ms DESC);
GO

IF OBJECT_ID('dbo.vw_RelationshipDashboard','V') IS NOT NULL DROP VIEW dbo.vw_RelationshipDashboard;
GO

CREATE VIEW dbo.vw_RelationshipDashboard AS
SELECT
    r.id,
    r.updated_ms,
    r.intimacy, r.passion, r.commitment,
    r.security, r.anxiety, r.avoidance,
    r.felt_understood, r.felt_cared_for, r.investment,
    r.total_interactions, r.meaningful_conversations,
    r.times_person_asked_about_her, r.conflicts_experienced,
    r.unresolved_tension, r.repair_motivation, r.conflicts_resolved,
    r.repair_uttered, r.repair_attempt_ms, r.repair_stable_since_ms,
    CAST((r.intimacy + r.felt_understood + r.felt_cared_for) / 3.0 AS FLOAT)
        AS affection_index,
    CAST((r.commitment + r.investment + r.security) / 3.0 AS FLOAT)
        AS commitment_index,
    CAST((r.anxiety + r.avoidance) / 2.0 AS FLOAT)
        AS distress_index,
    CASE WHEN r.total_interactions = 0 THEN 0.0
         ELSE CAST(r.meaningful_conversations AS FLOAT) /
              CAST(r.total_interactions AS FLOAT)
    END                                                         AS meaningful_ratio,
    CASE WHEN r.conflicts_experienced = 0 THEN 1.0
         ELSE CAST(r.conflicts_resolved AS FLOAT) /
              CAST(r.conflicts_experienced AS FLOAT)
    END                                                         AS conflict_resolution_ratio
FROM dbo.relationship_state AS r;
GO

IF OBJECT_ID('dbo.usp_BondingSnapshot','P') IS NOT NULL DROP PROCEDURE dbo.usp_BondingSnapshot;
GO

CREATE PROCEDURE dbo.usp_BondingSnapshot
    @Reason NVARCHAR(64) = N'periodic'
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @now BIGINT = CAST(DATEDIFF_BIG(MILLISECOND, '1970-01-01', SYSUTCDATETIME()) AS BIGINT);

    INSERT INTO dbo.relationship_history (
        snapshot_ms, intimacy, passion, commitment, security, anxiety,
        avoidance, felt_understood, felt_cared_for, investment,
        total_interactions, meaningful_conversations, times_person_asked_about_her,
        conflicts_experienced, unresolved_tension, repair_motivation,
        conflicts_resolved, snapshot_reason
    )
    SELECT
        @now, r.intimacy, r.passion, r.commitment, r.security, r.anxiety,
        r.avoidance, r.felt_understood, r.felt_cared_for, r.investment,
        r.total_interactions, r.meaningful_conversations, r.times_person_asked_about_her,
        r.conflicts_experienced, r.unresolved_tension, r.repair_motivation,
        r.conflicts_resolved, @Reason
    FROM dbo.relationship_state AS r WHERE r.id = 1;
END;
GO

IF OBJECT_ID('dbo.vw_RelationshipTrajectory','V') IS NOT NULL DROP VIEW dbo.vw_RelationshipTrajectory;
GO

CREATE VIEW dbo.vw_RelationshipTrajectory AS
SELECT TOP (1000)
    history_id, snapshot_ms, snapshot_reason,
    intimacy, passion, commitment, security,
    anxiety, avoidance,
    felt_understood, felt_cared_for, investment,
    total_interactions, meaningful_conversations,
    conflicts_experienced, conflicts_resolved,
    repair_motivation, unresolved_tension,
    CAST((intimacy + felt_understood + felt_cared_for) / 3.0 AS FLOAT) AS affection_index,
    CAST((anxiety + avoidance) / 2.0 AS FLOAT)                          AS distress_index
FROM dbo.relationship_history
ORDER BY snapshot_ms DESC;
GO
