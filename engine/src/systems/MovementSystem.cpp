#include "systems/MovementSystem.hpp"
#include "components/Position.hpp"
#include <iostream>
#include <cmath>

MovementSystem::MovementSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

void MovementSystem::Init() {
    std::cout << "[MovementSystem] Initialized" << std::endl;
}

void MovementSystem::Update(float dt) {
    for (auto entity : mEntities) {
        // Check if entity has Position component
        if (!m_Coordinator->HasComponent<Position>(entity)) {
            continue;
        }
        
        auto& position = m_Coordinator->GetComponent<Position>(entity);
        auto& velocity = m_Coordinator->GetComponent<rtype::engine::ECS::Velocity>(entity);
        
        // Apply velocity to position
        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;
        
        // Clamp to max speed
        float speed = std::sqrt(velocity.dx * velocity.dx + velocity.dy * velocity.dy);
        if (speed > velocity.maxSpeed) {
            float scale = velocity.maxSpeed / speed;
            velocity.dx *= scale;
            velocity.dy *= scale;
        }
    }
}

void MovementSystem::Shutdown() {
    std::cout << "[MovementSystem] Shutdown" << std::endl;
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
