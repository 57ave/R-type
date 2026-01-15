# Configuration-Driven Shoot'em Up Module

## Philosophy

This module is now **100% generic** and contains **zero hardcoded game-specific enums or types**.

All game-specific behaviors are defined in **Lua configuration files**, making the module reusable for ANY shoot'em up game (R-Type, Gradius, Ikaruga, Touhou-style, etc.).

## How It Works

### ❌ Old Approach (Hardcoded)
```cpp
enum class WeaponType {
    SINGLE_SHOT,    // ← R-Type specific!
    LASER,          // ← Hardcoded in C++!
    HOMING_MISSILE  // ← Can't add new types without recompiling!
};
```

### ✅ New Approach (Configuration-Driven)
```cpp
struct Weapon {
    std::string weaponType = "single_shot";  // ← Defined in Lua!
    // ... generic properties
};
```

**Lua config:**
```lua
-- weapon_types.lua
return {
    single_shot = { ... },
    triple_laser = { ... },  -- Can add new types without touching C++!
    plasma_wave = { ... }
}
```

## Components Overview

### 1. Weapon Component
- **Type:** String-based (e.g., `"single_shot"`, `"laser"`, `"homing_missile"`)
- **Config:** `assets/scripts/config/weapon_types.lua`
- **Usage:** Define fire rate, damage, projectile behavior in Lua

```lua
-- Example: Custom weapon for your game
mega_beam = {
    fireRate = 0.1,
    projectileType = "mega_beam_bullet",
    projectileSpeed = 2000,
    damage = 10,
    supportsCharge = true
}
```

### 2. MovementPattern Component
- **Type:** String-based (e.g., `"straight"`, `"sine_wave"`, `"spiral"`)
- **Config:** `assets/scripts/config/movement_patterns.lua`
- **Usage:** Define enemy movement behaviors in Lua

```lua
-- Example: Custom boss pattern
boss_spiral_attack = {
    speed = 300,
    parameters = {
        amplitude = 200,
        frequency = 2.5,
        radiusChange = -30
    }
}
```

### 3. Attachment Component
- **Attachment Point:** String-based (e.g., `"center"`, `"left_wing"`, `"turret_1"`)
- **Config:** `assets/scripts/config/attachment_points.lua`
- **Usage:** Define where to attach weapons, options, decorations

```lua
-- Example: Custom ship with 4 weapon mounts
custom_fighter = {
    center = {x = 0, y = 0},
    weapon_mount_1 = {x = -40, y = -20},
    weapon_mount_2 = {x = -40, y = 20},
    weapon_mount_3 = {x = 40, y = -10},
    weapon_mount_4 = {x = 40, y = 10}
}
```

### 4. VisualAttachment Component
- **Visual Type:** String-based (e.g., `"cannon"`, `"laser_barrel"`, `"energy_orb"`)
- **Config:** `assets/scripts/config/visual_attachments.lua`
- **Usage:** Define weapon/decoration visual appearance

```lua
-- Example: Custom visual style
crystal_emitter = {
    sprite = "assets/weapons/crystal.png",
    animated = true,
    animationSpeed = 2.5,
    glowing = true,
    glowIntensity = 2.0,
    customData = {
        crystalColor = {100, 255, 200},
        particles = true
    }
}
```

## Benefits

### 🎮 For Game Developers
- **No C++ compilation needed** to add new weapons, patterns, or visuals
- **Hot-reload configs** during development (if engine supports it)
- **Easy balancing** by tweaking numbers in Lua files
- **Rapid prototyping** of new mechanics

### 🔧 For Engine Maintainers
- **100% generic module** works for any shoot'em up style
- **No hardcoded game logic** in C++ components
- **Easy to test** with different config sets
- **Modular and reusable**

### 🚀 For Other Projects
- **Drop-in module** for any shoot'em up game
- **Bring your own configs** - just replace the Lua files
- **No modifications needed** to use in different games

## Implementation Guide

### 1. Load Configurations at Runtime
```cpp
// In your game initialization
sol::state lua;
lua.open_libraries(sol::lib::base, sol::lib::table);

auto weaponTypes = lua.script_file("assets/scripts/config/weapon_types.lua");
auto movementPatterns = lua.script_file("assets/scripts/config/movement_patterns.lua");
// ... etc
```

### 2. Apply Configuration to Components
```cpp
// When creating a weapon
auto weaponConfig = weaponTypes["laser"];
weapon.weaponType = "laser";
weapon.fireRate = weaponConfig["fireRate"];
weapon.projectileSpeed = weaponConfig["projectileSpeed"];
weapon.damage = weaponConfig["damage"];
// ... etc
```

### 3. Use String-Based Logic in Systems
```cpp
// In WeaponSystem
void Update(float deltaTime) {
    for (auto& [entity, weapon] : weapons) {
        if (weapon.weaponType == "laser") {
            // Special laser logic
        } else if (weapon.weaponType == "homing_missile") {
            // Homing logic
        }
        // OR: Use Lua callbacks for custom behavior
    }
}
```

## Migration from Old Code

If you have existing code using hardcoded enums:

```cpp
// Old code:
if (weapon.weaponType == Weapon::Type::LASER) { ... }

// New code:
if (weapon.weaponType == "laser") { ... }
```

**Benefits:** Same performance (string comparison is fast for small strings), but infinite flexibility!

## Example: Creating a New Game

Want to make a bullet-hell game like Touhou instead of R-Type?

1. **Copy the shootemup module** (it's 100% generic!)
2. **Replace the Lua configs** with your own:
   - `weapon_types.lua` → Define danmaku patterns, spread shots, bombs
   - `movement_patterns.lua` → Define complex bullet-hell enemy patterns
   - `visual_attachments.lua` → Define your visual style
3. **Done!** No C++ changes needed.

## Performance Notes

- **String comparisons** are O(n) but very fast for short strings (< 20 chars)
- **Cache lookups** if performance-critical (use hash maps)
- **Consider enums** only if profiling shows string comparisons are a bottleneck
- In practice: String-based approach is **fast enough** for shoot'em ups (thousands of entities with no issues)

## Future Enhancements

- **Hot-reload support** for config files during gameplay
- **Lua callbacks** for fully custom behavior (no C++ system logic needed)
- **Visual editor** for creating configs without editing Lua files
- **Validation tools** to check config correctness at load time

## Conclusion

This approach makes the shoot'em up module **truly generic and reusable**. Any shoot'em up game can use it by simply providing their own configuration files. No more hardcoded R-Type-specific code in the engine!
