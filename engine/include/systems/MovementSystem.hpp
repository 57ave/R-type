#pragma once

#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <ecs/Components.hpp>

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
