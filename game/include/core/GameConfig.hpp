#pragma once

#include <string>
#include <scripting/LuaState.hpp>

namespace RType::Core {

/**
 * @brief Structure contenant la configuration du jeu chargée depuis Lua
 */
struct GameConfiguration {
    // Configuration fenêtre
    struct WindowConfig {
        int width = 1920;
        int height = 1080;
        std::string title = "R-Type - ECS Version";
        bool fullscreen = false;
        bool vsync = true;
    } window;
    
    // Configuration réseau
    struct NetworkConfig {
        std::string startMode = "local";        // "local" ou "network"
        bool autoConnect = false;
        
        struct ServerConfig {
            std::string defaultAddress = "127.0.0.1";
            int defaultPort = 12345;
        } server;
        
        struct ConnectionConfig {
            int timeoutMs = 5000;
            int retryAttempts = 3;
            int retryDelayMs = 1000;
        } connection;
    } network;
    
    // Configuration joueur
    struct PlayerConfig {
        float startX = 100.0f;
        float startY = 400.0f;
        int health = 1;
        float speed = 500.0f;
    } player;
};

/**
 * @brief Responsable du chargement et de la gestion des configurations
 * 
 * Cette classe gère :
 * - Chargement des configurations depuis les fichiers Lua
 * - Validation des configurations
 * - Accès centralisé aux paramètres du jeu
 */
class GameConfig {
public:
    /**
     * @brief Charge toutes les configurations depuis les fichiers Lua
     * @param luaState État Lua pour charger les configurations
     * @return true si toutes les configurations sont chargées avec succès
     */
    static bool LoadConfiguration(Scripting::LuaState& luaState);
    
    /**
     * @brief Obtient la configuration actuelle du jeu
     * @return Référence vers la configuration
     */
    static const GameConfiguration& GetConfiguration();
    
    /**
     * @brief Applique la configuration fenêtre au système
     * @param config Configuration à appliquer
     */
    static void ApplyWindowConfiguration(const GameConfiguration::WindowConfig& config);
    
    /**
     * @brief Résout le chemin d'un asset relativement au répertoire de base
     * @param relativePath Chemin relatif de l'asset
     * @return Chemin absolu résolu
     */
    static std::string ResolveAssetPath(const std::string& relativePath);

private:
    static GameConfiguration s_config;
    static std::string s_basePath;
    
    /**
     * @brief Charge la configuration réseau depuis Lua
     * @param lua État Lua
     * @return true si le chargement réussit
     */
    static bool LoadNetworkConfig(sol::state& lua);
    
    /**
     * @brief Charge la configuration de jeu depuis Lua
     * @param lua État Lua
     * @return true si le chargement réussit
     */
    static bool LoadGameConfig(sol::state& lua);
    
    /**
     * @brief Résout le chemin de base des assets
     */
    static void ResolveBasePath();
    
    GameConfig() = delete;  // Classe utilitaire uniquement
};

} // namespace RType::Core
