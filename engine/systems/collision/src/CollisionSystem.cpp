#include "CollisionSystem.hpp"
#include <cmath>
#include <iostream>
#include <vector>

CollisionSystem::CollisionSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator), m_CollisionCallback(nullptr) {
}

void CollisionSystem::Init() {
    std::cout << "[CollisionSystem] Initialized" << std::endl;
}

void CollisionSystem::Update(float dt) {
    // Convert set to vector for indexing
    std::vector<ECS::Entity> entities(mEntities.begin(), mEntities.end());
    
    // Broad-phase: Check all pairs (O(n²) - can be optimized with spatial partitioning)
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            if (CheckCollision(entities[i], entities[j])) {
                HandleCollision(entities[i], entities[j]);
            }
        }
    }
}

void CollisionSystem::Shutdown() {
    std::cout << "[CollisionSystem] Shutdown" << std::endl;
}

void CollisionSystem::SetCollisionCallback(CollisionCallback callback) {
    m_CollisionCallback = callback;
}

bool CollisionSystem::CheckCollision(ECS::Entity a, ECS::Entity b) {
    auto& transformA = m_Coordinator->GetComponent<ECS::Transform>(a);
    auto& transformB = m_Coordinator->GetComponent<ECS::Transform>(b);
    auto& colliderA = m_Coordinator->GetComponent<ECS::Collider>(a);
    auto& colliderB = m_Coordinator->GetComponent<ECS::Collider>(b);
    
    // Circle-circle collision
    float dx = transformA.x - transformB.x;
    float dy = transformA.y - transformB.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    float radiusSum = colliderA.radius + colliderB.radius;
    
    return distance < radiusSum;
}

void CollisionSystem::HandleCollision(ECS::Entity a, ECS::Entity b) {
    // Apply damage if applicable
    bool aHasHealth = m_Coordinator->HasComponent<ECS::Health>(a);
    bool bHasHealth = m_Coordinator->HasComponent<ECS::Health>(b);
    bool aHasDamage = m_Coordinator->HasComponent<ECS::Damage>(a);
    bool bHasDamage = m_Coordinator->HasComponent<ECS::Damage>(b);
    
    if (aHasHealth && bHasDamage) {
        auto& health = m_Coordinator->GetComponent<ECS::Health>(a);
        auto& damage = m_Coordinator->GetComponent<ECS::Damage>(b);
        health.TakeDamage(damage.value);
    }
    
    if (bHasHealth && aHasDamage) {
        auto& health = m_Coordinator->GetComponent<ECS::Health>(b);
        auto& damage = m_Coordinator->GetComponent<ECS::Damage>(a);
        health.TakeDamage(damage.value);
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
