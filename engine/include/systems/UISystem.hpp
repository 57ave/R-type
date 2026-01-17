#ifndef ENG_ENGINE_SYSTEMS_UISYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_UISYSTEM_HPP

#include <ecs/System.hpp>
#include <ecs/Types.hpp>
#include <rendering/IRenderer.hpp>
#include <rendering/IText.hpp>
#include <rendering/IFont.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <engine/Input.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <vector>

// Forward declarations
namespace ECS
{
    class Coordinator;
}

namespace sol
{
    class state;
}

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {
                class SFMLWindow;
            }
        }
    }
}

/**
 * @brief UI System - Handles rendering and interaction of all UI elements
 *
 * Responsibilities:
 * - Render UI elements (buttons, text, panels, sliders, etc.)
 * - Handle mouse input (hover, click)
 * - Handle keyboard navigation (up/down, enter, escape)
 * - Call Lua callbacks when interactions occur
 * - Manage font resources
 * - Support menu groups for showing/hiding entire menus
 */
class UISystem : public ECS::System
{
public:
    // Text alignment for DrawText helper
    enum class TextAlign
    {
        Left,
        Center,
        Right
    };

    UISystem();
    UISystem(ECS::Coordinator *coordinator);
    ~UISystem() override;

    // ECS System interface
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;

    // Render UI elements to a window
    void Render(eng::engine::rendering::sfml::SFMLWindow *window);

    // Handle input events
    void HandleEvent(const eng::engine::InputEvent &event);

    // Dependencies injection
    void SetRenderer(eng::engine::rendering::IRenderer *renderer);
    void SetCoordinator(ECS::Coordinator *coordinator);
    void SetLuaState(sol::state *lua);
    void SetWindow(eng::engine::rendering::sfml::SFMLWindow *window);

    // Font management
    bool LoadFont(const std::string &fontId, const std::string &filepath);
    eng::engine::rendering::IFont *GetFont(const std::string &fontId);

    // Keyboard navigation
    void SelectNext();
    void SelectPrevious();
    void ActivateSelected();
    void SetSelectedEntity(ECS::Entity entity);
    ECS::Entity GetSelectedEntity() const;

    // Menu management
    void ShowMenu(const std::string &menuGroup);
    void HideMenu(const std::string &menuGroup);
    void HideAllMenus();
    bool IsMenuVisible(const std::string &menuGroup) const;
    void SetActiveMenu(const std::string &menuGroup);
    std::string GetActiveMenu() const;

    // Input state
    void SetMousePosition(int x, int y);
    void SetMousePressed(bool pressed);
    void SetKeyPressed(int keyCode, bool pressed);
    void HandleTextInput(char character);
    void HandleBackspace();

    // Entity creation helpers (can be called from Lua bindings)
    ECS::Entity CreateButton(float x, float y, float width, float height,
                             const std::string &text, const std::string &callback,
                             const std::string &menuGroup = "");
    ECS::Entity CreateText(float x, float y, const std::string &text,
                           unsigned int fontSize = 24, uint32_t color = 0xFFFFFFFF,
                           const std::string &menuGroup = "");
    ECS::Entity CreateSlider(float x, float y, float width,
                             float minVal, float maxVal, float currentVal,
                             const std::string &callback,
                             const std::string &menuGroup = "");
    ECS::Entity CreatePanel(float x, float y, float width, float height,
                            uint32_t bgColor, bool modal = false,
                            const std::string &menuGroup = "");
    ECS::Entity CreateInputField(float x, float y, float width, float height,
                                 const std::string &placeholder,
                                 const std::string &menuGroup = "");
    ECS::Entity CreateCheckbox(float x, float y, const std::string &label,
                               bool initialState, const std::string &callback,
                               const std::string &menuGroup = "");
    ECS::Entity CreateDropdown(float x, float y, float width,
                               const std::vector<std::string> &options,
                               int selectedIndex, const std::string &callback,
                               const std::string &menuGroup = "");

