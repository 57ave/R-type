#ifndef ENG_ENGINE_COMPONENTS_UIBUTTON_HPP
#define ENG_ENGINE_COMPONENTS_UIBUTTON_HPP

#include <cstdint>
#include <string>

namespace Components {

/**
 * @brief Interactive button component for UI
 *
 * Supports multiple visual states and callback on click.
 */
struct UIButton {
    // Button text (can also use separate UIText component)
    std::string text;

    // Visual states
    enum class State {
        Normal,
        Hovered,
        Pressed,
        Disabled,
        Selected  // For keyboard navigation
    };
    State state = State::Normal;

    // Colors for each state (RGBA: 0xRRGGBBAA)
    uint32_t normalColor = 0xFFFFFFFF;
    uint32_t hoverColor = 0x00FFFFFF;     // Cyan
    uint32_t pressedColor = 0x00AAFFFF;   // Light cyan
    uint32_t disabledColor = 0x888888FF;  // Gray
    uint32_t selectedColor = 0xFFFF00FF;  // Yellow (keyboard selection)

    // Background colors
    uint32_t bgNormalColor = 0x00000000;  // Transparent
    uint32_t bgHoverColor = 0x333333AA;   // Semi-transparent dark
    uint32_t bgPressedColor = 0x222222CC;
    uint32_t bgSelectedColor = 0x444444AA;

    // Border
    uint32_t borderColor = 0x00FFFFFF;  // Cyan border
    float borderThickness = 0.0f;
    float borderHoverThickness = 2.0f;

    // Callback (Lua function name or C++ callback identifier)
    std::string onClickCallback;
    std::string onHoverCallback;
    std::string onUnhoverCallback;

    // Selection state (for keyboard navigation)
    bool selected = false;

    // Enable/disable
    bool enabled = true;

    // Sound effects (audio asset IDs, can be empty)
    std::string hoverSound;
    std::string clickSound;

    // Animation
    float hoverScale = 1.05f;  // Scale when hovered
    float currentScale = 1.0f;
    float scaleSpeed = 10.0f;  // Interpolation speed

    // Glow effect for selected button
    bool glowWhenSelected = true;
    float glowIntensity = 0.0f;
    float glowSpeed = 3.0f;

    UIButton() = default;
    UIButton(const std::string& buttonText, const std::string& callback = "")
        : text(buttonText), onClickCallback(callback) {}
};

}  // namespace Components

#endif  // ENG_ENGINE_COMPONENTS_UIBUTTON_HPP
