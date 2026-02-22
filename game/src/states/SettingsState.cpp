/**
 * SettingsState.cpp - Settings Menu Implementation
 */

#include "states/SettingsState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/MusicManager.hpp"
#include "managers/SFXManager.hpp"
#include <components/Position.hpp>
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UIPanel.hpp>
#include <components/UISlider.hpp>
#include <components/UICheckbox.hpp>
#include <scripting/LuaState.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include "core/Logger.hpp"

SettingsState::SettingsState(Game* game)
{
    game_ = game;
}

void SettingsState::onEnter()
{
    LOG_INFO("SETTINGS", "Entering settings menu");
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        lua.script_file("assets/scripts/ui/menu_settings.lua");
        sol::table menuConfig = lua["SettingsMenu"];
        
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
        
        // Create title
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create section titles
        auto audioTitle = menuConfig["audio"]["section_title"];
        ECS::Entity audioTitleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](audioTitleEntity,
                                 audioTitle["x"].get<float>(),
                                 audioTitle["y"].get<float>(),
                                 audioTitle["text"].get<std::string>(),
                                 audioTitle["fontSize"].get<int>());
        menuEntities_.push_back(audioTitleEntity);
        
        auto graphicsTitle = menuConfig["graphics"]["section_title"];
        ECS::Entity graphicsTitleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](graphicsTitleEntity,
                                 graphicsTitle["x"].get<float>(),
                                 graphicsTitle["y"].get<float>(),
                                 graphicsTitle["text"].get<std::string>(),
                                 graphicsTitle["fontSize"].get<int>());
        menuEntities_.push_back(graphicsTitleEntity);
        
        // Create Audio Sliders
        auto masterVol = menuConfig["audio"]["master_volume"];
        masterVolumeSlider_ = coordinator->CreateEntity();
        lua["ECS"]["AddUISlider"](masterVolumeSlider_,
                                   masterVol["x"].get<float>(),
                                   masterVol["y"].get<float>(),
                                   masterVol["width"].get<float>(),
                                   masterVol["height"].get<float>(),
                                   masterVol["min"].get<float>(),
                                   masterVol["max"].get<float>(),
                                   masterVol["default"].get<float>());
        menuEntities_.push_back(masterVolumeSlider_);
        
        // Master Volume Label
        ECS::Entity masterVolLabel = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](masterVolLabel,
                                 masterVol["x"].get<float>(),
                                 masterVol["y"].get<float>() - 30,
                                 masterVol["label"].get<std::string>(),
                                 18);
        menuEntities_.push_back(masterVolLabel);
        
        auto musicVol = menuConfig["audio"]["music_volume"];
        musicVolumeSlider_ = coordinator->CreateEntity();
        lua["ECS"]["AddUISlider"](musicVolumeSlider_,
                                   musicVol["x"].get<float>(),
                                   musicVol["y"].get<float>(),
                                   musicVol["width"].get<float>(),
                                   musicVol["height"].get<float>(),
                                   musicVol["min"].get<float>(),
                                   musicVol["max"].get<float>(),
                                   musicVol["default"].get<float>());
        menuEntities_.push_back(musicVolumeSlider_);
        
        // Music Volume Label
        ECS::Entity musicVolLabel = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](musicVolLabel,
                                 musicVol["x"].get<float>(),
                                 musicVol["y"].get<float>() - 30,
                                 musicVol["label"].get<std::string>(),
                                 18);
        menuEntities_.push_back(musicVolLabel);
        
        auto sfxVol = menuConfig["audio"]["sfx_volume"];
        sfxVolumeSlider_ = coordinator->CreateEntity();
        lua["ECS"]["AddUISlider"](sfxVolumeSlider_,
                                   sfxVol["x"].get<float>(),
                                   sfxVol["y"].get<float>(),
                                   sfxVol["width"].get<float>(),
                                   sfxVol["height"].get<float>(),
                                   sfxVol["min"].get<float>(),
                                   sfxVol["max"].get<float>(),
                                   sfxVol["default"].get<float>());
        menuEntities_.push_back(sfxVolumeSlider_);
        
        // SFX Volume Label
        ECS::Entity sfxVolLabel = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](sfxVolLabel,
                                 sfxVol["x"].get<float>(),
                                 sfxVol["y"].get<float>() - 30,
                                 sfxVol["label"].get<std::string>(),
                                 18);
        menuEntities_.push_back(sfxVolLabel);
        
        // Create Graphics Checkboxes
        auto vsync = menuConfig["graphics"]["vsync"];
        vsyncCheckbox_ = coordinator->CreateEntity();
        lua["ECS"]["AddUICheckbox"](vsyncCheckbox_,
                                     vsync["x"].get<float>(),
                                     vsync["y"].get<float>(),
                                     vsync["label"].get<std::string>(),
                                     vsync["default"].get<bool>());
        menuEntities_.push_back(vsyncCheckbox_);
        
        auto fullscreen = menuConfig["graphics"]["fullscreen"];
        fullscreenCheckbox_ = coordinator->CreateEntity();
        lua["ECS"]["AddUICheckbox"](fullscreenCheckbox_,
                                     fullscreen["x"].get<float>(),
                                     fullscreen["y"].get<float>(),
                                     fullscreen["label"].get<std::string>(),
                                     fullscreen["default"].get<bool>());
        menuEntities_.push_back(fullscreenCheckbox_);
        
        auto showFps = menuConfig["graphics"]["show_fps"];
        showFpsCheckbox_ = coordinator->CreateEntity();
        lua["ECS"]["AddUICheckbox"](showFpsCheckbox_,
                                     showFps["x"].get<float>(),
                                     showFps["y"].get<float>(),
                                     showFps["label"].get<std::string>(),
                                     showFps["default"].get<bool>());
        menuEntities_.push_back(showFpsCheckbox_);
        
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
        }
        
        LOG_INFO("SETTINGS", "Created " + std::to_string(menuEntities_.size()) + " UI entities");
        
        // Load saved settings from JSON
        loadSettings();
        
    } catch (const sol::error& e) {
        LOG_ERROR("SETTINGS", std::string("Error loading settings UI: ") + e.what());
    }
}

