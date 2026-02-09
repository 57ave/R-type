#include <systems/MovementPatternSystem.hpp>
#include <components/Position.hpp>
#include <components/MovementPattern.hpp>
#include <ecs/Coordinator.hpp>
#include <cmath>
#include <cstdlib>

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
        else if (pattern.patternType == "sine_wave" || pattern.patternType == "sinewave") {
            // Sine wave movement (both spellings supported)
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
        else if (pattern.patternType == "stationary" || pattern.patternType == "hover") {
            // Stationary - no movement
            // Entity stays at its current position
        }
        else if (pattern.patternType == "chase") {
            // Chase/Kamikaze - move towards player
            float playerX = 100.0f;  // Default fallback
            float playerY = windowHeight_ / 2.0f;
            
            // Get player position if player entity is set
            if (playerEntity_ != 0 && coordinator_->HasComponent<Position>(playerEntity_)) {
                auto& playerPos = coordinator_->GetComponent<Position>(playerEntity_);
                playerX = playerPos.x;
                playerY = playerPos.y;
            }
            
            // Calculate direction to player
            float dx = playerX - pos.x;
            float dy = playerY - pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            // Move towards player
            if (distance > 1.0f) {
                pos.x += (dx / distance) * pattern.speed * dt;
                pos.y += (dy / distance) * pattern.speed * dt;
            }
        }
        else if (pattern.patternType == "evasive") {
            // Evasive - dodge player shots by moving unpredictably
            pos.x -= pattern.speed * dt * 0.7f;
            
            // Random dodging every 0.5 seconds
            if (std::fmod(pattern.timeAlive, 0.5f) < dt) {
                pattern.startY = pos.y + ((std::rand() % 2 == 0) ? 50.0f : -50.0f);
            }
            
            // Smooth movement to dodge position
            float targetY = pattern.startY;
            float diff = targetY - pos.y;
            if (std::abs(diff) > 5.0f) {
                pos.y += (diff > 0 ? 1.0f : -1.0f) * pattern.speed * dt * 0.8f;
            }
        }

        // Clamp Y position to screen bounds
        if (pos.y < 0.0f) pos.y = 0.0f;
        if (pos.y > windowHeight_) pos.y = windowHeight_;
    }
}
