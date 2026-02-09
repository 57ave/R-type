/**
 * NetworkPlayState.cpp - Multiplayer Gameplay State Implementation (Phase 6.5)
 * 
 * Le serveur est autoritaire : positions/états viennent du WORLD_SNAPSHOT
 */

#include "states/NetworkPlayState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Tag.hpp>
#include <components/ScrollingBackground.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <scripting/LuaState.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <unordered_set>
#include <iostream>

NetworkPlayState::NetworkPlayState(Game* game)
{
    game_ = game;
}

NetworkPlayState::~NetworkPlayState() = default;

void NetworkPlayState::onEnter()
{
    std::cout << "[NetworkPlayState] 🌐 Initializing multiplayer gameplay..." << std::endl;

    // Get local player ID from NetworkManager
    auto* networkManager = game_->getNetworkManager();
    if (networkManager) {
        localPlayerId_ = networkManager->getLocalPlayerId();
        std::cout << "[NetworkPlayState] Local player ID: " << localPlayerId_ << std::endl;
    }

    // Load game configuration from Lua
    loadGameConfig();
    loadPlayerConfig();

    // Setup systems
    setupSystems();

    // Spawn background (client-side, not synced)
    spawnBackground();

    // Register world snapshot callback
    if (networkManager) {
        networkManager->setWorldSnapshotCallback([this](const std::vector<RType::EntityState>& entities) {
            this->onWorldSnapshot(entities);
        });
    }

    std::cout << "[NetworkPlayState] ✅ Multiplayer gameplay initialized" << std::endl;
    std::cout << "[NetworkPlayState] Controls: ZQSD/Arrows=Move, Space=Shoot, ESC=Disconnect" << std::endl;
}

void NetworkPlayState::onExit()
{
    std::cout << "[NetworkPlayState] Exiting multiplayer gameplay..." << std::endl;

    // Clear snapshot callback
    auto* networkManager = game_->getNetworkManager();
    if (networkManager) {
        networkManager->setWorldSnapshotCallback(nullptr);
    }

    // Destroy all network entities
    auto coordinator = game_->getCoordinator();
    if (coordinator) {
        for (auto& [serverId, localEntity] : networkEntities_) {
            coordinator->DestroyEntity(localEntity);
        }
    }
    networkEntities_.clear();

    std::cout << "[NetworkPlayState] Multiplayer cleanup complete" << std::endl;
}

void NetworkPlayState::setupSystems()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) {
        std::cerr << "[NetworkPlayState] ERROR: No coordinator!" << std::endl;
        return;
    }

    // Only register rendering systems - physics is server-side
    renderSystem_ = coordinator->RegisterSystem<RenderSystem>();
    animationSystem_ = coordinator->RegisterSystem<AnimationSystem>();
    scrollingSystem_ = coordinator->RegisterSystem<ScrollingBackgroundSystem>(coordinator);

    // Set coordinators
    if (renderSystem_) {
        renderSystem_->SetCoordinator(coordinator);
        renderSystem_->SetRenderer(game_->getRenderer());
    }
    if (animationSystem_) {
        animationSystem_->SetCoordinator(coordinator);
    }

    // Define system signatures
    // RenderSystem: Position + Sprite
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Sprite>());
        coordinator->SetSystemSignature<RenderSystem>(sig);
    }
    
    // ScrollingBackgroundSystem: Position + ScrollingBackground
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<ScrollingBackground>());
        coordinator->SetSystemSignature<ScrollingBackgroundSystem>(sig);
    }

    // Initialize systems
    if (renderSystem_) renderSystem_->Init();
    if (animationSystem_) animationSystem_->Init();
    if (scrollingSystem_) scrollingSystem_->Init();

    std::cout << "[NetworkPlayState] Systems registered" << std::endl;
}

void NetworkPlayState::loadGameConfig()
{
    auto& lua = game_->getLuaState();

    try {
        lua.GetState().script_file("assets/scripts/config/game_config.lua");
        
        sol::table gameConfig = lua.GetState()["Game"];
        if (gameConfig.valid()) {
            sol::table window = gameConfig["window"];
            windowWidth_ = window["width"].get_or(1920);
            windowHeight_ = window["height"].get_or(1080);
            
            sol::table background = gameConfig["background"];
            backgroundPath_ = background["path"].get_or<std::string>("assets/background.png");
            backgroundScrollSpeed_ = background["scroll_speed"].get_or(100.0f);
            backgroundOriginalWidth_ = background["original_width"].get_or(9306);
            backgroundOriginalHeight_ = background["original_height"].get_or(199);
            backgroundScaleToWindow_ = background["scale_to_window"].get_or(true);
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkPlayState] Error loading game config: " << e.what() << std::endl;
    }
}

