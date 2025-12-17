#pragma once

#include <cstdint>

// Component to link local entities with network entities
struct NetworkId {
    uint32_t networkId;  // ID from server
    bool isLocalPlayer;  // Is this the local player?
    uint8_t playerId;    // Player ID (for players)
    uint8_t playerLine;  // Player ship color (spritesheet line)

    NetworkId() : networkId(0), isLocalPlayer(false), playerId(0), playerLine(0) {}
    NetworkId(uint32_t id, bool isLocal = false, uint8_t pId = 0, uint8_t line = 0) 
        : networkId(id), isLocalPlayer(isLocal), playerId(pId), playerLine(line) {}
};
