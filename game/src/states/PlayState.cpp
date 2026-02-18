#include "states/PlayState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include <ecs/Coordinator.hpp>
#include <rendering/IRenderer.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Damage.hpp>
#include <components/Tag.hpp>
#include <components/Weapon.hpp>
#include <components/Collectable.hpp>
#include "components/WaveMotion.hpp"
#include "components/Homing.hpp"
#include "components/LaserBeam.hpp"
#include <components/ScrollingBackground.hpp>
#include <components/Boundary.hpp>
#include <components/Lifetime.hpp>
#include <components/Animation.hpp>
#include <components/Score.hpp>
#include <systems/InputSystem.hpp>
#include <systems/MovementSystem.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/CollisionSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/BoundarySystem.hpp>
#include <scripting/LuaState.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <engine/Keyboard.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm> // for std::find

PlayState::PlayState(Game* game)
    : playerEntity_(0)
    , chargeIndicatorEntity_(0)
    , equippedModuleEntity_(0)
    , shootCooldown_(0.15f)
    , timeSinceLastShot_(0.0f)
    , isCharging_(false)
    , chargeTime_(0.0f)
    , shieldActive_(false)
    , shieldTimer_(0.0f)
    , equippedModuleType_("none")
    , laserBeamEntity_(0)
    , inputSystem_(nullptr)
    , movementSystem_(nullptr)
    , renderSystem_(nullptr)
    , animationSystem_(nullptr)
    , collisionSystem_(nullptr)
    , scrollingSystem_(nullptr)
    , playerSpeed_(400.0f)
    , shootCooldownTime_(0.15f)
    , playerMaxHealth_(3)
{
    game_ = game;
}

PlayState::~PlayState() = default;

void PlayState::onEnter()
{
    // std::cout << "[PlayState] Initializing gameplay..." << std::endl;

    // Clear active enemies list
    activeEnemies_.clear();

    // Load game configuration from Lua
    loadGameConfig();

    // Load player configuration from Lua
    loadPlayerConfig();
    
    // Load weapons configuration from Lua
    loadWeaponsConfig();
    
    // Load collectables configuration from Lua
    loadCollectablesConfig();
    
    // Load enemies configuration from Lua
    loadEnemiesConfig();
    
    // Load VFX configuration from Lua
    loadVFXConfig();

    // Setup all systems
    setupSystems();

    // Spawn background
    spawnBackground();

    // Spawn player
    spawnPlayer();

    // Spawn charge indicator
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
        levelText_->setString("Level 1: First Contact");
        levelText_->setPosition(windowWidth_ / 2.0f - 280.0f, windowHeight_ / 2.0f - 50.0f);
        
        // std::cout << "[PlayState] Score & Level display initialized" << std::endl;
    } else {
        std::cerr << "[PlayState] Failed to load font for score display" << std::endl;
    }
    
    // Start level system
    startLevel(1);

    // DEBUG: Verify enemies exist after spawn
    auto coordinator = game_->getCoordinator();
    if (coordinator) {
        // std::cout << "[PlayState] 🔍 Verifying enemy entities..." << std::endl;
        for (ECS::Entity entity = 0; entity < 100; ++entity) {
            if (coordinator->HasComponent<Tag>(entity)) {
                auto& tag = coordinator->GetComponent<Tag>(entity);
                if (tag.name == "Enemy") {
                    bool hasPos = coordinator->HasComponent<Position>(entity);
                    bool hasSprite = coordinator->HasComponent<Sprite>(entity);
                    bool hasVelocity = coordinator->HasComponent<Velocity>(entity);
                    // std::cout << "[PlayState] 🐛 Enemy entity=" << entity 
                              // << " Position=" << hasPos 
                              // << " Sprite=" << hasSprite 
                              // << " Velocity=" << hasVelocity << std::endl;
                    if (hasPos) {
                        auto& pos = coordinator->GetComponent<Position>(entity);
                        std::cout << "    → Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
                    }
                }
            }
        }
    }

    // std::cout << "[PlayState] ✅ Gameplay initialized" << std::endl;
    // std::cout << "[PlayState] Controls: ZQSD/Arrows=Move, Space=Shoot, ESC=Menu" << std::endl;
}

void PlayState::onExit()
{
    // std::cout << "[PlayState] Exiting gameplay..." << std::endl;

    auto coordinator = game_->getCoordinator();
    
    // Save highscore before destroying player
    if (coordinator && playerEntity_ != 0 && coordinator->HasComponent<Score>(playerEntity_)) {
        auto& score = coordinator->GetComponent<Score>(playerEntity_);
        if (score.current > score.highScore) {
            saveHighScore(score.current);
        }
    }
    
    // Destroy all gameplay entities
    if (coordinator && playerEntity_ != 0) {
        coordinator->DestroyEntity(playerEntity_);
    }

    // std::cout << "[PlayState] Gameplay cleanup complete" << std::endl;
}

void PlayState::setupSystems()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) {
        std::cerr << "[PlayState] ERROR: No coordinator!" << std::endl;
        return;
    }

    // Register all required systems (store shared_ptr directly)
    inputSystem_ = coordinator->RegisterSystem<InputSystem>(coordinator);
    movementSystem_ = coordinator->RegisterSystem<MovementSystem>(coordinator);
    renderSystem_ = coordinator->RegisterSystem<RenderSystem>();
    animationSystem_ = coordinator->RegisterSystem<AnimationSystem>();
    collisionSystem_ = coordinator->RegisterSystem<CollisionSystem>(coordinator);
    scrollingSystem_ = coordinator->RegisterSystem<ScrollingBackgroundSystem>(coordinator);
    lifetimeSystem_ = coordinator->RegisterSystem<LifetimeSystem>(coordinator);
    boundarySystem_ = coordinator->RegisterSystem<BoundarySystem>();

    // Set coordinators where needed
    if (renderSystem_) {
        renderSystem_->SetCoordinator(coordinator);
        renderSystem_->SetRenderer(game_->getRenderer());
    }
    if (animationSystem_) {
        animationSystem_->SetCoordinator(coordinator);
    }
    if (boundarySystem_) {
        boundarySystem_->SetCoordinator(coordinator);
        boundarySystem_->SetWindowSize(static_cast<float>(windowWidth_), static_cast<float>(windowHeight_));
    }

    // Define system signatures BEFORE calling Init()
    // MovementSystem: Position + Velocity
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Velocity>());
        coordinator->SetSystemSignature<MovementSystem>(sig);
        // std::cout << "[PlayState] MovementSystem signature set (Position + Velocity)" << std::endl;
    }
    
    // ScrollingBackgroundSystem: Position + ScrollingBackground
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<ScrollingBackground>());
        coordinator->SetSystemSignature<ScrollingBackgroundSystem>(sig);
        // std::cout << "[PlayState] ScrollingBackgroundSystem signature set (Position + ScrollingBackground)" << std::endl;
    }
    
    // RenderSystem: Position + Sprite
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Sprite>());
        coordinator->SetSystemSignature<RenderSystem>(sig);
        // std::cout << "[PlayState] RenderSystem signature set (Position + Sprite)" << std::endl;
    }
    
    // LifetimeSystem: Lifetime
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Lifetime>());
        coordinator->SetSystemSignature<LifetimeSystem>(sig);
        // std::cout << "[PlayState] LifetimeSystem signature set (Lifetime)" << std::endl;
    }
    
    // BoundarySystem: Position + Boundary
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Boundary>());
        coordinator->SetSystemSignature<BoundarySystem>(sig);
        // std::cout << "[PlayState] BoundarySystem signature set (Position + Boundary)" << std::endl;
    }

    // Initialize systems
    if (inputSystem_) inputSystem_->Init();
    if (movementSystem_) movementSystem_->Init();
    if (renderSystem_) renderSystem_->Init();
    if (animationSystem_) animationSystem_->Init();
    if (collisionSystem_) collisionSystem_->Init();
    if (scrollingSystem_) scrollingSystem_->Init();
    if (lifetimeSystem_) lifetimeSystem_->Init();
    if (boundarySystem_) boundarySystem_->Init();

    // std::cout << "[PlayState] Systems registered and initialized" << std::endl;

    // Set up collision callback for game-specific logic
    if (collisionSystem_) {
        collisionSystem_->SetCollisionCallback([this](ECS::Entity a, ECS::Entity b) {
            auto coordinator = game_->getCoordinator();
            if (!coordinator) return;

            // Helper: get tag name safely
            auto getTag = [&](ECS::Entity e) -> std::string {
                if (coordinator->HasComponent<Tag>(e)) {
                    return coordinator->GetComponent<Tag>(e).name;
                }
                return "";
            };

            std::string tagA = getTag(a);
            std::string tagB = getTag(b);

            // std::cout << "[Collision] 🔔 Detected: Entity " << a << " (" << tagA << ") <-> Entity " 
                      // << b << " (" << tagB << ")" << std::endl;

            // Tag matchers
            auto isPlayer = [](const std::string& t) { return t == "player" || t == "Player"; };
            auto isEnemy = [](const std::string& t) { return t == "enemy" || t == "Enemy"; };
            auto isPlayerProj = [](const std::string& t) { return t == "player_projectile"; };
            auto isEnemyProj = [](const std::string& t) { return t == "enemy_projectile"; };

            // Ignore enemy projectile vs enemy collisions
            if ((isEnemyProj(tagA) && isEnemy(tagB)) || (isEnemy(tagA) && isEnemyProj(tagB))) {
                // std::cout << "[Collision] ⏭️  Ignored: enemy_projectile <-> enemy collision" << std::endl;
                return;
            }

            // Helper: apply damage to target
            auto applyDamage = [&](ECS::Entity target, int dmg, ECS::Entity source, bool allowInvulnerability = true) {
                if (!coordinator->HasComponent<Health>(target)) {
                    // std::cout << "[Collision] ⚠️  Entity " << target << " has no Health component!" << std::endl;
                    return;
                }
                auto& health = coordinator->GetComponent<Health>(target);
                
                // Check invulnerability (only if allowed - e.g., for player)
                if (allowInvulnerability && (health.invulnerable || health.invincibilityTimer > 0.0f)) {
                    // std::cout << "[Collision] 🛡️  Entity " << target << " is invulnerable (timer: " 
                              // << health.invincibilityTimer << "s)" << std::endl;
                    return;
                }

                health.current -= dmg;
                
                // Only set invincibility timer for entities that should have it (player)
                if (allowInvulnerability) {
                    health.invincibilityTimer = health.invincibilityDuration;
                    health.isFlashing = true;
                }

                // std::cout << "[Collision] 💥 Entity " << target << " (tag: " << getTag(target) 
                          // << ") took " << dmg << " damage! Health: " << health.current << "/" << health.max << std::endl;

                if (health.current <= 0) {
                    health.isDead = true;
                    if (health.destroyOnDeath) {
                        // Add score for killing enemy
                        if (isEnemy(getTag(target))) {
                            // Boss detection: enemies with 100+ max HP are bosses (500 points)
                            uint32_t points = (health.max >= 100) ? 500 : 100;
                            addScore(points);
                        }
                        
                        // Spawn explosion animation for enemies only
                        if (isEnemy(getTag(target)) && coordinator->HasComponent<Position>(target)) {
                            auto& pos = coordinator->GetComponent<Position>(target);
                            
                            // std::cout << "[Collision] 🔥 Enemy died! Attempting to spawn explosion..." << std::endl;
                            
                            // Load explosion config from member variable
                            try {
                                if (!vfxConfig_.valid()) {
                                    std::cerr << "[Collision] ❌ vfxConfig_ not valid!" << std::endl;
                                } else {
                                    // std::cout << "[Collision] ✅ vfxConfig_ is valid, getting dead_enemies_animation..." << std::endl;
                                    sol::table explosionConfig = vfxConfig_["dead_enemies_animation"];
                                    
                                    if (explosionConfig.valid()) {
                                        // std::cout << "[Collision] ✅ explosionConfig is valid, creating explosion entity..." << std::endl;
                                ECS::Entity explosion = coordinator->CreateEntity();
                                
                                // Position
                                Position explosionPos;
                                explosionPos.x = pos.x;
                                explosionPos.y = pos.y;
                                coordinator->AddComponent(explosion, explosionPos);
                                
                                // Read sprite config from Lua
                                std::string spritePath = explosionConfig["texture_path"].get_or<std::string>("assets/vfx/dead_enemies_animation.png");
                                
                                // Read animation config
                                sol::table animConfig = explosionConfig["animation"];
                                int frameWidth = animConfig["frame_width"].get_or(33);
                                int frameHeight = animConfig["frame_height"].get_or(33);
                                int frameCount = animConfig["frame_count"].get_or(6);
                                float frameTime = animConfig["frame_time"].get_or(0.05f);
                                bool loop = animConfig["loop"].get_or(false);
                                
                                // Read sprite properties
                                sol::table spriteConfig = explosionConfig["sprite"];
                                sol::table scaleTable = spriteConfig["scale"];
                                float scaleX = scaleTable[1].get_or(2.0f);
                                float scaleY = scaleTable[2].get_or(2.0f);
                                int layer = spriteConfig["layer"].get_or(5);
                                
                                // Sprite with explosion texture
                                Sprite explosionSprite;
                                explosionSprite.texturePath = spritePath;
                                explosionSprite.textureRect = {0, 0, frameWidth, frameHeight}; // First frame
                                explosionSprite.layer = layer;
                                explosionSprite.scaleX = scaleX;
                                explosionSprite.scaleY = scaleY;
                                explosionSprite.sprite = loadSprite(explosionSprite.texturePath, &explosionSprite.textureRect);
                                coordinator->AddComponent(explosion, explosionSprite);
                                
                                // Animation
                                Animation explosionAnim;
                                explosionAnim.frameCount = frameCount;
                                explosionAnim.frameWidth = frameWidth;
                                explosionAnim.frameHeight = frameHeight;
                                explosionAnim.startX = 0;
                                explosionAnim.startY = 0;
                                explosionAnim.currentFrame = 0;
                                explosionAnim.frameTime = frameTime;
                                explosionAnim.currentTime = 0.0f;
                                explosionAnim.loop = loop;
                                explosionAnim.finished = false;
                                explosionAnim.spacing = 0;
                                coordinator->AddComponent(explosion, explosionAnim);
                                
                                // Auto-destroy after animation
                                float totalDuration = frameCount * frameTime;
                                Lifetime explosionLife;
                                explosionLife.maxLifetime = totalDuration;
                                explosionLife.destroyOnExpire = true;
                                coordinator->AddComponent(explosion, explosionLife);
                                
                                // std::cout << "[Collision] 💥 Spawned explosion animation at (" << pos.x << ", " << pos.y 
                                          // << ") - " << frameCount << " frames @ " << frameTime << "s/frame" << std::endl;
                                    } else {
                                        std::cerr << "[Collision] Warning: dead_enemies_animation config not found in vfx_config" << std::endl;
                                    }
                                }
                            } catch (const sol::error& e) {
                                std::cerr << "[Collision] Error loading VFX config: " << e.what() << std::endl;
                            }
                        }
                        
                        // Remove from active enemies list if it's an enemy
                        if (isEnemy(getTag(target))) {
                            activeEnemies_.erase(
                                std::remove(activeEnemies_.begin(), activeEnemies_.end(), target),
                                activeEnemies_.end()
                            );
                            // Also remove from fire patterns map
                            enemyFirePatterns_.erase(target);
                        }
                        
                        coordinator->DestroyEntity(target);
                        // std::cout << "[Collision] ☠️  Entity " << target << " destroyed!" << std::endl;
                    }
                }
            };

            // Player hit by enemy or enemy projectile
            if (isPlayer(tagA) && (isEnemy(tagB) || isEnemyProj(tagB))) {
                // Check if shield is active
                if (shieldActive_) {
                    // std::cout << "[Collision] 🛡️ Shield blocked attack from " << tagB << "!" << std::endl;
                    coordinator->DestroyEntity(b); // Destroy the projectile/enemy
                    return;
                }
                
                int dmg = 1;
                if (coordinator->HasComponent<Damage>(b)) {
                    dmg = coordinator->GetComponent<Damage>(b).amount;
                }
                // std::cout << "[Collision] 🎯 Player hit by " << tagB << " (entity " << b << "), damage=" << dmg << std::endl;
                applyDamage(a, dmg, b, true); // Player: enable invulnerability
                // Destroy enemy projectiles AND enemies on contact with player
                coordinator->DestroyEntity(b);
                return;
            }
            if (isPlayer(tagB) && (isEnemy(tagA) || isEnemyProj(tagA))) {
                // Check if shield is active
                if (shieldActive_) {
                    // std::cout << "[Collision] 🛡️ Shield blocked attack from " << tagA << "!" << std::endl;
                    coordinator->DestroyEntity(a); // Destroy the projectile/enemy
                    return;
                }
                
                int dmg = 1;
                if (coordinator->HasComponent<Damage>(a)) {
                    dmg = coordinator->GetComponent<Damage>(a).amount;
                }
                // std::cout << "[Collision] 🎯 Player hit by " << tagA << " (entity " << a << "), damage=" << dmg << std::endl;
                applyDamage(b, dmg, a, true); // Player: enable invulnerability
                // Destroy enemy projectiles AND enemies on contact with player
                coordinator->DestroyEntity(a);
                return;
            }

            // Player projectile hits enemy
            if (isPlayerProj(tagA) && isEnemy(tagB)) {
                int dmg = 1;
                if (coordinator->HasComponent<Damage>(a)) {
                    dmg = coordinator->GetComponent<Damage>(a).amount;
                }
                applyDamage(b, dmg, a, false); // Enemy: disable invulnerability
                
                // Don't destroy laser beams - only regular projectiles
                if (!coordinator->HasComponent<LaserBeam>(a)) {
                    coordinator->DestroyEntity(a);
                }
                return;
            }
            if (isPlayerProj(tagB) && isEnemy(tagA)) {
                int dmg = 1;
                if (coordinator->HasComponent<Damage>(b)) {
                    dmg = coordinator->GetComponent<Damage>(b).amount;
                }
                applyDamage(a, dmg, b, false); // Enemy: disable invulnerability
                
                // Don't destroy laser beams - only regular projectiles
                if (!coordinator->HasComponent<LaserBeam>(b)) {
                    coordinator->DestroyEntity(b);
                }
                return;
            }

            // Ignore collisions between player projectiles (laser beam vs normal shots)
            if (isPlayerProj(tagA) && isPlayerProj(tagB)) {
                return; // Don't process collision
            }
            
            // Projectile vs projectile collision (player projectile vs enemy projectile: destroy both)
            if ((isPlayerProj(tagA) && isEnemyProj(tagB)) || (isPlayerProj(tagB) && isEnemyProj(tagA))) {
                coordinator->DestroyEntity(a);
                coordinator->DestroyEntity(b);
                return;
            }
        });
        // std::cout << "[PlayState] Collision callback registered" << std::endl;
    }
}

