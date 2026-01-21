#include "GameRefactored.hpp"
#include "core/GameStateCallbacks.hpp"
#include "network/NetworkBindings.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

namespace RType {

GameRefactored::GameRefactored()
    : initialized(false)
    , windowWidth(1920)
    , windowHeight(1080)
    , windowTitle("R-Type - ECS Version")
    , networkMode(false)
    , isNetworkClient(false)
{
}

GameRefactored::~GameRefactored() {
    if (initialized) {
        Shutdown();
    }
}

int GameRefactored::Run(int argc, char* argv[]) {
    auto& logger = rtype::core::Logger::getInstance();
    logger.info("Game", "R-Type starting...");
    
    if (!Initialize(argc, argv)) {
        logger.error("Game", "Failed to initialize");
        return 1;
    }
    
    // Boucle de jeu principale
    while (gameLoop && gameLoop->Update()) {
        // La boucle de jeu gère tout
    }
    
    Shutdown();
    return 0;
}

bool GameRefactored::Initialize(int argc, char* argv[]) {
    auto& logger = rtype::core::Logger::getInstance();
    
    if (initialized) {
        logger.warning("Game", "Already initialized");
        return true;
    }

    // ========================================
    // ANALYSE DES ARGUMENTS
    // ========================================
    ParseCommandLineArguments(argc, argv);

    // ========================================
    // INITIALISATION ECS
    // ========================================
    coordinator = std::make_unique<ECS::Coordinator>();
    coordinator->Init();
    
    Core::GameInitializer::RegisterComponents(*coordinator);

    // ========================================
    // INITIALISATION LUA ET CONFIGURATION
    // ========================================
    luaState = &Scripting::LuaState::Instance();
    luaState->Init();
    luaState->EnableHotReload(true);

    if (!Core::GameConfig::LoadConfiguration(*luaState)) {
        logger.warning("Game", "Using default configuration");
    }

    // Appliquer la configuration chargée
    ApplyConfiguration();

    // ========================================
    // INITIALISATION DES GESTIONNAIRES
    // ========================================
    
    // Asset Loader
    assetLoader = std::make_unique<Core::AssetLoader>();
    std::string basePath = Core::GameConfig::ResolveAssetPath("");
    if (!assetLoader->Initialize(basePath)) {
        logger.error("Game", "Failed to initialize asset loader");
        return false;
    }
    
    // Précharger les ressources
    assetLoader->PreloadAllTextures();
    assetLoader->PreloadAllSounds();

    // Audio Manager
    audioManager = std::make_shared<Core::AudioManager>();
    if (!audioManager->Initialize(assetLoader->GetBasePath())) {
        logger.warning("Game", "Audio manager initialization failed, continuing without audio");
    }

    // Input Handler
    inputHandler = std::make_shared<Core::InputHandler>();
    SetupInputCallbacks();

    // Gameplay Manager
    gameplayManager = std::make_shared<Core::GameplayManager>(coordinator.get());
    gameplayManager->Initialize(assetLoader->GetTextureMap(), assetLoader->GetAllSprites());
    gameplayManager->SetWindowSize(static_cast<float>(windowWidth), static_cast<float>(windowHeight));

    // Network Manager
    networkManager = std::make_unique<Core::NetworkManager>();
    SetupNetworkCallbacks();

    // ========================================
    // INITIALISATION WINDOW ET RENDERER
    // ========================================
    window = std::make_unique<eng::engine::rendering::sfml::SFMLWindow>();
    window->create(windowWidth, windowHeight, windowTitle);

    renderer = std::make_unique<eng::engine::rendering::sfml::SFMLRenderer>(&window->getSFMLWindow());

    // ========================================
    // INITIALISATION DES SYSTÈMES
    // ========================================
    if (!InitializeSystems()) {
        logger.error("Game", "Failed to initialize systems");
        return false;
    }

    // ========================================
    // INITIALISATION GAME LOOP
    // ========================================
    gameLoop = std::make_unique<Core::GameLoop>(coordinator.get());
    gameLoop->SetWindow(window.get());
    gameLoop->SetUISystem(uiSystem);
    gameLoop->SetAudioManager(audioManager);
    gameLoop->SetInputHandler(inputHandler);
    gameLoop->SetGameplayManager(gameplayManager);
    gameLoop->SetLuaState(luaState);
    gameLoop->SetNetworkMode(networkMode);
    
    if (networkManager->IsConnected()) {
        gameLoop->SetNetworkSystem(networkManager->GetNetworkSystem());
    }

    // ========================================
    // SCRIPTS ET UI
    // ========================================
    LoadScripts();
    InitializeUI();

    // ========================================
    // FINALISATION
    // ========================================
    initialized = true;
    logger.info("Game", "Initialization complete");
    
    return true;
}

void GameRefactored::Shutdown() {
    if (!initialized) return;

    auto& logger = rtype::core::Logger::getInstance();
    logger.info("Game", "Shutting down...");

    // Sauvegarder les paramètres utilisateur
    if (audioManager) {
        audioManager->SaveUserSettings(assetLoader->ResolveAssetPath("user_settings.lua"));
    }

    // Arrêter le réseau
    if (networkManager && networkManager->IsConnected()) {
        networkManager->Disconnect();
    }

    // Nettoyer les ressources
    gameLoop.reset();
    uiSystem.reset();
    networkManager.reset();
    gameplayManager.reset();
    inputHandler.reset();
    audioManager.reset();
    
    if (assetLoader) {
        assetLoader->CleanupSprites();
        assetLoader->UnloadAll();
        assetLoader.reset();
    }

    // LuaState is a singleton, just null the pointer
    luaState = nullptr;
    
    renderer.reset();
    window.reset();
    coordinator.reset();

    initialized = false;
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

void GameRefactored::ParseCommandLineArguments(int argc, char* argv[]) {
    auto& logger = rtype::core::Logger::getInstance();
    
    // Support de l'ancien flag --network pour la rétrocompatibilité
    if (argc > 1 && std::string(argv[1]) == "--network") {
        networkMode = true;
        isNetworkClient = true;
        
        std::string serverAddress = "127.0.0.1";
        short serverPort = 12345;
        
        if (argc > 2) {
            serverAddress = argv[2];
        }
        if (argc > 3) {
            serverPort = static_cast<short>(std::stoi(argv[3]));
        }
        
        legacyServerAddress = serverAddress;
        legacyServerPort = serverPort;
        
        logger.info("Game", "Network mode enabled - Server: " + serverAddress + ":" + std::to_string(serverPort));
    }
}

void GameRefactored::ApplyConfiguration() {
    auto& logger = rtype::core::Logger::getInstance();
    const auto& config = Core::GameConfig::GetConfiguration();

    // Configuration de la fenêtre
    windowWidth = config.window.width;
    windowHeight = config.window.height;
    windowTitle = config.window.title;

    // Configuration réseau (sauf si legacy CLI args)
    if (!networkMode) {
        networkMode = (config.network.startMode == "network");
        if (networkMode && config.network.autoConnect) {
            isNetworkClient = true;
            legacyServerAddress = config.network.server.defaultAddress;
            legacyServerPort = config.network.server.defaultPort;
            logger.info("Game", "Auto-connect enabled: " + legacyServerAddress + ":" + std::to_string(legacyServerPort));
        }
    }
}

bool GameRefactored::InitializeSystems() {
    if (!Core::GameInitializer::RegisterSystems(*coordinator, *renderer, &uiSystem)) {
        return false;
    }

    // UISystem is now set by RegisterSystems
    uiSystem->SetWindow(window.get());
    
    // Charger la police par défaut pour l'UI
    std::string fontPath = Core::GameConfig::ResolveAssetPath("game/assets/fonts/Roboto-Regular.ttf");
    if (!uiSystem->LoadFont("default", fontPath)) {
        auto& logger = rtype::core::Logger::getInstance();
        logger.warning("Game", "Could not load default UI font");
    }

    return true;
}

void GameRefactored::SetupInputCallbacks() {
    if (!inputHandler) return;

    // Callback pour la pause
    inputHandler->SetActionCallback(Core::InputAction::Pause, []() {
        auto& gsm = GameStateManager::Instance();
        if (gsm.GetState() == GameState::Playing) {
            gsm.SetState(GameState::Paused);
        } else if (gsm.GetState() == GameState::Paused) {
            gsm.SetState(GameState::Playing);
        }
    });

    // Callback pour la console de développement
    inputHandler->SetActionCallback(Core::InputAction::Console, []() {
        // TODO: Basculer la console de dev
    });
}

void GameRefactored::SetupNetworkCallbacks() {
    if (!networkManager) return;

    auto& logger = rtype::core::Logger::getInstance();

    // Callback de création d'entité
    networkManager->SetEntityCreatedCallback([&logger](ECS::Entity entity) {
        logger.debug("Network", "Entity created: " + std::to_string(entity));
    });

    // Callback de destruction d'entité
    networkManager->SetEntityDestroyedCallback([&logger](ECS::Entity entity, uint32_t /*networkId*/) {
        logger.debug("Network", "Entity destroyed: " + std::to_string(entity));
    });

    // Callback de début de partie
    networkManager->SetGameStartCallback([]() {
        GameStateManager::Instance().SetState(GameState::Playing);
    });

    // Callback de statut de connexion
    networkManager->SetConnectionStatusCallback([&logger](bool connected, const std::string& message) {
        if (connected) {
            logger.info("Network", "Connected - " + message);
        } else {
            logger.warning("Network", "Disconnected - " + message);
        }
    });
}

void GameRefactored::SetupCollisionCallbacks() {
    // Collision callbacks will be configured by the collision system itself
    // or through Lua scripts
}

void GameRefactored::HandleLocalCollision(ECS::Entity /*a*/, ECS::Entity /*b*/) {
    if (!gameplayManager) return;

    // Vérifier que les entités existent encore
    // TODO: Implémenter la logique de collision locale
    
    // Pour l'instant, déléguer à GameplayManager
    // gameplayManager->HandleCollision(a, b);
}

bool GameRefactored::LoadScripts() {
    if (!luaState || !assetLoader) return false;

    auto& logger = rtype::core::Logger::getInstance();

    // Charger le script d'initialisation principal
    std::string initScript = assetLoader->ResolveAssetPath("assets/scripts/init.lua");
    if (!luaState->LoadScript(initScript)) {
        logger.error("Game", "Failed to load init.lua");
        return false;
    }

    // Charger les scripts UI
    std::string uiScript = assetLoader->ResolveAssetPath("game/assets/scripts/ui_init.lua");
    if (!luaState->LoadScript(uiScript)) {
        logger.warning("Game", "Could not load ui_init.lua");
    }

    // Configurer les bindings Lua
    SetupLuaBindings();

    return true;
}

void GameRefactored::SetupLuaBindings() {
    if (!luaState) return;

    sol::state& lua = luaState->GetState();

    // Bindings réseau
    RType::Network::NetworkBindings::RegisterAll(lua);
    if (networkManager && networkManager->IsConnected()) {
        RType::Network::NetworkBindings::SetNetworkClient(networkManager->GetNetworkClient());
    }

    // Bindings pour l'audio
    SetupAudioBindings();

    // Bindings pour l'UI
    if (uiSystem) {
        uiSystem->SetLuaState(&lua);
    }

    // Callbacks de gestion d'état
    SetupGameStateBindings();

    // Chemin de base des assets pour Lua
    lua["ASSET_BASE_PATH"] = assetLoader->GetBasePath();
}

void GameRefactored::SetupAudioBindings() {
    if (!luaState || !audioManager) return;

    sol::state& lua = luaState->GetState();

    // Callbacks de volume
    lua["OnMusicVolumeChanged"] = [this](float value) {
        audioManager->SetMusicVolume(value);
    };

    lua["OnSFXVolumeChanged"] = [this](float value) {
        audioManager->SetSFXVolume(value);
    };

    lua["SaveUserSettingsToFile"] = [this]() {
        if (audioManager && assetLoader) {
            audioManager->SaveUserSettings(assetLoader->ResolveAssetPath("user_settings.lua"));
        }
    };

    // Namespace Audio pour Lua
    auto audioNamespace = lua["Audio"].get_or_create<sol::table>();
    
    audioNamespace["PlayMusic"] = [this](const std::string& name, bool loop) {
        audioManager->PlayMusic(name, loop);
    };

    audioNamespace["PlaySFX"] = [this](const std::string& name, float volumeMult) {
        audioManager->PlaySFX(name, volumeMult);
    };

    audioNamespace["SetMusicVolume"] = [this](float volume) {
        audioManager->SetMusicVolume(volume);
    };

    audioNamespace["SetSFXVolume"] = [this](float volume) {
        audioManager->SetSFXVolume(volume);
    };

    // Contrôles de contexte musical
    audioNamespace["SetStage"] = [this](int stage) {
        audioManager->SetCurrentStage(stage);
    };

    audioNamespace["OnBossSpawned"] = [this]() {
        audioManager->OnBossSpawned();
    };

    audioNamespace["OnVictory"] = [this]() {
        audioManager->OnVictory();
    };
}

void GameRefactored::SetupGameStateBindings() {
    if (!luaState) return;

    sol::state& lua = luaState->GetState();
    
    // Créer la table GameState pour Lua
    auto gameStateTable = lua["GameState"].get_or_create<sol::table>();
    
    gameStateTable["Set"] = [](const std::string& state) {
        auto& gsm = GameStateManager::Instance();
        if (state == "playing" || state == "Playing") {
            gsm.SetState(GameState::Playing);
        } else if (state == "paused" || state == "Paused") {
            gsm.SetState(GameState::Paused);
        } else if (state == "menu" || state == "MainMenu") {
            gsm.SetState(GameState::MainMenu);
        } else if (state == "options" || state == "Options") {
            gsm.SetState(GameState::Options);
        }
    };

    gameStateTable["Get"] = []() -> std::string {
        auto state = GameStateManager::Instance().GetState();
        switch (state) {
            case GameState::MainMenu: return "MainMenu";
            case GameState::Playing: return "Playing";
            case GameState::Paused: return "Paused";
            case GameState::Options: return "Options";
            case GameState::Lobby: return "Lobby";
            case GameState::Credits: return "Credits";
            default: return "Unknown";
        }
    };

    gameStateTable["IsPaused"] = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Paused;
    };

    gameStateTable["IsPlaying"] = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Playing;
    };

    gameStateTable["TogglePause"] = [this]() {
        if (networkMode && networkManager) {
            networkManager->SendTogglePause();
        } else {
            auto& gsm = GameStateManager::Instance();
            if (gsm.GetState() == GameState::Playing) {
                gsm.SetState(GameState::Paused);
            } else if (gsm.GetState() == GameState::Paused) {
                gsm.SetState(GameState::Playing);
            }
        }
    };
}

void GameRefactored::InitializeUI() {
    if (!luaState || !uiSystem) return;

    auto& logger = rtype::core::Logger::getInstance();
    sol::state& lua = luaState->GetState();
    sol::protected_function initUI = lua["InitUI"];
    
    if (initUI.valid()) {
        auto winSize = window->getSize();
        sol::protected_function_result result = initUI(winSize.x, winSize.y);
        if (!result.valid()) {
            sol::error err = result;
            logger.error("UI", std::string("InitUI() error: ") + err.what());
        }
    }
}

} // namespace RType
