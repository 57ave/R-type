#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <algorithm>
#include <cmath>
#include <components/UIButton.hpp>
#include <components/UICheckbox.hpp>
#include <components/UIDropdown.hpp>
#include <components/UIElement.hpp>
#include <components/UIInputField.hpp>
#include <components/UIPanel.hpp>
#include <components/UISlider.hpp>
#include <components/UIText.hpp>
#include <ecs/Coordinator.hpp>
#include <iostream>
#include <rendering/sfml/SFMLFont.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <systems/UISystem.hpp>

// Helper function to convert color from uint32_t (0xRRGGBBAA) to sf::Color
static sf::Color toSFColor(uint32_t color) {
    return sf::Color((color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

// ============================================
// PRIVATE HELPER METHODS FOR DIRECT SFML RENDERING
// ============================================

void UISystem::DrawRect(float x, float y, float width, float height, uint32_t fillColor,
                        uint32_t outlineColor, float outlineThickness) {
    if (!m_window)
        return;

    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(x, y);
    rect.setFillColor(toSFColor(fillColor));
    if (outlineThickness > 0) {
        rect.setOutlineColor(toSFColor(outlineColor));
        rect.setOutlineThickness(outlineThickness);
    }
    m_window->getSFMLWindow().draw(rect);
}

void UISystem::DrawText(const std::string& text, float x, float y, unsigned int fontSize,
                        uint32_t color, TextAlign align, const std::string& fontId) {
    if (!m_window)
        return;

    auto* fontPtr = GetFont(fontId);
    if (!fontPtr)
        fontPtr = GetFont("default");
    if (!fontPtr)
        return;

    // Cast to SFMLFont to get the underlying sf::Font
    auto* sfmlFont = dynamic_cast<eng::engine::rendering::sfml::SFMLFont*>(fontPtr);
    if (!sfmlFont)
        return;

    sf::Text sfText;
    sfText.setFont(sfmlFont->getNativeFont());
    sfText.setString(text);
    sfText.setCharacterSize(fontSize);
    sfText.setFillColor(toSFColor(color));

    // Apply alignment
    sf::FloatRect bounds = sfText.getLocalBounds();
    float offsetX = 0;
    if (align == TextAlign::Center) {
        offsetX = -bounds.width / 2.0f;
    } else if (align == TextAlign::Right) {
        offsetX = -bounds.width;
    }

    sfText.setPosition(x + offsetX - bounds.left, y - bounds.top);
    m_window->getSFMLWindow().draw(sfText);
}

// ============================================
// RENDERING METHODS
// ============================================

void UISystem::RenderElements(float dt) {
    if (!m_coordinator || !m_window)
        return;

    // Collect and sort entities by layer
    std::vector<std::pair<int, ECS::Entity>> sortedEntities;

    for (auto entity : mEntities) {
        if (!m_coordinator->HasComponent<Components::UIElement>(entity))
            continue;

        const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);

        if (!element.visible)
            continue;
        if (!element.menuGroup.empty() && !IsMenuVisible(element.menuGroup))
            continue;

        sortedEntities.push_back({element.layer, entity});
    }

    // Sort by layer (lower first)
    std::sort(sortedEntities.begin(), sortedEntities.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // First pass: render all elements except open dropdowns
    std::vector<ECS::Entity> openDropdowns;
    for (const auto& [layer, entity] : sortedEntities) {
        bool isDropdown = m_coordinator->HasComponent<Components::UIDropdown>(entity);
        bool isOpen = false;
        if (isDropdown) {
            const auto& dropdown = m_coordinator->GetComponent<Components::UIDropdown>(entity);
            isOpen = dropdown.isOpen;
        }
        if (isDropdown && isOpen) {
            openDropdowns.push_back(entity);
            continue;  // skip rendering open dropdowns for now
        }
        if (m_coordinator->HasComponent<Components::UIPanel>(entity)) {
            RenderPanel(entity);
        }
        if (m_coordinator->HasComponent<Components::UIButton>(entity)) {
            RenderButton(entity, dt);
        } else if (m_coordinator->HasComponent<Components::UIText>(entity)) {
            RenderText(entity, dt);
        }
        if (m_coordinator->HasComponent<Components::UISlider>(entity)) {
            RenderSlider(entity);
        }
        if (m_coordinator->HasComponent<Components::UIInputField>(entity)) {
            RenderInputField(entity, dt);
        }
        if (m_coordinator->HasComponent<Components::UICheckbox>(entity)) {
            RenderCheckbox(entity);
        }
        if (isDropdown && !isOpen) {
            RenderDropdown(entity);
        }
    }

    // Second pass: render all open dropdowns last (above everything else)
    for (ECS::Entity entity : openDropdowns) {
        RenderDropdown(entity);
    }
}

void UISystem::RenderPanel(ECS::Entity entity) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    const auto& panel = m_coordinator->GetComponent<Components::UIPanel>(entity);

    // Apply animation
    float alpha = 1.0f;
    float offsetX = 0.0f, offsetY = 0.0f;
    float scale = 1.0f;

    if (panel.animationProgress < 1.0f) {
        float progress = panel.animationProgress;
        switch (panel.currentAnimation) {
            case Components::UIPanel::Animation::FadeIn:
                alpha = progress;
                break;
            case Components::UIPanel::Animation::FadeOut:
                alpha = 1.0f - progress;
                break;
            case Components::UIPanel::Animation::SlideFromTop:
                offsetY = -element.height * (1.0f - progress);
                break;
            case Components::UIPanel::Animation::SlideFromBottom:
                offsetY = element.height * (1.0f - progress);
                break;
            case Components::UIPanel::Animation::SlideFromLeft:
                offsetX = -element.width * (1.0f - progress);
                break;
            case Components::UIPanel::Animation::SlideFromRight:
                offsetX = element.width * (1.0f - progress);
                break;
            case Components::UIPanel::Animation::Scale:
                scale = progress;
                break;
            default:
                break;
        }
    }

    // Adjust background color alpha
    uint32_t bgColor = panel.backgroundColor;
    uint8_t bgAlpha = static_cast<uint8_t>((bgColor & 0xFF) * alpha);
    bgColor = (bgColor & 0xFFFFFF00) | bgAlpha;

    // Draw background
    DrawRect(element.x + offsetX, element.y + offsetY, element.width * scale,
             element.height * scale, bgColor, panel.borderColor, panel.borderThickness);

    // Draw title bar if enabled
    if (panel.showTitleBar) {
        DrawRect(element.x + offsetX, element.y + offsetY, element.width * scale,
                 panel.titleBarHeight, panel.titleBarColor, 0, 0);

        // Draw title text
        DrawText(panel.title, element.x + offsetX + element.width * scale / 2.0f,
                 element.y + offsetY + 10.0f, 20, 0xFFFFFFFF, TextAlign::Center);
    }
}

void UISystem::RenderButton(ECS::Entity entity, float dt) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    auto& button = m_coordinator->GetComponent<Components::UIButton>(entity);

    // Update scale animation
    float targetScale = 1.0f;
    if (button.state == Components::UIButton::State::Hovered ||
        button.state == Components::UIButton::State::Selected || entity == m_selectedEntity) {
        targetScale = button.hoverScale;
    }

    if (dt > 0) {
        button.currentScale += (targetScale - button.currentScale) * button.scaleSpeed * dt;
    }

    // Determine colors based on state
    uint32_t textColor = button.normalColor;
    uint32_t bgColor = button.bgNormalColor;
    float borderThickness = button.borderThickness;

    if (!button.enabled) {
        textColor = button.disabledColor;
    } else if (entity == m_selectedEntity) {
        textColor = button.selectedColor;
        bgColor = button.bgSelectedColor;
        borderThickness = button.borderHoverThickness;
    } else {
        switch (button.state) {
            case Components::UIButton::State::Hovered:
                textColor = button.hoverColor;
                bgColor = button.bgHoverColor;
                borderThickness = button.borderHoverThickness;
                break;
            case Components::UIButton::State::Pressed:
                textColor = button.pressedColor;
                bgColor = button.bgPressedColor;
                borderThickness = button.borderHoverThickness;
                break;
            default:
                break;
        }
    }

    // Calculate scaled dimensions
    float scaledWidth = element.width * button.currentScale;
    float scaledHeight = element.height * button.currentScale;
    float offsetX = (element.width - scaledWidth) / 2.0f;
    float offsetY = (element.height - scaledHeight) / 2.0f;

    // Draw background
    if ((bgColor & 0xFF) > 0) {
        DrawRect(element.x + offsetX, element.y + offsetY, scaledWidth, scaledHeight, bgColor,
                 button.borderColor, borderThickness);
    }

    // Draw text
    unsigned int fontSize = static_cast<unsigned int>(24 * button.currentScale);
    float textX = element.x + element.width / 2.0f;
    float textY = element.y + element.height / 2.0f - 12.0f * button.currentScale;
    DrawText(button.text, textX, textY, fontSize, textColor, TextAlign::Center);
}

void UISystem::RenderText(ECS::Entity entity, float dt) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    auto& textComp = m_coordinator->GetComponent<Components::UIText>(entity);

    // Update pulsation
    float scale = 1.0f;
    if (textComp.pulsating && dt > 0) {
        textComp.currentPulseTime += dt * textComp.pulseSpeed;
        float t = (std::sin(textComp.currentPulseTime) + 1.0f) / 2.0f;
        scale = textComp.pulseMinScale + t * (textComp.pulseMaxScale - textComp.pulseMinScale);
    }

    TextAlign align = TextAlign::Left;
    switch (textComp.alignment) {
        case Components::UIText::Alignment::Center:
            align = TextAlign::Center;
            break;
        case Components::UIText::Alignment::Right:
            align = TextAlign::Right;
            break;
        default:
            align = TextAlign::Left;
    }

    // Draw shadow first if enabled
    if (textComp.shadow) {
        DrawText(textComp.content, element.x + textComp.shadowOffsetX,
                 element.y + textComp.shadowOffsetY,
                 static_cast<unsigned int>(textComp.fontSize * scale), textComp.shadowColor, align,
                 textComp.fontId);
    }

    // Draw main text
    DrawText(textComp.content, element.x, element.y,
             static_cast<unsigned int>(textComp.fontSize * scale), textComp.color, align,
             textComp.fontId);
}

