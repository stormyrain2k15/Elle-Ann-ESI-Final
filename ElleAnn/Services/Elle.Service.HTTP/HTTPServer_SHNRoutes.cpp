#include "HTTPServer.h"

void ElleHTTPService::RegisterSHNRoutes() {
        m_router.Register("POST", "/api/shn/save",
            [shnResolveRoot, shnValidateName](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                std::string root = body.value("root", std::string("Hero"));
                std::string name = body.value("name", std::string(""));
                std::string b64  = body.value("bytes_b64", std::string(""));
                if (name.empty() || b64.empty()) {
                    return HTTPResponse::Err(400,
                        "name and bytes_b64 are required");
                }
                std::string err;
                if (!shnValidateName(name, err))
                    return HTTPResponse::Err(400, err);

                std::string absDir;
                if (!shnResolveRoot(root, absDir, err))
                    return HTTPResponse::Err(400, err);

                std::string decoded;
                decoded.reserve(b64.size() * 3 / 4);
                int val = 0, bits = -8;
                for (unsigned char c : b64) {
                    int d;
                    if      (c >= 'A' && c <= 'Z') d = c - 'A';
                    else if (c >= 'a' && c <= 'z') d = c - 'a' + 26;
                    else if (c >= '0' && c <= '9') d = c - '0' + 52;
                    else if (c == '+')             d = 62;
                    else if (c == '/')             d = 63;
                    else continue;
                    val = (val << 6) | d; bits += 6;
                    if (bits >= 0) {
                        decoded += (char)((val >> bits) & 0xFF);
                        bits -= 8;
                    }
                }
                if (decoded.size() < 0x24) {
                    return HTTPResponse::Err(400,
                        "decoded payload too small for a valid SHN "
                        "(expected at least 0x24 header bytes)");
                }

                std::filesystem::path path =
                    std::filesystem::path(absDir) / name;

                std::filesystem::path tmp = path; tmp += ".tmp";
                {
                    std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
                    if (!f.is_open())
                        return HTTPResponse::Err(500,
                            "cannot open " + tmp.string() + " for write");
                    f.write(decoded.data(), (std::streamsize)decoded.size());
                    if (!f.good())
                        return HTTPResponse::Err(500,
                            "write to " + tmp.string() + " failed");
                }
                std::error_code ec;
                std::filesystem::rename(tmp, path, ec);
                if (ec) {
                    std::filesystem::copy_file(tmp, path,
                        std::filesystem::copy_options::overwrite_existing, ec);
                    std::filesystem::remove(tmp, ec);
                }

                ELLE_INFO("SHN saved: root=%s name=%s size=%zu",
                          root.c_str(), name.c_str(), decoded.size());
                ELLE_LOG_HTTP("SHN save OK root=%s name=%s size=%zu",
                              root.c_str(), name.c_str(), decoded.size());

                try {
                    std::filesystem::path historyDir =
                        std::filesystem::path(absDir).parent_path() / "shn_history";
                    std::error_code hec;
                    std::filesystem::create_directories(historyDir, hec);
                    std::filesystem::path hpath = historyDir /
                        (std::filesystem::path(name).stem().string() + ".log");
                    std::ofstream hf(hpath, std::ios::app);
                    if (hf.is_open()) {
                        auto user = req.headers.count("x-auth-user-id")
                            ? req.headers.at("x-auth-user-id")
                            : std::string("anon");
                        std::time_t tt = std::time(nullptr);
                        std::tm lt{};
#ifdef _WIN32
                        localtime_s(&lt, &tt);
#else
                        localtime_r(&tt, &lt);
#endif
                        char iso[32];
                        std::snprintf(iso, sizeof(iso),
                            "%04d-%02d-%02dT%02d:%02d:%02d",
                            lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
                            lt.tm_hour, lt.tm_min, lt.tm_sec);
                        uint64_t ms = (uint64_t)std::chrono::duration_cast<
                            std::chrono::milliseconds>(
                                std::chrono::system_clock::now()
                                    .time_since_epoch()).count();
                        hf << iso << '|' << ms << '|' << user << '|'
                           << decoded.size() << '|' << root << '\n';
                    }
                } catch (...) {

                }

                return HTTPResponse::OK({
                    {"ok",       true},
                    {"path",     path.string()},
                    {"bytes",    (uint64_t)decoded.size()},
                    {"root",     root},
                    {"name",     name}
                });
            } catch (const std::exception& e) {
                return HTTPResponse::Err(400,
                    std::string("invalid body: ") + e.what());
            }
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/list",
            [shnResolveRoot](const HTTPRequest& req) {
            std::string root = req.QueryString("root", "Hero");
            std::string absDir, err;
            if (!shnResolveRoot(root, absDir, err))
                return HTTPResponse::Err(400, err);

            json arr = json::array();
            std::error_code ec;
            if (std::filesystem::exists(absDir, ec)) {
                for (const auto& ent :
                        std::filesystem::directory_iterator(absDir, ec)) {
                    if (!ent.is_regular_file()) continue;
                    auto ext = ent.path().extension().string();
                    for (auto& c : ext)
                        c = (char)std::tolower((unsigned char)c);
                    if (ext != ".shn") continue;
                    arr.push_back({
                        {"name",     ent.path().filename().string()},
                        {"bytes",    (uint64_t)ent.file_size(ec)},
                        {"modified_ms",
                            (uint64_t)std::chrono::duration_cast<
                                std::chrono::milliseconds>(
                                    ent.last_write_time().time_since_epoch()
                                ).count()}
                    });
                }
            }
            return HTTPResponse::OK({
                {"root",    root},
                {"abs_dir", absDir},
                {"files",   arr}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/get",
            [shnResolveRoot, shnValidateName](const HTTPRequest& req) {
            std::string root = req.QueryString("root", "Hero");
            std::string name = req.QueryString("name", "");
            std::string err;
            if (!shnValidateName(name, err))
                return HTTPResponse::Err(400, err);
            std::string absDir;
            if (!shnResolveRoot(root, absDir, err))
                return HTTPResponse::Err(400, err);

            std::filesystem::path path =
                std::filesystem::path(absDir) / name;
            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
                return HTTPResponse::Err(404, "file not found: " + name);

            std::ifstream f(path, std::ios::binary);
            if (!f.is_open())
                return HTTPResponse::Err(500,
                    "cannot open " + path.string() + " for read");
            std::string bytes((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());

            static const char kTbl[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string b64;
            b64.reserve((bytes.size() + 2) / 3 * 4);
            int val = 0, bits = -6;
            for (unsigned char c : bytes) {
                val = (val << 8) | c; bits += 8;
                while (bits >= 0) {
                    b64 += kTbl[(val >> bits) & 0x3F];
                    bits -= 6;
                }
            }
            if (bits > -6)
                b64 += kTbl[((val << 8) >> (bits + 8)) & 0x3F];
            while (b64.size() % 4) b64 += '=';

            return HTTPResponse::OK({
                {"root",      root},
                {"name",      name},
                {"path",      path.string()},
                {"bytes",     (uint64_t)bytes.size()},
                {"bytes_b64", b64}
            });
        }, AUTH_ADMIN);

        m_router.Register("GET", "/api/shn/history",
            [shnValidateName](const HTTPRequest& req) {
            std::string name = req.QueryString("name", "");
            std::string err;
            if (!shnValidateName(name, err))
                return HTTPResponse::Err(400, err);
            int limit = 20;
            try { limit = std::stoi(req.QueryString("limit", "20")); } catch (...) {}
            if (limit <= 0 || limit > 500) limit = 20;

            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            std::filesystem::path hdir =
                std::filesystem::path(buf).parent_path() / "shn_history";
            std::filesystem::path hpath = hdir /
                (std::filesystem::path(name).stem().string() + ".log");

            json entries = json::array();
            std::error_code ec;
            if (!std::filesystem::exists(hpath, ec)) {
                return HTTPResponse::OK({
                    {"name", name}, {"count", 0}, {"entries", entries}
                });
            }
            std::ifstream f(hpath);
            if (!f.is_open())
                return HTTPResponse::Err(500,
                    "cannot open history for " + name);

            std::vector<std::string> lines;
            std::string ln;
            while (std::getline(f, ln)) {
                if (!ln.empty() && ln.back() == '\r') ln.pop_back();
                if (!ln.empty()) lines.push_back(ln);
            }

            const int n = (int)lines.size();
            const int start = std::max(0, n - limit);
            for (int i = n - 1; i >= start; --i) {

                const std::string& L = lines[i];
                std::vector<std::string> parts;
                std::string cur;
                for (char c : L) {
                    if (c == '|') { parts.push_back(cur); cur.clear(); }
                    else cur.push_back(c);
                }
                parts.push_back(cur);
                if (parts.size() < 5) continue;
                entries.push_back({
                    {"iso",   parts[0]},
                    {"ts_ms", (uint64_t)std::strtoull(parts[1].c_str(), nullptr, 10)},
                    {"user",  parts[2]},
                    {"bytes", (uint64_t)std::strtoull(parts[3].c_str(), nullptr, 10)},
                    {"root",  parts[4]},
                });
            }
            return HTTPResponse::OK({
                {"name",    name},
                {"count",   (int)entries.size()},
                {"entries", entries},
            });
        }, AUTH_ADMIN);

    }
