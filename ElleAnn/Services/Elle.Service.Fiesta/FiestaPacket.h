#pragma once
#ifndef ELLE_FIESTA_PACKET_H
#define ELLE_FIESTA_PACKET_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "FiestaProtoBase.h"

namespace Fiesta {

namespace Op {

    constexpr uint16_t NC_MISC_HEARTBEAT_REQ                     = 0x0204;
    constexpr uint16_t NC_MISC_HEARTBEAT_ACK                     = 0x0205;
    constexpr uint16_t NC_MISC_SEED_REQ                          = 0x0206;
    constexpr uint16_t NC_MISC_SEED_ACK                          = 0x0207;
    constexpr uint16_t NC_MISC_GAMETIME_REQ                      = 0x0212;

    constexpr uint16_t NC_USER_XTRAP_REQ                         = 0x0304;
    constexpr uint16_t NC_USER_XTRAP_ACK                         = 0x0305;
    constexpr uint16_t NC_USER_LOGIN_REQ                         = 0x0306;
    constexpr uint16_t NC_USER_PASSWORD_CHECK_REQ                = 0x0307;
    constexpr uint16_t NC_USER_PASSWORD_CHECK_ACK                = 0x0308;
    constexpr uint16_t NC_USER_LOGINFAIL_ACK                     = 0x0309;
    constexpr uint16_t NC_USER_LOGIN_ACK                         = 0x030A;
    constexpr uint16_t NC_USER_WORLDSELECT_REQ                   = 0x030B;
    constexpr uint16_t NC_USER_WORLDSELECT_ACK                   = 0x030C;
    constexpr uint16_t NC_USER_WILLLOGIN_REQ                     = 0x030D;
    constexpr uint16_t NC_USER_WILLLOGIN_ACK                     = 0x030E;
    constexpr uint16_t NC_USER_LOGINWORLD_REQ                    = 0x030F;
    constexpr uint16_t NC_USER_NORMALLOGOUT_CMD                  = 0x0318;
    constexpr uint16_t NC_USER_USE_BEAUTY_SHOP_CMD               = 0x0332;
    constexpr uint16_t NC_USER_CLIENT_VERSION_CHECK_REQ          = 0x0365;
    constexpr uint16_t NC_USER_CLIENT_WRONGVERSION_CHECK_ACK     = 0x0366;
    constexpr uint16_t NC_USER_CLIENT_RIGHTVERSION_CHECK_ACK     = 0x0367;

    constexpr uint16_t NC_LOG_MAP_NOBASE_CMD                     = 0x0384;

    constexpr uint16_t NC_CHAR_LOGIN_ACK                         = 0x0403;
    constexpr uint16_t NC_CHAR_BASE_CMD                          = 0x0407;
    constexpr uint16_t NC_CHAR_CLIENT_BASE_CMD                   = 0x0438;
    constexpr uint16_t NC_CHAR_REVIVE_REQ                        = 0x044E;
    constexpr uint16_t NC_CHAR_STAT_INCPOINT_REQ                 = 0x045C;
    constexpr uint16_t NC_CHAR_STAT_DECPOINT_REQ                 = 0x0462;
    constexpr uint16_t NC_CHAR_LOGOUTREADY_CMD                   = 0x0471;
    constexpr uint16_t NC_CHAR_LOGOUTCANCEL_CMD                  = 0x0472;
    constexpr uint16_t NC_CHAR_WEDDING_PARTNER_INFO_REQ          = 0x0493;
    constexpr uint16_t NC_CHAR_DEPOLYMORPH_CMD                   = 0x04B0;
    constexpr uint16_t NC_CHAR_SAVE_LINK_REQ                     = 0x04B9;
    constexpr uint16_t NC_CHAR_CLIENT_AUTO_PICK_REQ              = 0x04BC;
    constexpr uint16_t NC_CHAR_CLIENT_FREESTAT_APPLICATION_REQ   = 0x04C2;

