/**
 * MultiplayerMenuState.hpp - Multiplayer Lobby Menu (Phase 5)
 */

#pragma once

#include "GameState.hpp"
#include <ecs/Coordinator.hpp>
#include <vector>
#include <string>

class MultiplayerMenuState : public GameState
{
public:
    explicit MultiplayerMenuState(Game* game);
    ~MultiplayerMenuState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "MultiplayerMenu"; }

private:
    std::vector<ECS::Entity> menuEntities_;
    ECS::Entity hoveredButton_ = 0;
    
    // UI state
    enum class MenuMode {
        MAIN,       // Host/Join/Back buttons
        HOST,       // Hosting setup
        JOIN,       // Server browser
        LOBBY       // In a room
    };
    MenuMode currentMode_ = MenuMode::MAIN;
    
    // Network state
    std::string playerName_ = "Player";
    std::string serverAddress_ = "127.0.0.1";
    uint16_t serverPort_ = 12345;  // Match server port
    bool isReady_ = false;  // Local player ready state
    bool waitingForRoomList_ = false;  // True when we requested room list
    size_t lastRoomCount_ = 0;  // Track if room list changed
    bool isConnecting_ = false;  // True during connection process
    bool needsMenuRefresh_ = false;  // True when menu needs to be refreshed (deferred to next frame)
    bool isHandlingEvent_ = false;  // True during event handling (prevents menu refresh)
    size_t lastPlayerCount_ = 0;  // Track player count in lobby for auto-refresh
    std::vector<bool> lastReadyStates_;  // Track ready states for auto-refresh
    bool shouldStartGame_ = false;  // True when GAME_START is received, triggers PlayState transition
    
    void createMainMenu();
    void createHostMenu();
    void createJoinMenu(bool isRefresh = false);
    void createLobbyMenu();
    void clearMenu();
};
