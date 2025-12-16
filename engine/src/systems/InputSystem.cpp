#include "systems/InputSystem.hpp"
#include <iostream>

InputSystem::InputSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator), m_InputHandler(nullptr) {
}

void InputSystem::Init() {
    std::cout << "[InputSystem] Initialized" << std::endl;
}

void InputSystem::Update(float dt) {
    for (auto entity : mEntities) {
        // Only process player entities
        if (!m_Coordinator->HasComponent<rtype::engine::ECS::Player>(entity)) {
            continue;
        }
        
        auto& velocity = m_Coordinator->GetComponent<rtype::engine::ECS::Velocity>(entity);
        
        // Apply input to velocity
        velocity.dx = 0;
        velocity.dy = 0;
        
        float speed = 300.0f;
        
        if (m_KeyStates[InputKey::LEFT]) velocity.dx = -speed;
        if (m_KeyStates[InputKey::RIGHT]) velocity.dx = speed;
        if (m_KeyStates[InputKey::UP]) velocity.dy = -speed;
        if (m_KeyStates[InputKey::DOWN]) velocity.dy = speed;
        
        // Call custom handler for special keys
        if (m_InputHandler) {
            if (m_KeyStates[InputKey::SHOOT]) {
                m_InputHandler(entity, InputKey::SHOOT, dt);
            }
            if (m_KeyStates[InputKey::BOMB]) {
                m_InputHandler(entity, InputKey::BOMB, dt);
            }
        }
    }
}

void InputSystem::Shutdown() {
    std::cout << "[InputSystem] Shutdown" << std::endl;
}

void InputSystem::SetKeyState(InputKey key, bool pressed) {
    m_KeyStates[key] = pressed;
}

void InputSystem::SetInputHandler(InputHandler handler) {
    m_InputHandler = handler;
}

extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator) {
        return new InputSystem(coordinator);
    }
    
    void DestroySystem(ECS::System* system) {
        delete system;
    }
    
    const char* GetSystemName() {
        return "InputSystem";
    }
    
    uint32_t GetSystemVersion() {
        return 1;
    }
}