void UISystem::RenderSlider(ECS::Entity entity) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    const auto& slider = m_coordinator->GetComponent<Components::UISlider>(entity);

    // Draw track background
    float trackY = element.y + (element.height - slider.trackHeight) / 2.0f;
    DrawRect(element.x, trackY, element.width, slider.trackHeight, slider.trackColor, 0, 0);

    // Draw filled portion
    float fillWidth = element.width * slider.getNormalized();
    DrawRect(element.x, trackY, fillWidth, slider.trackHeight, slider.trackFillColor, 0, 0);

    // Draw handle
    float handleX = element.x + fillWidth - slider.handleWidth / 2.0f;
    float handleY = element.y + (element.height - slider.handleHeight) / 2.0f;

    uint32_t handleColor =
        slider.isHovered || slider.isDragging ? slider.handleHoverColor : slider.handleColor;

    DrawRect(handleX, handleY, slider.handleWidth, slider.handleHeight, handleColor, 0x00FFFFFF,
             2.0f);

    // Draw label if present
    if (!slider.label.empty()) {
        DrawText(slider.label, element.x, element.y - 25.0f, 18, 0xFFFFFFFF, TextAlign::Left);
    }

    // Draw value if enabled
    if (slider.showValue) {
        char valueStr[32];
        snprintf(valueStr, sizeof(valueStr), slider.valueFormat.c_str(), slider.currentValue);
        std::string displayValue = std::string(valueStr) + slider.suffix;
        DrawText(displayValue, element.x + element.width, element.y - 25.0f, 16, 0xFFFFFFFF,
                 TextAlign::Right);
    }
}

