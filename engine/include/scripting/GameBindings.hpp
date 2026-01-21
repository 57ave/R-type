/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for game-specific functions
*/

#pragma once

#include <ecs/Coordinator.hpp>
#include <functional>
#include <sol/sol.hpp>
#include <string>
#include <vector>

#include "LuaState.hpp"

namespace Scripting {

/**
 * @brief Entity info structure for Lua
 */
struct LuaEntityInfo {
    uint32_t id;
    std::string tag;
    float x;
    float y;
    std::string type;
};

/**
 * @brief GameBindings - Exposes game functions to Lua
 *
 * This class creates bindings that allow Lua scripts to interact
 * with the game's entity system and core functionality.
 *
 * The game registers callback functions that Lua will call for
 * entity creation, destruction, queries, etc.
 *
 * Usage in C++:
 *   GameBindings::Register(lua);
 *   GameBindings::SetCreateEnemyCallback([](float x, float y, const std::string& type) { ... });
 *
 * Usage in Lua:
 *   Game.createEnemy(800, 300, "basic")
 *   local entities = Game.getEntities("enemy")
 */
class GameBindings {
public:
    // Callback types
    using CreateEnemyCallback = std::function<uint32_t(float, float, const std::string&)>;
    using CreatePlayerCallback = std::function<uint32_t(float, float)>;
    using DestroyEntityCallback = std::function<void(uint32_t)>;
    using GetEntityCountCallback = std::function<size_t()>;
    using GetEntitiesCallback = std::function<std::vector<LuaEntityInfo>(const std::string&)>;
    using GetPlayerPositionCallback = std::function<std::pair<float, float>()>;
    using SetPlayerPositionCallback = std::function<void(float, float)>;
    using SetPlayerHealthCallback = std::function<void(int)>;
    using SpawnWaveCallback = std::function<void(int)>;
    using LoadLevelCallback = std::function<void(int)>;

    /**
     * @brief Register all game bindings to the Lua state
     * @param lua Reference to the Lua state
     */
    static void Register(sol::state& lua);

    /**
     * @brief Update GameState in Lua with current values
     * @param lua Reference to the Lua state
     */
    static void UpdateGameState(sol::state& lua);

    // ========================================
    // Callback setters - call these from game code
    // ========================================

    static void SetCreateEnemyCallback(CreateEnemyCallback cb) { s_CreateEnemy = cb; }
    static void SetCreatePlayerCallback(CreatePlayerCallback cb) { s_CreatePlayer = cb; }
    static void SetDestroyEntityCallback(DestroyEntityCallback cb) { s_DestroyEntity = cb; }
    static void SetGetEntityCountCallback(GetEntityCountCallback cb) { s_GetEntityCount = cb; }
    static void SetGetEntitiesCallback(GetEntitiesCallback cb) { s_GetEntities = cb; }
    static void SetGetPlayerPositionCallback(GetPlayerPositionCallback cb) { s_GetPlayerPos = cb; }
    static void SetSetPlayerPositionCallback(SetPlayerPositionCallback cb) { s_SetPlayerPos = cb; }
    static void SetSetPlayerHealthCallback(SetPlayerHealthCallback cb) { s_SetPlayerHealth = cb; }
    static void SetSpawnWaveCallback(SpawnWaveCallback cb) { s_SpawnWave = cb; }
    static void SetLoadLevelCallback(LoadLevelCallback cb) { s_LoadLevel = cb; }

    // ========================================
    // GameState accessors
    // ========================================

    static void SetDebugMode(bool value) { s_DebugMode = value; }
    static void SetGodMode(bool value) { s_GodMode = value; }
    static void SetNetworkConnected(bool value) { s_NetworkConnected = value; }
    static void SetEntityCount(size_t count) { s_EntityCount = count; }
    static void SetTimeScale(float scale) { s_TimeScale = scale; }

    static bool GetDebugMode() { return s_DebugMode; }
    static bool GetGodMode() { return s_GodMode; }
    static bool GetNetworkConnected() { return s_NetworkConnected; }
    static float GetTimeScale() { return s_TimeScale; }

private:
    // Callbacks (set by game code)
    static CreateEnemyCallback s_CreateEnemy;
    static CreatePlayerCallback s_CreatePlayer;
    static DestroyEntityCallback s_DestroyEntity;
    static GetEntityCountCallback s_GetEntityCount;
    static GetEntitiesCallback s_GetEntities;
    static GetPlayerPositionCallback s_GetPlayerPos;
    static SetPlayerPositionCallback s_SetPlayerPos;
    static SetPlayerHealthCallback s_SetPlayerHealth;
    static SpawnWaveCallback s_SpawnWave;
    static LoadLevelCallback s_LoadLevel;

    // State tracking
    static bool s_DebugMode;
    static bool s_GodMode;
    static bool s_NetworkConnected;
    static size_t s_EntityCount;
    static float s_TimeScale;
};

}  // namespace Scripting
