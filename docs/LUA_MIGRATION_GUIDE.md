# Migration Guide: From C++ Commands to Lua Configuration

This guide explains how to migrate game-specific console commands and configuration from `main.cpp` to Lua scripts.

## Overview

The R-Type engine now supports defining game logic and console commands in Lua configuration files. This provides several advantages:

- **Hot-reload**: Modify commands without recompiling
- **Moddability**: Players can add custom commands
- **Separation of concerns**: Engine code stays clean
- **Easier iteration**: Quick testing of new features

## File Structure

```
assets/scripts/config/
├── game_config.lua       # Game settings (window, player, difficulty)
├── console_commands.lua  # Engine-level console commands
└── game_commands.lua     # Game-specific commands (spawn, kill, etc.)
```

## Step-by-Step Migration

### 1. Include the Scripting Manager

In your `main.cpp`, add:

```cpp
#include <scripting/ScriptingManager.hpp>

// After initializing the ECS coordinator and DevConsole:
Scripting::ScriptingManager scripting;
if (scripting.init(&gCoordinator, &devConsole)) {
    // Load all game scripts
    scripting.loadGameScripts("assets/scripts/config/");
}
```

### 2. Register C++ Callbacks for Game Functions

The Lua scripts need access to game functions. Register callbacks:

```cpp
#if RTYPE_SCRIPTING_ENABLED
// Entity creation callback
Scripting::GameBindings::SetCreateEnemyCallback(
    [](float x, float y, const std::string& type) -> uint32_t {
        MovementPattern::Type pattern = MovementPattern::Type::STRAIGHT;
        if (type == "zigzag") pattern = MovementPattern::Type::ZIGZAG;
        else if (type == "sine") pattern = MovementPattern::Type::SINE;
        else if (type == "circle") pattern = MovementPattern::Type::CIRCLE;
        else if (type == "boss") pattern = MovementPattern::Type::DIVE;
        
        return CreateEnemy(x, y, pattern);
    });

// Entity destruction callback
Scripting::GameBindings::SetDestroyEntityCallback(
    [](uint32_t id) {
        DestroyEntityDeferred(static_cast<ECS::Entity>(id));
    });

// Entity count callback
Scripting::GameBindings::SetGetEntityCountCallback(
    []() -> size_t {
        return allEntities.size();
    });

// Entity list callback
Scripting::GameBindings::SetGetEntitiesCallback(
    [](const std::string& tag) -> std::vector<Scripting::LuaEntityInfo> {
        std::vector<Scripting::LuaEntityInfo> result;
        for (auto entity : allEntities) {
            if (tag == "all" || 
                (tag == "enemy" && gCoordinator.HasComponent<EnemyTag>(entity)) ||
                (tag == "player" && gCoordinator.HasComponent<PlayerTag>(entity))) {
                
                Scripting::LuaEntityInfo info;
                info.id = entity;
                info.tag = gCoordinator.HasComponent<Tag>(entity) ? 
                           gCoordinator.GetComponent<Tag>(entity).name : "unknown";
                
                if (gCoordinator.HasComponent<Position>(entity)) {
                    auto& pos = gCoordinator.GetComponent<Position>(entity);
                    info.x = pos.x;
                    info.y = pos.y;
                }
                result.push_back(info);
            }
        }
        return result;
    });

// Player position callbacks
Scripting::GameBindings::SetGetPlayerPositionCallback(
    [&player]() -> std::pair<float, float> {
        if (gCoordinator.HasComponent<Position>(player)) {
            auto& pos = gCoordinator.GetComponent<Position>(player);
            return {pos.x, pos.y};
        }
        return {0.0f, 0.0f};
    });

Scripting::GameBindings::SetSetPlayerPositionCallback(
    [&player](float x, float y) {
        if (gCoordinator.HasComponent<Position>(player)) {
            auto& pos = gCoordinator.GetComponent<Position>(player);
            pos.x = x;
            pos.y = y;
        }
    });
#endif
```

### 3. Sync Game State Each Frame

In your game loop, sync the game state to Lua:

```cpp
#if RTYPE_SCRIPTING_ENABLED
// Update game state for Lua
Scripting::GameBindings::SetDebugMode(debugMode);
Scripting::GameBindings::SetGodMode(godMode);
Scripting::GameBindings::SetNetworkConnected(networkMode);
Scripting::GameBindings::SetEntityCount(allEntities.size());

// Update scripting system (handles hot-reload)
scripting.update(deltaTime);
scripting.syncGameState();
#endif
```

### 4. Remove C++ Command Registrations

After migrating, you can remove the C++ `registerCommand` calls from `main.cpp`. The commands are now defined in:
- `assets/scripts/config/console_commands.lua` - Generic commands
- `assets/scripts/config/game_commands.lua` - Game-specific commands

### 5. Reading Configuration from Lua

You can read game configuration in C++:

```cpp
#if RTYPE_SCRIPTING_ENABLED
auto& lua = scripting.getLuaState();

// Read player config
sol::table playerConfig = lua["GameConfig"]["player"];
float playerStartX = playerConfig["startX"].get_or(100.0f);
float playerStartY = playerConfig["startY"].get_or(360.0f);
int playerHealth = playerConfig["health"].get_or(100);

// Read difficulty settings
sol::table difficulty = lua.script("return GetDifficulty('normal')");
float enemyHealthMult = difficulty["enemyHealth"].get_or(1.0f);
#endif
```

