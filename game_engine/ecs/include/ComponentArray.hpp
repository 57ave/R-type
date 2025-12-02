#pragma once

#include "Types.hpp"
#include <array>
#include <unordered_map>
#include <cassert>
#include <stdexcept>

namespace ECS {

    class IComponentArray {
     public:
            virtual ~IComponentArray() = default;
            virtual void EntityDestroyed(Entity entity) = 0;
    };

    template<typename T>
    class ComponentArray : public IComponentArray {
     public:
            void InsertData(Entity entity, T component)
            {
                if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end()) {
                    throw std::runtime_error("Component added to same entity more than once.");
                }

                // Put new entry at end and update the maps
                size_t newIndex = mSize;
                mEntityToIndexMap[entity] = newIndex;
                mIndexToEntityMap[newIndex] = entity;
                mComponentArray[newIndex] = component;
                ++mSize;
            }

            void RemoveData(Entity entity)
            {
                if (mEntityToIndexMap.find(entity) == mEntityToIndexMap.end()) {
                    throw std::runtime_error("Removing non-existent component.");
                }

                // Copy element at end into deleted element's place to maintain density
                size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
                size_t indexOfLastElement = mSize - 1;
                mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];

                // Update map to point to moved spot
                Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
                mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
                mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

                // Remove the entity's entry from the maps
                mEntityToIndexMap.erase(entity);
                mIndexToEntityMap.erase(indexOfLastElement);

                --mSize;
            }

            T& GetData(Entity entity)
            {
                if (mEntityToIndexMap.find(entity) == mEntityToIndexMap.end()) {
                    throw std::runtime_error("Retrieving non-existent component.");
                }

                // Return a reference to the entity's component
                return mComponentArray[mEntityToIndexMap[entity]];
            }

            void EntityDestroyed(Entity entity) override
            {
                if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end()) {
                    // Remove the entity's component if it existed
                    RemoveData(entity);
                }
            }

            size_t Size() const { return mSize; }

            bool HasData(Entity entity) const
            {
                return mEntityToIndexMap.find(entity) != mEntityToIndexMap.end();
            }

     private:
            std::array<T, MAX_ENTITIES> mComponentArray;
            std::unordered_map<Entity, size_t> mEntityToIndexMap;
            std::unordered_map<size_t, Entity> mIndexToEntityMap;
            size_t mSize = 0;
    };

} // namespace ECS
