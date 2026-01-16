#include "Game.hpp"
#include <filesystem>
#include <fstream>
#include <core/GameStateCallbacks.hpp>

// Cache for the resolved base path
static std::string g_basePath = "";

// Helper function to resolve asset paths from different working directories
std::string ResolveAssetPath(const std::string& relativePath) {
    // If we already found the base path, use it
    if (!g_basePath.empty()) {
        return g_basePath + relativePath;
    }

    // List of possible base paths to check
    std::vector<std::string> basePaths = {
        "",                    // Current directory (running from project root)
        "../../",              // Running from build/game/
        "../../../",           // Running from deeper build directories
    };

    // Test file to check if we're in the right directory
    std::string testFile = "game/assets/fonts/Roboto-Regular.ttf";

    for (const auto& base : basePaths) {
        std::string fullPath = base + testFile;
        std::ifstream file(fullPath);
        if (file.good()) {
            g_basePath = base;
            std::cout << "[AssetPath] Base path resolved to: " << (base.empty() ? "(current dir)" : base) << std::endl;
            return g_basePath + relativePath;
        }
    }

    // Fallback: return the path as-is
    std::cerr << "[AssetPath] Warning: Could not resolve base path, using relative path as-is" << std::endl;
    return relativePath;
}

void Game::RegisterEntity(ECS::Entity entity) {
    allEntities.push_back(entity);
}

void Game::DestroyEntityDeferred(ECS::Entity entity) {
    entitiesToDestroy.push_back(entity);
}

void Game::ProcessDestroyedEntities() {
    for (auto entity : entitiesToDestroy) {
        // Clean up sprite if exists
        if (gCoordinator.HasComponent<Sprite>(entity)) {
            auto& sprite = gCoordinator.GetComponent<Sprite>(entity);
            if (sprite.sprite) {
                // Remove from allSprites before deleting to avoid double-free
                auto it = std::find(allSprites.begin(), allSprites.end(), sprite.sprite);
                if (it != allSprites.end()) {
                    allSprites.erase(it);
                }
                delete sprite.sprite;
                sprite.sprite = nullptr;
            }
        }

        gCoordinator.DestroyEntity(entity);

        // Remove from allEntities
        allEntities.erase(std::remove(allEntities.begin(), allEntities.end(), entity), allEntities.end());
    }
    entitiesToDestroy.clear();
}

// Helper function to create player entity
ECS::Entity Game::CreatePlayer(float x, float y, int line) {
    ECS::Entity player = gCoordinator.CreateEntity();
    RegisterEntity(player);

    // Position
    gCoordinator.AddComponent(player, Position{x, y});

    // Velocity (controlled by input)
    gCoordinator.AddComponent(player, Velocity{0.0f, 0.0f});

    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(playerTexture.get());
    IntRect rect(33 * 2, line * 17, 33, 17);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 10;
    gCoordinator.AddComponent(player, spriteComp);

    // State machine animation
    StateMachineAnimation anim;
    anim.currentColumn = 2;
    anim.targetColumn = 2;
    anim.transitionSpeed = 0.15f;
    anim.spriteWidth = 33;
    anim.spriteHeight = 17;
    anim.currentRow = line;
    gCoordinator.AddComponent(player, anim);

    // Collider
    Collider collider;
    collider.width = 33 * 3.0f;
    collider.height = 17 * 3.0f;
    collider.tag = "player";
    gCoordinator.AddComponent(player, collider);

    // Health
    Health health;
    health.current = 100;
    health.max = 100;
    gCoordinator.AddComponent(player, health);

    // Damage (player deals damage on contact - destroys enemies)
    Damage damage;
    damage.amount = 100;  // High damage to kill enemies on contact
    damage.damageType = "contact";
    gCoordinator.AddComponent(player, damage);

    // Weapon
    ShootEmUp::Components::Weapon weapon;
    weapon.fireRate = 0.2f;
    weapon.supportsCharge = true;
    weapon.minChargeTime = 0.1f;
    weapon.maxChargeTime = 1.0f;
    weapon.projectileSpeed = 1000.0f;
    weapon.shootSound = "shoot";
    gCoordinator.AddComponent(player, weapon);

    // Tags
    gCoordinator.AddComponent(player, Tag{"player"});
    gCoordinator.AddComponent(player, ShootEmUp::Components::PlayerTag{0});

    return player;
}

// Helper function to create background entity
ECS::Entity Game::CreateBackground(float x, float y, float windowHeight, bool isFirst) {
    ECS::Entity bg = gCoordinator.CreateEntity();
    RegisterEntity(bg);

    // Position
    gCoordinator.AddComponent(bg, Position{x, y});

    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(backgroundTexture.get());
    sprite->setPosition(Vector2f(x, y));

    // Calculate scale to fit window height
    Vector2u texSize = backgroundTexture->getSize();
    float scale = windowHeight / texSize.y;

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.layer = -10;
    spriteComp.scaleX = scale;
    spriteComp.scaleY = scale;
    gCoordinator.AddComponent(bg, spriteComp);

    // Scrolling background
    ScrollingBackground scrolling;
    scrolling.scrollSpeed = 200.0f;
    scrolling.horizontal = true;
    scrolling.loop = true;
    scrolling.spriteWidth = texSize.x * scale;

    if (isFirst) {
        scrolling.sprite1X = 0.0f;
        scrolling.sprite2X = scrolling.spriteWidth;
    } else {
        scrolling.sprite1X = scrolling.spriteWidth;
        scrolling.sprite2X = 0.0f;
    }

    gCoordinator.AddComponent(bg, scrolling);
    gCoordinator.AddComponent(bg, Tag{"background"});

    return bg;
}

