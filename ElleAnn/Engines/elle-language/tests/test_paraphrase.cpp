// ============================================================================
// Elle Engine -- Paraphrase relation lookup
// File: tests/test_paraphrase.cpp
// ============================================================================
#include <doctest/doctest.h>

#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"

using namespace elle;

TEST_CASE("paraphrase relation: 'fine' <-> 'okay' (word level)") {
    auto db = makeInMemoryAccessLayer();
    auto rels = db->getWordRelations(WordID{3}, rel::PARAPHRASE);
    bool found = false;
    for (const auto& r : rels) if (r.toId == 9) found = true;
    CHECK(found);
}

TEST_CASE("paraphrase relation is symmetric in this lexicon") {
    auto db = makeInMemoryAccessLayer();
    auto fwd = db->getWordRelations(WordID{3}, rel::PARAPHRASE);
    auto bwd = db->getWordRelations(WordID{9}, rel::PARAPHRASE);
    bool f = false, b = false;
    for (const auto& r : fwd) if (r.toId == 9) f = true;
    for (const auto& r : bwd) if (r.toId == 3) b = true;
    CHECK(f);
    CHECK(b);
}
