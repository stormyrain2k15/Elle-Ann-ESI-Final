// ============================================================================
// Elle Engine -- Debug / Trace
// File: include/elle/DebugTrace.hpp
//
// Every decision the engine makes -- which sense won, which lost, why a
// context frame matched, which edges the graph walker took -- is recorded
// here. Renderable to JSON for persistence.
// ============================================================================
#pragma once

#include "elle/Types.hpp"

#include <nlohmann/json_fwd.hpp>

#include <memory>
#include <string>
#include <vector>

namespace elle {

struct TraceEntry {
    std::string  layer;       // e.g. "PhraseScanner"
    std::string  event;       // e.g. "phrase_matched"
    std::string  detail;      // free-form explanation
    nlohmann::json* payload = nullptr;   // unused; we copy via setPayload
};

class DebugTrace {
public:
    DebugTrace();
    ~DebugTrace();

    DebugTrace(const DebugTrace&)            = delete;
    DebugTrace& operator=(const DebugTrace&) = delete;
    DebugTrace(DebugTrace&&) noexcept;
    DebugTrace& operator=(DebugTrace&&) noexcept;

    void log(std::string layer, std::string event, std::string detail);
    void logJson(std::string layer, std::string event, std::string detail,
                 nlohmann::json payload);

    [[nodiscard]] std::string toJson(int indent = 2) const;
    [[nodiscard]] std::string toHumanReadable() const;
    [[nodiscard]] std::size_t size() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace elle
