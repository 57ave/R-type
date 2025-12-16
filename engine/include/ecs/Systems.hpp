#pragma once

#include "System.hpp"
#include "Components.hpp"
#include "Coordinator.hpp"
#include "RenderSystem.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

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
     * @brief CollisionSystem - Handles collision detection and response
     */
    class CollisionSystem : public System {
    public:
        void Init() override {}
        void Shutdown() override {}
        
        void Update(float deltaTime) override {
            std::vector<Entity> entities(mEntities.begin(), mEntities.end());
            
            for (size_t i = 0; i < entities.size(); ++i) {
                for (size_t j = i + 1; j < entities.size(); ++j) {
                    CheckCollision(entities[i], entities[j]);
                }
            }
        }
        
        void SetCoordinator(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }
        
    private:
        Coordinator* m_Coordinator = nullptr;
        
        void CheckCollision(Entity a, Entity b) {
            auto& transformA = m_Coordinator->GetComponent<Transform>(a);
            auto& transformB = m_Coordinator->GetComponent<Transform>(b);
            auto& colliderA = m_Coordinator->GetComponent<Collider>(a);
            auto& colliderB = m_Coordinator->GetComponent<Collider>(b);
            
            float dx = transformA.x - transformB.x;
            float dy = transformA.y - transformB.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < (colliderA.radius + colliderB.radius)) {
                OnCollision(a, b);
            }
        }
        
        void OnCollision(Entity a, Entity b) {
            // Handle collision response here
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
