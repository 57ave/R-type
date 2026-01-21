#include "core/GameplayManager.hpp"
#include "GameStateManager.hpp"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

// Inclure les composants nécessaires
#include <ecs/Components.hpp>
#include <ecs/Systems.hpp>
#include <components/Weapon.hpp>
#include <components/ShootEmUpTags.hpp>
#include <components/Score.hpp>
#include <components/PowerUp.hpp>
#include <components/Effect.hpp>
#include <components/AIController.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
// #include <components/StateMachineAnimation.hpp>  // Does not exist
#include <components/Health.hpp>
#include <components/Damage.hpp>
#include <components/MovementPattern.hpp>
#include <components/Tag.hpp>
#include <components/Lifetime.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/Types.hpp>

// Using declarations for commonly used types
using eng::engine::rendering::sfml::SFMLSprite;
using eng::engine::rendering::IntRect;
using eng::engine::rendering::Vector2f;

namespace RType::Core {

GameplayManager::GameplayManager(ECS::Coordinator* coordinator)
    : coordinator(coordinator)
    , textureMap(nullptr)
    , allSprites(nullptr)
    , windowWidth(1920.0f)
    , windowHeight(1080.0f)
    , enemySpawnRate(2.0f)
    , enemySpeed(200.0f)
    , enemyHealth(1)
    , maxEnemiesOnScreen(10)
{
    std::cout << "[GameplayManager] Created" << std::endl;
}

GameplayManager::~GameplayManager() {
    std::cout << "[GameplayManager] Destroyed" << std::endl;
}

void GameplayManager::Initialize(
    std::unordered_map<std::string, eng::engine::rendering::ITexture*>& texMap,
    std::vector<eng::engine::rendering::ISprite*>& sprites)
{
    textureMap = &texMap;
    allSprites = &sprites;
    
    std::cout << "[GameplayManager] Initialized with " << texMap.size() 
              << " textures and " << sprites.size() << " sprites" << std::endl;
}

void GameplayManager::SetEntityRegistrationCallback(std::function<void(ECS::Entity)> callback) {
    registerEntityCallback = callback;
    std::cout << "[GameplayManager] Entity registration callback set" << std::endl;
}

ECS::Entity GameplayManager::CreatePlayer(float x, float y, int line) {
    ECS::Entity player = coordinator->CreateEntity();
    RegisterEntity(player);

    // Position
    coordinator->AddComponent(player, Position{x, y});

    // Velocity (controlled by input)
    coordinator->AddComponent(player, Velocity{0.0f, 0.0f});

    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture("player");
    if (sprite) {
        eng::engine::rendering::IntRect rect(33 * 2, line * 17, 33, 17);
        sprite->setTextureRect(rect);
        sprite->setPosition(eng::engine::rendering::Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 10;
        coordinator->AddComponent(player, spriteComp);
    }

    // State machine animation
    StateMachineAnimation anim;
    anim.currentColumn = 2;
    anim.targetColumn = 2;
    anim.transitionSpeed = 0.15f;
    anim.spriteWidth = 33;
    anim.spriteHeight = 17;
    anim.currentRow = line;
    coordinator->AddComponent(player, anim);

    // Collider
    Collider collider;
    collider.width = 33 * 3.0f;
    collider.height = 17 * 3.0f;
    collider.tag = "player";
    coordinator->AddComponent(player, collider);

    // Health
    Health health;
    health.current = 50;  // Player starts with 50 HP
    health.max = 50;
    health.invincibilityDuration = 2.0f;
    coordinator->AddComponent(player, health);

    // Damage (player deals damage on contact)
    Damage damage;
    damage.amount = 100;
    damage.damageType = "contact";
    coordinator->AddComponent(player, damage);

    // Weapon
    ShootEmUp::Components::Weapon weapon;
    weapon.fireRate = defaultWeaponConfig.fireRate;
    weapon.supportsCharge = defaultWeaponConfig.supportsCharge;
    weapon.minChargeTime = defaultWeaponConfig.minChargeTime;
    weapon.maxChargeTime = defaultWeaponConfig.maxChargeTime;
    weapon.projectileSpeed = defaultWeaponConfig.projectileSpeed;
    weapon.shootSound = "shoot";
    coordinator->AddComponent(player, weapon);

    // Tags
    coordinator->AddComponent(player, Tag{"player"});
    coordinator->AddComponent(player, ShootEmUp::Components::PlayerTag{line});

    // Score - Initialize with 0 score
    ShootEmUp::Components::Score score;
    score.currentScore = 0;
    score.highScore = 0;
    score.comboMultiplier = 1;
    score.comboTimer = 0.0f;
    score.consecutiveKills = 0;
    coordinator->AddComponent(player, score);

    std::cout << "[GameplayManager] Player created at (" << x << ", " << y << ") with line " << line << std::endl;
    return player;
}

ECS::Entity GameplayManager::CreateEnemy(float x, float y, const std::string& patternType, const std::string& enemyType) {
    ECS::Entity enemy = coordinator->CreateEntity();
    RegisterEntity(enemy);

    // Position
    coordinator->AddComponent(enemy, Position{x, y});

    // Velocity
    coordinator->AddComponent(enemy, Velocity{0.0f, 0.0f});

    // Type d'ennemi - utiliser basic par défaut
    std::string actualType = enemyType.empty() ? "basic" : enemyType;
    
    // Valeurs par défaut
    std::string textureName = "enemy";
    IntRect rect(0, 0, 33, 36);
    float scale = 2.5f;
    int hp = static_cast<int>(enemyHealth);
    int scoreValue = 100;
    float speedMod = 1.0f;
    int frameCount = 8;
    std::string actualPattern = patternType.empty() ? "linear" : patternType;
    float amplitude = 80.0f;
    float frequency = 2.0f;
    
    // Configuration selon le type d'ennemi
    if (actualType == "basic") {
        textureName = "enemy";
        rect = IntRect(0, 0, 33, 36);
        scale = 2.5f;
        hp = 1;
        scoreValue = 100;
        speedMod = 1.0f;
        frameCount = 8;
        actualPattern = patternType.empty() ? "linear" : patternType;
    } else if (actualType == "zigzag") {
        textureName = "enemy";
        rect = IntRect(0, 0, 17, 18);
        scale = 2.5f;
        hp = 1;
        scoreValue = 150;
        speedMod = 1.1f;
        frameCount = 12;
        actualPattern = "zigzag";
        amplitude = 80.0f;
        frequency = 2.0f;
    } else if (actualType == "sinewave") {
        textureName = "enemy";
        rect = IntRect(0, 0, 17, 18);
        scale = 2.0f;
        hp = 1;
        scoreValue = 200;
        speedMod = 0.9f;
        frameCount = 12;
        actualPattern = "sine";
        amplitude = 120.0f;
        frequency = 1.5f;
    } else if (actualType == "kamikaze") {
        textureName = "enemy";
        rect = IntRect(0, 0, 32, 32);
        scale = 2.0f;
        hp = 1;
        scoreValue = 120;
        speedMod = 2.0f;  // Très rapide
        frameCount = 4;
        actualPattern = "chase";  // Poursuit le joueur
    } else if (actualType == "shooter") {
        textureName = "enemy";
        rect = IntRect(0, 0, 33, 36);
        scale = 2.5f;
        hp = 1;
        scoreValue = 300;
        speedMod = 0.75f;  // Plus lent mais tire
        frameCount = 8;
        actualPattern = "linear";
    } else {
        // Type inconnu - utiliser basic
        hp = 1;
        scoreValue = 100;
        speedMod = 1.0f;
    }
    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture(textureName);
    if (sprite) {
        sprite->setTextureRect(rect);
        sprite->setPosition(Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 5;
        spriteComp.scaleX = scale;
        spriteComp.scaleY = scale;
        coordinator->AddComponent(enemy, spriteComp);
    }

    // Animation
    Animation anim;
    anim.frameTime = 0.1f;
    anim.currentFrame = 0;
    anim.frameCount = frameCount;
    anim.loop = true;
    anim.frameWidth = rect.width;
    anim.frameHeight = rect.height;
    anim.startX = 0;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator->AddComponent(enemy, anim);

    // Movement pattern - utiliser le pattern selon le type d'ennemi
    ShootEmUp::Components::MovementPattern movementPattern;
    movementPattern.patternType = actualPattern;
    movementPattern.speed = (enemySpeed + (rand() % 100)) * speedMod;
    movementPattern.amplitude = amplitude;
    movementPattern.frequency = frequency;
    movementPattern.startX = x;
    movementPattern.startY = y;
    coordinator->AddComponent(enemy, movementPattern);

    // Collider - adapté à la taille
    Collider collider;
    collider.width = rect.width * scale;
    collider.height = rect.height * scale;
    collider.tag = "enemy";
    coordinator->AddComponent(enemy, collider);

    // Health - variable selon le type
    Health health;
    health.current = hp;
    health.max = hp;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator->AddComponent(enemy, health);

    // Damage
    Damage damage;
    damage.amount = 1;
    damage.damageType = "contact";
    coordinator->AddComponent(enemy, damage);

    // Tags
    coordinator->AddComponent(enemy, Tag{"enemy"});
    ShootEmUp::Components::EnemyTag enemyTag;
    enemyTag.enemyType = actualType;
    enemyTag.scoreValue = scoreValue;
    enemyTag.aiAggressiveness = 1.0f;
    coordinator->AddComponent(enemy, enemyTag);

    std::cout << "[GameplayManager] Created enemy type '" << actualType 
              << "' (HP:" << hp << ", Score:" << scoreValue << ", Scale:" << scale 
              << ", Pattern:" << actualPattern << ")" << std::endl;

    return enemy;
}

ECS::Entity GameplayManager::CreateMissile(float x, float y, bool isCharged, int chargeLevel) {
    ECS::Entity missile = coordinator->CreateEntity();
    RegisterEntity(missile);

    // Position
    coordinator->AddComponent(missile, Position{x, y});

    // Velocity
    float speed = isCharged ? 1500.0f : 1000.0f;
    coordinator->AddComponent(missile, Velocity{speed, 0.0f});

    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture("missile");
    if (sprite) {
        IntRect rect;
        if (!isCharged) {
            // Missiles normaux
            rect = IntRect(245, 85, 20, 20);
        } else {
            // Charged missile sprites
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
            ChargeData& data = chargeLevels[std::clamp(chargeLevel - 1, 0, 4)];
            rect = IntRect(data.xPos, data.yPos, data.width, data.height);
        }

        sprite->setTextureRect(rect);
        sprite->setPosition(Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 8;
        spriteComp.scaleX = 3.0f;
        spriteComp.scaleY = 3.0f;
        coordinator->AddComponent(missile, spriteComp);
    }

    // Animation pour les missiles chargés
    if (isCharged) {
        Animation anim;
        anim.frameTime = 0.1f;
        anim.currentFrame = 0;
        anim.frameCount = 2;
        anim.loop = true;
        anim.frameWidth = 20;  // Valeur par défaut
        anim.frameHeight = 20;
        anim.startX = 245;
        anim.startY = 85;
        anim.spacing = 2;
        coordinator->AddComponent(missile, anim);
    }

    // Collider
    Collider collider;
    collider.width = 20 * 3.0f;
    collider.height = 20 * 3.0f;
    collider.tag = isCharged ? "charged_bullet" : "bullet";
    coordinator->AddComponent(missile, collider);

    // Damage
    Damage damage;
    damage.amount = isCharged ? chargeLevel : 1;
    damage.damageType = isCharged ? "charged" : "normal";
    coordinator->AddComponent(missile, damage);

    // Tags
    coordinator->AddComponent(missile, Tag{isCharged ? "charged_bullet" : "bullet"});
    ShootEmUp::Components::ProjectileTag projTag;
    projTag.projectileType = isCharged ? "charged" : "normal";
    projTag.ownerId = 0;
    projTag.isPlayerProjectile = true;
    projTag.chargeLevel = isCharged ? chargeLevel : 0;
    coordinator->AddComponent(missile, projTag);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator->AddComponent(missile, lifetime);

    // Mise à jour des statistiques
    gameStats.shotsFired++;

    return missile;
}

ECS::Entity GameplayManager::CreateEnemyMissile(float x, float y, float directionX, float directionY) {
    ECS::Entity missile = coordinator->CreateEntity();
    RegisterEntity(missile);

    // Position
    coordinator->AddComponent(missile, Position{x, y});

    // Velocity - vitesse variable
    float speed = 350.0f + (rand() % 150);  // 350-500
    coordinator->AddComponent(missile, Velocity{speed * directionX, speed * directionY});

    // ===== DIFFERENT BULLET TYPES =====
    // Balles ennemies - taille réduite pour meilleure jouabilité
    int bulletType = rand() % 3;
    float scale = 3.0f;  // Réduit de 5.5f à 2.0f
    IntRect rect(166, 3, 12, 12);
    
    switch (bulletType) {
        case 0: // Balle standard
            rect = IntRect(166, 3, 12, 12);
            scale = 2.0f;  // Réduit de 5.5f à 2.0f
            break;
        case 1: // Balle moyenne
            rect = IntRect(166, 3, 12, 12);
            scale = 2.5f;  // Réduit de 6.5f à 2.5f
            break;
        case 2: // Grosse balle
            rect = IntRect(166, 3, 12, 12);
            scale = 3.0f;  // Réduit de 7.5f à 3.0f
            break;
    }

    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture("enemy_bullets");
    if (sprite) {
        sprite->setTextureRect(rect);
        sprite->setPosition(Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 8;
        spriteComp.scaleX = scale;
        spriteComp.scaleY = scale;
        coordinator->AddComponent(missile, spriteComp);
    }

    // Animation
    Animation anim;
    anim.frameTime = 0.08f;  // Animation un peu plus rapide
    anim.currentFrame = 0;
    anim.frameCount = 4;
    anim.loop = true;
    anim.frameWidth = rect.width;
    anim.frameHeight = rect.height;
    anim.startX = rect.left;
    anim.startY = rect.top;
    anim.spacing = 5;
    coordinator->AddComponent(missile, anim);

    // Collider - adapté à la taille visuelle
    Collider collider;
    collider.width = rect.width * scale * 0.8f;  // Légèrement plus petit que le sprite
    collider.height = rect.height * scale * 0.8f;
    collider.tag = "enemy_bullet";
    coordinator->AddComponent(missile, collider);

    // Damage
    Damage damage;
    damage.amount = 1;
    damage.damageType = "enemy";
    coordinator->AddComponent(missile, damage);

    // Tags
    coordinator->AddComponent(missile, Tag{"enemy_bullet"});
    ShootEmUp::Components::ProjectileTag projTag;
    projTag.projectileType = "enemy";
    projTag.ownerId = 0;
    projTag.isPlayerProjectile = false;
    coordinator->AddComponent(missile, projTag);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator->AddComponent(missile, lifetime);

    return missile;
}

ECS::Entity GameplayManager::CreateExplosion(float x, float y) {
    ECS::Entity explosion = coordinator->CreateEntity();
    RegisterEntity(explosion);

    // Position
    coordinator->AddComponent(explosion, Position{x, y});

    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture("explosion");
    if (sprite) {
        IntRect rect(129, 0, 34, 35);
        sprite->setTextureRect(rect);
        sprite->setPosition(Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 15;
        spriteComp.scaleX = 2.5f;
        spriteComp.scaleY = 2.5f;
        coordinator->AddComponent(explosion, spriteComp);
    }

    // Animation
    Animation anim;
    anim.frameTime = 0.1f;
    anim.currentFrame = 0;
    anim.frameCount = 6;
    anim.loop = false;
    anim.frameWidth = 34;
    anim.frameHeight = 35;
    anim.startX = 124;
    anim.startY = 0;
    anim.spacing = 0;
    coordinator->AddComponent(explosion, anim);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 1.0f;
    coordinator->AddComponent(explosion, lifetime);

    // Tag
    coordinator->AddComponent(explosion, Tag{"explosion"});

    return explosion;
}

ECS::Entity GameplayManager::CreateShootEffect(float x, float y, ECS::Entity parent) {
    ECS::Entity effect = coordinator->CreateEntity();
    RegisterEntity(effect);

    // Position
    coordinator->AddComponent(effect, Position{x, y});

    // Sprite - Utiliser CreateSpriteFromTexture
    auto* sprite = CreateSpriteFromTexture("missile");
    if (sprite) {
        IntRect rect(212, 80, 16, 16);
        sprite->setTextureRect(rect);
        sprite->setPosition(Vector2f(x, y));

        Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.textureRect = rect;
        spriteComp.layer = 12;
        coordinator->AddComponent(effect, spriteComp);
    }

    // Animation
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
    coordinator->AddComponent(effect, anim);

    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 0.1f;
    coordinator->AddComponent(effect, lifetime);

    // Effect tag
    ShootEmUp::Components::Effect effectTag;
    effectTag.effectType = "shoot";
    effectTag.followParent = true;
    coordinator->AddComponent(effect, effectTag);

    coordinator->AddComponent(effect, Tag{"effect"});

    return effect;
}

ECS::Entity GameplayManager::CreateChargingEffect(ECS::Entity player) {
    // Pour l'instant, retourne 0 (pas d'effet de charge implémenté)
    // TODO: Implémenter un effet visuel de charge
    return 0;
}

void GameplayManager::FireMissile(ECS::Entity playerEntity) {
    if (!coordinator->HasComponent<Position>(playerEntity)) {
        return;
    }

    auto& pos = coordinator->GetComponent<Position>(playerEntity);
    
    // Créer le missile à droite du joueur
    float missileX = pos.x + 50.0f;  // Offset pour que le missile apparaisse devant le joueur
    float missileY = pos.y + 10.0f;  // Centré verticalement
    
    CreateMissile(missileX, missileY, false, 0);
    
    // Créer l'effet de tir
    CreateShootEffect(missileX - 20.0f, missileY, playerEntity);
    
    std::cout << "[GameplayManager] Player fired missile" << std::endl;
}

void GameplayManager::FireChargedMissile(ECS::Entity playerEntity, int chargeLevel) {
    if (!coordinator->HasComponent<Position>(playerEntity)) {
        return;
    }

    auto& pos = coordinator->GetComponent<Position>(playerEntity);
    
    float missileX = pos.x + 50.0f;
    float missileY = pos.y + 10.0f;
    
    CreateMissile(missileX, missileY, true, chargeLevel);
    CreateShootEffect(missileX - 20.0f, missileY, playerEntity);
    
    std::cout << "[GameplayManager] Player fired charged missile (level " << chargeLevel << ")" << std::endl;
}

void GameplayManager::SpawnRandomEnemy() {
    // Vérifier le nombre d'ennemis actuels
    int currentEnemyCount = 0;
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(entity)) {
            currentEnemyCount++;
        }
    }
    
    if (currentEnemyCount >= maxEnemiesOnScreen) {
        return;  // Trop d'ennemis à l'écran
    }
    
    // Position d'apparition aléatoire
    float x = GetRandomSpawnX();
    float y = GetRandomSpawnY();
    
    // Pattern de mouvement aléatoire
    std::string pattern = GetRandomEnemyPattern();
    
    CreateEnemy(x, y, pattern);
    
    std::cout << "[GameplayManager] Spawned enemy at (" << x << ", " << y << ") with pattern " << pattern << std::endl;
}

void GameplayManager::MakeEnemiesShoot() {
    // Faire tirer tous les ennemis
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(entity) &&
            coordinator->HasComponent<Position>(entity)) {
            
            auto& pos = coordinator->GetComponent<Position>(entity);
            
            // Créer un projectile ennemi
            float bulletX = pos.x - 20.0f;  // À gauche de l'ennemi
            float bulletY = pos.y + 15.0f;  // Centré
            
            CreateEnemyMissile(bulletX, bulletY, -1.0f, 0.0f);
        }
    }
}

ECS::Entity GameplayManager::GetLocalPlayerEntity() const {
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(entity)) {
            auto& playerTag = coordinator->GetComponent<ShootEmUp::Components::PlayerTag>(entity);
            if (playerTag.playerId == 0) {  // Joueur local
                return entity;
            }
        }
    }
    return 0;  // Pas de joueur trouvé
}

