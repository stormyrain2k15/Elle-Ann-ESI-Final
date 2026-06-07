#ifndef ELLE_JSON_EXTRACT_H
#define ELLE_JSON_EXTRACT_H

#include "json.hpp"
#include <string>

namespace Elle {

enum class JsonExtractResult {
    Ok              = 0,
    NoBraceFound    = 1,
    Unbalanced      = 2,
    ParseFailed     = 3,
    FailClosed      = 4,
    RootNotObject   = 5
};

inline const char* JsonExtractResultName(JsonExtractResult r) {
    switch (r) {
        case JsonExtractResult::Ok:              return "Ok";
        case JsonExtractResult::NoBraceFound:    return "NoBraceFound";
        case JsonExtractResult::Unbalanced:      return "Unbalanced";
        case JsonExtractResult::ParseFailed:     return "ParseFailed";
        case JsonExtractResult::FailClosed:      return "FailClosed";
        case JsonExtractResult::RootNotObject:   return "RootNotObject";
    }
    return "?";
}

inline JsonExtractResult ExtractJsonObjectEx(const std::string& s, nlohmann::json& out) {
    if (s.empty()) return JsonExtractResult::NoBraceFound;
    constexpr int kMaxDepth = 1024;
    size_t i = 0;
    bool saw_any_brace     = false;
    bool saw_balanced      = false;
    bool saw_parse_failure = false;
    while (i < s.size()) {
        size_t start = s.find('{', i);
        if (start == std::string::npos) break;
        saw_any_brace = true;
        int depth = 0;
        bool inStr = false;
        size_t end = std::string::npos;
        bool aborted = false;
        for (size_t j = start; j < s.size(); j++) {
            unsigned char c = static_cast<unsigned char>(s[j]);
            if (inStr) {
                if (c == '\\') {
                    if (j + 1 >= s.size()) { aborted = true; break; }
                    char esc_char = s[j + 1];
                    if (esc_char == 'u') {
                        if (j + 5 >= s.size()) { aborted = true; break; }
                        j += 5;
                    } else {
                        j += 1;
                    }
                    continue;
                }
                if (c == '"') { inStr = false; continue; }
            } else {
                if (c == 0) { aborted = true; break; }
                if (c == '"') { inStr = true; continue; }
                if (c == '{') {
                    if (++depth > kMaxDepth) { aborted = true; break; }
                } else if (c == '}') {
                    depth--;
                    if (depth < 0) { aborted = true; break; }
                    if (depth == 0) { end = j; break; }
                }
            }
        }
        if (aborted) return JsonExtractResult::FailClosed;
        if (end == std::string::npos) { i = start + 1; continue; }
        saw_balanced = true;
        try {
            out = nlohmann::json::parse(s.substr(start, end - start + 1));
            if (out.is_object()) return JsonExtractResult::Ok;

            saw_parse_failure = true;
        } catch (const nlohmann::json::parse_error&) {

            saw_parse_failure = true;
        }
        i = start + 1;
    }
    if (!saw_any_brace)     return JsonExtractResult::NoBraceFound;
    if (saw_parse_failure)  return JsonExtractResult::ParseFailed;
    if (!saw_balanced)      return JsonExtractResult::Unbalanced;
    return JsonExtractResult::RootNotObject;
}

inline bool ExtractJsonObject(const std::string& s, nlohmann::json& out) {
    return ExtractJsonObjectEx(s, out) == JsonExtractResult::Ok;
}

}

#endif
