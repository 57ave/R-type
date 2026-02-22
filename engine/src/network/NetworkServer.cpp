#include "network/NetworkServer.hpp"
#include "core/Logger.hpp"

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
            LOG_WARNING("NETWORKSERVER", "WARNING: No session found for " + (sender.address().to_string() + ":" + std::to_string(sender.port())) + " - skipping packet");
            continue;
        }

        uint16_t type = packet.header.type;
        
        if (type == static_cast<uint16_t>(GamePacketType::CLIENT_HELLO)) {
            LOG_INFO("NETWORKSERVER", "Received CLIENT_HELLO from " + (sender.address().to_string() + ":" + std::to_string(sender.port())));
            NetworkPacket welcome(static_cast<uint16_t>(GamePacketType::SERVER_WELCOME));
            welcome.header.seq = packet.header.seq;
            welcome.setPayload({(char)session->playerId});
            server_.sendTo(welcome, sender);
            LOG_INFO("NETWORK", "Welcome sent to " + (sender.address().to_string() + ":" + std::to_string(sender.port())));
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
                LOG_INFO("ROOM", "Created room " + payload.name + " (ID: " + std::to_string(roomId) + ") by player " + std::to_string((int)session->playerId));
            } catch (const std::exception& e) {
                LOG_ERROR("ROOM", std::string("Error creating room: ") + e.what());
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::RENAME_ROOM)) {
             try {
                auto payload = RenameRoomPayload::deserialize(packet.payload);
                if (roomManager_.renameRoom(payload.roomId, session->playerId, payload.newName)) {
                    LOG_INFO("ROOM", "Room " + std::to_string(payload.roomId) + " renamed to " + payload.newName);
                } else {
                    LOG_WARNING("ROOM", "Failed to rename room " + std::to_string(payload.roomId) + " (Permission denied or not found)");
                }
            } catch (const std::exception& e) {
                LOG_ERROR("ROOM", std::string("Error renaming room: ") + e.what());
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::JOIN_ROOM)) {
            try {
                auto payload = JoinRoomPayload::deserialize(packet.payload);
                LOG_INFO("NETWORKSERVER", "Received JOIN_ROOM request from " + (sender.address().to_string() + ":" + std::to_string(sender.port())) + " for room " + std::to_string(payload.roomId));
                
                if (roomManager_.joinRoom(payload.roomId, session->playerId)) {
                    session->roomId = payload.roomId;
                    
                    // Get room details to send complete info to client
                    auto room = roomManager_.getRoom(payload.roomId);
                    if (room) {
                        RoomJoinedPayload replyPayload;
                        replyPayload.roomId = room->id;
                        replyPayload.roomName = room->name;
                        replyPayload.maxPlayers = room->maxPlayers;
                        replyPayload.hostPlayerId = room->hostPlayerId;
                        
                        NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_JOINED));
                        reply.setPayload(replyPayload.serialize());
                        server_.sendTo(reply, sender);
                        LOG_INFO("ROOM", "Player " + std::to_string((int)session->playerId) + " joined room " + std::to_string(payload.roomId)
                                  + " (" + std::to_string(room->playerIds.size()) + "/" + std::to_string((int)room->maxPlayers) + " players)");
                        
                        // Broadcast player list update to all players in the room
                        RoomPlayersPayload playersUpdate;
                        playersUpdate.roomId = room->id;
                        for (uint32_t pid : room->playerIds) {
                            PlayerInRoomInfo playerInfo;
                            playerInfo.playerId = pid;
                            playerInfo.playerName = "Player " + std::to_string(pid);
                            playerInfo.isHost = (pid == room->hostPlayerId);
                            playerInfo.isReady = false;
                            playersUpdate.players.push_back(playerInfo);
                        }
                        
                        NetworkPacket updatePacket(static_cast<uint16_t>(GamePacketType::ROOM_PLAYERS_UPDATE));
                        updatePacket.setPayload(playersUpdate.serialize());
                        
                        // Send to all players in the room
                        auto sessions = server_.getActiveSessions();
                        for (const auto& s : sessions) {
                            if (s.roomId == room->id) {
                                server_.sendTo(updatePacket, s.endpoint);
                            }
                        }
                        LOG_INFO("ROOM", "Sent player list update to all players in room " + std::to_string(room->id));
                    } else {
                        LOG_WARNING("ROOM", "Room " + std::to_string(payload.roomId) + " not found after join");
                    }
                } else {
                    LOG_ERROR("ROOM", "Failed to join room " + std::to_string(payload.roomId));
                }
            } catch (const std::exception& e) {
                LOG_ERROR("ROOM", std::string("Error joining room: ") + e.what());
            }
        }
        else if (type == static_cast<uint16_t>(GamePacketType::ROOM_LIST)) {
            LOG_INFO("NETWORKSERVER", "Received ROOM_LIST request from " + (sender.address().to_string() + ":" + std::to_string(sender.port())));
            auto rooms = roomManager_.getRooms();
            RoomListPayload listPayload;
            for (const auto& room : rooms) {
                RoomInfo info;
                info.id = room.id;
                info.name = room.name;
                info.currentPlayers = (uint8_t)room.playerIds.size();
                info.maxPlayers = room.maxPlayers;
                info.inGame = (room.state == RoomState::PLAYING);
                listPayload.rooms.push_back(info);
                LOG_INFO("NETWORKSERVER", "  Room '" + room.name + "' state=" + std::to_string((int)room.state) + " inGame=" + (info.inGame ? "true" : "false"));
            }
            LOG_INFO("NETWORKSERVER", "Sending ROOM_LIST_REPLY with " + std::to_string(listPayload.rooms.size()) + " rooms to " + (sender.address().to_string() + ":" + std::to_string(sender.port())));
            NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_LIST_REPLY));
            reply.setPayload(listPayload.serialize());
            server_.sendTo(reply, sender);
            LOG_INFO("NETWORKSERVER", "ROOM_LIST_REPLY sent");
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

std::vector<ClientSession> NetworkServer::getActiveSessions() const {
    return server_.getActiveSessions();
}