// Helper function to create enemy entity
ECS::Entity Game::CreateEnemy(float x, float y, std::string patternType) {
    ECS::Entity enemy = gCoordinator.CreateEntity();
    RegisterEntity(enemy);

    // Position
    gCoordinator.AddComponent(enemy, Position{x, y});

    // Velocity
    gCoordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(textureMap["enemy"]);
    IntRect rect(0, 0, 33, 32);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;  // Scale pour les ennemis
    spriteComp.scaleY = 2.5f;
    gCoordinator.AddComponent(enemy, spriteComp);

    // Animation
    Animation anim;
    anim.frameTime = 0.1f;
    anim.currentFrame = 0;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;  // Frames are adjacent (no gap)
    gCoordinator.AddComponent(enemy, anim);

    // Movement pattern
    ShootEmUp::Components::MovementPattern movementPattern;
    movementPattern.patternType = patternType;
    movementPattern.speed = 200.0f + (rand() % 200);
    movementPattern.amplitude = 50.0f + (rand() % 100);
    movementPattern.frequency = 1.0f + (rand() % 3);
    movementPattern.startX = x;
    movementPattern.startY = y;
    gCoordinator.AddComponent(enemy, movementPattern);

    // Collider
    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    gCoordinator.AddComponent(enemy, collider);

    // Health
    Health health;
    health.current = 1;
    health.max = 1;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    gCoordinator.AddComponent(enemy, health);

    // Damage (enemies deal damage on contact)
    Damage damage;
    damage.amount = 1;
    damage.damageType = "contact";
    gCoordinator.AddComponent(enemy, damage);

    // Tags
    gCoordinator.AddComponent(enemy, Tag{"enemy"});
    ShootEmUp::Components::EnemyTag enemyTag;
    enemyTag.enemyType = "basic";
    enemyTag.scoreValue = 100;
    enemyTag.aiAggressiveness = 1.0f;
    gCoordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Helper function to create missile entity
ECS::Entity Game::CreateMissile(float x, float y, bool isCharged, int chargeLevel) {
    ECS::Entity missile = gCoordinator.CreateEntity();
    RegisterEntity(missile);

    // Position
    gCoordinator.AddComponent(missile, Position{x, y});

    // Velocity
    float speed = isCharged ? 1500.0f : 1000.0f;
    gCoordinator.AddComponent(missile, Velocity{speed, 0.0f});

    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(missileTexture.get());

    IntRect rect;
    if (!isCharged) {
        // Missiles normaux - coordonnées de l'ancien fichier qui fonctionnait
        rect = IntRect(245, 85, 20, 20);
    } else {
        // Charged missile sprites (lignes 5-9 avec les gros missiles)
        struct ChargeData {
            int xPos, yPos, width, height;
        };
        ChargeData chargeLevels[5] = {
            {233, 100, 15, 15},  // Level 1
            {202, 117, 31, 15},  // Level 2
            {170, 135, 47, 15},  // Level 3
            {138, 155, 63, 15},  // Level 4
            {105, 170, 79, 17}   // Level 5
        };
        ChargeData& data = chargeLevels[chargeLevel - 1];
        rect = IntRect(data.xPos, data.yPos, data.width, data.height);
    }

    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 8;
    spriteComp.scaleX = 3.0f;  // Scale pour les missiles
    spriteComp.scaleY = 3.0f;
    gCoordinator.AddComponent(missile, spriteComp);

    // Animation seulement pour les missiles chargés
    if (isCharged) {
        Animation anim;
        anim.frameTime = 0.1f;
        anim.currentFrame = 0;
        anim.frameCount = 2;
        anim.loop = true;
        anim.frameWidth = rect.width;
        anim.frameHeight = rect.height;
        anim.startX = rect.left;
        anim.startY = rect.top;
        anim.spacing = rect.width + 2;
        gCoordinator.AddComponent(missile, anim);
    }

    // Collider
    Collider collider;
    collider.width = rect.width * 3.0f;
    collider.height = rect.height * 3.0f;
    collider.tag = isCharged ? "charged_bullet" : "bullet";
    gCoordinator.AddComponent(missile, collider);

    // Damage
    Damage damage;
    damage.amount = isCharged ? chargeLevel : 1;
    damage.damageType = isCharged ? "charged" : "normal";
    gCoordinator.AddComponent(missile, damage);

    // Tags
    gCoordinator.AddComponent(missile, Tag{isCharged ? "charged_bullet" : "bullet"});
    ShootEmUp::Components::ProjectileTag projTag;
    projTag.projectileType = isCharged ? "charged" : "normal";
    projTag.ownerId = 0;
    projTag.isPlayerProjectile = true;
    projTag.chargeLevel = isCharged ? chargeLevel : 0;
    gCoordinator.AddComponent(missile, projTag);

    // Lifetime (destroy after 5 seconds or when off-screen)
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    gCoordinator.AddComponent(missile, lifetime);

    return missile;
}

// Helper function to create explosion effect
ECS::Entity Game::CreateExplosion(float x, float y) {
    std::cout << "[CreateExplosion] Starting at (" << x << ", " << y << ")" << std::endl;

    // Verify explosion texture is loaded
    if (!explosionTexture || explosionTexture->getSize().x == 0) {
        std::cerr << "[Game] Cannot create explosion: texture not loaded" << std::endl;
        return 0;
    }

    std::cout << "[CreateExplosion] Creating entity..." << std::endl;
    ECS::Entity explosion = gCoordinator.CreateEntity();
    RegisterEntity(explosion);
    std::cout << "[CreateExplosion] Entity " << explosion << " created" << std::endl;

    // Position
    std::cout << "[CreateExplosion] Adding Position component..." << std::endl;
    gCoordinator.AddComponent(explosion, Position{x, y});

    // Sprite
    std::cout << "[CreateExplosion] Creating sprite..." << std::endl;
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(explosionTexture.get());
    IntRect rect(129, 0, 34, 35);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    std::cout << "[CreateExplosion] Adding Sprite component..." << std::endl;
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 15;
    spriteComp.scaleX = 2.5f;  // Scale up the explosion
    spriteComp.scaleY = 2.5f;
    gCoordinator.AddComponent(explosion, spriteComp);

    // Animation
    std::cout << "[CreateExplosion] Adding Animation component..." << std::endl;
    Animation anim;
    anim.frameTime = 0.1f; // Slower animation for visible effect
    anim.currentFrame = 0;
    anim.frameCount = 6;
    anim.loop = false;
    anim.frameWidth = 34;
    anim.frameHeight = 35;
    anim.startX = 124;
    anim.startY = 0;
    anim.spacing = 0;  // Spacing between explosion frames
    gCoordinator.AddComponent(explosion, anim);

    // Lifetime (destroy after animation finishes)
    std::cout << "[CreateExplosion] Adding Lifetime component..." << std::endl;
    Lifetime lifetime;
    lifetime.maxLifetime = 1.0f; // 1 second before disappearing
    gCoordinator.AddComponent(explosion, lifetime);

    // Tag for identification
    std::cout << "[CreateExplosion] Adding Tag component..." << std::endl;
    gCoordinator.AddComponent(explosion, Tag{"explosion"});

    std::cout << "[CreateExplosion] Explosion " << explosion << " created successfully!" << std::endl;
    return explosion;
}

// Helper function to create shoot effect
ECS::Entity Game::CreateShootEffect(float x, float y, ECS::Entity parent) {
    ECS::Entity effect = gCoordinator.CreateEntity();
    RegisterEntity(effect);

    // Position
    gCoordinator.AddComponent(effect, Position{x, y});

    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(missileTexture.get());
    IntRect rect(212, 80, 16, 16);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 12;
    gCoordinator.AddComponent(effect, spriteComp);

    // Animation (just 2 frames)
    Animation anim;
    anim.frameTime = 0.05f;
    anim.currentFrame = 0;
    anim.frameCount = 2;
    anim.loop = false;
    anim.frameWidth = 16;
    anim.frameHeight = 16;
    anim.startX = 212;
    anim.startY = 80;
    anim.spacing = 16;
    gCoordinator.AddComponent(effect, anim);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 0.1f;
    gCoordinator.AddComponent(effect, lifetime);

    // Effect tag
    ShootEmUp::Components::Effect effectTag;
    effectTag.effectType = "shoot";
    effectTag.followParent = true;
    gCoordinator.AddComponent(effect, effectTag);

    gCoordinator.AddComponent(effect, Tag{"effect"});

    return effect;
}

// Helper function to create enemy missile entity
ECS::Entity Game::CreateEnemyMissile(float x, float y) {
    ECS::Entity missile = gCoordinator.CreateEntity();
    RegisterEntity(missile);

    // Position
    gCoordinator.AddComponent(missile, Position{x, y});

    // Velocity (moves left towards player)
    float speed = -400.0f;  // Negative = moving left
    gCoordinator.AddComponent(missile, Velocity{speed, 0.0f});

    // Sprite - Use enemy_bullets.png with the orange ball (first row)
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    
    // First row orange balls: starts around x=166, y=3, each frame ~12x12 pixels, spacing ~17px
    IntRect rect(166, 3, 12, 12);  // First frame of orange ball
    sprite->setTexture(enemyBulletTexture.get());
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));

    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 8;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    gCoordinator.AddComponent(missile, spriteComp);

    // Animation - 4 frames looping
    Animation anim;
    anim.frameTime = 0.1f;
    anim.currentFrame = 0;
    anim.frameCount = 4;
    anim.loop = true;
    anim.frameWidth = 12;
    anim.frameHeight = 12;
    anim.startX = 166;
    anim.startY = 3;
    anim.spacing = 5;  // Spacing between frames (17 - 12 = 5)
    gCoordinator.AddComponent(missile, anim);

    // Collider
    Collider collider;
    collider.width = 12 * 2.5f;
    collider.height = 12 * 2.5f;
    collider.tag = "enemy_bullet";
    gCoordinator.AddComponent(missile, collider);

    // Damage
    Damage damage;
    damage.amount = 1;
    damage.damageType = "enemy";
    gCoordinator.AddComponent(missile, damage);

    // Tags
    gCoordinator.AddComponent(missile, Tag{"enemy_bullet"});
    ShootEmUp::Components::ProjectileTag projTag;
    projTag.projectileType = "enemy";
    projTag.ownerId = 0;
    projTag.isPlayerProjectile = false;
    gCoordinator.AddComponent(missile, projTag);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    gCoordinator.AddComponent(missile, lifetime);

    return missile;
}

