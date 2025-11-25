#pragma once

#include "Types.hpp"
#include <array>
#include <queue>
#include <cassert>

namespace ECS {

class EntityManager {
public:
    EntityManager();
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    void SetSignature(Entity entity, Signature signature);
    Signature GetSignature(Entity entity) const;
    std::uint32_t GetLivingEntityCount() const;

    void SetNetworkId(Entity entity, NetworkId networkId);
    NetworkId GetNetworkId(Entity entity) const;
    bool HasNetworkId(Entity entity) const;
    Entity GetEntityByNetworkId(NetworkId networkId) const;
    bool HasEntityForNetworkId(NetworkId networkId) const;

private:
    std::queue<Entity> mAvailableEntities;
    std::array<Signature, MAX_ENTITIES> mSignatures;
    std::uint32_t mLivingEntityCount;
    
    std::unordered_map<Entity, NetworkId> mEntityToNetworkId;
    std::unordered_map<NetworkId, Entity> mNetworkIdToEntity;
};

} // namespace ECS
