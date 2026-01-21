#include <scripting/UIBindings.hpp>
#include <systems/UISystem.hpp>
#include <core/GameStateCallbacks.hpp>
#include <iostream>

namespace Scripting {

    UISystem* UIBindings::s_uiSystem = nullptr;
    eng::engine::core::GameStateCallbacks UIBindings::s_gameStateCallbacks;

    void UIBindings::SetUISystem(UISystem* uiSystem)
    {
        s_uiSystem = uiSystem;
    }

    void UIBindings::SetGameStateCallbacks(const eng::engine::core::GameStateCallbacks& callbacks)
    {
        s_gameStateCallbacks = callbacks;
        std::cout << "[UIBindings] Game state callbacks set" << std::endl;
    }

    void UIBindings::RegisterAll(sol::state& lua, UISystem* uiSystem)
    {
        if (uiSystem) {
            s_uiSystem = uiSystem;
        }

        // Create UI namespace
        auto ui = lua["UI"].get_or_create<sol::table>();

        // ========================================
        // Element Creation
        // ========================================

        // CreateButton({ x, y, width, height, text, onClick, menuGroup })
        ui["CreateButton"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) {
                std::cerr << "[UIBindings] UISystem not set!" << std::endl;
                return 0;
            }

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            float width = config.get_or("width", 200.0f);
            float height = config.get_or("height", 50.0f);
            std::string text = config.get_or<std::string>("text", "Button");
            std::string onClick = config.get_or<std::string>("onClick", "");
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateButton(x, y, width, height, text, onClick, menuGroup);
        };

        // CreateText({ x, y, text, fontSize, color, menuGroup })
        ui["CreateText"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            std::string text = config.get_or<std::string>("text", "");
            unsigned int fontSize = config.get_or("fontSize", 24u);
            uint32_t color = config.get_or("color", 0xFFFFFFFFu);
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateText(x, y, text, fontSize, color, menuGroup);
        };

        // CreateSlider({ x, y, width, min, max, value, onChange, menuGroup })
        ui["CreateSlider"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            float width = config.get_or("width", 200.0f);
            float minVal = config.get_or("min", 0.0f);
            float maxVal = config.get_or("max", 100.0f);
            float value = config.get_or("value", 50.0f);
            std::string onChange = config.get_or<std::string>("onChange", "");
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateSlider(x, y, width, minVal, maxVal, value, onChange, menuGroup);
        };

        // CreatePanel({ x, y, width, height, backgroundColor, modal, menuGroup })
        ui["CreatePanel"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            float width = config.get_or("width", 300.0f);
            float height = config.get_or("height", 400.0f);
            uint32_t bgColor = config.get_or("backgroundColor", 0x000000AAu);
            bool modal = config.get_or("modal", false);
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreatePanel(x, y, width, height, bgColor, modal, menuGroup);
        };

        // CreateInputField({ x, y, width, height, placeholder, menuGroup })
        ui["CreateInputField"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            float width = config.get_or("width", 200.0f);
            float height = config.get_or("height", 40.0f);
            std::string placeholder = config.get_or<std::string>("placeholder", "Enter text...");
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateInputField(x, y, width, height, placeholder, menuGroup);
        };

        // CreateCheckbox({ x, y, label, checked, onChange, menuGroup })
        ui["CreateCheckbox"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            std::string label = config.get_or<std::string>("label", "");
            bool checked = config.get_or("checked", false);
            std::string onChange = config.get_or<std::string>("onChange", "");
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateCheckbox(x, y, label, checked, onChange, menuGroup);
        };

        // CreateDropdown({ x, y, width, options, selectedIndex, onChange, menuGroup })
        ui["CreateDropdown"] = [](sol::table config) -> ECS::Entity {
            if (!s_uiSystem) return 0;

            float x = config.get_or("x", 0.0f);
            float y = config.get_or("y", 0.0f);
            float width = config.get_or("width", 200.0f);
            
            std::vector<std::string> options;
            sol::optional<sol::table> optionsTable = config["options"];
            if (optionsTable) {
                for (auto& kv : *optionsTable) {
                    options.push_back(kv.second.as<std::string>());
                }
            }
            
            int selectedIndex = config.get_or("selectedIndex", 0);
            std::string onChange = config.get_or<std::string>("onChange", "");
            std::string menuGroup = config.get_or<std::string>("menuGroup", "");

            return s_uiSystem->CreateDropdown(x, y, width, options, selectedIndex, onChange, menuGroup);
        };

        // ========================================
        // Element Manipulation
        // ========================================

        ui["SetVisible"] = [](ECS::Entity entity, bool visible) {
            if (s_uiSystem) s_uiSystem->SetVisible(entity, visible);
        };

        ui["SetText"] = [](ECS::Entity entity, const std::string& text) {
            if (s_uiSystem) s_uiSystem->SetText(entity, text);
        };

        ui["SetPosition"] = [](ECS::Entity entity, float x, float y) {
            if (s_uiSystem) s_uiSystem->SetPosition(entity, x, y);
        };

        ui["GetSliderValue"] = [](ECS::Entity entity) -> float {
            return s_uiSystem ? s_uiSystem->GetSliderValue(entity) : 0.0f;
        };

        ui["SetSliderValue"] = [](ECS::Entity entity, float value) {
            if (s_uiSystem) s_uiSystem->SetSliderValue(entity, value);
        };

        ui["GetInputText"] = [](ECS::Entity entity) -> std::string {
            return s_uiSystem ? s_uiSystem->GetInputText(entity) : "";
        };

