/**
 * Game.hpp - Main Game Class
 * 
 * Manages the game lifecycle, ECS coordinator, window, and state manager.
 */

#pragma once

#include <memory>
#include <ecs/Coordinator.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <scripting/LuaState.hpp>
#include "GameConfig.hpp"

// Forward declarations
class StateManager;
class NetworkManager;
class UISystem;
class AudioManager;

namespace eng::engine::rendering {
    class IRenderer;
}

class Game
{
public:
    Game();
    ~Game();

    /**
     * Initialize the game (window, ECS, Lua, systems)
     * @return true if initialization succeeded
     */
    bool initialize();

    /**
     * Main game loop
     */
    void run();

    /**
     * Shutdown and cleanup
     */
    void shutdown();

    // Getters for subsystems
    ECS::Coordinator* getCoordinator() { return coordinator_.get(); }
    eng::engine::rendering::sfml::SFMLWindow* getWindow() { return window_.get(); }
    eng::engine::rendering::IRenderer* getRenderer() { return renderer_.get(); }
    Scripting::LuaState& getLuaState() { return Scripting::LuaState::Instance(); }
    StateManager* getStateManager() { return stateManager_.get(); }
    NetworkManager* getNetworkManager() { return networkManager_.get(); }
    UISystem* getUISystem() { return uiSystem_.get(); }
    AudioManager* getAudioManager() { return audioManager_.get(); }
    GameConfig& getConfig() { return config_; }

    /**
     * Reset ECS coordinator — destroys all entities/systems and re-registers
     * everything from scratch. Call from PlayState::onExit() to allow replaying.
     */
    void resetCoordinator();

private:
    void loadConfigurations();
    void setupECS();
    void setupLuaBindings();
    void handleEvents();
    void update(float deltaTime);
    void render();

    // Core subsystems
    std::unique_ptr<ECS::Coordinator> coordinator_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLWindow> window_;
    std::unique_ptr<eng::engine::rendering::IRenderer> renderer_;
    std::unique_ptr<StateManager> stateManager_;
    std::unique_ptr<NetworkManager> networkManager_;
    std::unique_ptr<AudioManager> audioManager_;
    
    // Systems
    std::shared_ptr<UISystem> uiSystem_;
    
    // Configuration
    GameConfig config_;
    
    // Game state
    bool isRunning_;
    float fixedTimeStep_;
    float accumulator_;
};
