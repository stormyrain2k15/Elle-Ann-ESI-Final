// ============================================================================
// Elle Engine -- OdbcConnection implementation
// File: src/OdbcConnection.cpp
// ============================================================================
#include "elle/OdbcConnection.hpp"
#include "elle/StringUtil.hpp"

#include <array>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace elle::odbc {

namespace {

std::string diagText(SQLSMALLINT handleType, SQLHANDLE handle,
                     std::string& sqlState, std::int32_t& nativeError) {
    SQLWCHAR     state[6] = {0};
    SQLINTEGER   nat      = 0;
    SQLWCHAR     msg[1024] = {0};
    SQLSMALLINT  msgLen    = 0;

    SQLRETURN rc = ::SQLGetDiagRecW(handleType, handle, 1, state,
                                    &nat, msg,
                                    static_cast<SQLSMALLINT>(std::size(msg)),
                                    &msgLen);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        sqlState    = "?????";
        nativeError = 0;
        return "ODBC diagnostic unavailable";
    }
    std::wstring wstate(state, state + 5);
    sqlState    = str::wideToUtf8(wstate);
    nativeError = static_cast<std::int32_t>(nat);
    return str::wideToUtf8(std::wstring(msg, msg + msgLen));
}

} // namespace

void checkRc(SQLRETURN rc, SQLSMALLINT handleType, SQLHANDLE handle, const char* op) {
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO || rc == SQL_NO_DATA) {
        return;
    }
    std::string state;
    std::int32_t nat = 0;
    const std::string detail = diagText(handleType, handle, state, nat);
    std::ostringstream oss;
    oss << "ODBC " << op << " failed: rc=" << rc
        << ", sqlstate=" << state
        << ", native=" << nat
        << ", message=" << detail;
    throw OdbcException(oss.str(), std::move(state), nat);
}

// ---------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------
Connection::Connection(const DatabaseConfig& cfg) {
    m_env = EnvHandle(SQL_NULL_HANDLE);
    // Set ODBC 3 behaviour.
    checkRc(::SQLSetEnvAttr(static_cast<SQLHENV>(m_env.get()),
                            SQL_ATTR_ODBC_VERSION,
                            reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0),
            SQL_HANDLE_ENV, m_env.get(), "SQLSetEnvAttr");

    m_dbc = DbcHandle(m_env.get());

    // Timeouts before connect.
    if (cfg.connectionTimeoutMs > 0) {
        ::SQLSetConnectAttr(static_cast<SQLHDBC>(m_dbc.get()),
                            SQL_ATTR_LOGIN_TIMEOUT,
                            reinterpret_cast<SQLPOINTER>(static_cast<SQLLEN>(cfg.connectionTimeoutMs / 1000)),
                            0);
    }

    const std::string  connStr = buildConnectionString(cfg);
    const std::wstring wConn   = str::utf8ToWide(connStr);

    SQLWCHAR    outStr[1024] = {0};
    SQLSMALLINT outStrLen    = 0;
    const SQLRETURN rc = ::SQLDriverConnectW(
        static_cast<SQLHDBC>(m_dbc.get()),
        nullptr,
        reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(wConn.c_str())),
        static_cast<SQLSMALLINT>(wConn.size()),
        outStr,
        static_cast<SQLSMALLINT>(std::size(outStr)),
        &outStrLen,
        SQL_DRIVER_NOPROMPT);
    checkRc(rc, SQL_HANDLE_DBC, m_dbc.get(), "SQLDriverConnect");
}

Connection::Connection(Connection&&) noexcept            = default;
Connection& Connection::operator=(Connection&&) noexcept = default;
Connection::~Connection()                                = default;

SQLHDBC Connection::dbc() const noexcept {
    return static_cast<SQLHDBC>(m_dbc.get());
}

