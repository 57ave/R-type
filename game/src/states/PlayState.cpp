#include "states/PlayState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Damage.hpp>
#include <components/Tag.hpp>
#include <components/Collectable.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Boundary.hpp>
#include <components/Lifetime.hpp>
#include <components/Animation.hpp>
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
#include <engine/Keyboard.hpp>
#include <iostream>

PlayState::PlayState(Game* game)
    : playerEntity_(0)
    , chargeIndicatorEntity_(0)
    , shootCooldown_(0.15f)
    , timeSinceLastShot_(0.0f)
    , isCharging_(false)
    , chargeTime_(0.0f)
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
    std::cout << "[PlayState] Initializing gameplay..." << std::endl;

    // Load game configuration from Lua
    loadGameConfig();

    // Load player configuration from Lua
    loadPlayerConfig();
    
    // Load weapons configuration from Lua
    loadWeaponsConfig();
    
    // Load collectables configuration from Lua
    loadCollectablesConfig();

    // Setup all systems
    setupSystems();

    // Spawn background
    spawnBackground();

    // Spawn player
    spawnPlayer();

    // Spawn charge indicator
    spawnChargeIndicator();
    
    // Spawn test collectables (temporary for testing)
    spawnTestCollectables();

    std::cout << "[PlayState] ✅ Gameplay initialized" << std::endl;
    std::cout << "[PlayState] Controls: ZQSD/Arrows=Move, Space=Shoot, ESC=Menu" << std::endl;
}

