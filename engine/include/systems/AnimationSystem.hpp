#ifndef ENG_ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP

#include <core/Export.hpp>
#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class RTYPE_API AnimationSystem : public ECS::System {
    public:
        AnimationSystem();
        ~AnimationSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

    private:
        ECS::Coordinator* coordinator_;
};

#endif // ENG_ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP
