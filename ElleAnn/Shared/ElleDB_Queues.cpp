#include "ElleSQLConn.h"
#include "ElleLogger.h"
#include "ElleConfig.h"
#include "ElleTypes.h"

#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cctype>
#include <mutex>
#include <set>
#include <random>

namespace ElleDB {

bool SubmitIntent(const ELLE_INTENT_RECORD& intent) {

    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.IntentQueue "
        "(IntentType, Status, SourceDrive, Urgency, Confidence, Description, Parameters, "
        "RequiredTrust, CreatedMs, TimeoutMs) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        {
            std::to_string(intent.type),
            std::to_string(INTENT_PENDING),
            std::to_string(intent.source_drive),
            std::to_string(intent.urgency),
            std::to_string(intent.confidence),
            std::string(intent.description),
            std::string(intent.parameters),
            std::to_string(intent.required_trust),
            std::to_string(ELLE_MS_NOW()),
            std::to_string(intent.timeout_ms)
        }).success;
}

bool GetPendingIntents(std::vector<ELLE_INTENT_RECORD>& out, uint32_t maxCount) {
    out.clear();

    ElleSQLPool::Instance().Exec(
        "IF NOT EXISTS (SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID(N'ElleCore.dbo.IntentQueue') "
        "    AND name = N'ProcessingMs') "
        "ALTER TABLE ElleCore.dbo.IntentQueue ADD ProcessingMs BIGINT NULL;");
    ElleSQLPool::Instance().Exec(
        "IF NOT EXISTS (SELECT 1 FROM sys.indexes "
        "  WHERE name = N'IX_IntentQueue_Processing' "
        "    AND object_id = OBJECT_ID(N'ElleCore.dbo.IntentQueue')) "
        "CREATE INDEX IX_IntentQueue_Processing "
        "  ON ElleCore.dbo.IntentQueue (Status, ProcessingMs);");

    auto rs = ElleSQLPool::Instance().QueryParams(
        "UPDATE TOP (" + std::to_string(maxCount) + ") q WITH (ROWLOCK, READPAST) "
        "SET Status = ?, ProcessingMs = ? "
        "OUTPUT inserted.IntentId, inserted.IntentType, inserted.Status, "
        "       inserted.SourceDrive, inserted.Urgency, inserted.Confidence, "
        "       inserted.Description, inserted.Parameters, "
        "       inserted.RequiredTrust, inserted.CreatedMs, inserted.TimeoutMs "
        "FROM ElleCore.dbo.IntentQueue AS q "
        "WHERE q.Status = ?;",
        { std::to_string((int)INTENT_PROCESSING),
          std::to_string((long long)ELLE_MS_NOW()),
          std::to_string((int)INTENT_PENDING) });
    if (!rs.success) return false;
    for (auto& row : rs.rows) {
        ELLE_INTENT_RECORD rec = {};
        rec.id = (uint64_t)row.GetIntOr(0, 0);
        rec.type = (uint32_t)row.GetIntOr(1, 0);
        rec.status = (uint32_t)row.GetIntOr(2, 0);
        rec.source_drive = (uint32_t)row.GetIntOr(3, 0);
        rec.urgency = (float)row.GetFloatOr(4, 0.0);
        rec.confidence = (float)row.GetFloatOr(5, 0.0);
        strncpy_s(rec.description,
                  row.values.size() > 6 ? row.values[6].c_str() : "", ELLE_MAX_MSG - 1);
        strncpy_s(rec.parameters,
                  row.values.size() > 7 ? row.values[7].c_str() : "", ELLE_MAX_MSG - 1);
        rec.required_trust = (uint32_t)row.GetIntOr(8, 0);
        rec.created_ms = (uint64_t)row.GetIntOr(9, 0);
        rec.timeout_ms = (uint64_t)row.GetIntOr(10, 0);
        out.push_back(rec);
    }
    return true;
}

bool UpdateIntentStatus(uint64_t intentId, ELLE_INTENT_STATUS status, const std::string& response) {
    return ElleSQLPool::Instance().CallProc("ElleCore.dbo.sp_SubmitIntentResponse",
        {std::to_string(intentId), std::to_string(status), response}).success;
}

static void EnsureActionQueueTable();

