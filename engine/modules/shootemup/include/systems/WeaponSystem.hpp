#ifndef SHOOTEMUP_SYSTEMS_WEAPONSYSTEM_HPP
#define SHOOTEMUP_SYSTEMS_WEAPONSYSTEM_HPP

#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class WeaponSystem : public ECS::System {
    public:
        WeaponSystem();
        ~WeaponSystem() override = default;
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

    private:
        ECS::Coordinator* coordinator_;

        void CreateProjectile(ECS::Entity owner, bool charged, int chargeLevel);
};

#endif // SHOOTEMUP_SYSTEMS_WEAPONSYSTEM_HPP
