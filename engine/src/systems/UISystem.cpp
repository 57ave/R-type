#include <systems/UISystem.hpp>
#include <ecs/Coordinator.hpp>
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UISlider.hpp>
#include <components/UIInputField.hpp>
#include <components/UIPanel.hpp>
#include <components/UICheckbox.hpp>
#include <components/UIDropdown.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

UISystem::UISystem()
{
    m_textRenderer = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
}

UISystem::UISystem(ECS::Coordinator* coordinator) : UISystem()
{
    m_coordinator = coordinator;
}

UISystem::~UISystem() = default;

void UISystem::Init()
{
    // Nothing special to initialize
}

void UISystem::Update(float dt)
{
    if (!m_coordinator || !m_window) {
        return;
    }

    // Update mouse position from window
    auto mousePos = m_window->getMousePosition();
    m_mouseX = mousePos.x;
    m_mouseY = mousePos.y;

    // Handle input
    HandleMouseInput();
    
    // Render all visible UI elements
    RenderElements(dt);

    // Update previous mouse state
    m_mousePreviouslyPressed = m_mousePressed;
}

void UISystem::Shutdown()
{
    m_fonts.clear();
}

void UISystem::Render(eng::engine::rendering::sfml::SFMLWindow* window)
{
    if (!m_coordinator || !window) {
        return;
    }

    // Set the window for rendering
    m_window = window;

    // Render all visible UI elements
    RenderElements(0.0f);
}

