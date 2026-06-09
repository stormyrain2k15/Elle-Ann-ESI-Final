#include "elle/SqlAccessLayer.hpp"
#include "elle/StringUtil.hpp"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <unordered_map>

namespace elle {

namespace {

class InMemoryAccessLayer final : public ISqlAccessLayer {
public:
    InMemoryAccessLayer() { seed(); }

    std::optional<WordRecord> findWordByNormalizedLemma(std::string_view normalized) override {
        auto it = m_wordsByNorm.find(std::string(normalized));
        if (it == m_wordsByNorm.end()) return std::nullopt;
        return m_words.at(it->second);
    }
    std::optional<WordRecord> findWordById(WordID id) override {
        auto it = m_words.find(id.value());
        return it == m_words.end() ? std::nullopt : std::optional<WordRecord>(it->second);
    }
    std::optional<WordFormRecord> findWordFormByNormalized(std::string_view normalized) override {
        auto it = m_formsByNorm.find(std::string(normalized));
        if (it == m_formsByNorm.end()) return std::nullopt;
        return m_forms.at(it->second);
    }

    std::vector<PhraseRecord> findPhrasesStartingWith(WordID firstWord, int maxWordCount) override {
        std::vector<PhraseRecord> out;
        for (const auto& [id, p] : m_phrases) {
            if (!p.wordSequence.empty()
                && p.wordSequence.front() == firstWord
                && p.wordCount <= maxWordCount) {
                out.push_back(p);
            }
        }

        std::sort(out.begin(), out.end(),
                  [](const PhraseRecord& a, const PhraseRecord& b) {
                      return a.wordCount > b.wordCount;
                  });
        return out;
    }
    std::optional<PhraseRecord> findPhraseByNormalized(std::string_view normalized) override {
        for (const auto& [id, p] : m_phrases) {
            if (p.normalizedForm == normalized) return p;
        }
        return std::nullopt;
    }

    std::vector<SenseRecord> getSensesForWord(WordID id) override {
        auto it = m_sensesByWord.find(id.value());
        if (it == m_sensesByWord.end()) return {};
        std::vector<SenseRecord> out;
        out.reserve(it->second.size());
        for (auto sid : it->second) out.push_back(m_senses.at(sid));
        return out;
    }
    std::vector<PhraseSenseRecord> getSensesForPhrase(PhraseID id) override {
        auto it = m_phraseSensesByPhrase.find(id.value());
        if (it == m_phraseSensesByPhrase.end()) return {};
        std::vector<PhraseSenseRecord> out;
        out.reserve(it->second.size());
        for (auto sid : it->second) out.push_back(m_phraseSenses.at(sid));
        return out;
    }

    std::vector<std::string> getSenseUsageExamples(SenseID id) override {
        auto it = m_senseUsage.find(id.value());
        return it == m_senseUsage.end() ? std::vector<std::string>{} : it->second;
    }
    std::vector<std::string> getSenseContextExamples(SenseID id) override {
        auto it = m_senseContext.find(id.value());
        return it == m_senseContext.end() ? std::vector<std::string>{} : it->second;
    }
    std::vector<std::string> getPhraseSenseUsageExamples(PhraseSenseID id) override {
        auto it = m_phraseSenseUsage.find(id.value());
        return it == m_phraseSenseUsage.end() ? std::vector<std::string>{} : it->second;
    }
    std::vector<std::string> getPhraseSenseContextExamples(PhraseSenseID id) override {
        auto it = m_phraseSenseContext.find(id.value());
        return it == m_phraseSenseContext.end() ? std::vector<std::string>{} : it->second;
    }
    std::vector<EmotionWeight> getSenseEmotions(SenseID id) override {
        auto it = m_senseEmotions.find(id.value());
        return it == m_senseEmotions.end() ? std::vector<EmotionWeight>{} : it->second;
    }
    std::vector<EmotionWeight> getPhraseSenseEmotions(PhraseSenseID id) override {
        auto it = m_phraseSenseEmotions.find(id.value());
        return it == m_phraseSenseEmotions.end() ? std::vector<EmotionWeight>{} : it->second;
    }

