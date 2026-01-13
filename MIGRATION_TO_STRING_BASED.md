# MIGRATION GUIDE: From Hardcoded Enums to String-Based Configuration

## 🎯 What Changed

The shoot'em up module has been refactored to remove **all hardcoded game-specific enums** and replace them with **string-based configuration** loaded from Lua files.

This makes the module **100% reusable** for any shoot'em up game!

## ⚠️ Breaking Changes

### 1. MovementPattern Component

#### ❌ OLD (Hardcoded Enum):
```cpp
#include <components/MovementPattern.hpp>

MovementPattern pattern;
pattern.pattern = MovementPattern::Type::SINE_WAVE;  // ❌ Enum!
```

#### ✅ NEW (String-Based):
```cpp
#include <shootemup/components/MovementPattern.hpp>
using namespace ShootEmUp::Components;

MovementPattern pattern;
pattern.patternType = "sine_wave";  // ✅ String!
```

### 2. Weapon Component

#### ❌ OLD (Hardcoded Enum):
```cpp
#include <components/Weapon.hpp>

Weapon weapon;
weapon.weaponType = Weapon::Type::LASER;  // ❌ Enum!
```

#### ✅ NEW (String-Based):
```cpp
#include <shootemup/components/Weapon.hpp>
using namespace ShootEmUp::Components;

Weapon weapon;
weapon.weaponType = "laser";  // ✅ String!
```

### 3. Attachment Component

#### ❌ OLD (Hardcoded Enum):
```cpp
Attachment attach;
attach.point = Attachment::AttachmentPoint::LEFT_WING;  // ❌ Enum!

WeaponAttachment visual;
visual.visualType = WeaponAttachment::VisualType::CANNON;  // ❌ Enum!
```

#### ✅ NEW (String-Based):
```cpp
using namespace ShootEmUp::Components;

Attachment attach;
attach.attachmentPoint = "left_wing";  // ✅ String!

VisualAttachment visual;
visual.visualType = "cannon";  // ✅ String!
```

## 🔧 Migration Steps for Your Code

### Step 1: Update Includes
```cpp
// OLD:
#include <components/MovementPattern.hpp>
#include <components/Weapon.hpp>
#include <components/Attachment.hpp>

// NEW:
#include <shootemup/components/MovementPattern.hpp>
#include <shootemup/components/Weapon.hpp>
#include <shootemup/components/Attachment.hpp>

using namespace ShootEmUp::Components;
```

### Step 2: Replace Enum Usage with Strings

#### Pattern A: Switch Statements → If-Else Chains
```cpp
// OLD:
switch (pattern.pattern) {
    case MovementPattern::Type::STRAIGHT:
        // ...
        break;
    case MovementPattern::Type::SINE_WAVE:
        // ...
        break;
}

// NEW:
if (pattern.patternType == "straight") {
    // ...
} else if (pattern.patternType == "sine_wave") {
    // ...
}
```

#### Pattern B: Function Parameters
```cpp
// OLD:
ECS::Entity CreateEnemy(float x, float y, MovementPattern::Type pattern);

// NEW:
ECS::Entity CreateEnemy(float x, float y, const std::string& patternType);
```

#### Pattern C: Array Initialization
```cpp
// OLD:
MovementPattern::Type patterns[] = {
    MovementPattern::Type::STRAIGHT,
    MovementPattern::Type::SINE_WAVE,
    MovementPattern::Type::ZIGZAG
};

// NEW:
const char* patterns[] = {
    "straight",
    "sine_wave",
    "zigzag"
};
// Or better:
std::vector<std::string> patterns = {
    "straight", "sine_wave", "zigzag"
};
```

### Step 3: Update Factory Code

```cpp
// OLD:
weapon.weaponType = Weapon::Type::SINGLE_SHOT;

// NEW:
weapon.weaponType = "single_shot";
```

### Step 4: Load Configuration from Lua (Optional but Recommended)

```cpp
// In your game initialization
sol::state lua;
lua.open_libraries(sol::lib::base, sol::lib::table);

// Load weapon types config
auto weaponConfigs = lua.script_file("assets/scripts/config/weapon_types.lua");

// Apply config to weapon component
void ConfigureWeapon(Weapon& weapon, const std::string& type) {
    auto config = weaponConfigs[type];
    weapon.weaponType = type;
    weapon.fireRate = config["fireRate"];
    weapon.projectileSpeed = config["projectileSpeed"];
    weapon.damage = config["damage"];
    // ... etc
}
```

