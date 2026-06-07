#include "FiestaClient.h"
#include "FiestaConsoleTrace.h"

#include <cstdio>
#include <cstring>
#include <sstream>

namespace Fiesta {

static std::string JsonEsc(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if ((unsigned char)c < 0x20) {
                    static const char hexdig[] = "0123456789abcdef";
                    const unsigned u = (unsigned char)c;
                    out += "\\u00";
                    out += hexdig[(u >> 4) & 0xF];
                    out += hexdig[u & 0xF];
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static std::string ToHex(const std::vector<uint8_t>& bytes) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(bytes.size() * 2);
    for (uint8_t b : bytes) {
        s += hex[b >> 4];
        s += hex[b & 0xF];
    }
    return s;
}

[[maybe_unused]] static std::string ReadStr8(Reader& r) {
    std::string s;
    r.Str8(s);
    return s;
}

void Client::OnLoginCharacter(const InPacket& pkt) {

    if (pkt.payload.size() < sizeof(PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD)) {
        return;
    }
    PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD head{};
    std::memcpy(&head, pkt.payload.data(), sizeof(head));
    char nameBuf[17] = {0};
    std::memcpy(nameBuf, head.charid, 16);
    std::string name(nameBuf);
    while (!name.empty() && name.back() == 0) name.pop_back();
    m_briefRing.Insert(head.handle, name);
    m_world.UpsertPlayer(head.handle, name);

    std::ostringstream o;
    o << "{\"kind\":\"player_appear\",\"handle\":" << head.handle
      << ",\"name\":\"" << JsonEsc(name) << "\"}";
    EmitEvent(o.str());
}

void Client::OnBriefInfoDelete(const InPacket& pkt) {

    if (pkt.payload.size() < 2) return;
    PROTO_NC_BRIEFINFO_BRIEFINFODELETE_CMD del{};
    std::memcpy(&del, pkt.payload.data(), sizeof(del));
    m_briefRing.Remove(del.hnd);
    m_world.RemoveEntity(del.hnd);

    std::ostringstream o;
    o << "{\"kind\":\"entity_disappear\",\"handle\":" << del.hnd << "}";
    EmitEvent(o.str());
}

void Client::OnRegenMob(const InPacket& pkt) {

    if (pkt.payload.size() < sizeof(PROTO_NC_BRIEFINFO_REGENMOB_CMD_HEAD)) return;
    PROTO_NC_BRIEFINFO_REGENMOB_CMD_HEAD head{};
    std::memcpy(&head, pkt.payload.data(), sizeof(head));

    m_world.UpsertMob(head.handle, head.mob_id);
    std::ostringstream o;
    o << "{\"kind\":\"mob_appear\",\"handle\":" << head.handle
      << ",\"mob_id\":" << head.mob_id << "}";
    EmitEvent(o.str());
}

void Client::OnNpcDisappear(const InPacket& pkt) {
    if (pkt.payload.size() < 2) return;
    PROTO_NC_BRIEFINFO_NPC_DISAPPEAR_CMD del{};
    std::memcpy(&del, pkt.payload.data(), sizeof(del));
    m_briefRing.Remove(del.handle);
    m_world.RemoveEntity(del.handle);
    std::ostringstream o;
    o << "{\"kind\":\"npc_disappear\",\"handle\":" << del.handle << "}";
    EmitEvent(o.str());
}

void Client::OnBriefCharacter(const InPacket& pkt) {

    if (pkt.payload.size() < sizeof(PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD)) return;
    PROTO_NC_BRIEFINFO_LOGINCHARACTER_CMD_HEAD head{};
    std::memcpy(&head, pkt.payload.data(), sizeof(head));
    char nameBuf[17] = {0};
    std::memcpy(nameBuf, head.charid, 16);
    std::string name(nameBuf);
    while (!name.empty() && name.back() == 0) name.pop_back();
    m_briefRing.Insert(head.handle, name);
    m_world.UpsertPlayer(head.handle, name);

    std::ostringstream o;
    o << "{\"kind\":\"player_update\",\"handle\":" << head.handle
      << ",\"name\":\"" << JsonEsc(name) << "\"}";
    EmitEvent(o.str());
}

void Client::OnPlayerListAppear(const InPacket& pkt) {

    Reader r(pkt.payload.data(), pkt.payload.size());
    uint8_t count = 0;
    if (!r.U8(count)) return;
    int parsed = 0;
    for (uint8_t i = 0; i < count; i++) {
        uint16_t handle = 0;
        char     charid[16] = {0};
        if (!r.U16(handle)) break;
        if (!r.Bytes(charid, sizeof(charid))) break;
        std::string name(charid, sizeof(charid));
        while (!name.empty() && name.back() == 0) name.pop_back();
        if (!name.empty()) {
            m_briefRing.Insert(handle, name);
            m_world.UpsertPlayer(handle, name);
        }
        parsed++;
    }
    std::ostringstream o;
    o << "{\"kind\":\"player_list\",\"count\":" << parsed << "}";
    EmitEvent(o.str());
}

void Client::OnCharBase(const InPacket& pkt) {

    if (pkt.payload.size() < sizeof(PROTO_NC_CHAR_BASE_CMD_HEAD)) return;
    PROTO_NC_CHAR_BASE_CMD_HEAD head{};
    std::memcpy(&head, pkt.payload.data(), sizeof(head));
    char nameBuf[17] = {0};
    std::memcpy(nameBuf, head.charid, 16);
    std::string name(nameBuf);
    while (!name.empty() && name.back() == 0) name.pop_back();

    m_world.UpdateSelfBase(head.chrregnum, name);

    std::ostringstream o;
    o << "{\"kind\":\"self_base\",\"chrregnum\":" << head.chrregnum
      << ",\"charid\":\"" << JsonEsc(name) << "\"}";
    EmitEvent(o.str());
}

void Client::OnChatLike(const InPacket& pkt) {

    const char* channel =
        (pkt.opcode == Op::NC_ACT_SHOUT_CMD) ? "shout" : "normal";

    uint16_t senderHandle = 0;
    std::string text;
    bool decoded = false;

    auto tryParseChatReq = [&](const uint8_t* p, size_t n) -> bool {
        if (n < sizeof(PROTO_NC_ACT_CHAT_REQ_HEAD)) return false;
        const uint8_t  len = p[1];
        if ((size_t)len > n - 2) return false;
        text.assign((const char*)(p + 2), len);
        return true;
    };

    if (pkt.payload.size() >= 6 + sizeof(PROTO_NC_ACT_CHAT_REQ_HEAD)) {
        const uint8_t* p = pkt.payload.data();
        const uint16_t handle = (uint16_t)(p[0] | (p[1] << 8));
        if (tryParseChatReq(p + 6, pkt.payload.size() - 6)) {
            senderHandle = handle;
            decoded = true;
        }
    }

    if (!decoded && pkt.payload.size() >= 2 + sizeof(PROTO_NC_ACT_CHAT_REQ_HEAD)) {
        const uint8_t* p = pkt.payload.data();
        const uint16_t handle = (uint16_t)(p[0] | (p[1] << 8));
        if (tryParseChatReq(p + 2, pkt.payload.size() - 2)) {
            senderHandle = handle;
            decoded = true;
        }
    }

    if (!decoded) {
        decoded = tryParseChatReq(pkt.payload.data(), pkt.payload.size());
    }

    const std::string speakerName = m_briefRing.Resolve(senderHandle);

    std::ostringstream o;
    o << "{\"kind\":\"chat\",\"channel\":\"" << channel
      << "\",\"speaker_handle\":" << senderHandle
      << ",\"speaker_name\":\"" << JsonEsc(speakerName) << "\""
      << ",\"text\":\"" << JsonEsc(text) << "\""
      << ",\"raw_hex\":\"" << ToHex(pkt.payload) << "\"}";
    EmitEvent(o.str());

    Fiesta::Trace::OnChat(speakerName.empty()
                              ? std::string_view{"?"}
                              : std::string_view{speakerName},
                          text, channel);
}

void Client::OnWhisper(const InPacket& pkt) {

    if (pkt.payload.size() < sizeof(PROTO_NC_ACT_WHISPER_REQ_HEAD)) return;
    PROTO_NC_ACT_WHISPER_REQ_HEAD head{};
    std::memcpy(&head, pkt.payload.data(), sizeof(head));

    char nameBuf[17] = {0};
    std::memcpy(nameBuf, head.handle, 16);
    std::string sender(nameBuf);
    while (!sender.empty() && sender.back() == 0) sender.pop_back();

    const size_t want = sizeof(head) + head.len;
    std::string text;
    if (pkt.payload.size() >= want) {
        text.assign((const char*)(pkt.payload.data() + sizeof(head)),
                    head.len);
    }

    std::ostringstream o;
    o << "{\"kind\":\"chat\",\"channel\":\"whisper_in\""
      << ",\"speaker_handle\":0"
      << ",\"speaker_name\":\"" << JsonEsc(sender) << "\""
      << ",\"text\":\"" << JsonEsc(text) << "\"}";
    EmitEvent(o.str());

    Fiesta::Trace::OnChat(sender, text, "whisper");
}

void Client::WireConnectionCallbacks() {
    m_conn.SetOnPacket([this](const InPacket& pkt) {
        if (m_onRawPacket) m_onRawPacket(pkt, true);
        HandlePacket(pkt);
    });
    m_conn.SetOnDisconnect([this](const std::string& why) {
        EmitEvent(std::string("{\"kind\":\"disconnected\",\"reason\":\"") +
                  JsonEsc(why) + "\"}");
        SetState(State::DISCONNECTED);
    });
}

bool Client::Connect(const std::string& loginHost, uint16_t loginPort,
                     const std::string& user, const std::string& pass) {
    Disconnect("re-connect");

    {
        std::lock_guard<std::mutex> lk(m_mx);
        m_user = user;
        m_pass = pass;
    }

    WireConnectionCallbacks();

    m_conn.MutableCipher().SetEnabled(false);

    if (!m_conn.Connect(loginHost, loginPort)) {
        SetState(State::DISCONNECTED);
        return false;
    }
    SetState(State::LOGIN_CONNECTING);
    return true;
}

void Client::Disconnect(const std::string& why) {
    StopHeartbeat();
    m_conn.Disconnect(why);
    SetState(State::DISCONNECTED);
}

bool Client::ReconnectTo(const std::string& host, uint16_t port) {

    m_conn.Disconnect("hop");
    m_conn.MutableCipher().SetEnabled(false);
    if (!m_conn.Connect(host, port)) return false;

    return true;
}

void Client::SetState(State s) {
    State previous;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        previous = m_state;
        m_state  = s;
    }
    if (previous != s) {
        Fiesta::Trace::OnStateChange(StateName(previous), StateName(s));
        m_world.SetLoginState(StateName(s));
        std::ostringstream o;
        o << "{\"kind\":\"login_state\",\"state\":\"" << StateName(s) << "\"}";
        EmitEvent(o.str());
    }
}

void Client::EmitEvent(const std::string& kindJson) {
    if (m_onEvent) {
        GameEvent e; e.kindJson = kindJson; m_onEvent(e);
    }
}

bool Client::SendVersionCheck() {
    PROTO_NC_USER_CLIENT_VERSION_CHECK_REQ req{};

    std::string key;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        key = m_versionKey;
    }
    const size_t maxChars = sizeof(req.sVersionKey) / sizeof(req.sVersionKey[0]);
    const size_t copy = (key.size() < maxChars) ? key.size() : maxChars;
    for (size_t i = 0; i < copy; i++) {
        req.sVersionKey[i] = (uint16_t)(unsigned char)key[i];
    }
    SetState(State::LOGIN_VERSION);
    return m_conn.Send(Op::NC_USER_CLIENT_VERSION_CHECK_REQ, ToBytes(req));
}

