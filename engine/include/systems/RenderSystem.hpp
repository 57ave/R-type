#ifndef RTYPE_ENGINE_SYSTEMS_RENDERSYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_RENDERSYSTEM_HPP

#include <ecs/System.hpp>
#include <memory>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            class IRenderer;
        }
    }
}

namespace ECS
{
    class Coordinator;
}

class RenderSystem : public ECS::System {
    public:
        RenderSystem();
        ~RenderSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetRenderer(rtype::engine::rendering::IRenderer *renderer) { renderer_ = renderer; }
        void SetCoordinator(ECS::Coordinator *coordinator) { coordinator_ = coordinator; }

    private:
        rtype::engine::rendering::IRenderer *renderer_;
        ECS::Coordinator *coordinator_;
};

#endif // RTYPE_ENGINE_SYSTEMS_RENDERSYSTEM_HPP
