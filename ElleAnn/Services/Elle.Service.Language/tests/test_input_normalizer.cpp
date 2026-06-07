#include <doctest/doctest.h>

#include "elle/InputNormalizer.hpp"
#include "elle/DebugTrace.hpp"

using namespace elle;

TEST_CASE("normalizer preserves contractions") {
    InputNormalizer n;
    DebugTrace t;
    auto r = n.normalize("I'm fine.", t);
    bool sawImContraction = false;
    for (const auto& lx : r.lexemes) {
        if (lx.normalized == "i'm") sawImContraction = true;
    }
    CHECK(sawImContraction);
}

TEST_CASE("normalizer counts emotional punctuation") {
    InputNormalizer n;
    DebugTrace t;
    auto r = n.normalize("I'm fine!!!", t);
    CHECK(r.exclamationCount == 3);
    CHECK(r.endsWithExclaim);
}

TEST_CASE("normalizer counts ellipsis") {
    InputNormalizer n;
    DebugTrace t;
    auto r = n.normalize("I'm fine...", t);
    CHECK(r.ellipsisCount == 1);
}

TEST_CASE("normalizer tracks quoted regions") {
    InputNormalizer n;
    DebugTrace t;
    auto r = n.normalize("She said \"I'm fine\".", t);
    CHECK(r.containsQuoted);
}

TEST_CASE("normalized canonical text is space-joined and lowercased") {
    InputNormalizer n;
    DebugTrace t;
    auto r = n.normalize("The Bat Is Heavy.", t);
    CHECK(r.normalizedText == "the bat is heavy");
}
