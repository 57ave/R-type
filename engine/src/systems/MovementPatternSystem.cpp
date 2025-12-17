#include <systems/MovementPatternSystem.hpp>
#include <components/Position.hpp>
#include <components/MovementPattern.hpp>
#include <ecs/Coordinator.hpp>
#include <cmath>

MovementPatternSystem::MovementPatternSystem(ECS::Coordinator* coordinator)
    : coordinator_(coordinator)
{
}

void MovementPatternSystem::Init()
{
}

void MovementPatternSystem::Shutdown()
{
}

void MovementPatternSystem::Update(float dt)
{
    if (!coordinator_) return;

    for (auto entity : mEntities) {
        if (!coordinator_->HasComponent<Position>(entity) || 
            !coordinator_->HasComponent<MovementPattern>(entity))
            continue;

        auto& pos = coordinator_->GetComponent<Position>(entity);
        auto& pattern = coordinator_->GetComponent<MovementPattern>(entity);

        pattern.timeAlive += dt;

        switch (pattern.pattern) {
            case MovementPattern::Type::STRAIGHT:
                // Simple horizontal movement to the left
                pos.x -= pattern.speed * dt;
                break;

            case MovementPattern::Type::SINE_WAVE:
                // Sine wave movement
                pos.x -= pattern.speed * dt;
                pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
                break;

            case MovementPattern::Type::ZIGZAG:
                // Zigzag movement
                pos.x -= pattern.speed * dt;
                pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive * 2.0f);
                break;

            case MovementPattern::Type::CIRCULAR:
                // Circular movement while advancing
                pos.x -= pattern.speed * dt * 0.5f;
                pos.x += pattern.amplitude * 0.3f * std::cos(pattern.frequency * pattern.timeAlive);
                pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
                break;

            case MovementPattern::Type::DIAGONAL_DOWN:
                // Diagonal downward
                pos.x -= pattern.speed * dt;
                pos.y += pattern.speed * dt * 0.5f;
                break;

            case MovementPattern::Type::DIAGONAL_UP:
                // Diagonal upward
                pos.x -= pattern.speed * dt;
                pos.y -= pattern.speed * dt * 0.5f;
                break;
        }

        // Clamp Y position to screen bounds
        if (pos.y < 0.0f) pos.y = 0.0f;
        if (pos.y > windowHeight_) pos.y = windowHeight_;
    }
}
