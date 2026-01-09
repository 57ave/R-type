#include "systems/LifetimeSystem.hpp"
#include "components/Lifetime.hpp"
#include <iostream>
#include <vector>

LifetimeSystem::LifetimeSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

void LifetimeSystem::Init() {
    std::cout << "[LifetimeSystem] Initialized" << std::endl;
}

void LifetimeSystem::Update(float dt) {
    std::vector<ECS::Entity> toDestroy;
    
    for (auto entity : mEntities) {
        bool shouldDestroy = false;
        
        // Check generic Lifetime component (for explosions, effects, etc.)
        if (m_Coordinator->HasComponent<Lifetime>(entity)) {
            auto& lifetime = m_Coordinator->GetComponent<Lifetime>(entity);
            lifetime.timeAlive += dt;
            
            if (lifetime.timeAlive >= lifetime.maxLifetime && lifetime.destroyOnExpire) {
                shouldDestroy = true;
            }
        }
        
        // Check projectile lifetime - WRAPPED IN TRY-CATCH since Projectile may not be registered
        try {
            if (m_Coordinator->HasComponent<rtype::engine::ECS::Projectile>(entity)) {
                auto& projectile = m_Coordinator->GetComponent<rtype::engine::ECS::Projectile>(entity);
                projectile.lifetime -= dt;
                
                if (projectile.lifetime <= 0) {
                    shouldDestroy = true;
                }
            }
        } catch (const std::exception&) {
            // Projectile component not registered - skip this check
        }
        
        // Check power-up duration - WRAPPED IN TRY-CATCH since PowerUp may not be registered
        try {
            if (m_Coordinator->HasComponent<rtype::engine::ECS::PowerUp>(entity)) {
                auto& powerup = m_Coordinator->GetComponent<rtype::engine::ECS::PowerUp>(entity);
                if (powerup.duration > 0) {
                    powerup.duration -= dt;
                    
                    if (powerup.duration <= 0) {
                        shouldDestroy = true;
                    }
                }
            }
        } catch (const std::exception&) {
            // PowerUp component not registered - skip this check
        }
        
        if (shouldDestroy) {
            toDestroy.push_back(entity);
        }
    }
    
    // Destroy expired entities
    for (auto entity : toDestroy) {
        m_Coordinator->DestroyEntity(entity);
    }
}

void LifetimeSystem::Shutdown() {
    std::cout << "[LifetimeSystem] Shutdown" << std::endl;
}

extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator) {
        return new LifetimeSystem(coordinator);
    }
    
    void DestroySystem(ECS::System* system) {
        delete system;
    }
    
    const char* GetSystemName() {
        return "LifetimeSystem";
    }
    
    uint32_t GetSystemVersion() {
        return 1;
    }
}
