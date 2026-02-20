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
#include <unordered_map>
#include <unordered_set>

// Forward declarations
namespace ECS {
    class Coordinator;
}
struct Position;  // Forward declare Position component
namespace eng::engine::rendering {
    class ISprite;
    namespace sfml {
        class SFMLTexture;
        class SFMLSprite;
        class SFMLText;
        class SFMLFont;
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
    void loadEnemiesConfig();   // Load enemies configuration from Lua
    void loadVFXConfig();       // Load VFX configuration from Lua
    void spawnPlayer();
    void spawnBackground(int level = 1);
    void spawnChargeIndicator();
    void updateChargeIndicator(float chargeTime);
    void handleShooting();
    int calculateChargeLevel() const;  // Calculate charge level from chargeTime_
    
    // Score system
    uint32_t loadHighScore();
    void saveHighScore(uint32_t score);
    void addScore(uint32_t points);
    
    // Level system
    void updateLevelSystem(float deltaTime);
    void spawnBoss();
    void startLevel(int level);
    
    // Enemy system
    void spawnEnemy(const std::string& enemyType, float x, float y);  // Spawn an enemy
    void spawnTestEnemies();  // Spawn enemies for testing
    void updateEnemyMovement(float deltaTime);  // Update enemy movement patterns
    void updateEnemyFiring(float deltaTime);   // Update enemy weapon firing
    void spawnEnemyProjectile(const Position& pos, float dx, float dy, int damage, float speed);  // Helper to spawn projectile
    void checkProjectileEnemyCollisions();  // Check player bullets hitting enemies
    void checkPlayerEnemyCollisions();  // Check enemies/enemy bullets hitting player
    
    // Module shooting systems
    void fireSpreadModule();  // Fire 3 projectiles in fan pattern
    void fireWaveModule();    // Fire wave projectile with sinusoidal motion
    void fireHomingModule();  // Fire homing missile that tracks enemies
    void startLaserBeam();    // Start continuous laser beam
    void stopLaserBeam();     // Stop laser beam
    void updateWaveProjectiles(float deltaTime);  // Update wave motion
    void updateHomingProjectiles(float deltaTime); // Update homing tracking
    void updateLaserBeam(float deltaTime);        // Update laser beam position
    
    // Collectables
    void spawnTestCollectables();  // Spawn collectables for testing
    void spawnPowerup(float x, float y, const std::string& type);
    void spawnModule(float x, float y, const std::string& moduleType);
    void checkCollectableCollisions();  // Check collisions with player
    void pickupPowerup(ECS::Entity powerupEntity, const std::string& type);
    void attachModule(ECS::Entity moduleEntity, const std::string& moduleType);
    void updateAttachedModule();  // Update module position to follow player
    
    eng::engine::rendering::ISprite* loadSprite(const std::string& texturePath, const eng::engine::rendering::IntRect* rect = nullptr);

    // Player state
    ECS::Entity playerEntity_;
    float shootCooldown_;
    float timeSinceLastShot_;
    bool isCharging_;
    float chargeTime_;
    
    // Shield state
    bool shieldActive_;
    float shieldTimer_;
    const float SHIELD_DURATION = 10.0f;

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
    
    // Equipped module tracking
    ECS::Entity equippedModuleEntity_;  // Entity of the attached module (0 if none)
    std::string equippedModuleType_;    // Type of equipped module ("laser", "homing", etc.)
    ECS::Entity laserBeamEntity_;       // Entity of the active laser beam (0 if none)

    // Enemy tracking for optimized iteration
    std::vector<ECS::Entity> activeEnemies_;  // Track all spawned enemies for efficient updates
    std::vector<ECS::Entity> activeCollectables_; // Track powerups/modules for efficient pickup checks
    std::unordered_set<ECS::Entity> kamikazeEntities_; // Enemies with homing_player movement pattern

    // Game config from Lua
    int windowWidth_;
    int windowHeight_;
    float inputSystemSpeed_;  // Speed used by InputSystem (for rescaling)
    ECS::Entity backgroundEntity_ = 0;  // Track current background entity
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
    
    // VFX config from Lua
    sol::table vfxConfig_;  // Full VFX config table from Lua
    
    // Enemy fire patterns (maps entity ID to fire pattern string)
    std::unordered_map<ECS::Entity, std::string> enemyFirePatterns_;

    // Loaded sprites/textures (we own them)
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture>> loadedTextures_;
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLSprite>> loadedSprites_;
    
    // UI elements for score display
    std::unique_ptr<eng::engine::rendering::sfml::SFMLFont> scoreFont_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> scoreText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> levelText_;

    // Boss health bar UI
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> bossNameText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> bossHpText_;
    int bossMaxHealth_ = 0;
    
    // Level system state
    int currentLevel_ = 1;
    bool gameOverTriggered_ = false;
    float levelTimer_ = 0.0f;
    float enemySpawnTimer_ = 0.0f;
    float powerupSpawnTimer_ = 0.0f;
    float moduleSpawnTimer_ = 0.0f;
    int currentWaveIndex_ = 0;
    bool bossSpawned_ = false;
    ECS::Entity bossEntity_ = 0;
    bool bossAlive_ = false;
    bool levelActive_ = false;
    float levelTransitionTimer_ = 0.0f;
    bool showLevelText_ = false;
    uint8_t moduleRotationIdx_ = 0;
    float bossMovementTimer_ = 0.0f; // Timer for boss bobbing movement
    
    // Wave spawn state
    struct WaveSpawnState {
        int enemyIdx = 0;
        int spawnedCount = 0;
        float spawnTimer = 0.0f;
        bool active = false;
    };
    WaveSpawnState waveSpawnState_;
};