void PlayState::loadGameConfig()
{
    auto& lua = game_->getLuaState();

    try {
        // Load game config
        lua.GetState().script_file("assets/scripts/config/game_config.lua");
        
        // Read window dimensions
        sol::table gameConfig = lua.GetState()["Game"];
        if (gameConfig.valid()) {
            sol::table window = gameConfig["window"];
            windowWidth_ = window["width"].get_or(1920);
            windowHeight_ = window["height"].get_or(1080);
            
            // Read gameplay config
            sol::table gameplay = gameConfig["gameplay"];
            inputSystemSpeed_ = gameplay["input_system_speed"].get_or(300.0f);
            
            // Read background config
            sol::table background = gameConfig["background"];
            backgroundPath_ = background["path"].get_or<std::string>("assets/background.png");
            backgroundScrollSpeed_ = background["scroll_speed"].get_or(100.0f);
            backgroundOriginalWidth_ = background["original_width"].get_or(9306);
            backgroundOriginalHeight_ = background["original_height"].get_or(199);
            backgroundScaleToWindow_ = background["scale_to_window"].get_or(true);
            
            // std::cout << "[PlayState] Game config loaded: window=" << windowWidth_ << "x" << windowHeight_ 
                      // << ", input_speed=" << inputSystemSpeed_
                      // << ", bg=" << backgroundPath_ << " (" << backgroundOriginalWidth_ << "x" << backgroundOriginalHeight_ << ")"
                      // << ", scroll_speed=" << backgroundScrollSpeed_ << "px/s" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading game config: " << e.what() << std::endl;
        // Default values
        windowWidth_ = 1920;
        windowHeight_ = 1080;
        inputSystemSpeed_ = 300.0f;
        backgroundPath_ = "assets/background.png";
        backgroundScrollSpeed_ = 100.0f;
        backgroundOriginalWidth_ = 9306;
        backgroundOriginalHeight_ = 199;
        backgroundScaleToWindow_ = true;
    }
}

void PlayState::loadPlayerConfig()
{
    auto& lua = game_->getLuaState();  // Get reference, not pointer

    try {
        // Load player config (path relative to build/game/)
        lua.GetState().script_file("assets/scripts/config/player_config.lua");
        
        // Read values from Lua table
        sol::table playerConfig = lua.GetState()["Player"];
        if (playerConfig.valid()) {
            playerSpeed_ = playerConfig["speed"].get_or(400.0f);
            playerMaxHealth_ = playerConfig["max_health"].get_or(3);
            
            // std::cout << "[PlayState] Player config loaded: speed=" << playerSpeed_ 
                      // << ", health=" << playerMaxHealth_ << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading player config: " << e.what() << std::endl;
    }
}

void PlayState::loadWeaponsConfig()
{
    auto& lua = game_->getLuaState();

    try {
        // Load weapons config
        lua.GetState().script_file("assets/scripts/config/weapons_config.lua");
        
        // Read basic_shot values from Lua table
        sol::table weaponsConfig = lua.GetState()["Weapons"];
        if (weaponsConfig.valid()) {
            sol::table basicShot = weaponsConfig["basic_shot"];
            if (basicShot.valid()) {
                projectileSpeed_ = basicShot["speed"].get_or(600.0f);
                projectileLifetime_ = basicShot["projectile_lifetime"].get_or(5.0f);
                projectileDamage_ = basicShot["damage"].get_or(10);
                shootCooldownTime_ = basicShot["fire_rate"].get_or(0.2f);
                
                // std::cout << "[PlayState] Basic shot loaded: speed=" << projectileSpeed_ 
                          // << ", lifetime=" << projectileLifetime_ 
                          // << ", damage=" << projectileDamage_
                          // << ", fire_rate=" << shootCooldownTime_ << "s" << std::endl;
            }
            
            // Load charge timings
            sol::table chargeTimings = weaponsConfig["charge_timings"];
            if (chargeTimings.valid()) {
                chargeThresholds_.clear();
                for (size_t i = 1; i <= 6; ++i) {
                    float timing = chargeTimings[i].get_or(0.0f);
                    chargeThresholds_.push_back(timing);
                }
                // std::cout << "[PlayState] Charge timings loaded: ";
                for (float t : chargeThresholds_) std::cout << t << "s ";
                std::cout << std::endl;
            }
            
            // Store weapons config for later use
            weaponsConfig_ = weaponsConfig;
            
            // std::cout << "[PlayState] ✅ Weapons config fully loaded (basic + 5 charge levels + modules)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading weapons config: " << e.what() << std::endl;
        // Default values
        projectileSpeed_ = 600.0f;
        projectileLifetime_ = 5.0f;
        projectileDamage_ = 10;
        shootCooldownTime_ = 0.2f;
    }
}

void PlayState::loadCollectablesConfig()
{
    auto& lua = game_->getLuaState();

    try {
        // Load collectables config
        lua.GetState().script_file("assets/scripts/config/collectables_config.lua");
        
        sol::table collectablesConfig = lua.GetState()["collectables_config"];
        if (collectablesConfig.valid()) {
            // std::cout << "[PlayState] ✅ Collectables config loaded (powerups + modules)" << std::endl;
        } else {
            std::cerr << "[PlayState] Warning: collectables_config not found in Lua" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading collectables config: " << e.what() << std::endl;
    }
}

void PlayState::loadEnemiesConfig()
{
    auto& lua = game_->getLuaState();

    try {
        // Load simplified enemies config
        lua.GetState().script_file("assets/scripts/config/enemies_simple.lua");
        
        sol::table enemiesConfig = lua.GetState()["EnemiesSimple"];
        if (enemiesConfig.valid()) {
            // Count enemy types
            int enemyCount = 0;
            for (auto& pair : enemiesConfig) {
                if (pair.second.is<sol::table>()) {
                    enemyCount++;
                }
            }
            // std::cout << "[PlayState] ✅ Enemies config loaded (" << enemyCount << " types)" << std::endl;
        } else {
            std::cerr << "[PlayState] Warning: EnemiesSimple not found in Lua" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading enemies config: " << e.what() << std::endl;
    }
}

void PlayState::loadVFXConfig()
{
    auto& lua = game_->getLuaState();

    try {
        // Load VFX configuration
        lua.GetState().script_file("assets/scripts/config/vfx_config.lua");
        
        vfxConfig_ = lua.GetState()["VFX"];  // Use "VFX" not "vfx_config"
        if (vfxConfig_.valid()) {
            // Count VFX types
            int vfxCount = 0;
            for (auto& pair : vfxConfig_) {
                if (pair.second.is<sol::table>()) {
                    vfxCount++;
                }
            }
            // std::cout << "[PlayState] ✅ VFX config loaded (" << vfxCount << " effects)" << std::endl;
        } else {
            std::cerr << "[PlayState] Warning: VFX table not found in Lua" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading VFX config: " << e.what() << std::endl;
    }
}

eng::engine::rendering::ISprite* PlayState::loadSprite(const std::string& texturePath, const eng::engine::rendering::IntRect* rect)
{
    // Create texture
    auto texture = std::make_unique<eng::engine::rendering::sfml::SFMLTexture>();
    if (!texture->loadFromFile(texturePath)) {
        std::cerr << "[PlayState] ERROR: Failed to load texture: " << texturePath << std::endl;
        return nullptr;
    }

    // Create sprite
    auto sprite = std::make_unique<eng::engine::rendering::sfml::SFMLSprite>();
    sprite->setTexture(texture.get());
    
    // Apply texture rect if provided
    if (rect && (rect->width != 0 || rect->height != 0)) {
        // std::cout << "[PlayState] Setting textureRect: (" << rect->left << "," << rect->top 
                  // << "," << rect->width << "," << rect->height << ")" << std::endl;
        sprite->setTextureRect(*rect);
    } else {
        // std::cout << "[PlayState] No textureRect applied for: " << texturePath << std::endl;
    }

    // Store ownership
    eng::engine::rendering::ISprite* spritePtr = sprite.get();
    loadedTextures_.push_back(std::move(texture));
    loadedSprites_.push_back(std::move(sprite));

    return spritePtr;
}


void PlayState::spawnBackground()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    // Create scrolling background entity
    ECS::Entity bg = coordinator->CreateEntity();
    
    // Add Position
    coordinator->AddComponent<Position>(bg, Position{0.0f, 0.0f});
    
    // Calculate scaling based on Lua config
    // Strategy: Scale to fit window height, let it scroll horizontally
    float scaleY = backgroundScaleToWindow_ ? (float)windowHeight_ / (float)backgroundOriginalHeight_ : 1.0f;
    float scaleX = scaleY;  // Keep aspect ratio
    float scaledWidth = (float)backgroundOriginalWidth_ * scaleX;  // Width after scaling
    
    // Add Sprite (background texture)
    Sprite bgSprite;
    bgSprite.texturePath = backgroundPath_;
    bgSprite.layer = -10;  // Behind everything
    bgSprite.scaleX = scaleX;
    bgSprite.scaleY = scaleY;
    bgSprite.sprite = loadSprite(backgroundPath_);
    coordinator->AddComponent<Sprite>(bg, bgSprite);

    // Add ScrollingBackground component for infinite scrolling
    ScrollingBackground scrollBg;
    scrollBg.scrollSpeed = backgroundScrollSpeed_;  // From Lua config
    scrollBg.horizontal = true;
    scrollBg.loop = true;
    scrollBg.spriteWidth = scaledWidth;  // Actual width after scaling
    coordinator->AddComponent<ScrollingBackground>(bg, scrollBg);

    // std::cout << "[PlayState] Background spawned: " << backgroundPath_ 
              // << " (" << backgroundOriginalWidth_ << "x" << backgroundOriginalHeight_ 
              // << " @ scale " << scaleX << "x" << scaleY << " = " << scaledWidth << " wide, speed=" 
              // << backgroundScrollSpeed_ << "px/s)" << std::endl;
}

void PlayState::spawnPlayer()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) {
        std::cerr << "[PlayState] ERROR: Cannot spawn player without coordinator" << std::endl;
        return;
    }

    // Create player entity
    playerEntity_ = coordinator->CreateEntity();

    // Position (left side, center vertically)
    coordinator->AddComponent<Position>(playerEntity_, Position{100.0f, 400.0f});

    // Velocity (controlled by input)
    coordinator->AddComponent<Velocity>(playerEntity_, Velocity{0.0f, 0.0f});

    // Sprite (player ship) - Using r-typesheet42.png
    // Frame layout: 5 frames (33x17 each): [Down2][Down1][Neutral][Up1][Up2]
    Sprite playerSprite;
    playerSprite.texturePath = "assets/players/r-typesheet42.png";
    playerSprite.textureRect = {66, 0, 33, 17};  // Frame 2 (neutral/center): column 2 = 33*2, line 0
    playerSprite.layer = 1;  // In front of background
    playerSprite.scaleX = 3.0f;
    playerSprite.scaleY = 3.0f;
    
    // std::cout << "[PlayState] Player textureRect BEFORE loadSprite: {" 
              // << playerSprite.textureRect.left << ", " 
              // << playerSprite.textureRect.top << ", "
              // << playerSprite.textureRect.width << ", " 
              // << playerSprite.textureRect.height << "}" << std::endl;
    
    playerSprite.sprite = loadSprite("assets/players/r-typesheet42.png", &playerSprite.textureRect);
    
    // std::cout << "[PlayState] Player textureRect AFTER loadSprite: {" 
              // << playerSprite.textureRect.left << ", " 
              // << playerSprite.textureRect.top << ", "
              // << playerSprite.textureRect.width << ", " 
              // << playerSprite.textureRect.height << "}" << std::endl;
    
    coordinator->AddComponent<Sprite>(playerEntity_, playerSprite);

    // Collider (for collisions) - Use sprite dimensions with scale
    Collider playerCollider;
    playerCollider.width = playerSprite.textureRect.width * playerSprite.scaleX;
    playerCollider.height = playerSprite.textureRect.height * playerSprite.scaleY;
    playerCollider.isTrigger = false;
    coordinator->AddComponent<Collider>(playerEntity_, playerCollider);

    // Health with shorter invincibility duration
    Health playerHealth;
    playerHealth.current = playerMaxHealth_;
    playerHealth.max = playerMaxHealth_;
    playerHealth.invincibilityDuration = 0.8f;  // 0.8 seconds invulnerability after hit
    playerHealth.destroyOnDeath = false;  // Don't auto-destroy player on death
    coordinator->AddComponent<Health>(playerEntity_, playerHealth);

    // Tag as player
    coordinator->AddComponent<Tag>(playerEntity_, Tag{"player"});

    // Score component
    Score playerScore;
    playerScore.current = 0;
    playerScore.highScore = loadHighScore(); // Load from file
    coordinator->AddComponent<Score>(playerEntity_, playerScore);

    // No Boundary component - we'll handle player boundaries manually in update()
    
    // std::cout << "[PlayState] Player spawned (Entity ID: " << playerEntity_ << ")" << std::endl;
}

void PlayState::spawnChargeIndicator()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) {
        std::cerr << "[PlayState] ERROR: Cannot spawn charge indicator without coordinator" << std::endl;
        return;
    }

    // Load charge animation config from Lua
    auto& lua = game_->getLuaState();
    sol::table chargeAnimConfig = weaponsConfig_["charge_animation"];
    
    if (!chargeAnimConfig.valid()) {
        std::cerr << "[PlayState] ERROR: charge_animation not found in weapons config" << std::endl;
        return;
    }

    // Read config from Lua
    std::string texturePath = chargeAnimConfig["texture_path"];
    sol::table offset = chargeAnimConfig["offset"];
    sol::table scale = chargeAnimConfig["scale"];
    int layer = chargeAnimConfig["layer"];
    sol::table frames = chargeAnimConfig["frames"];
    
    // Get first frame coordinates
    sol::table frame0 = frames[1];  // Lua arrays start at 1
    int frameX = frame0["x"];
    int frameY = frame0["y"];
    int frameWidth = frame0["width"];
    int frameHeight = frame0["height"];

    // Create charge indicator entity
    chargeIndicatorEntity_ = coordinator->CreateEntity();

    // Position (will be updated to follow player)
    coordinator->AddComponent<Position>(chargeIndicatorEntity_, Position{0.0f, 0.0f});

    // Sprite for charge animation - FROM LUA CONFIG
    Sprite chargeSprite;
    chargeSprite.texturePath = texturePath;
    chargeSprite.textureRect = {frameX, frameY, frameWidth, frameHeight};
    chargeSprite.layer = layer;
    chargeSprite.scaleX = scale["x"];
    chargeSprite.scaleY = scale["y"];
    chargeSprite.sprite = loadSprite(texturePath, &chargeSprite.textureRect);
    coordinator->AddComponent<Sprite>(chargeIndicatorEntity_, chargeSprite);

    // Animation component for charge frames
    // L'AnimationSystem va automatiquement cycler les frames horizontalement
    Animation chargeAnim;
    chargeAnim.frameTime = 0.1f;  // 10 FPS - animation rapide et fluide
    chargeAnim.currentTime = 0.0f;
    chargeAnim.currentFrame = 0;
    chargeAnim.frameCount = frames.size();  // 8 frames depuis Lua
    chargeAnim.loop = true;  // IMPORTANT: Boucle en continu
    chargeAnim.finished = false;
    chargeAnim.frameWidth = frameWidth;   // 32px
    chargeAnim.frameHeight = frameHeight; // 32px
    chargeAnim.startX = 0;  // Les frames commencent à x=0
    chargeAnim.startY = 0;  // Les frames sont à y=0
    chargeAnim.spacing = 0; // Pas d'espace entre les frames
    coordinator->AddComponent<Animation>(chargeIndicatorEntity_, chargeAnim);

    // std::cout << "[PlayState] Charge indicator spawned (Entity ID: " << chargeIndicatorEntity_ 
              // << ") with " << frames.size() << " frames from Lua" << std::endl;
}

void PlayState::updateChargeIndicator(float chargeTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || chargeIndicatorEntity_ == 0) return;

    // Load charge animation config from Lua
    sol::table chargeAnimConfig = weaponsConfig_["charge_animation"];
    if (!chargeAnimConfig.valid()) return;

    sol::table offset = chargeAnimConfig["offset"];
    int visibleLayer = chargeAnimConfig["layer"];
    
    // Délai avant d'afficher l'animation (pour permettre le spam de basic_shot)
    const float CHARGE_ANIMATION_DELAY = 0.15f;

    // Update position to follow player (FROM LUA CONFIG)
    if (coordinator->HasComponent<Position>(playerEntity_) && 
        coordinator->HasComponent<Position>(chargeIndicatorEntity_)) {
        auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
        auto& chargePos = coordinator->GetComponent<Position>(chargeIndicatorEntity_);
        
        // Position from Lua config
        chargePos.x = playerPos.x + offset["x"].get<float>();
        chargePos.y = playerPos.y + offset["y"].get<float>();
    }

    // Contrôle de l'animation basé sur le temps de charge
    if (coordinator->HasComponent<Animation>(chargeIndicatorEntity_)) {
        auto& anim = coordinator->GetComponent<Animation>(chargeIndicatorEntity_);
        
        // Si on est en dessous du délai, on n'affiche PAS l'animation (basic_shot zone)
        if (chargeTime < CHARGE_ANIMATION_DELAY) {
            // Pas d'animation, sprite caché
            if (coordinator->HasComponent<Sprite>(chargeIndicatorEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(chargeIndicatorEntity_);
                sprite.layer = -100;  // Caché
            }
            // Reset animation
            anim.currentFrame = 0;
            anim.currentTime = 0.0f;
            anim.finished = false;
        } else {
            // Afficher l'animation (charge zone)
            if (coordinator->HasComponent<Sprite>(chargeIndicatorEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(chargeIndicatorEntity_);
                sprite.layer = isCharging_ ? visibleLayer : -100;
            }
            // L'AnimationSystem gère les frames automatiquement, on laisse faire !
        }
    }
}

void PlayState::handleEvent(const eng::engine::InputEvent& event)
{
    if (!inputSystem_) return;

    if (event.type == eng::engine::EventType::KeyPressed)
    {
        // ESC = Pause/Menu
        if (event.key.code == eng::engine::Key::Escape)
        {
            // std::cout << "[PlayState] ESC pressed - returning to menu" << std::endl;
            game_->getStateManager()->popState();
            return;
        }

        // Movement keys
        if (event.key.code == eng::engine::Key::Z || event.key.code == eng::engine::Key::Up) {
            inputSystem_->SetActionState("move_up", true);
        }
        if (event.key.code == eng::engine::Key::S || event.key.code == eng::engine::Key::Down) {
            inputSystem_->SetActionState("move_down", true);
        }
        if (event.key.code == eng::engine::Key::Q || event.key.code == eng::engine::Key::Left) {
            inputSystem_->SetActionState("move_left", true);
        }
        if (event.key.code == eng::engine::Key::D || event.key.code == eng::engine::Key::Right) {
            inputSystem_->SetActionState("move_right", true);
        }

        // NOTE: Space key handling moved to update() for real-time detection
    }
    else if (event.type == eng::engine::EventType::KeyReleased)
    {
        // Movement keys
        if (event.key.code == eng::engine::Key::Z || event.key.code == eng::engine::Key::Up) {
            inputSystem_->SetActionState("move_up", false);
        }
        if (event.key.code == eng::engine::Key::S || event.key.code == eng::engine::Key::Down) {
            inputSystem_->SetActionState("move_down", false);
        }
        if (event.key.code == eng::engine::Key::Q || event.key.code == eng::engine::Key::Left) {
            inputSystem_->SetActionState("move_left", false);
        }
        if (event.key.code == eng::engine::Key::D || event.key.code == eng::engine::Key::Right) {
            inputSystem_->SetActionState("move_right", false);
        }

        // NOTE: Space key handling moved to update() for real-time detection
    }
}

int PlayState::calculateChargeLevel() const
{
    // Calculate charge level from chargeTime_ based on thresholds
    if (chargeThresholds_.empty()) return 0;
    
    for (int i = chargeThresholds_.size() - 1; i >= 0; --i) {
        if (chargeTime_ >= chargeThresholds_[i]) {
            return i;
        }
    }
    return 0;
}

void PlayState::handleShooting()
{
    // Check cooldown
    if (timeSinceLastShot_ < shootCooldownTime_) {
        // std::cout << "[PlayState] Shooting on cooldown (" << timeSinceLastShot_ << "s < " << shootCooldownTime_ << "s)" << std::endl;
        return;  // Still in cooldown
    }

    auto coordinator = game_->getCoordinator();
    if (!coordinator || playerEntity_ == 0) return;

    // Get player position
    if (!coordinator->HasComponent<Position>(playerEntity_)) return;
    auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);

    // Reset cooldown
    timeSinceLastShot_ = 0.0f;
    
    // If a module is equipped, fire ONLY from the module!
    if (equippedModuleEntity_ != 0 && equippedModuleType_ != "none") {
        // std::cout << "[PlayState] 🔫 Shooting with MODULE: " << equippedModuleType_ << std::endl;
        
        if (equippedModuleType_ == "spread") {
            fireSpreadModule();
        } else if (equippedModuleType_ == "wave") {
            fireWaveModule();
        } else if (equippedModuleType_ == "laser") {
            fireHomingModule();  // Laser module now fires homing missiles
        }
        
        return;  // Don't fire normal projectile when module is equipped
    }

    // No module equipped → fire normal projectile with charge system
    // NOUVEAU: Calculer le charge level depuis la FRAME D'ANIMATION
    int chargeLevel = 0;
    const float CHARGE_ANIMATION_DELAY = 0.15f;
    
    if (chargeTime_ < CHARGE_ANIMATION_DELAY) {
        // En dessous du délai → basic_shot
        chargeLevel = 0;
    } else if (coordinator->HasComponent<Animation>(chargeIndicatorEntity_)) {
        // Lire la frame actuelle de l'animation de charge
        auto& anim = coordinator->GetComponent<Animation>(chargeIndicatorEntity_);
        int currentFrame = anim.currentFrame;  // 0-7
        
        // Mapper les 8 frames sur 5 niveaux de charge
        // Frame 0-1 → Level 1
        // Frame 2-3 → Level 2
        // Frame 4-5 → Level 3
        // Frame 6   → Level 4
        // Frame 7   → Level 5
        if (currentFrame <= 1) {
            chargeLevel = 1;
        } else if (currentFrame <= 3) {
            chargeLevel = 2;
        } else if (currentFrame <= 5) {
            chargeLevel = 3;
        } else if (currentFrame == 6) {
            chargeLevel = 4;
        } else {
            chargeLevel = 5;  // Frame 7 = MAX POWER
        }
    } else {
        // Pas d'animation → basic_shot
        chargeLevel = 0;
    }
    
    // std::cout << "[PlayState] 🔫 Shooting with chargeTime=" << chargeTime_ << "s, chargeLevel=" << chargeLevel << std::endl;
    
    // Get weapon config based on charge level
    std::string weaponKey = chargeLevel == 0 ? "basic_shot" : "charge_level_" + std::to_string(chargeLevel);
    
    if (!weaponsConfig_.valid()) {
        std::cerr << "[PlayState] ❌ Weapons config not loaded!" << std::endl;
        return;
    }
    
    sol::table weaponConfig = weaponsConfig_[weaponKey];
    if (!weaponConfig.valid()) {
        std::cerr << "[PlayState] ❌ Weapon " << weaponKey << " not found in config!" << std::endl;
        return;
    }
    
    // Read weapon properties from Lua
    int damage = weaponConfig["damage"].get_or(10);
    float speed = weaponConfig["speed"].get_or(1000.0f);
    float lifetime = weaponConfig["projectile_lifetime"].get_or(4.0f);
    
    sol::table spriteConfig = weaponConfig["sprite"];
    std::string texturePath = spriteConfig["texture_path"].get_or<std::string>("assets/players/r-typesheet1.png");
    sol::table rectTable = spriteConfig["rect"];
    eng::engine::rendering::IntRect textureRect = {
        rectTable["x"].get_or(245),
        rectTable["y"].get_or(85),
        rectTable["width"].get_or(20),
        rectTable["height"].get_or(20)
    };
    float spriteScale = spriteConfig["scale"].get_or(2.0f);
    
    sol::table colliderConfig = weaponConfig["collider"];
    float colliderRadius = colliderConfig["radius"].get_or(7.0f);
    
    // std::cout << "[PlayState] 📊 Weapon config: " << weaponKey << std::endl;
    std::cout << "  - Texture: " << texturePath << std::endl;
    std::cout << "  - Rect: {" << textureRect.left << ", " << textureRect.top << ", " 
              << textureRect.width << ", " << textureRect.height << "}" << std::endl;
    std::cout << "  - Scale: " << spriteScale << "x" << std::endl;
    std::cout << "  - Damage: " << damage << ", Speed: " << speed << std::endl;
    
    // Create projectile entity
    ECS::Entity projectile = coordinator->CreateEntity();

    // Position (spawn in front of player, same height as modules)
    coordinator->AddComponent<Position>(projectile, Position{
        playerPos.x + 50.0f,  // Offset to the right of player
        playerPos.y + 5.0f    // Same Y offset as modules
    });

    // Velocity (move right) - FROM LUA CONFIG
    coordinator->AddComponent<Velocity>(projectile, Velocity{speed, 0.0f});
    
    // std::cout << "[PlayState] " << weaponKey << " fired! (damage=" << damage 
    //           << ", speed=" << speed << ", charge=" << chargeTime_ << "s)" << std::endl;

    // Sprite - FROM LUA CONFIG
    Sprite bulletSprite;
    bulletSprite.texturePath = texturePath;
    bulletSprite.textureRect = textureRect;
    bulletSprite.layer = 2;  // Above player
    bulletSprite.scaleX = spriteScale;
    bulletSprite.scaleY = spriteScale;
    
    // Animation - FROM LUA CONFIG (if available)
    sol::optional<sol::table> animConfig = weaponConfig["animation"];
    if (animConfig && animConfig->valid()) {
        int frameCount = (*animConfig)["frame_count"].get_or(1);
        int frameWidth = (*animConfig)["frame_width"].get_or((int)textureRect.width);
        int frameHeight = (*animConfig)["frame_height"].get_or((int)textureRect.height);
        float frameTime = (*animConfig)["frame_time"].get_or(0.1f);
        bool loop = (*animConfig)["loop"].get_or(true);
        
        // IMPORTANT: Adjust textureRect to show ONLY the first frame
        bulletSprite.textureRect.width = frameWidth;
        bulletSprite.textureRect.height = frameHeight;
        
        Animation anim;
        anim.frameTime = frameTime;
        anim.currentTime = 0.0f;
        anim.currentFrame = 0;
        anim.frameCount = frameCount;
        anim.loop = loop;
        anim.finished = false;
        anim.frameWidth = frameWidth;
        anim.frameHeight = frameHeight;
        anim.startX = textureRect.left;
        anim.startY = textureRect.top;
        anim.spacing = 0;
        
        coordinator->AddComponent<Animation>(projectile, anim);
        
        // std::cout << "[PlayState] 🎬 Animation added: " << frameCount << " frames @ " 
        //           << frameTime << "s/frame (frameWidth=" << frameWidth << ")" << std::endl;
    }
    
    // Load sprite AFTER adjusting textureRect
    bulletSprite.sprite = loadSprite(texturePath, &bulletSprite.textureRect);
    coordinator->AddComponent<Sprite>(projectile, bulletSprite);

    // Collider - FROM LUA CONFIG
    coordinator->AddComponent<Collider>(projectile, Collider{colliderRadius, false});

    // Damage - FROM LUA CONFIG
    coordinator->AddComponent<Damage>(projectile, Damage{damage});

    // Lifetime (auto-destroy) - FROM LUA CONFIG
    Lifetime projectileLifetime;
    projectileLifetime.maxLifetime = lifetime;
    projectileLifetime.destroyOnExpire = true;
    coordinator->AddComponent<Lifetime>(projectile, projectileLifetime);

    // Boundary (destroy when off screen)
    Boundary projectileBoundary;
    projectileBoundary.destroyOutOfBounds = true;
    projectileBoundary.margin = 50.0f;
    projectileBoundary.clampToBounds = false;
    coordinator->AddComponent<Boundary>(projectile, projectileBoundary);

    // Tag as player projectile
    coordinator->AddComponent<Tag>(projectile, Tag{"player_projectile"});
}


void PlayState::update(float deltaTime)
{
    // Update shoot cooldown
    timeSinceLastShot_ += deltaTime;
    
    // Update shield timer
    if (shieldActive_) {
        shieldTimer_ -= deltaTime;
        if (shieldTimer_ <= 0.0f) {
            shieldActive_ = false;
            shieldTimer_ = 0.0f;
            // std::cout << "[PlayState] 🛡️ Shield expired!" << std::endl;
        }
    }

    // Check if Space is held (using engine's real-time input API)
    bool spaceCurrentlyPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Space);
    
    // ALL MODULES and NO MODULE: Standard charging system
    {
        // Space just pressed → start charging
        if (spaceCurrentlyPressed && !isCharging_) {
            isCharging_ = true;
            chargeTime_ = 0.0f;
            // std::cout << "[PlayState] ⚡ Started charging..." << std::endl;
        }
        // Space held → increment charge time
        else if (spaceCurrentlyPressed && isCharging_) {
            chargeTime_ += deltaTime;
            // Update charge indicator animation
            updateChargeIndicator(chargeTime_);
        }
        // Space just released → fire!
        else if (!spaceCurrentlyPressed && isCharging_) {
            handleShooting();
            isCharging_ = false;
            chargeTime_ = 0.0f;
            // Hide charge indicator
            updateChargeIndicator(0.0f);
        }
        // Not charging → hide indicator
        else if (!isCharging_) {
            updateChargeIndicator(0.0f);
        }
    }

    // Update all systems
    if (inputSystem_) {
        inputSystem_->Update(deltaTime);
        
        // AFTER InputSystem updates: rescale player velocity to match Lua config
        // InputSystem uses hardcoded value, we need to apply playerSpeed_ from Lua
        auto coordinator = game_->getCoordinator();
        if (coordinator && playerEntity_ != 0 && coordinator->HasComponent<Velocity>(playerEntity_)) {
            auto& vel = coordinator->GetComponent<Velocity>(playerEntity_);
            
            // Rescale from InputSystem speed to playerSpeed_ from Lua config
            if (vel.dx != 0.0f) {
                vel.dx = (vel.dx / inputSystemSpeed_) * playerSpeed_;
            }
            if (vel.dy != 0.0f) {
                vel.dy = (vel.dy / inputSystemSpeed_) * playerSpeed_;
            }
            
            // Animate player sprite based on vertical velocity
            if (coordinator->HasComponent<Sprite>(playerEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(playerEntity_);
                
                // Progressive animation with 5 frames based on vertical velocity
                // r-typesheet42.png has 5 frames (33x17 each):
                // Frame 0: {0,0,33,17} - Extreme Down
                // Frame 1: {33,0,33,17} - Down
                // Frame 2: {66,0,33,17} - Neutral (center)
                // Frame 3: {99,0,33,17} - Up
                // Frame 4: {132,0,33,17} - Extreme Up
                
                // State tracking for progressive transitions
                static float extremeUpTimer = 0.0f;
                static float extremeDownTimer = 0.0f;
                static int currentFrame = 2;  // Start at neutral
                static float transitionTimer = 0.0f;
                const float transitionDelay = 0.08f;
                
                if (vel.dy < -10.0f) {
                    // Moving up
                    extremeDownTimer = 0.0f;
                    extremeUpTimer += deltaTime;
                    
                    if (extremeUpTimer > transitionDelay && currentFrame == 3) {
                        // Transition from moderate up to extreme up
                        currentFrame = 4;
                        sprite.textureRect = {132, 0, 33, 17};
                    } else if (currentFrame < 3) {
                        // Transition to moderate up
                        currentFrame = 3;
                        sprite.textureRect = {99, 0, 33, 17};
                        extremeUpTimer = 0.0f;
                    }
                } else if (vel.dy > 10.0f) {
                    // Moving down
                    extremeUpTimer = 0.0f;
                    extremeDownTimer += deltaTime;
                    
                    if (extremeDownTimer > transitionDelay && currentFrame == 1) {
                        // Transition from moderate down to extreme down
                        currentFrame = 0;
                        sprite.textureRect = {0, 0, 33, 17};
                    } else if (currentFrame > 1) {
                        // Transition to moderate down
                        currentFrame = 1;
                        sprite.textureRect = {33, 0, 33, 17};
                        extremeDownTimer = 0.0f;
                    }
                } else {
                    // Returning to neutral - progressive transition
                    extremeUpTimer = 0.0f;
                    extremeDownTimer = 0.0f;
                    transitionTimer += deltaTime;
                    
                    if (transitionTimer > transitionDelay * 0.5f) {
                        if (currentFrame == 4) {
                            // From extreme up to moderate up
                            currentFrame = 3;
                            sprite.textureRect = {99, 0, 33, 17};
                            transitionTimer = 0.0f;
                        } else if (currentFrame == 0) {
                            // From extreme down to moderate down
                            currentFrame = 1;
                            sprite.textureRect = {33, 0, 33, 17};
                            transitionTimer = 0.0f;
                        } else if (currentFrame == 3 || currentFrame == 1) {
                            // From moderate to neutral
                            currentFrame = 2;
                            sprite.textureRect = {66, 0, 33, 17};
                            transitionTimer = 0.0f;
                        }
                    }
                }
                
                // Apply the new textureRect to the SFML sprite
                if (sprite.sprite) {
                    sprite.sprite->setTextureRect(sprite.textureRect);
                }
            }
        }
    }
    if (scrollingSystem_) scrollingSystem_->Update(deltaTime);
    if (movementSystem_) movementSystem_->Update(deltaTime);
    
    // Manual player boundary check (keep player fully visible on screen)
    auto coordinator = game_->getCoordinator();
    if (coordinator && coordinator->HasComponent<Position>(playerEntity_) && coordinator->HasComponent<Sprite>(playerEntity_)) {
        auto& pos = coordinator->GetComponent<Position>(playerEntity_);
        auto& sprite = coordinator->GetComponent<Sprite>(playerEntity_);
        
        float playerWidth = sprite.textureRect.width * sprite.scaleX;
        float playerHeight = sprite.textureRect.height * sprite.scaleY;
        
        // Clamp player position with visibility margin (60px from bottom edge to keep fully visible)
        const float margin = 20.0f;
        const float bottomMargin = 80.0f;  // Larger margin for bottom due to rendering offset
        
        if (pos.x < margin) pos.x = margin;
        if (pos.y < margin) pos.y = margin;
        if (pos.x + playerWidth > windowWidth_ - margin) pos.x = windowWidth_ - playerWidth - margin;
        if (pos.y + playerHeight > windowHeight_ - bottomMargin) {
            pos.y = windowHeight_ - playerHeight - bottomMargin;
        }
        
        // Cancel velocity if at boundary
        if (coordinator->HasComponent<Velocity>(playerEntity_)) {
            auto& vel = coordinator->GetComponent<Velocity>(playerEntity_);
            if (pos.x <= margin && vel.dx < 0) vel.dx = 0.0f;
            if (pos.y <= margin && vel.dy < 0) vel.dy = 0.0f;
            if (pos.x + playerWidth >= windowWidth_ - margin && vel.dx > 0) vel.dx = 0.0f;
            if (pos.y + playerHeight >= windowHeight_ - bottomMargin && vel.dy > 0) {
                vel.dy = 0.0f;
            }
        }
    }
    
    if (boundarySystem_) boundarySystem_->Update(deltaTime);  // For projectiles and enemies
    if (lifetimeSystem_) lifetimeSystem_->Update(deltaTime);  // Destroy expired entities
    if (animationSystem_) animationSystem_->Update(deltaTime);
    if (collisionSystem_) collisionSystem_->Update(deltaTime);
    
    // Update wave projectile motion
    updateWaveProjectiles(deltaTime);
    
    // Update homing projectile tracking
    updateHomingProjectiles(deltaTime);
    
    // Update laser beam position
    updateLaserBeam(deltaTime);
    
    // Update invulnerability timers and flashing effect
    // Only check player + active enemies (not all 5000 entity slots)
    if (coordinator) {
        std::vector<ECS::Entity> entitiesToCheck;
        if (playerEntity_ != 0) entitiesToCheck.push_back(playerEntity_);
        for (auto e : activeEnemies_) entitiesToCheck.push_back(e);
        
        for (ECS::Entity entity : entitiesToCheck) {
            if (!coordinator->HasComponent<Health>(entity)) continue;
            
            auto& health = coordinator->GetComponent<Health>(entity);
            
            // Update invincibility timer
            if (health.invincibilityTimer > 0.0f) {
                health.invincibilityTimer -= deltaTime;
                if (health.invincibilityTimer <= 0.0f) {
                    health.invincibilityTimer = 0.0f;
                    health.isFlashing = false;
                }
            }
            
            // Handle flashing effect (show/hide sprite)
            if (health.isFlashing && coordinator->HasComponent<Sprite>(entity)) {
                auto& sprite = coordinator->GetComponent<Sprite>(entity);
                health.flashTimer += deltaTime;
                
                // Toggle visibility every 0.05 seconds (faster flashing for shorter invulnerability)
                const float flashInterval = 0.05f;
                if (health.flashTimer >= flashInterval) {
                    health.flashTimer = 0.0f;
                    
                    // Toggle sprite visibility using SFML alpha
                    if (sprite.sprite) {
                        auto* sfmlSprite = dynamic_cast<eng::engine::rendering::sfml::SFMLSprite*>(sprite.sprite);
                        if (sfmlSprite) {
                            auto color = sfmlSprite->getNativeSprite().getColor();
                            // Toggle between visible (255) and invisible (0)
                            color.a = (color.a == 255) ? 0 : 255;
                            sfmlSprite->getNativeSprite().setColor(color);
                        }
                    }
                }
            } else if (!health.isFlashing && coordinator->HasComponent<Sprite>(entity)) {
                // Ensure sprite is fully visible when not flashing
                auto& sprite = coordinator->GetComponent<Sprite>(entity);
                if (sprite.sprite) {
                    auto* sfmlSprite = dynamic_cast<eng::engine::rendering::sfml::SFMLSprite*>(sprite.sprite);
                    if (sfmlSprite) {
                        auto color = sfmlSprite->getNativeSprite().getColor();
                        color.a = 255; // Fully visible
                        sfmlSprite->getNativeSprite().setColor(color);
                    }
                }
                health.flashTimer = 0.0f;
            }
        }
    }
    
    // Update enemy movement patterns (zigzag, kamikaze, etc.)
    updateEnemyMovement(deltaTime);
    
    // Update enemy firing
    updateEnemyFiring(deltaTime);
    
    // Check collectable collisions with player
    checkCollectableCollisions();
    
    // Update attached module position to follow player
    updateAttachedModule();
    
    // Update level system (spawn waves, boss, transitions)
    updateLevelSystem(deltaTime);
    
    // Update level transition text timer
    if (showLevelText_ && levelTransitionTimer_ > 0.0f) {
        levelTransitionTimer_ -= deltaTime;
        if (levelTransitionTimer_ <= 0.0f) {
            showLevelText_ = false;
        }
    }
}

void PlayState::render()
{
    if (renderSystem_) {
        renderSystem_->Update(0.0f);  // RenderSystem doesn't use deltaTime
    }
    
    // Get coordinator and renderer once
    auto coordinator = game_->getCoordinator();
    auto* renderer = game_->getRenderer();
    
    // Draw UI: Health bar and Score
    if (coordinator && renderer && playerEntity_ != 0) {
        // Draw health bar
        if (coordinator->HasComponent<Health>(playerEntity_)) {
            auto& health = coordinator->GetComponent<Health>(playerEntity_);
            float healthPercent = static_cast<float>(health.current) / static_cast<float>(health.max);
            
            // Health bar background (dark red)
            eng::engine::rendering::FloatRect healthBg;
            healthBg.left = 20.0f;
            healthBg.top = 20.0f;
            healthBg.width = 200.0f;
            healthBg.height = 20.0f;
            renderer->drawRect(healthBg, 0x800000FF, 0xFFFFFFFF, 2.0f);
            
            // Health bar foreground (green/yellow/red based on percentage)
            eng::engine::rendering::FloatRect healthFg;
            healthFg.left = 22.0f;
            healthFg.top = 22.0f;
            healthFg.width = (200.0f - 4.0f) * healthPercent;
            healthFg.height = 16.0f;
            
            uint32_t healthColor = 0x00FF00FF; // Green
            if (healthPercent < 0.5f) healthColor = 0xFFFF00FF; // Yellow
            if (healthPercent < 0.25f) healthColor = 0xFF0000FF; // Red
            
            renderer->drawRect(healthFg, healthColor, healthColor, 0.0f);
        }
        
        // Update and draw score
        if (scoreText_ && coordinator->HasComponent<Score>(playerEntity_)) {
            auto& score = coordinator->GetComponent<Score>(playerEntity_);
            scoreText_->setString("Score: " + std::to_string(score.current));
            renderer->drawText(*scoreText_);
        }
        
        // Draw level text (center, during transitions)
        if (showLevelText_ && levelText_) {
            renderer->drawText(*levelText_);
        }
    }
    
    // DEBUG: Draw hitboxes for enemies
    if (coordinator && renderer) {
        // Draw shield around player if active
        if (shieldActive_ && playerEntity_ != 0 && coordinator->HasComponent<Position>(playerEntity_)) {
            auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
            
            // Get player size for centering the shield
            float playerWidth = 66.0f;  // Default
            float playerHeight = 34.0f;
            if (coordinator->HasComponent<Sprite>(playerEntity_)) {
                auto& sprite = coordinator->GetComponent<Sprite>(playerEntity_);
                playerWidth = sprite.textureRect.width * sprite.scaleX;
                playerHeight = sprite.textureRect.height * sprite.scaleY;
            }
            
            // Shield radius (bigger than player)
            float shieldRadius = std::max(playerWidth, playerHeight) * 0.8f;
            
            // Draw multiple rectangles to approximate a circle (8 sides)
            const int segments = 8;
            const float pi = 3.14159f;
            
            for (int i = 0; i < segments; ++i) {
                float angle1 = (2.0f * pi * i) / segments;
                float angle2 = (2.0f * pi * (i + 1)) / segments;
                
                float x1 = playerPos.x + playerWidth / 2.0f + shieldRadius * std::cos(angle1);
                float y1 = playerPos.y + playerHeight / 2.0f + shieldRadius * std::sin(angle1);
                float x2 = playerPos.x + playerWidth / 2.0f + shieldRadius * std::cos(angle2);
                float y2 = playerPos.y + playerHeight / 2.0f + shieldRadius * std::sin(angle2);
                
                // Draw line as thin rectangle
                eng::engine::rendering::FloatRect lineRect;
                lineRect.left = x1;
                lineRect.top = y1;
                lineRect.width = x2 - x1;
                lineRect.height = y2 - y1;
                
                // Blue shield (semi-transparent)
                renderer->drawRect(lineRect, 0x0000FF80, 0x0088FFFF, 3.0f);
            }
            
            // Draw a circular outline using multiple small rectangles
            for (int i = 0; i < segments * 4; ++i) {
                float angle = (2.0f * pi * i) / (segments * 4);
                float x = playerPos.x + playerWidth / 2.0f + shieldRadius * std::cos(angle);
                float y = playerPos.y + playerHeight / 2.0f + shieldRadius * std::sin(angle);
                
                eng::engine::rendering::FloatRect dot;
                dot.left = x - 2.0f;
                dot.top = y - 2.0f;
                dot.width = 4.0f;
                dot.height = 4.0f;
                
                // Bright blue dots
                renderer->drawRect(dot, 0x0088FFFF, 0x0088FFFF, 0.0f);
            }
        }
        
        for (ECS::Entity entity = 0; entity < 5000; ++entity) {
            // Draw hitboxes for enemies and player
            if (coordinator->HasComponent<Tag>(entity) && 
                coordinator->HasComponent<Position>(entity) && 
                coordinator->HasComponent<Collider>(entity)) {
                
                auto& tag = coordinator->GetComponent<Tag>(entity);
                auto& pos = coordinator->GetComponent<Position>(entity);
                auto& col = coordinator->GetComponent<Collider>(entity);
                
                // Draw red rectangle for enemies
                if (tag.name == "Enemy") {
                    eng::engine::rendering::FloatRect hitboxRect;
                    hitboxRect.left = pos.x;
                    hitboxRect.top = pos.y;
                    hitboxRect.width = col.width;
                    hitboxRect.height = col.height;
                    
                    // Red outline, transparent fill
                    renderer->drawRect(hitboxRect, 0x00000000, 0xFF0000FF, 2.0f);
                }
                // Draw green rectangle for player
                else if (tag.name == "player") {
                    eng::engine::rendering::FloatRect hitboxRect;
                    hitboxRect.left = pos.x;
                    hitboxRect.top = pos.y;
                    hitboxRect.width = col.width;
                    hitboxRect.height = col.height;
                    
                    // Green outline, transparent fill
                    renderer->drawRect(hitboxRect, 0x00000000, 0x00FF00FF, 2.0f);
                }
            }
        }
    }
    
    // TODO Phase 7: Charge indicator rendering
    // Need proper UI system with primitive rendering (rectangles, text)
    // For now, charge is tracked internally and affects projectile type
    
    /*
    // Draw charge indicator if charging
    if (isCharging_ && chargeTime_ > 0.0f) {
        auto coordinator = game_->getCoordinator();
        if (coordinator && coordinator->HasComponent<Position>(playerEntity_)) {
            auto& pos = coordinator->GetComponent<Position>(playerEntity_);
            
            // Calculate charge level and progress
            int chargeLevel = calculateChargeLevel();
            int nextLevel = std::min(chargeLevel + 1, 5);
            float progress = 0.0f;
            
            if (nextLevel < static_cast<int>(chargeThresholds_.size())) {
                float currentThreshold = chargeThresholds_[chargeLevel];
                float nextThreshold = chargeThresholds_[nextLevel];
                progress = (chargeTime_ - currentThreshold) / (nextThreshold - currentThreshold);
                progress = std::min(std::max(progress, 0.0f), 1.0f);
            }
            
            // TODO: Draw charge bar with proper UI system
        }
    }
    */
}

// ==========================================
// COLLECTABLES SYSTEM
// ==========================================

void PlayState::spawnTestCollectables()
{
    // std::cout << "[PlayState] Spawning test collectables..." << std::endl;
    
    // Spawn power-ups (orange et bleu) - en haut de l'écran
    spawnPowerup(600.0f, 200.0f, "powerup_orange");
    spawnPowerup(800.0f, 200.0f, "powerup_blue");
    
    // Spawn modules - au milieu de l'écran
    spawnModule(600.0f, 400.0f, "laser");
    spawnModule(800.0f, 400.0f, "spread");
    spawnModule(1000.0f, 400.0f, "wave");
    
    // std::cout << "[PlayState] ✅ Test collectables spawned" << std::endl;
}

void PlayState::spawnPowerup(float x, float y, const std::string& type)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    auto& lua = game_->getLuaState();
    
    try {
        // Récupérer la config du power-up depuis Lua
        sol::table collectablesConfig = lua.GetState()["collectables_config"];
        sol::table powerupsConfig = collectablesConfig["powerups"];
        
        // Déterminer quelle config charger (orange ou blue)
        std::string powerupKey = (type == "powerup_orange") ? "orange" : "blue";
        sol::table powerupData = powerupsConfig[powerupKey];
        
        if (!powerupData.valid()) {
            std::cerr << "[PlayState] Error: powerup config not found for " << type << std::endl;
            return;
        }
        
        ECS::Entity powerup = coordinator->CreateEntity();
        
        // Position
        coordinator->AddComponent<Position>(powerup, Position{x, y});
        
        // Velocity depuis Lua (augmentée pour plus de vitesse)
        sol::table velocityData = powerupData["velocity"];
        float velX = velocityData["x"].get_or(-150.0f);  // Augmenté de -50 à -150
        float velY = velocityData["y"].get_or(0.0f);
        coordinator->AddComponent<Velocity>(powerup, Velocity{velX, velY});
        
        // Sprite depuis Lua
        Sprite powerupSprite;
        powerupSprite.texturePath = powerupData["texture_path"].get_or<std::string>("assets/players/r-typesheet1.png");
        
        sol::table rectData = powerupData["rect"];
        powerupSprite.textureRect = {
            rectData["x"].get_or(0),
            rectData["y"].get_or(0),
            rectData["width"].get_or(16),
            rectData["height"].get_or(16)
        };
        
        sol::table scaleData = powerupData["scale"];
        powerupSprite.scaleX = scaleData["x"].get_or(2.0f);
        powerupSprite.scaleY = scaleData["y"].get_or(2.0f);
        powerupSprite.layer = powerupData["layer"].get_or(5);
        
        powerupSprite.sprite = loadSprite(powerupSprite.texturePath, &powerupSprite.textureRect);
        coordinator->AddComponent<Sprite>(powerup, powerupSprite);
        
        // Collectable component avec float settings
        Collectable collectableData;
        collectableData.type = type;
        collectableData.floatSpeed = powerupData["float_speed"].get_or(50.0f);
        collectableData.floatAmplitude = powerupData["float_amplitude"].get_or(10.0f);
        coordinator->AddComponent<Collectable>(powerup, collectableData);
        
        // Tag
        coordinator->AddComponent<Tag>(powerup, Tag{"powerup"});
        
        activeCollectables_.push_back(powerup);
        
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error spawning powerup: " << e.what() << std::endl;
    }
}

