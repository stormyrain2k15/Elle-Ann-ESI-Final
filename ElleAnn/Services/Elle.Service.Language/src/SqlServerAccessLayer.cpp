#include "elle/SqlServerAccessLayer.hpp"

#include <utility>

namespace elle {

namespace {

template <class IdT>
IdT idCast(std::int64_t v) { return IdT{v}; }

}

SqlServerAccessLayer::SqlServerAccessLayer(const DatabaseConfig& cfg)
    : m_conn(std::make_unique<odbc::Connection>(cfg)) {}

SqlServerAccessLayer::~SqlServerAccessLayer() = default;

std::optional<WordRecord>
SqlServerAccessLayer::findWordByNormalizedLemma(std::string_view normalized) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT WordID, Lemma, NormalizedLemma, IsPalindrome, Frequency "
        "FROM dbo.Word WHERE NormalizedLemma = ?");
    st.bindUtf8(normalized);
    st.execute();
    if (!st.fetch()) return std::nullopt;
    WordRecord w;
    w.wordId          = idCast<WordID>(st.getInt64(1));
    w.lemma           = st.getString(2);
    w.normalizedLemma = st.getString(3);
    w.isPalindrome    = st.getBool(4);
    w.frequency       = st.getInt64(5);
    return w;
}

std::optional<WordRecord> SqlServerAccessLayer::findWordById(WordID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT WordID, Lemma, NormalizedLemma, IsPalindrome, Frequency "
        "FROM dbo.Word WHERE WordID = ?");
    st.bindInt64(id.value());
    st.execute();
    if (!st.fetch()) return std::nullopt;
    WordRecord w;
    w.wordId          = idCast<WordID>(st.getInt64(1));
    w.lemma           = st.getString(2);
    w.normalizedLemma = st.getString(3);
    w.isPalindrome    = st.getBool(4);
    w.frequency       = st.getInt64(5);
    return w;
}

std::optional<WordFormRecord>
SqlServerAccessLayer::findWordFormByNormalized(std::string_view normalized) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT TOP 1 WordFormID, WordID, Form, NormalizedForm, "
        "PartOfSpeechID, ISNULL(InflectionTag,N'') "
        "FROM dbo.WordForm WHERE NormalizedForm = ? "
        "ORDER BY WordFormID");
    st.bindUtf8(normalized);
    st.execute();
    if (!st.fetch()) return std::nullopt;
    WordFormRecord f;
    f.wordFormId      = idCast<WordFormID>(st.getInt64(1));
    f.wordId          = idCast<WordID>(st.getInt64(2));
    f.form            = st.getString(3);
    f.normalizedForm  = st.getString(4);
    if (auto pos = st.getInt64Optional(5)) f.partOfSpeechId = PartOfSpeechID{*pos};
    f.inflectionTag   = st.getString(6);
    return f;
}

std::vector<PhraseRecord>
SqlServerAccessLayer::findPhrasesStartingWith(WordID firstWord, int maxWordCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PhraseRecord> phrases;
    {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT p.PhraseID, p.Surface, p.NormalizedForm, p.WordCount, p.Frequency "
            "FROM dbo.Phrase p "
            "INNER JOIN dbo.PhraseWord pw ON pw.PhraseID = p.PhraseID AND pw.Position = 0 "
            "WHERE pw.WordID = ? AND p.WordCount <= ? "
            "ORDER BY p.WordCount DESC, p.Frequency DESC");
        st.bindInt64(firstWord.value());
        st.bindInt64(maxWordCount);
        st.execute();
        while (st.fetch()) {
            PhraseRecord p;
            p.phraseId       = idCast<PhraseID>(st.getInt64(1));
            p.surface        = st.getString(2);
            p.normalizedForm = st.getString(3);
            p.wordCount      = static_cast<int>(st.getInt64(4));
            p.frequency      = st.getInt64(5);
            phrases.push_back(std::move(p));
        }
    }
    for (auto& p : phrases) {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT WordID FROM dbo.PhraseWord WHERE PhraseID = ? ORDER BY Position ASC");
        st.bindInt64(p.phraseId.value());
        st.execute();
        while (st.fetch()) {
            if (auto v = st.getInt64Optional(1)) p.wordSequence.push_back(WordID{*v});
        }
    }
    return phrases;
}

