#ifndef ENG_ENGINE_COMPONENTS_UITEXT_HPP
#define ENG_ENGINE_COMPONENTS_UITEXT_HPP

#include <cstdint>
#include <string>

namespace Components {

/**
 * @brief Text display component for UI
 *
 * Holds text content, font settings, and styling information.
 */
struct UIText {
    // Content
    std::string content;

    // Font reference (loaded font identifier)
    std::string fontId = "default";

    // Size
    unsigned int fontSize = 24;

    // Colors (RGBA format: 0xRRGGBBAA)
    uint32_t color = 0xFFFFFFFF;         // White
    uint32_t outlineColor = 0x000000FF;  // Black
    float outlineThickness = 0.0f;

    // Alignment
    enum class Alignment { Left, Center, Right };
    Alignment alignment = Alignment::Center;

    // Effects
    bool shadow = false;
    float shadowOffsetX = 2.0f;
    float shadowOffsetY = 2.0f;
    uint32_t shadowColor = 0x000000AA;

    // Animation (for pulsating effects)
    bool pulsating = false;
    float pulseSpeed = 2.0f;
    float pulseMinScale = 0.95f;
    float pulseMaxScale = 1.05f;
    float currentPulseTime = 0.0f;

    UIText() = default;
    UIText(const std::string& text, unsigned int size = 24, uint32_t col = 0xFFFFFFFF)
        : content(text), fontSize(size), color(col) {}
};

}  // namespace Components

#endif  // ENG_ENGINE_COMPONENTS_UITEXT_HPP