void SettingsState::onExit()
{
    LOG_INFO("SETTINGS", "Exiting settings menu");
    
    auto coordinator = game_->getCoordinator();
    for (auto entity : menuEntities_) {
        coordinator->DestroyEntity(entity);
    }
    menuEntities_.clear();
}

void SettingsState::handleEvent(const eng::engine::InputEvent& event)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Handle mouse movement
    if (event.type == eng::engine::EventType::MouseMoved)
    {
        float mouseX = static_cast<float>(event.mouseMove.x);
        float mouseY = static_cast<float>(event.mouseMove.y);
        
        for (auto entity : menuEntities_)
        {
            if (coordinator->HasComponent<Components::UIButton>(entity))
            {
                auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                button.state = Components::UIButton::State::Normal;
            }
        }
        
        for (auto entity : menuEntities_)
        {
            if (!coordinator->HasComponent<Components::UIButton>(entity)) continue;
            if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
            
            auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
            auto& button = coordinator->GetComponent<Components::UIButton>(entity);
            
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
        if (event.mouseButton.button == 0)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            
            for (auto entity : menuEntities_)
            {
                if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
                auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
                
                // Check collision
                if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                    mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
                {
                    // Handle button clicks
                    if (coordinator->HasComponent<Components::UIButton>(entity))
                    {
                        auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                        LOG_DEBUG("SETTINGS", "Button clicked: " + button.text);
                        
                        if (button.text == "Back")
                        {
                            LOG_INFO("SETTINGS", "Going back to main menu");
                            game_->getStateManager()->popState();
                        }
                        else if (button.text == "Apply")
                        {
                            saveSettings();
                        }
                        break;
                    }
                    
                    // Handle checkbox clicks
                    if (coordinator->HasComponent<Components::UICheckbox>(entity))
                    {
                        auto& checkbox = coordinator->GetComponent<Components::UICheckbox>(entity);
                        checkbox.checked = !checkbox.checked;
                        LOG_DEBUG("SETTINGS", "Checkbox '" + checkbox.label + "' toggled: "
                                  + (checkbox.checked ? "ON" : "OFF"));
                        break;
                    }
                }
            }
        }
    }
    
    // ESC to go back
    if (event.type == eng::engine::EventType::KeyPressed)
    {
        if (event.key.code == eng::engine::Key::Escape)
        {
            LOG_INFO("SETTINGS", "ESC pressed - going back");
            game_->getStateManager()->popState();
        }
    }
}

void SettingsState::update(float deltaTime)
{
    (void)deltaTime;
}

