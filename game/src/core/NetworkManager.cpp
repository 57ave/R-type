#include "core/NetworkManager.hpp"
#include "network/RTypeProtocol.hpp"
#include "network/NetworkBindings.hpp"
#include <iostream>
#include <chrono>
#include <thread>

namespace RType::Core {

NetworkManager::NetworkManager()
    : connected(false)
    , localPlayerId(0)
    , connectionTimeout(3.0f)  // Réduit de 5s à 3s pour éviter les freeze
    , retryAttempts(3)
    , retryDelay(1.0f)
    , debugMode(false)
{
    std::cout << "[NetworkManager] Created" << std::endl;
}

NetworkManager::~NetworkManager() {
    Disconnect();
    std::cout << "[NetworkManager] Destroyed" << std::endl;
}

bool NetworkManager::ConnectToServer(const std::string& address, short port, ECS::Coordinator* coordinator) {
    if (connected) {
        std::cout << "[NetworkManager] Already connected to server" << std::endl;
        return true;
    }

    serverAddress = address;
    serverPort = port;
    connectionStartTime = std::chrono::steady_clock::now();

    std::cout << "[NetworkManager] 🌐 Connecting to " << address << ":" << port << std::endl;

    try {
        // Créer le client réseau
        networkClient = std::make_shared<NetworkClient>(address, port);
        networkSystem = std::make_shared<eng::engine::systems::NetworkSystem>(coordinator, networkClient);

        // Configurer les callbacks du système réseau
        if (entityCreatedCallback) {
            networkSystem->setEntityCreatedCallback(entityCreatedCallback);
        }
        
        if (entityDestroyedCallback) {
            networkSystem->setEntityDestroyedCallback(entityDestroyedCallback);
        }
        
        if (gameStartCallback) {
            networkSystem->setGameStartCallback(gameStartCallback);
        }

        // Démarrer la connexion
        networkClient->start();
        networkClient->sendHello();

        // Attendre la réponse du serveur
        if (WaitForServerWelcome()) {
            connected = true;
            OnConnectionStatusChanged(true, "Connected successfully");
            std::cout << "[NetworkManager] ✅ Connected! Player ID: " << (int)localPlayerId << std::endl;
            return true;
        } else {
            std::cerr << "[NetworkManager] ⚠️ Connection timeout!" << std::endl;
            // Notifier AVANT de détruire les objets réseau
            OnConnectionStatusChanged(false, "Connection timeout");
            // NE PAS appeler disconnect() car on n'a jamais établi de connexion (pas de WELCOME reçu)
            // Reset directement pour éviter que le destructeur n'essaie d'envoyer un paquet
            networkClient.reset();
            networkSystem.reset();
            // Mettre à null dans NetworkBindings pour éviter les accès à un objet détruit
            RType::Network::NetworkBindings::SetNetworkClient(nullptr);
            return false;
        }

    } catch (const std::exception& e) {
        std::cerr << "[NetworkManager] ❌ Connection error: " << e.what() << std::endl;
        // Notifier AVANT de détruire les objets réseau
        OnConnectionStatusChanged(false, std::string("Connection error: ") + e.what());
        // NE PAS appeler disconnect() en cas d'erreur pour éviter un crash
        // Reset directement
        networkClient.reset();
        networkSystem.reset();
        // Mettre à null dans NetworkBindings pour éviter les accès à un objet détruit
        RType::Network::NetworkBindings::SetNetworkClient(nullptr);
        return false;
    }
}

void NetworkManager::Disconnect() {
    if (!connected) return;

    std::cout << "[NetworkManager] Disconnecting from server..." << std::endl;

    if (networkClient) {
        networkClient.reset();
    }
    
    if (networkSystem) {
        networkSystem.reset();
    }

    connected = false;
    localPlayerId = 0;
    
    OnConnectionStatusChanged(false, "Disconnected");
    std::cout << "[NetworkManager] Disconnected" << std::endl;
}

bool NetworkManager::IsConnected() const {
    return connected && networkClient && networkClient->isConnected();
}

std::string NetworkManager::GetServerAddress() const {
    return serverAddress;
}

short NetworkManager::GetServerPort() const {
    return serverPort;
}

std::shared_ptr<NetworkClient> NetworkManager::GetNetworkClient() const {
    return networkClient;
}

std::shared_ptr<eng::engine::systems::NetworkSystem> NetworkManager::GetNetworkSystem() const {
    return networkSystem;
}

void NetworkManager::SetEntityCreatedCallback(std::function<void(ECS::Entity)> callback) {
    entityCreatedCallback = callback;
    
    if (networkSystem) {
        networkSystem->setEntityCreatedCallback(callback);
    }
}

void NetworkManager::SetEntityDestroyedCallback(std::function<void(ECS::Entity, uint32_t)> callback) {
    entityDestroyedCallback = callback;
    
    if (networkSystem) {
        networkSystem->setEntityDestroyedCallback(callback);
    }
}

void NetworkManager::SetGameStartCallback(std::function<void()> callback) {
    gameStartCallback = callback;
    
    if (networkSystem) {
        networkSystem->setGameStartCallback(callback);
    }
}

void NetworkManager::SetConnectionStatusCallback(std::function<void(bool, const std::string&)> callback) {
    connectionStatusCallback = callback;
}

void NetworkManager::SendInput(uint8_t inputMask) {
    if (!connected || !networkSystem) return;

    networkSystem->sendInput(inputMask);
    
    if (debugMode) {
        std::cout << "[NetworkManager] Sent input: " << (int)inputMask << std::endl;
    }
}

void NetworkManager::SendTogglePause() {
    if (!connected || !networkSystem) return;

    networkSystem->sendTogglePause();
    
    if (debugMode) {
        std::cout << "[NetworkManager] Sent toggle pause" << std::endl;
    }
}

void NetworkManager::SendChatMessage(const std::string& message) {
    if (!connected || !networkClient) return;

    // TODO: Implémenter l'envoi de messages de chat
    std::cout << "[NetworkManager] Chat: " << message << std::endl;
}

void NetworkManager::Update(float deltaTime) {
    if (!connected || !networkSystem) return;

    // Mettre à jour le système réseau
    networkSystem->Update(deltaTime);
    
    // Mettre à jour les statistiques
    UpdateNetworkStats();
}

bool NetworkManager::LoadNetworkConfig(const std::string& configPath) {
    // TODO: Implémenter le chargement depuis Lua
    std::cout << "[NetworkManager] Loading network config from: " << configPath << std::endl;
    return true;
}

bool NetworkManager::SaveNetworkConfig(const std::string& configPath) {
    // TODO: Implémenter la sauvegarde vers Lua
    std::cout << "[NetworkManager] Saving network config to: " << configPath << std::endl;
    return true;
}

void NetworkManager::SetDebugMode(bool enabled) {
    debugMode = enabled;
    std::cout << "[NetworkManager] Debug mode: " << (enabled ? "enabled" : "disabled") << std::endl;
}

uint8_t NetworkManager::GetLocalPlayerId() const {
    return localPlayerId;
}

float NetworkManager::GetPing() const {
    return networkStats.ping;
}

uint32_t NetworkManager::GetPacketLoss() const {
    return networkStats.packetsLost;
}

NetworkManager::NetworkStats NetworkManager::GetNetworkStats() const {
    return networkStats;
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

bool NetworkManager::WaitForServerWelcome() {
    if (!networkClient) return false;

    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        networkClient->process();
        
        if (networkClient->hasReceivedPackets()) {
            auto packet = networkClient->getNextReceivedPacket();
            
            if (static_cast<GamePacketType>(packet.header.type) == GamePacketType::SERVER_WELCOME) {
                if (packet.payload.size() >= 1) {
                    localPlayerId = static_cast<uint8_t>(packet.payload[0]);
                    networkSystem->setLocalPlayerId(localPlayerId);
                    return true;
                }
            }
        }

        // Vérifier le timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        if (elapsed > connectionTimeout * 1000) {
            std::cerr << "[NetworkManager] Timeout after " << elapsed << "ms" << std::endl;
            return false;
        }

        // Petite pause pour ne pas saturer le CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void NetworkManager::OnConnectionStatusChanged(bool isConnected, const std::string& message) {
    if (connectionStatusCallback) {
        connectionStatusCallback(isConnected, message);
    }
    
    if (debugMode) {
        std::cout << "[NetworkManager] Connection status changed: " 
                  << (isConnected ? "connected" : "disconnected")
                  << " - " << message << std::endl;
    }
}

void NetworkManager::UpdateNetworkStats() {
    if (!networkClient) return;

    // Calculer le temps de connexion
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - connectionStartTime);
    networkStats.connectionTime = elapsed.count() / 1000.0f;

    // TODO: Obtenir les vraies statistiques du client réseau
    // Pour l'instant, valeurs par défaut
    networkStats.ping = 50.0f;  // 50ms de ping par défaut
    networkStats.packetsSent = 100;
    networkStats.packetsReceived = 98;
    networkStats.packetsLost = 2;
    networkStats.serverVersion = "1.0.0";
}

} // namespace RType::Core