## 📋 Files That Need Migration

### In game/src/:
- [x] **Game.cpp** - Lines 147-200 (CreateEnemy function) ← **TODO**
- [ ] **factories/*.cpp** - All factory implementations
- [ ] **FACTORY_EXAMPLES.cpp** - Example code using old enums

### In engine/modules/shootemup/src/systems/:
- [x] **MovementPatternSystem.cpp** - ✅ Already migrated
- [x] **EnemySpawnSystem.cpp** - ✅ Already migrated  
- [x] **WeaponSystem.cpp** - ✅ Already using strings

## 🎮 Configuration Files Created

New Lua configuration files (game-specific, not in engine):

```
assets/scripts/config/
├── weapon_types.lua          ← Define all weapon behaviors
├── movement_patterns.lua     ← Define all movement patterns
├── attachment_points.lua     ← Define attachment positions
└── visual_attachments.lua    ← Define visual styles
```

These files define **R-Type specific** behaviors. For a different game (e.g., Touhou-style bullet hell), you would create different configs.

## 🚀 Benefits

### Before (Hardcoded):
- ❌ Adding new weapon = Modify C++ enum + Recompile entire project
- ❌ Tweaking patterns = Edit C++ code + Recompile
- ❌ Module tied to R-Type specifics
- ❌ Can't be reused for other games

### After (String-Based):
- ✅ Adding new weapon = Edit Lua config file (no recompile!)
- ✅ Tweaking patterns = Edit Lua config (hot-reload possible)
- ✅ Module is 100% generic and reusable
- ✅ Works for any shoot'em up style

## 📝 Quick Reference

### Available Pattern Types (in Lua config):
- `"straight"` - Straight line movement
- `"sine_wave"` - Sinusoidal wave
- `"zigzag"` - Sharp zigzag
- `"circular"` - Circular motion
- `"diagonal_down"` - Diagonal downward
- `"diagonal_up"` - Diagonal upward
- `"spiral"` - Spiral pattern
- `"figure_eight"` - Figure-8 pattern
- `"charge_player"` - Charge at player
- `"follow_player"` - Track player
- ... *add your own in Lua!*

### Available Weapon Types (in Lua config):
- `"single_shot"` - Basic single projectile
- `"double_shot"` - Two parallel projectiles
- `"triple_shot"` - Three projectiles
- `"spread_shot"` - Wide spread
- `"laser"` - Continuous laser
- `"homing_missile"` - Heat-seeking
- `"wave_beam"` - Oscillating wave
- `"charge_cannon"` - Charged shot
- ... *add your own in Lua!*

## 🐛 Common Migration Issues

### Issue 1: Enum Comparison Doesn't Compile
```
error: no match for 'operator==' (operand types are 'MovementPattern::Type' and 'const char*')
```
**Fix:** Replace enum with string in your code.

### Issue 2: Missing Namespace
```
error: 'MovementPattern' was not declared in this scope
```
**Fix:** Add `using namespace ShootEmUp::Components;` or use `ShootEmUp::Components::MovementPattern`.

### Issue 3: Old Include Paths
```
fatal error: components/MovementPattern.hpp: No such file or directory
```
**Fix:** Change to `#include <shootemup/components/MovementPattern.hpp>`.

## 🎯 Next Steps

1. **Migrate `game/src/Game.cpp`** - Update CreateEnemy() function
2. **Migrate all factories** in `game/src/factories/`
3. **Test compilation** - `cmake --build . --target r-type_game`
4. **Load Lua configs** at runtime (optional but recommended)
5. **Test gameplay** - Verify patterns/weapons work correctly

## ❓ Questions?

- **Q: Will this be slower than enums?**
  - A: String comparison is O(n) but very fast for short strings. Negligible performance impact for shoot'em ups. If needed, cache comparisons.

- **Q: Can I still use enums in my game code?**
  - A: Yes! Your game-specific code in `game/` can use enums internally. Just convert to strings when interfacing with the module.

- **Q: Do I have to use Lua configs?**
  - A: No! You can hardcode the strings in your C++ if you prefer. The module just uses strings, not Lua.

---

**Status:** Module components are migrated ✅  
**Remaining:** Game code using the module needs migration ⚠️
