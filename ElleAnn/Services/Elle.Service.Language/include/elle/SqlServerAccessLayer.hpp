#pragma once

#include "elle/Config.hpp"
#include "elle/OdbcConnection.hpp"
#include "elle/SqlAccessLayer.hpp"

#include <memory>
#include <mutex>

namespace elle {

class SqlServerAccessLayer final : public ISqlAccessLayer {
public:
    explicit SqlServerAccessLayer(const DatabaseConfig& cfg);
    ~SqlServerAccessLayer() override;

    std::optional<WordRecord>     findWordByNormalizedLemma(std::string_view normalized) override;
    std::optional<WordRecord>     findWordById(WordID id) override;
    std::optional<WordFormRecord> findWordFormByNormalized(std::string_view normalized) override;

    std::vector<PhraseRecord>     findPhrasesStartingWith(WordID firstWord, int maxWordCount) override;
    std::optional<PhraseRecord>   findPhraseByNormalized(std::string_view normalized) override;

    std::vector<SenseRecord>       getSensesForWord(WordID id) override;
    std::vector<PhraseSenseRecord> getSensesForPhrase(PhraseID id) override;

    std::vector<std::string>   getSenseUsageExamples(SenseID id) override;
    std::vector<std::string>   getSenseContextExamples(SenseID id) override;
    std::vector<std::string>   getPhraseSenseUsageExamples(PhraseSenseID id) override;
    std::vector<std::string>   getPhraseSenseContextExamples(PhraseSenseID id) override;
    std::vector<EmotionWeight> getSenseEmotions(SenseID id) override;
    std::vector<EmotionWeight> getPhraseSenseEmotions(PhraseSenseID id) override;

    std::vector<RelationRecord> getWordRelations(WordID id, std::optional<RelationTypeID> filter) override;
    std::vector<RelationRecord> getSenseRelations(SenseID id, std::optional<RelationTypeID> filter) override;

    std::vector<ContextFrameRecord>   getAllContextFrames() override;
    std::vector<ContextKeywordRecord> getContextKeywordsForFrame(ContextID id) override;

    std::vector<ConceptMemberRecord> getConceptsForSense(SenseID id) override;
    std::vector<ConceptMemberRecord> getConceptsForPhraseSense(PhraseSenseID id) override;
    std::vector<SemanticNodeID>      getNodesForConcept(ConceptID id) override;
    std::vector<SemanticEdge>        getEdgesFromNode(SemanticNodeID id) override;

    std::int64_t persistAnalysisTrace(const std::string& rawInput,
                                      const std::string& normalizedInput,
                                      const std::string& meaningJson,
                                      const std::string& traceJson,
                                      double confidence,
                                      const std::string& engineVersion) override;

private:
    std::unique_ptr<odbc::Connection> m_conn;
    std::mutex                        m_mutex;
};

}
