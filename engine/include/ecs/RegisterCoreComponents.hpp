#ifndef RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
#define RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP

#include <ecs/Coordinator.hpp>
// Core components
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
// Game components
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Weapon.hpp>
#include <components/Health.hpp>
#include <components/MovementPattern.hpp>
#include <components/Lifetime.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/AudioSource.hpp>
#include <components/Tag.hpp>
#include <components/Boundary.hpp>

namespace ECS {
    /**
     * @brief Register all core engine components
     * Call this after Coordinator::Init()
     */
    inline void RegisterCoreComponents(Coordinator &coordinator)
    {
        // Core components (Position, Velocity, Sprite)
        coordinator.RegisterComponent<Position>();
        coordinator.RegisterComponent<Velocity>();
        coordinator.RegisterComponent<Sprite>();
        
        // Animation components
        coordinator.RegisterComponent<Animation>();
        coordinator.RegisterComponent<StateMachineAnimation>();
        coordinator.RegisterComponent<ChargeAnimation>();
        
        // Collision components
        coordinator.RegisterComponent<Collider>();
        coordinator.RegisterComponent<Hitbox>();
        
        // Combat components
        coordinator.RegisterComponent<Weapon>();
        coordinator.RegisterComponent<Damage>();
        coordinator.RegisterComponent<Health>();
        
        // Movement components
        coordinator.RegisterComponent<MovementPattern>();
        coordinator.RegisterComponent<EnemyAI>();
        
        // Lifetime components
        coordinator.RegisterComponent<Lifetime>();
        coordinator.RegisterComponent<Effect>();
        coordinator.RegisterComponent<Boundary>();
        
        // Environment components
        coordinator.RegisterComponent<ScrollingBackground>();
        coordinator.RegisterComponent<BackgroundTag>();
        
        // Audio components
        coordinator.RegisterComponent<AudioSource>();
        coordinator.RegisterComponent<SoundEffect>();
        
        // Tag components
        coordinator.RegisterComponent<Tag>();
        coordinator.RegisterComponent<PlayerTag>();
        coordinator.RegisterComponent<EnemyTag>();
        coordinator.RegisterComponent<ProjectileTag>();
    }
}

#endif // RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
