#include "GameRefactored.hpp"
#include "core/GameStateCallbacks.hpp"
#include "network/NetworkBindings.hpp"
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
    std::cout << "[GameRefactored] Created" << std::endl;
}

GameRefactored::~GameRefactored() {
    if (initialized) {
        Shutdown();
    }
    std::cout << "[GameRefactored] Destroyed" << std::endl;
}

int GameRefactored::Run(int argc, char* argv[]) {
    std::cout << "R-Type Game Starting with Refactored Architecture..." << std::endl;
    
    if (!Initialize(argc, argv)) {
        std::cerr << "[GameRefactored] Failed to initialize game" << std::endl;
        return 1;
    }
    
    // Boucle de jeu principale
    while (gameLoop && gameLoop->Update()) {
        // La boucle de jeu gère tout
    }
    
    Shutdown();
    std::cout << "[GameRefactored] Game ended" << std::endl;
    return 0;
}

bool GameRefactored::Initialize(int argc, char* argv[]) {
    if (initialized) {
        std::cout << "[GameRefactored] Already initialized" << std::endl;
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
    
    std::cout << "[GameRefactored] Initializing ECS components..." << std::endl;
    Core::GameInitializer::RegisterComponents(*coordinator);

    // ========================================
    // INITIALISATION LUA ET CONFIGURATION
    // ========================================
    luaState = ::Scripting::LuaState::Create();
    luaState->Init();
    luaState->EnableHotReload(true);

    if (!Core::GameConfig::LoadConfiguration(*luaState)) {
        std::cout << "[GameRefactored] Using default configuration" << std::endl;
    }

    // Appliquer la configuration chargée
    ApplyConfiguration();

    // ========================================
    // INITIALISATION DES GESTIONNAIRES
    // ========================================
    
    // Asset Loader
    assetLoader = std::make_unique<Core::AssetLoader>();
    const auto& config = Core::GameConfig::GetConfiguration();
    std::string basePath = Core::GameConfig::ResolveAssetPath("");
    if (!assetLoader->Initialize(basePath)) {
        std::cerr << "[GameRefactored] Failed to initialize asset loader" << std::endl;
        return false;
    }
    
    // Précharger les ressources
    assetLoader->PreloadAllTextures();
    assetLoader->PreloadAllSounds();

    // Audio Manager
    audioManager = std::make_shared<Core::AudioManager>();
    if (!audioManager->Initialize(assetLoader->GetBasePath())) {
        std::cout << "[GameRefactored] Audio manager initialization failed, continuing without audio" << std::endl;
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
        std::cerr << "[GameRefactored] Failed to initialize systems" << std::endl;
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
    std::cout << "[GameRefactored] ✅ Game fully initialized!" << std::endl;
    
    return true;
}

void GameRefactored::Shutdown() {
    if (!initialized) return;

    std::cout << "[GameRefactored] Shutting down..." << std::endl;

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

    if (luaState) {
        delete luaState;
        luaState = nullptr;
    }
    
    renderer.reset();
    window.reset();
    coordinator.reset();

    initialized = false;
    std::cout << "[GameRefactored] Shutdown complete" << std::endl;
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

void GameRefactored::ParseCommandLineArguments(int argc, char* argv[]) {
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
        
        std::cout << "[GameRefactored] Legacy --network flag detected" << std::endl;
        std::cout << "[GameRefactored] Server: " << serverAddress << ":" << serverPort << std::endl;
    }
}

void GameRefactored::ApplyConfiguration() {
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
            std::cout << "[GameRefactored] Auto-connect enabled: " 
                      << legacyServerAddress << ":" << legacyServerPort << std::endl;
        }
    }

    std::cout << "[GameRefactored] Configuration applied:" << std::endl;
    std::cout << "  Window: " << windowWidth << "x" << windowHeight << std::endl;
    std::cout << "  Network mode: " << (networkMode ? "enabled" : "disabled") << std::endl;
}

bool GameRefactored::InitializeSystems() {
    // Register and initialize all ECS systems
    std::cout << "[GameRefactored] Registering systems..." << std::endl;
    if (!Core::GameInitializer::RegisterSystems(*coordinator, *renderer)) {
        return false;
    }

    // UI System
    uiSystem = coordinator->RegisterSystem<UISystem>();
    uiSystem->SetCoordinator(coordinator.get());
    uiSystem->Init();
    uiSystem->SetWindow(window.get());
    
    // Charger la police par défaut pour l'UI
    std::string fontPath = Core::GameConfig::ResolveAssetPath("game/assets/fonts/Roboto-Regular.ttf");
    if (!uiSystem->LoadFont("default", fontPath)) {
        std::cout << "[GameRefactored] Warning: Could not load default UI font" << std::endl;
    }

    // Configurer les callbacks de collision (will be set up later if needed)
    // SetupCollisionCallbacks();

    std::cout << "[GameRefactored] All systems initialized" << std::endl;
    return true;
}