int Game::Run(int argc, char* argv[])
{
    std::cout << "R-Type Game Starting with ECS Engine (Refactored)..." << std::endl;

    // Parse command line arguments
    bool networkMode = false;
    std::string serverAddress = "127.0.0.1";
    short serverPort = 12345;

    if (argc > 1 && std::string(argv[1]) == "--network") {
        networkMode = true;
        isNetworkClient = true;  // Mark as network client
        if (argc > 2) {
            serverAddress = argv[2];
        }
        if (argc > 3) {
            serverPort = static_cast<short>(std::stoi(argv[3]));
        }
        std::cout << "[Game] Network mode enabled. Server: " << serverAddress << ":" << serverPort << std::endl;
        std::cout << "[Game] *** isNetworkClient = " << (isNetworkClient ? "TRUE" : "FALSE") << " ***" << std::endl;
    } else {
        std::cout << "[Game] Local mode (use --network <ip> <port> for multiplayer)" << std::endl;
        std::cout << "[Game] *** isNetworkClient = " << (isNetworkClient ? "TRUE" : "FALSE") << " ***" << std::endl;
    }

    // Initialize ECS Coordinator
    gCoordinator.Init();

    // Register all components
    gCoordinator.RegisterComponent<Position>();
    gCoordinator.RegisterComponent<Velocity>();
    gCoordinator.RegisterComponent<Sprite>();
    gCoordinator.RegisterComponent<Animation>();
    gCoordinator.RegisterComponent<StateMachineAnimation>();
    gCoordinator.RegisterComponent<Collider>();
    gCoordinator.RegisterComponent<Health>();
    gCoordinator.RegisterComponent<Boundary>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::Weapon>();
    gCoordinator.RegisterComponent<Tag>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::PlayerTag>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::EnemyTag>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::ProjectileTag>();
    gCoordinator.RegisterComponent<ScrollingBackground>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::MovementPattern>();
    gCoordinator.RegisterComponent<Lifetime>();
    gCoordinator.RegisterComponent<ShootEmUp::Components::Effect>();
    gCoordinator.RegisterComponent<Damage>();
    gCoordinator.RegisterComponent<ChargeAnimation>();
    gCoordinator.RegisterComponent<NetworkId>();

    // Register UI components
    gCoordinator.RegisterComponent<Components::UIElement>();
    gCoordinator.RegisterComponent<Components::UIText>();
    gCoordinator.RegisterComponent<Components::UIButton>();
    gCoordinator.RegisterComponent<Components::UISlider>();
    gCoordinator.RegisterComponent<Components::UIInputField>();
    gCoordinator.RegisterComponent<Components::UIPanel>();
    gCoordinator.RegisterComponent<Components::UICheckbox>();
    gCoordinator.RegisterComponent<Components::UIDropdown>();

    std::cout << "[Game] Components registered" << std::endl;

    // ========================================
    // INITIALIZE LUA SCRIPTING
    // ========================================
    std::cout << "🌙 Initializing Lua Scripting..." << std::endl;

    auto& luaState = Scripting::LuaState::Instance();
    luaState.Init();
    luaState.EnableHotReload(true);

    // Register all components to Lua
    Scripting::ComponentBindings::RegisterAll(luaState.GetState());
    Scripting::ComponentBindings::RegisterCoordinator(luaState.GetState(), &gCoordinator);

    std::cout << "[Game] Lua components registered" << std::endl;

    // ========================================
    // INITIALIZE GAME STATE MANAGER
    // ========================================
    std::cout << "🎮 Initializing Game State Manager..." << std::endl;

    auto& gameStateManager = GameStateManager::Instance();
    gameStateManager.SetState(GameState::MainMenu);  // Start at main menu

    std::cout << "[Game] Game State Manager initialized" << std::endl;

    // ========================================
    // INITIALIZE ALL SYSTEMS
    // ========================================
    std::cout << "🔧 Initializing Systems..." << std::endl;

    // Systems will be created by the Coordinator when registered
    std::shared_ptr<MovementSystem> movementSystem = nullptr;
    std::shared_ptr<AnimationSystem> animationSystem = nullptr;
    std::shared_ptr<StateMachineAnimationSystem> stateMachineAnimSystem = nullptr;
    std::shared_ptr<LifetimeSystem> lifetimeSystem = nullptr;
    std::shared_ptr<MovementPatternSystem> movementPatternSystem = nullptr;
    std::shared_ptr<ScrollingBackgroundSystem> scrollingBgSystem = nullptr;
    std::shared_ptr<BoundarySystem> boundarySystem = nullptr;
    std::shared_ptr<CollisionSystem> collisionSystem = nullptr;  // Generic engine CollisionSystem
    std::shared_ptr<HealthSystem> healthSystem = nullptr;
    std::shared_ptr<RenderSystem> renderSystem = nullptr;

    // Register systems with ECS and set signatures
    // Register and get all system instances from Coordinator
    movementSystem = gCoordinator.RegisterSystem<MovementSystem>(&gCoordinator);
    ECS::Signature movementSig;
    movementSig.set(gCoordinator.GetComponentType<Position>());
    movementSig.set(gCoordinator.GetComponentType<Velocity>());
    gCoordinator.SetSystemSignature<MovementSystem>(movementSig);

    animationSystem = gCoordinator.RegisterSystem<AnimationSystem>();
    animationSystem->SetCoordinator(&gCoordinator);
    ECS::Signature animSig;
    animSig.set(gCoordinator.GetComponentType<Animation>());
    animSig.set(gCoordinator.GetComponentType<Sprite>());
    gCoordinator.SetSystemSignature<AnimationSystem>(animSig);

    stateMachineAnimSystem = gCoordinator.RegisterSystem<StateMachineAnimationSystem>(&gCoordinator);
    ECS::Signature stateMachineSig;
    stateMachineSig.set(gCoordinator.GetComponentType<StateMachineAnimation>());
    stateMachineSig.set(gCoordinator.GetComponentType<Sprite>());
    gCoordinator.SetSystemSignature<StateMachineAnimationSystem>(stateMachineSig);

    lifetimeSystem = gCoordinator.RegisterSystem<LifetimeSystem>(&gCoordinator);
    ECS::Signature lifetimeSig;
    lifetimeSig.set(gCoordinator.GetComponentType<Lifetime>());
    gCoordinator.SetSystemSignature<LifetimeSystem>(lifetimeSig);

    movementPatternSystem = gCoordinator.RegisterSystem<MovementPatternSystem>();
    movementPatternSystem->SetCoordinator(&gCoordinator);
    ECS::Signature patternSig;
    patternSig.set(gCoordinator.GetComponentType<ShootEmUp::Components::MovementPattern>());
    patternSig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<MovementPatternSystem>(patternSig);

    scrollingBgSystem = gCoordinator.RegisterSystem<ScrollingBackgroundSystem>();
    scrollingBgSystem->SetCoordinator(&gCoordinator);
    ECS::Signature scrollingSig;
    scrollingSig.set(gCoordinator.GetComponentType<ScrollingBackground>());
    scrollingSig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<ScrollingBackgroundSystem>(scrollingSig);

    boundarySystem = gCoordinator.RegisterSystem<BoundarySystem>();
    boundarySystem->SetCoordinator(&gCoordinator);
    boundarySystem->SetWindowSize(1920.0f, 1080.0f);
    ECS::Signature boundarySig;
    boundarySig.set(gCoordinator.GetComponentType<Position>());
    boundarySig.set(gCoordinator.GetComponentType<Boundary>());
    gCoordinator.SetSystemSignature<BoundarySystem>(boundarySig);

    collisionSystem = gCoordinator.RegisterSystem<CollisionSystem>(&gCoordinator);
    ECS::Signature collisionSig;
    collisionSig.set(gCoordinator.GetComponentType<Position>());
    collisionSig.set(gCoordinator.GetComponentType<Collider>());
    gCoordinator.SetSystemSignature<CollisionSystem>(collisionSig);

    healthSystem = gCoordinator.RegisterSystem<HealthSystem>();
    healthSystem->SetCoordinator(&gCoordinator);
    ECS::Signature healthSig;
    healthSig.set(gCoordinator.GetComponentType<Health>());
    gCoordinator.SetSystemSignature<HealthSystem>(healthSig);

    renderSystem = gCoordinator.RegisterSystem<RenderSystem>();
    renderSystem->SetCoordinator(&gCoordinator);
    ECS::Signature renderSig;
    renderSig.set(gCoordinator.GetComponentType<Position>());
    renderSig.set(gCoordinator.GetComponentType<Sprite>());
    gCoordinator.SetSystemSignature<RenderSystem>(renderSig);

    // UI System
    uiSystem = gCoordinator.RegisterSystem<UISystem>();
    uiSystem->SetCoordinator(&gCoordinator);
    ECS::Signature uiSig;
    uiSig.set(gCoordinator.GetComponentType<Components::UIElement>());
    gCoordinator.SetSystemSignature<UISystem>(uiSig);

    // Initialize all systems
    movementSystem->Init();
    animationSystem->Init();
    stateMachineAnimSystem->Init();
    lifetimeSystem->Init();
    movementPatternSystem->Init();
    scrollingBgSystem->Init();
    boundarySystem->Init();
    collisionSystem->Init();
    healthSystem->Init();
    renderSystem->Init();
    uiSystem->Init();

    // Load default font for UI
    if (!uiSystem->LoadFont("default", ResolveAssetPath("game/assets/fonts/Roboto-Regular.ttf"))) {
        std::cerr << "Warning: Could not load default UI font" << std::endl;
    } else {
        std::cout << "[Game] Default UI font loaded" << std::endl;
    }

    // Setup collision callback after collision system is created
    collisionSystem->SetCollisionCallback([this](ECS::Entity a, ECS::Entity b) {
        // Check if entities still exist (might have been destroyed)
        bool aExists = std::find(allEntities.begin(), allEntities.end(), a) != allEntities.end();
        bool bExists = std::find(allEntities.begin(), allEntities.end(), b) != allEntities.end();
        
        if (!aExists || !bExists) return;  // Skip if either entity is already destroyed
        
        // Check if either entity is already marked for destruction
        bool aMarkedForDestruction = std::find(entitiesToDestroy.begin(), entitiesToDestroy.end(), a) != entitiesToDestroy.end();
        bool bMarkedForDestruction = std::find(entitiesToDestroy.begin(), entitiesToDestroy.end(), b) != entitiesToDestroy.end();
        
        if (aMarkedForDestruction || bMarkedForDestruction) return;  // Skip duplicate collisions

        // Check if player is involved and currently invincible
        bool aIsPlayer = gCoordinator.HasComponent<ShootEmUp::Components::PlayerTag>(a);
        bool bIsPlayer = gCoordinator.HasComponent<ShootEmUp::Components::PlayerTag>(b);
        
        if (aIsPlayer && gCoordinator.HasComponent<Health>(a)) {
            auto& health = gCoordinator.GetComponent<Health>(a);
            if (health.invincibilityTimer > 0.0f) return;  // Player is invincible, skip collision
        }
        if (bIsPlayer && gCoordinator.HasComponent<Health>(b)) {
            auto& health = gCoordinator.GetComponent<Health>(b);
            if (health.invincibilityTimer > 0.0f) return;  // Player is invincible, skip collision
        }

        // Check if both entities are projectiles - ignore projectile vs projectile collisions
        bool aIsProjectile = gCoordinator.HasComponent<ShootEmUp::Components::ProjectileTag>(a);
        bool bIsProjectile = gCoordinator.HasComponent<ShootEmUp::Components::ProjectileTag>(b);
        if (aIsProjectile && bIsProjectile) {
            return;  // Projectiles don't collide with each other
        }

        // Check for damage/health components. Only log collisions that are gameplay-relevant:
        // - a has Damage and b has Health
        // - b has Damage and a has Health
        // - OR either entity is the player (we want to always surface player hits)
    bool aHasDamage = gCoordinator.HasComponent<Damage>(a);
    bool bHasDamage = gCoordinator.HasComponent<Damage>(b);
    bool aHasHealth = gCoordinator.HasComponent<Health>(a);
    bool bHasHealth = gCoordinator.HasComponent<Health>(b);

        bool significant = (aHasDamage && bHasHealth) || (bHasDamage && aHasHealth) || aIsPlayer || bIsPlayer;
        if (!significant) {
            // Not relevant for damage flow/logging: skip verbose output
            return;
        }

        std::cout << "[Collision] Entity " << a << " <-> Entity " << b << std::endl;

        // Apply damage: B damages A
        if (aHasHealth && bHasDamage) {
            auto& health = gCoordinator.GetComponent<Health>(a);
            auto& damage = gCoordinator.GetComponent<Damage>(b);
            if (!health.invulnerable) {
                health.current -= damage.amount;
                std::cout << "[Damage] Entity " << a << " took " << damage.amount << " damage, health: " << health.current << std::endl;
            }
        }
        
        // Apply damage: A damages B
        if (bHasHealth && aHasDamage) {
            auto& health = gCoordinator.GetComponent<Health>(b);
            auto& damage = gCoordinator.GetComponent<Damage>(a);
            if (!health.invulnerable) {
                health.current -= damage.amount;
                std::cout << "[Damage] Entity " << b << " took " << damage.amount << " damage, health: " << health.current << std::endl;
            }
        }

        // Determine what died (health <= 0 after damage)
        bool aDied = false;
        bool bDied = false;
        bool playerWasHit = false;
        ECS::Entity playerEntity = 0;
        
        // Check deaths and player hit
        if (aHasHealth) {
            auto& health = gCoordinator.GetComponent<Health>(a);
            if (health.current <= 0 && health.destroyOnDeath) {
                aDied = true;
                DestroyEntityDeferred(a);
            } else if (aIsPlayer && health.current > 0 && health.current < health.max) {
                // Player was hit but not dead - activate invincibility
                playerWasHit = true;
                playerEntity = a;
                health.invincibilityTimer = health.invincibilityDuration;
                health.isFlashing = true;
                health.flashTimer = 0.0f;
                std::cout << "[Player] Hit! Health: " << health.current << "/" << health.max << " - Invincible for " << health.invincibilityDuration << "s" << std::endl;
            }
        }
        if (bHasHealth) {
            auto& health = gCoordinator.GetComponent<Health>(b);
            if (health.current <= 0 && health.destroyOnDeath) {
                bDied = true;
                DestroyEntityDeferred(b);
            } else if (bIsPlayer && health.current > 0 && health.current < health.max) {
                // Player was hit but not dead - activate invincibility
                playerWasHit = true;
                playerEntity = b;
                health.invincibilityTimer = health.invincibilityDuration;
                health.isFlashing = true;
                health.flashTimer = 0.0f;
                std::cout << "[Player] Hit! Health: " << health.current << "/" << health.max << " - Invincible for " << health.invincibilityDuration << "s" << std::endl;
            }
        }

        // Create explosion ONLY when something dies (not on every collision frame)
        if (!isNetworkClient && (aDied || bDied)) {
            // Create explosion at the center of the entity that died
            ECS::Entity deadEntity = bDied ? b : a;
            
            if (gCoordinator.HasComponent<Position>(deadEntity)) {
                auto& pos = gCoordinator.GetComponent<Position>(deadEntity);
                float centerX = pos.x;
                float centerY = pos.y;

                // Calculate sprite center (not collider center)
                if (gCoordinator.HasComponent<Sprite>(deadEntity)) {
                    auto& sprite = gCoordinator.GetComponent<Sprite>(deadEntity);
                    // Get actual sprite dimensions with scale
                    float spriteWidth = sprite.textureRect.width * sprite.scaleX;
                    float spriteHeight = sprite.textureRect.height * sprite.scaleY;
                    centerX += spriteWidth / 2.0f;
                    centerY += spriteHeight / 2.0f;
                }

                // Offset explosion to be centered (explosion is 34x35 scaled by 2.5)
                float explosionHalfWidth = (34 * 2.5f) / 2.0f;
                float explosionHalfHeight = (35 * 2.5f) / 2.0f;
                centerX -= explosionHalfWidth;
                centerY -= explosionHalfHeight;

                CreateExplosion(centerX, centerY);
            }
        }

        // Destroy projectiles on collision (they always get destroyed)
        if (gCoordinator.HasComponent<ShootEmUp::Components::ProjectileTag>(a)) {
            DestroyEntityDeferred(a);
        }
        if (gCoordinator.HasComponent<ShootEmUp::Components::ProjectileTag>(b)) {
            DestroyEntityDeferred(b);
        }
    });

    std::cout << "[Game] All Systems initialized!" << std::endl;

    // Network setup
    std::shared_ptr<NetworkClient> networkClient;
    std::shared_ptr<eng::engine::systems::NetworkSystem> networkSystem;

    if (networkMode) {
        try {
            networkClient = std::make_shared<NetworkClient>(serverAddress, serverPort);
            networkSystem = std::make_shared<eng::engine::systems::NetworkSystem>(&gCoordinator, networkClient);

            // Set callback to register new entities and add sprites
            networkSystem->setEntityCreatedCallback([this](ECS::Entity entity) {
                allEntities.push_back(entity);
                
                // Only add sprite if it doesn't already have one (network entities only)
                if (gCoordinator.HasComponent<Sprite>(entity)) {
                    std::cout << "[Game] Entity " << entity << " already has sprite, skipping" << std::endl;
                    return;
                }
                
                // Add sprite based on entity tag
                if (!gCoordinator.HasComponent<Tag>(entity)) {
                    std::cout << "[Game] ⚠️  Network entity " << entity << " has NO Tag component!" << std::endl;
                    return;
                }
                
                auto& tag = gCoordinator.GetComponent<Tag>(entity);
                std::cout << "[Game] 🎨 Creating sprite for network entity " << entity << " (Tag: " << tag.name << ")" << std::endl;
                    
                if (tag.name == "Player" && gCoordinator.HasComponent<NetworkId>(entity)) {
                    // Create player sprite
                    auto& netId = gCoordinator.GetComponent<NetworkId>(entity);
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    sprite->setTexture(playerTexture.get());
                    IntRect rect(33 * 2, netId.playerLine * 17, 33, 17);
                    sprite->setTextureRect(rect);
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.textureRect = rect;
                    gCoordinator.AddComponent(entity, spriteComp);
                    std::cout << "[Game] Created player sprite for entity " << entity << " (line " << (int)netId.playerLine << ")" << std::endl;
                }
                else if (tag.name == "Enemy") {
                    // Create enemy sprite
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    sprite->setTexture(textureMap["enemy"]);
                    IntRect rect(0, 0, 33, 36);
                    sprite->setTextureRect(rect);
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.textureRect = rect;
                    spriteComp.scaleX = 2.5f;
                    spriteComp.scaleY = 2.5f;
                    gCoordinator.AddComponent(entity, spriteComp);
                    
                    // Add animation for enemy (8 frames horizontally)
                    Animation anim;
                    anim.frameCount = 8;
                    anim.currentFrame = 0;
                    anim.frameTime = 0.1f;
                    anim.currentTime = 0.0f;
                    anim.loop = true;
                    anim.frameWidth = 33;
                    anim.frameHeight = 32;
                    anim.startX = 0;  // Start at frame 0 - les 8 premières frames
                    anim.startY = 0;
                    anim.spacing = 0;  // Frames are adjacent (no gap between them)
                    gCoordinator.AddComponent(entity, anim);
                    
                    std::cout << "[Game] Created enemy sprite for entity " << entity << " with animation" << std::endl;
                }
                else if (tag.name == "PlayerBullet") {
                    // Create player missile sprite
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    sprite->setTexture(missileTexture.get());
                    IntRect rect(245, 85, 20, 20);  // Correct rect from CreateMissile!
                    sprite->setTextureRect(rect);
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.textureRect = rect;
                    spriteComp.scaleX = 3.0f;  // Same as local mode
                    spriteComp.scaleY = 3.0f;
                    gCoordinator.AddComponent(entity, spriteComp);
                    std::cout << "[Game] Created player bullet sprite for entity " << entity << std::endl;
                }
                else if (tag.name == "EnemyBullet") {
                    // Create enemy bullet sprite - orange balls with animation
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    sprite->setTexture(enemyBulletTexture.get());
                    IntRect rect(135, 0, 17, 17);  // First frame position (17x17 frames)
                    sprite->setTextureRect(rect);
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.textureRect = rect;
                    spriteComp.scaleX = 4.0f;  // Scale for visibility
                    spriteComp.scaleY = 4.0f;
                    gCoordinator.AddComponent(entity, spriteComp);
                    
                    // Add animation - 4 frames of orange balls (17x17 each)
                    Animation anim;
                    anim.frameTime = 0.1f;
                    anim.currentFrame = 0;
                    anim.frameCount = 4;
                    anim.loop = true;
                    anim.frameWidth = 17;   // 17 pixels per frame (includes 1px padding)
                    anim.frameHeight = 17;
                    anim.startX = 135;
                    anim.startY = 0;
                    anim.spacing = 0;
                    gCoordinator.AddComponent(entity, anim);
                    
                    std::cout << "[Game] Created enemy bullet sprite for entity " << entity << " with animation (17x17)" << std::endl;
                }
                else if (tag.name == "Explosion") {
                    // Create explosion sprite with animation
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    sprite->setTexture(explosionTexture.get());
                    IntRect rect(130, 1, 33, 32);  // Match CreateExplosion!
                    sprite->setTextureRect(rect);
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.textureRect = rect;
                    spriteComp.scaleX = 2.5f;  // Match CreateExplosion scale
                    spriteComp.scaleY = 2.5f;
                    gCoordinator.AddComponent(entity, spriteComp);
                    
                    // Add animation - Match CreateExplosion parameters
                    Animation anim;
                    anim.frameCount = 6;
                    anim.frameTime = 0.08f;
                    anim.currentFrame = 0;
                    anim.loop = false;
                    anim.frameWidth = 32;
                    anim.frameHeight = 32;
                    anim.startX = 130;
                    anim.startY = 1;
                    anim.spacing = 1.5;
                    gCoordinator.AddComponent(entity, anim);
                    
                    // Add lifetime - very short
                    gCoordinator.AddComponent(entity, Lifetime{0.05f});
                    
                    std::cout << "[Game] Created explosion sprite for entity " << entity << std::endl;
                }
                else {
                    std::cout << "[Game] ⚠️  Unknown tag '" << tag.name << "' for entity " << entity << ", no sprite created" << std::endl;
                }
            });

            // Set callback for entity destruction
            // NOTE: We do NOT create explosions here on the client side!
            // The server creates explosion entities and sends them to clients.
            networkSystem->setEntityDestroyedCallback([this](ECS::Entity entity, uint32_t networkId) {
                std::cout << "[Game] Network entity " << entity << " (ID: " << networkId << ") destroyed by server" << std::endl;
                // Just log, don't create explosions - server handles that
            });

            networkClient->start();
            networkClient->sendHello();

            std::cout << "[Game] Network client started, waiting for SERVER_WELCOME..." << std::endl;

            // Wait for server welcome (with timeout)
            auto startTime = std::chrono::steady_clock::now();
            bool connected = false;
            while (!connected) {
                networkClient->process();
                if (networkClient->hasReceivedPackets()) {
                    auto packet = networkClient->getNextReceivedPacket();
                    if (static_cast<GamePacketType>(packet.header.type) == GamePacketType::SERVER_WELCOME) {
                        if (packet.payload.size() >= 1) {
                            uint8_t playerId = static_cast<uint8_t>(packet.payload[0]);
                            networkSystem->setLocalPlayerId(playerId);
                            std::cout << "[Game] Connected! Player ID: " << (int)playerId << std::endl;
                            connected = true;
                        }
                    }
                }

                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count() > 5) {
                    std::cerr << "[Game] Connection timeout!" << std::endl;
                    return 1;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } catch (const std::exception& e) {
            std::cerr << "[Game] Network error: " << e.what() << std::endl;
            return 1;
        }
    }

    // Create window and renderer
    SFMLWindow window;
    window.create(1920, 1080, "R-Type - ECS Version");

    SFMLRenderer renderer(&window.getSFMLWindow());

    // Set renderer for RenderSystem
    renderSystem->SetRenderer(&renderer);

    // Set window for UISystem
    uiSystem->SetWindow(&window);

    // Load textures using resolved asset paths
    backgroundTexture = std::make_unique<SFMLTexture>();
    if (!backgroundTexture->loadFromFile(ResolveAssetPath("game/assets/background.png"))) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        return 1;
    }

    playerTexture = std::make_unique<SFMLTexture>();
    if (!playerTexture->loadFromFile(ResolveAssetPath("game/assets/players/r-typesheet42.png"))) {
        std::cerr << "Error: Could not load player sprite" << std::endl;
        return 1;
    }

    missileTexture = std::make_unique<SFMLTexture>();
    if (!missileTexture->loadFromFile(ResolveAssetPath("game/assets/players/r-typesheet1.png"))) {
        std::cerr << "Error: Could not load missile sprite" << std::endl;
        return 1;
    }

    // Note: enemy textures are loaded per-enemy from Lua config later
    // Load enemy bullet texture (separate from enemy sprites)
    enemyBulletTexture = std::make_unique<SFMLTexture>();
    if (!enemyBulletTexture->loadFromFile(ResolveAssetPath("game/assets/enemies/enemy_bullets.png"))) {
        std::cerr << "Error: Could not load enemy bullet sprite" << std::endl;
        return 1;
    }
    std::cout << "[Game] ✅ Enemy bullet texture loaded: " << enemyBulletTexture->getSize().x << "x" << enemyBulletTexture->getSize().y << std::endl;

    explosionTexture = std::make_unique<SFMLTexture>();
    if (!explosionTexture->loadFromFile(ResolveAssetPath("game/assets/enemies/r-typesheet44.png"))) {
        std::cerr << "Error: Could not load explosion sprite" << std::endl;
        return 1;
    }

    // Load sound
    if (!shootBuffer.loadFromFile(ResolveAssetPath("game/assets/vfx/shoot.ogg"))) {
        std::cerr << "Warning: Could not load shoot.ogg" << std::endl;
    } else {
        shootSound.setBuffer(shootBuffer);
        shootSound.setVolume(80.f);
    }

    // NOTE: Factory registration moved later after texture preloading

    // Load Lua scripts
    std::cout << "📜 Loading Lua scripts..." << std::endl;

    // Load main initialization script (which loads all configs)
    if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/init.lua"))) {
        std::cerr << "Warning: Could not load init.lua, trying fallback..." << std::endl;
        // Fallback to old config if init.lua doesn't exist
        if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/config/game_config.lua"))) {
            std::cerr << "Warning: Could not load game_config.lua either" << std::endl;
        }
    } else {
        std::cout << "[Game] ✓ init.lua loaded - All configurations initialized" << std::endl;
        
        // Initialize mode based on network setting
        sol::state& lua = luaState.GetState();
        if (networkMode) {
            sol::protected_function initNetwork = lua["InitNetworkMode"];
            if (initNetwork.valid()) {
                initNetwork();
            }
        } else {
            sol::protected_function initSolo = lua["InitSoloMode"];
            if (initSolo.valid()) {
                initSolo();
                std::cout << "[Game] Solo mode initialized - Enemy showcase may be active" << std::endl;
            }
        }
    }

    // Set up game state callbacks for the engine (keeps engine abstract)
    eng::engine::core::GameStateCallbacks gameStateCallbacks;
    gameStateCallbacks.setState = [](const std::string& state) {
        auto& gsm = GameStateManager::Instance();
        if (state == "playing" || state == "Playing") {
            gsm.SetState(GameState::Playing);
        } else if (state == "paused" || state == "Paused") {
            gsm.SetState(GameState::Paused);
        } else if (state == "menu" || state == "MainMenu") {
            gsm.SetState(GameState::MainMenu);
        } else if (state == "options" || state == "Options") {
            gsm.SetState(GameState::Options);
        } else if (state == "lobby" || state == "Lobby") {
            gsm.SetState(GameState::Lobby);
        } else if (state == "credits" || state == "Credits") {
            gsm.SetState(GameState::Credits);
        }
    };
    gameStateCallbacks.getState = []() -> std::string {
        auto state = GameStateManager::Instance().GetState();
        switch (state) {
            case GameState::MainMenu: return "MainMenu";
            case GameState::Playing: return "Playing";
            case GameState::Paused: return "Paused";
            case GameState::Options: return "Options";
            case GameState::Lobby: return "Lobby";
            case GameState::Credits: return "Credits";
            default: return "Unknown";
        }
    };
    gameStateCallbacks.isPaused = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Paused;
    };
    gameStateCallbacks.isPlaying = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Playing;
    };
    gameStateCallbacks.togglePause = []() {
        auto& gsm = GameStateManager::Instance();
        if (gsm.GetState() == GameState::Playing) {
            gsm.SetState(GameState::Paused);
        } else if (gsm.GetState() == GameState::Paused) {
            gsm.SetState(GameState::Playing);
        }
    };
    gameStateCallbacks.goBack = []() {
        auto& gsm = GameStateManager::Instance();
        auto state = gsm.GetState();
        if (state == GameState::Paused) {
            gsm.SetState(GameState::Playing);
        } else if (state == GameState::Options || state == GameState::Credits || state == GameState::Lobby) {
            gsm.SetState(GameState::MainMenu);
        }
    };
    Scripting::UIBindings::SetGameStateCallbacks(gameStateCallbacks);
    std::cout << "[Game] Game state callbacks injected into engine" << std::endl;

    // Register UI bindings to Lua (must be after UISystem is created)
    Scripting::UIBindings::RegisterAll(luaState.GetState(), uiSystem.get());
    std::cout << "[Game] UI bindings registered to Lua" << std::endl;

    // Set Lua state for UISystem callbacks
    uiSystem->SetLuaState(&luaState.GetState());
    std::cout << "[Game] Lua state set for UISystem" << std::endl;

    // Pass base path to Lua for asset resolution
    luaState.GetState()["ASSET_BASE_PATH"] = g_basePath;
    std::cout << "[Game] Asset base path set for Lua: " << (g_basePath.empty() ? "(current dir)" : g_basePath) << std::endl;

    // ========================================
    // PRELOAD TEXTURES (including per-enemy textures from Lua config)
    // ========================================
    // Use Game::textureMap member so other Game methods (CreateEnemy, CreateEnemyMissile, etc.)
    // can access preloaded textures.
    // Keep ownership of dynamically loaded textures so they live for game lifetime
    std::vector<std::unique_ptr<SFMLTexture>> dynamicTextures;

    // Core textures we already loaded earlier
    textureMap["background"] = backgroundTexture.get();
    textureMap["player"] = playerTexture.get();
    textureMap["missile"] = missileTexture.get();
    textureMap["explosion"] = explosionTexture.get();
    textureMap["enemy_bullets"] = enemyBulletTexture.get();

    // Load enemy-specific textures referenced in EnemiesConfig (if available)
    try {
        sol::state& lua = luaState.GetState();
        sol::table enemiesConfig = lua["EnemiesConfig"];
        if (enemiesConfig.valid()) {
            for (auto& kv : enemiesConfig) {
                // kv.first = key, kv.second = value
                sol::object key = kv.first;
                sol::object val = kv.second;
                if (val.is<sol::table>()) {
                    sol::table cfg = val.as<sol::table>();
                    sol::table spriteTbl = cfg["sprite"];
                    if (spriteTbl.valid()) {
                        std::string texPath = spriteTbl["texture"].get_or(std::string());
                        if (!texPath.empty() && textureMap.find(texPath) == textureMap.end()) {
                            auto tex = std::make_unique<SFMLTexture>();
                            bool loaded = false;
                            // Try resolved candidate paths
                            std::string candidate1 = ResolveAssetPath(std::string("game/assets/") + texPath);
                            if (!candidate1.empty() && tex->loadFromFile(candidate1)) {
                                loaded = true;
                                std::cout << "[Game] Loaded enemy texture: " << candidate1 << std::endl;
                            } else {
                                std::string candidate2 = ResolveAssetPath(texPath);
                                if (!candidate2.empty() && tex->loadFromFile(candidate2)) {
                                    loaded = true;
                                    std::cout << "[Game] Loaded enemy texture: " << candidate2 << std::endl;
                                }
                            }
                            if (loaded) {
                                textureMap[texPath] = tex.get();
                                dynamicTextures.push_back(std::move(tex));
                            } else {
                                std::cerr << "[Game] Warning: could not load enemy texture '" << texPath << "'" << std::endl;
                            }
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Game] Exception while preloading enemy textures: " << e.what() << std::endl;
    }

    // If no generic "enemy" key exists, use the first loaded enemies/* texture as a fallback
    if (textureMap.find("enemy") == textureMap.end()) {
        for (auto& kv : textureMap) {
            if (kv.first.find("enemies/") == 0) {
                textureMap["enemy"] = kv.second;
                break;
            }
        }
    }

    // Register factories after textures are prepared so factories can use per-type textures
    // Register factories and give them access to our textureMap
    RType::Scripting::FactoryBindings::RegisterFactories(
        luaState.GetState(),
        &gCoordinator,
        textureMap,
        &allSprites,
        [this](ECS::Entity e) { this->RegisterEntity(e); }
    );

    // Load UI scripts
    std::cout << "🎨 Loading UI scripts..." << std::endl;
    if (!luaState.LoadScript(ResolveAssetPath("game/assets/scripts/ui_init.lua"))) {
        std::cerr << "Warning: Could not load ui_init.lua" << std::endl;
    } else {
        // Initialize UI from Lua
        sol::state& lua = luaState.GetState();
        sol::protected_function initUI = lua["InitUI"];
        if (initUI.valid()) {
            sol::protected_function_result result = initUI(1920, 1080);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[Game] InitUI() error: " << err.what() << std::endl;
            } else {
                std::cout << "[Game] UI initialized from Lua" << std::endl;
            }
        } else {
            std::cerr << "[Game] InitUI function not found in Lua" << std::endl;
        }
    }

    // Load spawn system
    spawnScriptSystem = Scripting::ScriptedSystemLoader::LoadSystem(
        ResolveAssetPath("assets/scripts/systems/spawn_system.lua"),
        &gCoordinator
    );

    if (spawnScriptSystem) {
        std::cout << "[Game] Spawn system loaded from Lua" << std::endl;
    } else {
        std::cerr << "[Game] Warning: Spawn system failed to load" << std::endl;
    }

    // Create game entities - but NOT the player yet (created when game starts)
    ECS::Entity player = 0;
    bool playerCreated = false;
    
    // Only create background for now - player will be created when game starts
    CreateBackground(0.0f, 0.0f, 1080.0f, true);

    // Game variables using engine abstractions
    eng::engine::Clock clock;
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 2.0f;
    float enemyShootTimer = 0.0f;
    float enemyShootInterval = 1.5f;  // Enemies shoot every 1.5 seconds

    bool spacePressed = false;
    float spaceHoldTime = 0.0f;
    const float chargeStartTime = 0.1f;
    ECS::Entity activeChargingEffect = 0;
    bool hasChargingEffect = false;

    std::cout << "Game initialized successfully!" << std::endl;

    // In network mode, start directly in Playing state (server handles game logic)
    if (networkMode) {
        GameStateManager::Instance().SetState(GameState::Playing);
        std::cout << "[Game] Network mode: Starting directly in Playing state" << std::endl;
    }

    // Input mask for network
    uint8_t inputMask = 0;

    // Track entities we've added sprites to
    std::set<ECS::Entity> entitiesWithSprites;

    // ========================================
    // MAIN GAME LOOP (System-Driven)
    // ========================================
    std::cout << "[Game] Starting game loop..." << std::endl;

    while (window.isOpen()) {
        float deltaTime = clock.restart();

        // Cap deltaTime to prevent huge jumps (max 0.1s = 10 FPS minimum)
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }

        // ========================================
        // GAME STATE MANAGEMENT
        // ========================================
        auto currentState = GameStateManager::Instance().GetState();
        
        // Create player when transitioning to Playing state
        if (currentState == GameState::Playing && !playerCreated && !networkMode) {
            player = CreatePlayer(100.0f, 400.0f);
            playerCreated = true;
            std::cout << "[Game] Player created - game starting!" << std::endl;
        }

        // Skip game logic updates when in menu states
        bool inMenu = (currentState == GameState::MainMenu || 
                       currentState == GameState::Paused ||
                       currentState == GameState::Options ||
                       currentState == GameState::Lobby ||
                       currentState == GameState::Credits);

        // Find local player entity in network mode
        if (networkMode && player == 0 && networkSystem) {
            // Find entity with isLocalPlayer = true
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<NetworkId>(entity)) {
                    auto& netId = gCoordinator.GetComponent<NetworkId>(entity);
                    if (netId.isLocalPlayer) {
                        player = entity;
                        std::cout << "[Game] Found local player entity: " << player << " (networkId: " << netId.networkId << ")" << std::endl;
                        break;
                    }
                }
            }
        }

        // ========================================
        // UPDATE GAME LOGIC (Lua callbacks, showcase, etc.)
        // ========================================
        if (!inMenu) {
            sol::state& lua = luaState.GetState();
            sol::protected_function updateGame = lua["UpdateGame"];
            if (updateGame.valid()) {
                sol::protected_function_result result = updateGame(deltaTime);
                if (!result.valid()) {
                    sol::error err = result;
                    // Silently fail - don't spam console with errors
                }
            }
        }

        // ========================================
        // 1. NETWORK UPDATE (Receives server state)
        // ========================================
        if (networkMode && networkSystem) {
            networkSystem->Update(deltaTime);

            // Count entities with Enemy tag
            int enemyCount = 0;
            int enemyWithSpriteCount = 0;
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<Tag>(entity)) {
                    auto& tag = gCoordinator.GetComponent<Tag>(entity);
                    if (tag.name == "Enemy") {
                        enemyCount++;
                        if (gCoordinator.HasComponent<Sprite>(entity)) {
                            enemyWithSpriteCount++;
                        }
                    }
                }
            }

            static int frameCounter = 0;
            if (frameCounter++ % 60 == 0) {  // Log every 60 frames
                std::cout << "[Game] Enemies: " << enemyCount << " total, " << enemyWithSpriteCount
                          << " with sprites, " << (enemyCount - enemyWithSpriteCount) << " invisible" << std::endl;
            }

            // Add sprites to network entities that don't have them
            for (auto entity : allEntities) {
                if (entitiesWithSprites.find(entity) == entitiesWithSprites.end() &&
                    !gCoordinator.HasComponent<Sprite>(entity) &&  // IMPORTANT: Don't add sprite if already exists
                    gCoordinator.HasComponent<NetworkId>(entity) &&
                    gCoordinator.HasComponent<Position>(entity) &&
                    gCoordinator.HasComponent<Tag>(entity)) {

                    auto& tag = gCoordinator.GetComponent<Tag>(entity);
                    auto& pos = gCoordinator.GetComponent<Position>(entity);
                    auto& networkId = gCoordinator.GetComponent<NetworkId>(entity);

                    std::cout << "[Game] Adding sprite to entity " << entity << " with tag: " << tag.name
                              << " at position (" << pos.x << ", " << pos.y << ")" << std::endl;

                    // Create sprite based on tag - with proper visuals
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);

                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.layer = 10;  // Default layer for players/enemies
                    spriteComp.scaleX = 3.0f;  // Default scale, will be overridden per entity type
                    spriteComp.scaleY = 3.0f;

                    if (tag.name == "Player") {
                        // Player sprite
                        sprite->setTexture(playerTexture.get());
                        int line = networkId.playerLine; // Use the server-assigned ship color
                        IntRect rect(33 * 2, line * 17, 33, 17);
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.layer = 10;  // Players on layer 10
                        // Player keeps default scale of 3.0x

                        // Add animation component
                        StateMachineAnimation anim;
                        anim.currentColumn = 2;
                        anim.targetColumn = 2;
                        anim.transitionSpeed = 0.15f;
                        anim.spriteWidth = 33;
                        anim.spriteHeight = 17;
                        anim.currentRow = line;
                        gCoordinator.AddComponent(entity, anim);

                    } else if (tag.name == "Enemy") {
                        // Enemy sprite - r-typesheet5.png is a single horizontal line (533x36)
                        sprite->setTexture(textureMap["enemy"]);

                        // Test: Use frame 10 (330 pixels) which should show enemy pointing left
                        IntRect rect(0, 0, 33, 32);  // Frame 0 - match CreateEnemy exactly
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.layer = 9;  // Enemies on layer 9 (behind players)
                        spriteComp.scaleX = 2.5f;  // CRITICAL: Enemy scale (same as CreateEnemy)
                        spriteComp.scaleY = 2.5f;

                        std::cout << "[Game] Enemy entity " << entity << " set to scale 2.5x" << std::endl;

                        // Add animation for enemy (8 first frames - all pointing left)
                        Animation anim;
                        anim.frameCount = 8;  // Only the first 8 frames pointing left
                        anim.currentFrame = 0;
                        anim.frameTime = 0.1f;
                        anim.currentTime = 0.0f;
                        anim.loop = true;
                        anim.frameWidth = 33;  // Match CreateEnemy
                        anim.frameHeight = 32;  // Match CreateEnemy (NOT 36!)
                        anim.startX = 0;  // Start at frame 0
                        anim.startY = 0;  // Always line 0
                        anim.spacing = 1000;  // Spacing between frames (33 pixels)
                        gCoordinator.AddComponent(entity, anim);

                    } else if (tag.name == "PlayerBullet" || tag.name == "bullet" || tag.name == "charged_bullet") {
                        // Missile sprite - check if charged
                        sprite->setTexture(missileTexture.get());

                        IntRect rect;
                        bool isCharged = false;
                        int chargeLevel = 0;

                        if (gCoordinator.HasComponent<ShootEmUp::Components::ProjectileTag>(entity)) {
                            auto& projTag = gCoordinator.GetComponent<ShootEmUp::Components::ProjectileTag>(entity);
                            isCharged = projTag.chargeLevel > 0;
                            chargeLevel = projTag.chargeLevel;
                        }

                        if (isCharged && chargeLevel > 0) {
                            // Charged missile sprites (lignes 5-9 avec les gros missiles)
                            struct ChargeData {
                                int xPos, yPos, width, height;
                            };
                            ChargeData chargeLevels[5] = {
                                {233, 100, 15, 15},  // Level 1
                                {202, 117, 31, 15},  // Level 2
                                {170, 135, 47, 15},  // Level 3
                                {138, 155, 63, 15},  // Level 4
                                {105, 170, 79, 17}   // Level 5
                            };
                            int idx = chargeLevel - 1;
                            if (idx < 0) idx = 0;
                            if (idx > 4) idx = 4;
                            ChargeData& data = chargeLevels[idx];
                            rect = IntRect(data.xPos, data.yPos, data.width, data.height);

                            // Add animation for charged missiles
                            Animation anim;
                            anim.frameTime = 0.1f;
                            anim.currentFrame = 0;
                            anim.frameCount = 2;
                            anim.loop = true;
                            anim.frameWidth = rect.width;
                            anim.frameHeight = rect.height;
                            anim.startX = rect.left;
                            anim.startY = rect.top;
                            anim.spacing = rect.width + 2;
                            gCoordinator.AddComponent(entity, anim);
                        } else {
                            // Normal missile
                            rect = IntRect(245, 85, 20, 20);
                        }

                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.layer = 7;  // Player bullets on layer 7 (behind enemies)
                        spriteComp.scaleX = 3.0f;  // Missile scale
                        spriteComp.scaleY = 3.0f;

                    } else if (tag.name == "EnemyBullet") {
                        // Enemy bullet sprite - use dedicated enemy_bullets.png texture
                        // Image is 400x85, isolate just the orange ball (not the trail)
                        sprite->setTexture(enemyBulletTexture.get());
                        IntRect rect(110, 0, 15, 15); // Match animation params: 15x15 at x=110
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.layer = 6;  // Enemy bullets on layer 6 (behind player bullets)
                        spriteComp.scaleX = 4.0f;  // Scale up for visibility
                        spriteComp.scaleY = 4.0f;

                        // Add animation - 4 first frames, just the orange balls
                        Animation anim;
                        anim.frameTime = 0.3f;  // Animation speed
                        anim.currentFrame = 0;
                        anim.frameCount = 4;  // Only first 4 frames
                        anim.loop = true;
                        anim.frameWidth = 15;   // Just the ball part
                        anim.frameHeight = 15;  // Just the ball part
                        anim.startX = 132;      // Offset to where balls start
                        anim.startY = 0;        // Top of image
                        anim.spacing = 1;      // Gap between balls (50 - 15 = 35)
                        gCoordinator.AddComponent(entity, anim);

                    } else if (tag.name == "Explosion") {
                        // Explosion sprite - animated explosion from explosion spritesheet
                        // IMPORTANT: Use EXACT same parameters as CreateExplosion()
                        sprite->setTexture(explosionTexture.get());
                        IntRect rect(130, 1, 33, 32);  // Match CreateExplosion parameters
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.layer = 15;  // Explosions on top layer
                        spriteComp.scaleX = 2.5f;  // Match CreateExplosion scale
                        spriteComp.scaleY = 2.5f;

                        // Add animation - EXACT same parameters as CreateExplosion
                        Animation anim;
                        anim.frameTime = 0.08f;
                        anim.currentFrame = 0;
                        anim.frameCount = 6;
                        anim.loop = false;  // Play once
                        anim.frameWidth = 32;
                        anim.frameHeight = 32;
                        anim.startX = 130;  // Match CreateExplosion
                        anim.startY = 1;    // Match CreateExplosion
                        anim.spacing = 1.5; // Match CreateExplosion
                        gCoordinator.AddComponent(entity, anim);

                        // Add lifetime component so explosion disappears after animation
                        Lifetime lifetime;
                        lifetime.maxLifetime = 0.05f;  // Very short lifetime
                        lifetime.timeAlive = 0.0f;
                        gCoordinator.AddComponent(entity, lifetime);

                        std::cout << "[Game] Added explosion sprite to network entity " << entity << std::endl;

                    } else {
                        // Unknown entity type - log warning and use default enemy appearance
                        std::cerr << "[Game] WARNING: Unknown entity tag '" << tag.name << "' for entity " << entity << std::endl;
                        sprite->setTexture(textureMap["enemy"]);
                        IntRect rect(0, 0, 32, 32);
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.scaleX = 2.5f;  // Default enemy scale
                        spriteComp.scaleY = 2.5f;
                    }

                    sprite->setPosition(Vector2f(pos.x, pos.y));

                    gCoordinator.AddComponent(entity, spriteComp);
                    entitiesWithSprites.insert(entity);

                    std::cout << "[Game] Added sprite to network entity " << entity << " (" << tag.name
                              << ") scale=" << spriteComp.scaleX << "x" << spriteComp.scaleY << std::endl;
                }
            }
        }

        // Reset input mask
        inputMask = 0;

        // Get current game state for input handling
        auto& gsm = GameStateManager::Instance();
        auto currentGameState = gsm.GetState();

        // Event handling using engine abstractions
        eng::engine::InputEvent event;
        while (window.pollEvent(event)) {
            if (event.type == eng::engine::EventType::Closed) {
                window.close();
            }

            // Handle Escape key for pause menu toggle
            if (event.type == eng::engine::EventType::KeyReleased && event.key.code == eng::engine::Key::Escape) {
                if (currentGameState == GameState::Playing) {
                    gsm.SetState(GameState::Paused);
                    std::cout << "[Game] Game paused" << std::endl;
                } else if (currentGameState == GameState::Paused) {
                    gsm.SetState(GameState::Playing);
                    std::cout << "[Game] Game resumed" << std::endl;
                } else if (currentGameState == GameState::Options || currentGameState == GameState::Credits) {
                    // Return to main menu from sub-menus
                    gsm.SetState(GameState::MainMenu);
                    std::cout << "[Game] Returned to main menu" << std::endl;
                }
            }

            // Pass events to UI system when in menu states
            if (currentGameState != GameState::Playing) {
                if (event.type == eng::engine::EventType::TextEntered) {
                    // UTF-32 vers char (ASCII seulement, sinon ignorer ou adapter)
                    if (event.text.unicode < 128 && event.text.unicode >= 32) {
                        uiSystem->HandleTextInput(static_cast<char>(event.text.unicode));
                    }
                }
                uiSystem->HandleEvent(event);
            }

            // Handle space key release for shooting (only during gameplay)
            if (currentGameState == GameState::Playing &&
                event.type == eng::engine::EventType::KeyReleased && event.key.code == eng::engine::Key::Space) {
                if (spacePressed && gCoordinator.HasComponent<Position>(player)) {
                    auto& playerPos = gCoordinator.GetComponent<Position>(player);

                    // Only create missiles locally in non-network mode
                    // In network mode, the server will create them based on our input
                    if (!networkMode) {
                        int chargeLevel = 0;
                        if (hasChargingEffect && spaceHoldTime >= chargeStartTime) {
                            // Calculate charge level based on hold time
                            float chargeProgress = (spaceHoldTime - chargeStartTime) / 0.8f;
                            if (chargeProgress < 0.2f)
                                chargeLevel = 1;
                            else if (chargeProgress < 0.4f)
                                chargeLevel = 2;
                            else if (chargeProgress < 0.6f)
                                chargeLevel = 3;
                            else if (chargeProgress < 0.8f)
                                chargeLevel = 4;
                            else
                                chargeLevel = 5;
                        }

                        if (chargeLevel > 0) {
                            // Create charged missile
                            CreateMissile(playerPos.x + 99.0f, playerPos.y + 25.0f, true, chargeLevel);
                        } else {
                            // Create normal missile
                            CreateMissile(playerPos.x + 99.0f, playerPos.y + 30.0f, false, 0);

                            // Play shoot sound using engine abstraction
                            // Stop the sound if it's playing, then play it again for rapid fire
                            shootSound.stop();
                            shootSound.play();

                            // Create shoot effect
                            CreateShootEffect(playerPos.x + 89.0f, playerPos.y + 10.0f, player);
                        }
                    }

                    // Clean up charging effect
                    if (hasChargingEffect) {
                        DestroyEntityDeferred(activeChargingEffect);
                        hasChargingEffect = false;
                    }

                    spacePressed = false;
                    spaceHoldTime = 0.0f;
                }
            }
        }

        // Handle continuous input (only during gameplay)
        if (!inMenu && eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Space)) {
            if (!spacePressed) {
                spacePressed = true;
            }
            spaceHoldTime += deltaTime;

            // Start charging effect
            if (spaceHoldTime >= chargeStartTime && !hasChargingEffect && gCoordinator.HasComponent<Position>(player)) {
                auto& playerPos = gCoordinator.GetComponent<Position>(player);

                ECS::Entity chargeEffect = gCoordinator.CreateEntity();
                RegisterEntity(chargeEffect);
                gCoordinator.AddComponent(chargeEffect, Position{playerPos.x + 99.0f, playerPos.y - 5.0f});

                auto* sprite = new SFMLSprite();
                allSprites.push_back(sprite);
                sprite->setTexture(missileTexture.get());
                IntRect rect(0, 50, 29, 35);
                sprite->setTextureRect(rect);
                sprite->setPosition(Vector2f(playerPos.x + 99.0f, playerPos.y - 5.0f));

                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.layer = 11;
                gCoordinator.AddComponent(chargeEffect, spriteComp);

                Animation anim;
                anim.frameTime = 0.08f;
                anim.currentFrame = 0;
                anim.frameCount = 8;
                anim.loop = true;
                anim.frameWidth = 29;
                anim.frameHeight = 35;
                anim.startX = 0;
                anim.startY = 50;
                anim.spacing = 34;
                gCoordinator.AddComponent(chargeEffect, anim);

                ShootEmUp::Components::Effect effectTag;
                effectTag.effectType = "charge";
                effectTag.followParent = true;
                gCoordinator.AddComponent(chargeEffect, effectTag);

                gCoordinator.AddComponent(chargeEffect, Tag{"charge_effect"});

                activeChargingEffect = chargeEffect;
                hasChargingEffect = true;
            }

            // Update charging effect position
            if (hasChargingEffect && gCoordinator.HasComponent<Position>(player) &&
                gCoordinator.HasComponent<Position>(activeChargingEffect)) {
                auto& playerPos = gCoordinator.GetComponent<Position>(player);
                auto& chargePos = gCoordinator.GetComponent<Position>(activeChargingEffect);
                chargePos.x = playerPos.x + 99.0f;
                chargePos.y = playerPos.y - 5.0f;

                if (gCoordinator.HasComponent<Sprite>(activeChargingEffect)) {
                    auto& chargeSprite = gCoordinator.GetComponent<Sprite>(activeChargingEffect);
                    if (chargeSprite.sprite) {
                        chargeSprite.sprite->setPosition(Vector2f(chargePos.x, chargePos.y));
                    }
                }
            }
        }

        // ========================================
        // 2. INPUT CAPTURE & NETWORK SEND (Only during gameplay)
        // ========================================
        bool movingUp = false, movingDown = false, movingLeft = false, movingRight = false;
        bool firing = false;
        
        // In network mode, always capture inputs (server handles game state)
        // In local mode, only capture when not in menu
        if (networkMode || !inMenu) {
            movingUp = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Up);
            movingDown = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Down);
            movingLeft = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Left);
            movingRight = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Right);
            firing = spacePressed;
            
            // Debug: Log inputs in network mode
            if (networkMode && (movingUp || movingDown || movingLeft || movingRight || firing)) {
                std::cout << "[Input] Up:" << movingUp << " Down:" << movingDown 
                          << " Left:" << movingLeft << " Right:" << movingRight 
                          << " Fire:" << firing << std::endl;
            }
        }

        // Build input mask for network (bit flags from Protocol.md)
        inputMask = 0;
        if (movingUp)
            inputMask |= (1 << 0);
        if (movingDown)
            inputMask |= (1 << 1);
        if (movingLeft)
            inputMask |= (1 << 2);
        if (movingRight)
            inputMask |= (1 << 3);
        if (firing)
            inputMask |= (1 << 4);

        // Calculate charge level based on hold time
        uint8_t chargeLevel = 0;
        if (hasChargingEffect && spaceHoldTime >= chargeStartTime) {
            float chargeProgress = (spaceHoldTime - chargeStartTime) / 0.8f;
            if (chargeProgress < 0.2f)
                chargeLevel = 1;
            else if (chargeProgress < 0.4f)
                chargeLevel = 2;
            else if (chargeProgress < 0.6f)
                chargeLevel = 3;
            else if (chargeProgress < 0.8f)
                chargeLevel = 4;
            else
                chargeLevel = 5;
        }

        // Send input to server if in network mode
        if (networkMode && networkSystem) {
            // Only log when there's actual input to reduce spam
            if (inputMask != 0) {
                std::cout << "[Network] Sending inputMask=" << (int)inputMask 
                          << " chargeLevel=" << (int)chargeLevel << std::endl;
            }
            networkSystem->sendInput(inputMask, chargeLevel);
        }

        // ========================================
        // 3. LOCAL PLAYER INPUT (Only in local mode - manual)
        // ========================================
        if (!networkMode && player != 0 && gCoordinator.HasComponent<Velocity>(player)) {
            auto& playerVel = gCoordinator.GetComponent<Velocity>(player);
            float speed = 500.0f;
            playerVel.dx = 0.0f;
            playerVel.dy = 0.0f;

            if (movingUp)
                playerVel.dy = -speed;
            if (movingDown)
                playerVel.dy = speed;
            if (movingLeft)
                playerVel.dx = -speed;
            if (movingRight)
                playerVel.dx = speed;

            // Update animation target
            if (gCoordinator.HasComponent<StateMachineAnimation>(player)) {
                auto& playerAnim = gCoordinator.GetComponent<StateMachineAnimation>(player);
                if (movingUp)
                    playerAnim.targetColumn = 4;
                else if (movingDown)
                    playerAnim.targetColumn = 0;
                else
                    playerAnim.targetColumn = 2;
            }
        }

        // ========================================
        // 4. LOCAL ENEMY SPAWNING (Only in local mode during gameplay)
        // ========================================
        if (!inMenu && !networkMode) {
            enemySpawnTimer += deltaTime;
            
            if (enemySpawnTimer >= enemySpawnInterval) {
                enemySpawnTimer = 0.0f;
                
                // Random Y position
                float randomY = 100.0f + static_cast<float>(rand() % 800);
                
                // Random pattern type
                std::vector<std::string> patterns = {"straight", "zigzag", "sinewave"};
                std::string pattern = patterns[rand() % patterns.size()];
                
                // Create enemy using the Game::CreateEnemy method (which has proper sprites & animations)
                ECS::Entity enemy = CreateEnemy(1920.0f, randomY, pattern);
                
                if (enemy != 0) {
                    std::cout << "[Game] Spawned " << pattern << " enemy at Y=" << randomY << std::endl;
                }
                
                // Vary spawn interval for unpredictability
                enemySpawnInterval = 1.5f + static_cast<float>(rand() % 20) / 10.0f; // 1.5 to 3.5 seconds
            }

            // ========================================
            // 4b. ENEMY SHOOTING
            // ========================================
            enemyShootTimer += deltaTime;
            
            if (enemyShootTimer >= enemyShootInterval) {
                enemyShootTimer = 0.0f;
                
                int enemyCount = 0;
                int shotsCreated = 0;
                
                // Make each enemy shoot
                for (auto entity : allEntities) {
                    if (gCoordinator.HasComponent<ShootEmUp::Components::EnemyTag>(entity) &&
                        gCoordinator.HasComponent<Position>(entity)) {
                        
                        enemyCount++;
                        auto& pos = gCoordinator.GetComponent<Position>(entity);
                        
                        // Only shoot if enemy is on screen (X < 1920 and X > 0)
                        if (pos.x > 50.0f && pos.x < 1800.0f) {
                            // 50% chance for each enemy to shoot
                            if (rand() % 2 == 0) {
                                CreateEnemyMissile(pos.x - 30.0f, pos.y + 20.0f);
                                shotsCreated++;
                            }
                        }
                    }
                }
                
                std::cout << "[Enemy Shoot] Enemies: " << enemyCount << ", Shots: " << shotsCreated << std::endl;
                
                // Vary shoot interval
                enemyShootInterval = 1.0f + static_cast<float>(rand() % 15) / 10.0f; // 1.0 to 2.5 seconds
            }
        }

        // ========================================
        // 4c. PLAYER INVINCIBILITY & FLASH UPDATE
        // ========================================
        if (!inMenu && player != 0 && gCoordinator.HasComponent<Health>(player)) {
            auto& health = gCoordinator.GetComponent<Health>(player);
            
            if (health.invincibilityTimer > 0.0f) {
                health.invincibilityTimer -= deltaTime;
                health.flashTimer += deltaTime;
                
                // Toggle visibility every 0.05 seconds for fast blinking
                if (gCoordinator.HasComponent<Sprite>(player)) {
                    auto& sprite = gCoordinator.GetComponent<Sprite>(player);
                    // Blink by toggling scale (0 = invisible, normal = visible)
                    bool visible = (static_cast<int>(health.flashTimer / 0.05f) % 2) == 0;
                    sprite.scaleX = visible ? 3.0f : 0.0f;
                    sprite.scaleY = visible ? 3.0f : 0.0f;
                }
                
                // End invincibility
                if (health.invincibilityTimer <= 0.0f) {
                    health.invincibilityTimer = 0.0f;
                    health.isFlashing = false;
                    health.flashTimer = 0.0f;
                    // Restore normal visibility
                    if (gCoordinator.HasComponent<Sprite>(player)) {
                        auto& sprite = gCoordinator.GetComponent<Sprite>(player);
                        sprite.scaleX = 3.0f;
                        sprite.scaleY = 3.0f;
                    }
                }
            }
        }

        // ========================================
        // 5. SYSTEM UPDATES (ECS Architecture!)
        // ========================================

        // Always update scrolling background (for visual effect even in menus)
        scrollingBgSystem->Update(deltaTime);

        // Only update game systems during gameplay
        if (!inMenu) {
            // In network mode, server handles physics - only update animations/visuals
            if (networkMode) {
                stateMachineAnimSystem->Update(deltaTime);
                animationSystem->Update(deltaTime);
                lifetimeSystem->Update(deltaTime);
            } else {
                // Local mode: Full simulation
                movementPatternSystem->Update(deltaTime);
                movementSystem->Update(deltaTime);
                boundarySystem->Update(deltaTime);
                collisionSystem->Update(deltaTime);
                healthSystem->Update(deltaTime);
                stateMachineAnimSystem->Update(deltaTime);
                animationSystem->Update(deltaTime);
                lifetimeSystem->Update(deltaTime);
            }
        }

        // Process destroyed entities
        ProcessDestroyedEntities();

        // ========================================
        // 6. UI SYSTEM UPDATE
        // ========================================
        // Update UI system (handles input when in menu states)
        if (inMenu) {
            uiSystem->Update(deltaTime);
        }

        // Render using RenderSystem
        window.clear();
        renderSystem->Update(deltaTime);

        // Render UI on top of game (always, so menus are visible)
        uiSystem->Render(&window);

        window.display();
    }

    // Cleanup
    std::cout << "[Game] Starting cleanup..." << std::endl;
    std::cout << "[Game] Deleting " << allSprites.size() << " sprites..." << std::endl;

    for (size_t i = 0; i < allSprites.size(); ++i) {
        if (allSprites[i]) {
            delete allSprites[i];
            allSprites[i] = nullptr;
        }
    }
    allSprites.clear();
    std::cout << "[Game] Sprites deleted." << std::endl;

    std::cout << "[Game] Shutting down Coordinator..." << std::endl;
    gCoordinator.Shutdown();
    std::cout << "[Game] Coordinator shutdown complete." << std::endl;

    std::cout << "Game shutdown complete." << std::endl;
    return 0;
}