bool GameplayManager::CheckWinCondition() const {
    // Condition de victoire : survivre 30 secondes (géré par GameLoop)
    // Pour l'instant, pas de condition spécifique ici
    return false;
}

bool GameplayManager::CheckLoseCondition() const {
    // Condition de défaite : TOUS les joueurs morts
    // Compter les joueurs vivants
    int alivePlayers = 0;
    int totalPlayers = 0;
    
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(entity)) {
            totalPlayers++;
            
            if (coordinator->HasComponent<Health>(entity)) {
                auto& health = coordinator->GetComponent<Health>(entity);
                if (health.current > 0) {
                    alivePlayers++;
                }
            }
        }
    }
    
    // ✅ S'il n'y a aucun joueur, ce n'est PAS une défaite (le jeu n'a pas encore commencé)
    if (totalPlayers == 0) {
        return false;  // Pas encore de joueur = pas de défaite
    }
    
    // Défaite uniquement si tous les joueurs sont morts (health <= 0)
    return alivePlayers == 0;
}

void GameplayManager::ProcessDestroyedEntities() {
    for (auto entity : entitiesToDestroy) {
        // Nettoyer les sprites
        if (coordinator->HasComponent<Sprite>(entity)) {
            auto& sprite = coordinator->GetComponent<Sprite>(entity);
            if (sprite.sprite && allSprites) {
                auto it = std::find(allSprites->begin(), allSprites->end(), sprite.sprite);
                if (it != allSprites->end()) {
                    allSprites->erase(it);
                }
                delete sprite.sprite;
                sprite.sprite = nullptr;
            }
        }

        coordinator->DestroyEntity(entity);

        // Retirer de la liste des entités
        allEntities.erase(std::remove(allEntities.begin(), allEntities.end(), entity), allEntities.end());
    }
    entitiesToDestroy.clear();
}

