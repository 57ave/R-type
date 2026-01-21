#include "systems/InputSystem.hpp"

#include <components/Velocity.hpp>
#include <iostream>

InputSystem::InputSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator), m_InputHandler(nullptr) {}

void InputSystem::Init() {
    std::cout << "[InputSystem] Initialized" << std::endl;
}

void InputSystem::Update(float dt) {
    for (auto entity : mEntities) {
        // Process entities with velocity component
        if (!m_Coordinator->HasComponent<Velocity>(entity)) {
            continue;
        }

        auto& velocity = m_Coordinator->GetComponent<Velocity>(entity);

        // Apply input to velocity
        velocity.dx = 0;
        velocity.dy = 0;

        float speed = 300.0f;

        // Generic directional actions
        if (m_ActionStates["move_left"])
            velocity.dx = -speed;
        if (m_ActionStates["move_right"])
            velocity.dx = speed;
        if (m_ActionStates["move_up"])
            velocity.dy = -speed;
        if (m_ActionStates["move_down"])
            velocity.dy = speed;

        // Call custom handler for any active actions
        if (m_InputHandler) {
            for (const auto& [action, pressed] : m_ActionStates) {
                if (pressed) {
                    m_InputHandler(entity, action, dt);
                }
            }
        }
    }
}

void InputSystem::Shutdown() {
    std::cout << "[InputSystem] Shutdown" << std::endl;
}

void InputSystem::SetActionState(const std::string& action, bool pressed) {
    m_ActionStates[action] = pressed;
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