    // UI element manipulation
    void SetVisible(ECS::Entity entity, bool visible);
    void SetText(ECS::Entity entity, const std::string &text);
    void SetPosition(ECS::Entity entity, float x, float y);
    float GetSliderValue(ECS::Entity entity) const;
    void SetSliderValue(ECS::Entity entity, float value);
    std::string GetInputText(ECS::Entity entity) const;
    void SetInputText(ECS::Entity entity, const std::string &text);
    bool GetCheckboxState(ECS::Entity entity) const;
    void SetCheckboxState(ECS::Entity entity, bool checked);
    int GetDropdownIndex(ECS::Entity entity) const;
    void SetDropdownIndex(ECS::Entity entity, int index);

    // C++ callback registration (alternative to Lua)
    using Callback = std::function<void()>;
    using ValueCallback = std::function<void(float)>;
    using StringCallback = std::function<void(const std::string &)>;

    void RegisterCallback(const std::string &name, Callback callback);
    void RegisterValueCallback(const std::string &name, ValueCallback callback);
    void RegisterStringCallback(const std::string &name, StringCallback callback);

private:
    // Direct SFML rendering helpers
    void DrawRect(float x, float y, float width, float height,
                  uint32_t fillColor, uint32_t outlineColor = 0, float outlineThickness = 0);
    void DrawText(const std::string &text, float x, float y,
                  unsigned int fontSize, uint32_t color,
                  TextAlign align = TextAlign::Left, const std::string &fontId = "default");

    // Rendering
    void RenderElements(float dt);
    void RenderPanel(ECS::Entity entity);
    void RenderButton(ECS::Entity entity, float dt);
    void RenderText(ECS::Entity entity, float dt);
    void RenderSlider(ECS::Entity entity);
    void RenderInputField(ECS::Entity entity, float dt);
    void RenderCheckbox(ECS::Entity entity);
    void RenderDropdown(ECS::Entity entity);

    // Input handling
    void HandleMouseInput();
    void HandleKeyboardNavigation();
    bool IsPointInRect(float px, float py, float rx, float ry, float rw, float rh) const;
    ECS::Entity GetEntityAtPosition(float x, float y) const;

    // Callbacks
    void CallLuaCallback(const std::string &callbackName);
    void CallLuaValueCallback(const std::string &callbackName, float value);
    void CallLuaStringCallback(const std::string &callbackName, const std::string &value);
    void CallCppCallback(const std::string &callbackName);
    void CallCppValueCallback(const std::string &callbackName, float value);
    void CallCppStringCallback(const std::string &callbackName, const std::string &value);

    // Navigation helpers
    std::vector<ECS::Entity> GetNavigableEntities() const;
    void UpdateNavigationOrder();

    // Dependencies
    eng::engine::rendering::IRenderer *m_renderer = nullptr;
    ECS::Coordinator *m_coordinator = nullptr;
    sol::state *m_lua = nullptr;
    eng::engine::rendering::sfml::SFMLWindow *m_window = nullptr;

    // Font management
    std::unordered_map<std::string, std::unique_ptr<eng::engine::rendering::sfml::SFMLFont>> m_fonts;

    // Text objects pool (reused for rendering)
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> m_textRenderer;

    // State
    ECS::Entity m_hoveredEntity = 0;
    ECS::Entity m_selectedEntity = 0;
    ECS::Entity m_focusedInputField = 0;
    ECS::Entity m_openDropdown = 0;

    std::string m_activeMenuGroup;
    std::unordered_map<std::string, bool> m_menuVisibility;

    // Input state
    int m_mouseX = 0;
    int m_mouseY = 0;
    bool m_mousePressed = false;
    bool m_mousePreviouslyPressed = false;

    // C++ callbacks
    std::unordered_map<std::string, Callback> m_callbacks;
    std::unordered_map<std::string, ValueCallback> m_valueCallbacks;
    std::unordered_map<std::string, StringCallback> m_stringCallbacks;

    // Navigation
    std::vector<ECS::Entity> m_navigableEntities;
    bool m_navigationDirty = true;

    // Tab index counter for auto-assignment
    int m_nextTabIndex = 0;
};

#endif // ENG_ENGINE_SYSTEMS_UISYSTEM_HPP
