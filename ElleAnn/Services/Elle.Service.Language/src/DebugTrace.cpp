#include "elle/DebugTrace.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <sstream>
#include <vector>

namespace elle {

struct DebugTrace::Impl {
    struct Entry {
        std::int64_t   monoNs;
        std::string    layer;
        std::string    event;
        std::string    detail;
        nlohmann::json payload;
    };
    std::vector<Entry>                                 entries;
    std::chrono::steady_clock::time_point              start = std::chrono::steady_clock::now();

    [[nodiscard]] std::int64_t now() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now() - start).count();
    }
};

DebugTrace::DebugTrace() : m_impl(std::make_unique<Impl>()) {}
DebugTrace::~DebugTrace() = default;
DebugTrace::DebugTrace(DebugTrace&&) noexcept            = default;
DebugTrace& DebugTrace::operator=(DebugTrace&&) noexcept = default;

void DebugTrace::log(std::string layer, std::string event, std::string detail) {
    m_impl->entries.push_back(Impl::Entry{
        m_impl->now(), std::move(layer), std::move(event), std::move(detail), nlohmann::json()
    });
}

void DebugTrace::logJson(std::string layer, std::string event, std::string detail,
                         nlohmann::json payload) {
    m_impl->entries.push_back(Impl::Entry{
        m_impl->now(), std::move(layer), std::move(event),
        std::move(detail), std::move(payload)
    });
}

std::string DebugTrace::toJson(int indent) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& e : m_impl->entries) {
        nlohmann::json obj = {
            {"t_ns",   e.monoNs},
            {"layer",  e.layer},
            {"event",  e.event},
            {"detail", e.detail}
        };
        if (!e.payload.is_null()) obj["payload"] = e.payload;
        arr.push_back(std::move(obj));
    }
    return arr.dump(indent);
}

std::string DebugTrace::toHumanReadable() const {
    std::ostringstream oss;
    for (const auto& e : m_impl->entries) {
        oss << "[" << e.layer << "::" << e.event << "] " << e.detail;
        if (!e.payload.is_null()) {
            oss << "  " << e.payload.dump();
        }
        oss << '\n';
    }
    return oss.str();
}

std::size_t DebugTrace::size() const { return m_impl->entries.size(); }

}
