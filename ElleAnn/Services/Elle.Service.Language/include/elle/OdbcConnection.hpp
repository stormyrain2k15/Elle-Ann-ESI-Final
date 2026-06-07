#pragma once

#include "elle/Config.hpp"
#include "elle/Types.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

namespace elle::odbc {

class OdbcException : public std::runtime_error {
public:
    OdbcException(std::string what, std::string sqlState, std::int32_t nativeError)
        : std::runtime_error(std::move(what)),
          m_sqlState(std::move(sqlState)),
          m_nativeError(nativeError) {}

    [[nodiscard]] const std::string& sqlState()   const noexcept { return m_sqlState; }
    [[nodiscard]] std::int32_t       nativeError() const noexcept { return m_nativeError; }

private:
    std::string  m_sqlState;
    std::int32_t m_nativeError;
};

void checkRc(SQLRETURN rc, SQLSMALLINT handleType, SQLHANDLE handle, const char* op);

template <SQLSMALLINT HandleType>
class Handle {
public:
    Handle() noexcept = default;
    explicit Handle(SQLHANDLE parent) {
        const SQLRETURN rc = ::SQLAllocHandle(HandleType, parent, &m_h);
        checkRc(rc, HandleType, m_h, "SQLAllocHandle");
    }
    Handle(const Handle&)            = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& other) noexcept : m_h(other.m_h) { other.m_h = SQL_NULL_HANDLE; }
    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) { reset(); m_h = other.m_h; other.m_h = SQL_NULL_HANDLE; }
        return *this;
    }
    ~Handle() { reset(); }

    void reset() noexcept {
        if (m_h != SQL_NULL_HANDLE) {
            ::SQLFreeHandle(HandleType, m_h);
            m_h = SQL_NULL_HANDLE;
        }
    }

    [[nodiscard]] SQLHANDLE get() const noexcept { return m_h; }
    explicit operator bool() const noexcept     { return m_h != SQL_NULL_HANDLE; }

private:
    SQLHANDLE m_h = SQL_NULL_HANDLE;
};

using EnvHandle  = Handle<SQL_HANDLE_ENV>;
using DbcHandle  = Handle<SQL_HANDLE_DBC>;
using StmtHandle = Handle<SQL_HANDLE_STMT>;

class Connection {
public:
    explicit Connection(const DatabaseConfig& cfg);
    Connection(const Connection&)            = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;
    ~Connection();

    [[nodiscard]] SQLHDBC dbc() const noexcept;

    class Statement {
    public:
        explicit Statement(SQLHDBC dbc, std::string_view sqlUtf8);
        ~Statement();
        Statement(const Statement&)            = delete;
        Statement& operator=(const Statement&) = delete;

        void bindInt64(std::int64_t value);
        void bindInt64(std::optional<std::int64_t> value);
        void bindWString(const std::wstring& value);
        void bindUtf8(std::string_view value);
        void bindDouble(double value);

        void execute();
        bool fetch();

        [[nodiscard]] std::int64_t getInt64(SQLUSMALLINT col);
        [[nodiscard]] std::optional<std::int64_t> getInt64Optional(SQLUSMALLINT col);
        [[nodiscard]] double       getDouble(SQLUSMALLINT col);
        [[nodiscard]] std::string  getString(SQLUSMALLINT col);
        [[nodiscard]] bool         getBool(SQLUSMALLINT col);

    private:
        StmtHandle  m_stmt;
        SQLUSMALLINT m_paramIndex = 0;

        std::vector<std::unique_ptr<std::int64_t>>  m_int64Holders;
        std::vector<std::unique_ptr<std::wstring>>  m_wstrHolders;
        std::vector<std::unique_ptr<double>>        m_doubleHolders;
        std::vector<std::unique_ptr<SQLLEN>>        m_lenHolders;
    };

private:
    static std::string buildConnectionString(const DatabaseConfig& cfg);

    EnvHandle m_env;
    DbcHandle m_dbc;
};

}
