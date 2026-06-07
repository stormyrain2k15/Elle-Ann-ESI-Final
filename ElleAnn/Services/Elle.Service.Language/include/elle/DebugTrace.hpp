#pragma once

#include "elle/Types.hpp"

#include <nlohmann/json_fwd.hpp>

#include <memory>
#include <string>
#include <vector>

namespace elle {

struct TraceEntry {
    std::string  layer;
    std::string  event;
    std::string  detail;
    nlohmann::json* payload = nullptr;
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

}
