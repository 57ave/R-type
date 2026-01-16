#pragma once

#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <ecs/Components.hpp>
#include <functional>

class CollisionSystem : public ECS::System {
    public:
        explicit CollisionSystem(ECS::Coordinator* coordinator);
        ~CollisionSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Register collision callback
        void SetCollisionCallback(std::function<void(ECS::Entity, ECS::Entity)> callback);

        const char* GetName() const { return "CollisionSystem"; }
        uint32_t GetVersion() const { return 1; }

    private:
        ECS::Coordinator* m_Coordinator;
        std::function<void(ECS::Entity, ECS::Entity)> m_CollisionCallback;

        bool CheckCollision(ECS::Entity a, ECS::Entity b);
        bool CheckCollisionAABB(ECS::Entity a, ECS::Entity b);
        void HandleCollision(ECS::Entity a, ECS::Entity b);
};

// C API for dynamic loading
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator);
    void DestroySystem(ECS::System* system);
    const char* GetSystemName();
    uint32_t GetSystemVersion();
}
