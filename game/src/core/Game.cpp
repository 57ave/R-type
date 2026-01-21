/**
 * Game.cpp - Main Game Implementation
 */

#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include "states/MainMenuState.hpp"
#include <rendering/sfml/SFMLRenderer.hpp>
#include <engine/Clock.hpp>

// Components
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Health.hpp>
#include <components/Collider.hpp>
#include <components/Damage.hpp>
#include <components/AudioSource.hpp>
#include <components/Boundary.hpp>
#include <components/Lifetime.hpp>
#include <components/NetworkId.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Tag.hpp>
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UIPanel.hpp>
#include <components/UISlider.hpp>
#include <components/UICheckbox.hpp>
#include <components/UIInputField.hpp>
#include <components/UIDropdown.hpp>

// Systems
#include <systems/MovementSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/AudioSystem.hpp>
#include <systems/BoundarySystem.hpp>
#include <systems/HealthSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <systems/StateMachineAnimationSystem.hpp>
#include <systems/UISystem.hpp>
#include <systems/InputSystem.hpp>

#include <iostream>

// Use UI components namespace
using namespace Components;

Game::Game()
    : isRunning_(false)
    , fixedTimeStep_(1.0f / 60.0f)
    , accumulator_(0.0f)
{
}

Game::~Game()
{
}

bool Game::initialize()
{
    std::cout << "[GAME] Initializing R-Type..." << std::endl;

    // Create window
    window_ = std::make_unique<eng::engine::rendering::sfml::SFMLWindow>();
    window_->create(
        config_.window.width,
        config_.window.height,
        config_.window.title
    );

    if (!window_ || !window_->isOpen())
    {
        std::cerr << "[GAME] Failed to create window" << std::endl;
        return false;
    }

    // Configure window settings via SFML window
    window_->getSFMLWindow().setFramerateLimit(config_.window.frameRateLimit);
    window_->getSFMLWindow().setVerticalSyncEnabled(config_.window.vsync);

    std::cout << "[GAME] Window created: " << config_.window.width << "x" 
              << config_.window.height << std::endl;

    // Create renderer (SFMLRenderer takes sf::RenderWindow*, get it from SFMLWindow)
    renderer_ = std::make_unique<eng::engine::rendering::sfml::SFMLRenderer>(&window_->getSFMLWindow());

    // Create ECS Coordinator
    coordinator_ = std::make_unique<ECS::Coordinator>();
    coordinator_->Init();
    std::cout << "[GAME] ECS initialized" << std::endl;

    // Setup ECS (register components and systems)
    setupECS();

    // Initialize Lua
    Scripting::LuaState::Instance().Init();
    setupLuaBindings();
    std::cout << "[GAME] Lua initialized" << std::endl;

    // Load configurations from Lua
    loadConfigurations();

    // Create state manager
    stateManager_ = std::make_unique<StateManager>(this);
    
    // Create network manager
    networkManager_ = std::make_unique<NetworkManager>();
    networkManager_->initialize();

    // Push initial state (Main Menu)
    stateManager_->pushState(std::make_unique<MainMenuState>(this));

    isRunning_ = true;
    std::cout << "[GAME] Initialization complete!" << std::endl;

    return true;
}

void Game::run()
{
    std::cout << "[GAME] Starting game loop..." << std::endl;

    eng::engine::Clock clock;
    
    while (isRunning_ && window_->isOpen())
    {
        float deltaTime = clock.restart();
        accumulator_ += deltaTime;

        // Handle events
        handleEvents();

        // Fixed timestep updates
        while (accumulator_ >= fixedTimeStep_)
        {
            update(fixedTimeStep_);
            accumulator_ -= fixedTimeStep_;
        }

        // Render
        render();

        // Check if we still have states
        if (!stateManager_->hasStates())
        {
            isRunning_ = false;
        }
    }

    std::cout << "[GAME] Game loop ended" << std::endl;
}

void Game::shutdown()
{
    std::cout << "[GAME] Shutting down..." << std::endl;

    stateManager_->clearStates();
    
    if (networkManager_ && networkManager_->isConnected())
    {
        networkManager_->disconnect();
    }

    Scripting::LuaState::Instance().Shutdown();

    std::cout << "[GAME] Shutdown complete" << std::endl;
}

