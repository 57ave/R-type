#ifndef ENG_ENGINE_ECS_SYSTEMMANAGER_HPP
#define ENG_ENGINE_ECS_SYSTEMMANAGER_HPP

#include "Types.hpp"
#include "System.hpp"
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <stdexcept>

namespace ECS {

    class SystemManager {
     public:
            template<typename T, typename... Args>
            std::shared_ptr<T> RegisterSystem(Args&&... args)
            {
                const char* typeName = typeid(T).name();

                if (mSystems.find(typeName) != mSystems.end()) {
                    throw std::runtime_error("Registering system more than once.");
                }

                auto system = std::make_shared<T>(std::forward<Args>(args)...);
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

#endif // ENG_ENGINE_ECS_SYSTEMMANAGER_HPP