bool Client::SendLoginRequest() {
    PROTO_NC_USER_LOGIN_REQ req{};
    std::string user, pass;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        user = m_user;
        pass = m_pass;
    }

    const size_t userCopy = (user.size() < sizeof(req.user)) ? user.size() : sizeof(req.user);
    std::memcpy(req.user, user.data(), userCopy);
    const size_t passCopy = (pass.size() < sizeof(req.password)) ? pass.size() : sizeof(req.password);
    std::memcpy(req.password, pass.data(), passCopy);
    SetState(State::LOGIN_AUTH);
    return m_conn.Send(Op::NC_USER_LOGIN_REQ, ToBytes(req));
}

bool Client::SelectWorld(uint8_t worldNo) {
    if (GetState() != State::WORLD_LIST) return false;
    return SendWorldSelect(worldNo);
}

bool Client::SendWorldSelect(uint8_t worldNo) {
    PROTO_NC_USER_WORLDSELECT_REQ req{};
    req.worldno = worldNo;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        m_selectedWorld = worldNo;
    }
    return m_conn.Send(Op::NC_USER_WORLDSELECT_REQ, ToBytes(req));
}

void Client::OnLoginAck(const InPacket& pkt) {

    Reader r(pkt.payload.data(), pkt.payload.size());
    uint8_t numWorlds = 0;
    r.U8(numWorlds);

    std::ostringstream o;
    o << "{\"kind\":\"world_list\",\"count\":" << (int)numWorlds
      << ",\"worlds\":[";
    for (uint8_t i = 0; i < numWorlds; i++) {
        if (i > 0) o << ",";
        uint8_t worldno = 0;
        r.U8(worldno);

        o << "{\"worldno\":" << (int)worldno << "}";

        const size_t stride = 20 - 1;
        if (r.Remaining() >= stride) {
            uint8_t skip[19];
            r.Bytes(skip, stride);
        }
    }
    o << "]}";
    EmitEvent(o.str());

    SetState(State::WORLD_LIST);

    if (numWorlds == 1) {
        SendWorldSelect(0);
    }
}

