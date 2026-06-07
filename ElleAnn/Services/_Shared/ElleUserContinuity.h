#pragma once
#ifndef ELLE_USER_CONTINUITY_H
#define ELLE_USER_CONTINUITY_H

#include <cstdint>
#include <string>

namespace ElleDB {

struct UserContinuityRow {
    int64_t     nUserNo            = 0;
    std::string sUserID_cached;
    std::string sUserName_cached;
    uint64_t    first_met_ms       = 0;
    uint64_t    last_seen_ms       = 0;
    int32_t     total_conversations = 0;
    int32_t     total_messages     = 0;
    int32_t     total_pairings     = 0;
    double      last_bond_score    = 0.0;
    std::string last_bond_label;
    uint64_t    last_bond_updated_ms = 0;
    std::string private_note;
    uint64_t    created_ms         = 0;
    uint64_t    updated_ms         = 0;
};

bool TouchUserContinuityOnPair(int64_t nUserNo,
                               const std::string& sUserID,
                               const std::string& sUserName);

bool UpdateUserBond(int64_t nUserNo,
                    double bondScore,
                    const std::string& bondLabel);

bool AppendUserNote(int64_t nUserNo, const std::string& note);

bool GetUserContinuity(int64_t nUserNo, UserContinuityRow& out);

struct GameSessionStateRow {
    int64_t     nUserNo                 = 0;
    int32_t     char_index              = -1;
    std::string char_name;
    int32_t     zone_id                 = -1;
    std::string zone_name;
    double      last_x                  = 0.0;
    double      last_y                  = 0.0;
    double      last_z                  = 0.0;
    int32_t     last_hp                 = 0;
    int32_t     last_hp_max             = 0;
    uint64_t    last_session_ms         = 0;
    uint64_t    last_disconnect_ms      = 0;
    std::string last_disconnect_reason;
};

bool UpsertGameSession(const GameSessionStateRow& row);

bool MarkGameSessionDisconnected(int64_t nUserNo,
                                 const std::string& reason);

bool GetGameSession(int64_t nUserNo, GameSessionStateRow& out);

}

#endif
