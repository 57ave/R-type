-- assets/scripts/config/network_config.lua
-- Network configuration for R-Type

NetworkConfig = {
    -- Mode de démarrage: "local" ou "network"
    startMode = "local",
    
    -- Configuration serveur par défaut
    server = {
        defaultAddress = "127.0.0.1",
        defaultPort = 12345
    },
    
    -- Connexion automatique au démarrage (seulement si startMode = "network")
    autoConnect = false,
    
    -- Serveurs sauvegardés pour connexion rapide
    savedServers = {
        { name = "Local", address = "127.0.0.1", port = 12345 },
        { name = "Server 1", address = "192.168.1.100", port = 12345 },
        { name = "Test Server", address = "localhost", port = 54321 }
    },
    
    -- Délais et timeouts
    connection = {
        timeoutMs = 5000,    -- timeout de connexion en millisecondes
        retryAttempts = 3,   -- nombre de tentatives de reconnexion
        retryDelayMs = 1000  -- délai entre les tentatives
    }
}

-- Fonction utilitaire pour obtenir l'adresse du serveur par défaut
function GetDefaultServerAddress()
    return NetworkConfig.server.defaultAddress
end

-- Fonction utilitaire pour obtenir le port du serveur par défaut
function GetDefaultServerPort()
    return NetworkConfig.server.defaultPort
end

-- Fonction pour se connecter à un serveur (appelée depuis le code C++)
function ConnectToServer(address, port)
    address = address or NetworkConfig.server.defaultAddress
    port = port or NetworkConfig.server.defaultPort
    
    print("[NetworkConfig] Attempting to connect to " .. address .. ":" .. tostring(port))
    
    -- Cette fonction sera appelée par le binding C++ Network.Connect()
    if Network and Network.Connect then
        return Network.Connect(address, port)
    else
        print("[NetworkConfig] Error: Network binding not available!")
        return false
    end
end

-- Fonction pour se déconnecter du serveur
function DisconnectFromServer()
    print("[NetworkConfig] Disconnecting from server")
    
    if Network and Network.Disconnect then
        return Network.Disconnect()
    else
        print("[NetworkConfig] Error: Network binding not available!")
        return false
    end
end

-- Fonction pour vérifier si on doit se connecter automatiquement
function ShouldAutoConnect()
    return NetworkConfig.startMode == "network" and NetworkConfig.autoConnect
end

-- Fonction pour obtenir la configuration de connexion par défaut
function GetDefaultConnection()
    return {
        address = NetworkConfig.server.defaultAddress,
        port = NetworkConfig.server.defaultPort,
        timeout = NetworkConfig.connection.timeoutMs
    }
end

print("[NetworkConfig] Configuration loaded - Start mode: " .. NetworkConfig.startMode)
