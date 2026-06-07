#include "ElleGameAccountDB.h"
#include "ElleLogger.h"

#include <chrono>

ElleGameAccountPool& ElleGameAccountPool::Instance() {
    static ElleGameAccountPool s;
    return s;
}

bool ElleGameAccountPool::Initialize(const std::string& connectionString,
                                     uint32_t poolSize) {
    if (m_initialized) return true;
    if (connectionString.empty()) {
        ELLE_WARN("ElleGameAccountPool: empty connection string — game-DB "
                  "auth path disabled");
        return false;
    }
    m_connStr  = connectionString;
    m_poolSize = poolSize;
    for (uint32_t i = 0; i < poolSize; ++i) {
        auto c = std::make_shared<SQLConnection>();
        if (!c->Connect(m_connStr)) {
            ELLE_ERROR("ElleGameAccountPool: failed to connect (pool[%u])", i);
            return false;
        }
        m_available.push(c);
    }
    m_initialized = true;
    ELLE_INFO("ElleGameAccountPool: %u connections to game Account DB",
              (unsigned)poolSize);
    return true;
}

void ElleGameAccountPool::Shutdown() {
    std::lock_guard<std::mutex> lk(m_mutex);
    while (!m_available.empty()) {
        auto c = m_available.front(); m_available.pop();
        if (c) c->Disconnect();
    }
    m_initialized = false;
}

static std::shared_ptr<SQLConnection> AcquireFrom(
        std::queue<std::shared_ptr<SQLConnection>>& q,
        std::mutex& mx, std::condition_variable& cv,
        uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lk(mx);
    if (!cv.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                     [&]{ return !q.empty(); })) {
        return nullptr;
    }
    auto c = q.front(); q.pop();
    return c;
}

SQLResultSet ElleGameAccountPool::Query(const std::string& sql) {
    SQLResultSet rs;
    if (!m_initialized) { rs.error = "game-DB pool not initialized"; return rs; }
    auto c = AcquireFrom(m_available, m_mutex, m_cv, 5000);
    if (!c) { rs.error = "game-DB pool acquire timeout"; return rs; }
    rs = c->Execute(sql);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_available.push(c);
    }
    m_cv.notify_one();
    return rs;
}

SQLResultSet ElleGameAccountPool::QueryParams(
        const std::string& sql,
        const std::vector<std::string>& params) {
    SQLResultSet rs;
    if (!m_initialized) { rs.error = "game-DB pool not initialized"; return rs; }
    auto c = AcquireFrom(m_available, m_mutex, m_cv, 5000);
    if (!c) { rs.error = "game-DB pool acquire timeout"; return rs; }
    rs = c->ExecuteParams(sql, params);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_available.push(c);
    }
    m_cv.notify_one();
    return rs;
}

