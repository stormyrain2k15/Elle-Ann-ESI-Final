#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include <vector>

enum Event { FIRST_CONTACT, LOST };

struct ReconnectorMock {
    std::set<int> everConnectedTo;
    std::vector<std::pair<Event, int>> log;

    void Tick(const std::vector<std::pair<int, bool>>& peerStates) {
        for (auto& [peer, alive] : peerStates) {
            const bool wasUp = everConnectedTo.count(peer) > 0;
            if (wasUp && !alive) {
                log.push_back({LOST, peer});
                everConnectedTo.erase(peer);
            }
            if (alive && !wasUp) {
                log.push_back({FIRST_CONTACT, peer});
                everConnectedTo.insert(peer);
            }
        }
    }
};

#define EXPECT(cond) do { if (!(cond)) { std::cerr << "FAIL line " << __LINE__ \
    << ": " #cond "\n"; std::exit(1); } } while (0)

int main() {

    {
        ReconnectorMock r;
        r.Tick({{1, false}, {2, false}, {3, false}});
        EXPECT(r.log.empty());
        EXPECT(r.everConnectedTo.empty());
        std::cout << "1. cold-boot silence: OK\n";
    }

    {
        ReconnectorMock r;

        r.Tick({{1, false}, {2, true}, {3, false}});
        EXPECT(r.log.size() == 1);
        EXPECT(r.log[0] == std::make_pair(FIRST_CONTACT, 2));
        r.Tick({{1, true}, {2, true}, {3, false}});
        EXPECT(r.log.size() == 2);
        EXPECT(r.log[1] == std::make_pair(FIRST_CONTACT, 1));
        r.Tick({{1, true}, {2, true}, {3, true}});
        EXPECT(r.log.size() == 3);
        EXPECT(r.log[2] == std::make_pair(FIRST_CONTACT, 3));

        for (int i = 0; i < 5; i++) r.Tick({{1, true}, {2, true}, {3, true}});
        EXPECT(r.log.size() == 3);
        std::cout << "2. arbitrary-order convergence: OK\n";
    }

    {
        ReconnectorMock r;
        r.Tick({{1, true}, {2, true}});
        EXPECT(r.log.size() == 2);
        r.Tick({{1, true}, {2, false}});
        EXPECT(r.log.size() == 3);
        EXPECT(r.log.back() == std::make_pair(LOST, 2));
        EXPECT(r.everConnectedTo.count(2) == 0);
        r.Tick({{1, true}, {2, false}});
        EXPECT(r.log.size() == 3);
        r.Tick({{1, true}, {2, true}});
        EXPECT(r.log.size() == 4);
        EXPECT(r.log.back() == std::make_pair(FIRST_CONTACT, 2));
        EXPECT(r.everConnectedTo.count(2) == 1);
        std::cout << "3. crash-and-recover: OK\n";
    }

    {
        ReconnectorMock r;
        for (int i = 0; i < 100; i++) r.Tick({{1, true}});
        EXPECT(r.log.size() == 1);
        std::cout << "4. idempotent steady state: OK\n";
    }

    std::cout << "\nALL PASSIVE-MESH TESTS PASSED\n";
    return 0;
}
