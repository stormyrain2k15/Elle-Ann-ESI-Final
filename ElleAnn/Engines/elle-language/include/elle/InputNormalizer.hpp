// ============================================================================
// Elle Engine -- Input Normalizer
// File: include/elle/InputNormalizer.hpp
// ============================================================================
#pragma once

#include "elle/Types.hpp"
#include "elle/DebugTrace.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace elle {

struct NormalizedInput {
    std::string            normalizedText;          // canonical form used downstream
    std::vector<Lexeme>    lexemes;                 // ordered, byte-aligned
    int                    exclamationCount = 0;
    int                    questionCount    = 0;
    int                    ellipsisCount    = 0;
    bool                   endsWithQuestion = false;
    bool                   endsWithExclaim  = false;
    bool                   containsQuoted   = false;
};

// Stateless and reusable.
class InputNormalizer {
public:
    NormalizedInput normalize(std::string_view rawInput, DebugTrace& trace) const;
};

} // namespace elle