void Client::OnWorldSelectAck(const InPacket& pkt) {
    if (pkt.payload.size() < sizeof(PROTO_NC_USER_WORLDSELECT_ACK)) {
        EmitEvent("{\"kind\":\"login_error\",\"reason\":\"worldselect_ack truncated\"}");
        Disconnect("worldselect_ack truncated");
        return;
    }
    std::memcpy(&m_wmHandoff, pkt.payload.data(), sizeof(m_wmHandoff));

    if (m_wmHandoff.worldstatus == 0) {
        std::ostringstream o;
        o << "{\"kind\":\"login_error\",\"reason\":\"world_down\",\"status\":"
          << (int)m_wmHandoff.worldstatus << "}";
        EmitEvent(o.str());
        Disconnect("world_down");
        return;
    }

    char ipbuf[17] = {0};
    std::memcpy(ipbuf, m_wmHandoff.ip, 16);
    std::string wmHost(ipbuf);

    while (!wmHost.empty() && wmHost.back() == 0) wmHost.pop_back();

    std::ostringstream o;
    o << "{\"kind\":\"wm_handoff\",\"ip\":\"" << JsonEsc(wmHost)
      << "\",\"port\":" << m_wmHandoff.port << "}";
    EmitEvent(o.str());

    SetState(State::WM_HANDOFF);
    if (!ReconnectTo(wmHost, m_wmHandoff.port)) {
        Disconnect("wm reconnect failed");
    }
}