void Game::handleEvents()
{
    eng::engine::InputEvent event;
    while (window_->pollEvent(event))
    {
        if (event.type == eng::engine::EventType::Closed)
        {
            isRunning_ = false;
            window_->close();
        }

        // Pass event to current state
        if (stateManager_->hasStates())
        {
            stateManager_->handleEvent(event);
        }
        
        // Pass event to UISystem for UI interactions
        if (uiSystem_)
        {
            uiSystem_->HandleEvent(event);
        }
    }
}

void Game::update(float deltaTime)
{
    // Update Lua hot-reload (check for script changes)
    Scripting::LuaState::Instance().CheckForChanges();

    // Update network
    if (networkManager_)
    {
        networkManager_->update();
    }

    // Update UISystem
    if (uiSystem_)
    {
        uiSystem_->Update(deltaTime);
    }

    // Update current state
    if (stateManager_->hasStates())
    {
        stateManager_->update(deltaTime);
    }
}

void Game::render()
{
    window_->clear();

    // Render current state first (background, game entities, etc.)
    if (stateManager_->hasStates())
    {
        stateManager_->render();
    }

    // Render UI on top
    if (uiSystem_)
    {
        uiSystem_->Render(window_.get());
    }

    window_->display();
}

void Game::setupECS()
{
    std::cout << "[GAME] Registering ECS components..." << std::endl;

    // Register essential components from engine (NO namespace)
    coordinator_->RegisterComponent<Position>();
    coordinator_->RegisterComponent<Velocity>();
    coordinator_->RegisterComponent<Sprite>();
    coordinator_->RegisterComponent<Animation>();
    coordinator_->RegisterComponent<Health>();
    coordinator_->RegisterComponent<Collider>();
    coordinator_->RegisterComponent<Damage>();
    coordinator_->RegisterComponent<Boundary>();
    coordinator_->RegisterComponent<Lifetime>();
    coordinator_->RegisterComponent<NetworkId>();
    coordinator_->RegisterComponent<ScrollingBackground>();
    coordinator_->RegisterComponent<Tag>();

    // Register UI components (Phase 4)
    coordinator_->RegisterComponent<UIElement>();
    coordinator_->RegisterComponent<UIText>();
    coordinator_->RegisterComponent<UIButton>();
    coordinator_->RegisterComponent<UIPanel>();
    coordinator_->RegisterComponent<UISlider>();
    coordinator_->RegisterComponent<UICheckbox>();
    coordinator_->RegisterComponent<UIInputField>();
    coordinator_->RegisterComponent<UIDropdown>();

    std::cout << "[GAME] " << 20 << " components registered (12 gameplay + 8 UI)" << std::endl;

    // === Register and configure Systems (Phase 4) ===
    std::cout << "[GAME] Registering ECS systems..." << std::endl;
    
    // Register UISystem with coordinator (it returns the shared_ptr)
    uiSystem_ = coordinator_->RegisterSystem<UISystem>(coordinator_.get());
    uiSystem_->SetRenderer(renderer_.get());
    uiSystem_->SetWindow(window_.get());
    // LuaState will be set in setupLuaBindings() after Lua is initialized
    
    // Define UISystem signature (components it needs)
    ECS::Signature uiSignature;
    uiSignature.set(coordinator_->GetComponentType<UIElement>());
    coordinator_->SetSystemSignature<UISystem>(uiSignature);
    
    // Initialize UISystem
    uiSystem_->Init();
    
    // Load default font
    uiSystem_->LoadFont("default", "assets/fonts/main_font.ttf");
    
    std::cout << "[GAME] ✅ UISystem registered and configured" << std::endl;
    std::cout << "[GAME] Systems setup complete" << std::endl;
}