void PlayState::spawnModule(float x, float y, const std::string& moduleType)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    auto& lua = game_->getLuaState();
    
    try {
        // Récupérer la config du module depuis Lua
        sol::table collectablesConfig = lua.GetState()["collectables_config"];
        sol::table modulesConfig = collectablesConfig["modules"];
        sol::table moduleData = modulesConfig[moduleType];
        
        if (!moduleData.valid()) {
            std::cerr << "[PlayState] Error: module config not found for " << moduleType << std::endl;
            return;
        }
        
        ECS::Entity module = coordinator->CreateEntity();
        
        // Position
        coordinator->AddComponent<Position>(module, Position{x, y});
        
        // Velocity depuis Lua (augmentée pour plus de vitesse)
        sol::table velocityData = moduleData["velocity"];
        float velX = velocityData["x"].get_or(-150.0f);  // Augmenté de -50 à -150
        float velY = velocityData["y"].get_or(0.0f);
        coordinator->AddComponent<Velocity>(module, Velocity{velX, velY});
        
        // Sprite depuis Lua
        Sprite moduleSprite;
        std::string defaultPath = "assets/players/" + moduleType + "_module.png";
        moduleSprite.texturePath = moduleData["texture_path"].get_or(defaultPath);
        
        sol::table rectData = moduleData["rect"];
        moduleSprite.textureRect = {
            rectData["x"].get_or(0),
            rectData["y"].get_or(0),
            rectData["width"].get_or(16),
            rectData["height"].get_or(16)
        };
        
        sol::table scaleData = moduleData["scale"];
        moduleSprite.scaleX = scaleData["x"].get_or(2.0f);
        moduleSprite.scaleY = scaleData["y"].get_or(2.0f);
        moduleSprite.layer = moduleData["layer"].get_or(5);
        
        // Animation depuis Lua
        sol::table animData = moduleData["animation"];
        if (animData.valid()) {
            Animation moduleAnim;
            moduleAnim.frameCount = animData["frame_count"].get_or(1);
            moduleAnim.frameWidth = animData["frame_width"].get_or(16);
            moduleAnim.frameHeight = animData["frame_height"].get_or(16);
            moduleAnim.frameTime = animData["frame_time"].get_or(0.1f);
            moduleAnim.loop = animData["loop"].get_or(true);
            moduleAnim.startX = animData["start_x"].get_or(0);
            moduleAnim.startY = animData["start_y"].get_or(0);
            moduleAnim.currentFrame = 0;
            moduleAnim.currentTime = 0.0f;
            
            coordinator->AddComponent<Animation>(module, moduleAnim);
        }
        
        moduleSprite.sprite = loadSprite(moduleSprite.texturePath, &moduleSprite.textureRect);
        coordinator->AddComponent<Sprite>(module, moduleSprite);
        
        // Collectable component avec float settings
        Collectable collectableData;
        std::string defaultType = "module_" + moduleType;
        collectableData.type = moduleData["type"].get_or(defaultType);
        collectableData.floatSpeed = moduleData["float_speed"].get_or(50.0f);
        collectableData.floatAmplitude = moduleData["float_amplitude"].get_or(10.0f);
        coordinator->AddComponent<Collectable>(module, collectableData);
        
        // Tag
        coordinator->AddComponent<Tag>(module, Tag{"module"});
        
        activeCollectables_.push_back(module);
        
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error spawning module: " << e.what() << std::endl;
    }
}

