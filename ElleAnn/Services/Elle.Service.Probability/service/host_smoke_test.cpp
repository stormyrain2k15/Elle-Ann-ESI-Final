#include "service/ProbabilityHost.h"
#include "elle/prob/SpeakerTrustModel.hpp"

#include <cstdio>
#include <iostream>
#include <string>

int main() {
    using namespace elleann::prob;

    HostConfig cfg;
    cfg.autoLoadOnStart     = true;
    cfg.useInMemoryLanguage = true;

    ProbabilityHost host;
    if (!host.start(cfg)) {
        std::cerr << "[host_smoke] start() failed\n";
        return 1;
    }
    if (!host.ready()) {
        std::cerr << "[host_smoke] not ready after start()\n";
        return 2;
    }

    elle::ConversationContext convo;
    convo.speakerRelationship = "intimate";

    auto outcome = host.analyzeText("I'm fine.", convo, "user_demo");
    if (!outcome.success) {
        std::cerr << "[host_smoke] analyzeText failed: " << outcome.error << "\n";
        return 3;
    }
    std::cout << "[host_smoke] analyzeText OK"
              << "  units=" << outcome.result.units.size()
              << "  trust=" << outcome.result.speakerTrust
              << "  confidence=" << outcome.result.overallConfidence
              << "\n";

    if (!host.injectHormonalState({{2, 0.6}, {5, 0.4}})) {
        std::cerr << "[host_smoke] injectHormonalState failed\n";
        return 4;
    }
    std::cout << "[host_smoke] injectHormonalState OK\n";

    if (!host.recordTrust("user_demo", elle::prob::TrustSignal::CONFIRMED_ACCURATE, 1.0)) {
        std::cerr << "[host_smoke] recordTrust failed\n";
        return 5;
    }
    std::cout << "[host_smoke] recordTrust OK\n";

    if (!host.feedback(0, 1, true, 0.9, "user_demo")) {
        std::cerr << "[host_smoke] feedback failed\n";
        return 6;
    }
    std::cout << "[host_smoke] feedback OK\n";

    auto w = host.queryWeights();
    std::cout << "[host_smoke] weights contextFrame=" << w.contextFrameMatch
              << " emotionalAlignment=" << w.emotionalAlignment << "\n";

    if (!host.reload()) {
        std::cerr << "[host_smoke] reload failed\n";
        return 7;
    }
    std::cout << "[host_smoke] reload OK (ready=" << host.ready() << ")\n";

    auto outcome2 = host.analyzeText("Are you okay?", convo, "user_demo");
    if (!outcome2.success) {
        std::cerr << "[host_smoke] analyzeText (post-reload) failed: "
                  << outcome2.error << "\n";
        return 8;
    }
    std::cout << "[host_smoke] post-reload analyzeText OK"
              << "  trust=" << outcome2.result.speakerTrust << "\n";

    host.stop();
    std::cout << "[host_smoke] PASS\n";
    return 0;
}
