/**
 * NetworkPlayState.cpp - Multiplayer Gameplay State Implementation (Phase 6.5)
 * 
 * Le serveur est autoritaire : positions/états viennent du WORLD_SNAPSHOT
 */

#include "states/NetworkPlayState.hpp"
#include "states/GameOverState.hpp"
#include "states/VictoryState.hpp"
#include "states/MainMenuState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Tag.hpp>
#include <components/Animation.hpp>
#include <components/ScrollingBackground.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <scripting/LuaState.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <rendering/IRenderer.hpp>
#include <unordered_set>
#include <iostream>
#include <cmath>

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

    // Spawn charge indicator (client-side visual)
    spawnChargeIndicator();

    // Initialize score display
    scoreFont_ = std::make_unique<eng::engine::rendering::sfml::SFMLFont>();
    if (scoreFont_->loadFromFile("assets/fonts/arial.ttf")) {
        scoreText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
        scoreText_->setFont(scoreFont_.get());
        scoreText_->setCharacterSize(36);
        scoreText_->setFillColor(0xFFFFFFFF); // White
        scoreText_->setPosition(windowWidth_ - 250.0f, 20.0f); // Top right
        scoreText_->setString("Score: 0");
        
        // Level text (center of screen, large, shown during transitions)
        levelText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
        levelText_->setFont(scoreFont_.get());
        levelText_->setCharacterSize(72);
        levelText_->setFillColor(0xFFFF00FF); // Yellow
        levelText_->setString("Level 1");
        // Center it
        levelText_->setPosition(windowWidth_ / 2.0f - 120.0f, windowHeight_ / 2.0f - 50.0f);
        showLevelText_ = true;
        levelTransitionTimer_ = 3.0f;
        
        std::cout << "[NetworkPlayState] Score & Level display initialized" << std::endl;
    } else {
        std::cerr << "[NetworkPlayState] Failed to load font for score display" << std::endl;
    }

    // Register world snapshot callback
    if (networkManager) {
        networkManager->setWorldSnapshotCallback([this](const RType::WorldSnapshotData& snapshot) {
            this->onWorldSnapshot(snapshot);
        });
        networkManager->setLevelChangeCallback([this](uint8_t level) {
            this->onLevelChange(level);
        });
        networkManager->setGameOverCallback([this](uint32_t totalScore) {
            if (!gameEndTriggered_) {
                gameEndTriggered_ = true;
                pendingGameOver_ = true;
                pendingScore_ = totalScore;
            }
        });
        networkManager->setVictoryCallback([this](uint32_t totalScore) {
            if (!gameEndTriggered_) {
                gameEndTriggered_ = true;
                pendingVictory_ = true;
                pendingScore_ = totalScore;
            }
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
        networkManager->setLevelChangeCallback(nullptr);
        networkManager->setGameOverCallback(nullptr);
        networkManager->setVictoryCallback(nullptr);
    }

    // Destroy charge indicator entity
    auto coordinator = game_->getCoordinator();
    if (chargeIndicatorEntity_ != 0 && coordinator) {
        coordinator->DestroyEntity(chargeIndicatorEntity_);
        chargeIndicatorEntity_ = 0;
    }

    // Destroy attached module entity
    if (attachedModuleEntity_ != 0 && coordinator) {
        coordinator->DestroyEntity(attachedModuleEntity_);
        attachedModuleEntity_ = 0;
        currentModuleType_ = 0;
    }

    // Destroy all network entities
    if (coordinator) {
        for (auto& [serverId, localEntity] : networkEntities_) {
            coordinator->DestroyEntity(localEntity);
        }
    }
    networkEntities_.clear();

    // Clean up spectator overlay
    spectatorText_.reset();
    spectatorSubText_.reset();
    isSpectating_ = false;

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

    // AnimationSystem: Sprite + Animation
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Sprite>());
        sig.set(coordinator->GetComponentType<Animation>());
        coordinator->SetSystemSignature<AnimationSystem>(sig);
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

    // Destroy previous background entity if exists
    if (backgroundEntity_ != 0) {
        coordinator->DestroyEntity(backgroundEntity_);
        backgroundEntity_ = 0;
    }

    ECS::Entity bg = coordinator->CreateEntity();
    backgroundEntity_ = bg;
    
    coordinator->AddComponent<Position>(bg, Position{0.0f, 0.0f});

    if (currentLevel_ == 1) {
        // Level 1: scrolling parallax background (from Lua config)
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
    } else {
        // Level 2/3: static background image
        std::string bgPath = (currentLevel_ == 2) 
            ? "assets/backgrounds/bg_level2.png" 
            : "assets/backgrounds/bg_level3.png";
        
        float scaleX = (float)windowWidth_ / 1920.0f;
        float scaleY = (float)windowHeight_ / 1080.0f;
        
        Sprite bgSprite;
        bgSprite.texturePath = bgPath;
        bgSprite.layer = -10;
        bgSprite.scaleX = scaleX;
        bgSprite.scaleY = scaleY;
        bgSprite.sprite = loadSprite(bgPath);
        coordinator->AddComponent<Sprite>(bg, bgSprite);
    }

    std::cout << "[NetworkPlayState] Background spawned for level " << (int)currentLevel_ << std::endl;
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
            // chargeLevel determines missile size/sprite
            switch (state.chargeLevel) {
                case 0: // Basic shot
                    info.texturePath = "assets/players/r-typesheet1.png";
                    info.textureRect = {245, 85, 20, 20};
                    break;
                case 1:
                    info.texturePath = "assets/players/charge_level1.png";
                    info.textureRect = {0, 0, 35, 17};
                    info.frameCount = 2; info.frameWidth = 18; info.frameHeight = 17; info.frameTime = 0.1f;
                    break;
                case 2:
                    info.texturePath = "assets/players/charge_level2.png";
                    info.textureRect = {0, 0, 68, 14};
                    info.frameCount = 2; info.frameWidth = 34; info.frameHeight = 14; info.frameTime = 0.1f;
                    break;
                case 3:
                    info.texturePath = "assets/players/charge_level3.png";
                    info.textureRect = {0, 0, 99, 15};
                    info.frameCount = 2; info.frameWidth = 50; info.frameHeight = 15; info.frameTime = 0.08f;
                    break;
                case 4:
                    info.texturePath = "assets/players/charge_level4.png";
                    info.textureRect = {0, 0, 99, 16};
                    info.frameCount = 2; info.frameWidth = 50; info.frameHeight = 16; info.frameTime = 0.08f;
                    break;
                default: // 5+
                    info.texturePath = "assets/players/charge_level5.png";
                    info.textureRect = {0, 0, 163, 18};
                    info.frameCount = 2; info.frameWidth = 82; info.frameHeight = 18; info.frameTime = 0.06f;
                    break;
            }
            info.scaleX = 3.0f;
            info.scaleY = 3.0f;
            info.layer = 2;
            break;
        }
        case RType::EntityType::ENTITY_MONSTER: {
            // Different sprite per enemyType matching Lua config
            switch (state.enemyType) {
                case 0: // Bug (basic_enemie.png: 255x29, 8 frames)
                    info.texturePath = "assets/enemies/basic_enemie.png";
                    info.textureRect = {0, 0, 33, 29};
                    info.frameCount = 8;
                    info.frameWidth = 33;
                    info.frameHeight = 29;
                    info.frameTime = 0.1f;
                    info.scaleX = 2.0f;
                    info.scaleY = 2.0f;
                    break;
                case 1: // Fighter / Bat (BatEnemies.png: 255x29, 5 frames ~16px)
                    info.texturePath = "assets/enemies/BatEnemies.png";
                    info.textureRect = {0, 0, 16, 13};
                    info.frameCount = 5;
                    info.frameWidth = 16;
                    info.frameHeight = 13;
                    info.frameTime = 0.12f;
                    info.scaleX = 2.0f;
                    info.scaleY = 2.0f;
                    break;
                case 2: // Kamikaze (kamikaze_enemies.png: 205x18, 12 frames)
                    info.texturePath = "assets/enemies/kamikaze_enemies.png";
                    info.textureRect = {0, 0, 17, 18};
                    info.frameCount = 12;
                    info.frameWidth = 17;
                    info.frameHeight = 18;
                    info.frameTime = 0.08f;
                    info.scaleX = 2.0f;
                    info.scaleY = 2.0f;
                    break;
                case 3: // FirstBoss (259x573, 4 vertical frames of 259x143)
                    info.texturePath = "assets/enemies/FirstBoss.png";
                    info.textureRect = {0, 0, 259, 143};
                    info.frameCount = 4;
                    info.frameWidth = 259;
                    info.frameHeight = 143;
                    info.frameTime = 0.15f;
                    info.scaleX = 1.5f;
                    info.scaleY = 1.5f;
                    info.vertical = true; // Vertical spritesheet
                    break;
                case 4: // SecondBoss (645x211, 4 horizontal frames of 161x211)
                    info.texturePath = "assets/enemies/SecondBoss.png";
                    info.textureRect = {0, 0, 161, 211};
                    info.frameCount = 4;
                    info.frameWidth = 161;
                    info.frameHeight = 211;
                    info.frameTime = 0.12f;
                    info.scaleX = 1.5f;
                    info.scaleY = 1.5f;
                    break;
                case 5: // LastBoss (324x71, 4 horizontal frames of 81x71)
                    info.texturePath = "assets/enemies/LastBossFly.png";
                    info.textureRect = {0, 0, 81, 71};
                    info.frameCount = 4;
                    info.frameWidth = 81;
                    info.frameHeight = 71;
                    info.frameTime = 0.1f;
                    info.scaleX = 2.5f;
                    info.scaleY = 2.5f;
                    break;
                default:
                    info.texturePath = "assets/enemies/basic_enemie.png";
                    info.textureRect = {0, 0, 33, 29};
                    info.frameCount = 8;
                    info.frameWidth = 33;
                    info.frameHeight = 29;
                    info.frameTime = 0.1f;
                    info.scaleX = 2.0f;
                    info.scaleY = 2.0f;
                    break;
            }
            info.layer = 1;
            break;
        }
        case RType::EntityType::ENTITY_MONSTER_MISSILE: {
            info.texturePath = "assets/enemies/enemy_bullets.png";
            info.textureRect = {0, 0, 13, 8};
            info.scaleX = 2.0f;
            info.scaleY = 2.0f;
            info.layer = 2;
            break;
        }
        case RType::EntityType::ENTITY_EXPLOSION: {
            info.texturePath = "assets/enemies/dead_enemies_animation.png";
            info.textureRect = {0, 0, 33, 33};
            info.scaleX = 2.0f;
            info.scaleY = 2.0f;
            info.frameCount = 6;
            info.frameWidth = 33;
            info.frameHeight = 33;
            info.frameTime = 0.05f;
            info.loop = false;
            info.layer = 5;
            break;
        }
        case RType::EntityType::ENTITY_POWERUP: {
            // enemyType: 0=orange (bomb), 1=blue (shield)
            if (state.enemyType == 0) {
                info.texturePath = "assets/players/powerup_orange.png";
            } else {
                info.texturePath = "assets/players/powerup_blue.png";
            }
            info.textureRect = {0, 0, 612, 408};
            info.scaleX = 0.2f;
            info.scaleY = 0.2f;
            info.layer = 5;
            break;
        }
        case RType::EntityType::ENTITY_MODULE: {
            // enemyType: 1=laser(homing), 3=spread, 4=wave
            switch (state.enemyType) {
                case 1: // Laser (fires homing missiles)
                    info.texturePath = "assets/players/laser_module.png";
                    info.textureRect = {0, 0, 34, 29};
                    info.frameCount = 4;
                    info.frameWidth = 34;
                    info.frameHeight = 29;
                    info.frameTime = 0.1f;
                    break;
                case 3: // Spread
                    info.texturePath = "assets/players/spread_module.png";
                    info.textureRect = {0, 0, 30, 24};
                    info.frameCount = 6;
                    info.frameWidth = 30;
                    info.frameHeight = 24;
                    info.frameTime = 0.1f;
                    break;
                case 4: // Wave
                    info.texturePath = "assets/players/wave_module.png";
                    info.textureRect = {0, 0, 24, 19};
                    info.frameCount = 6;
                    info.frameWidth = 24;
                    info.frameHeight = 19;
                    info.frameTime = 0.1f;
                    break;
                default:
                    info.texturePath = "assets/players/laser_module.png";
                    info.textureRect = {0, 0, 34, 29};
                    break;
            }
            info.scaleX = 2.0f;
            info.scaleY = 2.0f;
            info.layer = 5;
            break;
        }
        default:
            info.texturePath = "assets/players/r-typesheet1.png";
            info.textureRect = {0, 0, 32, 32};
            break;
    }

    return info;
}