uint32_t ReapStaleIntents(uint32_t defaultTimeoutMs, uint32_t maxRetries) {
    int64_t now = (int64_t)ELLE_MS_NOW();
    auto& pool = ElleSQLPool::Instance();

    auto rs1 = pool.QueryParams(
        "UPDATE ElleCore.dbo.IntentQueue WITH (ROWLOCK) "
        "SET Status = ?, CompletedMs = ?, Response = N'timeout_max_retries' "
        "WHERE Status = ? "
        "  AND RetryCount >= ? "
        "  AND ? - ISNULL(ProcessingMs, CreatedMs) > "
        "      CASE WHEN TimeoutMs > 0 THEN TimeoutMs ELSE ? END;",
        {
            std::to_string((int)INTENT_FAILED),
            std::to_string(now),
            std::to_string((int)INTENT_PROCESSING),
            std::to_string(maxRetries),
            std::to_string(now),
            std::to_string(defaultTimeoutMs)
        });
    uint32_t failed = rs1.success ? (uint32_t)rs1.rows_affected : 0;

    auto rs2 = pool.QueryParams(
        "UPDATE ElleCore.dbo.IntentQueue WITH (ROWLOCK) "
        "SET Status = ?, RetryCount = RetryCount + 1, ProcessingMs = NULL "
        "WHERE Status = ? "
        "  AND RetryCount < ? "
        "  AND ? - ISNULL(ProcessingMs, CreatedMs) > "
        "      CASE WHEN TimeoutMs > 0 THEN TimeoutMs ELSE ? END;",
        {
            std::to_string((int)INTENT_PENDING),
            std::to_string((int)INTENT_PROCESSING),
            std::to_string(maxRetries),
            std::to_string(now),
            std::to_string(defaultTimeoutMs)
        });
    uint32_t requeued = rs2.success ? (uint32_t)rs2.rows_affected : 0;

    return failed + requeued;
}

uint32_t ReapStaleActions(uint32_t defaultTimeoutMs, uint32_t maxAttempts) {
    int64_t now = (int64_t)ELLE_MS_NOW();
    auto& pool = ElleSQLPool::Instance();
    EnsureActionQueueTable();

    pool.Exec(
        "IF NOT EXISTS ("
        "  SELECT 1 FROM sys.columns "
        "  WHERE object_id = OBJECT_ID(N'ElleCore.dbo.action_queue') "
        "    AND name = N'attempts') "
        "ALTER TABLE ElleCore.dbo.action_queue "
        "  ADD attempts INT NOT NULL DEFAULT 0;");

    auto rs1 = pool.QueryParams(
        "UPDATE ElleCore.dbo.action_queue WITH (ROWLOCK) "
        "SET status = ?, completed_ms = ?, "
        "    result = N'timeout_max_attempts' "
        "WHERE status IN (?, ?) "
        "  AND attempts >= ? "
        "  AND ISNULL(started_ms, created_ms) IS NOT NULL "
        "  AND ? - ISNULL(started_ms, created_ms) > "
        "      CASE WHEN timeout_ms > 0 THEN timeout_ms ELSE ? END;",
        {
            std::to_string((int)ACTION_TIMEOUT),
            std::to_string(now),
            std::to_string((int)ACTION_LOCKED),
            std::to_string((int)ACTION_EXECUTING),
            std::to_string(maxAttempts),
            std::to_string(now),
            std::to_string(defaultTimeoutMs)
        });
    uint32_t failed = rs1.success ? (uint32_t)rs1.rows_affected : 0;

    auto rs2 = pool.QueryParams(
        "UPDATE ElleCore.dbo.action_queue WITH (ROWLOCK) "
        "SET status = ?, attempts = attempts + 1, started_ms = NULL "
        "WHERE status IN (?, ?) "
        "  AND attempts < ? "
        "  AND ISNULL(started_ms, created_ms) IS NOT NULL "
        "  AND ? - ISNULL(started_ms, created_ms) > "
        "      CASE WHEN timeout_ms > 0 THEN timeout_ms ELSE ? END;",
        {
            std::to_string((int)ACTION_QUEUED),
            std::to_string((int)ACTION_LOCKED),
            std::to_string((int)ACTION_EXECUTING),
            std::to_string(maxAttempts),
            std::to_string(now),
            std::to_string(defaultTimeoutMs)
        });
    uint32_t requeued = rs2.success ? (uint32_t)rs2.rows_affected : 0;

    return failed + requeued;
}

