#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleIdentityCore.h"
#include "../_Shared/ElleSQLConn.h"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

static bool Sha256Hex(const std::string& data, std::string& outHex) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return false;
    DWORD hashObjectLen = 0, cb = 0;
    if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
                          (PUCHAR)&hashObjectLen, sizeof(DWORD), &cb, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0); return false;
    }
    std::vector<UCHAR> hashObject(hashObjectLen);
    BCRYPT_HASH_HANDLE hHash = nullptr;
    if (BCryptCreateHash(hAlg, &hHash, hashObject.data(), hashObjectLen,
                         nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0); return false;
    }
    if (!data.empty()) {
        BCryptHashData(hHash, (PUCHAR)data.data(), (ULONG)data.size(), 0);
    }
    UCHAR digest[32];
    NTSTATUS st = BCryptFinishHash(hHash, digest, sizeof(digest), 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (st != 0) return false;
    char hex[65];
    for (int i = 0; i < 32; i++) snprintf(hex + i * 2, 3, "%02x", digest[i]);
    hex[64] = '\0';
    outHex = hex;
    return true;
}

class ElleIdentityService : public ElleServiceBase {
public:
    ElleIdentityService()
        : ElleServiceBase(SVC_IDENTITY, "ElleIdentity",
                          "Elle-Ann Identity Guard",
                          "Identity file monitoring and tamper detection") {}

protected:
    bool OnStart() override {

        ElleIdentityCore::Instance().Initialize();
        ElleIdentityCore::Instance().BecomeAuthoritative();
        ELLE_INFO("Identity service is now AUTHORITATIVE (single-writer fabric)");

        auto identityPath = ElleConfig::Instance().GetString("security.identity_file_path",
                                                              "C:\\ElleAnn\\identity.sig");
        m_identityPath = identityPath;
        m_checkInterval = (uint32_t)ElleConfig::Instance().GetInt(
            "security.identity_check_interval_seconds", 60) * 1000;

        ElleSQLPool::Instance().Query(
            "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
            "               JOIN sys.schemas s ON s.schema_id = t.schema_id "
            "               WHERE t.name = 'identity_integrity' AND s.name = 'dbo') "
            "CREATE TABLE ElleCore.dbo.identity_integrity ("
            "  path          NVARCHAR(512) NOT NULL PRIMARY KEY,"
            "  expected_hash NVARCHAR(128) NOT NULL,"
            "  algorithm     NVARCHAR(16)  NOT NULL,"
            "  seeded_ms     BIGINT        NOT NULL,"
            "  updated_ms    BIGINT        NOT NULL"
            ");");

        std::string currentHash;
        if (!ComputeIdentityHash(currentHash)) {
            ELLE_WARN("No identity file found — creating initial identity");
            CreateIdentityFile();
            if (!ComputeIdentityHash(currentHash)) {
                ELLE_ERROR("Identity file still unreadable after create — "
                           "refusing to start without a verifiable identity.");
                return false;
            }
        }

        auto rs = ElleSQLPool::Instance().QueryParams(
            "SELECT expected_hash, algorithm FROM ElleCore.dbo.identity_integrity "
            "WHERE path = ?;",
            { m_identityPath });
        if (!rs.success) {
            ELLE_ERROR("Identity: integrity table read failed: %s — refusing start",
                       rs.error.c_str());
            return false;
        }
        if (rs.rows.empty()) {

            uint64_t now = ELLE_MS_NOW();
            auto ir = ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleCore.dbo.identity_integrity "
                "(path, expected_hash, algorithm, seeded_ms, updated_ms) "
                "VALUES (?, ?, 'sha256', ?, ?);",
                { m_identityPath, currentHash,
                  std::to_string((int64_t)now),
                  std::to_string((int64_t)now) });
            if (!ir.success) {
                ELLE_ERROR("Identity: failed to seed trusted hash: %s",
                           ir.error.c_str());
                return false;
            }
            ELLE_INFO("Identity: seeded authoritative SHA-256 hash for %s",
                      m_identityPath.c_str());
            m_originalHash = currentHash;
        } else {
            const std::string& expected = rs.rows[0].values[0];
            std::string algo = rs.rows[0].values.size() > 1
                                 ? rs.rows[0].values[1]
                                 : std::string("sha256");
            if (algo != "sha256") {
                ELLE_ERROR("Identity: trusted hash uses unsupported algorithm '%s'",
                           algo.c_str());
                return false;
            }
            if (expected != currentHash) {

                ELLE_FATAL("IDENTITY TAMPER AT STARTUP — hash mismatch vs DB.");
                ELLE_FATAL("  Expected (SQL): %s", expected.c_str());
                ELLE_FATAL("  Got (file):     %s", currentHash.c_str());
                return false;
            }
            m_originalHash = expected;
            ELLE_INFO("Identity: hash verified against authoritative SQL row");
        }

