#pragma once

#include "elle/prob/Types.hpp"
#include "elle/prob/SpeakerTrustModel.hpp"
#include "elle/Types.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace elleann { namespace prob {

nlohmann::json distributionToJson(const elle::prob::Distribution& d);

nlohmann::json weightsToJson(const elle::prob::WeightVector& w);
elle::prob::WeightVector weightsFromJson(const nlohmann::json& j);

const char* pragmaticActName(elle::prob::PragmaticAct a);

nlohmann::json resultToJson(const elle::prob::ProbabilityResult& r);

elle::ConversationContext convoFromJson(const nlohmann::json& j);

elle::prob::ProbabilityRequest requestFromJson(const nlohmann::json& j);

elle::prob::TrustSignal trustSignalFromString(const std::string& s);

std::unordered_map<std::int64_t, double> hormonalStateFromJson(const nlohmann::json& j);

} }
