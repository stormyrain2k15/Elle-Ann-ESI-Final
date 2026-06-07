#include "/app/ElleAnn/Shared/ElleLuaScalarReader.h"
#include "/app/ElleAnn/Shared/ElleLuaScalarReader.cpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

static int g_fails = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::fprintf(stderr, "FAIL: %s\n", msg); g_fails++; } \
} while (0)

static void writeFile(const std::string& path, const std::string& body) {
    std::ofstream f(path); f << body;
}

int main() {

    {
        const char* fixture = R"LUA(
            -- inline comment
            --[[ block comment with ElleAnn.fiesta.region = "FAKE" ]]
            ElleAnn.fiesta = {
                region = "usa",
                headless_client_enabled = false,
                headless_tick_hz = 50,
            }
        )LUA";
        writeFile("/tmp/_tlsr1.lua", fixture);
        ElleLuaScalarReader r("/tmp/_tlsr1.lua");
        CHECK(r.IsLoaded(),                                       "T1: load");
        CHECK(r.GetString("region")               == "usa",       "T1: region");
        CHECK(r.GetBool  ("headless_client_enabled", true) == false,
              "T1: headless_client_enabled");
        CHECK(r.GetInt   ("headless_tick_hz")     == 50,          "T1: tick_hz");
    }

    {
        const char* fixture = R"LUA(
            ElleAnn.fiesta.region = "usa"
            --[[
            ElleAnn.fiesta.region = "ignore_me"
            ]]
            -- ElleAnn.fiesta.region = "also_ignore"
            ElleAnn.fiesta.region = "china"
        )LUA";
        writeFile("/tmp/_tlsr2.lua", fixture);
        ElleLuaScalarReader r("/tmp/_tlsr2.lua");
        CHECK(r.GetString("ElleAnn.fiesta.region") == "china",
              "T2: last write wins, comments stripped");
    }

    {
        ElleLuaScalarReader r("/tmp/_tlsr2.lua");
        CHECK(r.GetString("nonexistent.key", "fallback") == "fallback",
              "T3: missing → default");
        CHECK(r.GetInt("nonexistent.key", -1) == -1,
              "T3: missing int → default");
    }

    {
        ElleLuaScalarReader r("/tmp/_does_not_exist.lua");
        CHECK(!r.IsLoaded(),                              "T4: !IsLoaded");
        CHECK(r.GetString("anything", "x") == "x",        "T4: returns default");
    }

    {
        const char* fixture = R"LUA(
            ElleAnn.fiesta.region_mode = "trap"
            ElleAnn.fiesta.region      = "usa"
        )LUA";
        writeFile("/tmp/_tlsr5.lua", fixture);
        ElleLuaScalarReader r("/tmp/_tlsr5.lua");
        CHECK(r.GetString("ElleAnn.fiesta.region")      == "usa",
              "T5a: region exact match");
        CHECK(r.GetString("ElleAnn.fiesta.region_mode") == "trap",
              "T5b: region_mode exact match");
    }

    {
        const char* fixture = R"LUA(
            ElleAnn.x = seconds(5)
            ElleAnn.y = 5000
        )LUA";
        writeFile("/tmp/_tlsr6.lua", fixture);
        ElleLuaScalarReader r("/tmp/_tlsr6.lua");
        CHECK(r.GetInt("ElleAnn.x", -1) == -1, "T6: function-call rhs rejected");
        CHECK(r.GetInt("ElleAnn.y", -1) == 5000, "T6: bare int parsed");
    }

    if (g_fails == 0) std::printf("ALL TESTS PASS\n");
    else              std::printf("%d test(s) FAILED\n", g_fails);
    return g_fails == 0 ? 0 : 1;
}
