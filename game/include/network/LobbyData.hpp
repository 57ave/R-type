#ifndef RTYPE_GAME_NETWORK_LOBBYDATA_HPP
#define RTYPE_GAME_NETWORK_LOBBYDATA_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace Network {

    /**
     * @brief Information about a player in the lobby
     */
    struct PlayerInfo {
        uint32_t id = 0;                    // Unique player ID
        std::string pseudo = "Player";      // Player display name
        bool isReady = false;               // Ready to start
        bool isHost = false;                // Is this player the host
        int shipType = 0;                   // Selected ship type (0-3)
        int team = 0;                       // Team number (for team modes)
        
        // Network info
        std::string address;                // IP address (for host display)
        int ping = -1;                      // Current ping to server (-1 = unknown)

        PlayerInfo() = default;
        PlayerInfo(uint32_t playerId, const std::string& name, bool host = false)
            : id(playerId), pseudo(name), isHost(host) {}
    };

    /**
     * @brief Lobby state and configuration
     */
    struct LobbyInfo {
        std::string lobbyId;                // Unique lobby identifier
        std::string name = "R-Type Lobby";  // Display name
        std::string hostName;               // Host player name
        std::string password;               // Password (empty = public)
        
        std::vector<PlayerInfo> players;    // Players in lobby
        int maxPlayers = 4;                 // Maximum players allowed
        
        // Game settings
        int difficulty = 1;                 // 0=Easy, 1=Normal, 2=Hard
        int gameMode = 0;                   // 0=Coop, 1=Versus, 2=Survival
        std::string mapName = "Stage 1";    // Selected map/stage
        
        // Status
        enum class Status {
            Waiting,    // Waiting for players
            Starting,   // Countdown to start
            InGame,     // Game in progress
            Finished    // Game ended
        };
        Status status = Status::Waiting;
        
        // Countdown
        int countdownSeconds = 0;           // Seconds until game starts
        
        LobbyInfo() = default;
        LobbyInfo(const std::string& id, const std::string& lobbyName)
            : lobbyId(id), name(lobbyName) {}

        // Helper methods
        int getPlayerCount() const { return static_cast<int>(players.size()); }
        bool isFull() const { return getPlayerCount() >= maxPlayers; }
        bool isPublic() const { return password.empty(); }
        bool allPlayersReady() const {
            if (players.empty()) return false;
            for (const auto& p : players) {
                if (!p.isReady && !p.isHost) return false;
            }
            return true;
        }
        
        PlayerInfo* findPlayer(uint32_t playerId) {
            for (auto& p : players) {
                if (p.id == playerId) return &p;
            }
            return nullptr;
        }
        
        PlayerInfo* getHost() {
            for (auto& p : players) {
                if (p.isHost) return &p;
            }
            return nullptr;
        }
    };

    /**
     * @brief Information about a server in the server list
     */
    struct ServerInfo {
        std::string ip;                     // Server IP address
        uint16_t port = 4242;               // Server port
        std::string name = "R-Type Server"; // Server display name
        std::string version;                // Server game version
        
        int currentPlayers = 0;             // Current player count
        int maxPlayers = 4;                 // Maximum players
        int ping = -1;                      // Ping in ms (-1 = unknown)
        
        bool isOfficial = false;            // Official server flag
        bool isPasswordProtected = false;   // Requires password
        bool isOnline = true;               // Server responding
        
        // Game info
        std::string gameMode;               // Current game mode
        std::string mapName;                // Current map
        
        ServerInfo() = default;
        ServerInfo(const std::string& serverIp, uint16_t serverPort, const std::string& serverName)
            : ip(serverIp), port(serverPort), name(serverName) {}
        
        // Connection string
        std::string getConnectionString() const {
            return ip + ":" + std::to_string(port);
        }
        
        bool hasSpace() const { return currentPlayers < maxPlayers; }
    };

    /**
     * @brief Server list with refresh functionality
     */
    struct ServerList {
        std::vector<ServerInfo> servers;
        bool isRefreshing = false;
        std::string lastError;
        
        // Master server info
        std::string masterServerUrl = "http://localhost:8080/servers";
        
        // Filter options
        bool showFull = true;               // Show full servers
        bool showEmpty = true;              // Show empty servers
        bool showPasswordProtected = true;  // Show password-protected servers
        bool showOfficialOnly = false;      // Only official servers
        
        ServerList() = default;
        
        void clear() {
            servers.clear();
            lastError.clear();
        }
        
        // Apply filters and return filtered list
        std::vector<ServerInfo> getFiltered() const {
            std::vector<ServerInfo> result;
            for (const auto& server : servers) {
                if (!showFull && !server.hasSpace()) continue;
                if (!showEmpty && server.currentPlayers == 0) continue;
                if (!showPasswordProtected && server.isPasswordProtected) continue;
                if (showOfficialOnly && !server.isOfficial) continue;
                result.push_back(server);
            }
            return result;
        }
    };

    /**
     * @brief Chat message for lobby chat
     */
    struct ChatMessage {
        uint32_t senderId = 0;
        std::string senderName;
        std::string message;
        uint64_t timestamp = 0;             // Unix timestamp
        bool isSystem = false;              // System message (join/leave/etc)
        
        ChatMessage() = default;
        ChatMessage(const std::string& name, const std::string& msg, bool system = false)
            : senderName(name), message(msg), isSystem(system) {}
    };

    /**
     * @brief Lobby chat history
     */
    struct LobbyChat {
        std::vector<ChatMessage> messages;
        size_t maxMessages = 100;           // Max messages to keep
        
        void addMessage(const ChatMessage& msg) {
            messages.push_back(msg);
            if (messages.size() > maxMessages) {
                messages.erase(messages.begin());
            }
        }
        
        void addSystemMessage(const std::string& msg) {
            ChatMessage sysMsg;
            sysMsg.message = msg;
            sysMsg.isSystem = true;
            addMessage(sysMsg);
        }
        
        void clear() {
            messages.clear();
        }
    };

} // namespace Network

#endif // RTYPE_GAME_NETWORK_LOBBYDATA_HPP
