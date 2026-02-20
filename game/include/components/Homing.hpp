#ifndef GAME_COMPONENTS_HOMING_HPP
#define GAME_COMPONENTS_HOMING_HPP

#include <ecs/ECS.hpp>

/**
 * @brief Component for projectiles that home in on enemies
 * 
 * Makes projectiles track and follow the nearest enemy
 */
struct Homing {
    float turnSpeed = 200.0f;      // How fast the projectile can turn (degrees per second)
    float detectionRadius = 500.0f; // How far the projectile can detect enemies
    ECS::Entity targetEntity = 0;   // Current target entity (0 = no target)
    float maxSpeed = 600.0f;        // Maximum speed of the homing projectile
};

#endif // GAME_COMPONENTS_HOMING_HPP
