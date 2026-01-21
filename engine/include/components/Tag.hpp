#ifndef ENG_ENGINE_COMPONENTS_TAG_HPP
#define ENG_ENGINE_COMPONENTS_TAG_HPP

#include <string>

/**
 * @brief Generic Tag component for categorizing entities
 *
 * This is a GENERIC component that can be used in any game.
 * Use it to mark entities with string identifiers.
 *
 * Examples for ANY game:
 *   Tag{"player"}, Tag{"enemy"}, Tag{"projectile"}
 *   Tag{"npc"}, Tag{"obstacle"}, Tag{"collectible"}
 *   Tag{"boss"}, Tag{"vehicle"}, Tag{"door"}
 */
struct Tag {
    std::string name = "entity";

    Tag() = default;
    Tag(const std::string& n) : name(n) {}

    bool operator==(const std::string& other) const { return name == other; }
    bool operator!=(const std::string& other) const { return name != other; }
};

#endif  // ENG_ENGINE_COMPONENTS_TAG_HPP