void GameplayManager::ClearAllGameEntities() {
    std::cout << "[GameplayManager] Clearing all game entities for reset..." << std::endl;
    
    std::vector<ECS::Entity> toDestroy;
    
    // Collecter toutes les entités à détruire (ennemis, projectiles, powerups)
    for (auto entity : allEntities) {
        // Détruire les ennemis
        if (coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(entity)) {
            toDestroy.push_back(entity);
            continue;
        }
        // Détruire les projectiles
        if (coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(entity)) {
            toDestroy.push_back(entity);
            continue;
        }
        // Détruire les powerups
        if (coordinator->HasComponent<ShootEmUp::Components::PowerUp>(entity)) {
            toDestroy.push_back(entity);
            continue;
        }
    }
    
    std::cout << "[GameplayManager] Destroying " << toDestroy.size() << " entities" << std::endl;
    
    // Ajouter à la liste de destruction différée
    for (auto entity : toDestroy) {
        DestroyEntityDeferred(entity);
    }
    
    // Traiter immédiatement
    ProcessDestroyedEntities();
    
    std::cout << "[GameplayManager] Game entities cleared" << std::endl;
}

void GameplayManager::DestroyEntityDeferred(ECS::Entity entity) {
    entitiesToDestroy.push_back(entity);
}

