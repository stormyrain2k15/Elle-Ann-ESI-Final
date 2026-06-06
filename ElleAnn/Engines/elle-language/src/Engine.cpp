// ============================================================================
// Elle Engine -- Engine facade implementation
// File: src/Engine.cpp
// ============================================================================
#include "elle/Engine.hpp"
#include "elle/MeaningObjectBuilder.hpp"

#include <stdexcept>
#include <utility>

namespace elle {

Engine::Engine(std::shared_ptr<ISqlAccessLayer> db, EngineConfig cfg)
    : m_db(std::move(db)),
      m_config(std::move(cfg)),
      m_normalizer(),
      m_phraseScanner(*m_db, m_config.cache.phraseCacheSize),
      m_wordResolver(*m_db, m_config.cache.wordCacheSize, m_config.cache.formCacheSize),
      m_sequenceBuilder(*m_db, m_wordResolver),
      m_contextComparator(*m_db, m_config.cache.contextFrameCacheSize),
      m_senseResolver(*m_db, m_config.weights, m_config.cache.senseCacheSize),
      m_emotionProcessor(*m_db),
      m_graphWalker(*m_db, m_config.graph),
      m_meaningBuilder() {
    if (!m_db) {
        throw std::invalid_argument("Engine: ISqlAccessLayer must be non-null");
    }
}

AnalysisResult Engine::analyze(std::string_view raw, const ConversationContext& convo) {
    AnalysisResult result;
    DebugTrace& trace = result.trace;

    // 1. Normalize.
    auto normalized = m_normalizer.normalize(raw, trace);

    // 2. Phrase scan.
    auto phraseMatches = m_phraseScanner.scan(normalized.lexemes, trace);

    // 3-4. Resolve + build integer sequence.
    auto sequence = m_sequenceBuilder.build(normalized, phraseMatches, trace);

    // 6. Compare context first -- the sense resolver needs frame matches.
    auto matches = m_contextComparator.compare(sequence, convo, trace);

    // 5. Sense candidate resolver.
    auto resolved = m_senseResolver.resolve(sequence, matches, convo, trace);

    // 7. Emotional weight processor.
    auto emotional = m_emotionProcessor.aggregate(resolved, sequence, trace);

    // 8. Semantic graph walk.
    auto paths = m_graphWalker.walk(resolved, trace);

    // 9. Meaning object.
    result.meaning = m_meaningBuilder.build(
        std::string(raw), normalized.normalizedText, std::move(sequence),
        std::move(resolved), std::move(matches), std::move(emotional),
        std::move(paths), trace);
    return result;
}

std::int64_t Engine::persist(const AnalysisResult& result) {
    const std::string mj = meaningObjectToJson(result.meaning, /*indent*/ 0);
    const std::string tj = result.trace.toJson(/*indent*/ 0);
    return m_db->persistAnalysisTrace(
        result.meaning.rawInput,
        result.meaning.normalizedInput,
        mj,
        tj,
        result.meaning.overallConfidence,
        m_config.engineVersion);
}

} // namespace elle
