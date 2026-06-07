#pragma once
#include <cstdint>
#include <cstddef>

namespace ElleFiesta::Proto {

#pragma pack(push, 1)

struct NETPACKET {
    NETCOMMAND netcmd;
    uint8_t _pad_at_0000[2];
    uint8_t data[0];
};
static_assert(sizeof(NETPACKET) == 2, "NETPACKET size drift");

struct NETPACKETHEADER { uint8_t data[2]; };
static_assert(sizeof(NETPACKETHEADER) == 2, "NETPACKETHEADER size drift");

struct NETPACKETZONEHEADER { uint8_t data[6]; };
static_assert(sizeof(NETPACKETZONEHEADER) == 6, "NETPACKETZONEHEADER size drift");

struct PROTO_NC_ACT_ACTIONBYITEM_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ACT_ACTIONBYITEM_ACK) == 3, "PROTO_NC_ACT_ACTIONBYITEM_ACK size drift");

struct PROTO_NC_ACT_ACTIONBYITEM_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_ACTIONBYITEM_REQ) == 1, "PROTO_NC_ACT_ACTIONBYITEM_REQ size drift");

struct PROTO_NC_ACT_ANIMATION_LEVEL_CHANGE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ACT_ANIMATION_LEVEL_CHANGE_CMD) == 3, "PROTO_NC_ACT_ANIMATION_LEVEL_CHANGE_CMD size drift");

struct PROTO_NC_ACT_ANIMATION_START_CMD {
    uint8_t _pad_at_0000[2];
    wchar_t sAnimationIndex[32];
};
static_assert(sizeof(PROTO_NC_ACT_ANIMATION_START_CMD) == 34, "PROTO_NC_ACT_ANIMATION_START_CMD size drift");

struct PROTO_NC_ACT_ANIMATION_STOP_CMD {
    uint8_t _pad_at_0000[2];
    wchar_t sAnimationIndex[32];
};
static_assert(sizeof(PROTO_NC_ACT_ANIMATION_STOP_CMD) == 34, "PROTO_NC_ACT_ANIMATION_STOP_CMD size drift");

struct PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_ACK) == 2, "PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_ACK size drift");

struct PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_REQ) == 1, "PROTO_NC_ACT_AUTO_WAY_FINDING_USE_GATE_REQ size drift");

struct PROTO_NC_ACT_CHANGEMODE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_CHANGEMODE_REQ) == 1, "PROTO_NC_ACT_CHANGEMODE_REQ size drift");

struct PROTO_NC_ACT_CHAT_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_CHAT_REQ) == 2, "PROTO_NC_ACT_CHAT_REQ size drift");

struct PROTO_NC_ACT_CREATECASTBAR { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_CREATECASTBAR) == 2, "PROTO_NC_ACT_CREATECASTBAR size drift");

struct PROTO_NC_ACT_EFFECT_MESSAGE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_EFFECT_MESSAGE_CMD) == 4, "PROTO_NC_ACT_EFFECT_MESSAGE_CMD size drift");

struct PROTO_NC_ACT_EMOTICON_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_EMOTICON_CMD) == 1, "PROTO_NC_ACT_EMOTICON_CMD size drift");

struct PROTO_NC_ACT_EVENT_CODE_ACTION_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ACT_EVENT_CODE_ACTION_CMD) == 6, "PROTO_NC_ACT_EVENT_CODE_ACTION_CMD size drift");

struct PROTO_NC_ACT_FOLDTENT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_FOLDTENT_ACK) == 2, "PROTO_NC_ACT_FOLDTENT_ACK size drift");

struct PROTO_NC_ACT_GATHERCOMPLETE_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_GATHERCOMPLETE_ACK) == 4, "PROTO_NC_ACT_GATHERCOMPLETE_ACK size drift");

struct PROTO_NC_ACT_GATHERSTART_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ACT_GATHERSTART_ACK) == 6, "PROTO_NC_ACT_GATHERSTART_ACK size drift");

struct PROTO_NC_ACT_GATHERSTART_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_GATHERSTART_REQ) == 2, "PROTO_NC_ACT_GATHERSTART_REQ size drift");

struct PROTO_NC_ACT_MOVEFAIL_ACK {
    SHINE_XY_TYPE back;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_MOVEFAIL_ACK) == 8, "PROTO_NC_ACT_MOVEFAIL_ACK size drift");

struct PROTO_NC_ACT_MOVEFAIL_CMD {
    SHINE_XY_TYPE back;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_MOVEFAIL_CMD) == 8, "PROTO_NC_ACT_MOVEFAIL_CMD size drift");

struct PROTO_NC_ACT_MOVESPEED_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_MOVESPEED_CMD) == 4, "PROTO_NC_ACT_MOVESPEED_CMD size drift");

struct PROTO_NC_ACT_MOVEWALK_CMD {
    SHINE_XY_TYPE from;
    uint8_t _pad_at_0000[8];
    SHINE_XY_TYPE to;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_MOVEWALK_CMD) == 16, "PROTO_NC_ACT_MOVEWALK_CMD size drift");

struct PROTO_NC_ACT_NOTICE_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ACT_CHAT_REQ cmd;
};
static_assert(sizeof(PROTO_NC_ACT_NOTICE_CMD_SEND) == 5, "PROTO_NC_ACT_NOTICE_CMD_SEND size drift");

struct PROTO_NC_ACT_NPCCLICK_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_NPCCLICK_CMD) == 2, "PROTO_NC_ACT_NPCCLICK_CMD size drift");

struct PROTO_NC_ACT_NPCMENUOPEN_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_NPCMENUOPEN_ACK) == 1, "PROTO_NC_ACT_NPCMENUOPEN_ACK size drift");

struct PROTO_NC_ACT_NPCMENUOPEN_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_NPCMENUOPEN_REQ) == 2, "PROTO_NC_ACT_NPCMENUOPEN_REQ size drift");

struct PROTO_NC_ACT_NPC_ACTION_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_ACT_NPC_ACTION_CMD) == 7, "PROTO_NC_ACT_NPC_ACTION_CMD size drift");

struct PROTO_NC_ACT_NPC_MENU_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_NPC_MENU_CMD) == 1, "PROTO_NC_ACT_NPC_MENU_CMD size drift");

struct PROTO_NC_ACT_OBJECT_EFFECT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ACT_OBJECT_EFFECT_CMD) == 6, "PROTO_NC_ACT_OBJECT_EFFECT_CMD size drift");

struct PROTO_NC_ACT_OBJECT_SOUND_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t sSoundFileName[32];
};
static_assert(sizeof(PROTO_NC_ACT_OBJECT_SOUND_CMD) == 34, "PROTO_NC_ACT_OBJECT_SOUND_CMD size drift");

struct PROTO_NC_ACT_PARTYCHAT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_PARTYCHAT_ACK) == 2, "PROTO_NC_ACT_PARTYCHAT_ACK size drift");

struct PROTO_NC_ACT_PARTYCHAT_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ACT_PARTYCHAT_ACK ack;
};
static_assert(sizeof(PROTO_NC_ACT_PARTYCHAT_ACK_SEND) == 5, "PROTO_NC_ACT_PARTYCHAT_ACK_SEND size drift");

struct PROTO_NC_ACT_PARTYCHAT_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_PARTYCHAT_REQ) == 2, "PROTO_NC_ACT_PARTYCHAT_REQ size drift");

struct PROTO_NC_ACT_PARTYCHAT_CMD {
    Name5 talker;
    uint8_t _pad_at_0000[20];
    PROTO_NC_ACT_PARTYCHAT_REQ chat;
};
static_assert(sizeof(PROTO_NC_ACT_PARTYCHAT_CMD) == 22, "PROTO_NC_ACT_PARTYCHAT_CMD size drift");

struct PROTO_NC_ACT_PITCHTENT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_PITCHTENT_ACK) == 2, "PROTO_NC_ACT_PITCHTENT_ACK size drift");

struct PROTO_NC_ACT_PLAY_SOUND_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t FileName[0];
};
static_assert(sizeof(PROTO_NC_ACT_PLAY_SOUND_CMD) == 2, "PROTO_NC_ACT_PLAY_SOUND_CMD size drift");

struct PROTO_NC_ACT_PRODUCE_CAST_FAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_PRODUCE_CAST_FAIL_ACK) == 2, "PROTO_NC_ACT_PRODUCE_CAST_FAIL_ACK size drift");

struct PROTO_NC_ACT_PRODUCE_CAST_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_PRODUCE_CAST_REQ) == 2, "PROTO_NC_ACT_PRODUCE_CAST_REQ size drift");

struct PROTO_NC_ACT_PRODUCE_MAKE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_PRODUCE_MAKE_CMD) == 4, "PROTO_NC_ACT_PRODUCE_MAKE_CMD size drift");

struct PROTO_NC_ACT_REINFORCEMOVEBYPATH_CMD {
    uint8_t _pad_at_0000[6];
    SHINE_XY_TYPE_______0_bytes___ path;
};
static_assert(sizeof(PROTO_NC_ACT_REINFORCEMOVEBYPATH_CMD) == 6, "PROTO_NC_ACT_REINFORCEMOVEBYPATH_CMD size drift");

struct PROTO_NC_ACT_REINFORCE_RUN_CMD {
    SHINE_XY_TYPE xy;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_REINFORCE_RUN_CMD) == 8, "PROTO_NC_ACT_REINFORCE_RUN_CMD size drift");

struct PROTO_NC_ACT_REINFORCE_STOP_CMD {
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_REINFORCE_STOP_CMD) == 8, "PROTO_NC_ACT_REINFORCE_STOP_CMD size drift");

struct PROTO_NC_ACT_RIDE_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_RIDE_FAIL_CMD) == 2, "PROTO_NC_ACT_RIDE_FAIL_CMD size drift");

struct PROTO_NC_ACT_RIDE_FEEDING_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_RIDE_FEEDING_ACK) == 2, "PROTO_NC_ACT_RIDE_FEEDING_ACK size drift");

struct PROTO_NC_ACT_RIDE_FEEDING_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ACT_RIDE_FEEDING_REQ) == 1, "PROTO_NC_ACT_RIDE_FEEDING_REQ size drift");

struct PROTO_NC_ACT_RIDE_HUNGRY_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_RIDE_HUNGRY_CMD) == 2, "PROTO_NC_ACT_RIDE_HUNGRY_CMD size drift");

struct PROTO_NC_ACT_RIDE_ON_CMD {
    CHARBRIEFINFO_RIDE__RideInfo ride;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ACT_RIDE_ON_CMD) == 2, "PROTO_NC_ACT_RIDE_ON_CMD size drift");

struct PROTO_NC_ACT_ROAR_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_ROAR_ACK) == 2, "PROTO_NC_ACT_ROAR_ACK size drift");

struct PROTO_NC_ACT_ROAR_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_ROAR_REQ) == 2, "PROTO_NC_ACT_ROAR_REQ size drift");

struct PROTO_NC_ACT_SCRIPT_MSG_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t sScriptMsgIndex[32];
    uint8_t nlen[5];
    uint8_t sContent[0];
};
static_assert(sizeof(PROTO_NC_ACT_SCRIPT_MSG_CMD) == 39, "PROTO_NC_ACT_SCRIPT_MSG_CMD size drift");

struct PROTO_NC_ACT_SCRIPT_MSG_WORLD_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t sScriptMsgIndex[32];
    uint8_t nlen[5];
    uint8_t sContent[0];
};
static_assert(sizeof(PROTO_NC_ACT_SCRIPT_MSG_WORLD_CMD) == 39, "PROTO_NC_ACT_SCRIPT_MSG_WORLD_CMD size drift");

struct PROTO_NC_ACT_SETITEMHEALEFFECT___unnamed_type_flag_ {
    uint32_t  isheal;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ACT_SETITEMHEALEFFECT___unnamed_type_flag_) == 1, "PROTO_NC_ACT_SETITEMHEALEFFECT___unnamed_type_flag_ size drift");

struct PROTO_NC_ACT_SETITEMHEALEFFECT {
    uint8_t _pad_at_0000[2];
    PROTO_NC_ACT_SETITEMHEALEFFECT___unnamed_type_flag_ flag;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ACT_SETITEMHEALEFFECT) == 5, "PROTO_NC_ACT_SETITEMHEALEFFECT size drift");

struct PROTO_NC_ACT_SHOUT_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_SHOUT_CMD) == 2, "PROTO_NC_ACT_SHOUT_CMD size drift");

struct PROTO_NC_ACT_SHOW_CINEMATIC_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t pText[0];
};
static_assert(sizeof(PROTO_NC_ACT_SHOW_CINEMATIC_CMD) == 2, "PROTO_NC_ACT_SHOW_CINEMATIC_CMD size drift");

struct PROTO_NC_ACT_SOMEEONEJUMP_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEEONEJUMP_CMD) == 2, "PROTO_NC_ACT_SOMEEONEJUMP_CMD size drift");

struct PROTO_NC_ACT_SOMEONECHANGEMODE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONECHANGEMODE_CMD) == 3, "PROTO_NC_ACT_SOMEONECHANGEMODE_CMD size drift");

struct PROTO_NC_ACT_SOMEONECHAT_CMD___unnamed_type_flag_ {
    uint32_t  GMColor;
    uint32_t  chatwin;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONECHAT_CMD___unnamed_type_flag_) == 1, "PROTO_NC_ACT_SOMEONECHAT_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_ACT_SOMEONECHAT_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_ACT_SOMEONECHAT_CMD___unnamed_type_flag_ flag;
    uint8_t _pad_at_0005[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONECHAT_CMD) == 7, "PROTO_NC_ACT_SOMEONECHAT_CMD size drift");

struct PROTO_NC_ACT_SOMEONEEMOTICONSTOP_CMD {
    uint8_t _pad_at_0000[2];
    STOPEMOTICON_DESCRIPT emoticon;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEEMOTICONSTOP_CMD) == 5, "PROTO_NC_ACT_SOMEONEEMOTICONSTOP_CMD size drift");

struct PROTO_NC_ACT_SOMEONEEMOTICON_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEEMOTICON_CMD) == 3, "PROTO_NC_ACT_SOMEONEEMOTICON_CMD size drift");

struct PROTO_NC_ACT_SOMEONEFOLDTENT_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_NOTCAMP shape;
    uint8_t _tail[43];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEFOLDTENT_CMD) == 45, "PROTO_NC_ACT_SOMEONEFOLDTENT_CMD size drift");

struct PROTO_NC_ACT_SOMEONEGATHERCANCEL_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEGATHERCANCEL_CMD) == 4, "PROTO_NC_ACT_SOMEONEGATHERCANCEL_CMD size drift");

struct PROTO_NC_ACT_SOMEONEGATHERCOMPLETE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEGATHERCOMPLETE_CMD) == 4, "PROTO_NC_ACT_SOMEONEGATHERCOMPLETE_CMD size drift");

struct PROTO_NC_ACT_SOMEONEGATHERSTART_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEGATHERSTART_CMD) == 6, "PROTO_NC_ACT_SOMEONEGATHERSTART_CMD size drift");

struct PROTO_NC_ACT_SOMEONEMOVEWALK_CMD___unnamed_type_moveattr_ {
    uint32_t  direct;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEMOVEWALK_CMD___unnamed_type_moveattr_) == 2, "PROTO_NC_ACT_SOMEONEMOVEWALK_CMD___unnamed_type_moveattr_ size drift");

struct PROTO_NC_ACT_SOMEONEMOVEWALK_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE from;
    uint8_t _pad_at_0002[8];
    SHINE_XY_TYPE to;
    uint8_t _pad_at_000a[10];
    PROTO_NC_ACT_SOMEONEMOVEWALK_CMD___unnamed_type_moveattr_ moveattr;
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEMOVEWALK_CMD) == 22, "PROTO_NC_ACT_SOMEONEMOVEWALK_CMD size drift");

struct PROTO_NC_ACT_SOMEONEPITCHTENT_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_CAMP camp;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEPITCHTENT_CMD) == 14, "PROTO_NC_ACT_SOMEONEPITCHTENT_CMD size drift");

struct PROTO_NC_ACT_SOMEONEPRODUCE_CASTCUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEPRODUCE_CASTCUT_CMD) == 2, "PROTO_NC_ACT_SOMEONEPRODUCE_CASTCUT_CMD size drift");

struct PROTO_NC_ACT_SOMEONEPRODUCE_CAST_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEPRODUCE_CAST_CMD) == 4, "PROTO_NC_ACT_SOMEONEPRODUCE_CAST_CMD size drift");

struct PROTO_NC_ACT_SOMEONEPRODUCE_MAKE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONEPRODUCE_MAKE_CMD) == 4, "PROTO_NC_ACT_SOMEONEPRODUCE_MAKE_CMD size drift");

struct PROTO_NC_ACT_SOMEONERIDE_OFF_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONERIDE_OFF_CMD) == 2, "PROTO_NC_ACT_SOMEONERIDE_OFF_CMD size drift");

struct PROTO_NC_ACT_SOMEONERIDE_ON_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_RIDE__RideInfo ride;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONERIDE_ON_CMD) == 4, "PROTO_NC_ACT_SOMEONERIDE_ON_CMD size drift");

struct PROTO_NC_ACT_SOMEONESHOUT_CMD___unnamed_type_flag_ {
    uint32_t  GMColor;
    uint32_t  isMob;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONESHOUT_CMD___unnamed_type_flag_) == 1, "PROTO_NC_ACT_SOMEONESHOUT_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_ACT_SOMEONESHOUT_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_ACT_SOMEONESHOUT_CMD___unnamed_type_speaker_ speaker;
    uint8_t _pad_at_0001[20];
    PROTO_NC_ACT_SOMEONESHOUT_CMD___unnamed_type_flag_ flag;
    uint8_t _pad_at_0016[1];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONESHOUT_CMD) == 23, "PROTO_NC_ACT_SOMEONESHOUT_CMD size drift");

struct PROTO_NC_ACT_SOMEONESPEEDCHANGE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ACT_SOMEONESPEEDCHANGE_CMD) == 3, "PROTO_NC_ACT_SOMEONESPEEDCHANGE_CMD size drift");

struct PROTO_NC_ACT_SOMEONESTOP_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONESTOP_CMD) == 10, "PROTO_NC_ACT_SOMEONESTOP_CMD size drift");

struct PROTO_NC_ACT_SOMEONEWALK_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE from;
    uint8_t _pad_at_0002[8];
    SHINE_XY_TYPE to;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEWALK_CMD) == 18, "PROTO_NC_ACT_SOMEONEWALK_CMD size drift");

struct PROTO_NC_ACT_SOMEONEWHISPER_CMD___unnamed_type_flag_ {
    uint32_t  GMColor;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEWHISPER_CMD___unnamed_type_flag_) == 1, "PROTO_NC_ACT_SOMEONEWHISPER_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_ACT_SOMEONEWHISPER_CMD {
    uint8_t _pad_at_0000[1];
    Name5 talker;
    uint8_t _pad_at_0001[20];
    PROTO_NC_ACT_SOMEONEWHISPER_CMD___unnamed_type_flag_ flag;
    uint8_t _pad_at_0016[1];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_SOMEONEWHISPER_CMD) == 23, "PROTO_NC_ACT_SOMEONEWHISPER_CMD size drift");

struct PROTO_NC_ACT_STOP_REQ {
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_STOP_REQ) == 8, "PROTO_NC_ACT_STOP_REQ size drift");

struct PROTO_NC_ACT_WALK_REQ {
    SHINE_XY_TYPE from;
    uint8_t _pad_at_0000[8];
    SHINE_XY_TYPE to;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ACT_WALK_REQ) == 16, "PROTO_NC_ACT_WALK_REQ size drift");

struct PROTO_NC_ACT_WEDDING_AGREEMENT_DIVORCE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_WEDDING_AGREEMENT_DIVORCE_ACK) == 2, "PROTO_NC_ACT_WEDDING_AGREEMENT_DIVORCE_ACK size drift");

struct PROTO_NC_ACT_WEDDING_COMPULSORY_DIVORCE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_WEDDING_COMPULSORY_DIVORCE_ACK) == 2, "PROTO_NC_ACT_WEDDING_COMPULSORY_DIVORCE_ACK size drift");

struct PROTO_NC_ACT_WEDDING_COUPLE_ENTRANCE_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_COUPLE_ENTRANCE_RNG) == 16, "PROTO_NC_ACT_WEDDING_COUPLE_ENTRANCE_RNG size drift");

struct PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_ACK) == 2, "PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_ACK size drift");

struct PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_READY_ACK {
    uint8_t _pad_at_0000[2];
    Name5 GroomID;
    uint8_t _pad_at_0002[20];
    Name5 BrideID;
    uint8_t _pad_at_0016[44];
    tm tm_EnterStart;
    uint8_t _pad_at_0042[36];
    tm tm_WeddingStart;
    uint8_t _pad_at_0066[36];
    tm tm_WeddingEnd;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_READY_ACK) == 174, "PROTO_NC_ACT_WEDDING_HALL_GUEST_ENTER_READY_ACK size drift");

struct PROTO_NC_ACT_WEDDING_HALL_RESERV_ACK {
    uint8_t _pad_at_0000[10];
    tm tm_ReservedTime;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_HALL_RESERV_ACK) == 46, "PROTO_NC_ACT_WEDDING_HALL_RESERV_ACK size drift");

struct PROTO_NC_ACT_WEDDING_PROPOSEACK_ACK {
    uint8_t _pad_at_0000[3];
    wchar_t response_word[37];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_PROPOSEACK_ACK) == 40, "PROTO_NC_ACT_WEDDING_PROPOSEACK_ACK size drift");

struct PROTO_NC_ACT_WEDDING_PROPOSEACK_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t propose_word[37];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_PROPOSEACK_REQ) == 39, "PROTO_NC_ACT_WEDDING_PROPOSEACK_REQ size drift");

struct PROTO_NC_ACT_WEDDING_PROPOSEREQ_ACK {
    uint8_t _pad_at_0000[4];
    wchar_t response_word[37];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_PROPOSEREQ_ACK) == 41, "PROTO_NC_ACT_WEDDING_PROPOSEREQ_ACK size drift");

struct PROTO_NC_ACT_WEDDING_PROPOSEREQ_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t propose_word[37];
};
static_assert(sizeof(PROTO_NC_ACT_WEDDING_PROPOSEREQ_REQ) == 39, "PROTO_NC_ACT_WEDDING_PROPOSEREQ_REQ size drift");

struct PROTO_NC_ACT_WEDDING_SOMEONE { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ACT_WEDDING_SOMEONE) == 4, "PROTO_NC_ACT_WEDDING_SOMEONE size drift");

struct PROTO_NC_ACT_WHISPERFAIL_ACK {
    uint8_t _pad_at_0000[2];
    Name5 receiver;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_ACT_WHISPERFAIL_ACK) == 22, "PROTO_NC_ACT_WHISPERFAIL_ACK size drift");

struct PROTO_NC_ACT_WHISPERSUCCESS_ACK {
    uint8_t _pad_at_0000[1];
    Name5 receiver;
    uint8_t _pad_at_0001[21];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_WHISPERSUCCESS_ACK) == 22, "PROTO_NC_ACT_WHISPERSUCCESS_ACK size drift");

struct PROTO_NC_ACT_WHISPER_REQ {
    uint8_t _pad_at_0000[1];
    Name5 receiver;
    uint8_t _pad_at_0001[21];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_ACT_WHISPER_REQ) == 22, "PROTO_NC_ACT_WHISPER_REQ size drift");

struct PROTO_NC_ANNOUNCE_W2C_CMD {
    ANNOUNCE_MESSAGE AnnounceMessage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ANNOUNCE_W2C_CMD) == 2, "PROTO_NC_ANNOUNCE_W2C_CMD size drift");

struct PROTO_NC_ANNOUNCE_Z2W_CMD {
    ANNOUNCE_MESSAGE AnnounceMessage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ANNOUNCE_Z2W_CMD) == 2, "PROTO_NC_ANNOUNCE_Z2W_CMD size drift");

struct PROTO_NC_AVATAR_CREATEDATAFAIL_ACK {
    Name5 charid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATEDATAFAIL_ACK) == 22, "PROTO_NC_AVATAR_CREATEDATAFAIL_ACK size drift");

struct PROTO_NC_AVATAR_CREATEDATASUC_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_AVATARINFORMATION_______0_bytes___ avatar;
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATEDATASUC_ACK) == 1, "PROTO_NC_AVATAR_CREATEDATASUC_ACK size drift");

struct PROTO_NC_AVATAR_CREATE_REQ {
    uint8_t _pad_at_0000[1];
    Name5 name;
    uint8_t _pad_at_0001[20];
    PROTO_AVATAR_SHAPE_INFO char_shape;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATE_REQ) == 25, "PROTO_NC_AVATAR_CREATE_REQ size drift");

struct PROTO_NC_AVATAR_CREATEDATA_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _pad_at_0002[6];
    PROTO_NC_AVATAR_CREATE_REQ clientdata;
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATEDATA_REQ) == 33, "PROTO_NC_AVATAR_CREATEDATA_REQ size drift");

struct PROTO_NC_AVATAR_CREATEDATA_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_CREATEDATA_REQ req;
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATEDATA_REQ_SEND) == 36, "PROTO_NC_AVATAR_CREATEDATA_REQ_SEND size drift");

struct PROTO_NC_AVATAR_CREATEFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_AVATAR_CREATEFAIL_ACK) == 2, "PROTO_NC_AVATAR_CREATEFAIL_ACK size drift");

struct PROTO_NC_AVATAR_CREATEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_CREATEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATEFAIL_ACK_SEND) == 5, "PROTO_NC_AVATAR_CREATEFAIL_ACK_SEND size drift");

struct PROTO_NC_AVATAR_CREATESUCC_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_AVATARINFORMATION avatar;
    uint8_t _tail[130];
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATESUCC_ACK) == 131, "PROTO_NC_AVATAR_CREATESUCC_ACK size drift");

struct PROTO_NC_AVATAR_CREATESUCC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_CREATESUCC_ACK ack;
};
static_assert(sizeof(PROTO_NC_AVATAR_CREATESUCC_ACK_SEND) == 134, "PROTO_NC_AVATAR_CREATESUCC_ACK_SEND size drift");

struct PROTO_NC_AVATAR_ERASEDATAFAIL_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASEDATAFAIL_ACK) == 4, "PROTO_NC_AVATAR_ERASEDATAFAIL_ACK size drift");

struct PROTO_NC_AVATAR_ERASEDATASUC_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASEDATASUC_ACK) == 15, "PROTO_NC_AVATAR_ERASEDATASUC_ACK size drift");

struct PROTO_NC_AVATAR_ERASEDATA_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASEDATA_REQ) == 15, "PROTO_NC_AVATAR_ERASEDATA_REQ size drift");

struct PROTO_NC_AVATAR_ERASEDATA_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_ERASEDATA_REQ req;
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASEDATA_REQ_SEND) == 18, "PROTO_NC_AVATAR_ERASEDATA_REQ_SEND size drift");

struct PROTO_NC_AVATAR_ERASEFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_AVATAR_ERASEFAIL_ACK) == 2, "PROTO_NC_AVATAR_ERASEFAIL_ACK size drift");

struct PROTO_NC_AVATAR_ERASEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_ERASEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASEFAIL_ACK_SEND) == 5, "PROTO_NC_AVATAR_ERASEFAIL_ACK_SEND size drift");

struct PROTO_NC_AVATAR_ERASESUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_AVATAR_ERASESUC_ACK) == 1, "PROTO_NC_AVATAR_ERASESUC_ACK size drift");

struct PROTO_NC_AVATAR_ERASESUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_ERASESUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_AVATAR_ERASESUC_ACK_SEND) == 4, "PROTO_NC_AVATAR_ERASESUC_ACK_SEND size drift");

struct PROTO_NC_AVATAR_ERASE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_AVATAR_ERASE_REQ) == 1, "PROTO_NC_AVATAR_ERASE_REQ size drift");

struct PROTO_NC_AVATAR_GUILD_DATA_ACK {
    uint8_t _pad_at_0000[9];
    GUILD_CLIENT_______0_bytes___ Guild;
};
static_assert(sizeof(PROTO_NC_AVATAR_GUILD_DATA_ACK) == 9, "PROTO_NC_AVATAR_GUILD_DATA_ACK size drift");

struct PROTO_NC_AVATAR_GUILD_DATA_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_AVATAR_GUILD_DATA_REQ) == 7, "PROTO_NC_AVATAR_GUILD_DATA_REQ size drift");

struct PROTO_NC_AVATAR_GUILD_DATA_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_AVATAR_GUILD_DATA_REQ req;
};
static_assert(sizeof(PROTO_NC_AVATAR_GUILD_DATA_REQ_SEND) == 10, "PROTO_NC_AVATAR_GUILD_DATA_REQ_SEND size drift");

struct PROTO_NC_AVATAR_GUILD_MEMBER_DATA_ACK {
    uint8_t _pad_at_0000[14];
    GUILD_MEMEBER_INFO_______0_bytes___ MemberList;
};
static_assert(sizeof(PROTO_NC_AVATAR_GUILD_MEMBER_DATA_ACK) == 14, "PROTO_NC_AVATAR_GUILD_MEMBER_DATA_ACK size drift");

struct PROTO_NC_AVATAR_GUILD_MEMBER_DATA_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_AVATAR_GUILD_MEMBER_DATA_REQ) == 11, "PROTO_NC_AVATAR_GUILD_MEMBER_DATA_REQ size drift");

struct PROTO_NC_AVATAR_RENAME_ACK {
    uint8_t _pad_at_0000[1];
    Name5 NewName;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_AVATAR_RENAME_ACK) == 23, "PROTO_NC_AVATAR_RENAME_ACK size drift");

struct PROTO_NC_AVATAR_RENAME_DB_ACK {
    NETPACKETHEADER NetPacketHeader;
    uint8_t _pad_at_0002[5];
    Name5 NewName;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_AVATAR_RENAME_DB_ACK) == 29, "PROTO_NC_AVATAR_RENAME_DB_ACK size drift");

struct PROTO_NC_AVATAR_RENAME_DB_REQ {
    NETPACKETHEADER NetPacketHeader;
    uint8_t _pad_at_0002[5];
    Name5 NewName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_AVATAR_RENAME_DB_REQ) == 27, "PROTO_NC_AVATAR_RENAME_DB_REQ size drift");

struct PROTO_NC_AVATAR_RENAME_REQ {
    uint8_t _pad_at_0000[1];
    Name5 NewName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_AVATAR_RENAME_REQ) == 21, "PROTO_NC_AVATAR_RENAME_REQ size drift");

struct PROTO_NC_BAT_ABSTATEINFORM_CMD {
    ABSTATEINDEX abstate;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_ABSTATEINFORM_CMD) == 8, "PROTO_NC_BAT_ABSTATEINFORM_CMD size drift");

struct PROTO_NC_BAT_ABSTATEINFORM_NOEFFECT_CMD {
    ABSTATEINDEX abstate;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_ABSTATEINFORM_NOEFFECT_CMD) == 8, "PROTO_NC_BAT_ABSTATEINFORM_NOEFFECT_CMD size drift");

struct PROTO_NC_BAT_ABSTATERESET_CMD {
    uint8_t _pad_at_0000[2];
    ABSTATEINDEX abstate;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BAT_ABSTATERESET_CMD) == 6, "PROTO_NC_BAT_ABSTATERESET_CMD size drift");

struct PROTO_NC_BAT_ABSTATESET_CMD {
    uint8_t _pad_at_0000[2];
    ABSTATEINDEX abstate;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BAT_ABSTATESET_CMD) == 6, "PROTO_NC_BAT_ABSTATESET_CMD size drift");

struct PROTO_NC_BAT_ABSTATE_ERASE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_ABSTATE_ERASE_ACK) == 2, "PROTO_NC_BAT_ABSTATE_ERASE_ACK size drift");

struct PROTO_NC_BAT_ABSTATE_ERASE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_ABSTATE_ERASE_REQ) == 2, "PROTO_NC_BAT_ABSTATE_ERASE_REQ size drift");

struct PROTO_NC_BAT_APCHANGE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_APCHANGE_CMD) == 4, "PROTO_NC_BAT_APCHANGE_CMD size drift");

struct PROTO_NC_BAT_AREADOTDAMAGE_CMD {
    uint8_t _pad_at_0000[5];
    PROTO_NC_BAT_AREADOTDAMAGE_CMD__targetinfo_______0_bytes___ target;
};
static_assert(sizeof(PROTO_NC_BAT_AREADOTDAMAGE_CMD) == 5, "PROTO_NC_BAT_AREADOTDAMAGE_CMD size drift");

struct PROTO_NC_BAT_AREADOTDAMAGE_CMD__targetinfo { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_BAT_AREADOTDAMAGE_CMD__targetinfo) == 12, "PROTO_NC_BAT_AREADOTDAMAGE_CMD__targetinfo size drift");

struct PROTO_NC_BAT_ASSIST_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_ASSIST_ACK) == 2, "PROTO_NC_BAT_ASSIST_ACK size drift");

struct PROTO_NC_BAT_ASSIST_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_ASSIST_REQ) == 2, "PROTO_NC_BAT_ASSIST_REQ size drift");

struct PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_damage_ {
    uint32_t  iscritical;
    uint32_t  damage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_damage_) == 2, "PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_damage_ size drift");

struct PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_flag_ {
    uint32_t  actioncode;
    uint32_t  isresist;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_BASH_HITTED_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_damage_ damage;
    PROTO_NC_BAT_BASH_HITTED_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HITTED_CMD) == 13, "PROTO_NC_BAT_BASH_HITTED_CMD size drift");

struct PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_damage_ {
    uint32_t  iscritical;
    uint32_t  damage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_damage_) == 2, "PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_damage_ size drift");

struct PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_flag_ {
    uint32_t  actioncode;
    uint32_t  isresist;
    uint32_t  isImmune;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_BASH_HIT_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_damage_ damage;
    PROTO_NC_BAT_BASH_HIT_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_HIT_CMD) == 13, "PROTO_NC_BAT_BASH_HIT_CMD size drift");

struct PROTO_NC_BAT_BASH_MISSED_CMD___unnamed_type_flag_ {
    uint32_t  actioncode;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isresist;
    uint32_t  isImmune;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_MISSED_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BAT_BASH_MISSED_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_BASH_MISSED_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_BASH_MISSED_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_MISSED_CMD) == 5, "PROTO_NC_BAT_BASH_MISSED_CMD size drift");

struct PROTO_NC_BAT_BASH_MISS_CMD___unnamed_type_flag_ {
    uint32_t  actioncode;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isImmune;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_MISS_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BAT_BASH_MISS_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_BASH_MISS_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_BASH_MISS_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_BASH_MISS_CMD) == 5, "PROTO_NC_BAT_BASH_MISS_CMD size drift");

struct PROTO_NC_BAT_CEASE_FIRE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_CEASE_FIRE_CMD) == 2, "PROTO_NC_BAT_CEASE_FIRE_CMD size drift");

struct PROTO_NC_BAT_CLIENT_MOB_KILL_ANNOUNCE_CMD {
    MobKillAnnounceType nTextIndex;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BAT_CLIENT_MOB_KILL_ANNOUNCE_CMD) == 4, "PROTO_NC_BAT_CLIENT_MOB_KILL_ANNOUNCE_CMD size drift");

struct PROTO_NC_BAT_DOTDAMAGE_CMD { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_BAT_DOTDAMAGE_CMD) == 13, "PROTO_NC_BAT_DOTDAMAGE_CMD size drift");

struct PROTO_NC_BAT_EXPGAIN_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_EXPGAIN_CMD) == 6, "PROTO_NC_BAT_EXPGAIN_CMD size drift");

struct PROTO_NC_BAT_EXPLOST_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_EXPLOST_CMD) == 4, "PROTO_NC_BAT_EXPLOST_CMD size drift");

struct PROTO_NC_BAT_FAMEGAIN_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_FAMEGAIN_CMD) == 4, "PROTO_NC_BAT_FAMEGAIN_CMD size drift");

struct PROTO_NC_BAT_HIT_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BAT_HIT_REQ) == 3, "PROTO_NC_BAT_HIT_REQ size drift");

struct PROTO_NC_BAT_HPCHANGE_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_HPCHANGE_CMD) == 6, "PROTO_NC_BAT_HPCHANGE_CMD size drift");

struct PROTO_NC_BAT_LEVELUP_CMD {
    uint8_t _pad_at_0000[3];
    CHAR_PARAMETER_DATA newparam;
    uint8_t _tail[232];
};
static_assert(sizeof(PROTO_NC_BAT_LEVELUP_CMD) == 235, "PROTO_NC_BAT_LEVELUP_CMD size drift");

struct PROTO_NC_BAT_LPCHANGE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_LPCHANGE_CMD) == 4, "PROTO_NC_BAT_LPCHANGE_CMD size drift");

struct PROTO_NC_BAT_MOBSLAYER_CMD {
    uint8_t _pad_at_0000[3];
    uint32_t slayers[0];
};
static_assert(sizeof(PROTO_NC_BAT_MOBSLAYER_CMD) == 3, "PROTO_NC_BAT_MOBSLAYER_CMD size drift");

struct PROTO_NC_BAT_PKINPKFIELD_CLIENT_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_BAT_PKINPKFIELD_CLIENT_CMD) == 8, "PROTO_NC_BAT_PKINPKFIELD_CLIENT_CMD size drift");

struct PROTO_NC_BAT_PKINPKFIELD_WMS_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_BAT_PKINPKFIELD_WMS_CMD) == 10, "PROTO_NC_BAT_PKINPKFIELD_WMS_CMD size drift");

struct PROTO_NC_BAT_REALLYKILL_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_REALLYKILL_CMD) == 4, "PROTO_NC_BAT_REALLYKILL_CMD size drift");

struct PROTO_NC_BAT_REFLECTIONDAMAGE_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_BAT_REFLECTIONDAMAGE_CMD) == 14, "PROTO_NC_BAT_REFLECTIONDAMAGE_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_CASTABORT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_CASTABORT_ACK) == 2, "PROTO_NC_BAT_SKILLBASH_CASTABORT_ACK size drift");

struct PROTO_NC_BAT_SKILLBASH_CAST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_CAST_REQ) == 4, "PROTO_NC_BAT_SKILLBASH_CAST_REQ size drift");

struct PROTO_NC_BAT_SKILLBASH_FLD_CAST_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE locate;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_FLD_CAST_REQ) == 10, "PROTO_NC_BAT_SKILLBASH_FLD_CAST_REQ size drift");

struct PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_flag_ {
    uint32_t  isdamage;
    uint32_t  iscritical;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isheal;
    uint32_t  isenchant;
    uint32_t  isresist;
    uint32_t  IsCostumWeapon;
    uint8_t _pad_at_0000[1];
    uint32_t  isDead;
    uint32_t  IsCostumShield;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_flag_) == 2, "PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SKILLBASH_HITTED_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE targetpoint;
    uint8_t _pad_at_0004[8];
    PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_flag_ flag;
    uint8_t _pad_at_000e[11];
    PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_target________0_bytes___ target;
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HITTED_CMD) == 25, "PROTO_NC_BAT_SKILLBASH_HITTED_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_target_ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_target_) == 8, "PROTO_NC_BAT_SKILLBASH_HITTED_CMD___unnamed_type_target_ size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_BLAST_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_BLAST_CMD) == 6, "PROTO_NC_BAT_SKILLBASH_HIT_BLAST_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_CMD {
    uint8_t _pad_at_0000[3];
    SHINE_XY_TYPE targetpoint;
    uint8_t _pad_at_0003[8];
    PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target________0_bytes___ target;
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_CMD) == 11, "PROTO_NC_BAT_SKILLBASH_HIT_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ {
    uint32_t  isdamage;
    uint32_t  iscritical;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isheal;
    uint32_t  isenchant;
    uint32_t  isresist;
    uint32_t  IsCostumWeapon;
    uint8_t _pad_at_0000[1];
    uint32_t  isDead;
    uint32_t  isCostumShield;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_) == 2, "PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target_ {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ flag;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target_) == 14, "PROTO_NC_BAT_SKILLBASH_HIT_CMD___unnamed_type_target_ size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD {
    uint8_t _pad_at_0000[9];
    PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage_______0_bytes___ target;
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD) == 9, "PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage___unnamed_type_flag_ {
    uint32_t  isdamage;
    uint32_t  iscritical;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isheal;
    uint32_t  isenchant;
    uint32_t  isresist;
    uint32_t  IsCostumWeapon;
    uint8_t _pad_at_0000[1];
    uint32_t  isDead;
    uint32_t  isImmune;
    uint32_t  IsCostumShield;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage___unnamed_type_flag_) == 2, "PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage___unnamed_type_flag_ flag;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage) == 14, "PROTO_NC_BAT_SKILLBASH_HIT_DAMAGE_CMD__SkillDamage size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_FLD_START_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE targetloc;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_FLD_START_CMD) == 12, "PROTO_NC_BAT_SKILLBASH_HIT_FLD_START_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_HIT_OBJ_START_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_HIT_OBJ_START_CMD) == 6, "PROTO_NC_BAT_SKILLBASH_HIT_OBJ_START_CMD size drift");

struct PROTO_NC_BAT_SKILLBASH_OBJ_CAST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBASH_OBJ_CAST_REQ) == 4, "PROTO_NC_BAT_SKILLBASH_OBJ_CAST_REQ size drift");

struct PROTO_NC_BAT_SKILLBLAST_LIGHTNINGWAVE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLBLAST_LIGHTNINGWAVE_CMD) == 8, "PROTO_NC_BAT_SKILLBLAST_LIGHTNINGWAVE_CMD size drift");

struct PROTO_NC_BAT_SKILLCAST_FAIL_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLCAST_FAIL_ACK) == 1, "PROTO_NC_BAT_SKILLCAST_FAIL_ACK size drift");

struct PROTO_NC_BAT_SKILLCAST_REQ { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLCAST_REQ) == 5, "PROTO_NC_BAT_SKILLCAST_REQ size drift");

struct PROTO_NC_BAT_SKILLCAST_SUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLCAST_SUC_ACK) == 1, "PROTO_NC_BAT_SKILLCAST_SUC_ACK size drift");

struct PROTO_NC_BAT_SKILLENCHANT_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLENCHANT_REQ) == 4, "PROTO_NC_BAT_SKILLENCHANT_REQ size drift");

struct PROTO_NC_BAT_SKILLSMASH_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_CMD) == 5, "PROTO_NC_BAT_SKILLSMASH_CMD size drift");

struct PROTO_NC_BAT_SKILLSMASH_ENCHANT_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_ENCHANT_CMD) == 3, "PROTO_NC_BAT_SKILLSMASH_ENCHANT_CMD size drift");

struct PROTO_NC_BAT_SKILLSMASH_HIT_CMD___unnamed_type_damage_ {
    uint32_t  isdead;
    uint32_t  iscritical;
    uint32_t  damage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_HIT_CMD___unnamed_type_damage_) == 2, "PROTO_NC_BAT_SKILLSMASH_HIT_CMD___unnamed_type_damage_ size drift");

struct PROTO_NC_BAT_SKILLSMASH_HIT_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_BAT_SKILLSMASH_HIT_CMD___unnamed_type_damage_ damage;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_HIT_CMD) == 7, "PROTO_NC_BAT_SKILLSMASH_HIT_CMD size drift");

struct PROTO_NC_BAT_SKILLSMASH_HITTED_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLSMASH_HIT_CMD hitted;
};
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_HITTED_CMD) == 9, "PROTO_NC_BAT_SKILLSMASH_HITTED_CMD size drift");

struct PROTO_NC_BAT_SKILLSMASH_MISS_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BAT_SKILLSMASH_MISS_CMD) == 3, "PROTO_NC_BAT_SKILLSMASH_MISS_CMD size drift");

struct PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted____unnamed_type_damage_ {
    uint32_t  isdead;
    uint32_t  iscritical;
    uint32_t  damage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted____unnamed_type_damage_) == 2, "PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted____unnamed_type_damage_ size drift");

struct PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted_ {
    PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted____unnamed_type_damage_ damage;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted_) == 3, "PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted_ size drift");

struct PROTO_NC_BAT_SMASH_HITTED_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SMASH_HITTED_ACK___unnamed_type_hitted_ hitted;
};
static_assert(sizeof(PROTO_NC_BAT_SMASH_HITTED_ACK) == 5, "PROTO_NC_BAT_SMASH_HITTED_ACK size drift");

struct PROTO_NC_BAT_SMASH_HIT_ACK___unnamed_type_damage_ {
    uint32_t  isdead;
    uint32_t  iscritical;
    uint32_t  damage;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_SMASH_HIT_ACK___unnamed_type_damage_) == 2, "PROTO_NC_BAT_SMASH_HIT_ACK___unnamed_type_damage_ size drift");

struct PROTO_NC_BAT_SMASH_HIT_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SMASH_HIT_ACK___unnamed_type_damage_ damage;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SMASH_HIT_ACK) == 5, "PROTO_NC_BAT_SMASH_HIT_ACK size drift");

struct PROTO_NC_BAT_SMASH_MISS_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BAT_SMASH_MISS_ACK) == 3, "PROTO_NC_BAT_SMASH_MISS_ACK size drift");

struct PROTO_NC_BAT_SMASH_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_SMASH_REQ) == 2, "PROTO_NC_BAT_SMASH_REQ size drift");

struct PROTO_NC_BAT_SOMEONEBASH_HIT_CMD { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONEBASH_HIT_CMD) == 13, "PROTO_NC_BAT_SOMEONEBASH_HIT_CMD size drift");

struct PROTO_NC_BAT_SOMEONEBASH_MISS_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONEBASH_MISS_CMD) == 7, "PROTO_NC_BAT_SOMEONEBASH_MISS_CMD size drift");

struct PROTO_NC_BAT_SOMEONEDAMAGED_LARGE_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONEDAMAGED_LARGE_CMD) == 7, "PROTO_NC_BAT_SOMEONEDAMAGED_LARGE_CMD size drift");

struct PROTO_NC_BAT_SOMEONEDAMAGED_SMALL_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONEDAMAGED_SMALL_CMD) == 6, "PROTO_NC_BAT_SOMEONEDAMAGED_SMALL_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_CASTCUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_CASTCUT_CMD) == 2, "PROTO_NC_BAT_SOMEONESKILLBASH_CASTCUT_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_CAST_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_CAST_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLBASH_CAST_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_FLD_CAST_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_FLD_CAST_CMD) == 12, "PROTO_NC_BAT_SOMEONESKILLBASH_FLD_CAST_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE targetpoint;
    uint8_t _pad_at_0004[9];
    PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target________0_bytes___ target;
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD) == 13, "PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ {
    uint32_t  isdamage;
    uint32_t  ismissed;
    uint32_t  isheal;
    uint32_t  isenchant;
    uint32_t  damagedisplay;
    uint32_t  isDead;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_) == 1, "PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target_ {
    uint8_t _pad_at_0000[10];
    PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target____unnamed_type_flag_ flag;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target_) == 13, "PROTO_NC_BAT_SOMEONESKILLBASH_HIT_CMD___unnamed_type_target_ size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_HIT_FLD_START_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLBASH_HIT_FLD_START_CMD castinfo;
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_HIT_FLD_START_CMD) == 14, "PROTO_NC_BAT_SOMEONESKILLBASH_HIT_FLD_START_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_HIT_OBJ_START_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLBASH_HIT_OBJ_START_CMD castinfo;
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_HIT_OBJ_START_CMD) == 8, "PROTO_NC_BAT_SOMEONESKILLBASH_HIT_OBJ_START_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLBASH_OBJ_CAST_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLBASH_OBJ_CAST_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLBASH_OBJ_CAST_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLCASTCUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLCASTCUT_CMD) == 2, "PROTO_NC_BAT_SOMEONESKILLCASTCUT_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLCAST_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLCAST_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLCAST_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLENCHANT_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BAT_SKILLENCHANT_REQ skillenchant;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLENCHANT_REQ) == 8, "PROTO_NC_BAT_SOMEONESKILLENCHANT_REQ size drift");

struct PROTO_NC_BAT_SOMEONESKILLSMASH_DAMAGED_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLSMASH_DAMAGED_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLSMASH_DAMAGED_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLSMASH_DEAD_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLSMASH_DEAD_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLSMASH_DEAD_CMD size drift");

struct PROTO_NC_BAT_SOMEONESKILLSMASH_ENCHANT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESKILLSMASH_ENCHANT_CMD) == 6, "PROTO_NC_BAT_SOMEONESKILLSMASH_ENCHANT_CMD size drift");

struct PROTO_NC_BAT_SOMEONESMASH_DAMAGED_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_BAT_SOMEONESMASH_DAMAGED_CMD) == 5, "PROTO_NC_BAT_SOMEONESMASH_DAMAGED_CMD size drift");

struct PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD___unnamed_type_flag_ {
    uint32_t  isMissed;
    uint32_t  isCostumCharged;
    uint32_t  isCostumShieldCharged;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD) == 13, "PROTO_NC_BAT_SOMEONESWING_DAMAGE_CMD size drift");

struct PROTO_NC_BAT_SOULCOLLECT_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BAT_SOULCOLLECT_CMD) == 3, "PROTO_NC_BAT_SOULCOLLECT_CMD size drift");

struct PROTO_NC_BAT_SPCHANGE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_SPCHANGE_CMD) == 4, "PROTO_NC_BAT_SPCHANGE_CMD size drift");

struct PROTO_NC_BAT_SUMEONELEVELUP_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_BAT_SUMEONELEVELUP_CMD) == 4, "PROTO_NC_BAT_SUMEONELEVELUP_CMD size drift");

struct PROTO_NC_BAT_SUMEONESKILLCUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_SUMEONESKILLCUT_CMD) == 2, "PROTO_NC_BAT_SUMEONESKILLCUT_CMD size drift");

struct PROTO_NC_BAT_SWING_DAMAGE_CMD___unnamed_type_flag_ {
    uint32_t  iscritical;
    uint32_t  isresist;
    uint32_t  ismissed;
    uint32_t  isshieldblock;
    uint32_t  isCostumCharged;
    uint32_t  isDead;
    uint32_t  isDamege2Heal;
    uint32_t  isImmune;
    uint8_t _pad_at_0000[1];
    uint32_t  isCostumShieldCharged;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BAT_SWING_DAMAGE_CMD___unnamed_type_flag_) == 2, "PROTO_NC_BAT_SWING_DAMAGE_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BAT_SWING_DAMAGE_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_BAT_SWING_DAMAGE_CMD___unnamed_type_flag_ flag;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_BAT_SWING_DAMAGE_CMD) == 16, "PROTO_NC_BAT_SWING_DAMAGE_CMD size drift");

struct PROTO_NC_BAT_SWING_START_CMD { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_BAT_SWING_START_CMD) == 9, "PROTO_NC_BAT_SWING_START_CMD size drift");

struct PROTO_NC_BAT_TARGETCHANGE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_TARGETCHANGE_CMD) == 2, "PROTO_NC_BAT_TARGETCHANGE_CMD size drift");

struct PROTO_NC_BAT_TARGETINFO_CMD { uint8_t data[30]; };
static_assert(sizeof(PROTO_NC_BAT_TARGETINFO_CMD) == 30, "PROTO_NC_BAT_TARGETINFO_CMD size drift");

struct PROTO_NC_BAT_TARGET_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_TARGET_REQ) == 2, "PROTO_NC_BAT_TARGET_REQ size drift");

struct PROTO_NC_BAT_TOGGLESKILL_OFF_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_TOGGLESKILL_OFF_CMD) == 2, "PROTO_NC_BAT_TOGGLESKILL_OFF_CMD size drift");

struct PROTO_NC_BAT_TOGGLESKILL_ON_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BAT_TOGGLESKILL_ON_CMD) == 2, "PROTO_NC_BAT_TOGGLESKILL_ON_CMD size drift");

struct PROTO_NC_BAT_WORLD_MOB_KILL_ANNOUNCE_CMD {
    MobKillAnnounceType nTextIndex;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BAT_WORLD_MOB_KILL_ANNOUNCE_CMD) == 4, "PROTO_NC_BAT_WORLD_MOB_KILL_ANNOUNCE_CMD size drift");

struct PROTO_NC_BOOTH_BUYREFRESH_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BOOTH_BUYREFRESH_CMD) == 3, "PROTO_NC_BOOTH_BUYREFRESH_CMD size drift");

struct PROTO_NC_BOOTH_CLOSE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_CLOSE_ACK) == 2, "PROTO_NC_BOOTH_CLOSE_ACK size drift");

struct PROTO_NC_BOOTH_ENTRY_BUY_ACK {
    uint8_t _pad_at_0000[5];
    PROTO_NC_BOOTH_ENTRY_BUY_ACK__BoothItemList_______0_bytes___ items;
};
static_assert(sizeof(PROTO_NC_BOOTH_ENTRY_BUY_ACK) == 5, "PROTO_NC_BOOTH_ENTRY_BUY_ACK size drift");

struct PROTO_NC_BOOTH_ENTRY_BUY_ACK__BoothItemList { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_BOOTH_ENTRY_BUY_ACK__BoothItemList) == 13, "PROTO_NC_BOOTH_ENTRY_BUY_ACK__BoothItemList size drift");

struct PROTO_NC_BOOTH_ENTRY_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_ENTRY_REQ) == 2, "PROTO_NC_BOOTH_ENTRY_REQ size drift");

struct PROTO_NC_BOOTH_ENTRY_SELL_ACK {
    uint8_t _pad_at_0000[5];
    PROTO_NC_BOOTH_ENTRY_SELL_ACK__BoothItemList_______0_bytes___ items;
};
static_assert(sizeof(PROTO_NC_BOOTH_ENTRY_SELL_ACK) == 5, "PROTO_NC_BOOTH_ENTRY_SELL_ACK size drift");

struct PROTO_NC_BOOTH_ENTRY_SELL_ACK__BoothItemList {
    uint8_t _pad_at_0000[10];
    SHINE_ITEM_STRUCT item;
    uint8_t _tail[103];
};
static_assert(sizeof(PROTO_NC_BOOTH_ENTRY_SELL_ACK__BoothItemList) == 113, "PROTO_NC_BOOTH_ENTRY_SELL_ACK__BoothItemList size drift");

struct PROTO_NC_BOOTH_ITEMTRADE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_ITEMTRADE_ACK) == 2, "PROTO_NC_BOOTH_ITEMTRADE_ACK size drift");

struct PROTO_NC_BOOTH_ITEMTRADE_REQ { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_BOOTH_ITEMTRADE_REQ) == 5, "PROTO_NC_BOOTH_ITEMTRADE_REQ size drift");

struct PROTO_NC_BOOTH_OPEN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_OPEN_ACK) == 2, "PROTO_NC_BOOTH_OPEN_ACK size drift");

struct PROTO_NC_BOOTH_OPEN_REQ___unnamed_type_flag_ {
    uint32_t  issell;
    uint32_t  itemnum;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BOOTH_OPEN_REQ___unnamed_type_flag_) == 1, "PROTO_NC_BOOTH_OPEN_REQ___unnamed_type_flag_ size drift");

struct PROTO_NC_BOOTH_OPEN_REQ {
    STREETBOOTH_SIGNBOARD signboard;
    uint8_t _pad_at_0000[30];
    PROTO_NC_BOOTH_OPEN_REQ___unnamed_type_flag_ flag;
    PROTO_NC_BOOTH_OPEN_REQ__BoothItem_______0_bytes___ items;
};
static_assert(sizeof(PROTO_NC_BOOTH_OPEN_REQ) == 31, "PROTO_NC_BOOTH_OPEN_REQ size drift");

struct PROTO_NC_BOOTH_OPEN_REQ__BoothItem { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_BOOTH_OPEN_REQ__BoothItem) == 12, "PROTO_NC_BOOTH_OPEN_REQ__BoothItem size drift");

struct PROTO_NC_BOOTH_REFRESH_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_REFRESH_REQ) == 2, "PROTO_NC_BOOTH_REFRESH_REQ size drift");

struct PROTO_NC_BOOTH_SEARCH_BOOTH_CLOSED_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BOOTH_SEARCH_BOOTH_CLOSED_CMD) == 2, "PROTO_NC_BOOTH_SEARCH_BOOTH_CLOSED_CMD size drift");

struct PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_ACK {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE BoothPosition;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_ACK) == 12, "PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_ACK size drift");

struct PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_REQ) == 6, "PROTO_NC_BOOTH_SEARCH_BOOTH_POSITION_REQ size drift");

struct PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_ACK {
    uint8_t _pad_at_0000[3];
    BoothItemInfo_______0_bytes___ ItemList;
};
static_assert(sizeof(PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_ACK) == 3, "PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_ACK size drift");

struct PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_REQ {
    MarketSearch Category;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_REQ) == 6, "PROTO_NC_BOOTH_SEARCH_ITEM_LIST_CATEGORIZED_REQ size drift");

struct PROTO_NC_BOOTH_SOMEONECLOSE_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_NOTCAMP shape;
    uint8_t _tail[43];
};
static_assert(sizeof(PROTO_NC_BOOTH_SOMEONECLOSE_CMD) == 45, "PROTO_NC_BOOTH_SOMEONECLOSE_CMD size drift");

struct PROTO_NC_BOOTH_SOMEONEINTERIORSTART_CMD {
    uint8_t _pad_at_0000[2];
    STREETBOOTH_SIGNBOARD signboard;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_BOOTH_SOMEONEINTERIORSTART_CMD) == 32, "PROTO_NC_BOOTH_SOMEONEINTERIORSTART_CMD size drift");

struct PROTO_NC_BOOTH_SOMEONEOPEN_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_CAMP tent;
    uint8_t _pad_at_0002[13];
    STREETBOOTH_SIGNBOARD signboard;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_BOOTH_SOMEONEOPEN_CMD) == 45, "PROTO_NC_BOOTH_SOMEONEOPEN_CMD size drift");

struct PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_CMD {
    uint8_t _pad_at_0000[2];
    ABSTATE_INFORMATION info;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_CMD) == 14, "PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_CMD size drift");

struct PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_LIST_CMD {
    uint8_t _pad_at_0000[3];
    ABSTATE_INFORMATION_______0_bytes___ infoList;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_LIST_CMD) == 3, "PROTO_NC_BRIEFINFO_ABSTATE_CHANGE_LIST_CMD size drift");

struct PROTO_NC_BRIEFINFO_BRIEFINFODELETE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_BRIEFINFO_BRIEFINFODELETE_CMD) == 2, "PROTO_NC_BRIEFINFO_BRIEFINFODELETE_CMD size drift");

struct PROTO_NC_BRIEFINFO_BUILDDOOR_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_COORD_TYPE coord;
    uint8_t _pad_at_0004[10];
    Name8 blockindex;
    uint8_t _tail[34];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_BUILDDOOR_CMD) == 48, "PROTO_NC_BRIEFINFO_BUILDDOOR_CMD size drift");

struct PROTO_NC_BRIEFINFO_CHANGEDECORATE_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_BRIEFINFO_CHANGEDECORATE_CMD) == 5, "PROTO_NC_BRIEFINFO_CHANGEDECORATE_CMD size drift");

struct PROTO_NC_BRIEFINFO_CHANGEUPGRADE_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_BRIEFINFO_CHANGEUPGRADE_CMD) == 6, "PROTO_NC_BRIEFINFO_CHANGEUPGRADE_CMD size drift");

struct PROTO_NC_BRIEFINFO_CHANGEWEAPON_CMD {
    PROTO_NC_BRIEFINFO_CHANGEUPGRADE_CMD upgradeinfo;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_CHANGEWEAPON_CMD) == 9, "PROTO_NC_BRIEFINFO_CHANGEWEAPON_CMD size drift");

struct PROTO_NC_BRIEFINFO_CHARACTER_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_______0_bytes___ chars;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_CHARACTER_CMD) == 1, "PROTO_NC_BRIEFINFO_CHARACTER_CMD size drift");

struct PROTO_NC_BRIEFINFO_DOOR_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_BUILDDOOR_CMD_______0_bytes___ doors;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_DOOR_CMD) == 1, "PROTO_NC_BRIEFINFO_DOOR_CMD size drift");

struct PROTO_NC_BRIEFINFO_DROPEDITEM_CMD___unnamed_type_attr_ {
    uint32_t  rareness;
    uint32_t  looting;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_DROPEDITEM_CMD___unnamed_type_attr_) == 1, "PROTO_NC_BRIEFINFO_DROPEDITEM_CMD___unnamed_type_attr_ size drift");

struct PROTO_NC_BRIEFINFO_DROPEDITEM_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE location;
    uint8_t _pad_at_0004[10];
    PROTO_NC_BRIEFINFO_DROPEDITEM_CMD___unnamed_type_attr_ attr;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_DROPEDITEM_CMD) == 15, "PROTO_NC_BRIEFINFO_DROPEDITEM_CMD size drift");

struct PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD___unnamed_type_flag_ {
    uint32_t  loop;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD___unnamed_type_flag_) == 1, "PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD {
    uint8_t _pad_at_0000[2];
    Name8 effectname;
    uint8_t _pad_at_0002[32];
    SHINE_COORD_TYPE coord;
    uint8_t _pad_at_0022[13];
    PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD___unnamed_type_flag_ flag;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD) == 48, "PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD size drift");

struct PROTO_NC_BRIEFINFO_EFFECT_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_EFFECTBLAST_CMD_______0_bytes___ effects;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_EFFECT_CMD) == 1, "PROTO_NC_BRIEFINFO_EFFECT_CMD size drift");

struct PROTO_NC_BRIEFINFO_INFORM_CMD {
    uint8_t _pad_at_0000[2];
    NETCOMMAND ReceiveNetCommand;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_INFORM_CMD) == 6, "PROTO_NC_BRIEFINFO_INFORM_CMD size drift");

struct PROTO_NC_BRIEFINFO_ITEMONFIELD_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_DROPEDITEM_CMD_______0_bytes___ items;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_ITEMONFIELD_CMD) == 1, "PROTO_NC_BRIEFINFO_ITEMONFIELD_CMD size drift");

struct CHARTITLE_BRIEFINFO { uint8_t data[4]; };
static_assert(sizeof(CHARTITLE_BRIEFINFO) == 4, "CHARTITLE_BRIEFINFO size drift");

struct PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD {
    uint8_t _pad_at_0000[2];
    Name5 charid;
    uint8_t _pad_at_0002[20];
    SHINE_COORD_TYPE coord;
    uint8_t _pad_at_0016[11];
    PROTO_AVATAR_SHAPE_INFO shape;
    uint8_t _pad_at_0021[4];
    PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD___unnamed_type_shapedata_ shapedata;
    uint8_t _pad_at_0025[47];
    STOPEMOTICON_DESCRIPT emoticon;
    uint8_t _pad_at_0054[3];
    CHARTITLE_BRIEFINFO chartitle;
    ABNORMAL_STATE_BIT abstatebit;
    uint8_t _pad_at_005b[111];
    wchar_t sAnimation[32];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD) == 239, "PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD size drift");

struct PROTO_NC_BRIEFINFO_MAGICFIELDINFO_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_MAGICFIELDSPREAD_CMD_______0_bytes___ magicfield;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MAGICFIELDINFO_CMD) == 1, "PROTO_NC_BRIEFINFO_MAGICFIELDINFO_CMD size drift");

struct PROTO_NC_BRIEFINFO_MAGICFIELDSPREAD_CMD {
    uint8_t _pad_at_0000[6];
    SHINE_XY_TYPE location;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MAGICFIELDSPREAD_CMD) == 16, "PROTO_NC_BRIEFINFO_MAGICFIELDSPREAD_CMD size drift");

struct PROTO_NC_BRIEFINFO_MINIHOUSEBUILD_CMD {
    uint8_t _pad_at_0000[2];
    CHARBRIEFINFO_CAMP camp;
    uint8_t _pad_at_0002[12];
    Name5 charid;
    uint8_t _pad_at_000e[20];
    SHINE_COORD_TYPE coord;
    uint8_t _pad_at_0022[9];
    wchar_t title[21];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MINIHOUSEBUILD_CMD) == 64, "PROTO_NC_BRIEFINFO_MINIHOUSEBUILD_CMD size drift");

struct PROTO_NC_BRIEFINFO_MINIHOUSE_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_MINIHOUSEBUILD_CMD_______0_bytes___ minihouse;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MINIHOUSE_CMD) == 1, "PROTO_NC_BRIEFINFO_MINIHOUSE_CMD size drift");

struct PROTO_NC_BRIEFINFO_MOB_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_REGENMOB_CMD_______0_bytes___ mobs;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MOB_CMD) == 1, "PROTO_NC_BRIEFINFO_MOB_CMD size drift");

struct PROTO_NC_BRIEFINFO_MOVER_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_REGENMOVER_CMD_______0_bytes___ Movers;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_MOVER_CMD) == 1, "PROTO_NC_BRIEFINFO_MOVER_CMD size drift");

struct PROTO_NC_BRIEFINFO_PET_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_REGENPET_CMD_______0_bytes___ Pets;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_PET_CMD) == 1, "PROTO_NC_BRIEFINFO_PET_CMD size drift");

struct PROTO_NC_BRIEFINFO_PLAYER_INFO_APPEAR_CMD {
    uint8_t _pad_at_0000[2];
    Name5 sID;
    uint8_t _pad_at_0002[20];
    CHARTITLE_BRIEFINFO CharTitle;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_PLAYER_INFO_APPEAR_CMD) == 30, "PROTO_NC_BRIEFINFO_PLAYER_INFO_APPEAR_CMD size drift");

struct PROTO_NC_BRIEFINFO_PLAYER_LIST_INFO_APPEAR_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_BRIEFINFO_PLAYER_INFO_APPEAR_CMD_______0_bytes___ PlayerInfo;
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_PLAYER_LIST_INFO_APPEAR_CMD) == 1, "PROTO_NC_BRIEFINFO_PLAYER_LIST_INFO_APPEAR_CMD size drift");

struct PROTO_NC_BRIEFINFO_REGENMOB_CMD {
    uint8_t _pad_at_0000[5];
    SHINE_COORD_TYPE coord;
    uint8_t _pad_at_0005[10];
    PROTO_NC_BRIEFINFO_REGENMOB_CMD___unnamed_type_flag_ flag;
    uint8_t _pad_at_000f[103];
    wchar_t sAnimation[32];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_REGENMOB_CMD) == 153, "PROTO_NC_BRIEFINFO_REGENMOB_CMD size drift");

struct PROTO_NC_BRIEFINFO_REGENMOVER_CMD {
    uint8_t _pad_at_0000[10];
    SHINE_COORD_TYPE nCoord;
    uint8_t _pad_at_000a[9];
    ABNORMAL_STATE_BIT AbstateBit;
    uint8_t _pad_at_0013[104];
    uint16_t nSlotHandle[10];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_REGENMOVER_CMD) == 143, "PROTO_NC_BRIEFINFO_REGENMOVER_CMD size drift");

struct PROTO_NC_BRIEFINFO_REGENPET_CMD {
    uint8_t _pad_at_0000[6];
    SHINE_COORD_TYPE XYDir;
    uint8_t _pad_at_0006[9];
    Name4 sPetName;
    uint8_t _pad_at_000f[16];
    wchar_t sAnimationIndex[32];
};
static_assert(sizeof(PROTO_NC_BRIEFINFO_REGENPET_CMD) == 63, "PROTO_NC_BRIEFINFO_REGENPET_CMD size drift");

struct PROTO_NC_BRIEFINFO_UNEQUIP_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_BRIEFINFO_UNEQUIP_CMD) == 3, "PROTO_NC_BRIEFINFO_UNEQUIP_CMD size drift");

struct PROTO_NC_CHARGED_BOOTHSLOTSIZE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHARGED_BOOTHSLOTSIZE_CMD) == 1, "PROTO_NC_CHARGED_BOOTHSLOTSIZE_CMD size drift");

struct PROTO_NC_CHARGED_BUFFTERMINATE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHARGED_BUFFTERMINATE_CMD) == 4, "PROTO_NC_CHARGED_BUFFTERMINATE_CMD size drift");

struct PROTO_NC_CHARGED_DELETEWEAPONTITLE_CMD {
    ITEM_INVEN licenseitem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHARGED_DELETEWEAPONTITLE_CMD) == 2, "PROTO_NC_CHARGED_DELETEWEAPONTITLE_CMD size drift");

struct PROTO_NC_CHARGED_RESETBUFF_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_CHARGED_RESETBUFF_CMD) == 14, "PROTO_NC_CHARGED_RESETBUFF_CMD size drift");

struct PROTO_NC_CHARGED_SETBUFF_CMD {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER ItemKey;
    uint8_t _pad_at_0004[10];
    PROTO_CHARGEDBUFF_INFO ChargedBuff;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_CHARGED_SETBUFF_CMD) == 32, "PROTO_NC_CHARGED_SETBUFF_CMD size drift");

struct PROTO_NC_CHARGED_SKILLEMPOW_INITIALIZE_DB_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER restatitemkey;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_CHARGED_SKILLEMPOW_INITIALIZE_DB_REQ) == 21, "PROTO_NC_CHARGED_SKILLEMPOW_INITIALIZE_DB_REQ size drift");

struct PROTO_NC_CHARGED_STAT_INITIALIZE_DB_FAIL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHARGED_STAT_INITIALIZE_DB_FAIL_ACK) == 10, "PROTO_NC_CHARGED_STAT_INITIALIZE_DB_FAIL_ACK size drift");

struct PROTO_NC_CHARGED_STAT_INITIALIZE_DB_SUC_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHARGED_STAT_INITIALIZE_DB_SUC_ACK) == 8, "PROTO_NC_CHARGED_STAT_INITIALIZE_DB_SUC_ACK size drift");

struct PROTO_NC_CHARGED_STAT_INITIALIZE_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHARGED_STAT_INITIALIZE_FAIL_CMD) == 2, "PROTO_NC_CHARGED_STAT_INITIALIZE_FAIL_CMD size drift");

struct PROTO_NC_CHARGED_STAT_INITIALIZE_SUC_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHARGED_STAT_INITIALIZE_SUC_CMD) == 1, "PROTO_NC_CHARGED_STAT_INITIALIZE_SUC_CMD size drift");

struct PROTO_NC_CHARSAVE_ALL_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_ALL_ACK) == 6, "PROTO_NC_CHARSAVE_ALL_ACK size drift");

struct PROTO_NC_CHARSAVE_ABSTATE_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    uint8_t _pad_at_0006[2];
    ABSTATEREADBLOCK_______0_bytes___ abstate;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_ABSTATE_REQ) == 8, "PROTO_NC_CHARSAVE_ABSTATE_REQ size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_ABSTATE_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_ABSTATE_REQ abstate;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_ABSTATE_CMD) == 10, "PROTO_NC_CHARSAVE_2WLDMAN_ABSTATE_CMD size drift");

struct PROTO_NC_CHARSAVE_LEVEL_CMD { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_LEVEL_CMD) == 17, "PROTO_NC_CHARSAVE_LEVEL_CMD size drift");

struct PROTO_NC_CHARSAVE_LOCATION_CMD {
    uint8_t _pad_at_0000[4];
    Name3 map;
    uint8_t _pad_at_0004[12];
    SHINE_XY_TYPE coord;
    uint8_t _pad_at_0010[12];
    Name3 map_kq;
    uint8_t _pad_at_001c[12];
    SHINE_XY_TYPE coord_kq;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_LOCATION_CMD) == 48, "PROTO_NC_CHARSAVE_LOCATION_CMD size drift");

struct PROTO_NC_CHARSAVE_CHARSTAT_CMD {
    uint8_t _pad_at_0000[28];
    CHARSTATDISTSTR statdistribute;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_CHARSTAT_CMD) == 34, "PROTO_NC_CHARSAVE_CHARSTAT_CMD size drift");

struct PROTO_NC_CHARSAVE_ALL_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    PROTO_NC_CHARSAVE_LEVEL_CMD level;
    PROTO_NC_CHARSAVE_LOCATION_CMD location;
    PROTO_NC_CHARSAVE_CHARSTAT_CMD stat;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_ALL_REQ) == 105, "PROTO_NC_CHARSAVE_ALL_REQ size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_ALL_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_ALL_REQ all;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_ALL_CMD) == 107, "PROTO_NC_CHARSAVE_2WLDMAN_ALL_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_CHAT_COLOR_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_CHAT_COLOR_CMD) == 6, "PROTO_NC_CHARSAVE_2WLDMAN_CHAT_COLOR_CMD size drift");

struct PROTO_NC_CHAR_CHESTINFO_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    uint8_t _pad_at_0006[2];
    uint16_t loc[0];
};
static_assert(sizeof(PROTO_NC_CHAR_CHESTINFO_REQ) == 8, "PROTO_NC_CHAR_CHESTINFO_REQ size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_CHESTINFO_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_CHESTINFO_REQ chest;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_CHESTINFO_CMD) == 10, "PROTO_NC_CHARSAVE_2WLDMAN_CHESTINFO_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_COININFO_CMD { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_COININFO_CMD) == 22, "PROTO_NC_CHARSAVE_2WLDMAN_COININFO_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_GAME_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_GAMEDATA_CMD game;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_GAME_CMD) == 11, "PROTO_NC_CHARSAVE_2WLDMAN_GAME_CMD size drift");

struct PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_ACK {
    uint8_t _pad_at_0000[2];
    ActionCooltime_______0_bytes___ group;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_ACK) == 2, "PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_ACK size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_ITEMACTIONCOOLTIME_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_ACK ActionCoolTime;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_ITEMACTIONCOOLTIME_CMD) == 6, "PROTO_NC_CHARSAVE_2WLDMAN_ITEMACTIONCOOLTIME_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD___unnamed_type_PacketOrder_ {
    uint32_t  nPacketOrderNum;
    uint32_t  nIsLastOfInven;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD___unnamed_type_PacketOrder_) == 1, "PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD___unnamed_type_PacketOrder_ size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD___unnamed_type_PacketOrder_ PacketOrder;
    uint8_t _pad_at_0007[1];
    PROTO_ITEM_CMD itemstr;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD) == 9, "PROTO_NC_CHARSAVE_2WLDMAN_ITEM_CMD size drift");

struct PROTO_NC_CHAR_LINKFROM_CMD {
    uint8_t _pad_at_0000[4];
    Name3 map;
    uint8_t _pad_at_0004[12];
    SHINE_XY_TYPE coord;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHAR_LINKFROM_CMD) == 24, "PROTO_NC_CHAR_LINKFROM_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_LINK_FROM_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_LINKFROM_CMD linkfrom;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_LINK_FROM_CMD) == 26, "PROTO_NC_CHARSAVE_2WLDMAN_LINK_FROM_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_MISC_CMD { uint8_t data[18]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_MISC_CMD) == 18, "PROTO_NC_CHARSAVE_2WLDMAN_MISC_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_PET_LINK_RESUMMON_CMD { uint8_t data[23]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_PET_LINK_RESUMMON_CMD) == 23, "PROTO_NC_CHARSAVE_2WLDMAN_PET_LINK_RESUMMON_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD__tagQUESTDOING {
    uint8_t _pad_at_0000[6];
    PLAYER_QUEST_INFO_______0_bytes___ QuestDoingArray;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD__tagQUESTDOING) == 6, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD__tagQUESTDOING size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD__tagQUESTDOING questdoing;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD) == 8, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DOING_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD__tagQUESTDONE {
    uint8_t _pad_at_0000[12];
    PLAYER_QUEST_DONE_INFO_______0_bytes___ QuestDoneArray;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD__tagQUESTDONE) == 12, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD__tagQUESTDONE size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD__tagQUESTDONE questdone;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD) == 14, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_DONE_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD__tagQUESTREAD {
    uint8_t _pad_at_0000[6];
    uint16_t QuestReadIDArray[0];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD__tagQUESTREAD) == 6, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD__tagQUESTREAD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD__tagQUESTREAD questread;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD) == 8, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_READ_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD__tagQUESTREPEAT {
    uint8_t _pad_at_0000[6];
    PLAYER_QUEST_INFO_______0_bytes___ QuestRepeatArray;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD__tagQUESTREPEAT) == 6, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD__tagQUESTREPEAT size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD__tagQUESTREPEAT questrepeat;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD) == 8, "PROTO_NC_CHARSAVE_2WLDMAN_QUEST_REPEAT_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_SINGLE_OPTION_CMD { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_SINGLE_OPTION_CMD) == 17, "PROTO_NC_CHARSAVE_2WLDMAN_SINGLE_OPTION_CMD size drift");

struct PROTO_NC_CHARSAVE_SKILL_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    uint8_t _pad_at_0006[2];
    PROTO_SKILLREADBLOCK_______0_bytes___ skill;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_SKILL_REQ) == 8, "PROTO_NC_CHARSAVE_SKILL_REQ size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_SKILL_CMD {
    uint8_t _pad_at_0000[2];
    PARTMARK PartMark;
    uint8_t _pad_at_0002[3];
    PROTO_NC_CHARSAVE_SKILL_REQ skill;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_SKILL_CMD) == 13, "PROTO_NC_CHARSAVE_2WLDMAN_SKILL_CMD size drift");

struct PROTO_NC_CHARSAVE_2WLDMAN_TITLE_CMD {
    uint8_t _pad_at_0000[6];
    CHARACTER_TITLE_READBLOCK CTData;
    uint8_t _tail[1063];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_2WLDMAN_TITLE_CMD) == 1069, "PROTO_NC_CHARSAVE_2WLDMAN_TITLE_CMD size drift");

struct PROTO_NC_CHARSAVE_CHAT_COLOR_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_CHAT_COLOR_CMD) == 6, "PROTO_NC_CHARSAVE_CHAT_COLOR_CMD size drift");

struct PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_ACK) == 8, "PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_ACK size drift");

struct PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_REQ) == 7, "PROTO_NC_CHARSAVE_DB_UI_STATE_SAVE_REQ size drift");

struct PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_REQ {
    uint8_t _pad_at_0000[2];
    ActionCooltime_______0_bytes___ group;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_REQ) == 2, "PROTO_NC_CHARSAVE_ITEMACTIONCOOLTIME_REQ size drift");

struct PROTO_NC_CHARSAVE_PKCOUNT_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_PKCOUNT_CMD) == 8, "PROTO_NC_CHARSAVE_PKCOUNT_CMD size drift");

struct PROTO_NC_CHARSAVE_QUEST_DOING_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    uint8_t _pad_at_0006[2];
    PLAYER_QUEST_INFO_______0_bytes___ QuestDoingArray;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_QUEST_DOING_REQ) == 8, "PROTO_NC_CHARSAVE_QUEST_DOING_REQ size drift");

struct PROTO_NC_CHARSAVE_REST_EXP_LAST_EXEC_TIME_SAVE_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_REST_EXP_LAST_EXEC_TIME_SAVE_REQ) == 6, "PROTO_NC_CHARSAVE_REST_EXP_LAST_EXEC_TIME_SAVE_REQ size drift");

struct PROTO_NC_CHARSAVE_SELL_ITEM_INFO_CMD {
    uint8_t _pad_at_0000[7];
    PROTO_SELL_ITEM_INFO_SERVER_______0_bytes___ SellItemList;
};
static_assert(sizeof(PROTO_NC_CHARSAVE_SELL_ITEM_INFO_CMD) == 7, "PROTO_NC_CHARSAVE_SELL_ITEM_INFO_CMD size drift");

struct PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_DB_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_DB_CMD) == 12, "PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_DB_CMD size drift");

struct PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_WM_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_WM_CMD) == 14, "PROTO_NC_CHARSAVE_SET_CHAT_BLOCK_SPAMER_WM_CMD size drift");

struct PROTO_NC_CHARSAVE_TITLE_REQ {
    PROTO_NC_CHARSAVE_ALL_ACK handle;
    CHARACTER_TITLE_DB_SAVE CTData;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_TITLE_REQ) == 8, "PROTO_NC_CHARSAVE_TITLE_REQ size drift");

struct PROTO_NC_CHARSAVE_UI_STATE_SAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_UI_STATE_SAVE_ACK) == 2, "PROTO_NC_CHARSAVE_UI_STATE_SAVE_ACK size drift");

struct PROTO_NC_CHARSAVE_UI_STATE_SAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHARSAVE_UI_STATE_SAVE_REQ) == 1, "PROTO_NC_CHARSAVE_UI_STATE_SAVE_REQ size drift");

struct PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_DB_CMD {
    uint8_t _pad_at_0000[4];
    USEITEM_MINIMON_INFO UseItemMinimonInfo;
    uint8_t _pad_at_0004[50];
    uint8_t DelSlotList_Normal[12];
    uint8_t DelSlotList_Charged[12];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_DB_CMD) == 78, "PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_DB_CMD size drift");

struct PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_WORLD_CMD {
    uint8_t _pad_at_0000[6];
    USEITEM_MINIMON_INFO UseItemMinimonInfo;
    uint8_t _tail[50];
};
static_assert(sizeof(PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_WORLD_CMD) == 56, "PROTO_NC_CHARSAVE_USEITEM_MINIMON_INFO_WORLD_CMD size drift");

struct PROTO_NC_CHAR_ABSTATE_CMD {
    uint8_t _pad_at_0000[6];
    ABSTATEREADBLOCK_______0_bytes___ abstate;
};
static_assert(sizeof(PROTO_NC_CHAR_ABSTATE_CMD) == 6, "PROTO_NC_CHAR_ABSTATE_CMD size drift");

struct PROTO_NC_CHAR_ADMIN_LEVEL_INFORM_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_ADMIN_LEVEL_INFORM_CMD) == 1, "PROTO_NC_CHAR_ADMIN_LEVEL_INFORM_CMD size drift");

struct PROTO_NC_CHAR_ANI_FILE_CHECK_CMD {
    Name8 Checksum;
    uint8_t _pad_at_0000[32];
    Name8 SubDirectory;
    uint8_t _pad_at_0020[32];
    Name60Byte Filename;
    uint8_t _tail[60];
};
static_assert(sizeof(PROTO_NC_CHAR_ANI_FILE_CHECK_CMD) == 124, "PROTO_NC_CHAR_ANI_FILE_CHECK_CMD size drift");

struct PROTO_NC_CHAR_ARENA_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_ARENA_CMD) == 1, "PROTO_NC_CHAR_ARENA_CMD size drift");

struct PROTO_NC_CHAR_BASEPARAMCHANGE_CMD {
    uint8_t _pad_at_0000[1];
    CHAR_PARAMCHANGE_CMD_______0_bytes___ param;
};
static_assert(sizeof(PROTO_NC_CHAR_BASEPARAMCHANGE_CMD) == 1, "PROTO_NC_CHAR_BASEPARAMCHANGE_CMD size drift");

struct PROTO_NC_CHAR_BASE_CMD__LoginLocation {
    Name3 currentmap;
    uint8_t _pad_at_0000[12];
    SHINE_COORD_TYPE currentcoord;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_CHAR_BASE_CMD__LoginLocation) == 21, "PROTO_NC_CHAR_BASE_CMD__LoginLocation size drift");

struct PROTO_NC_CHAR_BASE_CMD {
    uint8_t _pad_at_0000[4];
    Name5 charid;
    uint8_t _pad_at_0004[62];
    PROTO_NC_CHAR_BASE_CMD__LoginLocation logininfo;
    CHARSTATDISTSTR statdistribute;
    uint8_t _pad_at_0057[14];
    PROTO_NC_CHAR_BASE_CMD___unnamed_type_flags_ flags;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_BASE_CMD) == 105, "PROTO_NC_CHAR_BASE_CMD size drift");

struct PROTO_NC_CHAR_BASE_CMD___unnamed_type_flags____unnamed_type_str_ {
    uint32_t  skillempower_can_reset;
    uint32_t  restunion;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_BASE_CMD___unnamed_type_flags____unnamed_type_str_) == 4, "PROTO_NC_CHAR_BASE_CMD___unnamed_type_flags____unnamed_type_str_ size drift");

struct PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD__CardBookmarkInfo_______0_bytes___ ViewList;
};
static_assert(sizeof(PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD) == 6, "PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD size drift");

struct PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD__CardBookmarkInfo { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD__CardBookmarkInfo) == 4, "PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD__CardBookmarkInfo size drift");

struct PROTO_NC_CHAR_CARDCOLLECT_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_NC_CHAR_CARDCOLLECT_CMD__CardInform_______0_bytes___ CardList;
};
static_assert(sizeof(PROTO_NC_CHAR_CARDCOLLECT_CMD) == 6, "PROTO_NC_CHAR_CARDCOLLECT_CMD size drift");

struct PROTO_NC_CHAR_CARDCOLLECT_CMD__CardInform { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_CHAR_CARDCOLLECT_CMD__CardInform) == 11, "PROTO_NC_CHAR_CARDCOLLECT_CMD__CardInform size drift");

struct PROTO_NC_CHAR_CARDCOLLECT_REWARD_CMD {
    uint8_t _pad_at_0000[6];
    uint16_t RewardID[0];
};
static_assert(sizeof(PROTO_NC_CHAR_CARDCOLLECT_REWARD_CMD) == 6, "PROTO_NC_CHAR_CARDCOLLECT_REWARD_CMD size drift");

struct PROTO_NC_CHAR_CENCHANGE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_CENCHANGE_CMD) == 8, "PROTO_NC_CHAR_CENCHANGE_CMD size drift");

struct PROTO_NC_CHAR_CHANGEBYCONDITION_PARAM_CMD {
    uint8_t _pad_at_0000[6];
    CHAR_CHANGEBYCONDITION_PARAM_______0_bytes___ aParam;
};
static_assert(sizeof(PROTO_NC_CHAR_CHANGEBYCONDITION_PARAM_CMD) == 6, "PROTO_NC_CHAR_CHANGEBYCONDITION_PARAM_CMD size drift");

struct PROTO_NC_CHAR_CHARDATAFAIL_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_CHARDATAFAIL_ACK) == 4, "PROTO_NC_CHAR_CHARDATAFAIL_ACK size drift");

struct PROTO_NC_CHAR_CHARDATA_ACK {
    NETPACKETHEADER netpacketheader;
};
static_assert(sizeof(PROTO_NC_CHAR_CHARDATA_ACK) == 2, "PROTO_NC_CHAR_CHARDATA_ACK size drift");

struct PROTO_NC_CHAR_CHARDATA_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_CHAR_CHARDATA_REQ) == 14, "PROTO_NC_CHAR_CHARDATA_REQ size drift");

struct PROTO_NC_CHAR_CHARGEDBUFF_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_CHARGEDBUFF_INFO_______0_bytes___ ChargedBuff;
};
static_assert(sizeof(PROTO_NC_CHAR_CHARGEDBUFF_CMD) == 2, "PROTO_NC_CHAR_CHARGEDBUFF_CMD size drift");

struct PROTO_NC_CHAR_CHARGEDBUFF_ERASE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_CHARGEDBUFF_ERASE_ACK) == 2, "PROTO_NC_CHAR_CHARGEDBUFF_ERASE_ACK size drift");

struct PROTO_NC_CHAR_CHARGEDBUFF_ERASE_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_CHARGEDBUFF_ERASE_REQ) == 4, "PROTO_NC_CHAR_CHARGEDBUFF_ERASE_REQ size drift");

struct PROTO_NC_CHAR_CHAT_COLOR_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_CHAT_COLOR_CMD) == 6, "PROTO_NC_CHAR_CHAT_COLOR_CMD size drift");

struct PROTO_NC_CHAR_CHESTINFO_CMD {
    uint8_t _pad_at_0000[2];
    uint16_t location[0];
};
static_assert(sizeof(PROTO_NC_CHAR_CHESTINFO_CMD) == 2, "PROTO_NC_CHAR_CHESTINFO_CMD size drift");

struct PROTO_NC_CHAR_CLASSCHANGE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_CLASSCHANGE_ACK) == 7, "PROTO_NC_CHAR_CLASSCHANGE_ACK size drift");

struct PROTO_NC_CHAR_CLASSCHANGE_CMD {
    NETPACKETZONEHEADER header;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_CHAR_CLASSCHANGE_CMD) == 9, "PROTO_NC_CHAR_CLASSCHANGE_CMD size drift");

struct PROTO_NC_CHAR_CLASSCHANGE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_CLASSCHANGE_REQ) == 8, "PROTO_NC_CHAR_CLASSCHANGE_REQ size drift");

struct PROTO_NC_CHAR_CLIENT_AUTO_PICK_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_AUTO_PICK_ACK) == 3, "PROTO_NC_CHAR_CLIENT_AUTO_PICK_ACK size drift");

struct PROTO_NC_CHAR_CLIENT_AUTO_PICK_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_AUTO_PICK_CMD) == 3, "PROTO_NC_CHAR_CLIENT_AUTO_PICK_CMD size drift");

struct PROTO_NC_CHAR_CLIENT_AUTO_PICK_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_AUTO_PICK_REQ) == 1, "PROTO_NC_CHAR_CLIENT_AUTO_PICK_REQ size drift");

struct PROTO_NC_CHAR_CLIENT_CHARTITLE_CMD {
    uint8_t _pad_at_0000[6];
    CT_INFO_______0_bytes___ TitleArray;
};
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_CHARTITLE_CMD) == 6, "PROTO_NC_CHAR_CLIENT_CHARTITLE_CMD size drift");

struct PROTO_NC_CHAR_CLIENT_ITEM_CMD___unnamed_type_flag_ {
    uint32_t  invenclear;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_ITEM_CMD___unnamed_type_flag_) == 1, "PROTO_NC_CHAR_CLIENT_ITEM_CMD___unnamed_type_flag_ size drift");

struct PROTO_NC_CHAR_CLIENT_ITEM_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_CLIENT_ITEM_CMD___unnamed_type_flag_ flag;
    PROTO_ITEMPACKET_INFORM_______0_bytes___ ItemArray;
};
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_ITEM_CMD) == 3, "PROTO_NC_CHAR_CLIENT_ITEM_CMD size drift");

struct PROTO_NC_CHAR_SKILLCLIENT_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_SKILLREADBLOCKCLIENT_______0_bytes___ skill;
};
static_assert(sizeof(PROTO_NC_CHAR_SKILLCLIENT_CMD) == 6, "PROTO_NC_CHAR_SKILLCLIENT_CMD size drift");

struct PROTO_NC_CHAR_CLIENT_SKILL_CMD {
    uint8_t _pad_at_0000[1];
    PARTMARK PartMark;
    uint8_t _pad_at_0001[3];
    PROTO_NC_CHAR_SKILLCLIENT_CMD skill;
};
static_assert(sizeof(PROTO_NC_CHAR_CLIENT_SKILL_CMD) == 10, "PROTO_NC_CHAR_CLIENT_SKILL_CMD size drift");

struct PROTO_NC_CHAR_COININFO_CMD { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_CHAR_COININFO_CMD) == 16, "PROTO_NC_CHAR_COININFO_CMD size drift");

struct PROTO_NC_CHAR_DATATRANSMISSION_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Proto_CharDataStruct data;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_DATATRANSMISSION_RNG) == 27, "PROTO_NC_CHAR_DATATRANSMISSION_RNG size drift");

struct PROTO_NC_CHAR_DB_AUTO_PICK_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_CHAR_DB_AUTO_PICK_CMD) == 5, "PROTO_NC_CHAR_DB_AUTO_PICK_CMD size drift");

struct PROTO_NC_CHAR_DB_NEWBIE_GUIDE_VIEW_SET_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_CHAR_DB_NEWBIE_GUIDE_VIEW_SET_CMD) == 5, "PROTO_NC_CHAR_DB_NEWBIE_GUIDE_VIEW_SET_CMD size drift");

struct PROTO_NC_CHAR_DB_REST_EXP_LAST_EXEC_TIME_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_DB_REST_EXP_LAST_EXEC_TIME_CMD) == 8, "PROTO_NC_CHAR_DB_REST_EXP_LAST_EXEC_TIME_CMD size drift");

struct PROTO_NC_CHAR_DEADMENU_CMD {
    uint8_t _pad_at_0000[5];
    DeadMenuType eMenuType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_DEADMENU_CMD) == 9, "PROTO_NC_CHAR_DEADMENU_CMD size drift");

struct PROTO_NC_CHAR_EMBLEM_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_EMBLEM_CMD) == 1, "PROTO_NC_CHAR_EMBLEM_CMD size drift");

struct PROTO_NC_CHAR_EMPTY_INSTANCE_DUNGEON_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[7];
    INSTANCE_DUNGEON__CATEGORY Category;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_EMPTY_INSTANCE_DUNGEON_RNG) == 11, "PROTO_NC_CHAR_EMPTY_INSTANCE_DUNGEON_RNG size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_CHANGE_DAY_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_CHANGE_DAY_CMD) == 1, "PROTO_NC_CHAR_EVENT_ATTENDANCE_CHANGE_DAY_CMD size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_CMD) == 1, "PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_CMD size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_ACK) == 6, "PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_ACK size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_REQ {
    uint8_t _pad_at_0000[4];
    tm tCheckTime;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_REQ) == 40, "PROTO_NC_CHAR_EVENT_ATTENDANCE_CHECK_DB_REQ size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_CLIENT_CMD {
    tm tEventStart;
    uint8_t _pad_at_0000[37];
    uint8_t AttendanceArray[28];
    uint8_t _pad_at_0041[1];
    uint8_t RewardList[0];
};
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_CLIENT_CMD) == 66, "PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_CLIENT_CMD size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_ACK {
    uint8_t _pad_at_0000[11];
    tm_______1008_bytes___ AttendanceArray;
    uint8_t _pad_at_000b[1009];
    uint8_t RewardList[0];
};
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_ACK) == 1020, "PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_ACK size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_REQ {
    uint8_t _pad_at_0000[4];
    tm tmEventStartDate;
    uint8_t _pad_at_0004[36];
    tm tmEventEndDate;
    uint8_t _tail[40];
};
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_REQ) == 80, "PROTO_NC_CHAR_EVENT_ATTENDANCE_LIST_DB_REQ size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_ACK) == 4, "PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_ACK size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_ACK) == 8, "PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_ACK size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_REQ {
    uint8_t _pad_at_0000[5];
    tm tmEventDay;
    uint8_t _pad_at_0005[37];
    tm tmCheckStartDay;
    uint8_t _pad_at_002a[36];
    tm tmCheckEndDay;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_REQ) == 114, "PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_DB_REQ size drift");

struct PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_REQ) == 2, "PROTO_NC_CHAR_EVENT_ATTENDANCE_REWARD_REQ size drift");

struct PROTO_NC_CHAR_EXP_CHANGED_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_CHAR_EXP_CHANGED_CMD) == 14, "PROTO_NC_CHAR_EXP_CHANGED_CMD size drift");

struct PROTO_NC_CHAR_FAMECHANGE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_FAMECHANGE_CMD) == 4, "PROTO_NC_CHAR_FAMECHANGE_CMD size drift");

struct PROTO_NC_CHAR_FAMESAVE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_FAMESAVE_CMD) == 8, "PROTO_NC_CHAR_FAMESAVE_CMD size drift");

struct PROTO_NC_CHAR_FREESTAT_SET_DB_ACK {
    NETPACKETZONEHEADER header;
    uint8_t nFreeStat[5];
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_FREESTAT_SET_DB_ACK) == 15, "PROTO_NC_CHAR_FREESTAT_SET_DB_ACK size drift");

struct PROTO_NC_CHAR_FREESTAT_SET_DB_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[1];
    uint8_t nFreeStat[5];
    uint8_t _pad_at_000c[3];
    PROTO_NC_CHAR_FREESTAT_SET_DB_REQ___unnamed_type_DecItemInfo________0_bytes___ DecItemInfo;
};
static_assert(sizeof(PROTO_NC_CHAR_FREESTAT_SET_DB_REQ) == 15, "PROTO_NC_CHAR_FREESTAT_SET_DB_REQ size drift");

struct PROTO_NC_CHAR_FREESTAT_SET_DB_REQ___unnamed_type_DecItemInfo_ {
    uint8_t _pad_at_0000[1];
    SHINE_ITEM_REGISTNUMBER nItmeKey;
    uint8_t _tail[11];
};
static_assert(sizeof(PROTO_NC_CHAR_FREESTAT_SET_DB_REQ___unnamed_type_DecItemInfo_) == 12, "PROTO_NC_CHAR_FREESTAT_SET_DB_REQ___unnamed_type_DecItemInfo_ size drift");

struct PROTO_NC_CHAR_FRIEND_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_FRIEND_CMD_INFO_______0_bytes___ friend_array;
};
static_assert(sizeof(PROTO_NC_CHAR_FRIEND_CMD) == 2, "PROTO_NC_CHAR_FRIEND_CMD size drift");

struct PROTO_NC_CHAR_FRIEND_CMD_INFO {
    uint8_t _pad_at_0000[4];
    Name5 charid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_CHAR_FRIEND_CMD_INFO) == 25, "PROTO_NC_CHAR_FRIEND_CMD_INFO size drift");

struct PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_CMD) == 8, "PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_CMD size drift");

struct PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_DB_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_DB_CMD) == 12, "PROTO_NC_CHAR_GET_CHAT_BLOCK_SPAMER_DB_CMD size drift");

struct PROTO_NC_CHAR_ITEM_CMD___unnamed_type_PacketOrder_ {
    uint32_t  nPacketOrderNum;
    uint32_t  nIsLastOfInven;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_ITEM_CMD___unnamed_type_PacketOrder_) == 1, "PROTO_NC_CHAR_ITEM_CMD___unnamed_type_PacketOrder_ size drift");

struct PROTO_NC_CHAR_ITEM_CMD {
    PROTO_NC_CHAR_ITEM_CMD___unnamed_type_PacketOrder_ PacketOrder;
    uint8_t _pad_at_0001[1];
    PROTO_ITEM_CMD ItemCmd;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_ITEM_CMD) == 3, "PROTO_NC_CHAR_ITEM_CMD size drift");

struct PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[7];
    PROTO_NC_CHAR_ITEM_CMD ItemCmd;
};
static_assert(sizeof(PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_ACK) == 16, "PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_ACK size drift");

struct PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[7];
    PROTO_NC_CHAR_ITEM_CMD ItemCmd;
};
static_assert(sizeof(PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_ACK) == 16, "PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_ACK size drift");

struct PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_REQ) == 16, "PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_NUM_REQ size drift");

struct PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_REQ) == 11, "PROTO_NC_CHAR_GET_ITEMLIST_BY_TYPE_REQ size drift");

struct PROTO_NC_CHAR_GUILD_ACADEMY_CMD {
    uint8_t _pad_at_0000[5];
    GUILD_ACADEMY_CLIENT_______0_bytes___ GuildAcademy;
};
static_assert(sizeof(PROTO_NC_CHAR_GUILD_ACADEMY_CMD) == 5, "PROTO_NC_CHAR_GUILD_ACADEMY_CMD size drift");

struct PROTO_NC_CHAR_GUILD_ACADEMY_ZONE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_GUILD_ACADEMY_ZONE_CMD) == 4, "PROTO_NC_CHAR_GUILD_ACADEMY_ZONE_CMD size drift");

struct PROTO_NC_CHAR_GUILD_CMD {
    uint8_t _pad_at_0000[4];
    GUILD_CLIENT_______0_bytes___ Guild;
};
static_assert(sizeof(PROTO_NC_CHAR_GUILD_CMD) == 4, "PROTO_NC_CHAR_GUILD_CMD size drift");

struct PROTO_NC_CHAR_HOUSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_HOUSE_CMD) == 1, "PROTO_NC_CHAR_HOUSE_CMD size drift");

struct PROTO_NC_CHAR_KICKPLAYEROUT_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[8];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_KICKPLAYEROUT_RNG) == 28, "PROTO_NC_CHAR_KICKPLAYEROUT_RNG size drift");

struct PROTO_NC_CHAR_KQMAP_CMD {
    uint8_t _pad_at_0000[4];
    Name3 sKQMapName;
    uint8_t _pad_at_0004[12];
    SHINE_XY_TYPE nKQCoord;
    uint8_t _pad_at_0010[8];
    SHINE_DATETIME dKQDate;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_KQMAP_CMD) == 28, "PROTO_NC_CHAR_KQMAP_CMD size drift");

struct PROTO_NC_CHAR_LEVEL_CHANGED_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_CHAR_LEVEL_CHANGED_CMD) == 7, "PROTO_NC_CHAR_LEVEL_CHANGED_CMD size drift");

struct PROTO_NC_CHAR_LOGINFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_LOGINFAIL_ACK) == 2, "PROTO_NC_CHAR_LOGINFAIL_ACK size drift");

struct PROTO_NC_CHAR_LOGIN_ACK {
    Name4 zoneip;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_CHAR_LOGIN_ACK) == 18, "PROTO_NC_CHAR_LOGIN_ACK size drift");

struct PROTO_NC_CHAR_LOGIN_DB {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[32];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_LOGIN_DB) == 56, "PROTO_NC_CHAR_LOGIN_DB size drift");

struct PROTO_NC_CHAR_LOGIN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_LOGIN_REQ) == 1, "PROTO_NC_CHAR_LOGIN_REQ size drift");

struct PROTO_NC_CHAR_LOGOUT_DB {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[31];
};
static_assert(sizeof(PROTO_NC_CHAR_LOGOUT_DB) == 35, "PROTO_NC_CHAR_LOGOUT_DB size drift");

struct PROTO_NC_CHAR_MAPLOGIN_ACK {
    uint8_t _pad_at_0000[2];
    CHAR_PARAMETER_DATA param;
    uint8_t _pad_at_0002[232];
    SHINE_XY_TYPE logincoord;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHAR_MAPLOGIN_ACK) == 242, "PROTO_NC_CHAR_MAPLOGIN_ACK size drift");

struct PROTO_NC_CHAR_MASPUP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_MASPUP_CMD) == 1, "PROTO_NC_CHAR_MASPUP_CMD size drift");

struct PROTO_NC_CHAR_MYSTERYVAULT_UI_STATE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_MYSTERYVAULT_UI_STATE_CMD) == 1, "PROTO_NC_CHAR_MYSTERYVAULT_UI_STATE_CMD size drift");

struct PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_LIST_CMD {
    uint8_t _pad_at_0000[1];
    uint8_t nGuideViewList[0];
};
static_assert(sizeof(PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_LIST_CMD) == 1, "PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_LIST_CMD size drift");

struct PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_SET_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_SET_CMD) == 1, "PROTO_NC_CHAR_NEWBIE_GUIDE_VIEW_SET_CMD size drift");

struct PROTO_NC_CHAR_OPTION_GAME {
    uint8_t Data[64];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GAME) == 64, "PROTO_NC_CHAR_OPTION_GAME size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_GAME_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_GAME Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_GAME_ACK) == 67, "PROTO_NC_CHAR_OPTION_DB_GET_GAME_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_GAME_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_GAME_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_GAME_REQ size drift");

struct PROTO_NC_CHAR_OPTION_KEYMAPPING {
    uint8_t Data[308];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_KEYMAPPING) == 308, "PROTO_NC_CHAR_OPTION_KEYMAPPING size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_KEYMAPPING Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_ACK) == 311, "PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_KEYMAPPING_REQ size drift");

struct PROTO_NC_CHAR_OPTION_SHORTCUTDATA {
    uint8_t Data[1024];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SHORTCUTDATA) == 1024, "PROTO_NC_CHAR_OPTION_SHORTCUTDATA size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_SHORTCUTDATA Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_ACK) == 1027, "PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_SHORTCUTSIZE {
    uint8_t Data[24];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SHORTCUTSIZE) == 24, "PROTO_NC_CHAR_OPTION_SHORTCUTSIZE size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_SHORTCUTSIZE Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_ACK) == 27, "PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_SHORTCUTSIZE_REQ size drift");

struct PROTO_NC_CHAR_OPTION_SOUND {
    uint8_t Data[1];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SOUND) == 1, "PROTO_NC_CHAR_OPTION_SOUND size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SOUND_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_SOUND Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SOUND_ACK) == 4, "PROTO_NC_CHAR_OPTION_DB_GET_SOUND_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_SOUND_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_SOUND_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_SOUND_REQ size drift");

struct PROTO_NC_CHAR_OPTION_VIDEO {
    uint8_t Data[60];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_VIDEO) == 60, "PROTO_NC_CHAR_OPTION_VIDEO size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_VIDEO Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_ACK) == 63, "PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_VIDEO_REQ size drift");

struct PROTO_NC_CHAR_OPTION_WINDOWPOS {
    uint8_t Data[392];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_WINDOWPOS) == 392, "PROTO_NC_CHAR_OPTION_WINDOWPOS size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_CHAR_OPTION_WINDOWPOS Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_ACK) == 395, "PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_ACK size drift");

struct PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_REQ) == 6, "PROTO_NC_CHAR_OPTION_DB_GET_WINDOWPOS_REQ size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_GAME_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_GAME Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_GAME_CMD) == 68, "PROTO_NC_CHAR_OPTION_DB_SET_GAME_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_KEYMAPPING_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_KEYMAPPING Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_KEYMAPPING_CMD) == 312, "PROTO_NC_CHAR_OPTION_DB_SET_KEYMAPPING_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTDATA_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_SHORTCUTDATA Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTDATA_CMD) == 1028, "PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTDATA_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTSIZE_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_SHORTCUTSIZE Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTSIZE_CMD) == 28, "PROTO_NC_CHAR_OPTION_DB_SET_SHORTCUTSIZE_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_SOUND_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_SOUND Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_SOUND_CMD) == 5, "PROTO_NC_CHAR_OPTION_DB_SET_SOUND_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_VIDEO_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_VIDEO Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_VIDEO_CMD) == 64, "PROTO_NC_CHAR_OPTION_DB_SET_VIDEO_CMD size drift");

struct PROTO_NC_CHAR_OPTION_DB_SET_WINDOWPOS_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_WINDOWPOS Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_DB_SET_WINDOWPOS_CMD) == 396, "PROTO_NC_CHAR_OPTION_DB_SET_WINDOWPOS_CMD size drift");

struct PROTO_NC_CHAR_OPTION_GET_ALL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_ALL_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_ALL_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_GAME_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_GAME Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_GAME_ACK) == 65, "PROTO_NC_CHAR_OPTION_GET_GAME_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_GAME_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_GAME_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_GAME_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_KEYMAPPING Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_ACK) == 309, "PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_KEYMAPPING_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_SHORTCUTDATA Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_ACK) == 1025, "PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_SHORTCUTSIZE Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_ACK) == 25, "PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_SHORTCUTSIZE_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_SOUND_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_SOUND Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SOUND_ACK) == 2, "PROTO_NC_CHAR_OPTION_GET_SOUND_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_SOUND_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_SOUND_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_SOUND_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_VIDEO_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_VIDEO Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_VIDEO_ACK) == 61, "PROTO_NC_CHAR_OPTION_GET_VIDEO_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_VIDEO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_VIDEO_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_VIDEO_REQ size drift");

struct PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_OPTION_WINDOWPOS Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_ACK) == 393, "PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_ACK size drift");

struct PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_REQ) == 1, "PROTO_NC_CHAR_OPTION_GET_WINDOWPOS_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_DATA_TYPE_CMD {
    uint8_t _pad_at_0000[4];
    CHAR_OPTION_DATA_TYPE DataType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_DATA_TYPE_CMD) == 8, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_DATA_TYPE_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC3_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC3_CMD) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC3_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC4_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC4_CMD) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_ETC4_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD {
    uint8_t _pad_at_0000[2];
    GAME_OPTION_DATA_______0_bytes___ GameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_GAMEOPTION_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD DBGameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_GAMEOPTION_CMD) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_GAMEOPTION_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD {
    uint8_t _pad_at_0000[2];
    KEY_MAP_DATA_______0_bytes___ KeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_KEYMAP_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD DBKeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_KEYMAP_CMD) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_KEYMAP_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_GET_SHORTCUTDATA_CMD {
    uint8_t _pad_at_0000[2];
    SHORT_CUT_DATA_______0_bytes___ ShortCutData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_GET_SHORTCUTDATA_CMD) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_GET_SHORTCUTDATA_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_SHORTCUTDATA_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_SHORTCUTDATA_CMD DBShortCutData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_SHORTCUTDATA_CMD) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_GET_SHORTCUTDATA_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC3_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_ETC4_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_ACK {
    uint8_t _pad_at_0000[6];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD DBGameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_ACK) == 8, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_REQ) == 4, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_GAMEOPTION_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_ACK {
    uint8_t _pad_at_0000[6];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD DBKeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_ACK) == 8, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_REQ) == 4, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_KEYMAP_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_INIT_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC3_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_ETC4_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_ACK) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD DBGameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_REQ) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_GAMEOPTION_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_ACK) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD DBKeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_REQ) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_KEYMAP_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_ACK) == 6, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_REQ {
    uint8_t _pad_at_0000[1];
    SHORT_CUT_DATA_______0_bytes___ ShortCutData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_REQ DBShortCutData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_REQ) == 5, "PROTO_NC_CHAR_OPTION_IMPROVE_DB_SET_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC3_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC3_CMD) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC3_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC4_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC4_CMD) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_GET_ETC4_CMD size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC3_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_ETC4_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_GAMEOPTION_CMD DBGameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_ACK) == 4, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_GAMEOPTION_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_CHAR_OPTION_IMPROVE_GET_KEYMAP_CMD DBKeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_ACK) == 4, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_KEYMAP_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_INIT_SHORTCUTDATA_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC3_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_ACK) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_REQ) == 1, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_ETC4_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_ACK) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_REQ {
    uint8_t _pad_at_0000[2];
    GAME_OPTION_DATA_______0_bytes___ GameOptionData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_REQ) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_GAMEOPTION_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_ACK) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_ACK size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_REQ {
    uint8_t _pad_at_0000[2];
    KEY_MAP_DATA_______0_bytes___ KeyMapData;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_REQ) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_KEYMAP_REQ size drift");

struct PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_ACK) == 2, "PROTO_NC_CHAR_OPTION_IMPROVE_SET_SHORTCUTDATA_ACK size drift");

struct PROTO_NC_CHAR_OPTION_LOGIN_BLOCKDATA_ERR_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_OPTION_LOGIN_BLOCKDATA_ERR_REQ) == 4, "PROTO_NC_CHAR_OPTION_LOGIN_BLOCKDATA_ERR_REQ size drift");

struct PROTO_NC_CHAR_OPTION_SET_GAME_CMD {
    PROTO_NC_CHAR_OPTION_GAME Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_GAME_CMD) == 64, "PROTO_NC_CHAR_OPTION_SET_GAME_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_KEYMAPPING_CMD {
    PROTO_NC_CHAR_OPTION_KEYMAPPING Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_KEYMAPPING_CMD) == 308, "PROTO_NC_CHAR_OPTION_SET_KEYMAPPING_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_SHORTCUTDATA_CMD {
    PROTO_NC_CHAR_OPTION_SHORTCUTDATA Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_SHORTCUTDATA_CMD) == 1024, "PROTO_NC_CHAR_OPTION_SET_SHORTCUTDATA_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_SHORTCUTSIZE_CMD {
    PROTO_NC_CHAR_OPTION_SHORTCUTSIZE Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_SHORTCUTSIZE_CMD) == 24, "PROTO_NC_CHAR_OPTION_SET_SHORTCUTSIZE_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_SOUND_CMD {
    PROTO_NC_CHAR_OPTION_SOUND Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_SOUND_CMD) == 1, "PROTO_NC_CHAR_OPTION_SET_SOUND_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_VIDEO_CMD {
    PROTO_NC_CHAR_OPTION_VIDEO Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_VIDEO_CMD) == 60, "PROTO_NC_CHAR_OPTION_SET_VIDEO_CMD size drift");

struct PROTO_NC_CHAR_OPTION_SET_WINDOWPOS_CMD {
    PROTO_NC_CHAR_OPTION_WINDOWPOS Data;
};
static_assert(sizeof(PROTO_NC_CHAR_OPTION_SET_WINDOWPOS_CMD) == 392, "PROTO_NC_CHAR_OPTION_SET_WINDOWPOS_CMD size drift");

struct PROTO_NC_CHAR_PET_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_PET_CMD) == 1, "PROTO_NC_CHAR_PET_CMD size drift");

struct PROTO_NC_CHAR_PLAYERBANNED_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERBANNED_RNG) == 27, "PROTO_NC_CHAR_PLAYERBANNED_RNG size drift");

struct PROTO_NC_CHAR_PLAYERFOUND_BY_NORMAL_USER_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name3 map;
    uint8_t _pad_at_0007[12];
    SHINE_XY_TYPE coord;
    uint8_t _pad_at_0013[8];
    Name5 charid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERFOUND_BY_NORMAL_USER_RNG) == 49, "PROTO_NC_CHAR_PLAYERFOUND_BY_NORMAL_USER_RNG size drift");

struct PROTO_NC_CHAR_PLAYERFOUND_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name3 map;
    uint8_t _pad_at_0007[12];
    SHINE_XY_TYPE coord;
    uint8_t _pad_at_0013[8];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERFOUND_RNG) == 47, "PROTO_NC_CHAR_PLAYERFOUND_RNG size drift");

struct PROTO_NC_CHAR_PLAYERSEARCH_BY_NORMAL_USER_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERSEARCH_BY_NORMAL_USER_RNG) == 27, "PROTO_NC_CHAR_PLAYERSEARCH_BY_NORMAL_USER_RNG size drift");

struct PROTO_NC_CHAR_PLAYERSEARCH_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERSEARCH_RNG) == 27, "PROTO_NC_CHAR_PLAYERSEARCH_RNG size drift");

struct PROTO_NC_CHAR_PLAYERSUMMON_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[7];
    Name5 charid;
    uint8_t _pad_at_0007[20];
    Name3 map;
    uint8_t _pad_at_001b[12];
    SHINE_XY_TYPE coord;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHAR_PLAYERSUMMON_RNG) == 47, "PROTO_NC_CHAR_PLAYERSUMMON_RNG size drift");

struct PROTO_NC_CHAR_POLYMORPH_CMD {
    uint8_t _pad_at_0000[4];
    ABSTATEINDEX AbstateIndex;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_POLYMORPH_CMD) == 8, "PROTO_NC_CHAR_POLYMORPH_CMD size drift");

struct PROTO_NC_CHAR_PROMOTE_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_PROMOTE_ACK) == 1, "PROTO_NC_CHAR_PROMOTE_ACK size drift");

struct PROTO_NC_CHAR_PROMOTE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_PROMOTE_REQ) == 1, "PROTO_NC_CHAR_PROMOTE_REQ size drift");

struct PROTO_NC_CHAR_QUEST_DOING_CMD {
    uint8_t _pad_at_0000[6];
    PLAYER_QUEST_INFO_______0_bytes___ QuestDoingArray;
};
static_assert(sizeof(PROTO_NC_CHAR_QUEST_DOING_CMD) == 6, "PROTO_NC_CHAR_QUEST_DOING_CMD size drift");

struct PROTO_NC_CHAR_QUEST_DONE_CMD {
    uint8_t _pad_at_0000[12];
    PLAYER_QUEST_DONE_INFO_______0_bytes___ QuestDoneArray;
};
static_assert(sizeof(PROTO_NC_CHAR_QUEST_DONE_CMD) == 12, "PROTO_NC_CHAR_QUEST_DONE_CMD size drift");

struct PROTO_NC_CHAR_QUEST_READ_CMD {
    uint8_t _pad_at_0000[6];
    uint16_t QuestReadIDArray[0];
};
static_assert(sizeof(PROTO_NC_CHAR_QUEST_READ_CMD) == 6, "PROTO_NC_CHAR_QUEST_READ_CMD size drift");

struct PROTO_NC_CHAR_QUEST_REPEAT_CMD {
    uint8_t _pad_at_0000[6];
    PLAYER_QUEST_INFO_______0_bytes___ QuestRepeatArray;
};
static_assert(sizeof(PROTO_NC_CHAR_QUEST_REPEAT_CMD) == 6, "PROTO_NC_CHAR_QUEST_REPEAT_CMD size drift");

struct PROTO_NC_CHAR_REBIRTH_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_REBIRTH_CMD) == 1, "PROTO_NC_CHAR_REBIRTH_CMD size drift");

struct PROTO_NC_CHAR_REBIRTH_REJECT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_REBIRTH_REJECT_CMD) == 1, "PROTO_NC_CHAR_REBIRTH_REJECT_CMD size drift");

struct PROTO_NC_CHAR_ZONE_CHARDATA_REQ {
    uint8_t _pad_at_0000[2];
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAR_ZONE_CHARDATA_REQ) == 22, "PROTO_NC_CHAR_ZONE_CHARDATA_REQ size drift");

struct PROTO_NC_CHAR_REGISTNUMBER_ACK {
    uint8_t _pad_at_0000[8];
    PROTO_NC_CHAR_ZONE_CHARDATA_REQ loginreq;
};
static_assert(sizeof(PROTO_NC_CHAR_REGISTNUMBER_ACK) == 30, "PROTO_NC_CHAR_REGISTNUMBER_ACK size drift");

struct PROTO_NC_CHAR_REGISTNUMBER_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER netpacketzoneheader;
    PROTO_NC_CHAR_REGISTNUMBER_ACK ack;
};
static_assert(sizeof(PROTO_NC_CHAR_REGISTNUMBER_ACK_SEND) == 39, "PROTO_NC_CHAR_REGISTNUMBER_ACK_SEND size drift");

struct PROTO_NC_CHAR_REGISTNUMBER_REQ {
    PROTO_NC_CHAR_ZONE_CHARDATA_REQ loginreq;
};
static_assert(sizeof(PROTO_NC_CHAR_REGISTNUMBER_REQ) == 22, "PROTO_NC_CHAR_REGISTNUMBER_REQ size drift");

struct PROTO_NC_CHAR_REGNUM_VARIFICATION_ACK { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_CHAR_REGNUM_VARIFICATION_ACK) == 7, "PROTO_NC_CHAR_REGNUM_VARIFICATION_ACK size drift");

struct PROTO_NC_CHAR_REGNUM_VARIFICATION_REQ {
    uint8_t _pad_at_0000[2];
    Name5 zone_charid;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_CHAR_REGNUM_VARIFICATION_REQ) == 26, "PROTO_NC_CHAR_REGNUM_VARIFICATION_REQ size drift");

struct PROTO_NC_CHAR_REVIVESAME_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE location;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHAR_REVIVESAME_CMD) == 10, "PROTO_NC_CHAR_REVIVESAME_CMD size drift");

struct PROTO_NC_CHAR_REVIVEOTHER_CMD {
    PROTO_NC_CHAR_REVIVESAME_CMD link;
    PROTO_NC_CHAR_LOGIN_ACK sock;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_REVIVEOTHER_CMD) == 30, "PROTO_NC_CHAR_REVIVEOTHER_CMD size drift");

struct PROTO_NC_CHAR_SEAWAR_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_SEAWAR_CMD) == 1, "PROTO_NC_CHAR_SEAWAR_CMD size drift");

struct PROTO_NC_CHAR_SELL_ITEM_INFO_ZONE_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_SELL_ITEM_INFO_SERVER_______0_bytes___ SellItemList;
};
static_assert(sizeof(PROTO_NC_CHAR_SELL_ITEM_INFO_ZONE_CMD) == 1, "PROTO_NC_CHAR_SELL_ITEM_INFO_ZONE_CMD size drift");

struct PROTO_NC_CHAR_SET_STYLE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_ACK) == 2, "PROTO_NC_CHAR_SET_STYLE_ACK size drift");

struct PROTO_NC_CHAR_SET_STYLE_DB_ACK {
    uint8_t _pad_at_0000[8];
    PROTO_AVATAR_SHAPE_INFO Info;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_DB_ACK) == 12, "PROTO_NC_CHAR_SET_STYLE_DB_ACK size drift");

struct PROTO_NC_CHAR_SET_STYLE_DB_REQ {
    uint8_t _pad_at_0000[6];
    PROTO_AVATAR_SHAPE_INFO Info;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_DB_REQ) == 10, "PROTO_NC_CHAR_SET_STYLE_DB_REQ size drift");

struct PROTO_NC_CHAR_SET_STYLE_GET_INFO_ACK {
    uint8_t _pad_at_0000[2];
    STYLE_ITEM_COUNTS_______24_bytes___ Items;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_GET_INFO_ACK) == 26, "PROTO_NC_CHAR_SET_STYLE_GET_INFO_ACK size drift");

struct PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_ACK {
    uint8_t _pad_at_0000[8];
    STYLE_ITEM_COUNTS_______24_bytes___ Items;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_ACK) == 32, "PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_ACK size drift");

struct PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_REQ) == 6, "PROTO_NC_CHAR_SET_STYLE_GET_INFO_DB_REQ size drift");

struct PROTO_NC_CHAR_SET_STYLE_GET_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_GET_INFO_REQ) == 1, "PROTO_NC_CHAR_SET_STYLE_GET_INFO_REQ size drift");

struct PROTO_NC_CHAR_SET_STYLE_REQ {
    PROTO_AVATAR_SHAPE_INFO Info;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_SET_STYLE_REQ) == 4, "PROTO_NC_CHAR_SET_STYLE_REQ size drift");

struct PROTO_NC_CHAR_SINGLE_OPTION_CMD { uint8_t data[15]; };
static_assert(sizeof(PROTO_NC_CHAR_SINGLE_OPTION_CMD) == 15, "PROTO_NC_CHAR_SINGLE_OPTION_CMD size drift");

struct PROTO_NC_CHAR_SKILL_CMD {
    uint8_t _pad_at_0000[4];
    PARTMARK PartMark;
    uint8_t _pad_at_0004[5];
    PROTO_SKILLREADBLOCK_______0_bytes___ skill;
};
static_assert(sizeof(PROTO_NC_CHAR_SKILL_CMD) == 9, "PROTO_NC_CHAR_SKILL_CMD size drift");

struct PROTO_NC_CHAR_SKILL_PASSIVE_CMD {
    uint8_t _pad_at_0000[2];
    uint16_t passive[0];
};
static_assert(sizeof(PROTO_NC_CHAR_SKILL_PASSIVE_CMD) == 2, "PROTO_NC_CHAR_SKILL_PASSIVE_CMD size drift");

struct PROTO_NC_CHAR_SOMEONEGUILDACADEMYCHANGE_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_SOMEONEGUILDACADEMYCHANGE_CMD) == 6, "PROTO_NC_CHAR_SOMEONEGUILDACADEMYCHANGE_CMD size drift");

struct PROTO_NC_CHAR_SOMEONEGUILDCHANGE_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_SOMEONEGUILDCHANGE_CMD) == 6, "PROTO_NC_CHAR_SOMEONEGUILDCHANGE_CMD size drift");

struct PROTO_NC_CHAR_SOMEONEPROMOTE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_CHAR_SOMEONEPROMOTE_CMD) == 3, "PROTO_NC_CHAR_SOMEONEPROMOTE_CMD size drift");

struct PROTO_NC_CHAR_STAT_DECPOINTFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINTFAIL_ACK) == 2, "PROTO_NC_CHAR_STAT_DECPOINTFAIL_ACK size drift");

struct PROTO_NC_CHAR_STAT_DECPOINTFAIL_DB_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINTFAIL_DB_ACK) == 10, "PROTO_NC_CHAR_STAT_DECPOINTFAIL_DB_ACK size drift");

struct PROTO_NC_CHAR_STAT_DECPOINTSUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINTSUC_ACK) == 1, "PROTO_NC_CHAR_STAT_DECPOINTSUC_ACK size drift");

struct PROTO_NC_CHAR_STAT_DECPOINTSUC_DB_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINTSUC_DB_ACK) == 9, "PROTO_NC_CHAR_STAT_DECPOINTSUC_DB_ACK size drift");

struct PROTO_NC_CHAR_STAT_DECPOINT_DB_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[3];
    SHINE_ITEM_REGISTNUMBER restatitemkey;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINT_DB_REQ) == 22, "PROTO_NC_CHAR_STAT_DECPOINT_DB_REQ size drift");

struct PROTO_NC_CHAR_STAT_DECPOINT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_DECPOINT_REQ) == 1, "PROTO_NC_CHAR_STAT_DECPOINT_REQ size drift");

struct PROTO_NC_CHAR_STAT_INCPOINTFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINTFAIL_ACK) == 2, "PROTO_NC_CHAR_STAT_INCPOINTFAIL_ACK size drift");

struct PROTO_NC_CHAR_STAT_INCPOINTFAIL_DB_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINTFAIL_DB_ACK) == 8, "PROTO_NC_CHAR_STAT_INCPOINTFAIL_DB_ACK size drift");

struct PROTO_NC_CHAR_STAT_INCPOINTSUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINTSUC_ACK) == 1, "PROTO_NC_CHAR_STAT_INCPOINTSUC_ACK size drift");

struct PROTO_NC_CHAR_STAT_INCPOINTSUC_DB_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINTSUC_DB_ACK) == 7, "PROTO_NC_CHAR_STAT_INCPOINTSUC_DB_ACK size drift");

struct PROTO_NC_CHAR_STAT_INCPOINT_DB_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINT_DB_REQ) == 7, "PROTO_NC_CHAR_STAT_INCPOINT_DB_REQ size drift");

struct PROTO_NC_CHAR_STAT_INCPOINT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_INCPOINT_REQ) == 1, "PROTO_NC_CHAR_STAT_INCPOINT_REQ size drift");

struct PROTO_NC_CHAR_STAT_REMAINPOINT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_STAT_REMAINPOINT_CMD) == 1, "PROTO_NC_CHAR_STAT_REMAINPOINT_CMD size drift");

struct PROTO_NC_CHAR_SUPPORT_REWARD_CMD {
    uint8_t _pad_at_0000[4];
    USER_TYPE eUserType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_SUPPORT_REWARD_CMD) == 8, "PROTO_NC_CHAR_SUPPORT_REWARD_CMD size drift");

struct PROTO_NC_CHAR_TUTORIAL_DOING_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_DOING_CMD) == 1, "PROTO_NC_CHAR_TUTORIAL_DOING_CMD size drift");

struct PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_ACK) == 6, "PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_ACK size drift");

struct PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_REQ {
    uint8_t _pad_at_0000[5];
    uint8_t nFreeStat[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_REQ) == 10, "PROTO_NC_CHAR_TUTORIAL_FREESTAT_INIT_REQ size drift");

struct PROTO_NC_CHAR_TUTORIAL_INFO_WORLD_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_INFO_WORLD_CMD) == 9, "PROTO_NC_CHAR_TUTORIAL_INFO_WORLD_CMD size drift");

struct PROTO_NC_CHAR_TUTORIAL_INFO_ZONE_CMD {
    uint8_t _pad_at_0000[6];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_INFO_ZONE_CMD) == 11, "PROTO_NC_CHAR_TUTORIAL_INFO_ZONE_CMD size drift");

struct PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_ACK { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_ACK) == 9, "PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_ACK size drift");

struct PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_REQ { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_REQ) == 12, "PROTO_NC_CHAR_TUTORIAL_MAKE_ITEM_REQ size drift");

struct PROTO_NC_CHAR_TUTORIAL_POPUP_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_POPUP_ACK) == 1, "PROTO_NC_CHAR_TUTORIAL_POPUP_ACK size drift");

struct PROTO_NC_CHAR_TUTORIAL_POPUP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_POPUP_REQ) == 1, "PROTO_NC_CHAR_TUTORIAL_POPUP_REQ size drift");

struct PROTO_NC_CHAR_TUTORIAL_STEP_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_STEP_ACK) == 1, "PROTO_NC_CHAR_TUTORIAL_STEP_ACK size drift");

struct PROTO_NC_CHAR_TUTORIAL_STEP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_STEP_REQ) == 1, "PROTO_NC_CHAR_TUTORIAL_STEP_REQ size drift");

struct PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_ACK {
    uint8_t _pad_at_0000[4];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_ACK) == 9, "PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_ACK size drift");

struct PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_CMD) == 9, "PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_CMD size drift");

struct PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_REQ {
    uint8_t _pad_at_0000[8];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_REQ) == 13, "PROTO_NC_CHAR_TUTORIAL_STEP_SAVE_REQ size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_ACK) == 1, "PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_ACK size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_REQ) == 1, "PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_OFF_REQ size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_ACK) == 2, "PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_ACK size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_REQ {
    uint16_t ChargedItemList[12];
};
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_REQ) == 24, "PROTO_NC_CHAR_USEITEM_MINIMON_CHARGED_ITEM_ON_REQ size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CLIENT_CMD {
    USEITEM_MINIMON_INFO UseItemMinimonInfo;
    uint8_t _tail[50];
};
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CLIENT_CMD) == 50, "PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CLIENT_CMD size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CMD {
    uint8_t _pad_at_0000[6];
    USEITEM_MINIMON_INFO UseItemMinimonInfo;
    uint8_t _tail[50];
};
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CMD) == 56, "PROTO_NC_CHAR_USEITEM_MINIMON_INFO_CMD size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_INFO_ZONE_CMD {
    uint8_t _pad_at_0000[6];
    USEITEM_MINIMON_INFO UseItemMinimonInfo;
    uint8_t _tail[50];
};
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_INFO_ZONE_CMD) == 56, "PROTO_NC_CHAR_USEITEM_MINIMON_INFO_ZONE_CMD size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_ACK) == 1, "PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_ACK size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_REQ) == 1, "PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_OFF_REQ size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_ACK) == 2, "PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_ACK size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_REQ {
    uint16_t NormalItemList[12];
};
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_REQ) == 24, "PROTO_NC_CHAR_USEITEM_MINIMON_NORMAL_ITEM_ON_REQ size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_NOTICE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_NOTICE_CMD) == 4, "PROTO_NC_CHAR_USEITEM_MINIMON_NOTICE_CMD size drift");

struct PROTO_NC_CHAR_USEITEM_MINIMON_USE_BROAD_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_CHAR_USEITEM_MINIMON_USE_BROAD_CMD) == 3, "PROTO_NC_CHAR_USEITEM_MINIMON_USE_BROAD_CMD size drift");

struct PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Status_ {
    uint32_t  Main;
    uint32_t  Sub;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Status_) == 1, "PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Status_ size drift");

struct PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Partner_ {
    Name5 Name;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Partner_) == 24, "PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Partner_ size drift");

struct PROTO_NC_CHAR_WEDDINGDATA_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Status_ Status;
    ShineDateTime When;
    uint8_t _pad_at_0009[4];
    ShineDateTime SubWhen;
    ShineDateTime ApplicationLimit;
    ShineDateTime DelayLimit;
    ShineDateTime DivorceLimit;
    ShineDateTime NewlyLimit;
    uint8_t _pad_at_000d[4];
    PROTO_NC_CHAR_WEDDINGDATA_ACK___unnamed_type_Partner_ Partner;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDINGDATA_ACK) == 43, "PROTO_NC_CHAR_WEDDINGDATA_ACK size drift");

struct PROTO_NC_CHAR_WEDDINGDATA_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDINGDATA_REQ) == 6, "PROTO_NC_CHAR_WEDDINGDATA_REQ size drift");

struct PROTO_NC_CHAR_WEDDING_CANCEL_WEDDING { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_CANCEL_WEDDING) == 4, "PROTO_NC_CHAR_WEDDING_CANCEL_WEDDING size drift");

struct PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_ACK) == 16, "PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_ACK size drift");

struct PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_REQ) == 11, "PROTO_NC_CHAR_WEDDING_DIVORCE_CANCEL_REQ size drift");

struct PROTO_NC_CHAR_WEDDING_DIVORCE_DO_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_DIVORCE_DO_REQ) == 11, "PROTO_NC_CHAR_WEDDING_DIVORCE_DO_REQ size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK___unnamed_type_PartnerInfo_ {
    uint8_t _pad_at_0000[2];
    ShineDateTime LastConnect;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK___unnamed_type_PartnerInfo_) == 8, "PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK___unnamed_type_PartnerInfo_ size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK {
    NETPACKETZONEHEADER header;
    PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK___unnamed_type_PartnerInfo_ PartnerInfo;
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK) == 14, "PROTO_NC_CHAR_WEDDING_PARTNER_INFO_ACK size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_INFO_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_INFO_REQ) == 10, "PROTO_NC_CHAR_WEDDING_PARTNER_INFO_REQ size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG___unnamed_type_PartnerInfo_ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG___unnamed_type_PartnerInfo_) == 8, "PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG___unnamed_type_PartnerInfo_ size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[7];
    PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG___unnamed_type_PartnerInfo_ PartnerInfo;
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG) == 15, "PROTO_NC_CHAR_WEDDING_PARTNER_INFO_RNG size drift");

struct PROTO_NC_CHAR_WEDDING_PARTNER_SUMMON_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[11];
    Name3 map;
    uint8_t _pad_at_000b[12];
    SHINE_XY_TYPE coord;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PARTNER_SUMMON_RNG) == 35, "PROTO_NC_CHAR_WEDDING_PARTNER_SUMMON_RNG size drift");

struct PROTO_NC_CHAR_WEDDING_PROPOSE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PROPOSE_ACK) == 14, "PROTO_NC_CHAR_WEDDING_PROPOSE_ACK size drift");

struct PROTO_NC_CHAR_WEDDING_PROPOSE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_PROPOSE_REQ) == 10, "PROTO_NC_CHAR_WEDDING_PROPOSE_REQ size drift");

struct PROTO_NC_CHAR_WEDDING_REFRESH_INFO_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _tail[7];
};
static_assert(sizeof(PROTO_NC_CHAR_WEDDING_REFRESH_INFO_RNG) == 7, "PROTO_NC_CHAR_WEDDING_REFRESH_INFO_RNG size drift");

struct PROTO_NC_CHAR_WORLD_AUTO_PICK_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAR_WORLD_AUTO_PICK_CMD) == 1, "PROTO_NC_CHAR_WORLD_AUTO_PICK_CMD size drift");

struct PROTO_NC_CHAR_WORLD_REST_EXP_LAST_EXEC_TIME_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_WORLD_REST_EXP_LAST_EXEC_TIME_CMD) == 4, "PROTO_NC_CHAR_WORLD_REST_EXP_LAST_EXEC_TIME_CMD size drift");

struct PROTO_NC_CHAR_ZONE_AUTO_PICK_CMD {
    NETPACKETZONEHEADER header;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_CHAR_ZONE_AUTO_PICK_CMD) == 9, "PROTO_NC_CHAR_ZONE_AUTO_PICK_CMD size drift");

struct PROTO_NC_CHAR_ZONE_CHARDATAFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER header;
    PROTO_ERRORCODE ack;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CHAR_ZONE_CHARDATAFAIL_ACK_SEND) == 11, "PROTO_NC_CHAR_ZONE_CHARDATAFAIL_ACK_SEND size drift");

struct PROTO_NC_CHAR_ZONE_CHARDATA_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAR_ZONE_CHARDATA_ACK) == 2, "PROTO_NC_CHAR_ZONE_CHARDATA_ACK size drift");

struct PROTO_NC_CHAR_ZONE_CHARDATA_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER header;
    PROTO_NC_CHAR_ZONE_CHARDATA_ACK ack;
};
static_assert(sizeof(PROTO_NC_CHAR_ZONE_CHARDATA_ACK_SEND) == 11, "PROTO_NC_CHAR_ZONE_CHARDATA_ACK_SEND size drift");

struct PROTO_NC_CHAR_ZONE_CHAT_COLOR_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAR_ZONE_CHAT_COLOR_CMD) == 8, "PROTO_NC_CHAR_ZONE_CHAT_COLOR_CMD size drift");

struct PROTO_NC_CHAR_ZONE_GUILD_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAR_ZONE_GUILD_CMD) == 4, "PROTO_NC_CHAR_ZONE_GUILD_CMD size drift");

struct PROTO_NC_CHAT_RESTRICT_ADD_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_CHAT_RESTRICT_INFO ChatRestrictData;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_ADD_ACK) == 24, "PROTO_NC_CHAT_RESTRICT_ADD_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_ADD_REQ {
    Name5 sChatRestrictCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_ADD_REQ) == 20, "PROTO_NC_CHAT_RESTRICT_ADD_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_ADD_ACK {
    uint8_t _pad_at_0000[4];
    PROTO_CHAT_RESTRICT_INFO ChatRestrictData;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_ADD_ACK) == 30, "PROTO_NC_CHAT_RESTRICT_DB_ADD_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_ADD_REQ {
    uint8_t _pad_at_0000[6];
    Name5 sChatRestrictCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_ADD_REQ) == 26, "PROTO_NC_CHAT_RESTRICT_DB_ADD_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_DEL_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_DEL_ACK) == 8, "PROTO_NC_CHAT_RESTRICT_DB_DEL_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_ACK) == 4, "PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_REQ) == 6, "PROTO_NC_CHAT_RESTRICT_DB_DEL_ALL_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_DEL_REQ { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_DEL_REQ) == 10, "PROTO_NC_CHAT_RESTRICT_DB_DEL_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_DB_LIST_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_CHAT_RESTRICT_DB_INFO_______0_bytes___ ChatRestrictData;
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DB_LIST_CMD) == 2, "PROTO_NC_CHAT_RESTRICT_DB_LIST_CMD size drift");

struct PROTO_NC_CHAT_RESTRICT_DEL_ACK {
    uint8_t _pad_at_0000[2];
    Name5 sChatRestrictCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DEL_ACK) == 22, "PROTO_NC_CHAT_RESTRICT_DEL_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_DEL_ALL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DEL_ALL_ACK) == 2, "PROTO_NC_CHAT_RESTRICT_DEL_ALL_ACK size drift");

struct PROTO_NC_CHAT_RESTRICT_DEL_ALL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DEL_ALL_REQ) == 1, "PROTO_NC_CHAT_RESTRICT_DEL_ALL_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_DEL_REQ {
    Name5 sChatRestrictCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_DEL_REQ) == 20, "PROTO_NC_CHAT_RESTRICT_DEL_REQ size drift");

struct PROTO_NC_CHAT_RESTRICT_LIST_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_CHAT_RESTRICT_INFO_______0_bytes___ ChatRestrictData;
};
static_assert(sizeof(PROTO_NC_CHAT_RESTRICT_LIST_CMD) == 2, "PROTO_NC_CHAT_RESTRICT_LIST_CMD size drift");

struct PROTO_NC_CHER_EVENT_ATTENDANCE_CHANGE_START_CMD {
    tm tEventStart;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_CHER_EVENT_ATTENDANCE_CHANGE_START_CMD) == 36, "PROTO_NC_CHER_EVENT_ATTENDANCE_CHANGE_START_CMD size drift");

struct PROTO_NC_COLLECT_BOOKMARK_REGIST_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_COLLECT_BOOKMARK_REGIST_ACK) == 6, "PROTO_NC_COLLECT_BOOKMARK_REGIST_ACK size drift");

struct PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_ACK) == 12, "PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_ACK size drift");

struct PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_REQ { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_REQ) == 10, "PROTO_NC_COLLECT_BOOKMARK_REGIST_DB_REQ size drift");

struct PROTO_NC_COLLECT_BOOKMARK_REGIST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_COLLECT_BOOKMARK_REGIST_REQ) == 4, "PROTO_NC_COLLECT_BOOKMARK_REGIST_REQ size drift");

struct PROTO_NC_COLLECT_CARDOPEN_CMD {
    uint8_t _pad_at_0000[3];
    SHINE_ITEM_STRUCT CardInform;
    uint8_t _tail[103];
};
static_assert(sizeof(PROTO_NC_COLLECT_CARDOPEN_CMD) == 106, "PROTO_NC_COLLECT_CARDOPEN_CMD size drift");

struct PROTO_NC_COLLECT_CARDREGIST_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_COLLECT_CARDREGIST_ACK) == 10, "PROTO_NC_COLLECT_CARDREGIST_ACK size drift");

struct PROTO_NC_COLLECT_CARDREGIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_COLLECT_CARDREGIST_REQ) == 1, "PROTO_NC_COLLECT_CARDREGIST_REQ size drift");

struct PROTO_NC_COLLECT_CHARACTERDB_UP_GET_CARDCOUNT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_COLLECT_CHARACTERDB_UP_GET_CARDCOUNT_CMD) == 1, "PROTO_NC_COLLECT_CHARACTERDB_UP_GET_CARDCOUNT_CMD size drift");

struct PROTO_NC_COLLECT_PEEPING_COLLECT_ACK {
    Name5 charname;
    uint8_t _pad_at_0000[22];
    PROTO_NC_CHAR_CARDCOLLECT_CMD collection;
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_COLLECT_ACK) == 28, "PROTO_NC_COLLECT_PEEPING_COLLECT_ACK size drift");

struct PROTO_NC_COLLECT_PEEPING_COLLECT_BOOKMARK_ACK {
    Name5 charname;
    uint8_t _pad_at_0000[22];
    PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD collectionview;
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_COLLECT_BOOKMARK_ACK) == 28, "PROTO_NC_COLLECT_PEEPING_COLLECT_BOOKMARK_ACK size drift");

struct PROTO_NC_COLLECT_PEEPING_DB_COLLECT_ACK {
    uint8_t _pad_at_0000[6];
    Name5 targetcharname;
    uint8_t _pad_at_0006[22];
    PROTO_NC_CHAR_CARDCOLLECT_CMD collection;
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_DB_COLLECT_ACK) == 34, "PROTO_NC_COLLECT_PEEPING_DB_COLLECT_ACK size drift");

struct PROTO_NC_COLLECT_PEEPING_DB_COLLECT_BOOKMARK_ACK {
    uint8_t _pad_at_0000[6];
    Name5 targetcharname;
    uint8_t _pad_at_0006[22];
    PROTO_NC_CHAR_CARDCOLLECT_BOOKMARK_CMD collectionview;
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_DB_COLLECT_BOOKMARK_ACK) == 34, "PROTO_NC_COLLECT_PEEPING_DB_COLLECT_BOOKMARK_ACK size drift");

struct PROTO_NC_COLLECT_PEEPING_DB_REQ {
    uint8_t _pad_at_0000[6];
    Name5 targetcharname;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_DB_REQ) == 26, "PROTO_NC_COLLECT_PEEPING_DB_REQ size drift");

struct PROTO_NC_COLLECT_PEEPING_REQ {
    Name5 charname;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_COLLECT_PEEPING_REQ) == 20, "PROTO_NC_COLLECT_PEEPING_REQ size drift");

struct PROTO_NC_COLLECT_REGIST_ACK {
    uint8_t _pad_at_0000[8];
    PROTO_NC_COLLECT_CARDREGIST_ACK CardInfo;
};
static_assert(sizeof(PROTO_NC_COLLECT_REGIST_ACK) == 18, "PROTO_NC_COLLECT_REGIST_ACK size drift");

struct PROTO_NC_COLLECT_REGIST_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER CardKey;
    uint8_t _pad_at_0008[8];
    PROTO_NC_COLLECT_CARDREGIST_ACK CardInfo;
};
static_assert(sizeof(PROTO_NC_COLLECT_REGIST_REQ) == 26, "PROTO_NC_COLLECT_REGIST_REQ size drift");

struct PROTO_NC_COLLECT_REWARD_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_COLLECT_REWARD_ACK) == 4, "PROTO_NC_COLLECT_REWARD_ACK size drift");

struct PROTO_NC_COLLECT_REWARD_DB_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_COLLECT_REWARD_DB_ACK) == 10, "PROTO_NC_COLLECT_REWARD_DB_ACK size drift");

struct PROTO_NC_COLLECT_REWARD_DB_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_COLLECT_REWARD_DB_REQ) == 8, "PROTO_NC_COLLECT_REWARD_DB_REQ size drift");

struct PROTO_NC_COLLECT_REWARD_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_COLLECT_REWARD_REQ) == 4, "PROTO_NC_COLLECT_REWARD_REQ size drift");

struct PROTO_NC_CT_ADD_FRIEND_CMD {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_ADD_FRIEND_CMD) == 8, "PROTO_NC_CT_ADD_FRIEND_CMD size drift");

struct PROTO_NC_CT_CHARTTING_CMD {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_CT_CHARTTING_CMD) == 6, "PROTO_NC_CT_CHARTTING_CMD size drift");

struct PROTO_NC_CT_DB_SET_CMD {
    NETPACKETZONEHEADER header;
    CT_INFO Info;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_DB_SET_CMD) == 8, "PROTO_NC_CT_DB_SET_CMD size drift");

struct PROTO_NC_CT_LUASCRIPT_SET_WORLD_CMD {
    uint8_t _pad_at_0000[4];
    CT_INFO CTInfo;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_LUASCRIPT_SET_WORLD_CMD) == 6, "PROTO_NC_CT_LUASCRIPT_SET_WORLD_CMD size drift");

struct PROTO_NC_CT_LUASCRIPT_SET_ZONE_CMD {
    uint8_t _pad_at_0000[4];
    CT_INFO CTInfo;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_LUASCRIPT_SET_ZONE_CMD) == 6, "PROTO_NC_CT_LUASCRIPT_SET_ZONE_CMD size drift");

struct PROTO_NC_CT_SET_CMD {
    CT_INFO Info;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_SET_CMD) == 2, "PROTO_NC_CT_SET_CMD size drift");

struct PROTO_NC_CT_SET_CURRENT_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CT_SET_CURRENT_ACK) == 6, "PROTO_NC_CT_SET_CURRENT_ACK size drift");

struct PROTO_NC_CT_SET_CURRENT_DB_CMD {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    CT_INFO UseTitle;
    uint8_t _pad_at_000a[2];
    CT_INFO UseAbleTitle;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_CT_SET_CURRENT_DB_CMD) == 14, "PROTO_NC_CT_SET_CURRENT_DB_CMD size drift");

struct PROTO_NC_CT_SET_CURRENT_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_CT_SET_CURRENT_REQ) == 4, "PROTO_NC_CT_SET_CURRENT_REQ size drift");

struct PROTO_NC_CT_SET_SOMEONECHANGE_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_CT_SET_SOMEONECHANGE_CMD) == 6, "PROTO_NC_CT_SET_SOMEONECHANGE_CMD size drift");

struct PROTO_NC_DATA_PRISON_ADD_GM_ACK {
    uint8_t _pad_at_0000[8];
    Name5 sImprison;
    uint8_t _pad_at_0008[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_DATA_PRISON_ADD_GM_ACK) == 110, "PROTO_NC_DATA_PRISON_ADD_GM_ACK size drift");

struct PROTO_NC_DATA_PRISON_ADD_GM_REQ {
    uint8_t _pad_at_0000[2];
    Name256Byte sGmUserID;
    uint8_t _pad_at_0002[260];
    Name5 sImprison;
    uint8_t _pad_at_0106[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_DATA_PRISON_ADD_GM_REQ) == 364, "PROTO_NC_DATA_PRISON_ADD_GM_REQ size drift");

struct PROTO_NC_DATA_PRISON_ALTER_GM_ACK {
    uint8_t _pad_at_0000[8];
    Name5 sImprison;
    uint8_t _pad_at_0008[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_DATA_PRISON_ALTER_GM_ACK) == 110, "PROTO_NC_DATA_PRISON_ALTER_GM_ACK size drift");

struct PROTO_NC_DATA_PRISON_ALTER_GM_REQ {
    uint8_t _pad_at_0000[2];
    Name256Byte sGmUserID;
    uint8_t _pad_at_0002[260];
    Name5 sImprison;
    uint8_t _pad_at_0106[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_DATA_PRISON_ALTER_GM_REQ) == 364, "PROTO_NC_DATA_PRISON_ALTER_GM_REQ size drift");

struct PROTO_NC_DATA_PRISON_GET_ACK {
    uint8_t _pad_at_0000[4];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_DATA_PRISON_GET_ACK) == 84, "PROTO_NC_DATA_PRISON_GET_ACK size drift");

struct PROTO_NC_DATA_PRISON_GET_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_DATA_PRISON_GET_REQ) == 6, "PROTO_NC_DATA_PRISON_GET_REQ size drift");

struct PROTO_NC_DATA_PRISON_UPDATE_MIN_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_DATA_PRISON_UPDATE_MIN_CMD) == 6, "PROTO_NC_DATA_PRISON_UPDATE_MIN_CMD size drift");

struct PROTO_NC_DATA_REPORT_ADD_ACK {
    uint8_t _pad_at_0000[4];
    Name5 sHarmer;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_DATA_REPORT_ADD_ACK) == 28, "PROTO_NC_DATA_REPORT_ADD_ACK size drift");

struct PROTO_NC_DATA_REPORT_ADD_REQ {
    uint8_t _pad_at_0000[7];
    Name5 sReporterCharID;
    uint8_t _pad_at_0007[20];
    Name5 sHarmer;
    uint8_t _pad_at_001b[20];
    uint8_t byReportType[16];
    uint8_t byReason[256];
    uint8_t _pad_at_013f[2];
    uint8_t byChatLog[2048];
};
static_assert(sizeof(PROTO_NC_DATA_REPORT_ADD_REQ) == 2369, "PROTO_NC_DATA_REPORT_ADD_REQ size drift");

struct PROTO_NC_DATA_REPORT_CANCEL_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_DATA_REPORT_CANCEL_ACK) == 8, "PROTO_NC_DATA_REPORT_CANCEL_ACK size drift");

struct PROTO_NC_DATA_REPORT_CANCEL_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_DATA_REPORT_CANCEL_REQ) == 6, "PROTO_NC_DATA_REPORT_CANCEL_REQ size drift");

struct PROTO_NC_DATA_REPORT_GET_ACK {
    uint8_t _pad_at_0000[9];
    uint8_t byRemark[128];
};
static_assert(sizeof(PROTO_NC_DATA_REPORT_GET_ACK) == 137, "PROTO_NC_DATA_REPORT_GET_ACK size drift");

struct PROTO_NC_DATA_REPORT_GET_REQ { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_DATA_REPORT_GET_REQ) == 7, "PROTO_NC_DATA_REPORT_GET_REQ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_ADD_ACK_ { uint8_t data[31]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_ADD_ACK_) == 31, "PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_ADD_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_CANCEL_ACK_ { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_CANCEL_ACK_) == 11, "PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_CANCEL_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_GET_ACK_ { uint8_t data[140]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_GET_ACK_) == 140, "PROTO_NC_DATA_SEND1_PROTO_NC_DATA_REPORT_GET_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK_ { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK_) == 12, "PROTO_NC_DATA_SEND1_PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM_ { uint8_t data[18]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM_) == 18, "PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_L20_DB_ACK_ { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_L20_DB_ACK_) == 16, "PROTO_NC_DATA_SEND1_PROTO_NC_MISC_EVENT_L20_DB_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_USER_GER_IS_IP_BLOCK_ACK_ { uint8_t data[293]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_USER_GER_IS_IP_BLOCK_ACK_) == 293, "PROTO_NC_DATA_SEND1_PROTO_NC_USER_GER_IS_IP_BLOCK_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_USER_IS_IP_BLOCK_ACK_ { uint8_t data[278]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_USER_IS_IP_BLOCK_ACK_) == 278, "PROTO_NC_DATA_SEND1_PROTO_NC_USER_IS_IP_BLOCK_ACK_ size drift");

struct PROTO_NC_DATA_SEND1_PROTO_NC_USER_TW_IS_IP_BLOCK_ACK_ { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_DATA_SEND1_PROTO_NC_USER_TW_IS_IP_BLOCK_ACK_) == 14, "PROTO_NC_DATA_SEND1_PROTO_NC_USER_TW_IS_IP_BLOCK_ACK_ size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_ACK) == 4, "PROTO_NC_DICE_TAISAI_BETTING_ACK size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_CANCEL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_CANCEL_ACK) == 4, "PROTO_NC_DICE_TAISAI_BETTING_CANCEL_ACK size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_CANCEL_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiBetting BettingCancelInfo;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_CANCEL_CMD) == 10, "PROTO_NC_DICE_TAISAI_BETTING_CANCEL_CMD size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_CANCEL_REQ {
    DiceTaiSaiBetting BettingCancelInfo;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_CANCEL_REQ) == 10, "PROTO_NC_DICE_TAISAI_BETTING_CANCEL_REQ size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiBetting BettingInfo;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_CMD) == 10, "PROTO_NC_DICE_TAISAI_BETTING_CMD size drift");

struct PROTO_NC_DICE_TAISAI_BETTING_REQ {
    DiceTaiSaiBetting BettingInfo;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BETTING_REQ) == 10, "PROTO_NC_DICE_TAISAI_BETTING_REQ size drift");

struct PROTO_NC_DICE_TAISAI_BET_START_CMD {
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _pad_at_0000[4];
    DiceTaiSaiInfo CurrentRollingDice;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BET_START_CMD) == 7, "PROTO_NC_DICE_TAISAI_BET_START_CMD size drift");

struct PROTO_NC_DICE_TAISAI_BOARD_FOLD_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BOARD_FOLD_CMD) == 1, "PROTO_NC_DICE_TAISAI_BOARD_FOLD_CMD size drift");

struct PROTO_NC_DICE_TAISAI_BOARD_FOLD_RESERVE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BOARD_FOLD_RESERVE_CMD) == 1, "PROTO_NC_DICE_TAISAI_BOARD_FOLD_RESERVE_CMD size drift");

struct PROTO_NC_DICE_TAISAI_BOARD_PITCH_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_BOARD_PITCH_CMD) == 1, "PROTO_NC_DICE_TAISAI_BOARD_PITCH_CMD size drift");

struct PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK {
    uint8_t _pad_at_0000[6];
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER nActiveDiceTaiSaiKey;
    uint8_t _pad_at_000c[10];
    DiceRollApplyResult_______0_bytes___ DiceRollResult;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK) == 22, "PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD {
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER nActiveDiceTaiSaiKey;
    uint8_t _pad_at_0006[14];
    DiceRollApplyResultLog_______0_bytes___ DiceRollResultLog;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD) == 20, "PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD size drift");

struct PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ {
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER nActiveDiceTaiSaiKey;
    uint8_t _pad_at_0006[8];
    DiceTaiSaiInfo DiceNum;
    uint8_t _pad_at_000e[5];
    DiceRollApplyResult_______0_bytes___ DiceRollResult;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ) == 19, "PROTO_NC_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_ACK {
    uint8_t _pad_at_0000[2];
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_0008[2];
    SHINE_ITEM_REGISTNUMBER nTaiSaiItemKey;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_ACK) == 30, "PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_REQ {
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER nTaiSaiItemKey;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_REQ) == 26, "PROTO_NC_DICE_TAISAI_DB_EXPECT_INCOME_MONEY_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_ACK {
    uint8_t _pad_at_0000[2];
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_0008[4];
    SHINE_ITEM_REGISTNUMBER nDiceTaiSaiKey;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_ACK) == 32, "PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_REQ {
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_0006[4];
    SHINE_ITEM_REGISTNUMBER nDiceTaiSaiKey;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_REQ) == 32, "PROTO_NC_DICE_TAISAI_DB_ITEM_ID_CHANGE_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DB_RANK_ACK {
    uint8_t _pad_at_0000[2];
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0008[5];
    DiceRankInfo_______0_bytes___ Rank;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_RANK_ACK) == 13, "PROTO_NC_DICE_TAISAI_DB_RANK_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DB_RANK_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    SHINE_ITEM_REGISTNUMBER nTaiSaiItemKey;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_RANK_REQ) == 18, "PROTO_NC_DICE_TAISAI_DB_RANK_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_ACK {
    uint8_t _pad_at_0000[4];
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_000a[2];
    SHINE_ITEM_REGISTNUMBER nTaiSaiItemKey;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_ACK) == 28, "PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_REQ {
    uint8_t _pad_at_0000[2];
    NETPACKETZONEHEADER header;
    ITEM_INVEN nInvenSlot;
    uint8_t _pad_at_0008[2];
    SHINE_ITEM_REGISTNUMBER nTaiSaiItemKey;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_REQ) == 18, "PROTO_NC_DICE_TAISAI_DB_RECEIPT_INCOME_MONEY_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DICE_ROLL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DICE_ROLL_ACK) == 2, "PROTO_NC_DICE_TAISAI_DICE_ROLL_ACK size drift");

struct PROTO_NC_DICE_TAISAI_DICE_ROLL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DICE_ROLL_CMD) == 2, "PROTO_NC_DICE_TAISAI_DICE_ROLL_CMD size drift");

struct PROTO_NC_DICE_TAISAI_DICE_ROLL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DICE_ROLL_REQ) == 1, "PROTO_NC_DICE_TAISAI_DICE_ROLL_REQ size drift");

struct PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiInfo nDiceInfo;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_CMD) == 5, "PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_CMD size drift");

struct PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD {
    uint8_t _pad_at_0000[1];
    EmotionInfo_______0_bytes___ Emotion;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD) == 1, "PROTO_NC_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD size drift");

struct PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_ACK {
    uint8_t _pad_at_0000[2];
    ITEM_INVEN nInvenSlot;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_ACK) == 16, "PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_ACK size drift");

struct PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_REQ {
    ITEM_INVEN nInvenSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_REQ) == 2, "PROTO_NC_DICE_TAISAI_EXPECT_INCOME_MONEY_REQ size drift");

struct PROTO_NC_DICE_TAISAI_GAME_BETTING_INFO_CMD {
    uint8_t _pad_at_0000[3];
    BettingInfo_______0_bytes___ Betting;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_BETTING_INFO_CMD) == 3, "PROTO_NC_DICE_TAISAI_GAME_BETTING_INFO_CMD size drift");

struct PROTO_NC_DICE_TAISAI_GAME_JOIN_ACK {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiInfo_______21_bytes___ DiceHistory;
    uint8_t _pad_at_0002[21];
    DICE_TAISAI_GAME_STATUS nGameStatus;
    uint8_t _pad_at_0017[4];
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _pad_at_001b[23];
    uint16_t nGamerHnd[0];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_JOIN_ACK) == 50, "PROTO_NC_DICE_TAISAI_GAME_JOIN_ACK size drift");

struct PROTO_NC_DICE_TAISAI_GAME_JOIN_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_JOIN_CMD) == 2, "PROTO_NC_DICE_TAISAI_GAME_JOIN_CMD size drift");

struct PROTO_NC_DICE_TAISAI_GAME_JOIN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_JOIN_REQ) == 1, "PROTO_NC_DICE_TAISAI_GAME_JOIN_REQ size drift");

struct PROTO_NC_DICE_TAISAI_GAME_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_LEAVE_ACK) == 2, "PROTO_NC_DICE_TAISAI_GAME_LEAVE_ACK size drift");

struct PROTO_NC_DICE_TAISAI_GAME_LEAVE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_LEAVE_CMD) == 3, "PROTO_NC_DICE_TAISAI_GAME_LEAVE_CMD size drift");

struct PROTO_NC_DICE_TAISAI_GAME_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_LEAVE_REQ) == 1, "PROTO_NC_DICE_TAISAI_GAME_LEAVE_REQ size drift");

struct PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_ACK {
    uint8_t _pad_at_0000[2];
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_ACK) == 6, "PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_ACK size drift");

struct PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_CMD {
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_CMD) == 4, "PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_CMD size drift");

struct PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_REQ) == 1, "PROTO_NC_DICE_TAISAI_GAME_MODE_CHANGE_REQ size drift");

struct PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_ACK) == 2, "PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_ACK size drift");

struct PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_CMD {
    uint8_t _pad_at_0000[1];
    LargeAmountInfo_______0_bytes___ LargeAmount;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_CMD) == 1, "PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_CMD size drift");

struct PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_REQ) == 4, "PROTO_NC_DICE_TAISAI_LARGE_AMOUNT_REQ size drift");

struct PROTO_NC_DICE_TAISAI_RANK_ACK {
    uint8_t _pad_at_0000[3];
    DiceRankInfo_______480_bytes___ RankHistory;
    uint8_t _pad_at_0003[481];
    DiceRankInfo_______0_bytes___ RankCurr;
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_RANK_ACK) == 484, "PROTO_NC_DICE_TAISAI_RANK_ACK size drift");

struct PROTO_NC_DICE_TAISAI_RANK_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_RANK_REQ) == 1, "PROTO_NC_DICE_TAISAI_RANK_REQ size drift");

struct PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_ACK {
    uint8_t _pad_at_0000[2];
    ITEM_INVEN nInvenSlot;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_ACK) == 12, "PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_ACK size drift");

struct PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_REQ {
    ITEM_INVEN nInvenSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_REQ) == 2, "PROTO_NC_DICE_TAISAI_RECEIPT_INCOME_MONEY_REQ size drift");

struct PROTO_NC_DICE_TAISAI_TIMER_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_DICE_TAISAI_TIMER_CMD) == 7, "PROTO_NC_DICE_TAISAI_TIMER_CMD size drift");

struct PROTO_NC_EVENT_ADD_EVENT_ACK {
    uint8_t _pad_at_0000[2];
    GM_EVENT_DATA EventData;
    uint8_t _tail[1363];
};
static_assert(sizeof(PROTO_NC_EVENT_ADD_EVENT_ACK) == 1365, "PROTO_NC_EVENT_ADD_EVENT_ACK size drift");

struct PROTO_NC_EVENT_ADD_EVENT_REQ {
    GM_EVENT_DATA EventData;
    uint8_t _tail[1363];
};
static_assert(sizeof(PROTO_NC_EVENT_ADD_EVENT_REQ) == 1363, "PROTO_NC_EVENT_ADD_EVENT_REQ size drift");

struct PROTO_NC_EVENT_ADD_UPDATE_EVENT_CMD {
    GM_EVENT_DATA EventData;
    uint8_t _tail[1363];
};
static_assert(sizeof(PROTO_NC_EVENT_ADD_UPDATE_EVENT_CMD) == 1363, "PROTO_NC_EVENT_ADD_UPDATE_EVENT_CMD size drift");

struct PROTO_NC_EVENT_DEL_EVENT_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_EVENT_DEL_EVENT_ACK) == 4, "PROTO_NC_EVENT_DEL_EVENT_ACK size drift");

struct PROTO_NC_EVENT_DEL_EVENT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_EVENT_DEL_EVENT_CMD) == 2, "PROTO_NC_EVENT_DEL_EVENT_CMD size drift");

struct PROTO_NC_EVENT_DEL_EVENT_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_EVENT_DEL_EVENT_REQ) == 2, "PROTO_NC_EVENT_DEL_EVENT_REQ size drift");

struct PROTO_NC_EVENT_GET_ALL_EVENT_INFO_ACK {
    uint8_t _pad_at_0000[6];
    GM_EVENT_DATA EventData;
    uint8_t _tail[1364];
};
static_assert(sizeof(PROTO_NC_EVENT_GET_ALL_EVENT_INFO_ACK) == 1370, "PROTO_NC_EVENT_GET_ALL_EVENT_INFO_ACK size drift");

struct PROTO_NC_EVENT_GET_ALL_EVENT_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_EVENT_GET_ALL_EVENT_INFO_REQ) == 1, "PROTO_NC_EVENT_GET_ALL_EVENT_INFO_REQ size drift");

struct PROTO_NC_EVENT_SET_ALL_READY_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_EVENT_SET_ALL_READY_ACK) == 4, "PROTO_NC_EVENT_SET_ALL_READY_ACK size drift");

struct PROTO_NC_EVENT_SET_ALL_READY_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_EVENT_SET_ALL_READY_REQ) == 3, "PROTO_NC_EVENT_SET_ALL_READY_REQ size drift");

struct PROTO_NC_EVENT_UPDATE_EVENT_ACK {
    uint8_t _pad_at_0000[2];
    GM_EVENT_DATA EventData;
    uint8_t _tail[1363];
};
static_assert(sizeof(PROTO_NC_EVENT_UPDATE_EVENT_ACK) == 1365, "PROTO_NC_EVENT_UPDATE_EVENT_ACK size drift");

struct PROTO_NC_EVENT_UPDATE_EVENT_REQ {
    GM_EVENT_DATA EventData;
    uint8_t _tail[1363];
};
static_assert(sizeof(PROTO_NC_EVENT_UPDATE_EVENT_REQ) == 1363, "PROTO_NC_EVENT_UPDATE_EVENT_REQ size drift");

struct PROTO_NC_FRIEND_ADD_CMD {
    PROTO_FRIEND_INFO friendinfo;
    uint8_t _tail[72];
};
static_assert(sizeof(PROTO_NC_FRIEND_ADD_CMD) == 72, "PROTO_NC_FRIEND_ADD_CMD size drift");

struct PROTO_NC_FRIEND_CLASS_CHANGE_CMD {
    Name5 charid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_FRIEND_CLASS_CHANGE_CMD) == 21, "PROTO_NC_FRIEND_CLASS_CHANGE_CMD size drift");

struct PROTO_NC_FRIEND_DB_DEL_ACK {
    uint8_t _pad_at_0000[6];
    Name5 charid;
    uint8_t _pad_at_0006[20];
    Name5 friendid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_FRIEND_DB_DEL_ACK) == 48, "PROTO_NC_FRIEND_DB_DEL_ACK size drift");

struct PROTO_NC_FRIEND_DB_DEL_REQ {
    uint8_t _pad_at_0000[6];
    Name5 charid;
    uint8_t _pad_at_0006[20];
    Name5 friendid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_DB_DEL_REQ) == 46, "PROTO_NC_FRIEND_DB_DEL_REQ size drift");

struct PROTO_NC_FRIEND_DB_GET_ACK {
    uint8_t _pad_at_0000[12];
    PROTO_FRIEND_INFO friendinfo;
    uint8_t _tail[72];
};
static_assert(sizeof(PROTO_NC_FRIEND_DB_GET_ACK) == 84, "PROTO_NC_FRIEND_DB_GET_ACK size drift");

struct PROTO_NC_FRIEND_DB_GET_REQ { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_FRIEND_DB_GET_REQ) == 10, "PROTO_NC_FRIEND_DB_GET_REQ size drift");

struct PROTO_NC_FRIEND_DB_POINT { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_FRIEND_DB_POINT) == 8, "PROTO_NC_FRIEND_DB_POINT size drift");

struct PROTO_NC_FRIEND_DB_POINT_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_FRIEND_DB_POINT_______0_bytes___ pointinfo;
};
static_assert(sizeof(PROTO_NC_FRIEND_DB_POINT_CMD) == 2, "PROTO_NC_FRIEND_DB_POINT_CMD size drift");

struct PROTO_NC_FRIEND_DB_SET_ACK { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_FRIEND_DB_SET_ACK) == 14, "PROTO_NC_FRIEND_DB_SET_ACK size drift");

struct PROTO_NC_FRIEND_DB_SET_REQ { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_FRIEND_DB_SET_REQ) == 13, "PROTO_NC_FRIEND_DB_SET_REQ size drift");

struct PROTO_NC_FRIEND_DEL_ACK {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_FRIEND_DEL_ACK) == 42, "PROTO_NC_FRIEND_DEL_ACK size drift");

struct PROTO_NC_FRIEND_DEL_CMD {
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_DEL_CMD) == 20, "PROTO_NC_FRIEND_DEL_CMD size drift");

struct PROTO_NC_FRIEND_DEL_REQ {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_DEL_REQ) == 40, "PROTO_NC_FRIEND_DEL_REQ size drift");

struct PROTO_NC_FRIEND_FIND_FRIENDS_ACK {
    uint8_t _pad_at_0000[4];
    PROTO_FRIEND_INFO_______0_bytes___ friendinfo;
};
static_assert(sizeof(PROTO_NC_FRIEND_FIND_FRIENDS_ACK) == 4, "PROTO_NC_FRIEND_FIND_FRIENDS_ACK size drift");

struct PROTO_NC_FRIEND_FIND_FRIENDS_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_FRIEND_FIND_FRIENDS_REQ) == 1, "PROTO_NC_FRIEND_FIND_FRIENDS_REQ size drift");

struct PROTO_NC_FRIEND_GET_DIFF_FRIEND_POINT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_FRIEND_GET_DIFF_FRIEND_POINT_CMD) == 2, "PROTO_NC_FRIEND_GET_DIFF_FRIEND_POINT_CMD size drift");

struct PROTO_NC_FRIEND_LEVEL_CMD {
    Name5 charid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_FRIEND_LEVEL_CMD) == 21, "PROTO_NC_FRIEND_LEVEL_CMD size drift");

struct PROTO_NC_FRIEND_LIST_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_FRIEND_INFO_______0_bytes___ friendinfo;
};
static_assert(sizeof(PROTO_NC_FRIEND_LIST_CMD) == 1, "PROTO_NC_FRIEND_LIST_CMD size drift");

struct PROTO_NC_FRIEND_LOGIN_CMD {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name3 map;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_FRIEND_LOGIN_CMD) == 32, "PROTO_NC_FRIEND_LOGIN_CMD size drift");

struct PROTO_NC_FRIEND_LOGOUT_CMD {
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_LOGOUT_CMD) == 20, "PROTO_NC_FRIEND_LOGOUT_CMD size drift");

struct PROTO_NC_FRIEND_MAP_CMD {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name3 map;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_FRIEND_MAP_CMD) == 32, "PROTO_NC_FRIEND_MAP_CMD size drift");

struct PROTO_NC_FRIEND_PARTY_CMD {
    Name5 charid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_FRIEND_PARTY_CMD) == 21, "PROTO_NC_FRIEND_PARTY_CMD size drift");

struct PROTO_NC_FRIEND_POINT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_FRIEND_POINT_ACK) == 2, "PROTO_NC_FRIEND_POINT_ACK size drift");

struct PROTO_NC_FRIEND_POINT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_FRIEND_POINT_REQ) == 1, "PROTO_NC_FRIEND_POINT_REQ size drift");

struct PROTO_NC_FRIEND_REFUSE_CMD {
    Name5 friendid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_REFUSE_CMD) == 20, "PROTO_NC_FRIEND_REFUSE_CMD size drift");

struct PROTO_NC_FRIEND_SET_ACK {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_FRIEND_SET_ACK) == 42, "PROTO_NC_FRIEND_SET_ACK size drift");

struct PROTO_NC_FRIEND_SET_CONFIRM_ACK {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_FRIEND_SET_CONFIRM_ACK) == 41, "PROTO_NC_FRIEND_SET_CONFIRM_ACK size drift");

struct PROTO_NC_FRIEND_SET_CONFIRM_REQ {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_SET_CONFIRM_REQ) == 40, "PROTO_NC_FRIEND_SET_CONFIRM_REQ size drift");

struct PROTO_NC_FRIEND_SET_REQ {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    Name5 friendid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_SET_REQ) == 40, "PROTO_NC_FRIEND_SET_REQ size drift");

struct PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_WORLD_CMD {
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_WORLD_CMD) == 20, "PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_WORLD_CMD size drift");

struct PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_ZONE_CMD {
    Name5 charid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_ZONE_CMD) == 20, "PROTO_NC_FRIEND_SOMEONE_GET_SPECIALITEM_ZONE_CMD size drift");

struct PROTO_NC_FRIEND_UES_FRIEND_POINT_ACK { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_FRIEND_UES_FRIEND_POINT_ACK) == 7, "PROTO_NC_FRIEND_UES_FRIEND_POINT_ACK size drift");

struct PROTO_NC_FRIEND_UES_FRIEND_POINT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_FRIEND_UES_FRIEND_POINT_REQ) == 1, "PROTO_NC_FRIEND_UES_FRIEND_POINT_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_BUY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_BUY_ACK) == 2, "PROTO_NC_GAMBLE_COIN_BUY_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_BUY_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_BUY_REQ) == 8, "PROTO_NC_GAMBLE_COIN_BUY_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_CHANGE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_CHANGE_CMD) == 8, "PROTO_NC_GAMBLE_COIN_CHANGE_CMD size drift");

struct PROTO_NC_GAMBLE_COIN_DB_ADD_ACK {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_ADD_ACK) == 10, "PROTO_NC_GAMBLE_COIN_DB_ADD_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_DB_ADD_REQ {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_ADD_REQ) == 16, "PROTO_NC_GAMBLE_COIN_DB_ADD_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_DB_BUY_ACK {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_BUY_ACK) == 10, "PROTO_NC_GAMBLE_COIN_DB_BUY_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_DB_BUY_REQ {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_BUY_REQ) == 32, "PROTO_NC_GAMBLE_COIN_DB_BUY_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_DB_SELL_ACK {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_SELL_ACK) == 10, "PROTO_NC_GAMBLE_COIN_DB_SELL_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_DB_SELL_REQ {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_SELL_REQ) == 24, "PROTO_NC_GAMBLE_COIN_DB_SELL_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_ACK {
    NETPACKETZONEHEADER nHeader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_ACK) == 18, "PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_REQ {
    NETPACKETZONEHEADER nHeader;
    SHINE_ITEM_REGISTNUMBER nItemReg;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_REQ) == 24, "PROTO_NC_GAMBLE_COIN_DB_USE_COINITEM_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_CLOSE_CMD) == 1, "PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_CLOSE_CMD size drift");

struct PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_OPEN_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_OPEN_CMD) == 1, "PROTO_NC_GAMBLE_COIN_EXCHANGEMACHINE_UI_OPEN_CMD size drift");

struct PROTO_NC_GAMBLE_COIN_SELL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_SELL_ACK) == 2, "PROTO_NC_GAMBLE_COIN_SELL_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_SELL_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_SELL_REQ) == 8, "PROTO_NC_GAMBLE_COIN_SELL_REQ size drift");

struct PROTO_NC_GAMBLE_COIN_USE_COINITEM_MESSAGE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_USE_COINITEM_MESSAGE_CMD) == 8, "PROTO_NC_GAMBLE_COIN_USE_COINITEM_MESSAGE_CMD size drift");

struct PROTO_NC_GAMBLE_COIN_VIPCARD_UI_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_VIPCARD_UI_CLOSE_CMD) == 1, "PROTO_NC_GAMBLE_COIN_VIPCARD_UI_CLOSE_CMD size drift");

struct PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_ACK) == 2, "PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_ACK size drift");

struct PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_REQ) == 1, "PROTO_NC_GAMBLE_COIN_VIPCARD_UI_OPEN_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_ACK {
    uint8_t _pad_at_0000[3];
    GDT_DiceRankInfo_______0_bytes___ RankAll;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_ACK) == 3, "PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_REQ) == 1, "PROTO_NC_GAMBLE_DICE_TAISAI_ALL_RANK_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_ACK) == 4, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_ACK) == 4, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiBetting BettingCancelInfo;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_CMD) == 10, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_REQ {
    DiceTaiSaiBetting BettingCancelInfo;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_REQ) == 10, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CANCEL_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiBetting BettingInfo;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CMD) == 10, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_REQ {
    DiceTaiSaiBetting BettingInfo;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_REQ) == 10, "PROTO_NC_GAMBLE_DICE_TAISAI_BETTING_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_BET_START_CMD {
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _pad_at_0000[4];
    DiceTaiSaiInfo CurrentRollingDice;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_BET_START_CMD) == 7, "PROTO_NC_GAMBLE_DICE_TAISAI_BET_START_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_ACK {
    uint8_t _pad_at_0000[3];
    GDT_DiceRankInfo_______0_bytes___ RankCurr;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_ACK) == 3, "PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_REQ) == 1, "PROTO_NC_GAMBLE_DICE_TAISAI_CURR_RANK_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_ACK {
    uint8_t _pad_at_0000[2];
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0008[1];
    GDT_DiceRankInfo_______0_bytes___ RankAll;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_ACK) == 9, "PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_REQ) == 6, "PROTO_NC_GAMBLE_DICE_TAISAI_DB_ALL_RANK_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK {
    uint8_t _pad_at_0000[6];
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_000c[2];
    GDT_DiceRollApplyResult_______0_bytes___ DiceRollResult;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK) == 14, "PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[6];
    GDT_DiceRollApplyResultLog_______0_bytes___ DiceRollResultLog;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD) == 12, "PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_LOG_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ {
    NETPACKETZONEHEADER header;
    DiceTaiSaiInfo DiceNum;
    uint8_t _pad_at_0006[5];
    GDT_DiceRollApplyResult_______0_bytes___ DiceRollResult;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ) == 11, "PROTO_NC_GAMBLE_DICE_TAISAI_DB_DICE_ROLL_RESULT_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_CMD) == 2, "PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_CMD {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiInfo nDiceInfo;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_CMD) == 5, "PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD {
    uint8_t _pad_at_0000[1];
    GDT_EmotionInfo_______0_bytes___ Emotion;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD) == 1, "PROTO_NC_GAMBLE_DICE_TAISAI_DICE_ROLL_RESULT_EMOTION_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_BETTING_INFO_CMD {
    uint8_t _pad_at_0000[3];
    GDT_BettingInfo_______0_bytes___ Betting;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_BETTING_INFO_CMD) == 3, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_BETTING_INFO_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_ACK {
    uint8_t _pad_at_0000[2];
    DiceTaiSaiInfo_______21_bytes___ DiceHistory;
    uint8_t _pad_at_0002[21];
    DICE_TAISAI_GAME_STATUS nGameStatus;
    uint8_t _pad_at_0017[4];
    DICE_TAISAI_GAME_MODE bModeType;
    uint8_t _pad_at_001b[23];
    uint16_t nGamerHnd[0];
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_ACK) == 50, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_CMD) == 2, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_REQ) == 4, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_JOIN_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_ACK) == 2, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_CMD) == 3, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_REQ) == 1, "PROTO_NC_GAMBLE_DICE_TAISAI_GAME_LEAVE_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_ACK) == 2, "PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_ACK size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_CMD {
    uint8_t _pad_at_0000[1];
    GDT_LargeAmountInfo_______0_bytes___ LargeAmount;
};
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_CMD) == 1, "PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_CMD size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_REQ) == 4, "PROTO_NC_GAMBLE_DICE_TAISAI_LARGE_AMOUNT_REQ size drift");

struct PROTO_NC_GAMBLE_DICE_TAISAI_TIMER_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_GAMBLE_DICE_TAISAI_TIMER_CMD) == 7, "PROTO_NC_GAMBLE_DICE_TAISAI_TIMER_CMD size drift");

struct PROTO_NC_GAMBLE_ENTER_PLAYER_DIRECT_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_GAMBLE_ENTER_PLAYER_DIRECT_CMD) == 3, "PROTO_NC_GAMBLE_ENTER_PLAYER_DIRECT_CMD size drift");

struct PROTO_NC_GAMBLE_EXCHANGECOIN_CHANGE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_EXCHANGECOIN_CHANGE_CMD) == 8, "PROTO_NC_GAMBLE_EXCHANGECOIN_CHANGE_CMD size drift");

struct PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_ACK) == 12, "PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_ACK size drift");

struct PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_REQ) == 8, "PROTO_NC_GAMBLE_EXCHANGEDCOIN_DB_INFO_REQ size drift");

struct PROTO_NC_GAMBLE_EXCHANGEDCOIN_INIT_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_EXCHANGEDCOIN_INIT_CMD) == 4, "PROTO_NC_GAMBLE_EXCHANGEDCOIN_INIT_CMD size drift");

struct PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_ACK) == 2, "PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_ACK size drift");

struct PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_REQ) == 1, "PROTO_NC_GAMBLE_GAMBLEHOUSE_EXIT_REQ size drift");

struct PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_CLOSE_CMD) == 1, "PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_CLOSE_CMD size drift");

struct PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_OPEN_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_OPEN_CMD) == 1, "PROTO_NC_GAMBLE_GAMBLEHOUSE_UI_OPEN_CMD size drift");

struct PROTO_NC_GAMBLE_PLAYERACT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GAMBLE_PLAYERACT_CMD) == 6, "PROTO_NC_GAMBLE_PLAYERACT_CMD size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_ACK {
    uint8_t _pad_at_0000[8];
    SLOTMACHINE_JackPotRank_______0_bytes___ RankList;
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_ACK) == 8, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ {
    uint8_t _pad_at_0000[1];
    PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ___unnamed_type_MachineIndex________0_bytes___ MachineIndex;
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ___unnamed_type_MachineIndex_ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ___unnamed_type_MachineIndex_) == 3, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_GAMEINFO_REQ___unnamed_type_MachineIndex_ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_JACKPOT_RANK_UPDATE_CMD {
    uint8_t _pad_at_0000[1];
    SLOTMACHINE_JackPotRank NewJackpot;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_JACKPOT_RANK_UPDATE_CMD) == 25, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_JACKPOT_RANK_UPDATE_CMD size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_ACK) == 10, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[6];
    wchar_t CardDisplay[10];
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_REQ) == 36, "PROTO_NC_GAMBLE_SLOTMACHINE_DB_RESULT_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_ACK) == 4, "PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_REQ) == 4, "PROTO_NC_GAMBLE_SLOTMACHINE_GAME_JOIN_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_ACK) == 2, "PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_GAME_LEAVE_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD___unnamed_type_JackPotList________0_bytes___ JackPotList;
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD___unnamed_type_JackPotList_ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD___unnamed_type_JackPotList_) == 6, "PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTINFO_CMD___unnamed_type_JackPotList_ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_ACK {
    uint8_t _pad_at_0000[1];
    SLOTMACHINE_JackPotRank_______0_bytes___ RankList;
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_ACK) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_JACKPOTRANKING_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_SOMEONE_GET_JACKPOT_CMD {
    Name5 charid;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_SOMEONE_GET_JACKPOT_CMD) == 28, "PROTO_NC_GAMBLE_SLOTMACHINE_SOMEONE_GET_JACKPOT_CMD size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_STAND_UP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_STAND_UP_CMD) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_STAND_UP_CMD size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_START_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_START_ACK) == 2, "PROTO_NC_GAMBLE_SLOTMACHINE_START_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_START_REQ {
    SLOTMACHINE_BettingLine betting;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_START_REQ) == 4, "PROTO_NC_GAMBLE_SLOTMACHINE_START_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_ACK {
    uint8_t _pad_at_0000[2];
    SLOTMACHINE_ScreenState screen;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_ACK) == 11, "PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_STOPBUTTON_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_ACK {
    uint8_t _pad_at_0000[8];
    SLOTMACHINE_WinState WinLine;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_ACK) == 10, "PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_WHEELSTOP_REQ size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_ACK {
    uint8_t _pad_at_0000[1];
    SLOTMACHINE_JackPotRank_______0_bytes___ RankList;
};
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_ACK) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_ACK size drift");

struct PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_REQ) == 1, "PROTO_NC_GAMBLE_SLOTMACHINE_WINRANKING_REQ size drift");

struct PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_ACK {
    uint8_t _pad_at_0000[4];
    GAMBLE_TYPE nGambleType;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_ACK) == 9, "PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_ACK size drift");

struct PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_REQ) == 2, "PROTO_NC_GAMBLE_TYPE_AND_WHERE_STAND_REQ size drift");

struct PROTO_NC_GAMBLE_WORLD_PREVMAPNAME_CMD {
    uint8_t _pad_at_0000[2];
    Name3 sPrevMapName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GAMBLE_WORLD_PREVMAPNAME_CMD) == 18, "PROTO_NC_GAMBLE_WORLD_PREVMAPNAME_CMD size drift");

struct PROTO_NC_GAMBLE_ZONE_PREVMAPNAME_CMD {
    Name3 sPrevMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_GAMBLE_ZONE_PREVMAPNAME_CMD) == 12, "PROTO_NC_GAMBLE_ZONE_PREVMAPNAME_CMD size drift");

struct PROTO_NC_GAMIGO_NEW_TUTORIAL_CHAR_ENTER_GAME {
    uint8_t _pad_at_0000[8];
    PROTO_TUTORIAL_INFO TutorialInfo;
    uint8_t _pad_at_0008[5];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_GAMIGO_NEW_TUTORIAL_CHAR_ENTER_GAME) == 43, "PROTO_NC_GAMIGO_NEW_TUTORIAL_CHAR_ENTER_GAME size drift");

struct PROTO_NC_GUILD_ACADEMY_CHAT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CHAT_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_CHAT_ACK size drift");

struct PROTO_NC_GUILD_CHAT_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_GUILD_CHAT_REQ) == 2, "PROTO_NC_GUILD_CHAT_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_CHAT_CMD {
    uint8_t _pad_at_0000[4];
    Name5 talker;
    uint8_t _pad_at_0004[20];
    PROTO_NC_GUILD_CHAT_REQ chat;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CHAT_CMD) == 26, "PROTO_NC_GUILD_ACADEMY_CHAT_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_CHAT_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CHAT_REQ) == 2, "PROTO_NC_GUILD_ACADEMY_CHAT_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_CLEAR_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _pad_at_0006[22];
    GUILD_ACADEMY_REWARD_ITEM WantItem;
    uint8_t _pad_at_001c[33];
    EACH_MODIFY_ITEM_______0_bytes___ ModifyItem;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_ACK) == 61, "PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_REQ) == 15, "PROTO_NC_GUILD_ACADEMY_DB_ACADEMY_REWARD_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_ACK) == 4, "PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_ACK) == 4, "PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_CLEAR_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_ACK {
    uint8_t _pad_at_0000[5];
    GRADUATE_DATA GraduateData;
    uint8_t _tail[64];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_ACK) == 69, "PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_GET_GRADUATE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_ACADEMY_RANKING_DATA RankData;
    uint8_t _tail[48];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_ACK) == 53, "PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_GET_RANKING_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_ACADEMY_REWARD_ITEM_______0_bytes___ RewardItem;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_ACK) == 5, "PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_ACADEMY_REWARD_MONEY_______0_bytes___ RewardMoney;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_ACK) == 5, "PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_DB_GET_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_ACK) == 8, "PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_CMD) == 1, "PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_REQ {
    uint8_t _pad_at_0000[10];
    Name5 sName;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_REQ) == 32, "PROTO_NC_GUILD_ACADEMY_DB_GRADUATE_JOIN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_ACK { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_ACK) == 16, "PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_CMD) == 1, "PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_REQ { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_REQ) == 14, "PROTO_NC_GUILD_ACADEMY_DB_LEVEL_UP_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_ACK {
    uint8_t _pad_at_0000[9];
    GUILD_ACADEMY_MEMEBER_INFO_______0_bytes___ AcademyMemberList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_ACK) == 9, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_ACK {
    uint8_t _pad_at_0000[6];
    Name5 sInviteCharID;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_ACK) == 33, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_REQ {
    uint8_t _pad_at_0000[6];
    Name5 sInviteCharID;
    uint8_t _pad_at_0006[24];
    Name5 sTargetCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_REQ) == 51, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_GUILD_JOIN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_ACK {
    uint8_t _pad_at_0000[11];
    SHINE_GUILD_ACADEMY_MEMBER_LOGON_INFO dJoinInfo;
    uint8_t _tail[7];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_ACK) == 18, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_REQ {
    uint8_t _pad_at_0000[10];
    Name5 sCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_REQ) == 32, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_JOIN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_ACK) == 12, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_REQ {
    uint8_t _pad_at_0000[10];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_REQ) == 30, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_REQ) == 4, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_ACK {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_ACK) == 32, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_REQ {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_REQ) == 30, "PROTO_NC_GUILD_ACADEMY_DB_MEMBER_VANISH_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_ACK {
    uint8_t _pad_at_0000[14];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_ACK) == 14, "PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_REQ {
    uint8_t _pad_at_0000[8];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_REQ) == 8, "PROTO_NC_GUILD_ACADEMY_DB_NOTIFY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_DB_RANK_BALANCE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_REWARD_WANT_CMD { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_REWARD_WANT_CMD) == 22, "PROTO_NC_GUILD_ACADEMY_DB_REWARD_WANT_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_ACK {
    uint8_t _pad_at_0000[8];
    Name5 sName;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_ACK) == 36, "PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_ACK {
    uint8_t _pad_at_0000[10];
    Name5 sOldAcademyMasterName;
    uint8_t _pad_at_000a[24];
    Name5 sNewAcademyMasterName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_ACK) == 54, "PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_REQ {
    uint8_t _pad_at_0000[8];
    Name5 sOldAcademyMasterName;
    uint8_t _pad_at_0008[24];
    Name5 sNewAcademyMasterName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_REQ) == 52, "PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_BY_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_REQ {
    uint8_t _pad_at_0000[6];
    Name5 sName;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_REQ) == 34, "PROTO_NC_GUILD_ACADEMY_DB_SET_MASTER_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_ACK) == 4, "PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_REQ {
    uint8_t _pad_at_0000[7];
    GUILD_ACADEMY_REWARD_ITEM_______0_bytes___ RewardItem;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_REQ) == 7, "PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_ACK) == 4, "PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_REQ {
    uint8_t _pad_at_0000[7];
    GUILD_ACADEMY_REWARD_MONEY_______0_bytes___ RewardMoney;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_REQ) == 7, "PROTO_NC_GUILD_ACADEMY_DB_SET_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_DELETE_CMD {
    Name4 Name;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DELETE_CMD) == 16, "PROTO_NC_GUILD_ACADEMY_DELETE_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_DISMISS_CMD {
    Name4 Name;
    uint8_t _pad_at_0000[21];
    tm tm_dDismissDate;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_DISMISS_CMD) == 57, "PROTO_NC_GUILD_ACADEMY_DISMISS_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_ACK {
    uint8_t _pad_at_0000[3];
    GRADUATE_DATA GraduateData;
    uint8_t _tail[64];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_ACK) == 67, "PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_REQ {
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_REQ) == 16, "PROTO_NC_GUILD_ACADEMY_GET_GRADUATE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_ACADEMY_RANKING_DATA RankData;
    uint8_t _tail[48];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_ACK) == 51, "PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_REQ) == 4, "PROTO_NC_GUILD_ACADEMY_GET_RANKING_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_ACADEMY_REWARD_ITEM_______0_bytes___ RewardItem;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_ACK) == 3, "PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_GET_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_ACADEMY_REWARD_MONEY_______0_bytes___ RewardMoney;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_ACK) == 3, "PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_GET_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_ACK {
    uint8_t _pad_at_0000[10];
    GUILD_HISTORY_______0_bytes___ HistoryList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_ACK) == 10, "PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_HISTORY_DB_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_ACK {
    uint8_t _pad_at_0000[8];
    GUILD_HISTORY_______0_bytes___ HistoryList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_ACK) == 8, "PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_HISTORY_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_LIST_ACK {
    uint8_t _pad_at_0000[11];
    SHINE_GUILD_ACADEMY_LIST_______0_bytes___ GuildAcademyList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_LIST_ACK) == 11, "PROTO_NC_GUILD_ACADEMY_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_LIST_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_REQ {
    Name5 sTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_MASTER_TELEPORT_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_CMD {
    Name5 sChatBanCancelSourceCharID;
    uint8_t _pad_at_0000[20];
    Name5 sChatBanCancelTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_CMD) == 40, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_REQ {
    Name5 sChatBanCancelTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CANCEL_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CMD {
    Name5 sChatBanSourceCharID;
    uint8_t _pad_at_0000[20];
    Name5 sChatBanTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CMD) == 40, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_REQ {
    Name5 sChatBanTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_CHAT_BAN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_CLASS_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_CLASS_CMD) == 21, "PROTO_NC_GUILD_ACADEMY_MEMBER_CLASS_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_ACK { uint8_t data[18]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_ACK) == 18, "PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_ACK { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_ACK) == 14, "PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_REQ { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_REQ) == 12, "PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_CANCEL_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_REQ { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_REQ) == 16, "PROTO_NC_GUILD_ACADEMY_MEMBER_DB_CHAT_BAN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_DB_SAVE_CHAT_BAN_TIME_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_DB_SAVE_CHAT_BAN_TIME_CMD) == 12, "PROTO_NC_GUILD_ACADEMY_MEMBER_DB_SAVE_CHAT_BAN_TIME_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_ACK {
    Name5 sInviteTargetCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_ACK) == 22, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_REQ {
    Name5 sInviteTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_INVITE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ACK {
    Name4 GuildName;
    uint8_t _pad_at_0000[16];
    Name5 sInviteSourceCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ACK) == 37, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_CMD {
    Name5 sInviteSourceCharID;
    uint8_t _pad_at_0000[20];
    GUILD_MEMBER_CLIENT Member;
    uint8_t _tail[110];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_CMD) == 130, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ERR {
    Name4 GuildName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ERR) == 18, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_ERR size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_REQ {
    Name4 GuildName;
    uint8_t _pad_at_0000[16];
    Name5 sInviteSourceCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_REQ) == 36, "PROTO_NC_GUILD_ACADEMY_MEMBER_GUILD_JOIN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_INTRO_CMD {
    Name5 CharID;
    uint8_t _pad_at_0000[22];
    wchar_t sMemberIntro[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_INTRO_CMD) == 22, "PROTO_NC_GUILD_ACADEMY_MEMBER_INTRO_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_ACK {
    Name4 GuildName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_ACK) == 18, "PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_CMD {
    GUILD_ACADEMY_MEMBER_CLIENT Member;
    uint8_t _tail[109];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_CMD) == 109, "PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_REQ {
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_REQ) == 16, "PROTO_NC_GUILD_ACADEMY_MEMBER_JOIN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_CMD) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_MEMBER_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LEVEL_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LEVEL_CMD) == 21, "PROTO_NC_GUILD_ACADEMY_MEMBER_LEVEL_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_ACK {
    uint8_t _pad_at_0000[6];
    GUILD_ACADEMY_MEMBER_CLIENT_______0_bytes___ AcademyMemberList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_ACK) == 6, "PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_MEMBER_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LOGOFF_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LOGOFF_CMD) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_LOGOFF_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_LOGON_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_LOGON_CMD) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_LOGON_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_MAP_CMD {
    Name5 CharID;
    uint8_t _pad_at_0000[20];
    Name3 sMap;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_MAP_CMD) == 32, "PROTO_NC_GUILD_ACADEMY_MEMBER_MAP_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_PARTY_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_PARTY_CMD) == 21, "PROTO_NC_GUILD_ACADEMY_MEMBER_PARTY_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_ACK {
    Name5 CharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_ACK) == 22, "PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_CMD) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_REQ {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_MEMBER_VANISH_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_ACK { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_ACK) == 14, "PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_MY_ACADEMY_RANK_INFO_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_ACK {
    uint8_t _pad_at_0000[2];
    GUILD_ACADEMY_CLIENT_______0_bytes___ GuildAcademyInfo;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_MY_GUILD_ACADEMY_INFO_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_NOTIFY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_NOTIFY_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_NOTIFY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_NOTIFY_CMD {
    uint8_t _pad_at_0000[4];
    tm tm_dNotifyDate;
    uint8_t _pad_at_0004[36];
    Name5 sNotifyCharID;
    uint8_t _pad_at_0028[22];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_NOTIFY_CMD) == 62, "PROTO_NC_GUILD_ACADEMY_NOTIFY_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_NOTIFY_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_NOTIFY_REQ) == 2, "PROTO_NC_GUILD_ACADEMY_NOTIFY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_GRADE_INFO_CMD { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_GRADE_INFO_CMD) == 9, "PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_GRADE_INFO_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_ITEM_INFO_ZONE_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[12];
    Name5 Charid;
    uint8_t _pad_at_000c[21];
    SHINE_INVEN_SLOT_INFO From;
    uint8_t _pad_at_0021[16];
    SHINE_INVEN_SLOT_INFO To;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_ITEM_INFO_ZONE_RNG) == 65, "PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_ITEM_INFO_ZONE_RNG size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_REWARD_ZONE_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[11];
    Name5 sCharID;
    uint8_t _pad_at_000b[37];
    EACH_MODIFY_ITEM_______0_bytes___ Reward_ItemInfo;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_REWARD_ZONE_RNG) == 48, "PROTO_NC_GUILD_ACADEMY_REWARDSTORAGE_REWARD_ZONE_RNG size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_CLOSE_CMD) == 1, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_CLOSE_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_CMD {
    uint8_t _pad_at_0000[2];
    Name5 charid;
    uint8_t _tail[39];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_CMD) == 41, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[15];
    Name5 charid;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_RNG) == 53, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_RNG size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK {
    uint8_t _pad_at_0000[11];
    PROTO_ITEMPACKET_INFORM_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK) == 11, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_CMD {
    uint8_t _pad_at_0000[2];
    Name5 charid;
    uint8_t _tail[39];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_CMD) == 41, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_ACK) == 9, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_REQ) == 6, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_GRADE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[15];
    Name5 charid;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_RNG) == 53, "PROTO_NC_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_RNG size drift");

struct PROTO_NC_GUILD_ACADEMY_REWARD_WANT_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_REWARD_WANT_CMD) == 14, "PROTO_NC_GUILD_ACADEMY_REWARD_WANT_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_MASTER_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_MASTER_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_SET_MASTER_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_MASTER_BY_LEAVE_CMD {
    Name5 sOldAcademyMasterName;
    uint8_t _pad_at_0000[20];
    Name5 sNewAcademyMasterName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_MASTER_BY_LEAVE_CMD) == 40, "PROTO_NC_GUILD_ACADEMY_SET_MASTER_BY_LEAVE_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_MASTER_CMD {
    Name5 sName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_MASTER_CMD) == 20, "PROTO_NC_GUILD_ACADEMY_SET_MASTER_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_MASTER_REQ {
    Name5 sName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_MASTER_REQ) == 20, "PROTO_NC_GUILD_ACADEMY_SET_MASTER_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_REQ {
    uint8_t _pad_at_0000[1];
    GUILD_ACADEMY_REWARD_ITEM_______0_bytes___ RewardItem;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_SET_REWARD_ITEM_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_REQ {
    uint8_t _pad_at_0000[1];
    GUILD_ACADEMY_REWARD_MONEY_______0_bytes___ RewardMoney;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_SET_REWARD_MONEY_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_ALL_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_ACADEMY_DB_______0_bytes___ GuildAcademyDB;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_ALL_ACK) == 5, "PROTO_NC_GUILD_ACADEMY_START_DB_ALL_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_ALL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_ALL_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_START_DB_ALL_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_ACADEMY_START_RANKING_DATA RankData;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_ACK) == 11, "PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_REQ) == 4, "PROTO_NC_GUILD_ACADEMY_START_DB_GET_RANKING_LIST_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_ACK) == 2, "PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_ACK size drift");

struct PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_REQ) == 1, "PROTO_NC_GUILD_ACADEMY_START_DB_RANK_BALANCE_REQ size drift");

struct PROTO_NC_GUILD_ACADEMY_WAR_END_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_WAR_END_CMD) == 5, "PROTO_NC_GUILD_ACADEMY_WAR_END_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD__WAR_INFO_LIST_______0_bytes___ WarInfoList;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD) == 2, "PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD__WAR_INFO_LIST { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD__WAR_INFO_LIST) == 5, "PROTO_NC_GUILD_ACADEMY_WAR_INFO_LIST_CMD__WAR_INFO_LIST size drift");

struct PROTO_NC_GUILD_ACADEMY_WAR_START_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_WAR_START_CMD) == 5, "PROTO_NC_GUILD_ACADEMY_WAR_START_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_ACADEMY_MASTER_BUFF_CMD {
    uint8_t _pad_at_0000[4];
    GUILD_MASTER_BUFF_______0_bytes___ Master;
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_ACADEMY_MASTER_BUFF_CMD) == 4, "PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_ACADEMY_MASTER_BUFF_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_BUFF_CMD {
    wchar_t BuffName[32];
    uint32_t Guild[0];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_BUFF_CMD) == 36, "PROTO_NC_GUILD_ACADEMY_ZONE_GUILD_BUFF_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_MASTER_TELEPORT_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sTargetCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_MASTER_TELEPORT_CMD) == 24, "PROTO_NC_GUILD_ACADEMY_ZONE_MASTER_TELEPORT_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_GUILD_JOIN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_GUILD_JOIN_CMD) == 8, "PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_GUILD_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_JOIN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_JOIN_CMD) == 8, "PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_LEAVE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_LEAVE_CMD) == 4, "PROTO_NC_GUILD_ACADEMY_ZONE_MEMBER_LEAVE_CMD size drift");

struct PROTO_NC_GUILD_CHAT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_CHAT_ACK) == 2, "PROTO_NC_GUILD_CHAT_ACK size drift");

struct PROTO_NC_GUILD_CHAT_CMD {
    uint8_t _pad_at_0000[4];
    Name5 talker;
    uint8_t _pad_at_0004[20];
    PROTO_NC_GUILD_CHAT_REQ chat;
};
static_assert(sizeof(PROTO_NC_GUILD_CHAT_CMD) == 26, "PROTO_NC_GUILD_CHAT_CMD size drift");

struct PROTO_NC_GUILD_DB_ACK {
    uint8_t _pad_at_0000[4];
    GUILD_CLIENT GuildClient;
    uint8_t _pad_at_0004[1067];
    GUILD_SERVER GuildServer;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_ACK) == 1091, "PROTO_NC_GUILD_DB_ACK size drift");

struct PROTO_NC_GUILD_DB_ALL_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_DB_______0_bytes___ GuildDB;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_ALL_ACK) == 5, "PROTO_NC_GUILD_DB_ALL_ACK size drift");

struct PROTO_NC_GUILD_DB_ALL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_ALL_REQ) == 1, "PROTO_NC_GUILD_DB_ALL_REQ size drift");

struct PROTO_NC_GUILD_DB_DELETE_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_DELETE_ACK) == 6, "PROTO_NC_GUILD_DB_DELETE_ACK size drift");

struct PROTO_NC_GUILD_DB_DELETE_CANCEL_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_DELETE_CANCEL_ACK) == 8, "PROTO_NC_GUILD_DB_DELETE_CANCEL_ACK size drift");

struct PROTO_NC_GUILD_DB_DELETE_CANCEL_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_DELETE_CANCEL_REQ) == 6, "PROTO_NC_GUILD_DB_DELETE_CANCEL_REQ size drift");

struct PROTO_NC_GUILD_DB_DELETE_REQ {
    uint8_t _pad_at_0000[4];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_DELETE_REQ) == 20, "PROTO_NC_GUILD_DB_DELETE_REQ size drift");

struct PROTO_NC_GUILD_DB_DISMISS_ACK { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_DISMISS_ACK) == 13, "PROTO_NC_GUILD_DB_DISMISS_ACK size drift");

struct PROTO_NC_GUILD_DB_DISMISS_REQ { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_DISMISS_REQ) == 11, "PROTO_NC_GUILD_DB_DISMISS_REQ size drift");

struct PROTO_NC_GUILD_DB_GRADE_GROWTH_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_GRADE_GROWTH_ACK) == 19, "PROTO_NC_GUILD_DB_GRADE_GROWTH_ACK size drift");

struct PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ) == 29, "PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ size drift");

struct PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ req;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ_SEND) == 32, "PROTO_NC_GUILD_DB_GRADE_GROWTH_REQ_SEND size drift");

struct PROTO_NC_GUILD_DB_INTRO_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_INTRO_ACK) == 12, "PROTO_NC_GUILD_DB_INTRO_ACK size drift");

struct PROTO_NC_GUILD_DB_INTRO_REQ {
    uint8_t _pad_at_0000[12];
    wchar_t sIntro[0];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_INTRO_REQ) == 12, "PROTO_NC_GUILD_DB_INTRO_REQ size drift");

struct PROTO_NC_GUILD_DB_LIST_ACK {
    uint8_t _pad_at_0000[5];
    SHINE_GUILD_LIST_______0_bytes___ GuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_LIST_ACK) == 5, "PROTO_NC_GUILD_DB_LIST_ACK size drift");

struct PROTO_NC_GUILD_DB_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_LIST_REQ) == 1, "PROTO_NC_GUILD_DB_LIST_REQ size drift");

struct PROTO_NC_GUILD_MAKE_REQ {
    Name4 Name;
    uint8_t _pad_at_0000[16];
    Name3 Password;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_GUILD_MAKE_REQ) == 29, "PROTO_NC_GUILD_MAKE_REQ size drift");

struct PROTO_NC_GUILD_DB_MAKE_ACK {
    uint8_t _pad_at_0000[6];
    PROTO_NC_GUILD_MAKE_REQ Make;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MAKE_ACK) == 45, "PROTO_NC_GUILD_DB_MAKE_ACK size drift");

struct PROTO_NC_GUILD_DB_MAKE_REQ {
    uint8_t _pad_at_0000[6];
    Name5 sCharID;
    uint8_t _pad_at_0006[20];
    PROTO_NC_GUILD_MAKE_REQ Make;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MAKE_REQ) == 55, "PROTO_NC_GUILD_DB_MAKE_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_ACK {
    uint8_t _pad_at_0000[9];
    GUILD_MEMEBER_INFO_______0_bytes___ MemberList;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_ACK) == 9, "PROTO_NC_GUILD_DB_MEMBER_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_GRADE_ACK {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[35];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_GRADE_ACK) == 37, "PROTO_NC_GUILD_DB_MEMBER_GRADE_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_GRADE_REQ {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[34];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_GRADE_REQ) == 36, "PROTO_NC_GUILD_DB_MEMBER_GRADE_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_INTRO_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_INTRO_ACK) == 12, "PROTO_NC_GUILD_DB_MEMBER_INTRO_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_INTRO_REQ {
    uint8_t _pad_at_0000[12];
    wchar_t sMemberIntro[0];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_INTRO_REQ) == 12, "PROTO_NC_GUILD_DB_MEMBER_INTRO_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_JOIN_ACK { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_JOIN_ACK) == 13, "PROTO_NC_GUILD_DB_MEMBER_JOIN_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_JOIN_REQ {
    uint8_t _pad_at_0000[10];
    Name5 sCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_JOIN_REQ) == 31, "PROTO_NC_GUILD_DB_MEMBER_JOIN_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_LEAVE_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_LEAVE_ACK) == 12, "PROTO_NC_GUILD_DB_MEMBER_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_LEAVE_REQ {
    uint8_t _pad_at_0000[10];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_LEAVE_REQ) == 30, "PROTO_NC_GUILD_DB_MEMBER_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_REQ) == 4, "PROTO_NC_GUILD_DB_MEMBER_REQ size drift");

struct PROTO_NC_GUILD_DB_MEMBER_VANISH_ACK {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_VANISH_ACK) == 32, "PROTO_NC_GUILD_DB_MEMBER_VANISH_ACK size drift");

struct PROTO_NC_GUILD_DB_MEMBER_VANISH_REQ {
    uint8_t _pad_at_0000[2];
    Name5 sCharID;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_MEMBER_VANISH_REQ) == 30, "PROTO_NC_GUILD_DB_MEMBER_VANISH_REQ size drift");

struct PROTO_NC_GUILD_DB_MONEY_ADD_ACK { uint8_t data[36]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MONEY_ADD_ACK) == 36, "PROTO_NC_GUILD_DB_MONEY_ADD_ACK size drift");

struct PROTO_NC_GUILD_DB_MONEY_ADD_REQ { uint8_t data[18]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MONEY_ADD_REQ) == 18, "PROTO_NC_GUILD_DB_MONEY_ADD_REQ size drift");

struct PROTO_NC_GUILD_DB_MONEY_SUB_ACK { uint8_t data[36]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MONEY_SUB_ACK) == 36, "PROTO_NC_GUILD_DB_MONEY_SUB_ACK size drift");

struct PROTO_NC_GUILD_DB_MONEY_SUB_REQ { uint8_t data[18]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_MONEY_SUB_REQ) == 18, "PROTO_NC_GUILD_DB_MONEY_SUB_REQ size drift");

struct PROTO_NC_GUILD_DB_NOTIFY_ACK {
    uint8_t _pad_at_0000[12];
    Name5 sNotifyCharID;
    uint8_t _pad_at_000c[22];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_NOTIFY_ACK) == 34, "PROTO_NC_GUILD_DB_NOTIFY_ACK size drift");

struct PROTO_NC_GUILD_DB_NOTIFY_REQ {
    uint8_t _pad_at_0000[6];
    Name5 sNotifyCharID;
    uint8_t _pad_at_0006[22];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_NOTIFY_REQ) == 28, "PROTO_NC_GUILD_DB_NOTIFY_REQ size drift");

struct PROTO_NC_GUILD_DB_RENAME_ACK {
    uint8_t _pad_at_0000[6];
    Name4 sGuildName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_RENAME_ACK) == 24, "PROTO_NC_GUILD_DB_RENAME_ACK size drift");

struct PROTO_NC_GUILD_DB_RENAME_REQ {
    uint8_t _pad_at_0000[6];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_RENAME_REQ) == 22, "PROTO_NC_GUILD_DB_RENAME_REQ size drift");

struct PROTO_NC_GUILD_DB_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_REQ) == 6, "PROTO_NC_GUILD_DB_REQ size drift");

struct PROTO_NC_GUILD_DB_RESULT_WRITE_CMD {
    GUILD_DB_RESULT_WRITE Guild1;
    uint8_t _pad_at_0000[20];
    GUILD_DB_RESULT_WRITE Guild2;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_DB_RESULT_WRITE_CMD) == 40, "PROTO_NC_GUILD_DB_RESULT_WRITE_CMD size drift");

struct PROTO_NC_GUILD_DB_RETYPE_ACK { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_RETYPE_ACK) == 9, "PROTO_NC_GUILD_DB_RETYPE_ACK size drift");

struct PROTO_NC_GUILD_DB_RETYPE_REQ { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_RETYPE_REQ) == 7, "PROTO_NC_GUILD_DB_RETYPE_REQ size drift");

struct PROTO_NC_GUILD_DB_TOKEN_ALL_ACK {
    uint8_t _pad_at_0000[5];
    GUILD_TOKEN_DB_______0_bytes___ GuildTokenDB;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_TOKEN_ALL_ACK) == 5, "PROTO_NC_GUILD_DB_TOKEN_ALL_ACK size drift");

struct PROTO_NC_GUILD_DB_TOKEN_ALL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_TOKEN_ALL_REQ) == 1, "PROTO_NC_GUILD_DB_TOKEN_ALL_REQ size drift");

struct PROTO_NC_GUILD_DB_WAR_REQ { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_GUILD_DB_WAR_REQ) == 22, "PROTO_NC_GUILD_DB_WAR_REQ size drift");

struct PROTO_NC_GUILD_DB_WAR_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_GUILD_DB_WAR_REQ WarReq;
};
static_assert(sizeof(PROTO_NC_GUILD_DB_WAR_ACK) == 24, "PROTO_NC_GUILD_DB_WAR_ACK size drift");

struct PROTO_NC_GUILD_DELETE_CMD {
    Name4 Name;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_DELETE_CMD) == 16, "PROTO_NC_GUILD_DELETE_CMD size drift");

struct PROTO_NC_GUILD_DISMISS_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_DISMISS_ACK) == 2, "PROTO_NC_GUILD_DISMISS_ACK size drift");

struct PROTO_NC_GUILD_DISMISS_CMD {
    Name4 Name;
    uint8_t _pad_at_0000[21];
    tm tm_dDismissDate;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_GUILD_DISMISS_CMD) == 57, "PROTO_NC_GUILD_DISMISS_CMD size drift");

struct PROTO_NC_GUILD_DISMISS_REQ {
    Name4 Name;
    uint8_t _pad_at_0000[16];
    Name3 Password;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_GUILD_DISMISS_REQ) == 28, "PROTO_NC_GUILD_DISMISS_REQ size drift");

struct PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_ACK) == 2, "PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_ACK size drift");

struct PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_REQ) == 1, "PROTO_NC_GUILD_EMBLEM_CHECK_AVAILABILITY_REQ size drift");

struct PROTO_NC_GUILD_EMBLEM_INFO_CMD {
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_INFO_CMD) == 3, "PROTO_NC_GUILD_EMBLEM_INFO_CMD size drift");

struct PROTO_NC_GUILD_EMBLEM_INFO_DB_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_EMBLEM_INFO_DB_______0_bytes___ EmblemInfo;
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_INFO_DB_ACK) == 3, "PROTO_NC_GUILD_EMBLEM_INFO_DB_ACK size drift");

struct PROTO_NC_GUILD_EMBLEM_INFO_DB_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_INFO_DB_REQ) == 1, "PROTO_NC_GUILD_EMBLEM_INFO_DB_REQ size drift");

struct PROTO_NC_GUILD_EMBLEM_INFO_NOTICE_CMD {
    uint8_t _pad_at_0000[4];
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_INFO_NOTICE_CMD) == 6, "PROTO_NC_GUILD_EMBLEM_INFO_NOTICE_CMD size drift");

struct PROTO_NC_GUILD_EMBLEM_LEVELUP_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_LEVELUP_CMD) == 4, "PROTO_NC_GUILD_EMBLEM_LEVELUP_CMD size drift");

struct PROTO_NC_GUILD_EMBLEM_OFF_MSG_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_OFF_MSG_CMD) == 2, "PROTO_NC_GUILD_EMBLEM_OFF_MSG_CMD size drift");

struct PROTO_NC_GUILD_EMBLEM_SAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_SAVE_ACK) == 2, "PROTO_NC_GUILD_EMBLEM_SAVE_ACK size drift");

struct PROTO_NC_GUILD_EMBLEM_SAVE_DB_ACK {
    uint8_t _pad_at_0000[4];
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_SAVE_DB_ACK) == 10, "PROTO_NC_GUILD_EMBLEM_SAVE_DB_ACK size drift");

struct PROTO_NC_GUILD_EMBLEM_SAVE_DB_REQ {
    uint8_t _pad_at_0000[8];
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_SAVE_DB_REQ) == 14, "PROTO_NC_GUILD_EMBLEM_SAVE_DB_REQ size drift");

struct PROTO_NC_GUILD_EMBLEM_SAVE_REQ {
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_SAVE_REQ) == 2, "PROTO_NC_GUILD_EMBLEM_SAVE_REQ size drift");

struct PROTO_NC_GUILD_EMBLEM_STATE_DB_ACK { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_STATE_DB_ACK) == 9, "PROTO_NC_GUILD_EMBLEM_STATE_DB_ACK size drift");

struct PROTO_NC_GUILD_EMBLEM_STATE_DB_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_EMBLEM_STATE_DB_REQ) == 9, "PROTO_NC_GUILD_EMBLEM_STATE_DB_REQ size drift");

struct PROTO_NC_GUILD_GRADE_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_ACK) == 3, "PROTO_NC_GUILD_GRADE_ACK size drift");

struct PROTO_NC_GUILD_GRADE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_CMD) == 1, "PROTO_NC_GUILD_GRADE_CMD size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_ACK) == 2, "PROTO_NC_GUILD_GRADE_GROWTH_ACK size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_DATA_ACK { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_DATA_ACK) == 14, "PROTO_NC_GUILD_GRADE_GROWTH_DATA_ACK size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_DATA_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_DATA_REQ) == 4, "PROTO_NC_GUILD_GRADE_GROWTH_DATA_REQ size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_REQ) == 1, "PROTO_NC_GUILD_GRADE_GROWTH_REQ size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_ZONE_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_ZONE_ACK) == 8, "PROTO_NC_GUILD_GRADE_GROWTH_ZONE_ACK size drift");

struct PROTO_NC_GUILD_GRADE_GROWTH_ZONE_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_GROWTH_ZONE_REQ) == 6, "PROTO_NC_GUILD_GRADE_GROWTH_ZONE_REQ size drift");

struct PROTO_NC_GUILD_GRADE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_GRADE_REQ) == 1, "PROTO_NC_GUILD_GRADE_REQ size drift");

struct PROTO_NC_GUILD_ZONE_WAR_START_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_WAR_START_CMD) == 8, "PROTO_NC_GUILD_ZONE_WAR_START_CMD size drift");

struct PROTO_NC_GUILD_GUILDWARCONFIRM_ACK {
    PROTO_NC_GUILD_ZONE_WAR_START_CMD guildwarA;
    PROTO_NC_GUILD_ZONE_WAR_START_CMD guildwarB;
};
static_assert(sizeof(PROTO_NC_GUILD_GUILDWARCONFIRM_ACK) == 16, "PROTO_NC_GUILD_GUILDWARCONFIRM_ACK size drift");

struct PROTO_NC_GUILD_GUILDWARCONFIRM_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_GUILDWARCONFIRM_REQ) == 8, "PROTO_NC_GUILD_GUILDWARCONFIRM_REQ size drift");

struct PROTO_NC_GUILD_GUILDWARSTATUS_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_GUILD_ZONE_WAR_START_CMD_______0_bytes___ war;
};
static_assert(sizeof(PROTO_NC_GUILD_GUILDWARSTATUS_ACK) == 2, "PROTO_NC_GUILD_GUILDWARSTATUS_ACK size drift");

struct PROTO_NC_GUILD_GUILDWARSTATUS_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_GUILDWARSTATUS_REQ) == 1, "PROTO_NC_GUILD_GUILDWARSTATUS_REQ size drift");

struct PROTO_NC_GUILD_HISTORY_DB_LIST_ACK {
    uint8_t _pad_at_0000[10];
    GUILD_HISTORY_______0_bytes___ HistoryList;
};
static_assert(sizeof(PROTO_NC_GUILD_HISTORY_DB_LIST_ACK) == 10, "PROTO_NC_GUILD_HISTORY_DB_LIST_ACK size drift");

struct PROTO_NC_GUILD_HISTORY_DB_LIST_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_HISTORY_DB_LIST_REQ) == 6, "PROTO_NC_GUILD_HISTORY_DB_LIST_REQ size drift");

struct PROTO_NC_GUILD_HISTORY_LIST_ACK {
    uint8_t _pad_at_0000[8];
    GUILD_HISTORY_______0_bytes___ HistoryList;
};
static_assert(sizeof(PROTO_NC_GUILD_HISTORY_LIST_ACK) == 8, "PROTO_NC_GUILD_HISTORY_LIST_ACK size drift");

struct PROTO_NC_GUILD_HISTORY_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_HISTORY_LIST_REQ) == 1, "PROTO_NC_GUILD_HISTORY_LIST_REQ size drift");

struct PROTO_NC_GUILD_INFO_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_INFO_ACK) == 1, "PROTO_NC_GUILD_INFO_ACK size drift");

struct PROTO_NC_GUILD_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_INFO_REQ) == 1, "PROTO_NC_GUILD_INFO_REQ size drift");

struct PROTO_NC_GUILD_LIST_ACK {
    uint8_t _pad_at_0000[7];
    SHINE_GUILD_LIST_NEW_______0_bytes___ GuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_LIST_ACK) == 7, "PROTO_NC_GUILD_LIST_ACK size drift");

struct PROTO_NC_GUILD_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_LIST_REQ) == 1, "PROTO_NC_GUILD_LIST_REQ size drift");

struct PROTO_NC_GUILD_MAKE_ACK {
    uint8_t _pad_at_0000[6];
    Name4 Name;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MAKE_ACK) == 27, "PROTO_NC_GUILD_MAKE_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_CLASS_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_CLASS_CMD) == 21, "PROTO_NC_GUILD_MEMBER_CLASS_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_EXP_RATIO_CMD {
    Name5 CharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_EXP_RATIO_CMD) == 22, "PROTO_NC_GUILD_MEMBER_EXP_RATIO_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_FLAGS_CMD {
    Name5 CharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_FLAGS_CMD) == 24, "PROTO_NC_GUILD_MEMBER_FLAGS_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_GRADE_ACK {
    Name5 CharID;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_GRADE_ACK) == 23, "PROTO_NC_GUILD_MEMBER_GRADE_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_GRADE_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_GRADE_CMD) == 21, "PROTO_NC_GUILD_MEMBER_GRADE_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_GRADE_REQ {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_GRADE_REQ) == 21, "PROTO_NC_GUILD_MEMBER_GRADE_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_INTRO_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_INTRO_ACK) == 2, "PROTO_NC_GUILD_MEMBER_INTRO_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_INTRO_CMD {
    Name5 CharID;
    uint8_t _pad_at_0000[22];
    wchar_t sMemberIntro[0];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_INTRO_CMD) == 22, "PROTO_NC_GUILD_MEMBER_INTRO_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_INTRO_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t sMemberIntro[0];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_INTRO_REQ) == 2, "PROTO_NC_GUILD_MEMBER_INTRO_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_INVITE_ACK {
    Name5 ToCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_INVITE_ACK) == 22, "PROTO_NC_GUILD_MEMBER_INVITE_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_INVITE_REQ {
    Name5 ToCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_INVITE_REQ) == 20, "PROTO_NC_GUILD_MEMBER_INVITE_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_JOIN_ACK {
    Name4 GuildName;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_JOIN_ACK) == 17, "PROTO_NC_GUILD_MEMBER_JOIN_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_JOIN_CMD {
    GUILD_MEMBER_CLIENT Member;
    uint8_t _tail[110];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_JOIN_CMD) == 110, "PROTO_NC_GUILD_MEMBER_JOIN_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_JOIN_ERR {
    Name4 GuildName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_JOIN_ERR) == 18, "PROTO_NC_GUILD_MEMBER_JOIN_ERR size drift");

struct PROTO_NC_GUILD_MEMBER_JOIN_REQ {
    Name4 GuildName;
    uint8_t _pad_at_0000[16];
    Name5 FromCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_JOIN_REQ) == 36, "PROTO_NC_GUILD_MEMBER_JOIN_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LEAVE_ACK) == 2, "PROTO_NC_GUILD_MEMBER_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_LEAVE_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LEAVE_CMD) == 20, "PROTO_NC_GUILD_MEMBER_LEAVE_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LEAVE_REQ) == 1, "PROTO_NC_GUILD_MEMBER_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_LEVEL_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LEVEL_CMD) == 21, "PROTO_NC_GUILD_MEMBER_LEVEL_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_LIST_ACK {
    uint8_t _pad_at_0000[6];
    GUILD_MEMBER_CLIENT_______0_bytes___ MemberList;
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LIST_ACK) == 6, "PROTO_NC_GUILD_MEMBER_LIST_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LIST_REQ) == 1, "PROTO_NC_GUILD_MEMBER_LIST_REQ size drift");

struct PROTO_NC_GUILD_MEMBER_LOGOFF_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LOGOFF_CMD) == 20, "PROTO_NC_GUILD_MEMBER_LOGOFF_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_LOGON_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_LOGON_CMD) == 20, "PROTO_NC_GUILD_MEMBER_LOGON_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_MAP_CMD {
    Name5 CharID;
    uint8_t _pad_at_0000[20];
    Name3 sMap;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_MAP_CMD) == 32, "PROTO_NC_GUILD_MEMBER_MAP_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_PARTY_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_PARTY_CMD) == 21, "PROTO_NC_GUILD_MEMBER_PARTY_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_VANISH_ACK {
    Name5 CharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_VANISH_ACK) == 22, "PROTO_NC_GUILD_MEMBER_VANISH_ACK size drift");

struct PROTO_NC_GUILD_MEMBER_VANISH_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_VANISH_CMD) == 20, "PROTO_NC_GUILD_MEMBER_VANISH_CMD size drift");

struct PROTO_NC_GUILD_MEMBER_VANISH_REQ {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_MEMBER_VANISH_REQ) == 20, "PROTO_NC_GUILD_MEMBER_VANISH_REQ size drift");

struct PROTO_NC_GUILD_MOBGUILD_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_MOBGUILD_CMD) == 6, "PROTO_NC_GUILD_MOBGUILD_CMD size drift");

struct PROTO_NC_GUILD_MONEY_ADD_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_GUILD_MONEY_ADD_ACK) == 10, "PROTO_NC_GUILD_MONEY_ADD_ACK size drift");

struct PROTO_NC_GUILD_MONEY_ADD_CMD {
    Name5 CharID;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_GUILD_MONEY_ADD_CMD) == 36, "PROTO_NC_GUILD_MONEY_ADD_CMD size drift");

struct PROTO_NC_GUILD_MONEY_ADD_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_MONEY_ADD_REQ) == 8, "PROTO_NC_GUILD_MONEY_ADD_REQ size drift");

struct PROTO_NC_GUILD_MONEY_SUB_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_GUILD_MONEY_SUB_ACK) == 10, "PROTO_NC_GUILD_MONEY_SUB_ACK size drift");

struct PROTO_NC_GUILD_MONEY_SUB_CMD {
    Name5 CharID;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_GUILD_MONEY_SUB_CMD) == 36, "PROTO_NC_GUILD_MONEY_SUB_CMD size drift");

struct PROTO_NC_GUILD_MONEY_SUB_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_MONEY_SUB_REQ) == 8, "PROTO_NC_GUILD_MONEY_SUB_REQ size drift");

struct PROTO_NC_GUILD_MY_GUILD_INFO_ACK {
    uint8_t _pad_at_0000[2];
    GUILD_CLIENT_______0_bytes___ GuildInfo;
};
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_INFO_ACK) == 2, "PROTO_NC_GUILD_MY_GUILD_INFO_ACK size drift");

struct PROTO_NC_GUILD_MY_GUILD_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_INFO_REQ) == 1, "PROTO_NC_GUILD_MY_GUILD_INFO_REQ size drift");

struct PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_ACK) == 10, "PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_ACK size drift");

struct PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_REQ) == 1, "PROTO_NC_GUILD_MY_GUILD_TOKEN_INFO_REQ size drift");

struct PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_ACK {
    uint8_t _pad_at_0000[2];
    GT_COND_INFO GTCondInfo;
    uint8_t _tail[43];
};
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_ACK) == 45, "PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_ACK size drift");

struct PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_REQ) == 1, "PROTO_NC_GUILD_MY_GUILD_TOURNAMENT_MATCH_TIME_REQ size drift");

struct PROTO_NC_GUILD_NAME_ACK {
    uint8_t _pad_at_0000[4];
    Name4 Name;
    uint8_t _pad_at_0004[16];
    GUILD_EMBLEM_INFO EmblemInfo;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_GUILD_NAME_ACK) == 22, "PROTO_NC_GUILD_NAME_ACK size drift");

struct PROTO_NC_GUILD_NAME_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_NAME_REQ) == 4, "PROTO_NC_GUILD_NAME_REQ size drift");

struct PROTO_NC_GUILD_NOTIFY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_NOTIFY_ACK) == 2, "PROTO_NC_GUILD_NOTIFY_ACK size drift");

struct PROTO_NC_GUILD_NOTIFY_CMD {
    uint8_t _pad_at_0000[4];
    tm tm_dNotifyDate;
    uint8_t _pad_at_0004[36];
    Name5 sNotifyCharID;
    uint8_t _pad_at_0028[22];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_NOTIFY_CMD) == 62, "PROTO_NC_GUILD_NOTIFY_CMD size drift");

struct PROTO_NC_GUILD_NOTIFY_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_GUILD_NOTIFY_REQ) == 2, "PROTO_NC_GUILD_NOTIFY_REQ size drift");

struct PROTO_NC_GUILD_RENAME_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_RENAME_ACK) == 2, "PROTO_NC_GUILD_RENAME_ACK size drift");

struct PROTO_NC_GUILD_RENAME_CMD {
    uint8_t _pad_at_0000[4];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_RENAME_CMD) == 20, "PROTO_NC_GUILD_RENAME_CMD size drift");

struct PROTO_NC_GUILD_RENAME_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_GUILD_RENAME_CMD cmd;
};
static_assert(sizeof(PROTO_NC_GUILD_RENAME_CMD_SEND) == 23, "PROTO_NC_GUILD_RENAME_CMD_SEND size drift");

struct PROTO_NC_GUILD_RENAME_REQ {
    uint8_t _pad_at_0000[1];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_RENAME_REQ) == 17, "PROTO_NC_GUILD_RENAME_REQ size drift");

struct PROTO_NC_GUILD_RETYPE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_RETYPE_ACK) == 2, "PROTO_NC_GUILD_RETYPE_ACK size drift");

struct PROTO_NC_GUILD_RETYPE_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_RETYPE_CMD) == 5, "PROTO_NC_GUILD_RETYPE_CMD size drift");

struct PROTO_NC_GUILD_RETYPE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_RETYPE_REQ) == 2, "PROTO_NC_GUILD_RETYPE_REQ size drift");

struct PROTO_NC_GUILD_STORAGEOPEN_ACK {
    uint8_t _pad_at_0000[19];
    PROTO_ITEMPACKET_INFORM_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_GUILD_STORAGEOPEN_ACK) == 19, "PROTO_NC_GUILD_STORAGEOPEN_ACK size drift");

struct PROTO_NC_GUILD_STORAGEWITHDRAW_CMD {
    uint8_t _pad_at_0000[2];
    Name5 charid;
    uint8_t _tail[31];
};
static_assert(sizeof(PROTO_NC_GUILD_STORAGEWITHDRAW_CMD) == 33, "PROTO_NC_GUILD_STORAGEWITHDRAW_CMD size drift");

struct PROTO_NC_GUILD_STORAGEWITHDRAW_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[14];
    Name5 charid;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_GUILD_STORAGEWITHDRAW_RNG) == 44, "PROTO_NC_GUILD_STORAGEWITHDRAW_RNG size drift");

struct PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_ACK) == 9, "PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_ACK size drift");

struct PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
};
static_assert(sizeof(PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_REQ) == 6, "PROTO_NC_GUILD_STORAGE_WITHDRAW_GRADE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENTSTART_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENTSTART_CMD) == 14, "PROTO_NC_GUILD_TOURNAMENTSTART_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENTSTOPMSG_CMD { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENTSTOPMSG_CMD) == 20, "PROTO_NC_GUILD_TOURNAMENTSTOPMSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENTSTOP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENTSTOP_CMD) == 1, "PROTO_NC_GUILD_TOURNAMENTSTOP_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_ACK {
    uint8_t _pad_at_0000[7];
    uint32_t nFSGuildNo[0];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_ACK) == 7, "PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_REQ { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_REQ) == 7, "PROTO_NC_GUILD_TOURNAMENT_DB_FINAL_SELECTION_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_GET_ACK {
    uint8_t _pad_at_0000[7];
    int32_t MatchTime[9];
    GUILD_TOURNAMENT_LIST_DB_______155_bytes___ TournamentTree;
    uint8_t _tail[161];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_GET_ACK) == 204, "PROTO_NC_GUILD_TOURNAMENT_DB_GET_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_GET_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_GET_REQ) == 4, "PROTO_NC_GUILD_TOURNAMENT_DB_GET_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_ACK { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_ACK) == 14, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_ACK {
    uint8_t _pad_at_0000[11];
    SHINE_GUILD_LIST_NEW_______0_bytes___ JoinGuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_ACK) == 11, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_REQ) == 6, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_LIST_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_ACK) == 35, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_REQ) == 33, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_NEW_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_REQ { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_REQ) == 12, "PROTO_NC_GUILD_TOURNAMENT_DB_JOIN_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_ACK { uint8_t data[28]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_ACK) == 28, "PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ {
    uint8_t _pad_at_0000[12];
    PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ__ITEM_OPT_______0_bytes___ ItemOpt;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ) == 12, "PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ__ITEM_OPT { uint8_t data[33]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ__ITEM_OPT) == 33, "PROTO_NC_GUILD_TOURNAMENT_DB_REWARD_CREATE_REQ__ITEM_OPT size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_ACK) == 6, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_ACK) == 6, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_REQ {
    uint8_t _pad_at_0000[6];
    GUILD_TOURNAMENT_LIST_DB_______155_bytes___ TournamentTree;
    uint8_t _tail[155];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_REQ) == 161, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_ACK) == 6, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_REQ {
    uint8_t _pad_at_0000[6];
    int32_t MatchTime[9];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_REQ) == 42, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_MATCH_TIME_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_REQ {
    uint8_t _pad_at_0000[7];
    GUILD_TOURNAMENT_LIST_DB_______155_bytes___ TournamentTree;
    uint8_t _tail[155];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_REQ) == 162, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_ACK { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_ACK) == 9, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_REQ { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_REQ) == 11, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_RESULT_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_ACK {
    uint8_t _pad_at_0000[8];
    GUILD_TOURNAMENT_LIST_DB_______0_bytes___ FinalSelectionGuild;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_ACK) == 8, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_REQ) == 8, "PROTO_NC_GUILD_TOURNAMENT_DB_SET_TYPE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DICEGAME_BEFORE_END_TIME_MSG_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DICEGAME_BEFORE_END_TIME_MSG_CMD) == 4, "PROTO_NC_GUILD_TOURNAMENT_DICEGAME_BEFORE_END_TIME_MSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DICEGAME_START_CMD {
    uint8_t _pad_at_0000[6];
    Name5 AGuildDelegateName;
    uint8_t _pad_at_0006[26];
    Name5 BGuildDelegateName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DICEGAME_START_CMD) == 52, "PROTO_NC_GUILD_TOURNAMENT_DICEGAME_START_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_ACK) == 2, "PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_CMD) == 8, "PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_DICEGAME_THROW_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_END_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_END_CMD) == 12, "PROTO_NC_GUILD_TOURNAMENT_END_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_ACK {
    uint8_t _pad_at_0000[5];
    GT_EnterList_______0_bytes___ EnterGuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_ACK) == 5, "PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_REQ) == 4, "PROTO_NC_GUILD_TOURNAMENT_ENTER_LIST_DB_GET_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_ACK) == 2, "PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_CMD) == 4, "PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_REQ) == 2, "PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_RESULT_CMD {
    Name5 CIDPlayer;
    uint8_t _pad_at_0000[20];
    Name5 CIDFlag;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_RESULT_CMD) == 48, "PROTO_NC_GUILD_TOURNAMENT_FLAGCAPTURE_RESULT_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_CMD) == 8, "PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_ZONE_CMD {
    uint8_t _pad_at_0000[3];
    GT_EnterList_______0_bytes___ EnterGuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_ZONE_CMD) == 3, "PROTO_NC_GUILD_TOURNAMENT_GOLD_REFUND_ZONE_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ITEM_EFFECT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ITEM_EFFECT_CMD) == 2, "PROTO_NC_GUILD_TOURNAMENT_ITEM_EFFECT_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ITEM_FLAG_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ITEM_FLAG_CMD) == 5, "PROTO_NC_GUILD_TOURNAMENT_ITEM_FLAG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ITEM_PICK_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ITEM_PICK_CMD) == 2, "PROTO_NC_GUILD_TOURNAMENT_ITEM_PICK_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD__Memberinfo_______0_bytes___ Members;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD) == 2, "PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD__Memberinfo {
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD__Memberinfo) == 8, "PROTO_NC_GUILD_TOURNAMENT_ITEM_SCAN_CMD__Memberinfo size drift");

struct PROTO_NC_GUILD_TOURNAMENT_JOIN_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_JOIN_ACK) == 8, "PROTO_NC_GUILD_TOURNAMENT_JOIN_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_ACK {
    uint8_t _pad_at_0000[9];
    SHINE_GUILD_LIST_NEW_______0_bytes___ JoinGuildList;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_ACK) == 9, "PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_JOIN_LIST_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_JOIN_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_JOIN_REQ) == 4, "PROTO_NC_GUILD_TOURNAMENT_JOIN_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_LAST_WINNER_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_LAST_WINNER_CMD) == 5, "PROTO_NC_GUILD_TOURNAMENT_LAST_WINNER_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_LEAVE_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_LEAVE_ACK) == 8, "PROTO_NC_GUILD_TOURNAMENT_LEAVE_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_LEAVE_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_LEAVE_REQ) == 4, "PROTO_NC_GUILD_TOURNAMENT_LEAVE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_LIST_ACK {
    uint8_t _pad_at_0000[38];
    tm tm_Time_Start;
    uint8_t _pad_at_0026[36];
    tm tm_Time_Practic;
    uint8_t _pad_at_004a[36];
    tm tm_Time_PracticEnd;
    uint8_t _pad_at_006e[36];
    tm tm_Time_Match_161;
    uint8_t _pad_at_0092[36];
    tm tm_Time_Match_162;
    uint8_t _pad_at_00b6[36];
    tm tm_Time_Match_8;
    uint8_t _pad_at_00da[36];
    tm tm_Time_Match_4;
    uint8_t _pad_at_00fe[36];
    tm tm_Time_Match_2;
    uint8_t _pad_at_0122[36];
    tm tm_Time_Match_End;
    uint8_t _pad_at_0146[37];
    GUILD_TOURNAMENT_LIST_______651_bytes___ TournamentList;
    uint8_t _tail[651];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_LIST_ACK) == 1014, "PROTO_NC_GUILD_TOURNAMENT_LIST_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_LIST_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_LIST_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_MANAGERUSERMSG_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_MANAGERUSERMSG_CMD) == 20, "PROTO_NC_GUILD_TOURNAMENT_MANAGERUSERMSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_NOTIFY_CMD {
    GUILD_TOURNAMENT_NOTIFY_TYPE Type;
    uint8_t _pad_at_0000[5];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_NOTIFY_CMD) == 21, "PROTO_NC_GUILD_TOURNAMENT_NOTIFY_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_ACK { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_ACK) == 11, "PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_OBSERVER_ENTER_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_ACK) == 2, "PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_OBSERVER_OUT_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_PLAYERDIEMSG_CMD {
    Name5 VictimCharID;
    uint8_t _pad_at_0000[20];
    Name5 MurderCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_PLAYERDIEMSG_CMD) == 44, "PROTO_NC_GUILD_TOURNAMENT_PLAYERDIEMSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_PLAYERKILLMSG_CMD {
    Name5 VictimCharID;
    uint8_t _pad_at_0000[20];
    Name5 MurderCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_PLAYERKILLMSG_CMD) == 44, "PROTO_NC_GUILD_TOURNAMENT_PLAYERKILLMSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_ACK) == 2, "PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_CMD) == 1, "PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_REQ) == 1, "PROTO_NC_GUILD_TOURNAMENT_RECALL_ENTER_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_CMD) == 8, "PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_ACK) == 2, "PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_REQ) == 8, "PROTO_NC_GUILD_TOURNAMENT_REFUND_NOTICE_DB_SET_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_RESET_CMD {
    uint8_t _pad_at_0000[5];
    int32_t MatchTime[9];
    GUILD_TOURNAMENT_LIST_DB_______155_bytes___ TournamentTree;
    uint8_t _tail[155];
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_RESET_CMD) == 196, "PROTO_NC_GUILD_TOURNAMENT_RESET_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_RESULT_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_RESULT_CMD) == 12, "PROTO_NC_GUILD_TOURNAMENT_RESULT_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_SCORE_CMD {
    uint8_t _pad_at_0000[17];
    TOURNAMENT_PLAYER_SCORE_______0_bytes___ PlayerScore;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_SCORE_CMD) == 17, "PROTO_NC_GUILD_TOURNAMENT_SCORE_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_SKILLPOINT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_SKILLPOINT_CMD) == 2, "PROTO_NC_GUILD_TOURNAMENT_SKILLPOINT_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_STARTMSG_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_STARTMSG_CMD) == 4, "PROTO_NC_GUILD_TOURNAMENT_STARTMSG_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_START_CMD { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_START_CMD) == 17, "PROTO_NC_GUILD_TOURNAMENT_START_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_TYPE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_TYPE_CMD) == 1, "PROTO_NC_GUILD_TOURNAMENT_TYPE_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_USESKILL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_USESKILL_ACK) == 4, "PROTO_NC_GUILD_TOURNAMENT_USESKILL_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_USESKILL_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_USESKILL_CMD) == 6, "PROTO_NC_GUILD_TOURNAMENT_USESKILL_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_USESKILL_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_USESKILL_REQ) == 2, "PROTO_NC_GUILD_TOURNAMENT_USESKILL_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_ENTER_CMD { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_ENTER_CMD) == 9, "PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_ENTER_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_OUT_CMD { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_OUT_CMD) == 9, "PROTO_NC_GUILD_TOURNAMENT_ZONE_FIGHTER_OUT_CMD size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_ACK { uint8_t data[27]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_ACK) == 27, "PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_REQ { uint8_t data[25]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_REQ) == 25, "PROTO_NC_GUILD_TOURNAMENT_ZONE_JOIN_NEW_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_ACK {
    uint8_t _pad_at_0000[9];
    GUILD_TOURNAMENT_MEMBER_GRADE_LIST_______0_bytes___ MemberGradeList;
};
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_ACK) == 9, "PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_REQ { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_REQ) == 5, "PROTO_NC_GUILD_TOURNAMENT_ZONE_MEMBERGRADE_REQ size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_ACK { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_ACK) == 5, "PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_ACK size drift");

struct PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_REQ) == 3, "PROTO_NC_GUILD_TOURNAMENT_ZONE_OBSERVER_ENTER_REQ size drift");

struct PROTO_NC_GUILD_WAR_ABLE_LIST_ACK {
    uint8_t _pad_at_0000[5];
    SHINE_GUILD_WAR_ABLE_INFO_______0_bytes___ WarInfoList;
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_ABLE_LIST_ACK) == 5, "PROTO_NC_GUILD_WAR_ABLE_LIST_ACK size drift");

struct PROTO_NC_GUILD_WAR_ABLE_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_WAR_ABLE_LIST_REQ) == 1, "PROTO_NC_GUILD_WAR_ABLE_LIST_REQ size drift");

struct PROTO_NC_GUILD_WAR_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_GUILD_WAR_ACK) == 2, "PROTO_NC_GUILD_WAR_ACK size drift");

struct PROTO_NC_GUILD_WAR_COOLDOWN_DONE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_WAR_COOLDOWN_DONE_CMD) == 1, "PROTO_NC_GUILD_WAR_COOLDOWN_DONE_CMD size drift");

struct PROTO_NC_GUILD_WAR_END_CMD {
    Name4 EnemyGuildName;
    uint8_t _tail[34];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_END_CMD) == 34, "PROTO_NC_GUILD_WAR_END_CMD size drift");

struct PROTO_NC_GUILD_WAR_LIST_ACK {
    uint8_t _pad_at_0000[5];
    SHINE_GUILD_WAR_INFO_______0_bytes___ WarInfoList;
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_LIST_ACK) == 5, "PROTO_NC_GUILD_WAR_LIST_ACK size drift");

struct PROTO_NC_GUILD_WAR_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_WAR_LIST_REQ) == 1, "PROTO_NC_GUILD_WAR_LIST_REQ size drift");

struct PROTO_NC_GUILD_WAR_REQ {
    Name4 TargetName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_REQ) == 16, "PROTO_NC_GUILD_WAR_REQ size drift");

struct PROTO_NC_GUILD_WAR_SCORE_ACK {
    uint8_t _pad_at_0000[2];
    Name4 EnemyGuildName;
    uint8_t _pad_at_0002[16];
    SHINE_GUILD_SCORE MyScore;
    uint8_t _pad_at_0012[42];
    SHINE_GUILD_SCORE EnemyScore;
    uint8_t _tail[42];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_SCORE_ACK) == 102, "PROTO_NC_GUILD_WAR_SCORE_ACK size drift");

struct PROTO_NC_GUILD_WAR_SCORE_CMD {
    Name4 GuildName1;
    uint8_t _pad_at_0000[16];
    SHINE_GUILD_SCORE GuildScore1;
    uint8_t _pad_at_0010[42];
    Name4 GuildName2;
    uint8_t _pad_at_003a[16];
    SHINE_GUILD_SCORE GuildScore2;
    uint8_t _tail[42];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_SCORE_CMD) == 116, "PROTO_NC_GUILD_WAR_SCORE_CMD size drift");

struct PROTO_NC_GUILD_WAR_SCORE_REQ {
    Name4 GuildNameEnemy;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_SCORE_REQ) == 16, "PROTO_NC_GUILD_WAR_SCORE_REQ size drift");

struct PROTO_NC_GUILD_WAR_START_CMD {
    Name4 WarGuildName;
    uint8_t _pad_at_0000[24];
    tm tm_EndDate;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_START_CMD) == 60, "PROTO_NC_GUILD_WAR_START_CMD size drift");

struct PROTO_NC_GUILD_WAR_TARGET_CMD {
    SHINE_GUILD_WAR_INFO WarInfo;
    uint8_t _tail[234];
};
static_assert(sizeof(PROTO_NC_GUILD_WAR_TARGET_CMD) == 234, "PROTO_NC_GUILD_WAR_TARGET_CMD size drift");

struct PROTO_NC_GUILD_WORLD_RENAME_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _pad_at_0006[1];
    Name4 sGuildName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RENAME_ACK) == 25, "PROTO_NC_GUILD_WORLD_RENAME_ACK size drift");

struct PROTO_NC_GUILD_WORLD_RENAME_CMD {
    NETPACKETZONEHEADER netpacketzoneheader;
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RENAME_CMD) == 22, "PROTO_NC_GUILD_WORLD_RENAME_CMD size drift");

struct PROTO_NC_GUILD_WORLD_RENAME_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _pad_at_0006[1];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RENAME_REQ) == 23, "PROTO_NC_GUILD_WORLD_RENAME_REQ size drift");

struct PROTO_NC_GUILD_WORLD_RETYPE_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RETYPE_ACK) == 10, "PROTO_NC_GUILD_WORLD_RETYPE_ACK size drift");

struct PROTO_NC_GUILD_WORLD_RETYPE_CMD {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RETYPE_CMD) == 7, "PROTO_NC_GUILD_WORLD_RETYPE_CMD size drift");

struct PROTO_NC_GUILD_WORLD_RETYPE_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_GUILD_WORLD_RETYPE_REQ) == 8, "PROTO_NC_GUILD_WORLD_RETYPE_REQ size drift");

struct PROTO_NC_GUILD_WORLD_SET_GUILD_TOKEN_CMD { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_GUILD_WORLD_SET_GUILD_TOKEN_CMD) == 16, "PROTO_NC_GUILD_WORLD_SET_GUILD_TOKEN_CMD size drift");

struct PROTO_NC_GUILD_WORLD_USE_GUILD_TOKEN_CMD { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_GUILD_WORLD_USE_GUILD_TOKEN_CMD) == 20, "PROTO_NC_GUILD_WORLD_USE_GUILD_TOKEN_CMD size drift");

struct PROTO_NC_GUILD_ZONE_DELETE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_DELETE_CMD) == 4, "PROTO_NC_GUILD_ZONE_DELETE_CMD size drift");

struct PROTO_NC_GUILD_ZONE_LIST_ACK {
    uint8_t _pad_at_0000[3];
    GUILD_ZONE_______0_bytes___ GuildZoneList;
};
static_assert(sizeof(PROTO_NC_GUILD_ZONE_LIST_ACK) == 3, "PROTO_NC_GUILD_ZONE_LIST_ACK size drift");

struct PROTO_NC_GUILD_ZONE_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_LIST_REQ) == 1, "PROTO_NC_GUILD_ZONE_LIST_REQ size drift");

struct PROTO_NC_GUILD_ZONE_MAKE_CMD {
    uint8_t _pad_at_0000[4];
    GUILD_ZONE GuildZone;
    uint8_t _tail[49];
};
static_assert(sizeof(PROTO_NC_GUILD_ZONE_MAKE_CMD) == 53, "PROTO_NC_GUILD_ZONE_MAKE_CMD size drift");

struct PROTO_NC_GUILD_ZONE_MEMBER_JOIN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_MEMBER_JOIN_CMD) == 8, "PROTO_NC_GUILD_ZONE_MEMBER_JOIN_CMD size drift");

struct PROTO_NC_GUILD_ZONE_MEMBER_LEAVE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_MEMBER_LEAVE_CMD) == 4, "PROTO_NC_GUILD_ZONE_MEMBER_LEAVE_CMD size drift");

struct PROTO_NC_GUILD_ZONE_USE_GUILD_TOKEN_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_USE_GUILD_TOKEN_CMD) == 12, "PROTO_NC_GUILD_ZONE_USE_GUILD_TOKEN_CMD size drift");

struct PROTO_NC_GUILD_ZONE_WAR_END_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_WAR_END_CMD) == 8, "PROTO_NC_GUILD_ZONE_WAR_END_CMD size drift");

struct PROTO_NC_GUILD_ZONE_WAR_KILL_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_GUILD_ZONE_WAR_KILL_CMD) == 4, "PROTO_NC_GUILD_ZONE_WAR_KILL_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_ADD_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_HOLY_PROMISE_INFO MemberInfo;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_ADD_CMD) == 28, "PROTO_NC_HOLY_PROMISE_ADD_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_CENTRANSFER_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[15];
    Name5 lower;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_CENTRANSFER_RNG) == 35, "PROTO_NC_HOLY_PROMISE_CENTRANSFER_RNG size drift");

struct PROTO_NC_HOLY_PROMISE_CLIENT_GET_REMAIN_MONEY_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_CLIENT_GET_REMAIN_MONEY_CMD) == 8, "PROTO_NC_HOLY_PROMISE_CLIENT_GET_REMAIN_MONEY_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_ACK) == 6, "PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_REQ) == 4, "PROTO_NC_HOLY_PROMISE_DB_DEL_CHAR_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_ACK {
    uint8_t _pad_at_0000[8];
    Name5 DownCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_ACK) == 30, "PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_REQ {
    uint8_t _pad_at_0000[8];
    Name5 DownCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_REQ) == 28, "PROTO_NC_HOLY_PROMISE_DB_DEL_DOWN_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_UP_ACK { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_UP_ACK) == 11, "PROTO_NC_HOLY_PROMISE_DB_DEL_UP_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_DEL_UP_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_DEL_UP_REQ) == 9, "PROTO_NC_HOLY_PROMISE_DB_DEL_UP_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_ACK) == 16, "PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_REQ) == 6, "PROTO_NC_HOLY_PROMISE_DB_GET_CEN_REWARD_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_ACK {
    uint8_t _pad_at_0000[8];
    PROTO_HOLY_PROMISE_INFO_DB_______0_bytes___ MemberInfo;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_ACK) == 8, "PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_REQ) == 4, "PROTO_NC_HOLY_PROMISE_DB_GET_MEMBER_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_REMAIN_MONEY_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_REMAIN_MONEY_CMD) == 12, "PROTO_NC_HOLY_PROMISE_DB_GET_REMAIN_MONEY_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_UP_ACK {
    uint8_t _pad_at_0000[6];
    PROTO_HOLY_PROMISE_INFO_DB UpMemberInfo;
    uint8_t _tail[42];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_UP_ACK) == 48, "PROTO_NC_HOLY_PROMISE_DB_GET_UP_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_GET_UP_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_GET_UP_REQ) == 4, "PROTO_NC_HOLY_PROMISE_DB_GET_UP_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK {
    uint8_t _pad_at_0000[9];
    PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK___unnamed_type_RewardList________0_bytes___ RewardList;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK) == 9, "PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK___unnamed_type_RewardList_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK___unnamed_type_RewardList_) == 4, "PROTO_NC_HOLY_PROMISE_DB_REWARD_ACK___unnamed_type_RewardList_ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_REWARD_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_REWARD_REQ) == 6, "PROTO_NC_HOLY_PROMISE_DB_REWARD_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_SET_DATE_ACK {
    uint8_t _pad_at_0000[4];
    PROTO_HOLY_PROMISE_DATE RejoinableDate;
    uint8_t _pad_at_0004[4];
    PROTO_HOLY_PROMISE_DATE MemberAcceptableDate;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_SET_DATE_ACK) == 14, "PROTO_NC_HOLY_PROMISE_DB_SET_DATE_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_SET_DATE_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_HOLY_PROMISE_DATE RejoinableDate;
    uint8_t _pad_at_0004[4];
    PROTO_HOLY_PROMISE_DATE MemberAcceptableDate;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_SET_DATE_REQ) == 12, "PROTO_NC_HOLY_PROMISE_DB_SET_DATE_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_SET_UP_ACK {
    PROTO_HOLY_PROMISE_INFO_DB ReqInfo;
    uint8_t _pad_at_0000[42];
    PROTO_HOLY_PROMISE_INFO_DB UpInfo;
    uint8_t _tail[44];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_SET_UP_ACK) == 86, "PROTO_NC_HOLY_PROMISE_DB_SET_UP_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_SET_UP_REQ {
    PROTO_HOLY_PROMISE_INFO_DB ReqInfo;
    uint8_t _pad_at_0000[42];
    PROTO_HOLY_PROMISE_INFO_DB UpInfo;
    uint8_t _tail[42];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_SET_UP_REQ) == 84, "PROTO_NC_HOLY_PROMISE_DB_SET_UP_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_ACK) == 16, "PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_REQ) == 6, "PROTO_NC_HOLY_PROMISE_DB_WITHDRAW_CEN_REWARD_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DEL_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DEL_CMD) == 21, "PROTO_NC_HOLY_PROMISE_DEL_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_DEL_DOWN_ACK {
    Name5 DownCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DEL_DOWN_ACK) == 22, "PROTO_NC_HOLY_PROMISE_DEL_DOWN_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DEL_DOWN_REQ {
    Name5 DownCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DEL_DOWN_REQ) == 20, "PROTO_NC_HOLY_PROMISE_DEL_DOWN_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_DEL_UP_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DEL_UP_ACK) == 3, "PROTO_NC_HOLY_PROMISE_DEL_UP_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_DEL_UP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_DEL_UP_REQ) == 1, "PROTO_NC_HOLY_PROMISE_DEL_UP_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_ACK) == 10, "PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_REQ) == 1, "PROTO_NC_HOLY_PROMISE_GET_CEN_REWARD_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_LEVEL_CMD {
    Name5 CharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_LEVEL_CMD) == 21, "PROTO_NC_HOLY_PROMISE_LEVEL_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_LIST_CMD {
    PROTO_HOLY_PROMISE_INFO UpInfo;
    uint8_t _pad_at_0000[29];
    PROTO_HOLY_PROMISE_INFO_______0_bytes___ MemberInfo;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_LIST_CMD) == 29, "PROTO_NC_HOLY_PROMISE_LIST_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_LOGIN_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_LOGIN_CMD) == 20, "PROTO_NC_HOLY_PROMISE_LOGIN_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_LOGOUT_CMD {
    Name5 CharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_LOGOUT_CMD) == 20, "PROTO_NC_HOLY_PROMISE_LOGOUT_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_MYUPPER_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_MYUPPER_ACK) == 10, "PROTO_NC_HOLY_PROMISE_MYUPPER_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_MYUPPER_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_MYUPPER_REQ) == 6, "PROTO_NC_HOLY_PROMISE_MYUPPER_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_MY_UP_ZONE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_MY_UP_ZONE) == 8, "PROTO_NC_HOLY_PROMISE_MY_UP_ZONE size drift");

struct PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD___unnamed_type_RewardList________0_bytes___ RewardList;
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD) == 1, "PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD___unnamed_type_RewardList_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD___unnamed_type_RewardList_) == 4, "PROTO_NC_HOLY_PROMISE_REWARD_ITEM_CMD___unnamed_type_RewardList_ size drift");

struct PROTO_NC_HOLY_PROMISE_REWARD_MONEY_CMD {
    Name5 MemberCharID;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_REWARD_MONEY_CMD) == 28, "PROTO_NC_HOLY_PROMISE_REWARD_MONEY_CMD size drift");

struct PROTO_NC_HOLY_PROMISE_SET_UP_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_HOLY_PROMISE_INFO UpInfo;
    uint8_t _pad_at_0002[30];
    tm tm_ResetableDateTime;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_SET_UP_ACK) == 68, "PROTO_NC_HOLY_PROMISE_SET_UP_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ACK {
    Name5 CharID;
    uint8_t _pad_at_0000[20];
    Name5 UpCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ACK) == 41, "PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ING {
    Name5 UpCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ING) == 20, "PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_ING size drift");

struct PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_REQ {
    Name5 CharID;
    uint8_t _pad_at_0000[20];
    Name5 UpCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_REQ) == 40, "PROTO_NC_HOLY_PROMISE_SET_UP_CONFIRM_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_SET_UP_REQ {
    Name5 CharID;
    uint8_t _pad_at_0000[20];
    Name5 UpCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_SET_UP_REQ) == 40, "PROTO_NC_HOLY_PROMISE_SET_UP_REQ size drift");

struct PROTO_NC_HOLY_PROMISE_USE_MONEY_ZONE { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_USE_MONEY_ZONE) == 12, "PROTO_NC_HOLY_PROMISE_USE_MONEY_ZONE size drift");

struct PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_ACK) == 10, "PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_ACK size drift");

struct PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_REQ) == 1, "PROTO_NC_HOLY_PROMISE_WITHDRAW_CEN_REWARD_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_DELETE_DUNGEON_CMD { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_DELETE_DUNGEON_CMD) == 9, "PROTO_NC_INSTANCE_DUNGEON_DELETE_DUNGEON_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_EMPTY_DUNGEON_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_EMPTY_DUNGEON_CMD) == 5, "PROTO_NC_INSTANCE_DUNGEON_EMPTY_DUNGEON_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_FIND_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[7];
    INSTANCE_DUNGEON__CATEGORY InstanceCategory;
    uint8_t _pad_at_0007[8];
    ORToken Argument;
    uint8_t _pad_at_000f[20];
    wchar_t ServerMapName[33];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_FIND_RNG) == 70, "PROTO_NC_INSTANCE_DUNGEON_FIND_RNG size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ACK {
    uint8_t _pad_at_0000[8];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ACK) == 12, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_ACK {
    uint8_t _pad_at_0000[9];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_ACK) == 13, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_REQ {
    uint8_t _pad_at_0000[9];
    INSTANCE_DUNGEON__CATEGORY IndunCategory;
    uint8_t _pad_at_0009[6];
    ORToken Argument;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_REQ) == 35, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_ECHO_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_REQ {
    uint8_t _pad_at_0000[10];
    INSTANCE_DUNGEON__CATEGORY IndunCategory;
    uint8_t _pad_at_000a[6];
    ORToken Argument;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_REQ) == 36, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_CHECK_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ACK {
    uint8_t _pad_at_0000[8];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _pad_at_0008[4];
    Name3 sMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ACK) == 24, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_ACK {
    uint8_t _pad_at_0000[9];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _pad_at_0009[4];
    Name3 sMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_ACK) == 25, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_REQ {
    uint8_t _pad_at_0000[9];
    INSTANCE_DUNGEON__CATEGORY IndunCategory;
    uint8_t _pad_at_0009[6];
    ORToken Argument;
    uint8_t _pad_at_000f[20];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_REQ) == 39, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_ECHO_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_REQ {
    uint8_t _pad_at_0000[10];
    INSTANCE_DUNGEON__CATEGORY IndunCategory;
    uint8_t _pad_at_000a[6];
    ORToken Argument;
    uint8_t _pad_at_0010[20];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_REQ) == 40, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_JOIN_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_ACK {
    uint8_t _pad_at_0000[2];
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_ACK) == 6, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_REQ {
    uint8_t _pad_at_0000[2];
    Name3 sMapIndex;
    uint8_t _pad_at_0002[13];
    ID_LEVEL_TYPE eLevelType[0];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_REQ) == 15, "PROTO_NC_INSTANCE_DUNGEON_LEVEL_SELECT_MENU_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_MAP_REGIST_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_INDUN_INFO_______0_bytes___ sIndunInfos;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_MAP_REGIST_CMD) == 2, "PROTO_NC_INSTANCE_DUNGEON_MAP_REGIST_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_ACK {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _pad_at_0000[13];
    PROTO_AVATAR_SHAPE_INFO damageDealtShapeInfo1st;
    uint8_t _pad_at_000d[4];
    PROTO_AVATAR_SHAPE_INFO damageDealtShapeInfo2nd;
    uint8_t _pad_at_0011[4];
    PROTO_AVATAR_SHAPE_INFO damageDealtShapeInfo3th;
    uint8_t _pad_at_0015[4];
    PROTO_AVATAR_SHAPE_INFO damageTakenShapeInfo1st;
    uint8_t _pad_at_0019[4];
    PROTO_AVATAR_SHAPE_INFO damageTakenShapeInfo2nd;
    uint8_t _pad_at_001d[4];
    PROTO_AVATAR_SHAPE_INFO damageTakenShapeInfo3th;
    uint8_t _pad_at_0021[4];
    PROTO_AVATAR_SHAPE_INFO healingDoneShapeInfo1st;
    uint8_t _pad_at_0025[4];
    PROTO_AVATAR_SHAPE_INFO healingDoneShapeInfo2nd;
    uint8_t _pad_at_0029[4];
    PROTO_AVATAR_SHAPE_INFO healingDoneShapeInfo3th;
    uint8_t _pad_at_002d[7];
    SHINE_INDUN_RANK_CLIENT_VALUE_______640_bytes___ damageDealtRanks;
    uint8_t _pad_at_0034[640];
    SHINE_INDUN_RANK_CLIENT_VALUE_______640_bytes___ damageTakenRanks;
    uint8_t _pad_at_02b4[640];
    SHINE_INDUN_RANK_CLIENT_VALUE_______640_bytes___ healingDoneRanks;
    uint8_t _tail[640];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_ACK) == 1972, "PROTO_NC_INSTANCE_DUNGEON_RANK_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_ACK {
    uint8_t _pad_at_0000[2];
    SHINE_INDUN_INFO_______0_bytes___ sIndunInfos;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_ACK) == 2, "PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_REQ) == 1, "PROTO_NC_INSTANCE_DUNGEON_RANK_LIST_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_MYRANK_CMD {
    uint8_t _pad_at_0000[6];
    SHINE_INDUN_RANK_MYRANK_______0_bytes___ MyRank;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_MYRANK_CMD) == 6, "PROTO_NC_INSTANCE_DUNGEON_RANK_MYRANK_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_NEW_RANK_MSG {
    Name5 nCharID;
    uint8_t _pad_at_0000[20];
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_NEW_RANK_MSG) == 38, "PROTO_NC_INSTANCE_DUNGEON_RANK_NEW_RANK_MSG size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_INDUN_INFO_______0_bytes___ sIndunInfos;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_REQ) == 2, "PROTO_NC_INSTANCE_DUNGEON_RANK_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_RESULT_CMD {
    uint8_t _pad_at_0000[5];
    SHINE_INDUN_RANK_CLIENT_______0_bytes___ ranks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_RESULT_CMD) == 5, "PROTO_NC_INSTANCE_DUNGEON_RANK_RESULT_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_DB_CMD {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _pad_at_0000[19];
    SHINE_INDUN_RANK_______0_bytes___ ranks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_DB_CMD) == 19, "PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_DB_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_WORLD_CMD {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _pad_at_0000[19];
    SHINE_INDUN_RANK_______0_bytes___ ranks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_WORLD_CMD) == 19, "PROTO_NC_INSTANCE_DUNGEON_RANK_SAVE_WORLD_CMD size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_ACK {
    PROTO_AVATAR_SHAPE_INFO shapeInfo1st;
    uint8_t _pad_at_0000[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo2nd;
    uint8_t _pad_at_0004[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo3th;
    uint8_t _pad_at_0008[11];
    SHINE_INDUN_RANK_CLIENT_VALUE_______0_bytes___ damageDealtRanks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_ACK) == 19, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_REQ {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_REQ) == 13, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGEDEALT_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_ACK {
    PROTO_AVATAR_SHAPE_INFO shapeInfo1st;
    uint8_t _pad_at_0000[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo2nd;
    uint8_t _pad_at_0004[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo3th;
    uint8_t _pad_at_0008[11];
    SHINE_INDUN_RANK_CLIENT_VALUE_______0_bytes___ damageTakenRanks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_ACK) == 19, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_REQ {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_REQ) == 13, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_DAMAGETAKEN_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_ACK {
    PROTO_AVATAR_SHAPE_INFO shapeInfo1st;
    uint8_t _pad_at_0000[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo2nd;
    uint8_t _pad_at_0004[4];
    PROTO_AVATAR_SHAPE_INFO shapeInfo3th;
    uint8_t _pad_at_0008[11];
    SHINE_INDUN_RANK_CLIENT_VALUE_______0_bytes___ healingDoneRanks;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_ACK) == 19, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_REQ {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_REQ) == 13, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_HEALINGDONE_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_ACK { uint8_t data[62]; };
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_ACK) == 62, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_REQ {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_REQ) == 13, "PROTO_NC_INSTANCE_DUNGEON_RANK_TAB_MYRANK_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_ACK {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _pad_at_0000[15];
    SHINE_INDUN_RANK_RANKING_______0_bytes___ CharList;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_ACK) == 15, "PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_ACK size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_REQ {
    SHINE_INDUN_INFO sIndunInfo;
    uint8_t _pad_at_0000[15];
    NETPACKETZONEHEADER_______0_bytes___ CharList;
};
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_REQ) == 15, "PROTO_NC_INSTANCE_DUNGEON_RANK_UPDATE_DB_REQ size drift");

struct PROTO_NC_INSTANCE_DUNGEON_RESET_COUNTDOWN_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_INSTANCE_DUNGEON_RESET_COUNTDOWN_CMD) == 5, "PROTO_NC_INSTANCE_DUNGEON_RESET_COUNTDOWN_CMD size drift");

struct PROTO_NC_ITEMDB_ADMINCREATEFAIL_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ADMINCREATEFAIL_ACK) == 6, "PROTO_NC_ITEMDB_ADMINCREATEFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_ADMINCREATESUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ADMINCREATESUC_ACK) == 2, "PROTO_NC_ITEMDB_ADMINCREATESUC_ACK size drift");

struct PROTO_NC_ITEMDB_ADMINCREATE_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_CREATE create;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ADMINCREATE_REQ) == 40, "PROTO_NC_ITEMDB_ADMINCREATE_REQ size drift");

struct PROTO_NC_ITEMDB_BOOTHTRADE_ACK {
    NETPACKETZONEHEADER headerSeller;
    NETPACKETZONEHEADER headerBuyer;
    uint8_t _pad_at_000c[4];
    BOOTH_TRADE_TYPE nBoothTradeType;
    uint8_t _pad_at_0010[4];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BOOTHTRADE_ACK) == 32, "PROTO_NC_ITEMDB_BOOTHTRADE_ACK size drift");

struct PROTO_NC_ITEMDB_BOOTHTRADE_ALL_REQ {
    NETPACKETZONEHEADER headerSeller;
    NETPACKETZONEHEADER headerBuyer;
    uint8_t _pad_at_000c[13];
    SHINE_ITEM_REGISTNUMBER itemregnum;
    uint8_t _tail[43];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BOOTHTRADE_ALL_REQ) == 68, "PROTO_NC_ITEMDB_BOOTHTRADE_ALL_REQ size drift");

struct PROTO_NC_ITEMDB_BOOTHTRADE_LOT_REQ {
    NETPACKETZONEHEADER headerSeller;
    NETPACKETZONEHEADER headerBuyer;
    uint8_t _pad_at_000c[15];
    SHINE_ITEM_REGISTNUMBER lotseller;
    uint8_t _pad_at_001b[8];
    PROTO_ITEM_CREATE lotcreate;
    uint8_t _tail[68];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BOOTHTRADE_LOT_REQ) == 103, "PROTO_NC_ITEMDB_BOOTHTRADE_LOT_REQ size drift");

struct PROTO_NC_ITEMDB_BOOTHTRADE_MERGE_REQ {
    NETPACKETZONEHEADER headerSeller;
    NETPACKETZONEHEADER headerBuyer;
    uint8_t _pad_at_000c[15];
    SHINE_ITEM_REGISTNUMBER lotseller;
    uint8_t _pad_at_001b[8];
    SHINE_ITEM_REGISTNUMBER lotbuyer;
    uint8_t _tail[39];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BOOTHTRADE_MERGE_REQ) == 74, "PROTO_NC_ITEMDB_BOOTHTRADE_MERGE_REQ size drift");

struct PROTO_NC_ITEMDB_BRACELET_UPGRADE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BRACELET_UPGRADE_ACK) == 10, "PROTO_NC_ITEMDB_BRACELET_UPGRADE_ACK size drift");

struct PROTO_NC_ITEMDB_BRACELET_UPGRADE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER upgrade_item_regnum;
    uint8_t _pad_at_0008[14];
    SHINE_ITEM_REGISTNUMBER raw_regnum;
    uint8_t _tail[11];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BRACELET_UPGRADE_REQ) == 33, "PROTO_NC_ITEMDB_BRACELET_UPGRADE_REQ size drift");

struct PROTO_NC_ITEMDB_BUYALLFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_BUYALLFAIL_ACK) == 4, "PROTO_NC_ITEMDB_BUYALLFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_BUYALLSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_BUYALLSUC_ACK) == 2, "PROTO_NC_ITEMDB_BUYALLSUC_ACK size drift");

struct PROTO_NC_ITEMDB_BUYALL_REQ {
    PROTO_ITEM_CREATE itembuy;
    uint8_t _pad_at_0000[62];
    wchar_t CharID[30];
    SHINE_ITEM_ATTRIBUTE_______0_bytes___ attr;
};
static_assert(sizeof(PROTO_NC_ITEMDB_BUYALL_REQ) == 96, "PROTO_NC_ITEMDB_BUYALL_REQ size drift");

struct PROTO_NC_ITEMDB_BUYCAPSULE_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BUYCAPSULE_ACK) == 12, "PROTO_NC_ITEMDB_BUYCAPSULE_ACK size drift");

struct PROTO_NC_ITEMDB_BUYCAPSULE_REQ {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[22];
    ITEM_INVEN CapsuleLocation;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BUYCAPSULE_REQ) == 30, "PROTO_NC_ITEMDB_BUYCAPSULE_REQ size drift");

struct PROTO_NC_ITEMDB_BUYLOTFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_BUYLOTFAIL_ACK) == 4, "PROTO_NC_ITEMDB_BUYLOTFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_BUYLOTSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_BUYLOTSUC_ACK) == 2, "PROTO_NC_ITEMDB_BUYLOTSUC_ACK size drift");

struct PROTO_NC_ITEMDB_BUYLOT_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_ATTRCHANGE iteminfo;
    uint8_t _pad_at_0002[36];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_ITEMDB_BUYLOT_REQ) == 72, "PROTO_NC_ITEMDB_BUYLOT_REQ size drift");

struct PROTO_NC_ITEMDB_CAPSULEITEM_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER nItemReg;
    uint8_t _pad_at_0008[8];
    ITEM_INVEN nLocation;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CAPSULEITEM_ACK) == 18, "PROTO_NC_ITEMDB_CAPSULEITEM_ACK size drift");

struct PROTO_NC_ITEMDB_CAPSULEITEM_REQ {
    NETPACKETZONEHEADER Header;
    SHINE_ITEM_REGISTNUMBER nItemReg;
    uint8_t _pad_at_0006[8];
    ITEM_INVEN nLocation;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CAPSULEITEM_REQ) == 16, "PROTO_NC_ITEMDB_CAPSULEITEM_REQ size drift");

struct PROTO_NC_ITEMDB_CHARGED_LIST_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _pad_at_0006[5];
    PROTO_CHARGED_ITEM_INFO_______0_bytes___ ChargedItemInfoList;
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHARGED_LIST_ACK) == 11, "PROTO_NC_ITEMDB_CHARGED_LIST_ACK size drift");

struct PROTO_NC_ITEMDB_CHARGED_LIST_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHARGED_LIST_REQ) == 14, "PROTO_NC_ITEMDB_CHARGED_LIST_REQ size drift");

struct PROTO_NC_ITEMDB_CHARGED_WITHDRAW_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _pad_at_0006[2];
    PROTO_CHARGED_ITEM_INFO ChargedItemInfo;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHARGED_WITHDRAW_ACK) == 26, "PROTO_NC_ITEMDB_CHARGED_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEMDB_CHARGED_WITHDRAW_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _pad_at_0006[6];
    PROTO_CHARGED_ITEM_INFO ChargedItemInfo;
    uint8_t _pad_at_000c[17];
    PROTO_ITEM_CREATE_______0_bytes___ ItemCreate;
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHARGED_WITHDRAW_REQ) == 29, "PROTO_NC_ITEMDB_CHARGED_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_ACK {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0008[10];
    CAHT_CHAT_COLOR_ITEM_TYPE eChatColorItemType;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_ACK) == 23, "PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_ACK size drift");

struct PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_REQ {
    uint8_t _pad_at_0000[6];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0006[14];
    CAHT_CHAT_COLOR_ITEM_TYPE eChatColorItemType;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_REQ) == 25, "PROTO_NC_ITEMDB_CHAT_COLOR_CHANGE_REQ size drift");

struct PROTO_NC_ITEMDB_CHESTITEM_ACK {
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER chest;
    uint8_t _pad_at_0006[8];
    ITEM_INVEN location;
    uint8_t _pad_at_000e[4];
    PROTO_ITEM_CMD item;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHESTITEM_ACK) == 19, "PROTO_NC_ITEMDB_CHESTITEM_ACK size drift");

struct PROTO_NC_ITEMDB_CHESTITEM_REQ {
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER chest;
    uint8_t _pad_at_0006[8];
    ITEM_INVEN location;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CHESTITEM_REQ) == 16, "PROTO_NC_ITEMDB_CHESTITEM_REQ size drift");

struct PROTO_NC_ITEMDB_CLASS_CHANGE_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_CLASS_CHANGE_ACK) == 10, "PROTO_NC_ITEMDB_CLASS_CHANGE_ACK size drift");

struct PROTO_NC_ITEMDB_CLASS_CHANGE_REQ {
    uint8_t _pad_at_0000[6];
    SHINE_ITEM_REGISTNUMBER nStuffKey;
    uint8_t _pad_at_0006[15];
    uint8_t nFreeStat[5];
    uint8_t _pad_at_001a[15];
    uint16_t nDeleteSkillID[0];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CLASS_CHANGE_REQ) == 41, "PROTO_NC_ITEMDB_CLASS_CHANGE_REQ size drift");

struct PROTO_NC_ITEMDB_CLOSE_GUILD_STORAGE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_CLOSE_GUILD_STORAGE_CMD) == 1, "PROTO_NC_ITEMDB_CLOSE_GUILD_STORAGE_CMD size drift");

struct PROTO_NC_ITEMDB_CREATEITEMLISTFAIL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CREATEITEMLISTFAIL_ACK) == 10, "PROTO_NC_ITEMDB_CREATEITEMLISTFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_CREATEITEMLISTSUC_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CREATEITEMLISTSUC_ACK) == 8, "PROTO_NC_ITEMDB_CREATEITEMLISTSUC_ACK size drift");

struct PROTO_NC_ITEMDB_CREATEITEMLIST_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[16];
    PROTO_ITEM_CMD itemlist;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_ITEMDB_CREATEITEMLIST_REQ) == 23, "PROTO_NC_ITEMDB_CREATEITEMLIST_REQ size drift");

struct PROTO_NC_ITEMDB_DEPOSIT_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DEPOSIT_ACK) == 18, "PROTO_NC_ITEMDB_DEPOSIT_ACK size drift");

struct PROTO_NC_ITEMDB_DEPOSIT_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DEPOSIT_REQ) == 20, "PROTO_NC_ITEMDB_DEPOSIT_REQ size drift");

struct PROTO_NC_ITEMDB_DESTROY_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DESTROY_ACK) == 10, "PROTO_NC_ITEMDB_DESTROY_ACK size drift");

struct PROTO_NC_ITEMDB_DISMANTLE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DISMANTLE_ACK) == 10, "PROTO_NC_ITEMDB_DISMANTLE_ACK size drift");

struct PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_raw_ {
    uint8_t _pad_at_0000[5];
    PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_raw____unnamed_type_iteminfo_ iteminfo;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_raw_) == 17, "PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_raw_ size drift");

struct PROTO_NC_ITEMDB_DISMANTLE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_raw_ raw;
    uint8_t _pad_at_0019[1];
    PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_produce________0_bytes___ produce;
};
static_assert(sizeof(PROTO_NC_ITEMDB_DISMANTLE_REQ) == 26, "PROTO_NC_ITEMDB_DISMANTLE_REQ size drift");

struct PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_produce_ {
    uint8_t _pad_at_0000[5];
    PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_produce____unnamed_type_iteminfo_ iteminfo;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_produce_) == 43, "PROTO_NC_ITEMDB_DISMANTLE_REQ___unnamed_type_produce_ size drift");

struct PROTO_NC_ITEMDB_DROPALLSUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_DROPALLSUC_ACK) == 1, "PROTO_NC_ITEMDB_DROPALLSUC_ACK size drift");

struct PROTO_NC_ITEMDB_DROPALL_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_VANISH dropitem;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DROPALL_REQ) == 12, "PROTO_NC_ITEMDB_DROPALL_REQ size drift");

struct PROTO_NC_ITEMDB_DROPLOT_REQ___unnamed_type_dropitem_ {
    SHINE_ITEM_REGISTNUMBER itemregnum;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DROPLOT_REQ___unnamed_type_dropitem_) == 12, "PROTO_NC_ITEMDB_DROPLOT_REQ___unnamed_type_dropitem_ size drift");

struct PROTO_NC_ITEMDB_DROPLOT_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_ATTRCHANGE iteminfo;
    uint8_t _pad_at_0002[12];
    PROTO_NC_ITEMDB_DROPLOT_REQ___unnamed_type_dropitem_ dropitem;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_DROPLOT_REQ) == 30, "PROTO_NC_ITEMDB_DROPLOT_REQ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_equipment_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_equipment_) == 6, "PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_gem_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_gem_) == 2, "PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_gem_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK___unnamed_type_gem_ gem;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK) == 18, "PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_ACK size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ___unnamed_type_equipment_ {
    SHINE_ITEM_REGISTNUMBER itemRegistNumber;
    uint8_t _pad_at_0000[10];
    ITEM_INVEN itemSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ___unnamed_type_equipment_) == 12, "PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ___unnamed_type_equipment_ gem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ) == 34, "PROTO_NC_ITEMDB_ENCHANT_ADD_GEM_REQ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_equipment_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_equipment_) == 3, "PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_drill_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_drill_) == 10, "PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_drill_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK___unnamed_type_drill_ drill;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK) == 23, "PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_ACK size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ___unnamed_type_equipment_ {
    SHINE_ITEM_REGISTNUMBER itemRegistNumber;
    uint8_t _pad_at_0000[10];
    ITEM_INVEN itemSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ___unnamed_type_equipment_) == 12, "PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ___unnamed_type_equipment_ drill;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ) == 32, "PROTO_NC_ITEMDB_ENCHANT_ADD_NEW_SOCKET_REQ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_equipment_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_equipment_) == 2, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_remover_ {
    ITEM_INVEN itemSlot;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_remover_) == 10, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_remover_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_remover_ remover;
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_gemSlot________3_bytes___ gemSlot;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK) == 26, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_gemSlot_ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_gemSlot_) == 1, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_ACK___unnamed_type_gemSlot_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_equipment_ {
    SHINE_ITEM_REGISTNUMBER itemRegistNumber;
    uint8_t _pad_at_0000[10];
    ITEM_INVEN itemSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_equipment_) == 12, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_equipment_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_equipment_ equipment;
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_equipment_ remover;
    PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_gemSlot________3_bytes___ gemSlot;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ) == 36, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_gemSlot_ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_gemSlot_) == 1, "PROTO_NC_ITEMDB_ENCHANT_REMOVE_GEM_REQ___unnamed_type_gemSlot_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER itemRegistNumber;
    uint8_t _pad_at_0008[9];
    PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK___unnamed_type_sockets________9_bytes___ sockets;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK) == 26, "PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK size drift");

struct PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK___unnamed_type_sockets_ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK___unnamed_type_sockets_) == 3, "PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_ACK___unnamed_type_sockets_ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ {
    NETPACKETZONEHEADER header;
    SHINE_ITEM_REGISTNUMBER itemRegistNumber;
    uint8_t _pad_at_0006[9];
    PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ___unnamed_type_sockets________9_bytes___ sockets;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ) == 24, "PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ size drift");

struct PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ___unnamed_type_sockets_ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ___unnamed_type_sockets_) == 3, "PROTO_NC_ITEMDB_ENCHANT_SET_GEM_LOT_REQ___unnamed_type_sockets_ size drift");

struct PROTO_NC_ITEMDB_EQUIPFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIPFAIL_ACK) == 4, "PROTO_NC_ITEMDB_EQUIPFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_EQUIPSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIPSUC_ACK) == 2, "PROTO_NC_ITEMDB_EQUIPSUC_ACK size drift");

struct PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[4];
    SHINE_ITEM_REGISTNUMBER nBelongedItem_ItemKey;
    uint8_t _pad_at_000a[10];
    SHINE_ITEM_REGISTNUMBER nCancelItem_ItemKey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_ACK) == 34, "PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_ACK size drift");

struct PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_REQ {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER nBelongedItem_ItemKey;
    uint8_t _pad_at_0008[10];
    SHINE_ITEM_REGISTNUMBER nCancelItem_ItemKey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_REQ) == 32, "PROTO_NC_ITEMDB_EQUIP_BELONGED_CANCEL_USE_REQ size drift");

struct PROTO_NC_ITEMDB_EQUIP_REQ {
    PROTO_ITEM_RELOC relocA;
    uint8_t _pad_at_0000[12];
    SHINE_PUT_ON_BELONGED_ITEM relocA_IsPutOnBelonged;
    uint8_t _pad_at_000c[4];
    PROTO_ITEM_RELOC relocB;
    uint8_t _pad_at_0010[13];
    PROTO_NC_ITEMDB_EQUIP_REQ___unnamed_type_unequip________0_bytes___ unequip;
};
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIP_REQ) == 29, "PROTO_NC_ITEMDB_EQUIP_REQ size drift");

struct PROTO_NC_ITEMDB_EQUIP_REQ___unnamed_type_unequip_ {
    SHINE_ITEM_REGISTNUMBER item;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_ITEMDB_EQUIP_REQ___unnamed_type_unequip_) == 9, "PROTO_NC_ITEMDB_EQUIP_REQ___unnamed_type_unequip_ size drift");

struct PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK) == 4, "PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_EXCHANGEFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_EXCHANGESUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_EXCHANGESUC_ACK) == 2, "PROTO_NC_ITEMDB_EXCHANGESUC_ACK size drift");

struct PROTO_NC_ITEMDB_EXCHANGESUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_EXCHANGESUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_EXCHANGESUC_ACK_SEND) == 11, "PROTO_NC_ITEMDB_EXCHANGESUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_EXCHANGE_REQ {
    uint8_t _pad_at_0000[10];
    PROTO_ITEM_RELOC relocA;
    uint8_t _pad_at_000a[10];
    PROTO_ITEM_RELOC relocB;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_ITEMDB_EXCHANGE_REQ) == 42, "PROTO_NC_ITEMDB_EXCHANGE_REQ size drift");

struct PROTO_NC_ITEMDB_FAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_FAIL_ACK) == 2, "PROTO_NC_ITEMDB_FAIL_ACK size drift");

struct PROTO_NC_ITEMDB_FURNITURE_ENDURE_ACK {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0004[8];
    ShineDateTime dNewEndureTime;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_FURNITURE_ENDURE_ACK) == 18, "PROTO_NC_ITEMDB_FURNITURE_ENDURE_ACK size drift");

struct PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ {
    NETPACKETZONEHEADER header;
    PROTO_CHANGEATTR attr;
    uint8_t _tail[15];
};
static_assert(sizeof(PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ) == 21, "PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ size drift");

struct PROTO_NC_ITEMDB_FURNITURE_ENDURE_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0002[8];
    ShineDateTime dNewEndureTime;
    uint8_t _pad_at_000a[4];
    PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ CommonData;
};
static_assert(sizeof(PROTO_NC_ITEMDB_FURNITURE_ENDURE_REQ) == 35, "PROTO_NC_ITEMDB_FURNITURE_ENDURE_REQ size drift");

struct PROTO_NC_ITEMDB_GETFROMCAPSULE_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GETFROMCAPSULE_ACK) == 10, "PROTO_NC_ITEMDB_GETFROMCAPSULE_ACK size drift");

struct PROTO_NC_ITEMDB_GETFROMCAPSULE_REQ {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[6];
    SHINE_ITEM_REGISTNUMBER nItmeReg;
    uint8_t _pad_at_000c[8];
    PROTO_ITEM_RELOC nReloc;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GETFROMCAPSULE_REQ) == 30, "PROTO_NC_ITEMDB_GETFROMCAPSULE_REQ size drift");

struct PROTO_NC_ITEMDB_GETFROMCHESTFAIL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GETFROMCHESTFAIL_ACK) == 10, "PROTO_NC_ITEMDB_GETFROMCHESTFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_GETFROMCHESTSUC_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GETFROMCHESTSUC_ACK) == 8, "PROTO_NC_ITEMDB_GETFROMCHESTSUC_ACK size drift");

struct PROTO_NC_ITEMDB_GETFROMCHEST_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[6];
    SHINE_ITEM_REGISTNUMBER chest;
    uint8_t _pad_at_000c[9];
    PROTO_ITEM_RELOC_______0_bytes___ reloc;
};
static_assert(sizeof(PROTO_NC_ITEMDB_GETFROMCHEST_REQ) == 21, "PROTO_NC_ITEMDB_GETFROMCHEST_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK) == 18, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ) == 20, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK {
    NETPACKETZONEHEADER zonepackheader;
    uint8_t _pad_at_0006[10];
    PROTO_ITEMPACKET_TOTAL_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK) == 16, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK) == 8, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK_SEND) == 11, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_FAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ) == 11, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_OPEN_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK) == 18, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ) == 20, "PROTO_NC_ITEMDB_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_ACK) == 18, "PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_REQ) == 20, "PROTO_NC_ITEMDB_GUILD_STORAGE_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLFAIL_ACK) == 4, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLSUC_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLSUC_ACK) == 10, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALLSUC_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALL_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_ITEM_CREATE ItemBuy;
    uint8_t _pad_at_0004[44];
    SHINE_ITEM_ATTRIBUTE_______0_bytes___ Attr;
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALL_REQ) == 48, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYALL_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTFAIL_ACK) == 4, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTSUC_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTSUC_ACK) == 10, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOTSUC_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOT_REQ {
    uint8_t _pad_at_0000[6];
    PROTO_ITEM_ATTRCHANGE ItemInfo;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOT_REQ) == 24, "PROTO_NC_ITEMDB_GUILD_TOKEN_BUYLOT_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_ACK { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_ACK) == 16, "PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_ACK size drift");

struct PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ {
    uint8_t _pad_at_0000[8];
    PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ__ITEM_OPT_______0_bytes___ ItemOpt;
};
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ) == 8, "PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ size drift");

struct PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ__ITEM_OPT { uint8_t data[33]; };
static_assert(sizeof(PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ__ITEM_OPT) == 33, "PROTO_NC_ITEMDB_GUILD_TOURNAMENT_REWARD_CREATE_REQ__ITEM_OPT size drift");

struct PROTO_NC_ITEMDB_INC_DEC_MONEYSUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_INC_DEC_MONEYSUC_ACK) == 1, "PROTO_NC_ITEMDB_INC_DEC_MONEYSUC_ACK size drift");

struct PROTO_NC_ITEMDB_INC_DEC_MONEY_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_ITEMDB_INC_DEC_MONEY_REQ) == 9, "PROTO_NC_ITEMDB_INC_DEC_MONEY_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMBREAKFAIL_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMBREAKFAIL_ACK) == 10, "PROTO_NC_ITEMDB_ITEMBREAKFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMBREAKSUC_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMBREAKSUC_ACK) == 8, "PROTO_NC_ITEMDB_ITEMBREAKSUC_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMBREAK_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER BreakItemKey;
    uint8_t _pad_at_0008[17];
    PROTO_ITEM_VANISH_______0_bytes___ DeleteChestItem;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMBREAK_REQ) == 25, "PROTO_NC_ITEMDB_ITEMBREAK_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMCHANGE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMCHANGE_ACK) == 10, "PROTO_NC_ITEMDB_ITEMCHANGE_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMCHANGE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_ITEMCHANGE_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMCHANGE_ACK_SEND) == 13, "PROTO_NC_ITEMDB_ITEMCHANGE_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_ITEMCHANGE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0008[8];
    SHINE_ITEM_STRUCT_______0_bytes___ itemstruct;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMCHANGE_REQ) == 16, "PROTO_NC_ITEMDB_ITEMCHANGE_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMINFO_ACK {
    uint8_t _pad_at_0000[8];
    NETCOMMAND FailProtocol;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMINFO_ACK) == 10, "PROTO_NC_ITEMDB_ITEMINFO_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMINFO_REQ {
    uint8_t _pad_at_0000[6];
    SHINE_ITEM_REGISTNUMBER ItemKey;
    uint8_t _pad_at_0006[8];
    NETCOMMAND FailProtocol;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMINFO_REQ) == 16, "PROTO_NC_ITEMDB_ITEMINFO_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ {
    uint8_t _pad_at_0000[14];
    SHINE_ITEM_REGISTNUMBER nItemMoneyKey;
    uint8_t _pad_at_000e[20];
    PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ__CREATE_ITEM_INFO_______0_bytes___ CreateItem;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ) == 34, "PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ__CREATE_ITEM_INFO {
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0000[10];
    ITEM_INVEN ItemLoc;
    uint8_t _pad_at_000a[2];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ__CREATE_ITEM_INFO) == 113, "PROTO_NC_ITEMDB_ITEMMONEY_BUYALL_REQ__CREATE_ITEM_INFO size drift");

struct PROTO_NC_ITEMDB_ITEMMONEY_BUYLOT_REQ {
    uint8_t _pad_at_0000[14];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_000e[18];
    SHINE_ITEM_REGISTNUMBER nItemMoneyKey;
    uint8_t _pad_at_0020[15];
    DELETE_ITEM_INFO_______0_bytes___ nDeleteItemInfo;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMMONEY_BUYLOT_REQ) == 47, "PROTO_NC_ITEMDB_ITEMMONEY_BUYLOT_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMMONEY_BUY_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMMONEY_BUY_ACK) == 10, "PROTO_NC_ITEMDB_ITEMMONEY_BUY_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMREBUILD_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMREBUILD_ACK) == 10, "PROTO_NC_ITEMDB_ITEMREBUILD_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMREBUILD_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER EraseItemKey;
    uint8_t _pad_at_0008[8];
    PROTO_ITEM_CREATE CreateItem;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMREBUILD_REQ) == 54, "PROTO_NC_ITEMDB_ITEMREBUILD_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMTOTALINFORM_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTOTALINFORM_ACK) == 1, "PROTO_NC_ITEMDB_ITEMTOTALINFORM_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMTOTALINFORM_REQ {
    SHINE_ITEM_REGISTNUMBER itemregnum;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTOTALINFORM_REQ) == 8, "PROTO_NC_ITEMDB_ITEMTOTALINFORM_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK) == 10, "PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_ITEMTRADEFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_ITEMTRADESUC_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADESUC_ACK) == 8, "PROTO_NC_ITEMDB_ITEMTRADESUC_ACK size drift");

struct PROTO_NC_ITEMDB_ITEMTRADESUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_ITEMTRADESUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADESUC_ACK_SEND) == 11, "PROTO_NC_ITEMDB_ITEMTRADESUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_cen_ { uint8_t data[25]; };
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_cen_) == 25, "PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_cen_ size drift");

struct PROTO_NC_ITEMDB_ITEMTRADE_REQ {
    NETPACKETZONEHEADER A;
    NETPACKETZONEHEADER B;
    uint8_t _pad_at_000c[4];
    PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_cen_ cen;
    uint8_t _pad_at_0029[1];
    PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem________0_bytes___ tradeitem;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADE_REQ) == 42, "PROTO_NC_ITEMDB_ITEMTRADE_REQ size drift");

struct PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem____unnamed_type_invenA_ {
    SHINE_ITEM_REGISTNUMBER regnum;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem____unnamed_type_invenA_) == 13, "PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem____unnamed_type_invenA_ size drift");

struct PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem_ {
    PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem____unnamed_type_invenA_ invenA;
    PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem____unnamed_type_invenA_ invenB;
};
static_assert(sizeof(PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem_) == 26, "PROTO_NC_ITEMDB_ITEMTRADE_REQ___unnamed_type_tradeitem_ size drift");

struct PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_ACK {
    uint8_t _pad_at_0000[8];
    Name3 mapName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_ACK) == 28, "PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_ACK size drift");

struct PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_REQ {
    uint8_t _pad_at_0000[8];
    Name3 mapName;
    uint8_t _pad_at_0008[18];
    SHINE_ITEM_REGISTNUMBER nStuffKey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_REQ) == 40, "PROTO_NC_ITEMDB_MAPLINK_ITEM_CONSUME_REQ size drift");

struct PROTO_NC_ITEMDB_MERGEFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_MERGEFAIL_ACK) == 4, "PROTO_NC_ITEMDB_MERGEFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_MERGEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_MERGEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_MERGEFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_MERGEFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_MERGESUC_ACK {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_REGISTNUMBER lotmain;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MERGESUC_ACK) == 12, "PROTO_NC_ITEMDB_MERGESUC_ACK size drift");

struct PROTO_NC_ITEMDB_MERGESUC_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_MERGESUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_MERGESUC_SEND) == 21, "PROTO_NC_ITEMDB_MERGESUC_SEND size drift");

struct PROTO_NC_ITEMDB_MERGE_REQ {
    uint8_t _pad_at_0000[12];
    SHINE_ITEM_REGISTNUMBER lotmain;
    uint8_t _pad_at_000c[8];
    SHINE_ITEM_REGISTNUMBER lotsub;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MERGE_REQ) == 36, "PROTO_NC_ITEMDB_MERGE_REQ size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK) == 12, "PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK_SEND) == 15, "PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER nDemandItemRegNum;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_REQ) == 32, "PROTO_NC_ITEMDB_MINIHOUSE_EFFECT_DEMANDGOOD_REQ size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK) == 12, "PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK_SEND) == 15, "PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER nDemandItemRegNum;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_REQ) == 32, "PROTO_NC_ITEMDB_MINIHOUSE_PORTAL_EFFECT_DEMANDGOOD_REQ size drift");

struct PROTO_NC_ITEMDB_MIX_ITEM_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_MIX_ITEM_ACK) == 10, "PROTO_NC_ITEMDB_MIX_ITEM_ACK size drift");

struct PROTO_NC_ITEMDB_MIX_ITEM_REQ {
    uint8_t _pad_at_0000[6];
    SHINE_ITEM_REGISTNUMBER nSubRawKey;
    uint8_t _pad_at_0006[14];
    SHINE_ITEM_REGISTNUMBER nRawLeftKey;
    uint8_t _pad_at_0014[14];
    SHINE_ITEM_REGISTNUMBER nRawRightKey;
    uint8_t _pad_at_0022[14];
    ITEM_INVEN MixItemRoc;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MIX_ITEM_REQ) == 52, "PROTO_NC_ITEMDB_MIX_ITEM_REQ size drift");

struct PROTO_NC_ITEMDB_MOB_DROP_CMD {
    uint8_t _pad_at_0000[4];
    MAPPOS map;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER createitemregnum;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MOB_DROP_CMD) == 38, "PROTO_NC_ITEMDB_MOB_DROP_CMD size drift");

struct PROTO_NC_ITEMDB_MOVER_RAREMOVER_ACK {
    uint8_t _pad_at_0000[13];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MOVER_RAREMOVER_ACK) == 114, "PROTO_NC_ITEMDB_MOVER_RAREMOVER_ACK size drift");

struct PROTO_NC_ITEMDB_MOVER_RAREMOVER_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nMainMoverItemKey;
    uint8_t _pad_at_0008[10];
    SHINE_ITEM_ATTRIBUTE MainMoverAttr;
    uint8_t _pad_at_0012[101];
    SHINE_ITEM_REGISTNUMBER nSubMoverItemKey;
    uint8_t _pad_at_0077[10];
    SHINE_ITEM_ATTRIBUTE SubMoverAttr;
    uint8_t _pad_at_0081[101];
    SHINE_ITEM_REGISTNUMBER nRareMoverItemKey;
    uint8_t _pad_at_00e6[10];
    ITEM_INVEN RareMoverLoc;
    uint8_t _pad_at_00f0[2];
    SHINE_ITEM_ATTRIBUTE RareMoverAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MOVER_RAREMOVER_REQ) == 343, "PROTO_NC_ITEMDB_MOVER_RAREMOVER_REQ size drift");

struct PROTO_NC_ITEMDB_MOVER_UPGRADE_ACK {
    uint8_t _pad_at_0000[14];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MOVER_UPGRADE_ACK) == 115, "PROTO_NC_ITEMDB_MOVER_UPGRADE_ACK size drift");

struct PROTO_NC_ITEMDB_MOVER_UPGRADE_REQ {
    uint8_t _pad_at_0000[9];
    SHINE_ITEM_REGISTNUMBER nSubMoverItemKey;
    uint8_t _pad_at_0009[10];
    SHINE_ITEM_ATTRIBUTE SubMoverAttr;
    uint8_t _pad_at_0013[101];
    SHINE_ITEM_REGISTNUMBER nMainMoverItemKey;
    uint8_t _pad_at_0078[10];
    ITEM_INVEN MainMoverLoc;
    uint8_t _pad_at_0082[2];
    SHINE_ITEM_ATTRIBUTE MainMoverAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MOVER_UPGRADE_REQ) == 233, "PROTO_NC_ITEMDB_MOVER_UPGRADE_REQ size drift");

struct PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_ACK) == 10, "PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_ACK size drift");

struct PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER UseVaultItemRegNo;
    uint8_t _pad_at_0008[11];
    PROTO_ITEM_CREATE_______0_bytes___ itemcreate;
};
static_assert(sizeof(PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_REQ) == 19, "PROTO_NC_ITEMDB_MYSTERY_VAULT_MAKEITEM_REQ size drift");

struct PROTO_NC_ITEMDB_NEW_UPGRADE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_NEW_UPGRADE_ACK) == 10, "PROTO_NC_ITEMDB_NEW_UPGRADE_ACK size drift");

struct PROTO_NC_ITEMDB_NEW_UPGRADE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_NEW_UPGRADE_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_NEW_UPGRADE_ACK_SEND) == 13, "PROTO_NC_ITEMDB_NEW_UPGRADE_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_NEW_UPGRADE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER upgrade_item_regnum;
    uint8_t _pad_at_0008[13];
    SHINE_ITEM_REGISTNUMBER raw_regnum;
    uint8_t _pad_at_0015[12];
    ItemOptionStorage__Element upgrade_randomoption;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEMDB_NEW_UPGRADE_REQ) == 36, "PROTO_NC_ITEMDB_NEW_UPGRADE_REQ size drift");

struct PROTO_NC_ITEMDB_OPENSTORAGE_ACK {
    NETPACKETZONEHEADER zonepackheader;
    uint8_t _pad_at_0006[11];
    PROTO_ITEMPACKET_TOTAL_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPENSTORAGE_ACK) == 17, "PROTO_NC_ITEMDB_OPENSTORAGE_ACK size drift");

struct PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK) == 8, "PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK size drift");

struct PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK_SEND) == 11, "PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_OPENSTORAGE_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPENSTORAGE_REQ) == 12, "PROTO_NC_ITEMDB_OPENSTORAGE_REQ size drift");

struct PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_ACK {
    NETPACKETZONEHEADER zonepackheader;
    uint8_t _pad_at_0006[17];
    PROTO_ITEMPACKET_TOTAL_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_ACK) == 23, "PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_ACK size drift");

struct PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK) == 8, "PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK size drift");

struct PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_OPENSTORAGE_FAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK_SEND) == 11, "PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_FAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_REQ) == 10, "PROTO_NC_ITEMDB_OPEN_GUILD_STORAGE_REQ size drift");

struct PROTO_NC_ITEMDB_PICKMERGE_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_REGISTNUMBER itemonfield;
    uint8_t _pad_at_0002[12];
    PROTO_ITEM_ATTRCHANGE itempick;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_PICKMERGE_REQ) == 26, "PROTO_NC_ITEMDB_PICKMERGE_REQ size drift");

struct PROTO_NC_ITEMDB_PICKMONEYSUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEMDB_PICKMONEYSUC_ACK) == 1, "PROTO_NC_ITEMDB_PICKMONEYSUC_ACK size drift");

struct PROTO_NC_ITEMDB_PICKMONEY_REQ {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemonfield;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_PICKMONEY_REQ) == 16, "PROTO_NC_ITEMDB_PICKMONEY_REQ size drift");

struct PROTO_NC_ITEMDB_PRODUCE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_PRODUCE_ACK) == 12, "PROTO_NC_ITEMDB_PRODUCE_ACK size drift");

struct PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_produce_ {
    uint8_t _pad_at_0000[1];
    PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_produce____unnamed_type_iteminfo_ iteminfo;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_produce_) == 39, "PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_produce_ size drift");

struct PROTO_NC_ITEMDB_PRODUCE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_produce_ produce;
    uint8_t _pad_at_0031[1];
    PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_raw________0_bytes___ raw;
};
static_assert(sizeof(PROTO_NC_ITEMDB_PRODUCE_REQ) == 50, "PROTO_NC_ITEMDB_PRODUCE_REQ size drift");

struct PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_raw_ {
    uint8_t _pad_at_0000[5];
    PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_raw____unnamed_type_iteminfo_ iteminfo;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_raw_) == 17, "PROTO_NC_ITEMDB_PRODUCE_REQ___unnamed_type_raw_ size drift");

struct PROTO_NC_ITEMDB_QUESTALL_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_CREATE questitem;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_ITEMDB_QUESTALL_REQ) == 40, "PROTO_NC_ITEMDB_QUESTALL_REQ size drift");

struct PROTO_NC_ITEMDB_QUESTITEMGET_REQ {
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_ADMINCREATE_REQ create;
};
static_assert(sizeof(PROTO_NC_ITEMDB_QUESTITEMGET_REQ) == 46, "PROTO_NC_ITEMDB_QUESTITEMGET_REQ size drift");

struct PROTO_NC_ITEMDB_QUESTLOT_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_ITEM_ATTRCHANGE lot;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_QUESTLOT_REQ) == 16, "PROTO_NC_ITEMDB_QUESTLOT_REQ size drift");

struct PROTO_NC_ITEMDB_QUESTREWARD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_QUESTREWARD_ACK) == 12, "PROTO_NC_ITEMDB_QUESTREWARD_ACK size drift");

struct PROTO_NC_ITEMDB_QUESTREWARD_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[39];
    PROTO_ITEM_CREATE_______0_bytes___ itemcreate;
};
static_assert(sizeof(PROTO_NC_ITEMDB_QUESTREWARD_REQ) == 45, "PROTO_NC_ITEMDB_QUESTREWARD_REQ size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_ACK) == 10, "PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_ACK size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_ACK {
    uint8_t _pad_at_0000[10];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _tail[11];
};
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_ACK) == 21, "PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_ACK size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nStuffKey;
    uint8_t _pad_at_0008[14];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0016[10];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _tail[102];
};
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_REQ) == 134, "PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_CONSUME_AND_COUNTING_REQ size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0008[10];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _pad_at_0012[101];
    SHINE_ITEM_ATTRIBUTE ItemAttrOrig;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_REQ) == 220, "PROTO_NC_ITEMDB_RANDOMOPTION_CHANGE_REQ size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK) == 10, "PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK size drift");

struct PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0008[10];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _pad_at_0012[115];
    SHINE_ITEM_REGISTNUMBER nStuffKey;
    uint8_t _pad_at_0085[13];
    SHINE_ITEM_REGISTNUMBER_______0_bytes___ nDeleteStuffKey;
};
static_assert(sizeof(PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ) == 146, "PROTO_NC_ITEMDB_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ size drift");

struct PROTO_NC_ITEMDB_REINFORCEUNEQUIPFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_REINFORCEUNEQUIPFAIL_ACK) == 4, "PROTO_NC_ITEMDB_REINFORCEUNEQUIPFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_REINFORCEUNEQUIPSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_REINFORCEUNEQUIPSUC_ACK) == 2, "PROTO_NC_ITEMDB_REINFORCEUNEQUIPSUC_ACK size drift");

struct PROTO_NC_ITEMDB_REINFORCEUNEQUIP_REQ {
    PROTO_ITEM_RELOC unequ;
    uint8_t _pad_at_0000[12];
    ITEM_INVEN equloc;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_REINFORCEUNEQUIP_REQ) == 14, "PROTO_NC_ITEMDB_REINFORCEUNEQUIP_REQ size drift");

struct PROTO_NC_ITEMDB_RELOCFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_RELOCFAIL_ACK) == 4, "PROTO_NC_ITEMDB_RELOCFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_RELOCFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_RELOCFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_RELOCFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_RELOCFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_RELOCSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_RELOCSUC_ACK) == 2, "PROTO_NC_ITEMDB_RELOCSUC_ACK size drift");

struct PROTO_NC_ITEMDB_RELOCSUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_RELOCSUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_RELOCSUC_ACK_SEND) == 11, "PROTO_NC_ITEMDB_RELOCSUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_RELOC_REQ {
    uint8_t _pad_at_0000[10];
    PROTO_ITEM_RELOC item;
    uint8_t _pad_at_000a[10];
    ITEM_INVEN inven_from;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_RELOC_REQ) == 28, "PROTO_NC_ITEMDB_RELOC_REQ size drift");

struct PROTO_NC_ITEMDB_REPURCHASE_ALL_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_REPURCHASE_ALL_ACK) == 10, "PROTO_NC_ITEMDB_REPURCHASE_ALL_ACK size drift");

struct PROTO_NC_ITEMDB_REPURCHASE_ALL_REQ {
    uint8_t _pad_at_0000[8];
    PROTO_ITEM_CREATE RepurchaseItem;
    uint8_t _pad_at_0008[42];
    SHINE_ITEM_REGISTNUMBER nSellItemKey;
    uint8_t _pad_at_0032[14];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_ITEMDB_REPURCHASE_ALL_REQ) == 98, "PROTO_NC_ITEMDB_REPURCHASE_ALL_REQ size drift");

struct PROTO_NC_ITEMDB_REPURCHASE_LOT_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_REPURCHASE_LOT_ACK) == 10, "PROTO_NC_ITEMDB_REPURCHASE_LOT_ACK size drift");

struct PROTO_NC_ITEMDB_REPURCHASE_LOT_REQ {
    uint8_t _pad_at_0000[10];
    PROTO_ITEM_ATTRCHANGE nItemAttrChange;
    uint8_t _pad_at_000a[16];
    SHINE_ITEM_REGISTNUMBER nSellItemKey;
    uint8_t _pad_at_001a[14];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_ITEMDB_REPURCHASE_LOT_REQ) == 74, "PROTO_NC_ITEMDB_REPURCHASE_LOT_REQ size drift");

struct PROTO_NC_ITEMDB_RESET_SCROLL_LINK_MAP_INFO_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_RESET_SCROLL_LINK_MAP_INFO_CMD) == 4, "PROTO_NC_ITEMDB_RESET_SCROLL_LINK_MAP_INFO_CMD size drift");

struct PROTO_NC_ITEMDB_SELLALLFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SELLALLFAIL_ACK) == 4, "PROTO_NC_ITEMDB_SELLALLFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_SELLALLSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SELLALLSUC_ACK) == 2, "PROTO_NC_ITEMDB_SELLALLSUC_ACK size drift");

struct PROTO_NC_ITEMDB_SELLALL_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_VANISH itemsell;
    uint8_t _pad_at_0002[30];
    wchar_t CharID[30];
    PROTO_ITEM_VANISH_______0_bytes___ DeleteChestItem;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SELLALL_REQ) == 67, "PROTO_NC_ITEMDB_SELLALL_REQ size drift");

struct PROTO_NC_ITEMDB_SELLLOTFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SELLLOTFAIL_ACK) == 4, "PROTO_NC_ITEMDB_SELLLOTFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_SELLLOTSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SELLLOTSUC_ACK) == 2, "PROTO_NC_ITEMDB_SELLLOTSUC_ACK size drift");

struct PROTO_NC_ITEMDB_SELLLOT_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_ITEM_ATTRCHANGE iteminfo;
    uint8_t _pad_at_0002[24];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SELLLOT_REQ) == 60, "PROTO_NC_ITEMDB_SELLLOT_REQ size drift");

struct PROTO_NC_ITEMDB_SHIELDENDURESET_CMD {
    SHINE_ITEM_REGISTNUMBER ShieldKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SHIELDENDURESET_CMD) == 12, "PROTO_NC_ITEMDB_SHIELDENDURESET_CMD size drift");

struct PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER ShieldKey;
    uint8_t _pad_at_0008[8];
    ITEM_INVEN ShieldSlot;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_ACK) == 24, "PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_ACK size drift");

struct PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_REQ {
    SHINE_ITEM_REGISTNUMBER ShieldKey;
    uint8_t _pad_at_0000[8];
    ITEM_INVEN ShieldSlot;
    uint8_t _pad_at_0008[6];
    PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ CommonData;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_REQ) == 35, "PROTO_NC_ITEMDB_SHIELDENDURE_CHARGE_REQ size drift");

struct PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK) == 4, "PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_SOULSTONEBUYFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK) == 2, "PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK size drift");

struct PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK_SEND) == 11, "PROTO_NC_ITEMDB_SOULSTONEBUYSUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SOULSTONEBUY_REQ { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SOULSTONEBUY_REQ) == 17, "PROTO_NC_ITEMDB_SOULSTONEBUY_REQ size drift");

struct PROTO_NC_ITEMDB_SPLITFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SPLITFAIL_ACK) == 4, "PROTO_NC_ITEMDB_SPLITFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_SPLITFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_SPLITFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLITFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_SPLITFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SPLITSUC_ACK {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_REGISTNUMBER lotmain;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLITSUC_ACK) == 12, "PROTO_NC_ITEMDB_SPLITSUC_ACK size drift");

struct PROTO_NC_ITEMDB_SPLITSUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_SPLITSUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLITSUC_ACK_SEND) == 21, "PROTO_NC_ITEMDB_SPLITSUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK) == 4, "PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK_SEND) == 13, "PROTO_NC_ITEMDB_SPLIT_N_MERGEFAIL_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_REGISTNUMBER FromItemKey;
    uint8_t _pad_at_0002[8];
    SHINE_ITEM_REGISTNUMBER ToItemKey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK) == 24, "PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK size drift");

struct PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER zoneheader;
    PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK_SEND) == 33, "PROTO_NC_ITEMDB_SPLIT_N_MERGESUC_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_SPLIT_N_MERGE_REQ {
    uint8_t _pad_at_0000[10];
    SHINE_ITEM_REGISTNUMBER FromItemKey;
    uint8_t _pad_at_000a[12];
    SHINE_ITEM_REGISTNUMBER ToItemKey;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_N_MERGE_REQ) == 38, "PROTO_NC_ITEMDB_SPLIT_N_MERGE_REQ size drift");

struct PROTO_NC_ITEMDB_SPLIT_REQ {
    uint8_t _pad_at_0000[12];
    SHINE_ITEM_REGISTNUMBER lotmain;
    uint8_t _pad_at_000c[8];
    PROTO_ITEM_CREATE lotcreate;
    uint8_t _tail[42];
};
static_assert(sizeof(PROTO_NC_ITEMDB_SPLIT_REQ) == 62, "PROTO_NC_ITEMDB_SPLIT_REQ size drift");

struct PROTO_NC_ITEMDB_TERMEXTEND_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEMDB_TERMEXTEND_ACK) == 10, "PROTO_NC_ITEMDB_TERMEXTEND_ACK size drift");

struct PROTO_NC_ITEMDB_TERMEXTEND_REQ {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nExtendItemKey;
    uint8_t _pad_at_0008[14];
    SHINE_ITEM_REGISTNUMBER nTermItemKey;
    uint8_t _pad_at_0016[10];
    SHINE_ITEM_ATTRIBUTE TermItemAttr;
    uint8_t _pad_at_0020[101];
    ShineDateTime OrigDelDateTime;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_TERMEXTEND_REQ) == 141, "PROTO_NC_ITEMDB_TERMEXTEND_REQ size drift");

struct PROTO_NC_ITEMDB_UES_FRIEND_POINT_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_ITEMDB_UES_FRIEND_POINT_ACK) == 15, "PROTO_NC_ITEMDB_UES_FRIEND_POINT_ACK size drift");

struct PROTO_NC_ITEMDB_UES_FRIEND_POINT_REQ {
    NETPACKETZONEHEADER header;
    ITEM_INVEN itemInven;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEMDB_UES_FRIEND_POINT_REQ) == 12, "PROTO_NC_ITEMDB_UES_FRIEND_POINT_REQ size drift");

struct PROTO_NC_ITEMDB_UNEQUIPFAIL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_UNEQUIPFAIL_ACK) == 4, "PROTO_NC_ITEMDB_UNEQUIPFAIL_ACK size drift");

struct PROTO_NC_ITEMDB_UNEQUIPSUC_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEMDB_UNEQUIPSUC_ACK) == 2, "PROTO_NC_ITEMDB_UNEQUIPSUC_ACK size drift");

struct PROTO_NC_ITEMDB_UNEQUIP_REQ {
    PROTO_ITEM_RELOC unequ;
    uint8_t _pad_at_0000[12];
    ITEM_INVEN equloc;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEMDB_UNEQUIP_REQ) == 14, "PROTO_NC_ITEMDB_UNEQUIP_REQ size drift");

struct PROTO_NC_ITEMDB_UPGRADE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEMDB_UPGRADE_ACK) == 10, "PROTO_NC_ITEMDB_UPGRADE_ACK size drift");

struct PROTO_NC_ITEMDB_UPGRADE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_ITEMDB_UPGRADE_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_UPGRADE_ACK_SEND) == 13, "PROTO_NC_ITEMDB_UPGRADE_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_UPGRADE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER upgrade_item_regnum;
    uint8_t _pad_at_0008[14];
    SHINE_ITEM_REGISTNUMBER raw_regnum;
    uint8_t _pad_at_0016[8];
    SHINE_ITEM_REGISTNUMBER raw_left_regnum;
    uint8_t _pad_at_001e[8];
    SHINE_ITEM_REGISTNUMBER raw_right_regnum;
    uint8_t _pad_at_0026[8];
    SHINE_ITEM_REGISTNUMBER raw_middle_regnum;
    uint8_t _pad_at_002e[29];
    ItemOptionStorage__Element upgrade_randomoption;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEMDB_UPGRADE_REQ) == 78, "PROTO_NC_ITEMDB_UPGRADE_REQ size drift");

struct PROTO_NC_ITEMDB_USEALL_REQ {
    SHINE_ITEM_REGISTNUMBER attritemregnum;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_USEALL_REQ) == 12, "PROTO_NC_ITEMDB_USEALL_REQ size drift");

struct PROTO_NC_ITEMDB_USELOT_REQ {
    PROTO_CHANGEATTR attr;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_USELOT_REQ) == 14, "PROTO_NC_ITEMDB_USELOT_REQ size drift");

struct PROTO_NC_ITEMDB_USE_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEMDB_USE_ACK) == 4, "PROTO_NC_ITEMDB_USE_ACK size drift");

struct PROTO_NC_ITEMDB_USE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_USE_ACK ack;
};
static_assert(sizeof(PROTO_NC_ITEMDB_USE_ACK_SEND) == 13, "PROTO_NC_ITEMDB_USE_ACK_SEND size drift");

struct PROTO_NC_ITEMDB_WEAPONENDURESET_CMD {
    SHINE_ITEM_REGISTNUMBER WeaponKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_WEAPONENDURESET_CMD) == 12, "PROTO_NC_ITEMDB_WEAPONENDURESET_CMD size drift");

struct PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[2];
    SHINE_ITEM_REGISTNUMBER WeaponKey;
    uint8_t _pad_at_0008[8];
    ITEM_INVEN WeaponSlot;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_ACK) == 24, "PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_ACK size drift");

struct PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_REQ {
    SHINE_ITEM_REGISTNUMBER WeaponKey;
    uint8_t _pad_at_0000[8];
    ITEM_INVEN WeaponSlot;
    uint8_t _pad_at_0008[6];
    PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ CommonData;
};
static_assert(sizeof(PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_REQ) == 35, "PROTO_NC_ITEMDB_WEAPONENDURE_CHARGE_REQ size drift");

struct PROTO_NC_ITEMDB_WITHDRAW_ACK {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_ITEMDB_WITHDRAW_ACK) == 18, "PROTO_NC_ITEMDB_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEMDB_WITHDRAW_REQ {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_ITEMDB_WITHDRAW_REQ) == 20, "PROTO_NC_ITEMDB_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEM_ACCOUNT_STORAGE_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_ACCOUNT_STORAGE_CLOSE_CMD) == 1, "PROTO_NC_ITEM_ACCOUNT_STORAGE_CLOSE_CMD size drift");

struct PROTO_NC_ITEM_ACCOUNT_STORAGE_OPEN_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_ACCOUNT_STORAGE_OPEN_CMD) == 1, "PROTO_NC_ITEM_ACCOUNT_STORAGE_OPEN_CMD size drift");

struct PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_ACK) == 2, "PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_ACK size drift");

struct PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_REQ) == 1, "PROTO_NC_ITEM_AUTO_ARRANGE_INVEN_REQ size drift");

struct PROTO_NC_ITEM_BRACELET_UPGRADE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_BRACELET_UPGRADE_ACK) == 2, "PROTO_NC_ITEM_BRACELET_UPGRADE_ACK size drift");

struct PROTO_NC_ITEM_BRACELET_UPGRADE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_BRACELET_UPGRADE_REQ) == 2, "PROTO_NC_ITEM_BRACELET_UPGRADE_REQ size drift");

struct PROTO_NC_ITEM_BREAKFAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_BREAKFAIL_CMD) == 2, "PROTO_NC_ITEM_BREAKFAIL_CMD size drift");

struct PROTO_NC_ITEM_BUY_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ITEM_BUY_REQ) == 6, "PROTO_NC_ITEM_BUY_REQ size drift");

struct PROTO_NC_ITEM_BUY_SUC_ACTION_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_BUY_SUC_ACTION_CMD) == 2, "PROTO_NC_ITEM_BUY_SUC_ACTION_CMD size drift");

struct PROTO_NC_ITEM_CELLCHANGE_CMD {
    ITEM_INVEN exchange;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN location;
    uint8_t _pad_at_0002[2];
    SHINE_ITEM_VAR_STRUCT item;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_CELLCHANGE_CMD) == 6, "PROTO_NC_ITEM_CELLCHANGE_CMD size drift");

struct PROTO_NC_ITEM_CHARGEDINVENOPEN_ACK {
    uint8_t _pad_at_0000[5];
    PROTO_CHARGED_ITEM_INFO_______0_bytes___ ChargedItemInfoList;
};
static_assert(sizeof(PROTO_NC_ITEM_CHARGEDINVENOPEN_ACK) == 5, "PROTO_NC_ITEM_CHARGEDINVENOPEN_ACK size drift");

struct PROTO_NC_ITEM_CHARGEDINVENOPEN_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_CHARGEDINVENOPEN_REQ) == 2, "PROTO_NC_ITEM_CHARGEDINVENOPEN_REQ size drift");

struct PROTO_NC_ITEM_CHARGED_WITHDRAW_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ITEM_CHARGED_WITHDRAW_ACK) == 6, "PROTO_NC_ITEM_CHARGED_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEM_CHARGED_WITHDRAW_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEM_CHARGED_WITHDRAW_REQ) == 4, "PROTO_NC_ITEM_CHARGED_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEM_CHAT_COLOR_CHANGE_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEM_CHAT_COLOR_CHANGE_ACK) == 3, "PROTO_NC_ITEM_CHAT_COLOR_CHANGE_ACK size drift");

struct PROTO_NC_ITEM_CHAT_COLOR_CHANGE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_CHAT_COLOR_CHANGE_REQ) == 1, "PROTO_NC_ITEM_CHAT_COLOR_CHANGE_REQ size drift");

struct PROTO_NC_ITEM_CLASS_CHANGE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_CLASS_CHANGE_ACK) == 2, "PROTO_NC_ITEM_CLASS_CHANGE_ACK size drift");

struct PROTO_NC_ITEM_CLASS_CHANGE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_CLASS_CHANGE_REQ) == 1, "PROTO_NC_ITEM_CLASS_CHANGE_REQ size drift");

struct PROTO_NC_ITEM_DEPOSIT_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_DEPOSIT_ACK) == 10, "PROTO_NC_ITEM_DEPOSIT_ACK size drift");

struct PROTO_NC_ITEM_DEPOSIT_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_ITEM_DEPOSIT_REQ) == 8, "PROTO_NC_ITEM_DEPOSIT_REQ size drift");

struct PROTO_NC_ITEM_DICE_GAME_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ITEM_DICE_GAME_CMD) == 6, "PROTO_NC_ITEM_DICE_GAME_CMD size drift");

struct PROTO_NC_ITEM_DICE_GAME_RESULT_CMD {
    uint8_t _pad_at_0000[2];
    Name5 WinnerID;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_ITEM_DICE_GAME_RESULT_CMD) == 28, "PROTO_NC_ITEM_DICE_GAME_RESULT_CMD size drift");

struct PROTO_NC_ITEM_DICE_GAME_START_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_DICE_GAME_START_ACK) == 2, "PROTO_NC_ITEM_DICE_GAME_START_ACK size drift");

struct PROTO_NC_ITEM_DICE_GAME_START_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ITEM_DICE_GAME_START_CMD) == 6, "PROTO_NC_ITEM_DICE_GAME_START_CMD size drift");

struct PROTO_NC_ITEM_DICE_GAME_START_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEM_DICE_GAME_START_REQ) == 3, "PROTO_NC_ITEM_DICE_GAME_START_REQ size drift");

struct PROTO_NC_ITEM_DISMANTLE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_DISMANTLE_ACK) == 2, "PROTO_NC_ITEM_DISMANTLE_ACK size drift");

struct PROTO_NC_ITEM_DISMANTLE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_DISMANTLE_REQ) == 1, "PROTO_NC_ITEM_DISMANTLE_REQ size drift");

struct PROTO_NC_ITEM_DROP_REQ {
    ITEM_INVEN slot;
    uint8_t _pad_at_0000[6];
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEM_DROP_REQ) == 14, "PROTO_NC_ITEM_DROP_REQ size drift");

struct PROTO_NC_ITEM_ENCHANT_ADD_GEM_ACK {
    uint8_t _pad_at_0000[2];
    ITEM_INVEN equipment;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_ADD_GEM_ACK) == 7, "PROTO_NC_ITEM_ENCHANT_ADD_GEM_ACK size drift");

struct PROTO_NC_ITEM_ENCHANT_ADD_GEM_REQ {
    ITEM_INVEN equipment;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN gem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_ADD_GEM_REQ) == 4, "PROTO_NC_ITEM_ENCHANT_ADD_GEM_REQ size drift");

struct PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_ACK {
    uint8_t _pad_at_0000[2];
    ITEM_INVEN equipment;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_ACK) == 5, "PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_ACK size drift");

struct PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_REQ {
    ITEM_INVEN equipment;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN drill;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_REQ) == 4, "PROTO_NC_ITEM_ENCHANT_ADD_NEW_SOCKET_REQ size drift");

struct PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_ACK {
    uint8_t _pad_at_0000[2];
    ITEM_INVEN equipment;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_ACK) == 5, "PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_ACK size drift");

struct PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_REQ {
    ITEM_INVEN equipment;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN remover;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_REQ) == 5, "PROTO_NC_ITEM_ENCHANT_REMOVE_GEM_REQ size drift");

struct PROTO_NC_ITEM_ENCHANT_SET_GEM_LOT_CMD {
    ITEM_INVEN equipment;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEM_ENCHANT_SET_GEM_LOT_CMD) == 4, "PROTO_NC_ITEM_ENCHANT_SET_GEM_LOT_CMD size drift");

struct PROTO_NC_ITEM_EQUIPCHANGE_CMD {
    ITEM_INVEN exchange;
    uint8_t _pad_at_0000[3];
    SHINE_ITEM_VAR_STRUCT item;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_EQUIPCHANGE_CMD) == 5, "PROTO_NC_ITEM_EQUIPCHANGE_CMD size drift");

struct PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_ACK) == 2, "PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_ACK size drift");

struct PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_REQ) == 2, "PROTO_NC_ITEM_EQUIP_BELONGED_CANCEL_USE_REQ size drift");

struct PROTO_NC_ITEM_EQUIP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_EQUIP_REQ) == 1, "PROTO_NC_ITEM_EQUIP_REQ size drift");

struct PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK) == 10, "PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_ACK size drift");

struct PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ) == 8, "PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_DEPOSIT_REQ size drift");

struct PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK) == 10, "PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ) == 8, "PROTO_NC_ITEM_GUILD_ACADEMY_REWARD_STORAGE_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_ACK) == 10, "PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_REQ) == 8, "PROTO_NC_ITEM_GUILD_STORAGE_WITHDRAW_REQ size drift");

struct PROTO_NC_ITEM_ITEMBREAK_CMD {
    ITEM_INVEN slot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_ITEMBREAK_CMD) == 2, "PROTO_NC_ITEM_ITEMBREAK_CMD size drift");

struct PROTO_NC_ITEM_MAPLINK_SCROLL_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEM_MAPLINK_SCROLL_ACK) == 4, "PROTO_NC_ITEM_MAPLINK_SCROLL_ACK size drift");

struct PROTO_NC_ITEM_MAPLINK_SCROLL_REQ {
    WM_Link targetMap;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEM_MAPLINK_SCROLL_REQ) == 4, "PROTO_NC_ITEM_MAPLINK_SCROLL_REQ size drift");

struct PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_ACK {
    ShineDateTime dEndEndureDate;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_ACK) == 6, "PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_ACK size drift");

struct PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_REQ) == 3, "PROTO_NC_ITEM_MH_FURNITURE_ENDURE_KIT_USE_REQ size drift");

struct PROTO_NC_ITEM_MINIMON_EQUIP_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_MINIMON_EQUIP_REQ) == 2, "PROTO_NC_ITEM_MINIMON_EQUIP_REQ size drift");

struct PROTO_NC_ITEM_MIX_ITEM_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_MIX_ITEM_ACK) == 2, "PROTO_NC_ITEM_MIX_ITEM_ACK size drift");

struct PROTO_NC_ITEM_MIX_ITEM_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_ITEM_MIX_ITEM_REQ) == 3, "PROTO_NC_ITEM_MIX_ITEM_REQ size drift");

struct PROTO_NC_ITEM_MOVER_UPGRADE_ACK {
    uint8_t _pad_at_0000[7];
    SHINE_ITEM_ATTRIBUTE ItemAttr;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_ITEM_MOVER_UPGRADE_ACK) == 108, "PROTO_NC_ITEM_MOVER_UPGRADE_ACK size drift");

struct PROTO_NC_ITEM_MOVER_UPGRADE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_MOVER_UPGRADE_REQ) == 2, "PROTO_NC_ITEM_MOVER_UPGRADE_REQ size drift");

struct PROTO_NC_ITEM_NEW_UPGRADE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_NEW_UPGRADE_ACK) == 2, "PROTO_NC_ITEM_NEW_UPGRADE_ACK size drift");

struct PROTO_NC_ITEM_NEW_UPGRADE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_NEW_UPGRADE_REQ) == 2, "PROTO_NC_ITEM_NEW_UPGRADE_REQ size drift");

struct PROTO_NC_ITEM_OPENCLASSCHANGEMENU_CMD { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_ITEM_OPENCLASSCHANGEMENU_CMD) == 11, "PROTO_NC_ITEM_OPENCLASSCHANGEMENU_CMD size drift");

struct PROTO_NC_ITEM_OPENSTORAGEPAGE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_OPENSTORAGEPAGE_REQ) == 1, "PROTO_NC_ITEM_OPENSTORAGEPAGE_REQ size drift");

struct PROTO_NC_ITEM_PICKOTHER_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEM_PICKOTHER_ACK) == 4, "PROTO_NC_ITEM_PICKOTHER_ACK size drift");

struct PROTO_NC_ITEM_PICK_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_PICK_ACK) == 10, "PROTO_NC_ITEM_PICK_ACK size drift");

struct PROTO_NC_ITEM_PICK_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_PICK_REQ) == 2, "PROTO_NC_ITEM_PICK_REQ size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_ACK) == 2, "PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_ACK size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_REQ) == 1, "PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACCEPT_REQ size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACK {
    uint8_t _pad_at_0000[2];
    ItemOptionStorage changeAbleOption;
    uint8_t _tail[25];
};
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACK) == 27, "PROTO_NC_ITEM_RANDOMOPTION_CHANGE_ACK size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_CHANGE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_CHANGE_REQ) == 2, "PROTO_NC_ITEM_RANDOMOPTION_CHANGE_REQ size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK) == 2, "PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_ACK size drift");

struct PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ) == 2, "PROTO_NC_ITEM_RANDOMOPTION_RECOVER_COUNT_LIMIT_REQ size drift");

struct PROTO_NC_ITEM_RELOC_REQ {
    ITEM_INVEN from;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN to;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_RELOC_REQ) == 4, "PROTO_NC_ITEM_RELOC_REQ size drift");

struct PROTO_NC_ITEM_REPURCHASE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_REPURCHASE_ACK) == 2, "PROTO_NC_ITEM_REPURCHASE_ACK size drift");

struct PROTO_NC_ITEM_REPURCHASE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_REPURCHASE_REQ) == 2, "PROTO_NC_ITEM_REPURCHASE_REQ size drift");

struct PROTO_NC_ITEM_REWARDINVENOPEN_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_ITEMPACKET_INFORM_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_ITEM_REWARDINVENOPEN_ACK) == 1, "PROTO_NC_ITEM_REWARDINVENOPEN_ACK size drift");

struct PROTO_NC_ITEM_REWARDINVENOPEN_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_REWARDINVENOPEN_REQ) == 2, "PROTO_NC_ITEM_REWARDINVENOPEN_REQ size drift");

struct PROTO_NC_ITEM_RINGEQUIP_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_RINGEQUIP_REQ) == 2, "PROTO_NC_ITEM_RINGEQUIP_REQ size drift");

struct PROTO_NC_ITEM_ROT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_ROT_CMD) == 2, "PROTO_NC_ITEM_ROT_CMD size drift");

struct PROTO_NC_ITEM_SELL_ITEM_DELETE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_SELL_ITEM_DELETE_CMD) == 2, "PROTO_NC_ITEM_SELL_ITEM_DELETE_CMD size drift");

struct PROTO_NC_ITEM_SELL_ITEM_INSERT_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_ITEM_STRUCT ItemInfo;
    uint8_t _tail[103];
};
static_assert(sizeof(PROTO_NC_ITEM_SELL_ITEM_INSERT_CMD) == 105, "PROTO_NC_ITEM_SELL_ITEM_INSERT_CMD size drift");

struct PROTO_NC_ITEM_SELL_ITEM_LIST_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_SELL_ITEM_INFO_______0_bytes___ SellItemList;
};
static_assert(sizeof(PROTO_NC_ITEM_SELL_ITEM_LIST_CMD) == 1, "PROTO_NC_ITEM_SELL_ITEM_LIST_CMD size drift");

struct PROTO_NC_ITEM_SELL_REQ { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_ITEM_SELL_REQ) == 5, "PROTO_NC_ITEM_SELL_REQ size drift");

struct PROTO_NC_ITEM_SHIELDENDURESET_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEM_SHIELDENDURESET_CMD) == 4, "PROTO_NC_ITEM_SHIELDENDURESET_CMD size drift");

struct PROTO_NC_ITEM_SHIELDENDURE_CHARGE_ACK {
    uint8_t _pad_at_0000[4];
    ITEM_INVEN ShieldSlot;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEM_SHIELDENDURE_CHARGE_ACK) == 8, "PROTO_NC_ITEM_SHIELDENDURE_CHARGE_ACK size drift");

struct PROTO_NC_ITEM_SHIELDENDURE_CHARGE_REQ {
    uint8_t _pad_at_0000[1];
    ITEM_INVEN ShieldSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_SHIELDENDURE_CHARGE_REQ) == 3, "PROTO_NC_ITEM_SHIELDENDURE_CHARGE_REQ size drift");

struct PROTO_NC_ITEM_SOMEONEPICK_CMD {
    Name5 pickerID;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_ITEM_SOMEONEPICK_CMD) == 26, "PROTO_NC_ITEM_SOMEONEPICK_CMD size drift");

struct PROTO_NC_ITEM_SOMEONEUSE_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE useloc;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_ITEM_SOMEONEUSE_CMD) == 12, "PROTO_NC_ITEM_SOMEONEUSE_CMD size drift");

struct PROTO_NC_ITEM_SPLIT_REQ {
    ITEM_INVEN from;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN to;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_ITEM_SPLIT_REQ) == 8, "PROTO_NC_ITEM_SPLIT_REQ size drift");

struct PROTO_NC_ITEM_TERMEXTEND_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_TERMEXTEND_ACK) == 2, "PROTO_NC_ITEM_TERMEXTEND_ACK size drift");

struct PROTO_NC_ITEM_TERMEXTEND_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_TERMEXTEND_REQ) == 2, "PROTO_NC_ITEM_TERMEXTEND_REQ size drift");

struct PROTO_NC_ITEM_UNEQUIP_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_UNEQUIP_REQ) == 2, "PROTO_NC_ITEM_UNEQUIP_REQ size drift");

struct PROTO_NC_ITEM_UPGRADE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_UPGRADE_ACK) == 2, "PROTO_NC_ITEM_UPGRADE_ACK size drift");

struct PROTO_NC_ITEM_UPGRADE_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_ITEM_UPGRADE_REQ) == 9, "PROTO_NC_ITEM_UPGRADE_REQ size drift");

struct PROTO_NC_ITEM_USE_ACK { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_ITEM_USE_ACK) == 5, "PROTO_NC_ITEM_USE_ACK size drift");

struct PROTO_NC_ITEM_USE_ACTIVESKILL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_USE_ACTIVESKILL_ACK) == 2, "PROTO_NC_ITEM_USE_ACTIVESKILL_ACK size drift");

struct PROTO_NC_ITEM_USE_ACTIVESKILL_REQ {
    uint8_t _pad_at_0000[3];
    SHINE_XY_TYPE TargetLoc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_ITEM_USE_ACTIVESKILL_REQ) == 11, "PROTO_NC_ITEM_USE_ACTIVESKILL_REQ size drift");

struct PROTO_NC_ITEM_USE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_ITEM_USE_REQ) == 2, "PROTO_NC_ITEM_USE_REQ size drift");

struct PROTO_NC_ITEM_WEAPONENDURESET_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_ITEM_WEAPONENDURESET_CMD) == 4, "PROTO_NC_ITEM_WEAPONENDURESET_CMD size drift");

struct PROTO_NC_ITEM_WEAPONENDURE_CHARGE_ACK {
    uint8_t _pad_at_0000[4];
    ITEM_INVEN WeaponSlot;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_ITEM_WEAPONENDURE_CHARGE_ACK) == 8, "PROTO_NC_ITEM_WEAPONENDURE_CHARGE_ACK size drift");

struct PROTO_NC_ITEM_WEAPONENDURE_CHARGE_REQ {
    uint8_t _pad_at_0000[1];
    ITEM_INVEN WeaponSlot;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_ITEM_WEAPONENDURE_CHARGE_REQ) == 3, "PROTO_NC_ITEM_WEAPONENDURE_CHARGE_REQ size drift");

struct PROTO_NC_ITEM_WITHDRAW_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_ITEM_WITHDRAW_ACK) == 10, "PROTO_NC_ITEM_WITHDRAW_ACK size drift");

struct PROTO_NC_ITEM_WITHDRAW_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_ITEM_WITHDRAW_REQ) == 8, "PROTO_NC_ITEM_WITHDRAW_REQ size drift");

struct PROTO_NC_KQ_COMPLETE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_COMPLETE_CMD) == 1, "PROTO_NC_KQ_COMPLETE_CMD size drift");

struct PROTO_NC_KQ_ENTRYRESPONCE_ACK { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_KQ_ENTRYRESPONCE_ACK) == 5, "PROTO_NC_KQ_ENTRYRESPONCE_ACK size drift");

struct PROTO_NC_KQ_ENTRYRESPONCE_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_KQ_ENTRYRESPONCE_REQ) == 6, "PROTO_NC_KQ_ENTRYRESPONCE_REQ size drift");

struct PROTO_NC_KQ_FAIL_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_FAIL_CMD) == 1, "PROTO_NC_KQ_FAIL_CMD size drift");

struct PROTO_NC_KQ_JOINER { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_KQ_JOINER) == 5, "PROTO_NC_KQ_JOINER size drift");

struct PROTO_NC_KQ_JOINING_ALARM_CMD {
    KQ_JOINING_ALARM_INFO KQInfo;
    uint8_t _pad_at_0000[9];
    uint8_t Msg[0];
};
static_assert(sizeof(PROTO_NC_KQ_JOINING_ALARM_CMD) == 9, "PROTO_NC_KQ_JOINING_ALARM_CMD size drift");

struct PROTO_NC_KQ_JOINING_ALARM_END_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_KQ_JOINING_ALARM_END_CMD) == 6, "PROTO_NC_KQ_JOINING_ALARM_END_CMD size drift");

struct PROTO_NC_KQ_JOINING_ALARM_LIST {
    uint8_t _pad_at_0000[2];
    KQ_JOINING_ALARM_INFO_______0_bytes___ KQList;
};
static_assert(sizeof(PROTO_NC_KQ_JOINING_ALARM_LIST) == 2, "PROTO_NC_KQ_JOINING_ALARM_LIST size drift");

struct PROTO_NC_KQ_JOIN_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_KQ_JOIN_ACK) == 6, "PROTO_NC_KQ_JOIN_ACK size drift");

struct PROTO_NC_KQ_JOIN_CANCEL_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_KQ_JOIN_CANCEL_ACK) == 6, "PROTO_NC_KQ_JOIN_CANCEL_ACK size drift");

struct PROTO_NC_KQ_JOIN_CANCEL_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_JOIN_CANCEL_REQ) == 4, "PROTO_NC_KQ_JOIN_CANCEL_REQ size drift");

struct PROTO_NC_KQ_JOIN_LIST_ACK {
    uint8_t _pad_at_0000[3];
    KQ_JOIN_CHAR_INFO_______0_bytes___ CharInfoList;
};
static_assert(sizeof(PROTO_NC_KQ_JOIN_LIST_ACK) == 3, "PROTO_NC_KQ_JOIN_LIST_ACK size drift");

struct PROTO_NC_KQ_JOIN_LIST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_JOIN_LIST_REQ) == 4, "PROTO_NC_KQ_JOIN_LIST_REQ size drift");

struct PROTO_NC_KQ_JOIN_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_JOIN_REQ) == 4, "PROTO_NC_KQ_JOIN_REQ size drift");

struct PROTO_NC_KQ_LINK_TO_FORCE_BY_BAN_CMD {
    uint8_t _pad_at_0000[4];
    Name3_______48_bytes___ sMapName;
    uint8_t _tail[48];
};
static_assert(sizeof(PROTO_NC_KQ_LINK_TO_FORCE_BY_BAN_CMD) == 52, "PROTO_NC_KQ_LINK_TO_FORCE_BY_BAN_CMD size drift");

struct PROTO_NC_KQ_LIST_ACK {
    uint8_t _pad_at_0000[4];
    tm tm_ServerTime;
    uint8_t _pad_at_0004[46];
    PROTO_KQ_INFO_CLIENT_______0_bytes___ NewQuestArray;
};
static_assert(sizeof(PROTO_NC_KQ_LIST_ACK) == 50, "PROTO_NC_KQ_LIST_ACK size drift");

struct PROTO_NC_KQ_LIST_ADD_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_KQ_INFO_CLIENT_______0_bytes___ AddQuestArray;
};
static_assert(sizeof(PROTO_NC_KQ_LIST_ADD_ACK) == 2, "PROTO_NC_KQ_LIST_ADD_ACK size drift");

struct PROTO_NC_KQ_LIST_DELETE_ACK {
    uint8_t _pad_at_0000[2];
    uint32_t DeleteQuestArray[0];
};
static_assert(sizeof(PROTO_NC_KQ_LIST_DELETE_ACK) == 2, "PROTO_NC_KQ_LIST_DELETE_ACK size drift");

struct PROTO_NC_KQ_LIST_REFRESH_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_LIST_REFRESH_REQ) == 1, "PROTO_NC_KQ_LIST_REFRESH_REQ size drift");

struct PROTO_NC_KQ_LIST_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_KQ_LIST_REQ) == 8, "PROTO_NC_KQ_LIST_REQ size drift");

struct PROTO_NC_KQ_LIST_TIME_ACK {
    uint8_t _pad_at_0000[4];
    tm tm_ServerTime;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_KQ_LIST_TIME_ACK) == 40, "PROTO_NC_KQ_LIST_TIME_ACK size drift");

struct PROTO_NC_KQ_LIST_UPDATE_ACK {
    uint8_t _pad_at_0000[2];
    KQ_UPDATE_ITEMS_______0_bytes___ UpdateQuestArray;
};
static_assert(sizeof(PROTO_NC_KQ_LIST_UPDATE_ACK) == 2, "PROTO_NC_KQ_LIST_UPDATE_ACK size drift");

struct PROTO_NC_KQ_MOBKILLNUMBER_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_MOBKILLNUMBER_CMD) == 4, "PROTO_NC_KQ_MOBKILLNUMBER_CMD size drift");

struct PROTO_NC_KQ_NOREWARD_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_KQ_NOREWARD_CMD) == 2, "PROTO_NC_KQ_NOREWARD_CMD size drift");

struct PROTO_NC_KQ_NOTIFY_CMD {
    uint8_t _pad_at_0000[1];
    uint8_t Msg[0];
};
static_assert(sizeof(PROTO_NC_KQ_NOTIFY_CMD) == 1, "PROTO_NC_KQ_NOTIFY_CMD size drift");

struct PROTO_NC_KQ_PLAYER_DISJOIN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_KQ_PLAYER_DISJOIN_CMD) == 8, "PROTO_NC_KQ_PLAYER_DISJOIN_CMD size drift");

struct PROTO_NC_KQ_RESTDEADNUM_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_RESTDEADNUM_CMD) == 1, "PROTO_NC_KQ_RESTDEADNUM_CMD size drift");

struct PROTO_NC_KQ_REWARDFAIL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_KQ_REWARDFAIL_ACK) == 10, "PROTO_NC_KQ_REWARDFAIL_ACK size drift");

struct PROTO_NC_KQ_REWARDSUC_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_KQ_REWARDSUC_ACK) == 8, "PROTO_NC_KQ_REWARDSUC_ACK size drift");

struct PROTO_NC_KQ_REWARD_REQ {
    uint8_t _pad_at_0000[12];
    PROTO_NC_ITEMDB_CREATEITEMLIST_REQ itmlst;
};
static_assert(sizeof(PROTO_NC_KQ_REWARD_REQ) == 35, "PROTO_NC_KQ_REWARD_REQ size drift");

struct PROTO_NC_KQ_SCHEDULE_ACK {
    uint8_t _pad_at_0000[10];
    PROTO_KQ_INFO_CLIENT_______0_bytes___ NewQuestArray;
};
static_assert(sizeof(PROTO_NC_KQ_SCHEDULE_ACK) == 10, "PROTO_NC_KQ_SCHEDULE_ACK size drift");

struct PROTO_NC_KQ_SCHEDULE_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_KQ_SCHEDULE_REQ) == 8, "PROTO_NC_KQ_SCHEDULE_REQ size drift");

struct PROTO_NC_KQ_SCORE_BOARD_INFO_CMD {
    uint8_t _pad_at_0000[2];
    TEAM_SCORE_INFO Red;
    uint8_t _pad_at_0002[8];
    TEAM_SCORE_INFO Blue;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_BOARD_INFO_CMD) == 20, "PROTO_NC_KQ_SCORE_BOARD_INFO_CMD size drift");

struct PROTO_NC_KQ_SCORE_CMD {
    uint8_t _pad_at_0000[5];
    PROTO_NC_KQ_SCORE_CMD__KQScore_______0_bytes___ array;
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_CMD) == 5, "PROTO_NC_KQ_SCORE_CMD size drift");

struct PROTO_NC_KQ_SCORE_CMD__KQScore___unnamed_type_flag_ {
    uint32_t  medal;
    uint32_t  gender;
    uint8_t _pad_at_0000[1];
    uint32_t  chrcls;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_CMD__KQScore___unnamed_type_flag_) == 2, "PROTO_NC_KQ_SCORE_CMD__KQScore___unnamed_type_flag_ size drift");

struct PROTO_NC_KQ_SCORE_CMD__KQScore {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    PROTO_NC_KQ_SCORE_CMD__KQScore___unnamed_type_flag_ flag;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_CMD__KQScore) == 38, "PROTO_NC_KQ_SCORE_CMD__KQScore size drift");

struct PROTO_NC_KQ_SCORE_INFO_CMD {
    uint32_t nScore[2];
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_INFO_CMD) == 8, "PROTO_NC_KQ_SCORE_INFO_CMD size drift");

struct PROTO_NC_KQ_SCORE_SIMPLE_CMD___unnamed_type_self_ { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_KQ_SCORE_SIMPLE_CMD___unnamed_type_self_) == 13, "PROTO_NC_KQ_SCORE_SIMPLE_CMD___unnamed_type_self_ size drift");

struct PROTO_NC_KQ_SCORE_SIMPLE_CMD {
    uint8_t _pad_at_0000[5];
    PROTO_NC_KQ_SCORE_SIMPLE_CMD___unnamed_type_self_ self;
    PROTO_NC_KQ_SCORE_SIMPLE_CMD__KQScore_______0_bytes___ array;
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_SIMPLE_CMD) == 18, "PROTO_NC_KQ_SCORE_SIMPLE_CMD size drift");

struct PROTO_NC_KQ_SCORE_SIMPLE_CMD__KQScore {
    Name5 charid;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_KQ_SCORE_SIMPLE_CMD__KQScore) == 32, "PROTO_NC_KQ_SCORE_SIMPLE_CMD__KQScore size drift");

struct PROTO_NC_KQ_START_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_START_CMD) == 1, "PROTO_NC_KQ_START_CMD size drift");

struct PROTO_NC_KQ_STATUS_ACK {
    uint8_t _pad_at_0000[7];
    Name5_______0_bytes___ JoinerNameList;
};
static_assert(sizeof(PROTO_NC_KQ_STATUS_ACK) == 7, "PROTO_NC_KQ_STATUS_ACK size drift");

struct PROTO_NC_KQ_STATUS_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_STATUS_REQ) == 4, "PROTO_NC_KQ_STATUS_REQ size drift");

struct PROTO_NC_KQ_TEAM_SELECT_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_KQ_TEAM_SELECT_ACK) == 3, "PROTO_NC_KQ_TEAM_SELECT_ACK size drift");

struct PROTO_NC_KQ_TEAM_SELECT_CMD {
    Name5 sCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_KQ_TEAM_SELECT_CMD) == 21, "PROTO_NC_KQ_TEAM_SELECT_CMD size drift");

struct PROTO_NC_KQ_TEAM_SELECT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_TEAM_SELECT_REQ) == 1, "PROTO_NC_KQ_TEAM_SELECT_REQ size drift");

struct PROTO_NC_KQ_TEAM_TYPE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_TEAM_TYPE_CMD) == 1, "PROTO_NC_KQ_TEAM_TYPE_CMD size drift");

struct PROTO_NC_KQ_VOTE_BAN_MSG_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_BAN_MSG_CMD) == 4, "PROTO_NC_KQ_VOTE_BAN_MSG_CMD size drift");

struct PROTO_NC_KQ_VOTE_BAN_MSG_LOGOFF_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_BAN_MSG_LOGOFF_CMD) == 1, "PROTO_NC_KQ_VOTE_BAN_MSG_LOGOFF_CMD size drift");

struct PROTO_NC_KQ_VOTE_CANCEL_CMD {
    Name5 sTarget;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_CANCEL_CMD) == 20, "PROTO_NC_KQ_VOTE_CANCEL_CMD size drift");

struct PROTO_NC_KQ_VOTE_RESULT_FAIL_CMD {
    Name5 sTarget;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_RESULT_FAIL_CMD) == 23, "PROTO_NC_KQ_VOTE_RESULT_FAIL_CMD size drift");

struct PROTO_NC_KQ_VOTE_RESULT_SUC_CMD {
    Name5 sTarget;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_RESULT_SUC_CMD) == 24, "PROTO_NC_KQ_VOTE_RESULT_SUC_CMD size drift");

struct PROTO_NC_KQ_VOTE_START_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_START_ACK) == 2, "PROTO_NC_KQ_VOTE_START_ACK size drift");

struct PROTO_NC_KQ_VOTE_START_CHECK_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_START_CHECK_ACK) == 2, "PROTO_NC_KQ_VOTE_START_CHECK_ACK size drift");

struct PROTO_NC_KQ_VOTE_START_CHECK_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_START_CHECK_REQ) == 1, "PROTO_NC_KQ_VOTE_START_CHECK_REQ size drift");

struct PROTO_NC_KQ_VOTE_START_REQ {
    Name5 sTarget;
    uint8_t _pad_at_0000[22];
    uint8_t sContents[0];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_START_REQ) == 22, "PROTO_NC_KQ_VOTE_START_REQ size drift");

struct PROTO_NC_KQ_VOTE_VOTING_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_KQ_VOTE_VOTING_ACK) == 2, "PROTO_NC_KQ_VOTE_VOTING_ACK size drift");

struct PROTO_NC_KQ_VOTE_VOTING_CMD {
    Name5 sStarter;
    uint8_t _pad_at_0000[20];
    Name5 sTarget;
    uint8_t _pad_at_0014[21];
    tm tEndTime;
    uint8_t _pad_at_0029[37];
    uint8_t sContents[0];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_VOTING_CMD) == 78, "PROTO_NC_KQ_VOTE_VOTING_CMD size drift");

struct PROTO_NC_KQ_VOTE_VOTING_REQ {
    KQ_VOTING_TYPE eVoteType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_KQ_VOTE_VOTING_REQ) == 4, "PROTO_NC_KQ_VOTE_VOTING_REQ size drift");

struct PROTO_NC_KQ_W2Z_DESTROY_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_W2Z_DESTROY_CMD) == 4, "PROTO_NC_KQ_W2Z_DESTROY_CMD size drift");

struct PROTO_NC_KQ_W2Z_MAKE_REQ {
    PROTO_KQ_INFO KQInfo;
    uint8_t _tail[377];
};
static_assert(sizeof(PROTO_NC_KQ_W2Z_MAKE_REQ) == 377, "PROTO_NC_KQ_W2Z_MAKE_REQ size drift");

struct PROTO_NC_KQ_W2Z_START_CMD {
    PROTO_KQ_INFO KQInfo;
    uint8_t _pad_at_0000[379];
    PROTO_NC_KQ_JOINER_______0_bytes___ JoinerArray;
};
static_assert(sizeof(PROTO_NC_KQ_W2Z_START_CMD) == 379, "PROTO_NC_KQ_W2Z_START_CMD size drift");

struct PROTO_NC_KQ_WINTER_EVENT_2014_SCORE_CMD {
    TEAM_SCORE_INFO Red;
    uint8_t _pad_at_0000[8];
    TEAM_SCORE_INFO Blue;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_KQ_WINTER_EVENT_2014_SCORE_CMD) == 16, "PROTO_NC_KQ_WINTER_EVENT_2014_SCORE_CMD size drift");

struct PROTO_NC_KQ_Z2W_END_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_KQ_Z2W_END_CMD) == 4, "PROTO_NC_KQ_Z2W_END_CMD size drift");

struct PROTO_NC_KQ_Z2W_MAKE_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_KQ_Z2W_MAKE_ACK) == 6, "PROTO_NC_KQ_Z2W_MAKE_ACK size drift");

struct PROTO_NC_LOG_GAME_ADD {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _pad_at_0008[32];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ADD) == 68, "PROTO_NC_LOG_GAME_ADD size drift");

struct PROTO_NC_LOG_GAME_ADD_ACK { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ADD_ACK) == 12, "PROTO_NC_LOG_GAME_ADD_ACK size drift");

struct PROTO_NC_LOG_GAME_ADD_REQ {
    uint8_t _pad_at_0000[4];
    PROTO_NC_LOG_GAME_ADD logdata;
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ADD_REQ) == 72, "PROTO_NC_LOG_GAME_ADD_REQ size drift");

struct PROTO_NC_LOG_GAME_ARENA_CNG { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ARENA_CNG) == 1, "PROTO_NC_LOG_GAME_ARENA_CNG size drift");

struct PROTO_NC_LOG_GAME_ARENA_FBZ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ARENA_FBZ) == 1, "PROTO_NC_LOG_GAME_ARENA_FBZ size drift");

struct PROTO_NC_LOG_GAME_ARENA_GUILD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ARENA_GUILD) == 1, "PROTO_NC_LOG_GAME_ARENA_GUILD size drift");

struct PROTO_NC_LOG_GAME_ARENA_PVP { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ARENA_PVP) == 1, "PROTO_NC_LOG_GAME_ARENA_PVP size drift");

struct PROTO_NC_LOG_GAME_CHANGE_CLASS {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_CHANGE_CLASS) == 26, "PROTO_NC_LOG_GAME_CHANGE_CLASS size drift");

struct PROTO_NC_LOG_GAME_CHARGED_BUFF_CLR { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_CHARGED_BUFF_CLR) == 14, "PROTO_NC_LOG_GAME_CHARGED_BUFF_CLR size drift");

struct PROTO_NC_LOG_GAME_CHARGED_BUFF_SET {
    uint8_t _pad_at_0000[14];
    ShineDateTime buff_endtime;
    uint8_t _pad_at_000e[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_CHARGED_BUFF_SET) == 26, "PROTO_NC_LOG_GAME_CHARGED_BUFF_SET size drift");

struct PROTO_NC_LOG_GAME_CHARGE_WITHDRAW { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_CHARGE_WITHDRAW) == 16, "PROTO_NC_LOG_GAME_CHARGE_WITHDRAW size drift");

struct PROTO_NC_LOG_GAME_CREATE_AVATAR { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_CREATE_AVATAR) == 4, "PROTO_NC_LOG_GAME_CREATE_AVATAR size drift");

struct PROTO_NC_LOG_GAME_CREATE_AVATAR_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_LOG_GAME_CREATE_AVATAR log;
};
static_assert(sizeof(PROTO_NC_LOG_GAME_CREATE_AVATAR_SEND) == 7, "PROTO_NC_LOG_GAME_CREATE_AVATAR_SEND size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_0 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[60];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_0) == 68, "PROTO_NC_LOG_GAME_DATA_TYPE_0 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_1 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_1) == 32, "PROTO_NC_LOG_GAME_DATA_TYPE_1 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_2 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_2) == 40, "PROTO_NC_LOG_GAME_DATA_TYPE_2 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_3 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_3) == 44, "PROTO_NC_LOG_GAME_DATA_TYPE_3 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_4 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_4) == 44, "PROTO_NC_LOG_GAME_DATA_TYPE_4 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_5 { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_5) == 16, "PROTO_NC_LOG_GAME_DATA_TYPE_5 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_6 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_6) == 44, "PROTO_NC_LOG_GAME_DATA_TYPE_6 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_7 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_7) == 44, "PROTO_NC_LOG_GAME_DATA_TYPE_7 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_8 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_8) == 36, "PROTO_NC_LOG_GAME_DATA_TYPE_8 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_9 {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[40];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_9) == 48, "PROTO_NC_LOG_GAME_DATA_TYPE_9 size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_A { uint8_t data[36]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_A) == 36, "PROTO_NC_LOG_GAME_DATA_TYPE_A size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_B {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[52];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_B) == 60, "PROTO_NC_LOG_GAME_DATA_TYPE_B size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_C {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[48];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_C) == 56, "PROTO_NC_LOG_GAME_DATA_TYPE_C size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_D { uint8_t data[24]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_D) == 24, "PROTO_NC_LOG_GAME_DATA_TYPE_D size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_E {
    uint8_t _pad_at_0000[4];
    Name3 sMap;
    uint8_t _tail[44];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_E) == 48, "PROTO_NC_LOG_GAME_DATA_TYPE_E size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_F {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[56];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_F) == 64, "PROTO_NC_LOG_GAME_DATA_TYPE_F size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_G {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[52];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_G) == 60, "PROTO_NC_LOG_GAME_DATA_TYPE_G size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_H {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_H) == 36, "PROTO_NC_LOG_GAME_DATA_TYPE_H size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_I {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_I) == 40, "PROTO_NC_LOG_GAME_DATA_TYPE_I size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_J {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[44];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_J) == 52, "PROTO_NC_LOG_GAME_DATA_TYPE_J size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_K {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[40];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_K) == 48, "PROTO_NC_LOG_GAME_DATA_TYPE_K size drift");

struct PROTO_NC_LOG_GAME_DATA_TYPE_L { uint8_t data[44]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_DATA_TYPE_L) == 44, "PROTO_NC_LOG_GAME_DATA_TYPE_L size drift");

struct PROTO_NC_LOG_GAME_DELETE_AVATAR { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_DELETE_AVATAR) == 4, "PROTO_NC_LOG_GAME_DELETE_AVATAR size drift");

struct PROTO_NC_LOG_GAME_DELETE_AVATAR_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_LOG_GAME_DELETE_AVATAR log;
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DELETE_AVATAR_SEND) == 7, "PROTO_NC_LOG_GAME_DELETE_AVATAR_SEND size drift");

struct PROTO_NC_LOG_GAME_DISENCHANT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[25];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_DISENCHANT) == 29, "PROTO_NC_LOG_GAME_DISENCHANT size drift");

struct PROTO_NC_LOG_GAME_EMBLEM { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_EMBLEM) == 1, "PROTO_NC_LOG_GAME_EMBLEM size drift");

struct PROTO_NC_LOG_GAME_ENCHANNT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[25];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ENCHANNT) == 29, "PROTO_NC_LOG_GAME_ENCHANNT size drift");

struct PROTO_NC_LOG_GAME_FRIEND_ADD {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_FRIEND_ADD) == 28, "PROTO_NC_LOG_GAME_FRIEND_ADD size drift");

struct PROTO_NC_LOG_GAME_FRIEND_DELETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_FRIEND_DELETE) == 28, "PROTO_NC_LOG_GAME_FRIEND_DELETE size drift");

struct PROTO_NC_LOG_GAME_GUILD_4_REWARD_FAME { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_4_REWARD_FAME) == 12, "PROTO_NC_LOG_GAME_GUILD_4_REWARD_FAME size drift");

struct PROTO_NC_LOG_GAME_GUILD_4_TOURNAMENT_RESULT { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_4_TOURNAMENT_RESULT) == 16, "PROTO_NC_LOG_GAME_GUILD_4_TOURNAMENT_RESULT size drift");

struct PROTO_NC_LOG_GAME_GUILD_CREATE {
    uint8_t _pad_at_0000[24];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_CREATE) == 40, "PROTO_NC_LOG_GAME_GUILD_CREATE size drift");

struct PROTO_NC_LOG_GAME_GUILD_DELETE {
    uint8_t _pad_at_0000[4];
    Name4 sGuildName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_DELETE) == 20, "PROTO_NC_LOG_GAME_GUILD_DELETE size drift");

struct PROTO_NC_LOG_GAME_GUILD_GRADE { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_GRADE) == 10, "PROTO_NC_LOG_GAME_GUILD_GRADE size drift");

struct PROTO_NC_LOG_GAME_GUILD_G_REWARD_EXP { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_G_REWARD_EXP) == 20, "PROTO_NC_LOG_GAME_GUILD_G_REWARD_EXP size drift");

struct PROTO_NC_LOG_GAME_GUILD_G_REWARD_MONEY { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_G_REWARD_MONEY) == 20, "PROTO_NC_LOG_GAME_GUILD_G_REWARD_MONEY size drift");

struct PROTO_NC_LOG_GAME_GUILD_G_REWARD_TOKEN { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_G_REWARD_TOKEN) == 20, "PROTO_NC_LOG_GAME_GUILD_G_REWARD_TOKEN size drift");

struct PROTO_NC_LOG_GAME_GUILD_K_MONEY_WITHDRAW { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_K_MONEY_WITHDRAW) == 20, "PROTO_NC_LOG_GAME_GUILD_K_MONEY_WITHDRAW size drift");

struct PROTO_NC_LOG_GAME_GUILD_M_BANISH { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_M_BANISH) == 16, "PROTO_NC_LOG_GAME_GUILD_M_BANISH size drift");

struct PROTO_NC_LOG_GAME_GUILD_M_GRADE { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_M_GRADE) == 14, "PROTO_NC_LOG_GAME_GUILD_M_GRADE size drift");

struct PROTO_NC_LOG_GAME_GUILD_M_JOIN { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_M_JOIN) == 12, "PROTO_NC_LOG_GAME_GUILD_M_JOIN size drift");

struct PROTO_NC_LOG_GAME_GUILD_M_LEAVE { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_M_LEAVE) == 12, "PROTO_NC_LOG_GAME_GUILD_M_LEAVE size drift");

struct PROTO_NC_LOG_GAME_GUILD_TYPE { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_TYPE) == 10, "PROTO_NC_LOG_GAME_GUILD_TYPE size drift");

struct PROTO_NC_LOG_GAME_GUILD_WAR_ACCEPT { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_WAR_ACCEPT) == 12, "PROTO_NC_LOG_GAME_GUILD_WAR_ACCEPT size drift");

struct PROTO_NC_LOG_GAME_GUILD_WAR_DECLARE { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_WAR_DECLARE) == 12, "PROTO_NC_LOG_GAME_GUILD_WAR_DECLARE size drift");

struct PROTO_NC_LOG_GAME_GUILD_WAR_RESULT { uint8_t data[29]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_GUILD_WAR_RESULT) == 29, "PROTO_NC_LOG_GAME_GUILD_WAR_RESULT size drift");

struct PROTO_NC_LOG_GAME_HIT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_HIT) == 31, "PROTO_NC_LOG_GAME_HIT size drift");

struct PROTO_NC_LOG_GAME_ITEM_BOOTH_BUY {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_BOOTH_BUY) == 38, "PROTO_NC_LOG_GAME_ITEM_BOOTH_BUY size drift");

struct PROTO_NC_LOG_GAME_ITEM_BOOTH_SELL {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_BOOTH_SELL) == 38, "PROTO_NC_LOG_GAME_ITEM_BOOTH_SELL size drift");

struct PROTO_NC_LOG_GAME_ITEM_BREAK {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_BREAK) == 16, "PROTO_NC_LOG_GAME_ITEM_BREAK size drift");

struct PROTO_NC_LOG_GAME_ITEM_BUY {
    uint8_t _pad_at_0000[7];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0007[10];
    ITEM_INVEN iteminven;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_BUY) == 23, "PROTO_NC_LOG_GAME_ITEM_BUY size drift");

struct PROTO_NC_LOG_GAME_ITEM_CREATE {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0008[10];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_CREATE) == 20, "PROTO_NC_LOG_GAME_ITEM_CREATE size drift");

struct PROTO_NC_LOG_GAME_ITEM_CW_BREAKATZERO {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[8];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_CW_BREAKATZERO) == 32, "PROTO_NC_LOG_GAME_ITEM_CW_BREAKATZERO size drift");

struct PROTO_NC_LOG_GAME_ITEM_DROP {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0018[10];
    ITEM_INVEN iteminven;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_DROP) == 38, "PROTO_NC_LOG_GAME_ITEM_DROP size drift");

struct PROTO_NC_LOG_GAME_ITEM_EQUIP {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_000c[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_EQUIP) == 16, "PROTO_NC_LOG_GAME_ITEM_EQUIP size drift");

struct PROTO_NC_LOG_GAME_ITEM_INVEN_MOVE {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_000c[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_INVEN_MOVE) == 20, "PROTO_NC_LOG_GAME_ITEM_INVEN_MOVE size drift");

struct PROTO_NC_LOG_GAME_ITEM_MERGE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[22];
    SHINE_ITEM_REGISTNUMBER itemkeyadd;
    uint8_t _pad_at_001a[8];
    SHINE_ITEM_REGISTNUMBER itemkeysub;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_MERGE) == 48, "PROTO_NC_LOG_GAME_ITEM_MERGE size drift");

struct PROTO_NC_LOG_GAME_ITEM_MOB_DROP {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_MOB_DROP) == 38, "PROTO_NC_LOG_GAME_ITEM_MOB_DROP size drift");

struct PROTO_NC_LOG_GAME_ITEM_MOB_DROP_RATE { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_MOB_DROP_RATE) == 12, "PROTO_NC_LOG_GAME_ITEM_MOB_DROP_RATE size drift");

struct PROTO_NC_LOG_GAME_ITEM_PRODUCT {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_PRODUCT) == 20, "PROTO_NC_LOG_GAME_ITEM_PRODUCT size drift");

struct PROTO_NC_LOG_GAME_ITEM_PRODUCT_STUFF {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[10];
    SHINE_ITEM_REGISTNUMBER itemkey_stuff;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_PRODUCT_STUFF) == 30, "PROTO_NC_LOG_GAME_ITEM_PRODUCT_STUFF size drift");

struct PROTO_NC_LOG_GAME_ITEM_PUT_ON_BELONGED {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_000c[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_PUT_ON_BELONGED) == 16, "PROTO_NC_LOG_GAME_ITEM_PUT_ON_BELONGED size drift");

struct PROTO_NC_LOG_GAME_ITEM_SELL {
    uint8_t _pad_at_0000[7];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0007[10];
    ITEM_INVEN iteminven;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_SELL) == 23, "PROTO_NC_LOG_GAME_ITEM_SELL size drift");

struct PROTO_NC_LOG_GAME_ITEM_SOULSTONEBUY {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[31];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_SOULSTONEBUY) == 35, "PROTO_NC_LOG_GAME_ITEM_SOULSTONEBUY size drift");

struct PROTO_NC_LOG_GAME_ITEM_SPLIT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[22];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_001a[8];
    SHINE_ITEM_REGISTNUMBER itemkeynew;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_SPLIT) == 46, "PROTO_NC_LOG_GAME_ITEM_SPLIT size drift");

struct PROTO_NC_LOG_GAME_ITEM_STORE_IN {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0018[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_0020[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_STORE_IN) == 36, "PROTO_NC_LOG_GAME_ITEM_STORE_IN size drift");

struct PROTO_NC_LOG_GAME_ITEM_STORE_OUT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0018[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_0020[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_STORE_OUT) == 36, "PROTO_NC_LOG_GAME_ITEM_STORE_OUT size drift");

struct PROTO_NC_LOG_GAME_ITEM_TAKE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0018[10];
    ITEM_INVEN iteminven;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_TAKE) == 38, "PROTO_NC_LOG_GAME_ITEM_TAKE size drift");

struct PROTO_NC_LOG_GAME_ITEM_TITLE {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER WeaponItemKey;
    uint8_t _pad_at_0004[8];
    SHINE_ITEM_REGISTNUMBER LicenseItemKey;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_TITLE) == 22, "PROTO_NC_LOG_GAME_ITEM_TITLE size drift");

struct PROTO_NC_LOG_GAME_ITEM_TRADE {
    uint8_t _pad_at_0000[10];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_TRADE) == 20, "PROTO_NC_LOG_GAME_ITEM_TRADE size drift");

struct PROTO_NC_LOG_GAME_ITEM_UNEQUIP {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0004[8];
    ITEM_INVEN iteminvenfrom;
    uint8_t _pad_at_000c[2];
    ITEM_INVEN iteminvento;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_UNEQUIP) == 16, "PROTO_NC_LOG_GAME_ITEM_UNEQUIP size drift");

struct PROTO_NC_LOG_GAME_ITEM_UPGRADE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[20];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_0018[16];
    ItemOptionStorage__Element randomoption_elemental;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_UPGRADE) == 43, "PROTO_NC_LOG_GAME_ITEM_UPGRADE size drift");

struct PROTO_NC_LOG_GAME_ITEM_USE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[22];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _pad_at_001a[10];
    ITEM_INVEN iteminven;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_USE) == 40, "PROTO_NC_LOG_GAME_ITEM_USE size drift");

struct PROTO_NC_LOG_GAME_ITEM_USEALL {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_USEALL) == 14, "PROTO_NC_LOG_GAME_ITEM_USEALL size drift");

struct PROTO_NC_LOG_GAME_ITEM_USELOT {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_ITEM_USELOT) == 16, "PROTO_NC_LOG_GAME_ITEM_USELOT size drift");

struct PROTO_NC_LOG_GAME_KQ_ENTER {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_KQ_ENTER) == 30, "PROTO_NC_LOG_GAME_KQ_ENTER size drift");

struct PROTO_NC_LOG_GAME_KQ_LEAVE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_KQ_LEAVE) == 31, "PROTO_NC_LOG_GAME_KQ_LEAVE size drift");

struct PROTO_NC_LOG_GAME_LEVEL_DOWN {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_LEVEL_DOWN) == 33, "PROTO_NC_LOG_GAME_LEVEL_DOWN size drift");

struct PROTO_NC_LOG_GAME_LEVEL_UP {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_LEVEL_UP) == 33, "PROTO_NC_LOG_GAME_LEVEL_UP size drift");

struct PROTO_NC_LOG_GAME_LINK {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[37];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_LINK) == 41, "PROTO_NC_LOG_GAME_LINK size drift");

struct PROTO_NC_LOG_GAME_LOGIN {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[32];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_LOGIN) == 56, "PROTO_NC_LOG_GAME_LOGIN size drift");

struct PROTO_NC_LOG_GAME_LOGOUT {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[31];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_LOGOUT) == 35, "PROTO_NC_LOG_GAME_LOGOUT size drift");

struct PROTO_NC_LOG_GAME_MAS_PUP_ADD {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[25];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MAS_PUP_ADD) == 29, "PROTO_NC_LOG_GAME_MAS_PUP_ADD size drift");

struct PROTO_NC_LOG_GAME_MAS_PUP_DELETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[25];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MAS_PUP_DELETE) == 29, "PROTO_NC_LOG_GAME_MAS_PUP_DELETE size drift");

struct PROTO_NC_LOG_GAME_MINIGAME { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MINIGAME) == 1, "PROTO_NC_LOG_GAME_MINIGAME size drift");

struct PROTO_NC_LOG_GAME_MINIHOUSE { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MINIHOUSE) == 1, "PROTO_NC_LOG_GAME_MINIHOUSE size drift");

struct PROTO_NC_LOG_GAME_MINIHOUSE_BUILDING {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MINIHOUSE_BUILDING) == 24, "PROTO_NC_LOG_GAME_MINIHOUSE_BUILDING size drift");

struct PROTO_NC_LOG_GAME_MINIHOUSE_VISIT {
    uint8_t _pad_at_0000[8];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MINIHOUSE_VISIT) == 28, "PROTO_NC_LOG_GAME_MINIHOUSE_VISIT size drift");

struct PROTO_NC_LOG_GAME_MK {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK) == 26, "PROTO_NC_LOG_GAME_MK size drift");

struct PROTO_NC_LOG_GAME_MK2_DEAD {
    uint8_t _pad_at_0000[2];
    MAPPOS mappos;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK2_DEAD) == 26, "PROTO_NC_LOG_GAME_MK2_DEAD size drift");

struct PROTO_NC_LOG_GAME_MK2_FAIL {
    uint8_t _pad_at_0000[8];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK2_FAIL) == 28, "PROTO_NC_LOG_GAME_MK2_FAIL size drift");

struct PROTO_NC_LOG_GAME_MK2_START {
    uint8_t _pad_at_0000[8];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK2_START) == 28, "PROTO_NC_LOG_GAME_MK2_START size drift");

struct PROTO_NC_LOG_GAME_MK2_SUCCESS {
    uint8_t _pad_at_0000[8];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK2_SUCCESS) == 28, "PROTO_NC_LOG_GAME_MK2_SUCCESS size drift");

struct PROTO_NC_LOG_GAME_MKED {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[30];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MKED) == 34, "PROTO_NC_LOG_GAME_MKED size drift");

struct PROTO_NC_LOG_GAME_MK_DROP_ITEM {
    uint8_t _pad_at_0000[4];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _pad_at_0004[10];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK_DROP_ITEM) == 34, "PROTO_NC_LOG_GAME_MK_DROP_ITEM size drift");

struct PROTO_NC_LOG_GAME_MK_GETEXP {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MK_GETEXP) == 30, "PROTO_NC_LOG_GAME_MK_GETEXP size drift");

struct PROTO_NC_LOG_GAME_MK_GETEXPINFIELD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MK_GETEXPINFIELD) == 12, "PROTO_NC_LOG_GAME_MK_GETEXPINFIELD size drift");

struct PROTO_NC_LOG_GAME_MONEY_CHANGE { uint8_t data[28]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MONEY_CHANGE) == 28, "PROTO_NC_LOG_GAME_MONEY_CHANGE size drift");

struct PROTO_NC_LOG_GAME_MONEY_DEPOSIT { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MONEY_DEPOSIT) == 20, "PROTO_NC_LOG_GAME_MONEY_DEPOSIT size drift");

struct PROTO_NC_LOG_GAME_MONEY_TRADE_INCOME { uint8_t data[24]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MONEY_TRADE_INCOME) == 24, "PROTO_NC_LOG_GAME_MONEY_TRADE_INCOME size drift");

struct PROTO_NC_LOG_GAME_MONEY_TRADE_OUTGO { uint8_t data[24]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MONEY_TRADE_OUTGO) == 24, "PROTO_NC_LOG_GAME_MONEY_TRADE_OUTGO size drift");

struct PROTO_NC_LOG_GAME_MONEY_WITHDRAW { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_MONEY_WITHDRAW) == 20, "PROTO_NC_LOG_GAME_MONEY_WITHDRAW size drift");

struct PROTO_NC_LOG_GAME_MOVE {
    uint8_t _pad_at_0000[4];
    MAPPOS startpos;
    uint8_t _pad_at_0004[20];
    SHINE_XY_TYPE endpos;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_MOVE) == 32, "PROTO_NC_LOG_GAME_MOVE size drift");

struct PROTO_NC_LOG_GAME_PARTY_BANISH {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_BANISH) == 30, "PROTO_NC_LOG_GAME_PARTY_BANISH size drift");

struct PROTO_NC_LOG_GAME_PARTY_CHG_MAS {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_CHG_MAS) == 30, "PROTO_NC_LOG_GAME_PARTY_CHG_MAS size drift");

struct PROTO_NC_LOG_GAME_PARTY_CREATE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_CREATE) == 26, "PROTO_NC_LOG_GAME_PARTY_CREATE size drift");

struct PROTO_NC_LOG_GAME_PARTY_DELETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_DELETE) == 26, "PROTO_NC_LOG_GAME_PARTY_DELETE size drift");

struct PROTO_NC_LOG_GAME_PARTY_JOIN {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_JOIN) == 26, "PROTO_NC_LOG_GAME_PARTY_JOIN size drift");

struct PROTO_NC_LOG_GAME_PARTY_LEAVE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PARTY_LEAVE) == 26, "PROTO_NC_LOG_GAME_PARTY_LEAVE size drift");

struct PROTO_NC_LOG_GAME_PET { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_PET) == 1, "PROTO_NC_LOG_GAME_PET size drift");

struct PROTO_NC_LOG_GAME_PK {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PK) == 33, "PROTO_NC_LOG_GAME_PK size drift");

struct PROTO_NC_LOG_GAME_PKED {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PKED) == 33, "PROTO_NC_LOG_GAME_PKED size drift");

struct PROTO_NC_LOG_GAME_PRISON { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_PRISON) == 7, "PROTO_NC_LOG_GAME_PRISON size drift");

struct PROTO_NC_LOG_GAME_PRISON_RELEASE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_PRISON_RELEASE) == 24, "PROTO_NC_LOG_GAME_PRISON_RELEASE size drift");

struct PROTO_NC_LOG_GAME_QUEST_COMPLETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_COMPLETE) == 26, "PROTO_NC_LOG_GAME_QUEST_COMPLETE size drift");

struct PROTO_NC_LOG_GAME_QUEST_DELETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_DELETE) == 26, "PROTO_NC_LOG_GAME_QUEST_DELETE size drift");

struct PROTO_NC_LOG_GAME_QUEST_GET {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_GET) == 26, "PROTO_NC_LOG_GAME_QUEST_GET size drift");

struct PROTO_NC_LOG_GAME_QUEST_ITEM_GET {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _pad_at_0004[24];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_ITEM_GET) == 38, "PROTO_NC_LOG_GAME_QUEST_ITEM_GET size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_ABSTATE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_ABSTATE) == 8, "PROTO_NC_LOG_GAME_QUEST_REWARD_ABSTATE size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_EXP { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_EXP) == 22, "PROTO_NC_LOG_GAME_QUEST_REWARD_EXP size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_FAME { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_FAME) == 14, "PROTO_NC_LOG_GAME_QUEST_REWARD_FAME size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_ITEM {
    uint8_t _pad_at_0000[6];
    SHINE_ITEM_REGISTNUMBER itemkey;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_ITEM) == 16, "PROTO_NC_LOG_GAME_QUEST_REWARD_ITEM size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_MINIHOUSE { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_MINIHOUSE) == 7, "PROTO_NC_LOG_GAME_QUEST_REWARD_MINIHOUSE size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_MONEY { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_MONEY) == 22, "PROTO_NC_LOG_GAME_QUEST_REWARD_MONEY size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_PET { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_PET) == 10, "PROTO_NC_LOG_GAME_QUEST_REWARD_PET size drift");

struct PROTO_NC_LOG_GAME_QUEST_REWARD_TITLE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_REWARD_TITLE) == 8, "PROTO_NC_LOG_GAME_QUEST_REWARD_TITLE size drift");

struct PROTO_NC_LOG_GAME_QUEST_SET_INFO {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_QUEST_SET_INFO) == 27, "PROTO_NC_LOG_GAME_QUEST_SET_INFO size drift");

struct PROTO_NC_LOG_GAME_SEAWAR { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_LOG_GAME_SEAWAR) == 1, "PROTO_NC_LOG_GAME_SEAWAR size drift");

struct PROTO_NC_LOG_GAME_SKILL_DELETE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_SKILL_DELETE) == 27, "PROTO_NC_LOG_GAME_SKILL_DELETE size drift");

struct PROTO_NC_LOG_GAME_SKILL_LEARN {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_SKILL_LEARN) == 27, "PROTO_NC_LOG_GAME_SKILL_LEARN size drift");

struct PROTO_NC_LOG_GAME_SKILL_USE {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[27];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_SKILL_USE) == 31, "PROTO_NC_LOG_GAME_SKILL_USE size drift");

struct PROTO_NC_LOG_GAME_STATE_CLEAR {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_STATE_CLEAR) == 26, "PROTO_NC_LOG_GAME_STATE_CLEAR size drift");

struct PROTO_NC_LOG_GAME_STATE_SET {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[28];
};
static_assert(sizeof(PROTO_NC_LOG_GAME_STATE_SET) == 32, "PROTO_NC_LOG_GAME_STATE_SET size drift");

struct PROTO_NC_LOG_GMAE_LOGOUT_ZONEINFO {
    uint8_t _pad_at_0000[4];
    MAPPOS mappos;
    uint8_t _tail[37];
};
static_assert(sizeof(PROTO_NC_LOG_GMAE_LOGOUT_ZONEINFO) == 41, "PROTO_NC_LOG_GMAE_LOGOUT_ZONEINFO size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_CHAT_BAN { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_CHAT_BAN) == 12, "PROTO_NC_LOG_GUILD_ACADEMY_CHAT_BAN size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_ITEM { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_ITEM) == 8, "PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_ITEM size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_MONEY { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_MONEY) == 8, "PROTO_NC_LOG_GUILD_ACADEMY_CLEAR_REWARD_MONEY size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_GRADUATE { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_GRADUATE) == 10, "PROTO_NC_LOG_GUILD_ACADEMY_GRADUATE size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_GUILD_INVITE { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_GUILD_INVITE) == 12, "PROTO_NC_LOG_GUILD_ACADEMY_GUILD_INVITE size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_JOIN { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_JOIN) == 8, "PROTO_NC_LOG_GUILD_ACADEMY_JOIN size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_LEAVE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_LEAVE) == 8, "PROTO_NC_LOG_GUILD_ACADEMY_LEAVE size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_LEVEL_UP { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_LEVEL_UP) == 17, "PROTO_NC_LOG_GUILD_ACADEMY_LEVEL_UP size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_MASTER_TELEPORT {
    uint8_t _pad_at_0000[12];
    MAPPOS ToMapPosition;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_MASTER_TELEPORT) == 32, "PROTO_NC_LOG_GUILD_ACADEMY_MASTER_TELEPORT size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM {
    uint8_t _pad_at_0000[9];
    ITEM_INVEN ItemInven;
    uint8_t _pad_at_0009[4];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM) == 25, "PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM_PAY {
    uint8_t _pad_at_0000[11];
    ITEM_INVEN ItemInven;
    uint8_t _pad_at_000b[2];
    SHINE_ITEM_REGISTNUMBER nItemKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM_PAY) == 25, "PROTO_NC_LOG_GUILD_ACADEMY_REWARD_ITEM_PAY size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_REWARD_MONEY { uint8_t data[25]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_REWARD_MONEY) == 25, "PROTO_NC_LOG_GUILD_ACADEMY_REWARD_MONEY size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_SET_MASTER { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_SET_MASTER) == 12, "PROTO_NC_LOG_GUILD_ACADEMY_SET_MASTER size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_ITEM { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_ITEM) == 13, "PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_ITEM size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_MONEY { uint8_t data[17]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_MONEY) == 17, "PROTO_NC_LOG_GUILD_ACADEMY_SET_REWARD_MONEY size drift");

struct PROTO_NC_LOG_GUILD_ACADEMY_VANISH { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_LOG_GUILD_ACADEMY_VANISH) == 12, "PROTO_NC_LOG_GUILD_ACADEMY_VANISH size drift");

struct PROTO_NC_LOG_ITEMMONEY_BUY {
    uint8_t _pad_at_0000[8];
    SHINE_ITEM_REGISTNUMBER nItemkey;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_LOG_ITEMMONEY_BUY) == 26, "PROTO_NC_LOG_ITEMMONEY_BUY size drift");

struct PROTO_NC_LOG_MOVER_RAREMOVER {
    uint8_t _pad_at_0000[11];
    SHINE_ITEM_REGISTNUMBER nRare_Key;
    uint8_t _pad_at_000b[12];
    SHINE_ITEM_REGISTNUMBER nConsum_Key;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_LOG_MOVER_RAREMOVER) == 31, "PROTO_NC_LOG_MOVER_RAREMOVER size drift");

struct PROTO_NC_LOG_MOVER_UPGRADE {
    uint8_t _pad_at_0000[11];
    SHINE_ITEM_REGISTNUMBER nMain_Key;
    uint8_t _pad_at_000b[11];
    SHINE_ITEM_REGISTNUMBER nConsum_Key;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_LOG_MOVER_UPGRADE) == 30, "PROTO_NC_LOG_MOVER_UPGRADE size drift");

struct PROTO_NC_LOG_RANDOMOPTION_CHANGE { uint8_t data[36]; };
static_assert(sizeof(PROTO_NC_LOG_RANDOMOPTION_CHANGE) == 36, "PROTO_NC_LOG_RANDOMOPTION_CHANGE size drift");

struct PROTO_NC_LOG_RANDOMOPTION_CHANGE_BEFORE {
    uint8_t _pad_at_0000[8];
    Name3 sMap;
    uint8_t _tail[60];
};
static_assert(sizeof(PROTO_NC_LOG_RANDOMOPTION_CHANGE_BEFORE) == 68, "PROTO_NC_LOG_RANDOMOPTION_CHANGE_BEFORE size drift");

struct PROTO_NC_LOG_REGENLOCATESAVE_CMD {
    uint8_t _pad_at_0000[4];
    Name3 SaveMap;
    uint8_t _pad_at_0004[12];
    SHINE_XY_TYPE SaveLocate;
    uint8_t _pad_at_0010[12];
    SHINE_XY_TYPE InvalidLocate;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_LOG_REGENLOCATESAVE_CMD) == 38, "PROTO_NC_LOG_REGENLOCATESAVE_CMD size drift");

struct PROTO_NC_LOG_USER_LOGIN {
    uint8_t _pad_at_0000[5];
    uint8_t ip[4];
};
static_assert(sizeof(PROTO_NC_LOG_USER_LOGIN) == 9, "PROTO_NC_LOG_USER_LOGIN size drift");

struct PROTO_NC_LOG_USER_LOGINFAIL {
    Name256Byte userid;
    uint8_t _pad_at_0000[256];
    Name4 userpassword;
    uint8_t _pad_at_0100[16];
    uint8_t userip[4];
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_LOG_USER_LOGINFAIL) == 278, "PROTO_NC_LOG_USER_LOGINFAIL size drift");

struct PROTO_NC_LOG_USER_LOGINFAIL_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_LOG_USER_LOGINFAIL log;
};
static_assert(sizeof(PROTO_NC_LOG_USER_LOGINFAIL_SEND) == 281, "PROTO_NC_LOG_USER_LOGINFAIL_SEND size drift");

struct PROTO_NC_LOG_USER_LOGIN_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_LOG_USER_LOGIN log;
};
static_assert(sizeof(PROTO_NC_LOG_USER_LOGIN_SEND) == 12, "PROTO_NC_LOG_USER_LOGIN_SEND size drift");

struct PROTO_NC_LOG_USER_LOGOUT { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_LOG_USER_LOGOUT) == 9, "PROTO_NC_LOG_USER_LOGOUT size drift");

struct PROTO_NC_LOG_USER_LOGOUT_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_LOG_USER_LOGOUT log;
};
static_assert(sizeof(PROTO_NC_LOG_USER_LOGOUT_SEND) == 12, "PROTO_NC_LOG_USER_LOGOUT_SEND size drift");

struct PROTO_NC_LOG_WEDDING_DIVORCE_CANCEL { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_DIVORCE_CANCEL) == 8, "PROTO_NC_LOG_WEDDING_DIVORCE_CANCEL size drift");

struct PROTO_NC_LOG_WEDDING_DIVORCE_EXE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_DIVORCE_EXE) == 8, "PROTO_NC_LOG_WEDDING_DIVORCE_EXE size drift");

struct PROTO_NC_LOG_WEDDING_DIVORCE_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_DIVORCE_REQ) == 9, "PROTO_NC_LOG_WEDDING_DIVORCE_REQ size drift");

struct PROTO_NC_LOG_WEDDING_HALL_CANCEL { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_HALL_CANCEL) == 8, "PROTO_NC_LOG_WEDDING_HALL_CANCEL size drift");

struct PROTO_NC_LOG_WEDDING_HALL_RESERVE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_HALL_RESERVE) == 8, "PROTO_NC_LOG_WEDDING_HALL_RESERVE size drift");

struct PROTO_NC_LOG_WEDDING_HALL_START { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_HALL_START) == 8, "PROTO_NC_LOG_WEDDING_HALL_START size drift");

struct PROTO_NC_LOG_WEDDING_PROPOSE_CANCEL { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_PROPOSE_CANCEL) == 8, "PROTO_NC_LOG_WEDDING_PROPOSE_CANCEL size drift");

struct PROTO_NC_LOG_WEDDING_PROPOSE_EXE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_PROPOSE_EXE) == 8, "PROTO_NC_LOG_WEDDING_PROPOSE_EXE size drift");

struct PROTO_NC_LOG_WEDDING_WEDDING_EXE { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_LOG_WEDDING_WEDDING_EXE) == 8, "PROTO_NC_LOG_WEDDING_WEDDING_EXE size drift");

struct PROTO_NC_MAP_CAN_USE_REVIVEITEM_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_CAN_USE_REVIVEITEM_CMD) == 1, "PROTO_NC_MAP_CAN_USE_REVIVEITEM_CMD size drift");

struct PROTO_NC_MAP_EXPBONUS_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_MAP_EXPBONUS_RNG) == 9, "PROTO_NC_MAP_EXPBONUS_RNG size drift");

struct PROTO_NC_MAP_FIELD_ATTRIBUTE_CMD {
    FIELD_MAP_TYPE eFieldMapType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_MAP_FIELD_ATTRIBUTE_CMD) == 4, "PROTO_NC_MAP_FIELD_ATTRIBUTE_CMD size drift");

struct PROTO_NC_MAP_INDUN_LEVEL_VIEW_CMD {
    ID_LEVEL_TYPE eLevelType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_MAP_INDUN_LEVEL_VIEW_CMD) == 4, "PROTO_NC_MAP_INDUN_LEVEL_VIEW_CMD size drift");

struct PROTO_NC_MAP_ITEMBONUS_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_MAP_ITEMBONUS_RNG) == 9, "PROTO_NC_MAP_ITEMBONUS_RNG size drift");

struct PROTO_NC_MAP_LINKALLOW_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_LINKALLOW_ACK) == 1, "PROTO_NC_MAP_LINKALLOW_ACK size drift");

struct PROTO_NC_MAP_LINKEND_CLIENT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_LINKEND_CLIENT_CMD) == 1, "PROTO_NC_MAP_LINKEND_CLIENT_CMD size drift");

struct PROTO_NC_MAP_LINKEND_CMD {
    uint8_t _pad_at_0000[8];
    Name3 map;
    uint8_t _pad_at_0008[13];
    SHINE_XY_TYPE location;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_MAP_LINKEND_CMD) == 47, "PROTO_NC_MAP_LINKEND_CMD size drift");

struct PROTO_NC_MAP_LINKRESERVE_ACK {
    NETPACKETZONEHEADER header;
    PROTO_NC_CHAR_REVIVEOTHER_CMD linkother;
};
static_assert(sizeof(PROTO_NC_MAP_LINKRESERVE_ACK) == 36, "PROTO_NC_MAP_LINKRESERVE_ACK size drift");

struct PROTO_NC_MAP_LINKRESERVE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_MAP_LINKRESERVE_ACK ack;
};
static_assert(sizeof(PROTO_NC_MAP_LINKRESERVE_ACK_SEND) == 39, "PROTO_NC_MAP_LINKRESERVE_ACK_SEND size drift");

struct PROTO_NC_MAP_LINKRESERVE_REQ___unnamed_type_linkto_ {
    Name3 mapname;
    uint8_t _pad_at_0000[12];
    SHINE_XY_TYPE location;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_MAP_LINKRESERVE_REQ___unnamed_type_linkto_) == 20, "PROTO_NC_MAP_LINKRESERVE_REQ___unnamed_type_linkto_ size drift");

struct PROTO_NC_MAP_LINKRESERVE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[3];
    PROTO_NC_MAP_LINKRESERVE_REQ___unnamed_type_linkto_ linkto;
    PROTO_NC_CHAR_REVIVEOTHER_CMD linkother;
};
static_assert(sizeof(PROTO_NC_MAP_LINKRESERVE_REQ) == 59, "PROTO_NC_MAP_LINKRESERVE_REQ size drift");

struct PROTO_NC_MAP_LINKSTART_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MAP_LINKSTART_CMD) == 6, "PROTO_NC_MAP_LINKSTART_CMD size drift");

struct PROTO_NC_MAP_LINKTO_REQ {
    uint8_t _pad_at_0000[1];
    PROTO_NC_CHAR_REVIVESAME_CMD_______0_bytes___ link;
};
static_assert(sizeof(PROTO_NC_MAP_LINKTO_REQ) == 1, "PROTO_NC_MAP_LINKTO_REQ size drift");

struct PROTO_NC_MAP_LINK_FAIL { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_LINK_FAIL) == 2, "PROTO_NC_MAP_LINK_FAIL size drift");

struct PROTO_NC_MAP_LOGINCOMPLETE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_LOGINCOMPLETE_CMD) == 1, "PROTO_NC_MAP_LOGINCOMPLETE_CMD size drift");

struct PROTO_NC_MAP_LOGINFAIL_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MAP_LOGINFAIL_ACK) == 3, "PROTO_NC_MAP_LOGINFAIL_ACK size drift");

struct PROTO_NC_MAP_LOGIN_REQ {
    PROTO_NC_CHAR_ZONE_CHARDATA_REQ chardata;
    Name8_______1600_bytes___ checksum;
    uint8_t _tail[1600];
};
static_assert(sizeof(PROTO_NC_MAP_LOGIN_REQ) == 1622, "PROTO_NC_MAP_LOGIN_REQ size drift");

struct PROTO_NC_MAP_LOGOUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_LOGOUT_CMD) == 2, "PROTO_NC_MAP_LOGOUT_CMD size drift");

struct PROTO_NC_MAP_MULTY_LINK_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE npcPosition;
    uint8_t _pad_at_0002[11];
    Name3_______60_bytes___ LinkMapName;
    uint8_t _tail[60];
};
static_assert(sizeof(PROTO_NC_MAP_MULTY_LINK_CMD) == 73, "PROTO_NC_MAP_MULTY_LINK_CMD size drift");

struct PROTO_NC_MAP_MULTY_LINK_SELECT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_MULTY_LINK_SELECT_ACK) == 2, "PROTO_NC_MAP_MULTY_LINK_SELECT_ACK size drift");

struct PROTO_NC_MAP_MULTY_LINK_SELECT_REQ {
    Name3 LinkMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_MAP_MULTY_LINK_SELECT_REQ) == 12, "PROTO_NC_MAP_MULTY_LINK_SELECT_REQ size drift");

struct PROTO_NC_MAP_REGIST_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_MAP_REGIST_CMD___unnamed_type_maparray________0_bytes___ maparray;
};
static_assert(sizeof(PROTO_NC_MAP_REGIST_CMD) == 2, "PROTO_NC_MAP_REGIST_CMD size drift");

struct PROTO_NC_MAP_REGIST_CMD___unnamed_type_maparray_ {
    uint8_t _pad_at_0000[1];
    Name3 mapname;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_MAP_REGIST_CMD___unnamed_type_maparray_) == 14, "PROTO_NC_MAP_REGIST_CMD___unnamed_type_maparray_ size drift");

struct PROTO_NC_MAP_TOWNPORTAL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_TOWNPORTAL_ACK) == 2, "PROTO_NC_MAP_TOWNPORTAL_ACK size drift");

struct PROTO_NC_MAP_TOWNPORTAL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_TOWNPORTAL_REQ) == 1, "PROTO_NC_MAP_TOWNPORTAL_REQ size drift");

struct PROTO_NC_MAP_WING_FLY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_WING_FLY_ACK) == 2, "PROTO_NC_MAP_WING_FLY_ACK size drift");

struct PROTO_NC_MAP_WING_FLY_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_WING_FLY_REQ) == 1, "PROTO_NC_MAP_WING_FLY_REQ size drift");

struct PROTO_NC_MAP_WING_SAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MAP_WING_SAVE_ACK) == 2, "PROTO_NC_MAP_WING_SAVE_ACK size drift");

struct PROTO_NC_MAP_WING_SAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MAP_WING_SAVE_REQ) == 1, "PROTO_NC_MAP_WING_SAVE_REQ size drift");

struct PROTO_NC_MENU_INDUNRANK_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MENU_INDUNRANK_CMD) == 1, "PROTO_NC_MENU_INDUNRANK_CMD size drift");

struct PROTO_NC_MENU_OPENSTORAGE_CMD {
    uint8_t _pad_at_0000[12];
    PROTO_ITEMPACKET_INFORM_______0_bytes___ itemarray;
};
static_assert(sizeof(PROTO_NC_MENU_OPENSTORAGE_CMD) == 12, "PROTO_NC_MENU_OPENSTORAGE_CMD size drift");

struct PROTO_NC_MENU_OPENSTORAGE_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MENU_OPENSTORAGE_FAIL_CMD) == 2, "PROTO_NC_MENU_OPENSTORAGE_FAIL_CMD size drift");

struct PROTO_NC_MENU_SERVERMENU_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MENU_SERVERMENU_ACK) == 1, "PROTO_NC_MENU_SERVERMENU_ACK size drift");

struct PROTO_NC_MENU_SERVERMENU_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MENU_SERVERMENU_CLOSE_CMD) == 1, "PROTO_NC_MENU_SERVERMENU_CLOSE_CMD size drift");

struct PROTO_NC_MENU_SERVERMENU_REQ {
    uint8_t title[128];
    uint8_t _pad_at_0080[3];
    SHINE_XY_TYPE npcPosition;
    uint8_t _pad_at_0083[11];
    SERVERMENU_______0_bytes___ menu;
};
static_assert(sizeof(PROTO_NC_MENU_SERVERMENU_REQ) == 142, "PROTO_NC_MENU_SERVERMENU_REQ size drift");

struct PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD__SOULSTONEMENU { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD__SOULSTONEMENU) == 12, "PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD__SOULSTONEMENU size drift");

struct PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD {
    PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD__SOULSTONEMENU hp;
    PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD__SOULSTONEMENU sp;
};
static_assert(sizeof(PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD) == 24, "PROTO_NC_MENU_SHOPOPENSOULSTONE_CMD size drift");

struct PROTO_NC_MENU_SHOPOPENTABLE_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_MENU_SHOPOPENTABLE_CMD__MENUITEM_______0_bytes___ itemlist;
};
static_assert(sizeof(PROTO_NC_MENU_SHOPOPENTABLE_CMD) == 4, "PROTO_NC_MENU_SHOPOPENTABLE_CMD size drift");

struct PROTO_NC_MENU_SHOPOPENTABLE_CMD__MENUITEM { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MENU_SHOPOPENTABLE_CMD__MENUITEM) == 3, "PROTO_NC_MENU_SHOPOPENTABLE_CMD__MENUITEM size drift");

struct PROTO_NC_MENU_SHOPOPEN_CMD {
    uint8_t _pad_at_0000[4];
    PROTO_NC_MENU_SHOPOPEN_CMD__MENUITEM_______0_bytes___ itemlist;
};
static_assert(sizeof(PROTO_NC_MENU_SHOPOPEN_CMD) == 4, "PROTO_NC_MENU_SHOPOPEN_CMD size drift");

struct PROTO_NC_MENU_SHOPOPEN_CMD__MENUITEM { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MENU_SHOPOPEN_CMD__MENUITEM) == 2, "PROTO_NC_MENU_SHOPOPEN_CMD__MENUITEM size drift");

struct PROTO_NC_MINIHOUSE_ACTIV_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_ACTIV_REQ) == 1, "PROTO_NC_MINIHOUSE_ACTIV_REQ size drift");

struct PROTO_NC_MINIHOUSE_ARRANGEMODE_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_ARRANGEMODE_ACK) == 4, "PROTO_NC_MINIHOUSE_ARRANGEMODE_ACK size drift");

struct PROTO_NC_MINIHOUSE_ARRANGEMODE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_ARRANGEMODE_REQ) == 1, "PROTO_NC_MINIHOUSE_ARRANGEMODE_REQ size drift");

struct PROTO_NC_MINIHOUSE_BUILDING_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_BUILDING_ACK) == 2, "PROTO_NC_MINIHOUSE_BUILDING_ACK size drift");

struct PROTO_NC_MINIHOUSE_BUILDING_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_BRIEFINFO_MINIHOUSEBUILD_CMD minihouseinfo;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_BUILDING_CMD) == 66, "PROTO_NC_MINIHOUSE_BUILDING_CMD size drift");

struct PROTO_NC_MINIHOUSE_BUILDING_REQ {
    uint8_t _pad_at_0000[1];
    wchar_t password[9];
    wchar_t title[21];
    wchar_t sNotify[101];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_BUILDING_REQ) == 133, "PROTO_NC_MINIHOUSE_BUILDING_REQ size drift");

struct PROTO_NC_MINIHOUSE_CHAR_ACTION_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_CHAR_ACTION_ACK) == 2, "PROTO_NC_MINIHOUSE_CHAR_ACTION_ACK size drift");

struct PROTO_NC_MINIHOUSE_CHAR_ACTION_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_CHAR_ACTION_CMD) == 10, "PROTO_NC_MINIHOUSE_CHAR_ACTION_CMD size drift");

struct PROTO_NC_MINIHOUSE_CHAR_ACTION_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_CHAR_ACTION_REQ) == 4, "PROTO_NC_MINIHOUSE_CHAR_ACTION_REQ size drift");

struct PROTO_NC_MINIHOUSE_COMPULSIONMOVETO_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE location;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_COMPULSIONMOVETO_REQ) == 10, "PROTO_NC_MINIHOUSE_COMPULSIONMOVETO_REQ size drift");

struct PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD___unnamed_type_rearrange________0_bytes___ rearrange;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD) == 1, "PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD size drift");

struct PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD___unnamed_type_rearrange_ {
    uint8_t _pad_at_0000[4];
    SHINE_SPACE_TYPE location;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD___unnamed_type_rearrange_) == 21, "PROTO_NC_MINIHOUSE_CREATE_FURNITURE_CMD___unnamed_type_rearrange_ size drift");

struct PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ {
    uint8_t _pad_at_0000[1];
    PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ___unnamed_type_rearrange________0_bytes___ rearrange;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ) == 1, "PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ size drift");

struct PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ___unnamed_type_rearrange_ {
    uint8_t _pad_at_0000[2];
    SHINE_SPACE_TYPE location;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ___unnamed_type_rearrange_) == 18, "PROTO_NC_MINIHOUSE_CREATE_FURNITURE_REQ___unnamed_type_rearrange_ size drift");

struct PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[7];
    uint8_t blogaddr[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_ACK) == 13, "PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_REQ) == 10, "PROTO_NC_MINIHOUSE_DB_OWNERBLOG_GET_REQ size drift");

struct PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[7];
    uint8_t blogaddr[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_ACK) == 13, "PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[5];
    uint8_t blogaddr[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_REQ) == 11, "PROTO_NC_MINIHOUSE_DB_OWNERBLOG_SET_REQ size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[11];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_ACK) == 17, "PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[9];
    PROTO_ITEM_RELOC Item;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_REQ) == 25, "PROTO_NC_MINIHOUSE_DB_PORTAL_ADD_REQ size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_ACK) == 8, "PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_REQ) == 8, "PROTO_NC_MINIHOUSE_DB_PORTAL_DEL_REQ size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _pad_at_0006[4];
    PORTAL_INFO_______0_bytes___ PortalInfo;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_ACK) == 10, "PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_REQ {
    NETPACKETZONEHEADER header;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_REQ) == 6, "PROTO_NC_MINIHOUSE_DB_PORTAL_LIST_REQ size drift");

struct PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_ACK) == 8, "PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_ACK size drift");

struct PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_REQ) == 10, "PROTO_NC_MINIHOUSE_DB_VISITER_COUNT_REQ size drift");

struct PROTO_NC_MINIHOUSE_DELETE_FURNITURE_REQ {
    uint8_t _pad_at_0000[1];
    uint16_t furniturehandle[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_DELETE_FURNITURE_REQ) == 1, "PROTO_NC_MINIHOUSE_DELETE_FURNITURE_REQ size drift");

struct PROTO_NC_MINIHOUSE_FUNICHER_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_MINIHOUSE_FUNICHER_CMD___unnamed_type_array________0_bytes___ array;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FUNICHER_CMD) == 2, "PROTO_NC_MINIHOUSE_FUNICHER_CMD size drift");

struct PROTO_NC_MINIHOUSE_FUNICHER_CMD___unnamed_type_array_ {
    uint8_t _pad_at_0000[4];
    SHINE_SPACE_TYPE location;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FUNICHER_CMD___unnamed_type_array_) == 21, "PROTO_NC_MINIHOUSE_FUNICHER_CMD___unnamed_type_array_ size drift");

struct PROTO_NC_MINIHOUSE_FUNITUREINFOCOMPLETE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FUNITUREINFOCOMPLETE_REQ) == 1, "PROTO_NC_MINIHOUSE_FUNITUREINFOCOMPLETE_REQ size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_ACK) == 4, "PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_ACK size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_CMD {
    uint8_t _pad_at_0000[4];
    uint16_t nApplyPlayerHandle[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_CMD) == 4, "PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_CMD size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_REQ) == 4, "PROTO_NC_MINIHOUSE_FURNITURE_EFFECT_REQ size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_ACK) == 6, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_ACK size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_ACK) == 2, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_ACK size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_CMD {
    uint8_t _pad_at_0000[2];
    uint16_t nCancelEmotion[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_CMD) == 2, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_CMD size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_REQ) == 1, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CANCEL_REQ size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CMD {
    uint8_t _pad_at_0000[2];
    FURNITURE_EMOTION_INFO_______0_bytes___ Emotion;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CMD) == 2, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_CMD size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_REQ) == 4, "PROTO_NC_MINIHOUSE_FURNITURE_EMOTION_REQ size drift");

struct PROTO_NC_MINIHOUSE_FURNITURE_ENDURE_CMD {
    uint8_t _pad_at_0000[1];
    ENDURE_FURNITURE_INFO_______0_bytes___ EndureFurniture;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_FURNITURE_ENDURE_CMD) == 1, "PROTO_NC_MINIHOUSE_FURNITURE_ENDURE_CMD size drift");

struct PROTO_NC_MINIHOUSE_KICKOUTCANCEL_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_KICKOUTCANCEL_REQ) == 2, "PROTO_NC_MINIHOUSE_KICKOUTCANCEL_REQ size drift");

struct PROTO_NC_MINIHOUSE_KICKOUT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_KICKOUT_CMD) == 1, "PROTO_NC_MINIHOUSE_KICKOUT_CMD size drift");

struct PROTO_NC_MINIHOUSE_KICKOUT_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_KICKOUT_REQ) == 2, "PROTO_NC_MINIHOUSE_KICKOUT_REQ size drift");

struct PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK {
    uint8_t _pad_at_0000[6];
    wchar_t sNotify[101];
    wchar_t TargetKey[32];
    PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK___unnamed_type_player________0_bytes___ player;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK) == 140, "PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK size drift");

struct PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK___unnamed_type_player_ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK___unnamed_type_player_) == 3, "PROTO_NC_MINIHOUSE_LOGINCOMPLETE_ACK___unnamed_type_player_ size drift");

struct PROTO_NC_MINIHOUSE_LOGINCOMPLETE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_LOGINCOMPLETE_CMD) == 3, "PROTO_NC_MINIHOUSE_LOGINCOMPLETE_CMD size drift");

struct PROTO_NC_MINIHOUSE_LOGOUTCOMPLETE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_LOGOUTCOMPLETE_CMD) == 2, "PROTO_NC_MINIHOUSE_LOGOUTCOMPLETE_CMD size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_ACK) == 2, "PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_ACK size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_CMD) == 1, "PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_CMD size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_REQ) == 1, "PROTO_NC_MINIHOUSE_MODIFY_ITEM_INFO_OPEN_REQ size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_MAXENTERNUM_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_MAXENTERNUM_REQ) == 1, "PROTO_NC_MINIHOUSE_MODIFY_MAXENTERNUM_REQ size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_ACK) == 2, "PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_ACK size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_CMD {
    uint8_t _pad_at_0000[1];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_CMD) == 1, "PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_CMD size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_REQ {
    uint8_t _pad_at_0000[1];
    wchar_t sNotify[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_REQ) == 1, "PROTO_NC_MINIHOUSE_MODIFY_NOTIFY_REQ size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_OUTSIDE_TITLE_CMD {
    uint8_t _pad_at_0000[2];
    wchar_t title[21];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_OUTSIDE_TITLE_CMD) == 23, "PROTO_NC_MINIHOUSE_MODIFY_OUTSIDE_TITLE_CMD size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_OWNERBLOG_REQ {
    uint8_t _pad_at_0000[1];
    wchar_t blogaddr[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_OWNERBLOG_REQ) == 1, "PROTO_NC_MINIHOUSE_MODIFY_OWNERBLOG_REQ size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_CMD) == 1, "PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_CMD size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_REQ {
    wchar_t password[9];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_REQ) == 9, "PROTO_NC_MINIHOUSE_MODIFY_PASSWORD_REQ size drift");

struct PROTO_NC_MINIHOUSE_MODIFY_TITLE_REQ {
    wchar_t title[21];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_MODIFY_TITLE_REQ) == 21, "PROTO_NC_MINIHOUSE_MODIFY_TITLE_REQ size drift");

struct PROTO_NC_MINIHOUSE_OWNERBLOG_ACK {
    uint8_t _pad_at_0000[5];
    wchar_t blogaddr[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_OWNERBLOG_ACK) == 5, "PROTO_NC_MINIHOUSE_OWNERBLOG_ACK size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_ADD_CMD {
    PORTAL_INFO NewPortal;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_ADD_CMD) == 6, "PROTO_NC_MINIHOUSE_PORTAL_ADD_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_CLOSE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_CLOSE_ACK) == 2, "PROTO_NC_MINIHOUSE_PORTAL_CLOSE_ACK size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_CLOSE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_CLOSE_CMD) == 1, "PROTO_NC_MINIHOUSE_PORTAL_CLOSE_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_CLOSE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_CLOSE_REQ) == 1, "PROTO_NC_MINIHOUSE_PORTAL_CLOSE_REQ size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_DEL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_DEL_ACK) == 2, "PROTO_NC_MINIHOUSE_PORTAL_DEL_ACK size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_DEL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_DEL_CMD) == 2, "PROTO_NC_MINIHOUSE_PORTAL_DEL_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_DEL_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_DEL_REQ) == 2, "PROTO_NC_MINIHOUSE_PORTAL_DEL_REQ size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_EFFECT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_EFFECT_ACK) == 2, "PROTO_NC_MINIHOUSE_PORTAL_EFFECT_ACK size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_EFFECT_CMD {
    uint8_t _pad_at_0000[4];
    uint16_t nApplyPlayerHandle[0];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_EFFECT_CMD) == 4, "PROTO_NC_MINIHOUSE_PORTAL_EFFECT_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_EFFECT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_EFFECT_REQ) == 1, "PROTO_NC_MINIHOUSE_PORTAL_EFFECT_REQ size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_LIST_CMD {
    uint8_t _pad_at_0000[2];
    PORTAL_INFO_______0_bytes___ PortalInfo;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_LIST_CMD) == 2, "PROTO_NC_MINIHOUSE_PORTAL_LIST_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_OPEN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_OPEN_ACK) == 2, "PROTO_NC_MINIHOUSE_PORTAL_OPEN_ACK size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_OPEN_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_SPACE_TYPE Location;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_OPEN_CMD) == 18, "PROTO_NC_MINIHOUSE_PORTAL_OPEN_CMD size drift");

struct PROTO_NC_MINIHOUSE_PORTAL_OPEN_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_PORTAL_OPEN_REQ) == 2, "PROTO_NC_MINIHOUSE_PORTAL_OPEN_REQ size drift");

struct PROTO_NC_MINIHOUSE_REARRANGE_REQ {
    uint8_t _pad_at_0000[1];
    PROTO_NC_MINIHOUSE_REARRANGE_REQ___unnamed_type_rearrange________0_bytes___ rearrange;
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_REARRANGE_REQ) == 1, "PROTO_NC_MINIHOUSE_REARRANGE_REQ size drift");

struct PROTO_NC_MINIHOUSE_REARRANGE_REQ___unnamed_type_rearrange_ {
    uint8_t _pad_at_0000[2];
    SHINE_SPACE_TYPE location;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_REARRANGE_REQ___unnamed_type_rearrange_) == 18, "PROTO_NC_MINIHOUSE_REARRANGE_REQ___unnamed_type_rearrange_ size drift");

struct PROTO_NC_MINIHOUSE_VISITREADY_ACK { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_VISITREADY_ACK) == 7, "PROTO_NC_MINIHOUSE_VISITREADY_ACK size drift");

struct PROTO_NC_MINIHOUSE_VISITREADY_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MINIHOUSE_VISITREADY_REQ) == 2, "PROTO_NC_MINIHOUSE_VISITREADY_REQ size drift");

struct PROTO_NC_MINIHOUSE_VISIT_REQ {
    uint8_t _pad_at_0000[2];
    wchar_t password[9];
};
static_assert(sizeof(PROTO_NC_MINIHOUSE_VISIT_REQ) == 11, "PROTO_NC_MINIHOUSE_VISIT_REQ size drift");

struct PROTO_NC_MISC_APEX_CLIENT_CHCSTART_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MISC_APEX_CLIENT_CHCSTART_CMD) == 4, "PROTO_NC_MISC_APEX_CLIENT_CHCSTART_CMD size drift");

struct PROTO_NC_MISC_APEX_CLIENT_DATA_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t Buff[0];
};
static_assert(sizeof(PROTO_NC_MISC_APEX_CLIENT_DATA_CMD) == 2, "PROTO_NC_MISC_APEX_CLIENT_DATA_CMD size drift");

struct PROTO_NC_MISC_APEX_SERVER_DATA_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t Buff[0];
};
static_assert(sizeof(PROTO_NC_MISC_APEX_SERVER_DATA_CMD) == 2, "PROTO_NC_MISC_APEX_SERVER_DATA_CMD size drift");

struct PROTO_NC_MISC_CHAR_LOGOFF_STATISTICS { uint8_t data[21]; };
static_assert(sizeof(PROTO_NC_MISC_CHAR_LOGOFF_STATISTICS) == 21, "PROTO_NC_MISC_CHAR_LOGOFF_STATISTICS size drift");

struct PROTO_NC_MISC_CHAT_BLOCK_SPAMER_BLOCKTIME_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MISC_CHAT_BLOCK_SPAMER_BLOCKTIME_CMD) == 4, "PROTO_NC_MISC_CHAT_BLOCK_SPAMER_BLOCKTIME_CMD size drift");

struct PROTO_NC_MISC_CLIENT_DEBUG_MSG_CMD {
    uint8_t _pad_at_0000[4];
    uint8_t msg[0];
};
static_assert(sizeof(PROTO_NC_MISC_CLIENT_DEBUG_MSG_CMD) == 4, "PROTO_NC_MISC_CLIENT_DEBUG_MSG_CMD size drift");

struct PROTO_NC_MISC_CLIENT_LOADING_BUG_DETECT_CMD {
    Name3 sLoadingMapName;
    uint8_t _pad_at_0000[16];
    Name3 sFromMapName;
    uint8_t _pad_at_0010[12];
    Name4 sFromZoneIP;
    uint8_t _pad_at_001c[16];
    Name4 sToZoneIP;
    uint8_t _pad_at_002c[20];
    CLBD_ErrorData_______0_bytes___ ErrorData;
};
static_assert(sizeof(PROTO_NC_MISC_CLIENT_LOADING_BUG_DETECT_CMD) == 64, "PROTO_NC_MISC_CLIENT_LOADING_BUG_DETECT_CMD size drift");

struct PROTO_NC_MISC_CONNECTER_ACK {
    uint8_t _pad_at_0000[3];
    uint16_t nEachClass[64];
};
static_assert(sizeof(PROTO_NC_MISC_CONNECTER_ACK) == 131, "PROTO_NC_MISC_CONNECTER_ACK size drift");

struct PROTO_NC_MISC_CONNECTER_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_CONNECTER_REQ) == 1, "PROTO_NC_MISC_CONNECTER_REQ size drift");

struct PROTO_NC_MISC_CONNECTFROMWHERE_ACK {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_MISC_CONNECTFROMWHERE_ACK) == 7, "PROTO_NC_MISC_CONNECTFROMWHERE_ACK size drift");

struct PROTO_NC_MISC_CONNECTFROMWHERE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_CONNECTFROMWHERE_CMD) == 1, "PROTO_NC_MISC_CONNECTFROMWHERE_CMD size drift");

struct PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK {
    uint8_t _pad_at_0000[2];
    PROTO_NC_MISC_CONNECTFROMWHERE_ACK ToZoneAck;
};
static_assert(sizeof(PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK) == 9, "PROTO_NC_MISC_CONNECTFROMWHERE_DB_ACK size drift");

struct PROTO_NC_MISC_CONNECTFROMWHERE_REQ {
    NETPACKETZONEHEADER netpacketzoneheader;
    uint8_t connectip[4];
};
static_assert(sizeof(PROTO_NC_MISC_CONNECTFROMWHERE_REQ) == 10, "PROTO_NC_MISC_CONNECTFROMWHERE_REQ size drift");

struct PROTO_NC_MISC_CONNECTFROMWHERE_DB_REQ {
    uint8_t _pad_at_0000[2];
    PROTO_NC_MISC_CONNECTFROMWHERE_REQ FromZoneReq;
};
static_assert(sizeof(PROTO_NC_MISC_CONNECTFROMWHERE_DB_REQ) == 12, "PROTO_NC_MISC_CONNECTFROMWHERE_DB_REQ size drift");

struct PROTO_NC_MISC_CS_ACK {
    uint8_t _pad_at_0000[2];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_MISC_CS_ACK) == 2, "PROTO_NC_MISC_CS_ACK size drift");

struct PROTO_NC_MISC_CS_CLOSE { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MISC_CS_CLOSE) == 4, "PROTO_NC_MISC_CS_CLOSE size drift");

struct PROTO_NC_MISC_CS_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_MISC_CS_REQ) == 2, "PROTO_NC_MISC_CS_REQ size drift");

struct PROTO_NC_MISC_DB_CLIENT_LOADING_BUG_DETECT_CMD {
    uint8_t _pad_at_0000[8];
    Name3 sLoadingMapName;
    uint8_t _pad_at_0008[16];
    Name3 sFromMapName;
    uint8_t _pad_at_0018[12];
    Name4 sFromZoneIP;
    uint8_t _pad_at_0024[16];
    Name4 sToZoneIP;
    uint8_t _pad_at_0034[20];
    CLBD_ErrorData_______0_bytes___ ErrorData;
};
static_assert(sizeof(PROTO_NC_MISC_DB_CLIENT_LOADING_BUG_DETECT_CMD) == 72, "PROTO_NC_MISC_DB_CLIENT_LOADING_BUG_DETECT_CMD size drift");

struct PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t packet[0];
};
static_assert(sizeof(PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD) == 2, "PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD size drift");

struct PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD cmd;
};
static_assert(sizeof(PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD_SEND) == 5, "PROTO_NC_MISC_DELIVER_WM_LOGIN_ACDB_CMD_SEND size drift");

struct PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t packet[0];
};
static_assert(sizeof(PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD) == 2, "PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD size drift");

struct PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD cmd;
};
static_assert(sizeof(PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD_SEND) == 5, "PROTO_NC_MISC_DELIVER_WM_LOGIN_ALDB_CMD_SEND size drift");

struct PROTO_NC_MISC_EVENTNPC_STANDEND_CLIENT_CMD {
    Name8 sMobIndex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_MISC_EVENTNPC_STANDEND_CLIENT_CMD) == 32, "PROTO_NC_MISC_EVENTNPC_STANDEND_CLIENT_CMD size drift");

struct PROTO_NC_MISC_EVENTNPC_STANDEND_ZONE_CMD {
    Name8 sMobIndex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_MISC_EVENTNPC_STANDEND_ZONE_CMD) == 32, "PROTO_NC_MISC_EVENTNPC_STANDEND_ZONE_CMD size drift");

struct PROTO_NC_MISC_EVENTNPC_STANDSTART_CLIENT_CMD {
    Name8 sMobIndex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_MISC_EVENTNPC_STANDSTART_CLIENT_CMD) == 32, "PROTO_NC_MISC_EVENTNPC_STANDSTART_CLIENT_CMD size drift");

struct PROTO_NC_MISC_EVENTNPC_STANDSTART_ZONE_CMD {
    Name8 sMobIndex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_MISC_EVENTNPC_STANDSTART_ZONE_CMD) == 32, "PROTO_NC_MISC_EVENTNPC_STANDSTART_ZONE_CMD size drift");

struct PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM { uint8_t data[15]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM) == 15, "PROTO_NC_MISC_EVENT_DONE_MUNSANG_ACC2WM size drift");

struct PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2ACC { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2ACC) == 13, "PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2ACC size drift");

struct PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2Z { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2Z) == 13, "PROTO_NC_MISC_EVENT_DONE_MUNSANG_WM2Z size drift");

struct PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2CLI { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2CLI) == 2, "PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2CLI size drift");

struct PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2WM { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2WM) == 11, "PROTO_NC_MISC_EVENT_DONE_MUNSANG_Z2WM size drift");

struct PROTO_NC_MISC_EVENT_HIT3_ADD_CASH {
    uint8_t UserID[20];
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_MISC_EVENT_HIT3_ADD_CASH) == 28, "PROTO_NC_MISC_EVENT_HIT3_ADD_CASH size drift");

struct PROTO_NC_MISC_EVENT_L20_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_L20_CMD) == 3, "PROTO_NC_MISC_EVENT_L20_CMD size drift");

struct PROTO_NC_MISC_EVENT_L20_DB_ACK { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_L20_DB_ACK) == 13, "PROTO_NC_MISC_EVENT_L20_DB_ACK size drift");

struct PROTO_NC_MISC_EVENT_L20_DB_REQ {
    uint8_t _pad_at_0000[2];
    Name256Byte sUserID;
    uint8_t _tail[270];
};
static_assert(sizeof(PROTO_NC_MISC_EVENT_L20_DB_REQ) == 272, "PROTO_NC_MISC_EVENT_L20_DB_REQ size drift");

struct PROTO_NC_MISC_EVENT_LONG_TIME_PLAY_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MISC_EVENT_LONG_TIME_PLAY_CMD) == 6, "PROTO_NC_MISC_EVENT_LONG_TIME_PLAY_CMD size drift");

struct PROTO_NC_MISC_GAMETIME_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_GAMETIME_ACK) == 3, "PROTO_NC_MISC_GAMETIME_ACK size drift");

struct PROTO_NC_MISC_GAMETIME_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_GAMETIME_REQ) == 1, "PROTO_NC_MISC_GAMETIME_REQ size drift");

struct PROTO_NC_MISC_GAMETIME_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MISC_GAMETIME_REQ_SEND) == 3, "PROTO_NC_MISC_GAMETIME_REQ_SEND size drift");

struct PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_CMD) == 3, "PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_CMD size drift");

struct PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_DB_CMD { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_DB_CMD) == 11, "PROTO_NC_MISC_GET_CHAT_BLOCK_SPAM_FILTER_DB_CMD size drift");

struct PROTO_NC_MISC_GM_CHAT_COLOR_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MISC_GM_CHAT_COLOR_REQ) == 4, "PROTO_NC_MISC_GM_CHAT_COLOR_REQ size drift");

struct PROTO_NC_MISC_HACK_SCAN_STORE_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t sDesc[0];
};
static_assert(sizeof(PROTO_NC_MISC_HACK_SCAN_STORE_CMD) == 2, "PROTO_NC_MISC_HACK_SCAN_STORE_CMD size drift");

struct PROTO_NC_MISC_HACK_SCAN_STORE_DB_CMD {
    uint8_t _pad_at_0000[9];
    uint8_t sIP[20];
    Name5 sCharID;
    uint8_t _pad_at_001d[20];
    PROTO_NC_MISC_HACK_SCAN_STORE_CMD ScanInfo;
};
static_assert(sizeof(PROTO_NC_MISC_HACK_SCAN_STORE_DB_CMD) == 51, "PROTO_NC_MISC_HACK_SCAN_STORE_DB_CMD size drift");

struct PROTO_NC_MISC_HEARTBEAT_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_HEARTBEAT_ACK) == 1, "PROTO_NC_MISC_HEARTBEAT_ACK size drift");

struct PROTO_NC_MISC_HEARTBEAT_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MISC_HEARTBEAT_ACK_SEND) == 3, "PROTO_NC_MISC_HEARTBEAT_ACK_SEND size drift");

struct PROTO_NC_MISC_HEARTBEAT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_HEARTBEAT_REQ) == 1, "PROTO_NC_MISC_HEARTBEAT_REQ size drift");

struct PROTO_NC_MISC_HEARTBEAT_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MISC_HEARTBEAT_REQ_SEND) == 3, "PROTO_NC_MISC_HEARTBEAT_REQ_SEND size drift");

struct PROTO_NC_MISC_ITEMSHOP_URL_ACK {
    uint8_t _pad_at_0000[4];
    wchar_t sURL[0];
};
static_assert(sizeof(PROTO_NC_MISC_ITEMSHOP_URL_ACK) == 4, "PROTO_NC_MISC_ITEMSHOP_URL_ACK size drift");

struct PROTO_NC_MISC_ITEMSHOP_URL_DB_ACK {
    uint8_t _pad_at_0000[8];
    wchar_t sURL[0];
};
static_assert(sizeof(PROTO_NC_MISC_ITEMSHOP_URL_DB_ACK) == 8, "PROTO_NC_MISC_ITEMSHOP_URL_DB_ACK size drift");

struct PROTO_NC_MISC_ITEMSHOP_URL_DB_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_MISC_ITEMSHOP_URL_DB_REQ) == 8, "PROTO_NC_MISC_ITEMSHOP_URL_DB_REQ size drift");

struct PROTO_NC_MISC_ITEMSHOP_URL_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_ITEMSHOP_URL_REQ) == 1, "PROTO_NC_MISC_ITEMSHOP_URL_REQ size drift");

struct PROTO_NC_MISC_PINGTEST_CLIENT_ZONE_DB {
    uint8_t _pad_at_0000[46];
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_BUYLOT_REQ buylot;
};
static_assert(sizeof(PROTO_NC_MISC_PINGTEST_CLIENT_ZONE_DB) == 124, "PROTO_NC_MISC_PINGTEST_CLIENT_ZONE_DB size drift");

struct PROTO_NC_MISC_PINGTEST_TOOL_WM_CLIENT_ZONE_DB {
    uint8_t _pad_at_0000[14];
    Name5 TargetCharName;
    uint8_t _pad_at_000e[72];
    NETPACKETZONEHEADER header;
    PROTO_NC_ITEMDB_BUYLOT_REQ buylot;
};
static_assert(sizeof(PROTO_NC_MISC_PINGTEST_TOOL_WM_CLIENT_ZONE_DB) == 164, "PROTO_NC_MISC_PINGTEST_TOOL_WM_CLIENT_ZONE_DB size drift");

struct PROTO_NC_MISC_PINGTEST_TOOL_WM_DB { uint8_t data[38]; };
static_assert(sizeof(PROTO_NC_MISC_PINGTEST_TOOL_WM_DB) == 38, "PROTO_NC_MISC_PINGTEST_TOOL_WM_DB size drift");

struct PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE { uint8_t data[34]; };
static_assert(sizeof(PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE) == 34, "PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE size drift");

struct PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE_DB { uint8_t data[58]; };
static_assert(sizeof(PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE_DB) == 58, "PROTO_NC_MISC_PINGTEST_TOOL_WM_ZONE_DB size drift");

struct PROTO_NC_MISC_RESTMINUTE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_RESTMINUTE_CMD) == 3, "PROTO_NC_MISC_RESTMINUTE_CMD size drift");

struct PROTO_NC_MISC_S2SCONNECTION_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_ACK) == 3, "PROTO_NC_MISC_S2SCONNECTION_ACK size drift");

struct PROTO_NC_MISC_S2SCONNECTION_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_MISC_S2SCONNECTION_ACK ack;
};
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_ACK_SEND) == 6, "PROTO_NC_MISC_S2SCONNECTION_ACK_SEND size drift");

struct PROTO_NC_MISC_S2SCONNECTION_RDY { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_RDY) == 1, "PROTO_NC_MISC_S2SCONNECTION_RDY size drift");

struct PROTO_NC_MISC_S2SCONNECTION_RDY_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_RDY_SEND) == 3, "PROTO_NC_MISC_S2SCONNECTION_RDY_SEND size drift");

struct PROTO_NC_MISC_S2SCONNECTION_REQ { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_REQ) == 7, "PROTO_NC_MISC_S2SCONNECTION_REQ size drift");

struct PROTO_NC_MISC_S2SCONNECTION_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_MISC_S2SCONNECTION_REQ req;
};
static_assert(sizeof(PROTO_NC_MISC_S2SCONNECTION_REQ_SEND) == 10, "PROTO_NC_MISC_S2SCONNECTION_REQ_SEND size drift");

struct PROTO_NC_MISC_SEED_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MISC_SEED_ACK) == 2, "PROTO_NC_MISC_SEED_ACK size drift");

struct PROTO_NC_MISC_SERVERPARAMETER_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_SERVERPARAMETER_ACK) == 3, "PROTO_NC_MISC_SERVERPARAMETER_ACK size drift");

struct PROTO_NC_MISC_SERVERPARAMETER_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_SERVERPARAMETER_REQ) == 1, "PROTO_NC_MISC_SERVERPARAMETER_REQ size drift");

struct PROTO_NC_MISC_SERVER_TIME_NOTIFY_CMD {
    tm dCurrentTM;
    uint8_t _tail[37];
};
static_assert(sizeof(PROTO_NC_MISC_SERVER_TIME_NOTIFY_CMD) == 37, "PROTO_NC_MISC_SERVER_TIME_NOTIFY_CMD size drift");

struct PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_CMD) == 3, "PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_CMD size drift");

struct PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_DB_CMD { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_DB_CMD) == 11, "PROTO_NC_MISC_SET_CHAT_BLOCK_SPAM_FILTER_DB_CMD size drift");

struct PROTO_NC_MISC_SPAMMER_CHAT_BAN_ACK {
    Name5 sSpammerName;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_CHAT_BAN_ACK) == 22, "PROTO_NC_MISC_SPAMMER_CHAT_BAN_ACK size drift");

struct PROTO_NC_MISC_SPAMMER_CHAT_BAN_REQ {
    Name5 sSpammerName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_CHAT_BAN_REQ) == 20, "PROTO_NC_MISC_SPAMMER_CHAT_BAN_REQ size drift");

struct PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_ACK) == 2, "PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_ACK size drift");

struct PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_ACK {
    uint8_t _pad_at_0000[4];
    Name5 sSpammerName;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_ACK) == 30, "PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_ACK size drift");

struct PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_REQ {
    uint8_t _pad_at_0000[4];
    Name5 sSpammerName;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_REQ) == 28, "PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_DB_REQ size drift");

struct PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_REQ {
    uint8_t _pad_at_0000[4];
    Name5 sSpammerName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_REQ) == 24, "PROTO_NC_MISC_SPAMMER_RELEASE_CHAT_BAN_REQ size drift");

struct PROTO_NC_MISC_SPAMMER_REPORT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_REPORT_ACK) == 2, "PROTO_NC_MISC_SPAMMER_REPORT_ACK size drift");

struct PROTO_NC_MISC_SPAMMER_REPORT_REQ {
    Name5 sSpammerName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_REPORT_REQ) == 20, "PROTO_NC_MISC_SPAMMER_REPORT_REQ size drift");

struct PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_ACK {
    uint8_t _pad_at_0000[4];
    Name5 sSpammerName;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_ACK) == 30, "PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_ACK size drift");

struct PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_REQ {
    uint8_t _pad_at_0000[4];
    Name5 sSpammerName;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_REQ) == 28, "PROTO_NC_MISC_SPAMMER_SET_DB_CHAT_BAN_REQ size drift");

struct PROTO_NC_MISC_SPAMMER_ZONE_CHAT_BAN_CMD {
    Name5 sSpammerName;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_MISC_SPAMMER_ZONE_CHAT_BAN_CMD) == 24, "PROTO_NC_MISC_SPAMMER_ZONE_CHAT_BAN_CMD size drift");

struct PROTO_NC_MISC_START_THE_BOOM_CMD {
    uint8_t sKey1[16];
    uint8_t sKey2[16];
    uint8_t sKey3[16];
    uint8_t sKey4[16];
};
static_assert(sizeof(PROTO_NC_MISC_START_THE_BOOM_CMD) == 64, "PROTO_NC_MISC_START_THE_BOOM_CMD size drift");

struct PROTO_NC_MISC_TIMEFROMWORLD_CMD {
    tm TimeStr;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_MISC_TIMEFROMWORLD_CMD) == 36, "PROTO_NC_MISC_TIMEFROMWORLD_CMD size drift");

struct PROTO_NC_MISC_USER_COUNT_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MISC_USER_COUNT_CMD) == 4, "PROTO_NC_MISC_USER_COUNT_CMD size drift");

struct PROTO_NC_MISC_WEB_DB_KEY_ACK {
    uint8_t _pad_at_0000[8];
    uint8_t WebKey[32];
};
static_assert(sizeof(PROTO_NC_MISC_WEB_DB_KEY_ACK) == 40, "PROTO_NC_MISC_WEB_DB_KEY_ACK size drift");

struct PROTO_NC_MISC_WEB_DB_KEY_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MISC_WEB_DB_KEY_REQ) == 6, "PROTO_NC_MISC_WEB_DB_KEY_REQ size drift");

struct PROTO_NC_MISC_WEB_KEY_ACK {
    uint8_t _pad_at_0000[2];
    uint8_t WebKey[32];
};
static_assert(sizeof(PROTO_NC_MISC_WEB_KEY_ACK) == 34, "PROTO_NC_MISC_WEB_KEY_ACK size drift");

struct PROTO_NC_MISC_WEB_KEY_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_WEB_KEY_REQ) == 1, "PROTO_NC_MISC_WEB_KEY_REQ size drift");

struct PROTO_NC_MISC_WHSHANDLEFIX_CMD {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_MISC_WHSHANDLEFIX_CMD) == 9, "PROTO_NC_MISC_WHSHANDLEFIX_CMD size drift");

struct PROTO_NC_MISC_WHSHANDLEREPAIR_CMD {
    NETPACKETZONEHEADER packheader;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_MISC_WHSHANDLEREPAIR_CMD) == 11, "PROTO_NC_MISC_WHSHANDLEREPAIR_CMD size drift");

struct PROTO_NC_MISC_XTRAP2_CLIENT_DATA_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_MISC_XTRAP2_CLIENT_DATA_CMD) == 2, "PROTO_NC_MISC_XTRAP2_CLIENT_DATA_CMD size drift");

struct PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_ACK) == 1, "PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_ACK size drift");

struct PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_REQ) == 1, "PROTO_NC_MISC_XTRAP2_OPTOOL_READ_CODEMAP_REQ size drift");

struct PROTO_NC_MISC_XTRAP2_SERVER_DATA_CMD {
    uint8_t _pad_at_0000[2];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_MISC_XTRAP2_SERVER_DATA_CMD) == 2, "PROTO_NC_MISC_XTRAP2_SERVER_DATA_CMD size drift");

struct PROTO_NC_MISC_ZONERINGLINKTEST_RNG {
    ZONERINGLINKAGESTART start;
    uint8_t _pad_at_0000[8];
    Name4_______0_bytes___ addr;
};
static_assert(sizeof(PROTO_NC_MISC_ZONERINGLINKTEST_RNG) == 8, "PROTO_NC_MISC_ZONERINGLINKTEST_RNG size drift");

struct PROTO_NC_MOVER_FEEDING_ERROR_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_FEEDING_ERROR_CMD) == 2, "PROTO_NC_MOVER_FEEDING_ERROR_CMD size drift");

struct PROTO_NC_MOVER_HUNGRY_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_HUNGRY_CMD) == 2, "PROTO_NC_MOVER_HUNGRY_CMD size drift");

struct PROTO_NC_MOVER_MOVESPEED_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MOVER_MOVESPEED_CMD) == 6, "PROTO_NC_MOVER_MOVESPEED_CMD size drift");

struct PROTO_NC_MOVER_RIDE_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_FAIL_CMD) == 2, "PROTO_NC_MOVER_RIDE_FAIL_CMD size drift");

struct PROTO_NC_MOVER_RIDE_OFF_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_OFF_CMD) == 1, "PROTO_NC_MOVER_RIDE_OFF_CMD size drift");

struct PROTO_NC_MOVER_RIDE_OFF_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_OFF_FAIL_CMD) == 2, "PROTO_NC_MOVER_RIDE_OFF_FAIL_CMD size drift");

struct PROTO_NC_MOVER_RIDE_OFF_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_OFF_REQ) == 1, "PROTO_NC_MOVER_RIDE_OFF_REQ size drift");

struct PROTO_NC_MOVER_RIDE_ON_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_ON_CMD) == 4, "PROTO_NC_MOVER_RIDE_ON_CMD size drift");

struct PROTO_NC_MOVER_RIDE_ON_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_ON_FAIL_CMD) == 2, "PROTO_NC_MOVER_RIDE_ON_FAIL_CMD size drift");

struct PROTO_NC_MOVER_RIDE_ON_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_MOVER_RIDE_ON_REQ) == 3, "PROTO_NC_MOVER_RIDE_ON_REQ size drift");

struct PROTO_NC_MOVER_SKILLBASH_FLD_CAST_REQ {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE nTargetLoc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_MOVER_SKILLBASH_FLD_CAST_REQ) == 10, "PROTO_NC_MOVER_SKILLBASH_FLD_CAST_REQ size drift");

struct PROTO_NC_MOVER_SKILLBASH_OBJ_CAST_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_MOVER_SKILLBASH_OBJ_CAST_REQ) == 4, "PROTO_NC_MOVER_SKILLBASH_OBJ_CAST_REQ size drift");

struct PROTO_NC_MOVER_SOMEONE_RIDE_OFF_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_MOVER_SOMEONE_RIDE_OFF_CMD) == 2, "PROTO_NC_MOVER_SOMEONE_RIDE_OFF_CMD size drift");

struct PROTO_NC_MOVER_SOMEONE_RIDE_ON_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_MOVER_SOMEONE_RIDE_ON_CMD) == 6, "PROTO_NC_MOVER_SOMEONE_RIDE_ON_CMD size drift");

struct PROTO_NC_OPTOOL_ACK_CLIENT_NUM_OF_USER_LIMIT { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_OPTOOL_ACK_CLIENT_NUM_OF_USER_LIMIT) == 9, "PROTO_NC_OPTOOL_ACK_CLIENT_NUM_OF_USER_LIMIT size drift");

struct PROTO_NC_OPTOOL_CHARACTER_DELETE_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_CHARACTER_DELETE_ACK) == 1, "PROTO_NC_OPTOOL_CHARACTER_DELETE_ACK size drift");

struct PROTO_NC_OPTOOL_CHARACTER_DELETE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_CHARACTER_DELETE_CMD) == 4, "PROTO_NC_OPTOOL_CHARACTER_DELETE_CMD size drift");

struct PROTO_NC_OPTOOL_CHARACTER_DELETE_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_CHARACTER_DELETE_REQ) == 4, "PROTO_NC_OPTOOL_CHARACTER_DELETE_REQ size drift");

struct PROTO_NC_OPTOOL_CLOSE_SERVER_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_OPTOOL_CLOSE_SERVER_ACK) == 2, "PROTO_NC_OPTOOL_CLOSE_SERVER_ACK size drift");

struct PROTO_NC_OPTOOL_CLOSE_SERVER_REQ {
    uint8_t _pad_at_0000[1];
    uint8_t closekey[32];
};
static_assert(sizeof(PROTO_NC_OPTOOL_CLOSE_SERVER_REQ) == 33, "PROTO_NC_OPTOOL_CLOSE_SERVER_REQ size drift");

struct PROTO_NC_OPTOOL_CONNECT_BRIF_ACK {
    uint8_t _pad_at_0000[2];
    uint16_t count[2];
    uint8_t _tail[164];
};
static_assert(sizeof(PROTO_NC_OPTOOL_CONNECT_BRIF_ACK) == 170, "PROTO_NC_OPTOOL_CONNECT_BRIF_ACK size drift");

struct PROTO_NC_OPTOOL_CONNECT_BRIF_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_CONNECT_BRIF_REQ) == 1, "PROTO_NC_OPTOOL_CONNECT_BRIF_REQ size drift");

struct PROTO_NC_OPTOOL_FIND_USER_ACK {
    uint8_t _pad_at_0000[5];
    Name256Byte sUserID;
    uint8_t _pad_at_0005[260];
    Name5 sCharID;
    uint8_t _pad_at_0109[20];
    Name3 sMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_OPTOOL_FIND_USER_ACK) == 297, "PROTO_NC_OPTOOL_FIND_USER_ACK size drift");

struct PROTO_NC_OPTOOL_FIND_USER_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_FIND_USER_REQ) == 4, "PROTO_NC_OPTOOL_FIND_USER_REQ size drift");

struct PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_ACK) == 2, "PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_ACK size drift");

struct PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_REQ { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_REQ) == 10, "PROTO_NC_OPTOOL_GUILD_CHANGE_MEMBER_GRADE_REQ size drift");

struct PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_ACK) == 2, "PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_ACK size drift");

struct PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_REQ {
    uint8_t _pad_at_0000[4];
    Name4 sName;
    uint8_t _pad_at_0004[16];
    Name3 sPassword;
    uint8_t _pad_at_0014[59];
    Name5 sNotifyCharID;
    uint8_t _pad_at_004f[20];
    wchar_t sIntro[128];
    wchar_t sNotify[512];
};
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_REQ) == 739, "PROTO_NC_OPTOOL_GUILD_DATA_CHANGE_REQ size drift");

struct PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_ACK) == 2, "PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_ACK size drift");

struct PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_REQ) == 4, "PROTO_NC_OPTOOL_GUILD_DISMISS_CANCEL_REQ size drift");

struct PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_ACK) == 2, "PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_ACK size drift");

struct PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_CMD {
    uint8_t _pad_at_0000[37];
    GUILD_TOURNAMENT_LIST_______651_bytes___ TournamentTree;
    uint8_t _tail[651];
};
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_CMD) == 688, "PROTO_NC_OPTOOL_GUILD_TOURNAMENT_CHANGE_CMD size drift");

struct PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_ACK) == 4, "PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_ACK size drift");

struct PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_REQ) == 1, "PROTO_NC_OPTOOL_GUILD_TOURNAMENT_SCHEDULE_RESET_REQ size drift");

struct PROTO_NC_OPTOOL_KICK_USER_ACK {
    uint8_t _pad_at_0000[5];
    Name256Byte sUserID;
    uint8_t _pad_at_0005[260];
    Name5 sCharID;
    uint8_t _pad_at_0109[20];
    Name3 sMapName;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_OPTOOL_KICK_USER_ACK) == 297, "PROTO_NC_OPTOOL_KICK_USER_ACK size drift");

struct PROTO_NC_OPTOOL_KICK_USER_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_KICK_USER_REQ) == 4, "PROTO_NC_OPTOOL_KICK_USER_REQ size drift");

struct PROTO_NC_OPTOOL_KQ_ALL_RESET_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_ALL_RESET_CMD) == 1, "PROTO_NC_OPTOOL_KQ_ALL_RESET_CMD size drift");

struct PROTO_NC_OPTOOL_KQ_CHANGE_CMD {
    PROTO_KQ_INFO KQInfo;
    uint8_t _tail[377];
};
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_CHANGE_CMD) == 377, "PROTO_NC_OPTOOL_KQ_CHANGE_CMD size drift");

struct PROTO_NC_OPTOOL_KQ_DELETE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_DELETE_CMD) == 4, "PROTO_NC_OPTOOL_KQ_DELETE_CMD size drift");

struct PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_ACK {
    uint8_t _pad_at_0000[4];
    KQ_MAP_ALLOC_TABLE_______0_bytes___ MapArray;
};
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_ACK) == 4, "PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_ACK size drift");

struct PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_REQ) == 1, "PROTO_NC_OPTOOL_KQ_MAP_ALLOC_INFO_REQ size drift");

struct PROTO_NC_OPTOOL_KQ_SCHEDULE_ACK {
    uint8_t _pad_at_0000[7];
    PROTO_KQ_INFO_______0_bytes___ QuestArray;
};
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_SCHEDULE_ACK) == 7, "PROTO_NC_OPTOOL_KQ_SCHEDULE_ACK size drift");

struct PROTO_NC_OPTOOL_KQ_SCHEDULE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_KQ_SCHEDULE_REQ) == 1, "PROTO_NC_OPTOOL_KQ_SCHEDULE_REQ size drift");

struct PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_ACK {
    uint8_t _pad_at_0000[1];
    LOGIN_USER_RATABLE RateTable;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_ACK) == 13, "PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_ACK size drift");

struct PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_REQ) == 1, "PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_GET_REQ size drift");

struct PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_SET_CMD {
    uint8_t _pad_at_0000[1];
    LOGIN_USER_RATABLE RateTable;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_SET_CMD) == 13, "PROTO_NC_OPTOOL_LOGIN_USER_RATABLE_SET_CMD size drift");

struct PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_ACK { uint8_t data[24]; };
static_assert(sizeof(PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_ACK) == 24, "PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_ACK size drift");

struct PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_CLR { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_CLR) == 1, "PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_CLR size drift");

struct PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_REQ) == 1, "PROTO_NC_OPTOOL_LOGON_PROCESS_TIME_VIEW_REQ size drift");

struct PROTO_NC_OPTOOL_MAP_USER_LIST_ACK {
    uint8_t _pad_at_0000[4];
    NC_OPTOOL_MAP_USER_LIST_INFO_______0_bytes___ user_info;
};
static_assert(sizeof(PROTO_NC_OPTOOL_MAP_USER_LIST_ACK) == 4, "PROTO_NC_OPTOOL_MAP_USER_LIST_ACK size drift");

struct PROTO_NC_OPTOOL_MAP_USER_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_MAP_USER_LIST_REQ) == 1, "PROTO_NC_OPTOOL_MAP_USER_LIST_REQ size drift");

struct PROTO_NC_OPTOOL_REQ_CLIENT_NUM_OF_USER_LIMIT { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_REQ_CLIENT_NUM_OF_USER_LIMIT) == 1, "PROTO_NC_OPTOOL_REQ_CLIENT_NUM_OF_USER_LIMIT size drift");

struct PROTO_NC_OPTOOL_S2SCONNECT_LIST_ACK {
    uint8_t _pad_at_0000[4];
    SERVER_CONN_INFO_______0_bytes___ connection_info;
};
static_assert(sizeof(PROTO_NC_OPTOOL_S2SCONNECT_LIST_ACK) == 4, "PROTO_NC_OPTOOL_S2SCONNECT_LIST_ACK size drift");

struct PROTO_NC_OPTOOL_S2SCONNECT_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_S2SCONNECT_LIST_REQ) == 1, "PROTO_NC_OPTOOL_S2SCONNECT_LIST_REQ size drift");

struct PROTO_NC_OPTOOL_SET_CLIENT_NUM_OF_USER_LIMIT { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_OPTOOL_SET_CLIENT_NUM_OF_USER_LIMIT) == 4, "PROTO_NC_OPTOOL_SET_CLIENT_NUM_OF_USER_LIMIT size drift");

struct PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK___unnamed_type_Data________0_bytes___ Data;
};
static_assert(sizeof(PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK) == 3, "PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK size drift");

struct PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK___unnamed_type_Data_ { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK___unnamed_type_Data_) == 14, "PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_ACK___unnamed_type_Data_ size drift");

struct PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_CLR { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_CLR) == 1, "PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_CLR size drift");

struct PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_REQ) == 1, "PROTO_NC_OPTOOL_WM_SEND_PACKET_STATISTICS_REQ size drift");

struct PROTO_NC_PARTY_CHANGEMASTER_ACK {
    Name5 newmaster;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PARTY_CHANGEMASTER_ACK) == 22, "PROTO_NC_PARTY_CHANGEMASTER_ACK size drift");

struct PROTO_NC_PARTY_CHANGEMASTER_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_CHANGEMASTER_ACK ack;
};
static_assert(sizeof(PROTO_NC_PARTY_CHANGEMASTER_ACK_SEND) == 25, "PROTO_NC_PARTY_CHANGEMASTER_ACK_SEND size drift");

struct PROTO_NC_PARTY_CHANGEMASTER_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PARTY_CHANGEMASTER_CMD) == 6, "PROTO_NC_PARTY_CHANGEMASTER_CMD size drift");

struct PROTO_NC_PARTY_CHANGEMASTER_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_CHANGEMASTER_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_CHANGEMASTER_CMD_SEND) == 9, "PROTO_NC_PARTY_CHANGEMASTER_CMD_SEND size drift");

struct PROTO_NC_PARTY_CHANGEMASTER_REQ {
    Name5 newmaster;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_CHANGEMASTER_REQ) == 20, "PROTO_NC_PARTY_CHANGEMASTER_REQ size drift");

struct PROTO_NC_PARTY_DISMISS_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PARTY_DISMISS_ACK) == 1, "PROTO_NC_PARTY_DISMISS_ACK size drift");

struct PROTO_NC_PARTY_DISMISS_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_PARTY_DISMISS_ACK_SEND) == 3, "PROTO_NC_PARTY_DISMISS_ACK_SEND size drift");

struct PROTO_NC_PARTY_DISMISS_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_DISMISS_CMD) == 2, "PROTO_NC_PARTY_DISMISS_CMD size drift");

struct PROTO_NC_PARTY_DISMISS_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_DISMISS_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_DISMISS_CMD_SEND) == 5, "PROTO_NC_PARTY_DISMISS_CMD_SEND size drift");

struct PROTO_NC_PARTY_FINDER_ADD_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_FINDER_ADD_ACK) == 2, "PROTO_NC_PARTY_FINDER_ADD_ACK size drift");

struct PROTO_NC_PARTY_FINDER_ADD_REQ {
    uint8_t sMsg[128];
};
static_assert(sizeof(PROTO_NC_PARTY_FINDER_ADD_REQ) == 128, "PROTO_NC_PARTY_FINDER_ADD_REQ size drift");

struct PROTO_NC_PARTY_FINDER_DELETE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_FINDER_DELETE_ACK) == 2, "PROTO_NC_PARTY_FINDER_DELETE_ACK size drift");

struct PROTO_NC_PARTY_FINDER_DELETE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PARTY_FINDER_DELETE_REQ) == 1, "PROTO_NC_PARTY_FINDER_DELETE_REQ size drift");

struct PROTO_NC_PARTY_FINDER_DELETE_YOUR_MSG_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PARTY_FINDER_DELETE_YOUR_MSG_CMD) == 1, "PROTO_NC_PARTY_FINDER_DELETE_YOUR_MSG_CMD size drift");

struct PROTO_NC_PARTY_FINDER_INFO {
    uint8_t _pad_at_0000[6];
    Name5 sCharID;
    uint8_t _pad_at_0006[20];
    uint8_t sMsg[128];
};
static_assert(sizeof(PROTO_NC_PARTY_FINDER_INFO) == 154, "PROTO_NC_PARTY_FINDER_INFO size drift");

struct PROTO_NC_PARTY_FINDER_LIST_ACK {
    uint8_t _pad_at_0000[8];
    PROTO_NC_PARTY_FINDER_INFO_______0_bytes___ InfoList;
};
static_assert(sizeof(PROTO_NC_PARTY_FINDER_LIST_ACK) == 8, "PROTO_NC_PARTY_FINDER_LIST_ACK size drift");

struct PROTO_NC_PARTY_FINDER_LIST_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_PARTY_FINDER_LIST_REQ) == 3, "PROTO_NC_PARTY_FINDER_LIST_REQ size drift");

struct PROTO_NC_PARTY_FUNDAMENTAL_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_PARTY_MEMBER_AND_ZONEMERCHINE master;
    uint8_t _pad_at_0002[32];
    PROTO_PARTY_MEMBER_AND_ZONEMERCHINE member;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_PARTY_FUNDAMENTAL_CMD) == 66, "PROTO_NC_PARTY_FUNDAMENTAL_CMD size drift");

struct PROTO_NC_PARTY_FUNDAMENTAL_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_FUNDAMENTAL_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_FUNDAMENTAL_CMD_SEND) == 69, "PROTO_NC_PARTY_FUNDAMENTAL_CMD_SEND size drift");

struct PROTO_NC_PARTY_ITEM_JOIN_LOOTING_CMD {
    uint8_t _pad_at_0000[2];
    Name5 LooterName;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_ITEM_JOIN_LOOTING_CMD) == 22, "PROTO_NC_PARTY_ITEM_JOIN_LOOTING_CMD size drift");

struct PROTO_NC_PARTY_ITEM_LOOTING_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_ITEM_LOOTING_CMD) == 2, "PROTO_NC_PARTY_ITEM_LOOTING_CMD size drift");

struct PROTO_NC_PARTY_ITEM_LOOTING_SET { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_ITEM_LOOTING_SET) == 2, "PROTO_NC_PARTY_ITEM_LOOTING_SET size drift");

struct PROTO_NC_PARTY_ITEM_LOOTING_ZONE_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PARTY_ITEM_LOOTING_ZONE_CMD) == 8, "PROTO_NC_PARTY_ITEM_LOOTING_ZONE_CMD size drift");

struct PROTO_NC_PARTY_JOINPROPOSE_REQ {
    Name5 mastername;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_JOINPROPOSE_REQ) == 20, "PROTO_NC_PARTY_JOINPROPOSE_REQ size drift");

struct PROTO_NC_PARTY_JOINPROPOSE_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_JOINPROPOSE_REQ req;
};
static_assert(sizeof(PROTO_NC_PARTY_JOINPROPOSE_REQ_SEND) == 23, "PROTO_NC_PARTY_JOINPROPOSE_REQ_SEND size drift");

struct PROTO_NC_PARTY_JOIN_ACK {
    Name5 memberid;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_PARTY_JOIN_ACK) == 23, "PROTO_NC_PARTY_JOIN_ACK size drift");

struct PROTO_NC_PARTY_JOIN_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_JOIN_ACK ack;
};
static_assert(sizeof(PROTO_NC_PARTY_JOIN_ACK_SEND) == 26, "PROTO_NC_PARTY_JOIN_ACK_SEND size drift");

struct PROTO_NC_PARTY_JOIN_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_PARTY_MEMBER_AND_ZONEMERCHINE joiner;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_PARTY_JOIN_CMD) == 34, "PROTO_NC_PARTY_JOIN_CMD size drift");

struct PROTO_NC_PARTY_JOIN_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_JOIN_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_JOIN_CMD_SEND) == 37, "PROTO_NC_PARTY_JOIN_CMD_SEND size drift");

struct PROTO_NC_PARTY_JOIN_REQ {
    Name5 target;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_JOIN_REQ) == 20, "PROTO_NC_PARTY_JOIN_REQ size drift");

struct PROTO_NC_PARTY_KICKOFF_ACK {
    Name5 memberid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PARTY_KICKOFF_ACK) == 22, "PROTO_NC_PARTY_KICKOFF_ACK size drift");

struct PROTO_NC_PARTY_KICKOFF_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_KICKOFF_ACK ack;
};
static_assert(sizeof(PROTO_NC_PARTY_KICKOFF_ACK_SEND) == 25, "PROTO_NC_PARTY_KICKOFF_ACK_SEND size drift");

struct PROTO_NC_PARTY_KICKOFF_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PARTY_KICKOFF_CMD) == 6, "PROTO_NC_PARTY_KICKOFF_CMD size drift");

struct PROTO_NC_PARTY_KICKOFF_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_KICKOFF_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_KICKOFF_CMD_SEND) == 9, "PROTO_NC_PARTY_KICKOFF_CMD_SEND size drift");

struct PROTO_NC_PARTY_KICKOFF_REQ {
    Name5 member;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_KICKOFF_REQ) == 20, "PROTO_NC_PARTY_KICKOFF_REQ size drift");

struct PROTO_NC_PARTY_LEAVE_ACK {
    Name5 memberid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PARTY_LEAVE_ACK) == 22, "PROTO_NC_PARTY_LEAVE_ACK size drift");

struct PROTO_NC_PARTY_LEAVE_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LEAVE_ACK ack;
};
static_assert(sizeof(PROTO_NC_PARTY_LEAVE_ACK_SEND) == 25, "PROTO_NC_PARTY_LEAVE_ACK_SEND size drift");

struct PROTO_NC_PARTY_LEAVE_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_PARTY_MEMBER leavemember;
    uint8_t _tail[31];
};
static_assert(sizeof(PROTO_NC_PARTY_LEAVE_CMD) == 33, "PROTO_NC_PARTY_LEAVE_CMD size drift");

struct PROTO_NC_PARTY_LEAVE_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LEAVE_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_LEAVE_CMD_SEND) == 36, "PROTO_NC_PARTY_LEAVE_CMD_SEND size drift");

struct PROTO_NC_PARTY_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PARTY_LEAVE_REQ) == 1, "PROTO_NC_PARTY_LEAVE_REQ size drift");

struct PROTO_NC_PARTY_LOGININFO_CMD {
    Name5 memberid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_LOGININFO_CMD) == 20, "PROTO_NC_PARTY_LOGININFO_CMD size drift");

struct PROTO_NC_PARTY_LOGININFO_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LOGININFO_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_LOGININFO_CMD_SEND) == 23, "PROTO_NC_PARTY_LOGININFO_CMD_SEND size drift");

struct PROTO_NC_PARTY_LOGIN_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PARTY_LOGIN_CMD) == 6, "PROTO_NC_PARTY_LOGIN_CMD size drift");

struct PROTO_NC_PARTY_LOGIN_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LOGIN_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_LOGIN_CMD_SEND) == 9, "PROTO_NC_PARTY_LOGIN_CMD_SEND size drift");

struct PROTO_NC_PARTY_LOGOUTINFO_CMD {
    Name5 memberid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_LOGOUTINFO_CMD) == 20, "PROTO_NC_PARTY_LOGOUTINFO_CMD size drift");

struct PROTO_NC_PARTY_LOGOUTINFO_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LOGOUTINFO_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_LOGOUTINFO_CMD_SEND) == 23, "PROTO_NC_PARTY_LOGOUTINFO_CMD_SEND size drift");

struct PROTO_NC_PARTY_LOGOUT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PARTY_LOGOUT_CMD) == 6, "PROTO_NC_PARTY_LOGOUT_CMD size drift");

struct PROTO_NC_PARTY_LOGOUT_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_PARTY_LOGOUT_CMD cmd;
};
static_assert(sizeof(PROTO_NC_PARTY_LOGOUT_CMD_SEND) == 9, "PROTO_NC_PARTY_LOGOUT_CMD_SEND size drift");

struct PROTO_NC_PARTY_MEMBERCLASS_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_PARTY_MEMBER_CLASS_______0_bytes___ member;
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERCLASS_CMD) == 1, "PROTO_NC_PARTY_MEMBERCLASS_CMD size drift");

struct PROTO_NC_PARTY_MEMBERINFORM_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_PARTY_MEMBER_______0_bytes___ Member;
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERINFORM_ACK) == 3, "PROTO_NC_PARTY_MEMBERINFORM_ACK size drift");

struct PROTO_NC_PARTY_MEMBERINFORM_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_PARTY_MEMBER_INFORM_______0_bytes___ member;
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERINFORM_CMD) == 1, "PROTO_NC_PARTY_MEMBERINFORM_CMD size drift");

struct PROTO_NC_PARTY_MEMBERINFORM_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PARTY_MEMBERINFORM_REQ) == 2, "PROTO_NC_PARTY_MEMBERINFORM_REQ size drift");

struct PROTO_NC_PARTY_MEMBERLOCATION_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_PARTY_MEMBERLOCATION_CMD___unnamed_type_member________0_bytes___ member;
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERLOCATION_CMD) == 1, "PROTO_NC_PARTY_MEMBERLOCATION_CMD size drift");

struct PROTO_NC_PARTY_MEMBERLOCATION_CMD___unnamed_type_member_ {
    Name5 charid;
    uint8_t _pad_at_0000[20];
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERLOCATION_CMD___unnamed_type_member_) == 28, "PROTO_NC_PARTY_MEMBERLOCATION_CMD___unnamed_type_member_ size drift");

struct PROTO_NC_PARTY_MEMBERMAPOUT {
    Name5 memberid;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBERMAPOUT) == 20, "PROTO_NC_PARTY_MEMBERMAPOUT size drift");

struct PROTO_NC_PARTY_MEMBER_LIST_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_PARTY_MEMBER_LIST_CMD__MEMBER_INFO_______0_bytes___ memberarray;
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBER_LIST_CMD) == 1, "PROTO_NC_PARTY_MEMBER_LIST_CMD size drift");

struct PROTO_NC_PARTY_MEMBER_LIST_CMD__MEMBER_INFO {
    Name5 memberid;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBER_LIST_CMD__MEMBER_INFO) == 22, "PROTO_NC_PARTY_MEMBER_LIST_CMD__MEMBER_INFO size drift");

struct PROTO_NC_PARTY_MEMBER_LIST_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[3];
    PROTO_NC_PARTY_MEMBER_LIST_CMD__MEMBER_INFO_______110_bytes___ memberarray;
    uint8_t _tail[110];
};
static_assert(sizeof(PROTO_NC_PARTY_MEMBER_LIST_CMD_SEND) == 114, "PROTO_NC_PARTY_MEMBER_LIST_CMD_SEND size drift");

struct PROTO_NC_PARTY_SET_LOOTER_ACK {
    Name5 sNewLooterCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PARTY_SET_LOOTER_ACK) == 22, "PROTO_NC_PARTY_SET_LOOTER_ACK size drift");

struct PROTO_NC_PARTY_SET_LOOTER_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    Name5 sNewLooterCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_SET_LOOTER_BROAD_CMD) == 40, "PROTO_NC_PARTY_SET_LOOTER_BROAD_CMD size drift");

struct PROTO_NC_PARTY_SET_LOOTER_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_SET_LOOTER_CMD) == 20, "PROTO_NC_PARTY_SET_LOOTER_CMD size drift");

struct PROTO_NC_PARTY_SET_LOOTER_REQ {
    Name5 sNewLooterCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_PARTY_SET_LOOTER_REQ) == 20, "PROTO_NC_PARTY_SET_LOOTER_REQ size drift");

struct PROTO_NC_PARTY_ZONE_JOIN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PARTY_ZONE_JOIN_CMD) == 8, "PROTO_NC_PARTY_ZONE_JOIN_CMD size drift");

struct PROTO_NC_PARTY_ZONE_LEAVE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PARTY_ZONE_LEAVE_CMD) == 4, "PROTO_NC_PARTY_ZONE_LEAVE_CMD size drift");

struct PROTO_NC_PARTY_ZONE_SET_LOOTER_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PARTY_ZONE_SET_LOOTER_CMD) == 6, "PROTO_NC_PARTY_ZONE_SET_LOOTER_CMD size drift");

struct PROTO_NC_PATCH_CLIENT_VERSION_ACK {
    PATCH_VERSION_MARK ClientVer;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_PATCH_CLIENT_VERSION_ACK) == 5, "PROTO_NC_PATCH_CLIENT_VERSION_ACK size drift");

struct PROTO_NC_PATCH_CLIENT_VERSION_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_CLIENT_VERSION_REQ) == 1, "PROTO_NC_PATCH_CLIENT_VERSION_REQ size drift");

struct PROTO_NC_PATCH_CLOSE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_CLOSE_REQ) == 1, "PROTO_NC_PATCH_CLOSE_REQ size drift");

struct PROTO_NC_PATCH_DATA_SERVER_READY_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_PATCH_DATA_SERVER_READY_CMD) == 5, "PROTO_NC_PATCH_DATA_SERVER_READY_CMD size drift");

struct PROTO_NC_PATCH_DATA_SERVER_USER_COUNT_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_PATCH_DATA_SERVER_USER_COUNT_CMD) == 3, "PROTO_NC_PATCH_DATA_SERVER_USER_COUNT_CMD size drift");

struct PROTO_NC_PATCH_FILE_DATA_ACK {
    uint8_t _pad_at_0000[12];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_PATCH_FILE_DATA_ACK) == 12, "PROTO_NC_PATCH_FILE_DATA_ACK size drift");

struct PROTO_NC_PATCH_FILE_DATA_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PATCH_FILE_DATA_REQ) == 8, "PROTO_NC_PATCH_FILE_DATA_REQ size drift");

struct PROTO_NC_PATCH_FILE_INFO_ACK {
    uint8_t _pad_at_0000[16];
    uint8_t FileName[260];
};
static_assert(sizeof(PROTO_NC_PATCH_FILE_INFO_ACK) == 276, "PROTO_NC_PATCH_FILE_INFO_ACK size drift");

struct PROTO_NC_PATCH_FILE_INFO_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PATCH_FILE_INFO_REQ) == 4, "PROTO_NC_PATCH_FILE_INFO_REQ size drift");

struct PROTO_NC_PATCH_INFO_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_PATCH_INFO_ACK) == 10, "PROTO_NC_PATCH_INFO_ACK size drift");

struct PROTO_NC_PATCH_INFO_DATA_ACK {
    uint8_t _pad_at_0000[8];
    uint8_t Data[0];
};
static_assert(sizeof(PROTO_NC_PATCH_INFO_DATA_ACK) == 8, "PROTO_NC_PATCH_INFO_DATA_ACK size drift");

struct PROTO_NC_PATCH_INFO_DATA_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PATCH_INFO_DATA_REQ) == 6, "PROTO_NC_PATCH_INFO_DATA_REQ size drift");

struct PROTO_NC_PATCH_INFO_REQ {
    PATCH_VERSION_MARK MyClientVer;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_PATCH_INFO_REQ) == 5, "PROTO_NC_PATCH_INFO_REQ size drift");

struct PROTO_NC_PATCH_INFO_VERIFY_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_INFO_VERIFY_ACK) == 1, "PROTO_NC_PATCH_INFO_VERIFY_ACK size drift");

struct PROTO_NC_PATCH_INFO_VERIFY_REQ {
    PATCH_VERSION_MARK LauncherVer;
    uint8_t _pad_at_0000[5];
    PATCH_VERSION_MARK LastVer;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_PATCH_INFO_VERIFY_REQ) == 22, "PROTO_NC_PATCH_INFO_VERIFY_REQ size drift");

struct PROTO_NC_PATCH_LAUNCHER_VERSION_ACK {
    PATCH_VERSION_MARK LauncherVer;
    uint8_t _tail[9];
};
static_assert(sizeof(PROTO_NC_PATCH_LAUNCHER_VERSION_ACK) == 9, "PROTO_NC_PATCH_LAUNCHER_VERSION_ACK size drift");

struct PROTO_NC_PATCH_LAUNCHER_VERSION_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_LAUNCHER_VERSION_REQ) == 1, "PROTO_NC_PATCH_LAUNCHER_VERSION_REQ size drift");

struct PROTO_NC_PATCH_NOTICE_ACK {
    uint8_t Patch_Notice_URL[128];
    uint8_t Launcher_Notice_URL[128];
};
static_assert(sizeof(PROTO_NC_PATCH_NOTICE_ACK) == 256, "PROTO_NC_PATCH_NOTICE_ACK size drift");

struct PROTO_NC_PATCH_NOTICE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_NOTICE_REQ) == 1, "PROTO_NC_PATCH_NOTICE_REQ size drift");

struct PROTO_NC_PATCH_NOTICE_SET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_NOTICE_SET_REQ) == 1, "PROTO_NC_PATCH_NOTICE_SET_REQ size drift");

struct PROTO_NC_PATCH_SERVER_ALLOC_ACK {
    uint8_t _pad_at_0000[1];
    wchar_t IP[16];
};
static_assert(sizeof(PROTO_NC_PATCH_SERVER_ALLOC_ACK) == 22, "PROTO_NC_PATCH_SERVER_ALLOC_ACK size drift");

struct PROTO_NC_PATCH_SERVER_ALLOC_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_SERVER_ALLOC_REQ) == 1, "PROTO_NC_PATCH_SERVER_ALLOC_REQ size drift");

struct PROTO_NC_PATCH_STATUS_SET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PATCH_STATUS_SET_REQ) == 1, "PROTO_NC_PATCH_STATUS_SET_REQ size drift");

struct PROTO_NC_PET_ASK_NEW_NAME_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PET_ASK_NEW_NAME_ACK) == 4, "PROTO_NC_PET_ASK_NEW_NAME_ACK size drift");

struct PROTO_NC_PET_ASK_NEW_NAME_REQ { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_PET_ASK_NEW_NAME_REQ) == 7, "PROTO_NC_PET_ASK_NEW_NAME_REQ size drift");

struct PROTO_NC_PET_CREATE_DB_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PET_CREATE_DB_ACK) == 8, "PROTO_NC_PET_CREATE_DB_ACK size drift");

struct PROTO_NC_PET_CREATE_DB_REQ {
    uint8_t _pad_at_0000[6];
    Name4 sPetName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_PET_CREATE_DB_REQ) == 24, "PROTO_NC_PET_CREATE_DB_REQ size drift");

struct PROTO_NC_PET_LINK_RESUMMON_CMD { uint8_t data[23]; };
static_assert(sizeof(PROTO_NC_PET_LINK_RESUMMON_CMD) == 23, "PROTO_NC_PET_LINK_RESUMMON_CMD size drift");

struct PROTO_NC_PET_LOAD_INFO_DB_ACK {
    uint8_t _pad_at_0000[8];
    Name4 sPetName;
    uint8_t _tail[18];
};
static_assert(sizeof(PROTO_NC_PET_LOAD_INFO_DB_ACK) == 26, "PROTO_NC_PET_LOAD_INFO_DB_ACK size drift");

struct PROTO_NC_PET_LOAD_INFO_DB_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PET_LOAD_INFO_DB_REQ) == 6, "PROTO_NC_PET_LOAD_INFO_DB_REQ size drift");

struct PROTO_NC_PET_REMOVE_DB_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PET_REMOVE_DB_ACK) == 4, "PROTO_NC_PET_REMOVE_DB_ACK size drift");

struct PROTO_NC_PET_REMOVE_DB_REQ { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_PET_REMOVE_DB_REQ) == 10, "PROTO_NC_PET_REMOVE_DB_REQ size drift");

struct PROTO_NC_PET_SET_NAME_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PET_SET_NAME_ACK) == 6, "PROTO_NC_PET_SET_NAME_ACK size drift");

struct PROTO_NC_PET_SET_NAME_CANCEL_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PET_SET_NAME_CANCEL_ACK) == 6, "PROTO_NC_PET_SET_NAME_CANCEL_ACK size drift");

struct PROTO_NC_PET_SET_NAME_CANCEL_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PET_SET_NAME_CANCEL_REQ) == 2, "PROTO_NC_PET_SET_NAME_CANCEL_REQ size drift");

struct PROTO_NC_PET_SET_NAME_CMD {
    uint8_t _pad_at_0000[2];
    Name4 sPetName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_PET_SET_NAME_CMD) == 18, "PROTO_NC_PET_SET_NAME_CMD size drift");

struct PROTO_NC_PET_SET_NAME_DB_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PET_SET_NAME_DB_ACK) == 8, "PROTO_NC_PET_SET_NAME_DB_ACK size drift");

struct PROTO_NC_PET_SET_NAME_DB_REQ {
    uint8_t _pad_at_0000[6];
    Name4 sPetName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_PET_SET_NAME_DB_REQ) == 22, "PROTO_NC_PET_SET_NAME_DB_REQ size drift");

struct PROTO_NC_PET_SET_NAME_REQ {
    uint8_t _pad_at_0000[2];
    Name4 sPetName;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_PET_SET_NAME_REQ) == 18, "PROTO_NC_PET_SET_NAME_REQ size drift");

struct PROTO_NC_PET_SET_TENDENCY_DB_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PET_SET_TENDENCY_DB_REQ) == 8, "PROTO_NC_PET_SET_TENDENCY_DB_REQ size drift");

struct PROTO_NC_PET_USE_ITEM_FAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PET_USE_ITEM_FAIL_ACK) == 2, "PROTO_NC_PET_USE_ITEM_FAIL_ACK size drift");

struct PROTO_NC_PRISON_ADD_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PRISON_ADD_ACK) == 2, "PROTO_NC_PRISON_ADD_ACK size drift");

struct PROTO_NC_PRISON_ADD_GM_ACK {
    Name5 sImprison;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PRISON_ADD_GM_ACK) == 22, "PROTO_NC_PRISON_ADD_GM_ACK size drift");

struct PROTO_NC_PRISON_ADD_GM_REQ {
    Name5 sImprison;
    uint8_t _pad_at_0000[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_PRISON_ADD_GM_REQ) == 102, "PROTO_NC_PRISON_ADD_GM_REQ size drift");

struct PROTO_NC_PRISON_ADD_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_PRISON_ADD_REQ) == 8, "PROTO_NC_PRISON_ADD_REQ size drift");

struct PROTO_NC_PRISON_ALTER_GM_ACK {
    Name5 sImprison;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_PRISON_ALTER_GM_ACK) == 22, "PROTO_NC_PRISON_ALTER_GM_ACK size drift");

struct PROTO_NC_PRISON_ALTER_GM_REQ {
    Name5 sImprison;
    uint8_t _pad_at_0000[22];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_PRISON_ALTER_GM_REQ) == 102, "PROTO_NC_PRISON_ALTER_GM_REQ size drift");

struct PROTO_NC_PRISON_END_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PRISON_END_ACK) == 6, "PROTO_NC_PRISON_END_ACK size drift");

struct PROTO_NC_PRISON_END_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PRISON_END_REQ) == 1, "PROTO_NC_PRISON_END_REQ size drift");

struct PROTO_NC_PRISON_GET_ACK {
    uint8_t _pad_at_0000[4];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_PRISON_GET_ACK) == 84, "PROTO_NC_PRISON_GET_ACK size drift");

struct PROTO_NC_PRISON_GET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PRISON_GET_REQ) == 1, "PROTO_NC_PRISON_GET_REQ size drift");

struct PROTO_NC_PRISON_GIVE_UP_FAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_PRISON_GIVE_UP_FAIL_ACK) == 2, "PROTO_NC_PRISON_GIVE_UP_FAIL_ACK size drift");

struct PROTO_NC_PRISON_GIVE_UP_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PRISON_GIVE_UP_REQ) == 1, "PROTO_NC_PRISON_GIVE_UP_REQ size drift");

struct PROTO_NC_PRISON_OK_CMD {
    uint8_t _pad_at_0000[2];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_PRISON_OK_CMD) == 82, "PROTO_NC_PRISON_OK_CMD size drift");

struct PROTO_NC_PRISON_UNDOING_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PRISON_UNDOING_CMD) == 1, "PROTO_NC_PRISON_UNDOING_CMD size drift");

struct PROTO_NC_PROMOTION_DB_REWARD_ACK {
    uint8_t _pad_at_0000[14];
    PROTO_NC_PROMOTION_DB_REWARD_ACK___unnamed_type_RewardList________0_bytes___ RewardList;
};
static_assert(sizeof(PROTO_NC_PROMOTION_DB_REWARD_ACK) == 14, "PROTO_NC_PROMOTION_DB_REWARD_ACK size drift");

struct PROTO_NC_PROMOTION_DB_REWARD_ACK___unnamed_type_RewardList_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PROMOTION_DB_REWARD_ACK___unnamed_type_RewardList_) == 4, "PROTO_NC_PROMOTION_DB_REWARD_ACK___unnamed_type_RewardList_ size drift");

struct PROTO_NC_PROMOTION_DB_REWARD_REQ { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_PROMOTION_DB_REWARD_REQ) == 11, "PROTO_NC_PROMOTION_DB_REWARD_REQ size drift");

struct PROTO_NC_PROMOTION_REWARD_ITEM_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_PROMOTION_REWARD_ITEM_CMD___unnamed_type_RewardList________0_bytes___ RewardList;
};
static_assert(sizeof(PROTO_NC_PROMOTION_REWARD_ITEM_CMD) == 1, "PROTO_NC_PROMOTION_REWARD_ITEM_CMD size drift");

struct PROTO_NC_PROMOTION_REWARD_ITEM_CMD___unnamed_type_RewardList_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_PROMOTION_REWARD_ITEM_CMD___unnamed_type_RewardList_) == 4, "PROTO_NC_PROMOTION_REWARD_ITEM_CMD___unnamed_type_RewardList_ size drift");

struct PROTO_NC_PROMOTION_USER_ACK { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_PROMOTION_USER_ACK) == 9, "PROTO_NC_PROMOTION_USER_ACK size drift");

struct PROTO_NC_PROMOTION_USER_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_PROMOTION_USER_CMD) == 1, "PROTO_NC_PROMOTION_USER_CMD size drift");

struct PROTO_NC_PROMOTION_USER_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_PROMOTION_USER_REQ) == 6, "PROTO_NC_PROMOTION_USER_REQ size drift");

struct PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_ACK) == 2, "PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_ACK size drift");

struct PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_REQ) == 2, "PROTO_NC_QUEST_CLIENT_SCENARIO_DONE_REQ size drift");

struct PROTO_NC_QUEST_DB_CLEAR_ACK {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_CLEAR_ACK) == 10, "PROTO_NC_QUEST_DB_CLEAR_ACK size drift");

struct PROTO_NC_QUEST_DB_CLEAR_REQ {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_CLEAR_REQ) == 8, "PROTO_NC_QUEST_DB_CLEAR_REQ size drift");

struct PROTO_NC_QUEST_DB_DONE_ACK {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _pad_at_0006[1];
    PLAYER_QUEST_INFO QuestInfo;
    uint8_t _tail[36];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_DONE_ACK) == 43, "PROTO_NC_QUEST_DB_DONE_ACK size drift");

struct PROTO_NC_QUEST_DB_DONE_REQ {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _pad_at_0006[7];
    PLAYER_QUEST_INFO QuestInfo;
    uint8_t _pad_at_000d[70];
    PROTO_ITEM_CREATE_______0_bytes___ ItemCreate;
};
static_assert(sizeof(PROTO_NC_QUEST_DB_DONE_REQ) == 83, "PROTO_NC_QUEST_DB_DONE_REQ size drift");

struct PROTO_NC_QUEST_DB_GIVE_UP_ACK {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_GIVE_UP_ACK) == 10, "PROTO_NC_QUEST_DB_GIVE_UP_ACK size drift");

struct PROTO_NC_QUEST_DB_GIVE_UP_REQ {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_GIVE_UP_REQ) == 8, "PROTO_NC_QUEST_DB_GIVE_UP_REQ size drift");

struct PROTO_NC_QUEST_DB_READ_ACK {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_READ_ACK) == 10, "PROTO_NC_QUEST_DB_READ_ACK size drift");

struct PROTO_NC_QUEST_DB_READ_REQ {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_READ_REQ) == 8, "PROTO_NC_QUEST_DB_READ_REQ size drift");

struct PROTO_NC_QUEST_DB_SET_INFO_ACK {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _pad_at_0006[1];
    PLAYER_QUEST_INFO QuestInfo;
    uint8_t _tail[34];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_SET_INFO_ACK) == 41, "PROTO_NC_QUEST_DB_SET_INFO_ACK size drift");

struct PROTO_NC_QUEST_DB_SET_INFO_REQ {
    NETPACKETZONEHEADER ZoneHeader;
    uint8_t _pad_at_0006[1];
    PLAYER_QUEST_INFO QuestInfo;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_QUEST_DB_SET_INFO_REQ) == 39, "PROTO_NC_QUEST_DB_SET_INFO_REQ size drift");

struct PROTO_NC_QUEST_GIVE_UP_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_QUEST_GIVE_UP_ACK) == 4, "PROTO_NC_QUEST_GIVE_UP_ACK size drift");

struct PROTO_NC_QUEST_GIVE_UP_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_GIVE_UP_REQ) == 2, "PROTO_NC_QUEST_GIVE_UP_REQ size drift");

struct PROTO_NC_QUEST_SCRIPT_CMD_ACK { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_QUEST_SCRIPT_CMD_ACK) == 7, "PROTO_NC_QUEST_SCRIPT_CMD_ACK size drift");

struct PROTO_NC_QUEST_JOBDUNGEON_FIND_RNG {
    ZONERINGLINKAGESTART Start;
    uint8_t _pad_at_0000[9];
    wchar_t ClientMapName[33];
    wchar_t ServerMapName[33];
    wchar_t ScriptName[33];
    PROTO_NC_QUEST_SCRIPT_CMD_ACK QuestScript;
};
static_assert(sizeof(PROTO_NC_QUEST_JOBDUNGEON_FIND_RNG) == 115, "PROTO_NC_QUEST_JOBDUNGEON_FIND_RNG size drift");

struct PROTO_NC_QUEST_JOBDUNGEON_LINK_FAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_JOBDUNGEON_LINK_FAIL_CMD) == 2, "PROTO_NC_QUEST_JOBDUNGEON_LINK_FAIL_CMD size drift");

struct PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD__MobOfQuest_______0_bytes___ mobofquest;
};
static_assert(sizeof(PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD) == 1, "PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD size drift");

struct PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD__MobOfQuest { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD__MobOfQuest) == 4, "PROTO_NC_QUEST_NOTIFY_MOB_KILL_CMD__MobOfQuest size drift");

struct PROTO_NC_QUEST_READ_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_QUEST_READ_ACK) == 4, "PROTO_NC_QUEST_READ_ACK size drift");

struct PROTO_NC_QUEST_READ_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_READ_REQ) == 2, "PROTO_NC_QUEST_READ_REQ size drift");

struct PROTO_NC_QUEST_RESET_TIME_CMD { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_QUEST_RESET_TIME_CMD) == 16, "PROTO_NC_QUEST_RESET_TIME_CMD size drift");

struct PROTO_NC_QUEST_RESET_TIME_ZONE_CMD { uint8_t data[20]; };
static_assert(sizeof(PROTO_NC_QUEST_RESET_TIME_ZONE_CMD) == 20, "PROTO_NC_QUEST_RESET_TIME_ZONE_CMD size drift");

struct PROTO_NC_QUEST_REWARD_NEED_SELECT_ITEM_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_REWARD_NEED_SELECT_ITEM_CMD) == 2, "PROTO_NC_QUEST_REWARD_NEED_SELECT_ITEM_CMD size drift");

struct PROTO_NC_QUEST_REWARD_SELECT_ITEM_INDEX_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_QUEST_REWARD_SELECT_ITEM_INDEX_CMD) == 6, "PROTO_NC_QUEST_REWARD_SELECT_ITEM_INDEX_CMD size drift");

struct PROTO_NC_QUEST_SCENARIO_RUN_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_SCENARIO_RUN_CMD) == 2, "PROTO_NC_QUEST_SCENARIO_RUN_CMD size drift");

struct PROTO_NC_QUEST_SCRIPT_CMD_REQ {
    uint8_t _pad_at_0000[2];
    STRUCT_QSC Command;
    uint8_t _tail[101];
};
static_assert(sizeof(PROTO_NC_QUEST_SCRIPT_CMD_REQ) == 103, "PROTO_NC_QUEST_SCRIPT_CMD_REQ size drift");

struct PROTO_NC_QUEST_SELECT_START_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_QUEST_SELECT_START_ACK) == 6, "PROTO_NC_QUEST_SELECT_START_ACK size drift");

struct PROTO_NC_QUEST_SELECT_START_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_QUEST_SELECT_START_REQ) == 4, "PROTO_NC_QUEST_SELECT_START_REQ size drift");

struct PROTO_NC_QUEST_START_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_START_ACK) == 2, "PROTO_NC_QUEST_START_ACK size drift");

struct PROTO_NC_QUEST_START_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_QUEST_START_REQ) == 2, "PROTO_NC_QUEST_START_REQ size drift");

struct PROTO_NC_RAID_CHAT_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_CHAT_ACK) == 2, "PROTO_NC_RAID_CHAT_ACK size drift");

struct PROTO_NC_RAID_CHAT_BROAD_CMD {
    uint8_t _pad_at_0000[1];
    Name5 sTalkerCharID;
    uint8_t _pad_at_0001[21];
    uint8_t sChat[0];
};
static_assert(sizeof(PROTO_NC_RAID_CHAT_BROAD_CMD) == 22, "PROTO_NC_RAID_CHAT_BROAD_CMD size drift");

struct PROTO_NC_RAID_CHAT_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t sChat[0];
};
static_assert(sizeof(PROTO_NC_RAID_CHAT_REQ) == 2, "PROTO_NC_RAID_CHAT_REQ size drift");

struct PROTO_NC_RAID_DEL_SUB_LEADER_ACK {
    Name5 sDelSubLeaderCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_DEL_SUB_LEADER_ACK) == 22, "PROTO_NC_RAID_DEL_SUB_LEADER_ACK size drift");

struct PROTO_NC_RAID_DEL_SUB_LEADER_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    Name5 sDelSubLeaderCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_RAID_DEL_SUB_LEADER_BROAD_CMD) == 44, "PROTO_NC_RAID_DEL_SUB_LEADER_BROAD_CMD size drift");

struct PROTO_NC_RAID_DEL_SUB_LEADER_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_DEL_SUB_LEADER_CMD) == 20, "PROTO_NC_RAID_DEL_SUB_LEADER_CMD size drift");

struct PROTO_NC_RAID_DEL_SUB_LEADER_REQ {
    Name5 sDelSubLeaderCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_DEL_SUB_LEADER_REQ) == 20, "PROTO_NC_RAID_DEL_SUB_LEADER_REQ size drift");

struct PROTO_NC_RAID_DISMISS_BROAD_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_RAID_DISMISS_BROAD_CMD) == 1, "PROTO_NC_RAID_DISMISS_BROAD_CMD size drift");

struct PROTO_NC_RAID_INVITATION_ACK {
    Name5 sInvitationCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_INVITATION_ACK) == 22, "PROTO_NC_RAID_INVITATION_ACK size drift");

struct PROTO_NC_RAID_INVITATION_ANSWER_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_INVITATION_ANSWER_ACK) == 2, "PROTO_NC_RAID_INVITATION_ANSWER_ACK size drift");

struct PROTO_NC_RAID_INVITATION_ANSWER_CMD {
    Name5 sInvitationCharID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_RAID_INVITATION_ANSWER_CMD) == 21, "PROTO_NC_RAID_INVITATION_ANSWER_CMD size drift");

struct PROTO_NC_RAID_INVITATION_ANSWER_REQ {
    uint8_t _pad_at_0000[1];
    Name5 sMasterCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_INVITATION_ANSWER_REQ) == 21, "PROTO_NC_RAID_INVITATION_ANSWER_REQ size drift");

struct PROTO_NC_RAID_INVITATION_CMD {
    Name5 sMasterCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_INVITATION_CMD) == 20, "PROTO_NC_RAID_INVITATION_CMD size drift");

struct PROTO_NC_RAID_INVITATION_REQ {
    Name5 sInvitationCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_INVITATION_REQ) == 20, "PROTO_NC_RAID_INVITATION_REQ size drift");

struct PROTO_NC_RAID_LEAVE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_LEAVE_ACK) == 2, "PROTO_NC_RAID_LEAVE_ACK size drift");

struct PROTO_NC_RAID_LEAVE_BROAD_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_LEAVE_BROAD_CMD) == 24, "PROTO_NC_RAID_LEAVE_BROAD_CMD size drift");

struct PROTO_NC_RAID_LEAVE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_RAID_LEAVE_REQ) == 1, "PROTO_NC_RAID_LEAVE_REQ size drift");

struct PROTO_NC_RAID_LOCATION_CMD {
    uint8_t _pad_at_0000[1];
    PROTO_NC_RAID_LOCATION_CMD__Memberinfo_______0_bytes___ RaidMembers;
};
static_assert(sizeof(PROTO_NC_RAID_LOCATION_CMD) == 1, "PROTO_NC_RAID_LOCATION_CMD size drift");

struct PROTO_NC_RAID_LOCATION_CMD__Memberinfo {
    uint8_t _pad_at_0000[4];
    SHINE_XY_TYPE loc;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_RAID_LOCATION_CMD__Memberinfo) == 12, "PROTO_NC_RAID_LOCATION_CMD__Memberinfo size drift");

struct PROTO_NC_RAID_LOGININFO_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_LOGININFO_CMD) == 24, "PROTO_NC_RAID_LOGININFO_CMD size drift");

struct PROTO_NC_RAID_LOGIN_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_RAID_LOGIN_CMD) == 6, "PROTO_NC_RAID_LOGIN_CMD size drift");

struct PROTO_NC_RAID_LOGOUTINFO_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_LOGOUTINFO_CMD) == 24, "PROTO_NC_RAID_LOGOUTINFO_CMD size drift");

struct PROTO_NC_RAID_LOGOUT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_RAID_LOGOUT_CMD) == 6, "PROTO_NC_RAID_LOGOUT_CMD size drift");

struct PROTO_NC_RAID_LOOTING_TYPE_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_LOOTING_TYPE_ACK) == 2, "PROTO_NC_RAID_LOOTING_TYPE_ACK size drift");

struct PROTO_NC_RAID_LOOTING_TYPE_BROAD_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_LOOTING_TYPE_BROAD_CMD) == 2, "PROTO_NC_RAID_LOOTING_TYPE_BROAD_CMD size drift");

struct PROTO_NC_RAID_LOOTING_TYPE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_LOOTING_TYPE_REQ) == 2, "PROTO_NC_RAID_LOOTING_TYPE_REQ size drift");

struct PROTO_NC_RAID_MAKE_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_MAKE_ACK) == 4, "PROTO_NC_RAID_MAKE_ACK size drift");

struct PROTO_NC_RAID_MAKE_BROAD_CMD {
    uint8_t _pad_at_0000[2];
    uint16_t nRaidPartyNo[4];
    Name5_______100_bytes___ sFirstPartyCharID;
    uint8_t _tail[100];
};
static_assert(sizeof(PROTO_NC_RAID_MAKE_BROAD_CMD) == 110, "PROTO_NC_RAID_MAKE_BROAD_CMD size drift");

struct PROTO_NC_RAID_MAKE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_RAID_MAKE_REQ) == 1, "PROTO_NC_RAID_MAKE_REQ size drift");

struct PROTO_NC_RAID_MAPOUT_CMD {
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_MAPOUT_CMD) == 20, "PROTO_NC_RAID_MAPOUT_CMD size drift");

struct PROTO_NC_RAID_MEMBERCLASS_CMD {
    uint8_t _pad_at_0000[1];
    RAID_MEMBERCLASS_______0_bytes___ RaidMembers;
};
static_assert(sizeof(PROTO_NC_RAID_MEMBERCLASS_CMD) == 1, "PROTO_NC_RAID_MEMBERCLASS_CMD size drift");

struct PROTO_NC_RAID_MEMBERINFORM_CMD { uint8_t data[16]; };
static_assert(sizeof(PROTO_NC_RAID_MEMBERINFORM_CMD) == 16, "PROTO_NC_RAID_MEMBERINFORM_CMD size drift");

struct PROTO_NC_RAID_MEMBER_BROADCAST_INFO_CMD {
    uint8_t _pad_at_0000[1];
    RAID_MEMBERCLASS_______0_bytes___ RaidMembers;
};
static_assert(sizeof(PROTO_NC_RAID_MEMBER_BROADCAST_INFO_CMD) == 1, "PROTO_NC_RAID_MEMBER_BROADCAST_INFO_CMD size drift");

struct PROTO_NC_RAID_MEMBER_JOIN_BROAD_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_MEMBER_JOIN_BROAD_CMD) == 24, "PROTO_NC_RAID_MEMBER_JOIN_BROAD_CMD size drift");

struct PROTO_NC_RAID_MEMBER_LIST_CMD {
    Name5 Leaderid;
    uint8_t _pad_at_0000[20];
    Name5_______80_bytes___ SubLeaderid;
    uint8_t _pad_at_0014[80];
    Name5 Looterid;
    uint8_t _pad_at_0064[22];
    PROTO_NC_RAID_MEMBER_LIST_CMD__MEMBER_INFO_______420_bytes___ memberarray;
    uint8_t _tail[420];
};
static_assert(sizeof(PROTO_NC_RAID_MEMBER_LIST_CMD) == 542, "PROTO_NC_RAID_MEMBER_LIST_CMD size drift");

struct PROTO_NC_RAID_MEMBER_LIST_CMD__MEMBER_INFO {
    Name5 memberid;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_RAID_MEMBER_LIST_CMD__MEMBER_INFO) == 21, "PROTO_NC_RAID_MEMBER_LIST_CMD__MEMBER_INFO size drift");

struct PROTO_NC_RAID_SET_LEADER_ACK {
    Name5 sNewLeaderCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LEADER_ACK) == 22, "PROTO_NC_RAID_SET_LEADER_ACK size drift");

struct PROTO_NC_RAID_SET_LEADER_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    Name5 sNewLeaderCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LEADER_BROAD_CMD) == 44, "PROTO_NC_RAID_SET_LEADER_BROAD_CMD size drift");

struct PROTO_NC_RAID_SET_LEADER_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LEADER_CMD) == 20, "PROTO_NC_RAID_SET_LEADER_CMD size drift");

struct PROTO_NC_RAID_SET_LEADER_REQ {
    Name5 sNewLeaderCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LEADER_REQ) == 20, "PROTO_NC_RAID_SET_LEADER_REQ size drift");

struct PROTO_NC_RAID_SET_LOOTER_ACK {
    Name5 sNewLooterCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LOOTER_ACK) == 22, "PROTO_NC_RAID_SET_LOOTER_ACK size drift");

struct PROTO_NC_RAID_SET_LOOTER_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    Name5 sNewLooterCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LOOTER_BROAD_CMD) == 44, "PROTO_NC_RAID_SET_LOOTER_BROAD_CMD size drift");

struct PROTO_NC_RAID_SET_LOOTER_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LOOTER_CMD) == 20, "PROTO_NC_RAID_SET_LOOTER_CMD size drift");

struct PROTO_NC_RAID_SET_LOOTER_REQ {
    Name5 sNewLooterCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_LOOTER_REQ) == 20, "PROTO_NC_RAID_SET_LOOTER_REQ size drift");

struct PROTO_NC_RAID_SET_SUB_LEADER_ACK {
    Name5 sNewSubLeaderCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_SET_SUB_LEADER_ACK) == 22, "PROTO_NC_RAID_SET_SUB_LEADER_ACK size drift");

struct PROTO_NC_RAID_SET_SUB_LEADER_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    Name5 sNewSubLeaderCharID;
    uint8_t _tail[24];
};
static_assert(sizeof(PROTO_NC_RAID_SET_SUB_LEADER_BROAD_CMD) == 44, "PROTO_NC_RAID_SET_SUB_LEADER_BROAD_CMD size drift");

struct PROTO_NC_RAID_SET_SUB_LEADER_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_SUB_LEADER_CMD) == 20, "PROTO_NC_RAID_SET_SUB_LEADER_CMD size drift");

struct PROTO_NC_RAID_SET_SUB_LEADER_REQ {
    Name5 sNewSubLeaderCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SET_SUB_LEADER_REQ) == 20, "PROTO_NC_RAID_SET_SUB_LEADER_REQ size drift");

struct PROTO_NC_RAID_SLOT_MOVE_ACK {
    Name5 sSlotMoveCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_ACK) == 22, "PROTO_NC_RAID_SLOT_MOVE_ACK size drift");

struct PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PrePosition_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PrePosition_) == 4, "PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PrePosition_ size drift");

struct PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PostPosition_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PostPosition_) == 4, "PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PostPosition_ size drift");

struct PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD {
    Name5 sCommandCharID;
    uint8_t _pad_at_0000[20];
    PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PrePosition_ PrePosition;
    PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD___unnamed_type_PostPosition_ PostPosition;
    Name5_______100_bytes___ sPreCharID;
    uint8_t _pad_at_001c[100];
    Name5_______100_bytes___ sPostCharID;
    uint8_t _tail[100];
};
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD) == 228, "PROTO_NC_RAID_SLOT_MOVE_BROAD_CMD size drift");

struct PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PrePosition_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PrePosition_) == 4, "PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PrePosition_ size drift");

struct PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PostPosition_ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PostPosition_) == 4, "PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PostPosition_ size drift");

struct PROTO_NC_RAID_SLOT_MOVE_REQ {
    PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PrePosition_ PrePosition;
    PROTO_NC_RAID_SLOT_MOVE_REQ___unnamed_type_PostPosition_ PostPosition;
    Name5 sPreSlotMoveCharID;
    uint8_t _pad_at_0008[20];
    Name5 sPostSlotMoveCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_SLOT_MOVE_REQ) == 48, "PROTO_NC_RAID_SLOT_MOVE_REQ size drift");

struct PROTO_NC_RAID_VANISH_ACK {
    Name5 sVanishCharID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_RAID_VANISH_ACK) == 22, "PROTO_NC_RAID_VANISH_ACK size drift");

struct PROTO_NC_RAID_VANISH_BROAD_CMD {
    uint8_t _pad_at_0000[4];
    Name5 sCommandCharID;
    uint8_t _pad_at_0004[20];
    Name5 sVanishCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_VANISH_BROAD_CMD) == 44, "PROTO_NC_RAID_VANISH_BROAD_CMD size drift");

struct PROTO_NC_RAID_VANISH_CMD {
    Name5 sCommandCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_VANISH_CMD) == 20, "PROTO_NC_RAID_VANISH_CMD size drift");

struct PROTO_NC_RAID_VANISH_REQ {
    Name5 sVanishCharID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_RAID_VANISH_REQ) == 20, "PROTO_NC_RAID_VANISH_REQ size drift");

struct PROTO_NC_RAID_WARNING_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_WARNING_ACK) == 2, "PROTO_NC_RAID_WARNING_ACK size drift");

struct PROTO_NC_RAID_WARNING_BROAD_CMD {
    uint8_t _pad_at_0000[1];
    Name5 sTalkerCharID;
    uint8_t _pad_at_0001[21];
    uint8_t sChat[0];
};
static_assert(sizeof(PROTO_NC_RAID_WARNING_BROAD_CMD) == 22, "PROTO_NC_RAID_WARNING_BROAD_CMD size drift");

struct PROTO_NC_RAID_WARNING_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t sChat[0];
};
static_assert(sizeof(PROTO_NC_RAID_WARNING_REQ) == 2, "PROTO_NC_RAID_WARNING_REQ size drift");

struct PROTO_NC_RAID_WORLD_MEMBERCLASS_CMD {
    uint8_t _pad_at_0000[3];
    RAID_MEMBERCLASS_______0_bytes___ RaidMembers;
};
static_assert(sizeof(PROTO_NC_RAID_WORLD_MEMBERCLASS_CMD) == 3, "PROTO_NC_RAID_WORLD_MEMBERCLASS_CMD size drift");

struct PROTO_NC_RAID_ZONE_DEL_SUB_LEADER_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_DEL_SUB_LEADER_CMD) == 10, "PROTO_NC_RAID_ZONE_DEL_SUB_LEADER_CMD size drift");

struct PROTO_NC_RAID_ZONE_DISMISS_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_DISMISS_CMD) == 2, "PROTO_NC_RAID_ZONE_DISMISS_CMD size drift");

struct PROTO_NC_RAID_ZONE_LEAVE_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_LEAVE_CMD) == 10, "PROTO_NC_RAID_ZONE_LEAVE_CMD size drift");

struct PROTO_NC_RAID_ZONE_LOOTING_TYPE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_LOOTING_TYPE_CMD) == 4, "PROTO_NC_RAID_ZONE_LOOTING_TYPE_CMD size drift");

struct PROTO_NC_RAID_ZONE_MAKE_CMD {
    uint8_t _pad_at_0000[4];
    uint16_t nRaidPartyNo[4];
    uint32_t FirstPartyCharRegNo[5];
};
static_assert(sizeof(PROTO_NC_RAID_ZONE_MAKE_CMD) == 32, "PROTO_NC_RAID_ZONE_MAKE_CMD size drift");

struct PROTO_NC_RAID_ZONE_MEMBER_JOIN_CMD {
    uint8_t _pad_at_0000[10];
    PROTO_PARTY_MEMBER_AND_ZONEMERCHINE tInfo;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_RAID_ZONE_MEMBER_JOIN_CMD) == 42, "PROTO_NC_RAID_ZONE_MEMBER_JOIN_CMD size drift");

struct PROTO_NC_RAID_ZONE_SET_LEADER_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_SET_LEADER_CMD) == 10, "PROTO_NC_RAID_ZONE_SET_LEADER_CMD size drift");

struct PROTO_NC_RAID_ZONE_SET_LOOTER_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_SET_LOOTER_CMD) == 10, "PROTO_NC_RAID_ZONE_SET_LOOTER_CMD size drift");

struct PROTO_NC_RAID_ZONE_SET_SUB_LEADER_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_SET_SUB_LEADER_CMD) == 10, "PROTO_NC_RAID_ZONE_SET_SUB_LEADER_CMD size drift");

struct PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PrePosition_ {
    uint8_t _pad_at_0000[4];
    uint32_t nPartySeqCharNo[5];
};
static_assert(sizeof(PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PrePosition_) == 24, "PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PrePosition_ size drift");

struct PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PostPosition_ {
    uint8_t _pad_at_0000[4];
    uint32_t nPartySeqCharNo[5];
};
static_assert(sizeof(PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PostPosition_) == 24, "PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PostPosition_ size drift");

struct PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PrePosition_ PrePosition;
    PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD___unnamed_type_PostPosition_ PostPosition;
};
static_assert(sizeof(PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD) == 50, "PROTO_NC_RAID_ZONE_SLOT_MOVE_CMD size drift");

struct PROTO_NC_RAID_ZONE_VANISH_CMD { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_RAID_ZONE_VANISH_CMD) == 10, "PROTO_NC_RAID_ZONE_VANISH_CMD size drift");

struct PROTO_NC_REPORT_ADD_ACK {
    Name5 sHarmer;
    uint8_t _tail[26];
};
static_assert(sizeof(PROTO_NC_REPORT_ADD_ACK) == 26, "PROTO_NC_REPORT_ADD_ACK size drift");

struct PROTO_NC_REPORT_ADD_REQ {
    Name5 sHarmer;
    uint8_t _pad_at_0000[20];
    uint8_t byReportType[16];
    uint8_t byReason[256];
    uint8_t _pad_at_0124[2];
    uint8_t byChatLog[2048];
};
static_assert(sizeof(PROTO_NC_REPORT_ADD_REQ) == 2342, "PROTO_NC_REPORT_ADD_REQ size drift");

struct PROTO_NC_REPORT_CANCEL_ACK { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_REPORT_CANCEL_ACK) == 6, "PROTO_NC_REPORT_CANCEL_ACK size drift");

struct PROTO_NC_REPORT_CANCEL_REQ { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_REPORT_CANCEL_REQ) == 4, "PROTO_NC_REPORT_CANCEL_REQ size drift");

struct PROTO_NC_REPORT_GET_ACK {
    uint8_t _pad_at_0000[7];
    uint8_t byRemark[128];
};
static_assert(sizeof(PROTO_NC_REPORT_GET_ACK) == 135, "PROTO_NC_REPORT_GET_ACK size drift");

struct PROTO_NC_REPORT_GET_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_REPORT_GET_REQ) == 1, "PROTO_NC_REPORT_GET_REQ size drift");

struct PROTO_NC_SCENARIO_ANIMATE_CMD {
    uint8_t _pad_at_0000[2];
    Name8 anicode;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_ANIMATE_CMD) == 34, "PROTO_NC_SCENARIO_ANIMATE_CMD size drift");

struct PROTO_NC_SCENARIO_ANIMATION_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_SCENARIO_ANIMATION_CMD) == 6, "PROTO_NC_SCENARIO_ANIMATION_CMD size drift");

struct PROTO_NC_SCENARIO_AREAENTRY_ACK {
    Name8 areaindex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_AREAENTRY_ACK) == 32, "PROTO_NC_SCENARIO_AREAENTRY_ACK size drift");

struct PROTO_NC_SCENARIO_AREAENTRY_REQ {
    Name8 areaindex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_AREAENTRY_REQ) == 32, "PROTO_NC_SCENARIO_AREAENTRY_REQ size drift");

struct PROTO_NC_SCENARIO_AREALEAVE_ACK {
    Name8 areaindex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_AREALEAVE_ACK) == 32, "PROTO_NC_SCENARIO_AREALEAVE_ACK size drift");

struct PROTO_NC_SCENARIO_AREALEAVE_REQ {
    Name8 areaindex;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_AREALEAVE_REQ) == 32, "PROTO_NC_SCENARIO_AREALEAVE_REQ size drift");

struct PROTO_NC_SCENARIO_BRIEFEFFECT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SCENARIO_BRIEFEFFECT_CMD) == 1, "PROTO_NC_SCENARIO_BRIEFEFFECT_CMD size drift");

struct PROTO_NC_SCENARIO_CAMERA_MOVE_CMD {
    uint8_t _pad_at_0000[1];
    SHINE_XY_TYPE ViewCoord;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_SCENARIO_CAMERA_MOVE_CMD) == 15, "PROTO_NC_SCENARIO_CAMERA_MOVE_CMD size drift");

struct PROTO_NC_SCENARIO_CHATWIN_CMD {
    uint8_t _pad_at_0000[3];
    uint8_t content[0];
};
static_assert(sizeof(PROTO_NC_SCENARIO_CHATWIN_CMD) == 3, "PROTO_NC_SCENARIO_CHATWIN_CMD size drift");

struct PROTO_NC_SCENARIO_DEL_DIRECTIONAL_ARROW_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SCENARIO_DEL_DIRECTIONAL_ARROW_CMD) == 1, "PROTO_NC_SCENARIO_DEL_DIRECTIONAL_ARROW_CMD size drift");

struct PROTO_NC_SCENARIO_DIALOG_CMD {
    wchar_t sFacecutIndex[33];
    uint8_t sContents[0];
};
static_assert(sizeof(PROTO_NC_SCENARIO_DIALOG_CMD) == 34, "PROTO_NC_SCENARIO_DIALOG_CMD size drift");

struct PROTO_NC_SCENARIO_DIRECTIONAL_ARROW_CMD {
    SHINE_XY_TYPE DirectionalPos;
    uint8_t _tail[8];
};
static_assert(sizeof(PROTO_NC_SCENARIO_DIRECTIONAL_ARROW_CMD) == 8, "PROTO_NC_SCENARIO_DIRECTIONAL_ARROW_CMD size drift");

struct PROTO_NC_SCENARIO_DOORSTATE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SCENARIO_DOORSTATE_CMD) == 3, "PROTO_NC_SCENARIO_DOORSTATE_CMD size drift");

struct PROTO_NC_SCENARIO_EFFECT_LOCATE_CMD {
    uint8_t _pad_at_0000[1];
    SHINE_XY_TYPE loc;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_SCENARIO_EFFECT_LOCATE_CMD) == 13, "PROTO_NC_SCENARIO_EFFECT_LOCATE_CMD size drift");

struct PROTO_NC_SCENARIO_EFFECT_OBJECT_CMD {
    Name8 effect;
    uint8_t _tail[38];
};
static_assert(sizeof(PROTO_NC_SCENARIO_EFFECT_OBJECT_CMD) == 38, "PROTO_NC_SCENARIO_EFFECT_OBJECT_CMD size drift");

struct PROTO_NC_SCENARIO_EFFECT_TIMER_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SCENARIO_EFFECT_TIMER_CMD) == 3, "PROTO_NC_SCENARIO_EFFECT_TIMER_CMD size drift");

struct PROTO_NC_SCENARIO_FOG_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SCENARIO_FOG_CMD) == 3, "PROTO_NC_SCENARIO_FOG_CMD size drift");

struct PROTO_NC_SCENARIO_HIDE_OTHER_PLAYER_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SCENARIO_HIDE_OTHER_PLAYER_CMD) == 1, "PROTO_NC_SCENARIO_HIDE_OTHER_PLAYER_CMD size drift");

struct PROTO_NC_SCENARIO_LIGHT_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SCENARIO_LIGHT_CMD) == 3, "PROTO_NC_SCENARIO_LIGHT_CMD size drift");

struct PROTO_NC_SCENARIO_MAPMARK_CMD {
    uint8_t _pad_at_0000[2];
    PROTO_NC_SCENARIO_MAPMARK_CMD__IconInfo_______0_bytes___ IconList;
};
static_assert(sizeof(PROTO_NC_SCENARIO_MAPMARK_CMD) == 2, "PROTO_NC_SCENARIO_MAPMARK_CMD size drift");

struct PROTO_NC_SCENARIO_MAPMARK_CMD__IconInfo {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE nCoord;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_SCENARIO_MAPMARK_CMD__IconInfo) == 15, "PROTO_NC_SCENARIO_MAPMARK_CMD__IconInfo size drift");

struct PROTO_NC_SCENARIO_MAP_OBJECT_CONTROL_CMD {
    wchar_t sObjectIndex[32];
    MAP_OBJECT_CONTROL_TYPE eControlType;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_SCENARIO_MAP_OBJECT_CONTROL_CMD) == 37, "PROTO_NC_SCENARIO_MAP_OBJECT_CONTROL_CMD size drift");

struct PROTO_NC_SCENARIO_MESSAGENOTICE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_SCENARIO_MESSAGENOTICE_CMD) == 4, "PROTO_NC_SCENARIO_MESSAGENOTICE_CMD size drift");

struct PROTO_NC_SCENARIO_MESSAGENPC_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_SCENARIO_MESSAGENPC_CMD) == 6, "PROTO_NC_SCENARIO_MESSAGENPC_CMD size drift");

struct PROTO_NC_SCENARIO_MESSAGE_CMD {
    wchar_t DialogIndex[17];
};
static_assert(sizeof(PROTO_NC_SCENARIO_MESSAGE_CMD) == 17, "PROTO_NC_SCENARIO_MESSAGE_CMD size drift");

struct PROTO_NC_SCENARIO_NPCACT_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_SCENARIO_NPCACT_CMD) == 7, "PROTO_NC_SCENARIO_NPCACT_CMD size drift");

struct PROTO_NC_SCENARIO_NPCCHAT_CMD {
    wchar_t DialogIndex[33];
};
static_assert(sizeof(PROTO_NC_SCENARIO_NPCCHAT_CMD) == 35, "PROTO_NC_SCENARIO_NPCCHAT_CMD size drift");

struct PROTO_NC_SCENARIO_OBJTYPECHANGE_CMD { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SCENARIO_OBJTYPECHANGE_CMD) == 3, "PROTO_NC_SCENARIO_OBJTYPECHANGE_CMD size drift");

struct PROTO_NC_SCENARIO_PLAYERLIKEMOB_ANIMATE_CMD { uint8_t data[7]; };
static_assert(sizeof(PROTO_NC_SCENARIO_PLAYERLIKEMOB_ANIMATE_CMD) == 7, "PROTO_NC_SCENARIO_PLAYERLIKEMOB_ANIMATE_CMD size drift");

struct PROTO_NC_SCENARIO_RANKING_LIST_CMD {
    SCENARIO_RANKING_INFO_______500_bytes___ PreviousRanking;
    uint8_t _pad_at_0000[500];
    SCENARIO_RANKING_INFO_______500_bytes___ CurrentRanking;
    uint8_t _tail[500];
};
static_assert(sizeof(PROTO_NC_SCENARIO_RANKING_LIST_CMD) == 1000, "PROTO_NC_SCENARIO_RANKING_LIST_CMD size drift");

struct PROTO_NC_SCENARIO_RUNEFFECT_CMD {
    Name4 effecname;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_SCENARIO_RUNEFFECT_CMD) == 32, "PROTO_NC_SCENARIO_RUNEFFECT_CMD size drift");

struct PROTO_NC_SCENARIO_SCRIPTMESSAGE_CMD {
    wchar_t DialogIndex[32];
    uint8_t nlen[5];
    uint8_t sContent[0];
};
static_assert(sizeof(PROTO_NC_SCENARIO_SCRIPTMESSAGE_CMD) == 37, "PROTO_NC_SCENARIO_SCRIPTMESSAGE_CMD size drift");

struct PROTO_NC_SCENARIO_SOUND_CMD { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_SCENARIO_SOUND_CMD) == 5, "PROTO_NC_SCENARIO_SOUND_CMD size drift");

struct PROTO_NC_SCENARIO_SYSTEM_MESSAGE_CMD {
    uint8_t _pad_at_0000[1];
    uint8_t sMessage[0];
};
static_assert(sizeof(PROTO_NC_SCENARIO_SYSTEM_MESSAGE_CMD) == 1, "PROTO_NC_SCENARIO_SYSTEM_MESSAGE_CMD size drift");

struct PROTO_NC_SCENARIO_TIMER_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SCENARIO_TIMER_CMD) == 2, "PROTO_NC_SCENARIO_TIMER_CMD size drift");

struct PROTO_NC_SCENARIO_TIMER_END_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_SCENARIO_TIMER_END_CMD) == 4, "PROTO_NC_SCENARIO_TIMER_END_CMD size drift");

struct PROTO_NC_SCENARIO_TIMER_START_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SCENARIO_TIMER_START_CMD) == 1, "PROTO_NC_SCENARIO_TIMER_START_CMD size drift");

struct PROTO_NC_SCENARIO_TOPVIEW_CMD {
    uint8_t _pad_at_0000[1];
    SHINE_XY_TYPE CenterPos;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_SCENARIO_TOPVIEW_CMD) == 13, "PROTO_NC_SCENARIO_TOPVIEW_CMD size drift");

struct PROTO_NC_SKILL_COOLTIME_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_SKILL_COOLTIME_CMD) == 6, "PROTO_NC_SKILL_COOLTIME_CMD size drift");

struct PROTO_NC_SKILL_EMPOWALLOC_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_SKILL_EMPOWALLOC_ACK) == 4, "PROTO_NC_SKILL_EMPOWALLOC_ACK size drift");

struct PROTO_NC_SKILL_EMPOWALLOC_DB_ACK { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_SKILL_EMPOWALLOC_DB_ACK) == 13, "PROTO_NC_SKILL_EMPOWALLOC_DB_ACK size drift");

struct PROTO_NC_SKILL_EMPOWALLOC_DB_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_SKILL_EMPOWALLOC_DB_ACK ack;
};
static_assert(sizeof(PROTO_NC_SKILL_EMPOWALLOC_DB_ACK_SEND) == 16, "PROTO_NC_SKILL_EMPOWALLOC_DB_ACK_SEND size drift");

struct PROTO_NC_SKILL_EMPOWALLOC_REQ {
    uint8_t _pad_at_0000[2];
    SKILL_EMPOWER plus;
    uint8_t _pad_at_0002[2];
    SKILL_EMPOWER minus;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_SKILL_EMPOWALLOC_REQ) == 6, "PROTO_NC_SKILL_EMPOWALLOC_REQ size drift");

struct PROTO_NC_SKILL_EMPOWALLOC_DB_REQ {
    uint8_t _pad_at_0000[11];
    PROTO_NC_SKILL_EMPOWALLOC_REQ empower;
    PROTO_ITEMDELETEREQUEST redistitem;
    uint8_t _tail[23];
};
static_assert(sizeof(PROTO_NC_SKILL_EMPOWALLOC_DB_REQ) == 40, "PROTO_NC_SKILL_EMPOWALLOC_DB_REQ size drift");

struct PROTO_NC_SKILL_EMPOWPOINT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SKILL_EMPOWPOINT_CMD) == 1, "PROTO_NC_SKILL_EMPOWPOINT_CMD size drift");

struct PROTO_NC_SKILL_EMPOW_RESET_DB_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_SKILL_EMPOW_RESET_DB_REQ) == 6, "PROTO_NC_SKILL_EMPOW_RESET_DB_REQ size drift");

struct PROTO_NC_SKILL_EMPOW_RESET_DB_FAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_SKILL_EMPOW_RESET_DB_REQ ack;
};
static_assert(sizeof(PROTO_NC_SKILL_EMPOW_RESET_DB_FAIL_ACK_SEND) == 9, "PROTO_NC_SKILL_EMPOW_RESET_DB_FAIL_ACK_SEND size drift");

struct PROTO_NC_SKILL_EMPOW_RESET_DB_SUC_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_SKILL_EMPOW_RESET_DB_REQ ack;
};
static_assert(sizeof(PROTO_NC_SKILL_EMPOW_RESET_DB_SUC_ACK_SEND) == 9, "PROTO_NC_SKILL_EMPOW_RESET_DB_SUC_ACK_SEND size drift");

struct PROTO_NC_SKILL_EMPOW_RESET_SUC_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SKILL_EMPOW_RESET_SUC_ACK) == 1, "PROTO_NC_SKILL_EMPOW_RESET_SUC_ACK size drift");

struct PROTO_NC_SKILL_ERASE_ACK {
    NETPACKETZONEHEADER header;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_SKILL_ERASE_ACK) == 10, "PROTO_NC_SKILL_ERASE_ACK size drift");

struct PROTO_NC_SKILL_ERASE_REQ {
    NETPACKETZONEHEADER header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_SKILL_ERASE_REQ) == 8, "PROTO_NC_SKILL_ERASE_REQ size drift");

struct PROTO_NC_SKILL_JUMP_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE From;
    uint8_t _pad_at_0002[8];
    SHINE_XY_TYPE To;
    uint8_t _tail[11];
};
static_assert(sizeof(PROTO_NC_SKILL_JUMP_CMD) == 21, "PROTO_NC_SKILL_JUMP_CMD size drift");

struct PROTO_NC_SKILL_PASSIVESKILL_LEARN_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_SKILL_PASSIVESKILL_LEARN_CMD) == 8, "PROTO_NC_SKILL_PASSIVESKILL_LEARN_CMD size drift");

struct PROTO_NC_SKILL_PRODUCTFIELD_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_SKILL_PRODUCTFIELD_ACK) == 4, "PROTO_NC_SKILL_PRODUCTFIELD_ACK size drift");

struct PROTO_NC_SKILL_PRODUCTFIELD_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SKILL_PRODUCTFIELD_REQ) == 2, "PROTO_NC_SKILL_PRODUCTFIELD_REQ size drift");

struct PROTO_NC_SKILL_REPLYREVIVE_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SKILL_REPLYREVIVE_CMD) == 1, "PROTO_NC_SKILL_REPLYREVIVE_CMD size drift");

struct PROTO_NC_SKILL_RESETABSTATE_CMD {
    ABSTATEINDEX abstateid;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_SKILL_RESETABSTATE_CMD) == 4, "PROTO_NC_SKILL_RESETABSTATE_CMD size drift");

struct PROTO_NC_SKILL_REVIVE_CMD { uint8_t data[12]; };
static_assert(sizeof(PROTO_NC_SKILL_REVIVE_CMD) == 12, "PROTO_NC_SKILL_REVIVE_CMD size drift");

struct PROTO_NC_SKILL_SKILLEXP_CLIENT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_SKILL_SKILLEXP_CLIENT_CMD) == 6, "PROTO_NC_SKILL_SKILLEXP_CLIENT_CMD size drift");

struct PROTO_NC_SKILL_SKILLTEACHFAIL_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SKILL_SKILLTEACHFAIL_ACK) == 1, "PROTO_NC_SKILL_SKILLTEACHFAIL_ACK size drift");

struct PROTO_NC_SKILL_SKILLTEACHSUC_ACK { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SKILL_SKILLTEACHSUC_ACK) == 3, "PROTO_NC_SKILL_SKILLTEACHSUC_ACK size drift");

struct PROTO_NC_SKILL_SKILLTEACH_REQ { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_SKILL_SKILLTEACH_REQ) == 9, "PROTO_NC_SKILL_SKILLTEACH_REQ size drift");

struct PROTO_NC_SKILL_SOMEONERESETABSTATE_CMD {
    uint8_t _pad_at_0000[2];
    ABSTATEINDEX abstateid;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_SKILL_SOMEONERESETABSTATE_CMD) == 6, "PROTO_NC_SKILL_SOMEONERESETABSTATE_CMD size drift");

struct PROTO_NC_SKILL_SOMEONEREVAVALTOME_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SKILL_SOMEONEREVAVALTOME_CMD) == 1, "PROTO_NC_SKILL_SOMEONEREVAVALTOME_CMD size drift");

struct PROTO_NC_SKILL_SOMEONEREVIVE_CMD { uint8_t data[14]; };
static_assert(sizeof(PROTO_NC_SKILL_SOMEONEREVIVE_CMD) == 14, "PROTO_NC_SKILL_SOMEONEREVIVE_CMD size drift");

struct PROTO_NC_SKILL_SOMEONESETABSTATE_CMD {
    uint8_t _pad_at_0000[2];
    ABSTATEINDEX abstateid;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_SKILL_SOMEONESETABSTATE_CMD) == 6, "PROTO_NC_SKILL_SOMEONESETABSTATE_CMD size drift");

struct PROTO_NC_SKILL_UNLEARN_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_SKILL_UNLEARN_ACK) == 4, "PROTO_NC_SKILL_UNLEARN_ACK size drift");

struct PROTO_NC_SKILL_UNLEARN_REQ { uint8_t data[3]; };
static_assert(sizeof(PROTO_NC_SKILL_UNLEARN_REQ) == 3, "PROTO_NC_SKILL_UNLEARN_REQ size drift");

struct PROTO_NC_SKILL_WARP_CMD {
    uint8_t _pad_at_0000[2];
    SHINE_XY_TYPE from;
    uint8_t _pad_at_0002[8];
    SHINE_XY_TYPE to;
    uint8_t _tail[10];
};
static_assert(sizeof(PROTO_NC_SKILL_WARP_CMD) == 20, "PROTO_NC_SKILL_WARP_CMD size drift");

struct PROTO_NC_SOULSTONE_BUYFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_BUYFAIL_ACK) == 2, "PROTO_NC_SOULSTONE_BUYFAIL_ACK size drift");

struct PROTO_NC_SOULSTONE_HP_BUY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_HP_BUY_ACK) == 2, "PROTO_NC_SOULSTONE_HP_BUY_ACK size drift");

struct PROTO_NC_SOULSTONE_HP_BUY_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_HP_BUY_REQ) == 2, "PROTO_NC_SOULSTONE_HP_BUY_REQ size drift");

struct PROTO_NC_SOULSTONE_HP_SOMEONEUSE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_HP_SOMEONEUSE_CMD) == 2, "PROTO_NC_SOULSTONE_HP_SOMEONEUSE_CMD size drift");

struct PROTO_NC_SOULSTONE_SP_BUY_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_SP_BUY_ACK) == 2, "PROTO_NC_SOULSTONE_SP_BUY_ACK size drift");

struct PROTO_NC_SOULSTONE_SP_BUY_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_SP_BUY_REQ) == 2, "PROTO_NC_SOULSTONE_SP_BUY_REQ size drift");

struct PROTO_NC_SOULSTONE_SP_SOMEONEUSE_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_SOULSTONE_SP_SOMEONEUSE_CMD) == 2, "PROTO_NC_SOULSTONE_SP_SOMEONEUSE_CMD size drift");

struct PROTO_NC_SYSLOG_ACCOUNT_LOGIN_FAILURE {
    wchar_t UserName[260];
    Name4 UserIP;
    uint8_t _pad_at_0104[16];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_SYSLOG_ACCOUNT_LOGIN_FAILURE) == 296, "PROTO_NC_SYSLOG_ACCOUNT_LOGIN_FAILURE size drift");

struct PROTO_NC_SYSLOG_ACCOUNT_LOGIN_SUCCESS {
    uint8_t _pad_at_0000[4];
    Name4 UserIP;
    uint8_t _pad_at_0004[16];
    wchar_t UserID[30];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_SYSLOG_ACCOUNT_LOGIN_SUCCESS) == 70, "PROTO_NC_SYSLOG_ACCOUNT_LOGIN_SUCCESS size drift");

struct PROTO_NC_SYSLOG_ACCOUNT_LOGOUT { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_SYSLOG_ACCOUNT_LOGOUT) == 8, "PROTO_NC_SYSLOG_ACCOUNT_LOGOUT size drift");

struct PROTO_NC_SYSLOG_CHAR_CREATED {
    uint8_t _pad_at_0000[9];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_CREATED) == 43, "PROTO_NC_SYSLOG_CHAR_CREATED size drift");

struct PROTO_NC_SYSLOG_CHAR_DEATH {
    uint8_t _pad_at_0000[14];
    SHINE_XY_TYPE DeathCoord;
    uint8_t _pad_at_000e[8];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_DEATH) == 60, "PROTO_NC_SYSLOG_CHAR_DEATH size drift");

struct PROTO_NC_SYSLOG_CHAR_DELETED {
    uint8_t _pad_at_0000[9];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_DELETED) == 43, "PROTO_NC_SYSLOG_CHAR_DELETED size drift");

struct PROTO_NC_SYSLOG_CHAR_ENTER_GAME {
    uint8_t _pad_at_0000[8];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ENTER_GAME) == 38, "PROTO_NC_SYSLOG_CHAR_ENTER_GAME size drift");

struct PROTO_NC_SYSLOG_CHAR_ITEMMONEY_BUY { uint8_t data[22]; };
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ITEMMONEY_BUY) == 22, "PROTO_NC_SYSLOG_CHAR_ITEMMONEY_BUY size drift");

struct PROTO_NC_SYSLOG_CHAR_ITEM_BUY {
    uint8_t _pad_at_0000[28];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ITEM_BUY) == 62, "PROTO_NC_SYSLOG_CHAR_ITEM_BUY size drift");

struct PROTO_NC_SYSLOG_CHAR_ITEM_REBUY {
    uint8_t _pad_at_0000[28];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ITEM_REBUY) == 62, "PROTO_NC_SYSLOG_CHAR_ITEM_REBUY size drift");

struct PROTO_NC_SYSLOG_CHAR_ITEM_SELL {
    uint8_t _pad_at_0000[28];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ITEM_SELL) == 62, "PROTO_NC_SYSLOG_CHAR_ITEM_SELL size drift");

struct PROTO_NC_SYSLOG_CHAR_LEAVE_GAME {
    uint8_t _pad_at_0000[12];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_LEAVE_GAME) == 46, "PROTO_NC_SYSLOG_CHAR_LEAVE_GAME size drift");

struct PROTO_NC_SYSLOG_CHAR_LEVEL_UP {
    uint8_t _pad_at_0000[9];
    wchar_t UserID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_LEVEL_UP) == 39, "PROTO_NC_SYSLOG_CHAR_LEVEL_UP size drift");

struct PROTO_NC_SYSLOG_CHAR_LOOT { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_LOOT) == 1, "PROTO_NC_SYSLOG_CHAR_LOOT size drift");

struct PROTO_NC_SYSLOG_CHAR_QUEST_FINISHED { uint8_t data[30]; };
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_QUEST_FINISHED) == 30, "PROTO_NC_SYSLOG_CHAR_QUEST_FINISHED size drift");

struct PROTO_NC_SYSLOG_CHAR_QUEST_STARTED { uint8_t data[13]; };
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_QUEST_STARTED) == 13, "PROTO_NC_SYSLOG_CHAR_QUEST_STARTED size drift");

struct PROTO_NC_SYSLOG_CHAR_VICTORY {
    uint8_t _pad_at_0000[14];
    SHINE_XY_TYPE KillCoord;
    uint8_t _pad_at_000e[8];
    wchar_t CharID[30];
};
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_VICTORY) == 56, "PROTO_NC_SYSLOG_CHAR_VICTORY size drift");

struct PROTO_NC_SYSLOG_CHAR_ZONE_TRANSITION { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_SYSLOG_CHAR_ZONE_TRANSITION) == 11, "PROTO_NC_SYSLOG_CHAR_ZONE_TRANSITION size drift");

struct PROTO_NC_SYSLOG_SERVER_CCU { uint8_t data[5]; };
static_assert(sizeof(PROTO_NC_SYSLOG_SERVER_CCU) == 5, "PROTO_NC_SYSLOG_SERVER_CCU size drift");

struct PROTO_NC_TRADE_CENBOARDINGFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_CENBOARDINGFAIL_ACK) == 2, "PROTO_NC_TRADE_CENBOARDINGFAIL_ACK size drift");

struct PROTO_NC_TRADE_CENBOARDING_ACK { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_TRADE_CENBOARDING_ACK) == 8, "PROTO_NC_TRADE_CENBOARDING_ACK size drift");

struct PROTO_NC_TRADE_CENBOARDING_REQ { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_TRADE_CENBOARDING_REQ) == 8, "PROTO_NC_TRADE_CENBOARDING_REQ size drift");

struct PROTO_NC_TRADE_DOWNBOARDFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_DOWNBOARDFAIL_ACK) == 2, "PROTO_NC_TRADE_DOWNBOARDFAIL_ACK size drift");

struct PROTO_NC_TRADE_DOWNBOARD_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_TRADE_DOWNBOARD_ACK) == 1, "PROTO_NC_TRADE_DOWNBOARD_ACK size drift");

struct PROTO_NC_TRADE_DOWNBOARD_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_TRADE_DOWNBOARD_REQ) == 1, "PROTO_NC_TRADE_DOWNBOARD_REQ size drift");

struct PROTO_NC_TRADE_OPPOSITCENBOARDING_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_TRADE_OPPOSITCENBOARDING_CMD) == 8, "PROTO_NC_TRADE_OPPOSITCENBOARDING_CMD size drift");

struct PROTO_NC_TRADE_OPPOSITDOWNBOARD_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_TRADE_OPPOSITDOWNBOARD_CMD) == 1, "PROTO_NC_TRADE_OPPOSITDOWNBOARD_CMD size drift");

struct PROTO_NC_TRADE_OPPOSITUPBOARD_CMD {
    uint8_t _pad_at_0000[1];
    SHINE_ITEM_STRUCT iteminfo;
    uint8_t _tail[103];
};
static_assert(sizeof(PROTO_NC_TRADE_OPPOSITUPBOARD_CMD) == 104, "PROTO_NC_TRADE_OPPOSITUPBOARD_CMD size drift");

struct PROTO_NC_TRADE_PROPOSE_ASK_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_PROPOSE_ASK_REQ) == 2, "PROTO_NC_TRADE_PROPOSE_ASK_REQ size drift");

struct PROTO_NC_TRADE_PROPOSE_REQ { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_PROPOSE_REQ) == 2, "PROTO_NC_TRADE_PROPOSE_REQ size drift");

struct PROTO_NC_TRADE_START_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_START_CMD) == 2, "PROTO_NC_TRADE_START_CMD size drift");

struct PROTO_NC_TRADE_TRADEFAIL_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_TRADEFAIL_CMD) == 2, "PROTO_NC_TRADE_TRADEFAIL_CMD size drift");

struct PROTO_NC_TRADE_UPBOARDFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_UPBOARDFAIL_ACK) == 2, "PROTO_NC_TRADE_UPBOARDFAIL_ACK size drift");

struct PROTO_NC_TRADE_UPBOARD_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_TRADE_UPBOARD_ACK) == 2, "PROTO_NC_TRADE_UPBOARD_ACK size drift");

struct PROTO_NC_TRADE_UPBOARD_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_TRADE_UPBOARD_REQ) == 1, "PROTO_NC_TRADE_UPBOARD_REQ size drift");

struct PROTO_NC_USER_AVATARINFO_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_AVATARINFORMATION_______0_bytes___ avatar;
};
static_assert(sizeof(PROTO_NC_USER_AVATARINFO_ACK) == 1, "PROTO_NC_USER_AVATARINFO_ACK size drift");

struct PROTO_NC_USER_AVATARINFO_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_USER_AVATARINFO_REQ) == 6, "PROTO_NC_USER_AVATARINFO_REQ size drift");

struct PROTO_NC_USER_AVATARINFO_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_AVATARINFO_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_AVATARINFO_REQ_SEND) == 9, "PROTO_NC_USER_AVATARINFO_REQ_SEND size drift");

struct PROTO_NC_USER_AVATAR_LIST_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_AVATAR_LIST_REQ) == 1, "PROTO_NC_USER_AVATAR_LIST_REQ size drift");

struct PROTO_NC_USER_CH_LOGIN_REQ {
    Name256Byte user;
    uint8_t _pad_at_0000[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_CH_LOGIN_REQ) == 272, "PROTO_NC_USER_CH_LOGIN_REQ size drift");

struct PROTO_NC_USER_CH_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_CH_LOGIN_REQ LoginData;
    uint8_t _pad_at_0110[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_CH_IS_IP_BLOCK_REQ) == 298, "PROTO_NC_USER_CH_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_CH_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    Name256Byte id;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_USER_CH_PASSWORD_CHECK_ACK) == 287, "PROTO_NC_USER_CH_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_CH_PASSWORD_CHECK_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_CH_PASSWORD_CHECK_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_CH_PASSWORD_CHECK_ACK_SEND) == 290, "PROTO_NC_USER_CH_PASSWORD_CHECK_ACK_SEND size drift");

struct PROTO_NC_USER_CH_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    Name256Byte user;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_CH_PASSWORD_CHECK_REQ) == 274, "PROTO_NC_USER_CH_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_CH_PASSWORD_CHECK_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_CH_PASSWORD_CHECK_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_CH_PASSWORD_CHECK_REQ_SEND) == 277, "PROTO_NC_USER_CH_PASSWORD_CHECK_REQ_SEND size drift");

struct PROTO_NC_USER_CLIENT_RIGHTVERSION_CHECK_ACK {
    uint8_t _pad_at_0000[1];
    uint8_t XTrapServerKey[0];
};
static_assert(sizeof(PROTO_NC_USER_CLIENT_RIGHTVERSION_CHECK_ACK) == 1, "PROTO_NC_USER_CLIENT_RIGHTVERSION_CHECK_ACK size drift");

struct PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ {
    wchar_t sVersionKey[64];
};
static_assert(sizeof(PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ) == 64, "PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ size drift");

struct PROTO_NC_USER_CLIENT_WRONGVERSION_CHECK_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_CLIENT_WRONGVERSION_CHECK_ACK) == 1, "PROTO_NC_USER_CLIENT_WRONGVERSION_CHECK_ACK size drift");

struct PROTO_NC_USER_CONNECTCUT2WORLDMANAGER_CMD { uint8_t data[8]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTCUT2WORLDMANAGER_CMD) == 8, "PROTO_NC_USER_CONNECTCUT2WORLDMANAGER_CMD size drift");

struct PROTO_NC_USER_CONNECTCUT2ZONE_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTCUT2ZONE_CMD) == 4, "PROTO_NC_USER_CONNECTCUT2ZONE_CMD size drift");

struct PROTO_NC_USER_CONNECTCUT2ZONE_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER netpacketzoneheader;
    PROTO_NC_USER_CONNECTCUT2ZONE_CMD cmd;
};
static_assert(sizeof(PROTO_NC_USER_CONNECTCUT2ZONE_CMD_SEND) == 13, "PROTO_NC_USER_CONNECTCUT2ZONE_CMD_SEND size drift");

struct PROTO_NC_USER_CONNECTCUT_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTCUT_CMD) == 2, "PROTO_NC_USER_CONNECTCUT_CMD size drift");

struct PROTO_NC_USER_CONNECTCUT_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_CONNECTCUT_CMD cmd;
};
static_assert(sizeof(PROTO_NC_USER_CONNECTCUT_CMD_SEND) == 5, "PROTO_NC_USER_CONNECTCUT_CMD_SEND size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACADEMY_MASTER_CMD {
    Name5 sMasterID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACADEMY_MASTER_CMD) == 20, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACADEMY_MASTER_CMD size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACK {
    uint8_t _pad_at_0000[3];
    Name5 sNewID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACK) == 23, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ACK size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_CMD {
    uint8_t _pad_at_0000[1];
    Name5 sOldID;
    uint8_t _pad_at_0001[20];
    Name5 sNewID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_CMD) == 41, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_CMD size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_ACK) == 4, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_ACK size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_REQ) == 1, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_ITEM_USE_REQ size drift");

struct PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_REQ {
    uint8_t _pad_at_0000[1];
    Name5 sNewID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_REQ) == 21, "PROTO_NC_USER_CONNECTION_CHANGE_CHAR_ID_REQ size drift");

struct PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ACK {
    uint8_t _pad_at_0000[12];
    Name5 sNewID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ACK) == 32, "PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ACK size drift");

struct PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_ACK) == 12, "PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_ACK size drift");

struct PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_REQ {
    SHINE_ITEM_REGISTNUMBER nCharIDChangeItemKey;
    uint8_t _pad_at_0000[8];
    ITEM_INVEN nCharIDChangeItemSlot;
    uint8_t _pad_at_0008[9];
    PROTO_NC_ITEMDB_USE_VARIATION_ITEM_REQ CommonData;
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_REQ) == 38, "PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_ITEM_USE_REQ size drift");

struct PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_REQ {
    uint8_t _pad_at_0000[14];
    Name5 sNewID;
    uint8_t _tail[21];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_REQ) == 35, "PROTO_NC_USER_CONNECTION_DB_CHANGE_CHAR_ID_REQ size drift");

struct PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_ALL_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_ALL_CMD) == 1, "PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_ALL_CMD size drift");

struct PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_CMD {
    uint8_t _pad_at_0000[12];
    Name5 sOldID;
    uint8_t _pad_at_000c[20];
    Name5 sNewID;
    uint8_t _tail[22];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_CMD) == 54, "PROTO_NC_USER_CONNECTION_SET_CHANGE_CHAR_ID_FLAG_CMD size drift");

struct PROTO_NC_USER_CONNECTION_ZONE_CHANGE_CHAR_ID_CMD {
    uint8_t _pad_at_0000[5];
    Name5 sOldID;
    uint8_t _pad_at_0005[20];
    Name5 sNewID;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_CONNECTION_ZONE_CHANGE_CHAR_ID_CMD) == 45, "PROTO_NC_USER_CONNECTION_ZONE_CHANGE_CHAR_ID_CMD size drift");

struct PROTO_NC_USER_CREATE_OTP_ACK {
    uint8_t _pad_at_0000[4];
    Name8 sOTP;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_USER_CREATE_OTP_ACK) == 36, "PROTO_NC_USER_CREATE_OTP_ACK size drift");

struct PROTO_NC_USER_CREATE_OTP_REQ {
    uint8_t _pad_at_0000[6];
    Name256Byte sUserID;
    uint8_t _pad_at_0006[256];
    in_addr nClientIP;
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_USER_CREATE_OTP_REQ) == 266, "PROTO_NC_USER_CREATE_OTP_REQ size drift");

struct PROTO_NC_USER_GER_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    Name256Byte id;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_USER_GER_PASSWORD_CHECK_ACK) == 287, "PROTO_NC_USER_GER_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_GER_IS_IP_BLOCK_ACK {
    PROTO_NC_USER_GER_PASSWORD_CHECK_ACK LoginData;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_USER_GER_IS_IP_BLOCK_ACK) == 290, "PROTO_NC_USER_GER_IS_IP_BLOCK_ACK size drift");

struct PROTO_NC_USER_GER_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_GER_PASSWORD_CHECK_ACK LoginData;
    uint8_t _pad_at_011f[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_GER_IS_IP_BLOCK_REQ) == 313, "PROTO_NC_USER_GER_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_GER_LOGIN_REQ {
    Name256Byte user;
    uint8_t _pad_at_0000[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_GER_LOGIN_REQ) == 272, "PROTO_NC_USER_GER_LOGIN_REQ size drift");

struct PROTO_NC_USER_GER_PASSWORD_CHECK_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_GER_PASSWORD_CHECK_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_GER_PASSWORD_CHECK_ACK_SEND) == 290, "PROTO_NC_USER_GER_PASSWORD_CHECK_ACK_SEND size drift");

struct PROTO_NC_USER_GER_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    Name256Byte user;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_GER_PASSWORD_CHECK_REQ) == 274, "PROTO_NC_USER_GER_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_GER_PASSWORD_CHECK_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_GER_PASSWORD_CHECK_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_GER_PASSWORD_CHECK_REQ_SEND) == 277, "PROTO_NC_USER_GER_PASSWORD_CHECK_REQ_SEND size drift");

struct PROTO_NC_USER_LOGIN_REQ {
    Name256Byte user;
    uint8_t _pad_at_0000[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_REQ) == 272, "PROTO_NC_USER_LOGIN_REQ size drift");

struct PROTO_NC_USER_IS_IP_BLOCK_ACK {
    PROTO_NC_USER_LOGIN_REQ LoginData;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_USER_IS_IP_BLOCK_ACK) == 275, "PROTO_NC_USER_IS_IP_BLOCK_ACK size drift");

struct PROTO_NC_USER_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_LOGIN_REQ LoginData;
    uint8_t _pad_at_0110[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_IS_IP_BLOCK_REQ) == 298, "PROTO_NC_USER_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_JP_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _pad_at_0002[6];
    wchar_t sUserName[24];
    wchar_t sChannel[254];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_JP_PASSWORD_CHECK_ACK) == 306, "PROTO_NC_USER_JP_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_JP_IS_IP_BLOCK_ACK {
    PROTO_NC_USER_JP_PASSWORD_CHECK_ACK LoginData;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_USER_JP_IS_IP_BLOCK_ACK) == 309, "PROTO_NC_USER_JP_IS_IP_BLOCK_ACK size drift");

struct PROTO_NC_USER_JP_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_JP_PASSWORD_CHECK_ACK LoginData;
    uint8_t _pad_at_0132[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_JP_IS_IP_BLOCK_REQ) == 332, "PROTO_NC_USER_JP_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_JP_LOGIN_REQ {
    wchar_t sUserName[24];
    wchar_t sPassword[36];
    wchar_t sChannel[254];
};
static_assert(sizeof(PROTO_NC_USER_JP_LOGIN_REQ) == 314, "PROTO_NC_USER_JP_LOGIN_REQ size drift");

struct PROTO_NC_USER_JP_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    wchar_t sUserName[24];
    wchar_t sPassword[36];
    wchar_t sChannel[254];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_JP_PASSWORD_CHECK_REQ) == 336, "PROTO_NC_USER_JP_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_KICKOFFFROMWORLD_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_KICKOFFFROMWORLD_CMD) == 4, "PROTO_NC_USER_KICKOFFFROMWORLD_CMD size drift");

struct PROTO_NC_USER_KICKOFFFROMWORLD_CMD_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    NETPACKETZONEHEADER netpacketzoneheader;
    PROTO_NC_USER_KICKOFFFROMWORLD_CMD cmd;
};
static_assert(sizeof(PROTO_NC_USER_KICKOFFFROMWORLD_CMD_SEND) == 13, "PROTO_NC_USER_KICKOFFFROMWORLD_CMD_SEND size drift");

struct PROTO_NC_USER_LOGINFAIL_ACK { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_USER_LOGINFAIL_ACK) == 2, "PROTO_NC_USER_LOGINFAIL_ACK size drift");

struct PROTO_NC_USER_LOGINFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_LOGINFAIL_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_LOGINFAIL_ACK_SEND) == 5, "PROTO_NC_USER_LOGINFAIL_ACK_SEND size drift");

struct PROTO_NC_USER_LOGINWORLDFAIL_ACK {
    PROTO_ERRORCODE errorcode;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_USER_LOGINWORLDFAIL_ACK) == 2, "PROTO_NC_USER_LOGINWORLDFAIL_ACK size drift");

struct PROTO_NC_USER_LOGINWORLDFAIL_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_LOGINWORLDFAIL_ACK Ack;
};
static_assert(sizeof(PROTO_NC_USER_LOGINWORLDFAIL_ACK_SEND) == 5, "PROTO_NC_USER_LOGINWORLDFAIL_ACK_SEND size drift");

struct PROTO_NC_USER_LOGINWORLD_ACK {
    uint8_t _pad_at_0000[3];
    PROTO_AVATARINFORMATION_______0_bytes___ avatar;
};
static_assert(sizeof(PROTO_NC_USER_LOGINWORLD_ACK) == 3, "PROTO_NC_USER_LOGINWORLD_ACK size drift");

struct PROTO_NC_USER_LOGINWORLD_REQ {
    Name256Byte user;
    uint8_t _pad_at_0000[256];
    uint16_t validate_new[32];
};
static_assert(sizeof(PROTO_NC_USER_LOGINWORLD_REQ) == 320, "PROTO_NC_USER_LOGINWORLD_REQ size drift");

struct PROTO_NC_USER_LOGIN_ACK {
    uint8_t _pad_at_0000[1];
    PROTO_NC_USER_LOGIN_ACK__WorldInfo_______0_bytes___ worldinfo;
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_ACK) == 1, "PROTO_NC_USER_LOGIN_ACK size drift");

struct PROTO_NC_USER_LOGIN_ACK__WorldInfo {
    uint8_t _pad_at_0000[1];
    Name4 worldname;
    uint8_t _tail[17];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_ACK__WorldInfo) == 18, "PROTO_NC_USER_LOGIN_ACK__WorldInfo size drift");

struct PROTO_NC_USER_LOGIN_DB {
    uint8_t _pad_at_0000[5];
    uint8_t ip[4];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_DB) == 9, "PROTO_NC_USER_LOGIN_DB size drift");

struct PROTO_NC_USER_LOGIN_DB_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_LOGIN_DB db;
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_DB_SEND) == 12, "PROTO_NC_USER_LOGIN_DB_SEND size drift");

struct PROTO_NC_USER_LOGIN_NETMARBLE_DB_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t UniID[20];
    uint8_t UserID[256];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_NETMARBLE_DB_REQ) == 278, "PROTO_NC_USER_LOGIN_NETMARBLE_DB_REQ size drift");

struct PROTO_NC_USER_LOGIN_NETMARBLE_REQ {
    uint8_t _pad_at_0000[2];
    uint8_t CPCookie[0];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_NETMARBLE_REQ) == 2, "PROTO_NC_USER_LOGIN_NETMARBLE_REQ size drift");

struct PROTO_NC_USER_LOGIN_OUTSPARK_REQ {
    uint8_t LogonToken[65];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_OUTSPARK_REQ) == 85, "PROTO_NC_USER_LOGIN_OUTSPARK_REQ size drift");

struct PROTO_NC_USER_LOGIN_WITH_OTP_REQ {
    Name8 sOTP;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_USER_LOGIN_WITH_OTP_REQ) == 32, "PROTO_NC_USER_LOGIN_WITH_OTP_REQ size drift");

struct PROTO_NC_USER_LOGOUT_DB { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_USER_LOGOUT_DB) == 9, "PROTO_NC_USER_LOGOUT_DB size drift");

struct PROTO_NC_USER_LOGOUT_DB_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_LOGOUT_DB db;
};
static_assert(sizeof(PROTO_NC_USER_LOGOUT_DB_SEND) == 12, "PROTO_NC_USER_LOGOUT_DB_SEND size drift");

struct PROTO_NC_USER_LOGOUT_LAST_DAY_ACK { uint8_t data[10]; };
static_assert(sizeof(PROTO_NC_USER_LOGOUT_LAST_DAY_ACK) == 10, "PROTO_NC_USER_LOGOUT_LAST_DAY_ACK size drift");

struct PROTO_NC_USER_LOGOUT_LAST_DAY_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_USER_LOGOUT_LAST_DAY_REQ) == 6, "PROTO_NC_USER_LOGOUT_LAST_DAY_REQ size drift");

struct PROTO_NC_USER_NORMALLOGOUT_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_NORMALLOGOUT_CMD) == 1, "PROTO_NC_USER_NORMALLOGOUT_CMD size drift");

struct PROTO_NC_USER_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    Name256Byte id;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[29];
};
static_assert(sizeof(PROTO_NC_USER_PASSWORD_CHECK_ACK) == 287, "PROTO_NC_USER_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_PASSWORD_CHECK_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_PASSWORD_CHECK_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_PASSWORD_CHECK_ACK_SEND) == 290, "PROTO_NC_USER_PASSWORD_CHECK_ACK_SEND size drift");

struct PROTO_NC_USER_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    Name256Byte user;
    uint8_t _pad_at_0002[256];
    Name4 password;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_USER_PASSWORD_CHECK_REQ) == 274, "PROTO_NC_USER_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_PASSWORD_CHECK_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_PASSWORD_CHECK_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_PASSWORD_CHECK_REQ_SEND) == 277, "PROTO_NC_USER_PASSWORD_CHECK_REQ_SEND size drift");

struct PROTO_NC_USER_POSSIBLE_NEW_CONNECT_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_USER_POSSIBLE_NEW_CONNECT_CMD) == 6, "PROTO_NC_USER_POSSIBLE_NEW_CONNECT_CMD size drift");

struct PROTO_NC_USER_REGISENUMBER_ACK { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_REGISENUMBER_ACK) == 4, "PROTO_NC_USER_REGISENUMBER_ACK size drift");

struct PROTO_NC_USER_RETURN_CHECK_ACK { uint8_t data[11]; };
static_assert(sizeof(PROTO_NC_USER_RETURN_CHECK_ACK) == 11, "PROTO_NC_USER_RETURN_CHECK_ACK size drift");

struct PROTO_NC_USER_RETURN_CHECK_REQ { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_USER_RETURN_CHECK_REQ) == 6, "PROTO_NC_USER_RETURN_CHECK_REQ size drift");

struct PROTO_NC_USER_SET_RETURN_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_SET_RETURN_CMD) == 4, "PROTO_NC_USER_SET_RETURN_CMD size drift");

struct PROTO_NC_USER_TEENAGER_CMD {
    uint8_t sUserKey[50];
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_USER_TEENAGER_CMD) == 55, "PROTO_NC_USER_TEENAGER_CMD size drift");

struct PROTO_NC_USER_TEENAGER_ACK {
    Name256Byte sUserID;
    uint8_t _pad_at_0000[258];
    PROTO_NC_USER_TEENAGER_CMD Data;
};
static_assert(sizeof(PROTO_NC_USER_TEENAGER_ACK) == 313, "PROTO_NC_USER_TEENAGER_ACK size drift");

struct PROTO_NC_USER_TEENAGER_REMAIN_MIN_CMD { uint8_t data[4]; };
static_assert(sizeof(PROTO_NC_USER_TEENAGER_REMAIN_MIN_CMD) == 4, "PROTO_NC_USER_TEENAGER_REMAIN_MIN_CMD size drift");

struct PROTO_NC_USER_TEENAGER_REQ {
    Name256Byte sUserID;
    uint8_t _tail[258];
};
static_assert(sizeof(PROTO_NC_USER_TEENAGER_REQ) == 258, "PROTO_NC_USER_TEENAGER_REQ size drift");

struct PROTO_NC_USER_TEENAGER_SET_CMD {
    uint8_t sUserKey[50];
    uint8_t _tail[4];
};
static_assert(sizeof(PROTO_NC_USER_TEENAGER_SET_CMD) == 54, "PROTO_NC_USER_TEENAGER_SET_CMD size drift");

struct PROTO_NC_USER_TUTORIAL_CAN_SKIP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_TUTORIAL_CAN_SKIP_CMD) == 1, "PROTO_NC_USER_TUTORIAL_CAN_SKIP_CMD size drift");

struct PROTO_NC_USER_TW_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _tail[6];
};
static_assert(sizeof(PROTO_NC_USER_TW_PASSWORD_CHECK_ACK) == 8, "PROTO_NC_USER_TW_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_TW_IS_IP_BLOCK_ACK {
    PROTO_NC_USER_TW_PASSWORD_CHECK_ACK LoginData;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_USER_TW_IS_IP_BLOCK_ACK) == 11, "PROTO_NC_USER_TW_IS_IP_BLOCK_ACK size drift");

struct PROTO_NC_USER_TW_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_TW_PASSWORD_CHECK_ACK LoginData;
    uint8_t _pad_at_0008[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_TW_IS_IP_BLOCK_REQ) == 34, "PROTO_NC_USER_TW_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_TW_LOGIN_REQ {
    wchar_t sUserName[36];
    wchar_t sSerial[36];
    wchar_t sUID[20];
    wchar_t sSID[8];
};
static_assert(sizeof(PROTO_NC_USER_TW_LOGIN_REQ) == 100, "PROTO_NC_USER_TW_LOGIN_REQ size drift");

struct PROTO_NC_USER_TW_PASSWORD_CHECK_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_TW_PASSWORD_CHECK_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_TW_PASSWORD_CHECK_ACK_SEND) == 11, "PROTO_NC_USER_TW_PASSWORD_CHECK_ACK_SEND size drift");

struct PROTO_NC_USER_TW_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    wchar_t sUserName[36];
    wchar_t sSerial[36];
    wchar_t sUID[20];
    wchar_t sSID[8];
};
static_assert(sizeof(PROTO_NC_USER_TW_PASSWORD_CHECK_REQ) == 102, "PROTO_NC_USER_TW_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_TW_PASSWORD_CHECK_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_TW_PASSWORD_CHECK_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_TW_PASSWORD_CHECK_REQ_SEND) == 105, "PROTO_NC_USER_TW_PASSWORD_CHECK_REQ_SEND size drift");

struct PROTO_NC_USER_USE_BEAUTY_SHOP_CMD { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_USE_BEAUTY_SHOP_CMD) == 1, "PROTO_NC_USER_USE_BEAUTY_SHOP_CMD size drift");

struct PROTO_NC_USER_US_PASSWORD_CHECK_ACK {
    NETPACKETHEADER netpacketheader;
    uint8_t _pad_at_0002[6];
    wchar_t sUserName[260];
    wchar_t sPassword[36];
    wchar_t sUserIP[20];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_US_PASSWORD_CHECK_ACK) == 353, "PROTO_NC_USER_US_PASSWORD_CHECK_ACK size drift");

struct PROTO_NC_USER_US_IS_IP_BLOCK_ACK {
    PROTO_NC_USER_US_PASSWORD_CHECK_ACK LoginData;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_USER_US_IS_IP_BLOCK_ACK) == 356, "PROTO_NC_USER_US_IS_IP_BLOCK_ACK size drift");

struct PROTO_NC_USER_US_IS_IP_BLOCK_REQ {
    PROTO_NC_USER_US_PASSWORD_CHECK_ACK LoginData;
    uint8_t _pad_at_0161[6];
    wchar_t sUserIP[20];
};
static_assert(sizeof(PROTO_NC_USER_US_IS_IP_BLOCK_REQ) == 379, "PROTO_NC_USER_US_IS_IP_BLOCK_REQ size drift");

struct PROTO_NC_USER_US_LOGIN_REQ {
    wchar_t sUserName[260];
    wchar_t sPassword[36];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_US_LOGIN_REQ) == 316, "PROTO_NC_USER_US_LOGIN_REQ size drift");

struct PROTO_NC_USER_US_PASSWORD_CHECK_REQ {
    NETPACKETHEADER netpacketheader;
    wchar_t sUserName[260];
    wchar_t sPassword[36];
    wchar_t sUserIP[20];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_US_PASSWORD_CHECK_REQ) == 338, "PROTO_NC_USER_US_PASSWORD_CHECK_REQ size drift");

struct PROTO_NC_USER_WILLLOGIN_REQ {
    NETPACKETHEADER netpacketheader;
    uint8_t _pad_at_0002[5];
    Name256Byte userid;
    uint8_t _pad_at_0007[256];
    uint16_t validate_new[32];
    Name5 spawnapps;
    uint8_t _tail[20];
};
static_assert(sizeof(PROTO_NC_USER_WILLLOGIN_REQ) == 347, "PROTO_NC_USER_WILLLOGIN_REQ size drift");

struct PROTO_NC_USER_WILLLOGIN_ACK {
    PROTO_NC_USER_WILLLOGIN_REQ will_login_req;
    uint8_t _tail[1];
};
static_assert(sizeof(PROTO_NC_USER_WILLLOGIN_ACK) == 348, "PROTO_NC_USER_WILLLOGIN_ACK size drift");

struct PROTO_NC_USER_WILLLOGIN_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_WILLLOGIN_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_WILLLOGIN_ACK_SEND) == 351, "PROTO_NC_USER_WILLLOGIN_ACK_SEND size drift");

struct PROTO_NC_USER_WILLLOGIN_REQ_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_WILLLOGIN_REQ req;
};
static_assert(sizeof(PROTO_NC_USER_WILLLOGIN_REQ_SEND) == 350, "PROTO_NC_USER_WILLLOGIN_REQ_SEND size drift");

struct PROTO_NC_USER_WILL_WORLD_SELECT_ACK {
    uint8_t _pad_at_0000[2];
    Name8 sOTP;
    uint8_t _tail[32];
};
static_assert(sizeof(PROTO_NC_USER_WILL_WORLD_SELECT_ACK) == 34, "PROTO_NC_USER_WILL_WORLD_SELECT_ACK size drift");

struct PROTO_NC_USER_WILL_WORLD_SELECT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_WILL_WORLD_SELECT_REQ) == 1, "PROTO_NC_USER_WILL_WORLD_SELECT_REQ size drift");

struct PROTO_NC_USER_WORLDSELECT_ACK {
    uint8_t _pad_at_0000[1];
    Name4 ip;
    uint8_t _pad_at_0001[18];
    uint16_t validate_new[32];
};
static_assert(sizeof(PROTO_NC_USER_WORLDSELECT_ACK) == 83, "PROTO_NC_USER_WORLDSELECT_ACK size drift");

struct PROTO_NC_USER_WORLDSELECT_ACK_SEND {
    uint8_t _pad_at_0000[1];
    NETCOMMAND netcmd;
    uint8_t _pad_at_0001[2];
    PROTO_NC_USER_WORLDSELECT_ACK ack;
};
static_assert(sizeof(PROTO_NC_USER_WORLDSELECT_ACK_SEND) == 86, "PROTO_NC_USER_WORLDSELECT_ACK_SEND size drift");

struct PROTO_NC_USER_WORLDSELECT_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_WORLDSELECT_REQ) == 1, "PROTO_NC_USER_WORLDSELECT_REQ size drift");

struct PROTO_NC_USER_WORLD_STATUS_REQ { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_WORLD_STATUS_REQ) == 1, "PROTO_NC_USER_WORLD_STATUS_REQ size drift");

struct PROTO_NC_USER_XTRAP_ACK { uint8_t data[1]; };
static_assert(sizeof(PROTO_NC_USER_XTRAP_ACK) == 1, "PROTO_NC_USER_XTRAP_ACK size drift");

struct PROTO_NC_USER_XTRAP_REQ {
    uint8_t _pad_at_0000[1];
    uint8_t XTrapClientKey[0];
};
static_assert(sizeof(PROTO_NC_USER_XTRAP_REQ) == 1, "PROTO_NC_USER_XTRAP_REQ size drift");

struct PROTO_NC_USP_USER_CHARACTER_DELETE { uint8_t data[9]; };
static_assert(sizeof(PROTO_NC_USP_USER_CHARACTER_DELETE) == 9, "PROTO_NC_USP_USER_CHARACTER_DELETE size drift");

struct PROTO_NC_USP_USER_CHARACTER_INSERT {
    uint8_t _pad_at_0000[9];
    wchar_t sCharName[40];
};
static_assert(sizeof(PROTO_NC_USP_USER_CHARACTER_INSERT) == 50, "PROTO_NC_USP_USER_CHARACTER_INSERT size drift");

struct PROTO_NC_WT_GRADE_CMD {
    ITEM_INVEN ItemInven;
    uint8_t _tail[3];
};
static_assert(sizeof(PROTO_NC_WT_GRADE_CMD) == 3, "PROTO_NC_WT_GRADE_CMD size drift");

struct PROTO_NC_WT_LICENSE_ACK {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_ACK) == 7, "PROTO_NC_WT_LICENSE_ACK size drift");

struct PROTO_NC_WT_LICENSE_CLR_ACK__echo_data {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_CLR_ACK__echo_data) == 4, "PROTO_NC_WT_LICENSE_CLR_ACK__echo_data size drift");

struct PROTO_NC_WT_LICENSE_CLR_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_WT_LICENSE_CLR_ACK__echo_data echo_data;
    SHINE_ITEM_REGISTNUMBER WeaponItemKey;
    uint8_t _tail[12];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_CLR_ACK) == 24, "PROTO_NC_WT_LICENSE_CLR_ACK size drift");

struct PROTO_NC_WT_LICENSE_CLR_REQ__echo_data {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_CLR_REQ__echo_data) == 4, "PROTO_NC_WT_LICENSE_CLR_REQ__echo_data size drift");

struct PROTO_NC_WT_LICENSE_CLR_REQ {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[2];
    PROTO_NC_WT_LICENSE_CLR_REQ__echo_data echo_data;
    SHINE_ITEM_REGISTNUMBER WeaponItemKey;
    uint8_t _pad_at_000c[10];
    SHINE_ITEM_REGISTNUMBER LicenseClearItemKey;
    uint8_t _tail[13];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_CLR_REQ) == 35, "PROTO_NC_WT_LICENSE_CLR_REQ size drift");

struct PROTO_NC_WT_LICENSE_CMD {
    ITEM_INVEN ItemInven;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_CMD) == 5, "PROTO_NC_WT_LICENSE_CMD size drift");

struct PROTO_NC_WT_LICENSE_REQ {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[5];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_REQ) == 7, "PROTO_NC_WT_LICENSE_REQ size drift");

struct PROTO_NC_WT_LICENSE_SET_ACK__echo_data {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_SET_ACK__echo_data) == 4, "PROTO_NC_WT_LICENSE_SET_ACK__echo_data size drift");

struct PROTO_NC_WT_LICENSE_SET_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[4];
    PROTO_NC_WT_LICENSE_SET_ACK__echo_data echo_data;
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_SET_ACK) == 14, "PROTO_NC_WT_LICENSE_SET_ACK size drift");

struct PROTO_NC_WT_LICENSE_SET_REQ__echo_data {
    ITEM_INVEN licenseitem;
    uint8_t _pad_at_0000[2];
    ITEM_INVEN weaponitem;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_SET_REQ__echo_data) == 4, "PROTO_NC_WT_LICENSE_SET_REQ__echo_data size drift");

struct PROTO_NC_WT_LICENSE_SET_REQ {
    NETPACKETZONEHEADER Header;
    uint8_t _pad_at_0006[6];
    Name5 chrname;
    uint8_t _pad_at_000c[20];
    SHINE_ITEM_REGISTNUMBER WeaponItemKey;
    uint8_t _pad_at_0020[8];
    SHINE_ITEM_REGISTNUMBER LicenItemKey;
    uint8_t _pad_at_0028[10];
    PROTO_NC_WT_LICENSE_SET_REQ__echo_data echo_data;
};
static_assert(sizeof(PROTO_NC_WT_LICENSE_SET_REQ) == 54, "PROTO_NC_WT_LICENSE_SET_REQ size drift");

struct PROTO_NC_WT_MOBINC_CMD { uint8_t data[2]; };
static_assert(sizeof(PROTO_NC_WT_MOBINC_CMD) == 2, "PROTO_NC_WT_MOBINC_CMD size drift");

struct PROTO_NC_WT_MOB_KILLCOUNT_SET_ACK {
    NETPACKETZONEHEADER Header;
    SHINE_ITEM_REGISTNUMBER ItemKey;
    uint8_t _tail[16];
};
static_assert(sizeof(PROTO_NC_WT_MOB_KILLCOUNT_SET_ACK) == 22, "PROTO_NC_WT_MOB_KILLCOUNT_SET_ACK size drift");

struct PROTO_NC_WT_MOB_KILLCOUNT_SET_REQ {
    NETPACKETZONEHEADER Header;
    SHINE_ITEM_REGISTNUMBER ItemKey;
    uint8_t _tail[14];
};
static_assert(sizeof(PROTO_NC_WT_MOB_KILLCOUNT_SET_REQ) == 20, "PROTO_NC_WT_MOB_KILLCOUNT_SET_REQ size drift");

struct PROTO_NC_WT_TITLE_CMD {
    ITEM_INVEN ItemInven;
    uint8_t _pad_at_0000[2];
    wchar_t Title[21];
};
static_assert(sizeof(PROTO_NC_WT_TITLE_CMD) == 23, "PROTO_NC_WT_TITLE_CMD size drift");

struct PROTO_NC_WT_TITLE_SET_ACK {
    NETPACKETZONEHEADER Header;
    uint8_t _tail[2];
};
static_assert(sizeof(PROTO_NC_WT_TITLE_SET_ACK) == 8, "PROTO_NC_WT_TITLE_SET_ACK size drift");

struct PROTO_NC_WT_TITLE_SET_REQ {
    NETPACKETZONEHEADER Header;
    SHINE_ITEM_REGISTNUMBER ItemKey;
    uint8_t _pad_at_0006[12];
    wchar_t Title[21];
};
static_assert(sizeof(PROTO_NC_WT_TITLE_SET_REQ) == 39, "PROTO_NC_WT_TITLE_SET_REQ size drift");

struct PROTO_NC_ZONE_PRISON_END_CMD { uint8_t data[6]; };
static_assert(sizeof(PROTO_NC_ZONE_PRISON_END_CMD) == 6, "PROTO_NC_ZONE_PRISON_END_CMD size drift");

struct PROTO_NC_ZONE_PRISON_GO_ACK {
    uint8_t _pad_at_0000[6];
    Name256Byte sGmUserID;
    uint8_t _pad_at_0006[256];
    Name5 sCharID;
    uint8_t _pad_at_0106[30];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_ZONE_PRISON_GO_ACK) == 374, "PROTO_NC_ZONE_PRISON_GO_ACK size drift");

struct PROTO_NC_ZONE_PRISON_GO_REQ {
    uint8_t _pad_at_0000[6];
    Name256Byte sGmUserID;
    uint8_t _pad_at_0006[256];
    Name5 sCharID;
    uint8_t _pad_at_0106[30];
    wchar_t sReason[16];
    wchar_t sRemark[64];
};
static_assert(sizeof(PROTO_NC_ZONE_PRISON_GO_REQ) == 372, "PROTO_NC_ZONE_PRISON_GO_REQ size drift");

#pragma pack(pop)

}