void NetworkPlayState::onWorldSnapshot(const RType::WorldSnapshotData& snapshot)
{
    // Find the ack for our local player
    uint32_t ackedInputSeq = 0;
    for (const auto& ack : snapshot.acks) {
        if (ack.playerId == static_cast<uint8_t>(localPlayerId_)) {
            ackedInputSeq = ack.lastProcessedInputSeq;
            break;
        }
    }

    // Process each entity
    for (const auto& state : snapshot.entities) {
        bool isLocalPlayer = (state.type == RType::EntityType::ENTITY_PLAYER &&
                              state.playerId == localPlayerId_);

        if (isLocalPlayer) {
            // Reconcile local player prediction with server state
            reconcileLocalPlayer(state, ackedInputSeq);
        } else {
            // Update interpolation buffer for remote entities
            updateInterpolationBuffer(state);
        }

        // Always sync ECS entity (sprite, tag, health, score, etc.)
        syncEntityFromState(state);
    }

    // Override local player position with predicted position (after syncEntityFromState set server pos)
    if (predictionInitialized_ && localPlayerEntity_ != 0) {
        auto coordinator = game_->getCoordinator();
        if (coordinator && coordinator->HasComponent<Position>(localPlayerEntity_)) {
            auto& pos = coordinator->GetComponent<Position>(localPlayerEntity_);
            pos.x = predictedX_;
            pos.y = predictedY_;
        }
    }

    // Remove entities that are no longer in the snapshot
    removeStaleEntities(snapshot.entities);

    // Clean up interpolation buffers for destroyed entities
    for (auto it = interpolationBuffers_.begin(); it != interpolationBuffers_.end();) {
        bool found = false;
        for (const auto& state : snapshot.entities) {
            if (state.id == it->first) { found = true; break; }
        }
        if (!found) {
            it = interpolationBuffers_.erase(it);
        } else {
            ++it;
        }
    }
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
            case RType::EntityType::ENTITY_POWERUP:
                tag = "powerup";
                break;
            default:
                tag = "unknown";
                break;
        }
        coordinator->AddComponent<Tag>(localEntity, Tag{tag});

        // Animation (if spriteInfo has animation data)
        if (spriteInfo.frameCount > 1) {
            Animation anim;
            anim.frameCount = spriteInfo.frameCount;
            anim.frameWidth = spriteInfo.frameWidth;
            anim.frameHeight = spriteInfo.frameHeight;
            anim.frameTime = spriteInfo.frameTime;
            anim.spacing = spriteInfo.spacing;
            anim.startX = spriteInfo.textureRect.left;
            anim.startY = spriteInfo.textureRect.top;
            anim.currentFrame = 0;
            anim.currentTime = 0.0f;
            anim.loop = spriteInfo.loop;
            anim.vertical = spriteInfo.vertical;
            coordinator->AddComponent<Animation>(localEntity, anim);
        }
    }

    // Update player animation based on vertical velocity
    if (state.type == RType::EntityType::ENTITY_PLAYER) {
        // Store HP for all players
        playerHealthMap_[state.id] = state.hp;
        
        // Update score for local player
        if (state.playerId == localPlayerId_) {
            currentScore_ = state.score;
        }
        
        // Detect hit on local player (HP decreased)
        if (state.playerId == localPlayerId_ && state.hp < lastPlayerHp_ && lastPlayerHp_ != 100) {
            hitBlinkTimer_ = HIT_BLINK_DURATION;
            std::cout << "[NetworkPlayState] 💥 Player hit! HP: " << (int)lastPlayerHp_ << " → " << (int)state.hp << std::endl;
        }
        if (state.playerId == localPlayerId_) {
            lastPlayerHp_ = state.hp;
            // Detect shield state from server (chargeLevel == 99 means shield active)
            shieldActive_ = (state.chargeLevel == 99);
            // Detect equipped module from server (projectileType = moduleType for players)
            updateAttachedModule(state.projectileType);
        }
        
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

    // Track boss HP for boss health bar
    if (state.type == RType::EntityType::ENTITY_MONSTER && state.enemyType >= 3) {
        if (bossServerId_ == 0) {
            // First time seeing this boss - set max HP
            bossServerId_ = state.id;
            bossMaxHp_ = state.hp;
            bossEnemyType_ = state.enemyType;
            std::cout << "[NetworkPlayState] 👹 Boss detected (type " << (int)state.enemyType
                      << ", HP: " << (int)state.hp << ")" << std::endl;
        }
        if (state.id == bossServerId_) {
            bossHp_ = state.hp;
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
            if (!gameEndTriggered_) {
                isSpectating_ = true;
                // Create spectator overlay text
                if (scoreFont_) {
                    spectatorText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
                    spectatorText_->setFont(scoreFont_.get());
                    spectatorText_->setCharacterSize(72);
                    spectatorText_->setFillColor(0xFF4444FF); // Red
                    spectatorText_->setString("VOUS ETES MORT");
                    spectatorText_->setPosition(windowWidth_ / 2.0f - 350.0f, windowHeight_ / 2.0f - 120.0f);

                    spectatorSubText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
                    spectatorSubText_->setFont(scoreFont_.get());
                    spectatorSubText_->setCharacterSize(36);
                    spectatorSubText_->setFillColor(0xCCCCCCFF); // Light gray
                    spectatorSubText_->setString("Mode spectateur...");
                    spectatorSubText_->setPosition(windowWidth_ / 2.0f - 180.0f, windowHeight_ / 2.0f - 20.0f);
                }
            }
            std::cout << "[NetworkPlayState] Local player destroyed! Entering spectator mode." << std::endl;
        }
        if (id == bossServerId_) {
            bossServerId_ = 0;
            bossHp_ = 0;
            std::cout << "[NetworkPlayState] 👹 Boss destroyed!" << std::endl;
        }
    }
}

