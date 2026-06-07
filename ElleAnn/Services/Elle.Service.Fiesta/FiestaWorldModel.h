#pragma once
#ifndef ELLE_FIESTA_WORLDMODEL_H
#define ELLE_FIESTA_WORLDMODEL_H

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../_Shared/json.hpp"

namespace Fiesta {

enum class EntityKind : uint8_t {
    UNKNOWN = 0,
    PLAYER  = 1,
    MOB     = 2,
    NPC     = 3,
};

inline const char* EntityKindName(EntityKind k) {
    switch (k) {
        case EntityKind::PLAYER: return "player";
        case EntityKind::MOB:    return "mob";
        case EntityKind::NPC:    return "npc";
        default:                 return "unknown";
    }
}

struct WorldEntity {
    uint16_t    handle     = 0;
    EntityKind  kind       = EntityKind::UNKNOWN;
    std::string name;
    uint16_t    mobId      = 0;
    uint64_t    firstSeen  = 0;
    uint64_t    lastSeen   = 0;
};

class WorldModel {
public:
    WorldModel() = default;

    void UpdateSelfBase(uint32_t chrRegNum, const std::string& charName) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_self.chrRegNum = chrRegNum;
        m_self.charName  = charName;
        m_self.lastUpdate = NowMs();
    }

    void UpdateSelfHandle(uint16_t handle) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_self.handle = handle;
        m_self.lastUpdate = NowMs();
    }

    void UpdateSelfPosition(uint32_t x, uint32_t y) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_self.posX = x;
        m_self.posY = y;
        m_self.lastUpdate = NowMs();
    }

    void SetLoginState(const std::string& stateName) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_self.stateName  = stateName;
        m_self.lastUpdate = NowMs();
    }

    void SetZone(const std::string& mapName) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_zone.mapName    = mapName;
        m_zone.enteredMs  = NowMs();
    }

    void ClearZone() {
        std::lock_guard<std::mutex> lk(m_mx);
        m_zone.mapName.clear();
        m_zone.enteredMs = 0;
        m_entities.clear();
    }

    void UpsertPlayer(uint16_t handle, const std::string& name) {
        std::lock_guard<std::mutex> lk(m_mx);
        auto& e = m_entities[handle];
        if (e.firstSeen == 0) e.firstSeen = NowMs();
        e.handle   = handle;
        e.kind     = EntityKind::PLAYER;
        if (!name.empty()) e.name = name;
        e.lastSeen = NowMs();
    }

    void UpsertMob(uint16_t handle, uint16_t mobId) {
        std::lock_guard<std::mutex> lk(m_mx);
        auto& e = m_entities[handle];
        if (e.firstSeen == 0) e.firstSeen = NowMs();
        e.handle   = handle;
        e.kind     = EntityKind::MOB;
        e.mobId    = mobId;
        e.lastSeen = NowMs();
    }

    void RemoveEntity(uint16_t handle) {
        std::lock_guard<std::mutex> lk(m_mx);
        m_entities.erase(handle);
    }

    size_t EntityCount() const {
        std::lock_guard<std::mutex> lk(m_mx);
        return m_entities.size();
    }

    nlohmann::json SnapshotJson() const {
        SelfChar selfCopy;
        Zone     zoneCopy;
        std::vector<WorldEntity> entitiesCopy;
        {
            std::lock_guard<std::mutex> lk(m_mx);
            selfCopy = m_self;
            zoneCopy = m_zone;
            entitiesCopy.reserve(m_entities.size());
            for (const auto& kv : m_entities) entitiesCopy.push_back(kv.second);
        }

        nlohmann::json ents = nlohmann::json::array();
        for (const auto& e : entitiesCopy) {
            nlohmann::json je = {
                {"handle",     e.handle},
                {"kind",       EntityKindName(e.kind)},
                {"first_seen", e.firstSeen},
                {"last_seen",  e.lastSeen},
            };
            if (!e.name.empty()) je["name"]   = e.name;
            if (e.mobId != 0)    je["mob_id"] = e.mobId;
            ents.push_back(std::move(je));
        }

        nlohmann::json self_j = {
            {"handle",      selfCopy.handle},
            {"chrregnum",   selfCopy.chrRegNum},
            {"char_name",   selfCopy.charName},
            {"state",       selfCopy.stateName},
            {"x",           selfCopy.posX},
            {"y",           selfCopy.posY},
            {"last_update", selfCopy.lastUpdate},
        };
        nlohmann::json zone_j = {
            {"map",        zoneCopy.mapName},
            {"entered_ms", zoneCopy.enteredMs},
        };

        return nlohmann::json{
            {"self",     std::move(self_j)},
            {"zone",     std::move(zone_j)},
            {"entities", std::move(ents)},
        };
    }

    std::string SnapshotString() const {
        return SnapshotJson().dump();
    }

private:
    struct SelfChar {
        uint16_t    handle     = 0;
        uint32_t    chrRegNum  = 0;
        std::string charName;
        std::string stateName  = "disconnected";
        uint32_t    posX       = 0;
        uint32_t    posY       = 0;
        uint64_t    lastUpdate = 0;
    };

    struct Zone {
        std::string mapName;
        uint64_t    enteredMs  = 0;
    };

    static uint64_t NowMs() {
        using namespace std::chrono;
        return (uint64_t)duration_cast<milliseconds>(
                   steady_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex                              m_mx;
    SelfChar                                         m_self;
    Zone                                             m_zone;
    std::unordered_map<uint16_t, WorldEntity>        m_entities;
};

}
#endif
