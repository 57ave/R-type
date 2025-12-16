#ifndef RTYPE_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP

#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class ScrollingBackgroundSystem : public ECS::System {
    public:
        ScrollingBackgroundSystem();
        ~ScrollingBackgroundSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

    private:
        ECS::Coordinator* coordinator_;
};

#endif // RTYPE_ENGINE_SYSTEMS_SCROLLINGBACKGROUNDSYSTEM_HPP
