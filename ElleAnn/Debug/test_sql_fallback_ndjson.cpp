#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>

static void AppendJsonEscaped(std::string& out, const std::string& in) {
    out.push_back('"');
    for (unsigned char c : in) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
                    out += buf;
                } else {
                    out.push_back((char)c);
                }
        }
    }
    out.push_back('"');
}

static bool ParseJsonString(const std::string& src, size_t& pos, std::string& out) {
    if (pos >= src.size() || src[pos] != '"') return false;
    ++pos;
    out.clear();
    while (pos < src.size() && src[pos] != '"') {
        char c = src[pos++];
        if (c == '\\' && pos < src.size()) {
            char e = src[pos++];
            switch (e) {
                case '"':  out.push_back('"');  break;
                case '\\': out.push_back('\\'); break;
                case '/':  out.push_back('/');  break;
                case 'n':  out.push_back('\n'); break;
                case 'r':  out.push_back('\r'); break;
                case 't':  out.push_back('\t'); break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'u': {
                    if (pos + 4 > src.size()) return false;
                    unsigned cp = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = src[pos++];
                        unsigned d;
                        if (h >= '0' && h <= '9') d = (unsigned)(h - '0');
                        else if (h >= 'a' && h <= 'f') d = 10u + (unsigned)(h - 'a');
                        else if (h >= 'A' && h <= 'F') d = 10u + (unsigned)(h - 'A');
                        else return false;
                        cp = (cp << 4) | d;
                    }
                    if (cp < 0x80) out.push_back((char)cp);
                    else if (cp < 0x800) {
                        out.push_back((char)(0xC0 | (cp >> 6)));
                        out.push_back((char)(0x80 | (cp & 0x3F)));
                    } else {
                        out.push_back((char)(0xE0 | (cp >> 12)));
                        out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                        out.push_back((char)(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default: out.push_back(e); break;
            }
        } else {
            out.push_back(c);
        }
    }
    if (pos >= src.size()) return false;
    ++pos;
    return true;
}

struct Line {
    uint64_t    ts;
    std::string kind;
    std::string sql;
    std::vector<std::string> params;
};

static std::string Encode(const Line& l) {
    std::string out = "{\"ts\":";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%llu", (unsigned long long)l.ts);
    out += buf;
    out += ",\"kind\":";
    AppendJsonEscaped(out, l.kind);
    out += ",\"sql\":";
    AppendJsonEscaped(out, l.sql);
    out += ",\"params\":[";
    for (size_t i = 0; i < l.params.size(); ++i) {
        if (i) out += ",";
        AppendJsonEscaped(out, l.params[i]);
    }
    out += "]}";
    return out;
}

static bool Decode(const std::string& src, Line& out) {
    size_t p = src.find("\"kind\":");
    if (p == std::string::npos) return false;
    p += 7;
    while (p < src.size() && src[p] != '"') ++p;
    if (!ParseJsonString(src, p, out.kind)) return false;

    p = src.find("\"sql\":");
    if (p == std::string::npos) return false;
    p += 6;
    while (p < src.size() && src[p] != '"') ++p;
    if (!ParseJsonString(src, p, out.sql)) return false;

    out.params.clear();
    p = src.find("\"params\":[");
    if (p != std::string::npos) {
        p += 10;
        while (p < src.size() && src[p] != ']') {
            while (p < src.size() && src[p] != '"' && src[p] != ']') ++p;
            if (p >= src.size() || src[p] == ']') break;
            std::string s;
            if (!ParseJsonString(src, p, s)) return false;
            out.params.push_back(std::move(s));
            while (p < src.size() && (src[p] == ',' || src[p] == ' ')) ++p;
        }
    }
    return true;
}

static int g_fails = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::fprintf(stderr, "FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__); g_fails++; } \
} while(0)

int main() {

    {
        Line in { 1739000000000ULL, "Exec", "UPDATE x SET y=1", {} };
        Line out;
        CHECK(Decode(Encode(in), out), "simple encode/decode");
        CHECK(out.kind == "Exec", "simple kind");
        CHECK(out.sql  == "UPDATE x SET y=1", "simple sql");
        CHECK(out.params.empty(), "simple params");
    }

    {
        std::string tricky = "he said \"hi\"\nand then\\backslash\ttab\r\b\f end";
        Line in { 1ULL, "QueryParams", tricky, {tricky, "plain"} };
        Line out;
        CHECK(Decode(Encode(in), out), "tricky encode/decode");
        CHECK(out.sql == tricky, "tricky sql roundtrip");
        CHECK(out.params.size() == 2, "tricky param count");
        CHECK(out.params[0] == tricky, "tricky param[0]");
        CHECK(out.params[1] == "plain", "tricky param[1]");
    }

    {
        std::string ctrl;
        ctrl.push_back('\x01');
        ctrl.push_back('\x1F');
        ctrl += "payload";
        Line in { 42ULL, "CallProc", "usp_test", {ctrl} };
        Line out;
        CHECK(Decode(Encode(in), out), "ctrl encode/decode");
        CHECK(out.sql == "usp_test", "ctrl sql");
        CHECK(out.params.size() == 1, "ctrl params count");
        CHECK(out.params[0] == ctrl, "ctrl param roundtrip");
    }

    {
        std::string a = Encode({1ULL, "Exec", "A", {}});
        std::string b = Encode({2ULL, "Exec", "B", {}});
        std::string file = a + "\n" + b + "\n";
        std::vector<std::string> lines;
        std::string cur;
        for (char c : file) {
            if (c == '\n') { if (!cur.empty()) lines.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        CHECK(lines.size() == 2, "line count");
        Line la, lb;
        CHECK(Decode(lines[0], la) && la.sql == "A", "line 0");
        CHECK(Decode(lines[1], lb) && lb.sql == "B", "line 1");
    }

    {
        Line in { 7ULL, "Exec", "TRUNCATE TABLE t", {} };
        Line out;
        std::string enc = Encode(in);
        CHECK(enc.find("\"params\":[]") != std::string::npos, "empty params encoded");
        CHECK(Decode(enc, out), "empty params decoded");
        CHECK(out.params.empty(), "empty params empty");
    }

    {
        std::string bad = "{\"ts\":1,\"sql\":\"X\",\"params\":[]}";
        Line out;
        CHECK(!Decode(bad, out), "missing kind rejected");
    }

    {
        Line in { 8ULL, "Exec", "SELECT 1\n  FROM dual", {} };
        Line out;
        std::string enc = Encode(in);

        CHECK(enc.find('\n') == std::string::npos, "no raw newline in encoded");
        CHECK(Decode(enc, out), "newline-sql decode");
        CHECK(out.sql == "SELECT 1\n  FROM dual", "newline-sql roundtrip");
    }

    if (g_fails == 0) std::printf("ALL TESTS PASS\n");
    else              std::printf("%d test(s) FAILED\n", g_fails);
    return g_fails == 0 ? 0 : 1;
}
