#ifndef RTYPE_ENGINE_COMPONENTS_ANIMATION_HPP
#define RTYPE_ENGINE_COMPONENTS_ANIMATION_HPP

#include <rendering/Types.hpp>
#include <vector>

struct Animation {
    float frameTime = 0.1f;        // Time per frame
    float currentTime = 0.0f;      // Accumulated time
    int currentFrame = 0;          // Current frame index
    int frameCount = 1;            // Total number of frames
    bool loop = true;              // Should animation loop?
    bool finished = false;         // Has animation finished?

    // Spritesheet layout
    int frameWidth = 0;            // Width of one frame
    int frameHeight = 0;           // Height of one frame
    int startX = 0;                // Start X position in spritesheet
    int startY = 0;                // Start Y position in spritesheet
    int spacing = 0;               // Spacing between frames
};

struct StateMachineAnimation {
    int currentColumn = 2;         // Current animation column
    int targetColumn = 2;          // Target animation column
    float transitionSpeed = 0.15f; // Speed of transition between states
    float transitionTime = 0.0f;   // Accumulated transition time

    int spriteWidth = 33;
    int spriteHeight = 17;
    int currentRow = 0;            // Current row in spritesheet
};

/**
 * @brief Charge animation (for charged shots)
 */
struct ChargeAnimation {
    float chargeTime = 0.0f;       // Time charging
    float maxChargeTime = 1.0f;    // Max charge time
    int chargeLevel = 0;           // Current charge level (0-5)
    bool isCharging = false;       // Is currently charging
    bool fullyCharged = false;     // Has reached max charge

    // Animation data per charge level
    struct ChargeLevelData {
        int xPos;
        int yPos;
        int width;
        int height;
    };
    std::vector<ChargeLevelData> chargeLevels;
};

#endif // RTYPE_ENGINE_COMPONENTS_ANIMATION_HPP
