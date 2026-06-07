USE EllesLanguage;
GO
SET NOCOUNT ON;
GO

SET IDENTITY_INSERT dbo.Sense ON;
INSERT INTO dbo.Sense
    (SenseID, WordID, PartOfSpeechID, Definition,                              Gloss,                          PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder) VALUES

    (1,  3,  3, N'In acceptable condition; satisfactory.',                     N'acceptable',                   0.30,  0.00,  0.20,  300000, 1),
    (2,  3,  3, N'Of superior quality; excellent.',                            N'excellent',                    0.60,  0.00,  0.55,  120000, 2),

    (3,  3,  2, N'To impose a monetary penalty on.',                           N'penalize',                     0.00,  0.60, -0.40,   12000, 3),

    (4,  4,  1, N'A nocturnal flying mammal of the order Chiroptera.',         N'flying mammal',                0.00,  0.10, -0.05,    8000, 1),

    (5,  4,  1, N'A wooden or metal club used to strike the ball in baseball.',N'baseball club',                0.10,  0.00,  0.10,   12000, 2),

    (6,  4,  2, N'To strike, especially with a bat.',                          N'strike',                       0.00,  0.00,  0.00,    5000, 3),

    (7,  5,  4, N'In, at, or to that place or position.',                      N'locative adverb',              0.00,  0.00,  0.00,  280000, 1),
    (8,  6,  6, N'Belonging to or associated with the people previously mentioned.',
                                                                               N'possessive determiner',        0.00,  0.00,  0.00,  260000, 1),
    (9,  7,  5, N'Contraction of "they are"; nominative plural with present-tense be.',
                                                                               N'they are (contraction)',       0.00,  0.00,  0.00,  240000, 1),

    (10, 9,  3, N'Satisfactory; all right.',                                   N'acceptable',                   0.30,  0.00,  0.25,  200000, 1),

    (11, 10, 3, N'Feeling or showing pleasure or contentment.',                N'pleased',                      0.70,  0.00,  0.70,  140000, 1),
    (12, 11, 3, N'Feeling or showing sorrow; unhappy.',                        N'sorrowful',                    0.00,  0.60, -0.65,  100000, 1),
    (13, 12, 3, N'Having a strong feeling of annoyance, displeasure, or hostility.',
                                                                               N'enraged',                      0.00,  0.70, -0.70,   90000, 1),
    (14, 13, 3, N'Pleased; delighted; especially in a reassuring sense.',      N'pleased (reassuring)',         0.60,  0.00,  0.55,   60000, 1),
    (15, 14, 2, N'To feel or cause to feel anxious or troubled.',              N'feel anxious',                 0.00,  0.50, -0.45,   55000, 1),

    (16, 20, 3, N'Without other people; on one''s own.',                       N'solitary',                     0.00,  0.20, -0.10,   65000, 1),

    (17, 21, 1, N'A car designed and built for racing.',                       N'racing car',                   0.10,  0.00,  0.05,    5000, 1),

    (18, 22, 1, N'A ball game played between two teams of nine.',              N'ball game',                    0.10,  0.00,  0.10,   30000, 1),
    (19, 23, 1, N'A living organism that feeds on organic matter.',            N'living organism',              0.00,  0.00,  0.00,   80000, 1),
    (20, 24, 1, N'A warm-blooded vertebrate animal of the class Mammalia.',    N'class Mammalia',               0.00,  0.00,  0.00,   30000, 1),
    (21, 25, 1, N'An activity involving physical exertion and skill.',         N'sporting activity',            0.10,  0.00,  0.10,   55000, 1);
SET IDENTITY_INSERT dbo.Sense OFF;

SET IDENTITY_INSERT dbo.PhraseSense ON;
INSERT INTO dbo.PhraseSense (PhraseSenseID, PhraseID, Definition, Gloss, PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder) VALUES
    (1, 1, N'Neutral self-assessment: the speaker reports being okay.',
              N'neutral_okay',         0.30,  0.00,  0.25,  45000, 1),
    (2, 1, N'Sad / withdrawn: the speaker is not okay but does not want to talk.',
              N'sad_withdrawn',        0.00,  0.55, -0.55,  18000, 2),
    (3, 1, N'Angry / dismissive: the speaker is rebuffing concern.',
              N'angry_dismissive',     0.00,  0.70, -0.65,  12000, 3),
    (4, 1, N'Reassuring: the speaker wants the listener not to worry.',
              N'reassuring',           0.60,  0.00,  0.50,  10000, 4);

