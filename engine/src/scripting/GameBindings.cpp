/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for game-specific functions - Implementation
*/

#include <core/Logger.hpp>
#include <iostream>
#include <scripting/GameBindings.hpp>

namespace Scripting {

// Static member definitions
GameBindings::CreateEnemyCallback GameBindings::s_CreateEnemy = nullptr;
GameBindings::CreatePlayerCallback GameBindings::s_CreatePlayer = nullptr;
GameBindings::DestroyEntityCallback GameBindings::s_DestroyEntity = nullptr;
GameBindings::GetEntityCountCallback GameBindings::s_GetEntityCount = nullptr;
GameBindings::GetEntitiesCallback GameBindings::s_GetEntities = nullptr;
GameBindings::GetPlayerPositionCallback GameBindings::s_GetPlayerPos = nullptr;
GameBindings::SetPlayerPositionCallback GameBindings::s_SetPlayerPos = nullptr;
GameBindings::SetPlayerHealthCallback GameBindings::s_SetPlayerHealth = nullptr;
GameBindings::SpawnWaveCallback GameBindings::s_SpawnWave = nullptr;
GameBindings::LoadLevelCallback GameBindings::s_LoadLevel = nullptr;

bool GameBindings::s_DebugMode = false;
bool GameBindings::s_GodMode = false;
bool GameBindings::s_NetworkConnected = false;
size_t GameBindings::s_EntityCount = 0;
float GameBindings::s_TimeScale = 1.0f;

void GameBindings::Register(sol::state& lua) {
    // Create/update GameState table in Lua
    lua["GameState"] = lua.create_table_with(
        "debugMode", s_DebugMode, "godMode", s_GodMode, "networkConnected", s_NetworkConnected,
        "entityCount", s_EntityCount, "timeScale", s_TimeScale, "showHitboxes", false,
        "showEntityInfo", false, "noclipMode", false, "serverAddress", "", "ping", 0, "playerCount",
        1);

    // Create Game table with bindings
    auto gameTable = lua.create_named_table("Game");

    // Entity creation
    gameTable.set_function("createEnemy",
                           [](float x, float y, const std::string& type) -> uint32_t {
                               if (s_CreateEnemy) {
                                   return s_CreateEnemy(x, y, type);
                               }
                               LOG_WARNING("SCRIPTING", "Game.createEnemy not bound");
                               return 0;
                           });

    gameTable.set_function("createPlayer", [](float x, float y) -> uint32_t {
        if (s_CreatePlayer) {
            return s_CreatePlayer(x, y);
        }
        LOG_WARNING("SCRIPTING", "Game.createPlayer not bound");
        return 0;
    });

    // Entity destruction
    gameTable.set_function("destroyEntity", [](uint32_t id) {
        if (s_DestroyEntity) {
            s_DestroyEntity(id);
        } else {
            LOG_WARNING("SCRIPTING", "Game.destroyEntity not bound");
        }
    });

    // Entity queries
    gameTable.set_function("getEntityCount", []() -> size_t {
        if (s_GetEntityCount) {
            return s_GetEntityCount();
        }
        return s_EntityCount;
    });

    gameTable.set_function("getEntities", [&lua](const std::string& tag) -> sol::table {
        sol::table result = lua.create_table();

        if (s_GetEntities) {
            auto entities = s_GetEntities(tag);
            int idx = 1;
            for (const auto& e : entities) {
                sol::table entityTable = lua.create_table();
                entityTable["id"] = e.id;
                entityTable["tag"] = e.tag;
                entityTable["x"] = e.x;
                entityTable["y"] = e.y;
                entityTable["type"] = e.type;
                result[idx++] = entityTable;
            }
        }

        return result;
    });

    // Player management
    gameTable.set_function("getPlayerPosition", [&lua]() -> sol::table {
        sol::table result = lua.create_table();
        if (s_GetPlayerPos) {
            auto [x, y] = s_GetPlayerPos();
            result["x"] = x;
            result["y"] = y;
        } else {
            result["x"] = 0.0f;
            result["y"] = 0.0f;
        }
        return result;
    });

    gameTable.set_function("setPlayerPosition", [](float x, float y) {
        if (s_SetPlayerPos) {
            s_SetPlayerPos(x, y);
        } else {
            LOG_WARNING("SCRIPTING", "Game.setPlayerPosition not bound");
        }
    });

    gameTable.set_function("setPlayerHealth", [](int health) {
        if (s_SetPlayerHealth) {
            s_SetPlayerHealth(health);
        } else {
            LOG_WARNING("SCRIPTING", "Game.setPlayerHealth not bound");
        }
    });

    // Wave/Level
    gameTable.set_function("spawnWave", [](int waveId) {
        if (s_SpawnWave) {
            s_SpawnWave(waveId);
        } else {
            LOG_WARNING("SCRIPTING", "Game.spawnWave not bound");
        }
    });

    gameTable.set_function("loadLevel", [](int levelId) {
        if (s_LoadLevel) {
            s_LoadLevel(levelId);
        } else {
            LOG_WARNING("SCRIPTING", "Game.loadLevel not bound");
        }
    });

    LOG_INFO("SCRIPTING", "Game bindings registered");
}

void GameBindings::UpdateGameState(sol::state& lua) {
    // Update the GameState table with current values
    sol::table gameState = lua["GameState"];
    if (gameState.valid()) {
        gameState["debugMode"] = s_DebugMode;
        gameState["godMode"] = s_GodMode;
        gameState["networkConnected"] = s_NetworkConnected;
        gameState["entityCount"] = s_EntityCount;
        gameState["timeScale"] = s_TimeScale;
    }
}

}  // namespace Scripting
