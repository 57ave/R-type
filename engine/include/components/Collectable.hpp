#ifndef ENG_ENGINE_COMPONENTS_COLLECTABLE_HPP
#define ENG_ENGINE_COMPONENTS_COLLECTABLE_HPP

#include <string>

/**
 * @brief Collectable component for items that can be picked up
 * 
 * Used for power-ups, modules, and other collectible items.
 */
struct Collectable {
    std::string type = "unknown";  // "powerup_orange", "powerup_blue", "module_laser", etc.
    bool pickedUp = false;         // Has this item been collected?
    float floatSpeed = 50.0f;      // Speed of floating motion
    float floatAmplitude = 10.0f;  // Amplitude of floating motion
    float floatTime = 0.0f;        // Current time in float cycle
    
    Collectable() = default;
    explicit Collectable(const std::string& t) : type(t) {}
};

#endif // ENG_ENGINE_COMPONENTS_COLLECTABLE_HPP
