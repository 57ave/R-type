#ifndef ENG_ENGINE_COMPONENTS_UIDROPDOWN_HPP
#define ENG_ENGINE_COMPONENTS_UIDROPDOWN_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace Components {

/**
 * @brief Dropdown/Select component for choosing from a list of options
 */
struct UIDropdown {
    // Options
    std::vector<std::string> options;
    int selectedIndex = 0;

    // State
    bool isOpen = false;
    bool isHovered = false;
    int hoveredOptionIndex = -1;

    // Visual
    float optionHeight = 35.0f;
    float maxDropdownHeight = 200.0f;  // Max height before scrolling

    // Colors (RGBA: 0xRRGGBBAA)
    uint32_t backgroundColor = 0x222222FF;
    uint32_t hoverBackgroundColor = 0x333333FF;
    uint32_t selectedBackgroundColor = 0x00666688;
    uint32_t textColor = 0xFFFFFFFF;
    uint32_t borderColor = 0x444444FF;
    uint32_t borderOpenColor = 0x00FFFFFF;
    float borderThickness = 2.0f;

    // Arrow indicator
    bool showArrow = true;
    uint32_t arrowColor = 0xFFFFFFFF;

    // Callbacks
    std::string onChangeCallback;

    // Label
    std::string label;

    // Scrolling
    float scrollOffset = 0.0f;

    UIDropdown() = default;
    UIDropdown(const std::vector<std::string>& opts, int selected = 0)
        : options(opts), selectedIndex(selected) {}

    // Get currently selected option text
    std::string getSelectedText() const {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(options.size())) {
            return options[selectedIndex];
        }
        return "";
    }

    // Add option
    void addOption(const std::string& option) { options.push_back(option); }

    // Select by text
    bool selectByText(const std::string& text) {
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i] == text) {
                selectedIndex = static_cast<int>(i);
                return true;
            }
        }
        return false;
    }
};

}  // namespace Components

#endif  // ENG_ENGINE_COMPONENTS_UIDROPDOWN_HPP
