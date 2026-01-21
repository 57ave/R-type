#include "core/GameLoop.hpp"
#include "core/GameStateCallbacks.hpp"
#include "GameStateManager.hpp"
#include "network/NetworkBindings.hpp"
#include <components/ShootEmUpTags.hpp>
#include <components/Score.hpp>
#include <components/Health.hpp>
#include <components/Damage.hpp>
#include <components/Animation.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <ecs/Components.hpp>
#include <engine/Keyboard.hpp>

using ShootEmUp::Components::Score;
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <SFML/Graphics.hpp>

namespace RType::Core {

GameLoop::GameLoop(ECS::Coordinator* coordinator)
    : coordinator(coordinator)
    , networkSystem(nullptr)
    , uiSystem(nullptr)
    , renderSystem(nullptr)
    , systemsManager(nullptr)
    , audioManager(nullptr)
    , inputHandler(nullptr)
    , gameplayManager(nullptr)
    , spawnScriptSystem(nullptr)
    , luaState(nullptr)
    , window(nullptr)
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
    , networkMode(false)
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
    
    // ✅ Gestion de la musique selon l'état du jeu
    static GameState previousState = GameState::MainMenu;
    static bool menuMusicStarted = false;  // ✅ Flag pour démarrer la musique au premier frame
    
    // ✅ Démarrer la musique du menu au premier frame si on est dans le menu
    if (!menuMusicStarted && currentState == GameState::MainMenu && audioManager) {
        std::cout << "[GameLoop] 🎵 Starting menu music (Title.ogg) at startup!" << std::endl;
        audioManager->PlayMusic("menu_music", true);
        menuMusicStarted = true;
    }
    
    if (currentState != previousState && audioManager) {
        std::cout << "[GameLoop] State changed: " << static_cast<int>(previousState) 
                  << " -> " << static_cast<int>(currentState) << std::endl;
        
        // Transition vers le menu
        if (currentState == GameState::MainMenu) {
            audioManager->FadeToMusic("menu_music", 1.0f);
            menuMusicStarted = true;
        }
        // Transition vers le jeu (depuis N'IMPORTE quel état)
        else if (currentState == GameState::Playing && previousState != GameState::Paused) {
            std::cout << "[GameLoop] 🎵 Starting stage music!" << std::endl;
            audioManager->FadeToMusic("stage_one_music", 1.5f);
        }
        // Pause
        else if (currentState == GameState::Paused) {
            audioManager->PauseMusic();
        }
        // Reprise après pause
        else if (currentState == GameState::Playing && previousState == GameState::Paused) {
            audioManager->ResumeMusic();
        }
        previousState = currentState;
    }
    
    // Créer le joueur quand on passe en mode Playing
    if (currentState == GameState::Playing && !playerCreated && !networkMode && gameplayManager) {
        player = gameplayManager->CreatePlayer(150.0f, static_cast<float>(window->getSize().y) / 2.0f, 0);
        playerCreated = true;
        std::cout << "[GameLoop] Player created: " << player << std::endl;
        
        // ✅ Initialiser l'UI du joueur (barre de vie + score)
        playerHealthBar.Init(20.0f, 20.0f, 250.0f, 30.0f);
        
        // Charger la police pour le score
        sf::Font gameFont;
        std::string fontPath = "game/assets/fonts/arial.ttf";
        // Essayer plusieurs chemins possibles
        if (!gameFont.loadFromFile(fontPath)) {
            fontPath = "../game/assets/fonts/arial.ttf";
            if (!gameFont.loadFromFile(fontPath)) {
                fontPath = "assets/fonts/arial.ttf";
                gameFont.loadFromFile(fontPath);
            }
        }
        
        if (gameFont.getInfo().family != "") {
            playerScoreUI.Init(gameFont, 20.0f, 60.0f, 32);
            gameFontLoaded = true;
            std::cout << "[GameLoop] Player UI initialized (HealthBar + Score)" << std::endl;
        } else {
            std::cerr << "[GameLoop] Warning: Could not load game font" << std::endl;
        }
    }
    
    // ✅ Mettre à jour l'UI du joueur (barre de vie + score)
    if (currentState == GameState::Playing && player != 0 && playerCreated && coordinator) {
        if (coordinator->HasComponent<Health>(player)) {
            auto& health = coordinator->GetComponent<Health>(player);
            playerHealthBar.Update(static_cast<int>(health.current), static_cast<int>(health.max));
        }
        
        if (coordinator->HasComponent<Score>(player)) {
            auto& score = coordinator->GetComponent<Score>(player);
            playerScoreUI.UpdateScore(score.currentScore);
        }
    }
    