void SettingsState::render()
{
    auto window = game_->getWindow();
    auto coordinator = game_->getCoordinator();
    if (!window || !coordinator) return;
    
    auto& sfmlWindow = window->getSFMLWindow();
    
    // Render all UI entities (same code as MainMenuState)
    for (auto entity : menuEntities_)
    {
        if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
        
        auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
        if (!uiElem.visible) continue;
        
        // Render panels
        if (coordinator->HasComponent<Components::UIPanel>(entity))
        {
            auto& panel = coordinator->GetComponent<Components::UIPanel>(entity);
            
            sf::RectangleShape rect(sf::Vector2f(uiElem.width, uiElem.height));
            rect.setPosition(uiElem.x, uiElem.y);
            
            sf::Color bgColor(
                (panel.backgroundColor >> 24) & 0xFF,
                (panel.backgroundColor >> 16) & 0xFF,
                (panel.backgroundColor >> 8) & 0xFF,
                panel.backgroundColor & 0xFF
            );
            rect.setFillColor(bgColor);
            
            if (panel.borderThickness > 0) {
                sf::Color borderCol(
                    (panel.borderColor >> 24) & 0xFF,
                    (panel.borderColor >> 16) & 0xFF,
                    (panel.borderColor >> 8) & 0xFF,
                    panel.borderColor & 0xFF
                );
                rect.setOutlineColor(borderCol);
                rect.setOutlineThickness(panel.borderThickness);
            }
            
            sfmlWindow.draw(rect);
        }
        
        // Render buttons
        if (coordinator->HasComponent<Components::UIButton>(entity))
        {
            auto& button = coordinator->GetComponent<Components::UIButton>(entity);
            
            sf::RectangleShape rect(sf::Vector2f(uiElem.width, uiElem.height));
            rect.setPosition(uiElem.x, uiElem.y);
            
            uint32_t colorVal = button.normalColor;
            if (button.state == Components::UIButton::State::Hovered) {
                colorVal = button.hoverColor;
            } else if (button.state == Components::UIButton::State::Pressed) {
                colorVal = button.pressedColor;
            }
            
            sf::Color btnColor(
                (colorVal >> 24) & 0xFF,
                (colorVal >> 16) & 0xFF,
                (colorVal >> 8) & 0xFF,
                colorVal & 0xFF
            );
            rect.setFillColor(btnColor);
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
            
            sfmlWindow.draw(rect);
            
            if (!button.text.empty())
            {
                static sf::Font font;
                static bool fontLoaded = false;
                if (!fontLoaded) {
                    font.loadFromFile("assets/fonts/main_font.ttf");
                    fontLoaded = true;
                }
                
                sf::Text text;
                text.setFont(font);
                text.setString(button.text);
                text.setCharacterSize(24);
                text.setFillColor(sf::Color::White);
                
                sf::FloatRect textBounds = text.getLocalBounds();
                text.setPosition(
                    uiElem.x + (uiElem.width - textBounds.width) / 2.0f - textBounds.left,
                    uiElem.y + (uiElem.height - textBounds.height) / 2.0f - textBounds.top
                );
                
                sfmlWindow.draw(text);
            }
        }
        
        // Render text
        if (coordinator->HasComponent<Components::UIText>(entity))
        {
            auto& uiText = coordinator->GetComponent<Components::UIText>(entity);
            
            static sf::Font font;
            static bool fontLoaded = false;
            if (!fontLoaded) {
                font.loadFromFile("assets/fonts/main_font.ttf");
                fontLoaded = true;
            }
            
            sf::Text text;
            text.setFont(font);
            text.setString(uiText.content);
            text.setCharacterSize(uiText.fontSize);
            
            sf::Color textColor(
                (uiText.color >> 24) & 0xFF,
                (uiText.color >> 16) & 0xFF,
                (uiText.color >> 8) & 0xFF,
                uiText.color & 0xFF
            );
            text.setFillColor(textColor);
            
            sf::FloatRect textBounds = text.getLocalBounds();
            float xPos = uiElem.x;
            if (uiText.alignment == Components::UIText::Alignment::Center) {
                xPos -= textBounds.width / 2.0f;
            } else if (uiText.alignment == Components::UIText::Alignment::Right) {
                xPos -= textBounds.width;
            }
            
            text.setPosition(xPos - textBounds.left, uiElem.y - textBounds.top);
            
            sfmlWindow.draw(text);
        }
        
        // Render sliders
        if (coordinator->HasComponent<Components::UISlider>(entity))
        {
            auto& slider = coordinator->GetComponent<Components::UISlider>(entity);
            
            // Slider track
            sf::RectangleShape track(sf::Vector2f(uiElem.width, 8));
            track.setPosition(uiElem.x, uiElem.y + uiElem.height / 2 - 4);
            track.setFillColor(sf::Color(80, 80, 80));
            track.setOutlineColor(sf::Color(150, 150, 150));
            track.setOutlineThickness(1);
            sfmlWindow.draw(track);
            
            // Slider handle
            float normalizedValue = (slider.currentValue - slider.minValue) / (slider.maxValue - slider.minValue);
            float handleX = uiElem.x + normalizedValue * uiElem.width;
            
            sf::CircleShape handle(12);
            handle.setPosition(handleX - 12, uiElem.y + uiElem.height / 2 - 12);
            handle.setFillColor(sf::Color(0, 200, 255));
            handle.setOutlineColor(sf::Color::White);
            handle.setOutlineThickness(2);
            sfmlWindow.draw(handle);
            
            // Value text
            static sf::Font font;
            static bool fontLoaded = false;
            if (!fontLoaded) {
                font.loadFromFile("assets/fonts/main_font.ttf");
                fontLoaded = true;
            }
            
            sf::Text valueText;
            valueText.setFont(font);
            valueText.setString(std::to_string(static_cast<int>(slider.currentValue)));
            valueText.setCharacterSize(18);
            valueText.setFillColor(sf::Color::White);
            valueText.setPosition(uiElem.x + uiElem.width + 20, uiElem.y + uiElem.height / 2 - 10);
            sfmlWindow.draw(valueText);
        }
        
        // Render checkboxes
        if (coordinator->HasComponent<Components::UICheckbox>(entity))
        {
            auto& checkbox = coordinator->GetComponent<Components::UICheckbox>(entity);
            
            // Checkbox box
            sf::RectangleShape box(sf::Vector2f(30, 30));
            box.setPosition(uiElem.x, uiElem.y);
            box.setFillColor(sf::Color(60, 60, 60));
            box.setOutlineColor(checkbox.checked ? sf::Color(0, 255, 100) : sf::Color(150, 150, 150));
            box.setOutlineThickness(2);
            sfmlWindow.draw(box);
            
            // Checkmark if checked
            if (checkbox.checked) {
                sf::RectangleShape checkmark(sf::Vector2f(20, 20));
                checkmark.setPosition(uiElem.x + 5, uiElem.y + 5);
                checkmark.setFillColor(sf::Color(0, 255, 100));
                sfmlWindow.draw(checkmark);
            }
            
            // Label text
            static sf::Font font;
            static bool fontLoaded = false;
            if (!fontLoaded) {
                font.loadFromFile("assets/fonts/main_font.ttf");
                fontLoaded = true;
            }
            
            sf::Text labelText;
            labelText.setFont(font);
            labelText.setString(checkbox.label);
            labelText.setCharacterSize(20);
            labelText.setFillColor(sf::Color::White);
            labelText.setPosition(uiElem.x + 40, uiElem.y + 3);
            sfmlWindow.draw(labelText);
        }
    }
}

