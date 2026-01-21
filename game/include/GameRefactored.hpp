#pragma once

#include <memory>
#include <string>
#include <ecs/Coordinator.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <rendering/sfml/SFMLRenderer.hpp>
#include <scripting/LuaState.hpp>
#include "core/GameInitializer.hpp"
#include "core/GameConfig.hpp"
#include "core/GameLoop.hpp"
#include "core/InputHandler.hpp"
#include "core/AudioManager.hpp"
#include "core/GameplayManager.hpp"
#include "core/NetworkManager.hpp"
#include "core/AssetLoader.hpp"
#include "systems/UISystem.hpp"

namespace RType {

/**
 * @brief Classe principale du jeu R-Type refactorisée
 * 
 * Cette classe orchestre les différents modules :
 * - GameInitializer : Initialisation ECS
 * - GameConfig : Configuration depuis Lua
 * - GameLoop : Boucle principale
 * - InputHandler : Gestion des entrées
 * - AudioManager : Gestion audio
 * - GameplayManager : Logique de gameplay
 * - NetworkManager : Gestion réseau
 * - AssetLoader : Chargement des ressources
 */
class GameRefactored {
public:
    GameRefactored();
    ~GameRefactored();
    
    /**
     * @brief Point d'entrée principal du jeu
     * @param argc Nombre d'arguments
     * @param argv Arguments de la ligne de commande
     * @return Code de retour (0 = succès)
     */
    int Run(int argc, char* argv[]);

private:
    // ========================================
    // MODULES PRINCIPAUX
    // ========================================
    
    // Core ECS
    std::unique_ptr<ECS::Coordinator> coordinator;
    
    // Configuration (Lua state is singleton, just store pointer for convenience)
    Scripting::LuaState* luaState;
    
    // Gestionnaires
    std::unique_ptr<Core::AssetLoader> assetLoader;
    std::shared_ptr<Core::AudioManager> audioManager;
    std::shared_ptr<Core::InputHandler> inputHandler;
    std::shared_ptr<Core::GameplayManager> gameplayManager;
    std::unique_ptr<Core::NetworkManager> networkManager;
    
    // Window et rendering
    std::unique_ptr<eng::engine::rendering::sfml::SFMLWindow> window;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLRenderer> renderer;
    
    // Systèmes
    std::shared_ptr<UISystem> uiSystem;
    
    // Boucle de jeu
    std::unique_ptr<Core::GameLoop> gameLoop;

    // ========================================
    // ÉTAT ET CONFIGURATION
    // ========================================
    
    bool initialized;
    
    // Configuration de la fenêtre
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    
    // Configuration réseau
    bool networkMode;
    bool isNetworkClient;
    std::string legacyServerAddress;
    short legacyServerPort;

    // ========================================
    // MÉTHODES PRIVÉES
    // ========================================
    
    /**
     * @brief Initialise tous les modules du jeu
     * @param argc Nombre d'arguments
     * @param argv Arguments de la ligne de commande
     * @return true si l'initialisation réussit
     */
    bool Initialize(int argc, char* argv[]);
    
    /**
     * @brief Arrête et nettoie tous les modules
     */
    void Shutdown();
    
    /**
     * @brief Parse les arguments de ligne de commande
     * @param argc Nombre d'arguments
     * @param argv Arguments
     */
    void ParseCommandLineArguments(int argc, char* argv[]);
    
    /**
     * @brief Applique la configuration chargée
     */
    void ApplyConfiguration();
    
    /**
     * @brief Initialise tous les systèmes ECS
     * @return true si l'initialisation réussit
     */
    bool InitializeSystems();
    
    /**
     * @brief Configure les callbacks d'entrée
     */
    void SetupInputCallbacks();
    
    /**
     * @brief Configure les callbacks réseau
     */
    void SetupNetworkCallbacks();
    
    /**
     * @brief Configure les callbacks de collision
     */
    void SetupCollisionCallbacks();
    
    /**
     * @brief Gère une collision en mode local
     * @param a Première entité
     * @param b Seconde entité
     */
    void HandleLocalCollision(ECS::Entity a, ECS::Entity b);
    
    /**
     * @brief Charge tous les scripts Lua
     * @return true si le chargement réussit
     */
    bool LoadScripts();
    
    /**
     * @brief Configure tous les bindings Lua
     */
    void SetupLuaBindings();
    
    /**
     * @brief Configure les bindings audio pour Lua
     */
    void SetupAudioBindings();
    
    /**
     * @brief Configure les bindings d'état de jeu pour Lua
     */
    void SetupGameStateBindings();
    
    /**
     * @brief Initialise l'interface utilisateur
     */
    void InitializeUI();
};

} // namespace RType
