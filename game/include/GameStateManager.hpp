#ifndef RTYPE_GAME_GAMESTATEMANAGER_HPP
#define RTYPE_GAME_GAMESTATEMANAGER_HPP

#include <functional>
#include <string>

/**
 * @brief Game states for R-Type
 */
enum class GameState {
    MainMenu,  // Main menu screen
    Playing,   // Active gameplay
    Paused,    // Game paused (from Playing)
    Options,   // Options/Settings menu
    Lobby,     // Multiplayer lobby
    Credits,   // Credits screen
    GameOver,  // Game over screen
    Victory    // Victory screen
};

/**
 * @brief Singleton manager for game state transitions
 *
 * Handles state changes and notifies listeners when transitions occur.
 * Can be bound to Lua for script-controlled state changes.
 */
class GameStateManager {
public:
    using StateChangeCallback = std::function<void(GameState oldState, GameState newState)>;

    /**
     * @brief Get the singleton instance
     */
    static GameStateManager& Instance();

    // Prevent copying
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    /**
     * @brief Set the current game state
     * @param newState The new state to transition to
     */
    void SetState(GameState newState);

    /**
     * @brief Get the current game state
     */
    GameState GetState() const { return m_currentState; }

    /**
     * @brief Get the previous game state
     */
    GameState GetPreviousState() const { return m_previousState; }

    /**
     * @brief Check if the game is currently paused
     */
    bool IsPaused() const { return m_currentState == GameState::Paused; }

    /**
     * @brief Check if in a menu state (not playing)
     */
    bool IsInMenu() const;

    /**
     * @brief Check if actively playing (not paused or in menu)
     */
    bool IsPlaying() const { return m_currentState == GameState::Playing; }

    /**
     * @brief Toggle pause state (Playing <-> Paused)
     */
    void TogglePause();

    /**
     * @brief Go back to previous state (useful for Options -> previous menu)
     */
    void GoBack();

    /**
     * @brief Register a callback for state changes
     * @param callback Function to call when state changes
     */
    void SetOnStateChange(StateChangeCallback callback);

    /**
     * @brief Convert GameState to string (for Lua/debugging)
     */
    static std::string StateToString(GameState state);

    /**
     * @brief Convert string to GameState (for Lua)
     */
    static GameState StringToState(const std::string& str);

private:
    GameStateManager();
    ~GameStateManager() = default;

    GameState m_currentState = GameState::MainMenu;
    GameState m_previousState = GameState::MainMenu;
    StateChangeCallback m_onStateChange;
};

#endif  // RTYPE_GAME_GAMESTATEMANAGER_HPP
