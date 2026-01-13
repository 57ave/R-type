#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

enum class RoomState {
    WAITING,
    PLAYING
};

class Room {
public:
    uint32_t id;
    std::string name;
    std::vector<uint32_t> playerIds;
    RoomState state;
    uint8_t maxPlayers;

    Room(uint32_t id, const std::string& name, uint8_t maxPlayers = 4)
        : id(id), name(name), state(RoomState::WAITING), maxPlayers(maxPlayers) {}

    bool addPlayer(uint32_t playerId) {
        if (playerIds.size() >= maxPlayers || state != RoomState::WAITING) {
            return false;
        }
        playerIds.push_back(playerId);
        return true;
    }

    bool removePlayer(uint32_t playerId) {
        auto it = std::find(playerIds.begin(), playerIds.end(), playerId);
        if (it != playerIds.end()) {
            playerIds.erase(it);
            return true;
        }
        return false;
    }

    bool hasPlayer(uint32_t playerId) const {
        return std::find(playerIds.begin(), playerIds.end(), playerId) != playerIds.end();
    }
    
    bool isEmpty() const {
        return playerIds.empty();
    }
};