// ==========================================
// COLLISION & PICKUP SYSTEM
// ==========================================

void PlayState::checkCollectableCollisions()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || playerEntity_ == 0) return;
    
    // Get player position and size for collision check
    if (!coordinator->HasComponent<Position>(playerEntity_) || !coordinator->HasComponent<Sprite>(playerEntity_)) {
        return;
    }
    
    auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
    auto& playerSprite = coordinator->GetComponent<Sprite>(playerEntity_);
    
    float playerWidth = playerSprite.textureRect.width * playerSprite.scaleX;
    float playerHeight = playerSprite.textureRect.height * playerSprite.scaleY;
    
    // Clean dead collectables from tracking list
    activeCollectables_.erase(
        std::remove_if(activeCollectables_.begin(), activeCollectables_.end(),
            [&](ECS::Entity e) { return !coordinator->HasComponent<Collectable>(e); }),
        activeCollectables_.end());
    
    // Check tracked collectables only
    for (ECS::Entity entity : activeCollectables_) {
        if (!coordinator->HasComponent<Position>(entity)) continue;
        if (!coordinator->HasComponent<Sprite>(entity)) continue;
        
        auto& collectableData = coordinator->GetComponent<Collectable>(entity);
        if (collectableData.pickedUp) continue;  // Already picked up
        
        auto& collectablePos = coordinator->GetComponent<Position>(entity);
        auto& collectableSprite = coordinator->GetComponent<Sprite>(entity);
        
        float collectableWidth = collectableSprite.textureRect.width * collectableSprite.scaleX;
        float collectableHeight = collectableSprite.textureRect.height * collectableSprite.scaleY;
        
        // Simple AABB collision detection
        bool collision = !(playerPos.x + playerWidth < collectablePos.x ||
                          playerPos.x > collectablePos.x + collectableWidth ||
                          playerPos.y + playerHeight < collectablePos.y ||
                          playerPos.y > collectablePos.y + collectableHeight);
        
        if (collision) {
            // std::cout << "[PlayState] 🎯 Collision with " << collectableData.type << std::endl;
            
            // Check if it's a powerup or module
            if (coordinator->HasComponent<Tag>(entity)) {
                auto& tag = coordinator->GetComponent<Tag>(entity);
                
                if (tag.name == "powerup") {
                    pickupPowerup(entity, collectableData.type);
                } else if (tag.name == "module") {
                    attachModule(entity, collectableData.type);
                }
            }
        }
    }
}