void NetworkPlayState::spawnChargeIndicator()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    // Charge animation: animation_charge.png, 8 frames 32x32, horizontal strip
    chargeIndicatorEntity_ = coordinator->CreateEntity();

    // Position (will follow player)
    coordinator->AddComponent<Position>(chargeIndicatorEntity_, Position{-100.0f, -100.0f});

    // Sprite - hidden by default (layer -100)
    Sprite chargeSprite;
    chargeSprite.texturePath = "assets/players/animation_charge.png";
    chargeSprite.textureRect = {0, 0, 32, 32};
    chargeSprite.layer = -100;  // Hidden initially
    chargeSprite.scaleX = 2.0f;
    chargeSprite.scaleY = 2.0f;
    chargeSprite.sprite = loadSprite("assets/players/animation_charge.png", &chargeSprite.textureRect);
    coordinator->AddComponent<Sprite>(chargeIndicatorEntity_, chargeSprite);

    // Animation: 8 frames, 32x32, looping
    Animation chargeAnim;
    chargeAnim.frameCount = 8;
    chargeAnim.frameWidth = 32;
    chargeAnim.frameHeight = 32;
    chargeAnim.frameTime = 0.1f;
    chargeAnim.currentTime = 0.0f;
    chargeAnim.currentFrame = 0;
    chargeAnim.loop = true;
    chargeAnim.finished = false;
    chargeAnim.startX = 0;
    chargeAnim.startY = 0;
    chargeAnim.spacing = 0;
    coordinator->AddComponent<Animation>(chargeIndicatorEntity_, chargeAnim);

    // Tag
    coordinator->AddComponent<Tag>(chargeIndicatorEntity_, Tag{"charge_indicator"});

    std::cout << "[NetworkPlayState] Charge indicator spawned (Entity ID: " << chargeIndicatorEntity_ << ")" << std::endl;
}