void NetworkPlayState::loadPlayerConfig()
{
    auto& lua = game_->getLuaState();

    try {
        lua.GetState().script_file("assets/scripts/config/player_config.lua");
        
        sol::table playerConfig = lua.GetState()["Player"];
        if (playerConfig.valid()) {
            playerSpeed_ = playerConfig["speed"].get_or(600.0f);
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkPlayState] Error loading player config: " << e.what() << std::endl;
    }
}

eng::engine::rendering::ISprite* NetworkPlayState::loadSprite(const std::string& texturePath, const eng::engine::rendering::IntRect* rect)
{
    auto texture = std::make_unique<eng::engine::rendering::sfml::SFMLTexture>();
    if (!texture->loadFromFile(texturePath)) {
        std::cerr << "[NetworkPlayState] ERROR: Failed to load texture: " << texturePath << std::endl;
        return nullptr;
    }

    auto sprite = std::make_unique<eng::engine::rendering::sfml::SFMLSprite>();
    sprite->setTexture(texture.get());
    
    if (rect && (rect->width != 0 || rect->height != 0)) {
        sprite->setTextureRect(*rect);
    }

    eng::engine::rendering::ISprite* spritePtr = sprite.get();
    loadedTextures_.push_back(std::move(texture));
    loadedSprites_.push_back(std::move(sprite));

    return spritePtr;
}

void NetworkPlayState::spawnBackground()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    ECS::Entity bg = coordinator->CreateEntity();
    
    coordinator->AddComponent<Position>(bg, Position{0.0f, 0.0f});
    
    float scaleY = backgroundScaleToWindow_ ? (float)windowHeight_ / (float)backgroundOriginalHeight_ : 1.0f;
    float scaleX = scaleY;
    float scaledWidth = (float)backgroundOriginalWidth_ * scaleX;
    
    Sprite bgSprite;
    bgSprite.texturePath = backgroundPath_;
    bgSprite.layer = -10;
    bgSprite.scaleX = scaleX;
    bgSprite.scaleY = scaleY;
    bgSprite.sprite = loadSprite(backgroundPath_);
    coordinator->AddComponent<Sprite>(bg, bgSprite);

    ScrollingBackground scrollBg;
    scrollBg.scrollSpeed = backgroundScrollSpeed_;
    scrollBg.horizontal = true;
    scrollBg.loop = true;
    scrollBg.spriteWidth = scaledWidth;
    coordinator->AddComponent<ScrollingBackground>(bg, scrollBg);

    std::cout << "[NetworkPlayState] Background spawned" << std::endl;
}

NetworkPlayState::SpriteInfo NetworkPlayState::getSpriteInfo(const RType::EntityState& state)
{
    SpriteInfo info;
    info.scaleX = 3.0f;
    info.scaleY = 3.0f;
    info.layer = 1;

    switch (state.type) {
        case RType::EntityType::ENTITY_PLAYER: {
            info.texturePath = "assets/players/r-typesheet42.png";
            // Each row is a different ship color, each row has 5 frames (33x17)
            // playerLine determines which row (color)
            int row = state.playerLine % 5;  // 5 different colors
            // Frame 2 (neutral) by default
            info.textureRect = {66, row * 17, 33, 17};
            info.layer = 1;
            break;
        }
        case RType::EntityType::ENTITY_PLAYER_MISSILE: {
            info.texturePath = "assets/players/r-typesheet1.png";
            info.textureRect = {245, 85, 20, 20};  // Normal missile
            info.layer = 2;
            break;
        }
        case RType::EntityType::ENTITY_MONSTER: {
            info.texturePath = "assets/enemies/r-typesheet5.png";
            // Different enemy types from enemyType field
            info.textureRect = {0, 0, 33, 36};  // Basic enemy
            info.layer = 1;
            break;
        }
        case RType::EntityType::ENTITY_MONSTER_MISSILE: {
            info.texturePath = "assets/players/r-typesheet1.png";
            info.textureRect = {245, 120, 20, 20};  // Enemy missile (different from player)
            info.layer = 2;
            break;
        }
        case RType::EntityType::ENTITY_EXPLOSION: {
            info.texturePath = "assets/vfx/r-typesheet44.png";
            info.textureRect = {0, 0, 32, 32};  // Explosion frame
            info.layer = 3;
            break;
        }
        default:
            info.texturePath = "assets/players/r-typesheet1.png";
            info.textureRect = {0, 0, 32, 32};
            break;
    }

    return info;
}