    constexpr uint16_t NC_MAP_LOGIN_REQ                          = 0x0601;
    constexpr uint16_t NC_MAP_LOGINCOMPLETE_CMD                  = 0x0603;
    constexpr uint16_t NC_MAP_WING_SAVE_REQ                      = 0x0614;
    constexpr uint16_t NC_MAP_WING_FLY_REQ                       = 0x0616;
    constexpr uint16_t NC_MAP_TOWNPORTAL_REQ                     = 0x061A;
    constexpr uint16_t NC_MAP_LINEUP_PARTYPLAYER_REQ             = 0x061C;
    constexpr uint16_t NC_MAP_LINEUP_GUILDPLAYER_REQ             = 0x061F;

    constexpr uint16_t NC_BRIEFINFO_INFORM_CMD                   = 0x0701;
    constexpr uint16_t NC_BRIEFINFO_LOGINCHARACTER_CMD           = 0x0706;
    constexpr uint16_t NC_BRIEFINFO_CHARACTER_CMD                = 0x0707;
    constexpr uint16_t NC_BRIEFINFO_REGENMOB_CMD                 = 0x0708;
    constexpr uint16_t NC_BRIEFINFO_BRIEFINFODELETE_CMD          = 0x070E;
    constexpr uint16_t NC_BRIEFINFO_NPC_DISAPPEAR_CMD            = 0x0715;
    constexpr uint16_t NC_BRIEFINFO_PLAYER_LIST_INFO_APPEAR_CMD  = 0x0716;

    constexpr uint16_t NC_ACT_CHAT_REQ                           = 0x0801;
    constexpr uint16_t NC_ACT_WALK_REQ                           = 0x0803;
    constexpr uint16_t NC_ACT_RUN_REQ                            = 0x0805;
    constexpr uint16_t NC_ACT_CHANGEMODE_REQ                     = 0x0808;
    constexpr uint16_t NC_ACT_NPCCLICK_CMD                       = 0x080A;
    constexpr uint16_t NC_ACT_ENDOFTRADE_CMD                     = 0x080B;
    constexpr uint16_t NC_ACT_WHISPER_REQ                        = 0x080C;
    constexpr uint16_t NC_ACT_STOP_REQ                           = 0x0812;
    constexpr uint16_t NC_ACT_MOVEWALK_CMD                       = 0x0817;
    constexpr uint16_t NC_ACT_MOVERUN_CMD                        = 0x0819;
    constexpr uint16_t NC_ACT_NPCMENUOPEN_ACK                    = 0x081D;
    constexpr uint16_t NC_ACT_SHOUT_CMD                          = 0x081E;
    constexpr uint16_t NC_ACT_EMOTICON_CMD                       = 0x0820;
    constexpr uint16_t NC_ACT_EMOTICONSTOP_CMD                   = 0x0822;
    constexpr uint16_t NC_ACT_JUMP_CMD                           = 0x0824;
    constexpr uint16_t NC_ACT_PITCHTENT_REQ                      = 0x0827;
    constexpr uint16_t NC_ACT_FOLDTENT_REQ                       = 0x082A;
    constexpr uint16_t NC_ACT_GATHERSTART_REQ                    = 0x082D;
    constexpr uint16_t NC_ACT_GATHERCANCEL_CMD                   = 0x0830;
    constexpr uint16_t NC_ACT_GATHERCOMPLETE_REQ                 = 0x0832;
    constexpr uint16_t NC_ACT_PRODUCE_CAST_REQ                   = 0x0835;
    constexpr uint16_t NC_ACT_PRODUCE_CASTABORT_CMD              = 0x0839;
    constexpr uint16_t NC_ACT_RIDE_FEEDING_REQ                   = 0x0844;
    constexpr uint16_t NC_ACT_ROAR_REQ                           = 0x084B;
    constexpr uint16_t NC_ACT_RIDE_OFF_REQ                       = 0x086D;
    constexpr uint16_t NC_ACT_AUTO_WAY_FINDING_USE_GATE_REQ      = 0x0870;

