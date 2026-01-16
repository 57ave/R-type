#ifndef RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
#define RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP

#include <ecs/Coordinator.hpp>
// GENERIC ENGINE components ONLY
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Lifetime.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/AudioSource.hpp>
#include <components/Tag.hpp>
#include <components/Boundary.hpp>

/**
 * @file RegisterCoreComponents.hpp
 * @brief Register GENERIC engine components only
 * 
 * WARNING: This file should ONLY contain generic, reusable components.
 * Game-specific components (Weapon, MovementPattern, PlayerTag, etc.) 
 * should be registered in the game project, NOT here.
 */

namespace ECS {
    /**
     * @brief Register all GENERIC engine components
     * Call this after Coordinator::Init()
     * 
     * Game projects should register their own game-specific components separately.
     */
    inline void RegisterCoreComponents(Coordinator &coordinator)
    {
        // Core transform components
        coordinator.RegisterComponent<Position>();
        coordinator.RegisterComponent<Velocity>();
        coordinator.RegisterComponent<Sprite>();
        
        // Animation components
        coordinator.RegisterComponent<Animation>();
        // NOTE: StateMachineAnimation, ChargeAnimation may be game-specific
        
        // Collision components
        coordinator.RegisterComponent<Collider>();
        // NOTE: Hitbox may be game-specific
        
        // Combat components (generic)
        coordinator.RegisterComponent<Health>();
        // NOTE: Weapon, Damage are R-Type specific - moved to game/
        
        // Lifetime components
        coordinator.RegisterComponent<Lifetime>();
        coordinator.RegisterComponent<Boundary>();
        // NOTE: Effect may be game-specific
        
        // Environment components
        coordinator.RegisterComponent<ScrollingBackground>();
        // NOTE: BackgroundTag may be game-specific
        
        // Audio components
        coordinator.RegisterComponent<AudioSource>();
        // NOTE: SoundEffect may be game-specific
        
        // Tag component (generic string-based tagging)
        coordinator.RegisterComponent<Tag>();
        
        // NOTE: The following are R-TYPE SPECIFIC and should be registered in game/:
        // - Weapon, Damage, MovementPattern, EnemyAI
        // - PlayerTag, EnemyTag, ProjectileTag
        // - ChargeAnimation, Hitbox, etc.
    }
}

#endif // RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
