#include "HTTPServer.h"

void ElleHTTPService::RegisterEmotionRoutes() {
        m_router.Register("GET", "/api/emotions", [this](const HTTPRequest&) {
            json j = {
                {"valence", m_cachedEmotions.valence},
                {"arousal", m_cachedEmotions.arousal},
                {"dominance", m_cachedEmotions.dominance},
                {"tick", m_cachedEmotions.tick_count},
                {"source", "live"}
            };
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/emotions/dimensions", [this](const HTTPRequest&) {

            json j = json::array();
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                j.push_back({
                    {"id", i},
                    {"name", kEmotionMeta[i].name},
                    {"category", kEmotionMeta[i].category},
                    {"weight", (double)m_cachedEmotions.dimensions[i]}
                });
            }
            return HTTPResponse::OK(j);
        });
        m_router.Register("GET", "/api/emotions/dimensions/{name}", [this](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-name");
            std::string needle = name;
            std::transform(needle.begin(), needle.end(), needle.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                if (needle == kEmotionMeta[i].name) {
                    return HTTPResponse::OK({
                        {"id", i},
                        {"name", kEmotionMeta[i].name},
                        {"category", kEmotionMeta[i].category},
                        {"weight", (double)m_cachedEmotions.dimensions[i]}
                    });
                }
            }
            return HTTPResponse::Err(404, "emotion not found");
        });
        m_router.Register("PUT", "/api/emotions/dimensions/{name}", [this](const HTTPRequest& req) {

            std::string name = req.headers.at("x-path-name");
            json body = req.BodyJSON();
            float weight = body.value("weight", 0.0f);
            int id = -1;
            std::string lc = name;
            std::transform(lc.begin(), lc.end(), lc.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                if (lc == kEmotionMeta[i].name) { id = i; break; }
            }
            if (id < 0) return HTTPResponse::Err(404, "emotion not found");

            auto msg = ElleIPCMessage::Create(IPC_EMOTION_UPDATE, SVC_HTTP_SERVER, SVC_EMOTIONAL);
            struct { uint32_t emoId; float absolute; } payload;
            payload.emoId = (uint32_t)id;
            payload.absolute = weight;
            msg.payload.resize(sizeof(payload));
            memcpy(msg.payload.data(), &payload, sizeof(payload));
            msg.header.payload_size = sizeof(payload);
            GetIPCHub().Send(SVC_EMOTIONAL, msg);

            return HTTPResponse::OK({{"id", id}, {"name", kEmotionMeta[id].name},
                                      {"weight", weight}, {"dispatched", true}});
        });
        m_router.Register("GET", "/api/emotions/weights", [this](const HTTPRequest&) {

            json j = json::array();
            for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
                j.push_back((double)m_cachedEmotions.dimensions[i]);
            }
            return HTTPResponse::OK({
                {"weights", j},
                {"count", ELLE_EMOTION_COUNT},
                {"valence",   m_cachedEmotions.valence},
                {"arousal",   m_cachedEmotions.arousal},
                {"dominance", m_cachedEmotions.dominance}
            });
        });

    }
