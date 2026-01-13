#pragma once

#include "Room.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>

class RoomManager {
public:
    RoomManager() : nextRoomId_(1) {}


    uint32_t createRoom(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        uint32_t id = nextRoomId_++;
        auto room = std::make_shared<Room>(id, name);
        rooms_[id] = room;
        return id;
    }


    bool joinRoom(uint32_t roomId, uint32_t playerId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it != rooms_.end()) {
            // Check if player is already in another room? 
            // For simplicity, we assume caller handles that or we don't care.
            return it->second->addPlayer(playerId);
        }
        return false;
    }


    void leaveRoom(uint32_t roomId, uint32_t playerId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it != rooms_.end()) {
            it->second->removePlayer(playerId);
            if (it->second->isEmpty()) {
                rooms_.erase(it); // Auto-close empty rooms
            }
        }
    }


    std::shared_ptr<Room> getRoom(uint32_t roomId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it != rooms_.end()) {
            return it->second;
        }
        return nullptr;
    }
    

    std::vector<Room> getRooms() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Room> list;
        for (const auto& pair : rooms_) {
            list.push_back(*pair.second);
        }
        return list;
    }

private:
    std::map<uint32_t, std::shared_ptr<Room>> rooms_;
    uint32_t nextRoomId_;
    std::mutex mutex_;
};
