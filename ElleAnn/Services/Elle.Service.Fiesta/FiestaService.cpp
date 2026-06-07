#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../../Shared/ElleTypes.h"
#include "../../Shared/ElleServiceBase.h"
#include "../../Shared/ElleConfig.h"
#include "../../Shared/ElleLogger.h"
#include "../../Shared/ElleLuaScalarReader.h"
#include "../../Shared/ElleUserContinuity.h"
#include "../../Shared/json.hpp"

#include "FiestaClient.h"
#include "FiestaConsoleTrace.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

static std::vector<uint8_t> HexToBytes(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) { out.clear(); break; }
        out.push_back((uint8_t)((hi << 4) | lo));
    }
    return out;
}

class ElleFiestaService : public ElleServiceBase {
public:
    ElleFiestaService()
        : ElleServiceBase(SVC_FIESTA, "ElleFiesta",
                          "Elle-Ann Fiesta Game Client",
                          "Headless Fiesta-protocol game client. Connects "
                          "to a Fiesta-style MMO server and bridges the "
                          "session to Elle's cognitive engine over IPC.") {}

protected:
    bool OnStart() override {

        bool interactive = false;
#ifdef _WIN32
        DWORD mode_unused = 0;
        interactive = (GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                                      &mode_unused) != 0);
#else
        interactive = (isatty(fileno(stdout)) != 0);
#endif
        if (interactive) {
            Fiesta::Trace::SetEnabled(true);
            Fiesta::Trace::EnsureWindowsConsole();
            Fiesta::Trace::Banner("Elle-Ann Fiesta Game Client (live trace)");
        }

        const auto& cfg = ElleConfig::Instance();
        m_loginHost  = cfg.GetString("fiesta.host", "");
        m_loginPort  = (uint16_t)cfg.GetInt("fiesta.port", 9010);
        m_username   = cfg.GetString("fiesta.username", "");
        m_password   = cfg.GetString("fiesta.password", "");
        m_autoLogin  = cfg.GetBool("fiesta.auto_login", false);
        m_nUserNo    = (int64_t)cfg.GetInt("fiesta.user_no", 0);

        {
            char exePath[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, exePath, MAX_PATH);
            std::string exeDir(exePath);
            size_t s = exeDir.find_last_of("\\/");
            if (s != std::string::npos) exeDir.resize(s + 1);
            std::string luaPath =
                exeDir + "9Data\\Hero\\LuaScript\\elle_settings.lua";

            ElleLuaScalarReader lua(luaPath);
            std::string region = lua.GetString("region", "usa");
            for (auto& c : region) c = (char)std::tolower((unsigned char)c);

            if (region == "china" || region == "cn" || region == "cn2012") {
                m_client.SetCipherKind(Fiesta::CipherKind::LCG);
                m_lastRegion = "china";
                ELLE_INFO("Fiesta cipher: LCG (CN2012) — region=%s",
                          region.c_str());
            } else {
                m_client.SetCipherKind(Fiesta::CipherKind::XOR499);
                m_lastRegion = (region == "usa" || region.empty()) ? "usa" : region;
                ELLE_INFO("Fiesta cipher: XOR499 (DragonFiesta) — region=%s",
                          region.c_str());
            }
        }

        m_client.SetVersionKey(
            cfg.GetString("fiesta.version_key", "SDO_FIESTA_NEW_VER_KEY"));
        m_client.SetSpawnApps(
            cfg.GetString("fiesta.spawn_apps", ""));

        m_client.SetOnEvent([this](const Fiesta::GameEvent& e) {
            BroadcastEvent(e.kindJson);
        });
        m_client.SetOnRawPacket([this](const Fiesta::InPacket& pkt,
                                       bool inbound) {

            if (!inbound) return;
            nlohmann::json j;
            j["kind"]      = "raw";
            j["direction"] = "in";
            j["opcode"]    = OpcodeHex(pkt.opcode);
            j["len"]       = (uint64_t)pkt.payload.size();
            j["hex"]       = BytesToHex(pkt.payload);
            BroadcastEvent(j.dump());
        });

