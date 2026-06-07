#pragma once

#include "elle/prob/BeliefStore.hpp"
#include "elle/prob/Types.hpp"

#include <memory>
#include <string>

namespace elle { namespace prob {

struct BetaParams {
    double alpha = 2.0;
    double beta  = 2.0;

    [[nodiscard]] double mean()     const noexcept { return alpha / (alpha + beta); }
    [[nodiscard]] double variance() const noexcept {
        const double n = alpha + beta;
        return (alpha * beta) / (n * n * (n + 1.0));
    }
    [[nodiscard]] double stddev()   const noexcept;
};

enum class TrustSignal : std::uint8_t {
    CONFIRMED_ACCURATE   = 0,
    KEPT_PROMISE         = 1,
    CONSISTENT_WITH_HISTORY = 2,
    CORRECTION_NEEDED    = 3,
    CONTRADICTED         = 4,
    HOSTILE_FRAMING      = 5,
    IDENTITY_CONFIRMED   = 6,
};

class SpeakerTrustModel {
public:

    SpeakerTrustModel(std::shared_ptr<BeliefStore> store,
                      std::string                  speakerId);

    void initialize(BetaParams prior = {2.0, 2.0}, double halfLifeSecs = 86400.0);

    void recordSignal(TrustSignal signal, double strength = 1.0);

    [[nodiscard]] double trustMean()     const;

    [[nodiscard]] double trustStddev()   const;

    [[nodiscard]] BetaParams currentParams() const;

    [[nodiscard]] double sourceWeight() const;

    void reset();

    [[nodiscard]] const std::string& speakerId() const noexcept { return m_speakerId; }

private:

    static constexpr std::int64_t ALPHA_KEY = 0;
    static constexpr std::int64_t BETA_KEY  = 1;

    std::string                  m_speakerId;
    std::string                  m_domain;
    std::shared_ptr<BeliefStore> m_store;
};

} }
