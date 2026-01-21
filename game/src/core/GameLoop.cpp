#include "core/GameLoop.hpp"
#include "core/GameStateCallbacks.hpp"
#include "GameStateManager.hpp"
#include <iostream>
#include <chrono>
#include <thread>

namespace RType::Core {

GameLoop::GameLoop(ECS::Coordinator* coordinator)
    : coordinator(coordinator)
    , clock()
    , deltaTime(0.0f)
    , enemySpawnTimer(0.0f)
    , enemySpawnInterval(2.0f)
    , enemyShootTimer(0.0f)
    , enemyShootInterval(1.5f)
    , spacePressed(false)
    , spaceHoldTime(0.0f)
    , activeChargingEffect(0)
    , hasChargingEffect(false)
    , gamePlayTime(0.0f)
    , winConditionTriggered(false)
    , winDisplayTimer(0.0f)
    , inputMask(0)
    , player(0)
    , playerCreated(false)
    , systemsManager(nullptr)
{
    std::cout << "[GameLoop] Initialized" << std::endl;
}

GameLoop::~GameLoop() {
    std::cout << "[GameLoop] Destroyed" << std::endl;
}

void GameLoop::SetNetworkMode(bool enable) {
    networkMode = enable;
    std::cout << "[GameLoop] Network mode: " << (enable ? "enabled" : "disabled") << std::endl;
}

void GameLoop::SetNetworkSystem(std::shared_ptr<eng::engine::systems::NetworkSystem> netSystem) {
    networkSystem = netSystem;
    if (networkSystem) {
        std::cout << "[GameLoop] Network system connected" << std::endl;
    }
}

void GameLoop::SetUISystem(std::shared_ptr<UISystem> ui) {
    uiSystem = ui;
    if (uiSystem) {
        std::cout << "[GameLoop] UI system connected" << std::endl;
    }
}

void GameLoop::SetRenderSystem(std::shared_ptr<RenderSystem> render) {
    renderSystem = render;
    if (renderSystem) {
        std::cout << "[GameLoop] Render system connected" << std::endl;
    }
}

void GameLoop::SetSystemsManager(SystemsManager* systems) {
    systemsManager = systems;
    if (systemsManager) {
        std::cout << "[GameLoop] Systems manager connected" << std::endl;
    }
}

void GameLoop::SetAudioManager(std::shared_ptr<AudioManager> audio) {
    audioManager = audio;
    if (audioManager) {
        std::cout << "[GameLoop] Audio manager connected" << std::endl;
    }
}

void GameLoop::SetInputHandler(std::shared_ptr<InputHandler> input) {
    inputHandler = input;
    if (inputHandler) {
        std::cout << "[GameLoop] Input handler connected" << std::endl;
    }
}

void GameLoop::SetGameplayManager(std::shared_ptr<GameplayManager> gameplay) {
    gameplayManager = gameplay;
    if (gameplayManager) {
        std::cout << "[GameLoop] Gameplay manager connected" << std::endl;
    }
}

void GameLoop::SetScriptSystem(std::shared_ptr<Scripting::ScriptSystem> script) {
    spawnScriptSystem = script;
    if (spawnScriptSystem) {
        std::cout << "[GameLoop] Spawn script system connected" << std::endl;
    }
}

void GameLoop::SetLuaState(Scripting::LuaState* lua) {
    luaState = lua;
    if (luaState) {
        std::cout << "[GameLoop] Lua state connected" << std::endl;
    }
}

void GameLoop::SetWindow(eng::engine::rendering::sfml::SFMLWindow* win) {
    window = win;
    if (window) {
        std::cout << "[GameLoop] Window connected" << std::endl;
    }
}

bool GameLoop::Update() {
    if (!window || !window->isOpen()) {
        return false;
    }

    // Calculate delta time
    deltaTime = clock.restart();
    
    // Cap deltaTime to prevent huge jumps (max 0.1s = 10 FPS minimum)
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }

    // ========================================
    // GAME STATE MANAGEMENT
    // ========================================
    auto currentState = GameStateManager::Instance().GetState();
    
    // Créer le joueur quand on passe en mode Playing
    if (currentState == GameState::Playing && !playerCreated && !networkMode && gameplayManager) {
        player = gameplayManager->CreatePlayer(150.0f, static_cast<float>(window->getSize().y) / 2.0f, 0);
        playerCreated = true;
        std::cout << "[GameLoop] Player created: " << player << std::endl;
    }
    
    // Skip game logic updates when in menu states
    bool inMenu = (currentState == GameState::MainMenu || 
                   currentState == GameState::Paused ||
                   currentState == GameState::Options ||
                   currentState == GameState::Lobby ||
                   currentState == GameState::Credits);

    // ========================================
    // HANDLE EVENTS
    // ========================================
    HandleEvents();

