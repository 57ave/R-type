#pragma once

#include <core/Export.hpp>
#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Collider.hpp>
#include <functional>
#include <vector>

/**
 * @brief Generic CollisionSystem - 100% Abstract
 * 
 * This system detects collisions between ALL entities with Collider components.
 * It does NOT know about game-specific entity types (player, enemy, projectile).
 * All game-specific logic (damage, destruction, effects) should be handled in the callback.
 * 
 * Usage:
 *   auto collisionSystem = coordinator.RegisterSystem<CollisionSystem>(&coordinator);
 *   collisionSystem->SetCollisionCallback([](Entity a, Entity b) {
 *       // Game-specific collision handling here
 *   });
 */
class RTYPE_API CollisionSystem : public ECS::System {
public:
    using CollisionCallback = std::function<void(ECS::Entity, ECS::Entity)>;
    
    CollisionSystem() = default;
    explicit CollisionSystem(ECS::Coordinator* coordinator) : m_Coordinator(coordinator) {}
    ~CollisionSystem() override = default;
    
    void Init() override {}
    void Shutdown() override {}
    
    void Update(float /* deltaTime */) override {
        if (!m_Coordinator) return;
        
        std::vector<ECS::Entity> entities(mEntities.begin(), mEntities.end());
        
        // Check all pairs of entities for collision
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                if (CheckCollisionAABB(entities[i], entities[j])) {
                    OnCollision(entities[i], entities[j]);
                }
            }
        }
    }
    
    void SetCoordinator(ECS::Coordinator* coordinator) {
        m_Coordinator = coordinator;
    }
    
    /**
     * @brief Set the callback to be called when a collision is detected
     * @param callback Function that receives both entities involved in the collision
     */
    void SetCollisionCallback(CollisionCallback callback) {
        m_CollisionCallback = std::move(callback);
    }
    
    const char* GetName() const { return "CollisionSystem"; }
    uint32_t GetVersion() const { return 1; }
    
private:
    ECS::Coordinator* m_Coordinator = nullptr;
    CollisionCallback m_CollisionCallback;
    
    /**
     * @brief Check AABB collision between two entities
     * Uses Position and Collider components
     */
    bool CheckCollisionAABB(ECS::Entity a, ECS::Entity b) {
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
    
    void OnCollision(ECS::Entity a, ECS::Entity b) {
        // Call game-specific callback if set
        if (m_CollisionCallback) {
            m_CollisionCallback(a, b);
        }
    }
};