void GameRefactored::SetupInputCallbacks() {
    if (!inputHandler) return;

    // Callback pour la pause
    inputHandler->SetActionCallback(Core::InputAction::Pause, [this]() {
        auto& gsm = GameStateManager::Instance();
        if (gsm.GetState() == GameState::Playing) {
            gsm.SetState(GameState::Paused);
        } else if (gsm.GetState() == GameState::Paused) {
            gsm.SetState(GameState::Playing);
        }
    });

    // Callback pour la console de développement
    inputHandler->SetActionCallback(Core::InputAction::Console, [this]() {
        // TODO: Basculer la console de dev
        std::cout << "[GameRefactored] Dev console toggled" << std::endl;
    });

    std::cout << "[GameRefactored] Input callbacks configured" << std::endl;
}

void GameRefactored::SetupNetworkCallbacks() {
    if (!networkManager) return;

    // Callback de création d'entité
    networkManager->SetEntityCreatedCallback([this](ECS::Entity entity) {
        // TODO: Registrer l'entité avec GameplayManager
        std::cout << "[GameRefactored] Network entity created: " << entity << std::endl;
    });

    // Callback de destruction d'entité
    networkManager->SetEntityDestroyedCallback([this](ECS::Entity entity, uint32_t networkId) {
        std::cout << "[GameRefactored] Network entity destroyed: " << entity << std::endl;
    });

    // Callback de début de partie
    networkManager->SetGameStartCallback([this]() {
        std::cout << "[GameRefactored] Game start received from server" << std::endl;
        GameStateManager::Instance().SetState(GameState::Playing);
    });

    // Callback de statut de connexion
    networkManager->SetConnectionStatusCallback([this](bool connected, const std::string& message) {
        std::cout << "[GameRefactored] Connection status: " 
                  << (connected ? "connected" : "disconnected") 
                  << " - " << message << std::endl;
    });

    std::cout << "[GameRefactored] Network callbacks configured" << std::endl;
}

void GameRefactored::SetupCollisionCallbacks() {
    // Collision callbacks will be configured by the collision system itself
    // or through Lua scripts
    std::cout << "[GameRefactored] Collision callbacks will be set up by systems" << std::endl;
}

void GameRefactored::HandleLocalCollision(ECS::Entity a, ECS::Entity b) {
    if (!gameplayManager) return;

    // Vérifier que les entités existent encore
    // TODO: Implémenter la logique de collision locale
    
    // Pour l'instant, déléguer à GameplayManager
    // gameplayManager->HandleCollision(a, b);
}

bool GameRefactored::LoadScripts() {
    if (!luaState || !assetLoader) return false;

    // Charger le script d'initialisation principal
    std::string initScript = assetLoader->ResolveAssetPath("assets/scripts/init.lua");
    if (!luaState->LoadScript(initScript)) {
        std::cerr << "[GameRefactored] Failed to load init.lua" << std::endl;
        return false;
    }

    // Charger les scripts UI
    std::string uiScript = assetLoader->ResolveAssetPath("game/assets/scripts/ui_init.lua");
    if (!luaState->LoadScript(uiScript)) {
        std::cout << "[GameRefactored] Warning: Could not load ui_init.lua" << std::endl;
    }

    // Configurer les bindings Lua
    SetupLuaBindings();

    std::cout << "[GameRefactored] Scripts loaded" << std::endl;
    return true;
}

void GameRefactored::SetupLuaBindings() {
    if (!luaState) return;

    sol::state& lua = luaState->GetState();

    // Bindings pour les composants (from engine)
    ::Scripting::ComponentBindings::RegisterAll(lua);
    ::Scripting::ComponentBindings::RegisterCoordinator(lua, coordinator.get());

    // Bindings réseau
    RType::Network::NetworkBindings::RegisterAll(lua);
    if (networkManager && networkManager->IsConnected()) {
        RType::Network::NetworkBindings::SetNetworkClient(networkManager->GetNetworkClient());
    }

    // Bindings pour l'audio
    SetupAudioBindings();

    // Bindings pour l'UI
    if (uiSystem) {
        ::Scripting::UIBindings::RegisterAll(lua, uiSystem.get());
        uiSystem->SetLuaState(&lua);
    }

    // Callbacks de gestion d'état
    SetupGameStateBindings();

    // Chemin de base des assets pour Lua
    lua["ASSET_BASE_PATH"] = assetLoader->GetBasePath();

    std::cout << "[GameRefactored] Lua bindings configured" << std::endl;
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

    // Configuration des callbacks d'état de jeu
    eng::engine::core::GameStateCallbacks gameStateCallbacks;
    
    gameStateCallbacks.setState = [](const std::string& state) {
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

    gameStateCallbacks.getState = []() -> std::string {
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

    gameStateCallbacks.isPaused = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Paused;
    };

    gameStateCallbacks.isPlaying = []() -> bool {
        return GameStateManager::Instance().GetState() == GameState::Playing;
    };

    gameStateCallbacks.togglePause = [this]() {
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

    ::Scripting::UIBindings::SetGameStateCallbacks(gameStateCallbacks);
}

void GameRefactored::InitializeUI() {
    if (!luaState || !uiSystem) return;

    sol::state& lua = luaState->GetState();
    sol::protected_function initUI = lua["InitUI"];
    if (initUI.valid()) {
        auto winSize = window->getSize();
        sol::protected_function_result result = initUI(winSize.x, winSize.y);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[GameRefactored] InitUI() error: " << err.what() << std::endl;
        } else {
            std::cout << "[GameRefactored] UI initialized from Lua" << std::endl;
        }
    }
}

} // namespace RType
