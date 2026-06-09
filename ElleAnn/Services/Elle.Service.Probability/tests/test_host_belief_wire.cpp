#include <doctest/doctest.h>

#include "service/ProbabilityHost.h"
#include "elle/prob/BeliefPersistence.hpp"
#include "elle/prob/MultiplexBeliefPersistence.hpp"

#include <filesystem>
#include <memory>
#include <string>

using namespace elleann::prob;
using elle::prob::InMemoryBeliefPersistence;
using elle::prob::makeInMemoryBeliefPersistence;
using elle::prob::Distribution;

namespace {

Distribution makeDist(std::initializer_list<std::pair<std::int64_t, double>> rows) {
    Distribution d;
    for (auto& [h, m] : rows) d.mass[h] = m;
    d.normalize();
    return d;
}

}

TEST_CASE("ProbabilityHost: attachBeliefPersistence + auto-restore happens before ready()") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    backend->upsertDomain("speaker_intent", makeDist({{1, 0.5}, {2, 0.5}}), 60.0);
    backend->replacePosterior("speaker_intent", makeDist({{1, 0.9}, {2, 0.1}}), 1700000000000LL);

    ProbabilityHost host;
    HostConfig cfg;
    cfg.autoLoadOnStart    = true;
    cfg.useInMemoryLanguage = true;
    cfg.useInMemoryBeliefs  = true;

    host.attachBeliefPersistence(backend);

    REQUIRE(host.start(cfg));
    CHECK(host.ready());

    auto restored = host.loadBeliefsFromPersistence();
    CHECK(restored >= 1);
}

TEST_CASE("ProbabilityHost: useInMemoryBeliefs=true installs an in-memory backend by default") {
    ProbabilityHost host;
    HostConfig cfg;
    cfg.autoLoadOnStart    = true;
    cfg.useInMemoryLanguage = true;
    cfg.useInMemoryBeliefs  = true;

    REQUIRE(host.start(cfg));
    CHECK(host.ready());

    auto backend = host.beliefPersistence();
    REQUIRE(static_cast<bool>(backend));
}

TEST_CASE("ProbabilityHost: beliefJsonlMirrorPath wraps the backend in a multiplex") {
    auto path = std::string("/tmp/elle_host_mirror_") +
                std::to_string(::getpid()) + ".jsonl";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    ProbabilityHost host;
    HostConfig cfg;
    cfg.autoLoadOnStart       = true;
    cfg.useInMemoryLanguage   = true;
    cfg.useInMemoryBeliefs    = true;
    cfg.beliefJsonlMirrorPath = path;

    REQUIRE(host.start(cfg));
    CHECK(host.ready());

    auto backend = host.beliefPersistence();
    REQUIRE(static_cast<bool>(backend));
    auto mux = std::dynamic_pointer_cast<elle::prob::MultiplexBeliefPersistence>(backend);
    REQUIRE(static_cast<bool>(mux));
    CHECK(mux->backendCount() == 2);
}

TEST_CASE("ProbabilityHost: stop + start round-trip preserves the backend pointer") {
    auto backend = std::make_shared<InMemoryBeliefPersistence>();
    ProbabilityHost host;
    HostConfig cfg;
    cfg.autoLoadOnStart    = true;
    cfg.useInMemoryLanguage = true;
    cfg.useInMemoryBeliefs  = true;

    host.attachBeliefPersistence(backend);
    REQUIRE(host.start(cfg));
    host.stop();
    REQUIRE(host.start(cfg));

    CHECK(host.beliefPersistence().get() == backend.get());
}
