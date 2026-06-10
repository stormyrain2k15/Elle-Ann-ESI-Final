#include "HTTPServer.h"

void ElleHTTPService::RegisterDictionaryRoutes() {
        m_router.Register("POST", "/api/dictionary/load", [](const HTTPRequest& req) {

            uint32_t start = 0, limit = 0;
            if (!req.body.empty()) {
                try {
                    json body = req.BodyJSON();
                    start = body.value("start", 0);
                    limit = body.value("limit", 0);
                } catch (const std::exception& e) {
                    return HTTPResponse::Err(400,
                        std::string("malformed JSON body: ") + e.what());
                }
            }
            if (!DictionaryLoader::Instance().StartLoad(start, limit)) {
                auto s = DictionaryLoader::Instance().GetState();
                return HTTPResponse::JSON(409, json({
                    {"error",    "already_running"},
                    {"status",   s.status},
                    {"loaded",   s.loaded},
                    {"failed",   s.failed},
                    {"last_word",s.last_word}
                }).dump());
            }
            return HTTPResponse::Accepted({{"status", "started"}, {"start", start}, {"limit", limit}});
        });
        m_router.Register("GET", "/api/dictionary/load/status", [](const HTTPRequest&) {
            auto s = DictionaryLoader::Instance().GetState();

            if (s.status.empty() || s.status == "idle") {
                ElleDB::DictionaryLoaderState db;
                ElleDB::GetDictionaryLoaderState(db);
                s.status     = db.status;
                s.loaded     = (uint32_t)db.loaded;
                s.failed     = (uint32_t)db.failed;
                s.skipped    = (uint32_t)db.skipped;
                s.last_word  = db.last_word;
                s.error      = db.error;
                s.started_ms = (uint64_t)db.started_ms;
                s.updated_ms = (uint64_t)db.updated_ms;
            }
            return HTTPResponse::OK({
                {"status", s.status}, {"loaded", s.loaded},
                {"failed", s.failed}, {"skipped", s.skipped},
                {"last_word", s.last_word}, {"error", s.error},
                {"started_ms", s.started_ms}, {"updated_ms", s.updated_ms},
                {"total_words_in_db", ElleDB::CountDictionaryWords()}
            });
        });
        m_router.Register("GET", "/api/dictionary/stats", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT COUNT(*), "
                "       SUM(CASE WHEN definition IS NOT NULL AND LEN(definition) > 0 THEN 1 ELSE 0 END), "
                "       SUM(CASE WHEN example    IS NOT NULL AND LEN(example)    > 0 THEN 1 ELSE 0 END) "
                "FROM ElleCore.dbo.dictionary_words;");
            int total = 0, defs = 0, ex = 0;
            if (rs.success && !rs.rows.empty()) {
                total = (int)rs.rows[0].GetIntOr(0, 0);
                defs  = (int)rs.rows[0].GetIntOr(1, 0);
                ex    = (int)rs.rows[0].GetIntOr(2, 0);
            }
            return HTTPResponse::OK({
                {"totalWords", total}, {"totalDefinitions", defs}, {"totalExamples", ex}
            });
        });
        m_router.Register("GET", "/api/dictionary/lookup/{word}", [](const HTTPRequest& req) {
            std::string word = req.headers.at("x-path-word");
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT ISNULL(definition,''), ISNULL(example,''), ISNULL(part_of_speech,'') "
                "FROM ElleCore.dbo.dictionary_words WHERE LOWER(word) = LOWER(?);",
                { word });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            if (rs.rows.empty()) return HTTPResponse::Err(404, "word not found");
            json defs = json::array();
            for (auto& r : rs.rows) {
                defs.push_back({
                    {"definition", r.values.size() > 0 ? r.values[0] : ""},
                    {"example",    r.values.size() > 1 ? r.values[1] : ""},
                    {"part_of_speech", r.values.size() > 2 ? r.values[2] : ""}
                });
            }
            return HTTPResponse::OK({{"word", word}, {"definitions", defs}});
        });
        m_router.Register("GET", "/api/dictionary/search", [](const HTTPRequest& req) {
            std::string prefix = req.QueryParam("prefix");
            if (prefix.empty()) return HTTPResponse::OK(json::array());
            std::string like = prefix + "%";
            auto rs = ElleSQLPool::Instance().QueryParams(
                "SELECT TOP 50 word, ISNULL(definition,'') FROM ElleCore.dbo.dictionary_words "
                "WHERE word LIKE ? ORDER BY word;",
                { like });
            if (!rs.success) return HTTPResponse::Err(500, rs.error);
            json arr = json::array();
            for (auto& r : rs.rows) {
                arr.push_back({
                    {"word", r.values.size() > 0 ? r.values[0] : ""},
                    {"definition", r.values.size() > 1 ? r.values[1] : ""}
                });
            }
            return HTTPResponse::OK(arr);
        });
        m_router.Register("GET", "/api/dictionary/random", [](const HTTPRequest&) {
            auto rs = ElleSQLPool::Instance().Query(
                "SELECT TOP 1 word, ISNULL(definition,'') FROM ElleCore.dbo.dictionary_words "
                "ORDER BY NEWID();");
            if (!rs.success || rs.rows.empty())
                return HTTPResponse::Err(404, "dictionary empty");
            auto& r = rs.rows[0];
            return HTTPResponse::OK({
                {"word", r.values.size() > 0 ? r.values[0] : ""},
                {"definition", r.values.size() > 1 ? r.values[1] : ""}
            });
        });

    }
