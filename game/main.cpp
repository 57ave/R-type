#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <functional>
#include <sstream>
#include <iomanip>

// Engine includes - Core
#include <ecs/ECS.hpp>
#include <ecs/Coordinator.hpp>
#include <core/Logger.hpp>
#include <core/Profiler.hpp>
#include <core/ProfilerOverlay.hpp>
#include <core/DevConsole.hpp>

// Engine includes - Rendering
#include <rendering/sfml/SFMLWindow.hpp>
#include <rendering/sfml/SFMLRenderer.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>

// Engine includes - Input/Time/Audio abstractions
#include <engine/Input.hpp>
#include <engine/Keyboard.hpp>
#include <engine/Clock.hpp>
#include <engine/Audio.hpp>

// Network includes
#include <network/NetworkClient.hpp>
#include <network/RTypeProtocol.hpp>
#include <systems/NetworkSystem.hpp>

// Systems
#include <systems/MovementSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/StateMachineAnimationSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/MovementPatternSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <systems/BoundarySystem.hpp>
#include <systems/CollisionSystem.hpp>
#include <systems/HealthSystem.hpp>

// Components
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Weapon.hpp>
#include <components/Tag.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/MovementPattern.hpp>
#include <components/Lifetime.hpp>
#include <components/NetworkId.hpp>

using namespace rtype::engine::rendering;
using namespace rtype::engine::rendering::sfml;

// Global coordinator
ECS::Coordinator gCoordinator;

// Entity tracking
std::vector<ECS::Entity> allEntities;
std::vector<ECS::Entity> entitiesToDestroy;

// Textures storage
std::unique_ptr<SFMLTexture> backgroundTexture;
std::unique_ptr<SFMLTexture> playerTexture;
std::unique_ptr<SFMLTexture> missileTexture;
std::unique_ptr<SFMLTexture> enemyTexture;
std::unique_ptr<SFMLTexture> explosionTexture;

// Sprite storage (to prevent memory leaks)
std::vector<SFMLSprite*> allSprites;

// Audio using engine abstractions
rtype::engine::SoundBuffer shootBuffer;
rtype::engine::Sound shootSound;

void RegisterEntity(ECS::Entity entity) {
    allEntities.push_back(entity);
}

void DestroyEntityDeferred(ECS::Entity entity) {
    entitiesToDestroy.push_back(entity);
    LOG_DEBUG("ENTITY", "Marked entity #" + std::to_string(entity) + " for destruction");
}

void ProcessDestroyedEntities() {
    if (!entitiesToDestroy.empty()) {
        LOG_DEBUG("ECS", "Processing " + std::to_string(entitiesToDestroy.size()) + " entities for destruction");
    }
    for (auto entity : entitiesToDestroy) {
        // Clean up sprite if exists
        if (gCoordinator.HasComponent<Sprite>(entity)) {
            auto& sprite = gCoordinator.GetComponent<Sprite>(entity);
            if (sprite.sprite) {
                delete sprite.sprite;
                sprite.sprite = nullptr;
            }
        }
        
        LOG_DEBUG("ENTITY", "Destroyed entity #" + std::to_string(entity));
        gCoordinator.DestroyEntity(entity);
        
        // Remove from allEntities
        allEntities.erase(std::remove(allEntities.begin(), allEntities.end(), entity), allEntities.end());
    }
    entitiesToDestroy.clear();
}

// Helper function to create player entity
ECS::Entity CreatePlayer(float x, float y, int line = 0) {
    ECS::Entity player = gCoordinator.CreateEntity();
    RegisterEntity(player);
    LOG_DEBUG("ENTITY", "Created player entity #" + std::to_string(player) + " at (" + std::to_string(x) + ", " + std::to_string(y) + ") line=" + std::to_string(line));
    
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
    
    // Weapon
    Weapon weapon;
    weapon.fireRate = 0.2f;
    weapon.supportsCharge = true;
    weapon.minChargeTime = 0.1f;
    weapon.maxChargeTime = 1.0f;
    weapon.projectileSpeed = 1000.0f;
    weapon.shootSound = "shoot";
    gCoordinator.AddComponent(player, weapon);
    
    // Tags
    gCoordinator.AddComponent(player, Tag{"player"});
    gCoordinator.AddComponent(player, PlayerTag{0});
    
    return player;
}

// Helper function to create background entity
ECS::Entity CreateBackground(float x, float y, float windowHeight, bool isFirst) {
    ECS::Entity bg = gCoordinator.CreateEntity();
    RegisterEntity(bg);
    LOG_DEBUG("ENTITY", "Created background entity #" + std::to_string(bg));
    
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
ECS::Entity CreateEnemy(float x, float y, MovementPattern::Type pattern) {
    ECS::Entity enemy = gCoordinator.CreateEntity();
    RegisterEntity(enemy);
    LOG_DEBUG("ENTITY", "Spawned enemy #" + std::to_string(enemy) + " at (" + std::to_string(x) + ", " + std::to_string(y) + ") pattern=" + std::to_string(static_cast<int>(pattern)));
    
    // Position
    gCoordinator.AddComponent(enemy, Position{x, y});
    
    // Velocity
    gCoordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});
    
    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(enemyTexture.get());
    IntRect rect(0, 0, 33, 32);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));
    
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 5;
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
    anim.spacing = 33;
    gCoordinator.AddComponent(enemy, anim);
    
    // Movement pattern
    MovementPattern movementPattern;
    movementPattern.pattern = pattern;
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
    
    // Tags
    gCoordinator.AddComponent(enemy, Tag{"enemy"});
    gCoordinator.AddComponent(enemy, EnemyTag{"basic"});
    
    return enemy;
}

