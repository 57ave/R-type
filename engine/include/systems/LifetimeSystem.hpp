#pragma once

#include <core/Export.hpp>
#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <ecs/Components.hpp>

class RTYPE_API LifetimeSystem : public ECS::System {
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
