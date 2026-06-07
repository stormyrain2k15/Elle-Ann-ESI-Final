#include "ElleSQLConn.h"
#include "ElleLogger.h"
#include "ElleSQLFallback.h"

#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <set>
#include <random>

int64_t SQLRow::GetIntOr(size_t idx, int64_t fallback) const {
    if (idx >= values.size() || values[idx].empty()) return fallback;

    const std::string& raw = values[idx];
    size_t b = 0, e = raw.size();
    while (b < e && (raw[b] == ' ' || raw[b] == '\t')) ++b;
    while (e > b && (raw[e-1] == ' ' || raw[e-1] == '\t')) --e;
    if (b == e) return fallback;
    std::string s = raw.substr(b, e - b);
    errno = 0;
    char* endp = nullptr;
    long long v = std::strtoll(s.c_str(), &endp, 10);
    if (errno == ERANGE || !endp || endp == s.c_str() || *endp != '\0') {
        ELLE_WARN("SQLRow::GetIntOr(col=%zu) coerced non-integer '%s' -> %lld",
                  idx, raw.c_str(), (long long)fallback);
        return fallback;
    }
    return (int64_t)v;
}

double SQLRow::GetFloatOr(size_t idx, double fallback) const {
    if (idx >= values.size() || values[idx].empty()) return fallback;
    const std::string& raw = values[idx];
    size_t b = 0, e = raw.size();
    while (b < e && (raw[b] == ' ' || raw[b] == '\t')) ++b;
    while (e > b && (raw[e-1] == ' ' || raw[e-1] == '\t')) --e;
    if (b == e) return fallback;
    std::string s = raw.substr(b, e - b);
    errno = 0;
    char* endp = nullptr;
    double v = std::strtod(s.c_str(), &endp);
    if (errno == ERANGE || !endp || endp == s.c_str() || *endp != '\0') {
        ELLE_WARN("SQLRow::GetFloatOr(col=%zu) coerced non-numeric '%s' -> %g",
                  idx, raw.c_str(), fallback);
        return fallback;
    }
    return v;
}

bool SQLRow::TryGetInt(size_t idx, int64_t& outVal) const {
    if (idx >= values.size() || values[idx].empty() ||
        values[idx] == "NULL") return false;
    const std::string& raw = values[idx];
    size_t b = 0, e = raw.size();
    while (b < e && (raw[b] == ' ' || raw[b] == '\t')) ++b;
    while (e > b && (raw[e-1] == ' ' || raw[e-1] == '\t')) --e;
    if (b == e) return false;
    std::string s = raw.substr(b, e - b);
    errno = 0;
    char* endp = nullptr;
    long long v = std::strtoll(s.c_str(), &endp, 10);
    if (errno == ERANGE || !endp || endp == s.c_str() || *endp != '\0')
        return false;
    outVal = (int64_t)v;
    return true;
}

bool SQLRow::TryGetFloat(size_t idx, double& outVal) const {
    if (idx >= values.size() || values[idx].empty() ||
        values[idx] == "NULL") return false;
    const std::string& raw = values[idx];
    size_t b = 0, e = raw.size();
    while (b < e && (raw[b] == ' ' || raw[b] == '\t')) ++b;
    while (e > b && (raw[e-1] == ' ' || raw[e-1] == '\t')) --e;
    if (b == e) return false;
    std::string s = raw.substr(b, e - b);
    errno = 0;
    char* endp = nullptr;
    double v = std::strtod(s.c_str(), &endp);
    if (errno == ERANGE || !endp || endp == s.c_str() || *endp != '\0')
        return false;
    outVal = v;
    return true;
}

bool SQLRow::IsNull(size_t idx) const {
    return idx >= values.size() || values[idx] == "NULL";
}

int SQLResultSet::ColIndex(const std::string& name) const {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == name) return (int)i;
    }
    return -1;
}

SQLConnection::SQLConnection() {}
SQLConnection::~SQLConnection() { Disconnect(); }

bool SQLConnection::AllocHandles() {
    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
    if (!SQL_SUCCEEDED(ret)) return false;

    ret = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret)) { FreeHandles(); return false; }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
    if (!SQL_SUCCEEDED(ret)) { FreeHandles(); return false; }

