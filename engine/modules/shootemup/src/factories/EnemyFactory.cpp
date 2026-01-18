#include "factories/EnemyFactory.hpp"
#include "components/ShootEmUpTags.hpp"
#include "components/Weapon.hpp"
#include <rendering/Types.hpp>

using namespace eng::engine::rendering;
using namespace ShootEmUp::Components;

// Helper pour créer le sprite de base
SFMLSprite* EnemyFactory::CreateEnemySprite(
    float x, float y,
    SFMLTexture* texture,
    int spriteX, int spriteY,
    int spriteWidth, int spriteHeight,
    std::vector<SFMLSprite*>& spriteList
) {
    auto* sprite = new SFMLSprite();
    spriteList.push_back(sprite);
    sprite->setTexture(texture);
    IntRect rect(spriteX, spriteY, spriteWidth, spriteHeight);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));
    return sprite;
}

// Ennemi BASIC - Simple mouvement horizontal
ECS::Entity EnemyFactory::CreateBasicEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    // Position & Velocity
    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    // Sprite
    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    spriteComp.scaleX = 2.5f;  // Scale pour les ennemis
    spriteComp.scaleY = 2.5f;
    coordinator.AddComponent(enemy, spriteComp);

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
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    // Movement pattern - Simple horizontal
    MovementPattern movementPattern;
    movementPattern.patternType = "straight";
    movementPattern.speed = 200.0f;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    // Collider
    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    // Health
    Health health;
    health.current = 1;
    health.max = 1;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    // Tags
    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "basic";
    enemyTag.scoreValue = 100;
    enemyTag.aiAggressiveness = 1.0f;
    enemyTag.enemyType = "basic"; // backward compatibility
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Ennemi ZIGZAG - Mouvement en zigzag
ECS::Entity EnemyFactory::CreateZigZagEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.1f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.patternType = "zigzag";
    movementPattern.speed = 250.0f;
    movementPattern.amplitude = 100.0f;
    movementPattern.frequency = 2.0f;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    Health health;
    health.current = 2;
    health.max = 2;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "zigzag";
    enemyTag.scoreValue = 200;
    enemyTag.aiAggressiveness = 1.2f;
    enemyTag.enemyType = "zigzag";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Ennemi SINE_WAVE - Mouvement sinusoïdal
ECS::Entity EnemyFactory::CreateSineWaveEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.1f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.patternType = "sine_wave";
    movementPattern.speed = 200.0f;
    movementPattern.amplitude = 80.0f;
    movementPattern.frequency = 2.5f;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    Health health;
    health.current = 2;
    health.max = 2;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "sine_wave";
    enemyTag.scoreValue = 150;
    enemyTag.aiAggressiveness = 1.1f;
    enemyTag.enemyType = "sine_wave";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Ennemi KAMIKAZE - Fonce vers le joueur
ECS::Entity EnemyFactory::CreateKamikazeEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.08f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.patternType = "diagonal_down";
    movementPattern.speed = 400.0f; // Plus rapide!
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    Health health;
    health.current = 1;
    health.max = 1;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "kamikaze";
    enemyTag.scoreValue = 250;
    enemyTag.aiAggressiveness = 2.0f;
    enemyTag.enemyType = "kamikaze";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// TURRET - Statique qui tire
ECS::Entity EnemyFactory::CreateTurretEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.15f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.patternType = "straight";
    movementPattern.speed = 0.0f;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    Collider collider;
    collider.width = 33 * 2.5f;
    collider.height = 32 * 2.5f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    Health health;
    health.current = 5;
    health.max = 5;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "turret";
    enemyTag.scoreValue = 300;
    enemyTag.aiAggressiveness = 1.5f;
    enemyTag.enemyType = "turret";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// BOSS - Ennemi puissant
ECS::Entity EnemyFactory::CreateBossEnemy(
    ECS::Coordinator& coordinator,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    ECS::Entity enemy = coordinator.CreateEntity();

    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});

    auto* sprite = CreateEnemySprite(x, y, texture, 0, 0, 33, 32, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(0, 0, 33, 32);
    spriteComp.layer = 5;
    spriteComp.scaleX = 2.5f;
    spriteComp.scaleY = 2.5f;
    spriteComp.scaleX = 2.0f; // Boss plus gros
    spriteComp.scaleY = 2.0f;
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.12f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.patternType = "circular";
    movementPattern.speed = 100.0f;
    movementPattern.amplitude = 150.0f;
    movementPattern.frequency = 1.0f;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);

    Collider collider;
    collider.width = 33 * 5.0f; // Boss plus gros collider
    collider.height = 32 * 5.0f;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);

    Health health;
    health.current = 50;
    health.max = 50;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator.AddComponent(enemy, health);

    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = "boss";
    enemyTag.scoreValue = 5000;
    enemyTag.aiAggressiveness = 3.0f;
    enemyTag.enemyType = "boss";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Factory générique qui dispatche selon le type (string-based)
