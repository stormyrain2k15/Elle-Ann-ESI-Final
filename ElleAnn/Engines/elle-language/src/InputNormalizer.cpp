// ============================================================================
// Elle Engine -- InputNormalizer implementation
// File: src/InputNormalizer.cpp
// ============================================================================
#include "elle/InputNormalizer.hpp"
#include "elle/StringUtil.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>

namespace elle {

namespace {

// Treat ASCII alpha + apostrophe + digits as word characters. This deliberately
// keeps contractions intact: "I'm", "don't", "they're".
bool isWordChar(char c) noexcept {
    const unsigned char uc = static_cast<unsigned char>(c);
    return std::isalnum(uc) || c == '\'';
}

// Detect the canonical "emotional" punctuation run (..., !!, !!!, ?!, !?).
// Otherwise a single-char punctuation lexeme is returned.
Lexeme makePunctLexeme(const std::string& run, std::size_t startByte) {
    Lexeme L;
    L.originalSpan  = run;
    L.normalized    = run;
    L.startByte     = startByte;
    L.endByte       = startByte + run.size();
    L.isPunctuation = true;
    L.emotionalPunctuationIntensity =
        str::isEmotionalPunctuation(run) ? static_cast<int>(run.size()) : 0;
    return L;
}

} // namespace

NormalizedInput InputNormalizer::normalize(std::string_view raw, DebugTrace& trace) const {
    NormalizedInput out;
    out.lexemes.reserve(16);

    bool inQuote = false;
    std::size_t i = 0;
    const std::size_t N = raw.size();

    while (i < N) {
        const char c   = raw[i];
        const unsigned char uc = static_cast<unsigned char>(c);

        // Quote tracking (both single straight " and ' may occur; we only
        // toggle on " to avoid treating contraction apostrophes as quotes.)
        if (c == '"') {
            inQuote = !inQuote;
            out.containsQuoted = true;
            Lexeme L; L.originalSpan = "\""; L.normalized = "\"";
            L.startByte = i; L.endByte = i + 1; L.isPunctuation = true;
            L.isQuotedRegion = true;
            out.lexemes.push_back(L);
            ++i;
            continue;
        }

        if (std::isspace(uc)) {
            ++i;
            continue;
        }

        if (isWordChar(c)) {
            const std::size_t start = i;
            while (i < N && isWordChar(raw[i])) ++i;
            Lexeme L;
            L.originalSpan   = std::string(raw.substr(start, i - start));
            L.normalized     = str::toLowerAscii(L.originalSpan);
            L.startByte      = start;
            L.endByte        = i;
            L.isPunctuation  = false;
            L.isQuotedRegion = inQuote;
            out.lexemes.push_back(L);
            continue;
        }

        if (str::isAsciiPunctuation(c)) {
            // Coalesce consecutive runs so "?!", "...", "!!!" become one lexeme.
            const std::size_t start = i;
            while (i < N && str::isAsciiPunctuation(raw[i]) && raw[i] != '"') ++i;
            const std::string run(raw.substr(start, i - start));
            Lexeme L = makePunctLexeme(run, start);
            L.isQuotedRegion = inQuote;
            if (run.find('!') != std::string::npos) out.exclamationCount += static_cast<int>(std::count(run.begin(), run.end(), '!'));
            if (run.find('?') != std::string::npos) out.questionCount    += static_cast<int>(std::count(run.begin(), run.end(), '?'));
            if (run.find('.') != std::string::npos && run.size() >= 3 && run.find_first_not_of('.') == std::string::npos)
                ++out.ellipsisCount;
            out.lexemes.push_back(L);
            continue;
        }

        // Unknown byte: skip; preserves robustness on garbled input.
        ++i;
    }

    // End-of-sentence flags: look at the last non-quote punctuation lexeme.
    for (auto it = out.lexemes.rbegin(); it != out.lexemes.rend(); ++it) {
        if (!it->isPunctuation || it->originalSpan == "\"") continue;
        if (it->originalSpan.find('?') != std::string::npos) out.endsWithQuestion = true;
        if (it->originalSpan.find('!') != std::string::npos) out.endsWithExclaim  = true;
        break;
    }

    // Canonical normalized text: rejoin normalized word lexemes with spaces.
    std::string canon;
    for (const auto& lx : out.lexemes) {
        if (!lx.isPunctuation) {
            if (!canon.empty()) canon.push_back(' ');
            canon.append(lx.normalized);
        }
    }
    out.normalizedText = canon;

    nlohmann::json payload = {
        {"raw_length",      raw.size()},
        {"lexeme_count",    out.lexemes.size()},
        {"exclamations",    out.exclamationCount},
        {"questions",       out.questionCount},
        {"ellipses",        out.ellipsisCount},
        {"contains_quoted", out.containsQuoted},
        {"normalized",      out.normalizedText}
    };
    trace.logJson("InputNormalizer", "normalized",
                  "Lexeme stream produced from raw input.", std::move(payload));

    return out;
}

} // namespace elle