uint32_t GameplayManager::GetPlayerScore(int playerId) const {
    // Chercher le joueur avec cet ID
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(entity) &&
            coordinator->HasComponent<ShootEmUp::Components::Score>(entity)) {
            auto& playerTag = coordinator->GetComponent<ShootEmUp::Components::PlayerTag>(entity);
            if (playerTag.playerId == playerId) {
                auto& score = coordinator->GetComponent<ShootEmUp::Components::Score>(entity);
                return score.currentScore;
            }
        }
    }
    return 0;
}

void GameplayManager::AddScore(uint32_t points, int playerId) {
    // Chercher le joueur avec cet ID
    for (auto entity : allEntities) {
        if (coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(entity) &&
            coordinator->HasComponent<ShootEmUp::Components::Score>(entity)) {
            auto& playerTag = coordinator->GetComponent<ShootEmUp::Components::PlayerTag>(entity);
            if (playerTag.playerId == playerId) {
                auto& score = coordinator->GetComponent<ShootEmUp::Components::Score>(entity);
                score.AddPoints(points);
                
                std::cout << "[GameplayManager] Added " << points << " points to player " << playerId 
                         << ". Score: " << score.currentScore << " (x" << score.comboMultiplier << " combo)" << std::endl;
                
                return;
            }
        }
    }
}

