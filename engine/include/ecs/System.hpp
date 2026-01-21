#ifndef ENG_ENGINE_ECS_SYSTEM_HPP
#define ENG_ENGINE_ECS_SYSTEM_HPP

#include <set>

#include "Types.hpp"

namespace ECS {

class System {
public:
    virtual ~System() = default;
    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual void Shutdown() = 0;

    // Helper methods for manual entity management (for dynamically loaded systems)
    virtual void AddEntityToSystem(Entity entity) { mEntities.insert(entity); }
    virtual void RemoveEntityFromSystem(Entity entity) { mEntities.erase(entity); }
    virtual size_t GetEntityCount() const { return mEntities.size(); }

protected:
    std::set<Entity> mEntities;
    friend class SystemManager;
};

}  // namespace ECS

#endif  // ENG_ENGINE_ECS_SYSTEM_HPP
