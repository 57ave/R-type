-- Test configuration for network mode
NetworkConfig = {
    startMode = "network",
    
    server = {
        defaultAddress = "127.0.0.1",
        defaultPort = 12345
    },
    
    autoConnect = false,  -- Manual connection for testing
    
    savedServers = {
        { name = "Test Local", address = "127.0.0.1", port = 12345 }
    },
    
    connection = {
        timeoutMs = 5000,
        retryAttempts = 3,
        retryDelayMs = 1000
    }
}

-- Test functions
function ConnectToServer(address, port)
    print("[TEST] ConnectToServer called: " .. (address or "nil") .. ":" .. (port or "nil"))
    return true
end

function DisconnectFromServer()
    print("[TEST] DisconnectFromServer called")
    return true
end

function ShouldAutoConnect()
    return NetworkConfig.startMode == "network" and NetworkConfig.autoConnect
end

function GetDefaultConnection()
    return {
        address = NetworkConfig.server.defaultAddress,
        port = NetworkConfig.server.defaultPort,
        timeout = NetworkConfig.connection.timeoutMs
    }
end

print("[TEST NetworkConfig] Configuration loaded - Start mode: " .. NetworkConfig.startMode)