void NetworkPlayState::updateChargeIndicator(float deltaTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || chargeIndicatorEntity_ == 0 || localPlayerEntity_ == 0) return;

    const float CHARGE_ANIMATION_DELAY = 0.15f;

    // Update position to follow local player + offset (x=80, y=0 from Lua config)
    if (coordinator->HasComponent<Position>(localPlayerEntity_) &&
        coordinator->HasComponent<Position>(chargeIndicatorEntity_)) {
        auto& playerPos = coordinator->GetComponent<Position>(localPlayerEntity_);
        auto& chargePos = coordinator->GetComponent<Position>(chargeIndicatorEntity_);
        chargePos.x = playerPos.x + 80.0f;
        chargePos.y = playerPos.y;
    }

    // Show/hide based on charging state
    if (coordinator->HasComponent<Sprite>(chargeIndicatorEntity_)) {
        auto& sprite = coordinator->GetComponent<Sprite>(chargeIndicatorEntity_);

        if (isCharging_ && chargeTime_ >= CHARGE_ANIMATION_DELAY) {
            sprite.layer = 10;  // Visible (layer from Lua config)
        } else {
            sprite.layer = -100;  // Hidden
        }
    }

    // Reset animation when not charging
    if (!isCharging_ || chargeTime_ < CHARGE_ANIMATION_DELAY) {
        if (coordinator->HasComponent<Animation>(chargeIndicatorEntity_)) {
            auto& anim = coordinator->GetComponent<Animation>(chargeIndicatorEntity_);
            anim.currentFrame = 0;
            anim.currentTime = 0.0f;
            anim.finished = false;
        }
    }
}

