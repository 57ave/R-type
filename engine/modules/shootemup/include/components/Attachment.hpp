#ifndef SHOOTEMUP_COMPONENTS_ATTACHMENT_HPP
#define SHOOTEMUP_COMPONENTS_ATTACHMENT_HPP

#include <ecs/ECS.hpp>
#include <string>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Generic component for attaching one entity to another (parent-child)
 * 
 * Used for visual attachments, options, effects following entities, etc.
 * All specific behaviors are configured through strings (defined in Lua).
 */
struct Attachment {
    ECS::Entity parent = 0;        // Parent entity
    
    // Relative position offset from parent
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    
    // Relative rotation (in degrees)
    float rotation = 0.0f;
    
    // Named attachment point (configurable in Lua)
    // Examples: "center", "left_wing", "right_wing", "top", "bottom", "custom"
    std::string attachmentPoint = "custom";
    
    // Behavior flags
    bool inheritRotation = true;   // Follows parent rotation
    bool inheritScale = true;      // Follows parent scale
    bool destroyWithParent = true; // Destroyed when parent is destroyed
};

/**
 * @brief Generic component for visual attachments (weapons, decorations, etc.)
 * 
 * All types are defined as strings, configured in Lua scripts.
 */
struct VisualAttachment {
    // Type identifier (defined in Lua config)
    // Examples: "cannon", "double_cannon", "laser_barrel", "missile_pod", etc.
    std::string visualType = "default";
    
    int level = 1;                 // Level (affects appearance)
    
    // Animation
    bool animated = false;         // Has animation
    float animationSpeed = 1.0f;   // Animation speed multiplier
    
    // Visual effects
    bool glowing = false;          // Glow effect enabled
    float glowIntensity = 1.0f;    // Glow intensity
    
    // Custom properties (can be read from Lua)
    std::string customData = "";   // JSON or custom format for extra data
};

} // namespace Components
} // namespace ShootEmUp

#endif // SHOOTEMUP_COMPONENTS_ATTACHMENT_HPP
