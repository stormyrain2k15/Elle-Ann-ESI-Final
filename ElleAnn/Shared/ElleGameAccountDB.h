#pragma once
#ifndef ELLE_GAME_ACCOUNT_DB_H
#define ELLE_GAME_ACCOUNT_DB_H

#include "ElleSQLConn.h"

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

class ElleGameAccountPool {
public:
    static ElleGameAccountPool& Instance();

    bool Initialize(const std::string& connectionString,
                    uint32_t poolSize = 4);
    void Shutdown();

    bool IsAvailable() const { return m_initialized; }

    SQLResultSet Query(const std::string& sql);
    SQLResultSet QueryParams(const std::string& sql,
                             const std::vector<std::string>& params);

private:
    ElleGameAccountPool() = default;
    ~ElleGameAccountPool() = default;
    ElleGameAccountPool(const ElleGameAccountPool&) = delete;
    ElleGameAccountPool& operator=(const ElleGameAccountPool&) = delete;

    std::string  m_connStr;
    uint32_t     m_poolSize = 0;
    bool         m_initialized = false;

    std::queue<std::shared_ptr<SQLConnection>> m_available;
    std::mutex                                  m_mutex;
    std::condition_variable                     m_cv;
};

namespace ElleGameAuth {

struct UserIdentity {
    int64_t     nUserNo = 0;
    std::string sUserID;
    std::string sUserName;
    bool        bIsBlock  = false;
    bool        bIsDelete = false;
    int32_t     nAuthID   = 0;
    char        QX        = 'A';
};

bool AuthenticateUser(const std::string& sUserID,
                      const std::string& sUserPW,
                      UserIdentity& out);

bool GetUserById(int64_t nUserNo, UserIdentity& out);

}

#endif