    constexpr uint16_t NC_BAT_TARGETING_REQ                      = 0x0901;
    constexpr uint16_t NC_BAT_HIT_REQ                            = 0x0903;
    constexpr uint16_t NC_BAT_UNTARGET_REQ                       = 0x0908;
    constexpr uint16_t NC_BAT_SMASH_REQ                          = 0x0912;
    constexpr uint16_t NC_BAT_SKILLCAST_REQ                      = 0x0918;
    constexpr uint16_t NC_BAT_SKILLCASTABORT_CMD                 = 0x091C;
    constexpr uint16_t NC_BAT_BASHSTART_CMD                      = 0x092B;
    constexpr uint16_t NC_BAT_BASHSTOP_CMD                       = 0x0932;
    constexpr uint16_t NC_BAT_ASSIST_REQ                         = 0x093E;
    constexpr uint16_t NC_BAT_SKILLBASH_OBJ_CAST_REQ             = 0x0940;
    constexpr uint16_t NC_BAT_SKILLBASH_FLD_CAST_REQ             = 0x0941;
    constexpr uint16_t NC_BAT_SKILLBASH_CASTABORT_REQ            = 0x0944;
    constexpr uint16_t NC_BAT_ABSTATE_ERASE_REQ                  = 0x0954;

    constexpr uint16_t NC_ITEM_BUY_REQ                           = 0x0C03;
    constexpr uint16_t NC_ITEM_SELL_REQ                          = 0x0C06;
    constexpr uint16_t NC_ITEM_DROP_REQ                          = 0x0C07;
    constexpr uint16_t NC_ITEM_PICKUP_REQ                        = 0x0C09;
    constexpr uint16_t NC_ITEM_RELOC_REQ                         = 0x0C0B;
    constexpr uint16_t NC_ITEM_USE_REQ                           = 0x0C0F;
    constexpr uint16_t NC_ITEM_QUICKSLOT_RELOC_REQ               = 0x0C42;
    constexpr uint16_t NC_ITEM_REVIVEITEMUSE_CMD                 = 0x0C48;

