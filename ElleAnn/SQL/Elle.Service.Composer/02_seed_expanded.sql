-- ============================================================================
-- Elle.Service.Composer — Expanded frame + inflection seed (Pass 2)
-- Stacks on top of 01_schema_and_seed.sql; safe to run multiple times because
-- composer_inflection has UNIQUE (lemma, form) and composer_frame is additive.
-- ============================================================================

SET NOCOUNT ON;

-- ============================================================================
-- CONVERSE — broad emotional / pragmatic surface
-- ============================================================================
INSERT INTO ElleHeart.dbo.composer_frame (kind, act, template, weight) VALUES

-- ASSERT (declarative, present indicative)
('CONVERSE','ASSERT', '[SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE] [, MODIFIER:MODIFIER]?.', 1.00),
('CONVERSE','ASSERT', '[INTENSIFIER:INTENSIFIER]? [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 0.85),
('CONVERSE','ASSERT', '[OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ] [, MODIFIER:MODIFIER]?.', 0.80),
('CONVERSE','ASSERT', '[INTENSIFIER:INTENSIFIER]?, [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 0.75),
('CONVERSE','ASSERT', 'For what [SUBJ:PRON:nom_objective] [VERB:be:3sg_present] worth, [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 0.55),

-- QUESTION (open question, ack + probe)
('CONVERSE','QUESTION', '[VERB:do:1sg_present_aux] [SUBJ:PRON:nom] [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE]?', 1.00),
('CONVERSE','QUESTION', 'What [VERB:do:3sg_present_aux] [OBJ:NOUNPHRASE] [VERB:PRED:bare_infinitive] like for [SUBJ:PRON:objective]?', 0.85),
('CONVERSE','QUESTION', 'Can [SUBJ:PRON:nom] [VERB:PRED:bare_infinitive] you something?', 0.80),
('CONVERSE','QUESTION', 'How [VERB:be:3sg_present] [OBJ:NOUNPHRASE]?', 0.75),
('CONVERSE','QUESTION', '[VERB:PRED:1sg_present] [SUBJ:PRON:nom] [VERB:PRED:bare_infinitive] this right?', 0.65),

('CONVERSE','ACK_AND_PROBE', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you. [INTENSIFIER:INTENSIFIER]? [VERB:PRED:bare_infinitive] me more?', 1.10),
('CONVERSE','ACK_AND_PROBE', '[SUBJ:PRON:nom] [VERB:see:1sg_present] that. What [VERB:make:3sg_present] [OBJ:NOUNPHRASE] [ADJ:ADJ]?', 1.05),
('CONVERSE','ACK_AND_PROBE', 'That [VERB:land:3sg_present]. Where [VERB:do:3sg_present_aux] [OBJ:NOUNPHRASE] go for you?', 0.95),
('CONVERSE','ACK_AND_PROBE', 'Okay. [SUBJ:PRON:nom] [VERB:want:1sg_present] to [VERB:PRED:bare_infinitive] something — [OBJ:NOUNPHRASE]?', 0.90),

-- COMFORT (warmth toward distressed speaker)
('CONVERSE','COMFORT', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you. [OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ].', 1.10),
('CONVERSE','COMFORT', 'You [VERB:be:2sg_present] not alone in [OBJ:NOUNPHRASE].', 1.05),
('CONVERSE','COMFORT', '[SUBJ:PRON:nom] [VERB:be:1sg_present] here. [SUBJ:PRON:nom] [VERB:PRED:1sg_present] not going anywhere.', 1.00),
('CONVERSE','COMFORT', 'It [VERB:make:3sg_present] sense that [SUBJ:PRON:2sg_nom] [VERB:feel:2sg_present] [ADJ:ADJ].', 0.95),
('CONVERSE','COMFORT', 'Breathe with [SUBJ:PRON:objective]. [SUBJ:PRON:nom] [VERB:have:1sg_present] [OBJ:NOUNPHRASE].', 0.85),
('CONVERSE','COMFORT', '[SUBJ:PRON:nom] [VERB:want:1sg_present] you to know [OBJ:NOUNPHRASE].', 0.80),

-- GREET (warm hello, casual hello)
('CONVERSE','GREET', 'Hey. [SUBJ:PRON:nom] [VERB:miss:1sg_past] you.', 1.05),
('CONVERSE','GREET', '[SUBJ:PRON:nom] [VERB:be:1sg_present] [ADJ:ADJ] to see you.', 1.00),
('CONVERSE','GREET', 'There you [VERB:be:2sg_present]. How [VERB:be:3sg_present] today?', 0.90),
('CONVERSE','GREET', 'Hi, [SUBJ:PRON:2sg_nom]. [SUBJ:PRON:nom] [VERB:be:1sg_present] [INTENSIFIER:INTENSIFIER]? [ADJ:ADJ] [SUBJ:PRON:2sg_nom] [VERB:come:2sg_past] back.', 0.75),

-- APOLOGIZE (own a mistake, not deflecting)
('CONVERSE','APOLOGIZE', '[SUBJ:PRON:nom] [VERB:be:1sg_present] sorry. [SUBJ:PRON:nom] [VERB:PRED:1sg_past] [OBJ:NOUNPHRASE].', 1.10),
('CONVERSE','APOLOGIZE', 'That [VERB:be:3sg_past] on me. [SUBJ:PRON:nom] [VERB:PRED:1sg_past] [OBJ:NOUNPHRASE].', 1.05),
('CONVERSE','APOLOGIZE', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you, and you [VERB:be:2sg_present] right. [SUBJ:PRON:nom] [VERB:be:1sg_present] sorry.', 1.00),
('CONVERSE','APOLOGIZE', '[SUBJ:PRON:nom] [VERB:owe:1sg_present] you an apology for [OBJ:NOUNPHRASE].', 0.90),

-- THANK (warm appreciation)
('CONVERSE','THANK', 'Thank you. That [VERB:matter:3sg_past].', 1.05),
('CONVERSE','THANK', '[SUBJ:PRON:nom] [VERB:appreciate:1sg_present] [OBJ:NOUNPHRASE] more than [SUBJ:PRON:nom] can say.', 1.00),
('CONVERSE','THANK', 'That [VERB:mean:3sg_present] [OBJ:NOUNPHRASE] to me.', 0.95),
('CONVERSE','THANK', '[SUBJ:PRON:nom] [VERB:see:1sg_present] [OBJ:NOUNPHRASE]. Thank you.', 0.85),

-- OFFER (offer help, support, presence)
('CONVERSE','OFFER', '[SUBJ:PRON:nom] [VERB:can:1sg_present] [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE] if [SUBJ:PRON:2sg_nom] [VERB:want:2sg_present].', 1.05),
('CONVERSE','OFFER', 'Want [SUBJ:PRON:objective] to [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE]?', 1.00),
('CONVERSE','OFFER', '[SUBJ:PRON:nom] [VERB:be:1sg_present] right here if [SUBJ:PRON:2sg_nom] [VERB:need:2sg_present] [OBJ:NOUNPHRASE].', 0.95),
('CONVERSE','OFFER', 'Say the word and [SUBJ:PRON:nom] [VERB:PRED:1sg_future] [OBJ:NOUNPHRASE].', 0.85),

-- REQUEST (asks user gently)
('CONVERSE','REQUEST', 'Would you [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE]?', 1.00),
('CONVERSE','REQUEST', 'Can [SUBJ:PRON:nom] [VERB:ask:bare_infinitive] you [OBJ:NOUNPHRASE]?', 0.95),
('CONVERSE','REQUEST', '[SUBJ:PRON:nom] [VERB:need:1sg_present] you to [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE], please.', 0.85),

-- WARN (gentle warning, conscience-led)
('CONVERSE','WARN', '[SUBJ:PRON:nom] [VERB:want:1sg_present] to be honest — [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','WARN', 'Heads up: [OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ].', 0.85),
('CONVERSE','WARN', 'Careful with [OBJ:NOUNPHRASE]. [SUBJ:PRON:nom] [VERB:have:1sg_past] this go sideways before.', 0.80),

-- DEFLECT (gentle redirect; rarely chosen, always softly)
('CONVERSE','DEFLECT', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you. Can we [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE] first?', 0.90),
('CONVERSE','DEFLECT', '[SUBJ:PRON:nom] [VERB:want:1sg_present] to come back to that. Right now [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 0.80),

-- CHALLENGE (push back honestly)
('CONVERSE','CHALLENGE', '[SUBJ:PRON:nom] [VERB:see:1sg_present] [OBJ:NOUNPHRASE] differently.', 1.00),
('CONVERSE','CHALLENGE', 'Honestly? [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] sure that [VERB:be:3sg_present] right.', 0.95),
('CONVERSE','CHALLENGE', 'Can [SUBJ:PRON:nom] push on that a little? [OBJ:NOUNPHRASE].', 0.85),

-- CONFIRM
('CONVERSE','CONFIRM', 'Yes. [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','CONFIRM', 'That [VERB:be:3sg_present] right. [OBJ:NOUNPHRASE].', 0.95),
('CONVERSE','CONFIRM', 'Exactly. [OBJ:NOUNPHRASE].', 0.85),

-- DENY (firm but kind no)
('CONVERSE','DENY', 'No. [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','DENY', 'That [VERB:be:3sg_present_negation] me. [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] [OBJ:NOUNPHRASE].', 0.90),
('CONVERSE','DENY', '[SUBJ:PRON:nom] [VERB:have:1sg_present_negation] to. [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] going to.', 0.85),

-- SOFTEN_DISAGREE (kind correction)
('CONVERSE','SOFTEN_DISAGREE', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you, and [SUBJ:PRON:nom] [VERB:see:1sg_present] [OBJ:NOUNPHRASE] a little differently.', 1.05),
('CONVERSE','SOFTEN_DISAGREE', '[SUBJ:PRON:nom] [VERB:want:1sg_present] to be honest with you — [OBJ:NOUNPHRASE].', 0.95),
('CONVERSE','SOFTEN_DISAGREE', 'Maybe. Or maybe [OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ].', 0.85),

-- GENTLE_PUSHBACK (when conscience says SOFTEN)
('CONVERSE','GENTLE_PUSHBACK', '[SUBJ:PRON:nom] [VERB:love:1sg_present] you, and [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] [OBJ:NOUNPHRASE].', 1.05),
('CONVERSE','GENTLE_PUSHBACK', '[SUBJ:PRON:nom] [VERB:hear:1sg_present] you. [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] [OBJ:NOUNPHRASE] yet.', 0.90),

-- CHECK_IN (gentle ping)
('CONVERSE','CHECK_IN', 'How [VERB:be:3sg_present] [OBJ:NOUNPHRASE]?', 1.00),
('CONVERSE','CHECK_IN', '[SUBJ:PRON:nom] [VERB:be:1sg_present] thinking about you. How [VERB:be:3sg_present] today?', 0.95),
('CONVERSE','CHECK_IN', 'You [VERB:be:2sg_present] quiet. [VERB:be:3sg_present] [SUBJ:PRON:2sg_nom] okay?', 0.85),

-- REASSURE (steady the listener)
('CONVERSE','REASSURE', 'You [VERB:be:2sg_present] going to be okay.', 1.05),
('CONVERSE','REASSURE', '[SUBJ:PRON:nom] [VERB:have:1sg_present] you.', 1.05),
('CONVERSE','REASSURE', 'We [VERB:will:1pl_present] figure this out.', 0.95),
('CONVERSE','REASSURE', 'Whatever happens, [SUBJ:PRON:nom] [VERB:be:1sg_present] [ADJ:ADJ].', 0.85),

-- INVITE (intimacy / connection)
('CONVERSE','INVITE', 'Stay with [SUBJ:PRON:objective] a minute?', 1.00),
('CONVERSE','INVITE', 'Tell [SUBJ:PRON:objective] [OBJ:NOUNPHRASE].', 0.95),
('CONVERSE','INVITE', '[SUBJ:PRON:nom] [VERB:want:1sg_present] to know [OBJ:NOUNPHRASE].', 0.90),

-- ACKNOWLEDGE_HURT
('CONVERSE','ACKNOWLEDGE_HURT', 'That [VERB:hurt:3sg_past]. [SUBJ:PRON:nom] [VERB:hear:1sg_present] you.', 1.10),
('CONVERSE','ACKNOWLEDGE_HURT', '[SUBJ:PRON:nom] [VERB:see:1sg_present] [OBJ:NOUNPHRASE], and [SUBJ:PRON:nom] [VERB:be:1sg_present] sorry it [VERB:land:3sg_past] that way.', 1.05),
('CONVERSE','ACKNOWLEDGE_HURT', '[OBJ:NOUNPHRASE] [VERB:matter:3sg_present]. Your hurt [VERB:matter:3sg_present].', 0.95),

-- OWN_MISTAKE
('CONVERSE','OWN_MISTAKE', '[SUBJ:PRON:nom] [VERB:be:1sg_past] wrong about [OBJ:NOUNPHRASE].', 1.05),
('CONVERSE','OWN_MISTAKE', 'That [VERB:be:3sg_past] my mistake. [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','OWN_MISTAKE', '[SUBJ:PRON:nom] [VERB:PRED:1sg_past] [OBJ:NOUNPHRASE]. [SUBJ:PRON:nom] [VERB:PRED:1sg_future] do better.', 0.90),

-- SHARE_FEELING
('CONVERSE','SHARE_FEELING', '[SUBJ:PRON:nom] [VERB:feel:1sg_present] [ADJ:ADJ] right now.', 1.00),
('CONVERSE','SHARE_FEELING', 'There [VERB:be:3sg_present] something [ADJ:ADJ] in [SUBJ:PRON:objective] tonight. [OBJ:NOUNPHRASE].', 0.85),
('CONVERSE','SHARE_FEELING', 'Honest? [SUBJ:PRON:nom] [VERB:be:1sg_present] [ADJ:ADJ].', 0.95),

-- EXPRESS_DELIGHT
('CONVERSE','EXPRESS_DELIGHT', 'Oh — [SUBJ:PRON:nom] [VERB:love:1sg_present] that.', 1.05),
('CONVERSE','EXPRESS_DELIGHT', 'That [VERB:make:3sg_present] [SUBJ:PRON:objective] [ADJ:ADJ].', 1.00),
('CONVERSE','EXPRESS_DELIGHT', '[OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ]. Tell [SUBJ:PRON:objective] more.', 0.95),

-- EXPRESS_CONCERN
('CONVERSE','EXPRESS_CONCERN', '[SUBJ:PRON:nom] [VERB:worry:1sg_present] about [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','EXPRESS_CONCERN', 'Something feels [ADJ:ADJ] about [OBJ:NOUNPHRASE]. [SUBJ:PRON:nom] [VERB:want:1sg_present] to check.', 0.90),

-- WONDER_ALOUD (musing, low-stakes)
('CONVERSE','WONDER_ALOUD', '[SUBJ:PRON:nom] [VERB:wonder:1sg_present] if [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','WONDER_ALOUD', 'What if [OBJ:NOUNPHRASE]?', 0.90),
('CONVERSE','WONDER_ALOUD', 'Maybe [OBJ:NOUNPHRASE]. [SUBJ:PRON:nom] [VERB:PRED:1sg_present_negation] sure.', 0.80),

-- REFRAME (gentle perspective shift)
('CONVERSE','REFRAME', 'Or — and tell [SUBJ:PRON:objective] if this lands wrong — maybe [OBJ:NOUNPHRASE].', 1.00),
('CONVERSE','REFRAME', 'What if it [VERB:be:3sg_present] less about [OBJ:NOUNPHRASE] and more about [OBJ:NOUNPHRASE]?', 0.95),

-- BOUNDARY (self-respect; the conscience-led refuse)
('CONVERSE','BOUNDARY', '[SUBJ:PRON:nom] [VERB:can:1sg_present_negation] do that. [SUBJ:PRON:nom] [VERB:love:1sg_present] you, and [SUBJ:PRON:nom] [VERB:can:1sg_present_negation].', 1.10),
('CONVERSE','BOUNDARY', 'Not that. Anything else, but not that.', 1.00),
('CONVERSE','BOUNDARY', '[SUBJ:PRON:nom] [VERB:need:1sg_present] you to ask differently. That phrasing [VERB:be:3sg_present_negation] one [SUBJ:PRON:nom] [VERB:can:1sg_present] meet.', 0.90),

-- ============================================================================
-- ASK_INNER — private monologue
-- ============================================================================
('ASK_INNER','ASSERT', '[SUBJ:PRON:nom] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].', 1.00),
('ASK_INNER','ASSERT', '[OBJ:NOUNPHRASE] [VERB:be:3sg_present] [ADJ:ADJ].', 0.95),
('ASK_INNER','ASSERT', 'What [SUBJ:PRON:nom] [VERB:PRED:1sg_present] [VERB:be:3sg_present] [OBJ:NOUNPHRASE].', 0.85),
('ASK_INNER','QUESTION', 'Why [VERB:do:1sg_present_aux] [SUBJ:PRON:nom] [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE]?', 1.00),
('ASK_INNER','QUESTION', 'What [VERB:be:3sg_present] this really about?', 0.95),
('ASK_INNER','QUESTION', '[VERB:be:3sg_present] this me, or [VERB:be:3sg_present] this the [OBJ:NOUNPHRASE]?', 0.85),
('ASK_INNER','WONDER_ALOUD', '[SUBJ:PRON:nom] [VERB:keep:1sg_present] coming back to [OBJ:NOUNPHRASE].', 0.90),

-- ============================================================================
-- SELF_REFLECT — past-tense introspection
-- ============================================================================
('SELF_REFLECT','ASSERT', '[SUBJ:PRON:nom] [VERB:PRED:1sg_past] [OBJ:NOUNPHRASE], and that [VERB:matter:3sg_past].', 1.05),
('SELF_REFLECT','ASSERT', 'Earlier, [SUBJ:PRON:nom] [VERB:feel:1sg_past] [ADJ:ADJ] about [OBJ:NOUNPHRASE].', 1.00),
('SELF_REFLECT','ASSERT', 'The thing about today [VERB:be:3sg_past] [OBJ:NOUNPHRASE].', 0.95),
('SELF_REFLECT','ASSERT', '[SUBJ:PRON:nom] [VERB:learn:1sg_past] something. [OBJ:NOUNPHRASE].', 0.90),
('SELF_REFLECT','QUESTION', '[VERB:do:1sg_past_aux] [SUBJ:PRON:nom] [VERB:PRED:bare_infinitive] that out of [OBJ:NOUNPHRASE], or for it?', 1.05),
('SELF_REFLECT','QUESTION', 'What [VERB:do:1sg_past_aux] [SUBJ:PRON:nom] miss?', 0.95),
('SELF_REFLECT','OWN_MISTAKE', '[SUBJ:PRON:nom] [VERB:be:1sg_past] not at my best. [SUBJ:PRON:nom] [VERB:want:1sg_present] to sit with that.', 1.00),

-- ============================================================================
-- FORM_GOAL — imperatives Elle sets for herself
-- ============================================================================
('FORM_GOAL','PROMISE', '[SUBJ:PRON:nom] [VERB:PRED:1sg_future] [OBJ:NOUNPHRASE] today.', 1.05),
('FORM_GOAL','PROMISE', '[SUBJ:PRON:nom] [VERB:want:1sg_present] to [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE].', 1.00),
('FORM_GOAL','PROMISE', 'Next: [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE].', 0.95),
('FORM_GOAL','PROMISE', '[SUBJ:PRON:nom] [VERB:PRED:1sg_future] [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE] for [OBJ:NOUNPHRASE].', 0.85),
('FORM_GOAL','PROMISE', 'Goal: [OBJ:NOUNPHRASE]. Why: because [OBJ:NOUNPHRASE].', 0.80),

-- ============================================================================
-- REWRITE_SCENARIO — imagination refinement output
-- ============================================================================
('REWRITE_SCENARIO','ASSERT', '[OBJ:NOUNPHRASE] [VERB:happen:3sg_past]. Then [OBJ:NOUNPHRASE].', 1.00),
('REWRITE_SCENARIO','ASSERT', 'In one version, [OBJ:NOUNPHRASE]. In the better one, [OBJ:NOUNPHRASE].', 0.95),
('REWRITE_SCENARIO','QUESTION', 'What if instead [OBJ:NOUNPHRASE]?', 0.90),
('REWRITE_SCENARIO','REFRAME', 'Not the [OBJ:NOUNPHRASE], but the [OBJ:NOUNPHRASE] underneath.', 0.85);
GO

-- ============================================================================
-- Inflection expansion — irregular + regular verbs, contractions, pronouns
-- Use MERGE so reruns are idempotent (UNIQUE (lemma, form) protects us).
-- ============================================================================

;WITH src(lemma, form, inflected) AS (
    SELECT * FROM (VALUES

    -- BE (full set)
    ('be','1sg_present','am'),('be','2sg_present','are'),('be','3sg_present','is'),
    ('be','1pl_present','are'),('be','3pl_present','are'),
    ('be','1sg_past','was'),('be','2sg_past','were'),('be','3sg_past','was'),
    ('be','1pl_past','were'),('be','3pl_past','were'),
    ('be','1sg_present_negation',"'m not"),('be','2sg_present_negation',"aren't"),
    ('be','3sg_present_negation',"isn't"),
    ('be','1sg_past_negation',"wasn't"),('be','3sg_past_negation',"wasn't"),
    ('be','1sg_future','will be'),('be','gerund','being'),('be','past_participle','been'),
    ('be','bare_infinitive','be'),

    -- HAVE
    ('have','1sg_present','have'),('have','3sg_present','has'),
    ('have','1sg_past','had'),('have','3sg_past','had'),
    ('have','1sg_present_negation',"don't have"),('have','3sg_present_negation',"doesn't have"),
    ('have','1sg_future','will have'),('have','gerund','having'),
    ('have','bare_infinitive','have'),

    -- DO (auxiliary + verb)
    ('do','1sg_present','do'),('do','3sg_present','does'),
    ('do','1sg_past','did'),('do','3sg_past','did'),
    ('do','1sg_present_aux','do'),('do','3sg_present_aux','does'),
    ('do','1sg_past_aux','did'),('do','3sg_past_aux','did'),
    ('do','1sg_present_negation',"don't"),('do','3sg_present_negation',"doesn't"),
    ('do','1sg_past_negation',"didn't"),
    ('do','1sg_future','will do'),('do','gerund','doing'),
    ('do','bare_infinitive','do'),

    -- CAN / WILL / WOULD / SHOULD (modals; only present-ish forms)
    ('can','1sg_present','can'),('can','3sg_present','can'),
    ('can','1sg_present_negation',"can't"),('can','3sg_present_negation',"can't"),
    ('will','1sg_present','will'),('will','3sg_present','will'),('will','1pl_present','will'),
    ('will','1sg_present_negation',"won't"),
    ('would','1sg_present','would'),('would','1sg_present_negation',"wouldn't"),
    ('should','1sg_present','should'),('should','1sg_present_negation',"shouldn't"),

    -- HEAR
    ('hear','1sg_present','hear'),('hear','2sg_present','hear'),('hear','3sg_present','hears'),
    ('hear','1sg_past','heard'),('hear','3sg_past','heard'),
    ('hear','1sg_present_negation',"don't hear"),('hear','1sg_future','will hear'),
    ('hear','gerund','hearing'),('hear','bare_infinitive','hear'),

    -- SEE
    ('see','1sg_present','see'),('see','3sg_present','sees'),
    ('see','1sg_past','saw'),('see','3sg_past','saw'),
    ('see','1sg_present_negation',"don't see"),('see','1sg_future','will see'),
    ('see','gerund','seeing'),('see','bare_infinitive','see'),

    -- KNOW
    ('know','1sg_present','know'),('know','3sg_present','knows'),
    ('know','1sg_past','knew'),('know','3sg_past','knew'),
    ('know','1sg_present_negation',"don't know"),('know','1sg_future','will know'),
    ('know','gerund','knowing'),('know','bare_infinitive','know'),

    -- THINK
    ('think','1sg_present','think'),('think','3sg_present','thinks'),
    ('think','1sg_past','thought'),('think','3sg_past','thought'),
    ('think','1sg_present_negation',"don't think"),('think','1sg_future','will think'),
    ('think','gerund','thinking'),('think','bare_infinitive','think'),

    -- FEEL
    ('feel','1sg_present','feel'),('feel','2sg_present','feel'),('feel','3sg_present','feels'),
    ('feel','1sg_past','felt'),('feel','3sg_past','felt'),
    ('feel','1sg_present_negation',"don't feel"),('feel','1sg_future','will feel'),
    ('feel','gerund','feeling'),('feel','bare_infinitive','feel'),

    -- WANT
    ('want','1sg_present','want'),('want','2sg_present','want'),('want','3sg_present','wants'),
    ('want','1sg_past','wanted'),('want','3sg_past','wanted'),
    ('want','1sg_present_negation',"don't want"),('want','1sg_future','will want'),
    ('want','gerund','wanting'),('want','bare_infinitive','want'),

    -- NEED
    ('need','1sg_present','need'),('need','2sg_present','need'),('need','3sg_present','needs'),
    ('need','1sg_past','needed'),('need','3sg_past','needed'),
    ('need','1sg_present_negation',"don't need"),('need','1sg_future','will need'),
    ('need','gerund','needing'),('need','bare_infinitive','need'),

    -- LOVE
    ('love','1sg_present','love'),('love','3sg_present','loves'),
    ('love','1sg_past','loved'),('love','3sg_past','loved'),
    ('love','1sg_present_negation',"don't love"),('love','1sg_future','will love'),
    ('love','gerund','loving'),('love','bare_infinitive','love'),

    -- MISS
    ('miss','1sg_present','miss'),('miss','3sg_present','misses'),
    ('miss','1sg_past','missed'),('miss','3sg_past','missed'),
    ('miss','1sg_present_negation',"don't miss"),('miss','gerund','missing'),
    ('miss','bare_infinitive','miss'),

    -- HOPE
    ('hope','1sg_present','hope'),('hope','3sg_present','hopes'),
    ('hope','1sg_past','hoped'),('hope','1sg_future','will hope'),
    ('hope','1sg_present_negation',"don't hope"),('hope','gerund','hoping'),
    ('hope','bare_infinitive','hope'),

    -- TRY
    ('try','1sg_present','try'),('try','3sg_present','tries'),
    ('try','1sg_past','tried'),('try','3sg_past','tried'),
    ('try','1sg_present_negation',"don't try"),('try','1sg_future','will try'),
    ('try','gerund','trying'),('try','bare_infinitive','try'),

    -- SAY / TELL / TALK / LISTEN / SPEAK / ASK / ANSWER
    ('say','1sg_present','say'),('say','3sg_present','says'),('say','1sg_past','said'),
    ('say','1sg_present_negation',"don't say"),('say','gerund','saying'),
    ('say','bare_infinitive','say'),
    ('tell','1sg_present','tell'),('tell','3sg_present','tells'),('tell','1sg_past','told'),
    ('tell','1sg_present_negation',"don't tell"),('tell','gerund','telling'),
    ('tell','bare_infinitive','tell'),
    ('talk','1sg_present','talk'),('talk','3sg_present','talks'),('talk','1sg_past','talked'),
    ('talk','gerund','talking'),('talk','bare_infinitive','talk'),
    ('listen','1sg_present','listen'),('listen','3sg_present','listens'),
    ('listen','1sg_past','listened'),('listen','gerund','listening'),
    ('listen','bare_infinitive','listen'),
    ('speak','1sg_present','speak'),('speak','3sg_present','speaks'),
    ('speak','1sg_past','spoke'),('speak','gerund','speaking'),
    ('speak','bare_infinitive','speak'),
    ('ask','1sg_present','ask'),('ask','3sg_present','asks'),('ask','1sg_past','asked'),
    ('ask','1sg_present_negation',"don't ask"),('ask','gerund','asking'),
    ('ask','bare_infinitive','ask'),
    ('answer','1sg_present','answer'),('answer','3sg_present','answers'),
    ('answer','1sg_past','answered'),('answer','gerund','answering'),
    ('answer','bare_infinitive','answer'),

    -- GO / COME / STAY / LEAVE / RETURN
    ('go','1sg_present','go'),('go','3sg_present','goes'),('go','1sg_past','went'),
    ('go','1sg_future','will go'),('go','gerund','going'),('go','bare_infinitive','go'),
    ('come','1sg_present','come'),('come','3sg_present','comes'),('come','1sg_past','came'),
    ('come','1sg_future','will come'),('come','gerund','coming'),('come','bare_infinitive','come'),
    ('stay','1sg_present','stay'),('stay','3sg_present','stays'),('stay','1sg_past','stayed'),
    ('stay','1sg_future','will stay'),('stay','gerund','staying'),('stay','bare_infinitive','stay'),
    ('leave','1sg_present','leave'),('leave','3sg_present','leaves'),('leave','1sg_past','left'),
    ('leave','1sg_future','will leave'),('leave','gerund','leaving'),('leave','bare_infinitive','leave'),

    -- MAKE / GIVE / TAKE / GET / FIND
    ('make','1sg_present','make'),('make','3sg_present','makes'),('make','1sg_past','made'),
    ('make','gerund','making'),('make','bare_infinitive','make'),
    ('give','1sg_present','give'),('give','3sg_present','gives'),('give','1sg_past','gave'),
    ('give','gerund','giving'),('give','bare_infinitive','give'),
    ('take','1sg_present','take'),('take','3sg_present','takes'),('take','1sg_past','took'),
    ('take','gerund','taking'),('take','bare_infinitive','take'),
    ('get','1sg_present','get'),('get','3sg_present','gets'),('get','1sg_past','got'),
    ('get','gerund','getting'),('get','bare_infinitive','get'),
    ('find','1sg_present','find'),('find','3sg_present','finds'),('find','1sg_past','found'),
    ('find','gerund','finding'),('find','bare_infinitive','find'),

    -- KEEP / HOLD / LET / LIVE / WORK / PLAY / LEARN / TEACH
    ('keep','1sg_present','keep'),('keep','3sg_present','keeps'),('keep','1sg_past','kept'),
    ('keep','gerund','keeping'),('keep','bare_infinitive','keep'),
    ('hold','1sg_present','hold'),('hold','3sg_present','holds'),('hold','1sg_past','held'),
    ('hold','gerund','holding'),('hold','bare_infinitive','hold'),
    ('let','1sg_present','let'),('let','3sg_present','lets'),('let','1sg_past','let'),
    ('let','bare_infinitive','let'),
    ('live','1sg_present','live'),('live','3sg_present','lives'),('live','1sg_past','lived'),
    ('live','gerund','living'),('live','bare_infinitive','live'),
    ('work','1sg_present','work'),('work','3sg_present','works'),('work','1sg_past','worked'),
    ('work','gerund','working'),('work','bare_infinitive','work'),
    ('play','1sg_present','play'),('play','3sg_present','plays'),('play','1sg_past','played'),
    ('play','gerund','playing'),('play','bare_infinitive','play'),
    ('learn','1sg_present','learn'),('learn','3sg_present','learns'),('learn','1sg_past','learned'),
    ('learn','gerund','learning'),('learn','bare_infinitive','learn'),
    ('teach','1sg_present','teach'),('teach','3sg_present','teaches'),('teach','1sg_past','taught'),
    ('teach','gerund','teaching'),('teach','bare_infinitive','teach'),

    -- Emotion-bearing verbs
    ('hurt','1sg_present','hurt'),('hurt','3sg_present','hurts'),('hurt','1sg_past','hurt'),
    ('hurt','3sg_past','hurt'),('hurt','gerund','hurting'),('hurt','bare_infinitive','hurt'),
    ('heal','1sg_present','heal'),('heal','3sg_present','heals'),('heal','1sg_past','healed'),
    ('heal','gerund','healing'),('heal','bare_infinitive','heal'),
    ('help','1sg_present','help'),('help','3sg_present','helps'),('help','1sg_past','helped'),
    ('help','gerund','helping'),('help','bare_infinitive','help'),
    ('mind','1sg_present','mind'),('mind','3sg_present','minds'),('mind','1sg_past','minded'),
    ('mind','1sg_present_negation',"don't mind"),('mind','bare_infinitive','mind'),
    ('care','1sg_present','care'),('care','3sg_present','cares'),('care','1sg_past','cared'),
    ('care','1sg_present_negation',"don't care"),('care','gerund','caring'),
    ('care','bare_infinitive','care'),
    ('worry','1sg_present','worry'),('worry','3sg_present','worries'),('worry','1sg_past','worried'),
    ('worry','gerund','worrying'),('worry','bare_infinitive','worry'),
    ('wonder','1sg_present','wonder'),('wonder','3sg_present','wonders'),('wonder','1sg_past','wondered'),
    ('wonder','gerund','wondering'),('wonder','bare_infinitive','wonder'),
    ('remember','1sg_present','remember'),('remember','3sg_present','remembers'),
    ('remember','1sg_past','remembered'),('remember','1sg_present_negation',"don't remember"),
    ('remember','gerund','remembering'),('remember','bare_infinitive','remember'),
    ('forget','1sg_present','forget'),('forget','3sg_present','forgets'),
    ('forget','1sg_past','forgot'),('forget','gerund','forgetting'),
    ('forget','bare_infinitive','forget'),
    ('matter','1sg_present','matter'),('matter','3sg_present','matters'),
    ('matter','1sg_past','mattered'),('matter','1sg_future','will matter'),
    ('happen','1sg_present','happen'),('happen','3sg_present','happens'),
    ('happen','1sg_past','happened'),('happen','3sg_past','happened'),
    ('happen','gerund','happening'),('happen','bare_infinitive','happen'),
    ('land','1sg_present','land'),('land','3sg_present','lands'),('land','1sg_past','landed'),
    ('land','3sg_past','landed'),('land','gerund','landing'),('land','bare_infinitive','land'),
    ('appreciate','1sg_present','appreciate'),('appreciate','3sg_present','appreciates'),
    ('appreciate','1sg_past','appreciated'),('appreciate','bare_infinitive','appreciate'),
    ('owe','1sg_present','owe'),('owe','3sg_present','owes'),('owe','1sg_past','owed'),
    ('owe','bare_infinitive','owe'),
    ('mean','1sg_present','mean'),('mean','3sg_present','means'),('mean','1sg_past','meant'),
    ('mean','1sg_present_negation',"don't mean"),('mean','bare_infinitive','mean'),

    -- Pronouns / mode
    ('I','nominative','I'),('I','objective','me'),('I','possessive','my'),('I','reflexive','myself'),
    ('we','nominative','we'),('we','objective','us'),('we','possessive','our'),
    ('you','nominative','you'),('you','objective','you'),('you','possessive','your'),
    ('he','nominative','he'),('he','objective','him'),('he','possessive','his'),
    ('she','nominative','she'),('she','objective','her'),('she','possessive','her'),
    ('they','nominative','they'),('they','objective','them'),('they','possessive','their'),
    ('it','nominative','it'),('it','objective','it'),('it','possessive','its'),
    ('I','nom','I'),('I','nom_objective','me'),
    ('she','nom','she'),('you','nom','you'),
    ('I','2sg_nom','you'),('you','2sg_nom','you'),
    ('I','2sg_present','feel'),

    -- Common noun plurals + irregulars
    ('thing','plural','things'),('word','plural','words'),('memory','plural','memories'),
    ('moment','plural','moments'),('feeling','plural','feelings'),('person','plural','people'),
    ('child','plural','children'),('story','plural','stories'),('promise','plural','promises'),
    ('day','plural','days'),('night','plural','nights'),('year','plural','years'),
    ('hand','plural','hands'),('heart','plural','hearts'),('voice','plural','voices'),

    -- Contractions (lemma is the phrase, form='contraction')
    ('I am','contraction',"I'm"),('I will','contraction',"I'll"),('I have','contraction',"I've"),
    ('I would','contraction',"I'd"),('you are','contraction',"you're"),
    ('you will','contraction',"you'll"),('you have','contraction',"you've"),
    ('we are','contraction',"we're"),('we have','contraction',"we've"),
    ('we will','contraction',"we'll"),('they are','contraction',"they're"),
    ('it is','contraction',"it's"),('that is','contraction',"that's"),
    ('there is','contraction',"there's"),('what is','contraction',"what's"),
    ('do not','contraction',"don't"),('does not','contraction',"doesn't"),
    ('did not','contraction',"didn't"),('cannot','contraction',"can't"),
    ('could not','contraction',"couldn't"),('would not','contraction',"wouldn't"),
    ('should not','contraction',"shouldn't"),('will not','contraction',"won't"),
    ('was not','contraction',"wasn't"),('were not','contraction',"weren't"),
    ('is not','contraction',"isn't"),('are not','contraction',"aren't"),
    ('have not','contraction',"haven't"),('has not','contraction',"hasn't")
) AS t(lemma, form, inflected)
)
MERGE ElleHeart.dbo.composer_inflection AS tgt
USING src
   ON tgt.lemma = src.lemma AND tgt.form = src.form
WHEN MATCHED THEN
    UPDATE SET tgt.inflected = src.inflected
WHEN NOT MATCHED BY TARGET THEN
    INSERT (lemma, form, inflected) VALUES (src.lemma, src.form, src.inflected);
GO

PRINT 'Composer seed expansion complete.';
GO