std::optional<PhraseRecord>
SqlServerAccessLayer::findPhraseByNormalized(std::string_view normalized) {
    std::lock_guard<std::mutex> lock(m_mutex);
    PhraseRecord p;
    {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT PhraseID, Surface, NormalizedForm, WordCount, Frequency "
            "FROM dbo.Phrase WHERE NormalizedForm = ?");
        st.bindUtf8(normalized);
        st.execute();
        if (!st.fetch()) return std::nullopt;
        p.phraseId       = idCast<PhraseID>(st.getInt64(1));
        p.surface        = st.getString(2);
        p.normalizedForm = st.getString(3);
        p.wordCount      = static_cast<int>(st.getInt64(4));
        p.frequency      = st.getInt64(5);
    }
    {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT WordID FROM dbo.PhraseWord WHERE PhraseID = ? ORDER BY Position ASC");
        st.bindInt64(p.phraseId.value());
        st.execute();
        while (st.fetch()) {
            if (auto v = st.getInt64Optional(1)) p.wordSequence.push_back(WordID{*v});
        }
    }
    return p;
}

std::vector<SenseRecord> SqlServerAccessLayer::getSensesForWord(WordID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT SenseID, WordID, PartOfSpeechID, Definition, ISNULL(Gloss,N''), "
        "       PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder "
        "FROM dbo.Sense WHERE WordID = ? ORDER BY SenseOrder ASC");
    st.bindInt64(id.value());
    st.execute();
    std::vector<SenseRecord> out;
    while (st.fetch()) {
        SenseRecord s;
        s.senseId     = SenseID{st.getInt64(1)};
        s.wordId      = WordID{st.getInt64(2)};
        if (auto pos = st.getInt64Optional(3)) s.partOfSpeechId = PartOfSpeechID{*pos};
        s.definition  = st.getString(4);
        s.gloss       = st.getString(5);
        s.positiveDraw= st.getDouble(6);
        s.negativeDraw= st.getDouble(7);
        s.valence     = st.getDouble(8);
        s.frequency   = st.getInt64(9);
        s.senseOrder  = static_cast<int>(st.getInt64(10));
        out.push_back(std::move(s));
    }
    return out;
}

std::vector<PhraseSenseRecord> SqlServerAccessLayer::getSensesForPhrase(PhraseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT PhraseSenseID, PhraseID, Definition, ISNULL(Gloss,N''), "
        "       PositiveDraw, NegativeDraw, Valence, Frequency, SenseOrder "
        "FROM dbo.PhraseSense WHERE PhraseID = ? ORDER BY SenseOrder ASC");
    st.bindInt64(id.value());
    st.execute();
    std::vector<PhraseSenseRecord> out;
    while (st.fetch()) {
        PhraseSenseRecord s;
        s.phraseSenseId = PhraseSenseID{st.getInt64(1)};
        s.phraseId      = PhraseID{st.getInt64(2)};
        s.definition    = st.getString(3);
        s.gloss         = st.getString(4);
        s.positiveDraw  = st.getDouble(5);
        s.negativeDraw  = st.getDouble(6);
        s.valence       = st.getDouble(7);
        s.frequency     = st.getInt64(8);
        s.senseOrder    = static_cast<int>(st.getInt64(9));
        out.push_back(std::move(s));
    }
    return out;
}

namespace {

std::vector<std::string> fetchStrings(odbc::Connection::Statement& st) {
    std::vector<std::string> out;
    while (st.fetch()) out.push_back(st.getString(1));
    return out;
}

std::vector<EmotionWeight> fetchEmotions(odbc::Connection::Statement& st) {
    std::vector<EmotionWeight> out;
    while (st.fetch()) {
        EmotionWeight e;
        e.emotionId = EmotionID{st.getInt64(1)};
        e.weight    = st.getDouble(2);
        out.push_back(e);
    }
    return out;
}

}

std::vector<std::string> SqlServerAccessLayer::getSenseUsageExamples(SenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ExampleText FROM dbo.SenseUsageExample WHERE SenseID = ? ORDER BY Slot");
    st.bindInt64(id.value()); st.execute();
    return fetchStrings(st);
}

