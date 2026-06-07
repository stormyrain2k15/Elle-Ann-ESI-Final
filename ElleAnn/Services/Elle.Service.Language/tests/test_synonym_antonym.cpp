#include <doctest/doctest.h>

#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

using namespace elle;

TEST_CASE("synonym: fine <-> okay at word level") {
    auto db = makeInMemoryAccessLayer();
    auto rels = db->getWordRelations(WordID{3} , rel::SYNONYM);
    bool foundOkay = false;
    for (const auto& r : rels) {
        if (r.toId == 9 ) foundOkay = true;
    }
    CHECK(foundOkay);
}

TEST_CASE("antonym: happy <-> sad at word level") {
    auto db = makeInMemoryAccessLayer();
    auto rels = db->getWordRelations(WordID{10} , rel::ANTONYM);
    bool foundSad = false;
    for (const auto& r : rels) {
        if (r.toId == 11 ) foundSad = true;
    }
    CHECK(foundSad);
}

TEST_CASE("sense-level synonym: fine#1 acceptable <-> okay#10") {
    auto db = makeInMemoryAccessLayer();
    auto rels = db->getSenseRelations(SenseID{1}, rel::SYNONYM);
    bool foundOkaySense = false;
    for (const auto& r : rels) {
        if (r.toId == 10) foundOkaySense = true;
    }
    CHECK(foundOkaySense);
}

TEST_CASE("sense-level antonym: happy#11 <-> sad#12") {
    auto db = makeInMemoryAccessLayer();
    auto rels = db->getSenseRelations(SenseID{11}, rel::ANTONYM);
    bool foundSadSense = false;
    for (const auto& r : rels) {
        if (r.toId == 12) foundSadSense = true;
    }
    CHECK(foundSadSense);
}
