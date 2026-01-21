#include "core/GameConfig.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace RType::Core {

// Static member definitions
GameConfiguration GameConfig::s_config;
std::string GameConfig::s_basePath = "";

bool GameConfig::LoadConfiguration(Scripting::LuaState& luaState) {
    std::cout << "📁 [GameConfig] Loading configuration files..." << std::endl;
    
    try {
        // Resolve base path first
        ResolveBasePath();
        
        // Set base path in Lua
        luaState.GetState()["ASSET_BASE_PATH"] = s_basePath;
        
        // Load main initialization script (which loads all configs)
        if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/init.lua"))) {
            std::cerr << "[GameConfig] Warning: Could not load init.lua, trying fallback..." << std::endl;
            // Fallback to loading configs individually
            if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/config/game_config.lua"))) {
                std::cerr << "[GameConfig] Warning: Could not load game_config.lua either" << std::endl;
            }
            if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/config/network_config.lua"))) {
                std::cerr << "[GameConfig] Warning: Could not load network_config.lua" << std::endl;
            }
        } else {
            std::cout << "[GameConfig] ✓ init.lua loaded - All configurations initialized" << std::endl;
        }
        
        // Load configurations from Lua state
        auto& lua = luaState.GetState();
        
        if (!LoadNetworkConfig(lua)) {
            std::cerr << "[GameConfig] Warning: Failed to load network configuration" << std::endl;
        }
        
        if (!LoadGameConfig(lua)) {
            std::cerr << "[GameConfig] Warning: Failed to load game configuration" << std::endl;
        }
        
        std::cout << "[GameConfig] ✓ Configuration loaded successfully" << std::endl;
        std::cout << "[GameConfig]   Window: " << s_config.window.width << "x" << s_config.window.height << std::endl;
        std::cout << "[GameConfig]   Network: " << s_config.network.startMode << " mode" << std::endl;
        std::cout << "[GameConfig]   Server: " << s_config.network.server.defaultAddress << ":" << s_config.network.server.defaultPort << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[GameConfig] ERROR loading configuration: " << e.what() << std::endl;
        return false;
    }
}

const GameConfiguration& GameConfig::GetConfiguration() {
    return s_config;
}

void GameConfig::ApplyWindowConfiguration(const GameConfiguration::WindowConfig& config) {
    std::cout << "[GameConfig] Applying window configuration: " 
              << config.width << "x" << config.height 
              << " (" << config.title << ")" << std::endl;
    // Window configuration will be applied by the caller
}

std::string GameConfig::ResolveAssetPath(const std::string& relativePath) {
    if (!s_basePath.empty()) {
        return s_basePath + relativePath;
    }
    
    // If base path not resolved yet, try to resolve it
    ResolveBasePath();
    return s_basePath + relativePath;
}

bool GameConfig::LoadNetworkConfig(sol::state& lua) {
    try {
        sol::table networkConfig = lua["NetworkConfig"];
        if (!networkConfig.valid()) {
            std::cout << "[GameConfig] NetworkConfig table not found in Lua, using defaults" << std::endl;
            return false;
        }
        
        s_config.network.startMode = networkConfig["startMode"].get<sol::optional<std::string>>().value_or("local");
        s_config.network.autoConnect = networkConfig["autoConnect"].get<sol::optional<bool>>().value_or(false);
        
        sol::table serverConfig = networkConfig["server"];
        if (serverConfig.valid()) {
            s_config.network.server.defaultAddress = serverConfig["defaultAddress"].get<sol::optional<std::string>>().value_or("127.0.0.1");
            s_config.network.server.defaultPort = serverConfig["defaultPort"].get<sol::optional<int>>().value_or(12345);
        }
        
        sol::table connectionConfig = networkConfig["connection"];
        if (connectionConfig.valid()) {
            s_config.network.connection.timeoutMs = connectionConfig["timeoutMs"].get<sol::optional<int>>().value_or(5000);
            s_config.network.connection.retryAttempts = connectionConfig["retryAttempts"].get<sol::optional<int>>().value_or(3);
            s_config.network.connection.retryDelayMs = connectionConfig["retryDelayMs"].get<sol::optional<int>>().value_or(1000);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[GameConfig] Error loading network config: " << e.what() << std::endl;
        return false;
    }
}

bool GameConfig::LoadGameConfig(sol::state& lua) {
    try {
        sol::table gameConfig = lua["GameConfig"];
        if (!gameConfig.valid()) {
            std::cout << "[GameConfig] GameConfig table not found in Lua, using defaults" << std::endl;
            return false;
        }
        
        sol::table windowConfig = gameConfig["window"];
        if (windowConfig.valid()) {
            s_config.window.width = windowConfig["width"].get<sol::optional<int>>().value_or(1920);
            s_config.window.height = windowConfig["height"].get<sol::optional<int>>().value_or(1080);
            s_config.window.title = windowConfig["title"].get<sol::optional<std::string>>().value_or("R-Type - ECS Version");
            s_config.window.fullscreen = windowConfig["fullscreen"].get<sol::optional<bool>>().value_or(false);
            s_config.window.vsync = windowConfig["vsync"].get<sol::optional<bool>>().value_or(true);
        }
        
        sol::table playerConfig = gameConfig["player"];
        if (playerConfig.valid()) {
            s_config.player.startX = playerConfig["startX"].get<sol::optional<float>>().value_or(100.0f);
            s_config.player.startY = playerConfig["startY"].get<sol::optional<float>>().value_or(400.0f);
            s_config.player.health = playerConfig["health"].get<sol::optional<int>>().value_or(1);
            s_config.player.speed = playerConfig["speed"].get<sol::optional<float>>().value_or(500.0f);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[GameConfig] Error loading game config: " << e.what() << std::endl;
        return false;
    }
}

void GameConfig::ResolveBasePath() {
    if (!s_basePath.empty()) {
        return; // Already resolved
    }
    
    // List of possible base paths to check
    std::vector<std::string> basePaths = {
        "",                    // Current directory (running from project root)
        "../../",              // Running from build/game/
        "../../../",           // Running from deeper build directories
    };
    
    // Test file to check if we're in the right directory
    std::string testFile = "game/assets/fonts/Roboto-Regular.ttf";
    
    for (const auto& base : basePaths) {
        std::string fullPath = base + testFile;
        std::ifstream file(fullPath);
        if (file.good()) {
            s_basePath = base;
            std::cout << "[GameConfig] Base path resolved to: " << (base.empty() ? "(current dir)" : base) << std::endl;
            return;
        }
    }
    
    // Fallback: use empty base path
    std::cerr << "[GameConfig] Warning: Could not resolve base path, using current directory" << std::endl;
    s_basePath = "";
}

} // namespace RType::Core