## Creating Custom Lua Commands

### Simple Command

```lua
Console.register("hello", "Say hello", "hello [name]", function(args)
    local name = args[2] or "World"
    Console.success("Hello, " .. name .. "!")
    return "Greeted"
end)
```

### Command with Game Interaction

```lua
Console.register("spawn_boss", "Spawn a boss", "spawn_boss [x] [y]", function(args)
    local x = tonumber(args[2]) or 700
    local y = tonumber(args[3]) or 360
    
    -- Check if allowed
    if GameState.networkConnected and not GameState.debugMode then
        Console.error("Enable debug mode first!")
        return "Not allowed"
    end
    
    -- Call C++ function
    Game.createEnemy(x, y, "boss")
    Console.success("BOSS spawned!")
    
    return "Boss created"
end)
```

## Hot-Reload

The scripting system automatically checks for file changes every second. When you modify a Lua file, the changes are applied immediately without restarting the game.

To manually reload:
```
/reload_lua
```

## API Reference

### Console API (Lua)

| Function | Description |
|----------|-------------|
| `Console.print(msg)` | Print info message |
| `Console.success(msg)` | Print success message (green) |
| `Console.warning(msg)` | Print warning message (yellow) |
| `Console.error(msg)` | Print error message (red) |
| `Console.clear()` | Clear console |
| `Console.execute(cmd)` | Execute a command |
| `Console.register(name, desc, usage, fn)` | Register new command |
| `Console.unregister(name)` | Remove a command |

### Logger API (Lua)

| Function | Description |
|----------|-------------|
| `Log.debug(category, msg)` | Log debug message |
| `Log.info(category, msg)` | Log info message |
| `Log.success(category, msg)` | Log success message |
| `Log.warning(category, msg)` | Log warning message |
| `Log.error(category, msg)` | Log error message |
| `Log.d(msg)` | Debug with default category "LUA" |
| `Log.i(msg)` | Info with default category "LUA" |
| `Log.w(msg)` | Warning with default category "LUA" |
| `Log.e(msg)` | Error with default category "LUA" |
| `Log.setLevel(level)` | Set min level (debug/info/warning/error) |
| `Log.getLevel()` | Get current log level |
| `Log.setLogFile(path)` | Set log file path |
| `Log.enableColors(bool)` | Enable/disable colored output |
| `Log.enableTimestamps(bool)` | Enable/disable timestamps |
| `Log.flush()` | Flush log buffers |

### Profiler API (Lua)

| Function | Description |
|----------|-------------|
| `Profiler.beginSection(name)` | Start timing a section |
| `Profiler.endSection(name)` | End timing a section |
| `Profiler.getFPS()` | Get current FPS |
| `Profiler.getAverageFPS()` | Get average FPS |
| `Profiler.getFrameTime()` | Get frame time in ms |
| `Profiler.getMinFPS()` | Get minimum recorded FPS |
| `Profiler.getMaxFPS()` | Get maximum recorded FPS |
| `Profiler.getMemoryUsage()` | Get memory usage in MB |
| `Profiler.getEntityCount()` | Get entity count |
| `Profiler.getDrawCalls()` | Get draw call count |
| `Profiler.getSectionTime(name)` | Get section time in ms |
| `Profiler.getSectionAverage(name)` | Get section average in ms |
| `Profiler.enable()` | Enable profiler |
| `Profiler.disable()` | Disable profiler |
| `Profiler.isEnabled()` | Check if enabled |
| `Profiler.reset()` | Reset all stats |
| `Profiler.getReport()` | Get full report string |
| `Profiler.printReport()` | Print report to log |
| `Profiler.getStats()` | Get all stats as table |
| `Profiler.getBytesSent()` | Network bytes sent |
| `Profiler.getBytesReceived()` | Network bytes received |

### Game API (Lua)

| Function | Description |
|----------|-------------|
| `Game.createEnemy(x, y, type)` | Spawn enemy |
| `Game.createPlayer(x, y)` | Create player |
| `Game.destroyEntity(id)` | Destroy entity |
| `Game.getEntityCount()` | Get total entities |
| `Game.getEntities(tag)` | Get entities by tag |
| `Game.getPlayerPosition()` | Get player {x, y} |
| `Game.setPlayerPosition(x, y)` | Teleport player |
| `Game.setPlayerHealth(hp)` | Set player HP |
| `Game.spawnWave(id)` | Trigger wave |
| `Game.loadLevel(id)` | Load level |

### GameState (Lua)

| Variable | Type | Description |
|----------|------|-------------|
| `GameState.debugMode` | bool | Cheats enabled |
| `GameState.godMode` | bool | Invincibility |
| `GameState.networkConnected` | bool | In multiplayer |
| `GameState.entityCount` | int | Total entities |
| `GameState.timeScale` | float | Game speed |

## Troubleshooting

### "Lua not found" during build

Install Lua development libraries:
```bash
# macOS
brew install lua

# Ubuntu
sudo apt install liblua5.4-dev

# Windows (vcpkg)
vcpkg install lua
```

### Sol3 not found

Run the download script:
```bash
./engine/scripts/download_sol3.sh
```

### Commands not loading

1. Check that Lua files exist in `assets/scripts/config/`
2. Check console output for Lua syntax errors
3. Verify file permissions
4. Try running `/lua_info` to check Lua state

## Complete Example

See `docs/examples/lua_integration_example.cpp` for a complete working example of the integration.