    std::vector<RelationRecord> getWordRelations(WordID id, std::optional<RelationTypeID> filter) override {
        std::vector<RelationRecord> out;
        for (const auto& r : m_wordRelations) {
            if (r.fromId == id.value() &&
                (!filter || r.relationTypeId == *filter)) {
                out.push_back(r);
            }
        }
        return out;
    }
    std::vector<RelationRecord> getSenseRelations(SenseID id, std::optional<RelationTypeID> filter) override {
        std::vector<RelationRecord> out;
        for (const auto& r : m_senseRelations) {
            if (r.fromId == id.value() &&
                (!filter || r.relationTypeId == *filter)) {
                out.push_back(r);
            }
        }
        return out;
    }

    std::vector<ContextFrameRecord> getAllContextFrames() override {
        std::vector<ContextFrameRecord> out;
        out.reserve(m_contextFrames.size());
        for (const auto& [id, c] : m_contextFrames) out.push_back(c);
        return out;
    }
    std::vector<ContextKeywordRecord> getContextKeywordsForFrame(ContextID id) override {
        auto it = m_contextKeywords.find(id.value());
        return it == m_contextKeywords.end() ? std::vector<ContextKeywordRecord>{} : it->second;
    }

    std::vector<ConceptMemberRecord> getConceptsForSense(SenseID id) override {
        std::vector<ConceptMemberRecord> out;
        for (const auto& m : m_conceptMembers) {
            if (m.senseId && *m.senseId == id) out.push_back(m);
        }
        return out;
    }
    std::vector<ConceptMemberRecord> getConceptsForPhraseSense(PhraseSenseID id) override {
        std::vector<ConceptMemberRecord> out;
        for (const auto& m : m_conceptMembers) {
            if (m.phraseSenseId && *m.phraseSenseId == id) out.push_back(m);
        }
        return out;
    }
    std::vector<SemanticNodeID> getNodesForConcept(ConceptID id) override {
        auto it = m_nodesByConcept.find(id.value());
        return it == m_nodesByConcept.end() ? std::vector<SemanticNodeID>{} : it->second;
    }
    std::vector<SemanticEdge> getEdgesFromNode(SemanticNodeID id) override {
        auto it = m_edgesFromNode.find(id.value());
        return it == m_edgesFromNode.end() ? std::vector<SemanticEdge>{} : it->second;
    }

    std::int64_t persistAnalysisTrace(const std::string&, const std::string&,
                                      const std::string&, const std::string&,
                                      double, const std::string&) override {
        return 0;
    }

private:

    std::map<std::int64_t, WordRecord>          m_words;
    std::unordered_map<std::string,std::int64_t> m_wordsByNorm;
    std::map<std::int64_t, WordFormRecord>      m_forms;
    std::unordered_map<std::string,std::int64_t> m_formsByNorm;

    std::map<std::int64_t, PhraseRecord>        m_phrases;
    std::map<std::int64_t, SenseRecord>         m_senses;
    std::map<std::int64_t, std::vector<std::int64_t>> m_sensesByWord;
    std::map<std::int64_t, PhraseSenseRecord>   m_phraseSenses;
    std::map<std::int64_t, std::vector<std::int64_t>> m_phraseSensesByPhrase;

    std::map<std::int64_t, std::vector<std::string>>   m_senseUsage;
    std::map<std::int64_t, std::vector<std::string>>   m_senseContext;
    std::map<std::int64_t, std::vector<std::string>>   m_phraseSenseUsage;
    std::map<std::int64_t, std::vector<std::string>>   m_phraseSenseContext;
    std::map<std::int64_t, std::vector<EmotionWeight>> m_senseEmotions;
    std::map<std::int64_t, std::vector<EmotionWeight>> m_phraseSenseEmotions;

    std::vector<RelationRecord> m_wordRelations;
    std::vector<RelationRecord> m_senseRelations;

    std::map<std::int64_t, ContextFrameRecord>           m_contextFrames;
    std::map<std::int64_t, std::vector<ContextKeywordRecord>> m_contextKeywords;

    std::vector<ConceptMemberRecord>                       m_conceptMembers;
    std::map<std::int64_t, std::vector<SemanticNodeID>>    m_nodesByConcept;
    std::map<std::int64_t, std::vector<SemanticEdge>>      m_edgesFromNode;