        ui["SetInputText"] = [](ECS::Entity entity, const std::string& text) {
            if (s_uiSystem) s_uiSystem->SetInputText(entity, text);
        };

        ui["GetCheckboxState"] = [](ECS::Entity entity) -> bool {
            return s_uiSystem ? s_uiSystem->GetCheckboxState(entity) : false;
        };

        ui["SetCheckboxState"] = [](ECS::Entity entity, bool checked) {
            if (s_uiSystem) s_uiSystem->SetCheckboxState(entity, checked);
        };

        ui["GetDropdownIndex"] = [](ECS::Entity entity) -> int {
            return s_uiSystem ? s_uiSystem->GetDropdownIndex(entity) : -1;
        };

        ui["SetDropdownIndex"] = [](ECS::Entity entity, int index) {
            if (s_uiSystem) s_uiSystem->SetDropdownIndex(entity, index);
        };

        // ========================================
        // Navigation
        // ========================================

        ui["SelectNext"] = []() {
            if (s_uiSystem) s_uiSystem->SelectNext();
        };

        ui["SelectPrevious"] = []() {
            if (s_uiSystem) s_uiSystem->SelectPrevious();
        };

        ui["ActivateSelected"] = []() {
            if (s_uiSystem) s_uiSystem->ActivateSelected();
        };

        ui["SetSelected"] = [](ECS::Entity entity) {
            if (s_uiSystem) s_uiSystem->SetSelectedEntity(entity);
        };

        ui["GetSelected"] = []() -> ECS::Entity {
            return s_uiSystem ? s_uiSystem->GetSelectedEntity() : 0;
        };

        // ========================================
        // Menu Management
        // ========================================

        ui["ShowMenu"] = [](const std::string& menuGroup) {
            if (s_uiSystem) s_uiSystem->ShowMenu(menuGroup);
        };

        ui["HideMenu"] = [](const std::string& menuGroup) {
            if (s_uiSystem) s_uiSystem->HideMenu(menuGroup);
        };

        ui["HideAllMenus"] = []() {
            if (s_uiSystem) s_uiSystem->HideAllMenus();
        };

        ui["IsMenuVisible"] = [](const std::string& menuGroup) -> bool {
            return s_uiSystem ? s_uiSystem->IsMenuVisible(menuGroup) : false;
        };

        ui["SetActiveMenu"] = [](const std::string& menuGroup) {
            if (s_uiSystem) s_uiSystem->SetActiveMenu(menuGroup);
        };

        ui["GetActiveMenu"] = []() -> std::string {
            return s_uiSystem ? s_uiSystem->GetActiveMenu() : "";
        };

        // ========================================
        // Font Loading
        // ========================================

        ui["LoadFont"] = [](const std::string& fontId, const std::string& filepath) -> bool {
            return s_uiSystem ? s_uiSystem->LoadFont(fontId, filepath) : false;
        };

        // ========================================
        // Asset Path Resolution
        // ========================================

        // Set the base path for assets (set from C++ side)
        ui["SetBasePath"] = [&lua](const std::string& basePath) {
            lua["ASSET_BASE_PATH"] = basePath;
        };

        // Helper function to resolve a relative path
        ui["ResolvePath"] = [&lua](const std::string& relativePath) -> std::string {
            std::string basePath = lua.get_or<std::string>("ASSET_BASE_PATH", "");
            return basePath + relativePath;
        };

        // Register GameState bindings
        RegisterGameState(lua);

        std::cout << "[UIBindings] UI bindings registered" << std::endl;
    }

    void UIBindings::RegisterGameState(sol::state& lua)
    {
        // Create GameState namespace
        auto gameState = lua["GameState"].get_or_create<sol::table>();

        // Use injected callbacks for game state management
        // This keeps the engine fully abstract - no knowledge of specific game states
        
        gameState["Set"] = [](const std::string& state) {
            std::cout << "[GameState] Set called with: " << state << std::endl;
            if (s_gameStateCallbacks.setState) {
                s_gameStateCallbacks.setState(state);
            } else {
                std::cerr << "[GameState] Warning: setState callback not set" << std::endl;
            }
        };

        gameState["Get"] = []() -> std::string {
            if (s_gameStateCallbacks.getState) {
                return s_gameStateCallbacks.getState();
            }
            std::cerr << "[GameState] Warning: getState callback not set" << std::endl;
            return "Unknown";
        };

        gameState["IsPaused"] = []() -> bool {
            if (s_gameStateCallbacks.isPaused) {
                return s_gameStateCallbacks.isPaused();
            }
            return false;
        };

        gameState["IsPlaying"] = []() -> bool {
            if (s_gameStateCallbacks.isPlaying) {
                return s_gameStateCallbacks.isPlaying();
            }
            return false;
        };

        gameState["TogglePause"] = []() {
            if (s_gameStateCallbacks.togglePause) {
                s_gameStateCallbacks.togglePause();
            }
        };

        gameState["GoBack"] = []() {
            if (s_gameStateCallbacks.goBack) {
                s_gameStateCallbacks.goBack();
            }
        };

        gameState["Reset"] = []() {
            std::cout << "[GameState] Reset called - cleaning up game entities" << std::endl;
            if (s_gameStateCallbacks.resetGame) {
                s_gameStateCallbacks.resetGame();
            } else {
                std::cerr << "[GameState] Warning: resetGame callback not set" << std::endl;
            }
        };

        std::cout << "[UIBindings] GameState bindings registered" << std::endl;
    }

} // namespace Scripting
