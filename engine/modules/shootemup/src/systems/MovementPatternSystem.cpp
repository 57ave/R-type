#include <systems/MovementPatternSystem.hpp>
#include <components/Position.hpp>
#include <components/MovementPattern.hpp>
#include <ecs/Coordinator.hpp>
#include <cmath>

using namespace ShootEmUp::Components;

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

        // String-based pattern matching (configured in Lua)
        if (pattern.patternType == "straight") {
            // Simple horizontal movement to the left
            pos.x -= pattern.speed * dt;
        }
        else if (pattern.patternType == "sine_wave") {
            // Sine wave movement
            pos.x -= pattern.speed * dt;
            pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
        }
        else if (pattern.patternType == "zigzag") {
            // Zigzag movement
            pos.x -= pattern.speed * dt;
            pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive * 2.0f);
        }

        else if (pattern.patternType == "circular") {
            // Circular movement while advancing
            pos.x -= pattern.speed * dt * 0.5f;
            pos.x += pattern.amplitude * 0.3f * std::cos(pattern.frequency * pattern.timeAlive);
            pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
        }
        else if (pattern.patternType == "diagonal_down") {
            // Diagonal downward
            pos.x -= pattern.speed * dt;
            pos.y += pattern.speed * dt * 0.5f;
        }
        else if (pattern.patternType == "diagonal_up") {
            // Diagonal upward
            pos.x -= pattern.speed * dt;
            pos.y -= pattern.speed * dt * 0.5f;
        }

        // Clamp Y position to screen bounds
        if (pos.y < 0.0f) pos.y = 0.0f;
        if (pos.y > windowHeight_) pos.y = windowHeight_;
    }
}