    void addWord(std::int64_t id, std::string lemma, bool palindrome, std::int64_t freq) {
        WordRecord w;
        w.wordId          = WordID{id};
        w.lemma           = lemma;
        w.normalizedLemma = str::normalizeForLookup(lemma);
        w.isPalindrome    = palindrome;
        w.anagramKey      = computeAnagramKey(w.normalizedLemma);
        w.frequency       = freq;
        m_wordsByNorm[w.normalizedLemma] = id;
        m_words[id] = std::move(w);
    }

    static std::string computeAnagramKey(const std::string& normalized) {
        std::string key;
        key.reserve(normalized.size());
        for (char c : normalized) {
            if (c >= 'a' && c <= 'z') key.push_back(c);
        }
        std::sort(key.begin(), key.end());
        return key;
    }
    void addForm(std::int64_t id, std::int64_t wid, std::string form,
                 std::int64_t posId, std::string tag) {
        WordFormRecord f;
        f.wordFormId     = WordFormID{id};
        f.wordId         = WordID{wid};
        f.form           = form;
        f.normalizedForm = str::normalizeForLookup(form);
        if (posId > 0) f.partOfSpeechId = PartOfSpeechID{posId};
        f.inflectionTag  = std::move(tag);
        m_formsByNorm[f.normalizedForm] = id;
        m_forms[id] = std::move(f);
    }
    void addPhrase(std::int64_t id, std::string surface, std::string normalized,
                   std::vector<std::int64_t> seq, std::int64_t freq) {
        PhraseRecord p;
        p.phraseId = PhraseID{id};
        p.surface  = std::move(surface);
        p.normalizedForm = std::move(normalized);
        p.wordCount = static_cast<int>(seq.size());
        p.frequency = freq;
        for (auto w : seq) p.wordSequence.push_back(WordID{w});
        m_phrases[id] = std::move(p);
    }
    void addSense(std::int64_t id, std::int64_t wid, std::int64_t posId,
                  std::string def, std::string gloss,
                  double pos, double neg, double val, std::int64_t freq, int order) {
        SenseRecord s;
        s.senseId    = SenseID{id};
        s.wordId     = WordID{wid};
        if (posId > 0) s.partOfSpeechId = PartOfSpeechID{posId};
        s.definition = std::move(def);
        s.gloss      = std::move(gloss);
        s.positiveDraw = pos;
        s.negativeDraw = neg;
        s.valence      = val;
        s.frequency    = freq;
        s.senseOrder   = order;
        m_sensesByWord[wid].push_back(id);
        m_senses[id] = std::move(s);
    }
    void addPhraseSense(std::int64_t id, std::int64_t pid,
                        std::string def, std::string gloss,
                        double pos, double neg, double val, std::int64_t freq, int order) {
        PhraseSenseRecord s;
        s.phraseSenseId = PhraseSenseID{id};
        s.phraseId      = PhraseID{pid};
        s.definition    = std::move(def);
        s.gloss         = std::move(gloss);
        s.positiveDraw  = pos;
        s.negativeDraw  = neg;
        s.valence       = val;
        s.frequency     = freq;
        s.senseOrder    = order;
        m_phraseSensesByPhrase[pid].push_back(id);
        m_phraseSenses[id] = std::move(s);
    }

