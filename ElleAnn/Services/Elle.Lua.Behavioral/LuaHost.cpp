#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

static std::string ElleExeDir();
static std::string ElleRepoRootFromExeDir();

static std::string ElleLuaRoot() {

    std::string cfg = ElleConfig::Instance().GetString(
        "lua.scripts_directory", "");
    if (!cfg.empty()) return cfg;

    return ElleRepoRootFromExeDir() + "\\Lua\\Elle.Lua.Behavioral\\scripts";
}

#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include <cctype>

static std::string ElleExeDir() {
    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string p(exePath);
    size_t last = p.find_last_of("\\/");
    if (last != std::string::npos) p.resize(last);
    else p.clear();
    return p;
}

static std::string ElleRepoRootFromExeDir() {

    std::filesystem::path p(ElleExeDir());
    for (int i = 0; i < 3 && !p.empty(); i++) p = p.parent_path();
    return p.string();
}

class LuaHost {
public:
    LuaHost() : m_L(nullptr) {}
    ~LuaHost() { Shutdown(); }

    bool Initialize() {
        m_L = luaL_newstate();
        if (!m_L) return false;

        luaL_requiref(m_L, "_G", luaopen_base, 1);
        luaL_requiref(m_L, "table", luaopen_table, 1);
        luaL_requiref(m_L, "string", luaopen_string, 1);
        luaL_requiref(m_L, "math", luaopen_math, 1);
        lua_pop(m_L, 4);

        lua_pushnil(m_L); lua_setglobal(m_L, "os");
        lua_pushnil(m_L); lua_setglobal(m_L, "io");
        lua_pushnil(m_L); lua_setglobal(m_L, "loadfile");
        lua_pushnil(m_L); lua_setglobal(m_L, "dofile");

        RegisterBindings();

        LoadAllScripts();

        ELLE_INFO("Lua host initialized (%d scripts loaded)", (int)m_loadedScripts.size());
        return true;
    }

    void Shutdown() {

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_L) {
            lua_close(m_L);
            m_L = nullptr;
        }
    }

    std::string Eval(const std::string& code) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_L) return "ERROR: lua state closed";
        int baseTop = lua_gettop(m_L);
        std::string wrapped = "return " + code;
        int rc = luaL_dostring(m_L, wrapped.c_str());
        if (rc != LUA_OK) {

            lua_pop(m_L, 1);
            if (luaL_dostring(m_L, code.c_str()) != LUA_OK) {
                std::string err = lua_tostring(m_L, -1) ? lua_tostring(m_L, -1) : "?";
                lua_pop(m_L, 1);

                if (lua_gettop(m_L) != baseTop) {
                    lua_settop(m_L, baseTop);
                }
                return "ERROR: " + err;
            }
        }
        std::string result;
        if (lua_gettop(m_L) > baseTop) {
            const char* s = lua_tostring(m_L, -1);
            result = s ? s : "";

            lua_settop(m_L, baseTop);
        }
        return result;
    }

    void ReloadScripts() {
        ELLE_INFO("Hot-reloading Lua scripts (fresh state swap)...");
        std::lock_guard<std::mutex> lock(m_mutex);
        lua_State* newL = luaL_newstate();
        if (!newL) {
            ELLE_ERROR("Lua reload: luaL_newstate() returned null — keeping old state");
            return;
        }
        luaL_requiref(newL, "_G", luaopen_base, 1);
        luaL_requiref(newL, "table", luaopen_table, 1);
        luaL_requiref(newL, "string", luaopen_string, 1);
        luaL_requiref(newL, "math", luaopen_math, 1);
        lua_pop(newL, 4);
        lua_pushnil(newL); lua_setglobal(newL, "os");
        lua_pushnil(newL); lua_setglobal(newL, "io");
        lua_pushnil(newL); lua_setglobal(newL, "loadfile");
        lua_pushnil(newL); lua_setglobal(newL, "dofile");

        lua_State* oldL = m_L;
        std::vector<std::string> oldLoaded = std::move(m_loadedScripts);
        m_L = newL;
        m_loadedScripts.clear();
        RegisterBindings();
        size_t loaded = LoadAllScripts();
        if (loaded == 0) {

            ELLE_ERROR("Lua reload: no scripts loaded — rolling back to old state");
            lua_close(newL);
            m_L = oldL;
            m_loadedScripts = std::move(oldLoaded);
            return;
        }

        lua_close(oldL);
        ELLE_INFO("Lua reload: %zu scripts loaded onto fresh state", loaded);
        ElleDB::RecordMetric("lua_reload_count",       (double)++m_reloadCount);
        ElleDB::RecordMetric("lua_scripts_loaded",     (double)loaded);
        ElleDB::RecordMetric("lua_last_reload_ms",     (double)ELLE_MS_NOW());
    }

