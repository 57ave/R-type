/**
 * NetworkManager.hpp - Network Client Manager
 * 
 * Manages connection to server and network communication.
 * Will be implemented in Phase 5.
 */

#pragma once

#include <string>

class NetworkManager
{
public:
    NetworkManager() = default;
    ~NetworkManager() = default;

    /**
     * Initialize network subsystem
     */
    bool initialize();

    /**
     * Connect to server
     */
    bool connect(const std::string& address, unsigned short port);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool isConnected() const { return connected_; }

    /**
     * Update network (process packets)
     */
    void update();

private:
    bool connected_ = false;
    // NetworkClient will be added in Phase 5
};