ECS::Entity EnemyFactory::CreateEnemy(
    ECS::Coordinator& coordinator,
    const std::string& enemyType,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    if (enemyType == "basic") {
        return CreateBasicEnemy(coordinator, x, y, texture, spriteList);
    } else if (enemyType == "zigzag") {
        return CreateZigZagEnemy(coordinator, x, y, texture, spriteList);
    } else if (enemyType == "sine_wave") {
        return CreateSineWaveEnemy(coordinator, x, y, texture, spriteList);
    } else if (enemyType == "kamikaze") {
        return CreateKamikazeEnemy(coordinator, x, y, texture, spriteList);
    } else if (enemyType == "turret") {
        return CreateTurretEnemy(coordinator, x, y, texture, spriteList);
    } else if (enemyType == "boss") {
        return CreateBossEnemy(coordinator, x, y, texture, spriteList);
    } else {
        // Default to basic if unknown type
        return CreateBasicEnemy(coordinator, x, y, texture, spriteList);
    }
}

// ============================================================================
// CRÉATION D'ENNEMIS DEPUIS LES CONFIGS LUA
// ============================================================================

ECS::Entity EnemyFactory::CreateEnemyFromLuaConfig(
    ECS::Coordinator& coordinator,
    float x, float y,
    sol::table config,
    std::unordered_map<std::string, eng::engine::rendering::sfml::SFMLTexture*>& textures,
    std::vector<eng::engine::rendering::sfml::SFMLSprite*>& spriteList
) {
    using namespace ShootEmUp::Components;
    
    if (!config.valid()) {
        std::cerr << "[EnemyFactory] Invalid config table!" << std::endl;
        return 0;
    }
    
    ECS::Entity enemy = coordinator.CreateEntity();
    
    // Lire la config
    int health = config.get_or("health", 10);
    int scoreValue = config.get_or("scoreValue", 100);
    
    sol::table movement = config.get_or("movement", sol::table());
    std::string patternType = movement.get_or<std::string>("pattern", "straight");
    float speed = config.get_or("speed", 200.0f);
    float amplitude = movement.get_or("amplitude", 80.0f);
    float frequency = movement.get_or("frequency", 2.0f);
    
    sol::table sprite = config.get_or("sprite", sol::table());
    int frameWidth = sprite.get_or("frameWidth", 33);
    int frameHeight = sprite.get_or("frameHeight", 32);
    float scale = sprite.get_or("scale", 2.5f);
    int startX = sprite.get_or("startX", 0);
    int startY = sprite.get_or("startY", 0);
    int spacing = sprite.get_or("spacing", 0);
    
    sol::table animation = config.get_or("animation", sol::table());
    int frameCount = animation.get_or("frameCount", 8);
    float frameTime = animation.get_or("frameTime", 0.1f);
    bool loop = animation.get_or("loop", true);
    
    sol::table hitbox = config.get_or("hitbox", sol::table());
    int hitboxWidth = hitbox.get_or("width", frameWidth);
    int hitboxHeight = hitbox.get_or("height", frameHeight);
    
    std::string enemyName = config.get_or<std::string>("name", "Unknown");
    std::string enemyType = config.get_or<std::string>("enemyType", "basic");
    
    // Trouver la texture - preferer la texture indiquée dans la config (sprite.texture)
    eng::engine::rendering::sfml::SFMLTexture* texture = nullptr;
    std::string texPath = std::string();
    try {
        texPath = sprite.get_or("texture", std::string());
    } catch (...) {
        texPath = std::string();
    }

    if (!texPath.empty()) {
        auto it = textures.find(texPath);
        if (it != textures.end()) {
            texture = it->second;
            std::cout << "[EnemyFactory] Using texture from config: " << texPath << std::endl;
        } else {
            std::cout << "[EnemyFactory] Texture '" << texPath << "' not found in cache, falling back to generic 'enemy'" << std::endl;
        }
    }

    if (!texture) {
        auto it2 = textures.find("enemy");
        if (it2 != textures.end()) {
            texture = it2->second;
            std::cout << "[EnemyFactory] Using fallback texture 'enemy'" << std::endl;
        }
    }

    if (!texture) {
        std::cerr << "[EnemyFactory] No texture available!" << std::endl;
        coordinator.DestroyEntity(enemy);
        return 0;
    }
    
    // Créer les composants
    coordinator.AddComponent(enemy, Position{x, y});
    coordinator.AddComponent(enemy, Velocity{0.0f, 0.0f});
    
    auto* sfmlSprite = new eng::engine::rendering::sfml::SFMLSprite();
    spriteList.push_back(sfmlSprite);
    sfmlSprite->setTexture(texture);
    eng::engine::rendering::IntRect rect(startX, startY, frameWidth, frameHeight);
    sfmlSprite->setTextureRect(rect);
    sfmlSprite->setPosition(eng::engine::rendering::Vector2f(x, y));
    
    Sprite spriteComp;
    spriteComp.sprite = sfmlSprite;
    spriteComp.textureRect = rect;
    spriteComp.layer = 5;
    spriteComp.scaleX = scale;
    spriteComp.scaleY = scale;
    coordinator.AddComponent(enemy, spriteComp);
    
    Animation anim;
    anim.frameTime = frameTime;
    anim.currentFrame = 0;
    anim.frameCount = frameCount;
    anim.loop = loop;
    anim.frameWidth = frameWidth;
    anim.frameHeight = frameHeight;
    anim.startX = startX;
    anim.startY = startY;
    anim.spacing = spacing;
    coordinator.AddComponent(enemy, anim);
    
    MovementPattern movementPattern;
    movementPattern.patternType = patternType;
    movementPattern.speed = speed;
    movementPattern.amplitude = amplitude;
    movementPattern.frequency = frequency;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator.AddComponent(enemy, movementPattern);
    
    Collider collider;
    collider.width = hitboxWidth * scale;
    collider.height = hitboxHeight * scale;
    collider.tag = "enemy";
    coordinator.AddComponent(enemy, collider);
    
    Health healthComp;
    healthComp.current = health;
    healthComp.max = health;
    healthComp.destroyOnDeath = true;
    healthComp.deathEffect = "explosion";
    coordinator.AddComponent(enemy, healthComp);
    
    coordinator.AddComponent(enemy, Tag{"enemy"});
    EnemyTag enemyTag;
    enemyTag.enemyType = enemyType;
    enemyTag.scoreValue = scoreValue;
    enemyTag.aiAggressiveness = 1.0f;
    coordinator.AddComponent(enemy, enemyTag);
    
    // Weapon (optionnel) - peut être une string (weapon id), une table (config détaillée)
    try {
        sol::object weaponObj = config["weapon"];
        float shootInterval = config.get_or("shootInterval", 0.0f);

        bool shouldAddWeapon = false;
        ShootEmUp::Components::Weapon weaponComp;

        if (weaponObj.valid()) {
            if (weaponObj.get_type() == sol::type::table) {
                sol::table weaponTable = config.get<sol::table>("weapon");
                weaponComp.weaponType = weaponTable.get_or("weaponType", std::string("single_shot"));
                weaponComp.level = weaponTable.get_or("level", 1);
                weaponComp.fireRate = weaponTable.get_or("fireRate", 0.5f);
                weaponComp.projectileType = weaponTable.get_or("projectileType", std::string("enemy_bullet"));
                weaponComp.projectileSpeed = weaponTable.get_or("projectileSpeed", 600.0f);
                weaponComp.damage = weaponTable.get_or("damage", 1);
                weaponComp.projectileCount = weaponTable.get_or("projectileCount", 1);
                weaponComp.spreadAngle = weaponTable.get_or("spreadAngle", 0.0f);
                shouldAddWeapon = true;
            } else if (weaponObj.get_type() == sol::type::string) {
                // Simple form: weapon = "enemy_bullet"
                std::string weaponId = config.get<std::string>("weapon");
                weaponComp.weaponType = weaponId;
                weaponComp.projectileType = weaponId; // use same id for projectile lookup
                weaponComp.fireRate = shootInterval > 0.0f ? shootInterval : 1.0f;
                weaponComp.projectileSpeed = 600.0f;
                weaponComp.damage = 1;
                weaponComp.projectileCount = 1;
                weaponComp.lastFireTime = 0.0f;
                weaponComp.canFire = true;
                shouldAddWeapon = true;
            }
        } else if (shootInterval > 0.0f) {
            // Backwards compatibility: some configs may set shootInterval without a weapon id
            weaponComp.weaponType = "enemy_bullet";
            weaponComp.projectileType = "enemy_bullet";
            weaponComp.fireRate = shootInterval;
            weaponComp.projectileSpeed = 600.0f;
            weaponComp.damage = 1;
            weaponComp.projectileCount = 1;
            weaponComp.lastFireTime = 0.0f;
            weaponComp.canFire = true;
            shouldAddWeapon = true;
        }

        if (shouldAddWeapon) {
            coordinator.AddComponent(enemy, weaponComp);
            std::cout << "[EnemyFactory] Added Weapon component to '" << enemyName << "' (proj:" << weaponComp.projectileType << ")" << std::endl;
        }
    } catch (...) {
        // ignore malformed weapon tables or types
    }

    std::cout << "[EnemyFactory] Created '" << enemyName << "' (type: " << enemyType 
              << ") at (" << x << ", " << y << ") with " << health << " HP" << std::endl;
    
    return enemy;
}
