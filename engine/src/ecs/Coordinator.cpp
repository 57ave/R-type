#include <engine/ecs/Coordinator.hpp>

namespace ECS {

void Coordinator::Init()
{
    // Create pointers to each manager
    mComponentManager = std::make_unique<ComponentManager>();
    mEntityManager = std::make_unique<EntityManager>();
    mSystemManager = std::make_unique<SystemManager>();
}

void Coordinator::Shutdown()
{
    // Shutdown all systems
    mSystemManager->ShutdownAll();
}

Entity Coordinator::CreateEntity()
{
    return mEntityManager->CreateEntity();
}

void Coordinator::DestroyEntity(Entity entity)
{
    mEntityManager->DestroyEntity(entity);
    mComponentManager->EntityDestroyed(entity);
    mSystemManager->EntityDestroyed(entity);
}

Signature Coordinator::GetEntitySignature(Entity entity) const
{
    return mEntityManager->GetSignature(entity);
}

std::uint32_t Coordinator::GetLivingEntityCount() const
{
    return mEntityManager->GetLivingEntityCount();
}

void Coordinator::SetNetworkId(Entity entity, NetworkId networkId)
{
    mEntityManager->SetNetworkId(entity, networkId);
}

NetworkId Coordinator::GetNetworkId(Entity entity) const
{
    return mEntityManager->GetNetworkId(entity);
}

bool Coordinator::HasNetworkId(Entity entity) const
{
    return mEntityManager->HasNetworkId(entity);
}

Entity Coordinator::GetEntityByNetworkId(NetworkId networkId) const
{
    return mEntityManager->GetEntityByNetworkId(networkId);
}

bool Coordinator::HasEntityForNetworkId(NetworkId networkId) const
{
    return mEntityManager->HasEntityForNetworkId(networkId);
}

} // namespace ECS
