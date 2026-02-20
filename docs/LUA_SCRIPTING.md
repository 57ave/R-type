# Lua Scripting

The game uses Lua for all gameplay configuration. No game values are hardcoded in C++. This means you can tweak enemies, weapons, levels, and UI without recompiling.

Scripts live in `game/assets/scripts/`. The engine loads `init.lua` first, which pulls in all other config files.

---

## Entry Point

`game/assets/scripts/init.lua` is the first script executed. It loads every config module:

```lua
-- init.lua
require("config/game_config")
require("config/player_config")
require("config/weapons_config")
require("config/enemies_config")
require("config/bosses_config")
require("config/collectables_config")
require("config/assets_paths")
require("config/vfx_config")
```

---

## Config Files

### `config/game_config.lua`

General game settings: window resolution, target FPS, scroll speed, difficulty multiplier per stage.

### `config/player_config.lua`

Player stats: HP, movement speed, fire rate, charge shot levels (1–5) with damage and sprite per level.

### `config/weapons_config.lua`

All weapons defined as named entries. Each weapon specifies:
- `damage`, `fire_rate`, `projectile_speed`
- `sprite` path and animation data
- `sound` effect path

### `config/enemies_config.lua`

All enemy types. Each entry specifies:

```lua
basic = {
    sprite     = "assets/enemies/basic_enemie.png",
    frames     = 2,
    anim_speed = 0.2,
    hp         = 1,
    speed      = 200,
    points     = 100,
    pattern    = "straight_left",
    shoot_rate = 0.0,     -- 0 = does not shoot
},
```

Available movement patterns: `straight_left`, `zigzag`, `sine_wave`, `kamikaze`, `spiral`.

### `config/bosses_config.lua`

Boss definitions with multiple phases. Each phase can have a different movement pattern, attack pattern, and HP threshold trigger.

### `config/collectables_config.lua`

Power-up types with drop weights and effects (health restore, speed boost, weapon upgrade, etc.).

### `config/assets_paths.lua`

All asset file paths centralized in one place. Any change to a file name only needs updating here.

### `config/vfx_config.lua`

Visual effect definitions: explosion animations, hit flashes, particle systems.

---

## Level and Wave Files

### Levels (`scripts/levels/`)

One file per level. Each level defines:
- background sprite and scroll speed
- music track
- ordered list of wave files to run

```lua
-- levels/level_1.lua
return {
    background = "assets/backgrounds/bg_level1.png",
    music      = "assets/sounds/level1.ogg",
    waves      = {
        "levels/waves/l1_wave1",
        "levels/waves/l1_wave2",
        "levels/waves/l1_boss",
    },
}
```

### Waves (`scripts/levels/`)

Each wave is a timed spawn list:

```lua
return {
    duration = 30.0,
    enemies  = {
        { type = "basic",   spawn_time = 0.0, x = 1920, y = 150 },
        { type = "zigzag",  spawn_time = 2.0, x = 1920, y = 400 },
        { type = "shooter", spawn_time = 5.0, x = 1920, y = 300 },
    },
}
```

`spawn_time` is in seconds from wave start. `x` and `y` are the initial spawn position.

---

## UI Scripts (`scripts/ui/`)

Each menu has its own Lua file defining button layout, text, and callbacks. Menu structure is configured entirely from Lua — button positions, labels, and target states are read at runtime.

---

## C++ API Available in Lua

These functions are registered by the engine and available in any script:

| Function | Description |
|---|---|
| `GameAPI.SpawnEnemy(type, x, y)` | Spawn an enemy entity at position |
| `GameAPI.SpawnProjectile(type, x, y, vx, vy)` | Spawn a projectile |
| `GameAPI.GetPlayerConfig()` | Returns the player config table |
| `GameAPI.SetDifficulty(level)` | Set global difficulty multiplier |
| `DevConsole.RegisterCommand(name, desc, fn)` | Register a dev console command |
| `DevConsole.AddLog(message, level)` | Print to the dev console |
| `LuaProfiler.Toggle()` | Toggle the Lua profiler overlay |
| `LuaProfiler.Profile(name, fn)` | Profile a function by name |

---

## Hot-Reload

During development, the Lua scripting system monitors config files for changes. When a file is modified, it is reloaded automatically without restarting the game.

This only applies to config values. C++ bindings and ECS component registrations are not hot-reloadable.

To enable hot-reload, set `hot_reload = true` in `config/game_config.lua`.
