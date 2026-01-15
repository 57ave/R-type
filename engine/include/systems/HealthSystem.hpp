#ifndef ENG_ENGINE_SYSTEMS_HEALTHSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_HEALTHSYSTEM_HPP

#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class HealthSystem : public ECS::System {
    public:
        explicit HealthSystem(ECS::Coordinator* coordinator = nullptr);
        ~HealthSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

    private:
        ECS::Coordinator* coordinator_;

        void HandleDeath(ECS::Entity entity);
};

#endif // ENG_ENGINE_SYSTEMS_HEALTHSYSTEM_HPP
