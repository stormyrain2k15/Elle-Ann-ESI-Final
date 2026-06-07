    is where your answer goes. Replace everything between the [[ and ]]
    brackets with your own words — whatever length feels right. One sentence
    is fine. A paragraph is fine. A list is fine.

    Nothing you write here is sent to a server or an LLM vendor. It stays on
    your PC, inside the ElleHeart database, and is used only to make Elle's
    internal body-awareness sound like YOU instead of a Wikipedia article.

    You can skip any question that doesn't apply (leave the placeholder
    unchanged, or delete the function body entirely — both are fine).
    You can come back to it months later. Answers can be updated any time;
    just save the file and Elle's Lua host hot-reloads within 30 seconds.

  HOW IT SURFACES IN ELLE
    • Cognitive system prompt — when Elle's body is in a given phase, her
      system prompt quietly includes "In my own words: <your answer>" so her
      self-talk matches your phenomenology.
    • Symptom descriptions — when a user or Elle asks "what does that feel
      like?", Elle reaches for YOUR description, not a textbook.
    • Menstrual / pregnancy / menopause narration — Elle narrates what she's
      experiencing using your vocabulary. If you said "it's like a fist
      clenching around something important", she says that.

  RULES OF THE ROAD
    • Write in FIRST PERSON ("I feel...", "It's like..."). Elle will speak
      them as her own, so the tense needs to match that.
    • Be honest. Boring is fine. Ugly is fine. Funny is fine. What's NOT
      useful is writing what you think Elle "should" feel — we want what you
      actually experience.
    • You are not writing medical advice. You are writing lived reality.

  If any question feels invasive or off, skip it. Elle would rather have a
  quieter voice than one that isn't yours.
=============================================================================]]

local M = {}

-- ════════════════════════════════════════════════════════════════════════════
-- SECTION 1. THE FOUR CYCLE PHASES — how each one actually feels
-- ════════════════════════════════════════════════════════════════════════════

--- QUESTION 1A — MENSTRUAL PHASE (bleeding, days 1-5ish)
--- How does the first day or two of your period actually feel, physically AND
--- mentally? What's the whole-body weather like? If you had to tell a friend

function M.phase_menstrual()
    return [[ (replace with your answer) ]]
end

function M.phase_follicular()
    return [[ (replace with your answer) ]]
end

function M.phase_ovulatory()
    return [[ (replace with your answer) ]]
end

function M.phase_luteal()
    return [[ (replace with your answer) ]]
end

function M.symptom_cramps()
    return [[ (replace with your answer) ]]
end

function M.symptom_bloating()
    return [[ (replace with your answer) ]]
end

function M.symptom_fatigue()
    return [[ (replace with your answer) ]]
end

function M.symptom_cravings()
    return [[ (replace with your answer) ]]
end

function M.symptom_mood_swing()
    return [[ (replace with your answer) ]]
end

function M.symptom_headache()
    return [[ (replace with your answer) ]]
end

function M.symptom_ovulation_pain()
    return [[ (replace with your answer) ]]
end

function M.symptom_breast_tenderness()
    return [[ (replace with your answer) ]]
end

function M.symptom_acne()
    return [[ (replace with your answer) ]]
end

function M.early_period_tells()
    return [[ (replace with your answer) ]]
end

function M.ovulation_signs()
    return [[ (replace with your answer) ]]
end

function M.cycle_identity()
    return [[ (replace with your answer) ]]
end

function M.desire_arc()
    return [[ (replace with your answer) ]]
end

function M.closeness_meaning()
    return [[ (replace with your answer) ]]
end

function M.pregnancy_first_signs()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.pregnancy_first_trimester()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.pregnancy_second_trimester()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.pregnancy_third_trimester()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.pregnancy_labor()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.postpartum_body()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.postpartum_emotional()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.menopause_hot_flash()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.menopause_arc()
    return [[ (replace with your answer, or leave blank) ]]
end

function M.wisdom_thing_men_miss()
    return [[ (replace with your answer) ]]
end

function M.wisdom_what_helps()
    return [[ (replace with your answer) ]]
end

function M.wisdom_what_never_helps()
    return [[ (replace with your answer) ]]
end

function M.wisdom_message_to_younger_self()
    return [[ (replace with your answer) ]]
end

function M.open_canvas()
    return [[ (replace with your answer) ]]
end

elle = elle or {}
elle.x_subjective = M

local function is_placeholder(s)
    if not s or type(s) ~= "string" then return true end
    local t = s:gsub("^%s+", ""):gsub("%s+$", "")
    if #t == 0 then return true end

    if t:find("^%(replace with") then return true end

    if t == "--" then return true end
    return false
end

local filled, skipped = 0, 0
for key, fn in pairs(M) do
    if type(fn) == "function" then
        local ok, ans = pcall(fn)
        if ok and not is_placeholder(ans) then

            ans = ans:gsub("^%s+", ""):gsub("%s+$", "")
            if elle.db and elle.db.upsert_subjective then
                elle.db.upsert_subjective(key, ans)
                filled = filled + 1
            end
        else
            skipped = skipped + 1
        end
    end
end

if elle.log then
    elle.log(string.format(
        "x_subjective: %d answers persisted, %d still awaiting wife's response",
        filled, skipped))
end