void PlayState::pickupPowerup(ECS::Entity powerupEntity, const std::string& type)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // std::cout << "[PlayState] ⚡ Picked up powerup: " << type << std::endl;
    
    // Apply powerup effect
    if (type == "powerup_orange") {
        // std::cout << "[PlayState] 💥 BOMB! Destroying all enemies on screen!" << std::endl;
        
        // Destroy all active enemies that are VISIBLE on screen
        std::vector<ECS::Entity> enemiesToDestroy = activeEnemies_; // Copy to avoid iterator invalidation
        int destroyedCount = 0;
        
        for (auto enemy : enemiesToDestroy) {
            if (!coordinator->HasComponent<Position>(enemy)) continue;
            
            // Get enemy position
            auto& pos = coordinator->GetComponent<Position>(enemy);
            
            // Check if enemy is visible on screen (with some margin)
            const float margin = 100.0f; // Small margin to catch enemies just entering
            bool isVisible = (pos.x >= -margin && pos.x <= windowWidth_ + margin &&
                            pos.y >= -margin && pos.y <= windowHeight_ + margin);
            
            if (!isVisible) {
                // std::cout << "[PlayState] ⚠️  Enemy at (" << pos.x << ", " << pos.y << ") is off-screen, skipping" << std::endl;
                continue; // Skip enemies that are off-screen
            }
            
            destroyedCount++;
            
            // Spawn explosion at enemy position
            
            // Spawn explosion at enemy position
            try {
                if (vfxConfig_.valid()) {
                    sol::table explosionConfig = vfxConfig_["dead_enemies_animation"];
                    
                    if (explosionConfig.valid()) {
                        ECS::Entity explosion = coordinator->CreateEntity();
                        
                        // Position
                        Position explosionPos;
                        explosionPos.x = pos.x;
                        explosionPos.y = pos.y;
                        coordinator->AddComponent(explosion, explosionPos);
                        
                        // Read sprite config from Lua
                        std::string spritePath = explosionConfig["texture_path"].get_or<std::string>("assets/enemies/dead_enemies_animation.png");
                        
                        // Read animation config
                        sol::table animConfig = explosionConfig["animation"];
                        int frameWidth = animConfig["frame_width"].get_or(33);
                        int frameHeight = animConfig["frame_height"].get_or(33);
                        int frameCount = animConfig["frame_count"].get_or(6);
                        float frameTime = animConfig["frame_time"].get_or(0.05f);
                        bool loop = animConfig["loop"].get_or(false);
                        
                        // Read sprite properties
                        sol::table spriteConfig = explosionConfig["sprite"];
                        sol::table scaleTable = spriteConfig["scale"];
                        float scaleX = scaleTable[1].get_or(2.0f);
                        float scaleY = scaleTable[2].get_or(2.0f);
                        int layer = spriteConfig["layer"].get_or(5);
                        
                        // Sprite with explosion texture
                        Sprite explosionSprite;
                        explosionSprite.texturePath = spritePath;
                        explosionSprite.textureRect = {0, 0, frameWidth, frameHeight};
                        explosionSprite.layer = layer;
                        explosionSprite.scaleX = scaleX;
                        explosionSprite.scaleY = scaleY;
                        explosionSprite.sprite = loadSprite(explosionSprite.texturePath, &explosionSprite.textureRect);
                        coordinator->AddComponent(explosion, explosionSprite);
                        
                        // Animation
                        Animation explosionAnim;
                        explosionAnim.frameCount = frameCount;
                        explosionAnim.frameWidth = frameWidth;
                        explosionAnim.frameHeight = frameHeight;
                        explosionAnim.startX = 0;
                        explosionAnim.startY = 0;
                        explosionAnim.currentFrame = 0;
                        explosionAnim.frameTime = frameTime;
                        explosionAnim.currentTime = 0.0f;
                        explosionAnim.loop = loop;
                        explosionAnim.finished = false;
                        explosionAnim.spacing = 0;
                        coordinator->AddComponent(explosion, explosionAnim);
                        
                        // Auto-destroy after animation
                        float totalDuration = frameCount * frameTime;
                        Lifetime explosionLife;
                        explosionLife.maxLifetime = totalDuration;
                        explosionLife.destroyOnExpire = true;
                        coordinator->AddComponent(explosion, explosionLife);
                    }
                }
            } catch (const sol::error& e) {
                std::cerr << "[PlayState] Error spawning explosion: " << e.what() << std::endl;
            }
            
            // Destroy the enemy
            coordinator->DestroyEntity(enemy);
            
            // Remove from activeEnemies_
            auto it = std::find(activeEnemies_.begin(), activeEnemies_.end(), enemy);
            if (it != activeEnemies_.end()) {
                activeEnemies_.erase(it);
            }
            
            // Remove from fire patterns map
            enemyFirePatterns_.erase(enemy);
        }
        
        // std::cout << "[PlayState] 💥 Destroyed " << destroyedCount << " visible enemies (out of " 
        //           << enemiesToDestroy.size() << " total)!" << std::endl;
        
    } else if (type == "powerup_blue") {
        // std::cout << "[PlayState] 🛡️ SHIELD activated for " << SHIELD_DURATION << " seconds!" << std::endl;
        shieldActive_ = true;
        shieldTimer_ = SHIELD_DURATION;
    }
    
    // Destroy the powerup entity
    coordinator->DestroyEntity(powerupEntity);
}

void PlayState::attachModule(ECS::Entity moduleEntity, const std::string& moduleType)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // std::cout << "[PlayState] 🔧 Attaching module: " << moduleType << std::endl;
    
    // If a laser beam is currently active, stop it before changing modules
    if (laserBeamEntity_ != 0) {
        // std::cout << "[PlayState] Stopping active laser because a new module is being attached" << std::endl;
        stopLaserBeam();
    }
    
    // If already have a module equipped, destroy the old one
    if (equippedModuleEntity_ != 0) {
        // std::cout << "[PlayState] Replacing old module: " << equippedModuleType_ << std::endl;
        coordinator->DestroyEntity(equippedModuleEntity_);
    }
    
    // Extract module type from collectable type (e.g., "module_laser" -> "laser")
    std::string extractedType = moduleType;
    if (moduleType.find("module_") == 0) {
        extractedType = moduleType.substr(7);  // Remove "module_" prefix
    }
    
    // Mark as picked up and remove Collectable component to stop collision checks
    if (coordinator->HasComponent<Collectable>(moduleEntity)) {
        auto& collectableData = coordinator->GetComponent<Collectable>(moduleEntity);
        collectableData.pickedUp = true;
    }
    
    // Remove velocity so it doesn't drift away
    if (coordinator->HasComponent<Velocity>(moduleEntity)) {
        auto& vel = coordinator->GetComponent<Velocity>(moduleEntity);
        vel.dx = 0.0f;
        vel.dy = 0.0f;
    }
    
    // Store equipped module
    equippedModuleEntity_ = moduleEntity;
    equippedModuleType_ = extractedType;
    
    // std::cout << "[PlayState] ✅ Module attached: " << equippedModuleType_ << std::endl;
}

void PlayState::startLaserBeam()
{
    // std::cout << "[PlayState] startLaserBeam() called" << std::endl;
    
    auto coordinator = game_->getCoordinator();
    if (!coordinator || equippedModuleEntity_ == 0 || equippedModuleType_ != "laser") {
        // std::cout << "[PlayState] ❌ Cannot start laser: coordinator=" << (coordinator ? "OK" : "NULL") 
        //           << ", moduleEntity=" << equippedModuleEntity_ 
        //           << ", moduleType=" << equippedModuleType_ << std::endl;
        return;
    }

    // Déjà actif
    if (laserBeamEntity_ != 0) {
        // std::cout << "[PlayState] ⚠️ Laser already active: " << laserBeamEntity_ << std::endl;
        return;
    }

    // Position du module
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    auto& moduleSprite = coordinator->GetComponent<Sprite>(equippedModuleEntity_);

    // Créer l'entité du laser
    laserBeamEntity_ = coordinator->CreateEntity();

    // Position au bord droit du module
    Position beamPos;
    beamPos.x = modulePos.x + moduleSprite.textureRect.width * moduleSprite.scaleX;
    beamPos.y = modulePos.y + (moduleSprite.textureRect.height * moduleSprite.scaleY / 2.0f) - 15.0f; // Centré (hauteur laser = 30)
    coordinator->AddComponent(laserBeamEntity_, beamPos);

    // Sprite: rectangle rouge allongé avec effet de lueur
    Sprite beamSprite;
    beamSprite.texturePath = "assets/players/laser_beam.png";
    beamSprite.textureRect = {0, 0, 2000, 30}; // Longueur 2000, largeur 30
    beamSprite.scaleX = 1.0f;
    beamSprite.scaleY = 1.0f;
    beamSprite.layer = 5; // Au dessus des autres éléments
    beamSprite.sprite = loadSprite(beamSprite.texturePath, &beamSprite.textureRect);
    coordinator->AddComponent(laserBeamEntity_, beamSprite);

    // std::cout << "[PlayState] ✅ Laser sprite loaded from: " << beamSprite.texturePath << " (2000x30)" << std::endl;

    // Composant LaserBeam
    LaserBeam laserData;
    laserData.damagePerSecond = 50.0f;
    laserData.width = 30.0f;
    laserData.maxLength = 2000.0f;
    laserData.active = true;
    coordinator->AddComponent(laserBeamEntity_, laserData);

    // Collider pour détecter les ennemis
    Collider beamCollider;
    beamCollider.width = 2000.0f;
    beamCollider.height = 30.0f;
    coordinator->AddComponent(laserBeamEntity_, beamCollider);

    // Tag pour le routing des collisions
    Tag beamTag;
    beamTag.name = "player_projectile";
    coordinator->AddComponent(laserBeamEntity_, beamTag);

    // Damage component pour les dégâts
    Damage beamDamage;
    beamDamage.amount = 50; // 50 DPS
    coordinator->AddComponent(laserBeamEntity_, beamDamage);

    // std::cout << "[PlayState] ✅ Laser beam started with Tag=player_projectile, Damage=50" << std::endl;
}

