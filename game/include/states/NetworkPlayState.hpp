/**
 * NetworkPlayState.hpp - Multiplayer Gameplay State (Phase 6.5)
 * 
 * État de jeu synchronisé par le serveur. Le serveur est autoritaire :
 * - Le client envoie ses inputs (CLIENT_INPUT)
 * - Le serveur calcule la physique et renvoie les positions (WORLD_SNAPSHOT)
 * - Le client affiche simplement les entités selon le snapshot
 */

#pragma once

#include "GameState.hpp"
#include "network/RTypeProtocol.hpp"
#include <ecs/Types.hpp>
#include <rendering/Types.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declarations
namespace ECS {
    class Coordinator;
}
namespace eng::engine::rendering {
    class ISprite;
    namespace sfml {
        class SFMLTexture;
        class SFMLSprite;
        class SFMLText;
        class SFMLFont;
    }
}
class RenderSystem;
class AnimationSystem;
class ScrollingBackgroundSystem;

class NetworkPlayState : public GameState
{
public:
    explicit NetworkPlayState(Game* game);
    ~NetworkPlayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "NetworkPlay"; }

private:
    // Helper methods
    void setupSystems();
    void loadGameConfig();
    void loadPlayerConfig();
    void spawnBackground();
    
    // Network synchronization
    void onWorldSnapshot(const std::vector<RType::EntityState>& entities);
    void syncEntityFromState(const RType::EntityState& state);
    void removeStaleEntities(const std::vector<RType::EntityState>& entities);
    
    // Sprite loading
    eng::engine::rendering::ISprite* loadSprite(const std::string& texturePath, const eng::engine::rendering::IntRect* rect = nullptr);
    
    // Get sprite info based on entity type and state
    struct SpriteInfo {
        std::string texturePath;
        eng::engine::rendering::IntRect textureRect;
        float scaleX;
        float scaleY;
        int layer;
        // Animation data (frameCount=0 means no animation)
        int frameCount = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        float frameTime = 0.1f;
        int spacing = 0;
        bool loop = true;
        bool vertical = false; // Vertical spritesheet (frames stacked top-to-bottom)
    };
    SpriteInfo getSpriteInfo(const RType::EntityState& state);

    // Entity tracking (server entity ID -> local ECS entity)
    std::unordered_map<uint32_t, ECS::Entity> networkEntities_;
    
    // Local player info
    uint32_t localPlayerId_ = 0;
    ECS::Entity localPlayerEntity_ = 0;

    // Charge indicator (client-side visual, follows local player)
    ECS::Entity chargeIndicatorEntity_ = 0;
    void spawnChargeIndicator();
    void updateChargeIndicator(float deltaTime);

    // Attached module visual (client-side, follows local player)
    ECS::Entity attachedModuleEntity_ = 0;
    uint8_t currentModuleType_ = 0; // 0=none, 1=laser, 2=homing, 3=spread, 4=wave
    void updateAttachedModule(uint8_t moduleType);

    // Input state (sent to server each frame)
    bool inputUp_ = false;
    bool inputDown_ = false;
    bool inputLeft_ = false;
    bool inputRight_ = false;
    bool inputFire_ = false;
    bool isCharging_ = false;
    float chargeTime_ = 0.0f;

    // Systems (stored as shared_ptr)
    std::shared_ptr<RenderSystem> renderSystem_;
    std::shared_ptr<AnimationSystem> animationSystem_;
    std::shared_ptr<ScrollingBackgroundSystem> scrollingSystem_;

    // Game config from Lua
    int windowWidth_ = 1920;
    int windowHeight_ = 1080;
    std::string backgroundPath_;
    float backgroundScrollSpeed_ = 100.0f;
    int backgroundOriginalWidth_ = 9306;
    int backgroundOriginalHeight_ = 199;
    bool backgroundScaleToWindow_ = true;

    // Player config from Lua (for sprite dimensions)
    float playerSpeed_ = 600.0f;

    // Hit effect (blink)
    uint8_t lastPlayerHp_ = 100;
    float hitBlinkTimer_ = 0.0f;
    static constexpr float HIT_BLINK_DURATION = 1.5f;  // 1.5s d'invincibilité visuelle

    // Store HP for all players (key = server entity ID)
    std::unordered_map<uint32_t, uint16_t> playerHealthMap_;

    // Shield effect
    bool shieldActive_ = false;

    // Loaded sprites/textures (we own them)
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture>> loadedTextures_;
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLSprite>> loadedSprites_;
    
    // UI elements for score display
    std::unique_ptr<eng::engine::rendering::sfml::SFMLFont> scoreFont_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> scoreText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> levelText_;
    uint32_t currentScore_ = 0;
    
    // Level system
    uint8_t currentLevel_ = 1;
    float levelTransitionTimer_ = 0.0f;
    bool showLevelText_ = false;
    void onLevelChange(uint8_t level);

    // Game end states (deferred from network callbacks)
    bool gameEndTriggered_ = false;
    bool pendingGameOver_ = false;
    bool pendingVictory_ = false;
    uint32_t pendingScore_ = 0;

    // Boss HP bar tracking
    uint32_t bossServerId_ = 0;    // Server entity ID of current boss (0 = no boss)
    uint16_t bossHp_ = 0;           // Current boss HP (from snapshot)
    uint16_t bossMaxHp_ = 1000;    // Max boss HP (from first snapshot)
    uint8_t bossEnemyType_ = 0;    // Boss type (3, 4, 5)
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> bossNameText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> bossHpText_;

    // Spectator mode (local player died but game continues)
    bool isSpectating_ = false;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> spectatorText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> spectatorSubText_;
    float spectatorBlinkTimer_ = 0.0f;

    // Background entity tracking for level changes
    ECS::Entity backgroundEntity_ = 0;
};