std::vector<std::string> SqlServerAccessLayer::getSenseContextExamples(SenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ContextText FROM dbo.SenseContextExample WHERE SenseID = ? ORDER BY Slot");
    st.bindInt64(id.value()); st.execute();
    return fetchStrings(st);
}

std::vector<std::string> SqlServerAccessLayer::getPhraseSenseUsageExamples(PhraseSenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ExampleText FROM dbo.SenseUsageExample WHERE PhraseSenseID = ? ORDER BY Slot");
    st.bindInt64(id.value()); st.execute();
    return fetchStrings(st);
}

std::vector<std::string> SqlServerAccessLayer::getPhraseSenseContextExamples(PhraseSenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ContextText FROM dbo.SenseContextExample WHERE PhraseSenseID = ? ORDER BY Slot");
    st.bindInt64(id.value()); st.execute();
    return fetchStrings(st);
}

std::vector<EmotionWeight> SqlServerAccessLayer::getSenseEmotions(SenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT EmotionID, Weight FROM dbo.SenseEmotion WHERE SenseID = ?");
    st.bindInt64(id.value()); st.execute();
    return fetchEmotions(st);
}

std::vector<EmotionWeight> SqlServerAccessLayer::getPhraseSenseEmotions(PhraseSenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT EmotionID, Weight FROM dbo.SenseEmotion WHERE PhraseSenseID = ?");
    st.bindInt64(id.value()); st.execute();
    return fetchEmotions(st);
}

namespace {

std::vector<RelationRecord> readRelations(odbc::Connection::Statement& st) {
    std::vector<RelationRecord> out;
    while (st.fetch()) {
        RelationRecord r;
        r.fromId         = st.getInt64(1);
        r.toId           = st.getInt64(2);
        r.relationTypeId = RelationTypeID{st.getInt64(3)};
        r.strength       = st.getDouble(4);
        out.push_back(r);
    }
    return out;
}

}

std::vector<RelationRecord>
SqlServerAccessLayer::getWordRelations(WordID id, std::optional<RelationTypeID> filter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (filter) {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT FromWordID, ToWordID, RelationTypeID, Strength "
            "FROM dbo.WordRelation WHERE FromWordID = ? AND RelationTypeID = ?");
        st.bindInt64(id.value());
        st.bindInt64(filter->value());
        st.execute();
        return readRelations(st);
    }
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT FromWordID, ToWordID, RelationTypeID, Strength "
        "FROM dbo.WordRelation WHERE FromWordID = ?");
    st.bindInt64(id.value());
    st.execute();
    return readRelations(st);
}

std::vector<RelationRecord>
SqlServerAccessLayer::getSenseRelations(SenseID id, std::optional<RelationTypeID> filter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (filter) {
        odbc::Connection::Statement st(m_conn->dbc(),
            "SELECT FromSenseID, ToSenseID, RelationTypeID, Strength "
            "FROM dbo.SenseRelation WHERE FromSenseID = ? AND RelationTypeID = ?");
        st.bindInt64(id.value());
        st.bindInt64(filter->value());
        st.execute();
        return readRelations(st);
    }
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT FromSenseID, ToSenseID, RelationTypeID, Strength "
        "FROM dbo.SenseRelation WHERE FromSenseID = ?");
    st.bindInt64(id.value());
    st.execute();
    return readRelations(st);
}

std::vector<ContextFrameRecord> SqlServerAccessLayer::getAllContextFrames() {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ContextID, Code, Name, ISNULL(ToneHint,N''), ISNULL(IntentHint,N''), Valence "
        "FROM dbo.ContextFrame");
    st.execute();
    std::vector<ContextFrameRecord> out;
    while (st.fetch()) {
        ContextFrameRecord c;
        c.contextId  = ContextID{st.getInt64(1)};
        c.code       = st.getString(2);
        c.name       = st.getString(3);
        c.toneHint   = st.getString(4);
        c.intentHint = st.getString(5);
        c.valence    = st.getDouble(6);
        out.push_back(std::move(c));
    }
    return out;
}

