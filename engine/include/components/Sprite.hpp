#ifndef RTYPE_ENGINE_COMPONENTS_SPRITE_HPP
#define RTYPE_ENGINE_COMPONENTS_SPRITE_HPP

#include <string>
#include <rendering/ISprite.hpp>
#include <rendering/Types.hpp>

struct Sprite {
    rtype::engine::rendering::ISprite *sprite = nullptr; // non-owning
    rtype::engine::rendering::IntRect textureRect; // optional sub-rect
    int layer = 0; // rendering layer/order
    std::string texturePath; // optional: hint to load resource
};

#endif // RTYPE_ENGINE_COMPONENTS_SPRITE_HPP
