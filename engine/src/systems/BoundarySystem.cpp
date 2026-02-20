#include <systems/BoundarySystem.hpp>
#include <components/Position.hpp>
#include <components/Boundary.hpp>
#include <components/Sprite.hpp>
#include <components/Velocity.hpp>
#include <ecs/Coordinator.hpp>
#include <iostream>
#include <vector>

BoundarySystem::BoundarySystem()
    : coordinator_(nullptr)
{
}

void BoundarySystem::Init()
{
}

void BoundarySystem::Shutdown()
{
}

void BoundarySystem::Update(float dt)
{
    if (!coordinator_) return;

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
            // Apply scale to get actual rendered size
            width = static_cast<float>(sprite.textureRect.width) * sprite.scaleX;
            height = static_cast<float>(sprite.textureRect.height) * sprite.scaleY;
        }

        // Check if clamping is needed
        if (boundary.clampToBounds) {
            // Clamp position to screen bounds
            bool wasClamped = false;
            
            if (pos.x < 0) {
                pos.x = 0;
                wasClamped = true;
            }
            if (pos.y < 0) {
                pos.y = 0;
                wasClamped = true;
            }
            if (pos.x + width > windowWidth_) {
                pos.x = windowWidth_ - width;
                wasClamped = true;
            }
            if (pos.y + height > windowHeight_) {
                pos.y = windowHeight_ - height;
                wasClamped = true;
            }
            
            // If clamped, also cancel velocity to prevent continuous pushing
            if (wasClamped && coordinator_->HasComponent<Velocity>(entity)) {
                auto& vel = coordinator_->GetComponent<Velocity>(entity);
                
                // Cancel velocity that would push us further out
                if (pos.x <= 0 && vel.dx < 0) vel.dx = 0.0f;
                if (pos.y <= 0 && vel.dy < 0) vel.dy = 0.0f;
                if (pos.x + width >= windowWidth_ && vel.dx > 0) vel.dx = 0.0f;
                if (pos.y + height >= windowHeight_ && vel.dy > 0) vel.dy = 0.0f;
            }
        } else if (boundary.destroyOutOfBounds) {
            // Check if completely out of bounds for destruction
            // NOTE: Don't destroy when going RIGHT (enemies spawn from right)
            // Only destroy when going LEFT, UP, or DOWN
            bool outOfBounds = false;
            
            // Left side (enemies and projectiles leaving screen to the left)
            if (pos.x + width < -boundary.margin) {
                outOfBounds = true;
            }
            
            // Top or bottom (entities going vertically out of bounds)
            if (pos.y + height < -boundary.margin || 
                pos.y > windowHeight_ + boundary.margin) {
                outOfBounds = true;
            }
            
            // NOTE: Removed right side check (pos.x > windowWidth_ + boundary.margin)
            // to allow enemies to spawn from the right side of the screen
            
            if (outOfBounds) {
                toDestroy.push_back(entity);
            }
        }
    }

    // Destroy entities that went out of bounds
    for (auto entity : toDestroy) {
        coordinator_->DestroyEntity(entity);
    }
}
