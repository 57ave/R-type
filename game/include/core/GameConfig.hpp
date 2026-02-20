/**
 * GameConfig.hpp - Game Configuration
 * 
 * Stores configuration loaded from Lua scripts.
 */

#pragma once

#include <string>

struct GameConfig
{
    // Window configuration
    struct WindowConfig
    {
        unsigned int width = 1920;
        unsigned int height = 1080;
        std::string title = "R-Type";
        bool fullscreen = false;
        unsigned int frameRateLimit = 60;
        bool vsync = false;
    } window;

    // Gameplay configuration (will be loaded from Lua)
    struct GameplayConfig
    {
        float scrollingSpeed = 200.0f;
        int playerMaxHP = 3;
        float playerSpeed = 500.0f;
        float enemySpawnRate = 2.0f;
    } gameplay;

    // Network configuration
    struct NetworkConfig
    {
        std::string serverAddress = "127.0.0.1";
        unsigned short serverPort = 12345;
        unsigned int timeoutMs = 5000;
    } network;

    // Paths configuration (will be loaded from Lua)
    struct PathsConfig
    {
        std::string assetsRoot = "assets/";
        std::string scriptsRoot = "assets/scripts/";
        std::string fontsRoot = "assets/fonts/";
        std::string soundsRoot = "assets/sounds/";
        std::string spritesRoot = "assets/";
    } paths;

    // Audio configuration
    struct AudioConfig
    {
        float musicVolume = 50.0f;
        float sfxVolume = 70.0f;
        bool enabled = true;
    } audio;
};
