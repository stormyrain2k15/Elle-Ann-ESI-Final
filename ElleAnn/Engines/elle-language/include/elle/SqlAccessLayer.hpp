// ============================================================================
// Elle Engine -- SQL access layer interface
// File: include/elle/SqlAccessLayer.hpp
//
// Pure virtual interface. Two implementations:
//   * SqlServerAccessLayer  -- production, ODBC, parameterized only
//   * InMemoryAccessLayer   -- deterministic fixture for tests / smoke
//
// All methods are read-only (the engine never writes to the dictionary).
// Optional analysis-trace persistence is the one exception.
//
// All lookups expect *already normalized* keys (lowercased + trimmed).
// ============================================================================
#pragma once

#include "elle/Types.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elle {

class ISqlAccessLayer {
public:
    virtual ~ISqlAccessLayer() = default;

    // ---- Word / WordForm -------------------------------------------------
    virtual std::optional<WordRecord>     findWordByNormalizedLemma(std::string_view normalized) = 0;
    virtual std::optional<WordRecord>     findWordById(WordID id) = 0;
    virtual std::optional<WordFormRecord> findWordFormByNormalized(std::string_view normalized) = 0;

    // ---- Phrase ----------------------------------------------------------
    virtual std::vector<PhraseRecord>     findPhrasesStartingWith(WordID firstWord, int maxWordCount) = 0;
    virtual std::optional<PhraseRecord>   findPhraseByNormalized(std::string_view normalized) = 0;

    // ---- Sense candidates -----------------------------------------------
    virtual std::vector<SenseRecord>       getSensesForWord(WordID id) = 0;
    virtual std::vector<PhraseSenseRecord> getSensesForPhrase(PhraseID id) = 0;

    // ---- Examples / emotion ---------------------------------------------
    virtual std::vector<std::string>       getSenseUsageExamples(SenseID id) = 0;
    virtual std::vector<std::string>       getSenseContextExamples(SenseID id) = 0;
    virtual std::vector<std::string>       getPhraseSenseUsageExamples(PhraseSenseID id) = 0;
    virtual std::vector<std::string>       getPhraseSenseContextExamples(PhraseSenseID id) = 0;
    virtual std::vector<EmotionWeight>     getSenseEmotions(SenseID id) = 0;
    virtual std::vector<EmotionWeight>     getPhraseSenseEmotions(PhraseSenseID id) = 0;

    // ---- Relations -------------------------------------------------------
    virtual std::vector<RelationRecord>    getWordRelations(WordID id, std::optional<RelationTypeID> filter) = 0;
    virtual std::vector<RelationRecord>    getSenseRelations(SenseID id, std::optional<RelationTypeID> filter) = 0;

    // ---- Context frames --------------------------------------------------
    virtual std::vector<ContextFrameRecord>   getAllContextFrames() = 0;
    virtual std::vector<ContextKeywordRecord> getContextKeywordsForFrame(ContextID id) = 0;

    // ---- Concepts / graph ------------------------------------------------
    virtual std::vector<ConceptMemberRecord> getConceptsForSense(SenseID id) = 0;
    virtual std::vector<ConceptMemberRecord> getConceptsForPhraseSense(PhraseSenseID id) = 0;
    virtual std::vector<SemanticNodeID>      getNodesForConcept(ConceptID id) = 0;
    virtual std::vector<SemanticEdge>        getEdgesFromNode(SemanticNodeID id) = 0;

    // ---- Optional persistence -------------------------------------------
    // Returns 0 if persistence is not supported (in-memory backend).
    virtual std::int64_t persistAnalysisTrace(const std::string& rawInput,
                                              const std::string& normalizedInput,
                                              const std::string& meaningJson,
                                              const std::string& traceJson,
                                              double confidence,
                                              const std::string& engineVersion) = 0;
};

// Factory for the in-memory access layer (always available, used by tests).
std::unique_ptr<ISqlAccessLayer> makeInMemoryAccessLayer();

} // namespace elle
