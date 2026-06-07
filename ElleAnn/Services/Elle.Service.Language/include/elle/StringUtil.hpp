#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace elle::str {

[[nodiscard]] std::string toLowerAscii(std::string_view s);

[[nodiscard]] std::string trim(std::string_view s);

[[nodiscard]] std::vector<std::string> splitWhitespace(std::string_view s);

[[nodiscard]] std::string normalizeForLookup(std::string_view s);

[[nodiscard]] bool isAsciiPunctuation(char c) noexcept;
[[nodiscard]] bool isEmotionalPunctuation(std::string_view s) noexcept;

[[nodiscard]] std::wstring utf8ToWide(std::string_view s);
[[nodiscard]] std::string  wideToUtf8(std::wstring_view s);

}
