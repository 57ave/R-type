/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** ScriptingManager implementation
*/

#include <scripting/ScriptingManager.hpp>

#if RTYPE_SCRIPTING_ENABLED

#include <scripting/CoreBindings.hpp>
#include <core/Logger.hpp>
#include <filesystem>

namespace Scripting {

bool ScriptingManager::init(ECS::Coordinator* coordinator, rtype::core::DevConsole* console) {
    if (m_initialized) {
        LOG_WARNING("SCRIPTING", "ScriptingManager already initialized");
        return true;
    }
    
    if (!coordinator) {
        LOG_ERROR("SCRIPTING", "Cannot initialize ScriptingManager: coordinator is null");
        return false;
    }
    
    m_coordinator = coordinator;
    m_console = console;
    
    // Initialize LuaState
    LuaState::Instance().Init();
    
    auto& lua = LuaState::Instance().GetState();
    
    // Register core bindings (Logger + Profiler)
    CoreBindings::Register(lua);
    
    // Register component bindings
    ComponentBindings::RegisterAll(lua);
    ComponentBindings::RegisterCoordinator(lua, coordinator);
    
    // Register game bindings
    GameBindings::Register(lua);
    
    // Register console bindings if console is provided
    if (console) {
        DevConsoleBindings::Register(lua, console);
    }
    
    // Create prefab manager
    m_prefabManager = std::make_unique<PrefabManager>(coordinator);
    
    // Set up error callback
    LuaState::Instance().SetErrorCallback([this](const std::string& error) {
        LOG_ERROR("LUA", error);
        if (m_console) {
            m_console->error("[Lua] " + error);
        }
    });
    
    // Enable hot-reload
    LuaState::Instance().EnableHotReload(true);
    
    m_initialized = true;
    LOG_INFO("SCRIPTING", "ScriptingManager initialized");
    
    return true;
}

void ScriptingManager::shutdown() {
    if (!m_initialized) return;
    
    m_prefabManager.reset();
    LuaState::Instance().Shutdown();
    
    m_coordinator = nullptr;
    m_console = nullptr;
    m_initialized = false;
    
    LOG_INFO("SCRIPTING", "ScriptingManager shutdown");
}

bool ScriptingManager::loadGameScripts(const std::string& configPath) {
    namespace fs = std::filesystem;
    
    if (!m_initialized) {
        LOG_ERROR("SCRIPTING", "Cannot load scripts: ScriptingManager not initialized");
        return false;
    }
    
    bool success = true;
    
    // Load game config
    std::string gameConfigPath = configPath + "/game_config.lua";
    if (fs::exists(gameConfigPath)) {
        if (!loadScript(gameConfigPath)) {
            success = false;
        }
    }
    
    // Load console commands
    if (m_console) {
        std::string consoleCommandsPath = configPath + "/console_commands.lua";
        if (fs::exists(consoleCommandsPath)) {
            if (!loadScript(consoleCommandsPath)) {
                success = false;
            }
        }
        
        // Load game-specific commands
        std::string gameCommandsPath = configPath + "/game_commands.lua";
        if (fs::exists(gameCommandsPath)) {
            if (!loadScript(gameCommandsPath)) {
                success = false;
            }
        }
        
        // Load debug tools (Logger + Profiler commands)
        std::string debugToolsPath = configPath + "/debug_tools.lua";
        if (fs::exists(debugToolsPath)) {
            if (!loadScript(debugToolsPath)) {
                success = false;
            }
        }
    }
    
    return success;
}

bool ScriptingManager::loadScript(const std::string& path) {
    return LuaState::Instance().LoadScript(path);
}

void ScriptingManager::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Periodic hot-reload check
    m_hotReloadTimer += deltaTime;
    if (m_hotReloadTimer >= m_hotReloadInterval) {
        m_hotReloadTimer = 0.0f;
        LuaState::Instance().CheckForChanges();
    }
}

void ScriptingManager::syncGameState() {
    if (!m_initialized) return;
    
    GameBindings::UpdateGameState(LuaState::Instance().GetState());
}

} // namespace Scripting

#endif // RTYPE_SCRIPTING_ENABLED
