#include <string>

#include "/app/ElleAnn/Shared/ElleConfig.h"
#include "/app/ElleAnn/Shared/ElleConfig.cpp"

#include <cassert>
#include <cstdio>
#include <fstream>

static int g_fails = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::fprintf(stderr, "FAIL: %s\n", msg); g_fails++; } \
} while (0)

int main() {

    const char* fixture = R"JSON({
        "llm": {
            "primary_provider": "groq",
            "providers": {
                "groq":   { "api_key": "sk-LIVE-SECRET-1", "model": "llama-3.3-70b" },
                "openai": { "api_key": "sk-LIVE-SECRET-2", "model": "gpt-4o" }
            }
        },
        "http_server": {
            "bind_address":  "0.0.0.0",
            "admin_key":     "ADMIN-SECRET-XYZ",
            "jwt_secret":    "JWT-SECRET-XYZ",
            "no_auth":       1
        },
        "fiesta": {
            "host":     "127.0.0.1",
            "port":     9010,
            "username": "operator",
            "password": "REAL-PWD"
        }
    })JSON";
    std::ofstream f("/tmp/_tcfg.json"); f << fixture; f.close();

    ElleConfig::Instance().LoadDefaults();
    bool ok = ElleConfig::Instance().LayerJsonOver("/tmp/_tcfg.json");
    CHECK(ok, "LayerJsonOver should accept fixture");

    std::string dump = ElleConfig::Instance().DumpJsonRedacted();
    CHECK(!dump.empty(), "dump non-empty");

    CHECK(dump.find("sk-LIVE-SECRET-1") == std::string::npos, "groq api_key redacted");
    CHECK(dump.find("sk-LIVE-SECRET-2") == std::string::npos, "openai api_key redacted");
    CHECK(dump.find("ADMIN-SECRET-XYZ") == std::string::npos, "admin_key redacted");
    CHECK(dump.find("JWT-SECRET-XYZ")   == std::string::npos, "jwt_secret redacted");
    CHECK(dump.find("REAL-PWD")         == std::string::npos, "fiesta.password redacted");

    CHECK(dump.find("\"primary_provider\"") != std::string::npos, "key preserved");
    CHECK(dump.find("\"groq\"")             != std::string::npos, "string value preserved");
    CHECK(dump.find("\"127.0.0.1\"")        != std::string::npos, "host preserved");
    CHECK(dump.find("\"0.0.0.0\"")          != std::string::npos, "bind_address preserved");
    CHECK(dump.find("9010")                 != std::string::npos, "numeric port preserved");
    CHECK(dump.find("\"***\"")              != std::string::npos, "redaction marker present");
    CHECK(dump.find("\"operator\"")         != std::string::npos, "non-secret string preserved");

    if (g_fails == 0) std::printf("ALL DUMP-REDACTED TESTS PASS\n");
    else              std::printf("%d test(s) FAILED\n", g_fails);
    return g_fails == 0 ? 0 : 1;
}
