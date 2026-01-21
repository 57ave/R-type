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
#include <systems/UISystem.hpp>

// ============================================
// ENTITY CREATION METHODS
// ============================================

ECS::Entity UISystem::CreateButton(float x, float y, float width, float height,
                                   const std::string& text, const std::string& callback,
                                   const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    Components::UIElement element;
    element.x = x - width / 2.0f;  // Center the button
    element.y = y - height / 2.0f;
    element.width = width;
    element.height = height;
    element.menuGroup = menuGroup;
    element.tabIndex = m_nextTabIndex++;
    m_coordinator->AddComponent(entity, element);

    Components::UIButton button;
    button.text = text;
    button.onClickCallback = callback;
    m_coordinator->AddComponent(entity, button);

    mEntities.insert(entity);
    m_navigationDirty = true;

    std::cout << "[UISystem] Created Button entity " << entity << " text='" << text
              << "' callback='" << callback << "' menu='" << menuGroup << "' at (" << element.x
              << "," << element.y << ") size(" << width << "," << height << ")" << std::endl;

    return entity;
}

ECS::Entity UISystem::CreateText(float x, float y, const std::string& text, unsigned int fontSize,
                                 uint32_t color, const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = 0;  // Will be determined by text bounds
    element.height = 0;
    element.menuGroup = menuGroup;
    element.interactable = false;
    m_coordinator->AddComponent(entity, element);

    Components::UIText textComp;
    textComp.content = text;
    textComp.fontSize = fontSize;
    textComp.color = color;
    m_coordinator->AddComponent(entity, textComp);

    mEntities.insert(entity);

    return entity;
}

ECS::Entity UISystem::CreateSlider(float x, float y, float width, float minVal, float maxVal,
                                   float currentVal, const std::string& callback,
                                   const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    float height = 40.0f;  // Fixed height for sliders

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.menuGroup = menuGroup;
    element.tabIndex = m_nextTabIndex++;
    m_coordinator->AddComponent(entity, element);

    Components::UISlider slider;
    slider.minValue = minVal;
    slider.maxValue = maxVal;
    slider.currentValue = currentVal;
    slider.onChangeCallback = callback;
    m_coordinator->AddComponent(entity, slider);

    mEntities.insert(entity);
    m_navigationDirty = true;

    return entity;
}

ECS::Entity UISystem::CreatePanel(float x, float y, float width, float height, uint32_t bgColor,
                                  bool modal, const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.menuGroup = menuGroup;
    element.layer = 50;  // Below buttons/text but above game
    element.interactable = false;
    m_coordinator->AddComponent(entity, element);

    Components::UIPanel panel;
    panel.backgroundColor = bgColor;
    panel.modal = modal;
    m_coordinator->AddComponent(entity, panel);

    mEntities.insert(entity);

    return entity;
}

ECS::Entity UISystem::CreateInputField(float x, float y, float width, float height,
                                       const std::string& placeholder, const std::string& onSubmit,
                                       const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.menuGroup = menuGroup;
    element.tabIndex = m_nextTabIndex++;
    m_coordinator->AddComponent(entity, element);

    Components::UIInputField input;
    input.placeholder = placeholder;
    input.onSubmitCallback = onSubmit;
    m_coordinator->AddComponent(entity, input);

    mEntities.insert(entity);
    m_navigationDirty = true;

    std::cout << "[UISystem] Created InputField entity " << entity 
              << " placeholder='" << placeholder << "' menu='" << menuGroup 
              << "' at (" << x << "," << y << ") size(" << width << "," << height << ")" << std::endl;

    return entity;
}

ECS::Entity UISystem::CreateCheckbox(float x, float y, const std::string& label, bool initialState,
                                     const std::string& callback, const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    float size = 24.0f;

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = size + 200.0f;  // Include label space
    element.height = size;
    element.menuGroup = menuGroup;
    element.tabIndex = m_nextTabIndex++;
    m_coordinator->AddComponent(entity, element);

    Components::UICheckbox checkbox;
    checkbox.label = label;
    checkbox.checked = initialState;
    checkbox.onChangeCallback = callback;
    m_coordinator->AddComponent(entity, checkbox);

    mEntities.insert(entity);
    m_navigationDirty = true;

    return entity;
}

