#ifndef RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP
#define RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP

#include <string>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Generic movement pattern component for shoot'em up entities
 *
 * All pattern types are defined as strings (configured in Lua).
 * The system implementing this should read the pattern name and apply the appropriate logic.
 */
struct MovementPattern {
    // Pattern type identifier (defined in Lua config)
    // Examples: "straight", "sine_wave", "zigzag", "circular", "diagonal_down", etc.
    std::string patternType = "straight";

    // Pattern parameters (generic, used by various patterns)
    float speed = 300.0f;
    float amplitude = 100.0f;  // For wave patterns
    float frequency = 2.0f;    // For wave patterns
    float timeAlive = 0.0f;    // Time since spawn

    // Starting position (for pattern calculations)
    float startX = 0.0f;
    float startY = 0.0f;

    // Custom properties (can be read from Lua for game-specific patterns)
    std::string customData = "";  // JSON or custom format for extra parameters
};

/**
 * @brief Generic AI controller for shoot'em up entities
 *
 * AI types are defined as strings (configured in Lua).
 */
struct EnemyAI {
    // AI type identifier (defined in Lua config)
    // Examples: "basic", "aggressive", "defensive", "kamikaze", "sniper", etc.
    std::string aiType = "basic";

    float aggroRange = 500.0f;
    bool isActive = true;

    // Custom properties for game-specific AI behaviors
    std::string customData = "";
};

}  // namespace Components
}  // namespace ShootEmUp

#endif  // RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP
