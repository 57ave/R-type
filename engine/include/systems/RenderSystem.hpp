#ifndef ENG_ENGINE_SYSTEMS_RENDERSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_RENDERSYSTEM_HPP

#include <ecs/System.hpp>
#include <memory>

namespace eng {
namespace engine {
namespace rendering {
class IRenderer;
}
}  // namespace engine
}  // namespace eng

namespace ECS {
class Coordinator;
}

class RenderSystem : public ECS::System {
public:
    RenderSystem();
    ~RenderSystem() override = default;

    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;

    void SetRenderer(eng::engine::rendering::IRenderer* renderer) { renderer_ = renderer; }
    void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }

private:
    eng::engine::rendering::IRenderer* renderer_;
    ECS::Coordinator* coordinator_;
};

#endif  // ENG_ENGINE_SYSTEMS_RENDERSYSTEM_HPP
