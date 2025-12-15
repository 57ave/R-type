#pragma once

#include <engine/ecs/System.hpp>
#include <engine/ecs/Coordinator.hpp>
#include <engine/ecs/Components.hpp>
#include <map>
#include <functional>

enum class InputKey {
    UP, DOWN, LEFT, RIGHT,
    SHOOT, BOMB,
    PAUSE, QUIT
};

class InputSystem : public ECS::System {
public:
    using InputHandler = std::function<void(ECS::Entity, InputKey, float)>;
    
    explicit InputSystem(ECS::Coordinator* coordinator);
    ~InputSystem() override = default;
    
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;
    
    void SetKeyState(InputKey key, bool pressed);
    void SetInputHandler(InputHandler handler);
    
    const char* GetName() const { return "InputSystem"; }
    uint32_t GetSystemVersion() const { return 1; }

private:
    ECS::Coordinator* m_Coordinator;
    std::map<InputKey, bool> m_KeyStates;
    InputHandler m_InputHandler;
};

extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator);
    void DestroySystem(ECS::System* system);
    const char* GetSystemName();
    uint32_t GetSystemVersion();
}
