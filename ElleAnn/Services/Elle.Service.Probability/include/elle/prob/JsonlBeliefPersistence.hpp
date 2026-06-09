#pragma once

#include "elle/prob/BeliefPersistence.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace elle::prob {

class JsonlBeliefPersistence final : public IBeliefPersistence {
public:
    explicit JsonlBeliefPersistence(std::string path)
        : m_path(std::move(path)) {}

    JsonlBeliefPersistence(const JsonlBeliefPersistence&)            = delete;
    JsonlBeliefPersistence& operator=(const JsonlBeliefPersistence&) = delete;

    void upsertDomain(const std::string& domain,
                      const Distribution& prior,
                      double halfLifeSecs) override {
        std::ostringstream s;
        s << R"({"op":"upsertDomain","ts_ms":)" << nowMs()
          << R"(,"domain":")" << escape(domain)
          << R"(","half_life_secs":)" << halfLifeSecs
          << R"(,"prior":)" << distAsJson(prior) << "}";
        writeLine(s.str());
    }

    void replacePosterior(const std::string& domain,
                          const Distribution& posterior,
                          std::int64_t whenMs) override {
        std::ostringstream s;
        s << R"({"op":"replacePosterior","ts_ms":)" << whenMs
          << R"(,"domain":")" << escape(domain)
          << R"(","posterior":)" << distAsJson(posterior) << "}";
        writeLine(s.str());
    }

    void appendEvidence(const std::string& domain,
                        const Evidence& ev) override {
        std::ostringstream s;
        s << R"({"op":"appendEvidence","ts_ms":)" << nowMs()
          << R"(,"domain":")" << escape(domain)
          << R"(","kind":)" << static_cast<int>(ev.kind)
          << R"(,"hypothesis_id":)" << ev.hypothesisId
          << R"(,"likelihood_ratio":)" << ev.likelihoodRatio
          << R"(,"source_weight":)" << ev.sourceWeight
          << R"(,"reason":")" << escape(ev.reason) << R"("})";
        writeLine(s.str());
    }

    void auditUpdate(const std::string& domain,
                     const std::string& operation,
                     std::size_t evidenceCount,
                     double entropyBefore,
                     double entropyAfter,
                     std::int64_t mapHypothesisId,
                     double mapProbability,
                     const std::string& detail) override {
        std::ostringstream s;
        s << R"({"op":"audit","ts_ms":)" << nowMs()
          << R"(,"domain":")" << escape(domain)
          << R"(","operation":")" << escape(operation)
          << R"(","evidence_count":)" << evidenceCount
          << R"(,"entropy_before":)" << entropyBefore
          << R"(,"entropy_after":)"  << entropyAfter
          << R"(,"map_hypothesis_id":)" << mapHypothesisId
          << R"(,"map_probability":)" << mapProbability
          << R"(,"detail":")" << escape(detail) << R"("})";
        writeLine(s.str());
    }

    std::vector<PersistedBelief> loadAll() override {
        return {};
    }

    std::string path() const { return m_path; }

private:
    static std::int64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static std::string escape(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
                case '"':  o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\n': o << "\\n";  break;
                case '\r': o << "\\r";  break;
                case '\t': o << "\\t";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                          << static_cast<int>(static_cast<unsigned char>(c))
                          << std::dec << std::setfill(' ');
                    } else {
                        o << c;
                    }
            }
        }
        return o.str();
    }

    static std::string distAsJson(const Distribution& d) {
        std::ostringstream o;
        o << "{";
        bool first = true;
        for (const auto& [hyp, mass] : d.mass) {
            if (!first) o << ",";
            o << "\"" << hyp << "\":" << mass;
            first = false;
        }
        o << "}";
        return o.str();
    }

    void writeLine(const std::string& line) {
        std::lock_guard<std::mutex> lk(m_mutex);
        std::ofstream f(m_path, std::ios::app | std::ios::binary);
        if (!f) return;
        f << line << "\n";
    }

    std::string         m_path;
    mutable std::mutex  m_mutex;
};

inline std::shared_ptr<IBeliefPersistence>
makeJsonlBeliefPersistence(const std::string& path) {
    return std::make_shared<JsonlBeliefPersistence>(path);
}

}
