#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <set>

// Engine includes - Core
#include <ecs/ECS.hpp>
#include <ecs/Coordinator.hpp>

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
}

void ProcessDestroyedEntities() {
    for (auto entity : entitiesToDestroy) {
        // Clean up sprite if exists
        if (gCoordinator.HasComponent<Sprite>(entity)) {
            auto& sprite = gCoordinator.GetComponent<Sprite>(entity);
            if (sprite.sprite) {
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
ECS::Entity CreatePlayer(float x, float y, int line = 0) {
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
    anim.frameTime = 0.1f;
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
    lifetime.maxLifetime = 0.6f;
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
    std::cout << "R-Type Game Starting with ECS Engine..." << std::endl;
    
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
        std::cout << "Network mode enabled. Server: " << serverAddress << ":" << serverPort << std::endl;
    } else {
        std::cout << "Local mode (use --network <ip> <port> for multiplayer)" << std::endl;
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
    
    // Network setup
    std::shared_ptr<NetworkClient> networkClient;
    std::shared_ptr<rtype::engine::systems::NetworkSystem> networkSystem;
    
    if (networkMode) {
        try {
            networkClient = std::make_shared<NetworkClient>(serverAddress, serverPort);
            networkSystem = std::make_shared<rtype::engine::systems::NetworkSystem>(&gCoordinator, networkClient);
            
            // Set callback to register new entities
            networkSystem->setEntityCreatedCallback([](ECS::Entity entity) {
                allEntities.push_back(entity);
                std::cout << "[Game] Registered network entity " << entity << std::endl;
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
    
    // Load textures (try multiple paths for flexibility)
    backgroundTexture = std::make_unique<SFMLTexture>();
    bool bgLoaded = backgroundTexture->loadFromFile("../../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("client/assets/background.png");
    if (!bgLoaded) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        std::cerr << "Tried paths: ../../client/assets/, ../client/assets/, client/assets/" << std::endl;
        return 1;
    }
    
    playerTexture = std::make_unique<SFMLTexture>();
    bool playerLoaded = playerTexture->loadFromFile("../../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("client/assets/players/r-typesheet42.png");
    if (!playerLoaded) {
        std::cerr << "Error: Could not load player sprite" << std::endl;
        return 1;
    }
    
    missileTexture = std::make_unique<SFMLTexture>();
    bool missileLoaded = missileTexture->loadFromFile("../../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("client/assets/players/r-typesheet1.png");
    if (!missileLoaded) {
        std::cerr << "Error: Could not load missile sprite" << std::endl;
        return 1;
    }
    
    enemyTexture = std::make_unique<SFMLTexture>();
    bool enemyLoaded = enemyTexture->loadFromFile("../../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("client/assets/enemies/r-typesheet5.png");
    if (!enemyLoaded) {
        std::cerr << "Error: Could not load enemy sprite" << std::endl;
        return 1;
    }
    
    explosionTexture = std::make_unique<SFMLTexture>();
    bool explosionLoaded = explosionTexture->loadFromFile("../../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("client/assets/enemies/r-typesheet44.png");
    if (!explosionLoaded) {
        std::cerr << "Error: Could not load explosion sprite" << std::endl;
        return 1;
    }
    
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
    
    // Input mask for network
    uint8_t inputMask = 0;
    
    // Track entities we've added sprites to
    std::set<ECS::Entity> entitiesWithSprites;
    
    // Game loop
    while (window.isOpen()) {
        float deltaTime = clock.restart();
        
        // Update network system
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
                        int line = networkId.playerId % 4; // Different colors for different players
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
                        anim.currentTime = 0.0f;
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
                    }
                    
                    sprite->setPosition(Vector2f(pos.x, pos.y));
                    
                    gCoordinator.AddComponent(entity, spriteComp);
                    entitiesWithSprites.insert(entity);
                    
                    std::cout << "[Game] Added sprite to network entity " << entity << " (" << tag.name << ")" << std::endl;
                }
            }
        }
        
        // Reset input mask
        inputMask = 0;
        
        // Event handling using engine abstractions
        rtype::engine::InputEvent event;
        while (window.pollEvent(event)) {
            if (event.type == rtype::engine::EventType::Closed) {
                window.close();
            }
            
            // Handle space key release for shooting
            if (event.type == rtype::engine::EventType::KeyReleased && event.key.code == rtype::engine::Key::Space) {
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
        
        // Handle continuous input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
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
        
        // Capture input
        bool movingUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        bool movingDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
        bool movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        bool movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
        bool firing = spacePressed;
        
        // Build input mask for network (bit flags from Protocol.md)
        if (movingUp) inputMask |= (1 << 0);
        if (movingDown) inputMask |= (1 << 1);
        if (movingLeft) inputMask |= (1 << 2);
        if (movingRight) inputMask |= (1 << 3);
        if (firing) inputMask |= (1 << 4);
        
        // Send input to server if in network mode
        if (networkMode && networkSystem && inputMask != 0) {
            networkSystem->sendInput(inputMask);
        }
        
        // Handle player input (only in local mode)
        if (!networkMode && gCoordinator.HasComponent<Velocity>(player) && gCoordinator.HasComponent<Position>(player)) {
            if (movingUp || movingDown || movingLeft || movingRight) {
                
                auto& playerVel = gCoordinator.GetComponent<Velocity>(player);
                auto& playerPos = gCoordinator.GetComponent<Position>(player);
                
                float speed = 500.0f;
                playerVel.vx = 0.0f;
                playerVel.vy = 0.0f;
                
                if (gCoordinator.HasComponent<StateMachineAnimation>(player)) {
                    auto& playerAnim = gCoordinator.GetComponent<StateMachineAnimation>(player);
                    
                    if (movingUp) {
                        playerVel.vy = -speed;
                        playerAnim.targetColumn = 4;
                    } else if (movingDown) {
                        playerVel.vy = speed;
                        playerAnim.targetColumn = 0;
                    } else {
                        playerAnim.targetColumn = 2;
                    }
                    
                    if (movingLeft) {
                        playerVel.vx = -speed;
                    }
                    if (movingRight) {
                        playerVel.vx = speed;
                    }
                    
                    // Update animation
                    playerAnim.transitionTime += deltaTime;
                    if (playerAnim.currentColumn != playerAnim.targetColumn && 
                        playerAnim.transitionTime >= playerAnim.transitionSpeed) {
                        playerAnim.transitionTime = 0.0f;
                        
                        if (playerAnim.currentColumn < playerAnim.targetColumn) {
                            playerAnim.currentColumn++;
                        } else if (playerAnim.currentColumn > playerAnim.targetColumn) {
                            playerAnim.currentColumn--;
                        }
                        
                        if (gCoordinator.HasComponent<Sprite>(player)) {
                            auto& sprite = gCoordinator.GetComponent<Sprite>(player);
                            IntRect rect(33 * playerAnim.currentColumn, playerAnim.currentRow * 17, 33, 17);
                            if (sprite.sprite) {
                                sprite.sprite->setTextureRect(rect);
                            }
                        }
                    }
                }
                
                // Apply movement
                playerPos.x += playerVel.vx * deltaTime;
                playerPos.y += playerVel.vy * deltaTime;
                
                // Boundary check
                if (playerPos.x < 0) playerPos.x = 0;
                if (playerPos.y < 0) playerPos.y = 0;
                if (playerPos.x + 99.0f > 1920) playerPos.x = 1920 - 99.0f;
                if (playerPos.y + 51.0f > 1080) playerPos.y = 1080 - 51.0f;
            } else {
                auto& playerVel = gCoordinator.GetComponent<Velocity>(player);
                playerVel.vx = 0.0f;
                playerVel.vy = 0.0f;
                
                if (gCoordinator.HasComponent<StateMachineAnimation>(player)) {
                    auto& playerAnim = gCoordinator.GetComponent<StateMachineAnimation>(player);
                    playerAnim.targetColumn = 2;
                    
                    // Continue animation transition even when not moving
                    playerAnim.transitionTime += deltaTime;
                    if (playerAnim.currentColumn != playerAnim.targetColumn && 
                        playerAnim.transitionTime >= playerAnim.transitionSpeed) {
                        playerAnim.transitionTime = 0.0f;
                        
                        if (playerAnim.currentColumn < playerAnim.targetColumn) {
                            playerAnim.currentColumn++;
                        } else if (playerAnim.currentColumn > playerAnim.targetColumn) {
                            playerAnim.currentColumn--;
                        }
                        
                        if (gCoordinator.HasComponent<Sprite>(player)) {
                            auto& sprite = gCoordinator.GetComponent<Sprite>(player);
                            IntRect rect(33 * playerAnim.currentColumn, playerAnim.currentRow * 17, 33, 17);
                            if (sprite.sprite) {
                                sprite.sprite->setTextureRect(rect);
                            }
                        }
                    }
                }
            }
        }
        
        // Enemy spawning (only in local mode)
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
                int patternIndex = rand() % 6;
                
                CreateEnemy(1920.0f + 50.0f, spawnY, patterns[patternIndex]);
            }
        }
        
        // Update scrolling background (always, even in network mode)
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<ScrollingBackground>(entity) && 
                gCoordinator.HasComponent<Position>(entity)) {
                auto& scrolling = gCoordinator.GetComponent<ScrollingBackground>(entity);
                auto& pos = gCoordinator.GetComponent<Position>(entity);
                
                pos.x -= scrolling.scrollSpeed * deltaTime;
                
                if (pos.x + scrolling.spriteWidth < 0) {
                    pos.x = scrolling.spriteWidth;
                }
            }
        }
        
        // Skip physics updates in network mode (server handles it)
        if (!networkMode) {
            // Update movement patterns
            for (auto entity : allEntities) {
                if (gCoordinator.HasComponent<MovementPattern>(entity) && 
                gCoordinator.HasComponent<Position>(entity)) {
                auto& pattern = gCoordinator.GetComponent<MovementPattern>(entity);
                auto& pos = gCoordinator.GetComponent<Position>(entity);
                
                pattern.timeAlive += deltaTime;
                
                switch (pattern.pattern) {
                    case MovementPattern::Type::STRAIGHT:
                        pos.x -= pattern.speed * deltaTime;
                        break;
                    case MovementPattern::Type::SINE_WAVE:
                        pos.x -= pattern.speed * deltaTime;
                        pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
                        break;
                    case MovementPattern::Type::ZIGZAG:
                        pos.x -= pattern.speed * deltaTime;
                        pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive * 2);
                        break;
                    case MovementPattern::Type::CIRCULAR:
                        pos.x -= pattern.speed * deltaTime * 0.5f;
                        pos.x += pattern.amplitude * 0.3f * std::cos(pattern.frequency * pattern.timeAlive);
                        pos.y = pattern.startY + pattern.amplitude * std::sin(pattern.frequency * pattern.timeAlive);
                        break;
                    case MovementPattern::Type::DIAGONAL_DOWN:
                        pos.x -= pattern.speed * deltaTime;
                        pos.y += pattern.speed * deltaTime * 0.5f;
                        break;
                    case MovementPattern::Type::DIAGONAL_UP:
                        pos.x -= pattern.speed * deltaTime;
                        pos.y -= pattern.speed * deltaTime * 0.5f;
                        break;
                }
                
                // Boundary
                if (pos.y < 0) pos.y = 0;
                if (pos.y + 80.0f > 1080) pos.y = 1080 - 80.0f;
                
                // Destroy if off-screen
                if (pos.x < -100) {
                    DestroyEntityDeferred(entity);
                }
            }
        }
        
        // Update velocity-based movement
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<Velocity>(entity) && 
                gCoordinator.HasComponent<Position>(entity) &&
                !gCoordinator.HasComponent<MovementPattern>(entity)) {  // Don't update if has movement pattern
                auto& vel = gCoordinator.GetComponent<Velocity>(entity);
                auto& pos = gCoordinator.GetComponent<Position>(entity);
                
                pos.x += vel.vx * deltaTime;
                pos.y += vel.vy * deltaTime;
                
                // Destroy projectiles off-screen
                if (gCoordinator.HasComponent<ProjectileTag>(entity)) {
                    if (pos.x > 2000 || pos.x < -100 || pos.y > 1200 || pos.y < -100) {
                        DestroyEntityDeferred(entity);
                    }
                }
            }
        }
        
        // Update animations
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<Animation>(entity) && 
                gCoordinator.HasComponent<Sprite>(entity)) {
                auto& anim = gCoordinator.GetComponent<Animation>(entity);
                auto& sprite = gCoordinator.GetComponent<Sprite>(entity);
                
                anim.currentTime += deltaTime;
                
                if (anim.currentTime >= anim.frameTime) {
                    anim.currentTime = 0.0f;
                    anim.currentFrame++;
                    
                    if (anim.currentFrame >= anim.frameCount) {
                        if (anim.loop) {
                            anim.currentFrame = 0;
                        } else {
                            anim.finished = true;
                            anim.currentFrame = anim.frameCount - 1;
                        }
                    }
                    
                    IntRect newRect(
                        anim.startX + (anim.currentFrame * anim.spacing),
                        anim.startY,
                        anim.frameWidth,
                        anim.frameHeight
                    );
                    if (sprite.sprite) {
                        sprite.sprite->setTextureRect(newRect);
                    }
                }
            }
        }
        
        // Update lifetimes
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<Lifetime>(entity)) {
                auto& lifetime = gCoordinator.GetComponent<Lifetime>(entity);
                lifetime.timeAlive += deltaTime;
                
                if (lifetime.timeAlive >= lifetime.maxLifetime && lifetime.destroyOnExpire) {
                    DestroyEntityDeferred(entity);
                }
            }
        }
        
        // Simple collision detection
        std::vector<ECS::Entity> bullets;
        std::vector<ECS::Entity> enemies;
        
        for (auto entity : allEntities) {
            if (gCoordinator.HasComponent<Tag>(entity)) {
                auto& tag = gCoordinator.GetComponent<Tag>(entity);
                if (tag.name == "bullet" || tag.name == "charged_bullet") {
                    bullets.push_back(entity);
                } else if (tag.name == "enemy") {
                    enemies.push_back(entity);
                }
            }
        }
        
        for (auto bullet : bullets) {
            if (!gCoordinator.HasComponent<Position>(bullet) || !gCoordinator.HasComponent<Collider>(bullet))
                continue;
                
            auto& bulletPos = gCoordinator.GetComponent<Position>(bullet);
            auto& bulletCol = gCoordinator.GetComponent<Collider>(bullet);
            
            bool bulletDestroyed = false;
            
            for (auto enemy : enemies) {
                if (!gCoordinator.HasComponent<Position>(enemy) || !gCoordinator.HasComponent<Collider>(enemy))
                    continue;
                    
                auto& enemyPos = gCoordinator.GetComponent<Position>(enemy);
                auto& enemyCol = gCoordinator.GetComponent<Collider>(enemy);
                
                // Simple AABB collision
                bool collision = (bulletPos.x < enemyPos.x + enemyCol.width &&
                                bulletPos.x + bulletCol.width > enemyPos.x &&
                                bulletPos.y < enemyPos.y + enemyCol.height &&
                                bulletPos.y + bulletCol.height > enemyPos.y);
                
                if (collision) {
                    // Create explosion
                    CreateExplosion(enemyPos.x, enemyPos.y);
                    
                    // Destroy both
                    DestroyEntityDeferred(bullet);
                    DestroyEntityDeferred(enemy);
                    bulletDestroyed = true;
                    break;
                }
            }
            
            if (bulletDestroyed) break;
        }
        
        } // End of !networkMode
        
        // Process destroyed entities
        ProcessDestroyedEntities();
        
        // Render
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
            }
        }
        
        window.display();
    }
    
    // Cleanup
    for (auto sprite : allSprites) {
        delete sprite;
    }
    allSprites.clear();
    
    gCoordinator.Shutdown();
    
    std::cout << "Game shutdown complete." << std::endl;
    return 0;
}
