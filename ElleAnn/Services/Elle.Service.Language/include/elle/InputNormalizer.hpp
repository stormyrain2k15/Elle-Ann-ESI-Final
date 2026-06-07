#pragma once

#include "elle/Types.hpp"
#include "elle/DebugTrace.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace elle {

struct NormalizedInput {
    std::string            normalizedText;
    std::vector<Lexeme>    lexemes;
    int                    exclamationCount = 0;
    int                    questionCount    = 0;
    int                    ellipsisCount    = 0;
    bool                   endsWithQuestion = false;
    bool                   endsWithExclaim  = false;
    bool                   containsQuoted   = false;
};

class InputNormalizer {
public:
    NormalizedInput normalize(std::string_view rawInput, DebugTrace& trace) const;
};

}