void NetworkPlayState::onWorldSnapshot(const std::vector<RType::EntityState>& entities)
{
    // Sync all entities from snapshot
    for (const auto& state : entities) {
        syncEntityFromState(state);
    }
    
    // Remove entities that are no longer in the snapshot
    removeStaleEntities(entities);
}

void NetworkPlayState::syncEntityFromState(const RType::EntityState& state)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    ECS::Entity localEntity;
    bool isNewEntity = false;

    // Check if we already have this entity
    auto it = networkEntities_.find(state.id);
    if (it != networkEntities_.end()) {
        localEntity = it->second;
    } else {
        // Create new entity
        localEntity = coordinator->CreateEntity();
        networkEntities_[state.id] = localEntity;
        isNewEntity = true;

        // Track local player entity
        if (state.type == RType::EntityType::ENTITY_PLAYER && state.playerId == localPlayerId_) {
            localPlayerEntity_ = localEntity;
            std::cout << "[NetworkPlayState] Local player entity created: " << localEntity << std::endl;
        }
    }

    // Update or add Position component
    if (coordinator->HasComponent<Position>(localEntity)) {
        auto& pos = coordinator->GetComponent<Position>(localEntity);
        pos.x = static_cast<float>(state.x);
        pos.y = static_cast<float>(state.y);
    } else {
        coordinator->AddComponent<Position>(localEntity, Position{
            static_cast<float>(state.x),
            static_cast<float>(state.y)
        });
    }

    // Update or add Velocity component (for interpolation/animation)
    if (coordinator->HasComponent<Velocity>(localEntity)) {
        auto& vel = coordinator->GetComponent<Velocity>(localEntity);
        vel.dx = static_cast<float>(state.vx);
        vel.dy = static_cast<float>(state.vy);
    } else {
        coordinator->AddComponent<Velocity>(localEntity, Velocity{
            static_cast<float>(state.vx),
            static_cast<float>(state.vy)
        });
    }

    // Create sprite for new entities
    if (isNewEntity) {
        SpriteInfo spriteInfo = getSpriteInfo(state);
        
        Sprite sprite;
        sprite.texturePath = spriteInfo.texturePath;
        sprite.textureRect = spriteInfo.textureRect;
        sprite.layer = spriteInfo.layer;
        sprite.scaleX = spriteInfo.scaleX;
        sprite.scaleY = spriteInfo.scaleY;
        sprite.sprite = loadSprite(spriteInfo.texturePath, &spriteInfo.textureRect);
        coordinator->AddComponent<Sprite>(localEntity, sprite);

        // Tag
        std::string tag;
        switch (state.type) {
            case RType::EntityType::ENTITY_PLAYER:
                tag = (state.playerId == localPlayerId_) ? "local_player" : "remote_player";
                break;
            case RType::EntityType::ENTITY_PLAYER_MISSILE:
                tag = "player_projectile";
                break;
            case RType::EntityType::ENTITY_MONSTER:
                tag = "enemy";
                break;
            case RType::EntityType::ENTITY_MONSTER_MISSILE:
                tag = "enemy_projectile";
                break;
            default:
                tag = "unknown";
                break;
        }
        coordinator->AddComponent<Tag>(localEntity, Tag{tag});
    }

    // Update player animation based on vertical velocity
    if (state.type == RType::EntityType::ENTITY_PLAYER) {
        if (coordinator->HasComponent<Sprite>(localEntity)) {
            auto& sprite = coordinator->GetComponent<Sprite>(localEntity);
            int row = state.playerLine % 5;
            
            // Determine animation frame based on velocity
            int frame = 2;  // Neutral
            if (state.vy < -50) frame = 4;       // Extreme up
            else if (state.vy < -10) frame = 3;  // Up
            else if (state.vy > 50) frame = 0;   // Extreme down
            else if (state.vy > 10) frame = 1;   // Down
            
            sprite.textureRect = {frame * 33, row * 17, 33, 17};
            if (sprite.sprite) {
                sprite.sprite->setTextureRect(sprite.textureRect);
            }
        }
    }
}