        SetTickInterval(1000);

        if (m_autoLogin) {
            if (m_loginHost.empty() || m_username.empty()) {
                ELLE_WARN("Fiesta auto_login enabled but fiesta.host or "
                          "fiesta.username is empty — staying idle until "
                          "an IPC_FIESTA_COMMAND login arrives.");
            } else {
                ELLE_INFO("Fiesta auto_login → %s:%u as %s",
                          m_loginHost.c_str(), (unsigned)m_loginPort,
                          m_username.c_str());
                if (!m_client.Connect(m_loginHost, m_loginPort,
                                      m_username, m_password)) {
                    ELLE_WARN("Fiesta initial connect failed; tick loop "
                              "will retry with backoff.");
                }
            }
        }

        ELLE_INFO("Fiesta service started");
        return true;
    }

    void OnStop() override {
        m_client.Disconnect("service stop");
        ELLE_INFO("Fiesta service stopped");
    }

    void OnTick() override {

        WriteDiagSnapshotIfDue();

        if (!m_autoLogin) return;
        if (m_loginHost.empty() || m_username.empty()) return;

        if (m_client.GetState() != Fiesta::State::DISCONNECTED) {

            m_backoffMs = 0;
            return;
        }

        const auto& cfg = ElleConfig::Instance();
        const uint64_t floorMs =
            (uint64_t)cfg.GetInt("fiesta.reconnect_min_ms", 5000);
        const uint64_t ceilMs =
            (uint64_t)cfg.GetInt("fiesta.reconnect_max_ms", 60000);

        const uint64_t now = ELLE_MS_NOW();
        if (m_backoffMs == 0) {
            m_backoffMs       = floorMs;
            m_nextAttemptMs   = now + m_backoffMs;
            ELLE_INFO("Fiesta disconnected — first reconnect in %llums",
                      (unsigned long long)m_backoffMs);
            return;
        }
        if (now < m_nextAttemptMs) return;

        ELLE_INFO("Fiesta reconnect attempt → %s:%u",
                  m_loginHost.c_str(), (unsigned)m_loginPort);
        if (!m_client.Connect(m_loginHost, m_loginPort,
                              m_username, m_password)) {
            m_backoffMs = std::min<uint64_t>(m_backoffMs * 2, ceilMs);
            m_nextAttemptMs = now + m_backoffMs;
            ELLE_WARN("Fiesta reconnect failed — next attempt in %llums",
                      (unsigned long long)m_backoffMs);
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID ) override {
        if (msg.header.msg_type != IPC_FIESTA_COMMAND) return;

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(msg.GetStringPayload());
        } catch (const std::exception& e) {
            ELLE_WARN("Fiesta: bad IPC_FIESTA_COMMAND JSON: %s", e.what());
            return;
        }

        const std::string op = j.value("op", std::string(""));
        if (op == "login") {
            const std::string host = j.value("host",     m_loginHost);
            const uint16_t    port = (uint16_t)j.value("port", (int)m_loginPort);
            const std::string user = j.value("username", m_username);
            const std::string pass = j.value("password", m_password);
            ELLE_INFO("Fiesta IPC login → %s:%u as %s",
                      host.c_str(), (unsigned)port, user.c_str());
            m_loginHost = host; m_loginPort = port;
            m_username  = user; m_password  = pass;
            m_client.Connect(host, port, user, pass);
        } else if (op == "select_world") {
            m_client.SelectWorld((uint8_t)j.value("world_no", 0));
        } else if (op == "chat") {
            m_client.Chat(j.value("text", std::string("")));
        } else if (op == "shout") {
            m_client.Shout(j.value("text", std::string("")));
        } else if (op == "whisper") {
            m_client.Whisper(j.value("recipient", std::string("")),
                             j.value("text", std::string("")));
        } else if (op == "emote") {
            m_client.Emote((uint16_t)j.value("emote_id", 0));
        } else if (op == "move") {

            m_client.MoveTo((uint32_t)j.value("x", 0),
                            (uint32_t)j.value("y", 0),
                            j.value("run", true));
        } else if (op == "stop") {
            m_client.Stop((uint32_t)j.value("x", 0),
                          (uint32_t)j.value("y", 0));
        } else if (op == "jump") {
            m_client.Jump();
        } else if (op == "npc_click") {
            m_client.NpcClick((uint16_t)j.value("npc_handle", 0));
        } else if (op == "target") {
            m_client.Target((uint16_t)j.value("target_handle", 0));
        } else if (op == "untarget") {
            m_client.Untarget((uint16_t)j.value("target_handle", 0));
        } else if (op == "hit") {
            m_client.Hit((uint16_t)j.value("target_handle", 0));
        } else if (op == "smash") {
            m_client.Smash((uint16_t)j.value("target_handle", 0));
        } else if (op == "attack") {
            m_client.Attack((uint16_t)j.value("target_handle", 0));
        } else if (op == "skill_cast") {
            m_client.SkillCast((uint16_t)j.value("skill_id", 0),
                               (uint16_t)j.value("target_handle", 0));
        } else if (op == "skill_cast_abort") {
            m_client.SkillCastAbort();
        } else if (op == "assist") {
            m_client.Assist((uint16_t)j.value("partner_handle", 0));
        } else if (op == "pickup") {
            m_client.Pickup((uint32_t)j.value("item_id", 0));
        } else if (op == "use_item") {
            m_client.UseItem((uint32_t)j.value("slot", 0));
        } else if (op == "respawn") {
            m_client.Respawn();
        } else if (op == "logout") {
            m_client.Logout();
        } else if (op == "heartbeat") {
            m_client.Heartbeat();
        } else if (op == "raw") {
            const std::string opcStr = j.value("opcode", std::string(""));
            const uint16_t opcode =
                (uint16_t)std::strtoul(opcStr.c_str(), nullptr, 0);
            const auto bytes = HexToBytes(j.value("hex", std::string("")));
            m_client.SendRaw(opcode, bytes);
        } else if (op == "disconnect") {
            m_client.Disconnect(j.value("reason", std::string("ipc disconnect")));
        } else if (op == "get_world") {

            nlohmann::json snap = m_client.World().SnapshotJson();
            const std::string reqId = j.value("request_id", std::string(""));
            nlohmann::json out = {
                {"kind",     "world_snapshot"},
                {"snapshot", std::move(snap)},
            };
            if (!reqId.empty()) out["request_id"] = reqId;
            BroadcastEvent(out.dump());
        } else {
            ELLE_WARN("Fiesta: unknown IPC op '%s'", op.c_str());
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {

        return { SVC_HEARTBEAT };
    }

private:

    void BroadcastEvent(const std::string& jsonPayload) {

        const ELLE_SERVICE_ID subscribers[] = {
            SVC_COGNITIVE, SVC_HTTP_SERVER, SVC_BONDING
        };
        for (auto target : subscribers) {
            auto m = ElleIPCMessage::Create(IPC_FIESTA_EVENT,
                                            SVC_FIESTA, target);
            m.SetStringPayload(jsonPayload);
            GetIPCHub().Send(target, m);
        }

        if (m_nUserNo > 0) {
            try {
                auto j = nlohmann::json::parse(jsonPayload);
                const std::string kind = j.value("kind", "");
                if (kind == "char_selected") {
                    ElleDB::GameSessionStateRow row;
                    row.nUserNo         = m_nUserNo;
                    row.char_index      = j.value("char_index", -1);
                    row.char_name       = j.value("char_name",  std::string(""));
                    row.zone_id         = j.value("zone_id",    -1);
                    row.zone_name       = j.value("zone_name",  std::string(""));
                    row.last_hp         = j.value("hp",         0);
                    row.last_hp_max     = j.value("hp_max",     0);
                    row.last_session_ms = (uint64_t)ELLE_MS_NOW();
                    ElleDB::UpsertGameSession(row);
                } else if (kind == "disconnect") {
                    ElleDB::MarkGameSessionDisconnected(
                        m_nUserNo, j.value("reason", std::string("unknown")));
                }
            } catch (const std::exception& e) {

                ELLE_DEBUG("FiestaService: GameSession persist skipped: %s",
                           e.what());
            }
        }
    }

    static std::string OpcodeHex(uint16_t op) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "0x%04x", (unsigned)op);
        return std::string(buf);
    }

    static std::string BytesToHex(const std::vector<uint8_t>& bytes) {
        static const char* hex = "0123456789abcdef";
        std::string s; s.reserve(bytes.size() * 2);
        for (uint8_t b : bytes) {
            s += hex[b >> 4];
            s += hex[b & 0xF];
        }
        return s;
    }

    void WriteDiagSnapshotIfDue() {
        const uint64_t now = ELLE_MS_NOW();
        auto snap = m_client.GetDiagSnapshot();

        const bool dirty =
            (snap.connected   != m_lastDiagSnap.connected) ||
            (snap.cipher_kind != m_lastDiagSnap.cipher_kind) ||
            (snap.last_seed   != m_lastDiagSnap.last_seed);
        if (!dirty && now < m_lastDiagWriteMs + 5000) return;

        snap.login_host = m_loginHost;
        snap.login_port = m_loginPort;
        m_lastDiagSnap   = snap;
        m_lastDiagWriteMs = now;

        char exePath[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string exeDir(exePath);
        size_t s = exeDir.find_last_of("\\/");
        if (s != std::string::npos) exeDir.resize(s + 1);
        std::string targetDir = exeDir + "diag\\";
        CreateDirectoryA(targetDir.c_str(), nullptr);
        std::string dst = targetDir + "fiesta_state.json";
        std::string tmp = dst + ".tmp";

        std::ostringstream js;
        js << "{\n"
           << "  \"connected\":   "  << (snap.connected ? "true" : "false") << ",\n"
           << "  \"cipher_kind\": \""<< snap.cipher_kind          << "\",\n"
           << "  \"last_seed\":   "  << snap.last_seed            << ",\n"
           << "  \"pkt_in\":      "  << snap.pkt_in               << ",\n"
           << "  \"pkt_out\":     "  << snap.pkt_out              << ",\n"
           << "  \"bytes_in\":    "  << snap.bytes_in             << ",\n"
           << "  \"bytes_out\":   "  << snap.bytes_out            << ",\n"
           << "  \"login_host\":  \""<< snap.login_host           << "\",\n"
           << "  \"login_port\":  "  << snap.login_port           << ",\n"
           << "  \"updated_ms\":  "  << now                       << ",\n"
           << "  \"region\":      \""<< m_lastRegion              << "\",\n"
           << "  \"user_no\":     "  << m_nUserNo                 << "\n"
           << "}\n";
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) return;
        f << js.str();
        f.close();

        MoveFileExA(tmp.c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING);
    }

    Fiesta::Client m_client;

    std::string m_loginHost;
    uint16_t    m_loginPort = 0;
    std::string m_username;
    std::string m_password;
    bool        m_autoLogin = false;

    int64_t     m_nUserNo   = 0;

    uint64_t m_backoffMs     = 0;
    uint64_t m_nextAttemptMs = 0;

    Fiesta::Client::DiagSnapshot m_lastDiagSnap{};
    uint64_t                     m_lastDiagWriteMs = 0;
    std::string                  m_lastRegion = "usa";
};

ELLE_SERVICE_MAIN(ElleFiestaService)
