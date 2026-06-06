// ============================================================================
// Elle Engine -- StringUtil implementation
// File: src/StringUtil.cpp
// ============================================================================
#include "elle/StringUtil.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>

namespace elle::str {

std::string toLowerAscii(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

std::string trim(std::string_view s) {
    std::size_t lo = 0;
    std::size_t hi = s.size();
    while (lo < hi && std::isspace(static_cast<unsigned char>(s[lo]))) {
        ++lo;
    }
    while (hi > lo && std::isspace(static_cast<unsigned char>(s[hi - 1]))) {
        --hi;
    }
    return std::string(s.substr(lo, hi - lo));
}

bool isAsciiPunctuation(char c) noexcept {
    // Match the "preserve punctuation that affects meaning" rule but treat
    // apostrophe as part of the word so contractions are preserved.
    switch (c) {
        case '.': case ',': case '!': case '?':
        case ';': case ':': case '"': case '`':
        case '(': case ')': case '[': case ']':
        case '{': case '}':
            return true;
        default:
            return false;
    }
}

bool isEmotionalPunctuation(std::string_view s) noexcept {
    if (s.empty()) {
        return false;
    }
    // "...", "!!", "!!!", "?!", etc.
    bool seenBang  = false;
    bool seenQues  = false;
    bool seenDot   = false;
    int  dotCount  = 0;
    for (char c : s) {
        if (c == '!') {
            seenBang = true;
        } else if (c == '?') {
            seenQues = true;
        } else if (c == '.') {
            seenDot = true;
            ++dotCount;
        } else {
            return false;
        }
    }
    if (seenBang && (s.size() > 1)) return true;            // "!!", "!!!"
    if (seenQues && seenBang)       return true;            // "?!" / "!?"
    if (seenDot && dotCount >= 3)   return true;            // "..."
    return false;
}

std::vector<std::string> splitWhitespace(std::string_view s) {
    std::vector<std::string> out;
    std::string current;
    auto flush = [&]() {
        if (!current.empty()) {
            out.push_back(current);
            current.clear();
        }
    };
    for (char c : s) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) {
            flush();
        } else {
            current.push_back(c);
        }
    }
    flush();
    return out;
}

std::string normalizeForLookup(std::string_view s) {
    return toLowerAscii(trim(s));
}

// ---------------------------------------------------------------------------
// Minimal UTF-8 / UTF-16 conversion. We avoid <codecvt> (deprecated in C++17)
// and hand-roll the conversion. Sufficient for SQL Server NVARCHAR boundary.
// ---------------------------------------------------------------------------
std::wstring utf8ToWide(std::string_view s) {
    std::wstring out;
    out.reserve(s.size());
    std::size_t i = 0;
    while (i < s.size()) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        std::uint32_t cp = 0;
        std::size_t   adv = 1;
        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
            cp = ((c & 0x1F) << 6) |
                 (static_cast<unsigned char>(s[i + 1]) & 0x3F);
            adv = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
            cp = ((c & 0x0F) << 12) |
                 ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(s[i + 2]) & 0x3F);
            adv = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
            cp = ((c & 0x07) << 18) |
                 ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(s[i + 3]) & 0x3F);
            adv = 4;
        } else {
            cp = 0xFFFD;  // replacement character
        }
        if (cp <= 0xFFFF) {
            out.push_back(static_cast<wchar_t>(cp));
        } else {
            cp -= 0x10000;
            out.push_back(static_cast<wchar_t>(0xD800 | (cp >> 10)));
            out.push_back(static_cast<wchar_t>(0xDC00 | (cp & 0x3FF)));
        }
        i += adv;
    }
    return out;
}

std::string wideToUtf8(std::wstring_view s) {
    std::string out;
    out.reserve(s.size());
    std::size_t i = 0;
    while (i < s.size()) {
        std::uint32_t cp = static_cast<std::uint16_t>(s[i]);
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < s.size()) {
            const std::uint32_t lo = static_cast<std::uint16_t>(s[i + 1]);
            cp = 0x10000 + (((cp - 0xD800) << 10) | (lo - 0xDC00));
            ++i;
        }
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            out.push_back(static_cast<char>(0xE0 |  (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 |  (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 |  (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >>  6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 |  (cp & 0x3F)));
        }
        ++i;
    }
    return out;
}

} // namespace elle::str
