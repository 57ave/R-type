#include "network/NetworkServer.hpp"
#include <iostream>

// R-Type specific protocol - TODO: This should be refactored to make engine protocol-agnostic
// For now, include the game-specific protocol from the server
#include "../../../server/include/network/RTypeProtocol.hpp"

NetworkServer::NetworkServer(short port)
    : server_(io_context_, port) {
}

NetworkServer::~NetworkServer() {
    io_context_.stop();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

void NetworkServer::start() {
    server_.start();
    io_thread_ = std::thread([this]() {
        io_context_.run();
    });
}

void NetworkServer::process() {
    NetworkPacket packet;
    udp::endpoint sender;

    while (server_.popPacket(packet, sender)) {
        auto session = server_.getSession(sender);
        if (!session) {

            continue;
        }

        uint16_t type = packet.header.type;
        
        if (type == static_cast<uint16_t>(GamePacketType::CLIENT_HELLO)) {
            NetworkPacket welcome(static_cast<uint16_t>(GamePacketType::SERVER_WELCOME));
            welcome.header.seq = packet.header.seq;
            welcome.setPayload({(char)session->playerId});
            server_.sendTo(welcome, sender);
            std::cout << "[Network] Welcome sent to " << sender << std::endl;
        } 
        else if (type == static_cast<uint16_t>(GamePacketType::CREATE_ROOM)) {
            try {
                auto payload = CreateRoomPayload::deserialize(packet.payload);
                uint32_t roomId = roomManager_.createRoom(payload.name, payload.maxPlayers, session->playerId);
                roomManager_.joinRoom(roomId, session->playerId);
                session->roomId = roomId;

                NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_CREATED));
                JoinRoomPayload replyPayload;
                replyPayload.roomId = roomId;
                reply.setPayload(replyPayload.serialize());
                server_.sendTo(reply, sender);
                std::cout << "[Room] Created room " << payload.name << " (ID: " << roomId << ") by player " << (int)session->playerId << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Room] Error creating room: " << e.what() << std::endl;
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::RENAME_ROOM)) {
             try {
                auto payload = RenameRoomPayload::deserialize(packet.payload);
                if (roomManager_.renameRoom(payload.roomId, session->playerId, payload.newName)) {
                    std::cout << "[Room] Room " << payload.roomId << " renamed to " << payload.newName << std::endl;
                } else {
                    std::cerr << "[Room] Failed to rename room " << payload.roomId << " (Permission denied or not found)" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "[Room] Error renaming room: " << e.what() << std::endl;
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::JOIN_ROOM)) {
            try {
                auto payload = JoinRoomPayload::deserialize(packet.payload);
                if (roomManager_.joinRoom(payload.roomId, session->playerId)) {
                    session->roomId = payload.roomId;
                    
                    NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_JOINED));
                    reply.setPayload(payload.serialize());
                    server_.sendTo(reply, sender);
                    std::cout << "[Room] Player " << (int)session->playerId << " joined room " << payload.roomId << std::endl;
                } else {

                    std::cerr << "[Room] Failed to join room " << payload.roomId << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "[Room] Error joining room: " << e.what() << std::endl;
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::ROOM_LIST)) {
            auto rooms = roomManager_.getRooms();
            RoomListPayload listPayload;
            for (const auto& room : rooms) {
                RoomInfo info;
                info.id = room.id;
                info.name = room.name;
                info.currentPlayers = (uint8_t)room.playerIds.size();
                info.maxPlayers = room.maxPlayers;
                listPayload.rooms.push_back(info);
            }
            NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_LIST_REPLY));
            reply.setPayload(listPayload.serialize());
            server_.sendTo(reply, sender);
        }
        else {
            std::lock_guard<std::mutex> lock(packetsMutex_);
            receivedPackets_.push({packet, sender});
        }
    }

    server_.checkTimeouts();
}

bool NetworkServer::hasReceivedPackets() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    return !receivedPackets_.empty();
}

std::pair<NetworkPacket, asio::ip::udp::endpoint> NetworkServer::getNextReceivedPacket() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    if (receivedPackets_.empty()) {
        throw std::runtime_error("No packets available");
    }
    auto packet = receivedPackets_.front();
    receivedPackets_.pop();
    return packet;
}

void NetworkServer::broadcast(const NetworkPacket& packet) {
    server_.broadcast(packet);
}

void NetworkServer::sendTo(const NetworkPacket& packet, const asio::ip::udp::endpoint& endpoint) {
    server_.sendTo(packet, endpoint);
}

void NetworkServer::checkTimeouts() {
    server_.checkTimeouts();
}

void NetworkServer::removeClient(const asio::ip::udp::endpoint& endpoint) {
    server_.removeSession(endpoint);
}

std::shared_ptr<ClientSession> NetworkServer::getSession(const asio::ip::udp::endpoint& endpoint) {
    return server_.getSession(endpoint);
}