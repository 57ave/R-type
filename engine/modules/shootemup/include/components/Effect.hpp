#ifndef SHOOTEMUP_COMPONENTS_EFFECT_HPP
#define SHOOTEMUP_COMPONENTS_EFFECT_HPP

#include <string>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Generic visual effect component for shoot'em up games
 * 
 * All effect types are defined as strings (configured in Lua).
 * Examples: "muzzle_flash", "explosion", "charge_effect", "particle", "spark", etc.
 */
struct Effect {
    // Effect type identifier (defined in Lua config)
    // Examples: "muzzle_flash", "explosion_small", "explosion_large", "charge", "spark", "smoke"
    std::string effectType = "generic";
    
    // Positioning
    bool followParent = false;      // Follow parent entity?
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    
    // Animation/Lifetime
    float duration = 1.0f;          // Effect duration in seconds
    bool loop = false;              // Loop animation
    
    // Custom properties (can be read from Lua)
    std::string customData = "";
};

} // namespace Components
} // namespace ShootEmUp

#endif // SHOOTEMUP_COMPONENTS_EFFECT_HPP
