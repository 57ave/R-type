#ifndef ENG_ENGINE_COMPONENTS_COLLIDER_HPP
#define ENG_ENGINE_COMPONENTS_COLLIDER_HPP

#include <string>

struct Collider {
    float width = 0.0f;
    float height = 0.0f;
    float offsetX = 0.0f;          // Offset from position
    float offsetY = 0.0f;
    bool isTrigger = false;        // Trigger-only (no physics response)

    // Collision layers/tags for filtering
    std::string tag = "default";   // e.g., "player", "enemy", "bullet"

    bool enabled = true;
};

struct Hitbox {
    bool visible = false;
    float outlineThickness = 2.0f;
    // Color stored as RGBA
    unsigned char r = 255;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 255;
};

#endif // ENG_ENGINE_COMPONENTS_COLLIDER_HPP