INSERT INTO dbo.PhraseSense (PhraseSenseID, PhraseID, Definition, Gloss, PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder) VALUES
    (5, 2, N'Request to be left in solitude; often emotional withdrawal.',
              N'request_solitude',     0.00,  0.55, -0.50,  9000, 1),
    (6, 3, N'Reassurance: the speaker does not want the listener to worry.',
              N'reassurance',          0.55,  0.00,  0.45,  4000, 1);
SET IDENTITY_INSERT dbo.PhraseSense OFF;

INSERT INTO dbo.SenseUsageExample (SenseID, ExampleText, Slot, PhraseSenseID) VALUES
    (1,  N'The weather is fine today.',           1, NULL),
    (1,  N'Your work is fine.',                   2, NULL),
    (2,  N'He owns a fine collection of watches.',1, NULL),
    (2,  N'A fine wine deserves a fine meal.',    2, NULL),
    (3,  N'The court will fine the company.',     1, NULL),
    (3,  N'They fined him for speeding.',         2, NULL),
    (4,  N'A bat flew out of the cave at dusk.',  1, NULL),
    (4,  N'The bat hangs upside down in trees.',  2, NULL),
    (5,  N'She swung the bat and hit a homer.',   1, NULL),
    (5,  N'A wooden bat is regulation in MLB.',   2, NULL);

INSERT INTO dbo.SenseUsageExample (SenseID, ExampleText, Slot, PhraseSenseID) VALUES
    (NULL, N'"I''m fine, thanks for asking."',                      1, 1),
    (NULL, N'"I''m fine."  (looks away, voice low)',                2, 2),
    (NULL, N'"I''m fine!"  (snaps; eye contact broken)',            1, 3),
    (NULL, N'"I''m fine -- really, don''t worry about me."',        2, 4);

INSERT INTO dbo.SenseContextExample (SenseID, ContextText, Slot, PhraseSenseID) VALUES
    (1,  N'small talk about weather, conditions, or status checks.',           1, NULL),
    (1,  N'replying to "how are you?" in a casual setting.',                   2, NULL),
    (4,  N'cave biology, nocturnal animals, mammals.',                         1, NULL),
    (5,  N'baseball, batting practice, sports equipment.',                     1, NULL);

INSERT INTO dbo.SenseContextExample (SenseID, ContextText, Slot, PhraseSenseID) VALUES
    (NULL, N'casual conversation, status check; no distress cues.',                  1, 1),
    (NULL, N'preceded or followed by withdrawal cues, silence, sad keywords.',       1, 2),
    (NULL, N'angry punctuation (!), hostile keywords nearby, dismissive cues.',      1, 3),
    (NULL, N'paired with "don''t worry about me" or other reassurance language.',    1, 4);

INSERT INTO dbo.SenseEmotion (SenseID, EmotionID, Weight, PhraseSenseID) VALUES

    (11, 5,  0.85, NULL),
    (11, 1,  0.70, NULL),

    (12, 4,  0.85, NULL),
    (12, 1, -0.65, NULL),

    (13, 2,  0.85, NULL),
    (13, 1, -0.70, NULL),

    (16, 4,  0.40, NULL),
    (16, 9,  0.30, NULL),

    (15, 3,  0.60, NULL);

INSERT INTO dbo.SenseEmotion (SenseID, EmotionID, Weight, PhraseSenseID) VALUES

    (NULL, 1,   0.25, 1),
    (NULL, 8,   0.20, 1),

    (NULL, 4,   0.80, 2),
    (NULL, 9,   0.30, 2),
    (NULL, 1,  -0.55, 2),

    (NULL, 2,   0.80, 3),
    (NULL, 1,  -0.65, 3),

    (NULL, 7,   0.70, 4),
    (NULL, 8,   0.55, 4),
    (NULL, 6,   0.50, 4),
    (NULL, 1,   0.50, 4);
GO
