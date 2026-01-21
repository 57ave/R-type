#ifndef ENG_ENGINE_CORE_GAMESTATECALLBACKS_HPP
#define ENG_ENGINE_CORE_GAMESTATECALLBACKS_HPP

#include <functional>
#include <string>

namespace eng {
namespace engine {
namespace core {

/**
 * @brief Abstract game state management through callbacks
 * 
 * This allows the engine to manage game states without knowing
 * the specific states of the game. The game injects its own
 * callbacks for state management.
 * 
 * Usage:
 *   GameStateCallbacks callbacks;
 *   callbacks.setState = [](const std::string& state) { ... };
 *   callbacks.getState = []() -> std::string { ... };
 *   UIBindings::SetGameStateCallbacks(callbacks);
 */
struct GameStateCallbacks {
    /// Callback to set the current game state
    std::function<void(const std::string&)> setState;
    
    /// Callback to get the current game state as string
    std::function<std::string()> getState;
    
    /// Callback to check if game is paused
    std::function<bool()> isPaused;
    
    /// Callback to check if game is playing
    std::function<bool()> isPlaying;
    
    /// Callback to toggle pause state
    std::function<void()> togglePause;
    
    /// Callback to go back to previous state
    std::function<void()> goBack;
    
    /// Callback to reset game state (clear entities, restart)
    std::function<void()> resetGame;
    
    /// Check if callbacks are properly set
    bool isValid() const {
        return setState && getState;
    }
};

} // namespace core
} // namespace engine
} // namespace eng

#endif // ENG_ENGINE_CORE_GAMESTATECALLBACKS_HPP