#ifdef SQL_COPT_SS_MARS_ENABLED
    SQLSetConnectAttrA(m_hDbc, SQL_COPT_SS_MARS_ENABLED,
                       (SQLPOINTER)SQL_MARS_ENABLED_YES, 0);
#endif

    return true;
}

void SQLConnection::FreeHandles() {
    if (m_hDbc) { SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); m_hDbc = nullptr; }
    if (m_hEnv) { SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv); m_hEnv = nullptr; }
}

bool SQLConnection::Connect(const std::string& connectionString) {
    if (m_connected) Disconnect();
    m_connStr = connectionString;

    if (m_connStr.find("MARS_Connection=") == std::string::npos) {
        if (!m_connStr.empty() && m_connStr.back() != ';') m_connStr.push_back(';');
        m_connStr += "MARS_Connection=Yes;";
    }

    if (!AllocHandles()) {
        m_lastError = "Failed to allocate ODBC handles";
        return false;
    }

    SQLSetConnectAttrA(m_hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)10, 0);

    SQLCHAR outStr[1024];
    SQLSMALLINT outLen;
    SQLRETURN ret = SQLDriverConnectA(m_hDbc, nullptr,
        (SQLCHAR*)m_connStr.c_str(), (SQLSMALLINT)m_connStr.length(),
        outStr, sizeof(outStr), &outLen, SQL_DRIVER_NOPROMPT);

    if (!SQL_SUCCEEDED(ret)) {
        m_lastError = GetDiagnostics(SQL_HANDLE_DBC, m_hDbc);
        ELLE_WARN("SQL connect failed: %s", m_lastError.c_str());
        ELLE_LOG_SQL("connect FAILED: %s", m_lastError.c_str());
        FreeHandles();
        return false;
    }

    m_connected = true;
    m_lastUsed = ELLE_MS_NOW();
    ELLE_LOG_SQL("connect OK conn=%p", (void*)m_hDbc);
    return true;
}

void SQLConnection::Disconnect() {
    if (m_inTransaction) Rollback();
    if (m_connected && m_hDbc) {
        SQLDisconnect(m_hDbc);
    }
    FreeHandles();
    m_connected = false;
}

bool SQLConnection::IsConnected() const { return m_connected; }

bool SQLConnection::Ping() {
    if (!m_connected) return false;
    SQLResultSet rs = Execute("SELECT 1");
    return rs.success;
}

