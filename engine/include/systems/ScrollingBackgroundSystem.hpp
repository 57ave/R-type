#ifndef ENG_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP

#include <core/Export.hpp>
#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class RTYPE_API ScrollingBackgroundSystem : public ECS::System {
    public:
        explicit ScrollingBackgroundSystem(ECS::Coordinator* coordinator = nullptr);
        ~ScrollingBackgroundSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

    private:
        ECS::Coordinator* coordinator_;
};

#endif // ENG_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP
