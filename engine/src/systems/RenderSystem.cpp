#include <systems/RenderSystem.hpp>
#include <components/Position.hpp>
#include <components/Sprite.hpp>
#include <ecs/Coordinator.hpp>
#include <rendering/Types.hpp>
#include <rendering/IRenderer.hpp>
#include <iostream>

RenderSystem::RenderSystem()
    : renderer_(nullptr)
    , coordinator_(nullptr)
{
}

void RenderSystem::Init()
{
    // nothing to init by default
}

void RenderSystem::Shutdown()
{
}

void RenderSystem::Update(float /*dt*/)
{
    if (!renderer_ || !coordinator_) {
        // Renderer or coordinator not set — nothing to draw
        return;
    }

    // Iterate entities registered in this system (populated by SystemManager)
    for (auto entity : mEntities) {
        // Check components exist
        if (!coordinator_->HasComponent<Position>(entity) || !coordinator_->HasComponent<Sprite>(entity))
            continue;

        auto &pos = coordinator_->GetComponent<Position>(entity);
        auto &spr = coordinator_->GetComponent<Sprite>(entity);

        if (!spr.sprite)
            continue; // nothing to draw

        // Build transform from Position
        rtype::engine::rendering::Transform t;
        t.position.x = pos.x;
        t.position.y = pos.y;
        t.rotation = 0.0f;
        t.scale.x = spr.scaleX;
        t.scale.y = spr.scaleY;

        // Apply texture rect if needed on the sprite instance
        if (spr.textureRect.width != 0 || spr.textureRect.height != 0) {
            spr.sprite->setTextureRect(spr.textureRect);
        }

        // Let the renderer draw the sprite with transform
        renderer_->draw(*spr.sprite, t);
    }
}