    // ========================================
    // UPDATE SYSTEMS ECS
    // ========================================
    if (systemsManager) {
        // Les systèmes visuels tournent toujours (même en menu)
        systemsManager->UpdateVisualSystems(deltaTime);
        
        // Les systèmes de gameplay uniquement en jeu
        if (!inMenu) {
            if (networkMode) {
                // Mode réseau : pas de simulation locale (serveur autoritaire)
                // On garde quand même les animations
            } else {
                // Mode local : simulation complète
                systemsManager->UpdateGameplaySystems(deltaTime);
            }
        }
    }

    // ========================================
    // HANDLE INPUT (only when not in menu)
    // ========================================
    if (!inMenu && inputHandler) {
        inputHandler->Update(deltaTime);
        
        // Handle charging input
        HandleChargingInput(deltaTime);
    }

    // ========================================
    // UPDATE GAME LOGIC
    // ========================================
    if (!inMenu) {
        UpdateGameLogic(deltaTime);
        UpdateWinCondition(deltaTime);
    }

    // ========================================
    // NETWORK UPDATES
    // ========================================
    if (networkMode && networkSystem) {
        UpdateNetworking(deltaTime);
    }

    // ========================================
    // SPAWN ENEMIES (local mode only)
    // ========================================
    if (!inMenu && !networkMode && gameplayManager) {
        UpdateEnemySpawning(deltaTime);
    }

    // ========================================
    // AUDIO UPDATES
    // ========================================
    if (audioManager) {
        audioManager->Update(deltaTime);
    }

    // ========================================
    // UI UPDATES
    // ========================================
    if (uiSystem && inMenu) {
        uiSystem->Update(deltaTime);
    }

    // ========================================
    // RENDER
    // ========================================
    Render();

    return true;
}

void GameLoop::HandleEvents() {
    if (!window) return;

    eng::engine::InputEvent event;
    while (window->pollEvent(event)) {
        // Handle window events
        if (event.type == eng::engine::EventType::Closed) {
            window->close();
            return;
        }
        
        // Handle resize events
        if (event.type == eng::engine::EventType::Resized) {
            std::cout << "[GameLoop] Window resized to " 
                      << event.size.width << "x" << event.size.height << std::endl;
        }
        
        // Forward events to UI system
        if (uiSystem) {
            uiSystem->HandleEvent(event);
        }
        
        // Forward events to input handler
        if (inputHandler) {
            inputHandler->HandleEvent(event);
        }
    }
}

void GameLoop::UpdateSystems(float dt, bool inMenu) {
    if (!coordinator) return;

    // ========================================
    // MISE À JOUR DES SYSTÈMES ECS
    // ========================================
    
    // Les systèmes sont appelés directement via leurs shared_ptr stockés
    // dans GameInitializer. Pour l'instant, on va utiliser une approche
    // où les systèmes s'auto-mettent à jour via leurs signatures.
    
    // Note: Les systèmes enregistrés via RegisterSystem sont automatiquement
    // notifiés des changements d'entités grâce aux signatures.
    // Mais ils ont besoin d'être explicitement mis à jour.
    
    // Pour une architecture propre, nous allons déléguer les appels
    // aux gestionnaires qui ont accès aux systèmes.
    
    // Les systèmes visuels (animation, scrolling) sont toujours actifs
    // Les systèmes de gameplay (mouvement, collision) uniquement en jeu
    
    // Cette méthode est un placeholder - les systèmes seront appelés
    // individuellement dans Update() avec les bonnes conditions
}

void GameLoop::UpdateGameLogic(float dt) {
    if (!luaState) return;

    // Update Lua game logic
    sol::state& lua = luaState->GetState();
    sol::protected_function updateGame = lua["UpdateGame"];
    if (updateGame.valid()) {
        sol::protected_function_result result = updateGame(dt);
        if (!result.valid()) {
            sol::error err = result;
            // Silently fail - don't spam console with errors
        }
    }

    // Update spawn script system
    if (spawnScriptSystem) {
        spawnScriptSystem->Update(dt);
    }
}

void GameLoop::UpdateWinCondition(float dt) {
    auto currentState = GameStateManager::Instance().GetState();
    
    if (currentState == GameState::Playing && !winConditionTriggered) {
        gamePlayTime += dt;
        
        // Check if 30 seconds have elapsed
        const float WIN_TIME_THRESHOLD = 30.0f;
        if (gamePlayTime >= WIN_TIME_THRESHOLD) {
            std::cout << "[GameLoop] 🎉 WIN! Player survived for " << gamePlayTime << " seconds!" << std::endl;
            winConditionTriggered = true;
            winDisplayTimer = 0.0f;
            
            // Transition to Victory state
            GameStateManager::Instance().SetState(GameState::Victory);
            
            // Play victory music/sound if available
            if (audioManager) {
                audioManager->OnVictory();
            }
        }
    }
}