void SettingsState::saveSettings()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    LOG_INFO("SETTINGS", "Saving user settings...");
    
    // Read current values from UI components
    auto& masterVol = coordinator->GetComponent<Components::UISlider>(masterVolumeSlider_);
    auto& musicVol = coordinator->GetComponent<Components::UISlider>(musicVolumeSlider_);
    auto& sfxVol = coordinator->GetComponent<Components::UISlider>(sfxVolumeSlider_);
    auto& vsync = coordinator->GetComponent<Components::UICheckbox>(vsyncCheckbox_);
    auto& fullscreen = coordinator->GetComponent<Components::UICheckbox>(fullscreenCheckbox_);
    auto& showFps = coordinator->GetComponent<Components::UICheckbox>(showFpsCheckbox_);
    
    // Build JSON string manually (simple format)
    std::ostringstream json;
    json << "{\n";
    json << "    \"audio\": {\n";
    json << "        \"master_volume\": " << static_cast<int>(masterVol.currentValue) << ",\n";
    json << "        \"music_volume\": " << static_cast<int>(musicVol.currentValue) << ",\n";
    json << "        \"sfx_volume\": " << static_cast<int>(sfxVol.currentValue) << "\n";
    json << "    },\n";
    json << "    \"graphics\": {\n";
    json << "        \"vsync\": " << (vsync.checked ? "true" : "false") << ",\n";
    json << "        \"fullscreen\": " << (fullscreen.checked ? "true" : "false") << ",\n";
    json << "        \"show_fps\": " << (showFps.checked ? "true" : "false") << "\n";
    json << "    }\n";
    json << "}\n";
    
    // Write to file
    std::ofstream file("assets/config/user_settings.json");
    if (file.is_open()) {
        file << json.str();
        file.close();
        LOG_INFO("SETTINGS", "Settings saved successfully! Master=" + std::to_string(static_cast<int>(masterVol.currentValue))
            + "% Music=" + std::to_string(static_cast<int>(musicVol.currentValue))
            + "% SFX=" + std::to_string(static_cast<int>(sfxVol.currentValue))
            + "% VSync=" + (vsync.checked ? "ON" : "OFF")
            + " Fullscreen=" + (fullscreen.checked ? "ON" : "OFF")
            + " ShowFPS=" + (showFps.checked ? "ON" : "OFF"));
    } else {
        LOG_ERROR("SETTINGS", "Failed to open settings file for writing!");
    }

    // Apply audio volumes to MusicManager and SFXManager in real-time
    if (auto* musicMgr = game_->getMusicManager()) {
        musicMgr->setMasterVolume(masterVol.currentValue);
        musicMgr->setMusicVolume(musicVol.currentValue);
    }
    if (auto* sfxMgr = game_->getSFXManager()) {
        sfxMgr->setMasterVolume(masterVol.currentValue);
        sfxMgr->setSFXVolume(sfxVol.currentValue);
    }

    // Update GameConfig too
    game_->getConfig().audio.musicVolume = musicVol.currentValue;
    game_->getConfig().audio.sfxVolume = sfxVol.currentValue;
}