void UISystem::HandleEvent(const eng::engine::InputEvent& event)
{
    using namespace eng::engine;

    // Handle mouse events
    if (event.type == EventType::MouseMoved) {
        m_mouseX = event.mouseMove.x;
        m_mouseY = event.mouseMove.y;
        // Update hovered entity immediately
        m_hoveredEntity = GetEntityAtPosition(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
    }
    else if (event.type == EventType::MouseButtonPressed) {
        if (event.mouseButton.button == 0) { // Left button
            m_mousePressed = true;
            // Update mouse position from click event
            m_mouseX = event.mouseButton.x;
            m_mouseY = event.mouseButton.y;
            // Find entity at click position
            m_hoveredEntity = GetEntityAtPosition(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
            
            // Selection is updated, but click handling is done in HandleMouseInput()
            // to avoid double-triggering callbacks
            if (m_hoveredEntity != 0) {
                m_selectedEntity = m_hoveredEntity;
                std::cout << "[UISystem] Click on entity " << m_hoveredEntity << std::endl;
            }
        }
    }
    else if (event.type == EventType::MouseButtonReleased) {
        if (event.mouseButton.button == 0) { // Left button
            m_mousePressed = false;
        }
    }
    // Handle keyboard events
    else if (event.type == EventType::KeyPressed) {
        if (event.key.code == Key::Up) {
            SelectPrevious();
        }
        else if (event.key.code == Key::Down) {
            SelectNext();
        }
        else if (event.key.code == Key::Enter) {
            ActivateSelected();
        }
        else if (event.key.code == Key::Backspace) {
            if (m_focusedInputField != 0) {
                HandleBackspace();
            }
        }
    }
    // Handle text input events for input fields
    else if (event.type == EventType::TextEntered) {
        if (m_focusedInputField != 0) {
            // Filter out control characters (backspace, enter, etc.)
            unsigned int unicode = event.text.unicode;
            if (unicode >= 32 && unicode < 127) {
                HandleTextInput(static_cast<char>(unicode));
            }
        }
    }

    // Update mouse input handling
    HandleMouseInput();
}
void UISystem::SetRenderer(eng::engine::rendering::IRenderer* renderer)
{
    m_renderer = renderer;
}

void UISystem::SetCoordinator(ECS::Coordinator* coordinator)
{
    m_coordinator = coordinator;
}

void UISystem::SetLuaState(sol::state* lua)
{
    m_lua = lua;
}

void UISystem::SetWindow(eng::engine::rendering::sfml::SFMLWindow* window)
{
    m_window = window;
}

// Font management
bool UISystem::LoadFont(const std::string& fontId, const std::string& filepath)
{
    auto font = std::make_unique<eng::engine::rendering::sfml::SFMLFont>();
    if (font->loadFromFile(filepath)) {
        m_fonts[fontId] = std::move(font);
        std::cout << "[UISystem] Loaded font '" << fontId << "' from " << filepath << std::endl;
        return true;
    }
    std::cerr << "[UISystem] Failed to load font from " << filepath << std::endl;
    return false;
}

eng::engine::rendering::IFont* UISystem::GetFont(const std::string& fontId)
{
    auto it = m_fonts.find(fontId);
    if (it != m_fonts.end()) {
        return it->second.get();
    }
    // Try default font
    it = m_fonts.find("default");
    if (it != m_fonts.end()) {
        return it->second.get();
    }
    return nullptr;
}

// Keyboard navigation
void UISystem::SelectNext()
{
    if (m_navigationDirty) {
        UpdateNavigationOrder();
    }

    if (m_navigableEntities.empty()) return;

    auto it = std::find(m_navigableEntities.begin(), m_navigableEntities.end(), m_selectedEntity);
    if (it == m_navigableEntities.end()) {
        m_selectedEntity = m_navigableEntities.front();
    } else {
        ++it;
        if (it == m_navigableEntities.end()) {
            it = m_navigableEntities.begin();
        }
        m_selectedEntity = *it;
    }
}

void UISystem::SelectPrevious()
{
    if (m_navigationDirty) {
        UpdateNavigationOrder();
    }

    if (m_navigableEntities.empty()) return;

    auto it = std::find(m_navigableEntities.begin(), m_navigableEntities.end(), m_selectedEntity);
    if (it == m_navigableEntities.end()) {
        m_selectedEntity = m_navigableEntities.back();
    } else {
        if (it == m_navigableEntities.begin()) {
            it = m_navigableEntities.end();
        }
        --it;
        m_selectedEntity = *it;
    }
}

void UISystem::ActivateSelected()
{
    if (m_selectedEntity == 0 || !m_coordinator) return;

    // Check if it's a button
    if (m_coordinator->HasComponent<Components::UIButton>(m_selectedEntity)) {
        auto& button = m_coordinator->GetComponent<Components::UIButton>(m_selectedEntity);
        if (button.enabled && !button.onClickCallback.empty()) {
            CallLuaCallback(button.onClickCallback);
            CallCppCallback(button.onClickCallback);
        }
    }
    // Check if it's a checkbox
    else if (m_coordinator->HasComponent<Components::UICheckbox>(m_selectedEntity)) {
        auto& checkbox = m_coordinator->GetComponent<Components::UICheckbox>(m_selectedEntity);
        if (checkbox.enabled) {
            checkbox.checked = !checkbox.checked;
            if (!checkbox.onChangeCallback.empty()) {
                CallLuaValueCallback(checkbox.onChangeCallback, checkbox.checked ? 1.0f : 0.0f);
                CallCppValueCallback(checkbox.onChangeCallback, checkbox.checked ? 1.0f : 0.0f);
            }
        }
    }
}

void UISystem::SetSelectedEntity(ECS::Entity entity)
{
    m_selectedEntity = entity;
}

ECS::Entity UISystem::GetSelectedEntity() const
{
    return m_selectedEntity;
}

// Menu management
void UISystem::ShowMenu(const std::string& menuGroup)
{
    m_menuVisibility[menuGroup] = true;
    m_navigationDirty = true;
}

void UISystem::HideMenu(const std::string& menuGroup)
{
    m_menuVisibility[menuGroup] = false;
    m_navigationDirty = true;
}

void UISystem::HideAllMenus()
{
    for (auto& [key, value] : m_menuVisibility) {
        value = false;
    }
    m_navigationDirty = true;
}

bool UISystem::IsMenuVisible(const std::string& menuGroup) const
{
    auto it = m_menuVisibility.find(menuGroup);
    return it != m_menuVisibility.end() && it->second;
}

void UISystem::SetActiveMenu(const std::string& menuGroup)
{
    m_activeMenuGroup = menuGroup;
    m_navigationDirty = true;
}

std::string UISystem::GetActiveMenu() const
{
    return m_activeMenuGroup;
}

// Input state
void UISystem::SetMousePosition(int x, int y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void UISystem::SetMousePressed(bool pressed)
{
    m_mousePreviouslyPressed = m_mousePressed;
    m_mousePressed = pressed;
}

void UISystem::SetKeyPressed(int keyCode, bool pressed)
{
    // Handle in Game.cpp or InputSystem
}

void UISystem::HandleTextInput(char character)
{
    if (m_focusedInputField == 0 || !m_coordinator) return;

    if (!m_coordinator->HasComponent<Components::UIInputField>(m_focusedInputField)) return;

    auto& input = m_coordinator->GetComponent<Components::UIInputField>(m_focusedInputField);
    
    // Check constraints
    if (input.text.length() >= input.maxLength) return;
    if (input.numbersOnly && (character < '0' || character > '9')) return;
    if (input.alphanumericOnly && !std::isalnum(character)) return;
    if (!input.allowedCharacters.empty() && 
        input.allowedCharacters.find(character) == std::string::npos) return;

    // Insert character at cursor
    input.text.insert(input.cursorPosition, 1, character);
    input.cursorPosition++;

    if (!input.onChangeCallback.empty()) {
        CallLuaStringCallback(input.onChangeCallback, input.text);
        CallCppStringCallback(input.onChangeCallback, input.text);
    }
}

void UISystem::HandleBackspace()
{
    if (m_focusedInputField == 0 || !m_coordinator) return;

    if (!m_coordinator->HasComponent<Components::UIInputField>(m_focusedInputField)) return;

    auto& input = m_coordinator->GetComponent<Components::UIInputField>(m_focusedInputField);
    
    if (input.cursorPosition > 0) {
        input.text.erase(input.cursorPosition - 1, 1);
        input.cursorPosition--;

        if (!input.onChangeCallback.empty()) {
            CallLuaStringCallback(input.onChangeCallback, input.text);
            CallCppStringCallback(input.onChangeCallback, input.text);
        }
    }
}

// Helper functions
bool UISystem::IsPointInRect(float px, float py, float rx, float ry, float rw, float rh) const
{
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

ECS::Entity UISystem::GetEntityAtPosition(float x, float y) const
{
    if (!m_coordinator) return 0;

    ECS::Entity topEntity = 0;
    int topLayer = -1;

    for (auto entity : mEntities) {
        try {
            if (!m_coordinator->HasComponent<Components::UIElement>(entity)) continue;

            const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
            
            // Check visibility
            if (!element.visible) continue;
            if (!element.menuGroup.empty() && !IsMenuVisible(element.menuGroup)) continue;

            // Check if point is inside
            if (IsPointInRect(x, y, element.x, element.y, element.width, element.height)) {
                if (element.layer > topLayer) {
                    topLayer = element.layer;
                    topEntity = entity;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[UISystem] Error checking entity " << entity << ": " << e.what() << std::endl;
        }
    }

    return topEntity;
}

void UISystem::HandleMouseInput()
{
    ECS::Entity entityUnderMouse = GetEntityAtPosition(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
    // Handle click (on mouse down)
    bool clicked = m_mousePressed && !m_mousePreviouslyPressed;
    if (clicked)
    {
        std::cout << "[UISystem] Mouse click at (" << m_mouseX << "," << m_mouseY << ") entityUnderMouse=" << entityUnderMouse << std::endl;
        // 1. Dropdown: priorité à la sélection d'option si un dropdown est ouvert
        if (m_openDropdown != 0)
        {
            auto &dropdown = m_coordinator->GetComponent<Components::UIDropdown>(m_openDropdown);
            if (dropdown.isOpen && m_coordinator->HasComponent<Components::UIElement>(m_openDropdown))
            {
                auto &element = m_coordinator->GetComponent<Components::UIElement>(m_openDropdown);
                float optionY = element.y + element.height;
                for (size_t i = 0; i < dropdown.options.size(); ++i)
                {
                    float optTop = optionY + i * dropdown.optionHeight;
                    float optBot = optTop + dropdown.optionHeight;
                    if (m_mouseY >= optTop && m_mouseY < optBot &&
                        m_mouseX >= element.x && m_mouseX < element.x + element.width)
                    {
                        dropdown.selectedIndex = static_cast<int>(i);
                        dropdown.isOpen = false;
                        m_openDropdown = 0;
                        dropdown.hoveredOptionIndex = -1;
                        if (!dropdown.onChangeCallback.empty())
                        {
                            CallLuaCallback(dropdown.onChangeCallback);
                            CallCppCallback(dropdown.onChangeCallback);
                        }
                        return;
                    }
                }
            }
        }

        // 2. Handle button click
        if (entityUnderMouse != 0)
        {
            if (m_coordinator->HasComponent<Components::UIButton>(entityUnderMouse))
            {
                auto &button = m_coordinator->GetComponent<Components::UIButton>(entityUnderMouse);
                // Log element/menu/callback info for debugging
                if (m_coordinator->HasComponent<Components::UIElement>(entityUnderMouse)) {
                    const auto &elem = m_coordinator->GetComponent<Components::UIElement>(entityUnderMouse);
                    std::cout << "[UISystem] Click on entity " << entityUnderMouse
                              << " menu='" << elem.menuGroup << "' visible=" << elem.visible
                              << " interactable=" << elem.interactable << std::endl;
                }
                std::cout << "[UISystem] UIButton callback='" << button.onClickCallback << "' enabled=" << button.enabled << std::endl;

                if (button.enabled)
                {
                    button.state = Components::UIButton::State::Pressed;
                    if (!button.onClickCallback.empty())
                    {
                        CallLuaCallback(button.onClickCallback);
                        CallCppCallback(button.onClickCallback);
                    }
                }
            }
            // Handle checkbox click
            if (m_coordinator->HasComponent<Components::UICheckbox>(entityUnderMouse))
            {
                auto &checkbox = m_coordinator->GetComponent<Components::UICheckbox>(entityUnderMouse);
                if (checkbox.enabled)
                {
                    checkbox.checked = !checkbox.checked;
                    if (!checkbox.onChangeCallback.empty())
                    {
                        CallLuaValueCallback(checkbox.onChangeCallback, checkbox.checked ? 1.0f : 0.0f);
                        CallCppValueCallback(checkbox.onChangeCallback, checkbox.checked ? 1.0f : 0.0f);
                    }
                }
            }
            // Handle input field focus
            if (m_coordinator->HasComponent<Components::UIInputField>(entityUnderMouse))
            {
                if (m_focusedInputField != 0 && m_focusedInputField != entityUnderMouse)
                {
                    auto &prevInput = m_coordinator->GetComponent<Components::UIInputField>(m_focusedInputField);
                    prevInput.isFocused = false;
                }
                auto &input = m_coordinator->GetComponent<Components::UIInputField>(entityUnderMouse);
                input.isFocused = true;
                m_focusedInputField = entityUnderMouse;
                std::cout << "[UISystem] InputField focused: entity " << entityUnderMouse << std::endl;
            }
            // Handle dropdown (ouvrir si fermé)
            if (m_coordinator->HasComponent<Components::UIDropdown>(entityUnderMouse))
            {
                auto &dropdown = m_coordinator->GetComponent<Components::UIDropdown>(entityUnderMouse);
                if (!dropdown.isOpen)
                {
                    dropdown.isOpen = true;
                    m_openDropdown = entityUnderMouse;
                }
            }
        }
        else if (m_openDropdown != 0)
        {
            // Fermer dropdown si clic ailleurs
            auto &dropdown = m_coordinator->GetComponent<Components::UIDropdown>(m_openDropdown);
            dropdown.isOpen = false;
            m_openDropdown = 0;
        }
        // Update selected entity
        m_selectedEntity = entityUnderMouse;
    }
    // Gestion du survol d'option pour dropdown ouvert
    if (m_openDropdown != 0)
    {
        auto &dropdown = m_coordinator->GetComponent<Components::UIDropdown>(m_openDropdown);
        if (dropdown.isOpen && m_coordinator->HasComponent<Components::UIElement>(m_openDropdown))
        {
            auto &element = m_coordinator->GetComponent<Components::UIElement>(m_openDropdown);
            float optionY = element.y + element.height;
            dropdown.hoveredOptionIndex = -1;
            for (size_t i = 0; i < dropdown.options.size(); ++i)
            {
                float optTop = optionY + i * dropdown.optionHeight;
                float optBot = optTop + dropdown.optionHeight;
                if (m_mouseY >= optTop && m_mouseY < optBot &&
                    m_mouseX >= element.x && m_mouseX < element.x + element.width)
                {
                    dropdown.hoveredOptionIndex = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    // Handle slider dragging (fonctionne même sans dropdown ouvert)
    if (m_mousePressed)
    {
        for (auto entity : mEntities)
        {
            if (m_coordinator->HasComponent<Components::UISlider>(entity))
            {
                auto &slider = m_coordinator->GetComponent<Components::UISlider>(entity);
                auto &element = m_coordinator->GetComponent<Components::UIElement>(entity);
                
                // Check visibility
                if (!element.visible) continue;
                if (!element.menuGroup.empty() && !IsMenuVisible(element.menuGroup)) continue;

                // Start dragging on click, or continue if already dragging
                bool isUnderMouse = IsPointInRect(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY),
                                                   element.x, element.y, element.width, element.height);
                
                if ((clicked && isUnderMouse) || slider.isDragging)
                {
                    slider.isDragging = true;

                    // Calculate new value based on mouse position
                    float relativeX = static_cast<float>(m_mouseX) - element.x;
                    float normalized = relativeX / element.width;
                    normalized = std::max(0.0f, std::min(1.0f, normalized));
                    slider.setFromNormalized(normalized);

                    if (!slider.onChangeCallback.empty())
                    {
                        CallLuaValueCallback(slider.onChangeCallback, slider.currentValue);
                        CallCppValueCallback(slider.onChangeCallback, slider.currentValue);
                    }
                }
            }
        }
    }

    // Handle button release and slider release (fonctionne même sans dropdown ouvert)
    if (!m_mousePressed && m_mousePreviouslyPressed)
    {
        for (auto entity : mEntities)
        {
            if (m_coordinator->HasComponent<Components::UIButton>(entity))
            {
                auto &button = m_coordinator->GetComponent<Components::UIButton>(entity);
                if (button.state == Components::UIButton::State::Pressed)
                {
                    button.state = (entity == m_hoveredEntity) ? Components::UIButton::State::Hovered : Components::UIButton::State::Normal;
                }
            }
            if (m_coordinator->HasComponent<Components::UISlider>(entity))
            {
                auto &slider = m_coordinator->GetComponent<Components::UISlider>(entity);
                if (slider.isDragging)
                {
                    slider.isDragging = false;
                    if (!slider.onReleaseCallback.empty())
                    {
                        CallLuaValueCallback(slider.onReleaseCallback, slider.currentValue);
                    }
                }
            }
        }
    }
}

void UISystem::UpdateNavigationOrder()
{
    m_navigableEntities.clear();

    if (!m_coordinator) return;

    std::vector<std::pair<int, ECS::Entity>> sortedEntities;

    for (auto entity : mEntities) {
        if (!m_coordinator->HasComponent<Components::UIElement>(entity)) continue;

        const auto& element = m_coordinator->GetComponent<Components::UIElement>(entity);
        
        // Check visibility
        if (!element.visible) continue;
        if (!element.menuGroup.empty() && !IsMenuVisible(element.menuGroup)) continue;
        if (!element.interactable) continue;
        if (element.tabIndex < 0) continue;

        // Check if it's a navigable type
        bool isNavigable = m_coordinator->HasComponent<Components::UIButton>(entity) ||
                          m_coordinator->HasComponent<Components::UISlider>(entity) ||
                          m_coordinator->HasComponent<Components::UIInputField>(entity) ||
                          m_coordinator->HasComponent<Components::UICheckbox>(entity) ||
                          m_coordinator->HasComponent<Components::UIDropdown>(entity);

        if (isNavigable) {
            sortedEntities.push_back({element.tabIndex, entity});
        }
    }

    // Sort by tab index
    std::sort(sortedEntities.begin(), sortedEntities.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [idx, entity] : sortedEntities) {
        m_navigableEntities.push_back(entity);
    }

    m_navigationDirty = false;
}

std::vector<ECS::Entity> UISystem::GetNavigableEntities() const
{
    return m_navigableEntities;
}

// Callbacks
void UISystem::CallLuaCallback(const std::string& callbackName)
{
    if (!m_lua || callbackName.empty()) return;
    std::cout << "[UISystem] Attempting to call Lua callback '" << callbackName << "'" << std::endl;
    
    try {
        sol::protected_function func = (*m_lua)[callbackName];
        if (func.valid()) {
            std::cout << "[UISystem] Lua function '" << callbackName << "' found, invoking..." << std::endl;
            auto result = func();
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[UISystem] Lua error in " << callbackName << ": " << err.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UISystem] Exception calling Lua callback " << callbackName << ": " << e.what() << std::endl;
    }
}

void UISystem::CallLuaValueCallback(const std::string& callbackName, float value)
{
    if (!m_lua || callbackName.empty()) return;

    try {
        sol::protected_function func = (*m_lua)[callbackName];
        if (func.valid()) {
            auto result = func(value);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[UISystem] Lua error in " << callbackName << ": " << err.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UISystem] Exception calling Lua callback " << callbackName << ": " << e.what() << std::endl;
    }
}

void UISystem::CallLuaStringCallback(const std::string& callbackName, const std::string& value)
{
    if (!m_lua || callbackName.empty()) return;

    try {
        sol::protected_function func = (*m_lua)[callbackName];
        if (func.valid()) {
            auto result = func(value);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[UISystem] Lua error in " << callbackName << ": " << err.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UISystem] Exception calling Lua callback " << callbackName << ": " << e.what() << std::endl;
    }
}

void UISystem::CallCppCallback(const std::string& callbackName)
{
    auto it = m_callbacks.find(callbackName);
    if (it != m_callbacks.end()) {
        it->second();
    }
}

void UISystem::CallCppValueCallback(const std::string& callbackName, float value)
{
    auto it = m_valueCallbacks.find(callbackName);
    if (it != m_valueCallbacks.end()) {
        it->second(value);
    }
}

void UISystem::CallCppStringCallback(const std::string& callbackName, const std::string& value)
{
    auto it = m_stringCallbacks.find(callbackName);
    if (it != m_stringCallbacks.end()) {
        it->second(value);
    }
}

void UISystem::RegisterCallback(const std::string& name, Callback callback)
{
    m_callbacks[name] = callback;
}

void UISystem::RegisterValueCallback(const std::string& name, ValueCallback callback)
{
    m_valueCallbacks[name] = callback;
}

void UISystem::RegisterStringCallback(const std::string& name, StringCallback callback)
{
    m_stringCallbacks[name] = callback;
}
