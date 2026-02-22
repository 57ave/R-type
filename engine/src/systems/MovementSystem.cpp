#include "systems/MovementSystem.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "core/Logger.hpp"
#include <cmath>

MovementSystem::MovementSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

void MovementSystem::Init() {
    LOG_INFO("MOVEMENTSYSTEM", "Initialized");
}

void MovementSystem::Update(float dt) {
    static int frameCount = 0;
    
    for (auto entity : mEntities) {
        // Check if entity has Position and Velocity components
        if (!m_Coordinator->HasComponent<Position>(entity) || 
            !m_Coordinator->HasComponent<Velocity>(entity)) {
            continue;
        }
        
        auto& position = m_Coordinator->GetComponent<Position>(entity);
        auto& velocity = m_Coordinator->GetComponent<Velocity>(entity);
        
        // Apply velocity to position
        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;
    }
}

void MovementSystem::Shutdown() {
    LOG_INFO("MOVEMENTSYSTEM", "Shutdown");
}

// C API implementation
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator) {
        return new MovementSystem(coordinator);
    }
    
    void DestroySystem(ECS::System* system) {
        delete system;
    }
    
    const char* GetSystemName() {
        return "MovementSystem";
    }
    
    uint32_t GetSystemVersion() {
        return 1;
    }
}
