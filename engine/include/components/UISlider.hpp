#ifndef ENG_ENGINE_COMPONENTS_UISLIDER_HPP
#define ENG_ENGINE_COMPONENTS_UISLIDER_HPP

#include <string>
#include <cstdint>

namespace Components {

    /**
     * @brief Slider component for adjustable values (volume, brightness, etc.)
     */
    struct UISlider {
        // Value range
        float minValue = 0.0f;
        float maxValue = 100.0f;
        float currentValue = 50.0f;
        float step = 1.0f;                     // Value increment for keyboard
        
        // Visual dimensions
        float trackHeight = 8.0f;
        float handleWidth = 20.0f;
        float handleHeight = 30.0f;
        
        // Colors (RGBA: 0xRRGGBBAA)
        uint32_t trackColor = 0x333333FF;      // Dark gray track
        uint32_t trackFillColor = 0x00FFFFFF;  // Cyan for filled portion
        uint32_t handleColor = 0xFFFFFFFF;     // White handle
        uint32_t handleHoverColor = 0x00FFFFFF;// Cyan on hover
        
        // State
        bool isDragging = false;
        bool isHovered = false;
        
        // Callbacks
        std::string onChangeCallback;          // Called when value changes
        std::string onReleaseCallback;         // Called when drag ends
        
        // Display value
        bool showValue = true;
        std::string valueFormat = "%.0f";      // Printf format for value display
        std::string suffix = "%";              // Suffix after value (e.g., "50%")
        
        // Label
        std::string label;                     // Optional label (e.g., "Volume")

        UISlider() = default;
        UISlider(float min, float max, float current, const std::string& callback = "")
            : minValue(min), maxValue(max), currentValue(current), onChangeCallback(callback) {}
        
        // Helper to get normalized value (0.0 - 1.0)
        float getNormalized() const {
            if (maxValue <= minValue) return 0.0f;
            return (currentValue - minValue) / (maxValue - minValue);
        }
        
        // Helper to set from normalized value
        void setFromNormalized(float normalized) {
            currentValue = minValue + normalized * (maxValue - minValue);
            // Clamp
            if (currentValue < minValue) currentValue = minValue;
            if (currentValue > maxValue) currentValue = maxValue;
        }
    };

} // namespace Components

#endif // ENG_ENGINE_COMPONENTS_UISLIDER_HPP
