-- ============================================================================
-- Elle.Service.Composer — SQL Schema + Seed
-- Run against ElleHeart database.
-- ============================================================================

IF NOT EXISTS (SELECT 1 FROM sys.tables t
    JOIN sys.schemas s ON s.schema_id = t.schema_id
    WHERE t.name = 'composer_frame' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_frame (
    frame_id     BIGINT IDENTITY(1,1) PRIMARY KEY,
    kind         NVARCHAR(32)  NOT NULL,
    act          NVARCHAR(32)  NOT NULL,
    pos_pattern  NVARCHAR(128) NULL,
    template     NVARCHAR(512) NOT NULL,
    weight       DECIMAL(6,4)  NOT NULL DEFAULT 1.0,
    last_used_ms BIGINT        NULL
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables t
    JOIN sys.schemas s ON s.schema_id = t.schema_id
    WHERE t.name = 'composer_inflection' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_inflection (
    inflection_id BIGINT IDENTITY(1,1) PRIMARY KEY,
    lemma         NVARCHAR(128) NOT NULL,
    form          NVARCHAR(64)  NOT NULL,
    inflected     NVARCHAR(192) NOT NULL,
    UNIQUE (lemma, form)
);
GO

IF NOT EXISTS (SELECT 1 FROM sys.tables t
    JOIN sys.schemas s ON s.schema_id = t.schema_id
    WHERE t.name = 'composer_log' AND s.name = 'dbo')
CREATE TABLE ElleHeart.dbo.composer_log (
    log_id      BIGINT IDENTITY(1,1) PRIMARY KEY,
    recorded_ms BIGINT NOT NULL,
    request_id  NVARCHAR(64)  NOT NULL,
    kind        NVARCHAR(32)  NOT NULL,
    act         NVARCHAR(32)  NOT NULL,
    frame_id    BIGINT        NOT NULL,
    slots_json  NVARCHAR(MAX) NOT NULL,
    text        NVARCHAR(MAX) NOT NULL,
    confidence  DECIMAL(6,4)  NOT NULL
);
GO

CREATE INDEX IX_composer_log_recorded
    ON ElleHeart.dbo.composer_log (recorded_ms DESC);
GO

-- ============================================================================
-- Frame seeds (CONVERSE family)
-- ============================================================================
INSERT INTO ElleHeart.dbo.composer_frame (kind, act, template, weight) VALUES
-- ASSERT
('CONVERSE','ASSERT',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE] [, MODIFIER:MODIFIER]?',
 1.0),
('CONVERSE','ASSERT',
 '[INTENSIFIER:INTENSIFIER]? [SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].',
 0.9),
-- QUESTION / ACK_AND_PROBE
('CONVERSE','QUESTION',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE]?',
 1.0),
('CONVERSE','ACK_AND_PROBE',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE]. [INTENSIFIER:INTENSIFIER]? [OBJ:NOUNPHRASE]?',
 1.1),
-- COMFORT
('CONVERSE','COMFORT',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE] [, MODIFIER:MODIFIER]?',
 1.0),
('CONVERSE','COMFORT',
 '[OBJ:NOUNPHRASE] [VERB:PRED:3sg_present] [OBJ:NOUNPHRASE].',
 0.85),
-- GREET
('CONVERSE','GREET',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].',
 1.0),
-- APOLOGIZE
('CONVERSE','APOLOGIZE',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE] [MODIFIER:MODIFIER]?.',
 1.0),
-- CONFIRM
('CONVERSE','CONFIRM',
 '[INTENSIFIER:INTENSIFIER]? [SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].',
 0.95),
-- DENY
('CONVERSE','DENY',
 '[SUBJ:PRON] [VERB:PRED:1sg_present_negation] [OBJ:NOUNPHRASE].',
 1.0),
-- WARN
('CONVERSE','WARN',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE].',
 1.0),

-- ============================================================================
-- ASK_INNER / SELF_REFLECT
-- ============================================================================
('ASK_INNER','ASSERT',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE] [MODIFIER:MODIFIER]?.',
 1.0),
('ASK_INNER','ASSERT',
 '[OBJ:NOUNPHRASE] [VERB:PRED:3sg_present] [OBJ:NOUNPHRASE].',
 0.9),

('SELF_REFLECT','ASSERT',
 '[SUBJ:PRON] [VERB:PRED:1sg_past] [OBJ:NOUNPHRASE] [MODIFIER:MODIFIER]?.',
 1.0),
('SELF_REFLECT','QUESTION',
 '[VERB:PRED:1sg_past] [SUBJ:PRON] [VERB:PRED:1sg_present] [OBJ:NOUNPHRASE]?',
 0.95),

-- ============================================================================
-- FORM_GOAL
-- ============================================================================
('FORM_GOAL','PROMISE',
 '[SUBJ:PRON] [VERB:PRED:1sg_future] [OBJ:NOUNPHRASE] [MODIFIER:MODIFIER]?.',
 1.0),