void PlayState::onExit()
{
    std::cout << "[PlayState] Exiting gameplay..." << std::endl;

    auto coordinator = game_->getCoordinator();
    
    // Destroy all gameplay entities
    if (coordinator && playerEntity_ != 0) {
        coordinator->DestroyEntity(playerEntity_);
    }

    std::cout << "[PlayState] Gameplay cleanup complete" << std::endl;
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
        std::cout << "[PlayState] MovementSystem signature set (Position + Velocity)" << std::endl;
    }
    
    // ScrollingBackgroundSystem: Position + ScrollingBackground
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<ScrollingBackground>());
        coordinator->SetSystemSignature<ScrollingBackgroundSystem>(sig);
        std::cout << "[PlayState] ScrollingBackgroundSystem signature set (Position + ScrollingBackground)" << std::endl;
    }
    
    // RenderSystem: Position + Sprite
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Sprite>());
        coordinator->SetSystemSignature<RenderSystem>(sig);
        std::cout << "[PlayState] RenderSystem signature set (Position + Sprite)" << std::endl;
    }
    
    // LifetimeSystem: Lifetime
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Lifetime>());
        coordinator->SetSystemSignature<LifetimeSystem>(sig);
        std::cout << "[PlayState] LifetimeSystem signature set (Lifetime)" << std::endl;
    }
    
    // BoundarySystem: Position + Boundary
    {
        ECS::Signature sig;
        sig.set(coordinator->GetComponentType<Position>());
        sig.set(coordinator->GetComponentType<Boundary>());
        coordinator->SetSystemSignature<BoundarySystem>(sig);
        std::cout << "[PlayState] BoundarySystem signature set (Position + Boundary)" << std::endl;
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

    std::cout << "[PlayState] Systems registered and initialized" << std::endl;
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
            
            std::cout << "[PlayState] Game config loaded: window=" << windowWidth_ << "x" << windowHeight_ 
                      << ", input_speed=" << inputSystemSpeed_
                      << ", bg=" << backgroundPath_ << " (" << backgroundOriginalWidth_ << "x" << backgroundOriginalHeight_ << ")"
                      << ", scroll_speed=" << backgroundScrollSpeed_ << "px/s" << std::endl;
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
            
            std::cout << "[PlayState] Player config loaded: speed=" << playerSpeed_ 
                      << ", health=" << playerMaxHealth_ << std::endl;
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
                
                std::cout << "[PlayState] Basic shot loaded: speed=" << projectileSpeed_ 
                          << ", lifetime=" << projectileLifetime_ 
                          << ", damage=" << projectileDamage_
                          << ", fire_rate=" << shootCooldownTime_ << "s" << std::endl;
            }
            
            // Load charge timings
            sol::table chargeTimings = weaponsConfig["charge_timings"];
            if (chargeTimings.valid()) {
                chargeThresholds_.clear();
                for (size_t i = 1; i <= 6; ++i) {
                    float timing = chargeTimings[i].get_or(0.0f);
                    chargeThresholds_.push_back(timing);
                }
                std::cout << "[PlayState] Charge timings loaded: ";
                for (float t : chargeThresholds_) std::cout << t << "s ";
                std::cout << std::endl;
            }
            
            // Store weapons config for later use
            weaponsConfig_ = weaponsConfig;
            
            std::cout << "[PlayState] ✅ Weapons config fully loaded (basic + 5 charge levels + modules)" << std::endl;
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
            std::cout << "[PlayState] ✅ Collectables config loaded (powerups + modules)" << std::endl;
        } else {
            std::cerr << "[PlayState] Warning: collectables_config not found in Lua" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error loading collectables config: " << e.what() << std::endl;
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
        std::cout << "[PlayState] Setting textureRect: (" << rect->left << "," << rect->top 
                  << "," << rect->width << "," << rect->height << ")" << std::endl;
        sprite->setTextureRect(*rect);
    } else {
        std::cout << "[PlayState] No textureRect applied for: " << texturePath << std::endl;
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

    std::cout << "[PlayState] Background spawned: " << backgroundPath_ 
              << " (" << backgroundOriginalWidth_ << "x" << backgroundOriginalHeight_ 
              << " @ scale " << scaleX << "x" << scaleY << " = " << scaledWidth << " wide, speed=" 
              << backgroundScrollSpeed_ << "px/s)" << std::endl;
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
    
    std::cout << "[PlayState] Player textureRect BEFORE loadSprite: {" 
              << playerSprite.textureRect.left << ", " 
              << playerSprite.textureRect.top << ", "
              << playerSprite.textureRect.width << ", " 
              << playerSprite.textureRect.height << "}" << std::endl;
    
    playerSprite.sprite = loadSprite("assets/players/r-typesheet42.png", &playerSprite.textureRect);
    
    std::cout << "[PlayState] Player textureRect AFTER loadSprite: {" 
              << playerSprite.textureRect.left << ", " 
              << playerSprite.textureRect.top << ", "
              << playerSprite.textureRect.width << ", " 
              << playerSprite.textureRect.height << "}" << std::endl;
    
    coordinator->AddComponent<Sprite>(playerEntity_, playerSprite);

    // Collider (for collisions) - Only radius argument
    coordinator->AddComponent<Collider>(playerEntity_, Collider{
        20.0f,   // radius (approximate size of ship)
        false    // not a trigger
    });

    // Health
    coordinator->AddComponent<Health>(playerEntity_, Health{
        playerMaxHealth_,
        playerMaxHealth_
    });

    // Tag as player
    coordinator->AddComponent<Tag>(playerEntity_, Tag{"player"});

    // No Boundary component - we'll handle player boundaries manually in update()
    
    std::cout << "[PlayState] Player spawned (Entity ID: " << playerEntity_ << ")" << std::endl;
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

    std::cout << "[PlayState] Charge indicator spawned (Entity ID: " << chargeIndicatorEntity_ 
              << ") with " << frames.size() << " frames from Lua" << std::endl;
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
            std::cout << "[PlayState] ESC pressed - returning to menu" << std::endl;
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
        std::cout << "[PlayState] Shooting on cooldown (" << timeSinceLastShot_ << "s < " << shootCooldownTime_ << "s)" << std::endl;
        return;  // Still in cooldown
    }

    auto coordinator = game_->getCoordinator();
    if (!coordinator || playerEntity_ == 0) return;

    // Get player position
    if (!coordinator->HasComponent<Position>(playerEntity_)) return;
    auto& playerPos = coordinator->GetComponent<Position>(playerEntity_);

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
    
    std::cout << "[PlayState] 🔫 Shooting with chargeTime=" << chargeTime_ << "s, chargeLevel=" << chargeLevel << std::endl;
    
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
    
    std::cout << "[PlayState] 📊 Weapon config: " << weaponKey << std::endl;
    std::cout << "  - Texture: " << texturePath << std::endl;
    std::cout << "  - Rect: {" << textureRect.left << ", " << textureRect.top << ", " 
              << textureRect.width << ", " << textureRect.height << "}" << std::endl;
    std::cout << "  - Scale: " << spriteScale << "x" << std::endl;
    std::cout << "  - Damage: " << damage << ", Speed: " << speed << std::endl;
    
    // Create projectile entity
    ECS::Entity projectile = coordinator->CreateEntity();

    // Position (spawn in front of player)
    coordinator->AddComponent<Position>(projectile, Position{
        playerPos.x + 50.0f,  // Offset to the right of player
        playerPos.y
    });

    // Velocity (move right) - FROM LUA CONFIG
    coordinator->AddComponent<Velocity>(projectile, Velocity{speed, 0.0f});
    
    std::cout << "[PlayState] " << weaponKey << " fired! (damage=" << damage 
              << ", speed=" << speed << ", charge=" << chargeTime_ << "s)" << std::endl;

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
        
        std::cout << "[PlayState] 🎬 Animation added: " << frameCount << " frames @ " 
                  << frameTime << "s/frame (frameWidth=" << frameWidth << ")" << std::endl;
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

    // Tag as player projectile
    coordinator->AddComponent<Tag>(projectile, Tag{"player_projectile"});

    // Reset cooldown (chargeTime_ reset in handleEvent)
    timeSinceLastShot_ = 0.0f;
}


