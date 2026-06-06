// ============================================================================
// Elle Engine -- Config implementation
// File: src/Config.cpp
// ============================================================================
#include "elle/Config.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace elle {

namespace {

template <class T>
void getIf(const nlohmann::json& j, const char* key, T& out) {
    if (j.contains(key) && !j[key].is_null()) {
        out = j[key].get<T>();
    }
}

EngineConfig parseJson(const nlohmann::json& j) {
    EngineConfig cfg = EngineConfig::defaults();

    if (j.contains("database")) {
        const auto& d = j["database"];
        getIf(d, "driver",                cfg.database.driver);
        getIf(d, "server",                cfg.database.server);
        getIf(d, "database",              cfg.database.database);
        getIf(d, "user",                  cfg.database.user);
        getIf(d, "password",              cfg.database.password);
        getIf(d, "trustedConnection",     cfg.database.trustedConnection);
        getIf(d, "encrypt",               cfg.database.encrypt);
        getIf(d, "trustServerCertificate",cfg.database.trustServerCert);
        getIf(d, "connectionTimeoutMs",   cfg.database.connectionTimeoutMs);
        getIf(d, "queryTimeoutMs",        cfg.database.queryTimeoutMs);
    }
    if (j.contains("cache")) {
        const auto& c = j["cache"];
        getIf(c, "enabled",              cfg.cache.enabled);
        getIf(c, "wordCacheSize",        cfg.cache.wordCacheSize);
        getIf(c, "formCacheSize",        cfg.cache.formCacheSize);
        getIf(c, "phraseCacheSize",      cfg.cache.phraseCacheSize);
        getIf(c, "senseCacheSize",       cfg.cache.senseCacheSize);
        getIf(c, "relationCacheSize",    cfg.cache.relationCacheSize);
        getIf(c, "contextFrameCacheSize",cfg.cache.contextFrameCacheSize);
        getIf(c, "partOfSpeechCacheSize",cfg.cache.partOfSpeechCacheSize);
        getIf(c, "graphNeighborhood",    cfg.cache.graphNeighborhood);
    }
    if (j.contains("weights")) {
        const auto& w = j["weights"];
        getIf(w, "contextFrameMatch",    cfg.weights.contextFrameMatch);
        getIf(w, "nearbyWordCooccur",    cfg.weights.nearbyWordCooccur);
        getIf(w, "senseExampleOverlap",  cfg.weights.senseExampleOverlap);
        getIf(w, "emotionalAlignment",   cfg.weights.emotionalAlignment);
        getIf(w, "frequency",            cfg.weights.frequency);
        getIf(w, "posCompatibility",     cfg.weights.posCompatibility);
        getIf(w, "posNegDrawAlignment",  cfg.weights.posNegDrawAlignment);
        getIf(w, "conversationHint",     cfg.weights.conversationHint);
    }
    if (j.contains("graph")) {
        const auto& g = j["graph"];
        getIf(g, "maxDepth",        cfg.graph.maxDepth);
        getIf(g, "maxNodesPerPath", cfg.graph.maxNodesPerPath);
        getIf(g, "maxTotalNodes",   cfg.graph.maxTotalNodes);
        getIf(g, "minEdgeStrength", cfg.graph.minEdgeStrength);
    }
    getIf(j, "engineVersion", cfg.engineVersion);
    return cfg;
}

} // namespace

EngineConfig EngineConfig::defaults() {
    return EngineConfig{};
}

EngineConfig EngineConfig::loadFromString(const std::string& text) {
    nlohmann::json j = nlohmann::json::parse(text, nullptr, /*allow_exceptions*/ true,
                                             /*ignore_comments*/ true);
    return parseJson(j);
}

EngineConfig EngineConfig::loadFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open config file: " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return loadFromString(ss.str());
}

} // namespace elle