void GameLoop::UpdateNetworking(float dt) {
    if (!networkSystem) return;

    // Process network packets
    networkSystem->Update(dt);
    
    // Send input updates
    if (inputHandler && networkMode) {
        uint8_t inputMask = inputHandler->GetNetworkInputMask();
        networkSystem->sendInput(inputMask);
    }
}

void GameLoop::UpdateEnemySpawning(float dt) {
    if (!gameplayManager) return;

    enemySpawnTimer += dt;
    enemyShootTimer += dt;

    // Spawn enemies with increasing difficulty
    if (enemySpawnTimer >= enemySpawnInterval) {
        gameplayManager->SpawnRandomEnemy();
        enemySpawnTimer = 0.0f;
        
        std::cout << "[GameLoop] Enemy spawned, next in " << enemySpawnInterval << "s" << std::endl;
        
        // Gradually decrease spawn interval (increase difficulty)
        if (enemySpawnInterval > 0.5f) {
            enemySpawnInterval -= 0.05f;
        }
    }

    // Make enemies shoot
    if (enemyShootTimer >= enemyShootInterval) {
        gameplayManager->MakeEnemiesShoot();
        enemyShootTimer = 0.0f;
    }
}

void GameLoop::HandleChargingInput(float dt) {
    if (!inputHandler) return;

    const float chargeStartTime = 0.1f; // Define at function scope
    bool currentSpacePressed = inputHandler->IsActionPressed(InputAction::Shoot);
    
    if (currentSpacePressed && !spacePressed) {
        // Space just pressed - start tracking hold time
        spaceHoldTime = 0.0f;
        spacePressed = true;
    } else if (currentSpacePressed && spacePressed) {
        // Space held - update hold time and charging effect
        spaceHoldTime += dt;
        
        if (spaceHoldTime >= chargeStartTime && !hasChargingEffect && gameplayManager) {
            // Start charging effect
            auto playerEntity = gameplayManager->GetLocalPlayerEntity();
            if (playerEntity != 0) {
                activeChargingEffect = gameplayManager->CreateChargingEffect(playerEntity);
                hasChargingEffect = true;
            }
        }
    } else if (!currentSpacePressed && spacePressed) {
        // Space released - fire missile(s)
        spacePressed = false;
        
        if (gameplayManager) {
            auto playerEntity = gameplayManager->GetLocalPlayerEntity();
            if (playerEntity != 0) {
                if (spaceHoldTime >= chargeStartTime) {
                    // Charged shot
                    int chargeLevel = CalculateChargeLevel(spaceHoldTime);
                    gameplayManager->FireChargedMissile(playerEntity, chargeLevel);
                } else {
                    // Normal shot
                    gameplayManager->FireMissile(playerEntity);
                }
            }
        }
        
        // Clean up charging effect
        if (hasChargingEffect && activeChargingEffect != 0) {
            coordinator->DestroyEntity(activeChargingEffect);
            activeChargingEffect = 0;
            hasChargingEffect = false;
        }
        
        spaceHoldTime = 0.0f;
    }
}

int GameLoop::CalculateChargeLevel(float holdTime) 
{
    const float chargeStartTime = 0.1f;
    const float maxChargeTime = 1.0f;
    
    if (holdTime < chargeStartTime) return 0;
    
    float chargeProgress = (holdTime - chargeStartTime) / (maxChargeTime - chargeStartTime);
    chargeProgress = std::min(1.0f, chargeProgress);
    
    return static_cast<int>(chargeProgress * 5) + 1; // 1-5 charge levels
}

void GameLoop::Render() {
    if (!window) return;

    window->clear();

    // Render game world through ECS render system
    if (renderSystem) {
        renderSystem->Update(deltaTime);
    }

    // Render UI on top
    if (uiSystem) {
        uiSystem->Render(window);
    }

    window->display();
}

float GameLoop::GetDeltaTime() const {
    return deltaTime;
}

float GameLoop::GetGamePlayTime() const {
    return gamePlayTime;
}

bool GameLoop::IsWinConditionTriggered() const {
    return winConditionTriggered;
}

void GameLoop::ResetGameState() {
    gamePlayTime = 0.0f;
    winConditionTriggered = false;
    winDisplayTimer = 0.0f;
    enemySpawnTimer = 0.0f;
    enemyShootTimer = 0.0f;
    enemySpawnInterval = 2.0f;
    spacePressed = false;
    spaceHoldTime = 0.0f;
    activeChargingEffect = 0;
    hasChargingEffect = false;
    
    std::cout << "[GameLoop] Game state reset" << std::endl;
}

} // namespace RType::Core