    // Skip game logic updates when in menu states
    bool inMenu = (currentState == GameState::MainMenu || 
                   currentState == GameState::Paused ||
                   currentState == GameState::Options ||
                   currentState == GameState::Lobby ||
                   currentState == GameState::Credits);
    
    // ✅ Pause globale - arrête TOUT (animations, mouvements, etc.)
    bool isPaused = (currentState == GameState::Paused);

    // ========================================
    // HANDLE EVENTS
    // ========================================
    HandleEvents();

    // ========================================
    // UPDATE SYSTEMS ECS
    // ========================================
    if (systemsManager) {
        // ✅ En pause : ne pas mettre à jour les systèmes visuels des entités de jeu
        // (seulement le scrolling background pour l'effet visuel)
        if (!isPaused) {
            // Les systèmes visuels (animations, etc.) seulement si pas en pause
            systemsManager->UpdateVisualSystems(deltaTime);
        } else {
            // En pause : seulement le background pour l'effet visuel
            if (systemsManager->scrollingBgSystem) {
                // Ne pas mettre à jour le scrolling non plus en pause
                // systemsManager->scrollingBgSystem->Update(deltaTime);
            }
        }
        
        // Les systèmes de gameplay uniquement en jeu (et pas en pause)
        if (!inMenu && !isPaused) {
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
    if (inMenu) {
        // Gestion des entrées du menu
        if (currentState == GameState::MainMenu) {
            // ESPACE ou ENTRÉE pour démarrer le jeu
            if (eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Space) ||
                eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Enter)) {
                std::cout << "[GameLoop] Starting game from menu..." << std::endl;
                GameStateManager::Instance().SetState(GameState::Playing);
            }
        }
    } else if (inputHandler) {
        inputHandler->Update(deltaTime);
        
        // Handle charging input
        HandleChargingInput(deltaTime);
        
        // Update player movement based on input (local mode only)
        UpdatePlayerMovement(deltaTime);
    }

    // ========================================
    // UPDATE GAME LOGIC
    // ========================================
    if (!inMenu) {
        UpdateGameLogic(deltaTime);
        UpdateWinCondition(deltaTime);
        
        // ✅ Spawn enemies in solo mode
        if (!networkMode && gameplayManager) {
            UpdateEnemySpawning(deltaTime);
        }
    }

    // ========================================
    // NETWORK UPDATES (always process if connected)
    // ========================================
    // Process network packets regardless of networkMode flag
    // This allows lobby/menu communications to work before gameplay starts
    if (networkSystem) {
        // Always process incoming packets (for lobby, room list, etc.)
        networkSystem->Update(deltaTime);
        
        // Only send gameplay inputs when in Playing state and networkMode is active
        if (networkMode && !inMenu && inputHandler) {
            uint8_t inputMask = inputHandler->GetNetworkInputMask();
            networkSystem->sendInput(inputMask);
        }
    }
    
    // ========================================
    // NETWORK BINDINGS UPDATE (keepalive for lobby NetworkClient)
    // ========================================
    // This ensures the NetworkClient used by UI/Lua bindings sends keepalive pings
    RType::Network::NetworkBindings::Update(deltaTime);
    
    // ========================================
    // NETWORK PACKET PROCESSING (for lobby-specific logic)
    // ========================================
    ProcessNetworkPackets();

    // ========================================

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
        
        // ========================================
        // CHECK LOSE CONDITION FIRST (BEFORE win condition!)
        // ========================================
        // Also check directly if the local player is dead
        bool playerDead = false;
        if (player != 0 && playerCreated && coordinator) {
            if (coordinator->HasComponent<Health>(player)) {
                auto& health = coordinator->GetComponent<Health>(player);
                if (health.current <= 0) {
                    playerDead = true;
                    std::cout << "[GameLoop] 💀 Local player health is 0!" << std::endl;
                }
            }
        }
        
        if (playerDead || (gameplayManager && gameplayManager->CheckLoseCondition())) {
            std::cout << "[GameLoop] 💀 GAME OVER! Player dead!" << std::endl;
            winConditionTriggered = true;  // Prevent further checks
            
            // Transition to GameOver state
            GameStateManager::Instance().SetState(GameState::GameOver);
            
            // Play game over music
            if (audioManager) {
                audioManager->OnGameOver();
            }
            
            // Call Lua to show GameOver screen
            if (luaState) {
                sol::state& lua = luaState->GetState();
                sol::protected_function showGameOver = lua["ShowGameOverScreen"];
                if (showGameOver.valid()) {
                    int score = 0;
                    int wave = 1;
                    if (gameplayManager) {
                        score = gameplayManager->GetPlayerScore(0);
                    }
                    showGameOver(score, wave);
                }
            }
            return;
        }
        