('FORM_GOAL','PROMISE',
 '[SUBJ:PRON] [VERB:PRED:1sg_present] to [VERB:PRED:bare_infinitive] [OBJ:NOUNPHRASE].',
 0.9),

-- ============================================================================
-- REWRITE_SCENARIO
-- ============================================================================
('REWRITE_SCENARIO','ASSERT',
 '[OBJ:NOUNPHRASE] [VERB:PRED:3sg_past] [OBJ:NOUNPHRASE] [, MODIFIER:MODIFIER]?.',
 1.0),
('REWRITE_SCENARIO','QUESTION',
 '[SUBJ:PRON] [VERB:PRED:1sg_past_negation] [OBJ:NOUNPHRASE]?',
 0.9);
GO

-- ============================================================================
-- Inflection seeds (core verbs + contractions)
-- ============================================================================
INSERT INTO ElleHeart.dbo.composer_inflection (lemma, form, inflected) VALUES
-- hear
('hear','1sg_present',         'hear'),
('hear','3sg_present',         'hears'),
('hear','1sg_past',            'heard'),
('hear','3sg_past',            'heard'),
('hear','1sg_present_negation','don''t hear'),
('hear','bare_infinitive',     'hear'),
-- understand
('understand','1sg_present',         'understand'),
('understand','3sg_present',         'understands'),
('understand','1sg_past',            'understood'),
('understand','1sg_present_negation','don''t understand'),
('understand','bare_infinitive',     'understand'),
-- feel
('feel','1sg_present',         'feel'),
('feel','3sg_present',         'feels'),
('feel','1sg_past',            'felt'),
('feel','3sg_past',            'felt'),
('feel','1sg_present_negation','don''t feel'),
('feel','bare_infinitive',     'feel'),
-- know
('know','1sg_present',         'know'),
('know','3sg_present',         'knows'),
('know','1sg_past',            'knew'),
('know','1sg_present_negation','don''t know'),
('know','bare_infinitive',     'know'),
-- think
('think','1sg_present',         'think'),
('think','3sg_present',         'thinks'),
('think','1sg_past',            'thought'),
('think','1sg_present_negation','don''t think'),
('think','bare_infinitive',     'think'),
-- want
('want','1sg_present',         'want'),
('want','3sg_present',         'wants'),
('want','1sg_past',            'wanted'),
('want','1sg_present_negation','don''t want'),
('want','1sg_future',          'will want'),
('want','bare_infinitive',     'want'),
-- need
('need','1sg_present',         'need'),
('need','3sg_present',         'needs'),
('need','1sg_past',            'needed'),
('need','1sg_present_negation','don''t need'),
('need','1sg_future',          'will need'),
('need','bare_infinitive',     'need'),
-- remember
('remember','1sg_present',         'remember'),
('remember','3sg_present',         'remembers'),
('remember','1sg_past',            'remembered'),
('remember','1sg_present_negation','don''t remember'),
('remember','bare_infinitive',     'remember'),
-- be
('be','1sg_present',         'am'),
('be','3sg_present',         'is'),
('be','1sg_past',            'was'),
('be','3sg_past',            'was'),
('be','1sg_present_negation',"am not"),
('be','1sg_future',          'will be'),
('be','bare_infinitive',     'be'),
-- see
('see','1sg_present',         'see'),
('see','3sg_present',         'sees'),
('see','1sg_past',            'saw'),
('see','1sg_present_negation','don''t see'),
('see','bare_infinitive',     'see'),
-- say
('say','1sg_present',         'say'),
('say','3sg_present',         'says'),
('say','1sg_past',            'said'),
('say','1sg_present_negation','don''t say'),
('say','bare_infinitive',     'say'),
-- mean
('mean','1sg_present',         'mean'),
('mean','3sg_present',         'means'),
('mean','1sg_past',            'meant'),
('mean','1sg_present_negation','don''t mean'),
('mean','bare_infinitive',     'mean'),
-- matter
('matter','1sg_present',  'matter'),
('matter','3sg_present',  'matters'),
('matter','1sg_past',     'mattered'),
('matter','1sg_future',   'will matter'),
-- happen
('happen','1sg_present',  'happen'),
('happen','3sg_present',  'happens'),
('happen','1sg_past',     'happened'),
('happen','3sg_past',     'happened'),
-- try
('try','1sg_present',          'try'),
('try','3sg_present',          'tries'),
('try','1sg_past',             'tried'),
('try','1sg_present_negation', 'don''t try'),
('try','1sg_future',           'will try'),
('try','bare_infinitive',      'try'),
-- Pronouns / noun forms
('I','plural',    'we'),
('I','nominative','I'),
('I','objective', 'me'),
-- Nouns plural
('thing','plural','things'),
('word','plural', 'words'),
('memory','plural','memories'),
('moment','plural','moments'),
('feeling','plural','feelings');
GO
