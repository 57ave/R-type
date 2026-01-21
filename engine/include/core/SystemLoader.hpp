#pragma once

#include <ecs/Coordinator.hpp>
#include <ecs/System.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

class SystemLoader {
public:
    explicit SystemLoader(ECS::Coordinator* coordinator);
    ~SystemLoader();

    std::shared_ptr<ECS::System> LoadSystem(const std::string& libPath,
                                            const std::string& systemName);

    void UnloadSystem(const std::string& systemName);

    std::shared_ptr<ECS::System> ReloadSystem(const std::string& systemName);

    std::shared_ptr<ECS::System> GetSystem(const std::string& systemName);

    bool IsLoaded(const std::string& systemName) const;

    std::vector<std::string> GetLoadedSystems() const;

private:
    struct SystemHandle {
        void* libraryHandle;                  // dlopen handle
        std::string libPath;                  // Path to .so
        std::shared_ptr<ECS::System> system;  // System instance (managed by Coordinator)
        ECS::Signature signature;             // Stored signature for reload
    };

    ECS::Coordinator* m_Coordinator;
    std::map<std::string, SystemHandle> m_LoadedSystems;  // Key = system name
};
