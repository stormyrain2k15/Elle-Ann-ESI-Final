#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${ELLE_SQL_DSN:-}" ]]; then
    echo "ELLE_SQL_DSN must point at the MS SQL Server hosting ElleCore."
    exit 2
fi

if ! command -v sqlcmd >/dev/null 2>&1; then
    echo "sqlcmd not on PATH. Install MSSQL command-line tools."
    exit 2
fi

run_sql() {
    sqlcmd -S "${ELLE_SQL_DSN}" -d ElleCore -b -h -1 -Q "$1"
}

run_sql_scalar() {
    sqlcmd -S "${ELLE_SQL_DSN}" -d ElleCore -b -h -1 -W -Q "$1" | head -1 | tr -d ' \r\n'
}

assert_eq() {
    local label="$1" expected="$2" actual="$3"
    if [[ "${expected}" != "${actual}" ]]; then
        echo "[queue-lifecycle] FAIL ${label}: expected=${expected} actual=${actual}"
        exit 1
    fi
    echo "[queue-lifecycle] PASS ${label} (${actual})"
}

CANARY_DESC="queue-lifecycle-canary-$(date +%s)-$$"

cleanup() {
    run_sql "DELETE FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%';" || true
}
trap cleanup EXIT

echo "[queue-lifecycle] === Submit phase ==="
run_sql "
INSERT INTO dbo.IntentQueue
    (IntentType, Status, SourceDrive, Urgency, Confidence, Description,
     Parameters, RequiredTrust, CreatedMs, TimeoutMs)
VALUES
    (1, 0, 1, 0.7, 0.8, N'${CANARY_DESC}-1',
     N'{}', 0, CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT),
     30000);
INSERT INTO dbo.IntentQueue
    (IntentType, Status, SourceDrive, Urgency, Confidence, Description,
     Parameters, RequiredTrust, CreatedMs, TimeoutMs)
VALUES
    (1, 0, 1, 0.5, 0.6, N'${CANARY_DESC}-2',
     N'{}', 0, CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT),
     30000);
"

pending_count=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND Status = 0;")
assert_eq "Submit creates PENDING rows" "2" "${pending_count}"

echo "[queue-lifecycle] === Lock (PENDING -> PROCESSING) phase ==="
run_sql "
UPDATE TOP (2) q WITH (ROWLOCK, READPAST)
    SET Status = 1,
        ProcessingMs = CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT)
FROM dbo.IntentQueue AS q
WHERE q.Status = 0
  AND q.Description LIKE N'${CANARY_DESC}%';
"

processing_count=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND Status = 1;")
assert_eq "Lock transitions to PROCESSING" "2" "${processing_count}"

processing_ms_set=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND ProcessingMs IS NOT NULL;")
assert_eq "Lock stamps ProcessingMs" "2" "${processing_ms_set}"

echo "[queue-lifecycle] === Complete (PROCESSING -> COMPLETED) phase ==="
run_sql "
DECLARE @id1 BIGINT, @id2 BIGINT;
SELECT TOP 1 @id1 = IntentId FROM dbo.IntentQueue
    WHERE Description = N'${CANARY_DESC}-1';
SELECT TOP 1 @id2 = IntentId FROM dbo.IntentQueue
    WHERE Description = N'${CANARY_DESC}-2';
EXEC dbo.sp_SubmitIntentResponse @IntentId = @id1, @Status = 2, @Response = N'ok-1';
EXEC dbo.sp_SubmitIntentResponse @IntentId = @id2, @Status = 2, @Response = N'ok-2';
"

completed_count=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND Status = 2;")
assert_eq "Complete transitions to COMPLETED" "2" "${completed_count}"

completed_ms_set=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND CompletedMs IS NOT NULL;")
assert_eq "Complete stamps CompletedMs" "2" "${completed_ms_set}"

response_count=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description LIKE N'${CANARY_DESC}%' AND Response IS NOT NULL;")
assert_eq "Complete writes Response" "2" "${response_count}"

echo "[queue-lifecycle] === Stale-reap phase (timeout sweep) ==="
STALE_DESC="${CANARY_DESC}-stale"
run_sql "
INSERT INTO dbo.IntentQueue
    (IntentType, Status, SourceDrive, Urgency, Confidence, Description,
     Parameters, RequiredTrust, CreatedMs, TimeoutMs, ProcessingMs)
VALUES
    (1, 1, 1, 0.9, 0.9, N'${STALE_DESC}',
     N'{}', 0,
     CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT) - 600000,
     1000,
     CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT) - 600000);
"

run_sql "
UPDATE dbo.IntentQueue WITH (ROWLOCK)
    SET Status = 3,
        CompletedMs = CAST(DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) AS BIGINT),
        Response = N'timeout_max_retries'
FROM dbo.IntentQueue
WHERE Description = N'${STALE_DESC}'
  AND Status = 1
  AND DATEDIFF_BIG(MILLISECOND,'1970-01-01',SYSUTCDATETIME()) - ProcessingMs > TimeoutMs;
"

stale_count=$(run_sql_scalar "SELECT COUNT(*) FROM dbo.IntentQueue WHERE Description = N'${STALE_DESC}' AND Status = 3;")
assert_eq "Reap stamps stale row TIMED-OUT" "1" "${stale_count}"

echo "[queue-lifecycle] All four phases passed (Submit → Lock → Execute → Complete + Reap)."
