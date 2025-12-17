#include <systems/WeaponSystem.hpp>
#include <components/Weapon.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Tag.hpp>
#include <ecs/Coordinator.hpp>

WeaponSystem::WeaponSystem()
    : coordinator_(nullptr)
{
}

void WeaponSystem::Init()
{
}

void WeaponSystem::Shutdown()
{
}

void WeaponSystem::Update(float dt)
{
    if (!coordinator_) return;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Weapon>(entity))
            continue;

        auto& weapon = coordinator_->GetComponent<Weapon>(entity);

        weapon.lastFireTime += dt;

        // Update charge time if charging
        if (weapon.isCharging && weapon.supportsCharge) {
            weapon.chargeTime += dt;
            
            // Clamp to max
            if (weapon.chargeTime > weapon.maxChargeTime) {
                weapon.chargeTime = weapon.maxChargeTime;
            }
        }

        // Note: Actual firing is triggered by InputSystem or AI
        // This system just manages cooldowns and charge state
    }
}

void WeaponSystem::CreateProjectile(ECS::Entity owner, bool charged, int chargeLevel)
{
    if (!coordinator_) return;
    if (!coordinator_->HasComponent<Position>(owner)) return;

    auto& ownerPos = coordinator_->GetComponent<Position>(owner);
    
    // Create projectile entity
    ECS::Entity projectile = coordinator_->CreateEntity();
    
    // Add Position
    Position projPos;
    projPos.x = ownerPos.x + 50.0f; // Offset from owner
    projPos.y = ownerPos.y;
    coordinator_->AddComponent<Position>(projectile, projPos);
    
    // Add Velocity
    Velocity projVel;
    projVel.vx = charged ? 1500.0f : 1000.0f;
    projVel.vy = 0.0f;
    coordinator_->AddComponent<Velocity>(projectile, projVel);
    
    // Add Tag
    ProjectileTag projTag;
    projTag.ownerId = static_cast<int>(owner);
    coordinator_->AddComponent<ProjectileTag>(projectile, projTag);
    
    // TODO: Add Sprite, Animation, Collider components
}