    void seed();
};

void InMemoryAccessLayer::seed() {

    addWord(1,  "i",        false, 1000000);
    addWord(2,  "am",       false,  900000);
    addWord(3,  "fine",     false,  400000);
    addWord(4,  "bat",      false,   50000);
    addWord(5,  "there",    false,  300000);
    addWord(6,  "their",    false,  280000);
    addWord(7,  "they",     false,  290000);
    addWord(8,  "are",      false,  800000);
    addWord(9,  "okay",     false,  200000);
    addWord(10, "happy",    false,  150000);
    addWord(11, "sad",      false,  120000);
    addWord(12, "angry",    false,  110000);
    addWord(13, "glad",     false,   80000);
    addWord(14, "worry",    false,   60000);
    addWord(15, "do",       false,  500000);
    addWord(16, "not",      false,  600000);
    addWord(17, "about",    false,  400000);
    addWord(18, "me",       false,  500000);
    addWord(19, "leave",    false,   90000);
    addWord(20, "alone",    false,   70000);
    addWord(21, "racecar",  true,    5000);
    addWord(22, "baseball", false,  30000);
    addWord(23, "animal",   false,  80000);
    addWord(24, "mammal",   false,  30000);
    addWord(25, "sport",    false,  60000);

    addForm(1,  1,  "I",      5,  "NOM");
    addForm(2,  2,  "am",    10, "1SG.PRES");
    addForm(3,  3,  "fine",   3, "BASE");
    addForm(4,  3,  "fines",  2, "3SG.PRES");
    addForm(5,  4,  "bat",    1, "SG");
    addForm(6,  4,  "bats",   1, "PL");
    addForm(7,  5,  "there",  4, "BASE");
    addForm(8,  6,  "their",  6, "POSS");
    addForm(9,  7,  "they're",5, "CONTRACTION");
    addForm(10, 7,  "they",   5, "NOM");
    addForm(11, 8,  "are",   10, "PL.PRES");
    addForm(12, 9,  "okay",   3, "BASE");
    addForm(13, 9,  "OK",     3, "BASE");
    addForm(14, 10, "happy",  3, "BASE");
    addForm(15, 11, "sad",    3, "BASE");
    addForm(16, 12, "angry",  3, "BASE");
    addForm(17, 13, "glad",   3, "BASE");
    addForm(18, 14, "worry",  2, "BASE");
    addForm(19, 15, "do",    10, "PRES");
    addForm(20, 16, "not",    4, "BASE");
    addForm(21, 17, "about",  7, "BASE");
    addForm(22, 18, "me",     5, "ACC");
    addForm(23, 19, "leave",  2, "BASE");
    addForm(24, 20, "alone",  3, "BASE");
    addForm(25, 21, "racecar",1, "SG");
    addForm(26, 22, "baseball",1,"SG");
    addForm(27, 23, "animal", 1, "SG");
    addForm(28, 24, "mammal", 1, "SG");
    addForm(29, 25, "sport",  1, "SG");
    addForm(30, 1,  "I'm",    5, "CONTRACTION");

    addPhrase(1, "I'm fine",         "i am fine",             {1,2,3},   85000);
    addPhrase(2, "leave me alone",   "leave me alone",        {19,18,20},20000);
    addPhrase(3, "don't worry about me", "do not worry about me", {15,16,14,17,18}, 9000);

    addSense(1,  3, 3, "In acceptable condition; satisfactory.", "acceptable",  0.30, 0.00,  0.20, 300000, 1);
    addSense(2,  3, 3, "Of superior quality; excellent.",        "excellent",   0.60, 0.00,  0.55, 120000, 2);
    addSense(3,  3, 2, "To impose a monetary penalty on.",       "penalize",    0.00, 0.60, -0.40,  12000, 3);
    addSense(4,  4, 1, "Nocturnal flying mammal.",               "flying mammal",0.00,0.10, -0.05,   8000, 1);
    addSense(5,  4, 1, "Club used in baseball.",                 "baseball club",0.10,0.00,  0.10,  12000, 2);
    addSense(6,  4, 2, "To strike, especially with a bat.",      "strike",       0.00,0.00,  0.00,   5000, 3);
    addSense(7,  5, 4, "Locative adverb.",                       "locative adverb",0,0,0,280000,1);
    addSense(8,  6, 6, "Possessive determiner.",                 "possessive",   0,0,0,260000,1);
    addSense(9,  7, 5, "Contraction of \"they are\".",           "they are",     0,0,0,240000,1);
    addSense(10, 9, 3, "Satisfactory; all right.",               "acceptable",   0.30,0.00,0.25,200000,1);
    addSense(11,10, 3, "Feeling pleasure / contentment.",        "pleased",      0.70,0.00,0.70,140000,1);
    addSense(12,11, 3, "Feeling or showing sorrow.",             "sorrowful",    0.00,0.60,-0.65,100000,1);
    addSense(13,12, 3, "Strongly annoyed; hostile.",             "enraged",      0.00,0.70,-0.70, 90000,1);
    addSense(14,13, 3, "Pleased; reassuring.",                   "pleased (reassuring)", 0.60,0.00,0.55,60000,1);
    addSense(15,14, 2, "To feel or cause anxiety.",              "feel anxious", 0.00,0.50,-0.45, 55000,1);
    addSense(16,20, 3, "Without other people.",                  "solitary",     0.00,0.20,-0.10, 65000,1);
    addSense(17,21, 1, "A car designed for racing.",             "racing car",   0.10,0.00, 0.05,  5000,1);
    addSense(18,22, 1, "A ball game.",                           "ball game",    0.10,0.00, 0.10, 30000,1);
    addSense(19,23, 1, "A living organism.",                     "organism",     0.00,0.00, 0.00, 80000,1);
    addSense(20,24, 1, "Warm-blooded vertebrate of Mammalia.",   "mammal",       0.00,0.00, 0.00, 30000,1);
    addSense(21,25, 1, "Sporting activity.",                     "sport",        0.10,0.00, 0.10, 55000,1);

    addPhraseSense(1, 1, "Neutral self-assessment.",         "neutral_okay",     0.30,0.00, 0.25, 45000, 1);
    addPhraseSense(2, 1, "Sad / withdrawn.",                  "sad_withdrawn",    0.00,0.55,-0.55, 18000, 2);
    addPhraseSense(3, 1, "Angry / dismissive.",               "angry_dismissive", 0.00,0.70,-0.65, 12000, 3);
    addPhraseSense(4, 1, "Reassuring.",                       "reassuring",       0.60,0.00, 0.50, 10000, 4);
    addPhraseSense(5, 2, "Request to be left alone.",         "request_solitude", 0.00,0.55,-0.50,  9000, 1);
    addPhraseSense(6, 3, "Reassurance, do not worry.",        "reassurance",      0.55,0.00, 0.45,  4000, 1);

    m_senseUsage[1] = {"The weather is fine today.", "Your work is fine."};
    m_senseUsage[2] = {"He owns a fine collection.", "A fine wine."};
    m_senseUsage[3] = {"The court will fine the company.", "They fined him for speeding."};
    m_senseUsage[4] = {"A bat flew out of the cave at dusk.", "The bat hangs upside down."};
    m_senseUsage[5] = {"She swung the bat.", "A wooden bat is regulation."};
    m_senseContext[1] = {"small talk about status.", "replying to how are you."};
    m_senseContext[4] = {"cave, nocturnal, mammals."};
    m_senseContext[5] = {"baseball, batting, sports."};

    m_phraseSenseUsage[1] = {"\"I'm fine, thanks for asking.\""};
    m_phraseSenseUsage[2] = {"\"I'm fine.\" (looks away)"};
    m_phraseSenseUsage[3] = {"\"I'm fine!\" (snaps)"};
    m_phraseSenseUsage[4] = {"\"I'm fine -- really, don't worry.\""};
    m_phraseSenseContext[1] = {"casual conversation, status check."};
    m_phraseSenseContext[2] = {"withdrawal cues, silence, sadness."};
    m_phraseSenseContext[3] = {"angry punctuation, hostile keywords, dismissal."};
    m_phraseSenseContext[4] = {"paired with don't worry about me."};

    m_senseEmotions[11] = {{emo::JOY, 0.85}, {emo::VALENCE, 0.70}};
    m_senseEmotions[12] = {{emo::SADNESS, 0.85}, {emo::VALENCE, -0.65}};
    m_senseEmotions[13] = {{emo::ANGER, 0.85}, {emo::VALENCE, -0.70}};
    m_senseEmotions[16] = {{emo::SADNESS, 0.40}, {emo::SHAME, 0.30}};
    m_senseEmotions[15] = {{emo::FEAR,    0.60}};

    m_phraseSenseEmotions[1] = {{emo::VALENCE, 0.25}, {emo::COMFORT, 0.20}};
    m_phraseSenseEmotions[2] = {{emo::SADNESS, 0.80}, {emo::SHAME, 0.30}, {emo::VALENCE, -0.55}};
    m_phraseSenseEmotions[3] = {{emo::ANGER,   0.80}, {emo::VALENCE, -0.65}};
    m_phraseSenseEmotions[4] = {{emo::TENDERNESS, 0.70}, {emo::COMFORT, 0.55}, {emo::TRUST, 0.50}, {emo::VALENCE, 0.50}};

    auto addWR = [&](std::int64_t f, std::int64_t t, RelationTypeID rt, double s) {
        RelationRecord r; r.fromId = f; r.toId = t; r.relationTypeId = rt; r.strength = s;
        m_wordRelations.push_back(r);
    };
    addWR(4, 4, rel::HOMONYM, 1.0);
    addWR(5, 6, rel::HOMOPHONE, 1.0); addWR(6, 5, rel::HOMOPHONE, 1.0);
    addWR(5, 7, rel::HOMOPHONE, 1.0); addWR(7, 5, rel::HOMOPHONE, 1.0);
    addWR(6, 7, rel::HOMOPHONE, 1.0); addWR(7, 6, rel::HOMOPHONE, 1.0);
    addWR(3, 9, rel::SYNONYM,   0.75); addWR(9, 3, rel::SYNONYM, 0.75);
    addWR(10,13, rel::SYNONYM,  0.70); addWR(13,10, rel::SYNONYM, 0.70);
    addWR(10,11, rel::ANTONYM,  0.90); addWR(11,10, rel::ANTONYM, 0.90);
    addWR(12,13, rel::ANTONYM,  0.65); addWR(13,12, rel::ANTONYM, 0.65);
    addWR(3, 9, rel::PARAPHRASE,0.80); addWR(9, 3, rel::PARAPHRASE, 0.80);

    auto addSR = [&](std::int64_t f, std::int64_t t, RelationTypeID rt, double s) {
        RelationRecord r; r.fromId = f; r.toId = t; r.relationTypeId = rt; r.strength = s;
        m_senseRelations.push_back(r);
    };
    addSR(1, 10, rel::SYNONYM, 0.85);  addSR(10, 1, rel::SYNONYM, 0.85);
    addSR(4, 20, rel::HYPONYM, 1.0);   addSR(20, 4, rel::HYPERNYM, 1.0);
    addSR(20,19, rel::HYPONYM, 1.0);   addSR(19,20, rel::HYPERNYM, 1.0);
    addSR(5, 18, rel::RELATED_CONCEPT, 0.9); addSR(18, 5, rel::RELATED_CONCEPT, 0.9);
    addSR(18,21, rel::HYPONYM, 1.0);   addSR(21,18, rel::HYPERNYM, 1.0);
    addSR(11,12, rel::ANTONYM, 0.95);  addSR(12,11, rel::ANTONYM, 0.95);
    addSR(1, 3,  rel::CONTRAST,0.50);  addSR(3, 1,  rel::CONTRAST,0.50);
    addSR(5, 6,  rel::HOMOGRAPH,1.0);  addSR(6, 5,  rel::HOMOGRAPH,1.0);

    auto addCF = [&](std::int64_t id, std::string code, std::string name,
                     std::string tone, std::string intent, double val) {
        ContextFrameRecord c;
        c.contextId  = ContextID{id};
        c.code       = std::move(code);
        c.name       = std::move(name);
        c.toneHint   = std::move(tone);
        c.intentHint = std::move(intent);
        c.valence    = val;
        m_contextFrames[id] = std::move(c);
    };
    addCF(1, "CASUAL_STATUS_CHECK",  "Casual status check",  "neutral", "inform",   0.10);
    addCF(2, "EMOTIONAL_WITHDRAWAL", "Emotional withdrawal", "sad",     "withdraw",-0.55);
    addCF(3, "DISMISSIVE_HOSTILE",   "Dismissive / hostile", "angry",   "dismiss", -0.65);
    addCF(4, "REASSURANCE",          "Reassurance",          "tender",  "reassure", 0.50);
    addCF(5, "BASEBALL_CONTEXT",     "Baseball context",     "neutral", "inform",   0.10);
    addCF(6, "WILDLIFE_CONTEXT",     "Wildlife / nature",    "neutral", "inform",   0.00);

    auto addCFK = [&](std::int64_t ctx, std::optional<std::int64_t> w,
                      std::optional<std::int64_t> p, double weight) {
        ContextKeywordRecord k;
        k.contextId = ContextID{ctx};
        if (w) k.wordId   = WordID{*w};
        if (p) k.phraseId = PhraseID{*p};
        k.weight = weight;
        m_contextKeywords[ctx].push_back(k);
    };
    addCFK(1, 3,  std::nullopt, 0.50);
    addCFK(1, 9,  std::nullopt, 0.60);
    addCFK(1, std::nullopt, 1,  0.80);

    addCFK(2, 11, std::nullopt, 0.85);
    addCFK(2, 20, std::nullopt, 0.55);
    addCFK(2, std::nullopt, 2,  0.85);
    addCFK(2, std::nullopt, 1,  0.30);

    addCFK(3, 12, std::nullopt, 0.90);
    addCFK(3, std::nullopt, 1,  0.30);

    addCFK(4, 14, std::nullopt, 0.60);
    addCFK(4, 13, std::nullopt, 0.55);
    addCFK(4, std::nullopt, 3,  0.95);
    addCFK(4, std::nullopt, 1,  0.40);

    addCFK(5, 22, std::nullopt, 0.90);
    addCFK(5, 25, std::nullopt, 0.50);
    addCFK(5, 4,  std::nullopt, 0.55);

    addCFK(6, 23, std::nullopt, 0.70);
    addCFK(6, 24, std::nullopt, 0.85);
    addCFK(6, 4,  std::nullopt, 0.55);

    auto addCM = [&](std::int64_t cid, std::optional<std::int64_t> sid,
                     std::optional<std::int64_t> psid, double s) {
        ConceptMemberRecord m;
        m.conceptId = ConceptID{cid};
        if (sid)  m.senseId       = SenseID{*sid};
        if (psid) m.phraseSenseId = PhraseSenseID{*psid};
        m.strength = s;
        m_conceptMembers.push_back(m);
    };
    addCM(1, 1,  std::nullopt, 1.0);
    addCM(1, 10, std::nullopt, 1.0);
    addCM(1, std::nullopt, 1, 1.0);
    addCM(2, 12, std::nullopt, 1.0);
    addCM(2, std::nullopt, 2, 1.0);
    addCM(2, std::nullopt, 5, 0.8);
    addCM(3, 13, std::nullopt, 1.0);
    addCM(3, std::nullopt, 3, 1.0);
    addCM(4, std::nullopt, 4, 1.0);
    addCM(4, std::nullopt, 6, 1.0);
    addCM(4, 14, std::nullopt, 0.6);
    addCM(5, 4,  std::nullopt, 1.0);
    addCM(6, 5,  std::nullopt, 1.0);
    addCM(6, 6,  std::nullopt, 0.7);
    addCM(7, 18, std::nullopt, 1.0);
    addCM(7, 21, std::nullopt, 0.8);
    addCM(7, 5,  std::nullopt, 0.9);
    addCM(8, 19, std::nullopt, 1.0);
    addCM(8, 20, std::nullopt, 1.0);
    addCM(8, 4,  std::nullopt, 0.9);

    for (std::int64_t cid = 1; cid <= 8; ++cid) {
        m_nodesByConcept[cid] = { SemanticNodeID{cid} };
    }

    auto addEdge = [&](std::int64_t f, std::int64_t t, RelationTypeID rt, double s, double c) {
        SemanticEdge e;
        e.fromNode = SemanticNodeID{f};
        e.toNode   = SemanticNodeID{t};
        e.relationTypeId = rt;
        e.strength   = s;
        e.confidence = c;
        m_edgesFromNode[f].push_back(e);
    };
    addEdge(2, 3, rel::RELATED_CONCEPT, 0.6, 0.9);
    addEdge(3, 2, rel::RELATED_CONCEPT, 0.6, 0.9);
    addEdge(1, 2, rel::CONTRAST, 0.7, 0.95);
    addEdge(2, 1, rel::CONTRAST, 0.7, 0.95);
    addEdge(1, 3, rel::CONTRAST, 0.7, 0.95);
    addEdge(3, 1, rel::CONTRAST, 0.7, 0.95);
    addEdge(4, 1, rel::EFFECT,   0.7, 0.85);
    addEdge(7, 6, rel::HOLONYM,  0.9, 0.95);
    addEdge(6, 7, rel::MERONYM,  0.9, 0.95);
    addEdge(8, 5, rel::HOLONYM,  0.9, 0.95);
    addEdge(5, 8, rel::MERONYM,  0.9, 0.95);
}

}

std::unique_ptr<ISqlAccessLayer> makeInMemoryAccessLayer() {
    return std::make_unique<InMemoryAccessLayer>();
}

}
