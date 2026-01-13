#ifndef RTYPE_ENGINE_COMPONENTS_UICHECKBOX_HPP
#define RTYPE_ENGINE_COMPONENTS_UICHECKBOX_HPP

#include <string>
#include <cstdint>

namespace Components {

    /**
     * @brief Checkbox component for boolean options (fullscreen, etc.)
     */
    struct UICheckbox {
        // State
        bool checked = false;
        
        // Label
        std::string label;
        bool labelOnRight = true;               // Label position
        float labelSpacing = 10.0f;
        
        // Visual dimensions
        float boxSize = 24.0f;
        
        // Colors (RGBA: 0xRRGGBBAA)
        uint32_t boxColor = 0x333333FF;
        uint32_t boxHoverColor = 0x444444FF;
        uint32_t borderColor = 0x888888FF;
        uint32_t borderHoverColor = 0x00FFFFFF;
        uint32_t checkColor = 0x00FFFFFF;       // Cyan checkmark
        
        // State
        bool isHovered = false;
        bool enabled = true;
        
        // Callbacks
        std::string onChangeCallback;
        
        // Sound
        std::string toggleSound;

        UICheckbox() = default;
        UICheckbox(const std::string& labelText, bool initialState = false)
            : checked(initialState), label(labelText) {}
    };

} // namespace Components

#endif // RTYPE_ENGINE_COMPONENTS_UICHECKBOX_HPP