        // ========================================
        // CHECK WIN CONDITION
        // ========================================
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
            
            // Call Lua to show Victory screen
            if (luaState) {
                sol::state& lua = luaState->GetState();
                sol::protected_function showVictory = lua["ShowVictoryScreen"];
                if (showVictory.valid()) {
                    int score = 0;
                    if (gameplayManager) {
                        score = gameplayManager->GetPlayerScore(0);
                    }
                    showVictory(score, gamePlayTime);
                }
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
                    // Play charged shot sound
                    if (audioManager) {
                        audioManager->PlaySFX("shoot_charged", 1.0f);
                    }
                } else {
                    // Normal shot
                    gameplayManager->FireMissile(playerEntity);
                    // Play normal shot sound
                    if (audioManager) {
                        audioManager->PlaySFX("shoot", 1.0f);
                    }
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

void GameLoop::UpdatePlayerMovement(float dt) {
    // Only update player movement in local (non-network) mode
    if (networkMode) return;
    if (!inputHandler) return;
    if (!coordinator) return;
    if (player == 0 || !playerCreated) return;
    
    // Check if player has Velocity component
    if (!coordinator->HasComponent<Velocity>(player)) return;
    
    auto& playerVel = coordinator->GetComponent<Velocity>(player);
    
    // Player movement speed
    const float speed = 500.0f;
    
    // Reset velocity
    playerVel.dx = 0.0f;
    playerVel.dy = 0.0f;
    
    // Apply movement based on input
    if (inputHandler->IsActionPressed(InputAction::MoveUp)) {
        playerVel.dy = -speed;
    }
    if (inputHandler->IsActionPressed(InputAction::MoveDown)) {
        playerVel.dy = speed;
    }
    if (inputHandler->IsActionPressed(InputAction::MoveLeft)) {
        playerVel.dx = -speed;
    }
    if (inputHandler->IsActionPressed(InputAction::MoveRight)) {
        playerVel.dx = speed;
    }
    
    // Update animation based on vertical movement
    if (coordinator->HasComponent<StateMachineAnimation>(player)) {
        auto& playerAnim = coordinator->GetComponent<StateMachineAnimation>(player);
        
        if (inputHandler->IsActionPressed(InputAction::MoveUp)) {
            // Moving up - tilt up animation (column 4)
            playerAnim.targetColumn = 4;
        } else if (inputHandler->IsActionPressed(InputAction::MoveDown)) {
            // Moving down - tilt down animation (column 0)
            playerAnim.targetColumn = 0;
        } else {
            // Neutral position (column 2)
            playerAnim.targetColumn = 2;
        }
    }
    
    // Clamp player position to screen bounds
    if (coordinator->HasComponent<Position>(player)) {
        auto& pos = coordinator->GetComponent<Position>(player);
        
        // Get window size (assuming 1920x1080 or use actual values)
        float maxX = 1920.0f - 100.0f;  // Account for ship width
        float maxY = 1080.0f - 50.0f;   // Account for ship height
        float minX = 0.0f;
        float minY = 0.0f;
        
        // Apply velocity with delta time for smooth movement
        pos.x += playerVel.dx * dt;
        pos.y += playerVel.dy * dt;
        
        // Clamp to screen bounds
        pos.x = std::max(minX, std::min(maxX, pos.x));
        pos.y = std::max(minY, std::min(maxY, pos.y));
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

void GameLoop::SetupCollisionCallback() {
    if (!systemsManager) {
        std::cerr << "[GameLoop] Cannot setup collision callback - no systems manager" << std::endl;
        return;
    }
    
    auto collisionSystem = systemsManager->GetCollisionSystem();
    if (!collisionSystem) {
        std::cerr << "[GameLoop] Cannot setup collision callback - no collision system" << std::endl;
        return;
    }
    
    std::cout << "[GameLoop] Setting up collision callback..." << std::endl;
    
    collisionSystem->SetCollisionCallback([this](ECS::Entity a, ECS::Entity b) {
        // ✅ EN MODE RÉSEAU: Le serveur gère TOUTES les collisions
        // Le client ne fait que du feedback visuel (sons, particules)
        if (networkMode) {
            std::cout << "[Client] Visual collision detected: " << a << " <-> " << b << " (server authoritative)" << std::endl;
            // TODO: Jouer un son de collision ou afficher des particules
            return; // ❌ PAS de destruction, PAS de dégâts en mode réseau
        }
        
        // ========================================
        // MODE LOCAL SEULEMENT: Collision autoritaire
        // ========================================
        
        // Check if player is involved
        bool aIsPlayer = coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(a);
        bool bIsPlayer = coordinator->HasComponent<ShootEmUp::Components::PlayerTag>(b);
        
        // NO INVINCIBILITY - Players can always be hit

        // Check if both entities are projectiles - ignore projectile vs projectile collisions
        bool aIsProjectile = coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(a);
        bool bIsProjectile = coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(b);
        if (aIsProjectile && bIsProjectile) {
            return;  // Projectiles don't collide with each other
        }

        // Check if both entities are enemies - ignore enemy vs enemy collisions
        bool aIsEnemy = coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(a);
        bool bIsEnemy = coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(b);
        if (aIsEnemy && bIsEnemy) {
            return;  // Enemies don't collide with each other
        }

        // Check if enemy projectile hits another enemy - ignore
        if (aIsProjectile && bIsEnemy) {
            auto& projTag = coordinator->GetComponent<ShootEmUp::Components::ProjectileTag>(a);
            if (!projTag.isPlayerProjectile) {
                return;  // Enemy projectile doesn't hit other enemies
            }
        }
        if (bIsProjectile && aIsEnemy) {
            auto& projTag = coordinator->GetComponent<ShootEmUp::Components::ProjectileTag>(b);
            if (!projTag.isPlayerProjectile) {
                return;  // Enemy projectile doesn't hit other enemies
            }
        }

        // Check for damage/health components
        bool aHasDamage = coordinator->HasComponent<Damage>(a);
        bool bHasDamage = coordinator->HasComponent<Damage>(b);
        bool aHasHealth = coordinator->HasComponent<Health>(a);
        bool bHasHealth = coordinator->HasComponent<Health>(b);

        bool significant = (aHasDamage && bHasHealth) || (bHasDamage && aHasHealth) || aIsPlayer || bIsPlayer;
        if (!significant) {
            return;
        }

        std::cout << "[Collision] Entity " << a << " <-> Entity " << b << std::endl;

        // Apply damage: B damages A
        if (aHasHealth && bHasDamage) {
            auto& health = coordinator->GetComponent<Health>(a);
            auto& damage = coordinator->GetComponent<Damage>(b);
            if (!health.invulnerable) {
                health.current -= damage.amount;
                std::cout << "[Damage] Entity " << a << " took " << damage.amount << " damage, health: " << health.current << std::endl;
            }
        }
        
        // Apply damage: A damages B
        if (bHasHealth && aHasDamage) {
            auto& health = coordinator->GetComponent<Health>(b);
            auto& damage = coordinator->GetComponent<Damage>(a);
            if (!health.invulnerable) {
                health.current -= damage.amount;
                std::cout << "[Damage] Entity " << b << " took " << damage.amount << " damage, health: " << health.current << std::endl;
            }
        }

        // Determine what died (health <= 0 after damage)
        bool aDied = false;
        bool bDied = false;
        
        // Check deaths and player hit
        if (aHasHealth) {
            auto& health = coordinator->GetComponent<Health>(a);
            if (health.current <= 0) {
                aDied = true;
                
                // ✅ Si c'est le joueur qui meurt -> GAME OVER immédiat
                if (aIsPlayer) {
                    std::cout << "[Collision] 💀 PLAYER DIED! Triggering Game Over..." << std::endl;
                    winConditionTriggered = true;
                    GameStateManager::Instance().SetState(GameState::GameOver);
                    
                    if (audioManager) {
                        audioManager->OnGameOver();
                    }
                    
                    if (luaState) {
                        sol::state& lua = luaState->GetState();
                        sol::protected_function showGameOver = lua["ShowGameOverScreen"];
                        if (showGameOver.valid()) {
                            int score = gameplayManager ? gameplayManager->GetPlayerScore(0) : 0;
                            showGameOver(score, 1);
                        }
                    }
                }
                
                if (health.destroyOnDeath && gameplayManager) {
                    gameplayManager->DestroyEntityDeferred(a);
                }
            }
        }
        if (bHasHealth) {
            auto& health = coordinator->GetComponent<Health>(b);
            if (health.current <= 0) {
                bDied = true;
                
                // ✅ Si c'est le joueur qui meurt -> GAME OVER immédiat
                if (bIsPlayer) {
                    std::cout << "[Collision] 💀 PLAYER DIED! Triggering Game Over..." << std::endl;
                    winConditionTriggered = true;
                    GameStateManager::Instance().SetState(GameState::GameOver);
                    
                    if (audioManager) {
                        audioManager->OnGameOver();
                    }
                    
                    if (luaState) {
                        sol::state& lua = luaState->GetState();
                        sol::protected_function showGameOver = lua["ShowGameOverScreen"];
                        if (showGameOver.valid()) {
                            int score = gameplayManager ? gameplayManager->GetPlayerScore(0) : 0;
                            showGameOver(score, 1);
                        }
                    }
                }
                
                if (health.destroyOnDeath && gameplayManager) {
                    gameplayManager->DestroyEntityDeferred(b);
                }
            }
        }

        // ========================================
        // SCORE SYSTEM - Award points when enemy is destroyed
        // ========================================
        if (bDied && bIsEnemy && aIsProjectile && gameplayManager) {
            auto& projTag = coordinator->GetComponent<ShootEmUp::Components::ProjectileTag>(a);
            if (projTag.isPlayerProjectile) {
                // Get enemy score value
                uint32_t pointsAwarded = 100;
                if (coordinator->HasComponent<ShootEmUp::Components::EnemyTag>(b)) {
                    auto& enemyTag = coordinator->GetComponent<ShootEmUp::Components::EnemyTag>(b);
                    pointsAwarded = enemyTag.scoreValue;
                }
                
                gameplayManager->AddScore(pointsAwarded, 0);  // Player 0 (local player)
            }
        }
        
        // Create explosion when something dies
        if (gameplayManager && (aDied || bDied)) {
            ECS::Entity deadEntity = bDied ? b : a;
            
            if (coordinator->HasComponent<Position>(deadEntity)) {
                auto& pos = coordinator->GetComponent<Position>(deadEntity);
                float centerX = pos.x;
                float centerY = pos.y;

                // Calculate sprite center
                if (coordinator->HasComponent<Sprite>(deadEntity)) {
                    auto& sprite = coordinator->GetComponent<Sprite>(deadEntity);
                    float spriteWidth = sprite.textureRect.width * sprite.scaleX;
                    float spriteHeight = sprite.textureRect.height * sprite.scaleY;
                    centerX += spriteWidth / 2.0f;
                    centerY += spriteHeight / 2.0f;
                }

                // Offset explosion to be centered
                float explosionHalfWidth = (34 * 2.5f) / 2.0f;
                float explosionHalfHeight = (35 * 2.5f) / 2.0f;
                centerX -= explosionHalfWidth;
                centerY -= explosionHalfHeight;

                gameplayManager->CreateExplosion(centerX, centerY);
            }
        }

        // Destroy projectiles on collision
        if (coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(a) && gameplayManager) {
            gameplayManager->DestroyEntityDeferred(a);
        }
        if (coordinator->HasComponent<ShootEmUp::Components::ProjectileTag>(b) && gameplayManager) {
            gameplayManager->DestroyEntityDeferred(b);
        }
    });
    
    std::cout << "[GameLoop] Collision callback configured" << std::endl;
}

void GameLoop::Render() {
    if (!window) return;

    window->clear();

    // Render game world through ECS render system
    if (renderSystem) {
        renderSystem->Update(deltaTime);
    }

    // ✅ Rendre l'UI du joueur (barre de vie + score) - visible même en pause
    if (playerCreated && player != 0) {
        if (playerHealthBar.IsInitialized()) {
            playerHealthBar.Render(window->getSFMLWindow());
        }
        if (gameFontLoaded && playerScoreUI.IsInitialized()) {
            playerScoreUI.Render(window->getSFMLWindow());
        }
    }

    // Render UI on top (menu, buttons, etc.) - le UISystem gère tout
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
    std::cout << "[GameLoop] 🔄 Resetting game state for new game..." << std::endl;
    
    // Reset all game variables
    gamePlayTime = 0.0f;
    winConditionTriggered = false;
    winDisplayTimer = 0.0f;
    playerCreated = false;
    player = 0;
    enemySpawnTimer = 0.0f;
    enemySpawnInterval = 2.0f;
    enemyShootTimer = 0.0f;
    spacePressed = false;
    spaceHoldTime = 0.0f;
    activeChargingEffect = 0;
    hasChargingEffect = false;
    
    // ✅ Supprimer toutes les entités de jeu (ennemis, projectiles, powerups)
    if (gameplayManager) {
        gameplayManager->ClearAllGameEntities();
    }
    
    // Reset UI
    playerHealthBar = HealthBarUI();
    playerScoreUI = ScoreUI();
    gameFontLoaded = false;
    
    std::cout << "[GameLoop] ✅ Game state reset complete" << std::endl;
}

void GameLoop::ProcessNetworkPackets() {
    // Network packets are processed inside NetworkSystem::Update().
    // This hook exists for future coordination between gameplay/UI and networking.
}

} // namespace RType::Core
