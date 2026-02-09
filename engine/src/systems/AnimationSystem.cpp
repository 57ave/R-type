#include <systems/AnimationSystem.hpp>
#include <components/Animation.hpp>
#include <components/Sprite.hpp>
#include <ecs/Coordinator.hpp>
#include <rendering/Types.hpp>

AnimationSystem::AnimationSystem()
    : coordinator_(nullptr)
{
}

void AnimationSystem::Init()
{
}

void AnimationSystem::Shutdown()
{
}

void AnimationSystem::Update(float dt)
{
    if (!coordinator_) return;

    // Update basic frame-based animations
    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Animation>(entity) || 
            !coordinator_->HasComponent<Sprite>(entity))
            continue;

        auto& anim = coordinator_->GetComponent<Animation>(entity);
        auto& sprite = coordinator_->GetComponent<Sprite>(entity);

        // If animation finished and not looping, hide the sprite immediately
        if (anim.finished && !anim.loop) {
            if (sprite.sprite) {
                // Hide sprite by setting scale to 0
                sprite.scaleX = 0.0f;
                sprite.scaleY = 0.0f;
            }
            continue;
        }

        anim.currentTime += dt;

        if (anim.currentTime >= anim.frameTime) {
            anim.currentTime = 0.0f;
            anim.currentFrame++;

            if (anim.currentFrame >= anim.frameCount) {
                if (anim.loop) {
                    anim.currentFrame = 0;
                } else {
                    // Animation finished - hide sprite IMMEDIATELY
                    anim.currentFrame = anim.frameCount - 1;
                    anim.finished = true;
                    if (sprite.sprite) {
                        sprite.scaleX = 0.0f;
                        sprite.scaleY = 0.0f;
                    }
                    continue;  // Don't update texture rect, sprite is hidden
                }
            }

            // Update sprite texture rect
            sprite.textureRect.left = anim.startX + (anim.currentFrame * (anim.frameWidth + anim.spacing));
            sprite.textureRect.top = anim.startY;
            sprite.textureRect.width = anim.frameWidth;
            sprite.textureRect.height = anim.frameHeight;

            // Apply to native sprite if exists
            if (sprite.sprite) {
                sprite.sprite->setTextureRect(sprite.textureRect);
            }
        }
    }
}
