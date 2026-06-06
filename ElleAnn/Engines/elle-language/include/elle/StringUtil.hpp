// ============================================================================
// Elle Engine -- String utilities (UTF-8 in, UTF-16 at the ODBC boundary)
// File: include/elle/StringUtil.hpp
// ============================================================================
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace elle::str {

// Lowercase ASCII letters only. Unicode case-folding is intentionally NOT
// applied here -- collation is the database's job. This keeps the engine
// deterministic and avoids ICU as a dependency.
[[nodiscard]] std::string toLowerAscii(std::string_view s);

// Trim ASCII whitespace.
[[nodiscard]] std::string trim(std::string_view s);

// Split on whitespace, preserving punctuation as separate fragments.
[[nodiscard]] std::vector<std::string> splitWhitespace(std::string_view s);

// Reversible normalisation used for DB lookup: lowercase + trim.
[[nodiscard]] std::string normalizeForLookup(std::string_view s);

// Boolean helpers
[[nodiscard]] bool isAsciiPunctuation(char c) noexcept;
[[nodiscard]] bool isEmotionalPunctuation(std::string_view s) noexcept;  // "!", "?!", "!!!", "..."

// UTF-8 <-> UTF-16 (used only by the ODBC layer; safe ASCII subset path here).
[[nodiscard]] std::wstring utf8ToWide(std::string_view s);
[[nodiscard]] std::string  wideToUtf8(std::wstring_view s);

} // namespace elle::str
