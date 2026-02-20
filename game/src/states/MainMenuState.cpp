/**
 * MainMenuState.cpp - Main Menu Implementation
 */

#include "states/MainMenuState.hpp"
#include "states/SettingsState.hpp"
#include "states/PlayState.hpp"
#include "states/MultiplayerMenuState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include <components/Position.hpp>
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UIPanel.hpp>
#include <scripting/LuaState.hpp>
#include <iostream>

MainMenuState::MainMenuState(Game* game)
{
    game_ = game;
}

void MainMenuState::onEnter()
{
    std::cout << "[MainMenuState] Entering main menu" << std::endl;
    
    // Load Main Menu UI from Lua
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        // Load menu config
        lua.script_file("assets/scripts/ui/menu_main.lua");
        sol::table menuConfig = lua["MainMenu"];
        
        std::cout << "[MainMenuState] Creating UI entities from Lua..." << std::endl;
        
        // Get ECS coordinator
        auto coordinator = game_->getCoordinator();
        
        // Create background panel
        auto panelConfig = menuConfig["background"]["panel"];
        ECS::Entity panelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIPanel"](panelEntity, 
                                  panelConfig["x"].get<float>(),
                                  panelConfig["y"].get<float>(),
                                  panelConfig["width"].get<float>(),
                                  panelConfig["height"].get<float>());
        menuEntities_.push_back(panelEntity);
        
        // Create title text
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create subtitle
        auto subtitleConfig = menuConfig["subtitle"];
        ECS::Entity subtitleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](subtitleEntity,
                                 subtitleConfig["x"].get<float>(),
                                 subtitleConfig["y"].get<float>(),
                                 subtitleConfig["text"].get<std::string>(),
                                 subtitleConfig["fontSize"].get<int>());
        menuEntities_.push_back(subtitleEntity);
        
        // Create buttons
        sol::table buttons = menuConfig["buttons"];
        for (size_t i = 1; i <= buttons.size(); ++i) {
            sol::table btn = buttons[i];
            
            ECS::Entity btnEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIButton"](btnEntity,
                                       btn["x"].get<float>(),
                                       btn["y"].get<float>(),
                                       btn["width"].get<float>(),
                                       btn["height"].get<float>(),
                                       btn["text"].get<std::string>(),
                                       btn["callback"].get<std::string>());
            menuEntities_.push_back(btnEntity);
            
            std::cout << "[MainMenuState] Created button: " << btn["text"].get<std::string>() << std::endl;
        }
        
        std::cout << "[MainMenuState] ✅ Created " << menuEntities_.size() << " UI entities" << std::endl;
        
    } catch (const sol::error& e) {
        std::cerr << "[MainMenuState] ❌ Error loading menu UI: " << e.what() << std::endl;
    }
}

void MainMenuState::onExit()
{
    std::cout << "[MainMenuState] Exiting main menu" << std::endl;
    
    // Destroy all menu entities
    auto coordinator = game_->getCoordinator();
    for (auto entity : menuEntities_) {
        coordinator->DestroyEntity(entity);
    }
    menuEntities_.clear();
    
    std::cout << "[MainMenuState] ✅ Cleaned up menu entities" << std::endl;
}

void MainMenuState::handleEvent(const eng::engine::InputEvent& event)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Handle mouse movement for hover effect
    if (event.type == eng::engine::EventType::MouseMoved)
    {
        float mouseX = static_cast<float>(event.mouseMove.x);
        float mouseY = static_cast<float>(event.mouseMove.y);
        
        // Reset all buttons to normal state first
        for (auto entity : menuEntities_)
        {
            if (coordinator->HasComponent<Components::UIButton>(entity))
            {
                auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                button.state = Components::UIButton::State::Normal;
            }
        }
        
        // Check which button is hovered
        for (auto entity : menuEntities_)
        {
            if (!coordinator->HasComponent<Components::UIButton>(entity)) continue;
            if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
            
            auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
            auto& button = coordinator->GetComponent<Components::UIButton>(entity);
            
            // Check if mouse is inside button bounds
            if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
            {
                button.state = Components::UIButton::State::Hovered;
                hoveredButton_ = entity;
                break;
            }
        }
    }
    
    // Handle mouse clicks
    if (event.type == eng::engine::EventType::MouseButtonPressed)
    {
        if (event.mouseButton.button == 0) // Left click
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            
            // Check which button was clicked
            for (auto entity : menuEntities_)
            {
                if (!coordinator->HasComponent<Components::UIButton>(entity)) continue;
                if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
                
                auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
                auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                
                // Check if click is inside button bounds
                if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                    mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
                {
                    button.state = Components::UIButton::State::Pressed;
                    
                    std::cout << "[MainMenuState] Button clicked: " << button.text << std::endl;
                    
                    // Call Lua callback
                    if (!button.onClickCallback.empty())
                    {
                        auto& lua = Scripting::LuaState::Instance().GetState();
                        try {
                            lua[button.onClickCallback]();
                            std::cout << "[MainMenuState] ✅ Callback executed: " << button.onClickCallback << std::endl;
                        } catch (const sol::error& e) {
                            std::cerr << "[MainMenuState] ❌ Error calling callback: " << e.what() << std::endl;
                        }
                    }
                    
                    // Handle specific actions based on button text
                    if (button.text == "Quit")
                    {
                        std::cout << "[MainMenuState] Closing game..." << std::endl;
                        game_->getWindow()->close();
                    }
                    else if (button.text == "Solo Play")
                    {
                        std::cout << "[MainMenuState] Launching Solo Play..." << std::endl;
                        game_->getStateManager()->pushState(std::make_unique<PlayState>(game_));
                    }
                    else if (button.text == "Multiplayer")
                    {
                        std::cout << "[MainMenuState] Opening Multiplayer Menu..." << std::endl;
                        game_->getStateManager()->pushState(std::make_unique<MultiplayerMenuState>(game_));
                    }
                    else if (button.text == "Settings")
                    {
                        std::cout << "[MainMenuState] Opening Settings..." << std::endl;
                        game_->getStateManager()->pushState(std::make_unique<SettingsState>(game_));
                    }
                    
                    break;
                }
            }
        }
    }
    
    // Handle ESC to quit
    if (event.type == eng::engine::EventType::KeyPressed)
    {
        if (event.key.code == eng::engine::Key::Escape)
        {
            std::cout << "[MainMenuState] ESC pressed - closing game" << std::endl;
            game_->getWindow()->close();
        }
    }
}

void MainMenuState::update(float deltaTime)
{
    // Update logic will be added in Phase 4
    (void)deltaTime; // Unused for now
}

void MainMenuState::render()
{
    // Rendering will be added in Phase 4
    // For now, just clear to a color to show it's working
    auto window = game_->getWindow();
    if (window)
    {
        // The window is already cleared in Game::render()
        // We'll add UI rendering in Phase 4
    }
}
