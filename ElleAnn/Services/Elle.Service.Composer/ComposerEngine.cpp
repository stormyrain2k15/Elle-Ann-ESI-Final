#include "ComposerEngine.h"
#include "../_Shared/ElleLogger.h"
#include <algorithm>
#include <cctype>
#include <numeric>
#include <sstream>
#include <regex>

ComposerEngine::ComposerEngine()
    : m_planner(m_inflection) {}

bool ComposerEngine::LoadFrames() {
    return m_frames.Load();
}

bool ComposerEngine::LoadInflections() {
    return m_inflection.Load();
}

// ---------------------------------------------------------------------------
// Step 1 — Select a PragmaticAct
// ---------------------------------------------------------------------------
std::string ComposerEngine::SelectAct(const json& probResult,
                                       const json& conscience,
                                       const json& emotion) const
{
    // Start from the Probability Engine's likelyAct.
    std::string act = "ASSERT";
    if (probResult.is_object() && probResult.contains("likelyAct")) {
        act = probResult.value("likelyAct", std::string("ASSERT"));
    }

    // Map MindManager verdict to an act modifier.
    act = ApplyConscienceModifier(act, conscience);

    // High-arousal + positive emotion biases toward warmth acts.
    if (emotion.is_object()) {
        float arousal = emotion.value("arousal", 0.0f);
        float valence = emotion.value("valence", 0.0f);
        if (arousal > 0.6f && valence > 0.4f && act == "ASSERT") {
            act = "ACK_AND_PROBE";
        }
    }

    return act;
}

std::string ComposerEngine::ApplyConscienceModifier(const std::string& act,
                                                      const json& conscience)
{
    if (!conscience.is_object()) return act;
    std::string verdict = conscience.value("verdict", std::string("PROCEED"));

    if (verdict == "SOFTEN") {
        if (act == "ASSERT")    return "COMFORT";
        if (act == "CHALLENGE") return "QUESTION";
        if (act == "DENY")      return "RECONSIDER_ACT";
    }
    if (verdict == "REFUSE") {
        return "DENY";
    }
    if (verdict == "ASK_FIRST") {
        return "QUESTION";
    }
    if (verdict == "RECONSIDER") {
        return "ACK_AND_PROBE";
    }
    if (verdict == "OWN_IT") {
        return "APOLOGIZE";
    }
    return act;
}

// ---------------------------------------------------------------------------
// Step 2 — Frame selection
// ---------------------------------------------------------------------------
const ComposerFrame* ComposerEngine::SelectFrame(const std::string& kind,
                                                  const std::string& act) const
{
    const ComposerFrame* f = m_frames.Pick(kind, act);
    if (!f) {
        // Fallback: try ASSERT for the same kind.
        f = m_frames.Pick(kind, "ASSERT");
    }
    if (!f) {
        // Final fallback: any frame for this kind.
        f = m_frames.Pick(kind, "*");
    }
    return f;
}

// ---------------------------------------------------------------------------
// Step 3 — Slot filling
// ---------------------------------------------------------------------------
std::vector<ResolvedSlot> ComposerEngine::FillSlots(
    const ComposerFrame&            frame,
    const json&                     userMeaning,
    const json&                     emotion,
    const json&                     identityThreads,
    const std::vector<std::string>& memoryCtx,
    const json&                     probResult) const
{
    auto slots   = SlotPlanner::ParseSlots(frame.templateStr);
    auto weights = ExtractWeights(probResult);
    return m_planner.Plan(slots, userMeaning, emotion, identityThreads,
                          memoryCtx, weights);
}

// ---------------------------------------------------------------------------
// Step 4+5 — Surface stitching
// ---------------------------------------------------------------------------
std::string ComposerEngine::Stitch(const ComposerFrame&             frame,
                                    const std::vector<ResolvedSlot>& slots,
                                    const std::string&               act) const
{
    std::string result = frame.templateStr;

    // Replace each [NAME:POS:form] token with its inflected surface.
    for (const auto& rs : slots) {
        if (rs.surface.empty()) {
            // Optional slot: remove the token entirely (including trailing space).
            std::regex optRe("\\[" + rs.name + ":[A-Z_]+(?::[A-Za-z0-9_]+)?\\]\\?\\s*");
            result = std::regex_replace(result, optRe, "");
        } else {
            std::regex tokenRe("\\[" + rs.name + ":[A-Z_]+(?::[A-Za-z0-9_]+)?\\]\\??");
            result = std::regex_replace(result, tokenRe, rs.surface);
        }
    }

    // Clean up any remaining unresolved optional tokens.
    result = std::regex_replace(result,
        std::regex(R"(\[[A-Z_]+:[A-Z_]+(?::[A-Za-z0-9_]+)?\]\?\s*)"), "");

    // Collapse multiple spaces.
    result = std::regex_replace(result, std::regex(R"(\s{2,})"), " ");
    while (!result.empty() && result.back() == ' ') result.pop_back();

    result = Capitalise(result);
    result = ApplyPunctuation(result, act);
    return result;
}

std::string ComposerEngine::Capitalise(const std::string& s) {
    if (s.empty()) return s;
    std::string out = s;
    out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
    return out;
}