void SettingsState::loadSettings()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    LOG_INFO("SETTINGS", "Loading user settings...");
    
    std::ifstream file("assets/config/user_settings.json");
    if (!file.is_open()) {
        LOG_WARNING("SETTINGS", "Settings file not found, using defaults");
        return;
    }
    
    // Simple JSON parsing (looking for numbers and booleans)
    std::string line;
    int masterVol = 100, musicVol = 70, sfxVol = 80;
    bool vsync = true, fullscreen = false, showFps = true;
    
    while (std::getline(file, line)) {
        // Parse master_volume
        if (line.find("\"master_volume\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                masterVol = std::stoi(value);
            }
        }
        // Parse music_volume
        else if (line.find("\"music_volume\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                musicVol = std::stoi(value);
            }
        }
        // Parse sfx_volume
        else if (line.find("\"sfx_volume\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                sfxVol = std::stoi(value);
            }
        }
        // Parse vsync
        else if (line.find("\"vsync\"") != std::string::npos) {
            vsync = (line.find("true") != std::string::npos);
        }
        // Parse fullscreen
        else if (line.find("\"fullscreen\"") != std::string::npos) {
            fullscreen = (line.find("true") != std::string::npos);
        }
        // Parse show_fps
        else if (line.find("\"show_fps\"") != std::string::npos) {
            showFps = (line.find("true") != std::string::npos);
        }
    }
    file.close();
    
    // Apply loaded values to UI components
    auto& masterVolSlider = coordinator->GetComponent<Components::UISlider>(masterVolumeSlider_);
    auto& musicVolSlider = coordinator->GetComponent<Components::UISlider>(musicVolumeSlider_);
    auto& sfxVolSlider = coordinator->GetComponent<Components::UISlider>(sfxVolumeSlider_);
    auto& vsyncCheck = coordinator->GetComponent<Components::UICheckbox>(vsyncCheckbox_);
    auto& fullscreenCheck = coordinator->GetComponent<Components::UICheckbox>(fullscreenCheckbox_);
    auto& showFpsCheck = coordinator->GetComponent<Components::UICheckbox>(showFpsCheckbox_);
    
    masterVolSlider.currentValue = static_cast<float>(masterVol);
    musicVolSlider.currentValue = static_cast<float>(musicVol);
    sfxVolSlider.currentValue = static_cast<float>(sfxVol);
    vsyncCheck.checked = vsync;
    fullscreenCheck.checked = fullscreen;
    showFpsCheck.checked = showFps;
    
    LOG_INFO("SETTINGS", "Settings loaded: Master=" + std::to_string(masterVol)
        + "% Music=" + std::to_string(musicVol)
        + "% SFX=" + std::to_string(sfxVol)
        + "% VSync=" + (vsync ? "ON" : "OFF")
        + " Fullscreen=" + (fullscreen ? "ON" : "OFF")
        + " ShowFPS=" + (showFps ? "ON" : "OFF"));

    // Apply loaded audio volumes to MusicManager and SFXManager
    if (auto* musicMgr = game_->getMusicManager()) {
        musicMgr->setMasterVolume(static_cast<float>(masterVol));
        musicMgr->setMusicVolume(static_cast<float>(musicVol));
    }
    if (auto* sfxMgr = game_->getSFXManager()) {
        sfxMgr->setMasterVolume(static_cast<float>(masterVol));
        sfxMgr->setSFXVolume(static_cast<float>(sfxVol));
    }
}
