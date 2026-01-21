#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <ecs/Coordinator.hpp>
#include <rendering/ITexture.hpp>
#include <rendering/ISprite.hpp>

namespace RType::Core {

/**
 * @brief Gestionnaire centralisé de la logique de gameplay
 * 
 * Cette classe gère :
 * - La création d'entités de jeu (player, enemies, projectiles)
 * - La logique de spawn des ennemis
 * - Le système de score et de victoire
 * - Les effets spéciaux
 * - La gestion des armes et du tir
 */
class GameplayManager {
public:
    GameplayManager(ECS::Coordinator* coordinator);
    ~GameplayManager();

    // ========================================
    // INITIALISATION
    // ========================================
    
    /**
     * @brief Initialise le gestionnaire avec les textures
     * @param textureMap Map des textures préchargées
     * @param allSprites Liste des sprites à gérer
     */
    void Initialize(
        std::unordered_map<std::string, eng::engine::rendering::ITexture*>& textureMap,
        std::vector<eng::engine::rendering::ISprite*>& allSprites
    );
    
    /**
     * @brief Définit la fonction de registration d'entités
     */
    void SetEntityRegistrationCallback(std::function<void(ECS::Entity)> callback);

    // ========================================
    // CRÉATION D'ENTITÉS
    // ========================================
    
    /**
     * @brief Crée l'entité joueur
     * @param x Position X initiale
     * @param y Position Y initiale
     * @param line Ligne de sprite du joueur (0-3 pour différents joueurs)
     */
    ECS::Entity CreatePlayer(float x, float y, int line = 0);
    
    /**
     * @brief Crée un ennemi
     * @param x Position X
     * @param y Position Y
     * @param patternType Type de mouvement
     * @param enemyType Type d'ennemi (optionnel)
     */
    ECS::Entity CreateEnemy(float x, float y, const std::string& patternType = "linear", const std::string& enemyType = "basic");
    
    /**
     * @brief Crée un projectile de joueur
     * @param x Position X
     * @param y Position Y
     * @param isCharged Si c'est un tir chargé
     * @param chargeLevel Niveau de charge (1-5)
     */
    ECS::Entity CreateMissile(float x, float y, bool isCharged = false, int chargeLevel = 1);
    
    /**
     * @brief Crée un projectile d'ennemi
     * @param x Position X
     * @param y Position Y
     * @param direction Direction du projectile
     */
    ECS::Entity CreateEnemyMissile(float x, float y, float directionX = -1.0f, float directionY = 0.0f);
    
    /**
     * @brief Crée un effet d'explosion
     * @param x Position X
     * @param y Position Y
     */
    ECS::Entity CreateExplosion(float x, float y);
    
    /**
     * @brief Crée un effet de tir
     * @param x Position X
     * @param y Position Y
     * @param parent Entité parent (optionnel)
     */
    ECS::Entity CreateShootEffect(float x, float y, ECS::Entity parent = 0);
    
    /**
     * @brief Crée un effet de charge
     * @param player Entité du joueur
     */
    ECS::Entity CreateChargingEffect(ECS::Entity player);

    // ========================================
    // LOGIQUE DE GAMEPLAY
    // ========================================
    
    /**
     * @brief Fait tirer le joueur
     * @param playerEntity Entité du joueur
     */
    void FireMissile(ECS::Entity playerEntity);
    
    /**
     * @brief Fait tirer un missile chargé
     * @param playerEntity Entité du joueur
     * @param chargeLevel Niveau de charge
     */
    void FireChargedMissile(ECS::Entity playerEntity, int chargeLevel);
    
    /**
     * @brief Fait apparaître un ennemi aléatoire
     */
    void SpawnRandomEnemy();
    
    /**
     * @brief Fait tirer tous les ennemis
     */
    void MakeEnemiesShoot();
    
    /**
     * @brief Obtient l'entité du joueur local
     */
    ECS::Entity GetLocalPlayerEntity() const;
    
    /**
     * @brief Vérifie les conditions de victoire
     */
    bool CheckWinCondition() const;
    
    /**
     * @brief Vérifie les conditions de défaite
     */
    bool CheckLoseCondition() const;

    // ========================================
    // GESTION DES RESSOURCES
    // ========================================
    
    /**
     * @brief Nettoie les entités marquées pour destruction
     */
    void ProcessDestroyedEntities();
    
    /**
     * @brief Marque une entité pour destruction différée
     * @param entity Entité à détruire
     */
    void DestroyEntityDeferred(ECS::Entity entity);
    
    /**
     * @brief Supprime toutes les entités de jeu (ennemis, projectiles, powerups)
     * Appelé lors du reset complet de la partie
     */
    void ClearAllGameEntities();

    // ========================================
    // STATISTICS ET SCORING
    // ========================================
    
    /**
     * @brief Obtient le score actuel du joueur
     * @param playerId ID du joueur (défaut: 0 pour joueur local)
     */
    uint32_t GetPlayerScore(int playerId = 0) const;
    
    /**
     * @brief Ajoute des points au score d'un joueur
     * @param points Points à ajouter
     * @param playerId ID du joueur
     */
    void AddScore(uint32_t points, int playerId = 0);
    
    /**
     * @brief Obtient les statistiques de jeu
     */
    struct GameStats {
        uint32_t enemiesKilled = 0;
        uint32_t shotsFired = 0;
        uint32_t accuracy = 0; // Pourcentage
        float playTime = 0.0f;
    };
    
    GameStats GetGameStats() const;

    // ========================================
    // CONFIGURATION
    // ========================================
    
    /**
     * @brief Définit la taille de la fenêtre pour le système de spawn
     * @param width Largeur
     * @param height Hauteur
     */
    void SetWindowSize(float width, float height);
    
    /**
     * @brief Charge la configuration de difficulté
     * @param difficulty Nom de la difficulté ("easy", "normal", "hard")
     */
    void LoadDifficulty(const std::string& difficulty);

private:
    ECS::Coordinator* coordinator;
    
    // Références vers les ressources du jeu
    std::unordered_map<std::string, eng::engine::rendering::ITexture*>* textureMap;
    std::vector<eng::engine::rendering::ISprite*>* allSprites;
    
    // Callback pour l'enregistrement d'entités
    std::function<void(ECS::Entity)> registerEntityCallback;
    
    // Listes de gestion des entités
    std::vector<ECS::Entity> allEntities;
    std::vector<ECS::Entity> entitiesToDestroy;
    
    // Configuration de la fenêtre
    float windowWidth;
    float windowHeight;
    
    // Configuration de difficulté
    float enemySpawnRate;
    float enemySpeed;
    float enemyHealth;
    int maxEnemiesOnScreen;
    
    // Statistiques
    mutable GameStats gameStats;
    
    // Méthodes privées
    void RegisterEntity(ECS::Entity entity);
    eng::engine::rendering::ITexture* GetTextureForEnemy(const std::string& enemyType) const;
    eng::engine::rendering::ISprite* CreateSpriteFromTexture(const std::string& textureName);
    float GetRandomSpawnX() const;
    float GetRandomSpawnY() const;
    std::string GetRandomEnemyPattern() const;
    
    // Configuration des armes par défaut
    struct WeaponConfig {
        float fireRate = 0.2f;
        float projectileSpeed = 1000.0f;
        bool supportsCharge = true;
        float minChargeTime = 0.1f;
        float maxChargeTime = 1.0f;
    };
    
    WeaponConfig defaultWeaponConfig;
};

} // namespace RType::Core
