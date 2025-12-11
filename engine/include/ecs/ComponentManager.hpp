#pragma once

#include "Types.hpp"
#include "ComponentArray.hpp"
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <stdexcept>

namespace ECS {

    class ComponentManager {
     public:
            template<typename T>
            void RegisterComponent()
            {
                const char* typeName = typeid(T).name();

                if (mComponentTypes.find(typeName) != mComponentTypes.end()) {
                    throw std::runtime_error("Registering component type more than once.");
                }

                mComponentTypes.insert({typeName, mNextComponentType});
                mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
                ++mNextComponentType;
            }

            template<typename T>
            ComponentType GetComponentType()
            {
                const char* typeName = typeid(T).name();

                if (mComponentTypes.find(typeName) == mComponentTypes.end()) {
                    throw std::runtime_error("Component not registered before use.");
                }

                return mComponentTypes[typeName];
            }

            template<typename T>
            void AddComponent(Entity entity, T component)
            {
                GetComponentArray<T>()->InsertData(entity, component);
            }

            template<typename T>
            void RemoveComponent(Entity entity)
            {
                GetComponentArray<T>()->RemoveData(entity);
            }

            template<typename T>
            T& GetComponent(Entity entity)
            {
                return GetComponentArray<T>()->GetData(entity);
            }

            template<typename T>
            bool HasComponent(Entity entity)
            {
                return GetComponentArray<T>()->HasData(entity);
            }

            void EntityDestroyed(Entity entity);

     private:
            std::unordered_map<const char*, ComponentType> mComponentTypes;
            std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays;
            ComponentType mNextComponentType = 0;

            template<typename T>
            std::shared_ptr<ComponentArray<T>> GetComponentArray()
            {
                const char* typeName = typeid(T).name();

                if (mComponentTypes.find(typeName) == mComponentTypes.end()) {
                    throw std::runtime_error("Component not registered before use.");
                }

                return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
            }
    };

} // namespace ECS
