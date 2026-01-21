#pragma once

#include <memory>
#include <network/NetworkClient.hpp>
#include <network/RTypeProtocol.hpp>
#include <sol/sol.hpp>
#include <vector>

namespace RType {
namespace Network {

/**
 * @brief Bindings between Lua UI and C++ network client
 *
 * This class provides the bridge between Lua scripts (UI) and the C++ NetworkClient,
 * allowing the UI to send network requests and receive callbacks when packets arrive.
 */
class NetworkBindings {
public:
    /**
     * @brief Register all network functions to Lua
     * @param lua The Lua state to register functions to
     */
    static void RegisterAll(sol::state& lua);

    /**
     * @brief Set the network client instance to use
     * @param client Shared pointer to the NetworkClient
     */
    static void SetNetworkClient(std::shared_ptr<NetworkClient> client);

    /**
     * @brief Get the current network client instance (may be null)
     * @return Shared pointer to the NetworkClient, or nullptr if not set
     */
    static std::shared_ptr<NetworkClient> GetNetworkClient();

    // ========== Functions called FROM Lua ==========

    /**
     * @brief Request the list of available rooms from the server
     */
    static void RequestRoomList();

    /**
     * @brief Create a new room on the server
     * @param name Room name
     * @param maxPlayers Maximum number of players (2-8)
     * @param password Password for the room (optional, not implemented yet)
     * @param difficulty Difficulty level (1-5, not implemented yet)
     */
    static void CreateRoom(const std::string& name, int maxPlayers, const std::string& password,
                           int difficulty);

    /**
     * @brief Join an existing room
     * @param roomId The ID of the room to join
     */
    static void JoinRoom(uint32_t roomId);

    /**
     * @brief Connect to a server by host:port and set the NetworkClient used by bindings.
     * Can be called at runtime even if the game wasn't started with --network.
     */
    static void Connect(const std::string& host, int port);

    /**
     * @brief Leave the current room
     */
    static void LeaveRoom();

    /**
     * @brief Set player ready status in the lobby
     * @param ready True if ready, false otherwise
     */
    static void SetPlayerReady(bool ready);

    /**
     * @brief Start the game (host only)
     */
    static void StartGame();

    // ========== Callbacks called BY C++ when packets arrive ==========

    /**
     * @brief Called when room list is received from server
     * @param rooms Vector of room information
     */
    static void OnRoomListReceived(const std::vector<RoomInfo>& rooms);

    /**
     * @brief Called when a room is successfully created
     * @param roomId The ID of the newly created room
     */
    static void OnRoomCreated(uint32_t roomId);

    /**
     * @brief Called when successfully joined a room
     * @param roomId The room ID
     * @param roomName The room name
     * @param maxPlayers Maximum players allowed
     * @param isHost Whether this player is the host
     */
    static void OnRoomJoined(uint32_t roomId, const std::string& roomName, uint8_t maxPlayers,
                             bool isHost);

    /**
     * @brief Called when another player joins the room
     * @param playerId The player ID
     * @param playerName The player name
     */
    static void OnPlayerJoinedRoom(uint32_t playerId, const std::string& playerName);

    /**
     * @brief Called when a player leaves the room
     * @param playerId The player ID
     */
    static void OnPlayerLeftRoom(uint32_t playerId);

    /**
     * @brief Called when a player's ready state changes
     * @param playerId The player ID
     * @param ready True if ready, false otherwise
     */
    static void OnPlayerReadyChanged(uint32_t playerId, bool ready);

    /**
     * @brief Called when the game is starting
     * @param countdown Countdown in seconds
     */
    static void OnGameStarting(int countdown);

    /**
     * @brief Called when the room player list is updated
     * @param players Vector of player information
     */
    static void OnRoomPlayersUpdated(const std::vector<PlayerInRoomInfo>& players);

    /**
     * @brief Called when a chat message is received
     * @param senderName Name of the sender
     * @param message The message content
     */
    static void OnChatMessage(const std::string& senderName, const std::string& message);

    /**
     * @brief Send a chat message to the room
     * @param message The message to send
     */
    static void SendChatMessage(const std::string& message);

private:
    static std::shared_ptr<NetworkClient> s_networkClient;
    static sol::state* s_lua;
};

}  // namespace Network
}  // namespace RType
