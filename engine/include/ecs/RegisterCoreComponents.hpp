#ifndef RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
#define RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP

#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>

namespace ECS {
    inline void RegisterCoreComponents(Coordinator &coordinator)
    {
        coordinator.RegisterComponent<Position>();
        coordinator.RegisterComponent<Velocity>();
        coordinator.RegisterComponent<Sprite>();
        // Add more here when additional component headers are available,
    }

}

#endif // RTYPE_ENGINE_ECS_REGISTER_CORE_COMPONENTS_HPP
