#include "GameStateManager.hpp"

#include <algorithm>
#include <iostream>

GameStateManager& GameStateManager::Instance() {
    static GameStateManager instance;
    return instance;
}

GameStateManager::GameStateManager() {
    // Start at main menu
    m_currentState = GameState::MainMenu;
    m_previousState = GameState::MainMenu;
}

void GameStateManager::SetState(GameState newState) {
    if (newState == m_currentState) {
        return;
    }

    GameState oldState = m_currentState;
    m_previousState = m_currentState;
    m_currentState = newState;

    std::cout << "[GameStateManager] State changed: " << StateToString(oldState) << " -> "
              << StateToString(newState) << std::endl;

    if (m_onStateChange) {
        m_onStateChange(oldState, newState);
    }
}

bool GameStateManager::IsInMenu() const {
    switch (m_currentState) {
        case GameState::MainMenu:
        case GameState::Options:
        case GameState::Lobby:
        case GameState::Credits:
        case GameState::Paused:
        case GameState::GameOver:
        case GameState::Victory:
            return true;
        case GameState::Playing:
            return false;
    }
    return false;
}

void GameStateManager::TogglePause() {
    if (m_currentState == GameState::Playing) {
        SetState(GameState::Paused);
    } else if (m_currentState == GameState::Paused) {
        SetState(GameState::Playing);
    }
}

void GameStateManager::GoBack() {
    // Handle specific back navigation
    switch (m_currentState) {
        case GameState::Options:
            // Go back to where we came from (MainMenu or Paused)
            SetState(m_previousState);
            break;
        case GameState::Credits:
            SetState(GameState::MainMenu);
            break;
        case GameState::Lobby:
            SetState(GameState::MainMenu);
            break;
        case GameState::Paused:
            SetState(GameState::Playing);
            break;
        case GameState::GameOver:
            SetState(GameState::MainMenu);
            break;
        case GameState::Victory:
            SetState(GameState::MainMenu);
            break;
        default:
            // No back action for MainMenu or Playing
            break;
    }
}

void GameStateManager::SetOnStateChange(StateChangeCallback callback) {
    m_onStateChange = callback;
}

std::string GameStateManager::StateToString(GameState state) {
    switch (state) {
        case GameState::MainMenu:
            return "MainMenu";
        case GameState::Playing:
            return "Playing";
        case GameState::Paused:
            return "Paused";
        case GameState::Options:
            return "Options";
        case GameState::Lobby:
            return "Lobby";
        case GameState::Credits:
            return "Credits";
        case GameState::GameOver:
            return "GameOver";
        case GameState::Victory:
            return "Victory";
    }
    return "Unknown";
}

GameState GameStateManager::StringToState(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "mainmenu" || lower == "main_menu" || lower == "menu")
        return GameState::MainMenu;
    if (lower == "playing" || lower == "play" || lower == "game")
        return GameState::Playing;
    if (lower == "paused" || lower == "pause")
        return GameState::Paused;
    if (lower == "options" || lower == "settings")
        return GameState::Options;
    if (lower == "lobby" || lower == "multiplayer")
        return GameState::Lobby;
    if (lower == "credits")
        return GameState::Credits;
    if (lower == "gameover" || lower == "game_over")
        return GameState::GameOver;
    if (lower == "victory" || lower == "win")
        return GameState::Victory;

    return GameState::MainMenu;  // Default
}