GameplayManager::GameStats GameplayManager::GetGameStats() const {
    return gameStats;
}

void GameplayManager::SetWindowSize(float width, float height) {
    windowWidth = width;
    windowHeight = height;
    std::cout << "[GameplayManager] Window size set to " << width << "x" << height << std::endl;
}

void GameplayManager::LoadDifficulty(const std::string& difficulty) {
    if (difficulty == "easy") {
        enemySpawnRate = 3.0f;
        enemySpeed = 150.0f;
        enemyHealth = 1;
        maxEnemiesOnScreen = 5;
    } else if (difficulty == "normal") {
        enemySpawnRate = 2.0f;
        enemySpeed = 200.0f;
        enemyHealth = 1;
        maxEnemiesOnScreen = 8;
    } else if (difficulty == "hard") {
        enemySpawnRate = 1.0f;
        enemySpeed = 300.0f;
        enemyHealth = 2;
        maxEnemiesOnScreen = 12;
    }
    
    std::cout << "[GameplayManager] Difficulty set to: " << difficulty << std::endl;
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

void GameplayManager::RegisterEntity(ECS::Entity entity) {
    allEntities.push_back(entity);
    
    if (registerEntityCallback) {
        registerEntityCallback(entity);
    }
}

eng::engine::rendering::ITexture* GameplayManager::GetTextureForEnemy(const std::string& enemyType) const {
    if (!textureMap) return nullptr;
    
    auto it = textureMap->find(enemyType);
    if (it != textureMap->end()) {
        return it->second;
    }
    
    // Fallback : chercher la première texture disponible
    if (!textureMap->empty()) {
        return textureMap->begin()->second;
    }
    
    return nullptr;
}

eng::engine::rendering::ISprite* GameplayManager::CreateSpriteFromTexture(const std::string& textureName) {
    auto* texture = GetTextureForEnemy(textureName);
    if (!texture) {
        std::cout << "[GameplayManager] Texture not found: " << textureName << std::endl;
        return nullptr;
    }
    
    auto* sprite = new SFMLSprite();
    sprite->setTexture(texture);
    allSprites->push_back(sprite);
    
    return sprite;
}

float GameplayManager::GetRandomSpawnX() const {
    // Apparition à droite de l'écran
    return windowWidth + 50.0f;
}

float GameplayManager::GetRandomSpawnY() const {
    // Position Y aléatoire dans la fenêtre (avec marges)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(50.0f, windowHeight - 100.0f);
    return dis(gen);
}

std::string GameplayManager::GetRandomEnemyPattern() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 3);
    
    switch (dis(gen)) {
        case 0: return "linear";
        case 1: return "sine";
        case 2: return "zigzag";
        case 3: return "circle";
        default: return "linear";
    }
}

} // namespace RType::Core
