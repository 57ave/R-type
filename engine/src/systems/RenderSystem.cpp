#include <systems/RenderSystem.hpp>
#include <components/Position.hpp>
#include <components/Sprite.hpp>
#include <components/Tag.hpp>
#include <ecs/Coordinator.hpp>
#include <rendering/Types.hpp>
#include <rendering/IRenderer.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

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

    // Collect all renderable entities and sort by layer
    std::vector<ECS::Entity> renderableEntities;
    for (auto entity : mEntities) {
        if (coordinator_->HasComponent<Position>(entity) && 
            coordinator_->HasComponent<Sprite>(entity)) {
            renderableEntities.push_back(entity);
        }
    }

    // Debug: Log enemy sprites being rendered
    static int frameCounter = 0;
    if (frameCounter++ % 60 == 0) {  // Every 60 frames
        int enemyCount = 0;
        int bulletCount = 0;
        int playerBulletCount = 0;
        int enemyBulletCount = 0;
        for (auto entity : renderableEntities) {
            if (coordinator_->HasComponent<Tag>(entity)) {
                auto& tag = coordinator_->GetComponent<Tag>(entity);
                if (tag.name == "Enemy") {
                    enemyCount++;
                } else if (tag.name == "PlayerBullet") {
                    playerBulletCount++;
                } else if (tag.name == "EnemyBullet") {
                    enemyBulletCount++;
                }
            }
        }
        std::cout << "[RenderSystem] Rendering " << renderableEntities.size() 
                  << " entities (" << enemyCount << " enemies, " 
                  << playerBulletCount << " player bullets, " 
                  << enemyBulletCount << " enemy bullets)" << std::endl;
    }

    // Sort by layer (lower layer = drawn first = background)
    std::sort(renderableEntities.begin(), renderableEntities.end(), 
        [this](ECS::Entity a, ECS::Entity b) {
            auto& spriteA = coordinator_->GetComponent<Sprite>(a);
            auto& spriteB = coordinator_->GetComponent<Sprite>(b);
            return spriteA.layer < spriteB.layer;
        });

    // Draw all entities in order
    for (auto entity : renderableEntities) {
        auto &pos = coordinator_->GetComponent<Position>(entity);
        auto &spr = coordinator_->GetComponent<Sprite>(entity);

        if (!spr.sprite)
            continue; // nothing to draw

        // Skip sprites with zero scale (hidden/finished animations)
        if (spr.scaleX <= 0.0f || spr.scaleY <= 0.0f)
            continue;

        // Build transform from Position
        eng::engine::rendering::Transform t;
        t.position.x = pos.x;
        t.position.y = pos.y;
        t.rotation = 0.0f;
        t.scale.x = spr.scaleX;  // Use scale from Sprite component
        t.scale.y = spr.scaleY;  // Use scale from Sprite component

        // Apply texture rect if needed on the sprite instance
        if (spr.textureRect.width != 0 || spr.textureRect.height != 0) {
            spr.sprite->setTextureRect(spr.textureRect);
        }

        // Let the renderer draw the sprite with transform
        renderer_->draw(*spr.sprite, t);
    }
}
