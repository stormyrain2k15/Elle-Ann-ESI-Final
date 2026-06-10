#include "HTTPServer.h"

void ElleHTTPService::RegisterMiscRoutes() {
        m_router.Register("GET", "/api/brain/status", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"status", "active"}});
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/api/hal/status", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"hardware", "nominal"}});
        }, AUTH_PUBLIC);

        m_router.Register("GET", "/api/diag/game-auth", [](const HTTPRequest&) {
            return HTTPResponse::OK(GetGameAuthDiag());
        }, AUTH_PUBLIC);
        m_router.Register("POST", "/api/admin/reload", [](const HTTPRequest&) {
            try {
                ElleConfig::Instance().Reload();
                return HTTPResponse::OK({{"reloaded", true}});
            } catch (const std::exception& e) {
                ELLE_ERROR("admin/reload failed: %s", e.what());
                return HTTPResponse::Err(500, std::string("reload failed: ") + e.what());
            }

        }, AUTH_ADMIN);

        auto shnResolveRoot = [](const std::string& root,
                                 std::string& outAbsDir,
                                 std::string& outErr) -> bool {

            std::string r = root;
            for (auto& c : r) c = (char)std::tolower((unsigned char)c);
            std::string sub;
            if      (r == "hero"     || r == "9data/hero")     sub = "9Data\\Hero";
            else if (r == "resystem" || r == "/resystem")      sub = "ReSystem";
            else { outErr = "root must be 'Hero' or 'ReSystem'"; return false; }

            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            std::filesystem::path exeDir =
                std::filesystem::path(buf).parent_path();
            std::filesystem::path full = exeDir / sub;
            std::error_code ec;
            std::filesystem::create_directories(full, ec);
            outAbsDir = full.string();
            return true;
        };

        auto shnValidateName = [](const std::string& name,
                                  std::string& outErr) -> bool {
            if (name.empty() || name.size() > 128) {
                outErr = "name must be 1..128 chars"; return false;
            }
            if (name.find('/') != std::string::npos ||
                name.find('\\') != std::string::npos ||
                name.find("..") != std::string::npos) {
                outErr = "name must not contain path separators or '..'";
                return false;
            }

            auto dot = name.rfind('.');
            if (dot == std::string::npos) { outErr = "name missing .shn extension"; return false; }
            std::string ext = name.substr(dot);
            for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
            if (ext != ".shn") { outErr = "name must end with .shn"; return false; }
            return true;
        };

    }