SQLResultSet SQLConnection::CollectStatementResults(SQLHSTMT hStmt, SQLRETURN execRet) {
    SQLResultSet result;

    auto drainAndClose = [&]() {
        for (;;) {
            SQLRETURN mr = SQLMoreResults(hStmt);
            if (mr == SQL_NO_DATA) break;
            if (mr != SQL_SUCCESS && mr != SQL_SUCCESS_WITH_INFO) break;
        }

        SQLCloseCursor(hStmt);
        SQLFreeStmt(hStmt, SQL_CLOSE);
    };

    if (execRet == SQL_ERROR) {
        SQLINTEGER native = 0;
        SQLCHAR state[6] = {0};
        SQLCHAR msg[256] = {0};
        SQLSMALLINT msgLen = 0;
        if (SQLGetDiagRecA(SQL_HANDLE_STMT, hStmt, 1, state, &native, msg, sizeof(msg), &msgLen) == SQL_SUCCESS) {
            if (std::string((char*)state) == "24000") {
                result.error = "24000: invalid cursor state";
                drainAndClose();
                return result;
            }
        }
    }

    if (execRet != SQL_SUCCESS && execRet != SQL_SUCCESS_WITH_INFO &&
        execRet != SQL_NO_DATA) {
        result.error = GetDiagnostics(SQL_HANDLE_STMT, hStmt);
        drainAndClose();
        return result;
    }

    SQLSMALLINT numCols = 0;
    SQLNumResultCols(hStmt, &numCols);
    for (SQLSMALLINT i = 1; i <= numCols; i++) {
        SQLColumn col;
        SQLCHAR colName[256];
        SQLSMALLINT nameLen = 0, dataType = 0, nullable = 0;
        SQLULEN colSize = 0;
        SQLSMALLINT decDigits = 0;
        SQLDescribeColA(hStmt, i, colName, sizeof(colName), &nameLen,
                        &dataType, &colSize, &decDigits, &nullable);
        col.name = std::string((char*)colName, nameLen);
        col.type = dataType;
        col.size = (uint32_t)colSize;
        result.columns.push_back(col);
    }

    for (;;) {
        SQLRETURN fr = SQLFetch(hStmt);
        if (fr == SQL_NO_DATA) break;
        if (fr != SQL_SUCCESS && fr != SQL_SUCCESS_WITH_INFO) {
            result.error = GetDiagnostics(SQL_HANDLE_STMT, hStmt);
            drainAndClose();
            return result;
        }
        SQLRow row;
        for (SQLSMALLINT i = 1; i <= numCols; i++) {
            std::string cell;
            SQLCHAR buf[8192];
            SQLLEN indicator = 0;
            SQLRETURN gr = SQLGetData(hStmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
            if (indicator == SQL_NULL_DATA) {
                row.values.push_back("NULL");
                continue;
            }
            if (gr == SQL_SUCCESS_WITH_INFO) {

                size_t firstLen = (indicator > 0 &&
                                   (SQLLEN)(sizeof(buf) - 1) < indicator)
                                    ? sizeof(buf) - 1
                                    : strnlen((char*)buf, sizeof(buf));
                cell.append((char*)buf, firstLen);
                while (gr == SQL_SUCCESS_WITH_INFO) {
                    gr = SQLGetData(hStmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
                    if (gr == SQL_NO_DATA) break;
                    if (gr != SQL_SUCCESS && gr != SQL_SUCCESS_WITH_INFO) {
                        ELLE_WARN("SQLGetData chunk failed: %s",
                                  GetDiagnostics(SQL_HANDLE_STMT, hStmt).c_str());
                        break;
                    }
                    size_t chunkLen = strnlen((char*)buf, sizeof(buf));
                    cell.append((char*)buf, chunkLen);
                }
            } else if (gr == SQL_SUCCESS) {
                cell = std::string((char*)buf);
            } else if (gr == SQL_NO_DATA) {

            } else {
                cell = "";
            }
            row.values.push_back(std::move(cell));
        }
        result.rows.push_back(std::move(row));
    }

    SQLLEN rowCount = 0;
    SQLRowCount(hStmt, &rowCount);
    result.rows_affected = rowCount;
    result.success = true;
    m_lastUsed = ELLE_MS_NOW();
    drainAndClose();
    return result;
}

SQLResultSet SQLConnection::Execute(const std::string& sql) {
    SQLResultSet result;
    if (!m_connected) {
        result.error = "Not connected";
        return result;
    }

    SQLHSTMT hStmt = nullptr;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
    if (!SQL_SUCCEEDED(ret)) {
        result.error = "Failed to allocate statement";
        return result;
    }

    ret = SQLExecDirectA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    result = CollectStatementResults(hStmt, ret);

    SQLCloseCursor(hStmt);
    SQLFreeStmt(hStmt, SQL_CLOSE);
    SQLFreeStmt(hStmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hStmt, SQL_UNBIND);
    SQLFreeStmt(hStmt, SQL_CLOSE);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return result;
}

SQLResultSet SQLConnection::ExecuteParams(const std::string& sql,
                                           const std::vector<std::string>& params) {

    if (!m_connected) {
        SQLResultSet rs; rs.success = false; rs.error = "not connected";
        return rs;
    }

    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
    if (!SQL_SUCCEEDED(ret)) {
        SQLResultSet rs; rs.success = false;
        rs.error = "SQLAllocHandle(STMT) failed: " + GetDiagnostics(SQL_HANDLE_DBC, m_hDbc);
        return rs;
    }

    ret = SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        SQLResultSet rs; rs.success = false;
        rs.error = "SQLPrepare failed: " + GetDiagnostics(SQL_HANDLE_STMT, hStmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return rs;
    }

    std::vector<SQLLEN> cbLens(params.size(), SQL_NTS);
    for (size_t i = 0; i < params.size(); i++) {

        cbLens[i] = (SQLLEN)params[i].size();
        SQLRETURN br = SQLBindParameter(
            hStmt,
            (SQLUSMALLINT)(i + 1),
            SQL_PARAM_INPUT,
            SQL_C_CHAR,
            SQL_VARCHAR,
            (SQLULEN)params[i].size() + 1,
            0,
            (SQLPOINTER)params[i].c_str(),
            (SQLLEN)params[i].size() + 1,
            &cbLens[i]);
        if (!SQL_SUCCEEDED(br)) {
            SQLResultSet rs; rs.success = false;
            rs.error = "SQLBindParameter(" + std::to_string(i + 1) + ") failed: "
                       + GetDiagnostics(SQL_HANDLE_STMT, hStmt);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return rs;
        }
    }

    ret = SQLExecute(hStmt);
    SQLResultSet rs = CollectStatementResults(hStmt, ret);

    SQLCloseCursor(hStmt);
    SQLFreeStmt(hStmt, SQL_CLOSE);

    SQLFreeStmt(hStmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hStmt, SQL_UNBIND);
    SQLFreeStmt(hStmt, SQL_CLOSE);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return rs;
}

SQLResultSet SQLConnection::CallProc(const std::string& proc,
                                      const std::vector<std::string>& params) {

    std::ostringstream ss;
    ss << "{CALL " << proc;
    if (!params.empty()) {
        ss << "(";
        for (size_t i = 0; i < params.size(); i++) {
            if (i) ss << ",";
            ss << "?";
        }
        ss << ")";
    }
    ss << "}";
    return ExecuteParams(ss.str(), params);
}

int64_t SQLConnection::ExecuteScalar(const std::string& sql) {
    SQLResultSet rs = Execute(sql);
    if (rs.success && !rs.rows.empty() && !rs.rows[0].values.empty()) {
        return rs.rows[0].GetIntOr(0, 0);
    }
    return 0;
}

bool SQLConnection::ExecuteNonQuery(const std::string& sql) {
    SQLResultSet rs = Execute(sql);
    return rs.success;
}

bool SQLConnection::BeginTransaction() {
    if (!m_connected || m_inTransaction) return false;
    SQLRETURN ret = SQLSetConnectAttrA(m_hDbc, SQL_ATTR_AUTOCOMMIT,
                                        (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    m_inTransaction = SQL_SUCCEEDED(ret);
    return m_inTransaction;
}

bool SQLConnection::Commit() {
    if (!m_inTransaction) return false;
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hDbc, SQL_COMMIT);
    SQLSetConnectAttrA(m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
    m_inTransaction = false;
    return SQL_SUCCEEDED(ret);
}

bool SQLConnection::Rollback() {
    if (!m_inTransaction) return false;
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hDbc, SQL_ROLLBACK);
    SQLSetConnectAttrA(m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
    m_inTransaction = false;
    return SQL_SUCCEEDED(ret);
}

std::string SQLConnection::GetDiagnostics(int16_t handleType, void* handle) {
    SQLCHAR state[6], msg[1024];
    SQLINTEGER nativeErr;
    SQLSMALLINT msgLen;
    std::string result;
    SQLSMALLINT i = 1;
    while (SQLGetDiagRecA(handleType, handle, i++, state, &nativeErr,
                           msg, sizeof(msg), &msgLen) == SQL_SUCCESS) {
        if (!result.empty()) result += "; ";
        result += std::string((char*)state) + ": " + std::string((char*)msg, msgLen);
    }
    return result.empty() ? "Unknown ODBC error" : result;
}

ElleSQLPool& ElleSQLPool::Instance() {
    static ElleSQLPool inst;
    return inst;
}

bool ElleSQLPool::Initialize(const std::string& connectionString, uint32_t poolSize) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return true;

    m_connStr = connectionString;
    m_poolSize = poolSize;

    ElleSQLFallback::Instance().Initialize(true);

    for (uint32_t i = 0; i < poolSize; i++) {
        std::shared_ptr<SQLConnection> conn;
        if (CreateConnection(conn)) {
            m_available.push(conn);
        } else {
            ELLE_WARN("SQL pool: Failed to create connection %d/%d", i + 1, poolSize);
        }
    }

    m_initialized = !m_available.empty();

    if (m_initialized) ElleSQLFallback::Instance().NudgeDrain();
    return m_initialized;
}

void ElleSQLPool::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_available.empty()) {
        m_available.front()->Disconnect();
        m_available.pop();
    }
    m_initialized = false;
}

bool ElleSQLPool::Reinitialize(const std::string& connectionString, uint32_t poolSize) {

    Shutdown();
    return Initialize(connectionString, poolSize);
}

bool ElleSQLPool::CreateConnection(std::shared_ptr<SQLConnection>& conn) {
    conn = std::make_shared<SQLConnection>();
    return conn->Connect(m_connStr);
}

std::shared_ptr<SQLConnection> ElleSQLPool::Acquire(uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                       [this] { return !m_available.empty(); })) {
        ELLE_WARN("SQL pool: Acquire timeout after %dms", timeoutMs);
        ELLE_LOG_SQL("pool acquire TIMEOUT %dms (available=%zu)",
                     timeoutMs, m_available.size());
        return nullptr;
    }

    auto conn = m_available.front();
    m_available.pop();

    if (!conn->Ping()) {
        ELLE_WARN("SQL pool: Stale connection, reconnecting...");
        ELLE_LOG_SQL("pool: stale conn, reconnecting");
        conn->Disconnect();
        if (!conn->Connect(m_connStr)) {
            ELLE_ERROR("SQL pool: Reconnection failed");
            ELLE_LOG_SQL("pool: reconnect FAILED");
            return nullptr;
        }
        ELLE_LOG_SQL("pool: reconnect OK");

        ElleSQLFallback::Instance().NudgeDrain();
    }

    return conn;
}

