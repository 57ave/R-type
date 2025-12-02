#pragma once

#include "Types.hpp"
#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include <memory>

namespace ECS {

    class Coordinator {
     public:
            void Init();
            void Shutdown();

            Entity CreateEntity();
            void DestroyEntity(Entity entity);

            template<typename T>
            void RegisterComponent()
            {
                mComponentManager->RegisterComponent<T>();
            }

            template<typename T>
            void AddComponent(Entity entity, T component)
            {
                mComponentManager->AddComponent<T>(entity, component);

                auto signature = mEntityManager->GetSignature(entity);
                signature.set(mComponentManager->GetComponentType<T>(), true);
                mEntityManager->SetSignature(entity, signature);

                mSystemManager->EntitySignatureChanged(entity, signature);
            }

            template<typename T>
            void RemoveComponent(Entity entity)
            {
                mComponentManager->RemoveComponent<T>(entity);

                auto signature = mEntityManager->GetSignature(entity);
                signature.set(mComponentManager->GetComponentType<T>(), false);
                mEntityManager->SetSignature(entity, signature);

                mSystemManager->EntitySignatureChanged(entity, signature);
            }

            template<typename T>
            T& GetComponent(Entity entity)
            {
                return mComponentManager->GetComponent<T>(entity);
            }

            template<typename T>
            bool HasComponent(Entity entity)
            {
                return mComponentManager->HasComponent<T>(entity);
            }

            template<typename T>
            ComponentType GetComponentType()
            {
                return mComponentManager->GetComponentType<T>();
            }

            template<typename T>
            std::shared_ptr<T> RegisterSystem()
            {
                return mSystemManager->RegisterSystem<T>();
            }

            template<typename T>
            void SetSystemSignature(Signature signature)
            {
                mSystemManager->SetSignature<T>(signature);
            }

            Signature GetEntitySignature(Entity entity) const;
            std::uint32_t GetLivingEntityCount() const;
            
            void SetNetworkId(Entity entity, NetworkId networkId);
            NetworkId GetNetworkId(Entity entity) const;
            bool HasNetworkId(Entity entity) const;
            Entity GetEntityByNetworkId(NetworkId networkId) const;
            bool HasEntityForNetworkId(NetworkId networkId) const;

     private:
            std::unique_ptr<ComponentManager> mComponentManager;
            std::unique_ptr<EntityManager> mEntityManager;
            std::unique_ptr<SystemManager> mSystemManager;
    };

} // namespace ECS
