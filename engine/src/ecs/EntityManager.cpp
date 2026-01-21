#include <ecs/EntityManager.hpp>
#include <stdexcept>

namespace ECS {

EntityManager::EntityManager() : mLivingEntityCount(0) {
    // Initialize the queue with all possible entity IDs
    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
        mAvailableEntities.push(entity);
    }
}

Entity EntityManager::CreateEntity() {
    if (mLivingEntityCount >= MAX_ENTITIES) {
        throw std::runtime_error("Too many entities in existence.");
    }

    // Take an ID from the front of the queue
    Entity id = mAvailableEntities.front();
    mAvailableEntities.pop();
    ++mLivingEntityCount;

    return id;
}

void EntityManager::DestroyEntity(Entity entity) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");

    // Remove network ID mapping if it exists
    if (mEntityToNetworkId.find(entity) != mEntityToNetworkId.end()) {
        NetworkId networkId = mEntityToNetworkId[entity];
        mNetworkIdToEntity.erase(networkId);
        mEntityToNetworkId.erase(entity);
    }

    // Invalidate the destroyed entity's signature
    mSignatures[entity].reset();

    // Put the destroyed ID at the back of the queue
    mAvailableEntities.push(entity);
    --mLivingEntityCount;
}

void EntityManager::SetSignature(Entity entity, Signature signature) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");

    // Put this entity's signature into the array
    mSignatures[entity] = signature;
}

Signature EntityManager::GetSignature(Entity entity) const {
    assert(entity < MAX_ENTITIES && "Entity out of range.");

    // Get this entity's signature from the array
    return mSignatures[entity];
}

std::uint32_t EntityManager::GetLivingEntityCount() const {
    return mLivingEntityCount;
}

void EntityManager::SetNetworkId(Entity entity, NetworkId networkId) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    assert(networkId != INVALID_NETWORK_ID && "Cannot set invalid network ID.");

    if (mNetworkIdToEntity.find(networkId) != mNetworkIdToEntity.end()) {
        throw std::runtime_error("NetworkId already assigned to another entity.");
    }

    // Remove old mapping if exists
    if (mEntityToNetworkId.find(entity) != mEntityToNetworkId.end()) {
        NetworkId oldNetworkId = mEntityToNetworkId[entity];
        mNetworkIdToEntity.erase(oldNetworkId);
    }

    mEntityToNetworkId[entity] = networkId;
    mNetworkIdToEntity[networkId] = entity;
}

NetworkId EntityManager::GetNetworkId(Entity entity) const {
    assert(entity < MAX_ENTITIES && "Entity out of range.");

    auto it = mEntityToNetworkId.find(entity);
    if (it == mEntityToNetworkId.end()) {
        return INVALID_NETWORK_ID;
    }

    return it->second;
}

bool EntityManager::HasNetworkId(Entity entity) const {
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    return mEntityToNetworkId.find(entity) != mEntityToNetworkId.end();
}

Entity EntityManager::GetEntityByNetworkId(NetworkId networkId) const {
    auto it = mNetworkIdToEntity.find(networkId);
    if (it == mNetworkIdToEntity.end()) {
        throw std::runtime_error("NetworkId not found.");
    }

    return it->second;
}

bool EntityManager::HasEntityForNetworkId(NetworkId networkId) const {
    return mNetworkIdToEntity.find(networkId) != mNetworkIdToEntity.end();
}

}  // namespace ECS
