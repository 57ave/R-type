#pragma once

#include <engine/ecs/System.hpp>
#include <engine/ecs/Coordinator.hpp>
#include <engine/ecs/Components.hpp>

/**
 * @brief MovementSystem - Updates entity positions based on velocity
 * 
 * This system is compiled as a shared library (libMovementSystem.so)
 * and can be dynamically loaded at runtime.
 * 
 * Signature: Transform + Velocity
 */
class MovementSystem : public ECS::System {
public:
    explicit MovementSystem(ECS::Coordinator* coordinator);
    ~MovementSystem() override = default;
    
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;
    
    const char* GetName() const { return "MovementSystem"; }
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
