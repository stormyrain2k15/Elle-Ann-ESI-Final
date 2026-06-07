#include "elle/prob/BayesianUpdater.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace elle { namespace prob {

namespace {

constexpr double LOG_LR_CLIP = 10.0;

double safeLogLR(double lr) noexcept {
    if (lr <= 0.0) lr = std::numeric_limits<double>::epsilon();
    const double raw = std::log(lr);
    return std::clamp(raw, -LOG_LR_CLIP, LOG_LR_CLIP);
}

}

void BayesianUpdater::applyLogEvidence(
    std::unordered_map<std::int64_t, double>& logMass,
    std::int64_t hypothesisId,
    double       likelihoodRatio,
    double       sourceWeight)
{

    const double logLR = safeLogLR(likelihoodRatio) * sourceWeight;

    auto it = logMass.find(hypothesisId);
    if (it != logMass.end()) {
        it->second += logLR;
    }

}

void BayesianUpdater::update(Belief& belief, const std::vector<Evidence>& evidence) const {
    if (evidence.empty()) return;

    std::unordered_map<std::int64_t, double> logMass;
    logMass.reserve(belief.posterior.mass.size());

    for (const auto& [k, v] : belief.posterior.mass) {
        const double safe = std::max(v, std::numeric_limits<double>::epsilon());
        logMass[k] = std::log(safe);
    }

    for (const auto& ev : evidence) {
        applyLogEvidence(logMass, ev.hypothesisId,
                         ev.likelihoodRatio, ev.sourceWeight);
        belief.evidenceLog.push_back(ev);
    }

    double maxLog = -std::numeric_limits<double>::infinity();
    for (const auto& [k, lv] : logMass) {
        if (lv > maxLog) maxLog = lv;
    }

    double sumExp = 0.0;
    std::unordered_map<std::int64_t, double> newMass;
    newMass.reserve(logMass.size());
    for (const auto& [k, lv] : logMass) {
        const double e = std::exp(lv - maxLog);
        newMass[k] = e;
        sumExp += e;
    }

    if (sumExp > std::numeric_limits<double>::epsilon()) {
        for (auto& [k, v] : newMass) v /= sumExp;
    } else {

        const double u = 1.0 / static_cast<double>(newMass.size());
        for (auto& [k, v] : newMass) v = u;
    }

    belief.posterior.mass = std::move(newMass);
    belief.lastUpdated    = now();
}

void BayesianUpdater::update(Belief& belief, const Evidence& ev) const {
    update(belief, std::vector<Evidence>{ev});
}

void BayesianUpdater::reset(Belief& belief) {
    belief.posterior    = belief.prior;
    belief.evidenceLog.clear();
    belief.lastUpdated  = now();
}

Distribution BayesianUpdater::uniformPrior(
    const std::vector<std::int64_t>& hypotheses)
{
    Distribution d;
    if (hypotheses.empty()) return d;
    const double u = 1.0 / static_cast<double>(hypotheses.size());
    for (auto h : hypotheses) d.mass[h] = u;
    return d;
}

Distribution BayesianUpdater::empiricalPrior(
    const std::unordered_map<std::int64_t, double>& counts)
{
    Distribution d;
    double total = 0.0;
    for (const auto& [k, v] : counts) {
        if (v > 0.0) { d.mass[k] = v; total += v; }
    }
    if (total > std::numeric_limits<double>::epsilon()) {
        for (auto& [k, v] : d.mass) v /= total;
    }
    return d;
}

double BayesianUpdater::kl(const Distribution& p, const Distribution& q) noexcept {
    double divergence = 0.0;
    for (const auto& [k, pv] : p.mass) {
        if (pv < std::numeric_limits<double>::epsilon()) continue;
        const double qv = q.p(k);
        if (qv < std::numeric_limits<double>::epsilon()) {
            return std::numeric_limits<double>::infinity();
        }
        divergence += pv * std::log(pv / qv);
    }
    return divergence;
}

double BayesianUpdater::jeffreys(const Distribution& p, const Distribution& q) noexcept {
    const double kl_pq = kl(p, q);
    const double kl_qp = kl(q, p);
    if (std::isinf(kl_pq) || std::isinf(kl_qp)) {
        return std::numeric_limits<double>::infinity();
    }
    return (kl_pq + kl_qp) * 0.5;
}

} }