namespace ElleGameAuth {

bool AuthenticateUser(const std::string& sUserID,
                      const std::string& sUserPW,
                      UserIdentity& out) {
    auto& pool = ElleGameAccountPool::Instance();
    if (!pool.IsAvailable()) return false;
    if (sUserID.empty() || sUserID.size() > 30) return false;
    if (sUserPW.empty() || sUserPW.size() > 20) return false;

    {

        {
            static constexpr const char* kSqlLoginGame =
                "DECLARE @userNo INT = 0, @authID TINYINT = 0, @block INT = 0, @isLoginable INT = 0; "
                "EXEC dbo.usp_User_loginGame @userID = ?, @userPW = ?, "
                "    @userNo = @userNo OUTPUT, @authID = @authID OUTPUT, "
                "    @block = @block OUTPUT, @isLoginable = @isLoginable OUTPUT; "
                "SELECT @userNo AS nUserNo, @authID AS nAuthID, @block AS bIsBlock, @isLoginable AS bIsLoginAble;";
            auto rs = pool.QueryParams(kSqlLoginGame, { sUserID, sUserPW });
            if (rs.success) {
                if (rs.rows.empty()) return false;
                const auto& r = rs.rows[0];
                int64_t v = 0;
                out.nUserNo   = r.TryGetInt(0, v) ? v : 0;
                out.sUserID   = sUserID;
                out.sUserName = "";
                out.bIsBlock  = (r.values.size() > 2 && r.GetIntOr(2, 0) != 0);
                out.bIsDelete = false;
                out.nAuthID   = (int32_t)(r.values.size() > 1 ? r.GetIntOr(1, 0) : 0);
                out.QX        = 'A';
                if (out.bIsBlock) return false;
                if (out.nUserNo <= 0) return false;
                return true;
            }
        }

        static constexpr const char* kSqlSetLogin =
            "EXEC dbo.usp_set_login @sUserID = ?, @sUserPW = ?;";
        {
            auto rs = pool.QueryParams(kSqlSetLogin, { sUserID, sUserPW });
            if (rs.success) {
                if (rs.rows.empty()) return false;
                const auto& r = rs.rows[0];
                int64_t v = 0;
                out.nUserNo   = r.TryGetInt(0, v) ? v : 0;
                out.sUserID   = r.values.size() > 1 ? r[1] : sUserID;
                out.sUserName = r.values.size() > 2 ? r[2] : "";
                out.bIsBlock  = (r.values.size() > 3 && r.GetIntOr(3, 0) != 0);
                out.bIsDelete = (r.values.size() > 4 && r.GetIntOr(4, 0) != 0);
                out.nAuthID   = r.values.size() > 5 ? (int32_t)r.GetIntOr(5, 0) : 0;
                out.QX        = (r.values.size() > 6 && !r[6].empty()) ? r[6][0] : 'A';
                if (out.bIsBlock || out.bIsDelete) return false;
                if (out.nUserNo <= 0) return false;
                return true;
            }
        }

        static std::once_flag s_procMissingLogged;
        std::call_once(s_procMissingLogged, [&](){
            ELLE_WARN("No usable login proc (tried usp_User_loginGame, usp_set_login); "
                      "falling back to direct SELECT on dbo.tUser");
        });
    }

    static constexpr const char* kSqlWithQx =
        "SELECT TOP 1 nUserNo, sUserID, sUserName, "
        "       ISNULL(bIsBlock,0) AS bIsBlock, "
        "       ISNULL(bIsDelete,0) AS bIsDelete, "
        "       ISNULL(nAuthID,0) AS nAuthID, "
        "       ISNULL(QX,'A')     AS QX "
        "  FROM dbo.tUser WITH(NOLOCK) "
        " WHERE sUserID = ? "
        "   AND sUserPW = ? "
        "   AND ISNULL(bIsDelete,0) = 0 "
        "   AND ISNULL(bIsBlock,0)  = 0;";
    static constexpr const char* kSqlNoQx =
        "SELECT TOP 1 nUserNo, sUserID, sUserName, "
        "       ISNULL(bIsBlock,0) AS bIsBlock, "
        "       ISNULL(bIsDelete,0) AS bIsDelete, "
        "       ISNULL(nAuthID,0) AS nAuthID "
        "  FROM dbo.tUser WITH(NOLOCK) "
        " WHERE sUserID = ? "
        "   AND sUserPW = ? "
        "   AND ISNULL(bIsDelete,0) = 0 "
        "   AND ISNULL(bIsBlock,0)  = 0;";

    auto rs = pool.QueryParams(kSqlWithQx, { sUserID, sUserPW });
    if (!rs.success) {

        if (rs.error.find("42S22") != std::string::npos) {
            rs = pool.QueryParams(kSqlNoQx, { sUserID, sUserPW });
        }
        if (!rs.success) {

            ELLE_WARN("game-auth query failed for sUserID=\"%s\": %s",
                      sUserID.c_str(), rs.error.c_str());
            return false;
        }
    }
    if (rs.rows.empty()) return false;

    const auto& r = rs.rows[0];
    int64_t v = 0;
    out.nUserNo   = r.TryGetInt(0, v) ? v : 0;
     out.sUserID   = r.values.size() > 1 ? r[1] : sUserID;
     out.sUserName = r.values.size() > 2 ? r[2] : "";
    out.bIsBlock  = (r.GetIntOr(3, 0) != 0);
    out.bIsDelete = (r.GetIntOr(4, 0) != 0);
    out.nAuthID   = (int32_t)r.GetIntOr(5, 0);
     out.QX        = (r.values.size() > 6 && !r[6].empty()) ? r[6][0] : 'A';

    if (out.nUserNo <= 0) return false;
    return true;
}

bool GetUserById(int64_t nUserNo, UserIdentity& out) {
    auto& pool = ElleGameAccountPool::Instance();
    if (!pool.IsAvailable() || nUserNo <= 0) return false;
    static constexpr const char* kSql =
        "SELECT nUserNo, sUserID, sUserName, "
        "       ISNULL(bIsBlock,0), ISNULL(bIsDelete,0), "
        "       ISNULL(nAuthID,0), ISNULL(QX,'A') "
        "  FROM dbo.tUser WITH(NOLOCK) WHERE nUserNo = ?;";
    auto rs = pool.QueryParams(kSql, { std::to_string(nUserNo) });
    if (!rs.success || rs.rows.empty()) return false;
    const auto& r = rs.rows[0];
    int64_t v = 0;
    out.nUserNo   = r.TryGetInt(0, v) ? v : 0;
    out.sUserID   = r[1];
    out.sUserName = r[2];
    out.bIsBlock  = (r.GetIntOr(3, 0) != 0);
    out.bIsDelete = (r.GetIntOr(4, 0) != 0);
    out.nAuthID   = (int32_t)r.GetIntOr(5, 0);
    out.QX        = r[6].empty() ? 'A' : r[6][0];
    if (out.nUserNo <= 0 || out.bIsBlock || out.bIsDelete) return false;
    return true;
}

}