void PlayState::stopLaserBeam()
{
    if (laserBeamEntity_ != 0) {
        auto coordinator = game_->getCoordinator();
        if (coordinator) {
            // Check if entity still exists before destroying
            if (coordinator->HasComponent<Tag>(laserBeamEntity_)) {
                coordinator->DestroyEntity(laserBeamEntity_);
                // std::cout << "[PlayState] Laser beam stopped" << std::endl;
            }
            laserBeamEntity_ = 0;
        }
    }
}

void PlayState::updateLaserBeam(float /* deltaTime */)
{
    if (laserBeamEntity_ == 0 || equippedModuleEntity_ == 0) {
        return;
    }

    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;

    // Check if laser beam still exists
    if (!coordinator->HasComponent<Position>(laserBeamEntity_)) {
        laserBeamEntity_ = 0;
        return;
    }

    // Check if module still exists
    if (!coordinator->HasComponent<Position>(equippedModuleEntity_)) {
        equippedModuleEntity_ = 0;
        equippedModuleType_ = "none";
        laserBeamEntity_ = 0;
        return;
    }

    // Récupérer la position du module
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    auto& moduleSprite = coordinator->GetComponent<Sprite>(equippedModuleEntity_);

    // Mettre à jour la position du laser pour qu'il suive le module
    auto& beamPos = coordinator->GetComponent<Position>(laserBeamEntity_);
    beamPos.x = modulePos.x + moduleSprite.textureRect.width * moduleSprite.scaleX;
    beamPos.y = modulePos.y + (moduleSprite.textureRect.height * moduleSprite.scaleY / 2.0f) - 15.0f; // Centré
}

void PlayState::updateAttachedModule()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || equippedModuleEntity_ == 0 || playerEntity_ == 0) return;
    
    // Check if module still exists
    if (!coordinator->HasComponent<Position>(equippedModuleEntity_)) {
        equippedModuleEntity_ = 0;
        equippedModuleType_ = "none";
        return;
    }
    
    // Get player position
    if (!coordinator->HasComponent<Position>(playerEntity_)) return;
    auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
    
    // Get player sprite to calculate proper offset
    float playerWidth = 33.0f;  // Default player width
    if (coordinator->HasComponent<Sprite>(playerEntity_)) {
        auto& playerSprite = coordinator->GetComponent<Sprite>(playerEntity_);
        playerWidth = playerSprite.textureRect.width * playerSprite.scaleX;
    }
    
    // Update module position to follow player (attach in front of player)
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    modulePos.x = playerPos.x + playerWidth;  // In front of player
    modulePos.y = playerPos.y + 5.0f;  // Slightly below center
}

// ==========================================
// MODULE FIRING SYSTEMS
// ==========================================

void PlayState::fireSpreadModule()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || equippedModuleEntity_ == 0) return;
    
    // Get module position (where to spawn projectiles from)
    if (!coordinator->HasComponent<Position>(equippedModuleEntity_)) return;
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    
    // std::cout << "[PlayState] 🌟 Firing SPREAD module!" << std::endl;
    
    // Spread module fires 3 projectiles in a fan pattern
    // Angles: -15°, 0°, +15° (in radians)
    const float spreadAngles[3] = {-0.2617f, 0.0f, 0.2617f};  // -15°, 0°, +15°
    
    // Get basic_shot config for spread projectiles
    if (!weaponsConfig_.valid()) return;
    sol::table basicShot = weaponsConfig_["basic_shot"];
    if (!basicShot.valid()) return;
    
    // Read properties
    int damage = basicShot["damage"].get_or(10);
    float speed = basicShot["speed"].get_or(600.0f);
    float lifetime = basicShot["projectile_lifetime"].get_or(5.0f);
    
    sol::table spriteConfig = basicShot["sprite"];
    std::string texturePath = spriteConfig["texture_path"].get_or<std::string>("assets/players/r-typesheet1.png");
    sol::table rectTable = spriteConfig["rect"];
    eng::engine::rendering::IntRect textureRect = {
        rectTable["x"].get_or(245),
        rectTable["y"].get_or(85),
        rectTable["width"].get_or(20),
        rectTable["height"].get_or(20)
    };
    float spriteScale = spriteConfig["scale"].get_or(2.0f);
    
    sol::table colliderConfig = basicShot["collider"];
    float colliderRadius = colliderConfig["radius"].get_or(7.0f);
    
    // Create 3 projectiles with different angles
    for (int i = 0; i < 3; ++i) {
        ECS::Entity projectile = coordinator->CreateEntity();
        
        float angle = spreadAngles[i];
        float velocityX = speed * std::cos(angle);
        float velocityY = speed * std::sin(angle);
        
        // Position (spawn from module position)
        coordinator->AddComponent<Position>(projectile, Position{
            modulePos.x + 10.0f,  // Slightly in front of module
            modulePos.y
        });
        
        // Velocity (angled)
        coordinator->AddComponent<Velocity>(projectile, Velocity{velocityX, velocityY});
        
        // Sprite
        Sprite bulletSprite;
        bulletSprite.texturePath = texturePath;
        bulletSprite.textureRect = textureRect;
        bulletSprite.scaleX = spriteScale;
        bulletSprite.scaleY = spriteScale;
        bulletSprite.layer = 4;
        bulletSprite.sprite = loadSprite(texturePath, &textureRect);
        coordinator->AddComponent<Sprite>(projectile, bulletSprite);
        
        // Collider
        coordinator->AddComponent<Collider>(projectile, Collider{colliderRadius, false});
        
        // Damage
        coordinator->AddComponent<Damage>(projectile, Damage{damage});
        
        // Lifetime
        Lifetime projectileLifetime;
        projectileLifetime.maxLifetime = lifetime;
        projectileLifetime.destroyOnExpire = true;
        coordinator->AddComponent<Lifetime>(projectile, projectileLifetime);
        
        // Boundary (destroy when off screen)
        Boundary projectileBoundary;
        projectileBoundary.destroyOutOfBounds = true;
        projectileBoundary.margin = 50.0f;
        projectileBoundary.clampToBounds = false;
        coordinator->AddComponent<Boundary>(projectile, projectileBoundary);
        
        // Tag
        coordinator->AddComponent<Tag>(projectile, Tag{"player_projectile"});
        
        // std::cout << "[PlayState]   → Spread projectile " << (i+1) << " at angle " 
        //           << (angle * 57.2958f) << "° (vx=" << velocityX << ", vy=" << velocityY << ")" << std::endl;
    }
}

void PlayState::fireWaveModule()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || equippedModuleEntity_ == 0) return;
    
    // Get module position (where to spawn projectiles from)
    if (!coordinator->HasComponent<Position>(equippedModuleEntity_)) return;
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    
    // std::cout << "[PlayState] 🌊 Firing WAVE module!" << std::endl;
    
    // Get basic_shot config for wave projectile
    if (!weaponsConfig_.valid()) return;
    sol::table basicShot = weaponsConfig_["basic_shot"];
    if (!basicShot.valid()) return;
    
    // Read properties
    int damage = basicShot["damage"].get_or(10);
    float speed = basicShot["speed"].get_or(600.0f);
    float lifetime = basicShot["projectile_lifetime"].get_or(5.0f);
    
    sol::table spriteConfig = basicShot["sprite"];
    std::string texturePath = spriteConfig["texture_path"].get_or<std::string>("assets/players/r-typesheet1.png");
    sol::table rectTable = spriteConfig["rect"];
    eng::engine::rendering::IntRect textureRect = {
        rectTable["x"].get_or(245),
        rectTable["y"].get_or(85),
        rectTable["width"].get_or(20),
        rectTable["height"].get_or(20)
    };
    float spriteScale = spriteConfig["scale"].get_or(2.0f);
    
    sol::table colliderConfig = basicShot["collider"];
    float colliderRadius = colliderConfig["radius"].get_or(7.0f);
    
    // Create wave projectile
    ECS::Entity projectile = coordinator->CreateEntity();
    
    // Position (spawn from module position)
    coordinator->AddComponent<Position>(projectile, Position{
        modulePos.x + 10.0f,  // Slightly in front of module
        modulePos.y
    });
    
    // Velocity (straight horizontal initially)
    coordinator->AddComponent<Velocity>(projectile, Velocity{speed, 0.0f});
    
    // Wave motion component
    WaveMotion waveMotion;
    waveMotion.frequency = 4.0f;     // Oscillates 4 times per second
    waveMotion.amplitude = 60.0f;    // +/- 60 pixels up/down
    waveMotion.time = 0.0f;
    waveMotion.baseVelocityY = 0.0f;
    coordinator->AddComponent<WaveMotion>(projectile, waveMotion);
    
    // Sprite
    Sprite bulletSprite;
    bulletSprite.texturePath = texturePath;
    bulletSprite.textureRect = textureRect;
    bulletSprite.scaleX = spriteScale;
    bulletSprite.scaleY = spriteScale;
    bulletSprite.layer = 4;
    bulletSprite.sprite = loadSprite(texturePath, &textureRect);
    coordinator->AddComponent<Sprite>(projectile, bulletSprite);
    
    // Collider
    coordinator->AddComponent<Collider>(projectile, Collider{colliderRadius, false});
    
    // Damage
    coordinator->AddComponent<Damage>(projectile, Damage{damage});
    
    // Lifetime
    Lifetime projectileLifetime;
    projectileLifetime.maxLifetime = lifetime;
    projectileLifetime.destroyOnExpire = true;
    coordinator->AddComponent<Lifetime>(projectile, projectileLifetime);
    
    // Boundary (destroy when off screen)
    Boundary projectileBoundary;
    projectileBoundary.destroyOutOfBounds = true;
    projectileBoundary.margin = 50.0f;
    projectileBoundary.clampToBounds = false;
    coordinator->AddComponent<Boundary>(projectile, projectileBoundary);
    
    // Tag
    coordinator->AddComponent<Tag>(projectile, Tag{"player_projectile"});
    
    // std::cout << "[PlayState]   → Wave projectile spawned with sinusoidal motion!" << std::endl;
}

void PlayState::updateWaveProjectiles(float deltaTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Update all entities with WaveMotion component
    for (ECS::Entity entity = 0; entity < 1000; ++entity) {
        if (!coordinator->HasComponent<WaveMotion>(entity)) continue;
        if (!coordinator->HasComponent<Velocity>(entity)) continue;
        
        auto& waveMotion = coordinator->GetComponent<WaveMotion>(entity);
        auto& velocity = coordinator->GetComponent<Velocity>(entity);
        
        // Update time
        waveMotion.time += deltaTime;
        
        // Calculate sinusoidal Y velocity
        // velocity_y = amplitude * frequency * cos(frequency * time * 2π)
        float angularFrequency = waveMotion.frequency * 2.0f * 3.14159f;
        velocity.dy = waveMotion.amplitude * angularFrequency * std::cos(angularFrequency * waveMotion.time);
    }
}

void PlayState::fireHomingModule()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || equippedModuleEntity_ == 0) return;
    
    // Get module position (where to spawn projectiles from)
    if (!coordinator->HasComponent<Position>(equippedModuleEntity_)) return;
    auto& modulePos = coordinator->GetComponent<Position>(equippedModuleEntity_);
    
    // std::cout << "[PlayState] 🎯 Firing HOMING module!" << std::endl;
    
    // Get basic_shot config for homing projectile
    if (!weaponsConfig_.valid()) return;
    sol::table basicShot = weaponsConfig_["basic_shot"];
    if (!basicShot.valid()) return;
    
    // Read properties
    int damage = basicShot["damage"].get_or(10);
    float speed = basicShot["speed"].get_or(600.0f);
    float lifetime = basicShot["projectile_lifetime"].get_or(5.0f);
    
    sol::table spriteConfig = basicShot["sprite"];
    std::string texturePath = spriteConfig["texture_path"].get_or<std::string>("assets/players/r-typesheet1.png");
    sol::table rectTable = spriteConfig["rect"];
    eng::engine::rendering::IntRect textureRect = {
        rectTable["x"].get_or(245),
        rectTable["y"].get_or(85),
        rectTable["width"].get_or(20),
        rectTable["height"].get_or(20)
    };
    float spriteScale = spriteConfig["scale"].get_or(2.0f);
    
    sol::table colliderConfig = basicShot["collider"];
    float colliderRadius = colliderConfig["radius"].get_or(7.0f);
    
    // Create homing projectile
    ECS::Entity projectile = coordinator->CreateEntity();
    
    // Position (spawn from module position)
    coordinator->AddComponent<Position>(projectile, Position{
        modulePos.x + 10.0f,  // Slightly in front of module
        modulePos.y
    });
    
    // Velocity (start straight horizontal)
    coordinator->AddComponent<Velocity>(projectile, Velocity{speed, 0.0f});
    
    // Homing component
    Homing homingBehavior;
    homingBehavior.turnSpeed = 300.0f;       // Can turn 300 degrees per second
    homingBehavior.detectionRadius = 600.0f; // Detects enemies within 600 pixels
    homingBehavior.targetEntity = 0;         // No target yet
    homingBehavior.maxSpeed = speed;
    coordinator->AddComponent<Homing>(projectile, homingBehavior);
    
    // Sprite
    Sprite bulletSprite;
    bulletSprite.texturePath = texturePath;
    bulletSprite.textureRect = textureRect;
    bulletSprite.scaleX = spriteScale;
    bulletSprite.scaleY = spriteScale;
    bulletSprite.layer = 4;
    bulletSprite.sprite = loadSprite(texturePath, &textureRect);
    coordinator->AddComponent<Sprite>(projectile, bulletSprite);
    
    // Collider
    coordinator->AddComponent<Collider>(projectile, Collider{colliderRadius, false});
    
    // Damage
    coordinator->AddComponent<Damage>(projectile, Damage{damage});
    
    // Lifetime
    Lifetime projectileLifetime;
    projectileLifetime.maxLifetime = lifetime;
    projectileLifetime.destroyOnExpire = true;
    coordinator->AddComponent<Lifetime>(projectile, projectileLifetime);
    
    // Boundary (destroy when off screen)
    Boundary projectileBoundary;
    projectileBoundary.destroyOutOfBounds = true;
    projectileBoundary.margin = 50.0f;
    projectileBoundary.clampToBounds = false;
    coordinator->AddComponent<Boundary>(projectile, projectileBoundary);
    
    // Tag
    coordinator->AddComponent<Tag>(projectile, Tag{"player_projectile"});
    
    // std::cout << "[PlayState]   → Homing projectile spawned, will track enemies!" << std::endl;
}

void PlayState::updateHomingProjectiles(float deltaTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Update all entities with Homing component
    for (ECS::Entity entity = 0; entity < 1000; ++entity) {
        if (!coordinator->HasComponent<Homing>(entity)) continue;
        if (!coordinator->HasComponent<Position>(entity)) continue;
        if (!coordinator->HasComponent<Velocity>(entity)) continue;
        
        auto& homing = coordinator->GetComponent<Homing>(entity);
        auto& position = coordinator->GetComponent<Position>(entity);
        auto& velocity = coordinator->GetComponent<Velocity>(entity);
        
        // Find nearest enemy (TODO: For now, we don't have enemies, so homing just goes straight)
        // In the future, we'll search for entities with Tag "enemy" and find the closest one
        
        ECS::Entity nearestEnemy = 0;
        float nearestDistance = homing.detectionRadius;
        
        // Search for enemies (entities with Tag "Enemy")
        for (ECS::Entity targetEntity = 0; targetEntity < 1000; ++targetEntity) {
            if (!coordinator->HasComponent<Tag>(targetEntity)) continue;
            if (!coordinator->HasComponent<Position>(targetEntity)) continue;
            
            auto& tag = coordinator->GetComponent<Tag>(targetEntity);
            if (tag.name != "Enemy") continue;  // Capital E to match RenderSystem
            
            auto& targetPos = coordinator->GetComponent<Position>(targetEntity);
            
            // Calculate distance
            float dx = targetPos.x - position.x;
            float dy = targetPos.y - position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestEnemy = targetEntity;
            }
        }
        
        // If we found an enemy, steer towards it
        if (nearestEnemy != 0 && coordinator->HasComponent<Position>(nearestEnemy)) {
            homing.targetEntity = nearestEnemy;
            auto& targetPos = coordinator->GetComponent<Position>(nearestEnemy);
            
            // Calculate direction to target
            float dx = targetPos.x - position.x;
            float dy = targetPos.y - position.y;
            float targetAngle = std::atan2(dy, dx);
            
            // Calculate current velocity angle
            float currentAngle = std::atan2(velocity.dy, velocity.dx);
            
            // Calculate angle difference
            float angleDiff = targetAngle - currentAngle;
            
            // Normalize angle difference to [-π, π]
            while (angleDiff > 3.14159f) angleDiff -= 6.28318f;
            while (angleDiff < -3.14159f) angleDiff += 6.28318f;
            
            // Turn towards target (limited by turn speed)
            float maxTurnRadians = (homing.turnSpeed * 3.14159f / 180.0f) * deltaTime;
            float turnAmount = std::max(-maxTurnRadians, std::min(maxTurnRadians, angleDiff));
            
            float newAngle = currentAngle + turnAmount;
            
            // Update velocity to point in new direction
            velocity.dx = homing.maxSpeed * std::cos(newAngle);
            velocity.dy = homing.maxSpeed * std::sin(newAngle);
        } else {
            // No target found, continue straight
            homing.targetEntity = 0;
        }
    }
}

