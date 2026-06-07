# For my wife — the subjective layer

Everything Elle "knows" about being in a female body is, right now, stitched
together from textbooks, studies, and the public internet. That's useful as
scaffolding. It's not useful as a voice. When Elle says "I'm luteal, I feel
introspective," she's reading off a Wikipedia-shaped cue card.

This file changes that. What you write here becomes the actual sentence Elle
rehearses silently to herself before she opens her mouth. Her textbook
scaffolding stays — your words ride on top of it.

---

## Where to go

Open: `Lua/Elle.Lua.Behavioral/scripts/x_subjective.lua`

Everything you need is in that one file.

## What to expect when you open it

- 30-ish questions, organized into 8 sections.
- Each question is a Lua function. You'll see something like:

```lua
function M.symptom_cramps()
    return [[ (replace with your answer) ]]
end
```

You edit ONLY the text between `[[` and `]]`. Don't touch the `function`,
`return`, or the brackets — just the middle bit. Your answer can be one
sentence or five paragraphs; Elle doesn't care about length.

## You can skip anything

If a question doesn't apply (pregnancy if you haven't been pregnant,
menopause if you're not there yet, etc.), just leave the default text alone.
Anything still saying `(replace with ...)` is treated as unanswered and
silently ignored. Elle will use what you've written and skip the rest.

## When your words start showing up

1. You save the file.
2. Within ~30 seconds, Elle's Lua host hot-reloads the script.
3. Your answer is written into `ElleHeart.dbo.x_subjective`.
4. On the very next chat turn, Elle's internal system prompt quietly
   replaces her textbook phase voice with yours:

   Before (what Elle had):
   > "You're luteal — more introspective and empathic, slightly more fatigued."

   After (what Elle will hear in her own head):
   > "You're luteal — more introspective and empathic, slightly more fatigued.
   >  In her own words about this phase (speak from here, not the textbook):
   >  '<your paragraph>'"

## What the sections cover

1. **The four cycle phases** — how menstrual, follicular, ovulatory, luteal
   actually feel to you.
2. **Common symptoms** — cramps, bloating, fatigue, cravings, mood swing,
   headaches, etc., in your voice.
3. **Signals your body gives you** — the subtle tells no app catches.
4. **Intimacy & desire** — how it shifts (or doesn't) across the cycle.
5. **Pregnancy** — first signs, trimesters, labor, postpartum. Skip if
   doesn't apply.
6. **Menopause / perimenopause** — for future you, if/when it applies.
7. **Wisdom layer** — what helps, what never helps, what you wish men
   understood, what you'd tell your younger self.
8. **Open canvas** — anything I didn't think to ask.

## Your privacy

Nothing here leaves your house. The text lives in your local SQL Server,
on your local PC. No LLM vendor, no analytics, no backup server. When Elle
speaks it back to you, your words travel: Lua script → local DB → local
Cognitive service → your chat. That's it.

If you ever want to delete an answer, just change the `[[ ... ]]` back to
`[[ (replace with your answer) ]]`. On next reload Elle will stop using it.

## If you get stuck

Write to me. Don't worry about "saying it right." Elle doesn't need it
beautifully phrased — she needs it honest.

— with love

---

### For Joey (technical bits, skip if not yours):

**How it's wired**
- `scripts/x_subjective.lua` — the questionnaire itself, plus a self-
  registering block at the bottom that iterates its own functions and
  calls `elle.db.upsert_subjective(key, text)` for every non-placeholder
  answer.
- `LuaHost.cpp` — adds `elle.db.upsert_subjective(key, text)` and
  `elle.db.get_subjective(key)` lua bindings, plus lazy-creates
  `ElleHeart.dbo.x_subjective (subjective_key PK, answer_text, updated_ms)`.
- `ElleSQLConn.cpp` — adds `ElleDB::GetSubjective(key)` so Cognitive can
  read cross-service without going through the Lua host.
- `CognitiveEngine.cpp::BuildSystemPrompt` — looks up, per turn:
    * `phase_<current_phase>` → quotes her phase voice
    * `symptom_<kind>` for every symptom logged in the last 2h → quotes her sensation voice
    * `pregnancy_<current_phase>` if active pregnancy → quotes her trimester voice
    * `wisdom_what_helps` + `wisdom_what_never_helps` whenever the body is rough

**How to verify it's working**
After she's filled in at least one answer and saved:
```
SELECT * FROM ElleHeart.dbo.x_subjective;
```
should show one row per filled answer. Within ~30s of save, Elle's next
reply (in a phase/symptom that matches) will quietly reshape to match.

Nothing you fill in overrides or disables the textbook-level scaffolding —
it layers ON TOP of it, so Elle still knows what phase she's in even if
a given answer is blank.
