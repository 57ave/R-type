#ifndef ENG_ENGINE_COMPONENTS_UIPANEL_HPP
#define ENG_ENGINE_COMPONENTS_UIPANEL_HPP

#include <algorithm>
#include <cstdint>
#include <ecs/Types.hpp>
#include <string>
#include <vector>

namespace Components {

/**
 * @brief Container panel for grouping UI elements
 *
 * Panels can have backgrounds, borders, and can be modal (blocking input below).
 */
struct UIPanel {
    // Child entities (managed by UISystem)
    std::vector<ECS::Entity> children;

    // Visual
    uint32_t backgroundColor = 0x000000AA;  // Semi-transparent black
    uint32_t borderColor = 0x00FFFFFFAA;    // Cyan border
    float borderThickness = 0.0f;
    float cornerRadius = 0.0f;  // For rounded corners (if supported)

    // Padding (internal spacing)
    float paddingTop = 10.0f;
    float paddingBottom = 10.0f;
    float paddingLeft = 10.0f;
    float paddingRight = 10.0f;

    // Modal behavior
    bool modal = false;                // If true, blocks input to elements below
    bool closeOnClickOutside = false;  // Close panel when clicking outside

    // Animation
    enum class Animation {
        None,
        FadeIn,
        FadeOut,
        SlideFromTop,
        SlideFromBottom,
        SlideFromLeft,
        SlideFromRight,
        Scale
    };
    Animation currentAnimation = Animation::None;
    float animationProgress = 1.0f;  // 0.0 to 1.0
    float animationDuration = 0.3f;  // Seconds

    // Scrolling (for content larger than panel)
    bool scrollable = false;
    float scrollOffsetY = 0.0f;
    float contentHeight = 0.0f;  // Total height of content

    // Title bar (optional)
    bool showTitleBar = false;
    std::string title;
    float titleBarHeight = 40.0f;
    uint32_t titleBarColor = 0x333333FF;
    bool draggable = false;  // Can drag by title bar

    // Close button
    bool showCloseButton = false;
    std::string onCloseCallback;

    UIPanel() = default;
    UIPanel(uint32_t bgColor, bool isModal = false) : backgroundColor(bgColor), modal(isModal) {}

    // Add child entity
    void addChild(ECS::Entity entity) { children.push_back(entity); }

    // Remove child entity
    void removeChild(ECS::Entity entity) {
        children.erase(std::remove(children.begin(), children.end(), entity), children.end());
    }
};

}  // namespace Components

#endif  // ENG_ENGINE_COMPONENTS_UIPANEL_HPP
