#pragma once

#include <network/NetworkClient.hpp>
#include <network/Packet.hpp>
#include <sol/sol.hpp>

namespace FlappyBird {

/**
 * @brief Bindings pour exposer NetworkClient à Lua
 */
class NetworkBindings {
public:
    static void RegisterNetworkClient(sol::state& lua, NetworkClient* client);
    static void RegisterPacketTypes(sol::state& lua);
};

}  // namespace FlappyBird
