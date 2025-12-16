#ifndef RTYPE_ENGINE_SYSTEMS_MOVEMENTPATTERNSYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_MOVEMENTPATTERNSYSTEM_HPP

#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class MovementPatternSystem : public ECS::System {
    public:
        MovementPatternSystem();
        ~MovementPatternSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }
        void SetWindowHeight(float height) { windowHeight_ = height; }

    private:
        ECS::Coordinator* coordinator_;
        float windowHeight_ = 1080.0f;
};

#endif // RTYPE_ENGINE_SYSTEMS_MOVEMENTPATTERNSYSTEM_HPP
