#pragma once

#include "Types.hpp"
#include "System.hpp"
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <stdexcept>

namespace ECS {

    class SystemManager {
     public:
            template<typename T>
            std::shared_ptr<T> RegisterSystem()
            {
                const char* typeName = typeid(T).name();

                if (mSystems.find(typeName) != mSystems.end()) {
                    throw std::runtime_error("Registering system more than once.");
                }

                auto system = std::make_shared<T>();
                mSystems.insert({typeName, system});
                system->Init();
                return system;
            }

            template<typename T>
            void SetSignature(Signature signature)
            {
                const char* typeName = typeid(T).name();

                if (mSystems.find(typeName) == mSystems.end()) {
                    throw std::runtime_error("System used before registered.");
                }

                mSignatures.insert({typeName, signature});
            }

            void EntityDestroyed(Entity entity);
            void EntitySignatureChanged(Entity entity, Signature entitySignature);
            void ShutdownAll();

     private:
            std::unordered_map<const char*, Signature> mSignatures;
            std::unordered_map<const char*, std::shared_ptr<System>> mSystems;
    };

} // namespace ECS
