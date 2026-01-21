#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Engine includes - Core
#include <ecs/Coordinator.hpp>
#include <ecs/ECS.hpp>

// Engine includes - Rendering
#include <rendering/sfml/SFMLRenderer.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLWindow.hpp>

// Engine includes - Input/Time abstractions
#include <engine/Clock.hpp>
#include <engine/Input.hpp>
#include <engine/Keyboard.hpp>

// Scripting - Engine generic
#include <scripting/ComponentBindings.hpp>
#include <scripting/LuaState.hpp>
#include <scripting/UIBindings.hpp>

// Network
#include <network/NetworkClient.hpp>

#include "NetworkBindings.hpp"

// Generic Engine Systems
#include <systems/AnimationSystem.hpp>
#include <systems/HealthSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/MovementSystem.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/UISystem.hpp>
// Note: CollisionSystem not used - collisions handled in Lua

// Generic Engine Components
#include <components/Animation.hpp>
#include <components/Lifetime.hpp>
#include <components/Position.hpp>  // Used by RenderSystem
#include <components/Sprite.hpp>    // Used by RenderSystem
#include <components/Tag.hpp>       // Used by RenderSystem
#include <ecs/Components.hpp>       // For Transform, Velocity, Sprite, etc. (used by Lua bindings)

namespace FlappyBird {

/**
 * @brief Main Flappy Bird game class
 *
 * This class is intentionally minimal - it only handles:
 * - ECS initialization
 * - Lua script loading
 * - SFML render loop
 * - Input event forwarding to Lua
 *
 * All game logic is implemented in Lua scripts.
 */
class FlappyGame {
public:
    FlappyGame();
    ~FlappyGame();

    /**
     * @brief Main entry point
     * @param argc Argument count
     * @param argv Arguments
     * @return Exit code
     */
    int Run(int argc, char* argv[]);

private:
    // --- Initialization ---
    void InitECS();
    void InitSystems();
    void InitLua();
    void LoadAssets();

    // --- Game loop helpers ---
    void ProcessEvents(eng::engine::rendering::sfml::SFMLWindow& window);
    void Update(float dt);
    void Render(eng::engine::rendering::sfml::SFMLRenderer& renderer);

    // --- Entity management ---
    void RegisterEntity(ECS::Entity entity);
    void DestroyEntityDeferred(ECS::Entity entity);
    void ProcessDestroyedEntities();

    // --- Asset path resolution ---
    std::string ResolveAssetPath(const std::string& relativePath);

    // ECS Coordinator
    ECS::Coordinator gCoordinator;

    // Entity tracking
    std::vector<ECS::Entity> allEntities;
    std::vector<ECS::Entity> entitiesToDestroy;
    std::vector<eng::engine::rendering::sfml::SFMLSprite*> allSprites;

    // Textures
    std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture> birdTexture;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture> pipeTexture;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture> backgroundTexture;

    // Systems (shared pointers managed by ECS)
    std::shared_ptr<MovementSystem> movementSystem;
    std::shared_ptr<AnimationSystem> animationSystem;
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<HealthSystem> healthSystem;
    std::shared_ptr<LifetimeSystem> lifetimeSystem;
    std::shared_ptr<UISystem> uiSystem;

    // UI rendering (simple - rectangles and text only)
    std::unique_ptr<sf::Font> uiFont;
    sf::RenderWindow* currentWindow = nullptr;  // Set during render loop

    // Network client (optional - only if multiplayer mode)
    std::unique_ptr<NetworkClient> networkClient;
    std::string serverAddress;
    short serverPort;

    // Base asset path
    std::string basePath;

    // Game state
    bool shouldQuit = false;

    // Input state tracking (for "JustPressed" detection)
    bool spaceWasPressed = false;
    bool escapeWasPressed = false;
    bool mWasPressed = false;
    bool key1WasPressed = false;
    bool key2WasPressed = false;
    bool key3WasPressed = false;
};

}  // namespace FlappyBird
