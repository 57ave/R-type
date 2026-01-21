#include "entities/EntityFactory.hpp"
#include "core/GameConfig.hpp"
#include <core/Logger.hpp>

// Include all necessary components
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Damage.hpp>
#include <components/Lifetime.hpp>
#include <components/Tag.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/NetworkId.hpp>

// ShootEmUp components
#include <components/Weapon.hpp>
#include <components/ShootEmUpTags.hpp>
#include <components/MovementPattern.hpp>
#include <components/Effect.hpp>

// UI components
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>

// Rendering
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>

#include <iostream>
#include <memory>

using namespace eng::engine::rendering::sfml;
using namespace eng::engine;

namespace RType::Entities {

// Static member definitions
ECS::Coordinator* EntityFactory::s_coordinator = nullptr;
Scripting::LuaState* EntityFactory::s_luaState = nullptr;

void EntityFactory::Initialize(ECS::Coordinator* coordinator, Scripting::LuaState* luaState) {
    s_coordinator = coordinator;
    s_luaState = luaState;
    
    auto& logger = rtype::core::Logger::getInstance();
    
    if (!s_coordinator) {
        logger.error("EntityFactory", "Coordinator cannot be null");
        throw std::runtime_error("[EntityFactory] Coordinator cannot be null");
    }
    if (!s_luaState) {
        logger.error("EntityFactory", "LuaState cannot be null");
        throw std::runtime_error("[EntityFactory] LuaState cannot be null");
    }
    
    logger.info("EntityFactory", "Initialized successfully");
}

ECS::Entity EntityFactory::CreatePlayer(float x, float y, int playerId) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    std::cout << "[EntityFactory] Creating player at (" << x << ", " << y << ") ID: " << playerId << std::endl;
    
    ECS::Entity player = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(player, pos);
    
    // Velocity (initially zero)
    Velocity vel;
    vel.dx = 0.0f;
    vel.dy = 0.0f;
    s_coordinator->AddComponent(player, vel);
    
    // Health
    Health health;
    const auto& config = Core::GameConfig::GetConfiguration();
    health.current = config.player.health;
    health.max = config.player.health;
    s_coordinator->AddComponent(player, health);
    
    // Collider
    Collider collider;
    collider.width = 32.0f;
    collider.height = 32.0f;
    s_coordinator->AddComponent(player, collider);
    
    // Player tag
    s_coordinator->AddComponent(player, ShootEmUp::Components::PlayerTag{});
    s_coordinator->AddComponent(player, Tag{"player"});
    
    // Network ID if needed
    if (playerId > 0) {
        NetworkId networkId;
        networkId.networkId = static_cast<uint32_t>(playerId);
        networkId.playerId = static_cast<uint8_t>(playerId);
        networkId.isLocalPlayer = false;
        s_coordinator->AddComponent(player, networkId);
    }
    
    std::cout << "[EntityFactory] Player " << player << " created successfully" << std::endl;
    return player;
}

ECS::Entity EntityFactory::CreateEnemy(float x, float y, const std::string& enemyType) {
    if (!s_coordinator || !s_luaState) {
        std::cerr << "[EntityFactory] ERROR: Not properly initialized" << std::endl;
        return 0;
    }
    
    std::cout << "[EntityFactory] Creating enemy '" << enemyType << "' at (" << x << ", " << y << ")" << std::endl;
    
    try {
        // Try to use Lua factory first
        auto& lua = s_luaState->GetState();
        sol::protected_function factory = lua["Factory"]["CreateEnemyFromType"];
        
        if (factory.valid()) {
            auto result = factory(x, y, enemyType);
            if (result.valid()) {
                ECS::Entity enemy = result;
                if (enemy != 0) {
                    std::cout << "[EntityFactory] Enemy " << enemy << " created via Lua factory" << std::endl;
                    return enemy;
                }
            }
        }
        
        // Fallback to basic enemy creation
        std::cout << "[EntityFactory] Using fallback enemy creation for " << enemyType << std::endl;
        
        ECS::Entity enemy = s_coordinator->CreateEntity();
        
        // Position
        Position pos;
        pos.x = x;
        pos.y = y;
        s_coordinator->AddComponent(enemy, pos);
        
        // Velocity (moving left by default)
        Velocity vel;
        vel.dx = -200.0f;  // Moving left
        vel.dy = 0.0f;
        s_coordinator->AddComponent(enemy, vel);
        
        // Health (default values)
        Health health;
        health.current = 30;
        health.max = 30;
        s_coordinator->AddComponent(enemy, health);
        
        // Collider
        Collider collider;
        collider.width = 32.0f;
        collider.height = 32.0f;
        s_coordinator->AddComponent(enemy, collider);
        
        // Enemy tag
        s_coordinator->AddComponent(enemy, ShootEmUp::Components::EnemyTag{});
        s_coordinator->AddComponent(enemy, Tag{"enemy"});
        
        std::cout << "[EntityFactory] Basic enemy " << enemy << " created" << std::endl;
        return enemy;
        
    } catch (const std::exception& e) {
        std::cerr << "[EntityFactory] ERROR creating enemy: " << e.what() << std::endl;
        return 0;
    }
}

