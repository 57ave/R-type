/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Main scripting integration header - Include this to use all scripting features
*/

#pragma once

// Check if scripting is enabled at compile time
#if RTYPE_SCRIPTING_ENABLED

#include "ComponentBindings.hpp"
#include "CoreBindings.hpp"
#include "DevConsoleBindings.hpp"
#include "GameBindings.hpp"
#include "LuaState.hpp"
#include "PrefabManager.hpp"
#include "ScriptSystem.hpp"

namespace Scripting {

/**
 * @brief ScriptingManager - Central manager for all Lua scripting
 *
 * This class simplifies the initialization and management of Lua scripting
 * for the game. It handles:
 * - LuaState initialization
 * - Component bindings registration
 * - DevConsole command bindings
 * - Game function bindings
 * - Script loading and hot-reload
 *
 * Usage:
 *   ScriptingManager scripting;
 *   scripting.init(&coordinator, &devConsole);
 *   scripting.loadGameScripts("assets/scripts/config/");
 *
 *   // In game loop:
 *   scripting.update(deltaTime);
 */
class ScriptingManager {
public:
    ScriptingManager() = default;
    ~ScriptingManager() = default;

    /**
     * @brief Initialize the scripting system
     * @param coordinator Pointer to the ECS coordinator
     * @param console Pointer to the DevConsole (optional)
     * @return true if initialization succeeded
     */
    bool init(ECS::Coordinator* coordinator, rtype::core::DevConsole* console = nullptr);

    /**
     * @brief Shutdown the scripting system
     */
    void shutdown();

    /**
     * @brief Load game configuration scripts
     * @param configPath Path to the config directory
     * @return true if loading succeeded
     */
    bool loadGameScripts(const std::string& configPath);

    /**
     * @brief Load a single Lua script
     * @param path Path to the script file
     * @return true if loading succeeded
     */
    bool loadScript(const std::string& path);

    /**
     * @brief Update the scripting system (hot-reload, etc.)
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    /**
     * @brief Sync GameState from C++ to Lua
     */
    void syncGameState();

    /**
     * @brief Get the Lua state
     * @return Reference to the Lua state
     */
    sol::state& getLuaState() { return LuaState::Instance().GetState(); }

    /**
     * @brief Get the prefab manager
     * @return Pointer to the prefab manager
     */
    PrefabManager* getPrefabManager() { return m_prefabManager.get(); }

    /**
     * @brief Check if scripting is initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    ECS::Coordinator* m_coordinator = nullptr;
    rtype::core::DevConsole* m_console = nullptr;
    std::unique_ptr<PrefabManager> m_prefabManager;
    float m_hotReloadTimer = 0.0f;
    float m_hotReloadInterval = 1.0f;  // Check every second
};

}  // namespace Scripting

#else  // RTYPE_SCRIPTING_ENABLED

// Stub class when scripting is disabled
namespace Scripting {

class ScriptingManager {
public:
    bool init(void*, void*) { return false; }
    void shutdown() {}
    bool loadGameScripts(const std::string&) { return false; }
    bool loadScript(const std::string&) { return false; }
    void update(float) {}
    void syncGameState() {}
    bool isInitialized() const { return false; }
};

}  // namespace Scripting

#endif  // RTYPE_SCRIPTING_ENABLED
