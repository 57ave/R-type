#ifndef ENG_ENGINE_SYSTEMS_BOUNDARYSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_BOUNDARYSYSTEM_HPP

#include <core/Export.hpp>
#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class RTYPE_API BoundarySystem : public ECS::System {
    public:
        BoundarySystem();
        ~BoundarySystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }
        void SetWindowSize(float width, float height) {
            windowWidth_ = width;
            windowHeight_ = height;
        }

    private:
        ECS::Coordinator *coordinator_;
        float windowWidth_ = 1920.0f;
        float windowHeight_ = 1080.0f;
};

#endif // ENG_ENGINE_SYSTEMS_BOUNDARYSYSTEM_HPP
