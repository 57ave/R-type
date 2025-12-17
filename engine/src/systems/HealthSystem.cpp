#include <systems/HealthSystem.hpp>
#include <components/Health.hpp>
#include <ecs/Coordinator.hpp>
#include <vector>

HealthSystem::HealthSystem(ECS::Coordinator* coordinator)
    : coordinator_(coordinator)
{
}

void HealthSystem::Init()
{
}

void HealthSystem::Shutdown()
{
}

void HealthSystem::Update(float dt)
{
    if (!coordinator_) return;

    std::vector<ECS::Entity> toDie;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Health>(entity))
            continue;

        auto& health = coordinator_->GetComponent<Health>(entity);

        // Check if dead
        if (health.current <= 0 && !health.isDead) {
            health.isDead = true;
            toDie.push_back(entity);
        }
    }

    // Handle deaths
    for (auto entity : toDie) {
        HandleDeath(entity);
    }
}

void HealthSystem::HandleDeath(ECS::Entity entity)
{
    if (!coordinator_->HasComponent<Health>(entity))
        return;

    auto& health = coordinator_->GetComponent<Health>(entity);

    // TODO: Spawn death effect if specified
    // if (!health.deathEffect.empty()) {
    //     SpawnEffect(entity, health.deathEffect);
    // }

    // Destroy entity if configured
    if (health.destroyOnDeath) {
        coordinator_->DestroyEntity(entity);
    }
}
