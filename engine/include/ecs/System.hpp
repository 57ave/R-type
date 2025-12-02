#pragma once

#include "Types.hpp"
#include <set>

namespace ECS {

    class System {
     public:
            virtual ~System() = default;
            virtual void Init() = 0;
            virtual void Update(float dt) = 0;
            virtual void Shutdown() = 0;

     protected:
            std::set<Entity> mEntities;
            friend class SystemManager;
    };

} // namespace ECS