void NetworkPlayState::updateAttachedModule(uint8_t moduleType)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    // If module changed or removed, destroy old entity
    if (moduleType != currentModuleType_) {
        if (attachedModuleEntity_ != 0) {
            coordinator->DestroyEntity(attachedModuleEntity_);
            attachedModuleEntity_ = 0;
        }
        currentModuleType_ = moduleType;

        // Create new attached module entity if equipped
        if (moduleType > 0) {
            attachedModuleEntity_ = coordinator->CreateEntity();

            coordinator->AddComponent<Position>(attachedModuleEntity_, Position{-100.0f, -100.0f});

            // Determine sprite based on module type
            std::string texturePath;
            eng::engine::rendering::IntRect rect;
            int frameCount = 1, frameWidth = 16, frameHeight = 16;
            float frameTime = 0.1f;

            switch (moduleType) {
                case 1: // Laser (fires homing missiles)
                    texturePath = "assets/players/laser_module.png";
                    rect = {0, 0, 34, 29};
                    frameCount = 4; frameWidth = 34; frameHeight = 29;
                    break;
                case 3: // Spread
                    texturePath = "assets/players/spread_module.png";
                    rect = {0, 0, 30, 24};
                    frameCount = 6; frameWidth = 30; frameHeight = 24;
                    break;
                case 4: // Wave
                    texturePath = "assets/players/wave_module.png";
                    rect = {0, 0, 24, 19};
                    frameCount = 6; frameWidth = 24; frameHeight = 19;
                    break;
            }

            Sprite modSprite;
            modSprite.texturePath = texturePath;
            modSprite.textureRect = rect;
            modSprite.scaleX = 2.0f;
            modSprite.scaleY = 2.0f;
            modSprite.layer = 3; // In front of player
            modSprite.sprite = loadSprite(texturePath, &rect);
            coordinator->AddComponent<Sprite>(attachedModuleEntity_, modSprite);

            if (frameCount > 1) {
                Animation anim;
                anim.frameCount = frameCount;
                anim.frameWidth = frameWidth;
                anim.frameHeight = frameHeight;
                anim.frameTime = frameTime;
                anim.loop = true;
                anim.startX = 0;
                anim.startY = 0;
                anim.currentFrame = 0;
                anim.currentTime = 0.0f;
                anim.spacing = 0;
                coordinator->AddComponent<Animation>(attachedModuleEntity_, anim);
            }

            coordinator->AddComponent<Tag>(attachedModuleEntity_, Tag{"attached_module"});

            const char* names[] = {"", "laser", "homing", "spread", "wave"};
            std::cout << "[NetworkPlayState] 🔧 Module attached: " << names[moduleType] << std::endl;
        }
    }

    // Update position to follow player
    if (attachedModuleEntity_ != 0 && localPlayerEntity_ != 0) {
        if (coordinator->HasComponent<Position>(localPlayerEntity_) &&
            coordinator->HasComponent<Position>(attachedModuleEntity_)) {
            auto& playerPos = coordinator->GetComponent<Position>(localPlayerEntity_);
            auto& modPos = coordinator->GetComponent<Position>(attachedModuleEntity_);

            // Get player width
            float playerWidth = 99.0f; // 33 * 3 scale
            if (coordinator->HasComponent<Sprite>(localPlayerEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(localPlayerEntity_);
                playerWidth = sprite.textureRect.width * sprite.scaleX;
            }

            modPos.x = playerPos.x + playerWidth; // In front of player
            modPos.y = playerPos.y + 5.0f;
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
            game_->resetCoordinator();
            game_->getStateManager()->changeState(std::make_unique<MainMenuState>(game_));
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
            if (!isCharging_) {
                isCharging_ = true;
                chargeTime_ = 0.0f;
            }
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
    // Handle pending game end transitions (deferred from callbacks)
    if (pendingGameOver_) {
        pendingGameOver_ = false;
        // Use server score, fallback to locally tracked score if server sent 0
        uint32_t finalScore = (pendingScore_ > 0) ? pendingScore_ : currentScore_;
        game_->resetCoordinator();
        game_->getStateManager()->changeState(
            std::make_unique<GameOverState>(game_, static_cast<int>(finalScore)));
        return;
    }
    if (pendingVictory_) {
        pendingVictory_ = false;
        uint32_t finalScore = (pendingScore_ > 0) ? pendingScore_ : currentScore_;
        game_->resetCoordinator();
        game_->getStateManager()->changeState(
            std::make_unique<VictoryState>(game_, static_cast<int>(finalScore)));
        return;
    }

    // Update level transition text timer
    if (showLevelText_ && levelTransitionTimer_ > 0.0f) {
        levelTransitionTimer_ -= deltaTime;
        if (levelTransitionTimer_ <= 0.0f) {
            showLevelText_ = false;
        }
    }

    // Update spectator blink timer
    if (isSpectating_) {
        spectatorBlinkTimer_ += deltaTime;
    }

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

    // Send input to server every frame + apply local prediction
    auto* networkManager = game_->getNetworkManager();
    if (networkManager) {
        // Build input mask for prediction
        uint8_t inputMask = RType::ClientInput::buildInputMask(
            inputUp_, inputDown_, inputLeft_, inputRight_, inputFire_);

        // Apply client-side prediction immediately (before server confirms)
        if (predictionInitialized_ && !isSpectating_) {
            applyInputToLocalPlayer(inputMask, deltaTime);

            // Store in pending buffer for reconciliation
            PredictedInput pi;
            pi.seq = networkManager->getInputSequence() + 1; // Will be incremented in sendInput
            pi.inputMask = inputMask;
            pi.dt = deltaTime;
            pendingInputs_.push_back(pi);
            if (pendingInputs_.size() > MAX_PENDING_INPUTS) {
                pendingInputs_.pop_front();
            }
        }

        networkManager->sendInput(inputUp_, inputDown_, inputLeft_, inputRight_, inputFire_, chargeLevel);

        // Update network (receive snapshots, send pings)
        networkManager->update(deltaTime);
    }

    // Update local clock for interpolation
    localClock_ += deltaTime;

    // Interpolate remote entities smoothly between snapshots
    interpolateRemoteEntities(deltaTime);

    // Update local systems
    if (scrollingSystem_) scrollingSystem_->Update(deltaTime);
    if (animationSystem_) animationSystem_->Update(deltaTime);

    // Update charge indicator (follows player, shows when charging)
    updateChargeIndicator(deltaTime);
    
    // Hit blink effect: toggle sprite visibility
    if (hitBlinkTimer_ > 0.0f) {
        hitBlinkTimer_ -= deltaTime;
        auto coordinator = game_->getCoordinator();
        if (coordinator && localPlayerEntity_ != 0 && coordinator->HasComponent<Sprite>(localPlayerEntity_)) {
            auto& sprite = coordinator->GetComponent<Sprite>(localPlayerEntity_);
            // Blink every 0.1s
            bool visible = (static_cast<int>(hitBlinkTimer_ * 10.0f) % 2 == 0);
            sprite.layer = visible ? 1 : -100; // Hide by setting layer very low
        }
        if (hitBlinkTimer_ <= 0.0f) {
            // Restore visibility
            auto coordinator2 = game_->getCoordinator();
            if (coordinator2 && localPlayerEntity_ != 0 && coordinator2->HasComponent<Sprite>(localPlayerEntity_)) {
                auto& sprite = coordinator2->GetComponent<Sprite>(localPlayerEntity_);
                sprite.layer = 1;
            }
        }
    }
}

void NetworkPlayState::render()
{
    if (renderSystem_) {
        renderSystem_->Update(0.0f);
    }

    auto coordinator = game_->getCoordinator();
    auto* renderer = game_->getRenderer();
    
    // Draw health bars for all players
    if (coordinator && renderer) {
        float yOffset = 20.0f;
        
        // Iterate through all network entities to find players
        for (const auto& [serverId, localEntity] : networkEntities_) {
            if (!coordinator->HasComponent<Tag>(localEntity)) continue;
            
            auto& tag = coordinator->GetComponent<Tag>(localEntity);
            if (tag.name != "local_player" && tag.name != "remote_player") continue;
            
            // Get player HP from stored map
            uint16_t hp = 100; // Default
            uint16_t maxHp = 100;
            
            auto hpIt = playerHealthMap_.find(serverId);
            if (hpIt != playerHealthMap_.end()) {
                hp = hpIt->second;
            }
            
            float healthPercent = static_cast<float>(hp) / static_cast<float>(maxHp);
            
            // Determine size based on local/remote player
            bool isLocal = (tag.name == "local_player");
            float barWidth = isLocal ? 300.0f : 150.0f;   // Local player bar is much bigger
            float barHeight = isLocal ? 35.0f : 18.0f;    // Local player bar is much taller
            
            // Health bar background (dark red)
            eng::engine::rendering::FloatRect healthBg;
            healthBg.left = 20.0f;
            healthBg.top = yOffset;
            healthBg.width = barWidth;
            healthBg.height = barHeight;
            renderer->drawRect(healthBg, 0x800000FF, 0xFFFFFFFF, 2.0f);
            
            // Health bar foreground (green/yellow/red based on percentage)
            eng::engine::rendering::FloatRect healthFg;
            healthFg.left = 22.0f;
            healthFg.top = yOffset + 2.0f;
            healthFg.width = (barWidth - 4.0f) * healthPercent;
            healthFg.height = barHeight - 4.0f;
            
            uint32_t healthColor = 0x00FF00FF; // Green
            if (healthPercent < 0.5f) healthColor = 0xFFFF00FF; // Yellow
            if (healthPercent < 0.25f) healthColor = 0xFF0000FF; // Red
            
            renderer->drawRect(healthFg, healthColor, healthColor, 0.0f);
            
            yOffset += barHeight + 10.0f; // Next health bar below with spacing
        }
        
        // Draw score (top right)
        if (scoreText_) {
            scoreText_->setString("Score: " + std::to_string(currentScore_));
            renderer->drawText(*scoreText_);
        }

        // Draw boss HP bar (centered at top, only when boss is alive)
        if (bossServerId_ != 0 && bossHp_ > 0 && bossMaxHp_ > 0) {
            float bossBarWidth = 600.0f;
            float bossBarHeight = 25.0f;
            float bossBarX = (windowWidth_ - bossBarWidth) / 2.0f;
            float bossBarY = 20.0f;
            float bossPercent = static_cast<float>(bossHp_) / static_cast<float>(bossMaxHp_);

            // Background
            eng::engine::rendering::FloatRect bossBg;
            bossBg.left = bossBarX;
            bossBg.top = bossBarY;
            bossBg.width = bossBarWidth;
            bossBg.height = bossBarHeight;
            renderer->drawRect(bossBg, 0x400000FF, 0xFF4444FF, 2.0f);

            // Foreground
            eng::engine::rendering::FloatRect bossFg;
            bossFg.left = bossBarX + 2.0f;
            bossFg.top = bossBarY + 2.0f;
            bossFg.width = (bossBarWidth - 4.0f) * bossPercent;
            bossFg.height = bossBarHeight - 4.0f;

            uint32_t bossColor = 0xFF0000FF; // Red
            if (bossPercent > 0.5f) bossColor = 0xFF6600FF; // Orange
            if (bossPercent > 0.75f) bossColor = 0xFFAA00FF; // Yellow-orange
            renderer->drawRect(bossFg, bossColor, bossColor, 0.0f);

            // Boss name text
            if (!bossNameText_ && scoreFont_) {
                bossNameText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
                bossNameText_->setFont(scoreFont_.get());
                bossNameText_->setCharacterSize(28);
                bossNameText_->setFillColor(0xFF4444FF);
            }
            if (bossNameText_) {
                const char* bossNames[] = {"", "", "", "GUARDIAN BOSS", "RISING THREAT", "FINAL BOSS"};
                std::string name = (bossEnemyType_ < 6) ? bossNames[bossEnemyType_] : "BOSS";
                bossNameText_->setString(name);
                bossNameText_->setPosition(bossBarX + bossBarWidth / 2.0f - 100.0f, bossBarY + bossBarHeight + 5.0f);
                renderer->drawText(*bossNameText_);
            }
        }
        
        // Draw level text (center, during transitions)
        if (showLevelText_ && levelText_) {
            renderer->drawText(*levelText_);
        }

        // Draw spectator overlay (player died, watching others)
        if (isSpectating_ && !gameEndTriggered_) {
            // Semi-transparent dark overlay at top
            eng::engine::rendering::FloatRect specBg;
            specBg.left = 0.0f;
            specBg.top = windowHeight_ / 2.0f - 150.0f;
            specBg.width = static_cast<float>(windowWidth_);
            specBg.height = 200.0f;
            renderer->drawRect(specBg, 0x00000099, 0x00000099, 0.0f);

            if (spectatorText_) {
                // Blink effect
                float alpha = (std::sin(spectatorBlinkTimer_ * 2.5f) * 0.3f + 0.7f) * 255.0f;
                uint32_t a = static_cast<uint32_t>(alpha);
                spectatorText_->setFillColor(0xFF444400 | a);
                renderer->drawText(*spectatorText_);
            }
            if (spectatorSubText_) {
                renderer->drawText(*spectatorSubText_);
            }
        }
    }

    // Draw shield around local player if active
    if (shieldActive_ && localPlayerEntity_ != 0) {
        auto coordinator = game_->getCoordinator();
        auto* renderer = game_->getRenderer();
        if (coordinator && renderer && coordinator->HasComponent<Position>(localPlayerEntity_)) {
            auto& playerPos = coordinator->GetComponent<Position>(localPlayerEntity_);

            // Get player size for centering the shield
            float playerWidth = 66.0f;  // Default (33 * 2 scale)
            float playerHeight = 34.0f; // Default (17 * 2 scale)
            if (coordinator->HasComponent<Sprite>(localPlayerEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(localPlayerEntity_);
                playerWidth = sprite.textureRect.width * sprite.scaleX;
                playerHeight = sprite.textureRect.height * sprite.scaleY;
            }

            // Shield radius (bigger than player)
            float shieldRadius = std::max(playerWidth, playerHeight) * 0.8f;
            float centerX = playerPos.x + playerWidth / 2.0f;
            float centerY = playerPos.y + playerHeight / 2.0f;

            // Draw circular shield using segments
            const int segments = 32;
            const float pi = 3.14159f;

            for (int i = 0; i < segments; ++i) {
                float angle = (2.0f * pi * i) / segments;
                float x = centerX + shieldRadius * std::cos(angle);
                float y = centerY + shieldRadius * std::sin(angle);

                eng::engine::rendering::FloatRect dot;
                dot.left = x - 3.0f;
                dot.top = y - 3.0f;
                dot.width = 6.0f;
                dot.height = 6.0f;

                // Bright blue shield dots
                renderer->drawRect(dot, 0x0088FFFF, 0x0088FFFF, 0.0f);
            }

            // Inner glow ring
            for (int i = 0; i < segments; ++i) {
                float angle1 = (2.0f * pi * i) / segments;
                float angle2 = (2.0f * pi * (i + 1)) / segments;

                float x1 = centerX + shieldRadius * std::cos(angle1);
                float y1 = centerY + shieldRadius * std::sin(angle1);
                float x2 = centerX + shieldRadius * std::cos(angle2);
                float y2 = centerY + shieldRadius * std::sin(angle2);

                eng::engine::rendering::FloatRect lineRect;
                lineRect.left = std::min(x1, x2);
                lineRect.top = std::min(y1, y2);
                lineRect.width = std::max(std::abs(x2 - x1), 2.0f);
                lineRect.height = std::max(std::abs(y2 - y1), 2.0f);

                // Semi-transparent blue
                renderer->drawRect(lineRect, 0x0000FF40, 0x0088FFFF, 2.0f);
            }
        }
    }
}

void NetworkPlayState::onLevelChange(uint8_t level) {
    currentLevel_ = level;
    showLevelText_ = true;
    levelTransitionTimer_ = 3.0f;
    
    // Reset boss tracking for new level
    bossServerId_ = 0;
    bossHp_ = 0;
    bossMaxHp_ = 1000;
    bossEnemyType_ = 0;

    const char* levelNames[] = {"", "First Contact", "Rising Threat", "Final Assault"};
    std::string name = (level >= 1 && level <= 3) ? levelNames[level] : "Unknown";
    
    if (levelText_) {
        levelText_->setString("Level " + std::to_string(level) + ": " + name);
        // Re-center text
        levelText_->setPosition(windowWidth_ / 2.0f - 200.0f, windowHeight_ / 2.0f - 50.0f);
    }

    // Change background based on level
    spawnBackground();
    
    std::cout << "[NetworkPlayState] 🎮 Level changed to " << (int)level << ": " << name << std::endl;
}

// === Client-side prediction ===

void NetworkPlayState::applyMovementInput(float& x, float& y, uint8_t inputMask, float speed, float dt,
                                           float minX, float minY, float maxX, float maxY)
{
    float vx = 0.0f, vy = 0.0f;
    if (inputMask & 0x01) vy = -speed; // Up
    if (inputMask & 0x02) vy =  speed; // Down
    if (inputMask & 0x04) vx = -speed; // Left
    if (inputMask & 0x08) vx =  speed; // Right

    x += vx * dt;
    y += vy * dt;

    // Clamp to bounds (same as server)
    if (x < minX) x = minX;
    if (y < minY) y = minY;
    if (x > maxX) x = maxX;
    if (y > maxY) y = maxY;
}

void NetworkPlayState::applyInputToLocalPlayer(uint8_t inputMask, float dt)
{
    applyMovementInput(predictedX_, predictedY_, inputMask, PLAYER_SPEED, dt,
                       SCREEN_MIN_X, SCREEN_MIN_Y, SCREEN_MAX_X, SCREEN_MAX_Y);

    // Write predicted position to ECS immediately for responsive feel
    auto coordinator = game_->getCoordinator();
    if (coordinator && localPlayerEntity_ != 0 && coordinator->HasComponent<Position>(localPlayerEntity_)) {
        auto& pos = coordinator->GetComponent<Position>(localPlayerEntity_);
        pos.x = predictedX_;
        pos.y = predictedY_;
    }
}

void NetworkPlayState::reconcileLocalPlayer(const RType::EntityState& serverState, uint32_t ackedInputSeq)
{
    // Initialize prediction from first server state
    if (!predictionInitialized_) {
        predictedX_ = static_cast<float>(serverState.x);
        predictedY_ = static_cast<float>(serverState.y);
        predictionInitialized_ = true;
        pendingInputs_.clear();
        return;
    }

    // Drop acknowledged inputs
    while (!pendingInputs_.empty() && pendingInputs_.front().seq <= ackedInputSeq) {
        pendingInputs_.pop_front();
    }

    // Start from server's authoritative position
    float reconciledX = static_cast<float>(serverState.x);
    float reconciledY = static_cast<float>(serverState.y);

    // Replay all unacknowledged inputs on top of server state
    for (const auto& input : pendingInputs_) {
        applyMovementInput(reconciledX, reconciledY, input.inputMask, PLAYER_SPEED, input.dt,
                           SCREEN_MIN_X, SCREEN_MIN_Y, SCREEN_MAX_X, SCREEN_MAX_Y);
    }

    // Check error between reconciled and current predicted position
    float dx = reconciledX - predictedX_;
    float dy = reconciledY - predictedY_;
    float error = std::sqrt(dx * dx + dy * dy);

    if (error > RECONCILIATION_THRESHOLD) {
        // Significant divergence: snap to reconciled position
        predictedX_ = reconciledX;
        predictedY_ = reconciledY;
    }
    // Otherwise: keep current predicted position (close enough, avoids jitter)
}

// === Entity interpolation for remote entities ===

void NetworkPlayState::updateInterpolationBuffer(const RType::EntityState& state)
{
    auto& interp = interpolationBuffers_[state.id];

    // Shift: current becomes previous
    interp.previous = interp.current;

    // New state becomes current
    interp.current.x = static_cast<float>(state.x);
    interp.current.y = static_cast<float>(state.y);
    interp.current.vx = static_cast<float>(state.vx);
    interp.current.vy = static_cast<float>(state.vy);
    interp.current.hp = state.hp;
    interp.current.timestamp = localClock_;

    if (!interp.hasTwoSnapshots) {
        // First snapshot: copy to previous too
        interp.previous = interp.current;
        interp.hasTwoSnapshots = true;
    }
}

void NetworkPlayState::interpolateRemoteEntities(float /*deltaTime*/)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    for (auto& [entityId, interp] : interpolationBuffers_) {
        if (!interp.hasTwoSnapshots) continue;

        // Skip local player (handled by prediction)
        auto netIt = networkEntities_.find(entityId);
        if (netIt == networkEntities_.end()) continue;

        // Check if this is the local player entity
        ECS::Entity localEntity = netIt->second;
        if (localEntity == localPlayerEntity_ && predictionInitialized_) continue;

        // Compute interpolation factor
        float elapsed = localClock_ - interp.current.timestamp;
        // We want to render one snapshot interval behind (interpolation delay)
        float t = elapsed / SNAPSHOT_INTERVAL;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f; // Hold at current, no extrapolation

        // Lerp position
        float interpX = interp.previous.x + (interp.current.x - interp.previous.x) * t;
        float interpY = interp.previous.y + (interp.current.y - interp.previous.y) * t;

        // Apply interpolated position to ECS
        if (coordinator->HasComponent<Position>(localEntity)) {
            auto& pos = coordinator->GetComponent<Position>(localEntity);
            pos.x = interpX;
            pos.y = interpY;
        }
    }
}
