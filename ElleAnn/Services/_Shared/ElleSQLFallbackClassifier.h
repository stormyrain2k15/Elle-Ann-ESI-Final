#pragma once
#ifndef ELLE_SQL_FALLBACK_CLASSIFIER_H
#define ELLE_SQL_FALLBACK_CLASSIFIER_H

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace ElleSQLFallbackClassifier {

enum class Idempotency {
    Unknown,
    Yes,
    No,
};

inline std::string ToUpperAscii(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (c >= 'a' && c <= 'z') out.push_back((char)(c - 32));
        else out.push_back((char)c);
    }
    return out;
}

inline std::string LeadingTokens(std::string_view s, std::size_t maxTokens) {
    std::string up = ToUpperAscii(s);
    std::string out;
    std::size_t i = 0, n = up.size(), tok = 0;
    while (i < n && tok < maxTokens) {
        while (i < n && std::isspace((unsigned char)up[i])) ++i;
        if (i >= n) break;
        std::size_t start = i;
        while (i < n && !std::isspace((unsigned char)up[i])) ++i;
        if (!out.empty()) out.push_back(' ');
        out.append(up.data() + start, i - start);
        ++tok;
    }
    return out;
}

inline bool StartsWithToken(const std::string& head, std::string_view tok) {
    if (head.size() < tok.size()) return false;
    for (std::size_t i = 0; i < tok.size(); ++i) {
        if (head[i] != tok[i]) return false;
    }
    if (head.size() == tok.size()) return true;
    char next = head[tok.size()];
    return std::isspace((unsigned char)next) != 0;
}

inline Idempotency ClassifyExec(const std::string& sql) {
    std::string head = LeadingTokens(sql, 3);
    if (head.empty()) return Idempotency::Unknown;

    if (StartsWithToken(head, "MERGE"))   return Idempotency::Yes;
    if (StartsWithToken(head, "TRUNCATE")) return Idempotency::Yes;
    if (StartsWithToken(head, "SELECT"))  return Idempotency::Yes;
    if (StartsWithToken(head, "WITH"))    return Idempotency::Yes;

    if (StartsWithToken(head, "INSERT"))  return Idempotency::No;
    if (StartsWithToken(head, "UPDATE"))  return Idempotency::No;
    if (StartsWithToken(head, "DELETE"))  return Idempotency::No;

    if (head.find("IF NOT EXISTS") != std::string::npos) return Idempotency::Yes;
    if (head.find("IF EXISTS")     != std::string::npos) return Idempotency::Yes;

    return Idempotency::Unknown;
}

inline Idempotency ClassifyCallProc(const std::string& procName) {
    std::string up = ToUpperAscii(procName);
    auto dot = up.rfind('.');
    std::string base = (dot == std::string::npos) ? up : up.substr(dot + 1);

    if (base.rfind("USP_RECORD", 0)      == 0) return Idempotency::Yes;
    if (base.rfind("USP_UPSERT", 0)      == 0) return Idempotency::Yes;
    if (base.rfind("USP_SNAPSHOT", 0)    == 0) return Idempotency::Yes;
    if (base.rfind("USP_LOG", 0)         == 0) return Idempotency::Yes;
    if (base.rfind("USP_ENSURE", 0)      == 0) return Idempotency::Yes;
    if (base.rfind("USP_MARK", 0)        == 0) return Idempotency::Yes;
    if (base.rfind("USP_TOUCH", 0)       == 0) return Idempotency::Yes;
    if (base.rfind("USP_HEARTBEAT", 0)   == 0) return Idempotency::Yes;
    if (base.rfind("USP_BOND", 0)        == 0) return Idempotency::Yes;
    if (base.rfind("USP_INTUITION", 0)   == 0) return Idempotency::Yes;

    if (base.rfind("USP_DELETE", 0)      == 0) return Idempotency::No;
    if (base.rfind("USP_PURGE", 0)       == 0) return Idempotency::No;
    if (base.rfind("USP_INSERT", 0)      == 0) return Idempotency::No;
    if (base.rfind("USP_CREATE", 0)      == 0) return Idempotency::No;

    return Idempotency::Unknown;
}

inline const char* IdempotencyToString(Idempotency i) {
    switch (i) {
        case Idempotency::Yes:     return "Yes";
        case Idempotency::No:      return "No";
        case Idempotency::Unknown: return "Unknown";
    }
    return "Unknown";
}

inline Idempotency IdempotencyFromString(std::string_view s) {
    if (s == "Yes") return Idempotency::Yes;
    if (s == "No")  return Idempotency::No;
    return Idempotency::Unknown;
}

}

#endif
