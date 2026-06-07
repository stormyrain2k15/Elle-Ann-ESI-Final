#pragma once
#include "FrameLibrary.h"
#include "InflectionTables.h"
#include "SlotPlanner.h"
#include "../_Shared/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

// One clause emitted during streaming.
struct ComposerClause {
    int         index  = 0;
    std::string text;
    bool        final  = false;
};

// Full result of one Compose call.
struct ComposeResult {
    bool        success    = false;
    std::string error;
    std::string text;
    std::string act;
    int64_t     frameId    = 0;
    std::vector<ResolvedSlot> slots;
    float       confidence = 0.0f;
    std::vector<ComposerClause> clauses;   // populated if stream=true
};

class ComposerEngine {
public:
    ComposerEngine();

    bool LoadFrames();
    bool LoadInflections();

    // Main entry point. json envelope matches the IPC spec.
    ComposeResult Compose(const json& envelope);

private:
    // Step 1: choose a PragmaticAct from prob_result + conscience + emotion.
    std::string SelectAct(const json& probResult,
                          const json& conscience,
                          const json& emotion) const;

    // Step 2: pick a frame from the library.
    const ComposerFrame* SelectFrame(const std::string& kind,
                                     const std::string& act) const;

    // Step 3: fill slots.
    std::vector<ResolvedSlot> FillSlots(
        const ComposerFrame& frame,
        const json&          userMeaning,
        const json&          emotion,
        const json&          identityThreads,
        const std::vector<std::string>& memoryCtx,
        const json&          probResult) const;

    // Step 4+5: stitch into surface form.
    std::string Stitch(const ComposerFrame&          frame,
                       const std::vector<ResolvedSlot>& slots,
                       const std::string&            act) const;

    // Apply punctuation per act.
    static std::string ApplyPunctuation(const std::string& text,
                                        const std::string& act);

    // Capitalise first letter.
    static std::string Capitalise(const std::string& s);

    // Extract SlotWeights from a prob_result JSON blob.
    static SlotWeights ExtractWeights(const json& probResult);

    // Extract memory_ctx array from envelope.
    static std::vector<std::string> ExtractMemoryCtx(const json& envelope);

    // Compute mean slot score.
    static float MeanScore(const std::vector<ResolvedSlot>& slots);

    // Map conscience verdict + emotion to act modifier.
    static std::string ApplyConscienceModifier(const std::string& act,
                                               const json& conscience);

    FrameLibrary      m_frames;
    InflectionTables  m_inflection;
    SlotPlanner       m_planner;
};