void ElleSQLPool::Release(std::shared_ptr<SQLConnection> conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_available.push(conn);
    m_cv.notify_one();
}

SQLResultSet ElleSQLPool::Query(const std::string& sql) {
    auto conn = Acquire();
    if (!conn) return {};
    m_totalQueries++;
    auto result = conn->Execute(sql);
    Release(conn);
    return result;
}

SQLResultSet ElleSQLPool::QueryParams(const std::string& sql, const std::vector<std::string>& params) {
    auto conn = Acquire();
    if (!conn) {

        if (ElleSQLFallback::Instance().IsEnabled()) {
            ElleSQLFallback::Instance().Enqueue(
                ElleSQLFallback::Kind::QueryParams, sql, params);
        }
        return {};
    }
    m_totalQueries++;
    auto result = conn->ExecuteParams(sql, params);
    Release(conn);
    return result;
}

SQLResultSet ElleSQLPool::CallProc(const std::string& proc, const std::vector<std::string>& params) {
    auto conn = Acquire();
    if (!conn) return {};
    m_totalQueries++;
    auto result = conn->CallProc(proc, params);
    Release(conn);
    return result;
}

bool ElleSQLPool::Exec(const std::string& sql) {
    auto conn = Acquire();
    if (!conn) {

        if (ElleSQLFallback::Instance().IsEnabled() &&
            ElleSQLFallback::Instance().Enqueue(ElleSQLFallback::Kind::Exec, sql, {})) {
            return true;
        }
        return false;
    }
    m_totalQueries++;
    bool ok = conn->ExecuteNonQuery(sql);
    Release(conn);
    if (!ok && ElleSQLFallback::Instance().IsEnabled()) {

        ElleSQLFallback::Instance().Enqueue(ElleSQLFallback::Kind::Exec, sql, {});
    }
    return ok;
}

int64_t ElleSQLPool::Scalar(const std::string& sql) {
    auto conn = Acquire();
    if (!conn) return 0;
    m_totalQueries++;
    int64_t val = conn->ExecuteScalar(sql);
    Release(conn);
    return val;
}

uint32_t ElleSQLPool::AvailableConnections() const {
    return (uint32_t)m_available.size();
}
