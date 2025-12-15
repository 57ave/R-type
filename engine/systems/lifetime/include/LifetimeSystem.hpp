#pragma once

#include <engine/ecs/System.hpp>
#include <engine/ecs/Coordinator.hpp>
#include <engine/ecs/Components.hpp>

/**
 * @brief LifetimeSystem - Destroys entities after their lifetime expires
 * 
 * Manages time-limited entities like projectiles and power-up effects.
 * 
 * Signature: Projectile OR PowerUp
 */
class LifetimeSystem : public ECS::System {
public:
    explicit LifetimeSystem(ECS::Coordinator* coordinator);
    ~LifetimeSystem() override = default;
    
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;
    
    const char* GetName() const { return "LifetimeSystem"; }
    uint32_t GetSystemVersion() const { return 1; }

private:
    ECS::Coordinator* m_Coordinator;
};

extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator);
    void DestroySystem(ECS::System* system);
    const char* GetSystemName();
    uint32_t GetSystemVersion();
}