    constexpr uint16_t NC_PARTY_REQUEST_REQ                      = 0x0E48;
    constexpr uint16_t NC_MENU_SERVERMENU_ACK                    = 0x0F02;
    constexpr uint16_t NC_QUEST_DOING_REQ                        = 0x1102;
    constexpr uint16_t NC_QUEST_REWARD_REQ                       = 0x1107;
    constexpr uint16_t NC_QUEST_GIVEUP_REQ                       = 0x110B;
    constexpr uint16_t NC_SKILL_LEARN_REQ                        = 0x1211;
    constexpr uint16_t NC_SKILL_REPLYREVIVE_CMD                  = 0x122A;
    constexpr uint16_t NC_TRADE_PROPOSE_REQ                      = 0x1301;
    constexpr uint16_t NC_TRADE_PROPOSEYES_ACK                   = 0x1306;
    constexpr uint16_t NC_TRADE_PROPOSE_ASKNO_ACK                = 0x1303;
    constexpr uint16_t NC_TRADE_REQADDITEM_REQ                   = 0x1307;
    constexpr uint16_t NC_TRADE_DECISION_REQ                     = 0x130A;
    constexpr uint16_t NC_TRADE_OK_REQ                           = 0x130D;
    constexpr uint16_t NC_TRADE_CANCEL_REQ                       = 0x1311;
    constexpr uint16_t NC_TRADE_REQADDMONEY_REQ                  = 0x1315;
    constexpr uint16_t NC_TRADE_FINAL_OK_REQ                     = 0x1319;
    constexpr uint16_t NC_TRADE_PARTNER_NOREADY_ACK              = 0x131F;
    constexpr uint16_t NC_SOULSTONE_HP_BUY_REQ                   = 0x1401;
    constexpr uint16_t NC_SOULSTONE_SP_BUY_REQ                   = 0x1402;
    constexpr uint16_t NC_SOULSTONE_USE_REQ                      = 0x1407;
    constexpr uint16_t NC_SOULSTONE_REVIVE_REQ                   = 0x1409;
    constexpr uint16_t NC_KQ_REWARD_REQ                          = 0x161A;
    constexpr uint16_t NC_WT_LICENSE_REQ                         = 0x1701;
    constexpr uint16_t NC_CT_SET_CURRENT_REQ                     = 0x1801;
    constexpr uint16_t NC_BOOTH_OPEN_REQ                         = 0x1A01;
    constexpr uint16_t NC_BOOTH_CLOSE_REQ                        = 0x1A04;
    constexpr uint16_t NC_BOOTH_ENTRY_REQ                        = 0x1A07;
    constexpr uint16_t NC_BOOTH_REFRESH_REQ                      = 0x1A0A;
    constexpr uint16_t NC_BOOTH_ITEMTRADE_REQ                    = 0x1A0D;
    constexpr uint16_t NC_BOOTH_INTERIORSTART_REQ                = 0x1A10;
    constexpr uint16_t NC_INSTANCE_DUNGEON_DELETE_DUNGEON_REQ    = 0x2905;
    constexpr uint16_t NC_DICE_TAISAI_GAME_JOIN_REQ              = 0x2B01;
    constexpr uint16_t NC_DICE_TAISAI_GAME_LEAVE_REQ             = 0x2B05;
    constexpr uint16_t NC_DICE_TAISAI_BETTING_REQ                = 0x2B0B;
    constexpr uint16_t NC_DICE_TAISAI_BETTING_CANCEL_REQ         = 0x2B0F;
    constexpr uint16_t NC_GAMBLE_GAMBLEHOUSE_EXIT_REQ            = 0x2F02;

}

#pragma pack(push, 1)

struct PROTO_NC_MISC_SEED_ACK {
    uint16_t seed;
};

struct PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ {
    uint16_t sVersionKey[32];
};

struct PROTO_NC_USER_LOGIN_REQ {
    char user[256];
    char password[16];
};

struct PROTO_NC_USER_WORLDSELECT_REQ {
    uint8_t worldno;
};

struct PROTO_NC_USER_WORLDSELECT_ACK {
    uint8_t  worldstatus;
    char     ip[16];
    uint16_t port;
    uint16_t validate_new[32];
};

struct PROTO_NC_USER_WILLLOGIN_REQ {
    uint8_t  netpacketheader[7];
    char     userid[256];
    uint16_t validate_new[32];
    char     spawnapps[20];
};

struct PROTO_NC_USER_WILLLOGIN_ACK {
    uint8_t  status;
    uint8_t  netpacketheader[7];
    char     userid[256];
    uint16_t validate_new[32];
    char     zone_ip[16];
    uint16_t zone_port;
    uint8_t  pad[2];
};

struct PROTO_NC_ACT_WALK_REQ {
    SHINE_XY_TYPE from;
    SHINE_XY_TYPE to;
};

struct PROTO_NC_ACT_NPCCLICK_CMD {
    uint16_t npchandle;
};

struct PROTO_NC_ACT_CHAT_REQ_HEAD {
    uint8_t itemLinkDataCount;
    uint8_t len;

};
static_assert(sizeof(PROTO_NC_ACT_CHAT_REQ_HEAD) == 2,
              "CHAT_REQ head wire-size mismatch");

struct PROTO_NC_ACT_WHISPER_REQ_HEAD {
    char    handle[16];
    uint8_t itemLinkDataCount;
    uint8_t len;
};
static_assert(sizeof(PROTO_NC_ACT_WHISPER_REQ_HEAD) == 18,
              "WHISPER_REQ head wire-size mismatch");

struct PROTO_NC_BAT_TARGETING_REQ {
    uint16_t target_handle;
};

struct PROTO_NC_BAT_HIT_REQ {
    uint16_t target_handle;
};

struct PROTO_NC_BAT_SKILLCAST_REQ {
    uint16_t skill_id;
    uint16_t target_handle;
};
static_assert(sizeof(PROTO_NC_BAT_SKILLCAST_REQ) == 4,
              "SKILLCAST_REQ wire-size mismatch");

struct PROTO_NC_ACT_EMOTICON_CMD {
    uint16_t emote_id;
};

struct PROTO_NC_BRIEFINFO_REGENMOB_CMD_HEAD {
    uint16_t handle;
    uint16_t mob_id;
};

struct PROTO_NC_BRIEFINFO_NPC_DISAPPEAR_CMD {
    uint16_t handle;
};

struct PROTO_NC_BRIEFINFO_INFORM_CMD {
    uint16_t nMyHnd;
    uint16_t ReceiveNetCommand;
    uint16_t hnd;
};

struct PROTO_NC_BRIEFINFO_BRIEFINFODELETE_CMD {
    uint16_t hnd;
};

struct PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD {
    uint16_t handle;
    char     charid[16];

};
static_assert(sizeof(PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD) == 18,
              "LOGINCHARACTER head wire-size mismatch");

struct PROTO_NC_CHAR_BASE_CMD_HEAD {
    uint32_t chrregnum;
    char     charid[16];

};
static_assert(sizeof(PROTO_NC_CHAR_BASE_CMD_HEAD) == 20,
              "CHAR_BASE head wire-size mismatch");

#pragma pack(pop)

static_assert(sizeof(SHINE_XY_TYPE)                         ==   8, "SHINE_XY_TYPE wire-size mismatch");
static_assert(sizeof(SHINE_COORD_TYPE)                      ==   9, "SHINE_COORD_TYPE wire-size mismatch");
static_assert(sizeof(PROTO_NC_MISC_SEED_ACK)                ==   2, "SEED_ACK wire-size mismatch");
static_assert(sizeof(PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ)==  64, "VERSION_CHECK_REQ wire-size mismatch");
static_assert(sizeof(PROTO_NC_USER_LOGIN_REQ)               == 272, "LOGIN_REQ wire-size mismatch");
static_assert(sizeof(PROTO_NC_USER_WORLDSELECT_REQ)         ==   1, "WORLDSELECT_REQ wire-size mismatch");
static_assert(sizeof(PROTO_NC_USER_WORLDSELECT_ACK)         ==  83, "WORLDSELECT_ACK wire-size mismatch");
static_assert(sizeof(PROTO_NC_USER_WILLLOGIN_REQ)           == 347, "WILLLOGIN_REQ wire-size mismatch");
static_assert(sizeof(PROTO_NC_ACT_WALK_REQ)                 ==  16, "WALK_REQ wire-size mismatch");
static_assert(sizeof(PROTO_NC_ACT_NPCCLICK_CMD)             ==   2, "NPCCLICK_CMD wire-size mismatch");
static_assert(sizeof(PROTO_NC_BRIEFINFO_INFORM_CMD)         ==   6, "BRIEFINFO_INFORM_CMD wire-size mismatch");
static_assert(sizeof(NETPACKETZONEHEADER)                   ==   6, "NETPACKETZONEHEADER wire-size mismatch");
static_assert(sizeof(PROTO_NC_BAT_TARGETING_REQ)            ==   2, "BAT_TARGETING wire-size mismatch");
static_assert(sizeof(PROTO_NC_BAT_HIT_REQ)                  ==   2, "BAT_HIT wire-size mismatch");
static_assert(sizeof(PROTO_NC_ACT_EMOTICON_CMD)             ==   2, "EMOTICON wire-size mismatch");
static_assert(sizeof(PROTO_NC_BRIEFINFO_REGENMOB_CMD_HEAD)  ==   4, "REGENMOB head wire-size mismatch");
static_assert(sizeof(PROTO_NC_BRIEFINFO_NPC_DISAPPEAR_CMD)  ==   2, "NPC_DISAPPEAR wire-size mismatch");

class Writer {
public:
    void U8(uint8_t v)   { m_buf.push_back(v); }
    void U16(uint16_t v) { m_buf.push_back((uint8_t)(v & 0xFF)); m_buf.push_back((uint8_t)(v >> 8)); }

    void Handle(uint16_t h) { U16(h); }
    void U32(uint32_t v) {
        for (int i = 0; i < 4; i++) m_buf.push_back((uint8_t)((v >> (i * 8)) & 0xFF));
    }
    void Bytes(const void* p, size_t n) {
        const auto* b = static_cast<const uint8_t*>(p);
        m_buf.insert(m_buf.end(), b, b + n);
    }

    void FixedStr(const std::string& s, size_t width) {
        const size_t copy = (s.size() < width) ? s.size() : width;
        m_buf.insert(m_buf.end(), s.begin(), s.begin() + copy);
        for (size_t i = copy; i < width; i++) m_buf.push_back(0);
    }

    void Str8(const std::string& s) {
        const size_t n = (s.size() > 0xFF) ? 0xFF : s.size();
        m_buf.push_back((uint8_t)n);
        m_buf.insert(m_buf.end(), s.begin(), s.begin() + n);
    }
    const std::vector<uint8_t>& Data() const { return m_buf; }
    std::vector<uint8_t>& Mutable() { return m_buf; }

private:
    std::vector<uint8_t> m_buf;
};

class Reader {
public:
    Reader(const uint8_t* data, size_t len) : m_p(data), m_end(data + len) {}
    bool U8(uint8_t& out) {
        if (m_p + 1 > m_end) return false;
        out = *m_p++; return true;
    }
    bool U16(uint16_t& out) {
        if (m_p + 2 > m_end) return false;
        out = (uint16_t)(m_p[0] | (m_p[1] << 8)); m_p += 2; return true;
    }
    bool U32(uint32_t& out) {
        if (m_p + 4 > m_end) return false;
        out = (uint32_t)(m_p[0] | (m_p[1] << 8) | (m_p[2] << 16) | (m_p[3] << 24));
        m_p += 4; return true;
    }
    bool Bytes(void* dst, size_t n) {
        if (m_p + n > m_end) return false;
        std::memcpy(dst, m_p, n); m_p += n; return true;
    }