void Game::setupLuaBindings()
{
    std::cout << "[GAME] Setting up Lua bindings..." << std::endl;
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    // Configure UISystem with LuaState
    if (uiSystem_) {
        uiSystem_->SetLuaState(&lua);
        std::cout << "[GAME] UISystem connected to Lua" << std::endl;
    }

    // Expose ECS Coordinator
    lua["ECS"] = lua.create_table();
    
    // Create Entity
    lua["ECS"]["CreateEntity"] = [this]() -> ECS::Entity {
        return coordinator_->CreateEntity();
    };
    
    // Destroy Entity
    lua["ECS"]["DestroyEntity"] = [this](ECS::Entity entity) {
        coordinator_->DestroyEntity(entity);
    };
    
    // Add Components
    lua["ECS"]["AddPosition"] = [this](ECS::Entity entity, float x, float y) {
        Position pos;
        pos.x = x;
        pos.y = y;
        coordinator_->AddComponent(entity, pos);
    };
    
    lua["ECS"]["AddVelocity"] = [this](ECS::Entity entity, float dx, float dy) {
        Velocity vel;
        vel.dx = dx;
        vel.dy = dy;
        coordinator_->AddComponent(entity, vel);
    };
    
    lua["ECS"]["AddHealth"] = [this](ECS::Entity entity, int current, int max) {
        Health health;
        health.current = current;
        health.max = max;
        coordinator_->AddComponent(entity, health);
    };
    
    lua["ECS"]["AddDamage"] = [this](ECS::Entity entity, int amount) {
        Damage damage;
        damage.amount = amount;
        coordinator_->AddComponent(entity, damage);
    };
    
    lua["ECS"]["AddTag"] = [this](ECS::Entity entity, const std::string& tag) {
        Tag tagComp;
        tagComp.name = tag;
        coordinator_->AddComponent(entity, tagComp);
    };
    
    // Get Components (read-only for now)
    lua["ECS"]["GetPosition"] = [this](ECS::Entity entity) -> sol::optional<sol::table> {
        if (coordinator_->HasComponent<Position>(entity)) {
            auto& pos = coordinator_->GetComponent<Position>(entity);
            auto& luaState = Scripting::LuaState::Instance().GetState();
            sol::table result = luaState.create_table();
            result["x"] = pos.x;
            result["y"] = pos.y;
            return result;
        }
        return sol::nullopt;
    };
    
    lua["ECS"]["GetHealth"] = [this](ECS::Entity entity) -> sol::optional<sol::table> {
        if (coordinator_->HasComponent<Health>(entity)) {
            auto& health = coordinator_->GetComponent<Health>(entity);
            auto& luaState = Scripting::LuaState::Instance().GetState();
            sol::table result = luaState.create_table();
            result["current"] = health.current;
            result["max"] = health.max;
            return result;
        }
        return sol::nullopt;
    };
    
    // Utility to get Coordinator (for advanced users)
    lua["ECS"]["GetCoordinator"] = [this]() -> ECS::Coordinator* {
        return coordinator_.get();
    };
    
    // === UI Component Bindings (Phase 4) ===
    
    // AddUIButton - Only parameters from Lua, rest uses defaults
    lua["ECS"]["AddUIButton"] = [this](ECS::Entity entity, float x, float y, float width, float height, 
                                        const std::string& text, const std::string& callback) {
        UIElement uiElem;
        uiElem.x = x;
        uiElem.y = y;
        uiElem.width = width;
        uiElem.height = height;
        coordinator_->AddComponent(entity, uiElem);
        
        Position pos{x, y};
        coordinator_->AddComponent(entity, pos);
        
        UIButton button;
        button.text = text;
        button.onClickCallback = callback;
        coordinator_->AddComponent(entity, button);
        
        Tag tag{"button"};
        coordinator_->AddComponent(entity, tag);
    };
    
    // AddUIText - Only parameters from Lua, rest uses defaults
    lua["ECS"]["AddUIText"] = [this](ECS::Entity entity, float x, float y, 
                                      const std::string& text, int fontSize) {
        UIElement uiElem;
        uiElem.x = x;
        uiElem.y = y;
        coordinator_->AddComponent(entity, uiElem);
        
        Position pos{x, y};
        coordinator_->AddComponent(entity, pos);
        
        UIText uiText;
        uiText.content = text;
        uiText.fontSize = fontSize;
        coordinator_->AddComponent(entity, uiText);
        
        Tag tag{"text"};
        coordinator_->AddComponent(entity, tag);
    };
    
    // AddUIPanel - Only parameters from Lua, rest uses defaults
    lua["ECS"]["AddUIPanel"] = [this](ECS::Entity entity, float x, float y, float width, float height) {
        UIElement uiElem;
        uiElem.x = x;
        uiElem.y = y;
        uiElem.width = width;
        uiElem.height = height;
        coordinator_->AddComponent(entity, uiElem);
        
        Position pos{x, y};
        coordinator_->AddComponent(entity, pos);
        
        UIPanel panel;
        // Make panel visible with proper colors (from engine defaults but ensure visibility)
        panel.backgroundColor = 0x323232CC;  // Dark gray, semi-transparent (more visible than black)
        panel.borderColor = 0x646464FF;      // Gray border
        panel.borderThickness = 2.0f;        // Visible border
        coordinator_->AddComponent(entity, panel);
        
        Tag tag{"panel"};
        coordinator_->AddComponent(entity, tag);
    };
    
    // AddUISlider - Only parameters from Lua, rest uses defaults
    lua["ECS"]["AddUISlider"] = [this](ECS::Entity entity, float x, float y, float width, float height,
                                        float minVal, float maxVal, float defaultVal) {
        UIElement uiElem;
        uiElem.x = x;
        uiElem.y = y;
        uiElem.width = width;
        uiElem.height = height;
        coordinator_->AddComponent(entity, uiElem);
        
        Position pos{x, y};
        coordinator_->AddComponent(entity, pos);
        
        UISlider slider;
        slider.minValue = minVal;
        slider.maxValue = maxVal;
        slider.currentValue = defaultVal;
        coordinator_->AddComponent(entity, slider);
        
        Tag tag{"slider"};
        coordinator_->AddComponent(entity, tag);
    };
    
    // AddUICheckbox - Only parameters from Lua, rest uses defaults
    lua["ECS"]["AddUICheckbox"] = [this](ECS::Entity entity, float x, float y, 
                                          const std::string& label, bool defaultState) {
        UIElement uiElem;
        uiElem.x = x;
        uiElem.y = y;
        uiElem.width = 30 + 10 + label.length() * 12; // Box + spacing + label estimate
        uiElem.height = 30;
        coordinator_->AddComponent(entity, uiElem);
        
        Position pos{x, y};
        coordinator_->AddComponent(entity, pos);
        
        UICheckbox checkbox;
        checkbox.label = label;
        checkbox.checked = defaultState;
        coordinator_->AddComponent(entity, checkbox);
        
        Tag tag{"checkbox"};
        coordinator_->AddComponent(entity, tag);
    };
    
    std::cout << "[GAME] ✅ Lua bindings registered (ECS + UI exposed to Lua)" << std::endl;
}

