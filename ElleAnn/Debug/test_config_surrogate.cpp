#include <cassert>
#include <cstdio>
#include <string>

static std::string DecodeJSONStringLiteral(const std::string& src) {
    std::string result;
    size_t pos = 0;
    if (pos < src.size() && src[pos] == '"') pos++;
    while (pos < src.size() && src[pos] != '"') {
        if (src[pos] == '\\' && pos + 1 < src.size()) {
            pos++;
            if (src[pos] == 'u') {
                auto readHex4 = [&](size_t at, unsigned& out) -> bool {
                    if (at + 3 >= src.size()) return false;
                    out = 0;
                    for (int k = 0; k < 4; k++) {
                        char c = src[at + k];
                        unsigned d;
                        if      (c >= '0' && c <= '9') d = (unsigned)(c - '0');
                        else if (c >= 'a' && c <= 'f') d = 10u + (unsigned)(c - 'a');
                        else if (c >= 'A' && c <= 'F') d = 10u + (unsigned)(c - 'A');
                        else return false;
                        out = (out << 4) | d;
                    }
                    return true;
                };
                unsigned cp = 0;
                if (!readHex4(pos + 1, cp)) { pos++; continue; }
                pos += 4;
                if (cp >= 0xD800 && cp <= 0xDBFF) {
                    unsigned lo = 0;
                    if (pos + 6 < src.size() &&
                        src[pos + 1] == '\\' && src[pos + 2] == 'u' &&
                        readHex4(pos + 3, lo) &&
                        lo >= 0xDC00 && lo <= 0xDFFF) {
                        cp = 0x10000u + (((cp - 0xD800u) << 10) | (lo - 0xDC00u));
                        pos += 6;
                    } else {
                        cp = 0xFFFD;
                    }
                } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                    cp = 0xFFFD;
                }
                if (cp < 0x80) {
                    result += (char)cp;
                } else if (cp < 0x800) {
                    result += (char)(0xC0 | (cp >> 6));
                    result += (char)(0x80 | (cp & 0x3F));
                } else if (cp < 0x10000) {
                    result += (char)(0xE0 | (cp >> 12));
                    result += (char)(0x80 | ((cp >> 6) & 0x3F));
                    result += (char)(0x80 | (cp & 0x3F));
                } else {
                    result += (char)(0xF0 |  (cp >> 18));
                    result += (char)(0x80 | ((cp >> 12) & 0x3F));
                    result += (char)(0x80 | ((cp >>  6) & 0x3F));
                    result += (char)(0x80 |  (cp        & 0x3F));
                }
                pos++;
                continue;
            }

            result += src[pos];
        } else {
            result += src[pos];
        }
        pos++;
    }
    return result;
}

static int g_pass = 0, g_fail = 0;
#define EXPECT(cond, label) do { \
    if (cond) { ++g_pass; std::printf("  ok    %s\n", label); } \
    else      { ++g_fail; std::printf("  FAIL  %s\n", label); } \
} while (0)

int main() {

    {
        std::string got = DecodeJSONStringLiteral("\"\\u00e9\"");
        std::string want = "\xC3\xA9";
        EXPECT(got == want, "BMP \\u00e9 -> C3 A9 (é)");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"\\ue000\"");
        std::string want = "\xEE\x80\x80";
        EXPECT(got == want, "BMP \\ue000 -> EE 80 80");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"\\uD83D\\uDE00\"");
        std::string want = "\xF0\x9F\x98\x80";
        EXPECT(got == want, "surrogate pair \\uD83D\\uDE00 -> F0 9F 98 80 (😀)");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"hi \\uD83D\\uDE00!\"");
        std::string want = "hi \xF0\x9F\x98\x80!";
        EXPECT(got == want, "surrogate pair with surrounding ASCII");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"\\uD83D x\"");
        std::string want = "\xEF\xBF\xBD x";
        EXPECT(got == want, "unpaired high surrogate -> U+FFFD");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"\\uDE00\"");
        std::string want = "\xEF\xBF\xBD";
        EXPECT(got == want, "unpaired low surrogate -> U+FFFD");
    }

    {
        std::string got = DecodeJSONStringLiteral("\"\\uD83D\\u0041\"");
        std::string want = "\xEF\xBF\xBD" "A";
        EXPECT(got == want, "high surrogate then non-low \\u0041 -> FFFD + A");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