std::vector<ContextKeywordRecord>
SqlServerAccessLayer::getContextKeywordsForFrame(ContextID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ContextID, WordID, PhraseID, Weight "
        "FROM dbo.ContextFrameKeyword WHERE ContextID = ?");
    st.bindInt64(id.value()); st.execute();
    std::vector<ContextKeywordRecord> out;
    while (st.fetch()) {
        ContextKeywordRecord k;
        k.contextId = ContextID{st.getInt64(1)};
        if (auto w = st.getInt64Optional(2)) k.wordId   = WordID{*w};
        if (auto p = st.getInt64Optional(3)) k.phraseId = PhraseID{*p};
        k.weight = st.getDouble(4);
        out.push_back(k);
    }
    return out;
}

namespace {

std::vector<ConceptMemberRecord> readConceptMembers(odbc::Connection::Statement& st) {
    std::vector<ConceptMemberRecord> out;
    while (st.fetch()) {
        ConceptMemberRecord m;
        m.conceptId = ConceptID{st.getInt64(1)};
        if (auto s = st.getInt64Optional(2)) m.senseId       = SenseID{*s};
        if (auto p = st.getInt64Optional(3)) m.phraseSenseId = PhraseSenseID{*p};
        m.strength = st.getDouble(4);
        out.push_back(m);
    }
    return out;
}

}

std::vector<ConceptMemberRecord> SqlServerAccessLayer::getConceptsForSense(SenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ConceptID, SenseID, PhraseSenseID, Strength "
        "FROM dbo.ConceptMember WHERE SenseID = ?");
    st.bindInt64(id.value()); st.execute();
    return readConceptMembers(st);
}

std::vector<ConceptMemberRecord> SqlServerAccessLayer::getConceptsForPhraseSense(PhraseSenseID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT ConceptID, SenseID, PhraseSenseID, Strength "
        "FROM dbo.ConceptMember WHERE PhraseSenseID = ?");
    st.bindInt64(id.value()); st.execute();
    return readConceptMembers(st);
}

std::vector<SemanticNodeID> SqlServerAccessLayer::getNodesForConcept(ConceptID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT SemanticNodeID FROM dbo.SemanticNode WHERE ConceptID = ?");
    st.bindInt64(id.value()); st.execute();
    std::vector<SemanticNodeID> out;
    while (st.fetch()) out.push_back(SemanticNodeID{st.getInt64(1)});
    return out;
}

std::vector<SemanticEdge> SqlServerAccessLayer::getEdgesFromNode(SemanticNodeID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "SELECT FromNodeID, ToNodeID, RelationTypeID, Strength, Confidence "
        "FROM dbo.SemanticRelation WHERE FromNodeID = ?");
    st.bindInt64(id.value()); st.execute();
    std::vector<SemanticEdge> out;
    while (st.fetch()) {
        SemanticEdge e;
        e.fromNode       = SemanticNodeID{st.getInt64(1)};
        e.toNode         = SemanticNodeID{st.getInt64(2)};
        e.relationTypeId = RelationTypeID{st.getInt64(3)};
        e.strength       = st.getDouble(4);
        e.confidence     = st.getDouble(5);
        out.push_back(e);
    }
    return out;
}

std::int64_t SqlServerAccessLayer::persistAnalysisTrace(
    const std::string& rawInput,
    const std::string& normalizedInput,
    const std::string& meaningJson,
    const std::string& traceJson,
    double confidence,
    const std::string& engineVersion) {
    std::lock_guard<std::mutex> lock(m_mutex);
    odbc::Connection::Statement st(m_conn->dbc(),
        "INSERT INTO dbo.AnalysisTrace "
        "(RawInput, NormalizedInput, MeaningJson, TraceJson, ConfidenceScore, EngineVersion) "
        "OUTPUT INSERTED.AnalysisTraceID "
        "VALUES (?,?,?,?,?,?)");
    st.bindUtf8(rawInput);
    st.bindUtf8(normalizedInput);
    st.bindUtf8(meaningJson);
    st.bindUtf8(traceJson);
    st.bindDouble(confidence);
    st.bindUtf8(engineVersion);
    st.execute();
    if (!st.fetch()) return 0;
    return st.getInt64(1);
}

}
