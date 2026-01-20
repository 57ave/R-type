#include <core/SystemLoader.hpp>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
    #define DL_HANDLE HMODULE
    #define DL_OPEN(path) LoadLibraryA(path)
    #define DL_SYM(handle, name) GetProcAddress(handle, name)
    #define DL_CLOSE(handle) FreeLibrary(handle)
    #define DL_ERROR() "GetLastError: " + std::to_string(GetLastError())
#else
    #include <dlfcn.h>
    #define DL_HANDLE void*
    #define DL_OPEN(path) dlopen(path, RTLD_LAZY)
    #define DL_SYM(handle, name) dlsym(handle, name)
    #define DL_CLOSE(handle) dlclose(handle)
    #define DL_ERROR() (dlerror() ? std::string(dlerror()) : std::string("Unknown error"))
#endif

SystemLoader::SystemLoader(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

SystemLoader::~SystemLoader() {
    auto systems = GetLoadedSystems();
    for (auto it = systems.rbegin(); it != systems.rend(); ++it) {
        UnloadSystem(*it);
    }
}

std::shared_ptr<ECS::System> SystemLoader::LoadSystem(const std::string& libPath, const std::string& systemName) {
    if (IsLoaded(systemName)) {
        std::cout << "[SystemLoader] System already loaded: " << systemName << std::endl;
        return m_LoadedSystems[systemName].system;
    }
    
    DL_HANDLE handle = DL_OPEN(libPath.c_str());
    if (!handle) {
        throw std::runtime_error("[SystemLoader] Failed to load library: " + libPath + 
                                 "\nError: " + DL_ERROR());
    }
    
    typedef ECS::System* (*CreateSystemFunc)(ECS::Coordinator*);
    auto createFunc = (CreateSystemFunc)DL_SYM(handle, "CreateSystem");
    
    if (!createFunc) {
        DL_CLOSE(handle);
        throw std::runtime_error("[SystemLoader] Failed to find CreateSystem: " + std::string(DL_ERROR()));
    }
    
    ECS::System* rawSystem = createFunc(m_Coordinator);
    if (!rawSystem) {
        DL_CLOSE(handle);
        throw std::runtime_error("[SystemLoader] CreateSystem returned null");
    }
    
    std::shared_ptr<ECS::System> system(rawSystem, [handle](ECS::System* sys) {
        typedef void (*DestroySystemFunc)(ECS::System*);
        auto destroyFunc = (DestroySystemFunc)DL_SYM(handle, "DestroySystem");
        
        if (destroyFunc) {
            destroyFunc(sys);
        } else {
            delete sys;
        }
    });
    
    
    SystemHandle sysHandle;
    sysHandle.libraryHandle = handle;
    sysHandle.libPath = libPath;
    sysHandle.system = system;
    sysHandle.signature = ECS::Signature();
    
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
    
    DL_HANDLE handle = (DL_HANDLE)it->second.libraryHandle;
    m_LoadedSystems.erase(it);
    DL_CLOSE(handle);
    
    std::cout << "[SystemLoader] Unloaded system: " << systemName << std::endl;
}

std::shared_ptr<ECS::System> SystemLoader::ReloadSystem(const std::string& systemName) {
    auto it = m_LoadedSystems.find(systemName);
    if (it == m_LoadedSystems.end()) {
        throw std::runtime_error("[SystemLoader] Cannot reload unknown system: " + systemName);
    }
    
    std::cout << "[SystemLoader] Reloading system: " << systemName << std::endl;
    
    std::string libPath = it->second.libPath;
    ECS::Signature signature = it->second.signature;
    UnloadSystem(systemName);
    
    auto system = LoadSystem(libPath, systemName);
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
