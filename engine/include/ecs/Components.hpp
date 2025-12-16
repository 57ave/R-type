#pragma once

#include <string>
#include <algorithm>
#include <memory>

// Forward declarations for rendering types
namespace rtype { namespace engine { namespace rendering {
    class ISprite;
}}}


namespace rtype {
    namespace engine {
        namespace ECS {
        
            /**
             * @brief Transform component - Entity position and rotation
             */
            struct Transform {
                float x, y, rotation;

                Transform() : x(0), y(0), rotation(0) {}
                Transform(float x, float y, float r = 0) : x(x), y(y), rotation(r) {}
            };
        
            /**
             * @brief Velocity component - Movement speed
             */
            struct Velocity {
                float dx, dy;
                float maxSpeed;

                Velocity() : dx(0), dy(0), maxSpeed(1000.0f) {}
                Velocity(float dx, float dy, float max = 1000.0f) 
                    : dx(dx), dy(dy), maxSpeed(max) {}
            };
        
            /**
             * @brief Sprite component - Visual representation
             */
            struct Sprite {
                std::string texturePath;
                std::shared_ptr<rtype::engine::rendering::ISprite> sprite;  // Real sprite instance for rendering
                int width, height;
                int layer = 0;          // Z-order for layered rendering (higher = front)
                bool visible = true;    // Visibility flag

                Sprite() : width(32), height(32) {}
                Sprite(const std::string& path, int w, int h) 
                    : texturePath(path), width(w), height(h) {}
            };
        
            /**
             * @brief Health component - Hit points
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
             */
            struct Damage {
                int value;

                Damage() : value(10) {}
                Damage(int dmg) : value(dmg) {}
            };
        
            /**
             * @brief AIController component - Enemy behavior patterns
             */
            struct AIController {
                std::string pattern;  // "straight", "zigzag", "circle", "dive"
                float timer;
                float shootTimer;
                float shootInterval;

                // Pattern-specific data
                float centerX, centerY, circleRadius;
                float targetY;

                AIController() 
                    : pattern("straight"), timer(0), shootTimer(0), shootInterval(2.0f),
                      centerX(0), centerY(0), circleRadius(100),
                      targetY(300) {}
            };
        
            /**
             * @brief Collider component - Physics collision
             */
            struct Collider {
                float radius;
                bool isTrigger;

                Collider() : radius(16.0f), isTrigger(false) {}
                Collider(float r, bool trigger = false) : radius(r), isTrigger(trigger) {}
            };
        
            /**
             * @brief Player component - Marks player entity
             */
            struct Player {
                int playerID;
                int score;

                Player() : playerID(0), score(0) {}
                Player(int id) : playerID(id), score(0) {}
            };
        
            /**
             * @brief Enemy component - Marks enemy entity
             */
            struct Enemy {
                int scoreValue;

                Enemy() : scoreValue(100) {}
                Enemy(int value) : scoreValue(value) {}
            };
        
            /**
             * @brief Projectile component - Bullets/missiles
             */
            struct Projectile {
                int ownerID;
                float lifetime;

                Projectile() : ownerID(0), lifetime(5.0f) {}
                Projectile(int owner, float life = 5.0f) : ownerID(owner), lifetime(life) {}
            };
        
            /**
             * @brief PowerUp component - Collectibles
             */
            struct PowerUp {
                enum Type {
                    SPEED_BOOST,
                    DAMAGE_BOOST,
                    HEALTH_RESTORE,
                    SHIELD,
                    WEAPON_UPGRADE
                };

                Type type;
                float duration;
                int value;

                PowerUp() : type(HEALTH_RESTORE), duration(0), value(25) {}
                PowerUp(Type t, float dur = 0, int val = 25) : type(t), duration(dur), value(val) {}
            };
        } // namespace ECS
    }
}

// Alias pour compatibilité et clarté
using TransformComponent = rtype::engine::ECS::Transform;
using VelocityComponent = rtype::engine::ECS::Velocity;
using SpriteComponent = rtype::engine::ECS::Sprite;
using HealthComponent = rtype::engine::ECS::Health;
using DamageComponent = rtype::engine::ECS::Damage;
using AIComponent = rtype::engine::ECS::AIController;
using CollisionComponent = rtype::engine::ECS::Collider;
using PlayerComponent = rtype::engine::ECS::Player;
using EnemyComponent = rtype::engine::ECS::Enemy;
using ProjectileComponent = rtype::engine::ECS::Projectile;
using PowerUpComponent = rtype::engine::ECS::PowerUp;
