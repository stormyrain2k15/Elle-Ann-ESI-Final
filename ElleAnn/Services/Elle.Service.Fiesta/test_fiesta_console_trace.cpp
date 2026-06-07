#include "FiestaProtoBase.h"
#include "FiestaProtoTable.h"
#include "FiestaConsoleTrace.h"

#include <cstdio>
#include <thread>
#include <vector>

int main() {
    std::printf("──── Phase 6 console-trace smoke test ────\n");

    std::printf("Step 1: trace DISABLED — should produce ZERO lines below this.\n");
    Fiesta::Trace::SetEnabled(false);
    Fiesta::Trace::OnRx(0x0306, {0x12, 0x34});
    Fiesta::Trace::OnTx(0x0306, {0xAA});
    Fiesta::Trace::OnStateChange("A", "B");
    Fiesta::Trace::OnChat("EllaAnn", "test", "chat");
    std::printf("…done.\n\n");

    std::printf("Step 2: trace ENABLED — should produce 5 coloured lines below.\n");
    Fiesta::Trace::SetEnabled(true);
    Fiesta::Trace::Banner("smoke test");
    std::vector<uint8_t> seed = {0x1F, 0x03};
    Fiesta::Trace::OnRx(0x0207 , seed);
    std::vector<uint8_t> login(272, 0);
    Fiesta::Trace::OnTx(0x0306 , login);
    Fiesta::Trace::OnStateChange("CONNECTING", "LOGIN_AWAIT_LOGIN_ACK");
    Fiesta::Trace::OnChat("Crystal", "hi", "chat");
    Fiesta::Trace::OnMove(0x46C4, 5515, 7466, 5572, 7500, 0x32);
    Fiesta::Trace::OnError("disconnected: socket reset by peer");

    std::printf("\nStep 3: 4 threads each emit 5 traces — lines must not interleave.\n");
    std::vector<std::thread> th;
    for (int t = 0; t < 4; ++t) {
        th.emplace_back([t]() {
            for (int i = 0; i < 5; ++i) {
                Fiesta::Trace::OnRx(0x0420 + t,
                                    std::vector<uint8_t>(static_cast<std::size_t>(i + 1) * 4, 0));
            }
        });
    }
    for (auto& x : th) x.join();

    std::printf("\n──── DONE ────\n");
    return 0;
}