bool Client::SendWillLogin() {
    PROTO_NC_USER_WILLLOGIN_REQ req{};

    std::string user;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        user = m_user;
    }
    const size_t userCopy = (user.size() < sizeof(req.userid)) ? user.size() : sizeof(req.userid);
    std::memcpy(req.userid, user.data(), userCopy);

    std::memcpy(req.validate_new, m_wmHandoff.validate_new, sizeof(req.validate_new));

    std::string sa;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        sa = m_spawnApps;
    }
    const size_t saCopy = (sa.size() < sizeof(req.spawnapps)) ? sa.size() : sizeof(req.spawnapps);
    std::memcpy(req.spawnapps, sa.data(), saCopy);

    SetState(State::WM_AUTH);
    return m_conn.Send(Op::NC_USER_WILLLOGIN_REQ, ToBytes(req));
}

void Client::OnWillLoginAck(const InPacket& pkt) {
    if (pkt.payload.size() < sizeof(PROTO_NC_USER_WILLLOGIN_ACK)) {
        EmitEvent("{\"kind\":\"login_error\",\"reason\":\"willlogin_ack truncated\"}");
        Disconnect("willlogin_ack truncated");
        return;
    }
    std::memcpy(&m_zoneHandoff, pkt.payload.data(), sizeof(m_zoneHandoff));

    if (m_zoneHandoff.status != 0) {
        std::ostringstream o;
        o << "{\"kind\":\"login_error\",\"reason\":\"wm_rejected\",\"status\":"
          << (int)m_zoneHandoff.status << "}";
        EmitEvent(o.str());
        Disconnect("wm_rejected");
        return;
    }

    char ipbuf[17] = {0};
    std::memcpy(ipbuf, m_zoneHandoff.zone_ip, 16);
    std::string zoneHost(ipbuf);
    while (!zoneHost.empty() && zoneHost.back() == 0) zoneHost.pop_back();

    std::ostringstream o;
    o << "{\"kind\":\"zone_handoff\",\"ip\":\"" << JsonEsc(zoneHost)
      << "\",\"port\":" << m_zoneHandoff.zone_port << "}";
    EmitEvent(o.str());

    SetState(State::ZONE_HANDOFF);
    if (!ReconnectTo(zoneHost, m_zoneHandoff.zone_port)) {
        Disconnect("zone reconnect failed");
    }
}

