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
        
        // NOTE: Game-specific lifetime components (Projectile, PowerUp, etc.) removed
        // The generic Lifetime component handles all entity lifetime management
        // Games can extend this system or create their own for specific behavior
        
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
