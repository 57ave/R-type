#include "factories/EnemyFactory.hpp"
#include <rendering/Types.hpp>

using namespace rtype::engine::rendering;

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
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    // Movement pattern - Simple horizontal
    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::STRAIGHT;
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
    enemyTag.type = EnemyTag::Type::BASIC;
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
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.1f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::ZIGZAG;
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
    enemyTag.type = EnemyTag::Type::ZIGZAG;
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
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.1f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::SINE_WAVE;
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
    enemyTag.type = EnemyTag::Type::SINE_WAVE;
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
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.08f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::DIAGONAL_DOWN;
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
    enemyTag.type = EnemyTag::Type::KAMIKAZE;
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
    coordinator.AddComponent(enemy, spriteComp);

    Animation anim;
    anim.frameTime = 0.15f;
    anim.frameCount = 8;
    anim.loop = true;
    anim.frameWidth = 33;
    anim.frameHeight = 32;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::STRAIGHT;
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
    enemyTag.type = EnemyTag::Type::TURRET;
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
    anim.spacing = 33;
    coordinator.AddComponent(enemy, anim);

    MovementPattern movementPattern;
    movementPattern.pattern = MovementPattern::Type::CIRCULAR;
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
    enemyTag.type = EnemyTag::Type::BOSS;
    enemyTag.scoreValue = 5000;
    enemyTag.aiAggressiveness = 3.0f;
    enemyTag.enemyType = "boss";
    coordinator.AddComponent(enemy, enemyTag);

    return enemy;
}

// Factory générique qui dispatche selon le type
ECS::Entity EnemyFactory::CreateEnemy(
    ECS::Coordinator& coordinator,
    EnemyTag::Type enemyType,
    float x, float y,
    SFMLTexture* texture,
    std::vector<SFMLSprite*>& spriteList
) {
    switch (enemyType) {
        case EnemyTag::Type::BASIC:
            return CreateBasicEnemy(coordinator, x, y, texture, spriteList);
        case EnemyTag::Type::ZIGZAG:
            return CreateZigZagEnemy(coordinator, x, y, texture, spriteList);
        case EnemyTag::Type::SINE_WAVE:
            return CreateSineWaveEnemy(coordinator, x, y, texture, spriteList);
        case EnemyTag::Type::KAMIKAZE:
            return CreateKamikazeEnemy(coordinator, x, y, texture, spriteList);
        case EnemyTag::Type::TURRET:
            return CreateTurretEnemy(coordinator, x, y, texture, spriteList);
        case EnemyTag::Type::BOSS:
            return CreateBossEnemy(coordinator, x, y, texture, spriteList);
        default:
            return CreateBasicEnemy(coordinator, x, y, texture, spriteList);
    }
}