bool Client::SendLoginWorld() {

    PROTO_NC_USER_WILLLOGIN_REQ req{};
    std::string user;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        user = m_user;
    }
    const size_t userCopy = (user.size() < sizeof(req.userid)) ? user.size() : sizeof(req.userid);
    std::memcpy(req.userid, user.data(), userCopy);
    std::memcpy(req.validate_new, m_zoneHandoff.validate_new, sizeof(req.validate_new));
    return m_conn.Send(Op::NC_USER_LOGINWORLD_REQ, ToBytes(req));
}

bool Client::SendMapLogin() {

    std::vector<uint8_t> payload(978, 0);
    SetState(State::ZONE_AUTH);
    return m_conn.Send(Op::NC_MAP_LOGIN_REQ, payload);
}

void Client::OnMapLoginComplete(const InPacket& pkt) {

    (void)pkt;
    SetState(State::IN_GAME);

    StartHeartbeat();
    EmitEvent("{\"kind\":\"in_game\"}");
}

void Client::OnSeedAck(const InPacket& pkt) {

    if (pkt.payload.size() < 2) {
        EmitEvent("{\"kind\":\"login_error\",\"reason\":\"seed_ack truncated\"}");
        Disconnect("seed_ack truncated");
        return;
    }
    const uint16_t seed = (uint16_t)(pkt.payload[0] | (pkt.payload[1] << 8));
    m_conn.MutableCipher().Reset(seed);

    std::ostringstream o;
    o << "{\"kind\":\"seed_received\",\"seed\":" << seed
      << ",\"hop\":\"" << StateName(GetState()) << "\"}";
    EmitEvent(o.str());

    switch (GetState()) {
        case State::LOGIN_CONNECTING:
            SendVersionCheck();
            break;
        case State::WM_HANDOFF:
            SendWillLogin();
            break;
        case State::ZONE_HANDOFF:
            SendLoginWorld();
            SendMapLogin();
            break;
        default:

            break;
    }
}

bool Client::MoveTo(uint32_t toX, uint32_t toY, bool run) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_ACT_WALK_REQ req{};
    req.from = m_lastPos;
    req.to.x = toX;
    req.to.y = toY;
    m_lastPos = req.to;
    m_world.UpdateSelfPosition(toX, toY);
    return m_conn.Send(run ? Op::NC_ACT_RUN_REQ : Op::NC_ACT_WALK_REQ,
                       ToBytes(req));
}

bool Client::Stop(uint32_t atX, uint32_t atY) {
    if (GetState() != State::IN_GAME) return false;

    SHINE_XY_TYPE xy{ atX, atY };
    m_lastPos = xy;
    m_world.UpdateSelfPosition(atX, atY);
    return m_conn.Send(Op::NC_ACT_STOP_REQ, ToBytes(xy));
}

bool Client::Jump() {
    if (GetState() != State::IN_GAME) return false;

    return m_conn.Send(Op::NC_ACT_JUMP_CMD, {});
}

bool Client::NpcClick(uint16_t npcHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_ACT_NPCCLICK_CMD req{};
    req.npchandle = npcHandle;
    return m_conn.Send(Op::NC_ACT_NPCCLICK_CMD, ToBytes(req));
}

bool Client::Target(uint16_t targetHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_TARGETING_REQ r{}; r.target_handle = targetHandle;
    return m_conn.Send(Op::NC_BAT_TARGETING_REQ, ToBytes(r));
}

bool Client::Untarget(uint16_t targetHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_TARGETING_REQ r{}; r.target_handle = targetHandle;
    return m_conn.Send(Op::NC_BAT_UNTARGET_REQ, ToBytes(r));
}

