#pragma once

#include <cstdint>

namespace FlappyBird {

/**
 * Packet types for Flappy Bird Battle Royale
 * Must match client's NetworkBindings.cpp
 */
enum class PacketType : uint16_t {
    // Connection (used by NetworkClient auto-send)
    CLIENT_HELLO    = 0x01,   // Client → Server: Initial connection
    SERVER_WELCOME  = 0x02,   // Server → Client: Welcome + assign player ID
    CLIENT_PING     = 0x03,   // Client → Server: Keep-alive (auto-sent every 1s)
    
    // Game Control
    GAME_START      = 0x10,   // Server → Clients: Game starts now!
    START_COUNTDOWN = 0x11,   // Server → Clients: Countdown (3, 2, 1)
    GAME_OVER       = 0x12,   // Server → Clients: Game ended (winnerId)
    
    // Gameplay
    PLAYER_INPUT    = 0x20,   // Client → Server: Flap input
    GAME_STATE      = 0x21,   // Server → Clients: Synchronized game state (30 Hz)
    SPAWN_PIPE      = 0x22,   // Server → Clients: New pipe spawned
    PLAYER_DIED     = 0x23,   // Server → Clients: Player died
};

} // namespace FlappyBird
