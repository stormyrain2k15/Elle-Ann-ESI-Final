#pragma once

#include "elle/DebugTrace.hpp"
#include "elle/Types.hpp"

#include <string>
#include <vector>

namespace elle {

class MeaningObjectBuilder {
public:
    MeaningObject build(std::string rawInput,
                        std::string normalizedInput,
                        IntegerSequence sequence,
                        std::vector<ResolvedSense> resolved,
                        std::vector<ContextFrameMatch> contextMatches,
                        EmotionalProfile emotional,
                        std::vector<ConceptPath> conceptPaths,
                        const DebugTrace& trace) const;
};

std::string meaningObjectToJson(const MeaningObject& m, int indent = 2);

}
