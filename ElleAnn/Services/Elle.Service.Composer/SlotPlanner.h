#pragma once
#include "../_Shared/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

// One slot declared in a frame template.
// e.g. "[VERB:PRED:1sg_present]"
struct SlotSpec {
    std::string name;       // VERB, OBJ, SUBJ, INTENSIFIER, MODIFIER ...
    std::string posTag;     // PRED, PRON, NOUNPHRASE, ADJ ...
    std::string form;       // inflection target, e.g. "1sg_present", "plural", "-"
    bool        optional = false;
};

// One resolved slot ready for stitching.
struct ResolvedSlot {
    std::string name;
    std::string lemma;
    std::string form;
    std::string surface;    // inflected form
    float       score = 0.0f;
};

// Weights fed in from the Probability Engine's current WeightVector.
struct SlotWeights {
    float contextFrameMatch   = 1.0f;
    float nearbyWordCooccur   = 0.6f;
    float senseExampleOverlap = 0.5f;
    float emotionalAlignment  = 0.7f;
    float frequency           = 0.3f;
    float posCompatibility    = 0.4f;
    float posNegDrawAlignment = 0.5f;
    float conversationHint    = 0.8f;
};

class InflectionTables;

class SlotPlanner {
public:
    explicit SlotPlanner(InflectionTables& inflection)
        : m_inflection(inflection) {}

    // Fill all slots for a frame template given the structured request payload.
    // Returns one ResolvedSlot per SlotSpec (optional slots may be empty).
    std::vector<ResolvedSlot> Plan(
        const std::vector<SlotSpec>& slots,
        const json&                  userMeaning,
        const json&                  emotionVec,
        const json&                  identityThreads,
        const std::vector<std::string>& memoryCtx,
        const SlotWeights&           weights) const;

private:
    // Parse "[NAME:POS:form?]" strings out of a frame template.
    static std::vector<SlotSpec> ParseTemplate(const std::string& tmpl);

    // Build candidate lemma list for one slot via semantic-graph hinting.
    std::vector<std::string> BuildCandidates(
        const SlotSpec& slot,
        const json&     userMeaning,
        const std::vector<std::string>& memoryCtx) const;

    // Score one candidate lemma against the slot and context.
    float ScoreCandidate(
        const std::string& lemma,
        const SlotSpec&    slot,
        const json&        emotionVec,
        const json&        userMeaning,
        const SlotWeights& weights) const;

    // POS-compatibility: does this lemma's guessed POS match the slot's posTag?
    float PosCompatScore(const std::string& lemma,
                         const std::string& posTag) const;

    // Emotional alignment: does this word's draw match the current emotion?
    float EmotionalAlignScore(const std::string& lemma,
                               const json&        emotionVec) const;

    // Frequency heuristic: common shorter words score higher.
    float FrequencyScore(const std::string& lemma) const;

    InflectionTables& m_inflection;

    friend class SlotPlannerTest;

public:
    // Expose template parser so ComposerEngine can call it.
    static std::vector<SlotSpec> ParseSlots(const std::string& tmpl) {
        return ParseTemplate(tmpl);
    }
};