void NetworkPlayState::removeStaleEntities(const std::vector<RType::EntityState>& entities)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    // Build set of current server entity IDs
    std::unordered_set<uint32_t> currentIds;
    for (const auto& state : entities) {
        currentIds.insert(state.id);
    }

    // Find and remove entities not in snapshot
    std::vector<uint32_t> toRemove;
    for (auto& [serverId, localEntity] : networkEntities_) {
        if (currentIds.find(serverId) == currentIds.end()) {
            toRemove.push_back(serverId);
        }
    }

    for (uint32_t id : toRemove) {
        ECS::Entity entity = networkEntities_[id];
        coordinator->DestroyEntity(entity);
        networkEntities_.erase(id);
        
        if (entity == localPlayerEntity_) {
            localPlayerEntity_ = 0;
            std::cout << "[NetworkPlayState] Local player destroyed!" << std::endl;
        }
    }
}

void NetworkPlayState::handleEvent(const eng::engine::InputEvent& event)
{
    if (event.type == eng::engine::EventType::KeyPressed) {
        // ESC = Return to menu
        if (event.key.code == eng::engine::Key::Escape) {
            std::cout << "[NetworkPlayState] ESC pressed - disconnecting" << std::endl;
            auto* networkManager = game_->getNetworkManager();
            if (networkManager) {
                networkManager->leaveRoom();
            }
            game_->getStateManager()->popState();
            return;
        }

        // Movement keys
        if (event.key.code == eng::engine::Key::Z || event.key.code == eng::engine::Key::Up) {
            inputUp_ = true;
        }
        if (event.key.code == eng::engine::Key::S || event.key.code == eng::engine::Key::Down) {
            inputDown_ = true;
        }
        if (event.key.code == eng::engine::Key::Q || event.key.code == eng::engine::Key::Left) {
            inputLeft_ = true;
        }
        if (event.key.code == eng::engine::Key::D || event.key.code == eng::engine::Key::Right) {
            inputRight_ = true;
        }

        // Shoot key
        if (event.key.code == eng::engine::Key::Space) {
            inputFire_ = true;
            isCharging_ = true;
            chargeTime_ = 0.0f;
        }
    }
    else if (event.type == eng::engine::EventType::KeyReleased) {
        // Movement keys
        if (event.key.code == eng::engine::Key::Z || event.key.code == eng::engine::Key::Up) {
            inputUp_ = false;
        }
        if (event.key.code == eng::engine::Key::S || event.key.code == eng::engine::Key::Down) {
            inputDown_ = false;
        }
        if (event.key.code == eng::engine::Key::Q || event.key.code == eng::engine::Key::Left) {
            inputLeft_ = false;
        }
        if (event.key.code == eng::engine::Key::D || event.key.code == eng::engine::Key::Right) {
            inputRight_ = false;
        }

        // Shoot key released
        if (event.key.code == eng::engine::Key::Space) {
            inputFire_ = false;
            isCharging_ = false;
        }
    }
}

void NetworkPlayState::update(float deltaTime)
{
    // Update charge time
    if (isCharging_) {
        chargeTime_ += deltaTime;
    }

    // Calculate charge level (0-5)
    uint8_t chargeLevel = 0;
    if (chargeTime_ > 0.2f) chargeLevel = 1;
    if (chargeTime_ > 0.5f) chargeLevel = 2;
    if (chargeTime_ > 0.8f) chargeLevel = 3;
    if (chargeTime_ > 1.2f) chargeLevel = 4;
    if (chargeTime_ > 1.6f) chargeLevel = 5;

    // Send input to server every frame
    auto* networkManager = game_->getNetworkManager();
    if (networkManager) {
        networkManager->sendInput(inputUp_, inputDown_, inputLeft_, inputRight_, inputFire_, chargeLevel);
        
        // Update network (receive snapshots)
        networkManager->update();
    }

    // Update local systems
    if (scrollingSystem_) scrollingSystem_->Update(deltaTime);
    if (animationSystem_) animationSystem_->Update(deltaTime);
}

void NetworkPlayState::render()
{
    if (renderSystem_) {
        renderSystem_->Update(0.0f);
    }
}