ECS::Entity EntityFactory::CreateEnemyFromConfig(float x, float y, sol::table enemyConfig) {
    if (!s_coordinator || !s_luaState) {
        std::cerr << "[EntityFactory] ERROR: Not properly initialized" << std::endl;
        return 0;
    }
    
    try {
        auto& lua = s_luaState->GetState();
        sol::protected_function factory = lua["Factory"]["CreateEnemyFromConfig"];
        
        if (factory.valid()) {
            auto result = factory(x, y, enemyConfig);
            if (result.valid()) {
                ECS::Entity enemy = result;
                std::cout << "[EntityFactory] Enemy created from config at (" << x << ", " << y << ")" << std::endl;
                return enemy;
            }
        }
        
        std::cerr << "[EntityFactory] ERROR: Lua CreateEnemyFromConfig not available" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[EntityFactory] ERROR creating enemy from config: " << e.what() << std::endl;
        return 0;
    }
}

ECS::Entity EntityFactory::CreateProjectile(float x, float y, float velocityX, float velocityY, 
                                           bool isPlayerProjectile, int damage) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity projectile = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(projectile, pos);
    
    // Velocity
    Velocity vel;
    vel.dx = velocityX;
    vel.dy = velocityY;
    s_coordinator->AddComponent(projectile, vel);
    
    // Damage
    Damage dmg;
    dmg.amount = damage;
    s_coordinator->AddComponent(projectile, dmg);
    
    // Collider
    Collider collider;
    collider.width = 8.0f;
    collider.height = 8.0f;
    s_coordinator->AddComponent(projectile, collider);
    
    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;  // 5 seconds before auto-destruction
    s_coordinator->AddComponent(projectile, lifetime);
    
    // Tags
    s_coordinator->AddComponent(projectile, ShootEmUp::Components::ProjectileTag{});
    s_coordinator->AddComponent(projectile, Tag{isPlayerProjectile ? "player_projectile" : "enemy_projectile"});
    
    std::cout << "[EntityFactory] Projectile " << projectile << " created" << std::endl;
    return projectile;
}

ECS::Entity EntityFactory::CreateProjectileFromWeapon(float x, float y, sol::table weaponConfig,
                                                     bool isPlayerProjectile, int ownerId) {
    if (!s_coordinator || !s_luaState) {
        std::cerr << "[EntityFactory] ERROR: Not properly initialized" << std::endl;
        return 0;
    }
    
    try {
        auto& lua = s_luaState->GetState();
        sol::protected_function factory = lua["Factory"]["CreateProjectileFromWeaponConfig"];
        
        if (factory.valid()) {
            auto result = factory(x, y, weaponConfig, isPlayerProjectile, ownerId);
            if (result.valid()) {
                ECS::Entity projectile = result;
                std::cout << "[EntityFactory] Projectile created from weapon config" << std::endl;
                return projectile;
            }
        }
        
        std::cerr << "[EntityFactory] ERROR: Lua weapon factory not available" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[EntityFactory] ERROR creating projectile from weapon: " << e.what() << std::endl;
        return 0;
    }
}

ECS::Entity EntityFactory::CreateBackground(float x, float y, float height, bool isPrimary) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity background = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(background, pos);
    
    // Scrolling Background component
    ScrollingBackground scrollBg;
    scrollBg.scrollSpeed = isPrimary ? 50.0f : 25.0f;  // Primary moves faster
    scrollBg.spriteWidth = 1920.0f;  // Will be updated from config
    scrollBg.horizontal = true;
    scrollBg.loop = true;
    s_coordinator->AddComponent(background, scrollBg);
    
    // Tag
    s_coordinator->AddComponent(background, Tag{isPrimary ? "primary_background" : "secondary_background"});
    
    std::cout << "[EntityFactory] Background " << background << " created" << std::endl;
    return background;
}