void Game::loadConfigurations()
{
    std::cout << "[GAME] Loading configurations from Lua..." << std::endl;
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    // Load init.lua which loads all other configs
    try {
        lua.script_file("assets/scripts/init.lua");
        std::cout << "[GAME] ✅ init.lua loaded successfully" << std::endl;
        
        // Access loaded configurations
        sol::optional<sol::table> gameConfig = lua["Game"];
        if (gameConfig) {
            std::cout << "[GAME] ✅ Game configuration accessible from Lua" << std::endl;
        }
        
        sol::optional<sol::table> playerConfig = lua["Player"];
        if (playerConfig) {
            std::cout << "[GAME] ✅ Player configuration accessible from Lua" << std::endl;
        }
        
        sol::optional<sol::table> weaponsConfig = lua["Weapons"];
        if (weaponsConfig) {
            std::cout << "[GAME] ✅ Weapons configuration accessible from Lua" << std::endl;
        }
        
        sol::optional<sol::table> enemiesConfig = lua["Enemies"];
        if (enemiesConfig) {
            std::cout << "[GAME] ✅ Enemies configuration accessible from Lua" << std::endl;
        }
        
        sol::optional<sol::table> bossesConfig = lua["Bosses"];
        if (bossesConfig) {
            std::cout << "[GAME] ✅ Bosses configuration accessible from Lua" << std::endl;
        }
        
        sol::optional<sol::table> assetsConfig = lua["Assets"];
        if (assetsConfig) {
            std::cout << "[GAME] ✅ Assets configuration accessible from Lua" << std::endl;
        }
        
        // Test ECS-Lua integration
        std::cout << "[GAME] Testing Lua-ECS integration..." << std::endl;
        lua.script_file("assets/scripts/test_entity.lua");
        std::cout << "[GAME] ✅ Lua-ECS integration test complete" << std::endl;
        
    } catch (const sol::error& e) {
        std::cerr << "[GAME] ❌ Error loading Lua configurations: " << e.what() << std::endl;
    }
    
    std::cout << "[GAME] Configurations loaded from Lua" << std::endl;
}