bool Client::Hit(uint16_t targetHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_HIT_REQ r{}; r.target_handle = targetHandle;
    return m_conn.Send(Op::NC_BAT_HIT_REQ, ToBytes(r));
}

bool Client::Smash(uint16_t targetHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_HIT_REQ r{}; r.target_handle = targetHandle;
    return m_conn.Send(Op::NC_BAT_SMASH_REQ, ToBytes(r));
}

bool Client::Attack(uint16_t targetHandle) {

    if (!Target(targetHandle)) return false;
    return Hit(targetHandle);
}

bool Client::SkillCast(uint16_t skillId, uint16_t targetHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_SKILLCAST_REQ r{};
    r.skill_id      = skillId;
    r.target_handle = targetHandle;
    return m_conn.Send(Op::NC_BAT_SKILLCAST_REQ, ToBytes(r));
}

bool Client::SkillCastAbort() {
    if (GetState() != State::IN_GAME) return false;
    return m_conn.Send(Op::NC_BAT_SKILLCASTABORT_CMD, {});
}

bool Client::Assist(uint16_t partnerHandle) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_BAT_TARGETING_REQ r{}; r.target_handle = partnerHandle;
    return m_conn.Send(Op::NC_BAT_ASSIST_REQ, ToBytes(r));
}

bool Client::Chat(const std::string& text) {
    if (GetState() != State::IN_GAME) return false;

    Writer w;
    w.U8(0);
    const size_t n = (text.size() > 0xFF) ? 0xFF : text.size();
    w.U8((uint8_t)n);
    w.Bytes(text.data(), n);
    return m_conn.Send(Op::NC_ACT_CHAT_REQ, w.Data());
}

bool Client::Shout(const std::string& text) {
    if (GetState() != State::IN_GAME) return false;

    Writer w;
    w.U8(0);
    const size_t n = (text.size() > 0xFF) ? 0xFF : text.size();
    w.U8((uint8_t)n);
    w.Bytes(text.data(), n);
    return m_conn.Send(Op::NC_ACT_SHOUT_CMD, w.Data());
}

bool Client::Whisper(const std::string& recipient, const std::string& text) {
    if (GetState() != State::IN_GAME) return false;

    Writer w;
    w.FixedStr(recipient, 16);
    w.U8(0);
    const size_t n = (text.size() > 0xFF) ? 0xFF : text.size();
    w.U8((uint8_t)n);
    w.Bytes(text.data(), n);

    {
        std::ostringstream o;
        o << "{\"kind\":\"chat\",\"channel\":\"whisper_out\""
          << ",\"speaker_handle\":" << m_selfHandle
          << ",\"speaker_name\":\"" << JsonEsc(recipient) << "\""
          << ",\"text\":\"" << JsonEsc(text) << "\"}";
        EmitEvent(o.str());
    }
    return m_conn.Send(Op::NC_ACT_WHISPER_REQ, w.Data());
}

bool Client::Emote(uint16_t emoteId) {
    if (GetState() != State::IN_GAME) return false;
    PROTO_NC_ACT_EMOTICON_CMD r{}; r.emote_id = emoteId;
    return m_conn.Send(Op::NC_ACT_EMOTICON_CMD, ToBytes(r));
}

bool Client::Pickup(uint32_t itemId) {
    if (GetState() != State::IN_GAME) return false;
    Writer w; w.U32(itemId);
    return m_conn.Send(Op::NC_ITEM_PICKUP_REQ, w.Data());
}

bool Client::UseItem(uint32_t slot) {
    if (GetState() != State::IN_GAME) return false;
    Writer w; w.U32(slot);
    return m_conn.Send(Op::NC_ITEM_USE_REQ, w.Data());
}

bool Client::Respawn() {
    Writer w;
    return m_conn.Send(Op::NC_CHAR_REVIVE_REQ, w.Data());
}

bool Client::Logout() {

    if (m_conn.IsConnected()) {
        m_conn.Send(Op::NC_USER_NORMALLOGOUT_CMD, {});
    }
    Disconnect("logout");
    return true;
}

bool Client::Heartbeat() {

    return m_conn.Send(Op::NC_MISC_HEARTBEAT_REQ, {});
}

bool Client::SendRaw(uint16_t opcode, const std::vector<uint8_t>& payload) {
    return m_conn.Send(opcode, payload);
}