// Helper function to create missile entity
ECS::Entity CreateMissile(float x, float y, bool isCharged, int chargeLevel) {
    ECS::Entity missile = gCoordinator.CreateEntity();
    RegisterEntity(missile);
    if (isCharged) {
        LOG_DEBUG("ENTITY", "Created charged missile #" + std::to_string(missile) + " (level " + std::to_string(chargeLevel) + ") at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    } else {
        LOG_DEBUG("ENTITY", "Created missile #" + std::to_string(missile) + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    }
    
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
        rect = IntRect(245, 85, 20, 20);
    } else {
        // Charged missile sprites (lines 5-9)
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
    gCoordinator.AddComponent(missile, spriteComp);
    
    // Animation for charged missiles
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
    gCoordinator.AddComponent(missile, ProjectileTag{0, true});
    
    // Lifetime (destroy after 5 seconds or when off-screen)
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    gCoordinator.AddComponent(missile, lifetime);
    
    return missile;
}

// Helper function to create explosion effect
ECS::Entity CreateExplosion(float x, float y) {
    ECS::Entity explosion = gCoordinator.CreateEntity();
    RegisterEntity(explosion);
    LOG_DEBUG("VFX", "Created explosion #" + std::to_string(explosion) + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    
    // Position
    gCoordinator.AddComponent(explosion, Position{x, y});
    
    // Sprite
    auto* sprite = new SFMLSprite();
    allSprites.push_back(sprite);
    sprite->setTexture(explosionTexture.get());
    IntRect rect(129, 0, 34, 35);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));
    
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 15;
    spriteComp.scaleX = 2.5f;  // Scale up the explosion
    spriteComp.scaleY = 2.5f;
    gCoordinator.AddComponent(explosion, spriteComp);
    
    // Animation
    Animation anim;
    anim.frameTime = 0.15f; // Slower animation (was 0.1f)
    anim.currentFrame = 0;
    anim.frameCount = 6;
    anim.loop = false;
    anim.frameWidth = 34;
    anim.frameHeight = 35;
    anim.startX = 129;
    anim.startY = 0;
    anim.spacing = 33;  // Average spacing between explosion frames
    gCoordinator.AddComponent(explosion, anim);
    
    // Lifetime (destroy after animation finishes)
    Lifetime lifetime;
    lifetime.maxLifetime = 1.0f; // 1 second before disappearing
    gCoordinator.AddComponent(explosion, lifetime);
    
    // Effect tag
    Effect effect;
    effect.effectType = Effect::Type::EXPLOSION;
    gCoordinator.AddComponent(explosion, effect);
    
    gCoordinator.AddComponent(explosion, Tag{"explosion"});
    
    return explosion;
}

// Helper function to create shoot effect
ECS::Entity CreateShootEffect(float x, float y, ECS::Entity parent) {
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
    Effect effectTag;
    effectTag.effectType = Effect::Type::SHOOT;
    effectTag.followParent = true;
    gCoordinator.AddComponent(effect, effectTag);
    
    gCoordinator.AddComponent(effect, Tag{"effect"});
    
    return effect;
}

int main(int argc, char* argv[])
{
    // Initialize Logger
    auto& logger = rtype::core::Logger::getInstance();
    logger.init(".log", "rtype_game.log");
    LOG_INFO("GAME", "R-Type Game Starting with ECS Engine (Refactored)...");
    
    // Initialize Profiler
    auto& profiler = rtype::core::Profiler::getInstance();
    profiler.init();
    
    // Create profiler overlay (will be initialized after window creation)
    rtype::core::ProfilerOverlay profilerOverlay;
    
    std::cout << "R-Type Game Starting with ECS Engine (Refactored)..." << std::endl;
    
    // Parse command line arguments
    bool networkMode = false;
    std::string serverAddress = "127.0.0.1";
    short serverPort = 12345;
    
    if (argc > 1 && std::string(argv[1]) == "--network") {
        networkMode = true;
        if (argc > 2) {
            serverAddress = argv[2];
        }
        if (argc > 3) {
            serverPort = static_cast<short>(std::stoi(argv[3]));
        }
        LOG_INFO("NETWORK", "Network mode enabled. Server: " + serverAddress + ":" + std::to_string(serverPort));
        std::cout << "[Game] Network mode enabled. Server: " << serverAddress << ":" << serverPort << std::endl;
    } else {
        LOG_INFO("GAME", "Local mode (use --network <ip> <port> for multiplayer)");
        std::cout << "[Game] Local mode (use --network <ip> <port> for multiplayer)" << std::endl;
    }
    
    // Initialize ECS Coordinator
    gCoordinator.Init();
    LOG_DEBUG("ECS", "Coordinator initialized");
    
    // Register all components
    gCoordinator.RegisterComponent<Position>();
    gCoordinator.RegisterComponent<Velocity>();
    gCoordinator.RegisterComponent<Sprite>();
    gCoordinator.RegisterComponent<Animation>();
    gCoordinator.RegisterComponent<StateMachineAnimation>();
    gCoordinator.RegisterComponent<Collider>();
    gCoordinator.RegisterComponent<Health>();
    gCoordinator.RegisterComponent<Weapon>();
    gCoordinator.RegisterComponent<Tag>();
    gCoordinator.RegisterComponent<PlayerTag>();
    gCoordinator.RegisterComponent<EnemyTag>();
    gCoordinator.RegisterComponent<ProjectileTag>();
    gCoordinator.RegisterComponent<ScrollingBackground>();
    gCoordinator.RegisterComponent<MovementPattern>();
    gCoordinator.RegisterComponent<Lifetime>();
    gCoordinator.RegisterComponent<Effect>();
    gCoordinator.RegisterComponent<Damage>();
    gCoordinator.RegisterComponent<ChargeAnimation>();
    gCoordinator.RegisterComponent<NetworkId>();
    
    LOG_INFO("ECS", "All components registered (20 types)");
    std::cout << "[Game] Components registered" << std::endl;
    
    // ========================================
    // INITIALIZE ALL SYSTEMS
    // ========================================
    LOG_INFO("ECS", "Initializing systems...");
    std::cout << "🔧 Initializing Systems..." << std::endl;
    
    // Create all systems (pass coordinator pointer)
    auto movementSystem = std::make_shared<MovementSystem>(&gCoordinator);
    auto animationSystem = std::make_shared<AnimationSystem>();
    animationSystem->SetCoordinator(&gCoordinator);
    auto stateMachineAnimSystem = std::make_shared<StateMachineAnimationSystem>(&gCoordinator);
    auto lifetimeSystem = std::make_shared<LifetimeSystem>(&gCoordinator);
    auto movementPatternSystem = std::make_shared<MovementPatternSystem>(&gCoordinator);
    auto scrollingBgSystem = std::make_shared<ScrollingBackgroundSystem>(&gCoordinator);
    auto boundarySystem = std::make_shared<BoundarySystem>();
    boundarySystem->SetCoordinator(&gCoordinator);
    boundarySystem->SetWindowSize(1920.0f, 1080.0f);
    auto collisionSystem = std::make_shared<CollisionSystem>(&gCoordinator);
    auto healthSystem = std::make_shared<HealthSystem>(&gCoordinator);
    
    // Setup collision callback
    collisionSystem->SetCollisionCallback([&](ECS::Entity a, ECS::Entity b) {
        // Get tag names for better logging
        std::string tagA = gCoordinator.HasComponent<Tag>(a) ? gCoordinator.GetComponent<Tag>(a).name : "unknown";
        std::string tagB = gCoordinator.HasComponent<Tag>(b) ? gCoordinator.GetComponent<Tag>(b).name : "unknown";
        LOG_DEBUG("COLLISION", "Entity #" + std::to_string(a) + " (" + tagA + ") <-> Entity #" + std::to_string(b) + " (" + tagB + ")");
        
        // Create explosion at collision point
        if (gCoordinator.HasComponent<Position>(a)) {
            auto& pos = gCoordinator.GetComponent<Position>(a);
            CreateExplosion(pos.x, pos.y);
        }
        
        // Damage entities with health
        if (gCoordinator.HasComponent<Health>(a)) {
            auto& health = gCoordinator.GetComponent<Health>(a);
            health.current -= 1;
            LOG_DEBUG("COMBAT", "Entity #" + std::to_string(a) + " took damage, health: " + std::to_string(health.current) + "/" + std::to_string(health.max));
            if (health.current <= 0 && health.destroyOnDeath) {
                LOG_INFO("COMBAT", "Entity #" + std::to_string(a) + " (" + tagA + ") destroyed!");
                DestroyEntityDeferred(a);
            }
        }
        if (gCoordinator.HasComponent<Health>(b)) {
            auto& health = gCoordinator.GetComponent<Health>(b);
            health.current -= 1;
            LOG_DEBUG("COMBAT", "Entity #" + std::to_string(b) + " took damage, health: " + std::to_string(health.current) + "/" + std::to_string(health.max));
            if (health.current <= 0 && health.destroyOnDeath) {
                LOG_INFO("COMBAT", "Entity #" + std::to_string(b) + " (" + tagB + ") destroyed!");
                DestroyEntityDeferred(b);
            }
        }
        
        // Destroy projectiles on collision
        if (gCoordinator.HasComponent<ProjectileTag>(a)) {
            DestroyEntityDeferred(a);
        }
        if (gCoordinator.HasComponent<ProjectileTag>(b)) {
            DestroyEntityDeferred(b);
        }
    });
    
    // Register systems with ECS and set signatures
    gCoordinator.RegisterSystem<MovementSystem>(&gCoordinator);
    ECS::Signature movementSig;
    movementSig.set(gCoordinator.GetComponentType<Position>());
    movementSig.set(gCoordinator.GetComponentType<Velocity>());
    gCoordinator.SetSystemSignature<MovementSystem>(movementSig);
    
    gCoordinator.RegisterSystem<AnimationSystem>();
    ECS::Signature animSig;
    animSig.set(gCoordinator.GetComponentType<Animation>());
    animSig.set(gCoordinator.GetComponentType<Sprite>());
    gCoordinator.SetSystemSignature<AnimationSystem>(animSig);
    
    gCoordinator.RegisterSystem<StateMachineAnimationSystem>(&gCoordinator);
    ECS::Signature stateMachineSig;
    stateMachineSig.set(gCoordinator.GetComponentType<StateMachineAnimation>());
    stateMachineSig.set(gCoordinator.GetComponentType<Sprite>());
    gCoordinator.SetSystemSignature<StateMachineAnimationSystem>(stateMachineSig);
    
    gCoordinator.RegisterSystem<LifetimeSystem>(&gCoordinator);
    ECS::Signature lifetimeSig;
    lifetimeSig.set(gCoordinator.GetComponentType<Lifetime>());
    gCoordinator.SetSystemSignature<LifetimeSystem>(lifetimeSig);
    
    gCoordinator.RegisterSystem<MovementPatternSystem>();
    ECS::Signature patternSig;
    patternSig.set(gCoordinator.GetComponentType<MovementPattern>());
    patternSig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<MovementPatternSystem>(patternSig);
    
    gCoordinator.RegisterSystem<ScrollingBackgroundSystem>();
    ECS::Signature scrollingSig;
    scrollingSig.set(gCoordinator.GetComponentType<ScrollingBackground>());
    scrollingSig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<ScrollingBackgroundSystem>(scrollingSig);
    
    gCoordinator.RegisterSystem<BoundarySystem>();
    ECS::Signature boundarySig;
    boundarySig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<BoundarySystem>(boundarySig);
    
    gCoordinator.RegisterSystem<CollisionSystem>(&gCoordinator);
    ECS::Signature collisionSig;
    collisionSig.set(gCoordinator.GetComponentType<Position>());
    collisionSig.set(gCoordinator.GetComponentType<Collider>());
    gCoordinator.SetSystemSignature<CollisionSystem>(collisionSig);
    
    gCoordinator.RegisterSystem<HealthSystem>();
    ECS::Signature healthSig;
    healthSig.set(gCoordinator.GetComponentType<Health>());
    gCoordinator.SetSystemSignature<HealthSystem>(healthSig);
    
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
    
    LOG_INFO("ECS", "All systems initialized (10 systems)");
    std::cout << "[Game] All Systems initialized!" << std::endl;
    
    // Network setup
    std::shared_ptr<NetworkClient> networkClient;
    std::shared_ptr<rtype::engine::systems::NetworkSystem> networkSystem;
    
    if (networkMode) {
        try {
            networkClient = std::make_shared<NetworkClient>(serverAddress, serverPort);
            networkSystem = std::make_shared<rtype::engine::systems::NetworkSystem>(&gCoordinator, networkClient);
            
            // Set callback to register new entities
            networkSystem->setEntityCreatedCallback([&](ECS::Entity entity) {
                allEntities.push_back(entity);
                std::cout << "[Game] Registered network entity " << entity << std::endl;
            });
            
            // Set callback for entity destruction (to create explosions)
            networkSystem->setEntityDestroyedCallback([&](ECS::Entity entity, uint32_t networkId) {
                // Get entity position before destroying
                if (gCoordinator.HasComponent<Position>(entity)) {
                    auto& pos = gCoordinator.GetComponent<Position>(entity);
                    
                    // Check if it's an enemy or player (not bullets)
                    if (gCoordinator.HasComponent<Tag>(entity)) {
                        auto& tag = gCoordinator.GetComponent<Tag>(entity);
                        if (tag.name == "Enemy" || tag.name == "Player") {
                            // Create explosion at entity position
                            std::cout << "[Game] Creating explosion at (" << pos.x << ", " << pos.y << ")" << std::endl;
                            CreateExplosion(pos.x, pos.y);
                        }
                    }
                }
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
    LOG_INFO("RENDERING", "Window created (1920x1080)");
    
    SFMLRenderer renderer(&window.getSFMLWindow());
    
    // Initialize profiler overlay
    profilerOverlay.init(); // Will auto-detect system fonts
    profilerOverlay.setNetworkMode(networkMode);
    profilerOverlay.setMode(rtype::core::OverlayMode::COMPACT); // Start with compact mode
    LOG_INFO("PROFILER", "Profiler overlay initialized (F3 to toggle, F4 to cycle modes)");
    
    // Initialize developer console
    rtype::core::DevConsole devConsole;
    devConsole.init();
    LOG_INFO("CONSOLE", "Developer console initialized (F1 or ` to toggle)");
    
    // Load textures (try multiple paths for flexibility)
    LOG_INFO("ASSETS", "Loading textures...");
    backgroundTexture = std::make_unique<SFMLTexture>();
    bool bgLoaded = backgroundTexture->loadFromFile("../../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("client/assets/background.png");
    if (!bgLoaded) {
        LOG_ERROR("ASSETS", "Failed to load background.png");
        std::cerr << "Error: Could not load background.png" << std::endl;
        std::cerr << "Tried paths: ../../client/assets/, ../client/assets/, client/assets/" << std::endl;
        return 1;
    }
    
    playerTexture = std::make_unique<SFMLTexture>();
    bool playerLoaded = playerTexture->loadFromFile("../../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("client/assets/players/r-typesheet42.png");
    if (!playerLoaded) {
        LOG_ERROR("ASSETS", "Failed to load player sprite");
        std::cerr << "Error: Could not load player sprite" << std::endl;
        return 1;
    }
    
    missileTexture = std::make_unique<SFMLTexture>();
    bool missileLoaded = missileTexture->loadFromFile("../../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("client/assets/players/r-typesheet1.png");
    if (!missileLoaded) {
        LOG_ERROR("ASSETS", "Failed to load missile sprite");
        std::cerr << "Error: Could not load missile sprite" << std::endl;
        return 1;
    }
    
    enemyTexture = std::make_unique<SFMLTexture>();
    bool enemyLoaded = enemyTexture->loadFromFile("../../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("client/assets/enemies/r-typesheet5.png");
    if (!enemyLoaded) {
        LOG_ERROR("ASSETS", "Failed to load enemy sprite");
        std::cerr << "Error: Could not load enemy sprite" << std::endl;
        return 1;
    }
    
    explosionTexture = std::make_unique<SFMLTexture>();
    bool explosionLoaded = explosionTexture->loadFromFile("../../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("client/assets/enemies/r-typesheet44.png");
    if (!explosionLoaded) {
        LOG_ERROR("ASSETS", "Failed to load explosion sprite");
        std::cerr << "Error: Could not load explosion sprite" << std::endl;
        return 1;
    }
    
    LOG_INFO("ASSETS", "All textures loaded successfully (5 textures)");
    
    // Load sound
    bool soundLoaded = shootBuffer.loadFromFile("../../client/assets/vfx/shoot.ogg") ||
                       shootBuffer.loadFromFile("../client/assets/vfx/shoot.ogg") ||
                       shootBuffer.loadFromFile("client/assets/vfx/shoot.ogg");
    if (!soundLoaded) {
        std::cerr << "Warning: Could not load shoot.ogg" << std::endl;
    } else {
        shootSound.setBuffer(shootBuffer);
        shootSound.setVolume(80.f);
    }
    
    // Create game entities
    ECS::Entity player = 0;
    if (!networkMode) {
        // Only create player in local mode - server creates it in network mode
        player = CreatePlayer(100.0f, 400.0f);
    }
    CreateBackground(0.0f, 0.0f, 1080.0f, true);
    
    // God mode state (used by console command)
    bool godMode = false;
    
    // Debug visualization states
    bool showHitboxes = false;
    bool showEntityInfo = false;
    bool debugMode = false; // Allows cheat commands even in network mode (for testing)
    
    // ========================================
    // REGISTER GAME-SPECIFIC CONSOLE COMMANDS
    // ========================================
    
    // Debug mode toggle - allows cheats in network mode for testing
    devConsole.registerCommand("debug", "Toggle debug mode (allows cheats in network)", "debug",
        [&](const std::vector<std::string>&) -> std::string {
            debugMode = !debugMode;
            return debugMode ? "Debug mode ON - Cheats enabled (may desync!)" : "Debug mode OFF";
        });
    
    devConsole.registerCommand("spawn", "Spawn an enemy", "spawn [x] [y]",
        [&](const std::vector<std::string>& args) -> std::string {
            if (networkMode && !debugMode) {
                return "Cannot spawn in network mode. Use 'debug' to enable cheats (will desync)";
            }
            float x = 1920.0f;
            float y = 500.0f;
            if (args.size() > 1) x = std::stof(args[1]);
            if (args.size() > 2) y = std::stof(args[2]);
            CreateEnemy(x, y, MovementPattern::Type::STRAIGHT);
            std::string result = "Spawned enemy at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
            if (networkMode) result += " [LOCAL ONLY - DESYNCED]";
            return result;
        });
    
    devConsole.registerCommand("entities", "Show entity count", "entities",
        [&](const std::vector<std::string>&) -> std::string {
            return "Active entities: " + std::to_string(allEntities.size());
        });
    
    devConsole.registerCommand("kill", "Destroy all enemies", "kill",
        [&](const std::vector<std::string>&) -> std::string {
            if (networkMode && !debugMode) {
                return "Cannot kill in network mode. Use 'debug' to enable cheats (will desync)";
            }
            int count = 0;
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<EnemyTag>(entity)) {
                    DestroyEntityDeferred(entity);
                    count++;
                }
            }
            std::string result = "Marked " + std::to_string(count) + " enemies for destruction";
            if (networkMode) result += " [LOCAL ONLY - DESYNCED]";
            return result;
        });
    
    devConsole.registerCommand("god", "Toggle god mode (invincibility)", "god",
        [&](const std::vector<std::string>&) -> std::string {
            godMode = !godMode;
            // Find player entity (works in both modes)
            ECS::Entity playerEntity = 0;
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<PlayerTag>(entity)) {
                    playerEntity = entity;
                    break;
                }
            }
            if (playerEntity != 0 && gCoordinator.HasComponent<Health>(playerEntity)) {
                auto& health = gCoordinator.GetComponent<Health>(playerEntity);
                if (godMode) {
                    health.current = 99999;
                    health.max = 99999;
                } else {
                    health.current = 100;
                    health.max = 100;
                }
            }
            std::string result = godMode ? "God mode ON" : "God mode OFF";
            if (networkMode) result += " [LOCAL - server may override]";
            return result;
        });
    
    devConsole.registerCommand("spawn_wave", "Spawn a wave of enemies", "spawn_wave [count]",
        [&](const std::vector<std::string>& args) -> std::string {
            if (networkMode && !debugMode) {
                return "Cannot spawn in network mode. Use 'debug' to enable cheats (will desync)";
            }
            int count = 5;
            if (args.size() > 1) count = std::stoi(args[1]);
            MovementPattern::Type patterns[] = {
                MovementPattern::Type::STRAIGHT,
                MovementPattern::Type::SINE_WAVE,
                MovementPattern::Type::ZIGZAG,
                MovementPattern::Type::DIAGONAL_DOWN,
                MovementPattern::Type::DIAGONAL_UP
            };
            for (int i = 0; i < count; i++) {
                float y = 100.0f + (i * (800.0f / count));
                CreateEnemy(1920.0f + 50.0f + (i * 50.0f), y, patterns[i % 5]);
            }
            std::string result = "Spawned wave of " + std::to_string(count) + " enemies";
            if (networkMode) result += " [LOCAL ONLY - DESYNCED]";
            return result;
        });
    
    devConsole.registerCommand("teleport", "Teleport player", "teleport <x> <y>",
        [&](const std::vector<std::string>& args) -> std::string {
            if (networkMode && !debugMode) {
                return "Cannot teleport in network mode. Use 'debug' to enable cheats";
            }
            if (args.size() < 3) {
                return "Usage: teleport <x> <y>";
            }
            float x = std::stof(args[1]);
            float y = std::stof(args[2]);
            // Find player entity
            ECS::Entity playerEntity = 0;
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<PlayerTag>(entity)) {
                    playerEntity = entity;
                    break;
                }
            }
            if (playerEntity != 0 && gCoordinator.HasComponent<Position>(playerEntity)) {
                auto& pos = gCoordinator.GetComponent<Position>(playerEntity);
                pos.x = x;
                pos.y = y;
                std::string result = "Teleported to (" + std::to_string(x) + ", " + std::to_string(y) + ")";
                if (networkMode) result += " [LOCAL - will rubber-band]";
                return result;
            }
            return "No player to teleport";
        });
    
    devConsole.registerCommand("network", "Show network status", "network",
        [&](const std::vector<std::string>&) -> std::string {
            if (!networkMode) {
                return "Not in network mode";
            }
            auto& stats = profiler.getNetworkStats();
            std::ostringstream ss;
            ss << "Network: " << stats.packetsSent << " sent, " << stats.packetsReceived << " recv, "
               << std::fixed << std::setprecision(1) << stats.latencyMs << "ms latency";
            return ss.str();
        });
    
    // Show hitboxes (works in all modes - visual only)
    devConsole.registerCommand("hitboxes", "Toggle hitbox visualization", "hitboxes",
        [&](const std::vector<std::string>&) -> std::string {
            showHitboxes = !showHitboxes;
            return showHitboxes ? "Hitboxes visible" : "Hitboxes hidden";
        });
    
    // Show entity info (works in all modes - visual only)
    devConsole.registerCommand("entityinfo", "Toggle entity info display", "entityinfo",
        [&](const std::vector<std::string>&) -> std::string {
            showEntityInfo = !showEntityInfo;
            return showEntityInfo ? "Entity info visible" : "Entity info hidden";
        });
    
    // List all entities with details (works in all modes)
    devConsole.registerCommand("list", "List all entities", "list [type]",
        [&](const std::vector<std::string>& args) -> std::string {
            std::ostringstream ss;
            std::string filter = args.size() > 1 ? args[1] : "";
            int count = 0;
            
            for (auto entity : allEntities) {
                std::string tag = "unknown";
                if (gCoordinator.HasComponent<Tag>(entity)) {
                    tag = gCoordinator.GetComponent<Tag>(entity).name;
                }
                
                // Filter if specified
                if (!filter.empty() && tag.find(filter) == std::string::npos) {
                    continue;
                }
                
                ss << "#" << entity << " [" << tag << "]";
                
                if (gCoordinator.HasComponent<Position>(entity)) {
                    auto& pos = gCoordinator.GetComponent<Position>(entity);
                    ss << " pos(" << (int)pos.x << "," << (int)pos.y << ")";
                }
                if (gCoordinator.HasComponent<Health>(entity)) {
                    auto& hp = gCoordinator.GetComponent<Health>(entity);
                    ss << " hp:" << hp.current << "/" << hp.max;
                }
                ss << "\n";
                count++;
                
                if (count >= 20) {
                    ss << "... and " << (allEntities.size() - 20) << " more\n";
                    break;
                }
            }
            
            if (count == 0) {
                return "No entities found" + (filter.empty() ? "" : " matching '" + filter + "'");
            }
            return ss.str();
        });
    
    // Player info (works in all modes)
    devConsole.registerCommand("player", "Show player info", "player",
        [&](const std::vector<std::string>&) -> std::string {
            std::ostringstream ss;
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<PlayerTag>(entity)) {
                    ss << "Player Entity #" << entity << "\n";
                    if (gCoordinator.HasComponent<Position>(entity)) {
                        auto& pos = gCoordinator.GetComponent<Position>(entity);
                        ss << "  Position: (" << pos.x << ", " << pos.y << ")\n";
                    }
                    if (gCoordinator.HasComponent<Velocity>(entity)) {
                        auto& vel = gCoordinator.GetComponent<Velocity>(entity);
                        ss << "  Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
                    }
                    if (gCoordinator.HasComponent<Health>(entity)) {
                        auto& hp = gCoordinator.GetComponent<Health>(entity);
                        ss << "  Health: " << hp.current << "/" << hp.max << "\n";
                    }
                    if (gCoordinator.HasComponent<NetworkId>(entity)) {
                        auto& netId = gCoordinator.GetComponent<NetworkId>(entity);
                        ss << "  Network ID: " << netId.id << (netId.isLocal ? " (local)" : "") << "\n";
                    }
                }
            }
            if (ss.str().empty()) {
                return "No player entity found";
            }
            return ss.str();
        });
    
    // Set time scale (visual slowmo - works in all modes for debugging)
    devConsole.registerCommand("timescale", "Set time scale (0.1-2.0)", "timescale <value>",
        [&](const std::vector<std::string>& args) -> std::string {
            static float timeScale = 1.0f;
            if (args.size() < 2) {
                return "Current time scale: " + std::to_string(timeScale);
            }
            timeScale = std::clamp(std::stof(args[1]), 0.1f, 2.0f);
            return "Time scale set to " + std::to_string(timeScale);
        });
    
    // Mode info
    devConsole.registerCommand("mode", "Show current game mode", "mode",
        [&](const std::vector<std::string>&) -> std::string {
            std::ostringstream ss;
            ss << "Game Mode: " << (networkMode ? "NETWORK" : "LOCAL") << "\n";
            ss << "Debug Mode: " << (debugMode ? "ON" : "OFF") << "\n";
            ss << "God Mode: " << (godMode ? "ON" : "OFF") << "\n";
            ss << "Entities: " << allEntities.size();
            return ss.str();
        });
    
    // Game variables using engine abstractions
    rtype::engine::Clock clock;
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 2.0f;
    
    bool spacePressed = false;
    float spaceHoldTime = 0.0f;
    const float chargeStartTime = 0.1f;
    ECS::Entity activeChargingEffect = 0;
    bool hasChargingEffect = false;
    
    std::cout << "Game initialized successfully!" << std::endl;
    LOG_INFO("GAME", "Game initialization complete");
    
    // Input mask for network
    uint8_t inputMask = 0;
    
    // Track entities we've added sprites to
    std::set<ECS::Entity> entitiesWithSprites;
    
    // Frame stats for periodic logging
    int frameCount = 0;
    float frameTimeAccum = 0.0f;
    float statsInterval = 5.0f; // Log stats every 5 seconds
    
    // ========================================
    // MAIN GAME LOOP (System-Driven)
    // ========================================
    std::cout << "[Game] Starting game loop..." << std::endl;
    LOG_INFO("GAMELOOP", "Entering main game loop");
    
    while (window.isOpen()) {
        // Start profiler frame
        profiler.beginFrame();
        
        float deltaTime = clock.restart();
        frameCount++;
        frameTimeAccum += deltaTime;
        
        // Update profiler metrics
        profiler.setEntityCount(allEntities.size());
        profiler.updateMemoryUsage();
        
        // Periodic stats logging
        if (frameTimeAccum >= statsInterval) {
            float avgFps = frameCount / frameTimeAccum;
            LOG_INFO("PERF", "Stats: " + std::to_string(avgFps) + " FPS, " + std::to_string(allEntities.size()) + " entities");
            frameCount = 0;
            frameTimeAccum = 0.0f;
        }
        
        // Cap deltaTime to prevent huge jumps (max 0.1s = 10 FPS minimum)
        if (deltaTime > 0.1f) {
            LOG_WARNING("PERF", "Frame time spike: " + std::to_string(deltaTime) + "s - capping to 0.1s");
            deltaTime = 0.1f;
        }
        
        // ========================================
        // 1. NETWORK UPDATE (Receives server state)
        // ========================================
        profiler.beginSection("Network");
        if (networkMode && networkSystem) {
            networkSystem->Update(deltaTime);
            
            // Add sprites to network entities that don't have them
            for (auto entity : allEntities) {
                if (entitiesWithSprites.find(entity) == entitiesWithSprites.end() &&
                    gCoordinator.HasComponent<NetworkId>(entity) &&
                    gCoordinator.HasComponent<Position>(entity) &&
                    gCoordinator.HasComponent<Tag>(entity)) {
                    
                    auto& tag = gCoordinator.GetComponent<Tag>(entity);
                    auto& pos = gCoordinator.GetComponent<Position>(entity);
                    auto& networkId = gCoordinator.GetComponent<NetworkId>(entity);
                    
                    // Create sprite based on tag - with proper visuals
                    auto* sprite = new SFMLSprite();
                    allSprites.push_back(sprite);
                    
                    Sprite spriteComp;
                    spriteComp.sprite = sprite;
                    spriteComp.layer = 10;
                    spriteComp.scaleX = 3.0f;
                    spriteComp.scaleY = 3.0f;
                    
                    if (tag.name == "Player") {
                        // Player sprite
                        sprite->setTexture(playerTexture.get());
                        int line = networkId.playerLine; // Use the server-assigned ship color
                        IntRect rect(33 * 2, line * 17, 33, 17);
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        
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
                        // Enemy sprite
                        sprite->setTexture(enemyTexture.get());
                        IntRect rect(0, 3 * 33, 32, 32);
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        
                        // Add animation for enemy
                        Animation anim;
                        anim.frameCount = 2;
                        anim.currentFrame = 0;
                        anim.frameTime = 0.2f;
                        anim.loop = true;
                        anim.frameWidth = 32;
                        anim.frameHeight = 32;
                        anim.startX = 0;
                        anim.startY = 3 * 33;
                        gCoordinator.AddComponent(entity, anim);
                        
                    } else if (tag.name == "PlayerBullet" || tag.name == "bullet" || tag.name == "charged_bullet") {
                        // Check if it's a charged bullet based on size or other properties
                        sprite->setTexture(missileTexture.get());
                        
                        // Normal missile
                        IntRect rect(232, 103, 16, 12);
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.scaleX = 2.0f;
                        spriteComp.scaleY = 2.0f;
                        
                    } else if (tag.name == "EnemyBullet") {
                        // Enemy missile sprite
                        sprite->setTexture(enemyTexture.get());
                        IntRect rect(0, 0, 16, 16); // Use enemy bullet sprite
                        sprite->setTextureRect(rect);
                        spriteComp.textureRect = rect;
                        spriteComp.scaleX = 2.0f;
                        spriteComp.scaleY = 2.0f;
                    }
                    
                    sprite->setPosition(Vector2f(pos.x, pos.y));
                    
                    gCoordinator.AddComponent(entity, spriteComp);
                    entitiesWithSprites.insert(entity);
                    
                    std::cout << "[Game] Added sprite to network entity " << entity << " (" << tag.name << ")" << std::endl;
                }
            }
        }
        profiler.endSection("Network");
        
        // Reset input mask
        inputMask = 0;
        
        // Event handling using SFML events (needed for DevConsole)
        profiler.beginSection("Input");
        
        // Update DevConsole
        devConsole.update(deltaTime);
        
        // Process SFML events for DevConsole and game
        sf::Event sfEvent;
        while (window.pollEventSFML(sfEvent)) {
            // Let DevConsole handle events first
            if (devConsole.handleEvent(sfEvent)) {
                // Console consumed the event, skip game input
                continue;
            }
            
            // Handle profiler overlay controls
            if (sfEvent.type == sf::Event::KeyPressed) {
                if (sfEvent.key.code == sf::Keyboard::F3) {
                    profilerOverlay.toggle();
                    LOG_DEBUG("PROFILER", "Overlay toggled");
                } else if (sfEvent.key.code == sf::Keyboard::F4) {
                    profilerOverlay.cycleMode();
                    LOG_DEBUG("PROFILER", "Overlay mode cycled");
                }
            }
            
            if (sfEvent.type == sf::Event::Closed) {
                LOG_INFO("GAME", "Window close requested");
                window.close();
            }
            
            // Handle space key release for shooting (only when console is closed)
            if (!devConsole.isOpen() && sfEvent.type == sf::Event::KeyReleased && sfEvent.key.code == sf::Keyboard::Space) {
                if (spacePressed && gCoordinator.HasComponent<Position>(player)) {
                    auto& playerPos = gCoordinator.GetComponent<Position>(player);
                    
                    int chargeLevel = 0;
                    if (hasChargingEffect && spaceHoldTime >= chargeStartTime) {
                        // Calculate charge level based on hold time
                        float chargeProgress = (spaceHoldTime - chargeStartTime) / 0.8f;
                        if (chargeProgress < 0.2f) chargeLevel = 1;
                        else if (chargeProgress < 0.4f) chargeLevel = 2;
                        else if (chargeProgress < 0.6f) chargeLevel = 3;
                        else if (chargeProgress < 0.8f) chargeLevel = 4;
                        else chargeLevel = 5;
                    }
                    
                    if (chargeLevel > 0) {
                        // Create charged missile
                        LOG_INFO("INPUT", "Player fired charged shot (level " + std::to_string(chargeLevel) + ")");
                        CreateMissile(playerPos.x + 99.0f, playerPos.y + 25.0f, true, chargeLevel);
                    } else {
                        // Create normal missile
                        LOG_DEBUG("INPUT", "Player fired normal shot");
                        CreateMissile(playerPos.x + 99.0f, playerPos.y + 30.0f, false, 0);
                        
                        // Play shoot sound using engine abstraction
                        // Stop the sound if it's playing, then play it again for rapid fire
                        shootSound.stop();
                        shootSound.play();
                        
                        // Create shoot effect
                        CreateShootEffect(playerPos.x + 89.0f, playerPos.y + 10.0f, player);
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
        
        // Handle continuous input (only when console is closed)
        if (!devConsole.isOpen() && rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Space)) {
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
                
                Effect effectTag;
                effectTag.effectType = Effect::Type::CHARGE;
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
        // 2. INPUT CAPTURE & NETWORK SEND
        // ========================================
        bool movingUp = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Up);
        bool movingDown = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Down);
        bool movingLeft = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Left);
        bool movingRight = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Right);
        bool firing = spacePressed;
        
        // Build input mask for network (bit flags from Protocol.md)
        inputMask = 0;
        if (movingUp) inputMask |= (1 << 0);
        if (movingDown) inputMask |= (1 << 1);
        if (movingLeft) inputMask |= (1 << 2);
        if (movingRight) inputMask |= (1 << 3);
        if (firing) inputMask |= (1 << 4);
        
        // Send input to server if in network mode
        if (networkMode && networkSystem) {
            networkSystem->sendInput(inputMask);
        }
        
        // ========================================
        // 3. LOCAL PLAYER INPUT (Only in local mode - manual)
        // ========================================
        if (!networkMode && player != 0 && gCoordinator.HasComponent<Velocity>(player)) {
            auto& playerVel = gCoordinator.GetComponent<Velocity>(player);
            float speed = 500.0f;
            playerVel.vx = 0.0f;
            playerVel.vy = 0.0f;
            
            if (movingUp) playerVel.vy = -speed;
            if (movingDown) playerVel.vy = speed;
            if (movingLeft) playerVel.vx = -speed;
            if (movingRight) playerVel.vx = speed;
            
            // Update animation target
            if (gCoordinator.HasComponent<StateMachineAnimation>(player)) {
                auto& playerAnim = gCoordinator.GetComponent<StateMachineAnimation>(player);
                if (movingUp) playerAnim.targetColumn = 4;
                else if (movingDown) playerAnim.targetColumn = 0;
                else playerAnim.targetColumn = 2;
            }
        }
        profiler.endSection("Input");
        
        // ========================================
        // 4. LOCAL ENEMY SPAWNING (Only in local mode)
        // ========================================
        if (!networkMode) {
            enemySpawnTimer += deltaTime;
            if (enemySpawnTimer >= enemySpawnInterval) {
                enemySpawnTimer = 0.0f;
                float spawnY = 100.0f + (rand() % 800);
                MovementPattern::Type patterns[] = {
                    MovementPattern::Type::STRAIGHT,
                    MovementPattern::Type::SINE_WAVE,
                    MovementPattern::Type::ZIGZAG,
                    MovementPattern::Type::CIRCULAR,
                    MovementPattern::Type::DIAGONAL_DOWN,
                    MovementPattern::Type::DIAGONAL_UP
                };
                CreateEnemy(1920.0f + 50.0f, spawnY, patterns[rand() % 6]);
            }
        }
        
        // ========================================
        // 5. SYSTEM UPDATES (ECS Architecture!)
        // ========================================
        profiler.beginSection("Systems");
        
        // Always update scrolling background
        scrollingBgSystem->Update(deltaTime);
        
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
        profiler.endSection("Systems");
        
        // Process destroyed entities
        ProcessDestroyedEntities();
        
        // Render
        profiler.beginSection("Rendering");
        profiler.resetDrawCalls();
        window.clear();
        
        // Manual render (sort by layer)
        std::vector<ECS::Entity> renderableEntities;
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<Position>(entity) && gCoordinator.HasComponent<Sprite>(entity)) {
                renderableEntities.push_back(entity);
            }
        }
        
        std::sort(renderableEntities.begin(), renderableEntities.end(), [&](ECS::Entity a, ECS::Entity b) {
            auto& spriteA = gCoordinator.GetComponent<Sprite>(a);
            auto& spriteB = gCoordinator.GetComponent<Sprite>(b);
            return spriteA.layer < spriteB.layer;
        });
        
        for (auto entity : renderableEntities) {
            auto& pos = gCoordinator.GetComponent<Position>(entity);
            auto& sprite = gCoordinator.GetComponent<Sprite>(entity);
            
            if (sprite.sprite) {
                sprite.sprite->setPosition(Vector2f(pos.x, pos.y));
                
                Transform transform;
                transform.position = Vector2f(pos.x, pos.y);
                transform.rotation = 0.0f;
                transform.scale = Vector2f(sprite.scaleX, sprite.scaleY);
                
                renderer.draw(*sprite.sprite, transform);
                profiler.addDrawCall();
            }
        }
        profiler.endSection("Rendering");
        
        // Update and render profiler overlay
        profilerOverlay.update();
        profilerOverlay.render(window.getSFMLWindow());
        
        // Render developer console (on top of everything)
        devConsole.render(window.getSFMLWindow());
        
        // End profiler frame
        profiler.endFrame();
        
        window.display();
    }
    
    // Cleanup
    profiler.logReport();
    profiler.shutdown();
    
    for (auto sprite : allSprites) {
        delete sprite;
    }
    allSprites.clear();
    
    gCoordinator.Shutdown();
    
    LOG_INFO("GAME", "Game shutdown complete.");
    logger.shutdown();
    
    std::cout << "Game shutdown complete." << std::endl;
    return 0;
}
