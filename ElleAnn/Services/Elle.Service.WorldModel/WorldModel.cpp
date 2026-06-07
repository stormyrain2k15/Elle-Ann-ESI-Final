#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/json.hpp"
#include <vector>
#include <unordered_map>
#include <algorithm>

using json = nlohmann::json;

class WorldModel {
public:
    bool Initialize() {

        std::vector<ELLE_WORLD_ENTITY> rows;
        if (ElleDB::GetAllEntities(rows)) {
            m_entities = std::move(rows);
            ELLE_INFO("World model initialized (hydrated %zu entities)", m_entities.size());
        } else {
            ELLE_INFO("World model initialized (cold start, no entities)");
        }
        return true;
    }

    void UpdateEntity(const std::string& name, const std::string& type,
                      const std::string& observation) {
        for (auto& e : m_entities) {
            if (std::string(e.name) == name) {
                e.interaction_count++;
                e.last_interaction_ms = ELLE_MS_NOW();
                e.familiarity = std::min(1.0f, e.familiarity + 0.05f);

                std::string model(e.mental_model);
                model += "\n" + observation;
                if (model.size() > ELLE_MAX_MSG - 1) model = model.substr(model.size() - ELLE_MAX_MSG + 1);
                strncpy_s(e.mental_model, model.c_str(), ELLE_MAX_MSG - 1);

                ElleDB::StoreEntity(e);
                return;
            }
        }

        ELLE_WORLD_ENTITY entity = {};
        strncpy_s(entity.name, name.c_str(), ELLE_MAX_NAME - 1);
        strncpy_s(entity.type, type.c_str(), ELLE_MAX_TAG - 1);
        strncpy_s(entity.description, observation.c_str(), ELLE_MAX_MSG - 1);
        entity.familiarity = 0.1f;
        entity.sentiment = 0.0f;
        entity.trust = 0.5f;
        entity.interaction_count = 1;
        entity.last_interaction_ms = ELLE_MS_NOW();
        strncpy_s(entity.mental_model, observation.c_str(), ELLE_MAX_MSG - 1);

        m_entities.push_back(entity);
        ElleDB::StoreEntity(entity);
        ELLE_INFO("New entity: %s (%s)", name.c_str(), type.c_str());
    }

    void Tick() {
        float decay = (float)ElleConfig::Instance().GetFloat("world_model.familiarity_decay_rate", 0.001);
        for (auto& e : m_entities) {
            e.familiarity = std::max(0.0f, e.familiarity - decay);
        }
    }

    std::vector<ELLE_WORLD_ENTITY> Query(const std::vector<std::string>& names,
                                         const std::vector<std::string>& types,
                                         float minFamiliarity,
                                         size_t limit) const {
        std::vector<ELLE_WORLD_ENTITY> out;
        auto nameMatches = [&](const char* n) {
            if (names.empty()) return true;
            for (auto& q : names) if (q == n) return true;
            return false;
        };
        auto typeMatches = [&](const char* t) {
            if (types.empty()) return true;
            for (auto& q : types) if (q == t) return true;
            return false;
        };
        for (const auto& e : m_entities) {
            if (!nameMatches(e.name)) continue;
            if (!typeMatches(e.type)) continue;
            if (e.familiarity < minFamiliarity) continue;
            out.push_back(e);
        }

        std::sort(out.begin(), out.end(),
                  [](const ELLE_WORLD_ENTITY& a, const ELLE_WORLD_ENTITY& b) {
                      return a.last_interaction_ms > b.last_interaction_ms;
                  });
        if (limit > 0 && out.size() > limit) out.resize(limit);
        return out;
    }

private:
    std::vector<ELLE_WORLD_ENTITY> m_entities;
};

class ElleWorldModelService : public ElleServiceBase {
public:
    ElleWorldModelService()
        : ElleServiceBase(SVC_WORLD_MODEL, "ElleWorldModel",
                          "Elle-Ann World Model",
                          "Environmental awareness and theory of mind") {}

protected:
    bool OnStart() override {
        m_model.Initialize();
        SetTickInterval(60000);
        ELLE_INFO("World model service started");
        return true;
    }

    void OnStop() override { ELLE_INFO("World model service stopped"); }
    void OnTick() override { m_model.Tick(); }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_WORLD_STATE: {
                ELLE_WORLD_ENTITY entity;
                if (msg.GetPayload(entity)) {
                    m_model.UpdateEntity(entity.name, entity.type, entity.description);
                }
                break;
            }
            case IPC_WORLD_QUERY: {

                std::string bodyStr = msg.GetStringPayload();
                std::string requestId;
                std::vector<std::string> names, types;
                float  minFam = 0.0f;
                size_t limit  = 16;
                try {
                    json req = json::parse(bodyStr);
                    if (req.contains("request_id") && req["request_id"].is_string())
                        requestId = req["request_id"].get<std::string>();
                    if (req.contains("names") && req["names"].is_array())
                        for (auto& n : req["names"])
                            if (n.is_string()) names.push_back(n.get<std::string>());
                    if (req.contains("types") && req["types"].is_array())
                        for (auto& t : req["types"])
                            if (t.is_string()) types.push_back(t.get<std::string>());
                    if (req.contains("min_familiarity") && req["min_familiarity"].is_number())
                        minFam = req["min_familiarity"].get<float>();
                    if (req.contains("limit") && req["limit"].is_number_integer())
                        limit = (size_t)std::max<int64_t>(1,
                                    std::min<int64_t>(256,
                                        req["limit"].get<int64_t>()));
                } catch (const std::exception& e) {
                    ELLE_WARN("IPC_WORLD_QUERY malformed JSON from svc=%d: %s — returning empty result",
                              (int)sender, e.what());
                }

                auto hits = m_model.Query(names, types, minFam, limit);

                json resp;
                resp["request_id"] = requestId;
                resp["entities"]   = json::array();
                for (const auto& e : hits) {
                    json j;
                    j["name"]                = std::string(e.name);
                    j["type"]                = std::string(e.type);
                    j["description"]         = std::string(e.description);
                    j["familiarity"]         = e.familiarity;
                    j["sentiment"]           = e.sentiment;
                    j["trust"]               = e.trust;
                    j["interaction_count"]   = e.interaction_count;
                    j["last_interaction_ms"] = (uint64_t)e.last_interaction_ms;
                    j["mental_model"]        = std::string(e.mental_model);
                    resp["entities"].push_back(j);
                }

                auto out = ElleIPCMessage::Create(IPC_WORLD_RESPONSE,
                                                  SVC_WORLD_MODEL, sender);
                out.header.correlation_id = msg.header.correlation_id;
                out.SetStringPayload(resp.dump());
                GetIPCHub().Send(sender, out);
                break;
            }
            default: break;
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT };
    }

private:
    WorldModel m_model;
};

ELLE_SERVICE_MAIN(ElleWorldModelService)
