#include <ecs/ComponentManager.hpp>

namespace ECS {

void ComponentManager::EntityDestroyed(Entity entity) {
    // Notify each component array that an entity has been destroyed
    // If it has a component for that entity, it will remove it
    for (auto const& pair : mComponentArrays) {
        auto const& component = pair.second;
        component->EntityDestroyed(entity);
    }
}

}  // namespace ECS
