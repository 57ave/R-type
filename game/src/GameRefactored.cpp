#include "GameRefactored.hpp"
#include "core/GameStateCallbacks.hpp"
#include "network/NetworkBindings.hpp"
#include "core/Logger.hpp"
#include <scripting/UIBindings.hpp>
#include <components/Position.hpp>
#include <components/Sprite.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Tag.hpp>
#include <components/Animation.hpp>
#include <components/NetworkId.hpp>
#include <components/ShootEmUpTags.hpp>
#include <rendering/Types.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>


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

int GameRefactored::Run() {
    auto& logger = rtype::core::Logger::getInstance();
    logger.info("Game", "R-Type starting...");
    
    if (!Initialize()) {
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

bool GameRefactored::Initialize() {
    auto& logger = rtype::core::Logger::getInstance();
    
    if (initialized) {
        logger.warning("Game", "Already initialized");
        return true;
    }

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

    // Load user settings (volumes, etc.)
    if (audioManager && assetLoader) {
        audioManager->LoadUserSettings(assetLoader->ResolveAssetPath("user_settings.lua"));
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
    gameLoop->SetRenderSystem(renderSystem);  // RenderSystem was set in InitializeSystems()
    gameLoop->SetSystemsManager(&systemsManager);  // Pass systems manager
    gameLoop->SetAudioManager(audioManager);
    gameLoop->SetInputHandler(inputHandler);
    gameLoop->SetGameplayManager(gameplayManager);
    gameLoop->SetLuaState(luaState);
    gameLoop->SetNetworkMode(networkMode);
    
    if (networkManager->IsConnected()) {
        gameLoop->SetNetworkSystem(networkManager->GetNetworkSystem());
    }
    
    // Setup collision callback (doit être fait après que les systèmes soient initialisés)
    gameLoop->SetupCollisionCallback();

    // ========================================
    // SCRIPTS ET UI
    // ========================================
    LoadScripts();
    InitializeUI();
    InitializeWorld();  // Create background and initial game entities

    // Définir l'état initial du jeu
    auto& gsm = GameStateManager::Instance();
    if (networkMode) {
        gsm.SetState(GameState::Lobby);  // Attendre la connexion en mode réseau
        logger.info("Game", "Initial game state: Lobby");
    } else {
        gsm.SetState(GameState::MainMenu);  // ✅ Démarrer au menu principal
        logger.info("Game", "Initial game state: MainMenu");
    }

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

void GameRefactored::ApplyConfiguration() {
    auto& logger = rtype::core::Logger::getInstance();
    const auto& config = Core::GameConfig::GetConfiguration();

    // Configuration réseau
    networkMode = (config.network.startMode == "network");
    isNetworkClient = config.network.autoConnect && networkMode;
    
    // Connexion si auto-connect
    if (networkMode && config.network.autoConnect && networkManager) {
        std::string addr = config.network.server.defaultAddress;
        int port = config.network.server.defaultPort;
        
        if (networkManager->ConnectToServer(addr, port, coordinator.get())) {
            logger.info("Network", "Connected to " + addr + ":" + std::to_string(port));
        } else {
            logger.error("Network", "Connection failed");
        }
    }
}

bool GameRefactored::InitializeSystems() {
    if (!Core::GameInitializer::RegisterSystems(*coordinator, *renderer, &uiSystem, &renderSystem, &systemsManager)) {
        return false;
    }

    // UISystem and RenderSystem are now set by RegisterSystems
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

    // Callback de création d'entité - AJOUTER SPRITES ET ANIMATIONS
    networkManager->SetEntityCreatedCallback([this, &logger](ECS::Entity entity) {
        logger.debug("Network", "Entity created: " + std::to_string(entity));
        
        // Vérifier si l'entité a déjà un sprite
        if (coordinator->HasComponent<Sprite>(entity)) {
            return;
        }
        
        // Récupérer le tag pour savoir quel type d'entité
        if (!coordinator->HasComponent<Tag>(entity)) {
            return;
        }
        
        auto& tag = coordinator->GetComponent<Tag>(entity);
        auto& texMap = assetLoader->GetTextureMap();
        auto& sprites = assetLoader->GetAllSprites();
        
        logger.debug("Network", "Creating sprite for entity " + std::to_string(entity) + " (Tag: " + tag.name + ")");
        
        using namespace eng::engine::rendering;
        using eng::engine::rendering::sfml::SFMLSprite;
        
        if (tag.name == "Player") {
            auto playerTexIt = texMap.find("player");
            if (playerTexIt != texMap.end()) {
                auto* sprite = new SFMLSprite();
                sprites.push_back(sprite);
                sprite->setTexture(playerTexIt->second);
                
                // Récupérer la ligne du joueur depuis NetworkId si disponible
                int playerLine = 0;
                if (coordinator->HasComponent<NetworkId>(entity)) {
                    auto& netId = coordinator->GetComponent<NetworkId>(entity);
                    playerLine = netId.playerLine;
                }
                
                IntRect rect(33 * 2, playerLine * 17, 33, 17);
                sprite->setTextureRect(rect);
                
                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.scaleX = 3.0f;
                spriteComp.scaleY = 3.0f;
                spriteComp.layer = 10;
                coordinator->AddComponent(entity, spriteComp);
                
                // Ajouter animation de state machine pour le joueur
                StateMachineAnimation anim;
                anim.currentColumn = 2;
                anim.targetColumn = 2;
                anim.transitionSpeed = 0.15f;
                anim.spriteWidth = 33;
                anim.spriteHeight = 17;
                anim.currentRow = playerLine;
                coordinator->AddComponent(entity, anim);
                
                logger.info("Network", "✅ Player sprite created for entity " + std::to_string(entity));
            }
        }
        else if (tag.name == "Enemy") {
            auto enemyTexIt = texMap.find("enemy");
            if (enemyTexIt != texMap.end()) {
                auto* sprite = new SFMLSprite();
                sprites.push_back(sprite);
                sprite->setTexture(enemyTexIt->second);
                
                IntRect rect(0, 0, 33, 32);
                sprite->setTextureRect(rect);
                
                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.scaleX = 2.5f;
                spriteComp.scaleY = 2.5f;
                spriteComp.layer = 5;
                coordinator->AddComponent(entity, spriteComp);
                
                // Ajouter animation pour l'ennemi
                Animation anim;
                anim.frameCount = 8;
                anim.currentFrame = 0;
                anim.frameTime = 0.1f;
                anim.currentTime = 0.0f;
                anim.loop = true;
                anim.frameWidth = 33;
                anim.frameHeight = 32;
                anim.startX = 0;
                anim.startY = 0;
                anim.spacing = 0;
                coordinator->AddComponent(entity, anim);
                
                logger.info("Network", "✅ Enemy sprite created for entity " + std::to_string(entity));
            }
        }
        else if (tag.name == "PlayerBullet") {
            auto missileTexIt = texMap.find("missile");
            if (missileTexIt != texMap.end()) {
                auto* sprite = new SFMLSprite();
                sprites.push_back(sprite);
                sprite->setTexture(missileTexIt->second);
                
                // Vérifier le niveau de charge pour le bon sprite
                IntRect rect(249, 88, 16, 12);  // Missile normal
                if (coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(entity)) {
                    auto& projTag = coordinator->GetComponent<ShootEmUp::Components::ProjectileTag>(entity);
                    if (projTag.chargeLevel > 0) {
                        rect = IntRect(232, 103, 32, 14);  // Missile chargé
                    }
                }
                sprite->setTextureRect(rect);
                
                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.scaleX = 2.0f;
                spriteComp.scaleY = 2.0f;
                spriteComp.layer = 8;
                coordinator->AddComponent(entity, spriteComp);
                
                logger.debug("Network", "✅ PlayerBullet sprite created for entity " + std::to_string(entity));
            }
        }
        else if (tag.name == "EnemyBullet") {
            auto missileTexIt = texMap.find("missile");
            if (missileTexIt != texMap.end()) {
                auto* sprite = new SFMLSprite();
                sprites.push_back(sprite);
                sprite->setTexture(missileTexIt->second);
                
                IntRect rect(200, 88, 16, 12);  // Missile ennemi
                sprite->setTextureRect(rect);
                
                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.scaleX = 2.0f;
                spriteComp.scaleY = 2.0f;
                spriteComp.layer = 7;
                coordinator->AddComponent(entity, spriteComp);
            }
        }
        else if (tag.name == "Explosion") {
            auto explosionTexIt = texMap.find("explosion");
            if (explosionTexIt != texMap.end()) {
                auto* sprite = new SFMLSprite();
                sprites.push_back(sprite);
                sprite->setTexture(explosionTexIt->second);
                
                IntRect rect(0, 0, 33, 30);
                sprite->setTextureRect(rect);
                
                Sprite spriteComp;
                spriteComp.sprite = sprite;
                spriteComp.textureRect = rect;
                spriteComp.scaleX = 3.0f;
                spriteComp.scaleY = 3.0f;
                spriteComp.layer = 15;
                coordinator->AddComponent(entity, spriteComp);
                
                // Animation d'explosion
                Animation anim;
                anim.frameCount = 6;
                anim.currentFrame = 0;
                anim.frameTime = 0.08f;
                anim.currentTime = 0.0f;
                anim.loop = false;
                anim.frameWidth = 33;
                anim.frameHeight = 30;
                anim.startX = 0;
                anim.startY = 0;
                anim.spacing = 0;
                coordinator->AddComponent(entity, anim);
            }
        }
    });

    // Callback de destruction d'entité - EFFET D'EXPLOSION
    networkManager->SetEntityDestroyedCallback([this, &logger](ECS::Entity entity, uint32_t networkId) {
        logger.debug("Network", "Entity destroyed: " + std::to_string(entity) + " (netId: " + std::to_string(networkId) + ")");
        
        // Créer un effet d'explosion à la position de l'entité si c'est un ennemi ou joueur
        if (coordinator->HasComponent<Tag>(entity)) {
            auto& tag = coordinator->GetComponent<Tag>(entity);
            
            if (tag.name == "Enemy" || tag.name == "Player") {
                // Jouer le son d'explosion
                if (audioManager) {
                    audioManager->PlaySFX("explosion");
                }
            }
        }
    });

    // Callback de début de partie
    networkManager->SetGameStartCallback([this]() {
        GameStateManager::Instance().SetState(GameState::Playing);
        
        // Jouer la musique de jeu
        if (audioManager) {
            audioManager->FadeToMusic("stage_one_music", 1.5f);
        }
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

    // Charger le script d'initialisation principal (MAINTENANT dans game/assets/scripts/)
    std::string initScript = assetLoader->ResolveAssetPath("game/assets/scripts/init.lua");
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

    // Override Network.Connect to use the refactored Core::NetworkManager + NetworkSystem.
    // This ensures incoming packets are processed and forwarded to Lua callbacks.
    auto netTable = lua["Network"].get_or_create<sol::table>();
    netTable["Connect"] = [this](const std::string& host, int port) {
        if (!luaState) {
            std::cerr << "[GameRefactored] LuaState not available" << std::endl;
            return;
        }

        sol::state& lua = luaState->GetState();
        if (!networkManager || !coordinator) {
            std::cerr << "[GameRefactored] NetworkManager/Coordinator not available" << std::endl;
            return;
        }

        if (host.empty() || port <= 0) {
            std::cerr << "[GameRefactored] Invalid host/port" << std::endl;
            return;
        }

        // If already connected, just notify UI.
        if (networkManager->IsConnected()) {
            if (gameLoop) {
                gameLoop->SetNetworkSystem(networkManager->GetNetworkSystem());
                gameLoop->SetNetworkMode(true);
            }
            RType::Network::NetworkBindings::SetNetworkClient(networkManager->GetNetworkClient());

            sol::protected_function onConnected = lua["OnConnected"];
            if (onConnected.valid()) {
                onConnected(host, port);
            }
            GameStateManager::Instance().SetState(GameState::Lobby);
            return;
        }

        const bool ok = networkManager->ConnectToServer(host, static_cast<short>(port), coordinator.get());
        if (!ok) {
            std::cerr << "[GameRefactored] Connection failed" << std::endl;
            return;
        }

        networkMode = true;
        isNetworkClient = true;

        // Hook the live client/system into the game loop and Lua bindings.
        if (gameLoop) {
            gameLoop->SetNetworkSystem(networkManager->GetNetworkSystem());
            gameLoop->SetNetworkMode(true);
        }
        RType::Network::NetworkBindings::SetNetworkClient(networkManager->GetNetworkClient());

        // Notify Lua UI
        sol::protected_function onConnected = lua["OnConnected"];
        if (onConnected.valid()) {
            auto res = onConnected(host, port);
            if (!res.valid()) {
                sol::error err = res;
                std::cerr << "[GameRefactored] Lua OnConnected error: " << err.what() << std::endl;
            }
        }

        // Lobby is the multiplayer/menu state.
        GameStateManager::Instance().SetState(GameState::Lobby);
    };

    // Settings: apply resolution/fullscreen (used by ui_init.lua)
    lua["ApplyResolution"] = [this](int resolutionIndex, bool fullscreen) {
        if (!window) return;

        static int lastAppliedResolution = -1;
        static bool lastAppliedFullscreen = false;

        if (resolutionIndex == lastAppliedResolution && fullscreen == lastAppliedFullscreen) {
            return;
        }

        const std::vector<std::pair<uint32_t, uint32_t>> resolutions = {
            {1920, 1080},
            {1280, 720},
            {1600, 900}
        };

        if (resolutionIndex < 0 || resolutionIndex >= static_cast<int>(resolutions.size())) {
            std::cerr << "[GameRefactored] Invalid resolution index: " << resolutionIndex << std::endl;
            return;
        }

        auto [w, h] = resolutions[resolutionIndex];

        if (fullscreen) {
            windowWidth = static_cast<int>(w);
            windowHeight = static_cast<int>(h);
            window->setSize(w, h);
            window->setFullscreen(true);
        } else {
            windowWidth = static_cast<int>(w);
            windowHeight = static_cast<int>(h);
            window->setSize(w, h);
            window->setFullscreen(false);
        }

        if (gameplayManager) {
            gameplayManager->SetWindowSize(static_cast<float>(w), static_cast<float>(h));
        }

        lastAppliedResolution = resolutionIndex;
        lastAppliedFullscreen = fullscreen;
    };

    // Bindings pour l'audio
    SetupAudioBindings();
    
    // Bindings pour l'UI
    if (uiSystem) {
        uiSystem->SetLuaState(&lua);
        Scripting::UIBindings::RegisterAll(lua, uiSystem.get());
    }

    // Callbacks de gestion d'état
    SetupGameStateBindings();

    // Namespace Game pour la création d'entités
    auto gameNamespace = lua["Game"].get_or_create<sol::table>();
    
    gameNamespace["CreatePlayer"] = [this](float x, float y, int line) -> int {
        if (!gameplayManager) {
            auto& logger = rtype::core::Logger::getInstance();
            logger.error("Lua", "GameplayManager not available");
            return 0;
        }
        ECS::Entity player = gameplayManager->CreatePlayer(x, y, line);
        auto& logger = rtype::core::Logger::getInstance();
        logger.info("Lua", "Player created: " + std::to_string(player));
        return static_cast<int>(player);
    };

    gameNamespace["CreateEnemy"] = [this](float x, float y, const std::string& pattern, const std::string& type) -> int {
        if (!gameplayManager) return 0;
        ECS::Entity enemy = gameplayManager->CreateEnemy(x, y, pattern, type);
        return static_cast<int>(enemy);
    };
    
    gameNamespace["CreateMissile"] = [this](float x, float y, bool isCharged, int chargeLevel) -> int {
        if (!gameplayManager) return 0;
        ECS::Entity missile = gameplayManager->CreateMissile(x, y, isCharged, chargeLevel);
        return static_cast<int>(missile);
    };
    
    gameNamespace["CreateExplosion"] = [this](float x, float y) -> int {
        if (!gameplayManager) return 0;
        ECS::Entity explosion = gameplayManager->CreateExplosion(x, y);
        return static_cast<int>(explosion);
    };
    
    gameNamespace["CreateEnemyMissile"] = [this](float x, float y) -> int {
        if (!gameplayManager) return 0;
        ECS::Entity missile = gameplayManager->CreateEnemyMissile(x, y);
        return static_cast<int>(missile);
    };

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
    
    audioNamespace["FadeToMusic"] = [this](const std::string& name, float duration) {
        audioManager->FadeToMusic(name, duration);
    };
    
    audioNamespace["StopMusic"] = [this]() {
        audioManager->StopMusic();
    };
    
    audioNamespace["PauseMusic"] = [this]() {
        audioManager->PauseMusic();
    };
    
    audioNamespace["ResumeMusic"] = [this]() {
        audioManager->ResumeMusic();
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
    
    audioNamespace["GetMusicVolume"] = [this]() -> float {
        return audioManager->GetMusicVolume();
    };
    
    audioNamespace["GetSFXVolume"] = [this]() -> float {
        return audioManager->GetSFXVolume();
    };

    // Contrôles de contexte musical
    audioNamespace["SetStage"] = [this](int stage) {
        audioManager->SetCurrentStage(stage);
    };

    audioNamespace["OnBossSpawned"] = [this]() {
        audioManager->OnBossSpawned();
    };
    
    audioNamespace["OnBossDefeated"] = [this]() {
        audioManager->OnBossDefeated();
    };
    
    audioNamespace["OnGameOver"] = [this]() {
        audioManager->OnGameOver();
    };

    audioNamespace["OnVictory"] = [this]() {
        audioManager->OnVictory();
    };
    
    std::cout << "[GameRefactored] Audio bindings configured" << std::endl;
}

void GameRefactored::SetupGameStateBindings() {
    if (!luaState) return;

    sol::state& lua = luaState->GetState();
    
    // Difficulty callback
    lua["OnDifficultyChanged"] = [this](int index) {
        std::vector<std::string> difficulties = {"easy", "normal", "hard"};
        if (index >= 0 && index < 3 && gameplayManager) {
            gameplayManager->LoadDifficulty(difficulties[index]);
        }
    };
    
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

    // ✅ Reset game state for retry/restart
    gameStateTable["Reset"] = [this]() {
        std::cout << "[GameState] Reset called - cleaning up game entities" << std::endl;
        if (gameLoop) {
            gameLoop->ResetGameState();
        }
        // Also reset gameplay manager entities
        if (gameplayManager) {
            gameplayManager->ProcessDestroyedEntities();
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

void GameRefactored::InitializeWorld() {
    if (!coordinator || !assetLoader) return;

    auto& logger = rtype::core::Logger::getInstance();
    logger.info("Game", "Creating game world (background)...");

    // Créer le background scrollant
    auto* backgroundTexture = assetLoader->GetTexture("background");
    if (!backgroundTexture) {
        logger.error("Game", "Background texture not loaded!");
        return;
    }

    // Créer 2 sprites de background pour le parallaxe infini
    // Ils doivent être côte à côte pour créer un scrolling continu
    for (int i = 0; i < 2; ++i) {
        ECS::Entity bg = coordinator->CreateEntity();

        // Position : sprite 0 à x=0, sprite 1 à x=windowWidth
        Position pos;
        pos.x = static_cast<float>(i * windowWidth);
        pos.y = 0.0f;
        coordinator->AddComponent(bg, pos);

        // Sprite - Utiliser AssetLoader::CreateSprite
        auto* sprite = assetLoader->CreateSprite("background");
        if (!sprite) {
            logger.error("Game", "Failed to create background sprite " + std::to_string(i));
            continue;
        }
        sprite->setPosition(eng::engine::rendering::Vector2f(pos.x, pos.y));

        ::Sprite spriteComp;
        spriteComp.sprite = sprite;
        spriteComp.layer = -10;  // Background layer (lowest)
        spriteComp.scaleX = static_cast<float>(windowWidth) / backgroundTexture->getSize().x;
        spriteComp.scaleY = static_cast<float>(windowHeight) / backgroundTexture->getSize().y;
        coordinator->AddComponent(bg, spriteComp);

        // ScrollingBackground component - Configuration pour parallaxe infini
        ::ScrollingBackground scrollComp;
        scrollComp.scrollSpeed = 50.0f;  // Vitesse réduite pour un meilleur effet
        scrollComp.horizontal = true;
        scrollComp.loop = true;
        scrollComp.spriteWidth = static_cast<float>(windowWidth);
        coordinator->AddComponent(bg, scrollComp);

        // Tag
        ::Tag tag;
        tag.name = "Background";
        coordinator->AddComponent(bg, tag);

        logger.debug("Game", "Background " + std::to_string(i) + " created at x=" + std::to_string(pos.x));
    }

    logger.info("Game", "Game world initialized");
    
    // Ne PAS créer le joueur ici - il sera créé par GameLoop
    // quand on passe en mode Playing
}

} // namespace RType
