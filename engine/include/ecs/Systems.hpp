#pragma once

#include "System.hpp"
#include "Components.hpp"
#include "Coordinator.hpp"
#include "RenderSystem.hpp"
#include <components/Collider.hpp>
#include <components/Position.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>

namespace ECS {

    /**
     * @brief MovementSystem - Handles entity movement based on velocity
     */
    class MovementSystem : public System {
    public:
        void Init() override {}
        void Shutdown() override {}
        
        void Update(float deltaTime) override {
            for (auto entity : mEntities) {
                auto& transform = m_Coordinator->GetComponent<Transform>(entity);
                auto& velocity = m_Coordinator->GetComponent<Velocity>(entity);
                
                transform.x += velocity.dx * deltaTime;
                transform.y += velocity.dy * deltaTime;
            }
        }
        
        void SetCoordinator(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }
        
    private:
        Coordinator* m_Coordinator = nullptr;
    };

    /**
     * @brief CollisionSystem - 100% Generic collision detection
     * 
     * This system detects collisions between ALL entities with Collider components.
     * It does NOT know about game-specific entity types (player, enemy, projectile).
     * All game-specific logic should be handled in the callback.
     */
    class CollisionSystem : public System {
    public:
        using CollisionCallback = std::function<void(Entity, Entity)>;
        
        CollisionSystem() = default;
        explicit CollisionSystem(Coordinator* coordinator) : m_Coordinator(coordinator) {}
        ~CollisionSystem() override = default;
        
        void Init() override {}
        void Shutdown() override {}
        
        void Update(float deltaTime) override {
            std::vector<Entity> entities(mEntities.begin(), mEntities.end());
            
            // Check all pairs of entities
            for (size_t i = 0; i < entities.size(); ++i) {
                for (size_t j = i + 1; j < entities.size(); ++j) {
                    if (CheckCollisionAABB(entities[i], entities[j])) {
                        OnCollision(entities[i], entities[j]);
                    }
                }
            }
        }
        
        void SetCoordinator(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }
        
        /**
         * @brief Set the callback to be called when a collision is detected
         * @param callback Function that receives both entities involved in the collision
         */
        void SetCollisionCallback(CollisionCallback callback) {
            m_CollisionCallback = std::move(callback);
        }
        
    private:
        Coordinator* m_Coordinator = nullptr;
        CollisionCallback m_CollisionCallback;
        
        /**
         * @brief Check AABB collision between two entities
         * Uses Position and Collider components
         */
        bool CheckCollisionAABB(Entity a, Entity b) {
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
        
        void OnCollision(Entity a, Entity b) {
            // Call game-specific callback if set
            if (m_CollisionCallback) {
                m_CollisionCallback(a, b);
            }
        }
    };


    /**
     * @brief LifetimeSystem - Destroys entities after their lifetime expires
     */
    class LifetimeSystem : public System {
    public:
        void Init() override {}
        void Shutdown() override {}
        
        void Update(float deltaTime) override {
            std::vector<Entity> toDestroy;
            
            for (auto entity : mEntities) {
                auto& projectile = m_Coordinator->GetComponent<Projectile>(entity);
                
                projectile.lifetime -= deltaTime;
                
                if (projectile.lifetime <= 0) {
                    toDestroy.push_back(entity);
                }
            }
            
            for (auto entity : toDestroy) {
                m_Coordinator->DestroyEntity(entity);
            }
        }
        
        void SetCoordinator(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }
        
    private:
        Coordinator* m_Coordinator = nullptr;
    };

    /**
     * @brief BoundarySystem - Destroys entities that go out of bounds
     */
    class BoundarySystem : public System {
    public:
        BoundarySystem(float minX = -100, float maxX = 900, float minY = -100, float maxY = 700)
            : m_MinX(minX), m_MaxX(maxX), m_MinY(minY), m_MaxY(maxY) {}
        
        void Init() override {}
        void Shutdown() override {}
        
        void Update(float deltaTime) override {
            std::vector<Entity> toDestroy;
            
            for (auto entity : mEntities) {
                auto& transform = m_Coordinator->GetComponent<Transform>(entity);
                
                if (transform.x < m_MinX || transform.x > m_MaxX ||
                    transform.y < m_MinY || transform.y > m_MaxY) {
                    toDestroy.push_back(entity);
                }
            }
            
            for (auto entity : toDestroy) {
                m_Coordinator->DestroyEntity(entity);
            }
        }
        
        void SetCoordinator(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }
        
    private:
        Coordinator* m_Coordinator = nullptr;
        float m_MinX, m_MaxX, m_MinY, m_MaxY;
    };

} // namespace ECS
