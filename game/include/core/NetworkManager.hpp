#pragma once

#include <memory>
#include <string>
#include <functional>
#include <systems/NetworkSystem.hpp>
#include <network/NetworkClient.hpp>
#include "GameStateManager.hpp"

namespace RType::Core {

/**
 * @brief Gestionnaire centralisé du réseau
 * 
 * Cette classe gère :
 * - La connexion/déconnexion au serveur
 * - L'initialisation du client réseau
 * - La configuration des callbacks réseau
 * - La synchronisation des états de jeu
 */
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // ========================================
    // CONNEXION
    // ========================================
    
    /**
     * @brief Initialise et se connecte à un serveur
     * @param address Adresse du serveur
     * @param port Port du serveur
     * @param coordinator Référence vers le coordinateur ECS
     * @return true si la connexion est réussie
     */
    bool ConnectToServer(const std::string& address, short port, ECS::Coordinator* coordinator);
    
    /**
     * @brief Se déconnecte du serveur
     */
    void Disconnect();
    
    /**
     * @brief Vérifie si connecté au serveur
     */
    bool IsConnected() const;
    
    /**
     * @brief Obtient l'adresse du serveur actuel
     */
    std::string GetServerAddress() const;
    
    /**
     * @brief Obtient le port du serveur actuel
     */
    short GetServerPort() const;

    // ========================================
    // SYSTÈMES RÉSEAU
    // ========================================
    
    /**
     * @brief Obtient le client réseau
     */
    std::shared_ptr<NetworkClient> GetNetworkClient() const;
    
    /**
     * @brief Obtient le système réseau
     */
    std::shared_ptr<eng::engine::systems::NetworkSystem> GetNetworkSystem() const;

    // ========================================
    // CALLBACKS
    // ========================================
    
    /**
     * @brief Définit le callback de création d'entité
     */
    void SetEntityCreatedCallback(std::function<void(ECS::Entity)> callback);
    
    /**
     * @brief Définit le callback de destruction d'entité
     */
    void SetEntityDestroyedCallback(std::function<void(ECS::Entity, uint32_t)> callback);
    
    /**
     * @brief Définit le callback de début de partie
     */
    void SetGameStartCallback(std::function<void()> callback);
    
    /**
     * @brief Définit le callback d'état de connexion
     */
    void SetConnectionStatusCallback(std::function<void(bool, const std::string&)> callback);

    // ========================================
    // COMMUNICATION
    // ========================================
    
    /**
     * @brief Envoie les entrées du joueur au serveur
     * @param inputMask Masque des entrées
     */
    void SendInput(uint8_t inputMask);
    
    /**
     * @brief Envoie une demande de basculement de pause
     */
    void SendTogglePause();
    
    /**
     * @brief Envoie un message de chat
     * @param message Message à envoyer
     */
    void SendChatMessage(const std::string& message);

    // ========================================
    // MISE À JOUR
    // ========================================
    
    /**
     * @brief Met à jour le système réseau
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    void Update(float deltaTime);

    // ========================================
    // CONFIGURATION
    // ========================================
    
    /**
     * @brief Charge la configuration réseau depuis Lua
     * @param configPath Chemin vers la configuration
     */
    bool LoadNetworkConfig(const std::string& configPath);
    
    /**
     * @brief Sauvegarde la configuration réseau
     * @param configPath Chemin de sauvegarde
     */
    bool SaveNetworkConfig(const std::string& configPath);
    
    /**
     * @brief Active/désactive le mode debug
     */
    void SetDebugMode(bool enabled);

    // ========================================
    // INFORMATIONS DE CONNEXION
    // ========================================
    
    /**
     * @brief Obtient l'ID du joueur local
     */
    uint8_t GetLocalPlayerId() const;
    
    /**
     * @brief Obtient le ping actuel
     */
    float GetPing() const;
    
    /**
     * @brief Obtient le nombre de paquets perdus
     */
    uint32_t GetPacketLoss() const;
    
    /**
     * @brief Obtient les statistiques de connexion
     */
    struct NetworkStats {
        float ping = 0.0f;
        uint32_t packetsSent = 0;
        uint32_t packetsReceived = 0;
        uint32_t packetsLost = 0;
        float connectionTime = 0.0f;
        std::string serverVersion;
    };
    
    NetworkStats GetNetworkStats() const;

private:
    // Systèmes réseau
    std::shared_ptr<NetworkClient> networkClient;
    std::shared_ptr<eng::engine::systems::NetworkSystem> networkSystem;
    
    // État de la connexion
    bool connected;
    std::string serverAddress;
    short serverPort;
    uint8_t localPlayerId;
    
    // Callbacks
    std::function<void(ECS::Entity)> entityCreatedCallback;
    std::function<void(ECS::Entity, uint32_t)> entityDestroyedCallback;
    std::function<void()> gameStartCallback;
    std::function<void(bool, const std::string&)> connectionStatusCallback;
    
    // Configuration
    float connectionTimeout;
    int retryAttempts;
    float retryDelay;
    bool debugMode;
    
    // Statistiques
    mutable NetworkStats networkStats;
    std::chrono::steady_clock::time_point connectionStartTime;
    
    // Méthodes privées
    bool WaitForServerWelcome();
    void OnConnectionStatusChanged(bool connected, const std::string& message);
    void UpdateNetworkStats();
};

} // namespace RType::Core
