// ============================================================================
// Elle Engine -- IntegerSequenceBuilder implementation
// File: src/IntegerSequenceBuilder.cpp
//
// Build the integer-backed sequence:
//   * phrase spans collapse into a single WordUnit carrying PhraseID +
//     PhraseSense candidates,
//   * remaining word lexemes resolve to WordID / WordFormID and pull their
//     SenseID candidates from the DB,
//   * punctuation lexemes do not produce WordUnits but their counts feed
//     IntegerSequence fields (already set by InputNormalizer).
// ============================================================================
#include "elle/IntegerSequenceBuilder.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>

namespace elle {

IntegerSequenceBuilder::IntegerSequenceBuilder(ISqlAccessLayer& db,
                                               WordFormResolver& resolver)
    : m_db(db), m_resolver(resolver) {}

IntegerSequence IntegerSequenceBuilder::build(const NormalizedInput& input,
                                              const std::vector<PhraseMatch>& matches,
                                              DebugTrace& trace) {
    IntegerSequence seq;
    seq.exclamationCount  = input.exclamationCount;
    seq.questionCount     = input.questionCount;
    seq.ellipsisCount     = input.ellipsisCount;
    seq.endsWithQuestion  = input.endsWithQuestion;
    seq.endsWithExclaim   = input.endsWithExclaim;
    seq.containsQuoted    = input.containsQuoted;

    // Walk lexemes left-to-right; if the index falls inside a phrase match,
    // emit one PhraseUnit and skip ahead.
    std::vector<bool> claimed(input.lexemes.size(), false);
    for (const auto& m : matches) {
        for (std::size_t i = m.startLexemeIdx; i < m.endLexemeIdx && i < claimed.size(); ++i) {
            claimed[i] = true;
        }
    }

    auto matchAt = [&](std::size_t i) -> const PhraseMatch* {
        for (const auto& m : matches) {
            if (m.startLexemeIdx == i) return &m;
        }
        return nullptr;
    };

    std::size_t position = 0;
    for (std::size_t i = 0; i < input.lexemes.size(); ) {
        const Lexeme& lx = input.lexemes[i];
        if (lx.isPunctuation) { ++i; continue; }

        if (const PhraseMatch* m = matchAt(i)) {
            WordUnit u;
            u.positionInSentence = position++;
            u.phraseId           = m->phraseId;
            u.phraseSenseCandidates = m->phraseSenseCandidates;
            u.normalized         = m->normalizedForm;
            for (std::size_t k = m->startLexemeIdx; k < m->endLexemeIdx; ++k) {
                u.lexemes.push_back(input.lexemes[k]);
                if (!input.lexemes[k].isPunctuation) {
                    if (!u.originalSpan.empty()) u.originalSpan.push_back(' ');
                    u.originalSpan.append(input.lexemes[k].originalSpan);
                }
            }
            seq.units.push_back(std::move(u));
            i = m->endLexemeIdx;
            continue;
        }

        if (claimed[i]) { ++i; continue; }

        // Plain word path -- resolve and load sense candidates.
        WordUnit u;
        u.positionInSentence = position++;
        u.lexemes.push_back(lx);
        u.originalSpan = lx.originalSpan;
        u.normalized   = lx.normalized;

        const auto resolved = m_resolver.resolve(lx.normalized, trace);
        u.wordId     = resolved.wordId;
        u.wordFormId = resolved.wordFormId;
        u.isUnknown  = resolved.isUnknown;

        if (u.wordId) {
            const auto senses = m_db.getSensesForWord(*u.wordId);
            u.senseCandidates.reserve(senses.size());
            for (const auto& s : senses) u.senseCandidates.push_back(s.senseId);
        }
        seq.units.push_back(std::move(u));
        ++i;
    }

    nlohmann::json payload = {
        {"unit_count",        seq.units.size()},
        {"phrase_units",      std::count_if(seq.units.begin(), seq.units.end(),
                                             [](const WordUnit& u){ return u.phraseId.has_value(); })},
        {"unknown_units",     std::count_if(seq.units.begin(), seq.units.end(),
                                             [](const WordUnit& u){ return u.isUnknown; })}
    };
    trace.logJson("IntegerSequenceBuilder", "sequence_built",
                  "Integer-backed sequence produced.", std::move(payload));

    return seq;
}

} // namespace elle
