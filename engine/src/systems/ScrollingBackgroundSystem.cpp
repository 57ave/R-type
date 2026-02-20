#include <systems/ScrollingBackgroundSystem.hpp>
#include <components/Position.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Sprite.hpp>
#include <ecs/Coordinator.hpp>
#include <iostream>

ScrollingBackgroundSystem::ScrollingBackgroundSystem(ECS::Coordinator* coordinator)
    : coordinator_(coordinator)
{
}

void ScrollingBackgroundSystem::Init()
{
    std::cout << "[ScrollingBackgroundSystem] Initialized" << std::endl;
}

void ScrollingBackgroundSystem::Shutdown()
{
}

void ScrollingBackgroundSystem::Update(float dt)
{
    if (!coordinator_) return;

    static int frameCount = 0;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Position>(entity) || 
            !coordinator_->HasComponent<ScrollingBackground>(entity))
            continue;

        auto& pos = coordinator_->GetComponent<Position>(entity);
        auto& scroll = coordinator_->GetComponent<ScrollingBackground>(entity);

        if (scroll.horizontal) {
            // Move background to the left
            pos.x -= scroll.scrollSpeed * dt;

            // Loop background when it goes off screen
            if (scroll.loop && pos.x + scroll.spriteWidth < 0) {
                pos.x += scroll.spriteWidth;
            }
        } else {
            // Vertical scrolling
            pos.y += scroll.scrollSpeed * dt;

            // Loop background
            if (scroll.loop && pos.y > scroll.spriteWidth) {
                pos.y = 0.0f;
            }
        }
    }
}
