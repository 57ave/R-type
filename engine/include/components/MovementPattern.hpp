#ifndef RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP
#define RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP

#include <string>

struct MovementPattern {
    enum class Type {
        STRAIGHT,      // Straight line from right to left
        SINE_WAVE,     // Sinusoidal wave
        ZIGZAG,        // Zigzag up/down
        CIRCULAR,      // Circular motion
        DIAGONAL_DOWN, // Diagonal downward
        DIAGONAL_UP    // Diagonal upward
    };

    Type pattern = Type::STRAIGHT;

    // Pattern parameters
    float speed = 300.0f;
    float amplitude = 100.0f;      // For wave patterns
    float frequency = 2.0f;        // For wave patterns
    float timeAlive = 0.0f;        // Time since spawn

    // Starting position (for pattern calculations)
    float startX = 0.0f;
    float startY = 0.0f;
};

struct EnemyAI {
    std::string aiType = "basic";  // "basic", "aggressive", "defensive", etc.
    float aggroRange = 500.0f;
    bool isActive = true;
};

#endif // RTYPE_ENGINE_COMPONENTS_MOVEMENTPATTERN_HPP
