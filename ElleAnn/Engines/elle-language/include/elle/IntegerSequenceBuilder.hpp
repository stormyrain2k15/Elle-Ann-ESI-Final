// ============================================================================
// Elle Engine -- Integer Sequence Builder
// File: include/elle/IntegerSequenceBuilder.hpp
//
// Consumes the lexeme stream and the phrase matches and produces the ordered
// integer-backed IntegerSequence the rest of the engine operates on.
// ============================================================================
#pragma once

#include "elle/DebugTrace.hpp"
#include "elle/InputNormalizer.hpp"
#include "elle/PhraseScanner.hpp"
#include "elle/SqlAccessLayer.hpp"
#include "elle/Types.hpp"
#include "elle/WordFormResolver.hpp"

#include <vector>

namespace elle {

class IntegerSequenceBuilder {
public:
    IntegerSequenceBuilder(ISqlAccessLayer& db,
                           WordFormResolver& resolver);

    IntegerSequence build(const NormalizedInput& input,
                          const std::vector<PhraseMatch>& phraseMatches,
                          DebugTrace& trace);

private:
    ISqlAccessLayer&   m_db;
    WordFormResolver&  m_resolver;
};

} // namespace elle
