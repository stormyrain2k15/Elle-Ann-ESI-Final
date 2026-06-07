#pragma once

#include "elle/Config.hpp"
#include "elle/ContextComparator.hpp"
#include "elle/DebugTrace.hpp"
#include "elle/EmotionalWeightProcessor.hpp"
#include "elle/InputNormalizer.hpp"
#include "elle/IntegerSequenceBuilder.hpp"
#include "elle/MeaningObjectBuilder.hpp"
#include "elle/PhraseScanner.hpp"
#include "elle/SemanticGraphWalker.hpp"
#include "elle/SenseCandidateResolver.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"
#include "elle/WordFormResolver.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace elle {

struct AnalysisResult {
    MeaningObject meaning;
    DebugTrace    trace;
};

class Engine {
public:
    Engine(std::shared_ptr<ISqlAccessLayer> db, EngineConfig config);

    AnalysisResult analyze(std::string_view rawInput,
                           const ConversationContext& convo = {});

    std::int64_t persist(const AnalysisResult& result);

    [[nodiscard]] const EngineConfig& config() const noexcept { return m_config; }

private:
    std::shared_ptr<ISqlAccessLayer> m_db;
    EngineConfig                     m_config;

    InputNormalizer                  m_normalizer;
    PhraseScanner                    m_phraseScanner;
    WordFormResolver                 m_wordResolver;
    IntegerSequenceBuilder           m_sequenceBuilder;
    ContextComparator                m_contextComparator;
    SenseCandidateResolver           m_senseResolver;
    EmotionalWeightProcessor         m_emotionProcessor;
    SemanticGraphWalker              m_graphWalker;
    MeaningObjectBuilder             m_meaningBuilder;
};

}
