#include <engine/core/SystemLoader.hpp>
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>

SystemLoader::SystemLoader(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

SystemLoader::~SystemLoader() {
    // Unload all systems (in reverse order)
    auto systems = GetLoadedSystems();
    for (auto it = systems.rbegin(); it != systems.rend(); ++it) {
        UnloadSystem(*it);
    }
}

std::shared_ptr<ECS::System> SystemLoader::LoadSystem(const std::string& libPath, const std::string& systemName) {
    // Check if already loaded
    if (IsLoaded(systemName)) {
        std::cout << "[SystemLoader] System already loaded: " << systemName << std::endl;
        return m_LoadedSystems[systemName].system;
    }
    
    // Open shared library
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error("[SystemLoader] Failed to load library: " + libPath + 
                                 "\nError: " + dlerror());
    }
    
    // Get CreateSystem function
    typedef ECS::System* (*CreateSystemFunc)(ECS::Coordinator*);
    auto createFunc = (CreateSystemFunc)dlsym(handle, "CreateSystem");
    
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        dlclose(handle);
        throw std::runtime_error("[SystemLoader] Failed to find CreateSystem: " + 
                                 std::string(dlsym_error));
    }
    
    // Create system instance (raw pointer from factory)
    ECS::System* rawSystem = createFunc(m_Coordinator);
    if (!rawSystem) {
        dlclose(handle);
        throw std::runtime_error("[SystemLoader] CreateSystem returned null");
    }
    
    // Wrap in shared_ptr with custom deleter
    // IMPORTANT: Don't dlclose in the deleter! It will be called in UnloadSystem
    // after the shared_ptr is destroyed.
    std::shared_ptr<ECS::System> system(rawSystem, [handle](ECS::System* sys) {
        // Get DestroySystem function
        typedef void (*DestroySystemFunc)(ECS::System*);
        auto destroyFunc = (DestroySystemFunc)dlsym(handle, "DestroySystem");
        
        if (destroyFunc) {
            destroyFunc(sys);
        } else {
            delete sys;
        }
        // DO NOT dlclose here - it's done in UnloadSystem
    });
    
    
    SystemHandle sysHandle;
    sysHandle.libraryHandle = handle;
    sysHandle.libPath = libPath;
    sysHandle.system = system;
    sysHandle.signature = ECS::Signature();  // Empty, will be set by user
    
    m_LoadedSystems[systemName] = sysHandle;
    
    std::cout << "[SystemLoader] Loaded system '" << systemName << "' from: " << libPath << std::endl;
    return system;
}

void SystemLoader::UnloadSystem(const std::string& systemName) {
    auto it = m_LoadedSystems.find(systemName);
    if (it == m_LoadedSystems.end()) {
        std::cerr << "[SystemLoader] System not loaded: " << systemName << std::endl;
        return;
    }
    
    void* handle = it->second.libraryHandle;
    
    // First, erase from map (this will destroy the shared_ptr and call the deleter)
    m_LoadedSystems.erase(it);
    
    // Now that the system is destroyed, we can safely close the library
    dlclose(handle);
    
    std::cout << "[SystemLoader] Unloaded system: " << systemName << std::endl;
}

std::shared_ptr<ECS::System> SystemLoader::ReloadSystem(const std::string& systemName) {
    auto it = m_LoadedSystems.find(systemName);
    if (it == m_LoadedSystems.end()) {
        throw std::runtime_error("[SystemLoader] Cannot reload unknown system: " + systemName);
    }
    
    std::cout << "[SystemLoader] Reloading system: " << systemName << std::endl;
    
    // Store info before unloading
    std::string libPath = it->second.libPath;
    ECS::Signature signature = it->second.signature;
    
    // Unload
    UnloadSystem(systemName);
    
    // Reload
    auto system = LoadSystem(libPath, systemName);
    
    // Restore signature
    m_LoadedSystems[systemName].signature = signature;
    
    return system;
}

std::shared_ptr<ECS::System> SystemLoader::GetSystem(const std::string& systemName) {
    auto it = m_LoadedSystems.find(systemName);
    if (it == m_LoadedSystems.end()) {
        return nullptr;
    }
    return it->second.system;
}

bool SystemLoader::IsLoaded(const std::string& systemName) const {
    return m_LoadedSystems.find(systemName) != m_LoadedSystems.end();
}

std::vector<std::string> SystemLoader::GetLoadedSystems() const {
    std::vector<std::string> systems;
    for (const auto& [name, _] : m_LoadedSystems) {
        systems.push_back(name);
    }
    return systems;
}
