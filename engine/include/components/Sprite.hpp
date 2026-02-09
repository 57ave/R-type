#ifndef ENG_ENGINE_COMPONENTS_SPRITE_HPP
#define ENG_ENGINE_COMPONENTS_SPRITE_HPP

#include <string>
#include <rendering/ISprite.hpp>
#include <rendering/Types.hpp>

struct Sprite {
    eng::engine::rendering::ISprite *sprite = nullptr; // non-owning
    eng::engine::rendering::IntRect textureRect; // optional sub-rect
    int layer = 0; // rendering layer/order
    std::string texturePath; // optional: hint to load resource
    float scaleX = 3.0f; // horizontal scale (default 3.0)
    float scaleY = 3.0f; // vertical scale (default 3.0)
};

#endif // ENG_ENGINE_COMPONENTS_SPRITE_HPP
