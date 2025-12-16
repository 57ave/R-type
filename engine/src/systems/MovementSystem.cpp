#include "systems/MovementSystem.hpp"
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
        auto& transform = m_Coordinator->GetComponent<ECS::Transform>(entity);
        auto& velocity = m_Coordinator->GetComponent<ECS::Velocity>(entity);
        
        // Apply velocity to position
        transform.x += velocity.dx * dt;
        transform.y += velocity.dy * dt;
        
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
