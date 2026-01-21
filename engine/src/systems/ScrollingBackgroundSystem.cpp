#include <systems/ScrollingBackgroundSystem.hpp>
#include <components/Position.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Sprite.hpp>
#include <ecs/Coordinator.hpp>

ScrollingBackgroundSystem::ScrollingBackgroundSystem(ECS::Coordinator* coordinator)
    : coordinator_(coordinator)
{
}

void ScrollingBackgroundSystem::Init()
{
}

void ScrollingBackgroundSystem::Shutdown()
{
}

void ScrollingBackgroundSystem::Update(float dt)
{
    if (!coordinator_) return;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Position>(entity) || 
            !coordinator_->HasComponent<ScrollingBackground>(entity) ||
            !coordinator_->HasComponent<Sprite>(entity))
            continue;

        auto& pos = coordinator_->GetComponent<Position>(entity);
        auto& scroll = coordinator_->GetComponent<ScrollingBackground>(entity);
        auto& sprite = coordinator_->GetComponent<Sprite>(entity);

        if (scroll.horizontal) {
            // Déplacer le background vers la gauche
            pos.x -= scroll.scrollSpeed * dt;

            // Loop infini : quand ce sprite sort complètement à gauche de l'écran,
            // le repositionner à droite (2 * spriteWidth car il y a 2 sprites)
            if (scroll.loop && pos.x <= -scroll.spriteWidth) {
                pos.x += scroll.spriteWidth * 2.0f;
            }

            // Mettre à jour la position du sprite SFML
            if (sprite.sprite) {
                sprite.sprite->setPosition(eng::engine::rendering::Vector2f(pos.x, pos.y));
            }
        } else {
            // Scrolling vertical
            pos.y += scroll.scrollSpeed * dt;

            // Loop background
            if (scroll.loop && pos.y > scroll.spriteWidth) {
                pos.y = -scroll.spriteWidth;
            }

            // Mettre à jour la position du sprite SFML
            if (sprite.sprite) {
                sprite.sprite->setPosition(eng::engine::rendering::Vector2f(pos.x, pos.y));
            }
        }
    }
}
