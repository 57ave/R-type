#pragma once

#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <components/Animation.hpp>
#include <components/Sprite.hpp>

/**
 * @brief StateMachineAnimationSystem - Handles state-based animations for player ships
 * 
 * This system manages animations that transition between states (columns in spritesheet)
 * Used for player ships that tilt up/down/neutral
 */
class StateMachineAnimationSystem : public ECS::System {
public:
    explicit StateMachineAnimationSystem(ECS::Coordinator* coordinator);
    ~StateMachineAnimationSystem() override = default;

    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;

    const char* GetName() const { return "StateMachineAnimationSystem"; }
    uint32_t GetVersion() const { return 1; }

private:
    ECS::Coordinator* m_Coordinator;
};

// C API for dynamic loading
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator);
    void DestroySystem(ECS::System* system);
    const char* GetSystemName();
    uint32_t GetSystemVersion();
}
