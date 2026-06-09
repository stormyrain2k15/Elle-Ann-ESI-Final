#include <doctest/doctest.h>
#include "../core/SlotSpecParser.h"

using elle::composer::core::ParseSlotSpecs;

TEST_CASE("ParseSlotSpecs returns empty vector for templates without slots") {
    auto slots = ParseSlotSpecs("Hello there, I missed you.");
    CHECK(slots.empty());
}

TEST_CASE("ParseSlotSpecs extracts name and posTag from a simple slot") {
    auto slots = ParseSlotSpecs("[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].");
    REQUIRE(slots.size() == 3);
    CHECK(slots[0].name == "SUBJ");
    CHECK(slots[0].posTag == "PRON");
    CHECK(slots[0].form == "-");
    CHECK(slots[0].optional == false);
    CHECK(slots[1].name == "VERB");
    CHECK(slots[1].posTag == "PRED");
    CHECK(slots[1].form == "1sg_present");
    CHECK(slots[2].name == "OBJ");
    CHECK(slots[2].posTag == "NOUNPHRASE");
}

TEST_CASE("ParseSlotSpecs marks optional slots with trailing ?") {
    auto slots = ParseSlotSpecs("I think [INTENSIFIER:ADJ]? this matters.");
    REQUIRE(slots.size() == 1);
    CHECK(slots[0].name == "INTENSIFIER");
    CHECK(slots[0].optional == true);
}

TEST_CASE("ParseSlotSpecs ignores malformed slot syntax") {
    auto slots = ParseSlotSpecs("[lowercase:PRON] [SUBJ:lowercase] [SUBJ-:PRON]");
    CHECK(slots.empty());
}

TEST_CASE("ParseSlotSpecs is deterministic across repeated calls") {
    const std::string tmpl =
        "[SUBJ:PRON] [VERB:PRED:1sg_present] [MOD:ADV]? [OBJ:NOUNPHRASE:plural].";
    auto a = ParseSlotSpecs(tmpl);
    auto b = ParseSlotSpecs(tmpl);
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].name == b[i].name);
        CHECK(a[i].posTag == b[i].posTag);
        CHECK(a[i].form == b[i].form);
        CHECK(a[i].optional == b[i].optional);
    }
}

TEST_CASE("ParseSlotSpecs preserves left-to-right slot order") {
    auto slots = ParseSlotSpecs(
        "[A:PRON] middle text [B:NOUN] more text [C:ADJ]");
    REQUIRE(slots.size() == 3);
    CHECK(slots[0].name == "A");
    CHECK(slots[1].name == "B");
    CHECK(slots[2].name == "C");
}