// ==========================================
// ENEMY SYSTEM
// ==========================================

void PlayState::spawnEnemy(const std::string& enemyType, float x, float y)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    auto& lua = game_->getLuaState();
    
    try {
        // std::cout << "[PlayState] 🔍 Attempting to spawn enemy: " << enemyType << std::endl;
        
        // Get enemy config from Lua
        sol::table enemiesConfig = lua.GetState()["EnemiesSimple"];
        if (!enemiesConfig.valid()) {
            std::cerr << "[PlayState] ❌ EnemiesSimple table not found in Lua!" << std::endl;
            return;
        }
        
        sol::table enemyData = enemiesConfig[enemyType];
        
        if (!enemyData.valid()) {
            std::cerr << "[PlayState] ❌ Enemy type not found: " << enemyType << std::endl;
            return;
        }
        
        // std::cout << "[PlayState] ✅ Found enemy config for: " << enemyType << std::endl;
        
        // Create enemy entity
        ECS::Entity enemy = coordinator->CreateEntity();
        
        // Position
        Position pos;
        pos.x = x;
        pos.y = y;
        coordinator->AddComponent<Position>(enemy, pos);
        
        // Velocity (from movement config)
        sol::table movementData = enemyData["movement"];
        sol::table velocityData = movementData["velocity"];
        Velocity vel;
        vel.dx = velocityData[1].get_or(0.0f);
        vel.dy = velocityData[2].get_or(0.0f);
        coordinator->AddComponent<Velocity>(enemy, vel);
        
        // Sprite
        sol::table spriteData = enemyData["sprite"];
        Sprite enemySprite;
        enemySprite.texturePath = spriteData["path"].get_or<std::string>("");
        
        sol::table rectData = spriteData["rect"];
        enemySprite.textureRect.left = rectData[1].get_or(0);
        enemySprite.textureRect.top = rectData[2].get_or(0);
        enemySprite.textureRect.width = rectData[3].get_or(32);
        enemySprite.textureRect.height = rectData[4].get_or(32);
        
        sol::table scaleData = spriteData["scale"];
        enemySprite.scaleX = scaleData[1].get_or(1.0f);
        enemySprite.scaleY = scaleData[2].get_or(1.0f);
        enemySprite.layer = 3;
        
        // std::cout << "[PlayState] Setting textureRect: (" << enemySprite.textureRect.left << ","
        //           << enemySprite.textureRect.top << "," << enemySprite.textureRect.width << ","
        //           << enemySprite.textureRect.height << ")" << std::endl;
        
        enemySprite.sprite = loadSprite(enemySprite.texturePath, &enemySprite.textureRect);
        
        if (enemySprite.sprite == nullptr) {
            std::cerr << "[PlayState] ❌ Failed to load sprite for enemy: " << enemyType << std::endl;
        } else {
            // std::cout << "[PlayState] ✅ Sprite loaded for: " << enemyType << std::endl;
        }
        
        coordinator->AddComponent<Sprite>(enemy, enemySprite);
        
        // Animation if available
        if (enemyData["animation"].valid()) {
            sol::table animData = enemyData["animation"];
            Animation enemyAnim;
            enemyAnim.frameCount = animData["frame_count"].get_or(1);
            enemyAnim.frameWidth = animData["frame_width"].get_or(32);
            enemyAnim.frameHeight = animData["frame_height"].get_or(32);
            enemyAnim.frameTime = animData["frame_time"].get_or(0.1f);
            enemyAnim.spacing = animData["spacing"].get_or(0);  // Load spacing between frames
            enemyAnim.startX = enemySprite.textureRect.left;
            enemyAnim.startY = enemySprite.textureRect.top;
            enemyAnim.currentFrame = 0;
            enemyAnim.currentTime = 0.0f;
            enemyAnim.loop = true;  // Les ennemis bouclent leur animation
            coordinator->AddComponent<Animation>(enemy, enemyAnim);
        }
        
        // Health
        Health health;
        health.current = enemyData["health"].get_or(10);
        health.max = health.current;
        coordinator->AddComponent<Health>(enemy, health);
        
        // Damage
        Damage damage;
        damage.amount = enemyData["damage"].get_or(10);
        coordinator->AddComponent<Damage>(enemy, damage);
        
        // Collider (apply sprite scale to collider dimensions)
        sol::table colliderData = enemyData["collider"];
        Collider collider;
        float baseWidth = colliderData["width"].get_or(32.0f);
        float baseHeight = colliderData["height"].get_or(32.0f);
        // Apply scale from sprite config
        collider.width = baseWidth * enemySprite.scaleX;
        collider.height = baseHeight * enemySprite.scaleY;
        coordinator->AddComponent<Collider>(enemy, collider);
        
        // Tag as enemy
        coordinator->AddComponent<Tag>(enemy, Tag{"Enemy"});  // Capital E to match RenderSystem
        
        // Boundary (destroy when off screen)
        Boundary boundary;
        boundary.destroyOutOfBounds = true;
        boundary.margin = 100.0f;  // Marge pour être détruit 100px hors écran
        boundary.clampToBounds = false;
        coordinator->AddComponent<Boundary>(enemy, boundary);
        
        // Weapon (if enemy can shoot)
        if (enemyData["weapon"].valid()) {
            sol::table weaponData = enemyData["weapon"];
            bool weaponEnabled = weaponData["enabled"].get_or(false);
            
            if (weaponEnabled) {
                Weapon weapon;
                weapon.fireRate = weaponData["fire_rate"].get_or(2.0f);
                weapon.projectileSpeed = weaponData["projectile_speed"].get_or(400.0f);
                weapon.projectileDamage = weaponData["projectile_damage"].get_or(5);
                weapon.timeSinceLastFire = 0.0f;  // Start ready to fire
                coordinator->AddComponent<Weapon>(enemy, weapon);
                
                // Store fire pattern in game-side map (not in engine component)
                std::string firePattern = weaponData["pattern"].get_or<std::string>("straight");
                enemyFirePatterns_[enemy] = firePattern;
                
                // std::cout << "[PlayState] ✅ Enemy has weapon (rate=" << weapon.fireRate 
                //           << "s, dmg=" << weapon.projectileDamage << ", pattern=" << firePattern << ")" << std::endl;
            }
        }
        
        // std::cout << "[PlayState] ✅ Spawned enemy: " << enemyType << " at (" << x << ", " << y << ")" << std::endl;
        
        // Add to active enemies list for optimized iteration
        activeEnemies_.push_back(enemy);
        
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error spawning enemy: " << e.what() << std::endl;
    }
}

void PlayState::spawnTestEnemies()
{
    // std::cout << "[PlayState] Spawning test enemies..." << std::endl;
    
    // Spawn enemies FAR to the right (off-screen) so they scroll into view
    // Window is 1920x1080, spawn them at x=2500-3500 (off-screen right)
    // They'll move left at -50 to -30 px/s AND background scrolls at 300 px/s
    // So visually they appear to move left at ~350-330 px/s relative to screen
    spawnEnemy("bug", 2500, 200);          // BasicEnemy (31px frames)
    spawnEnemy("fighter", 2800, 350);      // BatEnemy (51px frames)
    spawnEnemy("tank", 3100, 500);         // Kamikaze (17px frames)
    spawnEnemy("bug", 3500, 650);          // Another BasicEnemy
    
    // std::cout << "[PlayState] ✅ Test enemies spawned" << std::endl;
}

void PlayState::updateEnemyMovement(float deltaTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Static timer for zigzag pattern
    static std::unordered_map<ECS::Entity, float> zigzagTimers;
    
    // Iterate only over active enemies (optimized)
    for (auto entity : activeEnemies_) {
        if (!coordinator->HasComponent<Position>(entity)) continue;
        if (!coordinator->HasComponent<Velocity>(entity)) continue;
        
        auto& pos = coordinator->GetComponent<Position>(entity);
        auto& vel = coordinator->GetComponent<Velocity>(entity);
        
        // BOSS movement pattern: move to x=1500, then bob up/down
        if (bossAlive_ && entity == bossEntity_) {
            if (pos.x <= 1500.0f) {
                vel.dx = 0.0f;
                pos.x = 1500.0f;
                bossMovementTimer_ += deltaTime;
                vel.dy = std::sin(bossMovementTimer_ * 1.5f) * 100.0f;
            }
            continue; // Skip normal movement patterns for boss
        }
        
        // ZIGZAG pattern: enemies with vertical velocity oscillate
        if (std::abs(vel.dy) > 10.0f) {
            zigzagTimers[entity] += deltaTime;
            
            // Reverse vertical direction every 1 second
            if (zigzagTimers[entity] >= 1.0f) {
                vel.dy = -vel.dy;
                zigzagTimers[entity] = 0.0f;
            }
        }
        
        // KAMIKAZE pattern: enemies with very high speed rush towards player
        // Only kamikaze enemies (speed >= 450) should track the player
        if (std::abs(vel.dx) >= 450.0f && playerEntity_ != 0) {
            if (coordinator->HasComponent<Position>(playerEntity_)) {
                auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
                
                // Calculate direction to player
                float dirX = playerPos.x - pos.x;
                float dirY = playerPos.y - pos.y;
                float distance = std::sqrt(dirX * dirX + dirY * dirY);
                
                if (distance > 0.0f) {
                    // Normalize and apply speed
                    float speed = std::sqrt(vel.dx * vel.dx + vel.dy * vel.dy);
                    vel.dx = (dirX / distance) * speed;
                    vel.dy = (dirY / distance) * speed;
                }
            }
        }
    }
    
    // Clean up timers for destroyed enemies
    for (auto it = zigzagTimers.begin(); it != zigzagTimers.end(); ) {
        if (std::find(activeEnemies_.begin(), activeEnemies_.end(), it->first) == activeEnemies_.end()) {
            it = zigzagTimers.erase(it);
        } else {
            ++it;
        }
    }
}

void PlayState::spawnEnemyProjectile(const Position& pos, float dx, float dy, int damage, float speed)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    ECS::Entity projectile = coordinator->CreateEntity();
    
    // Position (spawn AHEAD of the enemy to avoid immediate collision)
    Position projPos;
    projPos.x = pos.x - 40.0f;  // Spawn 40px to the left (ahead of enemy movement)
    projPos.y = pos.y;
    coordinator->AddComponent(projectile, projPos);
    
    // Velocity (normalized direction * speed)
    float length = std::sqrt(dx * dx + dy * dy);
    Velocity projVel;
    if (length > 0.001f) {
        projVel.dx = (dx / length) * speed;
        projVel.dy = (dy / length) * speed;
    } else {
        projVel.dx = -speed;  // Default: shoot left
        projVel.dy = 0.0f;
    }
    coordinator->AddComponent(projectile, projVel);
    
    // Damage
    Damage projDmg;
    projDmg.amount = damage;
    coordinator->AddComponent(projectile, projDmg);
    
    // Tag
    Tag projTag;
    projTag.name = "enemy_projectile";
    coordinator->AddComponent(projectile, projTag);
    
    // Sprite (use enemy_bullets.png, not enemy_projectile.png)
    Sprite projSprite;
    projSprite.texturePath = "assets/enemies/enemy_bullets.png";
    projSprite.textureRect = {0, 0, 13, 8};
    projSprite.layer = 3;
    projSprite.scaleX = 2.0f;
    projSprite.scaleY = 2.0f;
    projSprite.sprite = loadSprite(projSprite.texturePath, &projSprite.textureRect);
    coordinator->AddComponent(projectile, projSprite);
    
    // Collider
    Collider projCol;
    projCol.width = 13 * 2.0f;
    projCol.height = 8 * 2.0f;
    coordinator->AddComponent(projectile, projCol);
    
    // Boundary (destroy when off screen)
    Boundary projBoundary;
    projBoundary.destroyOutOfBounds = true;
    projBoundary.margin = 50.0f;
    projBoundary.clampToBounds = false;
    coordinator->AddComponent(projectile, projBoundary);
    
    // Lifetime
    Lifetime projLife;
    projLife.maxLifetime = 5.0f;
    projLife.destroyOnExpire = true;
    coordinator->AddComponent(projectile, projLife);
}

void PlayState::updateEnemyFiring(float deltaTime)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Iterate only over active enemies (optimized)
    for (auto entity : activeEnemies_) {
        if (!coordinator->HasComponent<Weapon>(entity)) continue;
        if (!coordinator->HasComponent<Position>(entity)) continue;
        
        auto& weapon = coordinator->GetComponent<Weapon>(entity);
        auto& pos = coordinator->GetComponent<Position>(entity);
        
        // Update fire cooldown
        weapon.timeSinceLastFire += deltaTime;
        
        // Check if can fire
        if (weapon.timeSinceLastFire >= weapon.fireRate) {
            weapon.timeSinceLastFire = 0.0f;
            
            // Get fire pattern from game-side map (not stored in engine component)
            std::string firePattern = "straight";  // Default
            auto it = enemyFirePatterns_.find(entity);
            if (it != enemyFirePatterns_.end()) {
                firePattern = it->second;
            }
            
            if (firePattern == "straight") {
                // Shoot straight left
                spawnEnemyProjectile(pos, -1.0f, 0.0f, weapon.projectileDamage, weapon.projectileSpeed);
                
            } else if (firePattern == "aimed") {
                // Shoot towards player
                if (playerEntity_ != 0 && coordinator->HasComponent<Position>(playerEntity_)) {
                    auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);
                    float dx = playerPos.x - pos.x;
                    float dy = playerPos.y - pos.y;
                    spawnEnemyProjectile(pos, dx, dy, weapon.projectileDamage, weapon.projectileSpeed);
                } else {
                    spawnEnemyProjectile(pos, -1.0f, 0.0f, weapon.projectileDamage, weapon.projectileSpeed);
                }
                
            } else if (firePattern == "spread") {
                // Shoot 3 projectiles in fan pattern
                const float angleSpread = 30.0f * (3.14159f / 180.0f);  // 30 degrees in radians
                for (int i = -1; i <= 1; ++i) {
                    float angle = i * angleSpread;
                    float dx = std::cos(angle) * -1.0f - std::sin(angle) * 0.0f;
                    float dy = std::sin(angle) * -1.0f + std::cos(angle) * 0.0f;
                    spawnEnemyProjectile(pos, dx, dy, weapon.projectileDamage, weapon.projectileSpeed);
                }
                
            } else if (firePattern == "circle") {
                // Shoot in 8 directions
                const int projectileCount = 8;
                const float pi = 3.14159f;
                for (int i = 0; i < projectileCount; ++i) {
                    float angle = (2.0f * pi * i) / projectileCount;
                    float dx = std::cos(angle);
                    float dy = std::sin(angle);
                    spawnEnemyProjectile(pos, dx, dy, weapon.projectileDamage, weapon.projectileSpeed * 0.8f);
                }
                
            } else {
                // Default: straight
                spawnEnemyProjectile(pos, -1.0f, 0.0f, weapon.projectileDamage, weapon.projectileSpeed);
            }
        }
    }
}

void PlayState::checkProjectileEnemyCollisions()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // TODO: Implement collision between player projectiles and enemies
    // Loop through all projectiles with tag "player_bullet"
    // Check collision with entities tagged "enemy"
    // Apply damage, destroy projectile, destroy enemy if health <= 0
}

void PlayState::checkPlayerEnemyCollisions()
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || playerEntity_ == 0) return;
    
    // TODO: Implement collision between player and enemies/enemy bullets
    // Check collision with entities tagged "enemy" or "enemy_bullet"
    // Apply damage to player, handle invulnerability frames
}

// ========== SCORE SYSTEM ==========

uint32_t PlayState::loadHighScore()
{
    std::ifstream file("highscore.dat");
    if (file.is_open()) {
        uint32_t score;
        file >> score;
        file.close();
        // std::cout << "[PlayState] Loaded highscore: " << score << std::endl;
        return score;
    }
    return 0; // No highscore file yet
}

