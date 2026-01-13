#include "Game.hpp"

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
    sprite->setTexture(enemyTexture.get());
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
    IntRect rect(130, 1, 33, 32);  // Match animation parameters
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
    anim.frameTime = 0.08f; // Faster animation for smoother effect
    anim.currentFrame = 0;
    anim.frameCount = 6;   // Try 5 frames instead of 6
    anim.loop = false;
    anim.frameWidth = 32;  // Adjusted frame width
    anim.frameHeight = 32; // Adjusted frame height
    anim.startX = 130;     // Adjusted start position
    anim.startY = 1;       // Slight Y offset
    anim.spacing = 1.5;     // Spacing between explosion frames
    gCoordinator.AddComponent(explosion, anim);

    // Lifetime (destroy after animation finishes)
    std::cout << "[CreateExplosion] Adding Lifetime component..." << std::endl;
    Lifetime lifetime;
    lifetime.maxLifetime = 0.05f; // Very short lifetime
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
    std::shared_ptr<CollisionSystem> collisionSystem = nullptr;
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

    // Setup collision callback after collision system is created
    collisionSystem->SetCollisionCallback([this](ECS::Entity a, ECS::Entity b) {
        std::cout << "[Collision] Entity " << a << " <-> Entity " << b
                  << " | isNetworkClient=" << (isNetworkClient ? "TRUE" : "FALSE") << std::endl;

        // Create explosion at collision point (ONLY if not a network client)
        // Network clients should receive explosion entities from server
        if (!isNetworkClient) {
            std::cout << "[Game] Creating explosion (NOT a network client)" << std::endl;
            if (gCoordinator.HasComponent<Position>(b)) {
                auto& posB = gCoordinator.GetComponent<Position>(b);
                float centerX = posB.x;
                float centerY = posB.y;

                // Adjust for sprite scale if available
                if (gCoordinator.HasComponent<Collider>(b)) {
                    auto& colliderB = gCoordinator.GetComponent<Collider>(b);
                    centerX += colliderB.width / 2.0f;
                    centerY += colliderB.height / 2.0f;
                }

                std::cout << "[Game] Creating explosion at (" << centerX << ", " << centerY << ")" << std::endl;
                CreateExplosion(centerX, centerY);
                std::cout << "[Game] Explosion created successfully" << std::endl;
            }
        } else {
            std::cout << "[Game] Skipping explosion creation (network client)" << std::endl;
        }

        // Check if entities should be destroyed based on health (damage already applied by CollisionSystem)
        if (gCoordinator.HasComponent<Health>(a)) {
            auto& health = gCoordinator.GetComponent<Health>(a);
            if (health.current <= 0 && health.destroyOnDeath) {
                DestroyEntityDeferred(a);
            }
        }
        if (gCoordinator.HasComponent<Health>(b)) {
            auto& health = gCoordinator.GetComponent<Health>(b);
            if (health.current <= 0 && health.destroyOnDeath) {
                DestroyEntityDeferred(b);
            }
        }

        // Destroy projectiles on collision
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
    std::shared_ptr<rtype::engine::systems::NetworkSystem> networkSystem;

    if (networkMode) {
        try {
            networkClient = std::make_shared<NetworkClient>(serverAddress, serverPort);
            networkSystem = std::make_shared<rtype::engine::systems::NetworkSystem>(&gCoordinator, networkClient);

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
                    sprite->setTexture(enemyTexture.get());
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

    // Load textures (try multiple paths for flexibility)
    backgroundTexture = std::make_unique<SFMLTexture>();
    bool bgLoaded = backgroundTexture->loadFromFile("game/assets/background.png") ||
                    backgroundTexture->loadFromFile("../../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("../client/assets/background.png") ||
                    backgroundTexture->loadFromFile("client/assets/background.png");
    if (!bgLoaded) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        std::cerr << "Tried paths: ../../client/assets/, ../client/assets/, client/assets/" << std::endl;
        return 1;
    }

    playerTexture = std::make_unique<SFMLTexture>();
    bool playerLoaded = playerTexture->loadFromFile("game/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("../../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("../client/assets/players/r-typesheet42.png") ||
                        playerTexture->loadFromFile("cliFent/assets/players/r-typesheet42.png");
    if (!playerLoaded) {
        std::cerr << "Error: Could not load player sprite" << std::endl;
        return 1;
    }

    missileTexture = std::make_unique<SFMLTexture>();
    bool missileLoaded = missileTexture->loadFromFile("game/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("../../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("../client/assets/players/r-typesheet1.png") ||
                         missileTexture->loadFromFile("client/assets/players/r-typesheet1.png");
    if (!missileLoaded) {
        std::cerr << "Error: Could not load missile sprite" << std::endl;
        return 1;
    }

    enemyTexture = std::make_unique<SFMLTexture>();
    bool enemyLoaded = enemyTexture->loadFromFile("game/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("../../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("../client/assets/enemies/r-typesheet5.png") ||
                       enemyTexture->loadFromFile("client/assets/enemies/r-typesheet5.png");
    if (!enemyLoaded) {
        std::cerr << "Error: Could not load enemy sprite" << std::endl;
        return 1;
    }
    std::cout << "[Game] ✅ Enemy texture loaded: " << enemyTexture->getSize().x << "x" << enemyTexture->getSize().y << std::endl;

    // Load enemy bullet texture (separate from enemy sprites)
    enemyBulletTexture = std::make_unique<SFMLTexture>();
    bool enemyBulletLoaded = enemyBulletTexture->loadFromFile("game/assets/enemies/enemy_bullets.png") ||
                             enemyBulletTexture->loadFromFile("../../client/assets/enemies/enemy_bullets.png") ||
                             enemyBulletTexture->loadFromFile("../client/assets/enemies/enemy_bullets.png") ||
                             enemyBulletTexture->loadFromFile("client/assets/enemies/enemy_bullets.png");
    if (!enemyBulletLoaded) {
        std::cerr << "Error: Could not load enemy bullet sprite" << std::endl;
        return 1;
    }
    std::cout << "[Game] ✅ Enemy bullet texture loaded: " << enemyBulletTexture->getSize().x << "x" << enemyBulletTexture->getSize().y << std::endl;

    explosionTexture = std::make_unique<SFMLTexture>();
    bool explosionLoaded = explosionTexture->loadFromFile("game/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("../../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("../client/assets/enemies/r-typesheet44.png") ||
                           explosionTexture->loadFromFile("client/assets/enemies/r-typesheet44.png");
    if (!explosionLoaded) {
        std::cerr << "Error: Could not load explosion sprite" << std::endl;
        return 1;
    }

    // Load sound
    bool soundLoaded = shootBuffer.loadFromFile("game/assets/vfx/shoot.ogg") ||
                       shootBuffer.loadFromFile("../../client/assets/vfx/shoot.ogg") ||
                       shootBuffer.loadFromFile("../client/assets/vfx/shoot.ogg") ||
                       shootBuffer.loadFromFile("client/assets/vfx/shoot.ogg");
    if (!soundLoaded) {
        std::cerr << "Warning: Could not load shoot.ogg" << std::endl;
    } else {
        shootSound.setBuffer(shootBuffer);
        shootSound.setVolume(80.f);
    }

    // ========================================
    // REGISTER FACTORIES TO LUA
    // ========================================
    std::cout << "🏭 Registering Factories to Lua..." << std::endl;

    // Prepare texture map for factories
    std::unordered_map<std::string, SFMLTexture*> textureMap;
    textureMap["enemy"] = enemyTexture.get();
    textureMap["missile"] = missileTexture.get();
    textureMap["player"] = playerTexture.get();
    textureMap["background"] = backgroundTexture.get();
    textureMap["explosion"] = explosionTexture.get();

    RType::Scripting::FactoryBindings::RegisterFactories(
        luaState.GetState(),
        &gCoordinator,
        textureMap,
        &allSprites
    );

    // Load Lua scripts
    std::cout << "📜 Loading Lua scripts..." << std::endl;

    // Load configuration
    if (!luaState.LoadScript("assets/scripts/config/game_config.lua")) {
        std::cerr << "Warning: Could not load game_config.lua" << std::endl;
    }

    // Load spawn system
    spawnScriptSystem = Scripting::ScriptedSystemLoader::LoadSystem(
        "assets/scripts/systems/spawn_system.lua",
        &gCoordinator
    );

    if (spawnScriptSystem) {
        std::cout << "[Game] Spawn system loaded from Lua" << std::endl;
    } else {
        std::cerr << "[Game] Warning: Spawn system failed to load" << std::endl;
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
                        sprite->setTexture(enemyTexture.get());

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
                        sprite->setTexture(enemyTexture.get());
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

        // Handle continuous input
        if (rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Space)) {
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
        // 2. INPUT CAPTURE & NETWORK SEND
        // ========================================
        bool movingUp = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Up);
        bool movingDown = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Down);
        bool movingLeft = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Left);
        bool movingRight = rtype::engine::Keyboard::isKeyPressed(rtype::engine::Key::Right);
        bool firing = spacePressed;

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
        // 4. LOCAL ENEMY SPAWNING (Only in local mode - NOW SCRIPTED!)
        // ========================================
        if (!networkMode && spawnScriptSystem) {
            // Hot-reload Lua scripts every frame
            luaState.CheckForChanges();

            // Update spawn system from Lua
            spawnScriptSystem->Update(deltaTime);
        }

        // ========================================
        // 5. SYSTEM UPDATES (ECS Architecture!)
        // ========================================

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

        // Process destroyed entities
        ProcessDestroyedEntities();

        // Render using RenderSystem
        window.clear();
        renderSystem->Update(deltaTime);
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