void Client::HeartbeatLoop() {
    using namespace std::chrono_literals;
    while (m_hbRunning.load()) {

        for (int i = 0; i < 30 && m_hbRunning.load(); i++) {
            std::this_thread::sleep_for(1s);
        }
        if (!m_hbRunning.load()) break;
        if (GetState() == State::IN_GAME) Heartbeat();
    }
}

void Client::StartHeartbeat() {
    if (m_hbRunning.exchange(true)) return;
    m_hbThread = std::thread(&Client::HeartbeatLoop, this);
}

void Client::StopHeartbeat() {
    if (!m_hbRunning.exchange(false)) return;
    if (m_hbThread.joinable() &&
        m_hbThread.get_id() != std::this_thread::get_id()) {
        m_hbThread.join();
    }
}

void Client::HandlePacket(const InPacket& pkt) {

    Fiesta::Trace::OnRx(pkt.opcode, pkt.payload);

    switch (pkt.opcode) {

        case Op::NC_MISC_SEED_ACK:
        case Op::NC_MISC_SEED_REQ:
            OnSeedAck(pkt);
            break;

        case Op::NC_USER_CLIENT_RIGHTVERSION_CHECK_ACK:

            SendLoginRequest();
            break;

        case Op::NC_USER_CLIENT_WRONGVERSION_CHECK_ACK:
            EmitEvent("{\"kind\":\"login_error\",\"reason\":\"version_mismatch\"}");
            Disconnect("version_mismatch");
            break;

        case Op::NC_USER_LOGIN_ACK:
            OnLoginAck(pkt);
            break;

        case Op::NC_USER_LOGINFAIL_ACK: {

            uint8_t reason = pkt.payload.empty() ? 0xFF : pkt.payload[0];
            std::ostringstream o;
            o << "{\"kind\":\"login_error\",\"reason\":\"loginfail\",\"code\":"
              << (int)reason << "}";
            EmitEvent(o.str());
            Disconnect("loginfail");
            break;
        }

        case Op::NC_USER_WORLDSELECT_ACK:
            OnWorldSelectAck(pkt);
            break;

        case Op::NC_USER_WILLLOGIN_ACK:
            OnWillLoginAck(pkt);
            break;

        case Op::NC_MAP_LOGINCOMPLETE_CMD:
            OnMapLoginComplete(pkt);
            break;

        case Op::NC_BRIEFINFO_INFORM_CMD: {
            std::ostringstream o;
            o << "{\"kind\":\"brief_info\",\"payload_hex\":\""
              << ToHex(pkt.payload) << "\"}";
            EmitEvent(o.str());
            break;
        }

        case Op::NC_BRIEFINFO_LOGINCHARACTER_CMD:
            OnLoginCharacter(pkt);
            break;
        case Op::NC_BRIEFINFO_BRIEFINFODELETE_CMD:
            OnBriefInfoDelete(pkt);
            break;
        case Op::NC_BRIEFINFO_REGENMOB_CMD:
            OnRegenMob(pkt);
            break;
        case Op::NC_BRIEFINFO_NPC_DISAPPEAR_CMD:
            OnNpcDisappear(pkt);
            break;
        case Op::NC_BRIEFINFO_CHARACTER_CMD:
            OnBriefCharacter(pkt);
            break;
        case Op::NC_BRIEFINFO_PLAYER_LIST_INFO_APPEAR_CMD:
            OnPlayerListAppear(pkt);
            break;
        case Op::NC_CHAR_BASE_CMD:
            OnCharBase(pkt);
            break;

        case Op::NC_ACT_CHAT_REQ:
        case Op::NC_ACT_SHOUT_CMD:
            OnChatLike(pkt);
            break;

        case Op::NC_ACT_WHISPER_REQ:
            OnWhisper(pkt);
            break;

        case Op::NC_MISC_HEARTBEAT_ACK:

            break;

        default: {

            std::ostringstream o;
            o << "{\"kind\":\"raw\",\"direction\":\"in\",\"opcode\":\""
              << "0x" << std::hex << pkt.opcode << std::dec
              << "\",\"len\":" << pkt.payload.size()
              << ",\"hex\":\"" << ToHex(pkt.payload) << "\"}";
            EmitEvent(o.str());
            break;
        }
    }
}

}
