/**
 * RTypeProtocol.hpp - R-Type Network Protocol Structures (Client-side)
 * 
 * Ces structures doivent être identiques à celles du serveur pour la sérialisation.
 * Copié de server/include/network/RTypeProtocol.hpp
 */

#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <network/Serializer.hpp>
#include <network/Packet.hpp>

namespace RType {

// Types de paquets - identiques au serveur
enum class GamePacketType : uint16_t {
    CLIENT_HELLO = 0x01,
    CLIENT_INPUT = 0x02,
    CLIENT_PING = 0x03,
    CLIENT_DISCONNECT = 0x04,
    CREATE_ROOM = 0x20,
    JOIN_ROOM = 0x21,
    ROOM_LIST = 0x22,
    GAME_START = 0x23,
    RENAME_ROOM = 0x24,
    ROOM_LEAVE = 0x25,
    SERVER_WELCOME = 0x10,
    WORLD_SNAPSHOT = 0x11,
    ENTITY_SPAWN = 0x12,
    ENTITY_DESTROY = 0x13,
    PLAYER_DIED = 0x14,
    SERVER_PING_REPLY = 0x15,
    CLIENT_LEFT = 0x16,
    ROOM_JOINED = 0x30,
    ROOM_LIST_REPLY = 0x31,
    ROOM_CREATED = 0x32,
    ROOM_PLAYERS_UPDATE = 0x33,
    CLIENT_TOGGLE_PAUSE = 0x34,
    SERVER_SET_PAUSE = 0x35,
    CHAT_MESSAGE = 0x40,
    PLAYER_READY = 0x50,
    LEVEL_CHANGE = 0x60,         // Server informs clients of level change (payload: uint8_t levelId)
    GAME_OVER = 0x70,            // Server informs clients all players are dead (payload: uint32_t totalScore)
    GAME_VICTORY = 0x71          // Server informs clients boss L3 killed (payload: uint32_t totalScore)
};

// Type d'entité - identique au serveur
enum class EntityType : uint8_t {
    ENTITY_PLAYER = 0,
    ENTITY_MONSTER = 1,
    ENTITY_PLAYER_MISSILE = 2,
    ENTITY_MONSTER_MISSILE = 3,
    ENTITY_OBSTACLE = 4,
    ENTITY_EXPLOSION = 5,
    ENTITY_POWERUP = 6,
    ENTITY_MODULE = 7
};

#pragma pack(push, 1)

// Input du client - identique au serveur
// Bits de inputMask: 0=up, 1=down, 2=left, 3=right, 4=fire
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;
    uint8_t chargeLevel;  // 0 = normal, 1-5 = charge levels
    uint32_t inputSeq;    // Monotonic input sequence number for prediction/reconciliation

    ClientInput() : playerId(0), inputMask(0), chargeLevel(0), inputSeq(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static ClientInput deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(ClientInput));
        return deserializer.read<ClientInput>();
    }

    // Helper pour construire inputMask
    static uint8_t buildInputMask(bool up, bool down, bool left, bool right, bool fire) {
        uint8_t mask = 0;
        if (up)    mask |= 0x01;
        if (down)  mask |= 0x02;
        if (left)  mask |= 0x04;
        if (right) mask |= 0x08;
        if (fire)  mask |= 0x10;
        return mask;
    }
};

// En-tête du snapshot
struct SnapshotHeader {
    uint32_t entityCount;
    uint32_t snapshotSeq;    // Monotonic snapshot counter for ordering
    uint8_t  playerAckCount; // Number of PlayerInputAck entries following this header

    SnapshotHeader() : entityCount(0), snapshotSeq(0), playerAckCount(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static SnapshotHeader deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(SnapshotHeader));
        return deserializer.read<SnapshotHeader>();
    }
};

// Per-player input acknowledgement included in snapshots
struct PlayerInputAck {
    uint8_t  playerId;
    uint32_t lastProcessedInputSeq;

    PlayerInputAck() : playerId(0), lastProcessedInputSeq(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static PlayerInputAck deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(PlayerInputAck));
        return deserializer.read<PlayerInputAck>();
    }
};

// État d'une entité dans le snapshot
struct EntityState {
    uint32_t id;
    EntityType type;
    int16_t x;
    int16_t y;
    int16_t vx;
    int16_t vy;
    uint16_t hp;
    uint8_t playerLine;     // Ligne de la spritesheet pour la couleur du vaisseau
    uint8_t playerId;       // ID du joueur propriétaire (0 = aucun)
    uint8_t chargeLevel;    // Pour missiles chargés
    uint8_t enemyType;      // Type d'ennemi
    uint8_t projectileType; // Type de projectile
    uint32_t score;         // Score du joueur (0 pour non-joueurs)

    EntityState() 
        : id(0), type(EntityType::ENTITY_PLAYER), x(0), y(0), vx(0), vy(0), hp(0),
          playerLine(0), playerId(0), chargeLevel(0), enemyType(0), projectileType(0), score(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static EntityState deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(EntityState));
        return deserializer.read<EntityState>();
    }
};

#pragma pack(pop)

// Parsed world snapshot data
struct WorldSnapshotData {
    SnapshotHeader header;
    std::vector<PlayerInputAck> acks;
    std::vector<EntityState> entities;
};

// Fonctions utilitaires de protocole
class Protocol {
public:
    // Créer un paquet CLIENT_INPUT
    static NetworkPacket createInputPacket(const ClientInput& input) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_INPUT));
        packet.setPayload(input.serialize());
        return packet;
    }

    // Parser un WORLD_SNAPSHOT (new format: header + acks + entities)
    static WorldSnapshotData parseWorldSnapshot(const NetworkPacket& packet) {
        if (packet.header.type != static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT)) {
            throw std::runtime_error("Invalid packet type for WORLD_SNAPSHOT");
        }

        const char* data = packet.payload.data();
        size_t offset = 0;
        WorldSnapshotData result;

        // Read header
        result.header = SnapshotHeader::deserialize(data);
        offset += sizeof(SnapshotHeader);

        // Read player input acks
        result.acks.reserve(result.header.playerAckCount);
        for (uint8_t i = 0; i < result.header.playerAckCount; ++i) {
            result.acks.push_back(PlayerInputAck::deserialize(data + offset));
            offset += sizeof(PlayerInputAck);
        }

        // Read entities
        result.entities.reserve(result.header.entityCount);
        for (uint32_t i = 0; i < result.header.entityCount; ++i) {
            result.entities.push_back(EntityState::deserialize(data + offset));
            offset += sizeof(EntityState);
        }

        return result;
    }
};

} // namespace RType