bool GetQueueSnapshot(QueueSnapshot& out) {
    out = {};
    int64_t now      = (int64_t)ELLE_MS_NOW();
    int64_t hourAgo  = now - 3600000LL;
    auto& pool = ElleSQLPool::Instance();

    auto ri = pool.QueryParams(
        "SELECT "
        "  SUM(CASE WHEN Status = ? THEN 1 ELSE 0 END) AS pending, "
        "  SUM(CASE WHEN Status = ? THEN 1 ELSE 0 END) AS processing, "
        "  SUM(CASE WHEN Status = ? AND CompletedMs >= ? THEN 1 ELSE 0 END) AS done_1h, "
        "  SUM(CASE WHEN Status = ? AND CompletedMs >= ? THEN 1 ELSE 0 END) AS fail_1h, "
        "  SUM(CASE WHEN Status = ? AND ? - ISNULL(ProcessingMs, CreatedMs) > "
        "           CASE WHEN TimeoutMs > 0 THEN TimeoutMs ELSE 120000 END "
        "        THEN 1 ELSE 0 END) AS stale "
        "FROM ElleCore.dbo.IntentQueue;",
        {
            std::to_string((int)INTENT_PENDING),
            std::to_string((int)INTENT_PROCESSING),
            std::to_string((int)INTENT_COMPLETED), std::to_string(hourAgo),
            std::to_string((int)INTENT_FAILED),    std::to_string(hourAgo),
            std::to_string((int)INTENT_PROCESSING), std::to_string(now)
        });
    if (ri.success && !ri.rows.empty()) {
        auto& r = ri.rows[0];
        out.intent_pending          = (uint32_t)r.GetIntOr(0, 0);
        out.intent_processing       = (uint32_t)r.GetIntOr(1, 0);
        out.intent_completed_1h     = (uint32_t)r.GetIntOr(2, 0);
        out.intent_failed_1h        = (uint32_t)r.GetIntOr(3, 0);
        out.intent_stale_processing = (uint32_t)r.GetIntOr(4, 0);
    }

    EnsureActionQueueTable();
    auto ra = pool.QueryParams(
        "SELECT "
        "  SUM(CASE WHEN status = ? THEN 1 ELSE 0 END) AS queued, "
        "  SUM(CASE WHEN status = ? THEN 1 ELSE 0 END) AS locked, "
        "  SUM(CASE WHEN status = ? THEN 1 ELSE 0 END) AS executing, "
        "  SUM(CASE WHEN status = ? AND completed_ms >= ? THEN 1 ELSE 0 END) AS ok_1h, "
        "  SUM(CASE WHEN status = ? AND completed_ms >= ? THEN 1 ELSE 0 END) AS fail_1h, "
        "  SUM(CASE WHEN status = ? AND completed_ms >= ? THEN 1 ELSE 0 END) AS to_1h, "
        "  SUM(CASE WHEN status IN (?, ?) "
        "            AND ? - ISNULL(started_ms, created_ms) > "
        "                CASE WHEN timeout_ms > 0 THEN timeout_ms ELSE 60000 END "
        "        THEN 1 ELSE 0 END) AS stale "
        "FROM ElleCore.dbo.action_queue;",
        {
            std::to_string((int)ACTION_QUEUED),
            std::to_string((int)ACTION_LOCKED),
            std::to_string((int)ACTION_EXECUTING),
            std::to_string((int)ACTION_COMPLETED_SUCCESS), std::to_string(hourAgo),
            std::to_string((int)ACTION_COMPLETED_FAILURE), std::to_string(hourAgo),
            std::to_string((int)ACTION_TIMEOUT),           std::to_string(hourAgo),
            std::to_string((int)ACTION_LOCKED),
            std::to_string((int)ACTION_EXECUTING),
            std::to_string(now)
        });
    if (ra.success && !ra.rows.empty()) {
        auto& r = ra.rows[0];
        out.action_queued       = (uint32_t)r.GetIntOr(0, 0);
        out.action_locked       = (uint32_t)r.GetIntOr(1, 0);
        out.action_executing    = (uint32_t)r.GetIntOr(2, 0);
        out.action_success_1h   = (uint32_t)r.GetIntOr(3, 0);
        out.action_failure_1h   = (uint32_t)r.GetIntOr(4, 0);
        out.action_timeout_1h   = (uint32_t)r.GetIntOr(5, 0);
        out.action_stale_locked = (uint32_t)r.GetIntOr(6, 0);
    }

    auto rh = pool.Query(
        "IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'hardware_actions') "
        "  SELECT "
        "    SUM(CASE WHEN status = 'pending'    THEN 1 ELSE 0 END), "
        "    SUM(CASE WHEN status = 'dispatched' THEN 1 ELSE 0 END) "
        "  FROM ElleCore.dbo.hardware_actions;");
    if (rh.success && !rh.rows.empty()) {
        out.hardware_pending    = (uint32_t)rh.rows[0].GetIntOr(0, 0);
        out.hardware_dispatched = (uint32_t)rh.rows[0].GetIntOr(1, 0);
    }

    return true;
}

