#ifndef RTYPE_ENGINE_SHOOTEMUP_COMPONENTS_AICONTROLLER_HPP
#define RTYPE_ENGINE_SHOOTEMUP_COMPONENTS_AICONTROLLER_HPP

#include <string>

/**
 * @file AIController.hpp
 * @brief Generic AI controller for shoot'em up enemies
 * 
 * Provides common AI patterns found in shoot'em up games.
 */

namespace ShootEmUp {
namespace Components {

    /**
     * @brief AIController component - Enemy behavior patterns
     * Generic AI patterns that can be used in any shoot'em up
     */
    struct AIController {
        std::string pattern;  // "straight", "zigzag", "circle", "dive", "sine_wave", etc.
        float timer;
        float shootTimer;
        float shootInterval;

        // Pattern-specific data
        float centerX, centerY, circleRadius;
        float targetY;
        float amplitude;        // For wave patterns
        float frequency;        // For wave patterns

        AIController() 
            : pattern("straight"), timer(0), shootTimer(0), shootInterval(2.0f),
              centerX(0), centerY(0), circleRadius(100),
              targetY(300), amplitude(50.0f), frequency(1.0f) {}
    };

} // namespace Components
} // namespace ShootEmUp

// Convenience alias
using ShootEmUpAIController = ShootEmUp::Components::AIController;

#endif // RTYPE_ENGINE_SHOOTEMUP_COMPONENTS_AICONTROLLER_HPP