    bool FixedStr(std::string& out, size_t width) {
        if (m_p + width > m_end) return false;
        size_t end = width;
        while (end > 0 && m_p[end - 1] == 0) end--;
        out.assign((const char*)m_p, end);
        m_p += width;
        return true;
    }

    bool Str8(std::string& out) {
        uint8_t n; if (!U8(n)) return false;
        if (m_p + n > m_end) return false;
        out.assign((const char*)m_p, n); m_p += n; return true;
    }
    size_t Remaining() const { return (size_t)(m_end - m_p); }
    const uint8_t* Cursor() const { return m_p; }

private:
    const uint8_t* m_p;
    const uint8_t* m_end;
};

inline std::vector<uint8_t> BuildPacket(uint16_t opcode,
                                         const std::vector<uint8_t>& payload) {
    const size_t bodyLen = 2 + payload.size();

    std::vector<uint8_t> out;
    out.reserve(3 + bodyLen);
    if (bodyLen < 0xFF) {
        out.push_back((uint8_t)bodyLen);
    } else {
        out.push_back(0xFF);
        out.push_back((uint8_t)(bodyLen & 0xFF));
        out.push_back((uint8_t)((bodyLen >> 8) & 0xFF));
    }

    out.push_back((uint8_t)((opcode >> 8) & 0xFF));
    out.push_back((uint8_t)(opcode & 0xFF));
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

template <typename T>
inline std::vector<uint8_t> ToBytes(const T& proto) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "ToBytes only works with trivially-copyable PODs");
    std::vector<uint8_t> out(sizeof(T));
    std::memcpy(out.data(), &proto, sizeof(T));
    return out;
}

}
#endif
