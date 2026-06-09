#include <doctest/doctest.h>
#include "../core/SlotSpecParser.h"
#include <string>
#include <vector>

using elle::composer::core::ParseSlotSpecs;
using elle::composer::core::ScoreFrameByRecency;

namespace {

struct FakeFrame {
    int64_t     frameId;
    std::string kind;
    std::string act;
    std::string templ;
    float       weight;
    int64_t     lastUsedMs;
};

struct ProbResult {
    std::string topAct;
    float       confidence;
    float       emotionValence;
};

struct PickedFrame {
    const FakeFrame* frame = nullptr;
    float            score = -1.0f;
};

PickedFrame PickFrame(const std::vector<FakeFrame>& library,
                     const std::string& kind,
                     const std::string& act,
                     int64_t nowMs) {
    PickedFrame best;
    for (const auto& f : library) {
        if (f.kind != kind) continue;
        if (f.act != act && f.act != "*") continue;
        float s = ScoreFrameByRecency(f.weight, f.lastUsedMs, nowMs);
        if (s > best.score) {
            best.score = s;
            best.frame = &f;
        }
    }
    return best;
}

}

TEST_CASE("Chain: probability act + frame library + slot parse all wire deterministically") {
    std::vector<FakeFrame> library = {
        { 1, "statement",    "ASSERT",   "[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].", 1.0f, 0 },
        { 2, "statement",    "ASSERT",   "[SUBJ:PRON] honestly [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].", 0.9f, 0 },
        { 3, "question",     "ASK",      "[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE]?", 1.0f, 0 },
        { 4, "comfort",      "REASSURE", "[INTENSIFIER:ADV]? [SUBJ:PRON] [VERB:PRED:1sg_present] you.", 0.95f, 0 },
    };

    ProbResult prob;
    prob.topAct          = "ASSERT";
    prob.confidence      = 0.82f;
    prob.emotionValence  = 0.1f;

    int64_t nowMs = 1'700'000'000'000LL;

    auto pick = PickFrame(library, "statement", prob.topAct, nowMs);
    REQUIRE(pick.frame != nullptr);
    CHECK(pick.frame->frameId == 1);

    auto slots = ParseSlotSpecs(pick.frame->templ);
    REQUIRE(slots.size() == 3);
    CHECK(slots[0].name == "SUBJ");
    CHECK(slots[1].name == "VERB");
    CHECK(slots[1].form == "1sg_present");
    CHECK(slots[2].name == "OBJ");
}

TEST_CASE("Chain: recently-used frame is downranked vs equally-weighted fresh frame") {
    int64_t nowMs = 1'700'000'000'000LL;
    std::vector<FakeFrame> library = {
        { 1, "statement", "ASSERT", "[SUBJ:PRON] [VERB:PRED:1sg_present].", 1.0f, nowMs - 1000 },
        { 2, "statement", "ASSERT", "[SUBJ:PRON] truly [VERB:PRED:1sg_present].", 1.0f, 0 },
    };

    auto pick = PickFrame(library, "statement", "ASSERT", nowMs);
    REQUIRE(pick.frame != nullptr);
    CHECK(pick.frame->frameId == 2);
}

TEST_CASE("Chain: question act routes to question-kind frame, not statement") {
    int64_t nowMs = 1'700'000'000'000LL;
    std::vector<FakeFrame> library = {
        { 1, "statement", "ASSERT", "[SUBJ:PRON] [VERB:PRED:1sg_present].", 1.0f, 0 },
        { 2, "question",  "ASK",    "[SUBJ:PRON] [VERB:PRED:1sg_present]?", 1.0f, 0 },
    };
    auto pick = PickFrame(library, "question", "ASK", nowMs);
    REQUIRE(pick.frame != nullptr);
    CHECK(pick.frame->frameId == 2);

    auto slots = ParseSlotSpecs(pick.frame->templ);
    REQUIRE(slots.size() == 2);
    CHECK(slots[0].posTag == "PRON");
    CHECK(slots[1].form == "1sg_present");
}

TEST_CASE("Chain: wildcard-act frame matches any act for its kind") {
    int64_t nowMs = 1'700'000'000'000LL;
    std::vector<FakeFrame> library = {
        { 9, "fallback", "*", "[SUBJ:PRON] [VERB:PRED:1sg_present].", 0.4f, 0 },
    };
    auto pick = PickFrame(library, "fallback", "ANYTHING_ELSE", nowMs);
    REQUIRE(pick.frame != nullptr);
    CHECK(pick.frame->frameId == 9);
}

TEST_CASE("Chain: no matching frame yields null pick") {
    int64_t nowMs = 1'700'000'000'000LL;
    std::vector<FakeFrame> library = {
        { 1, "statement", "ASSERT", "[SUBJ:PRON].", 1.0f, 0 },
    };
    auto pick = PickFrame(library, "question", "ASK", nowMs);
    CHECK(pick.frame == nullptr);
}