void UISystem::RenderInputField(ECS::Entity entity, float dt) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    auto& input = m_coordinator->GetComponent<Components::UIInputField>(entity);

    // Update cursor blink
    if (input.isFocused && dt > 0) {
        input.cursorBlinkTimer += dt;
        if (input.cursorBlinkTimer >= input.cursorBlinkRate) {
            input.cursorBlinkTimer = 0;
            input.cursorVisible = !input.cursorVisible;
        }
    }

    // Draw background
    uint32_t borderColor = input.isFocused ? input.focusBorderColor : input.borderColor;
    DrawRect(element.x, element.y, element.width, element.height, input.backgroundColor,
             borderColor, input.borderThickness);

    // Draw text
    std::string displayText = input.getDisplayText();
    uint32_t textColor = input.isShowingPlaceholder() ? input.placeholderColor : input.textColor;
    DrawText(displayText, element.x + input.padding, element.y + element.height / 2.0f - 10.0f, 20,
             textColor, TextAlign::Left);

    // Draw cursor (simplified - just a rectangle after text)
    if (input.isFocused && input.cursorVisible && input.showCursor) {
        // Approximate cursor position based on text length
        float cursorX = element.x + input.padding + input.text.length() * 10.0f;
        DrawRect(cursorX, element.y + 5.0f, 2.0f, element.height - 10.0f, input.textColor, 0, 0);
    }
}

