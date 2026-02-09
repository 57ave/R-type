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
    };
    SpriteInfo getSpriteInfo(const RType::EntityState& state);

    // Entity tracking (server entity ID -> local ECS entity)
    std::unordered_map<uint32_t, ECS::Entity> networkEntities_;
    
    // Local player info
    uint32_t localPlayerId_ = 0;
    ECS::Entity localPlayerEntity_ = 0;

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

    // Loaded sprites/textures (we own them)
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture>> loadedTextures_;
    std::vector<std::unique_ptr<eng::engine::rendering::sfml::SFMLSprite>> loadedSprites_;
};