private:
    lua_State* m_L;
    std::mutex m_mutex;
    std::vector<std::string> m_loadedScripts;
    uint64_t   m_reloadCount = 0;

    void RegisterBindings() {

        lua_newtable(m_L);

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* msg = luaL_checkstring(L, 1);
            ELLE_INFO("[Lua] %s", msg);
            return 0;
        });
        lua_setfield(m_L, -2, "log");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* name = luaL_checkstring(L, 1);
            ELLE_EMOTION_STATE e{};
            if (!ElleDB::LoadLatestEmotionSnapshot(e)) {
                lua_pushnumber(L, 0.0);
                return 1;
            }
            double v = 0.0;
            if      (!strcmp(name, "valence"))   v = e.valence;
            else if (!strcmp(name, "arousal"))   v = e.arousal;
            else if (!strcmp(name, "dominance")) v = e.dominance;
            else {

                char* endp = nullptr;
                long idx = strtol(name, &endp, 10);
                if (endp != name && idx >= 0 && idx < ELLE_EMOTION_COUNT) {
                    v = e.dimensions[idx];
                }
            }
            lua_pushnumber(L, v);
            return 1;
        });
        lua_setfield(m_L, -2, "get_emotion");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            ELLE_TRUST_STATE ts{};
            if (ElleDB::GetTrustState(ts)) {
                lua_pushinteger(L, (lua_Integer)ts.score);
            } else {
                lua_pushinteger(L, 0);
            }
            return 1;
        });
        lua_setfield(m_L, -2, "get_trust");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* path = luaL_checkstring(L, 1);
            auto val = ElleConfig::Instance().GetString(path, "");
            lua_pushstring(L, val.c_str());
            return 1;
        });
        lua_setfield(m_L, -2, "get_config");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            lua_pushinteger(L, (lua_Integer)ELLE_MS_NOW());
            return 1;
        });
        lua_setfield(m_L, -2, "time_ms");

        lua_newtable(m_L);

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* key  = luaL_checkstring(L, 1);
            const char* text = luaL_checkstring(L, 2);
            ElleSQLPool::Instance().Exec(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
                "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                "  WHERE t.name = 'x_subjective' AND s.name = 'dbo') "
                "CREATE TABLE ElleHeart.dbo.x_subjective ("
                "  subjective_key NVARCHAR(128) NOT NULL PRIMARY KEY,"
                "  answer_text    NVARCHAR(MAX) NOT NULL,"
                "  updated_ms     BIGINT NOT NULL"
                ");");
            bool ok = ElleSQLPool::Instance().QueryParams(
                "MERGE ElleHeart.dbo.x_subjective AS tgt "
                "USING (SELECT ? AS k, ? AS t, ? AS m) AS src "
                "  ON tgt.subjective_key = src.k "
                "WHEN MATCHED THEN UPDATE SET answer_text = src.t, updated_ms = src.m "
                "WHEN NOT MATCHED THEN INSERT (subjective_key, answer_text, updated_ms) "
                "  VALUES (src.k, src.t, src.m);",
                { std::string(key), std::string(text),
                  std::to_string((int64_t)ELLE_MS_NOW()) }).success;
            lua_pushboolean(L, ok ? 1 : 0);
            return 1;
        });
        lua_setfield(m_L, -2, "upsert_subjective");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* key = luaL_checkstring(L, 1);
            auto rs = ElleSQLPool::Instance().QueryParams(
                "IF EXISTS (SELECT 1 FROM sys.tables t "
                "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                "  WHERE t.name = 'x_subjective' AND s.name = 'dbo') "
                "SELECT answer_text FROM ElleHeart.dbo.x_subjective "
                " WHERE subjective_key = ?;",
                { std::string(key) });
            if (rs.success && !rs.rows.empty() && !rs.rows[0].values.empty())
                lua_pushstring(L, rs.rows[0].values[0].c_str());
            else
                lua_pushstring(L, "");
            return 1;
        });
        lua_setfield(m_L, -2, "get_subjective");

        lua_setfield(m_L, -2, "db");

        lua_newtable(m_L);

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            auto rs = ElleSQLPool::Instance().Query(
                "IF EXISTS (SELECT 1 FROM sys.tables t JOIN sys.schemas s "
                "ON s.schema_id = t.schema_id WHERE t.name = 'x_hormone_snapshots' "
                "AND s.name = 'dbo') "
                "SELECT TOP 1 phase FROM ElleHeart.dbo.x_hormone_snapshots "
                "ORDER BY taken_ms DESC;");
            if (rs.success && !rs.rows.empty() && rs.rows[0].values.size() > 0)
                lua_pushstring(L, rs.rows[0].values[0].c_str());
            else
                lua_pushstring(L, "unknown");
            return 1;
        });
        lua_setfield(m_L, -2, "phase");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* name = luaL_checkstring(L, 1);
            int col = -1;
            if      (!strcmp(name, "estrogen"))     col = 0;
            else if (!strcmp(name, "progesterone")) col = 1;
            else if (!strcmp(name, "testosterone")) col = 2;
            else if (!strcmp(name, "oxytocin"))     col = 3;
            else if (!strcmp(name, "serotonin"))    col = 4;
            else if (!strcmp(name, "dopamine"))     col = 5;
            else if (!strcmp(name, "cortisol"))     col = 6;
            else if (!strcmp(name, "prolactin"))    col = 7;
            else if (!strcmp(name, "hcg"))          col = 8;
            if (col < 0) { lua_pushnumber(L, 0.0); return 1; }
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 estrogen, progesterone, testosterone, oxytocin, "
                "       serotonin, dopamine, cortisol, prolactin, hcg "
                "FROM ElleHeart.dbo.x_hormone_snapshots ORDER BY taken_ms DESC;");
            if (rs.success && !rs.rows.empty())
                lua_pushnumber(L, rs.rows[0].GetFloatOr(col, 0.0));
            else
                lua_pushnumber(L, 0.0);
            return 1;
        });
        lua_setfield(m_L, -2, "hormone");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* trait = luaL_checkstring(L, 1);
            int col = -1;
            if      (!strcmp(trait, "warmth"))        col = 0;
            else if (!strcmp(trait, "verbal") ||
                     !strcmp(trait, "verbal_fluency")) col = 1;
            else if (!strcmp(trait, "empathy"))       col = 2;
            else if (!strcmp(trait, "introspection")) col = 3;
            else if (!strcmp(trait, "arousal"))       col = 4;
            else if (!strcmp(trait, "fatigue"))       col = 5;
            if (col < 0) { lua_pushnumber(L, 1.0); return 1; }
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 warmth, verbal_fluency, empathy, introspection, "
                "       arousal, fatigue "
                "FROM ElleHeart.dbo.x_modulation_log ORDER BY computed_ms DESC;");
            if (rs.success && !rs.rows.empty())
                lua_pushnumber(L, rs.rows[0].GetFloatOr(col, 1.0));
            else
                lua_pushnumber(L, 1.0);
            return 1;
        });
        lua_setfield(m_L, -2, "modulation");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT active FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            bool a = false;
            if (rs.success && !rs.rows.empty()) a = rs.rows[0].GetIntOr(0, 0) != 0;
            lua_pushboolean(L, a ? 1 : 0);
            return 1;
        });
        lua_setfield(m_L, -2, "is_pregnant");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT active, ISNULL(conceived_ms, 0) "
                "FROM ElleHeart.dbo.x_pregnancy_state WHERE id = 1;");
            int wk = 0;
            if (rs.success && !rs.rows.empty() && rs.rows[0].GetIntOr(0, 0) != 0) {
                uint64_t conc = (uint64_t)rs.rows[0].GetIntOr(1, 0);
                uint64_t now = ELLE_MS_NOW();
                if (conc > 0 && now >= conc)
                    wk = (int)((now - conc) / (7ULL * 86400000ULL));
            }
            lua_pushinteger(L, wk);
            return 1;
        });
        lua_setfield(m_L, -2, "gestational_week");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            const char* kind = luaL_checkstring(L, 1);
            uint64_t since = ELLE_MS_NOW() - 2ULL * 3600000ULL;
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 1 intensity FROM ElleHeart.dbo.x_symptoms "
                " WHERE kind = ? AND observed_ms >= ? "
                " ORDER BY observed_ms DESC;",
                { std::string(kind), std::to_string((long long)since) });
            if (rs.success && !rs.rows.empty())
                lua_pushnumber(L, rs.rows[0].GetFloatOr(0, 0.0));
            else
                lua_pushnumber(L, 0.0);
            return 1;
        });
        lua_setfield(m_L, -2, "symptom_intensity");

        lua_pushcfunction(m_L, [](lua_State* L) -> int {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT stage FROM ElleHeart.dbo.x_lifecycle WHERE id = 1;");
            if (rs.success && !rs.rows.empty() && rs.rows[0].values.size() > 0)
                lua_pushstring(L, rs.rows[0].values[0].c_str());
            else
                lua_pushstring(L, "reproductive");
            return 1;
        });
        lua_setfield(m_L, -2, "lifecycle_stage");

        lua_setfield(m_L, -2, "x");

        lua_setglobal(m_L, "elle");
    }

    size_t LoadAllScripts() {
        auto scriptsDir = ElleLuaRoot();

        {
            std::filesystem::path p(scriptsDir);
            if (p.is_relative()) {
                std::filesystem::path repoRoot(ElleRepoRootFromExeDir());
                if (!repoRoot.empty()) {
                    auto candidate = (repoRoot / p).lexically_normal();
                    if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate)) {
                        scriptsDir = candidate.string();
                    } else {

                        std::filesystem::path exeBase(ElleExeDir());
                        if (!exeBase.empty()) {
                            scriptsDir = (exeBase / p).lexically_normal().string();
                        }
                    }
                }
            }
        }

        m_loadedScripts.clear();

        std::vector<std::string> scripts;
        try {
            std::filesystem::path root(scriptsDir);
            if (std::filesystem::exists(root) && std::filesystem::is_directory(root)) {
                for (auto& entry : std::filesystem::directory_iterator(root)) {
                    if (!entry.is_regular_file()) continue;
                    auto ext = entry.path().extension().string();

                    std::transform(ext.begin(), ext.end(), ext.begin(),
                                   [](unsigned char c){ return (char)std::tolower(c); });
                    if (ext == ".lua") scripts.push_back(entry.path().filename().string());
                }
                std::sort(scripts.begin(), scripts.end());
            } else {
                ELLE_WARN("Lua scripts directory missing or not a directory: %s",
                          scriptsDir.c_str());
            }
        } catch (const std::exception& e) {
            ELLE_WARN("Lua scripts directory scan error: %s", e.what());
        }

        for (auto& script : scripts) {
            std::string path = scriptsDir + "\\" + script;
            if (luaL_dofile(m_L, path.c_str()) != LUA_OK) {
                ELLE_WARN("Failed to load Lua script: %s — %s",
                          script.c_str(), lua_tostring(m_L, -1));
                lua_pop(m_L, 1);
            } else {
                m_loadedScripts.push_back(script);
            }
        }
        ELLE_INFO("Lua: loaded %zu script(s) from %s",
                  m_loadedScripts.size(), scriptsDir.c_str());
        return m_loadedScripts.size();
    }
};