void UISystem::RenderCheckbox(ECS::Entity entity) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    const auto& checkbox = m_coordinator->GetComponent<Components::UICheckbox>(entity);

    // Draw box
    uint32_t boxColor = checkbox.isHovered ? checkbox.boxHoverColor : checkbox.boxColor;
    uint32_t borderColor = checkbox.isHovered ? checkbox.borderHoverColor : checkbox.borderColor;

    DrawRect(element.x, element.y, checkbox.boxSize, checkbox.boxSize, boxColor, borderColor, 2.0f);

    // Draw checkmark if checked
    if (checkbox.checked) {
        float padding = checkbox.boxSize * 0.2f;
        DrawRect(element.x + padding, element.y + padding, checkbox.boxSize - padding * 2.0f,
                 checkbox.boxSize - padding * 2.0f, checkbox.checkColor, 0, 0);
    }

    // Draw label
    if (!checkbox.label.empty()) {
        float labelX = checkbox.labelOnRight ? element.x + checkbox.boxSize + checkbox.labelSpacing
                                             : element.x - checkbox.labelSpacing;
        TextAlign align = checkbox.labelOnRight ? TextAlign::Left : TextAlign::Right;
        DrawText(checkbox.label, labelX, element.y + checkbox.boxSize / 2.0f - 10.0f, 20,
                 0xFFFFFFFF, align);
    }
}

void UISystem::RenderDropdown(ECS::Entity entity) {
    const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    const auto& dropdown = m_coordinator->GetComponent<Components::UIDropdown>(entity);

    // Draw main box
    uint32_t borderColor = dropdown.isOpen ? dropdown.borderOpenColor : dropdown.borderColor;
    DrawRect(element.x, element.y, element.width, element.height, dropdown.backgroundColor,
             borderColor, dropdown.borderThickness);

    // Draw selected text
    DrawText(dropdown.getSelectedText(), element.x + 10.0f,
             element.y + element.height / 2.0f - 10.0f, 20, dropdown.textColor, TextAlign::Left);

    // Draw arrow
    if (dropdown.showArrow) {
        std::string arrow = dropdown.isOpen ? "^" : "v";
        DrawText(arrow, element.x + element.width - 20.0f,
                 element.y + element.height / 2.0f - 10.0f, 20, dropdown.textColor,
                 TextAlign::Right);
    }

    // Draw dropdown options if open
    if (dropdown.isOpen) {
        float optionY = element.y + element.height;

        for (size_t i = 0; i < dropdown.options.size(); ++i) {
            uint32_t optBg = dropdown.backgroundColor;
            if (static_cast<int>(i) == dropdown.hoveredOptionIndex) {
                optBg = dropdown.hoverBackgroundColor;
            } else if (static_cast<int>(i) == dropdown.selectedIndex) {
                optBg = dropdown.selectedBackgroundColor;
            }

            DrawRect(element.x, optionY, element.width, dropdown.optionHeight, optBg,
                     dropdown.borderColor, 1.0f);

            DrawText(dropdown.options[i], element.x + 10.0f,
                     optionY + dropdown.optionHeight / 2.0f - 9.0f, 18, dropdown.textColor,
                     TextAlign::Left);

            optionY += dropdown.optionHeight;
        }
    }

    // Draw label if present
    if (!dropdown.label.empty()) {
        DrawText(dropdown.label, element.x, element.y - 25.0f, 18, 0xFFFFFFFF, TextAlign::Left);
    }
}