static void EnsureActionQueueTable() {
    ElleSQLPool::Instance().Exec(
        "IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'action_queue') "
        "CREATE TABLE ElleCore.dbo.action_queue ("
        "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
        "  intent_id      BIGINT NULL,"
        "  action_type    INT NOT NULL,"
        "  status         INT NOT NULL DEFAULT 0,"
        "  command        NVARCHAR(MAX) NULL,"
        "  parameters     NVARCHAR(MAX) NULL,"
        "  result         NVARCHAR(MAX) NULL,"
        "  required_trust INT NOT NULL DEFAULT 0,"
        "  trust_delta    INT NOT NULL DEFAULT 0,"
        "  created_ms     BIGINT NOT NULL,"
        "  started_ms     BIGINT NULL,"
        "  completed_ms   BIGINT NULL,"
        "  timeout_ms     BIGINT NOT NULL DEFAULT 30000,"
        "  error_code     INT NOT NULL DEFAULT 0"
        ");");
}

bool SubmitAction(const ELLE_ACTION_RECORD& action) {
    EnsureActionQueueTable();
    return ElleSQLPool::Instance().QueryParams(
        "INSERT INTO ElleCore.dbo.action_queue "
        "(intent_id, action_type, status, command, parameters, required_trust, "
        " created_ms, timeout_ms) VALUES (NULLIF(?, '0'), ?, ?, ?, ?, ?, ?, ?);",
        {
            std::to_string((int64_t)action.intent_id),
            std::to_string(action.type),
            std::to_string(ACTION_QUEUED),
            std::string(action.command),
            std::string(action.parameters),
            std::to_string(action.required_trust),
            std::to_string((int64_t)ELLE_MS_NOW()),
            std::to_string((int64_t)action.timeout_ms)
        }).success;
}

bool GetPendingActions(std::vector<ELLE_ACTION_RECORD>& out, uint32_t maxCount) {
    out.clear();
    EnsureActionQueueTable();

    auto rs = ElleSQLPool::Instance().QueryParams(
        "UPDATE TOP (" + std::to_string(maxCount) + ") q WITH (ROWLOCK, READPAST) "
        "SET status = ?, started_ms = ? "
        "OUTPUT inserted.id, ISNULL(inserted.intent_id,0), inserted.action_type, "
        "       inserted.status, ISNULL(inserted.command, N''), "
        "       ISNULL(inserted.parameters, N''), inserted.required_trust, "
        "       inserted.created_ms, inserted.timeout_ms "
        "FROM ElleCore.dbo.action_queue AS q "
        "WHERE q.status = ?;",
        {
            std::to_string((int)ACTION_LOCKED),
            std::to_string((int64_t)ELLE_MS_NOW()),
            std::to_string((int)ACTION_QUEUED)
        });
    if (!rs.success) return false;
    for (auto& row : rs.rows) {
        ELLE_ACTION_RECORD a{};
        a.id             = (uint64_t)row.GetIntOr(0, 0);
        a.intent_id      = (uint64_t)row.GetIntOr(1, 0);
        a.type           = (uint32_t)row.GetIntOr(2, 0);
        a.status         = (uint32_t)row.GetIntOr(3, 0);
        std::string cmd  = row.values.size() > 4 ? row.values[4] : std::string();
        std::string prm  = row.values.size() > 5 ? row.values[5] : std::string();
        strncpy_s(a.command,    cmd.c_str(), ELLE_MAX_MSG - 1);
        strncpy_s(a.parameters, prm.c_str(), ELLE_MAX_MSG - 1);
        a.required_trust = (uint32_t)row.GetIntOr(6, 0);
        a.created_ms     = (uint64_t)row.GetIntOr(7, 0);
        a.timeout_ms     = (uint64_t)row.GetIntOr(8, 0);
        out.push_back(a);
    }
    return true;
}

bool UpdateActionStatus(uint64_t actionId, ELLE_ACTION_STATUS status,
                        const std::string& result) {
    EnsureActionQueueTable();
    int64_t nowMs = (int64_t)ELLE_MS_NOW();
    bool terminal = (status == ACTION_COMPLETED_SUCCESS ||
                     status == ACTION_COMPLETED_FAILURE ||
                     status == ACTION_TIMEOUT ||
                     status == ACTION_CANCELLED);
    bool starting = (status == ACTION_LOCKED || status == ACTION_EXECUTING);

    std::string sql = "UPDATE ElleCore.dbo.action_queue SET status = ?";
    std::vector<std::string> params = { std::to_string((int)status) };
    if (!result.empty()) {
        sql += ", result = ?";
        params.push_back(result);
    }
    if (starting) {
        sql += ", started_ms = COALESCE(started_ms, ?)";
        params.push_back(std::to_string(nowMs));
    }
    if (terminal) {
        sql += ", completed_ms = ?";
        params.push_back(std::to_string(nowMs));
    }
    sql += " WHERE id = ?;";
    params.push_back(std::to_string((int64_t)actionId));
    return ElleSQLPool::Instance().QueryParams(sql, params).success;
}

}
