#pragma once

#include <algorithm>
#include <memory>
#include <string>

// Forward declarations for rendering types
namespace eng {
namespace engine {
namespace rendering {
class ISprite;
}
}  // namespace engine
}  // namespace eng

/**
 * @file Components.hpp
 * @brief Generic ECS components for the game engine
 *
 * This file contains ONLY generic, reusable components that could be used
 * in ANY game. Game-specific components (Player, Enemy, etc.) should be
 * defined in the game project, not here.
 */

namespace eng {
namespace engine {
namespace ECS {

/**
 * @brief Transform component - Entity position and rotation
 * Generic: Used in any 2D/3D game
 */
struct Transform {
    float x, y, rotation;

    Transform() : x(0), y(0), rotation(0) {}
    Transform(float x, float y, float r = 0) : x(x), y(y), rotation(r) {}
};

/**
 * @brief Velocity component - Movement speed
 * Generic: Used in any game with physics/movement
 */
struct Velocity {
    float dx, dy;
    float maxSpeed;

    Velocity() : dx(0), dy(0), maxSpeed(1000.0f) {}
    Velocity(float dx, float dy, float max = 1000.0f) : dx(dx), dy(dy), maxSpeed(max) {}
};

/**
 * @brief Sprite component - Visual representation
 * Generic: Used in any 2D game with sprites
 */
struct Sprite {
    std::string texturePath;
    std::shared_ptr<eng::engine::rendering::ISprite> sprite;  // Real sprite instance for rendering
    int width, height;
    int layer = 0;        // Z-order for layered rendering (higher = front)
    bool visible = true;  // Visibility flag

    Sprite() : width(32), height(32) {}
    Sprite(const std::string& path, int w, int h) : texturePath(path), width(w), height(h) {}
};

/**
 * @brief Health component - Hit points
 * Generic: Used in any game with health/damage system
 */
struct Health {
    int current, maximum;

    Health() : current(100), maximum(100) {}
    Health(int hp, int max) : current(hp), maximum(max) {}

    bool IsAlive() const { return current > 0; }
    void TakeDamage(int amount) { current = std::max(0, current - amount); }
    void Heal(int amount) { current = std::min(maximum, current + amount); }
};

/**
 * @brief Damage component - Attack power
 * Generic: Used in any game with combat
 */
struct Damage {
    int value;

    Damage() : value(10) {}
    Damage(int dmg) : value(dmg) {}
};

/**
 * @brief Collider component - Physics collision
 * Generic: Used in any game with collision detection
 */
struct Collider {
    float radius;
    bool isTrigger;

    Collider() : radius(16.0f), isTrigger(false) {}
    Collider(float r, bool trigger = false) : radius(r), isTrigger(trigger) {}
};

/**
 * @brief Tag component - Generic string identifier
 * Generic: Used to mark/categorize entities in any game
 *
 * Examples:
 *   Tag{"Player"}, Tag{"Enemy"}, Tag{"Projectile"}, Tag{"Collectible"}
 *   Tag{"Boss"}, Tag{"NPC"}, Tag{"Obstacle"}, etc.
 *
 * This replaces game-specific marker components like Player, Enemy, etc.
 */
struct Tag {
    std::string value;

    Tag() : value("") {}
    Tag(const std::string& tag) : value(tag) {}

    bool operator==(const std::string& other) const { return value == other; }
    bool operator!=(const std::string& other) const { return value != other; }
};

}  // namespace ECS
}  // namespace engine
}  // namespace eng

// Alias pour compatibilité et clarté
using TransformComponent = eng::engine::ECS::Transform;
using VelocityComponent = eng::engine::ECS::Velocity;
using SpriteComponent = eng::engine::ECS::Sprite;
using HealthComponent = eng::engine::ECS::Health;
using DamageComponent = eng::engine::ECS::Damage;
using CollisionComponent = eng::engine::ECS::Collider;
using TagComponent = eng::engine::ECS::Tag;