ECS::Entity UISystem::CreateDropdown(float x, float y, float width,
                                     const std::vector<std::string>& options, int selectedIndex,
                                     const std::string& callback, const std::string& menuGroup) {
    if (!m_coordinator)
        return 0;

    ECS::Entity entity = m_coordinator->CreateEntity();

    float height = 40.0f;  // Fixed height for dropdown

    Components::UIElement element;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.menuGroup = menuGroup;
    element.tabIndex = m_nextTabIndex++;
    m_coordinator->AddComponent(entity, element);

    Components::UIDropdown dropdown;
    dropdown.options = options;
    dropdown.selectedIndex = selectedIndex;
    dropdown.onChangeCallback = callback;
    m_coordinator->AddComponent(entity, dropdown);

    mEntities.insert(entity);
    m_navigationDirty = true;

    return entity;
}

// ============================================
// UI ELEMENT MANIPULATION
// ============================================

void UISystem::SetVisible(ECS::Entity entity, bool visible) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIElement>(entity))
        return;
    m_coordinator->GetComponent<Components::UIElement>(entity).visible = visible;
    m_navigationDirty = true;
}

void UISystem::SetText(ECS::Entity entity, const std::string& text) {
    if (!m_coordinator)
        return;

    if (m_coordinator->HasComponent<Components::UIText>(entity)) {
        m_coordinator->GetComponent<Components::UIText>(entity).content = text;
    }
    if (m_coordinator->HasComponent<Components::UIButton>(entity)) {
        m_coordinator->GetComponent<Components::UIButton>(entity).text = text;
    }
}

void UISystem::SetPosition(ECS::Entity entity, float x, float y) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIElement>(entity))
        return;
    auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
    element.x = x;
    element.y = y;
}

float UISystem::GetSliderValue(ECS::Entity entity) const {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UISlider>(entity))
        return 0.0f;
    return m_coordinator->GetComponent<Components::UISlider>(entity).currentValue;
}

void UISystem::SetSliderValue(ECS::Entity entity, float value) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UISlider>(entity))
        return;
    auto& slider = m_coordinator->GetComponent<Components::UISlider>(entity);
    slider.currentValue = std::max(slider.minValue, std::min(slider.maxValue, value));
}

std::string UISystem::GetInputText(ECS::Entity entity) const {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIInputField>(entity))
        return "";
    return m_coordinator->GetComponent<Components::UIInputField>(entity).text;
}

void UISystem::SetInputText(ECS::Entity entity, const std::string& text) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIInputField>(entity))
        return;
    auto& input = m_coordinator->GetComponent<Components::UIInputField>(entity);
    input.text = text.substr(0, input.maxLength);
    input.cursorPosition = input.text.length();
}

bool UISystem::GetCheckboxState(ECS::Entity entity) const {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UICheckbox>(entity))
        return false;
    return m_coordinator->GetComponent<Components::UICheckbox>(entity).checked;
}

void UISystem::SetCheckboxState(ECS::Entity entity, bool checked) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UICheckbox>(entity))
        return;
    m_coordinator->GetComponent<Components::UICheckbox>(entity).checked = checked;
}

int UISystem::GetDropdownIndex(ECS::Entity entity) const {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIDropdown>(entity))
        return -1;
    return m_coordinator->GetComponent<Components::UIDropdown>(entity).selectedIndex;
}

void UISystem::SetDropdownIndex(ECS::Entity entity, int index) {
    if (!m_coordinator || !m_coordinator->HasComponent<Components::UIDropdown>(entity))
        return;
    auto& dropdown = m_coordinator->GetComponent<Components::UIDropdown>(entity);
    if (index >= 0 && index < static_cast<int>(dropdown.options.size())) {
        dropdown.selectedIndex = index;
    }
}
