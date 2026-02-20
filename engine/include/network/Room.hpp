#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>

enum class RoomState {
    WAITING,
    PLAYING,
    PAUSED
};

class Room {
public:
    uint32_t id;
    std::string name;
    std::vector<uint32_t> playerIds;
    std::map<uint32_t, bool> playerReadyStates; // playerId -> ready state
    RoomState state;
    uint8_t maxPlayers;
    uint32_t hostPlayerId;

    Room(uint32_t id, const std::string& name, uint8_t maxPlayers = 4, uint32_t hostId = 0)
        : id(id), name(name), state(RoomState::WAITING), maxPlayers(maxPlayers), hostPlayerId(hostId) {}
    
    void setName(const std::string& newName) {
        name = newName;
    }

    bool addPlayer(uint32_t playerId) {
        if (playerIds.size() >= maxPlayers || state != RoomState::WAITING) {
            return false;
        }
        playerIds.push_back(playerId);
        playerReadyStates[playerId] = false; // New players start as not ready
        return true;
    }

    bool removePlayer(uint32_t playerId) {
        auto it = std::find(playerIds.begin(), playerIds.end(), playerId);
        if (it != playerIds.end()) {
            playerIds.erase(it);
            playerReadyStates.erase(playerId);
            return true;
        }
        return false;
    }
    
    bool setPlayerReady(uint32_t playerId, bool ready) {
        if (!hasPlayer(playerId)) {
            return false;
        }
        playerReadyStates[playerId] = ready;
        return true;
    }
    
    bool isPlayerReady(uint32_t playerId) const {
        auto it = playerReadyStates.find(playerId);
        return (it != playerReadyStates.end()) ? it->second : false;
    }
    
    bool allPlayersReady() const {
        if (playerIds.empty()) return false;
        for (uint32_t pid : playerIds) {
            if (!isPlayerReady(pid)) return false;
        }
        return true;
    }

    bool hasPlayer(uint32_t playerId) const {
        return std::find(playerIds.begin(), playerIds.end(), playerId) != playerIds.end();
    }
    
    bool isEmpty() const {
        return playerIds.empty();
    }
};
