#include "LifetimeSystem.hpp"
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
        
        // Check projectile lifetime
        if (m_Coordinator->HasComponent<ECS::Projectile>(entity)) {
            auto& projectile = m_Coordinator->GetComponent<ECS::Projectile>(entity);
            projectile.lifetime -= dt;
            
            if (projectile.lifetime <= 0) {
                shouldDestroy = true;
            }
        }
        
        // Check power-up duration
        if (m_Coordinator->HasComponent<ECS::PowerUp>(entity)) {
            auto& powerup = m_Coordinator->GetComponent<ECS::PowerUp>(entity);
            if (powerup.duration > 0) {
                powerup.duration -= dt;
                
                if (powerup.duration <= 0) {
                    shouldDestroy = true;
                }
            }
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