        m_watchedPaths.push_back(identityPath);
        m_watchedPaths.push_back(ElleConfig::Instance().GetConfigPath());

        SetTickInterval(m_checkInterval);
        ELLE_INFO("Identity guard started — monitoring %d paths", (int)m_watchedPaths.size());
        return true;
    }

    void OnStop() override {

        ElleIdentityCore::Instance().SaveToDatabase();
        ELLE_INFO("Identity guard stopped");
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID ) override {
        if (msg.header.msg_type == IPC_IDENTITY_MUTATE) {

            ElleIdentityCore::Instance().ApplyMutate(msg.GetStringPayload());
        }
    }

    void OnTick() override {

        ElleIdentityCore::Instance().SaveToDatabase();

        std::string expected;
        {
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT expected_hash FROM ElleCore.dbo.identity_integrity "
                "WHERE path = ?;",
                { m_identityPath });
            if (!rs.success || rs.rows.empty()) {
                ELLE_ERROR("Identity: trusted-source row missing on tick — "
                           "failing closed.");
                Running().store(false);
                return;
            }
            expected = rs.rows[0].values[0];
        }
        m_originalHash = expected;

        std::string currentHash;
        if (ComputeIdentityHash(currentHash)) {
            if (currentHash != expected) {
                ELLE_FATAL("IDENTITY TAMPER DETECTED! Hash mismatch.");
                ELLE_FATAL("  Expected (SQL): %s", expected.c_str());
                ELLE_FATAL("  Got (file):     %s", currentHash.c_str());

                auto msg = ElleIPCMessage::Create(IPC_SHUTDOWN, SVC_IDENTITY, (ELLE_SERVICE_ID)0);
                msg.SetStringPayload("IDENTITY_TAMPER_DETECTED");
                msg.header.flags = ELLE_IPC_FLAG_BROADCAST | ELLE_IPC_FLAG_URGENT;
                GetIPCHub().Broadcast(msg);

                Running().store(false);
            }
        } else {
            ELLE_ERROR("Identity file missing or unreadable — failing closed.");
            Running().store(false);
        }

        CheckConfigIntegrity();
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT };
    }

private:
    std::string m_identityPath;
    std::string m_originalHash;
    std::string m_configHash;
    uint32_t    m_checkInterval = 60000;
    std::vector<std::string> m_watchedPaths;

    bool ComputeIdentityHash(std::string& outHash) {
        std::ifstream file(m_identityPath, std::ios::binary);
        if (!file.is_open()) return false;

        std::stringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        if (!Sha256Hex(content, outHash)) {
            ELLE_ERROR("IdentityGuard: SHA-256 failed (CNG error)");
            return false;
        }
        return true;
    }

    void CreateIdentityFile() {
        std::ofstream file(m_identityPath, std::ios::binary);
        if (file.is_open()) {
            file << "ELLE-ANN ESI v" << ELLE_VERSION_STRING << "\n"
                 << "GUID: " << ELLE_IDENTITY_GUID << "\n"
                 << "Created: " << ELLE_MS_NOW() << "\n"
                 << "Identity: " << ELLE_IDENTITY_NAME << "\n"
                 << "Signature: AUTHENTIC\n";
        }
    }

    void CheckConfigIntegrity() {
        auto& configPath = ElleConfig::Instance().GetConfigPath();
        if (configPath.empty()) return;

        std::ifstream file(configPath, std::ios::binary);
        if (!file.is_open()) return;

        std::stringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        std::string hex;
        if (!Sha256Hex(content, hex)) {
            ELLE_WARN("IdentityGuard: config SHA-256 failed");
            return;
        }

        if (m_configHash.empty()) {
            m_configHash = hex;
        } else if (m_configHash != hex) {
            ELLE_INFO("Config file changed — triggering reload");
            m_configHash = hex;
            ElleConfig::Instance().Reload();
        }
    }
};

ELLE_SERVICE_MAIN(ElleIdentityService)
