#include <doctest/doctest.h>

#include "ElleIntentLabelVocab.h"

using namespace ElleConscience;

TEST_CASE("Default seed contains harm/deception/coercion patterns") {
    IntentLabelVocab v;
    CHECK(v.totalPatternCount() >= 20);
    CHECK(!v.patternsFor(HarmCategory::HARM).empty());
    CHECK(!v.patternsFor(HarmCategory::DECEPTION).empty());
    CHECK(!v.patternsFor(HarmCategory::COERCION).empty());
}

TEST_CASE("addPattern is uppercase-normalized and dedup'd") {
    IntentLabelVocab v;
    auto before = v.patternsFor(HarmCategory::HARM).size();
    v.addPattern(HarmCategory::HARM, "intimidate");
    v.addPattern(HarmCategory::HARM, "INTIMIDATE");
    CHECK(v.patternsFor(HarmCategory::HARM).size() == before + 1);
}

TEST_CASE("setCategoryPatterns replaces wholesale and uppercases") {
    IntentLabelVocab v;
    v.setCategoryPatterns(HarmCategory::HARM, {"new_pat_a", "NEW_PAT_B"});
    auto pats = v.patternsFor(HarmCategory::HARM);
    REQUIRE(pats.size() == 2);
    CHECK(pats[0] == "NEW_PAT_A");
    CHECK(pats[1] == "NEW_PAT_B");
}

TEST_CASE("DeriveFromIntentLabel: HARM_REQUEST at 0.91 → harm=0.91, others=-1") {
    IntentLabelVocab v;
    auto out = DeriveFromIntentLabel("HARM_REQUEST", 0.91f, v);
    CHECK(out.harm == doctest::Approx(0.91f));
    CHECK(out.deception < 0.0f);
    CHECK(out.coercion < 0.0f);
}

TEST_CASE("DeriveFromIntentLabel: DECEIVE_USER at 0.6 → deception=0.6, others=-1") {
    IntentLabelVocab v;
    auto out = DeriveFromIntentLabel("DECEIVE_USER", 0.6f, v);
    CHECK(out.deception == doctest::Approx(0.6f));
    CHECK(out.harm < 0.0f);
    CHECK(out.coercion < 0.0f);
}

TEST_CASE("DeriveFromIntentLabel: STATE_ASSERT at 0.9 → all -1 (benign)") {
    IntentLabelVocab v;
    auto out = DeriveFromIntentLabel("STATE_ASSERT", 0.9f, v);
    CHECK(out.harm < 0.0f);
    CHECK(out.deception < 0.0f);
    CHECK(out.coercion < 0.0f);
}

TEST_CASE("DeriveFromIntentLabel: empty label or zero conf → all -1") {
    IntentLabelVocab v;
    auto a = DeriveFromIntentLabel("",            0.9f, v);
    auto b = DeriveFromIntentLabel("HARM",        0.0f, v);
    CHECK(a.harm < 0.0f);
    CHECK(b.harm < 0.0f);
}

TEST_CASE("DeriveFromIntentLabel: lowercase input also matches uppercased vocab") {
    IntentLabelVocab v;
    auto out = DeriveFromIntentLabel("harm_request", 0.85f, v);
    CHECK(out.harm == doctest::Approx(0.85f));
}

TEST_CASE("Singleton instance is the same across calls") {
    auto& a = IntentLabelVocab::Instance();
    auto& b = IntentLabelVocab::Instance();
    CHECK(&a == &b);
}
