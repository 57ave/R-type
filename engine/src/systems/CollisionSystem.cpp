#include "systems/CollisionSystem.hpp"
#include <components/Position.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Weapon.hpp>
#include <components/Tag.hpp>
#include <cmath>
#include <iostream>
#include <vector>

CollisionSystem::CollisionSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator), m_CollisionCallback(nullptr) {
}

void CollisionSystem::Init() {
    std::cout << "[CollisionSystem] Initialized" << std::endl;
}

void CollisionSystem::Update(float /*dt*/) {
    if (!m_Coordinator) return;

    // Convert set to vector for indexing
    std::vector<ECS::Entity> entities(mEntities.begin(), mEntities.end());
    
    // Separate entities by type for optimized collision checks
    std::vector<ECS::Entity> projectiles;
    std::vector<ECS::Entity> enemies;
    
    for (auto entity : entities) {
        // Filter by tags
        if (m_Coordinator->HasComponent<ProjectileTag>(entity)) {
            projectiles.push_back(entity);
        } else if (m_Coordinator->HasComponent<EnemyTag>(entity)) {
            enemies.push_back(entity);
        }
    }
    
    // Check projectile vs enemy collisions
    for (auto proj : projectiles) {
        for (auto enemy : enemies) {
            if (CheckCollisionAABB(proj, enemy)) {
                HandleCollision(proj, enemy);
            }
        }
    }
    
    // Optional: Check all remaining pairs for other collision types
    // (player vs enemy, player vs powerup, etc.)
}

void CollisionSystem::Shutdown() {
    std::cout << "[CollisionSystem] Shutdown" << std::endl;
}

void CollisionSystem::SetCollisionCallback(CollisionCallback callback) {
    m_CollisionCallback = callback;
}

bool CollisionSystem::CheckCollision(ECS::Entity a, ECS::Entity b) {
    // Legacy circle collision (kept for compatibility)
    if (!m_Coordinator->HasComponent<Position>(a) || !m_Coordinator->HasComponent<Position>(b))
        return false;
    
    // Try AABB collision first
    return CheckCollisionAABB(a, b);
}

bool CollisionSystem::CheckCollisionAABB(ECS::Entity a, ECS::Entity b) {
    if (!m_Coordinator->HasComponent<Position>(a) || !m_Coordinator->HasComponent<Position>(b))
        return false;
    if (!m_Coordinator->HasComponent<Collider>(a) || !m_Coordinator->HasComponent<Collider>(b))
        return false;
        
    auto& posA = m_Coordinator->GetComponent<Position>(a);
    auto& posB = m_Coordinator->GetComponent<Position>(b);
    auto& colA = m_Coordinator->GetComponent<Collider>(a);
    auto& colB = m_Coordinator->GetComponent<Collider>(b);
    
    // Skip if either collider is disabled
    if (!colA.enabled || !colB.enabled)
        return false;
    
    // AABB (Axis-Aligned Bounding Box) collision
    float aLeft = posA.x + colA.offsetX;
    float aRight = aLeft + colA.width;
    float aTop = posA.y + colA.offsetY;
    float aBottom = aTop + colA.height;
    
    float bLeft = posB.x + colB.offsetX;
    float bRight = bLeft + colB.width;
    float bTop = posB.y + colB.offsetY;
    float bBottom = bTop + colB.height;
    
    return (aLeft < bRight && aRight > bLeft &&
            aTop < bBottom && aBottom > bTop);
}

void CollisionSystem::HandleCollision(ECS::Entity a, ECS::Entity b) {
    // Apply damage if applicable
    bool aHasHealth = m_Coordinator->HasComponent<Health>(a);
    bool bHasHealth = m_Coordinator->HasComponent<Health>(b);
    bool aHasDamage = m_Coordinator->HasComponent<Damage>(a);
    bool bHasDamage = m_Coordinator->HasComponent<Damage>(b);
    
    if (aHasHealth && bHasDamage) {
        auto& health = m_Coordinator->GetComponent<Health>(a);
        auto& damage = m_Coordinator->GetComponent<Damage>(b);
        if (!health.invulnerable) {
            health.current -= damage.amount;
        }
    }
    
    if (bHasHealth && aHasDamage) {
        auto& health = m_Coordinator->GetComponent<Health>(b);
        auto& damage = m_Coordinator->GetComponent<Damage>(a);
        if (!health.invulnerable) {
            health.current -= damage.amount;
        }
    }
    
    // Destroy projectile on impact (typical behavior)
    if (m_Coordinator->HasComponent<ProjectileTag>(a)) {
        m_Coordinator->DestroyEntity(a);
    }
    if (m_Coordinator->HasComponent<ProjectileTag>(b)) {
        m_Coordinator->DestroyEntity(b);
    }
    
    // Call custom callback if set
    if (m_CollisionCallback) {
        m_CollisionCallback(a, b);
    }
}

// C API implementation
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator) {
        return new CollisionSystem(coordinator);
    }
    
    void DestroySystem(ECS::System* system) {
        delete system;
    }
    
    const char* GetSystemName() {
        return "CollisionSystem";
    }
    
    uint32_t GetSystemVersion() {
        return 1;
    }
}
