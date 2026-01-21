#pragma once

#include <memory>
#include <ecs/Coordinator.hpp>
#include <engine/Clock.hpp>
#include <engine/Input.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <systems/NetworkSystem.hpp>
#include "core/InputHandler.hpp"
#include "core/AudioManager.hpp"
#include "core/GameplayManager.hpp"
#include <systems/UISystem.hpp>
#include <scripting/LuaState.hpp>
#include <scripting/ScriptSystem.hpp>

namespace RType::Core {

/**
 * @brief Responsable de la boucle principale du jeu
 * 
 * Cette classe gère :
 * - La boucle de jeu principale
 * - Le traitement des événements
 * - La mise à jour des systèmes
 * - Le rendu
 * - La gestion du temps
 */
class GameLoop {
public:
    explicit GameLoop(ECS::Coordinator* coordinator);
    ~GameLoop();

    // ========================================
    // CONFIGURATION
    // ========================================
    
    /**
     * @brief Configure le mode réseau
     */
    void SetNetworkMode(bool enable);
    
    /**
     * @brief Définit le système réseau
     */
    void SetNetworkSystem(std::shared_ptr<eng::engine::systems::NetworkSystem> netSystem);
    
    /**
     * @brief Définit le système UI
     */
    void SetUISystem(std::shared_ptr<UISystem> ui);
    
    /**
     * @brief Définit le gestionnaire audio
     */
    void SetAudioManager(std::shared_ptr<AudioManager> audio);
    
    /**
     * @brief Définit le gestionnaire d'entrées
     */
    void SetInputHandler(std::shared_ptr<InputHandler> input);
    
    /**
     * @brief Définit le gestionnaire de gameplay
     */
    void SetGameplayManager(std::shared_ptr<GameplayManager> gameplay);
    
    /**
     * @brief Définit le système de scripts
     */
    void SetScriptSystem(std::shared_ptr<Scripting::ScriptSystem> script);
    
    /**
     * @brief Définit l'état Lua
     */
    void SetLuaState(Scripting::LuaState* lua);
    
    /**
     * @brief Définit la fenêtre
     */
    void SetWindow(eng::engine::rendering::sfml::SFMLWindow* win);

    // ========================================
    // CONTRÔLE DE LA BOUCLE
    // ========================================
    
    /**
     * @brief Met à jour une frame de jeu
     * @return true si le jeu continue, false si arrêt demandé
     */
    bool Update();
    
    /**
     * @brief Remet à zéro l'état du jeu
     */
    void ResetGameState();

    // ========================================
    // ACCESSEURS
    // ========================================
    
    /**
     * @brief Obtient le temps delta de la dernière frame
     */
    float GetDeltaTime() const;
    
    /**
     * @brief Obtient le temps de jeu total
     */
    float GetGamePlayTime() const;
    
    /**
     * @brief Vérifie si la condition de victoire est atteinte
     */
    bool IsWinConditionTriggered() const;

private:
    // Références vers les systèmes
    ECS::Coordinator* coordinator;
    std::shared_ptr<eng::engine::systems::NetworkSystem> networkSystem;
    std::shared_ptr<UISystem> uiSystem;
    std::shared_ptr<AudioManager> audioManager;
    std::shared_ptr<InputHandler> inputHandler;
    std::shared_ptr<GameplayManager> gameplayManager;
    std::shared_ptr<Scripting::ScriptSystem> spawnScriptSystem;
    Scripting::LuaState* luaState;
    eng::engine::rendering::sfml::SFMLWindow* window;
    
    // Horloge et timing
    eng::engine::Clock clock;
    float deltaTime;
    
    // Variables de jeu
    float enemySpawnTimer;
    float enemySpawnInterval;
    float enemyShootTimer;
    float enemyShootInterval;
    
    // Gestion des entrées
    bool spacePressed;
    float spaceHoldTime;
    ECS::Entity activeChargingEffect;
    bool hasChargingEffect;
    
    // État du jeu
    float gamePlayTime;
    bool winConditionTriggered;
    float winDisplayTimer;
    uint8_t inputMask;
    
    // Configuration
    bool networkMode;
    
    // Méthodes privées
    void HandleEvents();
    void UpdateSystems(float dt, bool inMenu);
    void UpdateGameLogic(float dt);
    void UpdateWinCondition(float dt);
    void UpdateNetworking(float dt);
    void UpdateEnemySpawning(float dt);
    void HandleChargingInput(float dt);
    void Render();
    
    // Utilitaires pour la charge
    int CalculateChargeLevel(float holdTime);
};

} // namespace RType::Core
