#pragma once
#ifndef ELLE_FIESTA_PROTO_BASE_H
#define ELLE_FIESTA_PROTO_BASE_H

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace Fiesta {

#pragma pack(push, 1)

typedef char Name3[4];
typedef char Name4[16];
typedef char Name5[20];
typedef char Name8[8];
typedef char Name256Byte[256];

static_assert(sizeof(Name3)       ==   4, "Name3 must be 4 bytes");
static_assert(sizeof(Name4)       ==  16, "Name4 must be 16 bytes");
static_assert(sizeof(Name5)       ==  20, "Name5 must be 20 bytes");
static_assert(sizeof(Name8)       ==   8, "Name8 must be 8 bytes");
static_assert(sizeof(Name256Byte) == 256, "Name256Byte must be 256 bytes");

typedef uint16_t SHINE_HANDLE_NUMBER;

struct NETCOMMAND {
    uint8_t dept;
    uint8_t cmdid;
};
static_assert(sizeof(NETCOMMAND) == 2, "NETCOMMAND must be exactly 2 wire bytes");

struct NETPACKETHEADER {
    uint16_t clienthandle;
    uint8_t  tail[5];
};
static_assert(sizeof(NETPACKETHEADER) == 7,
              "NETPACKETHEADER wire size must be 7 (2 named + 5 trailing)");

struct NETPACKETHEADER_NAMED {
    uint16_t clienthandle;
};
static_assert(sizeof(NETPACKETHEADER_NAMED) == 2,
              "NETPACKETHEADER_NAMED wire size must be 2");

struct NETPACKETZONEHEADER {
    uint16_t clienthandle;
    uint32_t charregistnumber;
};
static_assert(sizeof(NETPACKETZONEHEADER) == 6,
              "NETPACKETZONEHEADER wire size must be 6");

struct SHINE_XY_TYPE {
    uint32_t x;
    uint32_t y;
};
static_assert(sizeof(SHINE_XY_TYPE) == 8, "SHINE_XY_TYPE wire-size mismatch");

struct SHINE_COORD_TYPE {
    SHINE_XY_TYPE xy;
    uint8_t       dir;
};
static_assert(sizeof(SHINE_COORD_TYPE) == 9, "SHINE_COORD_TYPE wire-size mismatch");

#pragma pack(pop)

constexpr uint16_t MAKE_NETCMDID(uint8_t dept, uint8_t subid) {
    return static_cast<uint16_t>((static_cast<uint16_t>(dept) << 8) |
                                  static_cast<uint16_t>(subid));
}

constexpr uint8_t OPCODE_DEPT(uint16_t netcmdid) {
    return static_cast<uint8_t>((netcmdid >> 8) & 0xFF);
}

constexpr uint8_t OPCODE_SUBID(uint16_t netcmdid) {
    return static_cast<uint8_t>(netcmdid & 0xFF);
}

static_assert(MAKE_NETCMDID(0x02, 0x07) == 0x0207,
              "MAKE_NETCMDID(NC_MISC, SEED_ACK) must equal 0x0207");
static_assert(OPCODE_DEPT (0x0207) == 0x02,
              "OPCODE_DEPT(0x0207) must equal 0x02 (NC_MISC)");
static_assert(OPCODE_SUBID(0x0207) == 0x07,
              "OPCODE_SUBID(0x0207) must equal 0x07");

namespace Dept {
    constexpr uint8_t NC_NULL              = 0x00;
    constexpr uint8_t NC_LOG               = 0x01;
    constexpr uint8_t NC_MISC              = 0x02;
    constexpr uint8_t NC_USER              = 0x03;
    constexpr uint8_t NC_CHAR              = 0x04;
    constexpr uint8_t NC_AVATAR            = 0x05;
    constexpr uint8_t NC_MAP               = 0x06;
    constexpr uint8_t NC_BRIEFINFO         = 0x07;
    constexpr uint8_t NC_ACT               = 0x08;
    constexpr uint8_t NC_BAT               = 0x09;
    constexpr uint8_t NC_OPTOOL            = 0x0A;
    constexpr uint8_t NC_ITEM              = 0x0C;
    constexpr uint8_t NC_PARTY             = 0x0E;
    constexpr uint8_t NC_MENU              = 0x0F;
    constexpr uint8_t NC_CHARSAVE          = 0x10;
    constexpr uint8_t NC_QUEST             = 0x11;
    constexpr uint8_t NC_SKILL             = 0x12;
    constexpr uint8_t NC_TRADE             = 0x13;
    constexpr uint8_t NC_SOULSTONE         = 0x14;
    constexpr uint8_t NC_KQ                = 0x16;
    constexpr uint8_t NC_WT                = 0x17;
    constexpr uint8_t NC_CT                = 0x18;
    constexpr uint8_t NC_BOOTH             = 0x1A;
    constexpr uint8_t NC_SCENARIO          = 0x1B;
    constexpr uint8_t NC_GUILD             = 0x1D;
    constexpr uint8_t NC_MINIHOUSE         = 0x23;
    constexpr uint8_t NC_CHARGED           = 0x24;
    constexpr uint8_t NC_HOLY              = 0x25;
    constexpr uint8_t NC_GUILD_ACADEMY     = 0x26;
    constexpr uint8_t NC_INSTANCE          = 0x29;
    constexpr uint8_t NC_DICE              = 0x2B;
    constexpr uint8_t NC_USER_CONNECTION   = 0x2D;
    constexpr uint8_t NC_AUCTION           = 0x2E;
    constexpr uint8_t NC_GAMBLE            = 0x2F;
    constexpr uint8_t NC_PET               = 0x35;
}

enum class Direction : uint8_t {
    Unknown  = 0,
    C2S      = 1,
    S2C      = 2,
    Bidir    = 3,
};

enum class Decoder : uint8_t {
    Fixed       = 0,
    HeadAndTail = 1,
    Opaque      = 2,
    Unknown     = 3,
};

struct OpcodeMeta {
    uint16_t          opcode;
    const char*       opcode_name;
    const char*       struct_name;
    int32_t           pdb_sizeof;
    const char*       category;
};

}
#endif