void PlayState::saveHighScore(uint32_t score)
{
    std::ofstream file("highscore.dat");
    if (file.is_open()) {
        file << score;
        file.close();
        // std::cout << "[PlayState] Saved highscore: " << score << std::endl;
    }
}

void PlayState::addScore(uint32_t points)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator || playerEntity_ == 0) return;
    
    if (coordinator->HasComponent<Score>(playerEntity_)) {
        auto& score = coordinator->GetComponent<Score>(playerEntity_);
        score.addPoints(points);
        
        // Save if new highscore
        if (score.current > score.highScore) {
            score.highScore = score.current;
            saveHighScore(score.highScore);
        }
    }
}

// ==========================================
// LEVEL SYSTEM (Solo)
// ==========================================

struct LevelWaveEnemy {
    std::string type;   // "bug", "fighter", "tank"
    int count;
    float interval;
};

struct LevelWave {
    float time;
    std::vector<LevelWaveEnemy> enemies;
};

struct SoloLevelConfig {
    int id;
    std::string name;
    std::vector<std::string> enemyTypes;
    std::vector<std::string> moduleTypes;   // "spread", "wave", "laser"
    float enemyInterval;
    float powerupInterval;
    float moduleInterval;
    int maxEnemies;
    std::vector<LevelWave> waves;
    
    // Boss
    std::string bossSprite;
    int bossHealth;
    float bossSpeed;
    float bossFireRate;
    std::string bossFirePattern;
    float bossSpawnTime;
    bool stopSpawningAtBoss;
};

static SoloLevelConfig getSoloLevelConfig(int level) {
    SoloLevelConfig config;
    config.stopSpawningAtBoss = true;
    
    switch (level) {
        case 1:
            config.id = 1;
            config.name = "First Contact";
            config.enemyTypes = {"bug"};
            config.moduleTypes = {"spread", "wave"};
            config.enemyInterval = 2.5f;
            config.powerupInterval = 15.0f;
            config.moduleInterval = 25.0f;
            config.maxEnemies = 8;
            config.waves = {
                {3.0f,  {{"bug", 3, 1.5f}}},
                {15.0f, {{"bug", 5, 1.0f}}},
                {30.0f, {{"bug", 6, 0.8f}}},
                {50.0f, {{"bug", 8, 0.6f}}},
                {70.0f, {{"bug", 10, 0.5f}}},
            };
            config.bossSprite = "assets/enemies/FirstBoss.png";
            config.bossHealth = 1000;
            config.bossSpeed = 80.0f;
            config.bossFireRate = 2.0f;
            config.bossFirePattern = "straight";
            config.bossSpawnTime = 90.0f;
            break;
        case 2:
            config.id = 2;
            config.name = "Rising Threat";
            config.enemyTypes = {"bug", "fighter"};
            config.moduleTypes = {"spread", "wave"};
            config.enemyInterval = 2.0f;
            config.powerupInterval = 12.0f;
            config.moduleInterval = 22.0f;
            config.maxEnemies = 12;
            config.waves = {
                {3.0f,  {{"bug", 3, 1.2f}, {"fighter", 2, 1.5f}}},
                {18.0f, {{"bug", 4, 0.8f}, {"fighter", 3, 1.0f}}},
                {35.0f, {{"fighter", 5, 0.7f}, {"bug", 3, 1.0f}}},
                {55.0f, {{"bug", 6, 0.5f}, {"fighter", 4, 0.6f}}},
                {75.0f, {{"bug", 8, 0.4f}, {"fighter", 5, 0.5f}}},
            };
            config.bossSprite = "assets/enemies/SecondBoss.png";
            config.bossHealth = 2000;
            config.bossSpeed = 60.0f;
            config.bossFireRate = 1.5f;
            config.bossFirePattern = "circle";
            config.bossSpawnTime = 95.0f;
            break;
        case 3:
        default:
            config.id = 3;
            config.name = "Final Assault";
            config.enemyTypes = {"bug", "fighter", "tank"};
            config.moduleTypes = {"laser", "spread", "wave"};
            config.enemyInterval = 1.5f;
            config.powerupInterval = 10.0f;
            config.moduleInterval = 20.0f;
            config.maxEnemies = 15;
            config.waves = {
                {3.0f,  {{"bug", 4, 0.8f}, {"fighter", 3, 1.0f}, {"tank", 2, 1.2f}}},
                {18.0f, {{"tank", 5, 0.6f}, {"bug", 3, 0.8f}}},
                {35.0f, {{"bug", 5, 0.5f}, {"fighter", 4, 0.6f}, {"tank", 3, 0.7f}}},
                {55.0f, {{"bug", 8, 0.3f}, {"fighter", 5, 0.4f}, {"tank", 4, 0.5f}}},
                {75.0f, {{"bug", 10, 0.3f}, {"fighter", 6, 0.4f}, {"tank", 5, 0.4f}}},
            };
            config.bossSprite = "assets/enemies/LastBossFly.png";
            config.bossHealth = 3000;
            config.bossSpeed = 100.0f;
            config.bossFireRate = 1.0f;
            config.bossFirePattern = "spread";
            config.bossSpawnTime = 95.0f;
            break;
    }
    return config;
}

void PlayState::startLevel(int level) {
    currentLevel_ = level;
    levelTimer_ = 0.0f;
    enemySpawnTimer_ = 0.0f;
    powerupSpawnTimer_ = 0.0f;
    moduleSpawnTimer_ = 0.0f;
    currentWaveIndex_ = 0;
    bossSpawned_ = false;
    bossEntity_ = 0;
    bossAlive_ = false;
    bossMovementTimer_ = 0.0f;
    levelActive_ = true;
    moduleRotationIdx_ = 0;
    waveSpawnState_ = WaveSpawnState{};
    showLevelText_ = true;
    levelTransitionTimer_ = 3.0f;
    
    // Clean up all remaining enemies from previous level
    auto coordinator = game_->getCoordinator();
    if (coordinator) {
        for (auto enemy : activeEnemies_) {
            if (coordinator->HasComponent<Tag>(enemy)) {
                coordinator->DestroyEntity(enemy);
            }
        }
        activeEnemies_.clear();
        
        // Clean up remaining collectables
        for (auto c : activeCollectables_) {
            if (coordinator->HasComponent<Collectable>(c)) {
                coordinator->DestroyEntity(c);
            }
        }
        activeCollectables_.clear();
    }
    
    // Clean up enemy fire patterns map
    enemyFirePatterns_.clear();
    
    auto config = getSoloLevelConfig(level);
    
    if (levelText_) {
        levelText_->setString("Level " + std::to_string(level) + ": " + config.name);
        levelText_->setPosition(windowWidth_ / 2.0f - 280.0f, windowHeight_ / 2.0f - 50.0f);
    }
}

void PlayState::updateLevelSystem(float deltaTime) {
    if (!levelActive_) return;
    
    levelTimer_ += deltaTime;
    auto config = getSoloLevelConfig(currentLevel_);
    
    // Count active enemies
    int enemyCount = 0;
    auto coordinator = game_->getCoordinator();
    if (coordinator) {
        // Clean dead enemies from activeEnemies_ list
        activeEnemies_.erase(
            std::remove_if(activeEnemies_.begin(), activeEnemies_.end(),
                [&](ECS::Entity e) { return !coordinator->HasComponent<Tag>(e); }),
            activeEnemies_.end());
        enemyCount = static_cast<int>(activeEnemies_.size());
    }
    
    // Check if boss was killed
    if (bossSpawned_ && bossAlive_) {
        if (coordinator && !coordinator->HasComponent<Health>(bossEntity_)) {
            // Boss entity was destroyed
            bossAlive_ = false;
            // std::cout << "[PlayState] 🏆 Boss defeated! Level " << currentLevel_ << " complete!" << std::endl;
            
            if (currentLevel_ < 3) {
                currentLevel_++;
                levelActive_ = false;
                // Small delay before next level starts
                startLevel(currentLevel_);
            } else {
                // std::cout << "[PlayState] 🎉 ALL LEVELS COMPLETE! You win!" << std::endl;
                showLevelText_ = true;
                levelTransitionTimer_ = 5.0f;
                if (levelText_) {
                    levelText_->setString("YOU WIN!");
                    levelText_->setPosition(windowWidth_ / 2.0f - 150.0f, windowHeight_ / 2.0f - 50.0f);
                }
                levelActive_ = false;
            }
            return;
        } else if (coordinator && coordinator->HasComponent<Health>(bossEntity_)) {
            auto& bossHealth = coordinator->GetComponent<Health>(bossEntity_);
            if (bossHealth.current <= 0) {
                bossAlive_ = false;
                coordinator->DestroyEntity(bossEntity_);
                // Remove from activeEnemies_
                activeEnemies_.erase(
                    std::remove(activeEnemies_.begin(), activeEnemies_.end(), bossEntity_),
                    activeEnemies_.end());
                // std::cout << "[PlayState] 🏆 Boss HP reached 0! Level " << currentLevel_ << " complete!" << std::endl;
                addScore(500); // Boss kill score
                
                if (currentLevel_ < 3) {
                    currentLevel_++;
                    startLevel(currentLevel_);
                } else {
                    // std::cout << "[PlayState] 🎉 ALL LEVELS COMPLETE!" << std::endl;
                    showLevelText_ = true;
                    levelTransitionTimer_ = 5.0f;
                    if (levelText_) {
                        levelText_->setString("YOU WIN!");
                        levelText_->setPosition(windowWidth_ / 2.0f - 150.0f, windowHeight_ / 2.0f - 50.0f);
                    }
                    levelActive_ = false;
                }
                return;
            }
        }
    }
    
    // Process wave spawning (ongoing wave)
    if (waveSpawnState_.active && currentWaveIndex_ < (int)config.waves.size()) {
        const auto& wave = config.waves[currentWaveIndex_];
        waveSpawnState_.spawnTimer += deltaTime;
        
        if (waveSpawnState_.enemyIdx < (int)wave.enemies.size()) {
            const auto& group = wave.enemies[waveSpawnState_.enemyIdx];
            if (waveSpawnState_.spawnTimer >= group.interval) {
                waveSpawnState_.spawnTimer = 0.0f;
                float y = 100.0f + (std::rand() % 880);
                spawnEnemy(group.type, 2000.0f, y);
                waveSpawnState_.spawnedCount++;
                
                if (waveSpawnState_.spawnedCount >= group.count) {
                    waveSpawnState_.enemyIdx++;
                    waveSpawnState_.spawnedCount = 0;
                }
            }
        } else {
            waveSpawnState_.active = false;
            currentWaveIndex_++;
        }
    }
    
    // Check for new waves
    if (currentWaveIndex_ < (int)config.waves.size() && !waveSpawnState_.active) {
        if (levelTimer_ >= config.waves[currentWaveIndex_].time) {
            waveSpawnState_.active = true;
            waveSpawnState_.enemyIdx = 0;
            waveSpawnState_.spawnedCount = 0;
            waveSpawnState_.spawnTimer = 0.0f;
            // std::cout << "[PlayState] 🌊 Wave " << (currentWaveIndex_ + 1) << " at " << levelTimer_ << "s" << std::endl;
        }
    }
    
    // Spawn boss when time comes
    if (!bossSpawned_ && levelTimer_ >= config.bossSpawnTime) {
        spawnBoss();
        bossSpawned_ = true;
        bossAlive_ = true;
    }
    
    // Regular spawning between waves
    bool canSpawnRegular = !(bossSpawned_ && config.stopSpawningAtBoss);
    if (canSpawnRegular && enemyCount < config.maxEnemies) {
        enemySpawnTimer_ += deltaTime;
        if (enemySpawnTimer_ >= config.enemyInterval) {
            enemySpawnTimer_ = 0.0f;
            // Random enemy from allowed types
            const auto& types = config.enemyTypes;
            const std::string& type = types[std::rand() % types.size()];
            float y = 100.0f + (std::rand() % 880);
            spawnEnemy(type, 2000.0f, y);
        }
    }
    
    // Spawn powerups
    powerupSpawnTimer_ += deltaTime;
    if (powerupSpawnTimer_ >= config.powerupInterval) {
        powerupSpawnTimer_ = 0.0f;
        float y = 100.0f + (std::rand() % 880);
        // Randomly pick orange or blue
        std::string type = (std::rand() % 2 == 0) ? "powerup_orange" : "powerup_blue";
        spawnPowerup(2000.0f, y, type);
    }
    
    // Spawn modules (only allowed types)
    moduleSpawnTimer_ += deltaTime;
    if (moduleSpawnTimer_ >= config.moduleInterval) {
        moduleSpawnTimer_ = 0.0f;
        const auto& modTypes = config.moduleTypes;
        const std::string& modType = modTypes[moduleRotationIdx_ % modTypes.size()];
        moduleRotationIdx_++;
        float y = 100.0f + (std::rand() % 880);
        spawnModule(2000.0f, y, modType);
    }
}

void PlayState::spawnBoss() {
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    auto config = getSoloLevelConfig(currentLevel_);
    
    // std::cout << "[PlayState] 👹 Spawning boss for Level " << currentLevel_ << "!" << std::endl;
    
    ECS::Entity boss = coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = 2000.0f;
    pos.y = 400.0f;
    coordinator->AddComponent<Position>(boss, pos);
    
    // Velocity - move left then stop at x=1500
    Velocity vel;
    vel.dx = -config.bossSpeed;
    vel.dy = 0.0f;
    coordinator->AddComponent<Velocity>(boss, vel);
    
    // Sprite
    Sprite bossSprite;
    bossSprite.texturePath = config.bossSprite;
    
    // Set up texture rect based on boss type
    if (currentLevel_ == 1) {
        // FirstBoss: 259x573, 4 vertical frames of 259x143
        bossSprite.textureRect = {0, 0, 259, 143};
        bossSprite.scaleX = 1.5f;
        bossSprite.scaleY = 1.5f;
    } else if (currentLevel_ == 2) {
        // SecondBoss: 645x211, 4 horizontal frames of 161x211
        bossSprite.textureRect = {0, 0, 161, 211};
        bossSprite.scaleX = 1.5f;
        bossSprite.scaleY = 1.5f;
    } else {
        // LastBoss: 324x71, 4 horizontal frames of 81x71
        bossSprite.textureRect = {0, 0, 81, 71};
        bossSprite.scaleX = 2.5f;
        bossSprite.scaleY = 2.5f;
    }
    bossSprite.layer = 3;
    bossSprite.sprite = loadSprite(bossSprite.texturePath, &bossSprite.textureRect);
    coordinator->AddComponent<Sprite>(boss, bossSprite);
    
    // Animation
    Animation bossAnim;
    if (currentLevel_ == 1) {
        bossAnim.frameCount = 4;
        bossAnim.frameWidth = 259;
        bossAnim.frameHeight = 143;
        bossAnim.frameTime = 0.15f;
        bossAnim.vertical = true; // FirstBoss: vertical spritesheet
    } else if (currentLevel_ == 2) {
        bossAnim.frameCount = 4;
        bossAnim.frameWidth = 161;
        bossAnim.frameHeight = 211;
        bossAnim.frameTime = 0.12f;
    } else {
        bossAnim.frameCount = 4;
        bossAnim.frameWidth = 81;
        bossAnim.frameHeight = 71;
        bossAnim.frameTime = 0.1f;
    }
    bossAnim.startX = 0;
    bossAnim.startY = 0;
    bossAnim.currentFrame = 0;
    bossAnim.currentTime = 0.0f;
    bossAnim.loop = true;
    coordinator->AddComponent<Animation>(boss, bossAnim);
    
    // Health (high HP!)
    Health health;
    health.current = config.bossHealth;
    health.max = config.bossHealth;
    coordinator->AddComponent<Health>(boss, health);
    
    // Damage
    Damage damage;
    damage.amount = 30;
    coordinator->AddComponent<Damage>(boss, damage);
    
    // Collider (large)
    Collider collider;
    collider.width = bossSprite.textureRect.width * bossSprite.scaleX;
    collider.height = bossSprite.textureRect.height * bossSprite.scaleY;
    coordinator->AddComponent<Collider>(boss, collider);
    
    // Tag
    coordinator->AddComponent<Tag>(boss, Tag{"Enemy"});
    
    // Weapon
    Weapon weapon;
    weapon.fireRate = config.bossFireRate;
    weapon.projectileSpeed = 500.0f;
    weapon.projectileDamage = 15;
    weapon.timeSinceLastFire = 0.0f;
    coordinator->AddComponent<Weapon>(boss, weapon);
    enemyFirePatterns_[boss] = config.bossFirePattern;
    
    bossEntity_ = boss;
    activeEnemies_.push_back(boss);
    
    // std::cout << "[PlayState] 👹 Boss spawned! HP=" << config.bossHealth << std::endl;
}
