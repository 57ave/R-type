#include <components/Boundary.hpp>
#include <components/Position.hpp>
#include <components/Sprite.hpp>
#include <ecs/Coordinator.hpp>
#include <systems/BoundarySystem.hpp>
#include <vector>

BoundarySystem::BoundarySystem() : coordinator_(nullptr) {}

void BoundarySystem::Init() {}

void BoundarySystem::Shutdown() {}

void BoundarySystem::Update(float dt) {
    if (!coordinator_)
        return;

    std::vector<ECS::Entity> toDestroy;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Position>(entity) ||
            !coordinator_->HasComponent<Boundary>(entity))
            continue;

        auto& pos = coordinator_->GetComponent<Position>(entity);
        auto& boundary = coordinator_->GetComponent<Boundary>(entity);

        // Get entity size if it has a sprite
        float width = 0.0f;
        float height = 0.0f;
        if (coordinator_->HasComponent<Sprite>(entity)) {
            auto& sprite = coordinator_->GetComponent<Sprite>(entity);
            width = static_cast<float>(sprite.textureRect.width);
            height = static_cast<float>(sprite.textureRect.height);
        }

        bool outOfBounds = false;

        // Check if out of bounds
        if (pos.x + width < -boundary.margin || pos.x > windowWidth_ + boundary.margin ||
            pos.y + height < -boundary.margin || pos.y > windowHeight_ + boundary.margin) {
            outOfBounds = true;
        }

        if (outOfBounds) {
            if (boundary.destroyOutOfBounds) {
                toDestroy.push_back(entity);
            } else if (boundary.clampToBounds) {
                // Clamp to screen bounds
                if (pos.x < 0)
                    pos.x = 0;
                if (pos.y < 0)
                    pos.y = 0;
                if (pos.x + width > windowWidth_)
                    pos.x = windowWidth_ - width;
                if (pos.y + height > windowHeight_)
                    pos.y = windowHeight_ - height;
            }
        }
    }

    // Destroy entities that went out of bounds
    for (auto entity : toDestroy) {
        coordinator_->DestroyEntity(entity);
    }
}
