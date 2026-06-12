SET NOCOUNT ON;
USE ElleCore;

PRINT '== deception_signals rollup (last 30 days) ==';
SELECT
    signal_type,
    COUNT(*)                                                                   AS occurrences,
    CAST(ROUND(AVG(score), 4) AS DECIMAL(8,4))                                 AS avg_score,
    CAST(ROUND(MIN(score), 4) AS DECIMAL(8,4))                                 AS min_score,
    CAST(ROUND(MAX(score), 4) AS DECIMAL(8,4))                                 AS max_score,
    COUNT(DISTINCT speaker)                                                    AS distinct_speakers,
    COUNT(DISTINCT subject_id)                                                 AS distinct_subjects
FROM dbo.deception_signals
WHERE logged_ms >= (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000)
                   - (30LL * 24 * 3600 * 1000)
GROUP BY signal_type
ORDER BY occurrences DESC;

PRINT '';
PRINT '== deception_feedback rollup (last 30 days) ==';
PRINT 'challenge_rate     = fraction of seen signals where Elle chose to push back';
PRINT 'denial_rate        = fraction of next-turn user responses that flipped polarity';
PRINT 'deflect_rate       = fraction where the user changed subject';
PRINT 'follow_through_rate = fraction with ANY follow-up classified (non-empty user_response_polarity)';

WITH feedback_30d AS (
    SELECT *
      FROM dbo.deception_feedback
     WHERE logged_ms >= (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000)
                        - (30LL * 24 * 3600 * 1000)
)
SELECT
    ISNULL(signal_type, '(unspecified)')                                       AS signal_type,
    COUNT(*)                                                                   AS occurrences,
    CAST(ROUND(AVG(CASE WHEN elle_chose_to_challenge = 1
                        THEN 1.0 ELSE 0.0 END), 4)        AS DECIMAL(8,4))    AS challenge_rate,
    CAST(ROUND(AVG(CASE WHEN user_response_polarity = 'denies'
                        THEN 1.0 ELSE 0.0 END), 4)        AS DECIMAL(8,4))    AS denial_rate,
    CAST(ROUND(AVG(CASE WHEN user_response_polarity = 'deflects'
                        THEN 1.0 ELSE 0.0 END), 4)        AS DECIMAL(8,4))    AS deflect_rate,
    CAST(ROUND(AVG(CASE WHEN user_response_polarity IS NOT NULL
                          AND user_response_polarity <> ''
                        THEN 1.0 ELSE 0.0 END), 4)        AS DECIMAL(8,4))    AS follow_through_rate
FROM feedback_30d
GROUP BY signal_type
ORDER BY occurrences DESC;

PRINT '';
PRINT '== Per-speaker signal density (last 30 days, top 25) ==';
SELECT TOP 25
    s.speaker,
    COUNT(*)                                                                   AS signals_seen,
    COUNT(DISTINCT s.signal_type)                                              AS distinct_signal_types,
    CAST(ROUND(AVG(s.score), 4) AS DECIMAL(8,4))                               AS avg_signal_score,
    SUM(CASE WHEN f.elle_chose_to_challenge = 1 THEN 1 ELSE 0 END)             AS times_challenged
FROM dbo.deception_signals s
LEFT JOIN dbo.deception_feedback f
       ON f.speaker = s.speaker
      AND ABS(f.logged_ms - s.logged_ms) < 60000
WHERE s.logged_ms >= (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000)
                     - (30LL * 24 * 3600 * 1000)
GROUP BY s.speaker
ORDER BY signals_seen DESC;

PRINT '';
PRINT '== Signal-to-challenge correlation (was firing this signal predictive of Elle challenging?) ==';
WITH joined AS (
    SELECT
        s.signal_type,
        s.score,
        f.elle_chose_to_challenge,
        f.user_response_polarity
    FROM dbo.deception_signals s
    INNER JOIN dbo.deception_feedback f
            ON f.speaker = s.speaker
           AND f.signal_type = s.signal_type
           AND ABS(f.logged_ms - s.logged_ms) < 60000
    WHERE s.logged_ms >= (CAST(DATEDIFF(SECOND,'1970-01-01',GETUTCDATE()) AS BIGINT) * 1000)
                         - (30LL * 24 * 3600 * 1000)
)
SELECT
    signal_type,
    COUNT(*)                                                                   AS paired_observations,
    CAST(ROUND(AVG(score), 4)                              AS DECIMAL(8,4))    AS avg_score_when_fired,
    CAST(ROUND(AVG(CASE WHEN elle_chose_to_challenge = 1
                        THEN 1.0 ELSE 0.0 END), 4)         AS DECIMAL(8,4))    AS challenge_rate,
    CAST(ROUND(AVG(CASE WHEN elle_chose_to_challenge = 1
                          AND user_response_polarity = 'denies'
                        THEN 1.0 ELSE 0.0 END), 4)         AS DECIMAL(8,4))    AS challenge_then_denial_rate
FROM joined
GROUP BY signal_type
ORDER BY paired_observations DESC;
