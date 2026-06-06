// ============================================================================
// Elle Engine -- Config
// File: include/elle/Config.hpp
//
// Holds DB connection details, cache sizes, and the scoring weights used by
// the SenseCandidateResolver. Loaded from a JSON file via nlohmann::json.
// ============================================================================
#pragma once

#include <cstddef>
#include <string>

namespace elle {

struct DatabaseConfig {
    std::string driver        = "ODBC Driver 18 for SQL Server";
    std::string server        = "localhost";
    std::string database      = "EllesLanguage";
    std::string user;                                 // empty -> Windows trusted
    std::string password;
    bool        trustedConnection   = true;
    bool        encrypt             = true;
    bool        trustServerCert     = false;
    int         connectionTimeoutMs = 5000;
    int         queryTimeoutMs      = 8000;
};

struct CacheConfig {
    std::size_t wordCacheSize        = 50000;
    std::size_t formCacheSize        = 50000;
    std::size_t phraseCacheSize      = 10000;
    std::size_t senseCacheSize       = 50000;
    std::size_t relationCacheSize    = 10000;
    std::size_t contextFrameCacheSize= 1024;
    std::size_t partOfSpeechCacheSize= 64;
    std::size_t graphNeighborhood    = 8192;
    bool        enabled              = true;
};

struct ScoringWeights {
    double contextFrameMatch   = 1.0;
    double nearbyWordCooccur   = 0.6;
    double senseExampleOverlap = 0.5;
    double emotionalAlignment  = 0.7;
    double frequency           = 0.3;
    double posCompatibility    = 0.4;
    double posNegDrawAlignment = 0.5;
    double conversationHint    = 0.8;
};

struct GraphConfig {
    int    maxDepth        = 3;
    int    maxNodesPerPath = 8;
    int    maxTotalNodes   = 64;
    double minEdgeStrength = 0.10;
};

struct EngineConfig {
    DatabaseConfig database;
    CacheConfig    cache;
    ScoringWeights weights;
    GraphConfig    graph;
    std::string    engineVersion = "0.1.0";

    // Load from a JSON config file. Throws std::runtime_error on parse error.
    [[nodiscard]] static EngineConfig loadFromFile(const std::string& path);
    // Load from a JSON string.
    [[nodiscard]] static EngineConfig loadFromString(const std::string& text);
    // Default in-process config (used by InMemoryAccessLayer tests).
    [[nodiscard]] static EngineConfig defaults();
};

} // namespace elle