ECS::Entity EntityFactory::CreateExplosion(float x, float y, float scale) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity explosion = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(explosion, pos);
    
    // Animation (will be set up with sprite when loaded)
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
    s_coordinator->AddComponent(explosion, anim);
    
    // Lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 1.0f;
    s_coordinator->AddComponent(explosion, lifetime);
    
    // Effect component
    ShootEmUp::Components::Effect effect;
    effect.effectType = "explosion";
    effect.duration = 0.6f;
    effect.loop = false;
    s_coordinator->AddComponent(explosion, effect);
    
    // Tag
    s_coordinator->AddComponent(explosion, Tag{"explosion"});
    
    std::cout << "[EntityFactory] Explosion " << explosion << " created" << std::endl;
    return explosion;
}

ECS::Entity EntityFactory::CreateShootEffect(float x, float y, ECS::Entity parent) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity effect = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(effect, pos);
    
    // Lifetime (short effect)
    Lifetime lifetime;
    lifetime.maxLifetime = 0.2f;
    s_coordinator->AddComponent(effect, lifetime);
    
    // Effect component
    ShootEmUp::Components::Effect effectComp;
    effectComp.effectType = "shoot";
    effectComp.followParent = (parent != 0);
    effectComp.duration = 0.15f;
    s_coordinator->AddComponent(effect, effectComp);
    
    // Tag
    s_coordinator->AddComponent(effect, Tag{"shoot_effect"});
    
    std::cout << "[EntityFactory] Shoot effect " << effect << " created" << std::endl;
    return effect;
}

ECS::Entity EntityFactory::CreateEffect(float x, float y, const std::string& effectType) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity effect = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(effect, pos);
    
    // Effect component
    ShootEmUp::Components::Effect effectComp;
    effectComp.effectType = effectType;
    s_coordinator->AddComponent(effect, effectComp);
    
    // Default lifetime
    Lifetime lifetime;
    lifetime.maxLifetime = 1.0f;
    s_coordinator->AddComponent(effect, lifetime);
    
    // Tag
    s_coordinator->AddComponent(effect, Tag{effectType + "_effect"});
    
    std::cout << "[EntityFactory] Effect '" << effectType << "' " << effect << " created" << std::endl;
    return effect;
}

ECS::Entity EntityFactory::CreateUIButton(float x, float y, float width, float height, 
                                         const std::string& text) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity button = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(button, pos);
    
    // UI Element
    Components::UIElement uiElement;
    uiElement.width = width;
    uiElement.height = height;
    uiElement.visible = true;
    s_coordinator->AddComponent(button, uiElement);
    
    // UI Button
    Components::UIButton uiButton;
    uiButton.text = text;
    uiButton.enabled = true;
    s_coordinator->AddComponent(button, uiButton);
    
    // Tag
    s_coordinator->AddComponent(button, Tag{"ui_button"});
    
    std::cout << "[EntityFactory] UI Button " << button << " created" << std::endl;
    return button;
}

ECS::Entity EntityFactory::CreateUIText(float x, float y, const std::string& text, int fontSize) {
    if (!s_coordinator) {
        std::cerr << "[EntityFactory] ERROR: Coordinator not initialized" << std::endl;
        return 0;
    }
    
    ECS::Entity textEntity = s_coordinator->CreateEntity();
    
    // Position
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(textEntity, pos);
    
    // UI Element
    Components::UIElement uiElement;
    uiElement.width = static_cast<float>(text.length() * fontSize * 0.6f);  // Approximate width
    uiElement.height = static_cast<float>(fontSize);
    uiElement.visible = true;
    s_coordinator->AddComponent(textEntity, uiElement);
    
    // UI Text
    Components::UIText uiText;
    uiText.content = text;
    uiText.fontSize = fontSize;
    uiText.color = 0xFFFFFFFF;  // White color by default
    s_coordinator->AddComponent(textEntity, uiText);
    
    // Tag
    s_coordinator->AddComponent(textEntity, Tag{"ui_text"});
    
    std::cout << "[EntityFactory] UI Text " << textEntity << " created" << std::endl;
    return textEntity;
}

void EntityFactory::SetupBasicComponents(ECS::Entity entity, float x, float y) {
    if (!s_coordinator) return;
    
    Position pos;
    pos.x = x;
    pos.y = y;
    s_coordinator->AddComponent(entity, pos);
}

bool EntityFactory::LoadTexture(const std::string& texturePath) {
    // This would be implemented based on the texture loading system
    // For now, just return true as a placeholder
    std::cout << "[EntityFactory] Loading texture: " << texturePath << std::endl;
    return true;
}

} // namespace RType::Entities
