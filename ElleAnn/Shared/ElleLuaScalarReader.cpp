#include "ElleLuaScalarReader.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

ElleLuaScalarReader::ElleLuaScalarReader(const std::string& path) : m_path(path) {
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::stringstream ss; ss << f.rdbuf();
    std::string raw = ss.str();

    std::string s; s.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ) {
        if (i + 3 < raw.size() && raw[i] == '-' && raw[i+1] == '-'
                                && raw[i+2] == '[' && raw[i+3] == '[') {
            size_t end = raw.find("]]", i + 4);
            i = (end == std::string::npos) ? raw.size() : end + 2;
            continue;
        }
        if (i + 1 < raw.size() && raw[i] == '-' && raw[i+1] == '-') {

            size_t end = raw.find('\n', i);
            i = (end == std::string::npos) ? raw.size() : end;
            continue;
        }
        s.push_back(raw[i++]);
    }
    m_text = std::move(s);
    m_loaded = true;
}

std::string ElleLuaScalarReader::FindRhs(const std::string& dottedKey) const {
    if (!m_loaded || dottedKey.empty()) return "";
    std::string lastRhs;

    size_t lineStart = 0;
    while (lineStart <= m_text.size()) {
        size_t lineEnd = m_text.find('\n', lineStart);
        if (lineEnd == std::string::npos) lineEnd = m_text.size();
        std::string line = m_text.substr(lineStart, lineEnd - lineStart);
        lineStart = lineEnd + 1;

        size_t p = 0;
        while (p < line.size() && std::isspace((unsigned char)line[p])) ++p;
        if (line.compare(p, 6, "local ") == 0) p += 6;

        if (line.compare(p, dottedKey.size(), dottedKey) != 0) continue;
        size_t q = p + dottedKey.size();

        if (q < line.size() &&
            (std::isalnum((unsigned char)line[q]) || line[q] == '_')) continue;

        while (q < line.size() && std::isspace((unsigned char)line[q])) ++q;
        if (q >= line.size() || line[q] != '=') continue;
        ++q;
        while (q < line.size() && std::isspace((unsigned char)line[q])) ++q;

        std::string rhs = line.substr(q);
        while (!rhs.empty() &&
               (rhs.back() == ' ' || rhs.back() == '\t' ||
                rhs.back() == ',' || rhs.back() == '\r')) {
            rhs.pop_back();
        }
        lastRhs = rhs;
    }
    return lastRhs;
}

std::string ElleLuaScalarReader::GetString(const std::string& key,
                                           const std::string& def) const {
    std::string rhs = FindRhs(key);
    if (rhs.size() < 2) return def;
    if ((rhs.front() == '"' && rhs.back() == '"') ||
        (rhs.front() == '\'' && rhs.back() == '\'')) {
        return rhs.substr(1, rhs.size() - 2);
    }
    return def;
}

int64_t ElleLuaScalarReader::GetInt(const std::string& key, int64_t def) const {
    std::string rhs = FindRhs(key);
    if (rhs.empty()) return def;

    char* end = nullptr;
    int64_t v = std::strtoll(rhs.c_str(), &end, 0);
    if (end == rhs.c_str()) return def;
    while (end && *end && std::isspace((unsigned char)*end)) ++end;
    if (end && *end != '\0') return def;
    return v;
}

double ElleLuaScalarReader::GetDouble(const std::string& key, double def) const {
    std::string rhs = FindRhs(key);
    if (rhs.empty()) return def;
    char* end = nullptr;
    double v = std::strtod(rhs.c_str(), &end);
    if (end == rhs.c_str()) return def;
    while (end && *end && std::isspace((unsigned char)*end)) ++end;
    if (end && *end != '\0') return def;
    return v;
}

bool ElleLuaScalarReader::GetBool(const std::string& key, bool def) const {
    std::string rhs = FindRhs(key);
    if (rhs == "true")  return true;
    if (rhs == "false") return false;
    return def;
}
