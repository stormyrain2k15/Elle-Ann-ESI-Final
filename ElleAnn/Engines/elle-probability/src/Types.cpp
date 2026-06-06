// ============================================================================
// Elle Probability Engine -- Types implementation
// File: src/Types.cpp
// ============================================================================
#include "elle/prob/Types.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace elle { namespace prob {

// ---------------------------------------------------------------------------
// Distribution
// ---------------------------------------------------------------------------

void Distribution::normalize() {
    if (mass.empty()) return;

    double total = 0.0;
    for (auto& [k, v] : mass) {
        if (v < 0.0) v = 0.0;   // clamp negatives to zero
        total += v;
    }

    if (total < std::numeric_limits<double>::epsilon()) {
        // All zero: reset to uniform
        const double uniform = 1.0 / static_cast<double>(mass.size());
        for (auto& [k, v] : mass) v = uniform;
        return;
    }

    for (auto& [k, v] : mass) v /= total;
}

double Distribution::p(std::int64_t h) const noexcept {
    auto it = mass.find(h);
    return it == mass.end() ? 0.0 : it->second;
}

double Distribution::entropy() const noexcept {
    double h = 0.0;
    for (const auto& [k, v] : mass) {
        if (v > std::numeric_limits<double>::epsilon()) {
            h -= v * std::log2(v);
        }
    }
    return h;
}

std::int64_t Distribution::map() const noexcept {
    if (mass.empty()) return -1;
    auto it = std::max_element(mass.begin(), mass.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    return it->first;
}

std::int64_t Distribution::sample(double u01) const noexcept {
    if (mass.empty()) return -1;
    double cumulative = 0.0;
    std::int64_t lastKey = 0;
    for (const auto& [k, v] : mass) {
        cumulative += v;
        if (u01 < cumulative) return k;
        lastKey = k;
    }
    return lastKey;  // floating-point edge case: return last visited key
}

std::size_t Distribution::support() const noexcept {
    std::size_t n = 0;
    for (const auto& [k, v] : mass) {
        if (v > std::numeric_limits<double>::epsilon()) ++n;
    }
    return n;
}

// ---------------------------------------------------------------------------
// WeightVector
// ---------------------------------------------------------------------------

WeightVector WeightVector::hadamard(const WeightVector& rhs) const noexcept {
    WeightVector r;
    r.contextFrameMatch   = contextFrameMatch   * rhs.contextFrameMatch;
    r.nearbyWordCooccur   = nearbyWordCooccur   * rhs.nearbyWordCooccur;
    r.senseExampleOverlap = senseExampleOverlap * rhs.senseExampleOverlap;
    r.emotionalAlignment  = emotionalAlignment  * rhs.emotionalAlignment;
    r.frequency           = frequency           * rhs.frequency;
    r.posCompatibility    = posCompatibility    * rhs.posCompatibility;
    r.posNegDrawAlignment = posNegDrawAlignment * rhs.posNegDrawAlignment;
    r.conversationHint    = conversationHint    * rhs.conversationHint;
    return r;
}

double WeightVector::l1() const noexcept {
    return std::abs(contextFrameMatch)
         + std::abs(nearbyWordCooccur)
         + std::abs(senseExampleOverlap)
         + std::abs(emotionalAlignment)
         + std::abs(frequency)
         + std::abs(posCompatibility)
         + std::abs(posNegDrawAlignment)
         + std::abs(conversationHint);
}

WeightVector WeightVector::lerp(const WeightVector& target, double alpha) const noexcept {
    const double a = std::clamp(alpha, 0.0, 1.0);
    const double b = 1.0 - a;
    WeightVector r;
    r.contextFrameMatch   = b * contextFrameMatch   + a * target.contextFrameMatch;
    r.nearbyWordCooccur   = b * nearbyWordCooccur   + a * target.nearbyWordCooccur;
    r.senseExampleOverlap = b * senseExampleOverlap + a * target.senseExampleOverlap;
    r.emotionalAlignment  = b * emotionalAlignment  + a * target.emotionalAlignment;
    r.frequency           = b * frequency           + a * target.frequency;
    r.posCompatibility    = b * posCompatibility    + a * target.posCompatibility;
    r.posNegDrawAlignment = b * posNegDrawAlignment + a * target.posNegDrawAlignment;
    r.conversationHint    = b * conversationHint    + a * target.conversationHint;
    return r;
}

// ---------------------------------------------------------------------------
// Belief
// ---------------------------------------------------------------------------

void Belief::decayTowardPrior() {
    if (halfLifeSecs <= 0.0) return;
    const double elapsed = secondsSince(lastUpdated);
    if (elapsed <= 0.0) return;

    // Exponential decay: blend factor = 2^(-elapsed / halfLife)
    const double alpha = std::pow(2.0, -elapsed / halfLifeSecs);
    // alpha = 1.0 -> no decay (just updated); alpha = 0.0 -> full prior

    // Blend: posterior = alpha * posterior + (1 - alpha) * prior
    // First ensure both distributions cover the same keys.
    std::unordered_map<std::int64_t, double> blended;
    for (const auto& [k, pv] : prior.mass) {
        const double post = posterior.p(k);
        blended[k] = alpha * post + (1.0 - alpha) * pv;
    }
    for (const auto& [k, postv] : posterior.mass) {
        if (blended.find(k) == blended.end()) {
            blended[k] = alpha * postv;   // hypothesis not in prior decays away
        }
    }

    posterior.mass = std::move(blended);
    posterior.normalize();
    lastUpdated = now();
}

} } // namespace elle::prob