std::string Connection::buildConnectionString(const DatabaseConfig& cfg) {
    std::ostringstream oss;
    oss << "Driver={"  << cfg.driver   << "};";
    oss << "Server="   << cfg.server   << ";";
    oss << "Database=" << cfg.database << ";";
    if (cfg.trustedConnection) {
        oss << "Trusted_Connection=Yes;";
    } else {
        oss << "Uid=" << cfg.user << ";";
        oss << "Pwd=" << cfg.password << ";";
    }
    oss << "Encrypt=" << (cfg.encrypt ? "Yes" : "No") << ";";
    if (cfg.trustServerCert) {
        oss << "TrustServerCertificate=Yes;";
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// Statement
// ---------------------------------------------------------------------------
Connection::Statement::Statement(SQLHDBC dbc, std::string_view sqlUtf8)
    : m_stmt(dbc) {
    const std::wstring wsql = str::utf8ToWide(sqlUtf8);
    const SQLRETURN rc = ::SQLPrepareW(
        static_cast<SQLHSTMT>(m_stmt.get()),
        reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(wsql.c_str())),
        static_cast<SQLINTEGER>(wsql.size()));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLPrepare");
}

Connection::Statement::~Statement() = default;

void Connection::Statement::bindInt64(std::int64_t value) {
    ++m_paramIndex;
    auto holder = std::make_unique<std::int64_t>(value);
    auto lenH   = std::make_unique<SQLLEN>(sizeof(std::int64_t));
    const SQLRETURN rc = ::SQLBindParameter(
        static_cast<SQLHSTMT>(m_stmt.get()), m_paramIndex,
        SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT,
        0, 0, holder.get(), 0, lenH.get());
    m_int64Holders.push_back(std::move(holder));
    m_lenHolders.push_back(std::move(lenH));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLBindParameter(int64)");
}

void Connection::Statement::bindInt64(std::optional<std::int64_t> value) {
    ++m_paramIndex;
    auto holder = std::make_unique<std::int64_t>(value.value_or(0));
    auto lenH   = std::make_unique<SQLLEN>(value ? sizeof(std::int64_t) : SQL_NULL_DATA);
    const SQLRETURN rc = ::SQLBindParameter(
        static_cast<SQLHSTMT>(m_stmt.get()), m_paramIndex,
        SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT,
        0, 0, holder.get(), 0, lenH.get());
    m_int64Holders.push_back(std::move(holder));
    m_lenHolders.push_back(std::move(lenH));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLBindParameter(int64-opt)");
}

void Connection::Statement::bindWString(const std::wstring& value) {
    ++m_paramIndex;
    auto holder = std::make_unique<std::wstring>(value);
    auto lenH   = std::make_unique<SQLLEN>(static_cast<SQLLEN>(holder->size() * sizeof(wchar_t)));
    const SQLRETURN rc = ::SQLBindParameter(
        static_cast<SQLHSTMT>(m_stmt.get()), m_paramIndex,
        SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        holder->size(), 0,
        reinterpret_cast<SQLPOINTER>(const_cast<wchar_t*>(holder->c_str())),
        static_cast<SQLLEN>(holder->size() * sizeof(wchar_t)),
        lenH.get());
    m_wstrHolders.push_back(std::move(holder));
    m_lenHolders.push_back(std::move(lenH));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLBindParameter(wstring)");
}

void Connection::Statement::bindUtf8(std::string_view value) {
    bindWString(str::utf8ToWide(value));
}

void Connection::Statement::bindDouble(double value) {
    ++m_paramIndex;
    auto holder = std::make_unique<double>(value);
    auto lenH   = std::make_unique<SQLLEN>(sizeof(double));
    const SQLRETURN rc = ::SQLBindParameter(
        static_cast<SQLHSTMT>(m_stmt.get()), m_paramIndex,
        SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE,
        15, 0, holder.get(), 0, lenH.get());
    m_doubleHolders.push_back(std::move(holder));
    m_lenHolders.push_back(std::move(lenH));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLBindParameter(double)");
}

void Connection::Statement::execute() {
    const SQLRETURN rc = ::SQLExecute(static_cast<SQLHSTMT>(m_stmt.get()));
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLExecute");
}

bool Connection::Statement::fetch() {
    const SQLRETURN rc = ::SQLFetch(static_cast<SQLHSTMT>(m_stmt.get()));
    if (rc == SQL_NO_DATA) return false;
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLFetch");
    return true;
}

std::int64_t Connection::Statement::getInt64(SQLUSMALLINT col) {
    std::int64_t v = 0;
    SQLLEN ind = 0;
    const SQLRETURN rc = ::SQLGetData(static_cast<SQLHSTMT>(m_stmt.get()),
                                      col, SQL_C_SBIGINT,
                                      &v, sizeof(v), &ind);
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLGetData(int64)");
    return ind == SQL_NULL_DATA ? 0 : v;
}

std::optional<std::int64_t> Connection::Statement::getInt64Optional(SQLUSMALLINT col) {
    std::int64_t v = 0;
    SQLLEN ind = 0;
    const SQLRETURN rc = ::SQLGetData(static_cast<SQLHSTMT>(m_stmt.get()),
                                      col, SQL_C_SBIGINT,
                                      &v, sizeof(v), &ind);
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLGetData(int64-opt)");
    if (ind == SQL_NULL_DATA) return std::nullopt;
    return v;
}

double Connection::Statement::getDouble(SQLUSMALLINT col) {
    double v = 0.0;
    SQLLEN ind = 0;
    const SQLRETURN rc = ::SQLGetData(static_cast<SQLHSTMT>(m_stmt.get()),
                                      col, SQL_C_DOUBLE,
                                      &v, sizeof(v), &ind);
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLGetData(double)");
    return ind == SQL_NULL_DATA ? 0.0 : v;
}

bool Connection::Statement::getBool(SQLUSMALLINT col) {
    SQLCHAR v = 0;
    SQLLEN  ind = 0;
    const SQLRETURN rc = ::SQLGetData(static_cast<SQLHSTMT>(m_stmt.get()),
                                      col, SQL_C_BIT,
                                      &v, sizeof(v), &ind);
    checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLGetData(bool)");
    return ind != SQL_NULL_DATA && v != 0;
}

std::string Connection::Statement::getString(SQLUSMALLINT col) {
    // Fetch in chunks so we don't fail on NVARCHAR(MAX).
    std::wstring out;
    std::array<wchar_t, 1024> buf{};
    while (true) {
        SQLLEN ind = 0;
        const SQLRETURN rc = ::SQLGetData(static_cast<SQLHSTMT>(m_stmt.get()),
                                          col, SQL_C_WCHAR,
                                          buf.data(),
                                          static_cast<SQLLEN>(buf.size() * sizeof(wchar_t)),
                                          &ind);
        if (rc == SQL_NO_DATA) break;
        checkRc(rc, SQL_HANDLE_STMT, m_stmt.get(), "SQLGetData(string)");
        if (ind == SQL_NULL_DATA) return {};
        const std::size_t copied = (ind == SQL_NO_TOTAL || ind > static_cast<SQLLEN>(buf.size() * sizeof(wchar_t)))
            ? buf.size() - 1
            : static_cast<std::size_t>(ind / sizeof(wchar_t));
        out.append(buf.data(), copied);
        if (rc == SQL_SUCCESS) break;       // entire field delivered
    }
    return str::wideToUtf8(out);
}

} // namespace elle::odbc
