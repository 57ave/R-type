#pragma once

#include <engine/ecs/System.hpp>
#include <engine/ecs/Coordinator.hpp>
#include <engine/ecs/Components.hpp>
#include <functional>

/**
 * @brief CollisionSystem - Detects and handles collisions between entities
 * 
 * Performs broad-phase collision detection using spatial hashing
 * and narrow-phase using circle-circle intersection.
 * 
 * Signature: Transform + Collider
 */
class CollisionSystem : public ECS::System {
public:
    using CollisionCallback = std::function<void(ECS::Entity, ECS::Entity)>;
    
    explicit CollisionSystem(ECS::Coordinator* coordinator);
    ~CollisionSystem() override = default;
    
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;
    
    // Register collision callback
    void SetCollisionCallback(CollisionCallback callback);
    
    const char* GetName() const { return "CollisionSystem"; }
    uint32_t GetVersion() const { return 1; }

private:
    ECS::Coordinator* m_Coordinator;
    CollisionCallback m_CollisionCallback;
    
    bool CheckCollision(ECS::Entity a, ECS::Entity b);
    void HandleCollision(ECS::Entity a, ECS::Entity b);
};

// C API for dynamic loading
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator);
    void DestroySystem(ECS::System* system);
    const char* GetSystemName();
    uint32_t GetSystemVersion();
}
