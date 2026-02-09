/**
 * PlayState.hpp - Gameplay State (Phase 6-7)
 */

#pragma once

#include "GameState.hpp"
#include <ecs/Types.hpp>
#include <rendering/Types.hpp>
#include <scripting/LuaState.hpp>
#include <memory>
#include <vector>

// Forward declarations
namespace ECS {
    class Coordinator;
}
namespace eng::engine::rendering {
    class ISprite;
    namespace sfml {
        class SFMLTexture;
        class SFMLSprite;
    }
}
class InputSystem;
class MovementSystem;
class RenderSystem;
class AnimationSystem;
class CollisionSystem;
class ScrollingBackgroundSystem;
class LifetimeSystem;
class BoundarySystem;

class PlayState : public GameState
{
public:
    explicit PlayState(Game* game);
    ~PlayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "Play"; }

private:
    // Helper methods
    void setupSystems();
    void loadGameConfig();
    void loadPlayerConfig();
    void loadWeaponsConfig();
    void loadCollectablesConfig();
    void spawnPlayer();
    void spawnBackground();
    void spawnChargeIndicator();
    void updateChargeIndicator(float chargeTime);
    void handleShooting();
    int calculateChargeLevel() const;  // Calculate charge level from chargeTime_
    
    // Collectables
    void spawnTestCollectables();  // Spawn collectables for testing
    void spawnPowerup(float x, float y, const std::string& type);
    void spawnModule(float x, float y, const std::string& moduleType);
    
    eng::engine::rendering::ISprite* loadSprite(const std::string& texturePath, const eng::engine::rendering::IntRect* rect = nullptr);

    // Player state
    ECS::Entity playerEntity_;
    float shootCooldown_;
    float timeSinceLastShot_;
    bool isCharging_;
    float chargeTime_;

    // Systems (stored as shared_ptr)
    std::shared_ptr<InputSystem> inputSystem_;
    std::shared_ptr<MovementSystem> movementSystem_;
    std::shared_ptr<RenderSystem> renderSystem_;
    std::shared_ptr<AnimationSystem> animationSystem_;
    std::shared_ptr<CollisionSystem> collisionSystem_;
    std::shared_ptr<ScrollingBackgroundSystem> scrollingSystem_;
    std::shared_ptr<LifetimeSystem> lifetimeSystem_;
    std::shared_ptr<BoundarySystem> boundarySystem_;

    // Charge animation entity
    ECS::Entity chargeIndicatorEntity_;

    // Game config from Lua
    int windowWidth_;
    int windowHeight_;
    float inputSystemSpeed_;  // Speed used by InputSystem (for rescaling)
    std::string backgroundPath_;
    float backgroundScrollSpeed_;
    int backgroundOriginalWidth_;
    int backgroundOriginalHeight_;
    bool backgroundScaleToWindow_;

    // Player config from Lua
    float playerSpeed_;
    float shootCooldownTime_;
    int playerMaxHealth_;
    
    // Weapon config from Lua
    float projectileSpeed_;
    float projectileLifetime_;
    int projectileDamage_;
    std::vector<float> chargeThresholds_;  // Charge time thresholds for each level
    sol::table weaponsConfig_;  // Full weapons config table from Lua

    // Loaded sprites/textures (we own them)
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture>> loadedTextures_;
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLSprite>> loadedSprites_;
};
