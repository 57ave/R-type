#pragma once

#include <sol/sol.hpp>
#include <network/NetworkClient.hpp>
#include <network/Packet.hpp>

namespace FlappyBird {

/**
 * @brief Bindings pour exposer NetworkClient à Lua
 */
class NetworkBindings {
public:
    static void RegisterNetworkClient(sol::state& lua, NetworkClient* client);
    static void RegisterPacketTypes(sol::state& lua);
};

} // namespace FlappyBird
