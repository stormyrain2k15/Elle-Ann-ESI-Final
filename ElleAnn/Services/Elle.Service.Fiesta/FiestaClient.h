#pragma once
#ifndef ELLE_FIESTA_CLIENT_H
#define ELLE_FIESTA_CLIENT_H

#include "FiestaBriefInfoRing.h"
#include "FiestaConnection.h"
#include "FiestaPacket.h"
#include "FiestaWorldModel.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace Fiesta {

enum class State {
    DISCONNECTED,
    LOGIN_CONNECTING,
    LOGIN_VERSION,
    LOGIN_AUTH,
    WORLD_LIST,
    WM_HANDOFF,
    WM_AUTH,
    ZONE_HANDOFF,
    ZONE_AUTH,
    IN_GAME,
};

inline const char* StateName(State s) {
    switch (s) {
        case State::DISCONNECTED:      return "disconnected";
        case State::LOGIN_CONNECTING:  return "login_connecting";
        case State::LOGIN_VERSION:     return "login_version";
        case State::LOGIN_AUTH:        return "login_auth";
        case State::WORLD_LIST:        return "world_list";
        case State::WM_HANDOFF:        return "wm_handoff";
        case State::WM_AUTH:           return "wm_auth";
        case State::ZONE_HANDOFF:      return "zone_handoff";
        case State::ZONE_AUTH:         return "zone_auth";
        case State::IN_GAME:           return "in_game";
    }
    return "?";
}

struct GameEvent {
    std::string kindJson;
};

class Client {
public:
    using EventCallback     = std::function<void(const GameEvent&)>;
    using RawPacketCallback = std::function<void(const InPacket&, bool inbound)>;

    Client() = default;
    ~Client() { Disconnect("destructor"); }

    void SetOnEvent(EventCallback cb)         { m_onEvent     = std::move(cb); }
    void SetOnRawPacket(RawPacketCallback cb) { m_onRawPacket = std::move(cb); }

    State GetState() const {
        std::lock_guard<std::mutex> lk(m_mx);
        return m_state;
    }

    void SetVersionKey(const std::string& utf8Key) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_versionKey = utf8Key;
    }

    void SetCipherKind(CipherKind kind) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_conn.MutableCipher().SetKind(kind);
    }
    CipherKind GetCipherKind() const {
        std::lock_guard<std::mutex> lk(m_mx);
        return m_conn.GetCipher().Kind();
    }

    struct DiagSnapshot {
        bool       connected;
        std::string cipher_kind;
        uint16_t   last_seed;
        uint64_t   pkt_in;
        uint64_t   pkt_out;
        uint64_t   bytes_in;
        uint64_t   bytes_out;
        std::string login_host;
        uint16_t   login_port;
    };
    DiagSnapshot GetDiagSnapshot() const {
        std::lock_guard<std::mutex> lk(m_mx);
        DiagSnapshot s;
        s.connected   = m_conn.IsConnected();
        s.cipher_kind = (m_conn.GetCipher().Kind() == CipherKind::XOR499)
                            ? "xor499" : "lcg";
        s.last_seed   = m_conn.GetCipher().LastSeed();
        s.pkt_in      = m_conn.PacketsIn();
        s.pkt_out     = m_conn.PacketsOut();
        s.bytes_in    = m_conn.BytesIn();
        s.bytes_out   = m_conn.BytesOut();
        s.login_host  = "";
        s.login_port  = 0;
        return s;
    }

    void SetSpawnApps(const std::string& s) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_spawnApps = s;
    }

    bool Connect(const std::string& loginHost, uint16_t loginPort,
                 const std::string& user, const std::string& pass);

    void Disconnect(const std::string& why);

    bool SelectWorld(uint8_t worldNo);

    bool MoveTo(uint32_t toX, uint32_t toY, bool run = true);

    bool Stop(uint32_t atX, uint32_t atY);

    bool Jump();

    bool NpcClick(uint16_t npcHandle);

    bool Target(uint16_t targetHandle);
    bool Untarget(uint16_t targetHandle);
    bool Hit(uint16_t targetHandle);
    bool Smash(uint16_t targetHandle);

    bool Attack(uint16_t targetHandle);

    bool SkillCast(uint16_t skillId, uint16_t targetHandle);
    bool SkillCastAbort();

    bool Assist(uint16_t partnerHandle);

    bool Chat(const std::string& text);
    bool Shout(const std::string& text);

    bool Whisper(const std::string& recipient, const std::string& text);

    bool Emote(uint16_t emoteId);

    bool Pickup(uint32_t itemId);
    bool UseItem(uint32_t slot);

    bool Respawn();

    bool Logout();

    bool Heartbeat();

    bool SendRaw(uint16_t opcode, const std::vector<uint8_t>& payload);

    const BriefInfoRing& Ring() const { return m_briefRing; }

    const WorldModel& World() const { return m_world; }
    WorldModel&       MutableWorld()      { return m_world; }

private:
    void HandlePacket(const InPacket& pkt);
    void EmitEvent(const std::string& kindJson);
    void SetState(State s);
    void WireConnectionCallbacks();

    bool SendVersionCheck();
    bool SendLoginRequest();
    bool SendWorldSelect(uint8_t worldNo);
    bool SendWillLogin();
    bool SendLoginWorld();
    bool SendMapLogin();

    void OnSeedAck(const InPacket& pkt);

    void OnLoginAck(const InPacket& pkt);
    void OnWorldSelectAck(const InPacket& pkt);
    void OnWillLoginAck(const InPacket& pkt);
    void OnMapLoginComplete(const InPacket& pkt);

    void OnLoginCharacter(const InPacket& pkt);
    void OnBriefInfoDelete(const InPacket& pkt);
    void OnRegenMob(const InPacket& pkt);
    void OnNpcDisappear(const InPacket& pkt);
    void OnBriefCharacter(const InPacket& pkt);
    void OnPlayerListAppear(const InPacket& pkt);
    void OnCharBase(const InPacket& pkt);

    void OnChatLike(const InPacket& pkt);

    void OnWhisper(const InPacket& pkt);

    void HeartbeatLoop();
    void StartHeartbeat();
    void StopHeartbeat();

    bool ReconnectTo(const std::string& host, uint16_t port);

    mutable std::mutex     m_mx;
    Connection             m_conn;
    State                  m_state = State::DISCONNECTED;

    std::string            m_user;
    std::string            m_pass;

    PROTO_NC_USER_WORLDSELECT_ACK  m_wmHandoff{};
    PROTO_NC_USER_WILLLOGIN_ACK    m_zoneHandoff{};

    uint8_t                m_selectedWorld = 0;

    SHINE_XY_TYPE          m_lastPos{0, 0};

    uint16_t               m_selfHandle = 0;

    BriefInfoRing          m_briefRing;

    WorldModel             m_world;

    std::string            m_versionKey = "SDO_FIESTA_NEW_VER_KEY";
    std::string            m_spawnApps;

    std::thread            m_hbThread;
    std::atomic<bool>      m_hbRunning{false};

    EventCallback          m_onEvent;
    RawPacketCallback      m_onRawPacket;
};

}
#endif