std::string ComposerEngine::ApplyPunctuation(const std::string& text,
                                              const std::string& act)
{
    if (text.empty()) return text;
    const char last = text.back();
    if (last == '.' || last == '?' || last == '!') return text;

    if (act == "QUESTION")     return text + "?";
    if (act == "WARN")         return text + "!";
    if (act == "GREET")        return text + ".";
    if (act == "APOLOGIZE")    return text + ".";
    if (act == "COMFORT")      return text + ".";
    if (act == "ACK_AND_PROBE")return text + "?";
    return text + ".";
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
SlotWeights ComposerEngine::ExtractWeights(const json& probResult) {
    SlotWeights w;
    if (!probResult.is_object() || !probResult.contains("weights")) return w;
    const auto& wj = probResult["weights"];
    if (!wj.is_object()) return w;
    w.contextFrameMatch   = wj.value("contextFrameMatch",   w.contextFrameMatch);
    w.nearbyWordCooccur   = wj.value("nearbyWordCooccur",   w.nearbyWordCooccur);
    w.senseExampleOverlap = wj.value("senseExampleOverlap", w.senseExampleOverlap);
    w.emotionalAlignment  = wj.value("emotionalAlignment",  w.emotionalAlignment);
    w.frequency           = wj.value("frequency",           w.frequency);
    w.posCompatibility    = wj.value("posCompatibility",    w.posCompatibility);
    w.posNegDrawAlignment = wj.value("posNegDrawAlignment", w.posNegDrawAlignment);
    w.conversationHint    = wj.value("conversationHint",    w.conversationHint);
    return w;
}

std::vector<std::string> ComposerEngine::ExtractMemoryCtx(const json& envelope) {
    std::vector<std::string> ctx;
    if (!envelope.contains("memory_ctx")) return ctx;
    for (auto& m : envelope["memory_ctx"]) {
        if (m.is_string()) ctx.push_back(m.get<std::string>());
    }
    return ctx;
}

float ComposerEngine::MeanScore(const std::vector<ResolvedSlot>& slots) {
    if (slots.empty()) return 0.0f;
    float sum = 0.0f;
    int   cnt = 0;
    for (auto& s : slots) {
        if (!s.surface.empty()) { sum += s.score; ++cnt; }
    }
    return cnt > 0 ? sum / static_cast<float>(cnt) : 0.0f;
}

// ---------------------------------------------------------------------------
// Compose — full 5-step pipeline
// ---------------------------------------------------------------------------
ComposeResult ComposerEngine::Compose(const json& envelope) {
    ComposeResult result;

    const std::string kind      = envelope.value("kind",    std::string("CONVERSE"));
    const bool        streaming = envelope.value("stream",  false);

    const json& userMeaning     = envelope.contains("user_meaning")    ? envelope["user_meaning"]    : json{};
    const json& probResult      = envelope.contains("prob_result")     ? envelope["prob_result"]     : json{};
    const json& conscience      = envelope.contains("conscience")      ? envelope["conscience"]      : json{};
    const json& emotion         = envelope.contains("emotion")         ? envelope["emotion"]         : json{};
    const json& identityThreads = envelope.contains("identity_threads")? envelope["identity_threads"]: json::array();

    auto memoryCtx = ExtractMemoryCtx(envelope);

    // --- Step 1: Act selection ---
    std::string act = SelectAct(probResult, conscience, emotion);

    // --- Step 2: Frame selection ---
    const ComposerFrame* frame = SelectFrame(kind, act);
    if (!frame) {
        result.success = false;
        result.error   = "No frame found for kind=" + kind + " act=" + act;
        ELLE_WARN("Composer: %s", result.error.c_str());
        // Return a minimal safe fallback.
        result.text = "I hear you.";
        result.act  = act;
        return result;
    }

    // --- Step 3: Slot filling ---
    auto slots = FillSlots(*frame, userMeaning, emotion,
                           identityThreads, memoryCtx, probResult);

    // --- Step 4+5: Stitch ---
    std::string text = Stitch(*frame, slots, act);

    // Mark frame used (recency penalty).
    m_frames.MarkUsed(frame->frameId);

    // Populate result.
    result.success    = true;
    result.text       = text;
    result.act        = act;
    result.frameId    = frame->frameId;
    result.slots      = slots;
    result.confidence = MeanScore(slots);

    // Streaming: split on sentence boundaries.
    if (streaming) {
        // Split on ". " or "? " or "! ".
        std::regex sentRe(R"((?<=[.?!])\s+)");
        std::sregex_token_iterator it(text.begin(), text.end(), sentRe, -1);
        std::sregex_token_iterator end;
        int idx = 0;
        std::vector<std::string> sentences(it, end);
        for (size_t i = 0; i < sentences.size(); ++i) {
            ComposerClause c;
            c.index = idx++;
            c.text  = sentences[i];
            c.final = (i == sentences.size() - 1);
            result.clauses.push_back(std::move(c));
        }
        if (result.clauses.empty()) {
            ComposerClause c;
            c.index = 0;
            c.text  = text;
            c.final = true;
            result.clauses.push_back(c);
        }
    }

    return result;
}