class ElleLuaService : public ElleServiceBase {
public:
    ElleLuaService()
        : ElleServiceBase(SVC_LUA_BEHAVIORAL, "ElleLuaBehavioral",
                          "Elle-Ann Lua Behavioral Engine",
                          "Hot-reloadable behavioral scripts for personality and reasoning") {}

protected:
    bool OnStart() override {
        if (!m_host.Initialize()) return false;

        auto reloadInterval = (uint32_t)ElleConfig::Instance().GetInt("lua.reload_interval_seconds", 30);
        SetTickInterval(reloadInterval * 1000);

        ELLE_INFO("Lua behavioral service started");
        return true;
    }

    void OnStop() override {
        m_host.Shutdown();
        ELLE_INFO("Lua behavioral service stopped");
    }

    void OnTick() override {
        if (ElleConfig::Instance().GetBool("lua.auto_reload", true)) {
            m_host.ReloadScripts();
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        if (msg.header.msg_type == IPC_LUA_EVAL) {
            std::string code = msg.GetStringPayload();
            std::string result = m_host.Eval(code);

            auto resp = ElleIPCMessage::Create(IPC_LUA_EVAL, SVC_LUA_BEHAVIORAL, sender);
            resp.SetStringPayload(result);
            GetIPCHub().Send(sender, resp);
        }
        else if (msg.header.msg_type == IPC_CONFIG_RELOAD) {

            std::string script = msg.GetStringPayload();
            if (!script.empty()) {
                ELLE_INFO("Lua: reload requested by %d (script=%s)",
                          (int)sender, script.c_str());
            } else {
                ELLE_INFO("Lua: reload requested by %d", (int)sender);
            }

            ElleConfig::Instance().Reload();
            m_host.ReloadScripts();
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL };
    }

private:
    LuaHost m_host;
};

ELLE_SERVICE_MAIN(ElleLuaService)