void PlayState::update(float deltaTime)
{
    // Update shoot cooldown
    timeSinceLastShot_ += deltaTime;

    // Check if Space is held (using engine's real-time input API)
    bool spaceCurrentlyPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Space);
    
    // Space just pressed → start charging
    if (spaceCurrentlyPressed && !isCharging_) {
        isCharging_ = true;
        chargeTime_ = 0.0f;
        std::cout << "[PlayState] ⚡ Started charging..." << std::endl;
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
}

void PlayState::render()
{
    if (renderSystem_) {
        renderSystem_->Update(0.0f);  // RenderSystem doesn't use deltaTime
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
    std::cout << "[PlayState] Spawning test collectables..." << std::endl;
    
    // Spawn power-ups (orange et bleu) - en haut de l'écran
    spawnPowerup(600.0f, 200.0f, "powerup_orange");
    spawnPowerup(800.0f, 200.0f, "powerup_blue");
    
    // Spawn modules - au milieu de l'écran
    spawnModule(600.0f, 400.0f, "laser");
    spawnModule(800.0f, 400.0f, "homing");
    spawnModule(1000.0f, 400.0f, "spread");
    spawnModule(1200.0f, 400.0f, "wave");
    
    std::cout << "[PlayState] ✅ Test collectables spawned" << std::endl;
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
        
        // Velocity depuis Lua
        sol::table velocityData = powerupData["velocity"];
        float velX = velocityData["x"].get_or(-50.0f);
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
        
        std::cout << "[PlayState] Spawned powerup: " << type << " at (" << x << ", " << y << ")" << std::endl;
        
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
        
        // Velocity depuis Lua
        sol::table velocityData = moduleData["velocity"];
        float velX = velocityData["x"].get_or(-50.0f);
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
        
        std::cout << "[PlayState] Spawned module: " << moduleType << " at (" << x << ", " << y << ")" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[PlayState] Error spawning module: " << e.what() << std::endl;
    }
}
