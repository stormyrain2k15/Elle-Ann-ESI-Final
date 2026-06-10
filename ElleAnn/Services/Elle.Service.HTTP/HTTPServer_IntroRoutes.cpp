#include "HTTPServer.h"

void ElleHTTPService::RegisterIntroRoutes() {
        m_router.Register("GET", "/", [](const HTTPRequest&) {
            json j = {
                {"name", "Elle-Ann"},
                {"version", ELLE_VERSION_STRING},
                {"status", "online"},
                {"description", "Emotional Synthetic Intelligence"}
            };
            return HTTPResponse::OK(j);
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/healthz", [](const HTTPRequest&) {
            return HTTPResponse::OK({{"status", "ok"}});
        }, AUTH_PUBLIC);
        m_router.Register("GET", "/api/health", [](const HTTPRequest&) {
            json j = {
                {"status", "alive"},
                {"name", "Elle-Ann"},
                {"version", ELLE_VERSION_STRING}
            };
            return HTTPResponse::OK(j);
        }, AUTH_PUBLIC);

    }
