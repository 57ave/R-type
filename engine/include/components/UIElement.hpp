#ifndef RTYPE_ENGINE_COMPONENTS_UIELEMENT_HPP
#define RTYPE_ENGINE_COMPONENTS_UIELEMENT_HPP

#include <string>
#include <cstdint>

namespace Components {

    /**
     * @brief Base component for all UI elements
     * 
     * Contains common properties like position, size, visibility, and layer.
     * UI elements are typically rendered above game content (layer 100+).
     */
    struct UIElement {
        // Position (screen coordinates)
        float x = 0.0f;
        float y = 0.0f;
        
        // Size
        float width = 0.0f;
        float height = 0.0f;
        
        // Rendering layer (higher = on top)
        int layer = 100;
        
        // State
        bool visible = true;
        bool interactable = true;
        
        // Unique identifier for Lua/script reference
        std::string id;
        
        // Menu group (for showing/hiding entire menus)
        std::string menuGroup;
        
        // Tab index for keyboard navigation (-1 = not navigable)
        int tabIndex = -1;

        UIElement() = default;
        UIElement(float x, float y, float w, float h, const std::string& id = "")
            : x(x), y(y), width(w), height(h), id(id) {}
    };

} // namespace Components

#endif // RTYPE_ENGINE_COMPONENTS_UIELEMENT_HPP
