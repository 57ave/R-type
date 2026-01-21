#pragma once

#include <ecs/Coordinator.hpp>
#include <ecs/Types.hpp>
#include <scripting/LuaState.hpp>
#include <string>

namespace RType::Entities {

/**
 * @brief Factory responsable de la création de toutes les entités du jeu
 * 
 * Cette classe centralise la création des entités :
 * - Joueurs
 * - Ennemis (via configurations Lua)
 * - Projectiles
 * - Backgrounds
 * - Effets visuels
 * - Entités UI
 */
class EntityFactory {
public:
    /**
     * @brief Initialise la factory avec le coordinateur et l'état Lua
     * @param coordinator Référence vers le coordinateur ECS
     * @param luaState Référence vers l'état Lua
     */
    static void Initialize(ECS::Coordinator* coordinator, Scripting::LuaState* luaState);
    
    // ========================================
    // CRÉATION DE JOUEURS
    // ========================================
    
    /**
     * @brief Crée une entité joueur
     * @param x Position X initiale
     * @param y Position Y initiale
     * @param playerId ID du joueur pour le multijoueur
     * @return ID de l'entité créée
     */
    static ECS::Entity CreatePlayer(float x, float y, int playerId = 1);
    
    // ========================================
    // CRÉATION D'ENNEMIS
    // ========================================
    
    /**
     * @brief Crée un ennemi selon son type
     * @param x Position X de spawn
     * @param y Position Y de spawn
     * @param enemyType Type d'ennemi (défini dans enemies_config.lua)
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateEnemy(float x, float y, const std::string& enemyType);
    
    /**
     * @brief Crée un ennemi depuis une configuration Lua
     * @param x Position X de spawn
     * @param y Position Y de spawn
     * @param enemyConfig Table Lua contenant la configuration de l'ennemi
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateEnemyFromConfig(float x, float y, sol::table enemyConfig);
    
    // ========================================
    // CRÉATION DE PROJECTILES
    // ========================================
    
    /**
     * @brief Crée un projectile
     * @param x Position X initiale
     * @param y Position Y initiale
     * @param velocityX Vitesse X
     * @param velocityY Vitesse Y
     * @param isPlayerProjectile true si c'est un projectile du joueur
     * @param damage Dégâts du projectile
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateProjectile(float x, float y, float velocityX, float velocityY, 
                                      bool isPlayerProjectile = true, int damage = 1);
    
    /**
     * @brief Crée un projectile depuis une configuration d'arme
     * @param x Position X initiale
     * @param y Position Y initiale
     * @param weaponConfig Configuration de l'arme
     * @param isPlayerProjectile true si c'est un projectile du joueur
     * @param ownerId ID du propriétaire du projectile
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateProjectileFromWeapon(float x, float y, sol::table weaponConfig,
                                                 bool isPlayerProjectile = true, int ownerId = 0);
    
    // ========================================
    // CRÉATION D'ARRIÈRE-PLANS
    // ========================================
    
    /**
     * @brief Crée un arrière-plan défilant
     * @param x Position X initiale
     * @param y Position Y initiale
     * @param height Hauteur de l'arrière-plan
     * @param isPrimary true si c'est l'arrière-plan principal
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateBackground(float x, float y, float height, bool isPrimary = true);
    
    // ========================================
    // CRÉATION D'EFFETS VISUELS
    // ========================================
    
    /**
     * @brief Crée un effet d'explosion
     * @param x Position X de l'explosion
     * @param y Position Y de l'explosion
     * @param scale Échelle de l'explosion
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateExplosion(float x, float y, float scale = 1.0f);
    
    /**
     * @brief Crée un effet de tir
     * @param x Position X de l'effet
     * @param y Position Y de l'effet
     * @param parent Entité parente (optionnelle)
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateShootEffect(float x, float y, ECS::Entity parent = 0);
    
    /**
     * @brief Crée un effet générique
     * @param x Position X de l'effet
     * @param y Position Y de l'effet
     * @param effectType Type d'effet
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateEffect(float x, float y, const std::string& effectType);
    
    // ========================================
    // CRÉATION D'ENTITÉS UI
    // ========================================
    
    /**
     * @brief Crée un bouton UI
     * @param x Position X du bouton
     * @param y Position Y du bouton
     * @param width Largeur du bouton
     * @param height Hauteur du bouton
     * @param text Texte du bouton
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateUIButton(float x, float y, float width, float height, 
                                    const std::string& text);
    
    /**
     * @brief Crée un texte UI
     * @param x Position X du texte
     * @param y Position Y du texte
     * @param text Contenu du texte
     * @param fontSize Taille de la police
     * @return ID de l'entité créée
     */
    static ECS::Entity CreateUIText(float x, float y, const std::string& text, int fontSize = 24);

private:
    static ECS::Coordinator* s_coordinator;
    static Scripting::LuaState* s_luaState;
    
    /**
     * @brief Fonction helper pour configurer les composants de base d'une entité
     * @param entity ID de l'entité
     * @param x Position X
     * @param y Position Y
     */
    static void SetupBasicComponents(ECS::Entity entity, float x, float y);
    
    /**
     * @brief Fonction helper pour charger une texture
     * @param texturePath Chemin vers la texture
     * @return true si le chargement réussit
     */
    static bool LoadTexture(const std::string& texturePath);
    
    EntityFactory() = delete;  // Classe utilitaire uniquement
};

} // namespace RType::Entities
