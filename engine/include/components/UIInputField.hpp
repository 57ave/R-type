#ifndef ENG_ENGINE_COMPONENTS_UIINPUTFIELD_HPP
#define ENG_ENGINE_COMPONENTS_UIINPUTFIELD_HPP

#include <string>
#include <cstdint>

namespace Components {

    /**
     * @brief Text input field component for user text entry
     */
    struct UIInputField {
        // Content
        std::string text;
        std::string placeholder = "Enter text...";
        
        // Constraints
        size_t maxLength = 32;
        bool numbersOnly = false;
        bool alphanumericOnly = false;
        std::string allowedCharacters;         // If set, only these chars allowed
        
        // Visual
        uint32_t textColor = 0xFFFFFFFF;
        uint32_t placeholderColor = 0x888888FF;
        uint32_t backgroundColor = 0x222222FF;
        uint32_t borderColor = 0x444444FF;
        uint32_t focusBorderColor = 0x00FFFFFF;
        float borderThickness = 2.0f;
        float padding = 10.0f;
        
        // Cursor
        bool isFocused = false;
        size_t cursorPosition = 0;
        bool showCursor = true;
        float cursorBlinkRate = 0.5f;          // Seconds per blink
        float cursorBlinkTimer = 0.0f;
        bool cursorVisible = true;
        
        // Selection (for future copy/paste)
        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        bool hasSelection = false;
        
        // Callbacks
        std::string onChangeCallback;          // Called when text changes
        std::string onSubmitCallback;          // Called when Enter is pressed
        std::string onFocusCallback;
        std::string onBlurCallback;
        
        // Password mode
        bool isPassword = false;
        char passwordChar = '*';

        UIInputField() = default;
        UIInputField(const std::string& placeholderText, size_t maxLen = 32)
            : placeholder(placeholderText), maxLength(maxLen) {}
        
        // Get display text (handles password mode)
        std::string getDisplayText() const {
            if (isPassword && !text.empty()) {
                return std::string(text.length(), passwordChar);
            }
            return text.empty() ? placeholder : text;
        }
        
        // Check if showing placeholder
        bool isShowingPlaceholder() const {
            return text.empty() && !isFocused;
        }
    };

} // namespace Components

#endif // ENG_ENGINE_COMPONENTS_UIINPUTFIELD_HPP
